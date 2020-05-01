#include "stdafx.h"
#include "RtpThread.h"

#define		RTP_CONNTECTTIMEOUT		6
#define		RTP_ACCEPTTIMEOUT		6
#define		RTP_RW_TIMEOUT			10


scanf_thread::scanf_thread(void *pUser)
{
	m_pRtp = (RtpService *)pUser;
}
scanf_thread::~scanf_thread()
{
	m_pRtp = NULL;
}
void *scanf_thread::run()
{
	if (m_pRtp)
		m_pRtp->scanf_command();
	delete this;
	return NULL;
}

//任务处理线程池
sip_thread_pool::sip_thread_pool()
{
}
sip_thread_pool::~sip_thread_pool()
{
}
bool sip_thread_pool::thread_on_init()
{
	//printf("thread_on_init curtid: %lu\n", acl::thread::thread_self());
	return true;
}
void sip_thread_pool::thread_on_exit()
{
	//printf("thread_on_exit curtid: %lu\n", acl::thread::thread_self());
}

//任务处理子线程
sip_handle_thread::sip_handle_thread(socket_stream* sock_stream, const char *data, size_t dlen, void *rtp)
{
	m_pRtp = (RtpService *)rtp;
	memset(&m_sock_stream, 0, sizeof(m_sock_stream));
	memcpy(&m_sock_stream, sock_stream, sizeof(m_sock_stream));
	memset(m_pData, 0, SIP_RECVBUF_MAX);
	memcpy(m_pData, data, dlen);
	m_nDlen = dlen;
}
sip_handle_thread::~sip_handle_thread()
{
	m_pRtp = NULL;
}
void *sip_handle_thread::run()
{
	if (m_pRtp)
	{
		m_pRtp->on_handle(&m_sock_stream, m_pData, m_nDlen);
	}
	delete this;
	return NULL;
}

//注册管理线程
regmnger_thread::regmnger_thread(void *pUser)
{
	m_pRtp = (RtpService *)pUser;
}
regmnger_thread::~regmnger_thread()
{
	m_pRtp = NULL;
}
void *regmnger_thread::run()
{
	if (m_pRtp)
		m_pRtp->register_manager();
	delete this;
	return NULL;
}

//会话管理线程
sesmnger_thread::sesmnger_thread(void *pUser)
{
	m_pRtp = (RtpService *)pUser;
}
sesmnger_thread::~sesmnger_thread()
{
	m_pRtp = NULL;
}
void *sesmnger_thread::run()
{
	if (m_pRtp)
		m_pRtp->session_manager();
	delete this;
	return NULL;
}

//消息管理线程
msgmnger_thread::msgmnger_thread(void *pUser)
{
	m_pRtp = (RtpService *)pUser;
}
msgmnger_thread::~msgmnger_thread()
{
	m_pRtp = NULL;
}
void *msgmnger_thread::run()
{
	if (m_pRtp)
		m_pRtp->message_manager();
	delete this;
	return NULL;
}

rtp_sender::rtp_sender(const char *ip, int port, int tranfer)
{
	memset(&sendaddr, 0, sizeof(sendaddr));
	memset(&client_addr, 0, sizeof(client_addr));
	memset(rtp_buffer, 0, sizeof(rtp_buffer));
	memset(send_buffer, 0, sizeof(send_buffer));
	memset(sendip, 0, sizeof(sendip));
	strcpy(sendip, ip);
	send_bufsize = 0;
	dormant_time = 0;
	sendport = port;
	sendtranfer = tranfer;
	sendssrc = 0;
	rtpseq = 1;
	activate = false;

	rtcp_alive = NULL;
	stream = ACL_SOCKET_INVALID;
	client_stream = ACL_SOCKET_INVALID;

	if (tranfer == SIP_TRANSFER_UDP)
	{
		stream = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		rtcp_alive = new rtcp_recv_thread(port + 1, this);
		((rtcp_recv_thread *)rtcp_alive)->set_detachable(true);
		((rtcp_recv_thread *)rtcp_alive)->start();
	}
	else
		stream = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	int recv_timeot = RTP_RW_TIMEOUT * 1000;
	setsockopt(stream, SOL_SOCKET, SO_RCVTIMEO, (char *)&recv_timeot, sizeof(int));
	while (true)
	{
		sockaddr_in source = { 0 };
		source.sin_family = AF_INET;
		source.sin_addr.s_addr = inet_addr(ip);
		source.sin_port = htons(sendport);
		if (bind(stream, (sockaddr*)&source, sizeof(source)) == 0)
		{
			printf("inet_bind: send %s:%d ok\n", ip, sendport);
			break;
		}
		else
			sendport += 2;
	}
}

