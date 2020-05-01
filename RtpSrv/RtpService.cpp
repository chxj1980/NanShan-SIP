#include "stdafx.h"
#include "RtpService.h"
#include "RtpTransform.h"

static acl::thread_pool *__thdpool = NULL;

static register_map __regmap;		//注册管理
static acl::locker __reglocker;
static session_map	__sesmap;		//会话管理
static acl::locker __seslocker;

static std::map<int, rtp_sender *> _send_port_map;
static acl::locker		   _send_port_locker;
static std::map<int, rtp_recv_thread *> _recv_port_map;
static acl::locker		   _recv_port_locker;
static std::list<RtpTransform *> _recv_sdk_thdpool;


std::map<std::string, acl::thread *>					_recv_thd_map;
typedef std::map<std::string, acl::thread *>::iterator	_recvthdmap_itor;
acl::locker												_recvthdmap_locker;



acl::string acl_u2a(const char *text)
{
	acl::charset_conv conv;
	acl::string outbuf;
	if (conv.convert("utf-8", "gb18030", text, strlen(text), &outbuf))
		return outbuf;
	return "";
}

acl::string acl_a2u(const char *text)
{
	acl::charset_conv conv;
	acl::string outbuf;
	if (conv.convert("gb18030", "utf-8", text, strlen(text), &outbuf))
		return outbuf;
	return "";
}

acl::string acl_xml_getElementValue(acl::xml1 &xml, const char *tag)
{
	acl::xml_node *node = xml.getFirstElementByTag(tag);
	if (node == NULL)
		return "";
	return node->text();
}

acl::string acl_xml_getElementString(acl::xml1 &xml, const char *tag)
{
	acl::xml_node *node = xml.getFirstElementByTag(tag);
	if (node == NULL)
		return "";
	return acl_u2a(node->text());
}

int acl_xml_getElementInt(acl::xml1 &xml, const char *tag)
{
	acl::xml_node *node = xml.getFirstElementByTag(tag);
	if (node == NULL)
		return 0;
	return atoi(node->text());
}

	
RtpService::RtpService()
{
	m_running = true;
	m_conf.read();
	m_szRecvBuf = new char[SIP_RECVBUF_MAX];
	memset(m_szRecvBuf, 0, SIP_RECVBUF_MAX);
}


RtpService::~RtpService()
{
}

//void RtpService::on_read(acl::socket_stream * stream)
//{
//	acl::string strAddr = stream->get_peer(true);
//	memset(m_szRecvBuf, 0, SIP_RECVBUF_MAX);
//	int nBufSize = stream->read(m_szRecvBuf, SIP_RECVBUF_MAX, false);
//	if (nBufSize < 0)
//	{
//		m_conf.log(LOG_ERROR, "read socket %s error", strAddr.c_str());
//		regmap_iter riter = __regmap.find(strAddr.c_str());
//		if (riter != __regmap.end())
//		{
//			riter->second.online = false;
//		}
//	}
//	if (nBufSize > 0)
//	{
//		sip_handle_thread *handlethd = new sip_handle_thread(strAddr.c_str(), m_szRecvBuf, nBufSize, this);
//		__thdpool->run(handlethd);
//	}
//}

void RtpService::on_read(socket_stream* sock_stream, const char *data, size_t dlen)
{
	sip_handle_thread *handlethd = new sip_handle_thread(sock_stream, data, dlen, this);
	__thdpool->run(handlethd);
}

void RtpService::proc_on_init()
{
	__thdpool = new sip_thread_pool;

	__thdpool->set_limit(m_conf.srv_pool);
	__thdpool->set_idle(2);
	__thdpool->start();

	sip_init(m_conf.local_code, m_conf.local_addr, m_conf.local_port);
	rtp_init(m_conf.rtp_send_port, m_conf.rtp_recv_port);
	//开启管理线程
	regmnger_thread *regmnger_thd = new regmnger_thread(this);
	regmnger_thd->set_detachable(true);
	regmnger_thd->start();
	sesmnger_thread *sesmnger_thd = new sesmnger_thread(this);
	sesmnger_thd->set_detachable(true);
	sesmnger_thd->start();
	msgmnger_thread *msgmnger_thd = new msgmnger_thread(this);
	msgmnger_thd->set_detachable(true);
	msgmnger_thd->start();
#ifdef _WIN32
	scanf_thread *scanf_thd = new scanf_thread(this);
	scanf_thd->set_detachable(true);
	scanf_thd->start();
#endif
}

void RtpService::proc_on_exit()
{
	m_running = false;
	if (__thdpool)
	{
		__thdpool->stop();
		delete __thdpool;
		__thdpool = NULL;
	}
}

void RtpService::on_handle(socket_stream* stream, const char * data, size_t dlen)
{
	SIP_EVENT_TYPE even_type = sip_get_event_type(data);
	switch (even_type)
	{
	case SIP_REGISTER_SUCCESS:
		on_register_success(stream, data, dlen);
		break;
	case SIP_REGISTER_FAILURE:
		on_register_failure(stream, data, dlen);
		break;
	case SIP_INVITE_REQUEST:
		on_invite_request(stream, data, dlen);
		break;
	case SIP_CANCEL_REQUEST:
		on_cancel_request(stream, data, dlen);
		break;
	case SIP_ACK_REQUEST:
		on_ack_request(stream, data, dlen);
		break;
	case SIP_BYE_REQUEST:
		on_bye_request(stream, data, dlen);
		break;
	case SIP_BYE_SUCCESS:
		on_bye_success(stream, data, dlen);
		break;
	case SIP_BYE_FAILURE:
		on_bye_failure(stream, data, dlen);
		break;
	case SIP_INFO_REQUEST:
		on_info_request(stream, data, dlen);
		break;
	case SIP_SUBSCRIBE_REQUEST:
		on_subscribe_request(stream, data, dlen);
		break;
	case SIP_NOTIFY_SUCCESS:
		on_notify_success(stream, data, dlen);
		break;
	case SIP_NOTIFY_FAILURE:
		on_notify_failure(stream, data, dlen);
		break;
	case SIP_MESSAGE_EVENT:
		on_message_request(stream, data, dlen);
		break;
	case SIP_MESSAGE_SUCCESS:
		on_message_success(stream, data, dlen);
		break;
	case SIP_MESSAGE_FAILURES:
		on_message_failure(stream, data, dlen);
		break;
	case SIP_OPTIONS_REQUEST:
		on_options_request(stream, data, dlen);
		break;
	default:
		break;
	}
}

void RtpService::on_register_success(socket_stream* stream, const char * data, size_t dlen)
{
	const char *sAddr = stream->get_peer(true);
	regmap_iter riter = __regmap.find(sAddr);
	if (riter == __regmap.end())
	{
		m_conf.log(LOG_WARN, "未找到匹配的注册请求[%s]", sAddr);
		return;
	}
	if (riter->second.expires > 0)	//注册成功
	{
		if (riter->second.online)
		{
			printf("向SIP[%s]重注册成功\n", sAddr);
		}
		riter->second.online = true;
		riter->second.alivetime = time(NULL);
	}
	else                            //注销成功
	{
		riter->second.online = false;
	}
}

