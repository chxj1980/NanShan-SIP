#include "stdafx.h"
#include "SipRuner.h"

SipService SipRuner::m_sip;

SipRuner::SipRuner()
{
}


SipRuner::~SipRuner()
{
}

void * SipRuner::run()
{

#ifdef _WIN32
	//windows获取物理IP
	getIp(m_sip.m_conf.local_addr);
#endif
	//根据IP生成SIP编号
	sockaddr_in saddr;
	acl_inet_pton(AF_INET, m_sip.m_conf.local_addr, (sockaddr *)&saddr);
	srand(saddr.sin_addr.s_addr);
	acl::string strLocalCode(m_sip.m_conf.local_code, 14);
	strLocalCode.format_append("%06d", rand());
	memcpy(m_sip.m_conf.local_code, strLocalCode.c_str(), strLocalCode.size());
	//启动服务
	char szLocalAddr[128] = { 0 };
	sprintf(szLocalAddr, "%s:%d", m_sip.m_conf.local_addr, m_sip.m_conf.local_port);
	m_sip.m_conf.log(LOG_INFO, "绑定地址[%s]成功!", szLocalAddr);
	m_sip.run_alone(szLocalAddr, NULL, 0);
	delete this;
	return NULL;
}
