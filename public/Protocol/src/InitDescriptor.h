#ifndef __INIT_DESCRIPTOR_H__
#define __INIT_DESCRIPTOR_H__

#include <proto/client_link.pb.h>
#include <proto/client_logic.pb.h>
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
        cli::UserInfoRep::descriptor();
        cli::UserInfoRsp::descriptor();
        cli::LeaveReq::descriptor();
        cli::SignUpReq::descriptor();
        cli::SignUpRsp::descriptor();
        cli::SignInReq::descriptor();
        cli::SignInRsp::descriptor();
        cli::QueryPartReq::descriptor();
        cli::QueryPartRsp::descriptor();
        cli::QueryPartRsp_Part::descriptor();
        cli::EnterPartReq::descriptor();
        cli::EnterPartRsp::descriptor();
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
        svr::LoginGWReq::descriptor();
        svr::LoginGWRsp::descriptor();
        svr::GWInfoReq::descriptor();
        svr::GWInfoRsp::descriptor();
        svr::ForwardReq::descriptor();
        svr::BroadcastReq::descriptor();
        svr::GWNotify::descriptor();
        svr::LinkerLoginReq::descriptor();
        svr::LinkerLoginRsp::descriptor();
        svr::ReportLinkerReq::descriptor();
        svr::UserEnterRsp::descriptor();
        svr::UserLeaveRsp::descriptor();
        svr::CloseUser::descriptor();
        svr::LinkerForward::descriptor();
        svr::LinkerBroadcast::descriptor();
        svr::UpdateTokenReq::descriptor();
    }
};

static InitDescriptor g_once_init;

#endif