void RtpService::on_register_failure(socket_stream* stream, const char * data, size_t dlen)
{
	acl::string szData(data, dlen);
	const char *sAddr = stream->get_peer(true);
	char szBuffer[SIP_HEADER_LEN] = { 0 };
	acl::string strRealm = sip_get_www_authenticate_realm(szBuffer, sizeof(szBuffer), szData.c_str());
	acl::string strNonce = sip_get_www_authenticate_nonce(szBuffer, sizeof(szBuffer), szData.c_str());
	if (strRealm == "" || strNonce == "")
	{
		m_conf.log(LOG_WARN, "向[%s]注册失败!\n%s", sAddr, szData.c_str());
		return;
	}
	regmap_iter riter = __regmap.find(sAddr);
	if (riter == __regmap.end())
	{
		m_conf.log(LOG_WARN, "未找到匹配的注册请求[%s]", sAddr);
		return;
	}
	acl::string strUser = sip_get_from_user(szBuffer, sizeof(szBuffer), szData.c_str());
	acl::string strUri = sip_build_msg_to(szBuffer, sizeof(szBuffer), riter->second.user, riter->second.addr);
	char szAuth[SIP_AUTH_LEN] = { 0 };
	acl::string strRes = sip_build_authorization_response(szAuth, sizeof(szAuth), strUser.c_str(), strRealm.c_str(), riter->second.pwd, strNonce.c_str(), strUri.c_str());
	acl::string strAuth;
	strAuth.format("Digest username=\"%s\",realm=\"%s\","
		"nonce=\"%s\",uri=\"%s\","
		"response=\"%s\",algorithm=MD5",
		strUser.c_str(), strRealm.c_str(), strNonce.c_str(), strUri.c_str(), strRes.c_str());
	acl::string strCallID = sip_get_header(szBuffer, sizeof(szBuffer), szData.c_str(), SIP_H_CALL_ID);
	acl::string strFTag = sip_get_from_tag(szBuffer, sizeof(szBuffer), szData.c_str());
	char szMsg[SIP_MSG_LEN] = { 0 };
	int nLen = sip_build_msg_register(szMsg, sizeof(szMsg), strUri.c_str(), strAuth.c_str(), 2, riter->second.expires, strCallID.c_str(), strFTag.c_str());
	send_msg(sAddr, szMsg, nLen);
}

void RtpService::on_bye_success(socket_stream* stream, const char * data, size_t dlen)
{
}

void RtpService::on_bye_failure(socket_stream* stream, const char * data, size_t dlen)
{
}

void RtpService::on_notify_success(socket_stream* stream, const char * data, size_t dlen)
{
}

void RtpService::on_notify_failure(socket_stream* stream, const char * data, size_t dlen)
{
}

void RtpService::on_message_success(socket_stream* stream, const char * data, size_t dlen)
{
	acl::string szData(data, dlen);
	if (sip_get_cseq_num(szData.c_str()) == SIP_CSEQ_KEEPALIVE)	//心跳成功
	{
		const char *sAddr = stream->get_peer(true);
		regmap_iter riter = __regmap.find(sAddr);
		if (riter != __regmap.end())
		{
			riter->second.alivetime = time(NULL);
		}
	}
}

void RtpService::on_message_failure(socket_stream* stream, const char * data, size_t dlen)
{
	acl::string szData(data, dlen);
	if (sip_get_cseq_num(szData.c_str()) == SIP_CSEQ_KEEPALIVE)	//心跳失败
	{
		const char *sAddr = stream->get_peer(true);
		regmap_iter riter = __regmap.find(sAddr);
		if (riter != __regmap.end())
		{
			m_conf.log(LOG_WARN, "上级SIP[%s]掉线！\n", sAddr);
			riter->second.online = false;
		}
	}
}

void RtpService::on_options_request(socket_stream* stream, const char * data, size_t dlen)
{
	acl::string szData(data, dlen);
	char szAns[SIP_MSG_LEN] = { 0 };
	int nLen = sip_build_msg_answer(szAns, sizeof(szAns), szData.c_str(), SIP_STATUS_200);
	nLen = sip_set_header(szAns, sizeof(szAns), SIP_H_ALLOW, "INVITE,ACK,CANCEL,INFO,BYE,OPTIONS,MESSAGE,SUBSCRIBE,NOTIFY");
	stream->write(szAns, nLen);
}

