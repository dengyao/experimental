#ifndef __INIT_DESCRIPTOR_H__
#define __INIT_DESCRIPTOR_H__

#include <proto/client_link.pb.h>
#include <proto/client_login.pb.h>
#include <proto/public_enum.pb.h>
#include <proto/public_struct.pb.h>
#include <proto/server_internal.pb.h>

class InitDescriptor
{
public:
    InitDescriptor()
    {
        cli::UserAuthReq::descriptor();
        cli::UserAuthRsp::descriptor();
        cli::SignUpReq::descriptor();
        cli::SignUpRsp::descriptor();
        cli::SignInReq::descriptor();
        cli::SignInRsp::descriptor();
        cli::QueryPartitionReq::descriptor();
        cli::QueryPartitionRsp::descriptor();
        cli::QueryPartitionRsp_Partition::descriptor();
        cli::EntryPartitionReq::descriptor();
        cli::EntryPartitionRsp::descriptor();
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
        svr::LinkerLoginReq::descriptor();
        svr::LinkerLoginRsp::descriptor();
        svr::ReportLinkerReq::descriptor();
        svr::UpdateTokenReq::descriptor();
    }
};

static InitDescriptor g_once_init;

#endif
