#ifndef __DBCLIENT_H__
#define __DBCLIENT_H__

#include "eddy.h"

class DBClient : public eddy::TCPSessionHandle
{
public:
	virtual void OnConnect() override;

	virtual void OnMessage(eddy::NetMessage &message) override;

	virtual void OnClose() override;
};

#endif