void RtpService::on_invite_request(socket_stream* stream, const char * data, size_t dlen)
{
	acl::string szData(data, dlen);
	acl::string strAddr = stream->get_peer(true);
	char szBuffer[SIP_HEADER_LEN] = { 0 };
	char szAns[SIP_MSG_LEN] = { 0 };
	char szBody[SIP_BODY_LEN] = { 0 };
	int nAnsLen = 0;
	int nBodyLen = 0;
	acl::string strCallID = sip_get_header(szBuffer, sizeof(szBuffer), szData.c_str(), SIP_H_CALL_ID);
	acl::string strFtag = sip_get_from_tag(szBuffer, sizeof(szBuffer), szData.c_str());
	//acl::string strSubject = sip_get_header(szBuffer, sizeof(szBuffer), szData.c_str(), SIP_H_SUBJECT);
	acl::string strDeviceID = sip_get_subject_deviceid(szBuffer, sizeof(szBuffer), szData.c_str());
	int nPlayType = sip_get_subject_playtype(szData.c_str());
	int nTranFer = sip_get_subject_tranfer(szData.c_str());
	acl::string strNetTypeID = sip_get_subject_receiverid(szBuffer, sizeof(szBuffer), szData.c_str());
	int nNetType = atoi(strNetTypeID.c_str());
	sip_get_body(szData.c_str(), szBody, sizeof(szBody));

	nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), szData.c_str(), SIP_STATUS_200);
	char szContact[SIP_HEADER_LEN] = { 0 };
	sprintf(szContact, "<sip:%s@%s:%d>", m_conf.local_code, m_conf.local_addr, m_conf.local_port);
	nAnsLen = sip_set_header(szAns, sizeof(szAns), SIP_H_CONTACT, szContact);
	acl::string strTtag = sip_get_to_tag(szBuffer, sizeof(szBuffer), szData.c_str());
	if (strTtag == "")
		strTtag = sip_get_to_tag(szBuffer, sizeof(szBuffer), szAns);
	
	sesmap_iter siter = __sesmap.find(strCallID.c_str());
	if (siter == __sesmap.end()) //第一次点播请求
	{
		printf("=============================================================================\n");
		printf("点播镜头[%s] play:%d send:%d recv:%s\n", strDeviceID.c_str(), nPlayType, nTranFer, strNetTypeID.c_str());
		if (nNetType >= SIP_TRANSFER_SDK)	//非标镜头
		{
			on_invite_transfrom_sdk(stream, nPlayType, nNetType, strDeviceID.c_str(), strCallID.c_str(), strFtag.c_str(), strTtag.c_str(), szBody, szData.c_str());
			return;
		}
		rtp_recv_thread *pRecvThd = NULL;
		if (nPlayType == SIP_INVITE_PLAY)	//实时点播
		{
			_recvthdmap_locker.lock();
			_recvthdmap_itor itorRecv = _recv_thd_map.find(strDeviceID.c_str());
			if (itorRecv != _recv_thd_map.end())	//已经有流
			{
				pRecvThd = (rtp_recv_thread *)itorRecv->second;
				if (pRecvThd->activate)
				{
					printf("己找到媒体流，进入分发\n");
					rtp_sender *pSender = (rtp_sender *)rtp_get_send_port(nTranFer);
					if (pSender == NULL)
					{
						nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), szData.c_str(), SIP_STATUS_503);
						send_msg(strAddr.c_str(), szAns, nAnsLen);
						_recvthdmap_locker.unlock();
						return;
					}
					int nSendPort = pSender->sendport;
					nBodyLen = sip_build_msg_sdp(szBody, sizeof(szBody), m_conf.local_code, strDeviceID.c_str(), NULL, m_conf.local_addr, nSendPort, SIP_INVITE_PLAY, SIP_SENDRECV,
						pRecvThd->rtptranfer, 0, pRecvThd->media_type, pRecvThd->playload, pRecvThd->stream_type);
					nBodyLen = sip_set_sdp_y(szBody, sizeof(szBody), pRecvThd->stream_ssrc);
					nAnsLen = sip_set_body(szAns, sizeof(szAns), szBody, SIP_APPLICATION_SDP);
					if (send_msg(strAddr.c_str(), szAns, nAnsLen))
					{
						session_info *pInfo = new session_info;
						memset(pInfo, 0, sizeof(session_info));
						strcpy(pInfo->addr, strAddr.c_str());
						sip_get_header(pInfo->user_via, sizeof(pInfo->user_via), szData.c_str(), SIP_H_VIA);
						sip_get_header(pInfo->user_from, sizeof(pInfo->user_from), szData.c_str(), SIP_H_FROM);
						sip_get_header(pInfo->user_to, sizeof(pInfo->user_to), szData.c_str(), SIP_H_TO);
						strcpy(pInfo->callid, strCallID.c_str());
						strcpy(pInfo->ftag, strFtag.c_str());
						strcpy(pInfo->ttag, strTtag.c_str());
						strcpy(pInfo->deviceid, strDeviceID.c_str());
						strcpy(pInfo->ssrc, pRecvThd->stream_ssrc);
						pInfo->cseq = sip_get_cseq_num(szData.c_str());
						pInfo->invite_type = SIP_INVITE_PLAY;
						pInfo->session_time = time(NULL);
						pInfo->rtp_stream = pSender;
						__sesmap[strCallID.c_str()] = pInfo;
						pRecvThd->add_client(strCallID.c_str(), pSender);
						_recvthdmap_locker.unlock();

						if (nTranFer == SIP_TRANSFER_TCP_ACTIVE)
						{
							pSender->link();
						}
						return;
					}
				}
				else
				{
					_recv_thd_map.erase(itorRecv);
				}
			}
			_recvthdmap_locker.unlock();
		}
		pRecvThd = (rtp_recv_thread *)rtp_get_recv_thd(nNetType);
		if (pRecvThd == NULL)
		{
			nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), szData.c_str(), SIP_STATUS_503);
			send_msg(strAddr.c_str(), szAns, nAnsLen);
			return;
		}
		pRecvThd->set_playinfo(strCallID.c_str(), strDeviceID.c_str(), nPlayType);
		int nRecvPort = pRecvThd->rtpport;
		if (nPlayType == SIP_INVITE_PLAY)
			_recv_thd_map[strDeviceID.c_str()] = pRecvThd;
		else
			_recv_thd_map[strCallID.c_str()] = pRecvThd;
		//printf("_recv_thd_map add %s\n", strDeviceID.c_str());
		nBodyLen = sip_build_msg_sdp(szBody, sizeof(szBody), m_conf.local_code, strDeviceID.c_str(), NULL, m_conf.local_addr, nRecvPort, nPlayType, SIP_RECVONLY, nNetType);
		nAnsLen = sip_set_body(szAns, sizeof(szAns), szBody, SIP_APPLICATION_SDP);
		if (stream->write(szAns, nAnsLen))
		{
			session_info *pInfo = new session_info;
			memset(pInfo, 0, sizeof(session_info));
			strcpy(pInfo->addr, strAddr.c_str());
			strcpy(pInfo->callid, strCallID.c_str());
			strcpy(pInfo->deviceid, strDeviceID.c_str());
			pInfo->invite_type = nPlayType;
			pInfo->session_time = time(NULL);
			__sesmap[strCallID.c_str()] = pInfo;
		}
	}
	else                        //发流请求 第二次Invite
	{
		if (nNetType >= SIP_TRANSFER_SDK)	//非标镜头
			return;	//重复点播
		if (szBody[0] == '\0')
		{
			nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), szData.c_str(), SIP_STATUS_488);
			send_msg(strAddr.c_str(), szAns, nAnsLen);
			return;
		}
		sip_sdp_info sdp;
		memset(&sdp, 0, sizeof(sdp));
 		sip_get_sdp_info(szBody, &sdp);
		_recvthdmap_itor itorRecv;
		if (sdp.s_type == SIP_INVITE_PLAY)
			itorRecv = _recv_thd_map.find(strDeviceID.c_str());
		else
			itorRecv = _recv_thd_map.find(strCallID.c_str());
		if (itorRecv == _recv_thd_map.end())
		{
			nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), szData.c_str(), SIP_STATUS_408);
			send_msg(strAddr.c_str(), szAns, nAnsLen);
			return;
		}
		rtp_recv_thread *pRecvThd = (rtp_recv_thread *)itorRecv->second;

		rtp_sender *pSender = (rtp_sender *)rtp_get_send_port(sdp.m_protocol);
		if (pSender == NULL)
		{
			nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), szData.c_str(), SIP_STATUS_503);
			send_msg(strAddr.c_str(), szAns, nAnsLen);
			return;
		}
		addr_info addr;
		memset(&addr, 0, sizeof(addr_info));
		sprintf(addr.addr, "%s:%d", sdp.o_ip, sdp.m_port);
		addr.tranfer = sdp.m_protocol;
		strcpy(addr.ssrc, sdp.y_ssrc);
		pSender->add(addr);	//添加收流端
		pSender->sendtranfer = sdp.m_protocol;
		pSender->sendssrc = atoi(sdp.y_ssrc);
		int nSendPort = pSender->sendport;
		int nTranfer = 0;
		if (sdp.m_protocol == 1)
			nTranfer = 2;
		else if (sdp.m_protocol == 2)
			nTranfer = 1;
		char szPlayTime[64] = { 0 };
		sprintf(szPlayTime, "%lld %lld", sdp.t_begintime, sdp.t_endtime);
		nBodyLen = sip_build_msg_sdp(szBody, sizeof(szBody), m_conf.local_code, strDeviceID.c_str(), szPlayTime, m_conf.local_addr, nSendPort, sdp.s_type, SIP_SENDONLY,
			nTranfer, sdp.a_downloadspeed, sdp.m_type, sdp.a_playload, sdp.a_streamtype);
		nBodyLen = sip_set_sdp_y(szBody, sizeof(szBody), sdp.y_ssrc);
		nAnsLen = sip_set_body(szAns, sizeof(szAns), szBody, SIP_APPLICATION_SDP);
		if (stream->write(szAns, nAnsLen))
		{
			sip_get_header(siter->second->user_via, sizeof(siter->second->user_via), szData.c_str(), SIP_H_VIA);
			sip_get_header(siter->second->user_from, sizeof(siter->second->user_from), szData.c_str(), SIP_H_FROM);
			sip_get_header(siter->second->user_to, sizeof(siter->second->user_to), szData.c_str(), SIP_H_TO);
			strcpy(siter->second->callid, strCallID.c_str());
			strcpy(siter->second->ftag, strFtag.c_str());
			strcpy(siter->second->ttag, strTtag.c_str());
			strcpy(siter->second->invite_time, szPlayTime);
			siter->second->cseq = sip_get_cseq_num(szData.c_str());
			siter->second->invite_type = sdp.s_type;
			siter->second->session_time = time(NULL);
			siter->second->rtp_stream = pSender;
			pRecvThd->add_client(strCallID.c_str(), pSender); //将收流端添加到发流线程
			
			if (addr.tranfer == SIP_TRANSFER_TCP_ACTIVE)
			{
				pSender->link();
			}
		}
	}
}

