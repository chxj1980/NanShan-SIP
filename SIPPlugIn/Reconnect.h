#pragma once
#include "config.h"
#include "SIPPlugIn.h"

class Reconnect :
	public acl::thread
{
public:
	Reconnect(SIPHandler *pParam);
	~Reconnect();
protected:
	virtual void *run();
private:
	SIPHandler *pHandle;
};

