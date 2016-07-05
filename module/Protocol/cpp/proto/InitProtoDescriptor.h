﻿#ifndef __INIT_PROTO_DESCRIPTOR_H__
#define __INIT_PROTO_DESCRIPTOR_H__

#include <proto/global.Heartbeat.pb.h>
#include <proto/dbproxy/dbproxy.Request.pb.h>
#include <proto/dbproxy/dbproxy.Response.pb.h>

class InitProtoMessageDdasdas
{
public:
    InitProtoMessageDdasdas()
    {
        proto_global::Heartbeat::descriptor();
        proto_dbproxy::Request::descriptor();
        proto_dbproxy::DBError::descriptor();
        proto_dbproxy::ProxyError::descriptor();
        proto_dbproxy::Response::descriptor();
    }
};

static InitProtoMessageDdasdas g_once_init;

#endif
