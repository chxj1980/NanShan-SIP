#pragma once  

#include "SipService.h"

class SipRuner :
	public acl::thread
{
public:
	SipRuner();
	~SipRuner();
protected:
	virtual void *run();
public:
	static SipService m_sip;
};