void RtpService::on_cancel_request(socket_stream* stream, const char * data, size_t dlen)
{

}

void RtpService::on_ack_request(socket_stream* stream, const char * data, size_t dlen)
{
	acl::string szData(data, dlen);
	acl::string strAddr = stream->get_peer(true);
	char szBuffer[SIP_HEADER_LEN] = { 0 };
	char szBody[SIP_BODY_LEN] = { 0 };
	int nBodyLen = 0;
	acl::string strCallID = sip_get_header(szBuffer, sizeof(szBuffer), szData.c_str(), SIP_H_CALL_ID);
	sip_get_body(szData.c_str(), szBody, sizeof(szBody));

	sesmap_iter siter = __sesmap.find(strCallID.c_str());
	if (siter == __sesmap.end())
	{
		printf("ACK没有找到匹配的会话\n");
		return;
	}
	session_info *pSess = siter->second;
	_recvthdmap_itor iterRecv;
	if (pSess->invite_type == SIP_INVITE_PLAY)
	{
		iterRecv = _recv_thd_map.find(pSess->deviceid);
	}
	else
	{
		iterRecv = _recv_thd_map.find(pSess->callid);
	}
	if (iterRecv == _recv_thd_map.end())
	{
		printf("ACK没有找到匹配的连接\n");
		return;
	}
	
	if (szBody[0] != '\0') //确认收流
	{
		rtp_recv_thread *pRecvthd = (rtp_recv_thread *)iterRecv->second;
		sip_sdp_info sdp;
		memset(&sdp, 0, sizeof(sip_sdp_info));
		sip_get_sdp_info(szBody, &sdp);
		addr_info addr;
		memset(&addr, 0, sizeof(addr_info));
		sprintf(addr.addr, "%s:%d", sdp.o_ip, sdp.m_port);
		strcpy(addr.ssrc, sdp.y_ssrc);
		if (sdp.a_direct == SIP_SENDONLY)
		{
			int nTranfer = 0;
			if (sdp.m_protocol == 1)
				nTranfer = 2;
			else if (sdp.m_protocol == 2)
				nTranfer = 1;
			addr.tranfer = nTranfer;
			strcpy(pSess->ssrc, sdp.y_ssrc);
			strcpy(pRecvthd->stream_ssrc, sdp.y_ssrc);
			pRecvthd->rtptranfer = nTranfer;
			pRecvthd->media_type = sdp.m_type;
			pRecvthd->playload = sdp.a_playload;
			strcpy(pRecvthd->stream_type, sdp.a_streamtype);
			pRecvthd->set_mediainfo(&addr);	//连接发流端
		}
		else if (sdp.a_direct == SIP_RECVONLY)	//确认发流
		{
			addr.tranfer = sdp.m_protocol;
			rtp_sender *pSender = (rtp_sender *)pSess->rtp_stream;
			if (pSender == NULL)
			{
				return;
			}
			pSender->sendtranfer = sdp.m_protocol;
			pSender->sendssrc = atoi(sdp.y_ssrc);
			pSender->add(addr);
			pSender->link(); //连接收流端
		}
	}
	else                   //确认发流
	{
		rtp_sender *pSender = (rtp_sender *)pSess->rtp_stream;
		if (pSender == NULL)
		{
			return;
		}
		if (pSess->transfrom_sdk)
		{
			RtpTransform *pRecvthd = (RtpTransform *)iterRecv->second;
			pRecvthd->set_mediainfo(NULL);
		}
		if (pSender->sendaddr.tranfer != SIP_TRANSFER_TCP_ACTIVE)
		{
			pSender->link(); //连接收流端
		}
	}

}

void RtpService::on_bye_request(socket_stream* stream, const char * data, size_t dlen)
{
	acl::string szData(data, dlen);
	acl::string strAddr = stream->get_peer(true);
	char szSubject[SIP_HEADER_LEN] = { 0 };
	char szBuffer[SIP_HEADER_LEN] = { 0 };
	char szAns[SIP_MSG_LEN] = { 0 };
	int nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), szData.c_str(), SIP_STATUS_200);
	acl::string strCallID = sip_get_header(szBuffer, sizeof(szBuffer), szData.c_str(), SIP_H_CALL_ID);
	sesmap_iter siter = __sesmap.find(strCallID.c_str());
	if (siter != __sesmap.end())
	{
		session_info *pInfo = siter->second;
		acl::string strDeviceID = siter->second->deviceid;
		if (pInfo->invite_type == SIP_INVITE_PLAY)
		{
			_recvthdmap_locker.lock();
			_recvthdmap_itor itorRecv = _recv_thd_map.find(pInfo->deviceid);
			if (itorRecv != _recv_thd_map.end())
			{
				int nSize = 0;
				if (pInfo->transfrom_sdk)
				{
					RtpTransform *pRecvThd = (RtpTransform *)itorRecv->second;
					nSize = pRecvThd->client_size() - 1;
				}
				else
				{
					rtp_recv_thread *pRecvThd = (rtp_recv_thread *)itorRecv->second;
					nSize = pRecvThd->client_size() - 1;
				}
				sprintf(szSubject, "%s:%s,%s:%d", pInfo->deviceid, pInfo->ssrc, pInfo->callid, nSize>0?nSize:0);
				nAnsLen = sip_set_header(szAns, sizeof(szAns), SIP_H_SUBJECT, szSubject);
			}
			_recvthdmap_locker.unlock();
		}
		else
		{
			sprintf(szSubject, "%s:%s,%s:0", pInfo->deviceid, pInfo->ssrc, pInfo->callid);
			nAnsLen = sip_set_header(szAns, sizeof(szAns), SIP_H_SUBJECT, szSubject);
		}
		printf("关闭镜头[%s][%s]\n", strDeviceID.c_str(), strCallID.c_str());
		session_over(strCallID.c_str(), SESSION_SENDER_BYE); //通知点播结束
	}
	send_msg(strAddr.c_str(), szAns, nAnsLen);
}

void RtpService::on_info_request(socket_stream* stream, const char * data, size_t dlen)
{
}

void RtpService::on_subscribe_request(socket_stream* stream, const char * data, size_t dlen)
{
}

void RtpService::on_message_request(socket_stream* stream, const char * data, size_t dlen)
{
	SIP_MESSAGE_TYPE msg_type = sip_get_message_type(data);
	switch (msg_type)
	{
	case SIP_CONTROL_PTZCMD:
		on_msg_ptzcmd_request(stream, data, dlen);
		break;
	case SIP_CONTROL_TELEBOOT:
		on_msg_teleboot_request(stream, data, dlen);
		break;
	case SIP_CONTROL_RECORDCMD:
		on_msg_recordcmd_request(stream, data, dlen);
		break;
	case SIP_CONTROL_GUARDCMD:
		on_msg_guardcmd_request(stream, data, dlen);
		break;
	case SIP_CONTROL_ALARMCMD:
		on_msg_alarmcmd_request(stream, data, dlen);
		break;
	case SIP_CONTROL_IFAMECMD:
		on_msg_ifamecmd_request(stream, data, dlen);
		break;
	case SIP_CONTROL_DRAGZOOMIN:
		on_msg_dragzoomin_request(stream, data, dlen);
		break;
	case SIP_CONTROL_DRAGZOOMOUT:
		on_msg_dragzoomout_request(stream, data, dlen);
		break;
	case SIP_CONTROL_HOMEPOSITION:
		on_msg_homeposition_request(stream, data, dlen);
		break;
	case SIP_CONTROL_CONFIG:
		on_msg_config_request(stream, data, dlen);
		break;
	default:
		break;
	}
}

