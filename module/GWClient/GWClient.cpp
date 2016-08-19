#include "GWClient.h"
#include <iostream>
#include <ProtobufCodec.h>
#include <proto/public_struct.pb.h>
#include <proto/server_internal.pb.h>

namespace gateway
{
	static const size_t kExtrabufSize = 4096;

	/************************************************************************/

	class SessionHandle : public network::TCPSessionHandler
	{
		friend class GatewayClient;

	public:
		SessionHandle(GatewayClient *client, std::shared_ptr<bool> &life)
			: counter_(0)
			, client_(client)
			, is_logged_(false)
			, client_life_(life)
			, heartbeat_interval_(0)
		{
		}

		// 获取生命周期
		std::weak_ptr<bool>& GetLife()
		{
			return client_life_;
		}

		// 连接成功
		virtual void OnConnect() override
		{
			if (client_life_.expired())
			{
				Close();
			}
			else
			{
				network::NetMessage message;
				svr::LoginRouterReq request;
				request.mutable_node()->set_child_id(client_->GetChildNodeID());
				request.mutable_node()->set_type(static_cast<svr::NodeType>(client_->GetNodeType()));
				ProtubufCodec::Encode(&request, message);
				Send(message);
				client_->OnConnected(this);
			}
		}

		// 接收消息
		virtual void OnMessage(network::NetMessage &message) override
		{
			if (client_life_.expired())
			{
				Close();
				return;
			}

			auto response = ProtubufCodec::Decode(message);
			if (response == nullptr)
			{
				assert(false);
				return;
			}

			if (!is_logged_)
			{
				if (dynamic_cast<pub::PongRsp*>(response.get()) == nullptr)
				{
					if (dynamic_cast<svr::LoginRouterRsp*>(response.get()) != nullptr)
					{
						is_logged_ = true;
						counter_ = heartbeat_interval_ = static_cast<svr::LoginRouterRsp*>(response.get())->heartbeat_interval() / 2;
					}
					else
					{
						assert(false);
						return;
					}
				}
			}
			else if (dynamic_cast<pub::PongRsp*>(response.get()) == nullptr)
			{
				client_->OnMessage(this, response.get(), message);
			}
			else
			{
				assert(dynamic_cast<pub::PongRsp*>(response.get()) != nullptr);
			}
		}

		// 连接关闭
		virtual void OnClose() override
		{
			is_logged_ = false;
			if (!client_life_.expired())
			{
				client_->OnDisconnect(this);
			}
		}

		// 发送心跳倒计时
		void HeartbeatCountdown()
		{
			if (is_logged_ && heartbeat_interval_ > 0)
			{
				if (--counter_ == 0)
				{
					pub::PingReq request;
					network::NetMessage message;
					ProtubufCodec::Encode(&request, message);
					Send(message);
					counter_ = heartbeat_interval_;
				}
			}
		}

	private:
		GatewayClient*          client_;
		bool                is_logged_;
		std::weak_ptr<bool> client_life_;
		uint32_t            counter_;
		uint32_t            heartbeat_interval_;
	};

	/************************************************************************/
	/************************************************************************/

	class AsyncReconnectHandle : public std::enable_shared_from_this< AsyncReconnectHandle >
	{
	public:
		AsyncReconnectHandle(GatewayClient *client, std::shared_ptr<bool> &life)
			: client_(client)
			, client_life_(life)
		{
		}

		// 获取生命周期
		std::weak_ptr<bool>& GetLife()
		{
			return client_life_;
		}

		// 连接结果回调
		void ConnectCallback(asio::error_code error_code)
		{
			if (!client_life_.expired())
			{
				client_->AsyncReconnectResult(*this, error_code);
			}
		}

	private:
		GatewayClient*      client_;
		std::weak_ptr<bool> client_life_;
	};

	/************************************************************************/
	/************************************************************************/

	network::MessageFilterPointer CreaterMessageFilter()
	{
		return std::make_shared<network::DefaultMessageFilter>();
	}

	GatewayClient::GatewayClient(network::IOServiceThreadManager &threads, asio::ip::tcp::endpoint &endpoint, size_t connection_num, int node_type, int child_id)
		: threads_(threads)
		, connecting_num_(0)
		, endpoint_(endpoint)
		, child_id_(child_id)
		, node_type_(node_type)
		, next_client_index_(0)
		, connection_num_(connection_num)
		, timer_(threads.MainThread()->IOService(), std::chrono::seconds(1))
		, wait_handler_(std::bind(&GatewayClient::OnUpdateTimer, this, std::placeholders::_1))
		, session_handle_creator_(threads_, std::bind(&GatewayClient::CreateSessionHandle, this), std::bind(CreaterMessageFilter))
	{
		assert(connection_num > 0);
		assert(node_type_ >= svr::NodeType_MIN && node_type_ <= svr::NodeType_MAX);
		InitConnections();
		timer_.async_wait(wait_handler_);
	}

