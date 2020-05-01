
#pragma once
#include "RtpService.h"

class scanf_thread : public acl::thread
{
public:
	scanf_thread(void *pUser);
	~scanf_thread();
protected:
	virtual void *run();
private:
	RtpService *m_pRtp;
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
	sip_handle_thread(socket_stream* sock_stream, const char *data, size_t dlen, void *rtp);
	~sip_handle_thread();
protected:
	virtual void *run();
private:
	RtpService *m_pRtp;
	socket_stream m_sock_stream;
	size_t m_nDlen;
	char m_pData[SIP_RECVBUF_MAX];
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
	RtpService *m_pRtp;
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
	RtpService *m_pRtp;
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
	RtpService *m_pRtp;
};



//RTP发送ACL_SOCKET
class rtp_sender
{
public:
	rtp_sender(const char *ip, int port, int tranfer);
	~rtp_sender();
public:
	void add(addr_info &addr);
	bool status();
	bool link();
	void close();
	int send_data(unsigned char *buffer, size_t bufsize);				//for SDK
	int send_data(unsigned char *buffer, size_t bufsize, int tranfer);	//for 28181
public:
	bool activate;
	time_t dormant_time;

	ACL_SOCKET stream;
	ACL_SOCKET client_stream;

	void *rtcp_alive;

	int rtpseq;
	int send_bufsize;
	int sendtranfer;
	int sendssrc;
	int sendport;
	char sendip[16];
	unsigned char rtp_buffer[1500];
	unsigned char send_buffer[102400];
	addr_info sendaddr;
	struct sockaddr_in client_addr;
};

//RTCP接收线程
class rtcp_recv_thread : public acl::thread
{
public:
	rtcp_recv_thread(int rtcpport, rtp_sender * sender);
	~rtcp_recv_thread();
protected:
	virtual void *run();
public:
	void stop();
public:
	bool bRunning;
	ACL_SOCKET stream;
	rtp_sender *pSender;
};

//RTP接收线程
class rtp_recv_thread : public acl::thread
{
public:
	rtp_recv_thread(const char *ip, int port, int tranfer, void* rtp);
	~rtp_recv_thread();
protected:
	virtual void *run();
public:
	void stop();
	void dormant();
	bool get_status();
	void set_playinfo(const char *callid, const char *deviceid, int playtype);
	void set_mediainfo(addr_info *addr);
	int client_size();
	int add_client(const char *callid, rtp_sender * sender);
	int del_client(const char *callid);
	void cls_client();
	void send_rtcp_data(sockaddr *to = NULL);
	bool tranfer_data(unsigned char *buffer, size_t bufsize);
public:
	RtpService *rtpsrv;

	ACL_SOCKET stream;
	ACL_SOCKET rtcp_stream;
	ACL_SOCKET client_stream;

	std::map<std::string, rtp_sender *> client_list;
	acl::locker							client_locker;
	std::vector<addr_info>				media_list;
	acl::locker							media_locker;

	bool running;
	bool connectd;
	bool activate;
	time_t rtcp_time;
	time_t dormant_time;

	int invite_type;
	int media_type;
	int playload;
	int rtptranfer;
	int rtpport;
	char rtpip[16];
	char rtp_callid[64];
	char rtp_deviceid[21];
	char stream_type[64];
	char stream_ssrc[11];

};

