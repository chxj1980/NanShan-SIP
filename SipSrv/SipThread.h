#pragma once
#include "SipService.h"


class scanf_thread : public acl::thread
{
public:
	scanf_thread(void *pUser);
	~scanf_thread();
protected:
	virtual void *run();
private:
	SipService *m_pSip;
};

//任务处理线程池
class sip_thread_pool : public acl::thread_pool
{
public:
	sip_thread_pool();
	~sip_thread_pool();
protected:
	virtual bool thread_on_init();
	virtual void thread_on_exit();
};

//任务处理子线程
class sip_handle_thread : public acl::thread
{
public:
	sip_handle_thread(socket_stream* sock_stream, const char *data, size_t dlen, void *sip);
	~sip_handle_thread();
protected:
	virtual void *run();
private:
	SipService *m_pSip;
	socket_stream m_sock_stream;
	char m_pData[SIP_RECVBUF_MAX];
	size_t m_nDlen;
};

//注册管理线程
class regmnger_thread : public acl::thread
{
public:
	regmnger_thread(void *pUser);
	~regmnger_thread();
protected:
	virtual void *run();
private:
	SipService *m_pSip;
};

//会话管理线程
class sesmnger_thread : public acl::thread
{
public:
	sesmnger_thread(void *pUser);
	~sesmnger_thread();
protected:
	virtual void *run();
private:
	SipService *m_pSip;
};

//消息管理线程
class msgmnger_thread : public acl::thread
{
public:
	msgmnger_thread(void *pUser);
	~msgmnger_thread();
protected:
	virtual void *run();
private:
	SipService *m_pSip;
};

//目录推送线程
class catalog_thread : public acl::thread
{
public:
	catalog_thread(SipService *sip, session_info *session, int type=0);
	~catalog_thread();
protected:
	virtual void *run();
private:
	SipService *m_pSip;
	session_info *m_pSession;
	int			m_nType;
};

//目录更新线程
class catalog_update : public acl::thread
{
public:
	catalog_update(SipService *sip);
	~catalog_update();
protected:
	virtual void *run();
private:
	SipService *m_pSip;
};