	GatewayClient::~GatewayClient()
	{
		Clear();
	}

	// 服务器节点类型
	int GatewayClient::GetNodeType() const
	{
		return node_type_;
	}

	// 服务器子节点id
	int GatewayClient::GetChildNodeID() const
	{
		return child_id_;
	}

	// 设置消息回调
	void GatewayClient::SetMessageCallback(const Callback &cb)
	{
		message_cb_ = cb;
	}

	// 清理所有连接
	void GatewayClient::Clear()
	{
		lifetimes_.clear();
		for (auto session : session_handle_lists_)
		{
			if (session != nullptr)
			{
				session->Close();
			}
		}
		session_handle_lists_.clear();
	}

	// 创建会话处理器
	network::SessionHandlePointer GatewayClient::CreateSessionHandle()
	{
		auto life = std::make_shared<bool>();
		lifetimes_.insert(life);
		return std::make_shared<SessionHandle>(this, life);
	}

	// 初始化连接
	void GatewayClient::InitConnections()
	{
		asio::error_code error_code;
		for (size_t i = 0; i < connection_num_; ++i)
		{
			try
			{
				session_handle_creator_.Connect(endpoint_, error_code);
			}
			catch (const std::exception&)
			{
				Clear();
				throw ConnectGatewayFail(error_code.message().c_str());
			}

			if (error_code)
			{
				Clear();
				throw ConnectGatewayFail(error_code.message().c_str());
			}
		}
	}

	// 重新连接
	void GatewayClient::AsyncReconnect()
	{
		if (session_handle_lists_.size() < connection_num_)
		{
			const int diff = connection_num_ - session_handle_lists_.size();
			assert(connecting_num_ <= diff);
			if (connecting_num_ < diff)
			{
				const int lack = diff - connecting_num_;
				for (int i = 0; i < lack; ++i)
				{
					++connecting_num_;

					auto life = std::make_shared<bool>();
					lifetimes_.insert(life);

					auto handler = std::make_shared<AsyncReconnectHandle>(this, life);
					session_handle_creator_.AsyncConnect(endpoint_, std::bind(&AsyncReconnectHandle::ConnectCallback, std::move(handler), std::placeholders::_1));
				}
			}
		}
	}

	// 异步重连结果
	void GatewayClient::AsyncReconnectResult(AsyncReconnectHandle &handler, asio::error_code error_code)
	{
		if (error_code)
		{
			assert(connecting_num_ > 0);
			if (connecting_num_ > 0)
			{
				--connecting_num_;
			}
		}
		else
		{
			++connecting_num_;
		}
		lifetimes_.erase(handler.GetLife().lock());
	}

	// 获取会话处理器
	SessionHandle* GatewayClient::GetSessionHandle()
	{
		if (session_handle_lists_.size() < connection_num_)
		{
			AsyncReconnect();
			if (session_handle_lists_.empty())
			{
				assert(false);
				return nullptr;
			}
		}

		if (next_client_index_ >= session_handle_lists_.size())
		{
			next_client_index_ = 0;
		}
		return session_handle_lists_[next_client_index_++];
	}

	// 更新计时器
	void GatewayClient::OnUpdateTimer(asio::error_code error_code)
	{
		if (error_code)
		{
			std::cerr << error_code.message() << std::endl;
			return;
		}

		for (auto session : session_handle_lists_)
		{
			session->HeartbeatCountdown();
		}
		timer_.expires_from_now(std::chrono::seconds(1));
		timer_.async_wait(wait_handler_);
	}

	// 连接事件
	void GatewayClient::OnConnected(SessionHandle *session)
	{
		if (session_handle_lists_.size() >= connection_num_)
		{
			assert(false);
			session->Close();
			return;
		}

		if (std::find(session_handle_lists_.begin(), session_handle_lists_.end(), session) == session_handle_lists_.end())
		{
			session_handle_lists_.push_back(session);
		}
	}

	// 断开连接事件
	void GatewayClient::OnDisconnect(SessionHandle *session)
	{
		lifetimes_.erase(session->GetLife().lock());
		auto session_iter = std::find(session_handle_lists_.begin(), session_handle_lists_.end(), session);
		if (session_iter != session_handle_lists_.end())
		{
			session_handle_lists_.erase(session_iter);
		}
	}

