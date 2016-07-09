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
        internal::LoginDBProxyReq::descriptor();
        internal::LoginDBProxyRsp::descriptor();
        internal::QueryDBProxyReq::descriptor();
        internal::QueryDBProxyRsp::descriptor();
        internal::DBErrorRsp::descriptor();
        internal::DBProxyErrorRsp::descriptor();
        internal::LoginGatewayReq::descriptor();
        internal::LoginGatewayRsp::descriptor();
        internal::PauseWorkReq::descriptor();
        internal::ContinueWorkReq::descriptor();
        internal::ForwardMessageReq::descriptor();
        internal::ForwardMessageRsp::descriptor();
        internal::BroadcastMessageReq::descriptor();
        internal::BroadcastMessageRsp::descriptor();
    }
};

static InitProtoMessageDdasdas g_once_init;

#endif