void RtpService::on_msg_ptzcmd_request(socket_stream* stream, const char * data, size_t dlen)
{
	acl::string szData(data, dlen);
	char szAns[SIP_MSG_LEN] = { 0 };
	int nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), szData.c_str(), SIP_STATUS_200);
	stream->write(szAns, nAnsLen);
	char szBuffer[SIP_HEADER_LEN] = { 0 };
	acl::string strCallID = sip_get_header(szBuffer, sizeof(szBuffer), szData.c_str(), SIP_H_CALL_ID);
	
	char szBody[SIP_BODY_LEN] = { 0 };
	sip_get_body(szData.c_str(), szBody, sizeof(szBody));
	acl::xml1 xml(szBody);
	acl::string strDeviceID = acl_xml_getElementValue(xml, "DeviceID");
	acl::string strCmd = acl_xml_getElementValue(xml, "PTZCmd");
	xml.reset();

	int cmd, type, index, speed;
	if (!ptzcmd_parse(strCmd.c_str(), &cmd, &type, &index, &speed))
		return;

	_recvthdmap_locker.lock();
	_recvthdmap_itor itorRecv = _recv_thd_map.find(strDeviceID.c_str());
	if (itorRecv != _recv_thd_map.end())
	{
		RtpTransform *pRecvThd = (RtpTransform *)itorRecv->second;
		if (pRecvThd)
		{
			pRecvThd->ctronl_ptz(type, cmd, index, speed);
		}
	}
	_recvthdmap_locker.unlock();

}

void RtpService::on_msg_teleboot_request(socket_stream* stream, const char * data, size_t dlen)
{
}

void RtpService::on_msg_recordcmd_request(socket_stream* stream, const char * data, size_t dlen)
{
}

void RtpService::on_msg_guardcmd_request(socket_stream* stream, const char * data, size_t dlen)
{
}

void RtpService::on_msg_alarmcmd_request(socket_stream* stream, const char * data, size_t dlen)
{
}

void RtpService::on_msg_ifamecmd_request(socket_stream* stream, const char * data, size_t dlen)
{
}

void RtpService::on_msg_dragzoomin_request(socket_stream* stream, const char * data, size_t dlen)
{
}

void RtpService::on_msg_dragzoomout_request(socket_stream* stream, const char * data, size_t dlen)
{
}

void RtpService::on_msg_homeposition_request(socket_stream* stream, const char * data, size_t dlen)
{
}

void RtpService::on_msg_config_request(socket_stream* stream, const char * data, size_t dlen)
{
}

void RtpService::on_msg_preset_request(socket_stream* stream, const char * data, size_t dlen)
{
}

void RtpService::on_msg_devicestate_request(socket_stream* stream, const char * data, size_t dlen)
{
}

void RtpService::on_msg_deviceinfo_request(socket_stream* stream, const char * data, size_t dlen)
{
}

void RtpService::on_msg_recordinfo_request(socket_stream* stream, const char * data, size_t dlen)
{
}

void RtpService::on_msg_alarm_request(socket_stream* stream, const char * data, size_t dlen)
{
}

void RtpService::on_msg_configdownload_request(socket_stream* stream, const char * data, size_t dlen)
{
}

void RtpService::on_msg_mobileposition_request(socket_stream* stream, const char * data, size_t dlen)
{
}

void RtpService::on_msg_mediastatus_notify(socket_stream* stream, const char * data, size_t dlen)
{
}

void RtpService::scanf_command()
{
	char command[1024];
	while (m_running)
	{
		scanf("%s", command);
		if (strcmp(command, "cls") == 0)
		{
			system("cls");
		}
	}
}

void RtpService::register_manager()
{
	acl::string strRegList = m_conf.reg_list;
	std::vector<acl::string>& vecList = strRegList.split2("@,;");
	for (size_t i = 0; i < vecList.size(); i+=3)
	{
		register_info rinfo;
		memset(&rinfo, 0, sizeof(rinfo));
		strcpy(rinfo.user, vecList[i].c_str());
		strcpy(rinfo.addr, vecList[i + 1].c_str());
		strcpy(rinfo.pwd, m_conf.default_pwd);
		rinfo.vaild = atoi(vecList[i + 2].c_str())==1?true:false;
		rinfo.expires = 36000;
		rinfo.tranfer = m_conf.clnt_tranfer;
		rinfo.online = false;
		rinfo.role = SIP_ROLE_SUPERIOR;
		__regmap[rinfo.addr] = rinfo;
	}
	while (m_running)
	{
		regmap_iter riter = __regmap.begin();
		while (riter != __regmap.end())
		{
			time_t now = time(NULL);
			if (riter->second.vaild == 0)	//失效
			{
				riter++;
				continue;
			}
			if (riter->second.online == false && now - riter->second.alivetime > SIP_REGISTERTIME)
			{
				//不在线，每10秒注册一次
				char szTo[SIP_HEADER_LEN] = { 0 };
				char szMsg[SIP_MSG_LEN] = { 0 };
				sip_build_msg_to(szTo, sizeof(szTo), riter->second.user, riter->second.addr);
				int nMsgLen = sip_build_msg_register(szMsg, sizeof(szMsg), szTo, NULL, 1, 36000);
				int nSendReg = 1;
				while (!send_msg(riter->second.addr, szMsg, nMsgLen))
				{
					m_conf.log(LOG_ERROR, "regist is false try:%d", ++nSendReg);
					if (nSendReg > 10)
						break;
					sip_sleep(100);
				}
				riter->second.alivetime = time(NULL);
			}
			else if (now - riter->second.alivetime > SIP_ALIVETIMEOUT)
			{
				riter->second.online = false;
				//掉线,超时重新注册
				char szTo[SIP_HEADER_LEN] = { 0 };
				char szMsg[SIP_MSG_LEN] = { 0 };
				sip_build_msg_to(szTo, sizeof(szTo), riter->second.user, riter->second.addr);
				int nMsgLen = sip_build_msg_register(szMsg, sizeof(szMsg), szTo, NULL, 1, 36000);
				int nSendReg = 1;
				while (!send_msg(riter->second.addr, szMsg, nMsgLen))
				{
					m_conf.log(LOG_ERROR, "regist is false try:%d", ++nSendReg);
					if (nSendReg > 10)
						break;
					sip_sleep(100);
				}
				riter->second.alivetime = time(NULL);
			}
			if (riter->second.online && now - riter->second.heartbeat > SIP_ALIVETIME)
			{
				//每60秒发送心跳
				char szTo[SIP_HEADER_LEN] = { 0 };
				char szMsg[SIP_MSG_LEN] = { 0 };
				sip_build_msg_to(szTo, sizeof(szTo), riter->second.user, riter->second.addr);
				int nMsgLen = sip_build_msg_keepalive(szMsg, sizeof(szMsg), szTo);
				send_msg(riter->second.addr, szMsg, nMsgLen);
				riter->second.heartbeat = time(NULL);
			}
			riter++;
		}
		sleep(1);
	}
}