	// 接受消息事件
	void GatewayClient::OnMessage(SessionHandle *session, google::protobuf::Message *message, network::NetMessage &buffer)
	{
		if (dynamic_cast<svr::RouterNotify*>(message) != nullptr)
		{
			auto response = static_cast<svr::RouterNotify*>(message);
			context_.node_type = response->src().type();
			context_.child_id = response->src().child_id();

			buffer.Clear();
			buffer.Write(response->user_data().c_str(), response->user_data().size());
			auto forward_message = ProtubufCodec::Decode(buffer);
			assert(forward_message != nullptr);
			if (forward_message != nullptr && message_cb_ != nullptr)
			{
				message_cb_(this, forward_message.get(), buffer);
			}
		}
		else if (dynamic_cast<pub::ErrorRsp*>(message) != nullptr)
		{
			auto response = static_cast<pub::ErrorRsp*>(message);
			std::cerr << "gateway error code: " << response->error_code() << std::endl;
		}
		else
		{
			assert(false);
		}
	}

	// 回复消息
	void GatewayClient::Reply(google::protobuf::Message *message)
	{
		Send(context_.node_type, context_.child_id, message);
	}

	void GatewayClient::Reply(google::protobuf::Message *message, network::NetMessage &buffer)
	{
		Send(context_.node_type, context_.child_id, message, buffer);
	}

	// 发送消息
	void GatewayClient::Send(int dst_node_type, int dst_child_id, google::protobuf::Message *message)
	{
		network::NetMessage buffer;
		Send(dst_node_type, dst_child_id, message, buffer);
	}

	void GatewayClient::Send(int dst_node_type, int dst_child_id, google::protobuf::Message *message, network::NetMessage &buffer)
	{
		assert(dst_node_type >= svr::NodeType_MIN && dst_node_type <= svr::NodeType_MAX);
		SessionHandle *session = GetSessionHandle();
		if (session != nullptr)
		{
			buffer.Clear();
			std::vector<char> byte_array;
			std::array<char, kExtrabufSize> extrabuf;
			ProtubufCodec::Encode(message, buffer);
			const size_t byte_size = buffer.Readable();
			if (byte_size <= extrabuf.size())
			{
				memcpy(extrabuf.data(), buffer.Data(), byte_size);
			}
			else
			{
				byte_array.resize(byte_size);
				memcpy(byte_array.data(), buffer.Data(), byte_size);
			}

			buffer.Clear();
			svr::ForwardReq request;
			request.mutable_dst()->set_type(static_cast<svr::NodeType>(dst_node_type));
			request.mutable_dst()->set_child_id(dst_child_id);
			request.set_user_data(byte_array.empty() ? extrabuf.data() : byte_array.data(), byte_size);
			ProtubufCodec::Encode(&request, buffer);
			session->Send(buffer);
		}
	}

	// 广播消息
	void GatewayClient::Broadcast(const std::vector<int> &dst_type_lists, google::protobuf::Message *message)
	{
		network::NetMessage buffer;
		Broadcast(dst_type_lists, message, buffer);
	}

	void GatewayClient::Broadcast(const std::vector<int> &dst_type_lists, google::protobuf::Message *message, network::NetMessage &buffer)
	{
		SessionHandle *session = GetSessionHandle();
		if (session != nullptr)
		{
			buffer.Clear();
			std::vector<char> byte_array;
			std::array<char, kExtrabufSize> extrabuf;
			ProtubufCodec::Encode(message, buffer);
			const size_t byte_size = buffer.Readable();
			if (byte_size <= extrabuf.size())
			{
				memcpy(extrabuf.data(), buffer.Data(), byte_size);
			}
			else
			{
				byte_array.resize(byte_size);
				memcpy(byte_array.data(), buffer.Data(), byte_size);
			}

			buffer.Clear();
			svr::BroadcastReq request;
			for (size_t i = 0; i < dst_type_lists.size(); ++i)
			{
				int dst_node_type = dst_type_lists[i];
				assert(dst_node_type >= svr::NodeType_MIN && dst_node_type <= svr::NodeType_MAX);
				request.add_dst_lists(static_cast<svr::NodeType>(dst_node_type));
			}
			request.set_user_data(byte_array.empty() ? extrabuf.data() : byte_array.data(), byte_size);
			ProtubufCodec::Encode(&request, buffer);
			session->Send(buffer);
		}
	}
}