#include "TCPSession.h"
#include <cassert>
#include <iostream>
#include "IOServiceThread.h"
#include "TCPSessionHandle.h"
#include "IOServiceThreadManager.h"
#include "MessageFilterInterface.h"

namespace eddy
{
	namespace
	{
		typedef std::shared_ptr< std::vector<NetMessage> > NetMessageVecPointer;

		void SendMessageListToHandler(IOServiceThreadManager &manager, TCPSessionID id, NetMessageVecPointer messages)
		{
			SessionHandlerPointer handler_ptr = manager.SessionHandler(id);
			if (handler_ptr == nullptr)
			{
				return;
			}

			for (auto &message : *messages)
			{
				handler_ptr->OnMessage(message);
			}
		}

		void PackMessageList(SessionPointer session_ptr)
		{
			if (session_ptr->MessagesReceived().empty())
			{
				return;
			}

			NetMessageVecPointer messages = std::make_shared< std::vector<NetMessage> >(std::move(session_ptr->MessagesReceived()));
			session_ptr->Thread()->ThreadManager().MainThread()->Post(std::bind(
				SendMessageListToHandler, std::ref(session_ptr->Thread()->ThreadManager()), session_ptr->ID(), messages));
		}

		void SendMessageListDirectly(SessionPointer session_ptr)
		{
			SessionHandlerPointer handler_ptr = session_ptr->Thread()->ThreadManager().SessionHandler(session_ptr->ID());
			if (handler_ptr == nullptr)
			{
				return;
			}

			for (auto &message : session_ptr->MessagesReceived())
			{
				handler_ptr->OnMessage(message);
			}
			session_ptr->MessagesReceived().clear();
		}
	}

	TCPSession::TCPSession(ThreadPointer &thread_ptr, MessageFilterPointer &filter)
		: closed_(false)
		, filter_(filter)
		, num_handlers_(0)
		, thread_(thread_ptr)
		, socket_(thread_ptr->IOService())
		, session_id_(IDGenerator::kInvalidID)
	{
	}

	inline void TCPSession::UpdateLastActivity()
	{
		last_activity_ = std::chrono::steady_clock::now();
	}

	void TCPSession::Init(TCPSessionID id)
	{
		assert(IDGenerator::kInvalidID != id);

		SetSessionID(id);
		UpdateLastActivity();
		thread_->SessionQueue().Add(shared_from_this());

		asio::ip::tcp::no_delay option(true);
		socket_.set_option(option);

		size_t bytes_wanna_read = filter_->BytesWannaRead();
		if (bytes_wanna_read == 0)
		{
			return;
		}

		++num_handlers_;
		if (bytes_wanna_read == filter_->AnyBytes())
		{
			buffer_receiving_.resize(NetMessage::kDynamicThreshold);
			socket_.async_read_some(asio::buffer(buffer_receiving_.data(), buffer_receiving_.size()),
				std::bind(&TCPSession::HandleRead, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
		}
		else
		{
			buffer_receiving_.resize(bytes_wanna_read);
			socket_.async_receive(asio::buffer(buffer_receiving_.data(), bytes_wanna_read),
				std::bind(&TCPSession::HandleRead, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
		}
	}

	void TCPSession::Close()
	{
		if (closed_)
		{
			return;
		}
		closed_ = true;
		HanldeClose();
	}

	void TCPSession::HanldeClose()
	{
		if (num_handlers_ > 0)
		{
			return;
		}

		thread_->ThreadManager().MainThread()->Post(
			std::bind(&IOServiceThreadManager::OnSessionClose, &thread_->ThreadManager(), ID()));

		asio::error_code error_code;
		socket_.shutdown(asio::ip::tcp::socket::shutdown_both, error_code);
		if (error_code && error_code != asio::error::not_connected)
		{
			std::cerr << error_code.message() << std::endl;
		}

		socket_.close();
		thread_->SessionQueue().Remove(ID());
	}

	void TCPSession::PostMessageList(const std::vector<NetMessage> &messages)
	{
		if (closed_)
		{
			return;
		}

		assert(!messages.empty());

		size_t bytes_wanna_write = filter_->BytesWannaWrite(messages);
		if (bytes_wanna_write == 0)
		{
			return;
		}

		buffer_to_be_sent_.resize(buffer_to_be_sent_.size() + bytes_wanna_write);
		filter_->Write(messages, buffer_to_be_sent_);

		if (buffer_sending_.empty())
		{
			++num_handlers_;
			buffer_sending_.swap(buffer_to_be_sent_);
			socket_.async_send(asio::buffer(buffer_sending_.data(), bytes_wanna_write),
				std::bind(&TCPSession::HanldeWrite, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
		}
	}

	void TCPSession::HandleRead(asio::error_code error_code, size_t bytes_transferred)
	{
		--num_handlers_;
		assert(num_handlers_ >= 0);

		if (error_code || closed_)
		{
			closed_ = true;
			HanldeClose();
			return;
		}

		bool wanna_post = messages_received_.empty();
		size_t bytes_read = filter_->Read(buffer_receiving_, messages_received_);
		assert(bytes_read == bytes_transferred);
		buffer_receiving_.clear();
		wanna_post = wanna_post && !messages_received_.empty();

		if (wanna_post)
		{
			if (thread_->ID() == thread_->ThreadManager().MainThread()->ID())
			{
				thread_->Post(std::bind(SendMessageListDirectly, shared_from_this()));
			}
			else
			{
				thread_->Post(std::bind(PackMessageList, shared_from_this()));
			}
			UpdateLastActivity();
		}

		size_t bytes_wanna_read = filter_->BytesWannaRead();
		if (bytes_wanna_read == 0)
		{
			return;
		}

		++num_handlers_;
		if (bytes_wanna_read == filter_->AnyBytes())
		{
			buffer_receiving_.resize(NetMessage::kDynamicThreshold);
			socket_.async_read_some(asio::buffer(buffer_receiving_.data(), buffer_receiving_.size()),
				std::bind(&TCPSession::HandleRead, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
		}
		else
		{
			buffer_receiving_.resize(bytes_wanna_read);
			socket_.async_receive(asio::buffer(buffer_receiving_.data(), bytes_wanna_read),
				std::bind(&TCPSession::HandleRead, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
		}
	}

	void TCPSession::HanldeWrite(asio::error_code error_code, size_t bytes_transferred)
	{
		--num_handlers_;
		assert(num_handlers_ >= 0);

		if (error_code || closed_)
		{
			closed_ = true;
			HanldeClose();
			return;
		}

		buffer_sending_.clear();

		if (buffer_to_be_sent_.empty())
		{
			return;
		}

		++num_handlers_;
		buffer_sending_.swap(buffer_to_be_sent_);
		socket_.async_send(asio::buffer(buffer_sending_.data(), buffer_sending_.size()),
			std::bind(&TCPSession::HanldeWrite, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
	}
}