void RtpService::session_manager()
{
	while (m_running)
	{
		sesmap_iter siter = __sesmap.begin();
		while (siter != __sesmap.end())
		{
			if (siter->second->mediaover > 0)
			{
				acl::string strCallId = siter->second->callid;
				if (siter->second->flow != SESSION_SENDER_BYE)	//码流异常中断
				{
					printf("向SIP发送关闭[%s][%s]\n", siter->second->deviceid, siter->second->callid);
					send_bye(siter->second->callid);
				}
				if (siter->second->transfrom_sdk)
					rtp_stop_sdk_send(siter->second->callid, siter->second->deviceid, siter->second->invite_type);
				else
					rtp_stop_recv_send(siter->second->callid, siter->second->deviceid, siter->second->invite_type);
				rtp_sender *sender = (rtp_sender *)siter->second->rtp_stream;
				if (sender)
					sender->close();
				session_info *pSess = siter->second;
				siter = __sesmap.erase(siter);
				delete pSess;
				pSess = NULL;
				//printf("释放会话[%s]资源\n", strCallId.c_str());
			}
			else
				siter++;
		}
		sleep(1);
	}
}

void RtpService::session_over(const char * callid, int session_type)
{
	sesmap_iter siter = __sesmap.find(callid);
	if (siter != __sesmap.end())
	{
		if (siter->second->flow < session_type)
		{
			printf("通知会话[%s][%d]结束\n", callid, session_type);
			siter->second->flow = session_flow(session_type);
			siter->second->mediaover = time(NULL);
		}
	}
}

void RtpService::rtp_cls_recv_send(const char *callid, const char *deviceid, int playtype)
{
	if (playtype == SIP_INVITE_PLAY)
	{
		_recvthdmap_locker.lock();
		_recvthdmap_itor itorRecv = _recv_thd_map.find(deviceid);
		if (itorRecv != _recv_thd_map.end())
		{
			_recv_thd_map.erase(itorRecv);
			printf("停止收流端[%s][%s]\n", deviceid, callid);
		}
		_recvthdmap_locker.unlock();
		
	}
	else
	{
		_recvthdmap_itor itorRecv = _recv_thd_map.find(callid);
		if (itorRecv != _recv_thd_map.end())
		{
			_recv_thd_map.erase(itorRecv);
			printf("停止收流端[%s][%s]\n", deviceid, callid);
		}
	}
}

void RtpService::message_manager()
{
}

void RtpService::subscribe_manager()
{
}

void RtpService::rtp_init(const char * send_ports, const char * recv_ports)
{
	m_nBeginSendPort = 7210;
	m_nEndSendPort = 7710;
	m_nBeginRecvPort = 17210;
	m_nEndRecvPort = 17710;
	if (send_ports != NULL && send_ports[0] != '\0')
	{
		char *p = (char *)strchr(send_ports, '-');
		if (p)
		{
			sscanf(send_ports, "%d-%d", &m_nBeginSendPort, &m_nEndSendPort);
		}
		else
		{
			m_nBeginSendPort = atoi(send_ports);
			m_nEndSendPort = m_nBeginSendPort;
		}
	}
	if (recv_ports != NULL && recv_ports[0] != '\0')
	{
		char *p = (char *)strchr(recv_ports, '-');
		if (p)
		{
			sscanf(recv_ports, "%d-%d", &m_nBeginRecvPort, &m_nEndRecvPort);
		}
		//else
		//{
		//	m_nBeginRecvPort = atoi(recv_ports);
		//	m_nEndRecvPort = m_nBeginRecvPort;
		//}
	}
}

void *RtpService::rtp_get_send_port(int tranfer)
{
	int nType = tranfer == 0 ? 0 : 1;
	int nPort = m_nBeginSendPort;
	int nKey = 0;
	std::map<int, rtp_sender *>::iterator itor;
	_send_port_locker.lock();
	while (true)
	{
		if (nPort > m_nEndSendPort)
			break;
		nKey = nPort * 10 + nType;
		itor = _send_port_map.find(nKey);
		if (itor != _send_port_map.end())
		{
			if (itor->second && !itor->second->status()) //端口重复利用
			{
				_send_port_locker.unlock();
				return itor->second;
			}
			nPort += 2;
		}
		else                    //新增端口
		{
			rtp_sender *pSender = new rtp_sender(m_conf.local_addr, nPort, tranfer);
			//处理被其他程序占用的端口
			for (int i = nPort; i < pSender->sendport; i += 2)
			{
				nKey = i * 10 + nType;
				_recv_port_map[nKey] = NULL;
			}
			nKey = pSender->sendport * 10 + nType;
			_send_port_map[nKey] = pSender;
			_send_port_locker.unlock();
			return pSender;
		}
	}
	_send_port_locker.unlock();
	return NULL;
}

void RtpService::rtp_free_send_port(int port, int tranfer)
{
	int nType = tranfer == 0 ? 0 : 1;
	int nKey = port * 10 + nType;
	_send_port_locker.lock();
	std::map<int, rtp_sender *>::iterator itor = _send_port_map.find(nKey);
	if (itor != _send_port_map.end())
	{
		delete itor->second;
		itor = _send_port_map.erase(itor);
	}
	_send_port_locker.unlock();
}

void *RtpService::rtp_get_recv_thd(int tranfer)
{
	int nType = tranfer == 0 ? 0 : 1;
	int nPort = m_nBeginRecvPort;
	int nKey = 0;
	std::map<int, rtp_recv_thread *>::iterator itor;
	_recv_port_locker.lock();
	while (true)
	{
		if (nPort > m_nEndRecvPort)
			break;
		nKey = nPort * 10 + nType;
		itor = _recv_port_map.find(nKey);
		if (itor != _recv_port_map.end())
		{
			if (itor->second && !itor->second->get_status())//端口己运行,重复利用
			{
				_recv_port_locker.unlock();
				return itor->second;
			}
			nPort += 2;
		}
		else                    //新增端口
		{
			rtp_recv_thread *pThread = new rtp_recv_thread(m_conf.local_addr, nPort, tranfer, this);
			pThread->set_detachable(true);
			pThread->start();
			//处理被其他程序占用的端口
			for (int i = nPort; i < pThread->rtpport; i+=2)
			{
				nKey = i * 10 + nType;
				_recv_port_map[nKey] = NULL;
			}
			nKey = pThread->rtpport * 10 + nType;
			_recv_port_map[nKey] = pThread;
			_recv_port_locker.unlock();
			return pThread;
		}
	}
	printf("无可用端口\n");
	_recv_port_locker.unlock();
	return NULL;
}

void RtpService::rtp_stop_recv_send(const char * callid, const char * deviceid, int playtype)
{
	if (playtype == SIP_INVITE_PLAY)
	{
		_recvthdmap_locker.lock();
		_recvthdmap_itor itorRecv = _recv_thd_map.find(deviceid);
		if (itorRecv != _recv_thd_map.end())
		{
			rtp_recv_thread *pRecvThd = (rtp_recv_thread *)itorRecv->second;
			printf("关闭收流端[%s][%s]\n", deviceid, callid);
			if (pRecvThd && pRecvThd->del_client(callid) == 0)
			{
				pRecvThd->dormant();
				_recv_thd_map.erase(itorRecv);
				printf("关闭发流端[%s][%s]\n", deviceid, callid);
			}
		}
		_recvthdmap_locker.unlock();
	}
	else
	{
		_recvthdmap_itor itorRecv = _recv_thd_map.find(callid);
		if (itorRecv != _recv_thd_map.end())
		{
			rtp_recv_thread *pRecvThd = (rtp_recv_thread *)itorRecv->second;
			if (pRecvThd)
				pRecvThd->dormant();
			_recv_thd_map.erase(itorRecv);
			printf("关闭发流端[%s][%s]\n", deviceid, callid);
		}
	}
}

