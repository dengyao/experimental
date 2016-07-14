#ifndef __INIT_PROTO_DESCRIPTOR_H__
#define __INIT_PROTO_DESCRIPTOR_H__

#include <proto/externall.protocol.pb.h>
#include <proto/internal.protocol.pb.h>

class InitProtoMessageDdasdas
{
public:
    InitProtoMessageDdasdas()
    {
        internal::PingReq::descriptor();
        internal::PongRsp::descriptor();
        internal::LoginDBAgentReq::descriptor();
        internal::LoginDBAgentRsp::descriptor();
        internal::DBAgentInfoReq::descriptor();
        internal::DBAgentInfoRsp::descriptor();
        internal::QueryDBAgentReq::descriptor();
        internal::QueryDBAgentRsp::descriptor();
        internal::DBErrorRsp::descriptor();
        internal::DBAgentErrorRsp::descriptor();
        internal::RouterErrorRsp::descriptor();
        internal::LoginRouterReq::descriptor();
        internal::LoginRouterRsp::descriptor();
        internal::ForwardMessageReq::descriptor();
        internal::ForwardMessageRsp::descriptor();
        internal::BroadcastMessageReq::descriptor();
        internal::BroadcastMessageRsp::descriptor();
    }
};

static InitProtoMessageDdasdas g_once_init;

#endif
