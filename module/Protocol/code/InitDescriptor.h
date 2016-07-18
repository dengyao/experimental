#ifndef __INIT_DESCRIPTOR_H__
#define __INIT_DESCRIPTOR_H__

#include <proto/client_login.pb.h>
#include <proto/public_enum.pb.h>
#include <proto/public_struct.pb.h>
#include <proto/server_internal.pb.h>

class InitDescriptor
{
public:
    InitDescriptor()
    {
        login::SignUpReq::descriptor();
        login::SignUpRsp::descriptor();
        login::SignInReq::descriptor();
        login::SignInRsp::descriptor();
        login::QueryPartitionReq::descriptor();
        login::QueryPartitionRsp::descriptor();
        login::QueryPartitionRsp_Partition::descriptor();
        login::EntryPartitionReq::descriptor();
        login::EntryPartitionRsp::descriptor();
        pub::PingReq::descriptor();
        pub::PongRsp::descriptor();
        pub::ErrorRsp::descriptor();
        svr::LoginDBAgentReq::descriptor();
        svr::LoginDBAgentRsp::descriptor();
        svr::DBAgentInfoReq::descriptor();
        svr::DBAgentInfoRsp::descriptor();
        svr::QueryDBAgentReq::descriptor();
        svr::QueryDBAgentRsp::descriptor();
        svr::DBErrorRsp::descriptor();
        svr::DBAgentErrorRsp::descriptor();
        svr::ChildNode::descriptor();
        svr::LoginRouterReq::descriptor();
        svr::LoginRouterRsp::descriptor();
        svr::RouterInfoReq::descriptor();
        svr::RouterInfoRsp::descriptor();
        svr::ForwardReq::descriptor();
        svr::BroadcastReq::descriptor();
        svr::RouterNotify::descriptor();
        svr::RegisterLinkerReq::descriptor();
        svr::RegisterLinkerRsp::descriptor();
        svr::UpdateLinkerCapacityReq::descriptor();
    }
};

static InitDescriptor g_once_init;

#endif