void * RtpService::rtp_get_sdk_thd(ChannelInfo * pChn, void * rtp)
{
	std::list<RtpTransform *>::iterator itor = _recv_sdk_thdpool.begin();
	while (itor != _recv_sdk_thdpool.end())
	{
		if (!(*itor)->get_status())
		{
			(*itor)->set_channelinfo(pChn);
			return (*itor);
		}
		itor++;
	}
	RtpTransform *pRecvThd = new RtpTransform(pChn, rtp);
	pRecvThd->set_detachable(true);
	pRecvThd->start();
	return pRecvThd;
}

void RtpService::rtp_stop_sdk_send(const char * callid, const char * deviceid, int playtype)
{
	if (playtype == SIP_INVITE_PLAY)
	{
		_recvthdmap_locker.lock();
		_recvthdmap_itor itorRecv = _recv_thd_map.find(deviceid);
		if (itorRecv != _recv_thd_map.end())
		{
			RtpTransform *pRecvThd = (RtpTransform *)itorRecv->second;
			printf("关闭收流端[%s][%s]\n", deviceid, callid);
			if (pRecvThd && pRecvThd->del_client(callid) == 0)
			{
				pRecvThd->dormant();
				_recv_thd_map.erase(itorRecv);
				printf("关闭发流端[%s][%s]\n", deviceid, callid);
			}
		}
		_recvthdmap_locker.unlock();
	}
	else
	{
		_recvthdmap_itor itorRecv = _recv_thd_map.find(callid);
		if (itorRecv != _recv_thd_map.end())
		{
			RtpTransform *pRecvThd = (RtpTransform *)itorRecv->second;
			if (pRecvThd)
				pRecvThd->dormant();
			_recv_thd_map.erase(itorRecv);
			printf("关闭发流端[%s][%s]\n", deviceid, callid);
		}
	}
}

void RtpService::rtp_free_recv_port(int port, int tranfer)
{
	int nType = tranfer == 0 ? 0 : 1;
	int nKey = port * 10 + nType;
	_recv_port_locker.lock();
	std::map<int, rtp_recv_thread *>::iterator itor = _recv_port_map.find(nKey);
	if (itor != _recv_port_map.end())
	{
		itor = _recv_port_map.erase(itor);
	}
	_recv_port_locker.unlock();
}

bool RtpService::on_invite_transfrom_sdk(socket_stream* stream, int nPlayType, int nNetType, char *szDeviecID, char *szCallID, char *szFtag, char *szTtag, char *szBody, char *szMsg)

{
	char szAns[SIP_MSG_LEN] = { 0 };
	int nAnsLen = 0;
	int nBodyLen = 0;
	if (szBody[0] == '\0')
	{
		nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), szMsg, SIP_STATUS_488);
		stream->write(szAns, nAnsLen);
		return false;
	}
	sip_sdp_info sdp;
	memset(&sdp, 0, sizeof(sdp));
	sip_get_sdp_info(szBody, &sdp);
	int nTranfer = 0;
	if (sdp.m_protocol == 1)
		nTranfer = 2;
	else if (sdp.m_protocol == 2)
		nTranfer = 1;
	ChannelInfo cinfo;
	memset(&cinfo, 0, sizeof(cinfo));
	//解析SDP-U字段
	acl::string strChnInfo = sdp.u_uri;
	std::vector<acl::string> strParms = strChnInfo.split2(":"); //ChanNo:Channel:IP:Port:User:Pwd:Type
	if (strParms.size() < 7)
	{
		nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), szMsg, SIP_STATUS_488);
		stream->write(szAns, nAnsLen);
		return false;
	}
	strcpy(cinfo.szDeviceID, strParms[0].c_str());
	cinfo.nChannel = atoi(strParms[1].c_str());
	strcpy(cinfo.szIPAddress, strParms[2].c_str());
	cinfo.nPort = atoi(strParms[3].c_str());
	strcpy(cinfo.szUser, strParms[4].c_str());
	strcpy(cinfo.szPwd, strParms[5].c_str());
	strcpy(cinfo.szModel, strParms[6].c_str());
	RtpTransform *pRecvThd = NULL;
	if (nPlayType == SIP_INVITE_PLAY)	//实时点播
	{
		_recvthdmap_locker.lock();
		_recvthdmap_itor itorRecv = _recv_thd_map.find(szDeviecID);
		if (itorRecv != _recv_thd_map.end())	//已经有流
		{
			pRecvThd = (RtpTransform *)itorRecv->second;
			if (pRecvThd->get_status())
			{
				printf("己找到媒体流，进入分发\n");
			}
			else
			{
				pRecvThd->dormant();
				pRecvThd = NULL;
				_recv_thd_map.erase(itorRecv);
			}
		}
		_recvthdmap_locker.unlock();
	}
	if (pRecvThd == NULL)
	{
		pRecvThd = (RtpTransform *)rtp_get_sdk_thd(&cinfo, this);
		pRecvThd->set_stream_ssrc(sdp.y_ssrc);
		pRecvThd->set_playinfo(szCallID, szDeviecID, nPlayType);
		_recv_thd_map[szDeviecID] = pRecvThd;
	}
	rtp_sender *pSender = (rtp_sender *)rtp_get_send_port(sdp.m_protocol);
	if (pSender == NULL)
	{
		nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), szMsg, SIP_STATUS_503);
		stream->write(szAns, nAnsLen);
		return false;
	}
	printf("获取发送端口:%d\n", pSender->sendport);
	addr_info addr;
	memset(&addr, 0, sizeof(addr_info));
	sprintf(addr.addr, "%s:%d", sdp.o_ip, sdp.m_port);
	addr.tranfer = sdp.m_protocol;
	if (nNetType == SIP_TRANSFER_SDK_STANDARDING)
		addr.transfrom = true;
	strcpy(addr.ssrc, pRecvThd->get_stream_ssrc());
	pSender->add(addr);	//添加收流端
	pSender->sendssrc = atoi(pRecvThd->get_stream_ssrc());
	int nSendPort = pSender->sendport;
	char szPlayTime[64] = { 0 };
	sprintf(szPlayTime, "%lld %lld", sdp.t_begintime, sdp.t_endtime);
	nBodyLen = sip_build_msg_sdp(szBody, SIP_BODY_LEN, m_conf.local_code, szDeviecID, szPlayTime, m_conf.local_addr, nSendPort, sdp.s_type, SIP_SENDONLY,
		nTranfer, sdp.a_downloadspeed, sdp.m_type, pRecvThd->get_playload(), pRecvThd->get_stream_type());
	nBodyLen = sip_set_sdp_y(szBody, SIP_BODY_LEN, pRecvThd->get_stream_ssrc());
	nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), szMsg, SIP_STATUS_200);
	nAnsLen = sip_set_body(szAns, sizeof(szAns), szBody, SIP_APPLICATION_SDP);
	if (stream->write(szAns, nAnsLen))
	{
		session_info *pSess = new session_info;
		memset(pSess, 0, sizeof(session_info));
		strcpy(pSess->addr, stream->get_peer(true));
		strcpy(pSess->deviceid, szDeviecID);
		pSess->transfrom_sdk = true;
		sip_get_header(pSess->user_via, sizeof(pSess->user_via), szMsg, SIP_H_VIA);
		sip_get_header(pSess->user_from, sizeof(pSess->user_from), szMsg, SIP_H_FROM);
		sip_get_header(pSess->user_to, sizeof(pSess->user_to), szMsg, SIP_H_TO);
		strcpy(pSess->callid, szCallID);
		strcpy(pSess->ftag, szFtag);
		strcpy(pSess->ttag, szTtag);
		strcpy(pSess->invite_time, szPlayTime);
		pSess->cseq = sip_get_cseq_num(szMsg);
		pSess->invite_type = sdp.s_type;
		pSess->session_time = time(NULL);
		pSess->rtp_stream = pSender;
		pRecvThd->add_client(szCallID, pSender); //将收流端添加到发流线程
		pSess->session_time = time(NULL);
		__sesmap[szCallID] = pSess;

		if (addr.tranfer == SIP_TRANSFER_TCP_ACTIVE)
		{
			pSender->link();
		}

		return true;
	}
	return false;
}

