#ifndef __INIT_DESCRIPTOR_H__
#define __INIT_DESCRIPTOR_H__

#include <proto/internal.pb.h>

class InitDescriptor
{
public:
    InitDescriptor()
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
        internal::RouterInfoReq::descriptor();
        internal::RouterInfoRsp::descriptor();
        internal::ForwardReq::descriptor();
        internal::BroadcastReq::descriptor();
        internal::RouterNotify::descriptor();
    }
};

static InitDescriptor g_once_init;

#endif