rtp_sender::~rtp_sender()
{
	if (rtcp_alive)
	{
		((rtcp_recv_thread *)rtcp_alive)->stop();
	}
}

void rtp_sender::add(addr_info & addr)
{
	memset(&sendaddr, 0, sizeof(addr_info));
	memcpy(&sendaddr, &addr, sizeof(addr_info));
	char dst_addr[24] = { 0 };
	strcpy(dst_addr, sendaddr.addr);
	char *p = strchr(dst_addr, ':');
	if (!p)
		return;
	int dst_port = atoi(p + 1);
	p[0] = '\0';
	client_addr.sin_family = AF_INET;
	client_addr.sin_addr.S_un.S_addr = inet_addr(dst_addr);
	client_addr.sin_port = htons(dst_port);
}

bool rtp_sender::status()
{
	if ((!activate && time(0)-dormant_time<10) || activate)
		return true;
	else
		return false;
	//return activate;
}

bool rtp_sender::link()
{
	char dst_addr[24] = { 0 };
	strcpy(dst_addr, sendaddr.addr);
	char *p = strchr(dst_addr, ':');
	if (!p)
		return false;
	int dst_port = atoi(p + 1);
	p[0] = '\0';

	if (sendaddr.tranfer == SIP_TRANSFER_UDP)
	{
		sendaddr.online = true;
		activate = true;
		printf("[%010d]接收端[%s][%d]连接成功\n", sendssrc, sendaddr.addr, sendaddr.tranfer);
		return true;
	}
	else if (sendaddr.tranfer == SIP_TRANSFER_TCP_ACTIVE)
	{
		if (stream == ACL_SOCKET_INVALID)
		{
			stream = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			int recv_timeot = RTP_RW_TIMEOUT * 1000;
			setsockopt(stream, SOL_SOCKET, SO_RCVTIMEO, (char *)&recv_timeot, sizeof(int));
			sockaddr_in source = { 0 };
			source.sin_family = AF_INET;
			source.sin_addr.s_addr = inet_addr(sendip);
			source.sin_port = htons(sendport);
			if (bind(stream, (sockaddr*)&source, sizeof(source)) == 0)
				printf("inet_bind: send %s:%d ok\n", sendip, sendport);
		}
		if (stream != ACL_SOCKET_INVALID)
		{
			listen(stream, 0);
			int addr_size = sizeof(client_addr);
			client_stream = acl_sane_accept(stream, (sockaddr *)&client_addr, &addr_size);
			if (client_stream != ACL_SOCKET_INVALID)
			{
				printf("[%010d]接收端[%s][%d]连接成功\n", sendssrc, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
				activate = true;
				sendaddr.online = true;
			}
		}
	}
	else if (sendaddr.tranfer == SIP_TRANSFER_TCP_PASSIVE)
	{
		if (stream == ACL_SOCKET_INVALID)
		{
			stream = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			int recv_timeot = RTP_RW_TIMEOUT * 1000;
			setsockopt(stream, SOL_SOCKET, SO_RCVTIMEO, (char *)&recv_timeot, sizeof(int));
			sockaddr_in source = { 0 };
			source.sin_family = AF_INET;
			source.sin_addr.s_addr = inet_addr(sendip);
			source.sin_port = htons(sendport);
			if (bind(stream, (sockaddr*)&source, sizeof(source)) == 0)
				printf("inet_bind: send %s:%d ok\n", sendip, sendport);
		}
		if (stream != ACL_SOCKET_INVALID)
		{
			sockaddr_in destination = { 0 };
			destination.sin_family = AF_INET;
			destination.sin_addr.s_addr = inet_addr(dst_addr);
			destination.sin_port = htons(dst_port);
			if (acl_timed_connect(stream, (sockaddr*)&destination, sizeof(destination), 6) == 0)
			{
				printf("[%010d]接收端[%s][%d]连接成功\n", sendssrc, sendaddr.addr, sendaddr.tranfer);
				activate = true;
				sendaddr.online = true;
			}
			else
				printf("[%010d]接收端[%s][%d]连接超时\n", sendssrc, sendaddr.addr, sendaddr.tranfer);
		}
	}
	return false;
}

void rtp_sender::close()
{
	memset(&sendaddr, 0, sizeof(addr_info));
	sendaddr.online = false;
	if (sendtranfer != SIP_TRANSFER_UDP && stream != ACL_SOCKET_INVALID)
	{
		printf("send 关闭SOCKET\n");
		acl_socket_close(stream);
		stream = ACL_SOCKET_INVALID;
	}
	if (client_stream != ACL_SOCKET_INVALID)
	{
		acl_socket_close(client_stream);
		client_stream = ACL_SOCKET_INVALID;
	}
	activate = false;
	dormant_time = time(0);
}

int rtp_sender::send_data(unsigned char * buffer, size_t bufsize)	
{
	if (!activate)
		return 0;
	int nRet = -1;
	ACL_SOCKET sock_stream = stream;
	if (sendaddr.tranfer == SIP_TRANSFER_TCP_ACTIVE)
		sock_stream = client_stream;
	if (sock_stream != ACL_SOCKET_INVALID && sendaddr.online)
	{
		size_t buffer_size = bufsize;
		if (sendaddr.tranfer == SIP_TRANSFER_UDP)
		{
			nRet = sendto(sock_stream, (char *)buffer, (int)bufsize, 0, (sockaddr *)&client_addr, sizeof(client_addr));
		}
		else
		{
			unsigned char tcp_head[2] = { 0 };
			tcp_head[0] = (unsigned char)(bufsize >> 8);
			tcp_head[1] = (unsigned char)(bufsize & 0xff);
			memcpy(send_buffer, tcp_head, 2);
			memcpy(send_buffer + 2, buffer, bufsize);
			buffer_size += 2;
			nRet = send(sock_stream, (char *)send_buffer, (int)buffer_size, 0);
		}
		if (nRet <= 0)
		{
			close();
			return -1;
		}
	}
	return 0;
}

int rtp_sender::send_data(unsigned char *buffer, size_t bufsize, int tranfer)
{
	if (!activate)
		return 0;
	int nRet = -1;
	ACL_SOCKET sock_stream = stream;
	if (sendaddr.tranfer == SIP_TRANSFER_TCP_ACTIVE)
		sock_stream = client_stream;
	if (tranfer == SIP_TRANSFER_UDP)			  //udp转发udp
	{
		if (sendaddr.tranfer == SIP_TRANSFER_UDP)
		{
			if (sock_stream != ACL_SOCKET_INVALID  && sendaddr.online)
				nRet = sendto(sock_stream, (char *)buffer, (int)bufsize, 0, (sockaddr *)&client_addr, sizeof(client_addr));
		}
		else                                       //udp转发tcp
		{
			if (sock_stream != ACL_SOCKET_INVALID  && sendaddr.online)
			{
				unsigned char tcp_head[2] = { 0 };
				tcp_head[0] = (unsigned char)(bufsize >> 8);
				tcp_head[1] = (unsigned char)(bufsize & 0xff);
				memcpy(send_buffer, tcp_head, 2);
				memcpy(send_buffer + 2, buffer, bufsize);
				nRet = send(sock_stream, (char *)send_buffer, (int)(bufsize + 2), 0);
			}
		}
	}
	else
	{
		if (sendaddr.tranfer == SIP_TRANSFER_UDP)	//tcp转udp
		{
			if (sock_stream != ACL_SOCKET_INVALID  && sendaddr.online)
			{
				if (send_bufsize + bufsize > 102400)
				{
					send_bufsize = 0;
				}
				memcpy(send_buffer + send_bufsize, buffer, bufsize);
				send_bufsize += (int)bufsize;
				int i = 0;
				while (send_bufsize > 14)	//TCP RTP HEADER SIZE
				{
					if (((unsigned char)send_buffer[i + 2] == 0x80 || (unsigned char)send_buffer[i + 2] == 0x90) &&
						((unsigned char)send_buffer[i + 3] == 0x60 || (unsigned char)send_buffer[i + 3] == 0xE0 ||	//PS
						(unsigned char)send_buffer[i + 3] == 0x62 || (unsigned char)send_buffer[i + 3] == 0xE2 ||	//H264
						(unsigned char)send_buffer[i + 3] == 0x63 || (unsigned char)send_buffer[i + 3] == 0xE3 ||	//SVAC
						(unsigned char)send_buffer[i + 3] == 0x6C || (unsigned char)send_buffer[i + 3] == 0xEC ||	//H265
							(unsigned char)send_buffer[i + 3] == 0xA8)) //海康头
					{
						unsigned int ssrc = ntohl(*((unsigned int *)&send_buffer[i + 10]));
						if (sendssrc == ssrc)
						{
							int packlen = send_buffer[i] * 256 + send_buffer[i + 1];
							if (packlen + 2 > send_bufsize)
								break;
							else
							{
								char rtp_head[32] = { 0 };
								int rtplen = 12;
								if ((unsigned char)send_buffer[i+2] == 0x90)
									rtplen += 2 + 2 + ((send_buffer[i+2+14] * 256 + send_buffer[i+2+15]) * 4);
								memcpy(rtp_head, send_buffer+i+2, rtplen);
								unsigned char *rtp_data = send_buffer + i + 2 + rtplen;
								int send_len = packlen - rtplen;
								while (send_len > 1400)
								{
									if (rtp_head[1] >> 7 == 1)
										rtp_head[1] = rtp_head[1] & 0x7F;
									rtp_head[2] = rtpseq >> 8;
									rtp_head[3] = rtpseq & 0xFF;
									rtpseq++;
									memcpy(rtp_buffer, rtp_head, rtplen);
									memcpy(rtp_buffer + rtplen, rtp_data, 1400);
									nRet = send(sock_stream, (char *)rtp_buffer, 1400 + rtplen, 0);
									rtp_data += 1400;
									send_len -= 1400;
								}
								rtp_head[2] = rtpseq >> 8;
								rtp_head[3] = rtpseq & 0xFF;
								rtpseq++;
								memcpy(rtp_buffer, rtp_head, rtplen);
								memcpy(rtp_buffer + rtplen, rtp_data, send_len);
								nRet = send(sock_stream, (char *)rtp_buffer, send_len + rtplen, 0);
								i += packlen + 1;
								send_bufsize -= packlen + 1;
							}
						}
					}
					i++;
					send_bufsize--;
				}
				if (send_bufsize > 0 && i > 0)
					memcpy(send_buffer, send_buffer + i, send_bufsize);
			}
		}
		else                                        
		{
			if (sock_stream != ACL_SOCKET_INVALID  && sendaddr.online)
				nRet = send(sock_stream, (char *)buffer, (int)bufsize, 0);
		}
	}
	if (nRet <= 0)
	{
		close();
	}
	return nRet;
}

//媒体流接收线程
rtp_recv_thread::rtp_recv_thread(const char *ip, int port, int tranfer, void* rtp)
{
	memset(stream_type, 0, sizeof(stream_type));
	memset(stream_ssrc, 0, sizeof(stream_ssrc));
	memset(rtp_callid, 0, sizeof(rtp_callid));
	memset(rtp_deviceid, 0, sizeof(rtp_deviceid));
	memset(rtpip, 0, sizeof(rtpip));
	strcpy(rtpip, ip);
	rtpport = port;
	rtptranfer = tranfer;
	rtpsrv = (RtpService *)rtp;
	running = true;
	activate = false;
	connectd = false;
	invite_type = 0;
	media_type = 0;
	playload = 96;
	rtcp_time = 0;
	dormant_time = 0;
	
	stream = ACL_SOCKET_INVALID;
	rtcp_stream = ACL_SOCKET_INVALID;
	client_stream = ACL_SOCKET_INVALID;

	if (tranfer == SIP_TRANSFER_UDP)
	{
		stream = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		rtcp_stream = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	}
	else
		stream = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	int recv_timeot = RTP_RW_TIMEOUT*1000;
	setsockopt(stream, SOL_SOCKET, SO_RCVTIMEO, (char *)&recv_timeot, sizeof(int));
	while (true)
	{
		sockaddr_in source = { 0 };
		source.sin_family = AF_INET;
		source.sin_addr.s_addr = inet_addr(ip);
		source.sin_port = htons(rtpport);
		if (bind(stream, (sockaddr*)&source, sizeof(source)) == 0)
		{
			printf("inet_bind: recv %s:%d ok\n", ip, rtpport);
			break;
		}
		else
			rtpport += 2;
	}

}

rtp_recv_thread::~rtp_recv_thread()
{
}

void * rtp_recv_thread::run()
{
	unsigned char stream_buffer[6144] = { 0 };
	int buffer_size = 0;
	sockaddr_in destination = { 0 };
	int nTimeout = 0;
	int sockaddr_size = sizeof(sockaddr_in);
	while (running)
	{
		if (rtptranfer == SIP_TRANSFER_UDP)
		{
			if (activate)
			{
				memset(stream_buffer, 0, sizeof(stream_buffer));
				memset(&destination, 0, sockaddr_size);
				buffer_size = recvfrom(stream, (char *)stream_buffer, sizeof(stream_buffer), 0, (sockaddr *)&destination, &sockaddr_size);
				if (buffer_size <= 0)
				{
					printf("[%d][%s][%d]媒体流接收中断[%d:%d]!\n", rtpport, rtp_deviceid, rtptranfer, (int)client_stream, acl_last_error());
					activate = false;
					rtpsrv->rtp_cls_recv_send(rtp_callid, rtp_deviceid, invite_type);
					cls_client();
					dormant();
				}
				else
				{
					time_t now_time = time(0);
					if (now_time - rtcp_time > 30)
					{
						destination.sin_port += 256;
						send_rtcp_data((sockaddr *)&destination);	//30秒发送一次rtcp包
						rtcp_time = time(0);
					}
					if (!tranfer_data(stream_buffer, buffer_size))
						break;
					continue;
				}
			}
		}
		else if (rtptranfer == SIP_TRANSFER_TCP_ACTIVE)
		{
			memset(stream_buffer, 0, sizeof(stream_buffer));
			if (stream != ACL_SOCKET_INVALID && activate)
			{
				buffer_size = recv(stream, (char *)stream_buffer, sizeof(stream_buffer), 0);
				if (buffer_size <= 0 && stream != ACL_SOCKET_INVALID)
				{
					if (buffer_size == 0)
						printf("[%d][%s][%d]媒体流接收超时[%d:%d]!\n", rtpport, rtp_deviceid, rtptranfer, (int)client_stream, acl_last_error());
					else
						printf("[%d][%s][%d]媒体流接收中断[%d:%d]!\n", rtpport, rtp_deviceid, rtptranfer, (int)client_stream, acl_last_error());
					rtpsrv->rtp_cls_recv_send(rtp_callid, rtp_deviceid, invite_type);
					cls_client();
					dormant();
				}
				else
				{
					if (!tranfer_data(stream_buffer, buffer_size))
						break;
					continue;
				}
			}
		}
		else if (rtptranfer == SIP_TRANSFER_TCP_PASSIVE)
		{
			if (stream != ACL_SOCKET_INVALID && client_stream == ACL_SOCKET_INVALID && !connectd)
			{
				memset(&destination, 0, sockaddr_size);
				client_stream = acl_sane_accept(stream, (sockaddr *)&destination, &sockaddr_size);
				if (client_stream != ACL_SOCKET_INVALID)
				{
					connectd = true;
					activate = true;
					printf("[%s]发流端[%s:%d][%d][%s]连接成功\n", stream_ssrc, inet_ntoa(destination.sin_addr), ntohs(destination.sin_port), (int)client_stream, rtp_deviceid);
				}		
			}
			if (client_stream != ACL_SOCKET_INVALID && activate)
			{
				memset(stream_buffer, 0, sizeof(stream_buffer));
				buffer_size = recv(client_stream, (char *)stream_buffer, sizeof(stream_buffer), 0);
				if (!running)
					break;
				if (buffer_size <= 0 && client_stream != ACL_SOCKET_INVALID && acl_last_error() != 0)
				{
					if (buffer_size == 0)
						printf("[%d][%s][%d][%d]媒体流接收超时[%d]!\n", rtpport, rtp_deviceid, (int)client_stream, rtptranfer, acl_last_error());
					else
						printf("[%d][%s][%d][%d]媒体流接收中断[%d]!\n", rtpport, rtp_deviceid, (int)client_stream, rtptranfer, acl_last_error());
					
					printf("recv 关闭SOCKET!\n");
					rtpsrv->rtp_cls_recv_send(rtp_callid, rtp_deviceid, invite_type);
					cls_client();
					dormant();
				}
				else
				{
					if (!tranfer_data(stream_buffer, buffer_size))
						break;
					continue;
				}
			}
		}
		sleep(1);
	}
	//线程结束
	if (stream != ACL_SOCKET_INVALID)
	{
		acl_socket_close(stream);
	}
	if (client_stream != ACL_SOCKET_INVALID)
	{
		acl_socket_close(client_stream);
	}
	delete this;
	return NULL;
}

void rtp_recv_thread::stop()
{
	running = false;
}

void rtp_recv_thread::dormant()
{
	connectd = false;
	activate = false;

	if (rtptranfer != SIP_TRANSFER_UDP)
	{
		if (stream != ACL_SOCKET_INVALID)
		{
			printf("recv 关闭SOCKET!\n");
			acl_socket_close(stream);
			stream = ACL_SOCKET_INVALID;
		}
		if (client_stream != ACL_SOCKET_INVALID)
		{
			acl_socket_close(client_stream);
			client_stream = ACL_SOCKET_INVALID;
		}
	}
	dormant_time = time(0);
}

void rtp_recv_thread::set_playinfo(const char * callid, const char * deviceid, int playtype)
{
	strcpy(rtp_callid, callid);
	strcpy(rtp_deviceid, deviceid);
	invite_type = playtype;
}

bool rtp_recv_thread::get_status()
{
	if ((!activate && time(0)-dormant_time<10) || activate)
	{
		return true;
	}
	return false;
	//return activate;
}

void rtp_recv_thread::set_mediainfo(addr_info * addr)
{
	char dst_addr[24] = { 0 };
	strcpy(dst_addr, addr->addr);
	char *p = strchr(dst_addr, ':');
	if (!p)
		return;
	int dst_port = atoi(p + 1);
	p[0] = '\0';
	if (addr->tranfer == SIP_TRANSFER_UDP)
	{
		//创建RTCP	
		printf("[%s]发流端[%s][%s]连接成功\n", stream_ssrc, addr->addr, rtp_deviceid);
		activate = true;
	}
	else if (addr->tranfer == SIP_TRANSFER_TCP_ACTIVE)
	{
		if (stream == ACL_SOCKET_INVALID)
		{
			stream = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			int recv_timeot = RTP_RW_TIMEOUT * 1000;
			setsockopt(stream, SOL_SOCKET, SO_RCVTIMEO, (char *)&recv_timeot, sizeof(int));
			sockaddr_in source = { 0 };
			source.sin_family = AF_INET;
			source.sin_addr.s_addr = inet_addr(rtpip);
			source.sin_port = htons(rtpport);
			if (bind(stream, (sockaddr*)&source, sizeof(source)) == 0)
				printf("inet_bind: recv %s:%d ok\n", rtpip, rtpport);
		}
		if (stream != ACL_SOCKET_INVALID)
		{
			sockaddr_in destination = { 0 };
			destination.sin_family = AF_INET;
			destination.sin_addr.s_addr = inet_addr(dst_addr);
			destination.sin_port = htons(dst_port);
			if (acl_timed_connect(stream, (sockaddr*)&destination, sizeof(destination), 6) == 0)
			{
				printf("[%s]发流端[%s][%s]连接成功\n", stream_ssrc, addr->addr, rtp_deviceid);
				activate = true;
			}
			else
				printf("[%s]发流端[%s][%s]连接超时\n", stream_ssrc, addr->addr, rtp_deviceid);
		}
	}
	else if (addr->tranfer == SIP_TRANSFER_TCP_PASSIVE)
	{
		if (stream == ACL_SOCKET_INVALID)
		{
			stream = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			int recv_timeot = RTP_RW_TIMEOUT * 1000;
			setsockopt(stream, SOL_SOCKET, SO_RCVTIMEO, (char *)&recv_timeot, sizeof(int));
			sockaddr_in source = { 0 };
			source.sin_family = AF_INET;
			source.sin_addr.s_addr = inet_addr(rtpip);
			source.sin_port = htons(rtpport);
			if (bind(stream, (sockaddr*)&source, sizeof(source)) == 0)
				printf("inet_bind: recv %s:%d ok\n", rtpip, rtpport);
		}
		if (stream != ACL_SOCKET_INVALID)
		{
			listen(stream, 5);
		}
	}
}

int rtp_recv_thread::add_client(const char * callid, rtp_sender * sender)
{
	client_locker.lock();
	client_list[callid] = sender;
	client_locker.unlock();
	return (int)client_list.size();
}

int rtp_recv_thread::del_client(const char * callid)
{
	client_locker.lock();
	std::map<std::string, rtp_sender *>::iterator itor = client_list.find(callid);
	if (itor != client_list.end())
	{
		client_list.erase(itor);
	}
	client_locker.unlock();
	return (int)client_list.size();
}

int rtp_recv_thread::client_size()
{
	return (int)client_list.size();
}

bool rtp_recv_thread::tranfer_data(unsigned char * buffer, size_t bufsize)
{
	if (client_list.size() == 0)
		return true;
	client_locker.lock();
	std::map<std::string, rtp_sender *>::iterator itor = client_list.begin();
	while (itor != client_list.end())
	{
		if (!running)
		{
			client_locker.unlock();
			return false;
		}
		rtp_sender *pSend = itor->second;
		if (pSend)
		{
			if (pSend->send_data(buffer, bufsize, rtptranfer) == -1)
			{
				printf("接收端[%s]异常中断!\n", pSend->sendaddr.addr);
				rtpsrv->session_over(itor->first.c_str(), SESSION_RECVER_BYE);
				itor = client_list.erase(itor);
				if (client_list.size() == 0)
					break;
			}
			else
				itor++;
		}
		else
		{
			itor = client_list.erase(itor);
		}
	}
	client_locker.unlock();
	if (client_list.size() == 0)
	{
		rtpsrv->rtp_cls_recv_send(rtp_callid, rtp_deviceid, invite_type);
		dormant();
	}
	return true;
}

void rtp_recv_thread::cls_client()
{
	client_locker.lock();
	if (client_list.size() > 0)
	{
		//printf("释放所有媒体流接收端\n");
		std::map<std::string, rtp_sender *>::iterator itor = client_list.begin();
		while (itor != client_list.end())
		{
			rtpsrv->session_over(itor->first.c_str(), SESSION_SMEDIA_BYE);
			itor++;
		}
		client_list.clear();
	}
	client_locker.unlock();
}

void rtp_recv_thread::send_rtcp_data(sockaddr *to)
{
	unsigned char szRtcpPacket[28] = { 0 };
	szRtcpPacket[0] = 0x80;
	szRtcpPacket[1] = 0xc9;
	szRtcpPacket[3] = 0x01;
	szRtcpPacket[8] = 0x81;
	szRtcpPacket[9] = 0xca;
	szRtcpPacket[11] = 0x04;
	szRtcpPacket[16] = 0x01;
	szRtcpPacket[17] = 0x09;
	unsigned int ssrc = htonl(atoi(stream_ssrc));
	memcpy(szRtcpPacket + 4, &ssrc, 4);
	memcpy(szRtcpPacket + 18, "Heartbeat", 9);

	if (rtcp_stream != ACL_SOCKET_INVALID)
	{
		sendto(rtcp_stream, (char *)szRtcpPacket, sizeof(szRtcpPacket), 0, to, sizeof(sockaddr_in));
	}
}

rtcp_recv_thread::rtcp_recv_thread(int rtcpport, rtp_sender * sender)
{
	bRunning = true;
	pSender = sender;
	stream = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	sockaddr_in source = { 0 };
	source.sin_family = AF_INET;
	source.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	source.sin_port = htons(rtcpport);
	bind(stream, (sockaddr*)&source, sizeof(source));
}

rtcp_recv_thread::~rtcp_recv_thread()
{
}

void * rtcp_recv_thread::run()
{
	struct sockaddr_in cliet_addr;
	int addr_size = sizeof(cliet_addr);
	char szRtcpBuffer[1024] = { 0 };
	while (bRunning)
	{
		if (pSender && pSender->status())
		{
			if (stream != ACL_SOCKET_INVALID)
			{
				memset(&cliet_addr, 0, addr_size);
				int nBufSize = recvfrom(stream, (char *)szRtcpBuffer, (int)sizeof(szRtcpBuffer), 0, (sockaddr *)&cliet_addr, &addr_size);
				if (nBufSize > 0)
				{
					pSender->sendaddr.timeout = time(0);
				}
			}
		}
		sleep(1);
	}
	if (stream != ACL_SOCKET_INVALID)
	{
		acl_socket_close(stream);
		stream = ACL_SOCKET_INVALID;
	}
	delete this;
	return NULL;
}

void rtcp_recv_thread::stop()
{
	bRunning = true;
}
