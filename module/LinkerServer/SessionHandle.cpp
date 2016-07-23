#include "SessionHandle.h"
#include <ProtobufCodec.h>
#include <proto/client_link.pb.h>
#include <proto/public_struct.pb.h>
#include "LinkerManager.h"

SessionHandle::SessionHandle(LinkerManager &linker_manager)
	: linker_manager_(linker_manager)
{
}

bool SessionHandle::IsAuthTimeout() const
{
	return std::chrono::steady_clock::now() - connect_time_ >= std::chrono::seconds(1);
}

void SessionHandle::OnConnect()
{
	connect_time_ = std::chrono::steady_clock::now();
}

void SessionHandle::OnMessage(network::NetMessage &message)
{
	// 解析消息
	auto request = ProtubufCodec::Decode(message);
	if (request == nullptr)
	{
		return linker_manager_.RespondErrorCodeToUser(this, message, pub::kInvalidProtocol);
	}

	// 处理心跳
	if (dynamic_cast<pub::PingReq*>(request.get()) != nullptr)
	{
		message.Clear();
		pub::PongRsp response;
		ProtubufCodec::Encode(&response, message);	
		return Send(message);
	}

	// 处理请求
	linker_manager_.HandleMessageFromUser(this, request.get(), message);
}

void SessionHandle::OnClose()
{
	linker_manager_.HandleUserClose(this);
}

/************************************************************************/
// 创建消息筛选器
network::MessageFilterPointer CreateMessageFilter()
{
	return std::make_shared<network::DefaultMessageFilter>();
}

network::SessionHandlePointer CreateSessionHandle(LinkerManager &linker_manager)
{
	return std::make_shared<SessionHandle>(linker_manager);
}
/************************************************************************/