bool RtpService::send_msg(const char * addr, const char * msg, size_t mlen)
{
	//std::vector<acl::socket_stream *> streams = get_sstreams();
	//if (streams.size() == 0)
	//{
	//	m_conf.log(LOG_ERROR, "send socket is not already!");
	//	return false;
	//}
	//streams[0]->set_peer(addr);
	//if (streams[0]->write(msg, mlen) == -1)
	//{
	//	return false;
	//}
	if (on_write(addr, msg, mlen))
		return true;
	else
		return false;
}

bool RtpService::send_bye(const char * callid)
{
	sesmap_iter siter = __sesmap.find(callid);
	if (siter == __sesmap.end())
	{
		return true;
	}
	session_info *pInfo = siter->second;
	char szMsg[SIP_MSG_LEN] = { 0 };
	int nMsgLen = 0;
	char szTo[SIP_HEADER_LEN] = { 0 };
	sip_build_msg_to(szTo, sizeof(szTo), pInfo->deviceid, pInfo->addr);
	nMsgLen = sip_build_msg_bye(szMsg, sizeof(szMsg), szTo, siter->second->cseq + 1, callid, siter->second->ftag, siter->second->ttag);
	return send_msg(pInfo->addr, szMsg, nMsgLen);
}

bool RtpService::ptzcmd_parse(const char * ptzcmd, int * nCmd, int *nType, int * nIndex, int * nSpeed)
{
	unsigned int bit0, bit1, bit2, bit3, bit4, bit5, bit6, bit7;
	sscanf(ptzcmd, "%02X%02X%02X%02X%02X%02X%02X%02X", &bit0, &bit1, &bit2, &bit3, &bit4, &bit5, &bit6, &bit7);
	if (bit0 != 0xA5 || bit1 != 0x0F || (bit0+bit1+bit2+bit3+bit4+bit5+bit6)%0x100 != bit7)
	{
		return false;
	}
	*nType = 0;
	*nIndex = 0;
	*nSpeed = 0;
	switch (bit3)
	{
	case SIP_PTZCMD_STOP:
		*nCmd = 0;
		break;
	case SIP_PTZCMD_FI_STOP:
		*nCmd = 0;
		break;
	case SIP_PTZCMD_UP:
		*nCmd = CAMERA_PAN_UP;
		*nSpeed = (bit5 + 1) / 32;
		break;
	case SIP_PTZCMD_DOWN:
		*nCmd = CAMERA_PAN_DOWN;
		*nSpeed = (bit5 + 1) / 32;
		break;
	case SIP_PTZCMD_LEFT:
		*nCmd = CAMERA_PAN_LEFT;
		*nSpeed = (bit4 + 1) / 32;
		break;
	case SIP_PTZCMD_RIGHT:
		*nCmd = CAMERA_PAN_RIGHT;
		*nSpeed = (bit4 + 1) / 32;
		break;
	case SIP_PTZCMD_LU:
		*nCmd = CAMERA_PAN_LU;
		*nSpeed = (bit4 + 1) / 32;
		break;
	case SIP_PTZCMD_LD:
		*nCmd = CAMERA_PAN_LD;
		*nSpeed = (bit4 + 1) / 32;
		break;
	case SIP_PTZCMD_RU:
		*nCmd = CAMERA_PAN_RU;
		*nSpeed = (bit4 + 1) / 32;
		break;
	case SIP_PTZCMD_RD:
		*nCmd = CAMERA_PAN_RD;
		*nSpeed = (bit4 + 1) / 32;
		break;
	case SIP_PTZCMD_ZOOM_IN:
		*nCmd = CAMERA_ZOOM_IN;
		*nSpeed = (bit6 + 1) / 32;
		break;
	case SIP_PTZCMD_ZOOM_OUT:
		*nCmd = CAMERA_ZOOM_OUT;
		*nSpeed = (bit6 + 1) / 32;
		break;
	case SIP_PTZCMD_FI_IRIS_IN:
		*nCmd = CAMERA_PAN_UP;
		*nSpeed = (bit5 + 1) / 32;
		break;
	case SIP_PTZCMD_FI_IRIS_OUT:
		*nCmd = CAMERA_PAN_UP;
		*nSpeed = (bit5 + 1) / 32;
		break;
	case SIP_PTZCMD_FI_FOCUS_IN:
		*nCmd = CAMERA_FOCUS_IN;
		*nSpeed = (bit4 + 1) / 32;
		break;
	case SIP_PTZCMD_FI_FOCUS_OUT:
		*nCmd = CAMERA_FOCUS_OUT;
		*nSpeed = (bit4 + 1) / 32;
		break;
	case SIP_PTZCMD_SET:
		*nCmd = PTZ_PRESET_SET;
		*nType = 4;
		*nIndex = bit5;
		break;
	case SIP_PTZCMD_GOTO:
		*nCmd = PTZ_PRESET_GOTO;
		*nType = 4;
		*nIndex = bit5;
		break;
	case SIP_PTZCMD_DELETE:
		*nCmd = PTZ_PRESET_DELETE;
		*nType = 4;
		*nIndex = bit5;
		break;
	case SIP_PTZCMD_CRUISE_ADD:
		*nCmd = PTZ_CRUISE_ADD;
		*nType = 4;
		*nIndex = bit5;
		break;
	case SIP_PTZCMD_CRUISE_DEL:
		*nCmd = PTZ_CRUISE_DEL;
		*nType = 4;
		*nIndex = bit5;
		break;
	case SIP_PTZCMD_CRUISE_SPEED:
		break;
	case SIP_PTZCMD_CRUISE_DTIME:
		*nCmd = PTZ_CRUISE_TIME;
		*nType = 4;
		*nIndex = bit5;
		*nSpeed = bit6;
		break;
	case SIP_PTZCMD_CRUISE_RUN:
		*nCmd = PTZ_CRUISE_LOOP;
		*nType = 4;
		*nIndex = bit5;
		break;
	case SIP_PTZCMD_AUTO_RUN:
		*nCmd = CAMERA_AUTO_TURN;
		*nType = bit6;
		*nIndex = bit5;
		break;
	case SIP_PTZCMD_AUTO_SPEED:
		*nCmd = CAMERA_AUTO_TURN;
		*nType = 3;
		*nIndex = bit5;
		*nSpeed = bit6;
		break;
	case SIP_PTZCMD_AUXIOPEN:
		if (bit5 == 1)
			*nCmd = CAMERA_BRUSH_CTRL;
		else if (bit5 == 2)
			*nCmd = CAMERA_LIGHT_CTRL;
		break;
	case SIP_PTZCMD_AUXICLOSE:
		*nCmd = 0;
		break;
	default:
		return false;
	}
	return true;
}
