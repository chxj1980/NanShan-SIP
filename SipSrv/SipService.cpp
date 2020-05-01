 #include "stdafx.h"
#include "SipService.h"
#include "SipThread.h"

static SipService	    *__sipsrv = NULL;
static acl::thread_pool *__thdpool = NULL;


static register_map __regmap;		//注册管理
static acl::locker __reglocker;
static session_map	__sesmap;		//会话管理
static acl::locker __seslocker;
static session_map __subalmmap;		//报警订阅
static acl::locker __subalmlocker;
static session_map __subcatmap;		//目录订阅
static acl::locker __subcatlocker;
static session_map	__msgmap;		//消息管理
static acl::locker __msglocker;
static catalog_que __catalog;		//目录更新
static acl::locker __catlocker;

static channel_map  __chnmap;		//通道列表

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
	if (node ==  NULL)
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

//depth: 子级层数
acl::json_node *acl_json_get_child(acl::json_node *node, int depth)
{
	int i = 0;
	acl::json_node *child = node->first_child();
	while (child)
	{
		++i;
		if (i == depth)
		{
			return child;
		}
		child = child->first_child();
	}
	return NULL;
}

//Kafka消费者线程
#ifdef _KAFKA_USE
class consumer_thread : public acl::thread
{
public:
	consumer_thread(msg_consume cbMsg, void *pUser)
	{
		pSip = (SipService *)pUser;
		msg_cb = cbMsg;
	}
	~consumer_thread()
	{
		pSip = NULL;
	}
	void stop()
	{
		cons.Stop();
	}
protected:
	virtual void *run()
	{
		if (pSip)
		{
			cons.Start(pSip->m_conf.kfk_brokers,pSip->m_conf.kfk_topic,pSip->m_conf.kfk_group, msg_cb,pSip);
		}
		delete this;
		return NULL;
	}
private:
	SipService *pSip;
	Consumer	cons;
	msg_consume msg_cb;

};

void __stdcall consume_msgcb(int64_t offset, char *message, size_t msglen, void *user)
{
	//acl::charset_conv conv;
	//acl::string msgout;
	//conv.convert("utf-8", "gb2312", message, msglen, &msgout);
	//printf("offset:%I64d msglen:%d\n%s\n", offset, (int)msgout.size(), msgout.c_str());

	acl::json jsonstr(message);
	acl::json_node *alarmtype = jsonstr.getFirstElementByTagName("command");
	if (!alarmtype)
	{
		return;
	}
	char szTags[128] = { 0 };
	sprintf(szTags, "imagelist/%slist", alarmtype->get_text());
	printf("报警类型:%s\n", alarmtype->get_text());
	//acl::json_node *deviceid = jsonstr.getFirstElementByTags(szTags);
	const std::vector<acl::json_node*>& jsonlist = jsonstr.getElementsByTags(szTags);
	if (jsonlist.empty())
	{
		return;
	}
	acl::json_node *child = acl_json_get_child(jsonlist[0], 2);
	if (!child)
	{
		return;
	}
	acl::json_node *node = child->first_child();
	while (node)
	{
		printf("%s:%s\n", node->tag_name(), node->get_text());
		node = child->next_child();
	}
	jsonstr.reset();
}
#endif




/*****************************************************************************************************************/
/************************************************SIP服务消息处理类************************************************/
/*****************************************************************************************************************/
SipService::SipService()
{
	__sipsrv = this;
	m_running = true;
	m_conf.read();
	m_szRecvBuf = new char[SIP_RECVBUF_MAX];
	m_consumer_thd = NULL;
}

SipService::~SipService()
{
	m_running = false;
#ifdef _KAFKA_USE
	if (m_consumer_thd)
	{
		((consumer_thread *)m_consumer_thd)->stop();
		m_consumer_thd = NULL;
	}
#endif
	if (m_szRecvBuf)
	{
		delete[] m_szRecvBuf;
		m_szRecvBuf = NULL;
	}
	m_no_enter_list.clear();
}
//接收SIP消息
//void SipService::on_read(acl::socket_stream * stream)
//{
//	memset(m_szRecvBuf, 0, SIP_RECVBUF_MAX);
//	int nBufSize = stream->read(m_szRecvBuf, SIP_RECVBUF_MAX, false);
//	if (nBufSize < 0)
//	{
//		m_conf.log(LOG_ERROR, "read socket %s error", stream->get_peer(true));
//	}
//	if (nBufSize > 0)
//	{
//		if (is_no_enter_ip(stream->get_peer()))
//			return;
//		sip_handle_thread *handlethd = new sip_handle_thread(stream, m_szRecvBuf, nBufSize, this);
//		__thdpool->run(handlethd);
//	}
//}
void SipService::on_read(socket_stream * sock_stream, const char *data, size_t dlen)
{
	if (is_no_enter_ip(sock_stream->get_peer()))
		return;
	//__msglocker.lock();
	//on_handle(sock_stream,data,dlen);
	//__msglocker.unlock();
	sip_handle_thread *handlethd = new sip_handle_thread(sock_stream, data, dlen, this);
	__thdpool->run(handlethd);
}
//服务进程初始化
void SipService::proc_on_init()
{
	if (m_conf.no_enter_addrs[0] != '\0')
	{
		acl::string strNoEnterAddr = m_conf.no_enter_addrs;
		std::vector<acl::string> vecAddrLidt = strNoEnterAddr.split2(";");
		for (size_t i = 0; i < vecAddrLidt.size(); i++)
		{
			char *ip = strchr(vecAddrLidt[i].c_str(), '*');
			if (ip)
				ip[0] = '\0';
			m_no_enter_list.push_back(vecAddrLidt[i].c_str());
		}
	}

	__thdpool = new sip_thread_pool;
	__thdpool->set_limit(m_conf.srv_pool);
	__thdpool->set_idle(2);
	__thdpool->start();

	sip_init(m_conf.local_code, m_conf.local_addr, m_conf.local_port);
#ifdef _DBLIB_USE
	if (m_conf.db_driver != DB_NONE)	//读取数据库信息
	{
		if (m_db.Connect(m_conf.db_driver, m_conf.db_addr, m_conf.db_name, m_conf.db_user, m_conf.db_pwd, m_conf.db_pool))
		{
			m_db.GetRegisterInfo(m_conf.local_code, __regmap);
			m_db.GetChannelInfo(__chnmap, m_conf.srv_role);
			for (regmap_iter itor = __regmap.begin(); itor != __regmap.end(); itor++)
			{
				if (itor->second.role == SIP_ROLE_SUPERIOR && itor->second.protocol == SOCK_STREAM)
				{
					on_tcp_bind(itor->second.addr, itor->second.tcpport);
				}
			}
		}
#ifdef _WINDOWS
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN);
		printf("DB Read finished! register[%d], channelinfo[%d]\n", (int)__regmap.size(), (int)__chnmap.size());
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0x07);
#endif // DEBUG
	}
#endif
#ifdef _KAFKA_USE
	if (m_conf.kfk_brokers[0] != '\0' && m_conf.kfk_topic[0] != '\0')
	{
		printf("开启Kafka订阅线程!订阅地址:%s\n", m_conf.kfk_brokers);
		m_consumer_thd = new consumer_thread(consume_msgcb, this);
		m_consumer_thd->set_detachable(true);
		m_consumer_thd->start();
	}
#endif
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
	catalog_update *catalog_upthd = new catalog_update(this);
	catalog_upthd->set_detachable(true);
	catalog_upthd->start();
	if (m_conf.srv_role != SIP_ROLE_CLIENT)
	{
#ifdef _WINDOWS
		scanf_thread *scanf_thd = new scanf_thread(this);
		scanf_thd->set_detachable(true);
		scanf_thd->start();
#endif
	}
}
//服务进程退出
void SipService::proc_on_exit()
{
	m_running = false;
#ifdef _DBLIB_USE
	m_db.Disconnect();
#endif
#ifdef _KAFKA_USE
	if (m_consumer_thd)
	{
		((consumer_thread *)m_consumer_thd)->stop();
		m_consumer_thd = NULL;
	}
#endif
	if (__thdpool)
	{
		__thdpool->stop();
		delete __thdpool;
		__thdpool = NULL;
	}
	
}
//消息判断
void SipService::on_handle(socket_stream* stream, const char *data, size_t dlen)
{
	SIP_EVENT_TYPE even_type = sip_get_event_type(data);
	switch (even_type)
	{
	case SIP_REGISTER_REQUEST:
		on_register_request(stream, data, dlen);
		break;
	case SIP_REGISTER_SUCCESS:
		on_register_success(stream, data, dlen);
		break;
	case SIP_REGISTER_FAILURE:
		on_register_failure(stream, data, dlen);
		break;
	case SIP_INVITE_REQUEST:
		on_invite_request(stream, data, dlen);
		break;
	case SIP_INVITE_PROCEEDING:
		on_invite_proceeding(stream, data, dlen);
		break;
	case SIP_INVITE_SUCCESS:
		on_invite_success(stream, data, dlen);
		break;
	case SIP_INVITE_FAILURE:
		on_invite_failure(stream, data, dlen);
		break;
	case SIP_CANCEL_REQUEST:
		on_cancel_request(stream, data, dlen);
		break;
	case SIP_CANCEL_SUCCESS:
		on_cancel_success(stream, data, dlen);
		break;
	case SIP_CANCEL_FAILURE:
		on_cancel_failure(stream, data, dlen);
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
	case SIP_INFO_SUCCESS:
		on_info_success(stream, data, dlen);
		break;
	case SIP_INFO_FAILURE:
		on_info_failure(stream, data, dlen);
		break;
	case SIP_SUBSCRIBE_REQUEST:
		on_subscribe_request(stream, data, dlen);
		break;
	case SIP_SUBSCRIBE_SUCCESS:
		on_subscribe_success(stream, data, dlen);
		break;
	case SIP_SUBSCRIBE_FAILURE:
		on_subscribe_failure(stream, data, dlen);
		break;
	case SIP_NOTIFY_EVENT:
		on_notify_request(stream, data, dlen);
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
	case SIP_OPTIONS_RESPONSE:
		on_options_response(stream, data, dlen);
		break;
	default:
		break;
	}
}

//能力回应
void SipService::on_options_response(socket_stream * stream, const char * data, size_t dlen)
{
}
//注册成功
void SipService::on_register_success(socket_stream * stream, const char * data, size_t dlen)
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
			printf("重注册[%s]成功\n", sAddr);
		}
		riter->second.online = true;
		riter->second.alivetime = time(NULL);
	}
	else                            //注销成功
	{
		riter->second.online = false;
	}
}
//注册失败
void SipService::on_register_failure(socket_stream * stream, const char * data, size_t dlen)
{
	acl::string szData(data, dlen);
	char szBuffer[SIP_HEADER_LEN] = { 0 };
	const char *sAddr = stream->get_peer(true);
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
	int nLen = sip_build_msg_register(szMsg, sizeof(szMsg), strUri.c_str(),strAuth.c_str(),2,riter->second.expires, strCallID.c_str(), strFTag.c_str());
	stream->write(szMsg, nLen);
}
//点播等待
void SipService::on_invite_proceeding(socket_stream * stream, const char * data, size_t dle)
{
	
}
//点播成功
void SipService::on_invite_success(socket_stream * stream, const char * data, size_t dlen)
{
	acl::string szData(data, dlen);
	char szBuffer[SIP_HEADER_LEN] = { 0 };
	client_locker(true);
	acl::string strAddr = stream->get_peer(true);
	acl::string strTtag = sip_get_to_tag(szBuffer, sizeof(szBuffer), szData.c_str());
	acl::string strCallId = sip_get_header(szBuffer, sizeof(szBuffer), szData.c_str(), SIP_H_CALL_ID);
	int nCSeq = sip_get_cseq_num(szData.c_str());
	sesmap_iter siter = __sesmap.find(strCallId.c_str());
	if (siter == __sesmap.end())
	{
		client_locker(false);
		printf("收到[%s]200 OK:没有找到匹配的会话\n", strAddr.c_str());
		return;
	}
	session_info *pInfo = siter->second;
	if (pInfo->flow == SESSION_SENDER_REQ)
		pInfo->flow = SESSION_SENDER_OK;
	else if (pInfo->flow == SESSION_MEDIA_REQ)
		pInfo->flow = SESSION_MEDIA_OK;
	else if (pInfo->flow == SESSION_RECVER_REQ)
		pInfo->flow = SESSION_RECVER_OK;
	else if (pInfo->flow == SESSION_SMEDIA_REQ)
		pInfo->flow = SESSION_SMEDIA_OK;
	else
	{
		client_locker(false);
		return;
	}
	sip_sdp_info sdp;
	memset(&sdp, 0, sizeof(sip_sdp_info));
	char szBody[SIP_BODY_LEN] = { 0 };
	SIP_CONTENT_TYPE nBodyType = sip_get_body(szData.c_str(), szBody, sizeof(szBody));
	if (nBodyType == SIP_APPLICATION_SDP)
		sip_get_sdp_info(szBody, &sdp);
	else
	{
		client_locker(false);
		return;
	}
	if (m_conf.rtp_check_ssrc)
	{
		if (sdp.y_ssrc[0] != '\0')
			strcpy(pInfo->ssrc, sdp.y_ssrc);	
	}
	else
		memset(sdp.y_ssrc, 0, sizeof(sdp.y_ssrc));
	acl::string strUser = sip_get_to_user(szBuffer, sizeof(szBuffer), szData.c_str());
	int nRole = sip_get_user_role(strUser.c_str());
	if (nRole == SIP_ROLE_CHANNEL || nRole == SIP_ROLE_SENDER)
	{
		strcpy(pInfo->ttag, strTtag.c_str());
		if (pInfo->flow == SESSION_SMEDIA_OK)	
		{
			if (pInfo->terminalid[0] != '\0'  && strlen(pInfo->terminalid) == 20)  //第三方
			{
				char szAck[SIP_MSG_LEN] = { 0 };
				int nAckLen = 0;
				//D11 回复媒体接收者ACK(媒体SDP) !!
				acl::string strTo = sip_build_msg_to(szBuffer, sizeof(szBuffer), pInfo->terminalid, pInfo->terminal_addr);
				nAckLen = sip_build_msg_ack(szAck, sizeof(szAck), strTo.c_str(), nCSeq, pInfo->callid, pInfo->ftag, pInfo->terminal_ttag);
				nAckLen = sip_set_body(szAck, sizeof(szAck), szBody, nBodyType);
				send_msg(pInfo->terminal_addr, szAck, nAckLen);
				//D12 回复RTP流媒体ACK
				nAckLen = sip_build_msg_ack(szAck, sizeof(szAck), szData.c_str());
				stream->write(szAck, nAckLen);
			}
			//直连 B10 回复客户端200OK(设备SDP)
			char szAns[SIP_MSG_LEN] = { 0 };
			int nAnsLen = sip_build_invite_answer(szAns, sizeof(szAns), SIP_STATUS_200, pInfo->user_via, pInfo->user_from, pInfo->user_to, pInfo->ttag, pInfo->callid, pInfo->deviceid, pInfo->cseq);
			if (m_conf.public_mapping && is_client_addr(pInfo->user_addr))
			{
				sip_set_sdp_ip(szBody, sizeof(szBody), m_conf.public_addr);
			}
			nAnsLen = sip_set_body(szAns, sizeof(szAns), szBody, nBodyType);
			send_msg(pInfo->user_addr, szAns, nAnsLen);
		}
		else
		{
			char szAck[SIP_MSG_LEN] = { 0 };
			int nAckLen = 0;
			if (pInfo->media_addr[0] != '\0')//A6回复RTP流媒体ACK(下级SDP)| C5
			{
				acl::string strTo = sip_build_msg_to(szBuffer, sizeof(szBuffer), pInfo->media_user, pInfo->media_addr);
				nAckLen = sip_build_msg_ack(szAck, sizeof(szAck), strTo.c_str(), nCSeq, pInfo->callid, pInfo->ftag, pInfo->ttag);
				char szDstIp[24] = { 0 };
				strcpy(szDstIp, pInfo->addr);
				(strrchr(szDstIp, ':'))[0] = '\0';
				regmap_iter riter = __regmap.find(pInfo->addr);
				if (riter != __regmap.end())
				{
					if (riter->second.mapping_addr[0] != '\0')
						sip_set_sdp_ip(szBody, sizeof(szBody), szDstIp);
				}
				nAckLen = sip_set_body(szAck, sizeof(szAck), szBody, nBodyType);
				send_msg(pInfo->media_addr, szAck, nAckLen);
				pInfo->flow = SESSION_MEDIA_ACK;
			}
			//A7回复下级ACK | C6
			nAckLen = sip_build_msg_ack(szAck, sizeof(szAck), szData.c_str());
			stream->write(szAck, nAckLen);

			if (pInfo->flow == SESSION_MEDIA_ACK)//A8 向RTP流媒体再次点播请求(上级SDP)
			{
				char szInvite[SIP_MSG_LEN] = { 0 };
				char szTo[SIP_HEADER_LEN] = { 0 };
				char szSubJect[SIP_HEADER_LEN] = { 0 };
				sip_build_msg_to(szTo, sizeof(szTo), pInfo->media_user, pInfo->media_addr);
				sip_build_msg_subject(szSubJect, sizeof(szSubJect), pInfo->deviceid, pInfo->ssrc, pInfo->terminalid);
				int nMsgLen = sip_build_msg_invite(szInvite, sizeof(szInvite), szTo, szSubJect, nCSeq, pInfo->callid, pInfo->ftag);
				acl::string strRecvIp = pInfo->receiver.recvip;
				if (m_conf.public_mapping && is_client_addr(pInfo->user_addr))
					strRecvIp = pInfo->user_addr;
				char szClientBody[SIP_BODY_LEN] = { 0 };
				sip_build_msg_sdp(szClientBody, sizeof(szClientBody), pInfo->receiver.receiverid, pInfo->deviceid, pInfo->invite_time, strRecvIp.c_str(), pInfo->receiver.recvport,
					pInfo->invite_type, SIP_RECVONLY, pInfo->receiver.recvprotocol);
				sip_set_sdp_y(szClientBody, sizeof(szClientBody), pInfo->ssrc);
				nMsgLen = sip_set_body(szInvite, sizeof(szInvite), szClientBody, SIP_APPLICATION_SDP);
				send_msg(pInfo->media_addr, szInvite, nMsgLen);
				pInfo->flow = SESSION_SMEDIA_REQ;
			}
#ifdef _USRDLL
			acl::string strDst;
			strDst.format("%s:%d", sdp.o_ip, sdp.m_port);
			media_stream_cb pCb = (media_stream_cb)pInfo->stream_cb;
			if (pCb)
				pCb(200, NULL, sdp.a_playload, pInfo->stream_user);
			if (pInfo->terminalid[0] == '\0')
				pInfo->rtp_stream = sip_start_stream(m_conf.local_addr, pInfo->receiver.recvport, m_conf.clnt_tranfer, strDst.c_str(), pInfo, pCb, pInfo->stream_user);
#endif
		}
	}
	else if (nRole == SIP_ROLE_MEDIA)
	{
		if (sdp.a_direct == SIP_RECVONLY)	//RTP流媒体准备接收流
		{
			//A4向下级发送点播请求(媒体SDP) | C3
			char szInvite[SIP_MSG_LEN] = { 0 };
			char szTo[SIP_HEADER_LEN] = { 0 };
			char szSubJect[SIP_HEADER_LEN] = { 0 };
			sip_build_msg_to(szTo, sizeof(szTo), pInfo->deviceid, pInfo->addr);
			sip_build_msg_subject(szSubJect, sizeof(szSubJect), pInfo->deviceid, pInfo->ssrc, pInfo->terminalid);
			int nMsgLen = sip_build_msg_invite(szInvite, sizeof(szInvite), szTo, szSubJect, nCSeq, pInfo->callid, pInfo->ftag);
			regmap_iter riter = __regmap.find(pInfo->addr);
			if (riter != __regmap.end())
			{
				if (riter->second.mapping_addr[0] != '\0')
				{
					char szVia[SIP_HEADER_LEN] = {0};
					sip_bulid_msg_via(szVia, sizeof(szVia), riter->second.mapping_addr, m_conf.local_port);
					nMsgLen = sip_add_via(szInvite, sizeof(szInvite), szVia);
					acl::string strCotact;
					strCotact.format("<sip:%s@%s:%d>", m_conf.local_code, riter->second.mapping_addr, m_conf.local_port);
					nMsgLen = sip_set_header(szInvite, sizeof(szInvite), SIP_H_CONTACT, strCotact.c_str());
					sip_set_sdp_ip(szBody, sizeof(szBody), riter->second.mapping_addr);
				}
				riter->second.slist->push_back(pInfo);
				///printf("下级加入会话%s:%s\n", pInfo->addr, pInfo->callid);
			}
			sip_set_sdp_s(szBody, sizeof(szBody), SIP_INVITE_TYPE(pInfo->invite_type));
			sip_set_sdp_y(szBody, sizeof(szBody), pInfo->ssrc);
			nMsgLen = sip_set_body(szInvite, sizeof(szInvite), szBody, nBodyType);
			send_msg(pInfo->addr, szInvite, nMsgLen);
			pInfo->flow = SESSION_SENDER_REQ;		
		}
		else                                //RTP流媒体准备发送流
		{
			strcpy(pInfo->ttag, strTtag.c_str());
			if (sdp.a_direct == SIP_SENDRECV)
			{
				pInfo->flow = SESSION_SMEDIA_REP;
			}
			if (pInfo->terminalid[0] != '\0' && strlen(pInfo->terminalid) == 20) //第三方
			{
				char szAck[SIP_MSG_LEN] = { 0 };
				int nAckLen = 0;
				//C11 回复媒体接收者ACK(媒体SDP) 
				acl::string strTo = (szBuffer, sizeof(szBuffer), pInfo->terminalid, pInfo->terminal_addr);
				nAckLen = sip_build_msg_ack(szAck, sizeof(szAck), strTo.c_str(), nCSeq, pInfo->callid, pInfo->ftag, pInfo->terminal_ttag);
				nAckLen = sip_set_body(szAck, sizeof(szAck), szBody, nBodyType);
				send_msg(pInfo->terminal_addr, szAck, nAckLen);
				//C12 回复RTP流媒体ACK
				nAckLen = sip_build_msg_ack(szAck, sizeof(szAck), szData.c_str());
				stream->write(szAck, nAckLen);
			}
			//A10 回复客户端200OK(媒体SDP)
			char szAns[SIP_MSG_LEN] = { 0 };
			int nAnsLen = sip_build_invite_answer(szAns, sizeof(szAns), SIP_STATUS_200, pInfo->user_via, pInfo->user_from, pInfo->user_to, pInfo->ttag, pInfo->callid, pInfo->deviceid, pInfo->cseq);
			regmap_iter riter = __regmap.find(pInfo->user_addr);
			if (riter != __regmap.end())
			{
				if (riter->second.mapping_addr[0] != '\0')
					sip_set_sdp_ip(szBody, sizeof(szBody), riter->second.mapping_addr);
			}
			if (m_conf.public_mapping && is_client_addr(pInfo->user_addr))
			{
				sip_set_sdp_ip(szBody, sizeof(szBody), m_conf.public_addr);
			}
			nAnsLen = sip_set_body(szAns, sizeof(szAns), szBody, nBodyType);
			send_msg(pInfo->user_addr, szAns, nAnsLen);
		}
	}
	else if (nRole == SIP_ROLE_DECODER)
	{
		strcpy(pInfo->terminal_ttag, strTtag.c_str());
		if (pInfo->media_addr[0] != '\0')
		{
			//C9 向RTP流媒体再次点播请求(第三方SDP)
			char szInvite[SIP_MSG_LEN] = { 0 };
			char szTo[SIP_HEADER_LEN] = { 0 };
			char szSubJect[SIP_HEADER_LEN] = { 0 };
			sip_build_msg_to(szTo, sizeof(szTo), pInfo->media_user, pInfo->media_addr);
			sip_build_msg_subject(szSubJect, sizeof(szSubJect), pInfo->deviceid, pInfo->ssrc);
			int nMsgLen = sip_build_msg_invite(szInvite, sizeof(szInvite), szTo, szSubJect, nCSeq, pInfo->callid, pInfo->ftag);
			nMsgLen = sip_set_body(szInvite, sizeof(szInvite), szBody, SIP_APPLICATION_SDP);
			send_msg(pInfo->media_addr, szInvite, nMsgLen);
			pInfo->flow = SESSION_SMEDIA_REQ;
		}
		else
		{
			//D9 向下级送点播请求(第三方SDP)
			char szInvite[SIP_MSG_LEN] = { 0 };
			char szTo[SIP_HEADER_LEN] = { 0 };
			char szSubJect[SIP_HEADER_LEN] = { 0 };
			sip_build_msg_to(szTo, sizeof(szTo), pInfo->deviceid, pInfo->addr);
			sip_build_msg_subject(szSubJect, sizeof(szSubJect), pInfo->deviceid, pInfo->ssrc, pInfo->terminalid);
			int nMsgLen = sip_build_msg_invite(szInvite, sizeof(szInvite), szTo, szSubJect, nCSeq, pInfo->callid, pInfo->ftag);
			nMsgLen = sip_set_body(szInvite, sizeof(szInvite), szBody, SIP_APPLICATION_SDP);
			send_msg(pInfo->addr, szInvite, nMsgLen);
			pInfo->flow = SESSION_SMEDIA_REQ;
			regmap_iter riter = __regmap.find(pInfo->addr);
			if (riter != __regmap.end())
			{
				riter->second.slist->push_back(pInfo);
				//printf("下级加入会话%s:%s\n", pInfo->addr, pInfo->callid);
			}
		}
	}
	client_locker(false);
}
//点播失败
void SipService::on_invite_failure(socket_stream * stream, const char * data, size_t dlen)
{
	acl::string szData(data, dlen);
	char szBuffer[SIP_HEADER_LEN] = { 0 };
	client_locker(true);
	const char *sAddr = stream->get_peer();
	acl::string strTtag = sip_get_to_tag(szBuffer, sizeof(szBuffer), szData.c_str());
	acl::string strCallId = sip_get_header(szBuffer, sizeof(szBuffer), szData.c_str(), SIP_H_CALL_ID);
	int nCSeq = sip_get_cseq_num(szData.c_str());
	sesmap_iter siter = __sesmap.find(strCallId.c_str());
	if (siter == __sesmap.end())
	{
		client_locker(false);
		printf("收到[%s]失败回应:没有找到匹配的会话\n", sAddr);
		return;
	}
	session_info *pInfo = siter->second;
	strcpy(pInfo->ttag, strTtag.c_str());

	if (m_conf.srv_role != SIP_ROLE_CLIENT)
	{
		char szAns[SIP_MSG_LEN] = { 0 };
		int nAnsLen = sip_build_invite_answer(szAns, sizeof(szAns), SIP_STATUS_400, pInfo->user_via, pInfo->user_from, pInfo->user_to, pInfo->ttag, pInfo->callid, pInfo->deviceid, nCSeq);
		send_msg(pInfo->user_addr, szAns, nAnsLen);
	}
#ifdef _USRDLL
	media_stream_cb pCb = (media_stream_cb)pInfo->stream_cb;
	if (pCb && pInfo->stream_user)
		pCb(sip_get_status_code(szData.c_str()), NULL, 0, pInfo->stream_user);
#endif 
	pInfo->mediaover = time(NULL);
	client_locker(false);
}
//取消成功
void SipService::on_cancel_success(socket_stream * stream, const char * data, size_t dlen)
{
}
//取消失败
void SipService::on_cancel_failure(socket_stream * stream, const char * data, size_t dlen)
{
}
//关闭成功
void SipService::on_bye_success(socket_stream * stream, const char * data, size_t dlen)
{
	acl::string szData(data, dlen);
	const char *sAddr = stream->get_peer();
	char szBuffer[SIP_HEADER_LEN] = { 0 };
	acl::string strCallID = sip_get_header(szBuffer, sizeof(szBuffer), szData.c_str(), SIP_H_CALL_ID);
	acl::string strDeviceID = sip_get_subject_deviceid(szBuffer, sizeof(szBuffer), szData.c_str());
	acl::string strID = sip_get_subject_msid(szBuffer, sizeof(szBuffer), szData.c_str());
	int nUser = atoi(sip_get_subject_receiverid(szBuffer, sizeof(szBuffer), szData.c_str()));
	if (strID != "")
	{
		printf("当前镜头[%s]点播人数[%d]\n", strDeviceID.c_str(), nUser);
		if (nUser == 0)
		{
			sesmap_iter siter = __sesmap.find(strID.c_str());
			if (siter == __sesmap.end())
				return;
			session_info *pInfo = siter->second;
			char szMsg[SIP_MSG_LEN] = { 0 };
			char szTo[SIP_HEADER_LEN] = { 0 };
			sip_build_msg_to(szTo, sizeof(szTo), pInfo->deviceid, pInfo->addr);
			int nMsgLen = sip_build_msg_bye(szMsg, sizeof(szMsg), szTo, siter->second->cseq + 1, strID.c_str(), siter->second->ftag, siter->second->ttag);
			send_msg(pInfo->addr, szMsg, nMsgLen);
			pInfo->flow = SESSION_SENDER_BYE;
			pInfo->mediaover = time(NULL);
		}
		else
		{
			sesmap_iter siter = __sesmap.find(strCallID.c_str());
			if (siter == __sesmap.end())
				return;
			session_info *pInfo = siter->second;
			pInfo->flow = SESSION_SENDER_BYE;
			pInfo->mediaover = time(NULL);
		}
	}
}
//关闭失败
void SipService::on_bye_failure(socket_stream * stream, const char * data, size_t dlen)
{
}
//回放控制成功
void SipService::on_info_success(socket_stream * stream, const char * data, size_t dlen)
{
}
//回放控制失败
void SipService::on_info_failure(socket_stream * stream, const char * data, size_t dlen)
{
}
//订阅成功
void SipService::on_subscribe_success(socket_stream * stream, const char * data, size_t dlen)
{
}
//订阅失败
void SipService::on_subscribe_failure(socket_stream * stream, const char * data, size_t dlen)
{
}
//通知成功
void SipService::on_notify_success(socket_stream * stream, const char * data, size_t dlen)
{
}
//通知失败
void SipService::on_notify_failure(socket_stream * stream, const char * data, size_t dlen)
{
}
//查询成功
void SipService::on_message_success(socket_stream * stream, const char * data, size_t dlen)
{
	acl::string szData(data, dlen);
	if (sip_get_cseq_num(szData.c_str()) == SIP_CSEQ_KEEPALIVE)	//心跳
	{
		const char *sAddr = stream->get_peer(true);
		regmap_iter riter = __regmap.find(sAddr);
		if (riter != __regmap.end())
		{
			riter->second.alivetime = time(NULL);
		}
	}
}
//查询失败
void SipService::on_message_failure(socket_stream * stream, const char * data, size_t dlen)
{
}
//控制回应
void SipService::on_msg_control_response(socket_stream * stream, const char * data, size_t dlen)
{
	acl::string strAddr = stream->get_peer(true);
	acl::string szData(data, dlen);
	char szAns[SIP_MSG_LEN] = { 0 };
	int nLen = sip_build_msg_answer(szAns, sizeof(szAns), szData.c_str(), SIP_STATUS_200);
	stream->write(szAns, nLen);

	char szBody[SIP_BODY_LEN] = { 0 };
	SIP_CONTENT_TYPE nBodyType = sip_get_body(szData.c_str(), szBody, sizeof(szBody));
	if (nBodyType != SIP_APPLICATION_XML)
		return;
	acl::xml1 xml(szBody);
	acl::string strSn = acl_xml_getElementValue(xml, "SN");
	sesmap_iter siter = __msgmap.find(strSn.c_str());
	if (siter == __msgmap.end())
		return;
	__msglocker.lock();
	siter->second->session_time = time(NULL);
	if (m_conf.srv_role == SIP_ROLE_CLIENT)
	{
		acl::string strDeviceID = acl_xml_getElementValue(xml, "DeviceID");
		acl::string strResult = acl_xml_getElementValue(xml, "Result");
	}
	else
		response_msg(siter->second, szData.c_str(), szData.size());
	delete siter->second;
	__msgmap.erase(siter);
	__msglocker.unlock();
	xml.reset();
}
//预置位回应
void SipService::on_msg_preset_response(socket_stream * stream, const char * data, size_t dlen)
{
	acl::string strAddr = stream->get_peer(true);
	acl::string szData(data, dlen);
	char szAns[SIP_MSG_LEN] = { 0 };
	int nLen = sip_build_msg_answer(szAns, sizeof(szAns), szData.c_str(), SIP_STATUS_200);
	stream->write(szAns, nLen);

	char szBody[SIP_BODY_LEN] = { 0 };
	SIP_CONTENT_TYPE nBodyType = sip_get_body(szData.c_str(), szBody, sizeof(szBody));
	if (nBodyType != SIP_APPLICATION_XML)
		return;
	acl::xml1 xml(szBody);
	acl::string strSn = acl_xml_getElementValue(xml, "SN");
	acl::xml_node *pRecList = xml.getFirstElementByTag("PresetList");
	if (!pRecList)
		return;
	sesmap_iter siter = __msgmap.find(strSn.c_str());
	if (siter == __msgmap.end())
		return;
	__msglocker.lock();
	siter->second->session_time = time(NULL);
	int nNum = atoi(pRecList->attr_value("Num"));
	if (m_conf.srv_role == SIP_ROLE_CLIENT)
	{
		acl::xml_node *pRecItem = pRecList->first_child();
		while (pRecItem)
		{
			acl::xml_node *pItem = pRecItem->first_child();
			while (pItem)
			{
				if (strcmp(pItem->tag_name(), "PresetID") == 0){}
				else if (strcmp(pItem->tag_name(), "PresetName") == 0){}
				pItem = pRecItem->next_child();
			}
			pRecItem = pRecList->next_child();
		}
	}
	else
		response_msg(siter->second, szData.c_str(), szData.size());
	delete siter->second;
	__msgmap.erase(siter);
	__msglocker.unlock();
	xml.reset();
}
//设备状态回应
void SipService::on_msg_devicestate_response(socket_stream * stream, const char * data, size_t dlen)
{
	acl::string strAddr = stream->get_peer(true);
	acl::string szData(data, dlen);
	char szAns[SIP_MSG_LEN] = { 0 };
	int nLen = sip_build_msg_answer(szAns, sizeof(szAns), szData.c_str(), SIP_STATUS_200);
	stream->write(szAns, nLen);

	char szBody[SIP_BODY_LEN] = { 0 };
	SIP_CONTENT_TYPE nBodyType = sip_get_body(szData.c_str(), szBody, sizeof(szBody));
	if (nBodyType != SIP_APPLICATION_XML)
		return;
	acl::xml1 xml(szBody);
	acl::string strSn = acl_xml_getElementValue(xml, "SN");
	acl::string strResult = acl_xml_getElementValue(xml, "Result");
	acl::string strOnline = acl_xml_getElementValue(xml, "Online");
	acl::string strStatus = acl_xml_getElementValue(xml, "Status");
	acl::string strReason = acl_xml_getElementValue(xml, "Reason");
	acl::string strEncode = acl_xml_getElementValue(xml, "Encode");
	acl::string strRecord = acl_xml_getElementValue(xml, "Record");
	acl::string strDeviceTime = acl_xml_getElementValue(xml, "DeviceTime");
	acl::xml_node *pList = xml.getFirstElementByTag("Alarmstatus");
	if (!pList)
		return;
	sesmap_iter siter = __msgmap.find(strSn.c_str());
	if (siter == __msgmap.end())
		return;
	__msglocker.lock();
	siter->second->session_time = time(NULL);
	if (m_conf.srv_role == SIP_ROLE_CLIENT)
	{
		int nNum = atoi(pList->attr_value("Num"));
		acl::xml_node *pSubItem = pList->first_child();
		while (pSubItem)
		{
			acl::xml_node *pItem = pSubItem->first_child();
			while (pItem)
			{
				if (strcmp(pItem->tag_name(), "DeviceID") == 0){}
				else if (strcmp(pItem->tag_name(), "DutyStatus") == 0){}
				pItem = pSubItem->next_child();
			}
			pSubItem = pList->next_child();
		}
	}
	else
		response_msg(siter->second, szData.c_str(), szData.size());
	delete siter->second;
	__msgmap.erase(siter);
	__msglocker.unlock();
	xml.reset();
}
//目录查询回应
void SipService::on_msg_catalog_response(socket_stream * stream, const char * data, size_t dlen)
{
	acl::string strAddr = stream->get_peer(true);
	acl::string szData(data, dlen);
	char szAns[SIP_MSG_LEN] = { 0 };
	int nLen = sip_build_msg_answer(szAns, sizeof(szAns), szData.c_str(), SIP_STATUS_200);
	stream->write(szAns, nLen);
	char szUser[SIP_RAND_LEN] = { 0 };
	sip_get_from_user(szUser, sizeof(szUser), szData.c_str());
	char szBody[SIP_BODY_LEN] = { 0 };
	SIP_CONTENT_TYPE nBodyType = sip_get_body(szData.c_str(), szBody, sizeof(szBody));
	if (nBodyType != SIP_APPLICATION_XML)
		return;
	acl::xml1 xml(szBody);
	int nSumNum	= acl_xml_getElementInt(xml, "SumNum");
	acl::string strSn = acl_xml_getElementValue(xml, "SN");
	acl::string strDeviceID = acl_xml_getElementValue(xml, "DeviceID");
	acl::xml_node *pDevList = xml.getFirstElementByTag("DeviceList");
	if (!pDevList)
		return;
	__msglocker.lock();
	sesmap_iter siter = __msgmap.find(strSn.c_str());
	if (siter == __msgmap.end())
	{
		acl::xml_node *pDevItem = pDevList->first_child();
		while (pDevItem)
		{
			CatalogInfo *pDev = new CatalogInfo;
			memset(pDev, 0, sizeof(CatalogInfo));
			acl::xml_node *pItem = pDevItem->first_child();
			while (pItem)
			{
				if (strcmp(pItem->tag_name(), "DeviceID") == 0)
					strcpy(pDev->szDeviceID, pItem->text());
				else if (strcmp(pItem->tag_name(), "Name") == 0)
					strcpy(pDev->szName, pItem->text()); //strcpy(pDev->szName, acl_u2a(pItem->text()).c_str());
				else if (strcmp(pItem->tag_name(), "Manufacturer") == 0)
					strcpy(pDev->szManufacturer, pItem->text());
				else if (strcmp(pItem->tag_name(), "Model") == 0)
					strcpy(pDev->szModel, pItem->text());
				else if (strcmp(pItem->tag_name(), "Owner") == 0)
					strcpy(pDev->szOwner, pItem->text());
				else if (strcmp(pItem->tag_name(), "CivilCode") == 0)
					strcpy(pDev->szCivilCode, pItem->text());
				else if (strcmp(pItem->tag_name(), "Block") == 0)
					strcpy(pDev->szBlock, pItem->text());
				else if (strcmp(pItem->tag_name(), "Address") == 0)
					strcpy(pDev->szAddress, pItem->text());
				else if (strcmp(pItem->tag_name(), "Parental") == 0)
					pDev->nParental = atoi(pItem->text());
				else if (strcmp(pItem->tag_name(), "ParentID") == 0)
					strcpy(pDev->szParentID, pItem->text());
				else if (strcmp(pItem->tag_name(), "SafetyWay") == 0)
					pDev->nSatetyWay = atoi(pItem->text());
				else if (strcmp(pItem->tag_name(), "RegisterWay") == 0)
					pDev->nRegisterWay = atoi(pItem->text());
				else if (strcmp(pItem->tag_name(), "CertNum") == 0)
					strcpy(pDev->szCertNum, pItem->text());
				else if (strcmp(pItem->tag_name(), "Certifiable") == 0)
					pDev->nCertifiable = atoi(pItem->text());
				else if (strcmp(pItem->tag_name(), "ErrCode") == 0)
					pDev->nErrCode = atoi(pItem->text());
				else if (strcmp(pItem->tag_name(), "EndTime") == 0)
					strcpy(pDev->szEndTime, pItem->text());
				else if (strcmp(pItem->tag_name(), "Secrecy") == 0)
					pDev->nSecrecy = atoi(pItem->text());
				else if (strcmp(pItem->tag_name(), "IPAddress") == 0)
					strcpy(pDev->szIPAddress, pItem->text());
				else if (strcmp(pItem->tag_name(), "Port") == 0)
					pDev->nPort = atoi(pItem->text());
				else if (strcmp(pItem->tag_name(), "Password") == 0)
					strcpy(pDev->szPassword, pItem->text());
				else if (strcmp(pItem->tag_name(), "Status") == 0)
					strcpy(pDev->szStatus, pItem->text());
				else if (strcmp(pItem->tag_name(), "Longitude") == 0)
					strcpy(pDev->szLongitude, pItem->text());
				else if (strcmp(pItem->tag_name(), "Latitude") == 0)
					strcpy(pDev->szLatitude, pItem->text());
				else if (strcmp(pItem->tag_name(), "Info") == 0)
				{
					acl::xml_node *pInfoItem = pItem->first_child();
					while (pInfoItem)
					{
						if (strcmp(pItem->tag_name(), "PTZType") == 0)
							pDev->nPTZType = atoi(pItem->text());
						else if (strcmp(pItem->tag_name(), "PositionType") == 0)
							pDev->nPositionType = atoi(pItem->text());
						else if (strcmp(pItem->tag_name(), "RoomType") == 0)
							pDev->nRoomType = atoi(pItem->text());
						else if (strcmp(pItem->tag_name(), "SupplyLightType") == 0)
							pDev->nSupplyLightType = atoi(pItem->text());
						else if (strcmp(pItem->tag_name(), "DirectionType") == 0)
							pDev->nDirectionType = atoi(pItem->text());
						else if (strcmp(pItem->tag_name(), "Resolution") == 0)
							strcpy(pDev->szResolution, pItem->text());
						else if (strcmp(pItem->tag_name(), "BusinessGroupID") == 0)
							strcpy(pDev->szBusinessGroupID, pItem->text());
						else if (strcmp(pItem->tag_name(), "DownloadSpeed") == 0)
							pDev->nPort = atoi(pItem->text());
						else if (strcmp(pItem->tag_name(), "SVCSpaceSupportMode") == 0)
							pDev->nSVCpaceSupportMode = atoi(pItem->text());
						else if (strcmp(pItem->tag_name(), "SVCTimeSupportMode") == 0)
							pDev->nSVCTimeSupportMode = atoi(pItem->text());
						pInfoItem = pItem->next_child();
					}
				}
				pItem = pDevItem->next_child();
			}
			std::vector<acl::string> &vecAddr = strAddr.split2(":");
			strcpy(pDev->szIPAddress, vecAddr[0].c_str());
			pDev->nPort = atoi(vecAddr[1].c_str());
			if (strlen(szUser) == 20)
				strcpy(pDev->szServiceID, szUser);
			else
				strcpy(pDev->szServiceID, strDeviceID.c_str());
			__catlocker.lock();
			__catalog.push(pDev);
			__catlocker.unlock();
			//添加通道信息到通道表
			if (sip_get_user_role(pDev->szDeviceID) == SIP_ROLE_CHANNEL)
			{
				chnmap_iter citer = __chnmap.find(strDeviceID.c_str());
				if (citer == __chnmap.end())
				{
					ChannelInfo cinfo;
					memset(&cinfo, 0, sizeof(ChannelInfo));
					strcpy(cinfo.szDeviceID, pDev->szDeviceID);
					strcpy(cinfo.szName, pDev->szName);
					if (strstr(pDev->szStatus, "ON"))
						cinfo.nStatus = 1;
					else
						cinfo.nStatus = 0;
					strcpy(cinfo.szSIPCode, pDev->szServiceID);
					strcpy(cinfo.szIPAddress, pDev->szIPAddress);
					cinfo.nPort = pDev->nPort;
					cinfo.nPTZType = pDev->nPTZType;
					if (pDev->szResolution[0] != '\0')
						strcpy(cinfo.szResolution, pDev->szResolution);
					__chnmap[pDev->szDeviceID] = cinfo;
				}
				else
				{
					strcpy(citer->second.szIPAddress, pDev->szIPAddress);
					citer->second.nPort = pDev->nPort;
					if (strstr(pDev->szStatus, "ON"))
						citer->second.nStatus = 1;
					else
						citer->second.nStatus = 0;
				}
			}
			pDevItem = pDevList->next_child();
		}
	}
	else
	{
		response_msg(siter->second, szData.c_str(), szData.size());
		int nNum = atoi(pDevList->attr_value("Num"));
		siter->second->session_time = time(NULL);
		siter->second->rec_count += nNum;
		if (siter->second->rec_count == nSumNum)
		{
			printf("向[%s]发送目录[%d]完毕!\n", siter->second->user_from, nSumNum);
		}
	}
	__msglocker.unlock();
	xml.reset();
}
//设备信息回应
void SipService::on_msg_deviceinfo_response(socket_stream * stream, const char * data, size_t dlen)
{
	acl::string strAddr = stream->get_peer(true);
	acl::string szData(data, dlen);
	char szAns[SIP_MSG_LEN] = { 0 };
	int nLen = sip_build_msg_answer(szAns, sizeof(szAns), szData.c_str(), SIP_STATUS_200);
	stream->write(szAns, nLen);

	char szBody[SIP_BODY_LEN] = { 0 };
	SIP_CONTENT_TYPE nBodyType = sip_get_body(szData.c_str(), szBody, sizeof(szBody));
	if (nBodyType != SIP_APPLICATION_XML)
		return;
	acl::xml1 xml(szBody);
	acl::string strSn = acl_xml_getElementValue(xml, "SN");
	sesmap_iter siter = __msgmap.find(strSn.c_str());
	if (siter == __msgmap.end())
		return;
	__msglocker.lock();
	siter->second->session_time = time(NULL);
	if (m_conf.srv_role == SIP_ROLE_CLIENT)
	{
		acl::string strResult = acl_xml_getElementValue(xml, "Result");
		acl::string strDeviceName = acl_xml_getElementValue(xml, "DeviceName");
		acl::string strManufacturer = acl_xml_getElementValue(xml, "Manufacturer");
		acl::string strModel = acl_xml_getElementValue(xml, "Model");
		acl::string strFirmware = acl_xml_getElementValue(xml, "Firmware");
		acl::string strChannel = acl_xml_getElementValue(xml, "Channel");
	}
	else
		response_msg(siter->second, szData.c_str(), szData.size());
	xml.reset();
	delete siter->second;
	__msgmap.erase(siter);
	__msglocker.unlock();
}
//录像查询回应
void SipService::on_msg_recordinfo_response(socket_stream * stream, const char * data, size_t dlen)
{
	acl::string strAddr = stream->get_peer(true);
	acl::string szData(data, dlen);
	char szBuffer[SIP_HEADER_LEN] = { 0 };
	char szBody[SIP_BODY_LEN] = { 0 };
	SIP_CONTENT_TYPE nBodyType = sip_get_body(szData.c_str(), szBody, sizeof(szBody));
	if (nBodyType != SIP_APPLICATION_XML)
		return;
	acl::xml1 xml(szBody);
	int nSumNum = acl_xml_getElementInt(xml, "SumNum");
	acl::string strSn = acl_xml_getElementValue(xml, "SN");
	acl::xml_node *pRecList = xml.getFirstElementByTag("RecordList");
	if (!pRecList)
		return;
	sesmap_iter siter = __msgmap.find(strSn.c_str());
	if (siter == __msgmap.end())
		return;
	__msglocker.lock();
	siter->second->session_time = time(NULL);
	if (m_conf.srv_role == SIP_ROLE_CLIENT)
	{
		record_info *pInfo = siter->second->rec_info;
		acl::xml_node *pRecItem = pRecList->first_child();
		while (pRecItem)
		{
			acl::xml_node *pItem = pRecItem->first_child();
			while (pItem)
			{
				if (strcmp(pItem->tag_name(), "FilePath") == 0)
					strcpy(pInfo->sFileName, pItem->text());
				else if (strcmp(pItem->tag_name(), "StartTime") == 0)
					strcpy(pInfo->startTime, sip_get_generaltime(szBuffer, sizeof(szBuffer), (char *)pItem->text()));
				else if (strcmp(pItem->tag_name(), "EndTime") == 0)
					strcpy(pInfo->endTime, sip_get_generaltime(szBuffer, sizeof(szBuffer), (char *)pItem->text()));
				else if (strcmp(pItem->tag_name(), "Type") == 0)
					pInfo->nType = (int)sip_get_record_type(pItem->text());
				else if (strcmp(pItem->tag_name(), "FileSize") == 0)
					pInfo->nSize = atoi(pItem->text());
				pItem = pRecItem->next_child();
			}
			pInfo++;
			siter->second->rec_count++;
			pRecItem = pRecList->next_child();
		}
		if (siter->second->rec_count == nSumNum)
		{
			printf("接收录像文件索引:%d\n", nSumNum);
			siter->second->rec_finish = true;
		}
		char szAns[SIP_MSG_LEN] = { 0 };
		int nLen = sip_build_msg_answer(szAns, sizeof(szAns), szData.c_str(), SIP_STATUS_200);
		stream->write(szAns, nLen);
	}
	else
	{
		int nNum = atoi(pRecList->attr_value("Num"));
		char szAns[SIP_MSG_LEN] = { 0 };
		int nLen = sip_build_msg_answer(szAns, sizeof(szAns), szData.c_str(), SIP_STATUS_200);
		stream->write(szAns, nLen);

		response_msg(siter->second, szData.c_str(), szData.size());
		siter->second->rec_count += nNum;
		if (siter->second->rec_count == nSumNum)
		{
			delete siter->second;
			__msgmap.erase(siter);
		}
	}
	__msglocker.unlock();
	xml.reset();
}
//报警通知回应
void SipService::on_msg_alarm_response(socket_stream * stream, const char * data, size_t dlen)
{
	acl::string szData(data, dlen);
	char szAns[SIP_MSG_LEN] = { 0 };
	int nLen = sip_build_msg_answer(szAns, sizeof(szAns), szData.c_str(), SIP_STATUS_200);
	stream->write(szAns, nLen);
}
//配置查询回应
void SipService::on_msg_configdownload_response(socket_stream * stream, const char * data, size_t dlen)
{
	acl::string strAddr = stream->get_peer(true);
	acl::string szData(data, dlen);
	char szAns[SIP_MSG_LEN] = { 0 };
	int nLen = sip_build_msg_answer(szAns, sizeof(szAns), szData.c_str(), SIP_STATUS_200);
	stream->write(szAns, nLen);

	char szBody[SIP_BODY_LEN] = { 0 };
	SIP_CONTENT_TYPE nBodyType = sip_get_body(szData.c_str(), szBody, sizeof(szBody));
	if (nBodyType != SIP_APPLICATION_XML)
		return;
	acl::xml1 xml(szBody);
	acl::string strSn = acl_xml_getElementValue(xml, "SN");
	sesmap_iter siter = __msgmap.find(strSn.c_str());
	if (siter == __msgmap.end())
		return;
	__msglocker.lock();
	siter->second->session_time = time(NULL);
	if (m_conf.srv_role == SIP_ROLE_CLIENT)
	{
		acl::string strResult = acl_xml_getElementValue(xml, "Result");
		acl::string strBasicParam = acl_xml_getElementValue(xml, "BasicParam");
		if (strBasicParam != "")
		{
			acl::string strName = acl_xml_getElementString(xml, "Name");
			acl::string strExpiration = acl_xml_getElementValue(xml, "Expiration");
			acl::string strHeartBeatInterval = acl_xml_getElementValue(xml, "HeartBeatInterval");
			acl::string strHeartBeatCount = acl_xml_getElementValue(xml, "HeartBeatCount");
			acl::string strPositionCapability = acl_xml_getElementValue(xml, "PositionCapability");
			acl::string strLongitude = acl_xml_getElementValue(xml, "Longitude");
			acl::string strLatitude = acl_xml_getElementValue(xml, "Latitude");
		}
		acl::string strVideoParamOpt = acl_xml_getElementValue(xml, "VideoParamOpt");
		if (strVideoParamOpt != "")
		{
			acl::string strDownloadSpeed = acl_xml_getElementValue(xml, "DownloadSpeed");
			acl::string strResolution = acl_xml_getElementValue(xml, "Resolution");
		}
		acl::xml_node *pSVACEncodeConfig = xml.getFirstElementByTag("SVACEncodeConfig");
		if (pSVACEncodeConfig)
		{
			acl::xml_node *pROIParam = xml.getFirstElementByTag("ROIParam");
			if (pROIParam)
			{
				acl::string strExpiration = acl_xml_getElementValue(xml, "ROIFlag");
				acl::string strHeartBeatCount = acl_xml_getElementValue(xml, "BackGroundQP");
				acl::string strPositionCapability = acl_xml_getElementValue(xml, "BackGroundSkipFlag");
				acl::xml_node *pROINumber = xml.getFirstElementByTag("ROINumber");
				if (pROINumber)
				{
					acl::xml_node *pSubItem = pROINumber->first_child();
					while (pSubItem)
					{
						acl::xml_node *pItem = pSubItem->first_child();
						while (pItem)
						{
							if (strcmp(pItem->tag_name(), "ROISeq") == 0) {}
							else if (strcmp(pItem->tag_name(), "TopLeft") == 0) {}
							else if (strcmp(pItem->tag_name(), "BottomRight") == 0) {}
							else if (strcmp(pItem->tag_name(), "ROIQP") == 0) {}
							pItem = pSubItem->next_child();
						}
						pSubItem = pROINumber->next_child();
					}
				}
			}
			acl::xml_node *pSVCParam = xml.getFirstElementByTag("SVCParam");
			if (pSVCParam)
			{
				acl::string strSVCSpaceDomainMode = acl_xml_getElementValue(xml, "SVCSpaceDomainMode");
				acl::string strSVCTimeDomainMode = acl_xml_getElementValue(xml, "SVCTimeDomainMode");
				acl::string strSVCSpaceSupportMode = acl_xml_getElementValue(xml, "SVCSpaceSupportMode");
				acl::string strSVCTimeSupportMode = acl_xml_getElementValue(xml, "SVCTimeSupportMode");
			}
			acl::xml_node *pSurveillanceParam = xml.getFirstElementByTag("SurveillanceParam");
			if (pSurveillanceParam)
			{
				acl::string strTimeFlag = acl_xml_getElementValue(xml, "TimeFlag");
				acl::string strEventFlag = acl_xml_getElementValue(xml, "EventFlag");
				acl::string strAlerFlag = acl_xml_getElementValue(xml, "AlerFlag");
			}
			acl::xml_node *pAudioParam = xml.getFirstElementByTag("AudioParam");
			if (pAudioParam)
			{
				acl::string strAudioRecognitionFlag = acl_xml_getElementValue(xml, "AudioRecognitionFlag");
			}
		}
		acl::xml_node *pSVACDecodeConfig = xml.getFirstElementByTag("SVACDecodeConfig");
		if (pSVACDecodeConfig)
		{
			acl::xml_node *pSVCParam = xml.getFirstElementByTag("SVCParam");
			if (pSVCParam)
			{
				acl::string strSVCSpaceSupportMode = acl_xml_getElementValue(xml, "SVCSpaceSupportMode");
				acl::string strSVCTimeSupportMode = acl_xml_getElementValue(xml, "SVCTimeSupportMode");
			}
			acl::xml_node *pSurveillanceParam = xml.getFirstElementByTag("SurveillanceParam");
			if (pSurveillanceParam)
			{
				acl::string strTimeShowFlag = acl_xml_getElementValue(xml, "TimeShowFlag");
				acl::string strEventShowFlag = acl_xml_getElementValue(xml, "EventShowFlag");
				acl::string strAlerShowtFlag = acl_xml_getElementValue(xml, "AlerShowtFlag");
			}
		}
	}
	else
		response_msg(siter->second, szData.c_str(), szData.size());
	xml.reset();
	delete siter->second;
	__msgmap.erase(siter);
	__msglocker.unlock();
}
//语音广播回应
void SipService::on_msg_broadcast_response(socket_stream * stream, const char * data, size_t dlen)
{
	acl::string strAddr = stream->get_peer(true);
	acl::string szData(data, dlen);
	char szAns[SIP_MSG_LEN] = { 0 };
	int nLen = sip_build_msg_answer(szAns, sizeof(szAns), szData.c_str(), SIP_STATUS_200);
	stream->write(szAns, nLen);

	char szBody[SIP_BODY_LEN] = { 0 };
	SIP_CONTENT_TYPE nBodyType = sip_get_body(szData.c_str(), szBody, sizeof(szBody));
	if (nBodyType != SIP_APPLICATION_XML)
		return;
	acl::xml1 xml(szBody);
	acl::string strSn = acl_xml_getElementValue(xml, "SN");
	sesmap_iter siter = __msgmap.find(strSn.c_str());
	if (siter == __msgmap.end())
		return;
	__msglocker.lock();
	siter->second->session_time = time(NULL);
	if (m_conf.srv_role == SIP_ROLE_CLIENT)
	{
		acl::string strDeviceID = acl_xml_getElementValue(xml, "DeviceID");
		acl::string strResult = acl_xml_getElementValue(xml, "Result");
	}
	else
		response_msg(siter->second, szData.c_str(), szData.size());
	xml.reset();
	delete siter->second;
	__msgmap.erase(siter);
	__msglocker.unlock();
}
//能力查询
void SipService::on_options_request(socket_stream * stream, const char * data, size_t dlen)
{
	acl::string szData(data, dlen);
	char szAns[SIP_MSG_LEN] = { 0 };
	int nLen = sip_build_msg_answer(szAns, sizeof(szAns), szData.c_str(), SIP_STATUS_200);
	nLen = sip_set_header(szAns, sizeof(szAns), SIP_H_ALLOW, "INVITE,ACK,CANCEL,INFO,BYE,OPTIONS,MESSAGE,SUBSCRIBE,NOTIFY");
	stream->write(szAns, nLen);
}
//注册请求
void SipService::on_register_request(socket_stream * stream, const char * data, size_t dlen)
{
	acl::string szData(data, dlen);
	acl::string strAddr = stream->get_peer(true);
	char szBuffer[SIP_AUTH_LEN] = { 0 };
	char szAns[SIP_MSG_LEN] = { 0 };
	int nAnsLen = 0;
	acl::string strContact = sip_get_header(szBuffer, sizeof(szBuffer), szData.c_str(), SIP_H_CONTACT);
	acl::string strExpires = sip_get_header(szBuffer, sizeof(szBuffer), szData.c_str(), SIP_H_EXPIRES);
	int nExpires = atoi(strExpires.c_str());
	acl::string sAuthorization = sip_get_header(szBuffer, sizeof(szBuffer), szData.c_str(), SIP_H_AUTHORIZATION);
	if (sAuthorization == "")	//未认证的注册/注销
	{
		acl::string strRealm(m_conf.local_code);
		acl::string strAuth = "";
		strAuth.format("Digest realm=\"%s\",nonce=\"%s\"", strRealm.left(10).c_str(), sip_build_nonce(szBuffer, sizeof(szBuffer)));
		nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), szData.c_str(), SIP_STATUS_401);
		nAnsLen = sip_set_header(szAns, sizeof(szAns), SIP_H_WWW_AUTHENTICATE, strAuth.c_str());
		stream->write(szAns, nAnsLen);	//回复401
		//m_conf.log(LOG_WARN, "收到下级%s%s请求", strContact.c_str(), nExpires > 0 ? "注册" : "注销");
	}
	else                        //带认证再次注册/注销
	{
		acl::string strUsername = sip_get_authorization_username(szBuffer, sizeof(szBuffer), sAuthorization.c_str());
		acl::string strRealm = sip_get_authorization_realm(szBuffer, sizeof(szBuffer), sAuthorization.c_str());
		acl::string strNonce = sip_get_authorization_nonce(szBuffer, sizeof(szBuffer), sAuthorization.c_str());
		acl::string strUri = sip_get_authorization_uri(szBuffer, sizeof(szBuffer), sAuthorization.c_str());
		char szAuth[SIP_AUTH_LEN] = { 0 };
		acl::string strResponse = sip_get_authorization_response(szAuth, sizeof(szAuth), sAuthorization.c_str());
		if (strUsername == "" || strRealm == "" || strNonce == "" || strUri == "" || strResponse == "")
		{
			nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), szData.c_str(), SIP_STATUS_400);
			stream->write(szAns, nAnsLen);
			m_conf.log(LOG_ERROR, "下级%s%s失败,无效的认证参数", strContact.c_str(), nExpires > 0 ? "注册" : "注销");
			return;
		}
		register_info *rinfo = NULL;
		acl::string strPwd(m_conf.default_pwd);
		__reglocker.lock();
		regmap_iter riter = __regmap.find(strAddr.c_str());
		if (riter != __regmap.end())
		{
			strPwd = riter->second.pwd;
			rinfo = &riter->second;
		}
		__reglocker.unlock();
		acl::string strRes = sip_build_authorization_response(szAuth, sizeof(szAuth), strUsername.c_str(), strRealm.c_str(), strPwd.c_str(), strNonce.c_str(), strUri.c_str());
		if (strcmp(strResponse.c_str(), strRes.c_str()) != 0)
		{
			nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), szData.c_str(), SIP_STATUS_400);
			stream->write(szAns, nAnsLen);
			m_conf.log(LOG_ERROR, "下级%s%s失败,无效的认证密钥", strContact.c_str(), nExpires > 0 ? "注册" : "注销");
			return;
		}
		
		nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), szData.c_str(), SIP_STATUS_200);
		nAnsLen = sip_set_header(szAns, sizeof(szAns), SIP_H_CONTACT, strContact.c_str());
		nAnsLen = sip_set_header(szAns, sizeof(szAns), SIP_H_DATE, sip_get_localtime(szBuffer, sizeof(szBuffer)));
		nAnsLen = sip_set_header(szAns, sizeof(szAns), SIP_H_EXPIRES, strExpires.c_str());
		stream->write(szAns, nAnsLen);

		acl::string strUser = sip_get_from_user(szBuffer, sizeof(szBuffer), szData.c_str());
		
		if (rinfo)	//己注册过下级
		{
			if (nExpires == 0)
			{
				rinfo->online = false;
				rinfo->alivetime = time(NULL);
				size_t nCount = rinfo->slist->size();
				if (nCount > 0)
					register_slist_clear(rinfo->addr);
#ifdef _DBLIB_USE
				m_db.UpdateRegister(strAddr.c_str(), false);
#endif
				m_conf.log(LOG_WARN, "下级%s注销成功", strContact.c_str());
			}
			else
			{
				if (rinfo->online == false)
				{
					size_t nCount = rinfo->slist->size();
					if (nCount > 0)
						register_slist_clear(rinfo->addr);
				}
				rinfo->online = true;
				rinfo->alivetime = time(NULL);
#ifdef _DBLIB_USE
				m_db.UpdateRegister(strAddr.c_str(), true);
#endif
				m_conf.log(LOG_INFO, "下级%s重注册成功", strContact.c_str());
			}
			return;
		}			//未注册过的下级
		char szMappingAddr[24] = { 0 };
		register_update(strUser.c_str(),szMappingAddr);
		__reglocker.lock();
		register_info info;
		memset(&info, 0, sizeof(register_info));
		sip_get_header(info.name, sizeof(info.name), szData.c_str(), SIP_H_USER_AGENT);
		strcpy(info.addr, strAddr.c_str());
		strcpy(info.user, strUser.c_str());
		strcpy(info.pwd, strPwd.c_str());
		info.role = sip_get_user_role(strUser.c_str());
		info.tranfer = m_conf.clnt_tranfer;
		info.expires = nExpires;
		if (szMappingAddr[0] != '\0')
		{
			strcpy(info.mapping_addr, szMappingAddr);
			printf("下级[%s]地址变更为[%s]!\n", strUser.c_str(), strAddr.c_str());
		}
		info.online = true;
		info.vaild = true;
		info.alivetime = time(NULL);
		info.slist = new session_list;
		__regmap[strAddr.c_str()] = info;
		m_conf.log(LOG_WARN, "下级%s%s成功", strContact.c_str(), nExpires > 0 ? "注册" : "注销");
		if (nExpires > 0 && info.role != SIP_ROLE_CLIENT && info.role != SIP_ROLE_DIRECT && info.role != SIP_ROLE_MEDIA && info.role != SIP_ROLE_INVALID)
		{
#ifdef _DBLIB_USE
			m_db.SaveRegister(m_conf.local_code, info);
#endif
			send_message_catalog(strUser.c_str(), strAddr.c_str());	//发送目录查询
		}
		__reglocker.unlock();
	}
}
//点播请求
void SipService::on_invite_request(socket_stream * stream, const char * data, size_t dlen)
{
	acl::string szData(data, dlen);
	char szBuffer[SIP_HEADER_LEN] = { 0 };
	char szAns[SIP_MSG_LEN] = { 0 };
	int nAnsLen = 0;
	acl::string strAddr = stream->get_peer(true);
	acl::string strCallID = sip_get_header(szBuffer, sizeof(szBuffer), szData.c_str(), SIP_H_CALL_ID);
	acl::string strUserID = sip_get_from_user(szBuffer, sizeof(szBuffer), szData.c_str());
	acl::string strFtag = sip_get_from_tag(szBuffer, sizeof(szBuffer), szData.c_str());
	acl::string strDeviceID = sip_get_subject_deviceid(szBuffer, sizeof(szBuffer), szData.c_str());
	acl::string strReceiverID = sip_get_subject_receiverid(szBuffer, sizeof(szBuffer), szData.c_str());
	char *terminalid = NULL;
	if (sip_get_user_role(strReceiverID.c_str()) == SIP_ROLE_MONITOR)	//是否矩阵通道
		terminalid = strReceiverID.c_str();
	char szBody[SIP_BODY_LEN] = { 0 };
	sip_get_body(szData.c_str(), szBody, sizeof(szBody));
	if (szBody[0] == '\0')
	{
		nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), szData.c_str(), SIP_STATUS_400);
		stream->write(szAns, nAnsLen);
		return;
	}
	sip_sdp_info sdp;
	memset(&sdp, 0, sizeof(sip_sdp_info));
	sip_get_sdp_info(szBody, &sdp);
	int nPlayType = sdp.s_type;
	recv_info rinfo;
	strcpy(rinfo.receiverid, sdp.o_user);
	strcpy(rinfo.recvip, sdp.o_ip);
	rinfo.recvport = sdp.m_port;
	rinfo.recvprotocol = sdp.m_protocol;
	if (strDeviceID == "")
	{
		nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), szData.c_str(), SIP_STATUS_400);
		stream->write(szAns, nAnsLen);
		return;
	}
	sesmap_iter siter = __sesmap.find(strCallID.c_str());
	if (siter != __sesmap.end())
	{
		return;	//重复的会话
	}
	session_info *pSession = register_slist_find(strAddr.c_str(), strDeviceID.c_str());
	if (pSession)	//同一用户重复请求
	{
		if (pSession->invite_type == nPlayType && nPlayType == SIP_INVITE_PLAY)
		{
			time_t nowtime = time(NULL);
			if (pSession->session_time == 0 || nowtime - pSession->session_time < 6)
				return; //频繁请求同一路
			send_bye_by_cid(pSession->callid);
		}
	}
	nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), szData.c_str(), SIP_STATUS_100);
	stream->write(szAns, nAnsLen);

	//查询镜头详细信息
	acl::string strChnAddr;
	chnmap_iter citer = __chnmap.find(strDeviceID.c_str());
	if (citer == __chnmap.end())
	{
		nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), szData.c_str(), SIP_STATUS_400);
		stream->write(szAns, nAnsLen);
		return;
	}
	ChannelInfo *pChn = &(citer->second);
	strChnAddr.format("%s:%d", pChn->szIPAddress, pChn->nPort);
	acl::string strDecAddr;
	if (terminalid && terminalid[0] != '\0')
	{
		citer = __chnmap.find(terminalid);
		if (citer == __chnmap.end())
		{
			nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), szData.c_str(), SIP_STATUS_400);
			stream->write(szAns, nAnsLen);
			return;
		}
		strDecAddr.format("%s:%d", citer->second.szIPAddress, citer->second.nPort);
	}
	invite_info info;
	memset(&info, 0, sizeof(info));
	if (strcmp(m_conf.local_code, pChn->szSIPCode) == 0) //从SDK取流
	{
		nPlayType += SIP_INVITE_PLAY_SDK;
		sprintf(info.invite_uri, "%s:%d:%s:%d:%s:%s:%s", strDeviceID.c_str(), pChn->nChannel,pChn->szIPAddress,pChn->nPort,pChn->szUser,pChn->szPwd,pChn->szModel);
		if (is_superior_addr(strAddr.c_str()))	
			info.transfrom = 1;					//上级请求，需要转换
	}
	strcpy(info.deviceid, strDeviceID.c_str());
	strcpy(info.addr, strChnAddr.c_str());
	strcpy(info.user_addr, strAddr.c_str());
	sip_get_header(info.user_via, sizeof(info.user_via), szData.c_str(), SIP_H_VIA);
	sip_get_header(info.user_from, sizeof(info.user_from), szData.c_str(), SIP_H_FROM);
	sip_get_header(info.user_to, sizeof(info.user_to), szData.c_str(), SIP_H_TO);
	info.cseq = sip_get_cseq_num(szData.c_str());
	if (terminalid)
	{
		strcpy(info.terminalid, terminalid);
		strcpy(info.terminal_addr, strDecAddr.c_str());
	}
	strcpy(info.callid, strCallID.c_str());
	strcpy(info.ftag, strFtag.c_str());
	memcpy(&info.receiver, &rinfo, sizeof(recv_info));
	sprintf(info.invite_time, "%lld %lld", sdp.t_begintime, sdp.t_endtime);

	sip_build_sdp_ssrc(info.ssrc, sizeof(info.ssrc), SIP_INVITE_TYPE(nPlayType), SIP_PROTOCOL_TYPE(rinfo.recvprotocol));
	//printf("invite ssrc:%s\n", info.ssrc);
	info.invite_type = sdp.s_type;

	register_info *pReg = NULL;
	if (pChn->szMediaAddress[0] != '\0')						//是否指定流媒体
		pReg = register_get_info(pChn->szMediaAddress);		
	else
	{
		if (sip_get_user_role(strUserID.c_str()) != SIP_ROLE_DIRECT)
			pReg = register_get_mediaservice(strDeviceID.c_str());
	}
	if (pReg)	//>>走RTP流媒体
	{
		strcpy(info.media_addr, pReg->addr);
		if (info.invite_uri != '\0')
			info.carrysdp = true;
		else
			info.carrysdp = false;
		send_invite(&info);
	}
	else		//>>直连下级
	{
		if (info.invite_uri[0] != '\0')	//流媒体还没注册成功
		{
			nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), szData.c_str(), SIP_STATUS_486);
			stream->write(szAns, nAnsLen);
			return;
		}
		info.carrysdp = true;
		send_invite(&info);
	}
}
//点播取消
void SipService::on_cancel_request(socket_stream * stream, const char * data, size_t dlen)
{
	acl::string szData(data, dlen);
	char szAns[SIP_MSG_LEN] = { 0 };
	int nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), szData.c_str(), SIP_STATUS_200);
	stream->write(szAns, nAnsLen);

	acl::string strAddr = stream->get_peer(true);
	char szCallID[SIP_HEADER_LEN] = { 0 };
	sip_get_header(szCallID, sizeof(szCallID), szData.c_str(), SIP_H_CALL_ID);
	send_bye_by_cid(szCallID, false);
}
//点播确认
void SipService::on_ack_request(socket_stream * stream, const char * data, size_t dlen)
{
	acl::string szData(data, dlen);
	acl::string strAddr = stream->get_peer(true);
	char szCallID[SIP_HEADER_LEN] = { 0 };
	sip_get_header(szCallID, sizeof(szCallID), szData.c_str(), SIP_H_CALL_ID);
	int nCSeq = sip_get_cseq_num(szData.c_str());
	sesmap_iter siter = __sesmap.find(szCallID);
	if (siter == __sesmap.end())
	{
		printf("收到[%s]ACK:没有找到匹配的会话\n", strAddr.c_str());
		return;
	}
	session_info *pInfo = siter->second;
	if (pInfo->media_addr[0] != '\0')
	{
		char szTo[SIP_HEADER_LEN] = { 0 };
		sip_build_msg_to(szTo, sizeof(szTo), pInfo->media_user, pInfo->media_addr);
		char szAck[SIP_MSG_LEN] = { 0 };
		int nAckLen = sip_build_msg_ack(szAck, sizeof(szAck), szTo, nCSeq, pInfo->callid, pInfo->ftag, pInfo->ttag);
		if (pInfo->flow == SESSION_SMEDIA_REP)
		{
			char szClientBody[SIP_BODY_LEN] = { 0 };
			sip_build_msg_sdp(szClientBody, sizeof(szClientBody), pInfo->receiver.receiverid, pInfo->deviceid, pInfo->invite_time, pInfo->receiver.recvip, pInfo->receiver.recvport,
				pInfo->invite_type, SIP_RECVONLY, pInfo->receiver.recvprotocol);
			sip_set_sdp_y(szClientBody, sizeof(szClientBody), pInfo->ssrc);
			nAckLen = sip_set_body(szAck, sizeof(szAck), szClientBody, SIP_APPLICATION_SDP);
		}
		send_msg(pInfo->media_addr, szAck, nAckLen);
		pInfo->flow = SESSION_SMEDIA_ACK;
	}
	else
	{
		char szUserID[21] = { 0 };
		regmap_iter riter = __regmap.find(pInfo->addr);
		if (riter != __regmap.end())
			strcpy(szUserID, riter->second.user);
		else
			strcpy(szUserID, pInfo->deviceid);
		char szTo[SIP_HEADER_LEN] = { 0 };
		sip_build_msg_to(szTo, sizeof(szTo), szUserID, pInfo->addr);
		char szAck[SIP_MSG_LEN] = { 0 };
		int nAckLen = sip_build_msg_ack(szAck, sizeof(szAck), szTo, nCSeq, pInfo->callid, pInfo->ftag, pInfo->ttag);
		send_msg(pInfo->addr, szAck, nAckLen);
		pInfo->flow = SESSION_SENDER_ACK;
	}

}
//点播关闭
void SipService::on_bye_request(socket_stream * stream, const char * data, size_t dlen)
{
	acl::string szData(data, dlen);
	acl::string strAddr = stream->get_peer(true);
	char szAns[SIP_MSG_LEN] = { 0 };
	int nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), szData.c_str(), SIP_STATUS_200);
	stream->write(szAns, nAnsLen);
	char szCallID[SIP_HEADER_LEN] = { 0 };
	sip_get_header(szCallID, sizeof(szCallID), szData.c_str(), SIP_H_CALL_ID);
	char szUser[SIP_HEADER_LEN] = { 0 };
	sip_get_from_user(szUser, sizeof(szUser), szData.c_str());
	int nRole = sip_get_user_role(szUser);
	m_conf.log(LOG_INFO, "关闭镜头[%s][%s]\n", szUser, szCallID);
	if (nRole == SIP_ROLE_CLIENT || nRole == SIP_ROLE_SENDER)
	{
		if (m_conf.srv_role == SIP_ROLE_CLIENT)
		{
			sesmap_iter siter = __sesmap.find(szCallID);
			if (siter == __sesmap.end())
				return;
			session_info *pInfo = siter->second;
#ifdef _USRDLL
			media_stream_cb pCb = (media_stream_cb)pInfo->stream_cb;
			if (pCb && pInfo->stream_user)
				pCb(408, NULL, 0, pInfo->stream_user);
#endif 
			pInfo->mediaover = time(NULL);
		}
		else
			send_bye_by_cid(szCallID, false);
	}
	else if (nRole == SIP_ROLE_MEDIA)
	{
		sesmap_iter siter = __sesmap.find(szCallID);
		if (siter == __sesmap.end())
			return;
		session_info *pInfo = siter->second;

		char szMsg[SIP_MSG_LEN] = { 0 };
		int nMsgLen = 0;
		char szTo[SIP_HEADER_LEN] = { 0 };
		if (pInfo->terminal_addr[0] != '\0' && m_conf.srv_role != SIP_ROLE_CLIENT)
		{
			sip_build_msg_to(szTo, sizeof(szTo), pInfo->terminalid, pInfo->terminal_addr);
			nMsgLen = sip_build_msg_bye(szMsg, sizeof(szMsg), szTo, pInfo->cseq + 1, szCallID, pInfo->ftag, pInfo->terminal_ttag);
			send_msg(pInfo->terminal_addr, szMsg, nMsgLen);
		}
		if (pInfo->user_addr[0] != '\0')
		{
			nMsgLen = sip_build_msg_bye(szMsg, sizeof(szMsg), pInfo->user_to, pInfo->cseq + 1, szCallID, pInfo->ftag, pInfo->ttag, pInfo->user_from);
			send_msg(pInfo->user_addr, szMsg, nMsgLen);
		}
		sip_build_msg_to(szTo, sizeof(szTo), pInfo->deviceid, pInfo->addr);
		nMsgLen = sip_build_msg_bye(szMsg, sizeof(szMsg), szTo, pInfo->cseq + 1, szCallID, pInfo->ftag, pInfo->ttag);
		send_msg(pInfo->addr, szMsg, nMsgLen);
		pInfo->flow = SESSION_SENDER_BYE;
		pInfo->stream_cb = NULL;
		pInfo->stream_user = NULL;
		if (pInfo->rtp_stream)
			sip_set_stream_cb((RTP_STREAM)pInfo->rtp_stream, NULL, NULL);
		pInfo->mediaover = time(NULL);
	}
	else
	{
		sesmap_iter siter = __sesmap.find(szCallID);
		if (siter == __sesmap.end())
			return;
		session_info *pInfo = siter->second;
		pInfo->flow = SESSION_SENDER_BYE;
		pInfo->stream_cb = NULL;
		pInfo->stream_user = NULL;
		if (pInfo->rtp_stream)
			sip_set_stream_cb((RTP_STREAM)pInfo->rtp_stream, NULL, NULL);
		pInfo->mediaover = time(NULL);
	}
}
//回放控制
void SipService::on_info_request(socket_stream * stream, const char * data, size_t dlen)
{
	acl::string strMsg(data, dlen);
	char szAns[SIP_MSG_LEN] = { 0 };
	int nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), strMsg.c_str(), SIP_STATUS_200);
	stream->write(szAns, nAnsLen);

	acl::string strDeviceID;
	char szDeviceID[64] = { 0 };
	sip_get_to_user(szDeviceID, sizeof(szDeviceID), strMsg.c_str());
	if (sip_get_user_role(szDeviceID) == SIP_ROLE_CHANNEL)
		strDeviceID = szDeviceID;
	else
	{
		sip_get_header(szDeviceID, sizeof(szDeviceID), strMsg.c_str(), SIP_H_CALL_ID);
		sesmap_iter siter = __sesmap.find(szDeviceID);
		if (siter != __sesmap.end())
			strDeviceID = siter->second->deviceid;
		else
			return;
	}
	transfer_msg(strDeviceID.c_str(), strMsg.c_str(), strMsg.size());
}
//订阅请求
void SipService::on_subscribe_request(socket_stream * stream, const char * data, size_t dlen)
{
	acl::string strAddr = stream->get_peer(true);
	acl::string szData(data, dlen);
	char szBuffer[SIP_HEADER_LEN] = { 0 };
	char szAns[SIP_MSG_LEN] = { 0 };
	int nAnsLen = 0;
	acl::string strUser = sip_get_from_user(szBuffer, sizeof(szBuffer), szData.c_str());
	acl::string strCallID = sip_get_header(szBuffer, sizeof(szBuffer), szData.c_str(), SIP_H_CALL_ID);
	int nCSeq = sip_get_cseq_num(szData.c_str());
	acl::string strExpires = sip_get_header(szBuffer, sizeof(szBuffer), szData.c_str(), SIP_H_EXPIRES);
	int nExpires = atoi(strExpires.c_str());
	acl::string strEvent = sip_get_header(szBuffer, sizeof(szBuffer), szData.c_str(), SIP_H_EVENT);
	int nEventtype = 0;
	if (strEvent.compare("presence", false) == 0)
		nEventtype = 1;
	char szBody[SIP_BODY_LEN] = { 0 };
	SIP_CONTENT_TYPE nBodyType = sip_get_body(szData.c_str(), szBody, sizeof(szBody));
	if (nBodyType != SIP_APPLICATION_XML)
	{
		nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), szData.c_str(), SIP_STATUS_400);
		stream->write(szAns, nAnsLen);
		return;
	}
	acl::xml1 xml(szBody);
	acl::string strCmdType = acl_xml_getElementValue(xml, "CmdType");
	acl::string strSN = acl_xml_getElementValue(xml, "SN");
	acl::string strDevice = acl_xml_getElementValue(xml, "DeviceID");
	acl::string strStartTime = acl_xml_getElementValue(xml, "StartTime");
	acl::string strEndTime = acl_xml_getElementValue(xml, "EndTime");

	nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), szData.c_str(), SIP_STATUS_200);
	nAnsLen = sip_set_header(szAns, sizeof(szAns), SIP_H_EXPIRES, strExpires.c_str());
	nAnsLen = sip_set_header(szAns, sizeof(szAns), SIP_H_EVENT, strEvent.c_str());
	acl::string strAnsBody;
	strAnsBody.format(
		"<?xml version=\"1.0\"?>%s"
		"<Response>%s"
		"<CmdType>%s</CmdType>%s"
		"<SN>%s</SN>%s"
		"<DeviceID>%s</DeviceID>%s"
		"<Result>OK</Result>%s"
		"</Response>%s", 
		Line, Line, strCmdType.c_str(), Line, strSN.c_str(), Line, strDevice.c_str(), Line, Line, Line);
	nAnsLen = sip_set_body(szAns, sizeof(szAns), szBody, nBodyType);
	stream->write(szAns, nAnsLen);
	if (m_conf.srv_role == SIP_ROLE_CLIENT)
		return;
	if (atoi(strExpires.c_str()) == 0)	//取消订阅
	{
		//释放订阅
		__subcatlocker.lock();
		sesmap_iter miter = __subcatmap.find(strAddr.c_str());
		if (miter != __subcatmap.end())
		{
			session_info *pSess =  miter->second;
			__subcatmap.erase(miter);
			__msglocker.lock();
			sesmap_iter siter = __msgmap.find(pSess->sn);
			if (siter != __msgmap.end())
				__msgmap.erase(siter);
			__msglocker.unlock();
			delete pSess;
			pSess = NULL;
		}
		__subcatlocker.unlock();
		__subalmlocker.lock();
		miter = __subalmmap.find(strAddr.c_str());
		if (miter != __subalmmap.end())
		{
			session_info *pSess = miter->second;
			__subcatmap.erase(miter);
			__msglocker.lock();
			sesmap_iter siter = __msgmap.find(pSess->sn);
			if (siter != __msgmap.end())
				__msgmap.erase(siter);
			__msglocker.unlock();
			delete pSess;
			pSess = NULL;
		}
		__subalmlocker.unlock();
		return;
	}
	if (strCmdType.compare("Alarm",false) == 0)			//报警订阅
	{
		acl::string strAlarmMethod = acl_xml_getElementValue(xml, "AlarmMethod");
		int nAlarmType = acl_xml_getElementInt(xml, "AlarmType");
		__subalmlocker.lock();
		sesmap_iter miter = __subalmmap.find(strAddr.c_str());
		if (miter == __subalmmap.end())
		{
			session_info *pSession = new session_info;
			memset(pSession, 0, sizeof(session_info));
			regmap_iter riter = __regmap.find(strAddr.c_str());
			if (riter != __regmap.end())	//查询用户角色
			{
				pSession->user_role = riter->second.role;
			}
			sip_get_from_uri(pSession->user_from, sizeof(pSession->user_from), szData.c_str());
			sip_get_from_tag(pSession->ftag, sizeof(pSession->ftag), szData.c_str());
			sip_get_to_uri(pSession->user_to, sizeof(pSession->user_to), szData.c_str());
			sip_get_to_tag(pSession->ttag, sizeof(pSession->ttag), szData.c_str());
			strcpy(pSession->user_addr, strAddr.c_str());
			strcpy(pSession->deviceid, strDevice.c_str());
			strcpy(pSession->callid, strCallID.c_str());
			strcpy(pSession->sn, strSN.c_str());
			strcpy(pSession->starttime, strStartTime.c_str());
			strcpy(pSession->endtime, strEndTime.c_str());
			strcpy(pSession->alarm_method, strAlarmMethod.c_str());
			pSession->alarm_type = nAlarmType;
			pSession->cseq = nCSeq;
			pSession->expires = nExpires;
			pSession->event_type = nEventtype;
			pSession->session_time = time(NULL);
			__subalmmap[strAddr.c_str()] = pSession;
		}
		else
		{
			strcpy(miter->second->sn, strSN.c_str());
			miter->second->expires = nExpires;
			miter->second->session_time = time(NULL);
		}
		__subalmlocker.unlock();
	}
	else if (strCmdType.compare("Catalog", false) == 0)	//目录订阅
	{
		__subcatlocker.lock();
		sesmap_iter miter = __subcatmap.find(strAddr.c_str());
		if (miter == __subcatmap.end())
		{
			session_info *pSession = new session_info;
			memset(pSession, 0, sizeof(session_info));
			regmap_iter riter = __regmap.find(strAddr.c_str());
			if (riter != __regmap.end())	//查询用户角色
			{
				pSession->user_role = riter->second.role;
			}
			sip_get_from_uri(pSession->user_from, sizeof(pSession->user_from), szData.c_str());
			sip_get_from_tag(pSession->ftag, sizeof(pSession->ftag), szData.c_str());
			sip_get_to_uri(pSession->user_to, sizeof(pSession->user_to), szData.c_str());
			sip_get_to_tag(pSession->ttag, sizeof(pSession->ttag), szData.c_str());
			strcpy(pSession->user_addr, strAddr.c_str());
			strcpy(pSession->deviceid, strDevice.c_str());
			strcpy(pSession->callid, strCallID.c_str());
			strcpy(pSession->sn, strSN.c_str());
			strcpy(pSession->starttime, strStartTime.c_str());
			strcpy(pSession->endtime, strEndTime.c_str());
			pSession->cseq = nCSeq;
			pSession->expires = nExpires;
			pSession->event_type = nEventtype;
			__subcatmap[strAddr.c_str()] = pSession;
			if (nEventtype == 0)
				pSession->session_time = time(NULL);
			else
			{
				if (m_conf.srv_role == SIP_ROLE_SUPERIOR)
				{
					//创建目录推送线程
					catalog_thread *pCatalogThd = new catalog_thread(this, pSession, 1);
					pCatalogThd->set_detachable(true);
					pCatalogThd->start();
				}
				else
				{
					__msgmap[strSN.c_str()] = pSession;
					transfer_msg(pSession->deviceid, szData.c_str(), szData.size());
				}
			}
		}
		else
		{
			strcpy(miter->second->sn, strSN.c_str());
			miter->second->expires = nExpires;
			miter->second->session_time = time(NULL);
		}
		__subcatlocker.unlock();
	}
	xml.reset();
}
//通知消息
void SipService::on_notify_request(socket_stream * stream, const char * data, size_t dlen)
{
	acl::string strAddr = stream->get_peer(true);
	acl::string szData(data, dlen);
	char szAns[SIP_MSG_LEN] = { 0 };
	int nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), szData.c_str(), SIP_STATUS_200);
	stream->write(szAns, nAnsLen);

	char szBody[SIP_BODY_LEN] = { 0 };
	SIP_CONTENT_TYPE nBodyType = sip_get_body(szData.c_str(), szBody, sizeof(szBody));
	if (nBodyType != SIP_APPLICATION_XML)
		return;
	acl::xml1 xml(szBody);
	acl::string strCmdType = acl_xml_getElementValue(xml, "CmdType");
	acl::string strSn = acl_xml_getElementValue(xml, "SN");
	acl::string strDeviceID = acl_xml_getElementValue(xml, "DeviceID");
	if (strCmdType.compare("Alarm", false) == 0)
	{
		__subalmlocker.lock();
		sesmap_iter miter = __subalmmap.begin();
		while (miter != __subalmmap.end())
		{
			if (get_user_allow_device(miter->second->userid, strDeviceID.c_str()))
			{
				memset(szAns, 0, sizeof(szAns));
				char szFrom[SIP_HEADER_LEN] = { 0 };
				sprintf(szFrom, "sip:%s@%s:%d", strDeviceID.c_str(), m_conf.local_addr, m_conf.local_port);
				nAnsLen = sip_build_msg_request(szAns, sizeof(szAns), "NOTIFY", szFrom, NULL, miter->second->cseq + 1, szFrom, miter->second->user_from, miter->second->ttag, miter->second->ftag);
				nAnsLen = sip_set_header(szAns, sizeof(szAns), SIP_H_EVENT, "presence");
				nAnsLen = sip_set_header(szAns, sizeof(szAns), SIP_H_SUBCRIPTION_STATE, "active;expires=90;retry-after=0");
				sip_set_xml_sn(szBody, sizeof(szBody), miter->second->sn);
				nAnsLen = sip_set_body(szAns, sizeof(szAns), szBody, SIP_APPLICATION_XML);
				send_msg(miter->second->user_addr, szAns, nAnsLen);
			}
			miter++;
		}
		__subalmlocker.unlock();
		return;
	}
	int nSumNum = acl_xml_getElementInt(xml, "SumNum");
	acl::xml_node *pDevList = xml.getFirstElementByTag("DeviceList");
	if (!pDevList)
		return;
	__msglocker.lock();
	sesmap_iter siter = __msgmap.find(strSn.c_str());
	if (siter != __msgmap.end())
	{
		response_msg(siter->second, szData.c_str(), szData.size());
		int nNum = atoi(pDevList->attr_value("Num"));
		siter->second->session_time = time(NULL);
		siter->second->rec_count += nNum;
		if (siter->second->rec_count == nSumNum)
		{
			printf("向[%s]发送目录[%d]完毕\n", siter->second->user_from, nSumNum);
		}
		__msglocker.unlock();
	}
	else
	{
		__msglocker.unlock();
		acl::xml_node *pDevItem = pDevList->first_child();
		while (pDevItem)
		{
			acl::xml_node *pItem = pDevItem->first_child();
			acl::string strItemDeviceID;
			acl::string strItemEvent;
			while (pItem)
			{
				if (strcmp(pItem->tag_name(), "DeviceID") == 0)
					strItemDeviceID = pItem->text();
				else if (strcmp(pItem->tag_name(), "Event") == 0)
					strItemEvent = pItem->text();
				pItem = pDevItem->next_child();
			}
			__subcatlocker.lock();
			siter = __subcatmap.begin();
			while (siter != __subcatmap.end())
			{
				//查询订阅用户权限
				if (get_user_allow_device(siter->second->userid, strItemDeviceID.c_str()))
				{
					memset(szAns, 0, sizeof(szAns));
					char szFrom[SIP_HEADER_LEN] = { 0 };
					sprintf(szFrom, "sip:%s@%s:%d", strDeviceID.c_str(), m_conf.local_addr, m_conf.local_port);
					nAnsLen = sip_build_msg_request(szAns, sizeof(szAns), "NOTIFY", szFrom, NULL, siter->second->cseq + 1, szFrom, siter->second->user_from, siter->second->ttag, siter->second->ftag);
					char szContact[SIP_HEADER_LEN] = { 0 };
					sprintf(szContact, "<%s:%d>", m_conf.local_addr, m_conf.local_port);
					nAnsLen = sip_set_header(szAns, sizeof(szAns), SIP_H_CONTACT, szContact);
					char szEvent[SIP_HEADER_LEN] = { 0 };
					sprintf(szEvent, "Catalog;id=%s", siter->second->sn);
					nAnsLen = sip_set_header(szAns, sizeof(szAns), SIP_H_EVENT, szEvent);
					nAnsLen = sip_set_header(szAns, sizeof(szAns), SIP_H_SUBCRIPTION_STATE, "active");
					memset(szBody, 0, sizeof(szBody));
					sprintf(szBody,
						"<?xml version=\"1.0\"?>%s"
						"<Notify>%s"
						"<CmdType>Catalog</CmdType>%s"
						"<SN>%s</SN>%s"
						"<DeviceID>%s</DeviceID>%s"
						"<SumNum>1</SumNum>%s"
						"<DeviceList Num=\"1\">%s"
						"<Item>%s"
						"<DeviceID>%s</DeviceID>%s"
						"<Event>%s</Event>%s"
						"</Item>%s"
						"</DeviceList>%s"
						"</Notify>%s",
						Line, Line, Line, siter->second->sn, Line, strDeviceID.c_str(), Line, Line, Line, Line,
						strItemDeviceID.c_str(), Line, strItemEvent.c_str(), Line, Line, Line, Line);
					nAnsLen = sip_set_body(szAns, sizeof(szAns), szBody, SIP_APPLICATION_XML);
					send_msg(siter->second->user_addr, szAns, nAnsLen);
				}
				siter++;
			}
			__subcatlocker.unlock();
			pDevItem = pDevList->next_child();
		}
	}
	xml.reset();
}
//查询消息判断
void SipService::on_message_request(socket_stream * stream, const char * data, size_t dlen)
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
	case SIP_RESPONSE_CONTROL:
		on_msg_control_response(stream, data, dlen);
		break;
	case SIP_QUERY_PRESETQUERY:
		on_msg_preset_request(stream, data, dlen);
		break;
	case SIP_RESPONSE_PRESETQUERY:
		on_msg_preset_response(stream, data, dlen);
		break;
	case SIP_QUERY_DEVICESTATUS:
		on_msg_devicestate_request(stream, data, dlen);
		break;
	case SIP_RESPONSE_DEVICESTATUS:
		on_msg_devicestate_response(stream, data, dlen);
		break;
	case SIP_QUERY_CATALOG:
		on_msg_catalog_request(stream, data, dlen);
		break;
	case SIP_RESPONSE_CATALOG:
		on_msg_catalog_response(stream, data, dlen);
		break;
	case SIP_QUERY_DEVICEINFO:
		on_msg_deviceinfo_request(stream, data, dlen);
		break;
	case SIP_RESPONSE_DEVICEINFO:
		on_msg_deviceinfo_response(stream, data, dlen);
		break;
	case SIP_QUERY_RECORDINFO:
		on_msg_recordinfo_request(stream, data, dlen);
		break;
	case SIP_RESPONSE_RECORDINFO:
		on_msg_recordinfo_response(stream, data, dlen);
		break;
	case SIP_QUERY_ALARM:
		on_msg_alarm_request(stream, data, dlen);
		break;
	case SIP_RESPONSE_QUERY_ALARM:
		on_msg_alarm_response(stream, data, dlen);
		break;
	case SIP_QUERY_CONFIGDOWNLOAD:
		on_msg_configdownload_request(stream, data, dlen);
		break;
	case SIP_RESPONSE_CONFIGDOWNLOAD:
		on_msg_configdownload_response(stream, data, dlen);
		break;
	case SIP_QUERY_MOBILEPOSITION:
		on_msg_mobileposition_request(stream, data, dlen);
		break;
	case SIP_NOTIFY_MOBILEPOSITION:
		on_msg_mobileposition_notify(stream, data, dlen);
		break;
	case SIP_NOTIFY_KEEPALIVE:
		on_msg_keepalive_notify(stream, data, dlen);
		break;
	case SIP_NOTIFY_MEDIASTATUS:
		on_msg_mediastatus_notify(stream, data, dlen);
		break;
	case SIP_NOTIFY_ALARM:
		on_msg_alarm_notify(stream, data, dlen);
		break;
	case SIP_NOTIFY_BROADCAST:
		on_msg_broadcast_request(stream, data, dlen);
		break;
	case SIP_RESPONSE_BROADCAST:
		on_msg_broadcast_response(stream, data, dlen);
		break;
	default:
		break;
	}
}

bool SipService::transfer_msg(const char * deviceid, const char * msg, size_t mlen)
{
	chnmap_iter citer = __chnmap.find(deviceid);
	if (citer == __chnmap.end())
		return false;
	char szAns[SIP_MSG_LEN] = { 0 };
	int nAnsLen = 0;
	memcpy(szAns, msg, mlen);
	char szUri[SIP_HEADER_LEN] = { 0 };
	sprintf(szUri, "%s:%d", m_conf.local_addr, m_conf.local_port);
	nAnsLen = sip_set_via_addr(szAns, sizeof(szAns), szUri);
	sprintf(szUri, "sip:%s@%s:%d", m_conf.local_code, m_conf.local_addr, m_conf.local_port);
	nAnsLen = sip_set_from_uri(szAns, sizeof(szAns), szUri);
	sprintf(szUri, "sip:%s@%s:%d", deviceid, citer->second.szIPAddress, citer->second.nPort);
	nAnsLen = sip_set_start_line_uri(szAns, sizeof(szAns), szUri);
	nAnsLen = sip_set_to_uri(szAns, sizeof(szAns), szUri);
	nAnsLen = sip_set_user_agent(szAns, sizeof(szAns), SIP_USER_AGENT);
	if (strcmp(m_conf.local_code, citer->second.szSIPCode) == 0)	//SDK取流
	{
		register_info *pReg = register_get_mediaservice(deviceid);
		if (pReg)
			return send_msg(pReg->addr, szAns, nAnsLen);
	}
	char szAddr[24] = { 0 };
	sprintf(szAddr, "%s:%d", citer->second.szIPAddress, citer->second.nPort);
	return send_msg(szAddr, szAns, nAnsLen);
}

bool SipService::response_msg(session_info * pInfo, const char * msg, size_t mlen)
{
	char szAns[SIP_MSG_LEN] = { 0 };
	int nAnsLen = 0;
	memcpy(szAns, msg, mlen);
	char szUri[SIP_HEADER_LEN] = { 0 };
	sprintf(szUri, "%s:%d", m_conf.local_addr, m_conf.local_port);
	nAnsLen = sip_set_via_addr(szAns, sizeof(szAns), szUri);
	sprintf(szUri, "sip:%s@%s:%d", pInfo->deviceid, m_conf.local_addr, m_conf.local_port);
	nAnsLen = sip_set_start_line_uri(szAns, sizeof(szAns), szUri);
	nAnsLen = sip_set_from_uri(szAns, sizeof(szAns), szUri);
	nAnsLen = sip_set_to_uri(szAns, sizeof(szAns), pInfo->user_from);
	nAnsLen = sip_set_user_agent(szAns, sizeof(szAns), SIP_USER_AGENT);
	return send_msg(pInfo->user_addr, szAns, nAnsLen);
}
//云台控制
void SipService::on_msg_ptzcmd_request(socket_stream * stream, const char * data, size_t dlen)
{
	acl::string strMsg(data, dlen);
	char szAns[SIP_MSG_LEN] = { 0 };
	int nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), strMsg.c_str(), SIP_STATUS_200);
	stream->write(szAns, nAnsLen);
	if (m_conf.srv_role == SIP_ROLE_CLIENT)
		return;
	char szUser[SIP_HEADER_LEN] = { 0 };
	sip_get_from_user(szUser, sizeof(szUser), strMsg.c_str());
	char szBody[SIP_BODY_LEN] = { 0 };
	SIP_CONTENT_TYPE nBodyType = sip_get_body(strMsg, szBody, sizeof(szBody));
	if (nBodyType != SIP_APPLICATION_XML)
		return;
	acl::xml1 xml(szBody);
	acl::string strDeviceID = acl_xml_getElementValue(xml, "DeviceID");
	acl::string strPTZCmd = acl_xml_getElementValue(xml, "PTZCmd");
	xml.reset();
	transfer_msg(strDeviceID.c_str(), strMsg.c_str(), strMsg.size());
}
//设备远程重启
void SipService::on_msg_teleboot_request(socket_stream * stream, const char * data, size_t dlen)
{
	acl::string strMsg(data, dlen);
	char szAns[SIP_MSG_LEN] = { 0 };
	int nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), strMsg.c_str(), SIP_STATUS_200);
	stream->write(szAns, nAnsLen);
	if (m_conf.srv_role == SIP_ROLE_CLIENT)
		return;
	char szBody[SIP_BODY_LEN] = { 0 };
	SIP_CONTENT_TYPE nBodyType = sip_get_body(strMsg, szBody, sizeof(szBody));
	if (nBodyType != SIP_APPLICATION_XML)
		return;
	acl::xml1 xml(szBody);
	acl::string strDeviceID = acl_xml_getElementValue(xml, "DeviceID");
	xml.reset();
	transfer_msg(strDeviceID.c_str(), strMsg.c_str(), strMsg.size());
}
//设备手动录像
void SipService::on_msg_recordcmd_request(socket_stream * stream, const char * data, size_t dlen)
{
	acl::string strAddr = stream->get_peer(true);
	acl::string strMsg(data, dlen);
	char szAns[SIP_MSG_LEN] = { 0 };
	int nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), strMsg.c_str(), SIP_STATUS_200);
	stream->write(szAns, nAnsLen);
	if (m_conf.srv_role == SIP_ROLE_CLIENT)
		return;
	char szBody[SIP_BODY_LEN] = { 0 };
	SIP_CONTENT_TYPE nBodyType = sip_get_body(strMsg, szBody, sizeof(szBody));
	if (nBodyType != SIP_APPLICATION_XML)
		return;
	acl::xml1 xml(szBody);
	acl::string strDeviceID = acl_xml_getElementValue(xml, "DeviceID");
	acl::string strSN = acl_xml_getElementValue(xml, "SN");
	xml.reset();

	__msglocker.lock();
	sesmap_iter miter = __msgmap.find(strSN.c_str());
	if (miter == __msgmap.end())
	{
		session_info *pInfo = new session_info;
		memset(pInfo, 0, sizeof(session_info));
		strcpy(pInfo->user_addr, strAddr.c_str());
		strcpy(pInfo->deviceid, strDeviceID.c_str());
		sip_get_from_uri(pInfo->user_from, sizeof(pInfo->user_from), strMsg.c_str());
		sip_get_from_tag(pInfo->ftag, sizeof(pInfo->ftag), strMsg.c_str());
		pInfo->cseq = sip_get_cseq_num(strMsg.c_str());
		pInfo->session_time = time(NULL);
		__msgmap[strSN.c_str()] = pInfo;
		transfer_msg(strDeviceID.c_str(), strMsg.c_str(), strMsg.size());
	}
	__msglocker.unlock();
}
//布防撤防
void SipService::on_msg_guardcmd_request(socket_stream * stream, const char * data, size_t dlen)
{
	acl::string strAddr = stream->get_peer(true);
	acl::string strMsg(data, dlen);
	char szAns[SIP_MSG_LEN] = { 0 };
	int nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), strMsg.c_str(), SIP_STATUS_200);
	stream->write(szAns, nAnsLen);
	if (m_conf.srv_role == SIP_ROLE_CLIENT)
		return;
	char szBody[SIP_BODY_LEN] = { 0 };
	SIP_CONTENT_TYPE nBodyType = sip_get_body(strMsg, szBody, sizeof(szBody));
	if (nBodyType != SIP_APPLICATION_XML)
		return;
	acl::xml1 xml(szBody);
	acl::string strDeviceID = acl_xml_getElementValue(xml, "DeviceID");
	acl::string strSN = acl_xml_getElementValue(xml, "SN");
	xml.reset();

	__msglocker.lock();
	sesmap_iter miter = __msgmap.find(strSN.c_str());
	if (miter == __msgmap.end())
	{
		session_info *pInfo = new session_info;
		memset(pInfo, 0, sizeof(session_info));
		strcpy(pInfo->user_addr, strAddr.c_str());
		strcpy(pInfo->deviceid, strDeviceID.c_str());
		sip_get_from_uri(pInfo->user_from, sizeof(pInfo->user_from), strMsg.c_str());
		sip_get_from_tag(pInfo->ftag, sizeof(pInfo->ftag), strMsg.c_str());
		pInfo->cseq = sip_get_cseq_num(strMsg.c_str());
		pInfo->session_time = time(NULL);
		__msgmap[strSN.c_str()] = pInfo;
		transfer_msg(strDeviceID.c_str(), strMsg.c_str(), strMsg.size());
	}
	__msglocker.unlock();
}
//报警复位
void SipService::on_msg_alarmcmd_request(socket_stream * stream, const char * data, size_t dlen)
{
	acl::string strAddr = stream->get_peer(true);
	acl::string strMsg(data, dlen);
	char szAns[SIP_MSG_LEN] = { 0 };
	int nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), strMsg.c_str(), SIP_STATUS_200);
	stream->write(szAns, nAnsLen);
	if (m_conf.srv_role == SIP_ROLE_CLIENT)
		return;
	char szBody[SIP_BODY_LEN] = { 0 };
	SIP_CONTENT_TYPE nBodyType = sip_get_body(strMsg, szBody, sizeof(szBody));
	if (nBodyType != SIP_APPLICATION_XML)
		return;
	acl::xml1 xml(szBody);
	acl::string strDeviceID = acl_xml_getElementValue(xml, "DeviceID");
	acl::string strSN = acl_xml_getElementValue(xml, "SN");
	xml.reset();
	
	__msglocker.lock();
	sesmap_iter miter = __msgmap.find(strSN.c_str());
	if (miter == __msgmap.end())
	{
		session_info *pInfo = new session_info;
		memset(pInfo, 0, sizeof(session_info));
		strcpy(pInfo->user_addr, strAddr.c_str());
		strcpy(pInfo->deviceid, strDeviceID.c_str());
		sip_get_from_uri(pInfo->user_from, sizeof(pInfo->user_from), strMsg.c_str());
		sip_get_from_tag(pInfo->ftag, sizeof(pInfo->ftag), strMsg.c_str());
		pInfo->cseq = sip_get_cseq_num(strMsg.c_str());
		pInfo->session_time = time(NULL);
		__msgmap[strSN.c_str()] = pInfo;
		transfer_msg(strDeviceID.c_str(), strMsg.c_str(), strMsg.size());
	}
	__msglocker.unlock();
}
//强制I帧
void SipService::on_msg_ifamecmd_request(socket_stream * stream, const char * data, size_t dlen)
{
	acl::string strMsg(data, dlen);
	char szAns[SIP_MSG_LEN] = { 0 };
	int nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), strMsg.c_str(), SIP_STATUS_200);
	stream->write(szAns, nAnsLen);
	if (m_conf.srv_role == SIP_ROLE_CLIENT)
		return;
	char szBody[SIP_BODY_LEN] = { 0 };
	SIP_CONTENT_TYPE nBodyType = sip_get_body(strMsg, szBody, sizeof(szBody));
	if (nBodyType != SIP_APPLICATION_XML)
		return;
	acl::xml1 xml(szBody);
	acl::string strDeviceID = acl_xml_getElementValue(xml, "DeviceID");
	xml.reset();
	transfer_msg(strDeviceID.c_str(), strMsg.c_str(), strMsg.size());
}
//拉框放大
void SipService::on_msg_dragzoomin_request(socket_stream * stream, const char * data, size_t dlen)
{
	acl::string strMsg(data, dlen);
	char szAns[SIP_MSG_LEN] = { 0 };
	int nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), strMsg.c_str(), SIP_STATUS_200);
	stream->write(szAns, nAnsLen);
	if (m_conf.srv_role == SIP_ROLE_CLIENT)
		return;
	char szBody[SIP_BODY_LEN] = { 0 };
	SIP_CONTENT_TYPE nBodyType = sip_get_body(strMsg, szBody, sizeof(szBody));
	if (nBodyType != SIP_APPLICATION_XML)
		return;
	acl::xml1 xml(szBody);
	acl::string strDeviceID = acl_xml_getElementValue(xml, "DeviceID");
	xml.reset();
	transfer_msg(strDeviceID.c_str(), strMsg.c_str(), strMsg.size());
}
//拉框缩小
void SipService::on_msg_dragzoomout_request(socket_stream * stream, const char * data, size_t dlen)
{
	acl::string strMsg(data, dlen);
	char szAns[SIP_MSG_LEN] = { 0 };
	int nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), strMsg.c_str(), SIP_STATUS_200);
	stream->write(szAns, nAnsLen);
	if (m_conf.srv_role == SIP_ROLE_CLIENT)
		return;
	char szBody[SIP_BODY_LEN] = { 0 };
	SIP_CONTENT_TYPE nBodyType = sip_get_body(strMsg, szBody, sizeof(szBody));
	if (nBodyType != SIP_APPLICATION_XML)
		return;
	acl::xml1 xml(szBody);
	acl::string strDeviceID = acl_xml_getElementValue(xml, "DeviceID");
	xml.reset();
	transfer_msg(strDeviceID.c_str(), strMsg.c_str(), strMsg.size());
}
//守望控制
void SipService::on_msg_homeposition_request(socket_stream * stream, const char * data, size_t dlen)
{
	acl::string strAddr = stream->get_peer(true);
	acl::string strMsg(data, dlen);
	char szAns[SIP_MSG_LEN] = { 0 };
	int nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), strMsg.c_str(), SIP_STATUS_200);
	stream->write(szAns, nAnsLen);
	if (m_conf.srv_role == SIP_ROLE_CLIENT)
		return;
	char szBody[SIP_BODY_LEN] = { 0 };
	SIP_CONTENT_TYPE nBodyType = sip_get_body(strMsg, szBody, sizeof(szBody));
	if (nBodyType != SIP_APPLICATION_XML)
		return;
	acl::xml1 xml(szBody);
	acl::string strDeviceID = acl_xml_getElementValue(xml, "DeviceID");
	acl::string strSN = acl_xml_getElementValue(xml, "SN");
	xml.reset();
	
	__msglocker.lock();
	sesmap_iter miter = __msgmap.find(strSN.c_str());
	if (miter == __msgmap.end())
	{
		session_info *pInfo = new session_info;
		memset(pInfo, 0, sizeof(session_info));
		strcpy(pInfo->user_addr, strAddr.c_str());
		strcpy(pInfo->deviceid, strDeviceID.c_str());
		sip_get_from_uri(pInfo->user_from, sizeof(pInfo->user_from), strMsg.c_str());
		sip_get_from_tag(pInfo->ftag, sizeof(pInfo->ftag), strMsg.c_str());
		pInfo->cseq = sip_get_cseq_num(strMsg.c_str());
		pInfo->session_time = time(NULL);
		__msgmap[strSN.c_str()] = pInfo;
		transfer_msg(strDeviceID.c_str(), strMsg.c_str(), strMsg.size());
	}
	__msglocker.unlock();
}
//配置设置
void SipService::on_msg_config_request(socket_stream * stream, const char * data, size_t dlen)
{
	acl::string strAddr = stream->get_peer(true);
	acl::string strMsg(data, dlen);
	char szAns[SIP_MSG_LEN] = { 0 };
	int nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), strMsg.c_str(), SIP_STATUS_200);
	stream->write(szAns, nAnsLen);
	if (m_conf.srv_role == SIP_ROLE_CLIENT)
		return;
	char szBody[SIP_BODY_LEN] = { 0 };
	SIP_CONTENT_TYPE nBodyType = sip_get_body(strMsg, szBody, sizeof(szBody));
	if (nBodyType != SIP_APPLICATION_XML)
		return;
	acl::xml1 xml(szBody);
	acl::string strDeviceID = acl_xml_getElementValue(xml, "DeviceID");
	acl::string strSN = acl_xml_getElementValue(xml, "SN");
	xml.reset();
	
	__msglocker.lock();
	sesmap_iter miter = __msgmap.find(strSN.c_str());
	if (miter == __msgmap.end())
	{
		session_info *pInfo = new session_info;
		memset(pInfo, 0, sizeof(session_info));
		strcpy(pInfo->user_addr, strAddr.c_str());
		strcpy(pInfo->deviceid, strDeviceID.c_str());
		sip_get_from_uri(pInfo->user_from, sizeof(pInfo->user_from), strMsg.c_str());
		sip_get_from_tag(pInfo->ftag, sizeof(pInfo->ftag), strMsg.c_str());
		pInfo->cseq = sip_get_cseq_num(strMsg.c_str());
		pInfo->session_time = time(NULL);
		__msgmap[strSN.c_str()] = pInfo;
		transfer_msg(strDeviceID.c_str(), strMsg.c_str(), strMsg.size());
	}
	__msglocker.unlock();
}
//预置位查询
void SipService::on_msg_preset_request(socket_stream * stream, const char * data, size_t dlen)
{
	acl::string strAddr = stream->get_peer(true);
	acl::string strMsg(data, dlen);
	char szAns[SIP_MSG_LEN] = { 0 };
	int nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), strMsg.c_str(), SIP_STATUS_200);
	stream->write(szAns, nAnsLen);
	if (m_conf.srv_role == SIP_ROLE_CLIENT)
		return;
	char szBody[SIP_BODY_LEN] = { 0 };
	SIP_CONTENT_TYPE nBodyType = sip_get_body(strMsg, szBody, sizeof(szBody));
	if (nBodyType != SIP_APPLICATION_XML)
		return;
	acl::xml1 xml(szBody);
	acl::string strDeviceID = acl_xml_getElementValue(xml, "DeviceID");
	acl::string strSN = acl_xml_getElementValue(xml, "SN");
	xml.reset();
	
	__msglocker.lock();
	sesmap_iter miter = __msgmap.find(strSN.c_str());
	if (miter == __msgmap.end())
	{
		session_info *pInfo = new session_info;
		memset(pInfo, 0, sizeof(session_info));
		strcpy(pInfo->user_addr, strAddr.c_str());
		strcpy(pInfo->deviceid, strDeviceID.c_str());
		sip_get_from_uri(pInfo->user_from, sizeof(pInfo->user_from), strMsg.c_str());
		sip_get_from_tag(pInfo->ftag, sizeof(pInfo->ftag), strMsg.c_str());
		pInfo->cseq = sip_get_cseq_num(strMsg.c_str());
		pInfo->session_time = time(NULL);
		__msgmap[strSN.c_str()] = pInfo;
		transfer_msg(strDeviceID.c_str(), strMsg.c_str(), strMsg.size());
	}
	__msglocker.unlock();
}
//设备状态查询
void SipService::on_msg_devicestate_request(socket_stream * stream, const char * data, size_t dlen)
{
	acl::string strAddr = stream->get_peer(true);
	acl::string strMsg(data, dlen);
	char szAns[SIP_MSG_LEN] = { 0 };
	int nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), strMsg.c_str(), SIP_STATUS_200);
	stream->write(szAns, nAnsLen);
	if (m_conf.srv_role == SIP_ROLE_CLIENT)
		return;
	char szBody[SIP_BODY_LEN] = { 0 };
	SIP_CONTENT_TYPE nBodyType = sip_get_body(strMsg, szBody, sizeof(szBody));
	if (nBodyType != SIP_APPLICATION_XML)
		return;
	acl::xml1 xml(szBody);
	acl::string strDeviceID = acl_xml_getElementValue(xml, "DeviceID");
	acl::string strSN = acl_xml_getElementValue(xml, "SN");
	xml.reset();

	__msglocker.lock();
	sesmap_iter miter = __msgmap.find(strSN.c_str());
	if (miter == __msgmap.end())
	{
		session_info *pInfo = new session_info;
		memset(pInfo, 0, sizeof(session_info));
		strcpy(pInfo->user_addr, strAddr.c_str());
		strcpy(pInfo->deviceid, strDeviceID.c_str());
		sip_get_to_uri(pInfo->user_to, sizeof(pInfo->user_to), strMsg.c_str());
		sip_get_to_tag(pInfo->ttag, sizeof(pInfo->ttag), strMsg.c_str());
		pInfo->cseq = sip_get_cseq_num(strMsg.c_str());
		pInfo->session_time = time(NULL);
		__msgmap[strSN.c_str()] = pInfo;
		transfer_msg(strDeviceID.c_str(), strMsg.c_str(), strMsg.size());
	}
	__msglocker.unlock();
}
//目录查询
void SipService::on_msg_catalog_request(socket_stream * stream, const char * data, size_t dlen)
{
	acl::string strAddr = stream->get_peer(true);
	acl::string strMsg(data, dlen);
	char szAns[SIP_MSG_LEN] = { 0 };
	int nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), strMsg.c_str(), SIP_STATUS_200);
	stream->write(szAns, nAnsLen);
	if (m_conf.srv_role == SIP_ROLE_CLIENT)
		return;
	char szUser[21] = { 0 };
	sip_get_from_user(szUser, sizeof(szUser), strMsg.c_str());
	char szBody[SIP_BODY_LEN] = { 0 };
	SIP_CONTENT_TYPE nBodyType = sip_get_body(strMsg, szBody, sizeof(szBody));
	if (nBodyType != SIP_APPLICATION_XML)
		return;
	acl::xml1 xml(szBody);
	acl::string strDeviceID = acl_xml_getElementValue(xml, "DeviceID");
	acl::string strSN = acl_xml_getElementValue(xml, "SN");
	xml.reset();
	__subcatlocker.lock();
	sesmap_iter miter = __subcatmap.find(szUser);
	if (miter == __subcatmap.end())
	{
		session_info *pInfo = new session_info;
		memset(pInfo, 0, sizeof(session_info));
		regmap_iter riter = __regmap.find(strAddr.c_str());
		if (riter != __regmap.end())	//查询用户角色
		{
			pInfo->user_role = riter->second.role;
		}
		strcpy(pInfo->user_addr, strAddr.c_str());
		strcpy(pInfo->deviceid, strDeviceID.c_str());
		strcpy(pInfo->sn, strSN.c_str());
		sip_get_from_uri(pInfo->user_from, sizeof(pInfo->user_from), strMsg.c_str());
		sip_get_to_uri(pInfo->user_to, sizeof(pInfo->user_to), strMsg.c_str());
		sip_get_from_tag(pInfo->ttag, sizeof(pInfo->ttag), strMsg.c_str());
		sip_get_to_tag(pInfo->ftag, sizeof(pInfo->ftag), szAns);
		pInfo->cseq = sip_get_cseq_num(strMsg.c_str());
		__subcatmap[szUser] = pInfo;
		__msglocker.lock();
		__msgmap[strSN.c_str()] = pInfo;
		__msglocker.unlock();
		if (m_conf.srv_role == SIP_ROLE_SUPERIOR)
		{
			//创建目录推送线程
			catalog_thread *pCatalogThd = new catalog_thread(this, pInfo);
			pCatalogThd->set_detachable(true);
			pCatalogThd->start();
		}
		else
			transfer_msg(strDeviceID.c_str(), strMsg.c_str(), strMsg.size());
	}
	__subcatlocker.unlock();
}
//设备信息查询
void SipService::on_msg_deviceinfo_request(socket_stream * stream, const char * data, size_t dlen)
{
	acl::string strAddr = stream->get_peer(true);
	acl::string strMsg(data, dlen);
	char szAns[SIP_MSG_LEN] = { 0 };
	int nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), strMsg.c_str(), SIP_STATUS_200);
	stream->write(szAns, nAnsLen);
	if (m_conf.srv_role == SIP_ROLE_CLIENT)
		return;
	char szBody[SIP_BODY_LEN] = { 0 };
	SIP_CONTENT_TYPE nBodyType = sip_get_body(strMsg, szBody, sizeof(szBody));
	if (nBodyType != SIP_APPLICATION_XML)
		return;
	acl::xml1 xml(szBody);
	acl::string strDeviceID = acl_xml_getElementValue(xml, "DeviceID");
	acl::string strSN = acl_xml_getElementValue(xml, "SN");
	xml.reset();
	
	__msglocker.lock();
	sesmap_iter miter = __msgmap.find(strSN.c_str());
	if (miter == __msgmap.end())
	{
		session_info *pInfo = new session_info;
		memset(pInfo, 0, sizeof(session_info));
		strcpy(pInfo->user_addr, strAddr.c_str());
		strcpy(pInfo->deviceid, strDeviceID.c_str());
		sip_get_to_uri(pInfo->user_to, sizeof(pInfo->user_to), strMsg.c_str());
		sip_get_to_tag(pInfo->ttag, sizeof(pInfo->ttag), strMsg.c_str());
		pInfo->cseq = sip_get_cseq_num(strMsg.c_str());
		pInfo->session_time = time(NULL);
		__msgmap[strSN.c_str()] = pInfo;
		transfer_msg(strDeviceID.c_str(), strMsg.c_str(), strMsg.size());
	}
	__msglocker.unlock();
}
//录像查询
void SipService::on_msg_recordinfo_request(socket_stream * stream, const char * data, size_t dlen)
{
	acl::string strAddr = stream->get_peer(true);
	acl::string strMsg(data, dlen);
	char szAns[SIP_MSG_LEN] = { 0 };
	int nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), strMsg.c_str(), SIP_STATUS_200);
	stream->write(szAns, nAnsLen);
	if (m_conf.srv_role == SIP_ROLE_CLIENT)
		return;
	char szBody[SIP_BODY_LEN] = { 0 };
	SIP_CONTENT_TYPE nBodyType = sip_get_body(strMsg, szBody, sizeof(szBody));
	if (nBodyType != SIP_APPLICATION_XML)
		return;
	acl::xml1 xml(szBody);
	acl::string strDeviceID = acl_xml_getElementValue(xml, "DeviceID");
	acl::string strSN = acl_xml_getElementValue(xml, "SN");
	xml.reset();

	__msglocker.lock();
	sesmap_iter miter = __msgmap.find(strSN.c_str());
	if (miter == __msgmap.end())
	{
		session_info *pInfo = new session_info;
		memset(pInfo, 0, sizeof(session_info));
		strcpy(pInfo->user_addr, strAddr.c_str());
		strcpy(pInfo->deviceid, strDeviceID.c_str());
		sip_get_to_uri(pInfo->user_to, sizeof(pInfo->user_to), strMsg.c_str());
		sip_get_to_tag(pInfo->ttag, sizeof(pInfo->ttag), strMsg.c_str());
		pInfo->cseq = sip_get_cseq_num(strMsg.c_str());
		pInfo->session_time = time(NULL);
		__msgmap[strSN.c_str()] = pInfo;
		transfer_msg(strDeviceID.c_str(), strMsg.c_str(), strMsg.size());
	}
	__msglocker.unlock();
}
//报警查询
void SipService::on_msg_alarm_request(socket_stream * stream, const char * data, size_t dlen)
{
	acl::string strAddr = stream->get_peer(true);
	acl::string strMsg(data, dlen);
	char szAns[SIP_MSG_LEN] = { 0 };
	int nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), strMsg.c_str(), SIP_STATUS_200);
	stream->write(szAns, nAnsLen);
	if (m_conf.srv_role == SIP_ROLE_CLIENT)
		return;
	char szBody[SIP_BODY_LEN] = { 0 };
	SIP_CONTENT_TYPE nBodyType = sip_get_body(strMsg, szBody, sizeof(szBody));
	if (nBodyType != SIP_APPLICATION_XML)
		return;
	acl::xml1 xml(szBody);
	acl::string strDeviceID = acl_xml_getElementValue(xml, "DeviceID");
	acl::string strSN = acl_xml_getElementValue(xml, "SN");
	xml.reset();

	__msglocker.lock();
	sesmap_iter miter = __msgmap.find(strSN.c_str());
	if (miter == __msgmap.end())
	{
		session_info *pInfo = new session_info;
		memset(pInfo, 0, sizeof(session_info));
		strcpy(pInfo->user_addr, strAddr.c_str());
		strcpy(pInfo->deviceid, strDeviceID.c_str());
		sip_get_to_uri(pInfo->user_to, sizeof(pInfo->user_to), strMsg.c_str());
		sip_get_to_tag(pInfo->ttag, sizeof(pInfo->ttag), strMsg.c_str());
		pInfo->cseq = sip_get_cseq_num(strMsg.c_str());
		pInfo->session_time = time(NULL);
		__msgmap[strSN.c_str()] = pInfo;
		transfer_msg(strDeviceID.c_str(), strMsg.c_str(), strMsg.size());
	}
	__msglocker.unlock();
}
//配置查询
void SipService::on_msg_configdownload_request(socket_stream * stream, const char * data, size_t dlen)
{
	acl::string strAddr = stream->get_peer(true);
	acl::string strMsg(data, dlen);
	char szAns[SIP_MSG_LEN] = { 0 };
	int nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), strMsg.c_str(), SIP_STATUS_200);
	stream->write(szAns, nAnsLen);
	if (m_conf.srv_role == SIP_ROLE_CLIENT)
		return;
	char szBody[SIP_BODY_LEN] = { 0 };
	SIP_CONTENT_TYPE nBodyType = sip_get_body(strMsg, szBody, sizeof(szBody));
	if (nBodyType != SIP_APPLICATION_XML)
		return;
	acl::xml1 xml(szBody);
	acl::string strDeviceID = acl_xml_getElementValue(xml, "DeviceID");
	acl::string strSN = acl_xml_getElementValue(xml, "SN");
	xml.reset();

	__msglocker.lock();
	sesmap_iter miter = __msgmap.find(strSN.c_str());
	if (miter == __msgmap.end())
	{
		session_info *pInfo = new session_info;
		memset(pInfo, 0, sizeof(session_info));
		strcpy(pInfo->user_addr, strAddr.c_str());
		strcpy(pInfo->deviceid, strDeviceID.c_str());
		sip_get_to_uri(pInfo->user_to, sizeof(pInfo->user_to), strMsg.c_str());
		sip_get_to_tag(pInfo->ttag, sizeof(pInfo->ttag), strMsg.c_str());
		pInfo->cseq = sip_get_cseq_num(strMsg.c_str());
		pInfo->session_time = time(NULL);
		__msgmap[strSN.c_str()] = pInfo;
		transfer_msg(strDeviceID.c_str(), strMsg.c_str(), strMsg.size());
	}
	__msglocker.unlock();
	
}
//GPS查询
void SipService::on_msg_mobileposition_request(socket_stream * stream, const char * data, size_t dlen)
{
	acl::string strAddr = stream->get_peer(true);
	acl::string strMsg(data, dlen);
	char szAns[SIP_MSG_LEN] = { 0 };
	int nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), strMsg.c_str(), SIP_STATUS_200);
	stream->write(szAns, nAnsLen);
	if (m_conf.srv_role == SIP_ROLE_CLIENT)
		return;
	char szBody[SIP_BODY_LEN] = { 0 };
	SIP_CONTENT_TYPE nBodyType = sip_get_body(strMsg, szBody, sizeof(szBody));
	if (nBodyType != SIP_APPLICATION_XML)
		return;
	acl::xml1 xml(szBody);
	acl::string strDeviceID = acl_xml_getElementValue(xml, "DeviceID");
	acl::string strSN = acl_xml_getElementValue(xml, "SN");
	xml.reset();

	__msglocker.lock();
	sesmap_iter miter = __msgmap.find(strSN.c_str());
	if (miter == __msgmap.end())
	{
		session_info *pInfo = new session_info;
		memset(pInfo, 0, sizeof(session_info));
		strcpy(pInfo->user_addr, strAddr.c_str());
		strcpy(pInfo->deviceid, strDeviceID.c_str());
		sip_get_to_uri(pInfo->user_to, sizeof(pInfo->user_to), strMsg.c_str());
		sip_get_to_tag(pInfo->ttag, sizeof(pInfo->ttag), strMsg.c_str());
		pInfo->cseq = sip_get_cseq_num(strMsg.c_str());
		pInfo->session_time = time(NULL);
		__msgmap[strSN.c_str()] = pInfo;
		transfer_msg(strDeviceID.c_str(), strMsg.c_str(), strMsg.size());
	}
	__msglocker.unlock();
	
}
//GPS通知
void SipService::on_msg_mobileposition_notify(socket_stream * stream, const char * data, size_t dlen)
{
	acl::string strMsg(data, dlen);
	char szAns[SIP_MSG_LEN] = { 0 };
	int nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), strMsg.c_str(), SIP_STATUS_200);
	stream->write(szAns, nAnsLen);

	char szBody[SIP_BODY_LEN] = { 0 };
	SIP_CONTENT_TYPE nBodyType = sip_get_body(strMsg, szBody, sizeof(szBody));
	if (nBodyType != SIP_APPLICATION_XML)
		return;
	acl::xml1 xml(szBody);
	acl::string strSN = acl_xml_getElementValue(xml, "SN");
	sesmap_iter siter = __msgmap.find(strSN.c_str());
	if (siter == __msgmap.end())
		return;
	siter->second->session_time = time(NULL);
	if (m_conf.srv_role == SIP_ROLE_CLIENT)
	{
		acl::string strTime = acl_xml_getElementValue(xml, "Time");
		acl::string strLongitude = acl_xml_getElementValue(xml, "Longitude");
		acl::string strLatitude = acl_xml_getElementValue(xml, "Latitude");
		acl::string strSpeed = acl_xml_getElementValue(xml, "Speed");
		acl::string strDirection = acl_xml_getElementValue(xml, "Direction");
		acl::string strAltitude = acl_xml_getElementValue(xml, "Altitude");
	}
	else
		response_msg(siter->second, strMsg.c_str(), strMsg.size());
	xml.reset();
}
//心跳通知
void SipService::on_msg_keepalive_notify(socket_stream * stream, const char * data, size_t dlen)
{
	acl::string szData(data, dlen);
	acl::string strAddr = stream->get_peer(true);
	regmap_iter riter = __regmap.find(strAddr.c_str());
	if (riter != __regmap.end())
	{
		if (riter->second.online)
		{
			char szAns[SIP_MSG_LEN] = { 0 };
			riter->second.alivetime = time(NULL);
			int nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), szData.c_str(), SIP_STATUS_200);
			stream->write(szAns, nAnsLen);
		}
		else
		{
			char szAns[SIP_MSG_LEN] = { 0 };
			int nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), szData.c_str(), SIP_STATUS_400);
			stream->write(szAns, nAnsLen);
		}
	}
	else
	{
		char szAns[SIP_MSG_LEN] = { 0 };
		int nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), szData.c_str(), SIP_STATUS_400);
		stream->write(szAns, nAnsLen);
	}
}
//媒体流结束通知
void SipService::on_msg_mediastatus_notify(socket_stream * stream, const char * data, size_t dlen)
{
	acl::string strAddr = stream->get_peer(true);
	acl::string szData(data, dlen);
	char szAns[SIP_MSG_LEN] = { 0 };
	int nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), data, SIP_STATUS_200);
	stream->write(szAns, nAnsLen);

	char szCallID[SIP_HEADER_LEN] = { 0 };
	sip_get_header(szCallID, sizeof(szCallID), szData.c_str(), SIP_H_CALL_ID);
	char szUser[SIP_HEADER_LEN] = { 0 };
	sip_get_to_user(szUser, sizeof(szUser), szData.c_str());

	sesmap_iter siter = __sesmap.find(szCallID);
	if (siter == __sesmap.end())
		return;
	session_info *pInfo = siter->second;

	if (m_conf.srv_role == SIP_ROLE_CLIENT)
	{
#ifdef _USRDLL
		media_stream_cb pCb = (media_stream_cb)pInfo->stream_cb;
		if (pCb && pInfo->stream_user)
			pCb(121, NULL, 0, pInfo->stream_user);
#endif
	}
	else
	{
		memcpy(szAns, szData.c_str(), szData.size());
		char szUri[SIP_HEADER_LEN] = { 0 };
		sprintf(szUri, "%s:%d", m_conf.local_addr, m_conf.local_port);
		nAnsLen = sip_set_via_addr(szAns, sizeof(szAns), szUri);
		sprintf(szUri, "sip:%s@%s:%d", pInfo->deviceid, m_conf.local_addr, m_conf.local_port);
		nAnsLen = sip_set_start_line_uri(szAns, sizeof(szAns), szUri);
		nAnsLen = sip_set_from_uri(szAns, sizeof(szAns), szUri);
		nAnsLen = sip_set_to_uri(szAns, sizeof(szAns), pInfo->user_from);
		nAnsLen = sip_set_user_agent(szAns, sizeof(szAns), SIP_USER_AGENT);
		send_msg(pInfo->user_addr, szAns, nAnsLen);
		if (pInfo->media_addr[0] != '\0')
		{
			sprintf(szUri, "sip:%s@%s", pInfo->media_user, pInfo->media_addr);
			nAnsLen = sip_set_to_uri(szAns, sizeof(szAns), szUri);
			register_info *pReg = register_get_mediaservice(pInfo->deviceid);
			if (pReg)
				send_msg(pReg->addr, szAns, nAnsLen);
		}
	}
	pInfo->mediaover = time(NULL);
}
//报警通知
void SipService::on_msg_alarm_notify(socket_stream * stream, const char * data, size_t dlen)
{
	acl::string strAddr = stream->get_peer(true);
	acl::string strMsg(data, dlen);
	char szAns[SIP_MSG_LEN] = { 0 };
	int nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), strMsg.c_str(), SIP_STATUS_200);
	stream->write(szAns, nAnsLen);

	char szTag[SIP_HEADER_LEN] = { 0 };
	sip_get_to_tag(szTag, sizeof(szTag), szAns);
	char szTo[SIP_HEADER_LEN] = { 0 };
	sip_get_from_uri(szTo, sizeof(szTo), szAns);

	char szBody[SIP_BODY_LEN] = { 0 };
	SIP_CONTENT_TYPE nBodyType = sip_get_body(strMsg.c_str(), szBody, sizeof(szBody));
	if (nBodyType != SIP_APPLICATION_XML)
		return;
	acl::xml1 xml(szBody);
	acl::string strSn = acl_xml_getElementValue(xml, "SN");
	acl::string strDeviceID = acl_xml_getElementValue(xml, "DeviceID");
	sesmap_iter siter = __msgmap.find(strSn.c_str());
	if (siter == __msgmap.end())
		return;
	session_info *pInfo = siter->second;
	pInfo->session_time = time(NULL);

	char szMsg[SIP_MSG_LEN] = { 0 };
	int nMsgLen = sip_build_msg_request(szMsg, sizeof(szMsg), "MESSAGE", strAddr.c_str(), NULL, pInfo->cseq + 1, NULL, szTo, szTag, NULL);
	memset(szBody, 0, sizeof(szBody));
	sprintf(szBody,
		"<?xml version=\"1.0\"?>%s"
		"<Response>%s"
		"<CmdType>Alarm</CmdType>%s"
		"<SN>%s</SN>%s"
		"<DeviceID>%s</DeviceID>%s"
		"<Result>OK</Result>%s"
		"</Response>%s",
		Line, Line, Line, strSn.c_str(), Line, strDeviceID.c_str(), Line, Line, Line);
	nMsgLen = sip_set_body(szMsg, sizeof(szMsg), szBody, SIP_APPLICATION_XML);
	stream->write(szMsg, nMsgLen);
	if (m_conf.srv_role == SIP_ROLE_CLIENT)
	{
		acl::string strAlarmPriority = acl_xml_getElementValue(xml, "AlarmPriority");
		acl::string strAlarmMethod = acl_xml_getElementValue(xml, "AlarmMethod");
		acl::string strAlarmTime = acl_xml_getElementValue(xml, "AlarmTime");
		acl::string strAlarmDescription = acl_xml_getElementValue(xml, "AlarmDescription");
		acl::string strLongitude = acl_xml_getElementValue(xml, "Longitude");
		acl::string strLatitude = acl_xml_getElementValue(xml, "Latitude");
	}
	else
		response_msg(siter->second, strMsg.c_str(), strMsg.size());
	xml.reset();
}
//语音广播通知
void SipService::on_msg_broadcast_request(socket_stream * stream, const char * data, size_t dlen)
{
	acl::string strAddr = stream->get_peer(true);
	acl::string strMsg(data, dlen);
	char szAns[SIP_MSG_LEN] = { 0 };
	int nAnsLen = sip_build_msg_answer(szAns, sizeof(szAns), strMsg.c_str(), SIP_STATUS_200);
	stream->write(szAns, nAnsLen);

	char szBody[SIP_BODY_LEN] = { 0 };
	SIP_CONTENT_TYPE nBodyType = sip_get_body(strMsg, szBody, sizeof(szBody));
	if (nBodyType != SIP_APPLICATION_XML)
		return;
	acl::xml1 xml(szBody);
	acl::string strDeviceID = acl_xml_getElementValue(xml, "DeviceID");
	acl::string strSN = acl_xml_getElementValue(xml, "SN");
	xml.reset();

	__msglocker.lock();
	sesmap_iter miter = __msgmap.find(strSN.c_str());
	if (miter == __msgmap.end())
	{
		session_info *pInfo = new session_info;
		memset(pInfo, 0, sizeof(session_info));
		strcpy(pInfo->user_addr, strAddr.c_str());
		strcpy(pInfo->deviceid, strDeviceID.c_str());
		sip_get_from_uri(pInfo->user_from, sizeof(pInfo->user_from), strMsg.c_str());
		sip_get_from_tag(pInfo->ftag, sizeof(pInfo->ftag), strMsg.c_str());
		pInfo->cseq = sip_get_cseq_num(strMsg.c_str());
		pInfo->session_time = time(NULL);
		__msgmap[strSN.c_str()] = pInfo;
		transfer_msg(strDeviceID.c_str(), strMsg.c_str(), strMsg.size());
	}
	__msglocker.unlock();
}

void printf_help()
{
	printf("================================================================\n");
	printf("==quit     退出服务=============================================\n");
	printf("==cls      清屏=================================================\n");
	printf("==ses      查询当前点播会话=====================================\n");
	printf("==reg      查询当前注册用户=====================================\n");
	printf("==dev      查询当前注册设备=====================================\n");
	printf("================================================================\n");
}

void SipService::scanf_command()
{
	char command[1024];
	while (m_running)
	{
		scanf("%s", command);
		if (strcmp(command, "help") == 0)
		{
			printf_help();
		}
		else if (strcmp(command, "quit") == 0)
		{
			m_running = false;
			exit(0);
		}
		else if (strcmp(command, "cls") == 0)
		{
			system("cls");
		}

		else if (strcmp(command, "ses") == 0)
		{
			printf("===============================================================\n");
			printf("当前播放进行中的会话[%d]\n", (int)__sesmap.size());
			sesmap_iter siter = __sesmap.begin();
			while (siter != __sesmap.end())
			{
				printf("DeviceID:%s SSRC:%s Port:%d flow:%d\n ", siter->second->deviceid, siter->second->ssrc, siter->second->receiver.recvport, siter->second->flow);
				siter++;
			}
			printf("===============================================================\n");
		}
		else if (strcmp(command, "reg") == 0)
		{
			printf("===============================================================\n");
			printf("上级SIP共享用户：\n");
			regmap_iter siter = __regmap.begin();
			while (siter != __regmap.end())
			{
				if (siter->second.role == SIP_ROLE_SUPERIOR)
					printf("Name:%s RegisterID:%s Addr:%s Status:%d\n ", siter->second.name, siter->second.user, siter->second.addr, siter->second.online);
				siter++;
			}
			printf("下级SIP注册用户：\n");
			siter = __regmap.begin();
			while (siter != __regmap.end())
			{
				if (siter->second.role == SIP_ROLE_SENDER)
					printf("Name:%s RegisterID:%s Addr:%s Status:%d\n ", siter->second.name, siter->second.user, siter->second.addr, siter->second.online);
				siter++;
			}
			printf("RTP流媒体用户：\n");
			siter = __regmap.begin();
			while (siter != __regmap.end())
			{
				if (siter->second.role == SIP_ROLE_MEDIA)
					printf("Name:%s RegisterID:%s Addr:%s Status:%d\n ", siter->second.name, siter->second.user, siter->second.addr, siter->second.online);
				siter++;
			}
			printf("客户端注册用户：\n");
			siter = __regmap.begin();
			while (siter != __regmap.end())
			{
				if (siter->second.role == SIP_ROLE_CLIENT)
					printf("Name:%s RegisterID:%s Addr:%s Status:%d\n ", siter->second.name, siter->second.user, siter->second.addr, siter->second.online);
				siter++;
			}
			printf("===============================================================\n");
			while (m_running)
			{
				printf("输入Addr内容发送目录查询，输入back返回上一级\n");
				scanf("%s", command);
				if (strcmp(command, "back") == 0)
				{
					printf_help();
					break;
				}
				else if (strchr(command, ':') && strlen(command) < 22)
				{
					siter = __regmap.find(command);
					if (siter != __regmap.end())
					{
						send_message_catalog(siter->second.user, siter->second.addr);
						printf("向[%s:%s]发送目录查询\n", siter->second.user, siter->second.addr);
						printf_help();
						break;
					}
				}
			}
		}
		else if (strcmp(command, "dev") == 0)
		{
			printf("===============================================================\n");
			printf("下级注册设备：\n");
			regmap_iter siter = __regmap.begin();
			while (siter != __regmap.end())
			{
				if (siter->second.role >= SIP_ROLE_DEVICE)
					printf("RegisterID:%s Addr:%s Status:%d\n ", siter->second.user, siter->second.addr, siter->second.online);
				siter++;
			}
			printf("===============================================================\n");
			while (m_running)
			{
				printf("输入Addr内容发送目录查询，输入back返回上一级\n");
				scanf("%s", command);
				if (strcmp(command, "back") == 0)
				{
					printf_help();
					break;
				}
				else if (strchr(command, ':') && strlen(command) < 22)
				{
					siter = __regmap.find(command);
					if (siter != __regmap.end())
					{
						send_message_catalog(siter->second.user, siter->second.addr);
						printf("向[%s:%s]发送目录查询\n", siter->second.user, siter->second.addr);
						printf_help();
						break;
					}
				}
			}
		}
	}
}

void SipService::register_manager()
{
	while (m_running)
	{
		regmap_iter riter = __regmap.begin();
		while (riter != __regmap.end())
		{
			time_t now = time(NULL);
			if (riter->second.role == SIP_ROLE_SUPERIOR)	//上级管理
			{
				if (riter->second.vaild == 0)	//失效
				{
					riter++;
					continue;
				}
				if (riter->second.online == false && now - riter->second.alivetime > SIP_REGISTERTIME)
				{
					//不在线，每60秒注册一次
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
					register_slist_clear(riter->second.addr);
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
			}
			else if (riter->second.role == SIP_ROLE_LOWER)	//下级管理
			{
				if (riter->second.online && now - riter->second.alivetime > SIP_ALIVETIMEOUT)
				{
					riter->second.online = false;
					register_slist_clear(riter->second.addr);
				}
				else if (riter->second.online && now - riter->second.heartbeat > 3600)
				{
					//每小时发送订阅
					if (riter->second.sn == 0)
						riter->second.sn = sip_build_randsn();
					send_subscribe(riter->second.user, riter->second.addr, riter->second.sn);
					riter->second.heartbeat = time(NULL);
				}
			}
			else if (riter->second.role == SIP_ROLE_CLIENT) //客户端管理
			{
				if (riter->second.online && now - riter->second.alivetime > SIP_ALIVETIMEOUT)
				{
					riter->second.online = false;
					register_slist_clear(riter->second.addr);
				}
			}
			else if (riter->second.role == SIP_ROLE_MEDIA)	//RTP流媒体管理
			{
				if (riter->second.online && now - riter->second.alivetime > SIP_ALIVETIMEOUT)
				{
					riter->second.online = false;
					register_slist_clear(riter->second.addr);
				}
			}
			riter++;
		}
		sleep(1);
	}
}

void SipService::register_slist_erase(const char * addr, const char * callid)
{
	__reglocker.lock();
	regmap_iter riter = __regmap.find(addr);
	if (riter != __regmap.end())
	{
		seslist_iter slter = riter->second.slist->begin();
		while (slter != riter->second.slist->end())
		{
			if (strcmp((*slter)->callid, callid) == 0)
			{
				riter->second.slist->erase(slter);
				__reglocker.unlock();
				return;
			}
			slter++;
		}
	}
	__reglocker.unlock();
}

void SipService::register_slist_erase_by_cid(const char * callid)
{
	__reglocker.lock();
	regmap_iter riter = __regmap.begin();
	while (riter != __regmap.end())
	{
		seslist_iter slter = riter->second.slist->begin();
		while (slter != riter->second.slist->end())
		{
			if (strcmp((*slter)->callid, callid) == 0)
			{
				//printf("删除会话%s:%s\n", riter->first.c_str(), callid);
				riter->second.slist->erase(slter);
				break;
			}
			slter++;
		}
		riter++;
	}
	__reglocker.unlock();
}

void SipService::register_slist_clear(const char *addr)
{
	regmap_iter riter = __regmap.find(addr);
	if (riter != __regmap.end())
	{
		seslist_iter slter = riter->second.slist->begin();
		while (slter != riter->second.slist->end())
		{
			send_bye_by_cid((*slter)->callid, true, true);
			slter++;
		}
	}
}

session_info * SipService::register_slist_find(const char * addr, const char * deviceid)
{
	regmap_iter riter = __regmap.find(addr);
	if (riter != __regmap.end())
	{
		seslist_iter slter = riter->second.slist->begin();
		while (slter != riter->second.slist->end())
		{
			if (strcmp((*slter)->deviceid, deviceid) == 0)
			{
				return (*slter);
			}
			slter++;
		}
	}
	return NULL;
}

register_info * SipService::register_get_mediaservice(const char * deviceid)
{
	register_info *pReg = NULL;
	size_t nUsers = 0;	//用户数
	regmap_iter riter = __regmap.begin();
	while (riter != __regmap.end())
	{
		if (riter->second.role == SIP_ROLE_MEDIA && riter->second.online)	//RTP流媒体
		{
			size_t nSize = riter->second.slist->size();	//转发用户数
			if (nUsers == 0)
				nUsers = nSize;
			if (nSize <= nUsers)
			{
				nUsers = nSize;
				pReg = &(riter->second);
			}
			seslist_iter slter = riter->second.slist->begin();
			while (slter != riter->second.slist->end())
			{
				if (strcmp((*slter)->deviceid, deviceid) == 0)
				{
					return &(riter->second);
				}
				slter++;
			}
		}
		riter++;
	}
	return pReg;
}

register_info * SipService::register_get_info(const char * addr)
{
	regmap_iter riter = __regmap.find(addr);
	if (riter != __regmap.end())
		return &(riter->second);
	return NULL;
}

bool SipService::is_superior_addr(const char * addr)
{
	regmap_iter riter = __regmap.find(addr);
	if (riter != __regmap.end())
	{
		if (riter->second.role == SIP_ROLE_SUPERIOR)
			return true;
	}
	return false;
}

bool SipService::is_public_addr(const char * addr)
{
	char szIp[24] = { 0 };
	strcpy(szIp, addr);
	char *p = strchr(szIp, '.');
	if (!p)
		return true;
	p += 1;
	p = strchr(p, '.');
	if (!p)
		return true;
	p[0] = '\0';
	size_t n = strlen(szIp);
	if (strncmp(addr, m_conf.local_addr, n) == 0)
		return false;
	return true;
}

bool SipService::is_client_addr(const char * addr)
{
	char szIP[24] = { 0 };
	strcpy(szIP, addr);
	char *p = strrchr(szIP, ':');
	if (!p)
		return false;
	p[0] = '\0';
	if (m_conf.client_addrs[0] != '\0')
	{
		if (strstr(m_conf.client_addrs, szIP))
			return true;
	}
	return false;
}

bool SipService::get_user_allow_all(const char * user)
{
	return true;
}

bool SipService::get_user_allow_catalog(const char * user, catalog_list & catlist)
{
	return true;
}

bool SipService::get_user_allow_device(const char * user, const char * deviceid)
{
	return true;
}

void SipService::session_manager()
{
	while (m_running)
	{
		sesmap_iter siter = __sesmap.begin();
		while (siter != __sesmap.end())
		{
			time_t now = time(NULL);
			if (siter->second->mediaover > 0 && now - siter->second->mediaover >= SIP_SESSIONOVER)
			{
				acl::string strAddr = siter->second->addr;
				acl::string strCallId = siter->second->callid;
				if (siter->second->flow < SESSION_SENDER_BYE)
				{
					send_bye_by_cid(siter->second->callid, siter->second->clsmedia);
				}
#ifdef _USRDLL
				sip_free_media_port(siter->second->receiver.recvport);
#endif
				session_info *pSess = siter->second;
				siter = __sesmap.erase(siter);
				register_slist_erase_by_cid(strCallId.c_str());
				delete pSess;
				pSess = NULL;
			}
			else
				siter++;	
		}
		sleep(1);
	}
}

void SipService::message_manager()
{
	while (m_running)
	{
		__msglocker.lock();
		sesmap_iter miter = __msgmap.begin();
		while (miter != __msgmap.end())
		{
			time_t now = time(NULL);
			if (miter->second->session_time > 0 && now - miter->second->session_time >= SIP_ALIVETIME)
			{
				session_info *pSess = miter->second;
				miter = __msgmap.erase(miter);
				if (pSess->userid[0] != '\0')
				{
					__subcatlocker.lock();
					sesmap_iter siter = __subcatmap.find(pSess->userid);
					if (siter != __subcatmap.end())
						__subcatmap.erase(siter);
					__subcatlocker.unlock();
				}
				delete pSess;
				pSess = NULL;
			}
			else
				miter++;
		}
		__msglocker.unlock();
		sleep(1);
	}
}

void SipService::subscribe_manager()
{
	while (m_running)
	{
		__subcatlocker.lock();
		sesmap_iter siter = __subcatmap.begin();
		while (siter != __subcatmap.end())
		{
			time_t now = time(NULL);
			if (siter->second->session_time > 0 && now - siter->second->mediaover >= siter->second->expires)
			{
				session_info *pSess = siter->second;
				siter = __subcatmap.erase(siter);
				__msglocker.lock();
				sesmap_iter miter = __msgmap.find(pSess->sn);
				if (miter != __msgmap.end())
					__msgmap.erase(miter);
				__msglocker.unlock();
				delete pSess;
				pSess = NULL;
			}
			else
				siter++;
		}
		__subcatlocker.unlock();
		__subalmlocker.lock();
		siter = __subalmmap.begin();
		while (siter != __subalmmap.end())
		{
			time_t now = time(NULL);
			if (siter->second->session_time > 0 && now - siter->second->mediaover >= siter->second->expires)
			{
				session_info *pSess = siter->second;
				siter = __subalmmap.erase(siter);
				__msglocker.lock();
				sesmap_iter miter = __msgmap.find(pSess->sn);
				if (miter != __msgmap.end())
					__msgmap.erase(miter);
				__msglocker.unlock();
				delete pSess;
				pSess = NULL;
			}
			else
				siter++;
		}
		__subalmlocker.unlock();
		sleep(1);
	}
}

void SipService::catalog_manager()
{
#ifdef _DBLIB_USE
	m_db.Open();
	while (m_running)
	{
		size_t nCount = __catalog.size();
		if (nCount == 0)
		{
			sleep(1);
			continue;
		}
		__catlocker.lock();
		CatalogInfo *pInfo = __catalog.front();
		__catalog.pop();
		__catlocker.unlock();

		int nRole = sip_get_user_role(pInfo->szDeviceID);
		if (pInfo->szCivilCode[0] == '\0' || strlen(pInfo->szCivilCode) > 10 || atoi(pInfo->szCivilCode) == 0)//纠正CiviCode
		{
			size_t nPos = 10;
			acl::string strCivilCode(pInfo->szDeviceID, nPos);
			while (strCivilCode.rncompare("00", 2) == 0)
			{
				nPos -= 2;
				strCivilCode = strCivilCode.left(nPos);
			}
			strcpy(pInfo->szCivilCode, strCivilCode.c_str());
		}
		if (strcmp(pInfo->szParentID, pInfo->szDeviceID) == 0)			//纠正ParentID
			memset(pInfo->szParentID, 0, sizeof(pInfo->szParentID));
		else if (strlen(pInfo->szParentID) > 20)
		{
			acl::string strParentID = pInfo->szParentID;
			strParentID = strParentID.right(20);
			strcpy(pInfo->szParentID, strParentID.c_str());
		}
		if (pInfo->szStatus[0] != '\0' && strstr(pInfo->szStatus, "ON"))
			strcpy(pInfo->szStatus, "ON");
		else
			strcpy(pInfo->szStatus, "OFF");
		m_db.SaveCatalog(pInfo, nRole);
		delete pInfo;
		pInfo = NULL;
	}
#endif
}


bool SipService::is_no_enter_ip(const char * ip)
{
	for (size_t i = 0; i < m_no_enter_list.size(); i++)
	{
		if (strncmp(m_no_enter_list[i].c_str(), ip, m_no_enter_list[i].size()) == 0)
			return true;
	}
	return false;
}

bool SipService::build_ssrc(char * ssrc, int slen, int type)
{
	if (sip_build_sdp_ssrc(ssrc, slen, SIP_INVITE_TYPE(type)))
		return true;
	return false;
}

bool SipService::convert_grouping(const char * deviceid, char * groupid)
{
	if (strlen(deviceid) != 20)
		return false;
	strncpy(groupid, deviceid, 10);
	strcat(groupid, "216");
	strcat(groupid, deviceid + 13);
	return true;
}

void SipService::client_locker(bool lock)
{
#ifdef _USRDLL
	if (lock)
		__seslocker.lock();
	else
		__seslocker.unlock();
#endif
}

void SipService::register_update(const char *deviceid, char * outMappingAddr)
{
	__reglocker.lock();
	regmap_iter riter = __regmap.begin();
	while (riter != __regmap.end())
	{
		if (strcmp(riter->second.user, deviceid) == 0)
		{
			if (riter->second.mapping_addr[0] != '\0')
			{
				strcpy(outMappingAddr, riter->second.mapping_addr);
			}
			
			seslist_iter slter = riter->second.slist->begin();
			while (slter != riter->second.slist->end())
			{
				send_bye_by_cid((*slter)->callid, true, true);
				slter++;
			}
			delete riter->second.slist;
			riter = __regmap.erase(riter);
			__reglocker.unlock();
			return;
		}
		riter++;
	}
	__reglocker.unlock();
}

bool SipService::send_msg(const char * addr, const char * msg, size_t mlen)
{
	//std::vector<acl::socket_stream *> streams = get_sstreams();
	//if (streams.size() == 0)
	//{
	//	m_conf.log(LOG_ERROR, "socket is null");
	//	return false;
	//}
	//streams[0]->set_peer(addr);
	//if (streams[0]->write(msg, mlen) == -1)
	//{
	//	return false;
	//}
	//return true;
	if (on_write(addr, msg, mlen))
		return true;
	else
		return false;
}

void SipService::send_register(const char * ip, int port, const char * user, const char * pwd, int expires)
{
	char szAddr[32] = { 0 };
	sprintf(szAddr, "%s:%d", ip, port);
	__reglocker.lock();
	regmap_iter riter = __regmap.find(szAddr);
	if (riter == __regmap.end())
	{
		register_info rinfo;
		memset(&rinfo, 0, sizeof(register_info));
		strcpy(rinfo.addr, szAddr);
		strcpy(rinfo.user, user);
		strcpy(rinfo.pwd, pwd);
		rinfo.role = SIP_ROLE_SUPERIOR;
		rinfo.expires = expires;
		rinfo.tranfer = m_conf.clnt_tranfer;
		rinfo.online = false;
		rinfo.vaild = true;
		rinfo.slist = new session_list;
		__regmap.insert(register_map::value_type(szAddr, rinfo));
	}
	__reglocker.unlock();
}

int SipService::send_invite(invite_info *info, media_stream_cb stream_cb, void *user)
{
	int nTranfer = 1;
	int nCseq = 20;
	for (size_t i = 0; i < 3; i++)	//尝试三次，等待注册成功。
	{
		regmap_iter riter = __regmap.find(info->addr);
		if (riter != __regmap.end())
			nTranfer = riter->second.tranfer;
		if (m_conf.srv_role != SIP_ROLE_CLIENT)
		{
			if (info->media_addr[0] != '\0')
				riter = __regmap.find(info->media_addr);
			else if (info->terminalid[0] != '\0')
			{
				riter = __regmap.find(info->terminal_addr);
				info->carrysdp = false;
			}
		}
		if (riter != __regmap.end())
		{
			if (riter->second.online)
			{
				char szBuf[SIP_HEADER_LEN] = { 0 };
				char szMsg[SIP_MSG_LEN] = { 0 };
				int nMsgLen = 0;
				char szUri[256] = { 0 };
				strcpy(szUri, info->deviceid);
				acl::string strTo;
				if (m_conf.srv_role == SIP_ROLE_CLIENT)
					strTo = sip_build_msg_to(szBuf, sizeof(szBuf), info->deviceid, riter->second.addr);
				else
					strTo = sip_build_msg_to(szBuf, sizeof(szBuf), riter->second.user, riter->second.addr);
	
				acl::string strSub;
				if (info->terminalid[0] != '\0' && m_conf.srv_role != SIP_ROLE_CLIENT)
					strSub = sip_build_msg_subject(szBuf, sizeof(szBuf), info->terminalid, info->ssrc);
				else
				{
					if (info->media_addr[0] != '\0')
					{
						acl::string strNetType;
						strNetType.format("%d", nTranfer);
						if (info->invite_uri[0] != '\0')
						{
							strcpy(szUri, info->invite_uri);
							if (info->transfrom)
								strNetType.format("%d", SIP_TRANSFER_SDK_STANDARDING);
							else
								strNetType.format("%d", SIP_TRANSFER_SDK);
						}
						strSub = sip_build_msg_subject(szBuf, sizeof(szBuf), info->deviceid, info->ssrc, strNetType.c_str());
					}
					else
						strSub = sip_build_msg_subject(szBuf, sizeof(szBuf), info->deviceid, info->ssrc, info->terminalid);
				}
				acl::string strCallId;
				if (info->callid != NULL && info->callid[0] != '\0')
					strCallId = info->callid;
				else
					strCallId = sip_build_msg_call_id(szBuf, sizeof(szBuf));
				acl::string strFtag;
				if (info->ftag != NULL && info->ftag[0] != '\0')
					strFtag = info->ftag;
				else
					strFtag = sip_build_randnum(szBuf, sizeof(szBuf));
				if (info->cseq > 0)
					nCseq = info->cseq;
				nMsgLen = sip_build_msg_invite(szMsg, sizeof(szMsg), strTo.c_str(), strSub.c_str(), nCseq, strCallId.c_str(), strFtag.c_str(), info->ttag);
				if (m_conf.srv_role == SIP_ROLE_CLIENT)
				{
					if (info->receiver.receiverid[0] == '\0')
					{
						strcpy(info->receiver.receiverid, m_conf.local_code);
						strcpy(info->receiver.recvip, m_conf.local_addr);
						info->receiver.recvprotocol = m_conf.clnt_tranfer;
						info->receiver.recvport = sip_get_media_port(m_conf.rtp_port);
					}
				}
				if (info->carrysdp)
				{
					char szBody[SIP_BODY_LEN] = { 0 };
					char szReceiverIp[16] = { 0 };
					if (info->media_addr[0] == '\0' && riter->second.mapping_addr[0] != '\0' && is_public_addr(riter->second.addr))
						strcpy(szReceiverIp, riter->second.mapping_addr);
					else
						strcpy(szReceiverIp, info->receiver.recvip);
					sip_build_msg_sdp(szBody, sizeof(szBody), info->receiver.receiverid, szUri, info->invite_time, szReceiverIp, info->receiver.recvport, info->invite_type,
						SIP_RECVONLY, info->receiver.recvprotocol, info->downloadspeed);
					nMsgLen = sip_set_sdp_y(szBody, sizeof(szBody), info->ssrc);
					nMsgLen = sip_set_body(szMsg, sizeof(szMsg), szBody, SIP_APPLICATION_SDP);
				}
				if (send_msg(riter->second.addr, szMsg, nMsgLen))
				{
					sesmap_iter siter = __sesmap.find(strCallId.c_str());
					if (siter == __sesmap.end())
					{
						session_info *pInfo = new session_info;
						memset(pInfo, 0, sizeof(session_info));
						pInfo->invite_type = info->invite_type;
						pInfo->cseq = nCseq;
						memcpy(&(pInfo->receiver), &(info->receiver),sizeof(recv_info));
						if (info->addr && info->addr[0] != '\0')
						{
							strcpy(pInfo->addr, info->addr);
							if (m_conf.srv_role == SIP_ROLE_CLIENT)
							{
								pInfo->clsmedia = true;
								pInfo->flow = SESSION_SENDER_REQ;
							}
							else
								pInfo->flow = SESSION_SMEDIA_REQ;
						}
						if (info->media_addr && info->media_addr[0] != '\0')
						{
							strcpy(pInfo->media_user, riter->second.user);
							strcpy(pInfo->media_addr, info->media_addr);
							pInfo->flow = SESSION_MEDIA_REQ;
						}
						if (info->terminalid && info->terminalid[0] != '\0')
						{
							strcpy(pInfo->terminalid, info->terminalid);
							if (pInfo->flow != SESSION_MEDIA_REQ && m_conf.srv_role != SIP_ROLE_CLIENT)
							{
								strcpy(pInfo->terminal_addr, info->terminal_addr);
								pInfo->flow = SESSION_RECVER_REQ;
							}
						}
						if (info->user_addr && info->user_addr[0] != '\0')
							strcpy(pInfo->user_addr, info->user_addr);
						if (info->user_via && info->user_via[0] != '\0')
							strcpy(pInfo->user_via, info->user_via);
						if (info->user_from && info->user_from[0] != '\0')
							strcpy(pInfo->user_from, info->user_from);
						if (info->user_to && info->user_to[0] != '\0')
							strcpy(pInfo->user_to, info->user_to);
						strcpy(pInfo->callid, strCallId.c_str());
						strcpy(pInfo->ftag, strFtag.c_str());
						if (info->ttag && info->ttag[0] != '\0')
							strcpy(pInfo->ttag, info->ttag);
						if (info->invite_uri[0] != '\0')
							pInfo->transfrom_sdk = true;
						strcpy(pInfo->deviceid, info->deviceid);
						strcpy(pInfo->invite_time, info->invite_time);
						if (info->ssrc && info->ssrc[0] != '\0')
							strcpy(pInfo->ssrc, info->ssrc);
						if (stream_cb)
							pInfo->stream_cb = stream_cb;
						if (user)
							pInfo->stream_user = user;
						pInfo->session_time = time(NULL);
						__sesmap[strCallId.c_str()] = pInfo;
						riter->second.slist->push_back(pInfo);
						//printf("加入会话%s:%s\n", riter->first.c_str(), pInfo->callid);
						if (info->user_addr && info->user_addr[0] != '\0')
						{
							regmap_iter uiter = __regmap.find(info->user_addr);
							if (uiter != __regmap.end())
							{
								uiter->second.slist->push_back(pInfo);
								//printf("客户端加入会话%s:%s\n", info->user_addr, pInfo->callid);
							}
						}
					}
					m_conf.log(LOG_INFO, "点播镜头[%s][%s]\n", info->deviceid, strCallId.c_str());
					return atoi(strCallId.c_str());
				}
			}
		}
		sleep(1);
	}
	m_conf.log(LOG_WARN, "注册列表找不到对应!\n");
	return 0;
}

void SipService::send_bye(int invite_cid, bool clsrtp, bool force)
{
	char szBuf[SIP_HEADER_LEN] = { 0 };
	acl::string strCallId = sip_build_msg_call_id(szBuf, sizeof(szBuf), invite_cid);
	sesmap_iter siter = __sesmap.find(strCallId.c_str());
	if (siter != __sesmap.end())
	{
		if (siter->second->flow < SESSION_SENDER_BYE)
		{
			m_conf.log(LOG_INFO, "发送关闭请求成功![%s]\n", strCallId.c_str());
			send_bye_by_cid(strCallId.c_str(), clsrtp, force);
		}
	}
}

void SipService::send_bye_by_cid(const char * cid, bool clsrtp, bool force)
{
	client_locker(true);
	sesmap_iter siter = __sesmap.find(cid);
	if (siter == __sesmap.end())
	{
		client_locker(false);
		return;
	}
	session_info *pInfo = siter->second;
#ifdef _USRDLL
	if (clsrtp && pInfo->rtp_stream)
		sip_stop_stream(pInfo->rtp_stream);
	pInfo->stream_cb = NULL;
	pInfo->stream_user = NULL;
	if (pInfo->rtp_stream)
		sip_set_stream_cb((RTP_STREAM)pInfo->rtp_stream, NULL, NULL);
#endif
	char szMsg[SIP_MSG_LEN] = { 0 };
	int nMsgLen = 0;
	char szTo[SIP_HEADER_LEN] = { 0 };
	if (force && pInfo->user_addr[0] != '\0')
	{
		sip_build_msg_to(szTo, sizeof(szTo), pInfo->deviceid, pInfo->user_addr);
		nMsgLen = sip_build_msg_bye(szMsg, sizeof(szMsg), szTo, siter->second->cseq + 1, cid, siter->second->ftag, siter->second->ttag);
		send_msg(pInfo->user_addr, szMsg, nMsgLen);
	}
	if (pInfo->terminal_addr[0] != '\0' && m_conf.srv_role != SIP_ROLE_CLIENT)
	{
		sip_build_msg_to(szTo, sizeof(szTo), pInfo->terminalid, pInfo->terminal_addr);
		nMsgLen = sip_build_msg_bye(szMsg, sizeof(szMsg), szTo, siter->second->cseq + 1, cid, siter->second->ftag, siter->second->terminal_ttag);
		send_msg(pInfo->terminal_addr, szMsg, nMsgLen);
	}
	if (pInfo->media_addr[0] != '\0')
	{
		sip_build_msg_to(szTo, sizeof(szTo), pInfo->deviceid, pInfo->media_addr);
		nMsgLen = sip_build_msg_bye(szMsg, sizeof(szMsg), szTo, siter->second->cseq + 1, cid, siter->second->ftag, siter->second->ttag);
		send_msg(pInfo->media_addr, szMsg, nMsgLen);
		if (pInfo->invite_type == SIP_INVITE_PLAY)
		{
			client_locker(false);
			return;
		}
	}
	sip_build_msg_to(szTo, sizeof(szTo), pInfo->deviceid, pInfo->addr);
	nMsgLen = sip_build_msg_bye(szMsg, sizeof(szMsg), szTo, siter->second->cseq + 1, cid, siter->second->ftag, siter->second->ttag);
	send_msg(pInfo->addr, szMsg, nMsgLen);
	pInfo->flow = SESSION_SENDER_BYE;
	pInfo->mediaover = time(NULL);
	client_locker(false);
}

void SipService::send_ptzcmd(const char * addr, char * deviceid, unsigned char cmd, unsigned char param1, unsigned short param2, SIP_PTZ_RECT *param)
{
	int nSwitch = 0;
	char szPtzcmd[17] = { 0 };
	unsigned char szBit[8] = { 0 };
	szBit[0] = 0xA5;
	szBit[1] = 0x0F;
	szBit[2] = 0x01;
	szBit[3] = cmd;
	switch (cmd)
	{
	case SIP_PTZCMD_STOP:
		break;
	case SIP_PTZCMD_UP:
	case SIP_PTZCMD_DOWN:
		szBit[5] = param1 * 32 - 1;
		break;
	case SIP_PTZCMD_LEFT:
	case SIP_PTZCMD_RIGHT:
		szBit[4] = param1 * 32 - 1;
		break;
	case SIP_PTZCMD_LU:
	case SIP_PTZCMD_LD:;
	case SIP_PTZCMD_RU:
	case SIP_PTZCMD_RD:
		szBit[4] = param1 * 32 - 1;
		szBit[5] = param1 * 32 - 1;
		break;
	case SIP_PTZCMD_ZOOM_IN:
	case SIP_PTZCMD_ZOOM_OUT:
		szBit[6] = param1 * 32 - 1;
		break;
	case SIP_PTZCMD_FI_IRIS_IN:
	case SIP_PTZCMD_FI_IRIS_OUT:
		szBit[5] = param1 * 32 - 1;
		break;
	case SIP_PTZCMD_FI_FOCUS_IN:
	case SIP_PTZCMD_FI_FOCUS_OUT:
		szBit[4] = param1 * 32 - 1;
		break;
	case SIP_PTZCMD_SET:
	case SIP_PTZCMD_GOTO:
	case SIP_PTZCMD_DELETE:
		szBit[5] = param1;
		break;
	case SIP_PTZCMD_CRUISE_ADD:
	case SIP_PTZCMD_CRUISE_DEL:
		szBit[5] = param1;
		szBit[6] = (unsigned char)param2;
		break;
	case SIP_PTZCMD_CRUISE_SPEED:
	case SIP_PTZCMD_CRUISE_DTIME:
		szBit[5] = param1;
		szBit[6] = (unsigned char)param2;
		szBit[7] = param2 >> 12;
		break;
	case SIP_PTZCMD_CRUISE_RUN:
		szBit[5] = param1;
		break;
	case SIP_PTZCMD_AUTO_RUN:
		szBit[5] = param1;
		szBit[6] = (unsigned char)param2;	//0 开始 1 往左 2 往右
		break;
	case SIP_PTZCMD_AUTO_SPEED:
		szBit[5] = param1;
		szBit[6] = (unsigned char)param2;
		szBit[7] = param2 >> 12;
		break;
	case SIP_PTZCMD_AUXIOPEN:
	case SIP_PTZCMD_AUXICLOSE:
		szBit[5] = 0x01;
		break;
	case SIP_PTZCMD_LOCK:
	case SIP_PTZCMD_UNLOCK:
		break;
	case SIP_PTZCMD_DRAGZOOM_IN:
		strcpy(szPtzcmd, "DragZoomIn");
		break;
	case SIP_PTZCMD_DRAGZOOM_OUT:
		strcpy(szPtzcmd, "DragZoomOut");
		break;
	case SIP_PTZCMD_INITIAL:
		nSwitch = 1;
		break;
	case SIP_PTZCMD_UNINITIAL:
		nSwitch = 0;
		break;
	default:
		return;
	}
	char szBuf[SIP_HEADER_LEN] = { 0 };
	char szMsg[SIP_MSG_LEN] = { 0 };
	acl::string strTo = sip_build_msg_to(szBuf, sizeof(szBuf), deviceid, addr);
	int nMsgLen = sip_build_msg_request(szMsg, sizeof(szMsg), "MESSAGE", NULL, NULL, 20, NULL, strTo.c_str(), NULL, NULL);
	acl::string strBody;
	if (cmd == SIP_PTZCMD_DRAGZOOM_IN || cmd == SIP_PTZCMD_DRAGZOOM_OUT)
	{
		strBody.format(
			"<?xml version=\"1.0\"?>%s"
			"<Control>%s"
			"<CmdType>DeviceControl</CmdType>%s"
			"<SN>%d</SN>%s"
			"<DeviceID>%s</DeviceID>%s"
			"<%s>%s"
			"<Length>%d</Length>%s"
			"<Width>%d</Width>%s"
			"<MidPointX>%d</MidPointX>%s"
			"<MidPointY>%d</MidPointY>%s"
			"<LengthX>%d</LengthX>%s"
			"<LengthY>%d</LengthY>%s"
			"</%s>%s"
			"</Control>%s",
			Line, Line, Line, sip_build_randsn(), Line, deviceid, Line, szPtzcmd, Line, param->height, Line, param->width, Line, 
			param->left, Line, param->top, Line, param->right, Line, param->bottom, Line, szPtzcmd, Line, Line);
	}
	else if (cmd == SIP_PTZCMD_INITIAL || cmd == SIP_PTZCMD_UNINITIAL)
	{
		strBody.format(
			"<?xml version=\"1.0\"?>%s"
			"<Control>%s"
			"<CmdType>DeviceControl</CmdType>%s"
			"<SN>%d</SN>%s"
			"<DeviceID>%s</DeviceID>%s"
			"<HomePosition>%s"
			"<Enable>%d</Enable>%s"
			"<ResetTime>%d</ResetTime>%s"
			"<PresetIndex>%d</PresetIndex>%s"
			"</HomePosition>%s"
			"</Control>%s",
			Line, Line, Line, sip_build_randsn(), Line, deviceid, Line, Line, nSwitch, Line,
			param2, Line, param1, Line, Line, Line);
	}
	else
	{
		szBit[7] = (szBit[0] + szBit[1] + szBit[2] + szBit[3] + szBit[4] + szBit[5] + szBit[6]) % 0x100;
		sprintf(szPtzcmd, "A50F%02X%02X%02X%02X%02X%02X", szBit[2], szBit[3], szBit[4], szBit[5], szBit[6], szBit[7]);
		strBody.format(
			"<?xml version=\"1.0\"?>%s"
			"<Control>%s"
			"<CmdType>DeviceControl</CmdType>%s"
			"<SN>%d</SN>%s"
			"<DeviceID>%s</DeviceID>%s"
			"<PTZCmd>%s</PTZCmd>%s"
			"<Info>%s"
			"<ControlPriority>5</ControlPriority>%s"
			"</Info>%s"
			"</Control>%s",
			Line, Line, Line, sip_build_randsn(), Line, deviceid, Line, szPtzcmd, Line, Line, Line, Line, Line);
	}
	nMsgLen = sip_set_body(szMsg, sizeof(szMsg), strBody.c_str(), SIP_APPLICATION_XML);
	send_msg(addr, szMsg, nMsgLen);
}

int SipService::send_recordinfo(const char * addr, char * deviceid, int rectype, char * starttime, char * endtime, record_info *recordinfo, int recordsize)
{
	memset(recordinfo, 0, sizeof(recordinfo)*recordsize);
	acl::string strRecId = deviceid;
	regmap_iter riter = __regmap.find(addr);
	if (riter != __regmap.end())
	{
		strRecId = riter->second.user;
	}
	acl::string strRecType = "all";
	switch (rectype)
	{
	case SIP_RECORDTYPE_TIME:
		strRecType = "time";
		break;
	case SIP_RECORDTYPE_MOVE:
		strRecType = "move";
		break;
	case SIP_RECORDTYPE_ALARM:
		strRecType = "alarm";
		break;
	case SIP_RECORDTYPE_MANUAL:
		strRecType = "manual";
		break;
	default:
		break;
	}
	char szBuf[SIP_HEADER_LEN] = { 0 };
	char szMsg[SIP_MSG_LEN] = { 0 };
	acl::string strSn;
	strSn.format("%d", sip_build_randsn());
	acl::string strStartTime = sip_get_formattime(szBuf, sizeof(szBuf), starttime);
	acl::string strEndTime = sip_get_formattime(szBuf, sizeof(szBuf), endtime);
	acl::string strTo = sip_build_msg_to(szBuf, sizeof(szBuf), deviceid, addr);
	acl::string strCallId = sip_build_msg_call_id(szBuf, sizeof(szBuf));
	int nMsgLen = sip_build_msg_request(szMsg, sizeof(szMsg), "MESSAGE", NULL, strCallId.c_str(), 20, NULL, strTo.c_str(), NULL, NULL);
	acl::string strBody;
	strBody.format(
		"<?xml version=\"1.0\"?>%s"
		"<Query>%s"
		"<CmdType>RecordInfo</CmdType>%s"
		"<SN>%s</SN>%s"
		"<DeviceID>%s</DeviceID>%s"
		"<StartTime>%s</StartTime>%s"
		"<EndTime>%s</EndTime>%s"
		"<Secrecy>0</Secrecy>%s"
		"<Type>%s</Type>%s"
		"<RecorderID>%s</RecorderID>%s"
		"</Query>%s",
		Line, Line, Line, strSn.c_str(), Line, deviceid, Line,
		strStartTime.c_str(), Line, strEndTime.c_str(), Line, Line, strRecType.c_str(), Line,
		strRecId.c_str(), Line, Line);
	nMsgLen = sip_set_body(szMsg, sizeof(szMsg), strBody.c_str(), SIP_APPLICATION_XML);
	if (send_msg(addr, szMsg, nMsgLen))
	{
		session_info *pInfo = new session_info;
		memset(pInfo, 0, sizeof(session_info));
		pInfo->rec_info = recordinfo;
		pInfo->rec_count = 0;
		pInfo->rec_finish = false;
		__msgmap[strSn.c_str()] = pInfo;
		int nWaitTime = 0;
		while (nWaitTime < 6)
		{
			if (pInfo->rec_finish)
				break;
			nWaitTime++;
			sleep(1);
		}
		sesmap_iter siter = __msgmap.find(strSn.c_str());
		if (siter == __msgmap.end())
			return 0;
		int nRecordCount = siter->second->rec_count;
		delete siter->second;
		__msglocker.lock();
		__msgmap.erase(siter);
		__msglocker.unlock();
		return nRecordCount;
	}
	return 0;
}

bool SipService::send_info(int invite_cid, unsigned char cmd, unsigned char param)
{
	char szCallID[SIP_HEADER_LEN] = { 0 };
	sip_build_msg_call_id(szCallID, sizeof(szCallID), invite_cid);
	sesmap_iter siter = __sesmap.find(szCallID);
	if (siter != __sesmap.end())
	{
		if (send_info_by_session(siter->second, cmd, param))
			return true;
	}
	return false;
}

int SipService::send_info_by_session(session_info *pSession, unsigned char cmd, unsigned char param)
{
	char szTo[SIP_HEADER_LEN] = { 0 };
	sip_build_msg_to(szTo, sizeof(szTo), pSession->deviceid, pSession->addr);
	char szInfo[SIP_MSG_LEN] = { 0 };
	sip_build_msg_request(szInfo, sizeof(szInfo), "INFO", NULL, pSession->callid, pSession->cseq + 1, NULL, szTo, NULL, NULL);
	char szBody[SIP_BODY_LEN] = { 0 };
	switch (cmd)
	{
	case SIP_PLAYBACK_TEARDOWN:
		{
			sprintf(szBody,
				"TEARDOWN RTSP/1.0%s"
				"CSeq: %d%s",
				Line, ++(pSession->info_cseq), Line);
		}
		break;
	case SIP_PLAYBACK_PAUSE:
		{
			sprintf(szBody,
				"PAUSE RTSP/1.0%s"
				"CSeq: %d%s"
				"PauseTime: now%s",
				Line, ++(pSession->info_cseq), Line, Line);
		}
		break;
	case SIP_PLAYBACK_RESUME:
		{
			sprintf(szBody,
				"PLAY RTSP/1.0%s"
				"CSeq: %d%s"
				"Range: npt=now-%s",
				Line, ++(pSession->info_cseq), Line, Line);
		}
		break;
	case SIP_PLAYBACE_FAST:
		{
			pSession->info_scale *= 2.0;
			if (pSession->info_scale == 0)
				pSession->info_scale = 2.0;
			else if (pSession->info_scale > 8.0)
				return 0;
			char szScale[SIP_RAND_LEN] = { 0 };
			sprintf(szBody,
				"PLAY RTSP/1.0%s"
				"CSeq: %d%s"
				"Scale: %s%s",
				Line, ++(pSession->info_cseq), Line, sip_build_scale(szScale,sizeof(szScale), pSession->info_scale), Line);
		}
		break;
	case SIP_PLAYBACK_SLOW:
		{
			pSession->info_scale *= 0.5;
			if (pSession->info_scale == 0)
				pSession->info_scale = 0.5;
			else if (pSession->info_scale < 0.125)
				return 0;
			char szScale[SIP_RAND_LEN] = { 0 };
			sprintf(szBody,
				"PLAY RTSP/1.0%s"
				"CSeq: %d%s"
				"Scale: %s%s",
				Line, ++(pSession->info_cseq), Line, sip_build_scale(szScale, sizeof(szScale), pSession->info_scale), Line);
		}
		break;
	case SIP_PLAYBACK_NORMAL:
		{
			pSession->info_scale = 1.0;
			sprintf(szBody,
				"PLAY RTSP/1.0%s"
				"CSeq: %d%s"
				"Scale: 1.0%s",
				Line, ++(pSession->info_cseq), Line, Line);
		}
		break;
	case SIP_PLAYBACK_SETTIME:
		{
			sprintf(szBody,
				"PLAY RTSP/1.0%s"
				"CSeq: %d%s"
				"Range: npt=%d-%s",
				Line, ++(pSession->info_cseq), Line, param, Line);
		}
		break;
	default:
		return 0;
	}
	int nInfoLen = sip_set_body(szInfo, sizeof(szInfo), szBody, SIP_APPLICATION_RTSP);
	if (nInfoLen > 0)
	{
		send_msg(pSession->addr, szInfo, nInfoLen);
	}
	return 0;
}

int SipService::send_subscribe(const char *addr, char *deviceid, int sn, int event_type, int aram_type, int expires)
{
	char szTo[SIP_HEADER_LEN] = { 0 };
	sip_build_msg_to(szTo, sizeof(szTo), addr, deviceid);

	char szMsg[SIP_MSG_LEN] = { 0 };
	int nMsgLen = sip_build_msg_request(szMsg, sizeof(szMsg), "SUBSCRIBE", NULL, NULL, 20, NULL, szTo, NULL, NULL);
	char szContact[SIP_HEADER_LEN] = { 0 };
	sprintf(szContact, "<%s:%d>", m_conf.local_addr, m_conf.local_port);
	nMsgLen = sip_set_header(szMsg, sizeof(szMsg), SIP_H_CONTACT, szContact);
	char szEvent[SIP_HEADER_LEN] = { 0 };
	if (event_type == 0)
		sprintf(szEvent, "Catalog;id=%d", sn);
	else
		sprintf(szEvent, "presence");
	nMsgLen = sip_set_header(szMsg, sizeof(szMsg), SIP_H_EVENT, szEvent);
	char szExpires[10] = { 0 };
	sprintf(szExpires, "%d", expires);
	nMsgLen = sip_set_header(szMsg, sizeof(szMsg), SIP_H_EXPIRES, szExpires);
	char szStartTime[24];
	char szEndTime[24];
	sip_get_localtime(szStartTime, sizeof(szStartTime));
	sip_get_localtime(szEndTime, sizeof(szEndTime), expires);
	char szBody[SIP_BODY_LEN] = { 0 };
	if (event_type == 0)
	{
		sprintf(szBody,
			"<?xml version=\"1.0\"?>%s"
			"<Query>%s"
			"<CmdType>Catalog</CmdType>%s"
			"<SN>%d</SN>%s"
			"<DeviceID>%s</DeviceID>%s"
			"</Query>%s",
			Line, Line, Line, sn, Line, deviceid, Line, Line);
	}
	else
	{
		sprintf(szBody,
			"<?xml version=\"1.0\"?>%s"
			"<Query>%s"
			"<CmdType>Alarm</CmdType>%s"
			"<SN>%d</SN>%s"
			"<DeviceID>%s</DeviceID>%s"
			"<StartAlarmPriority>1</StartAlarmPriority>%s"
			"<EndAlarmPriority>4</EndAlarmPriority>%s"
			"<AlarmMethod>%d</AlarmMethod>%s"
			"<StartTime>%s</StartTime>%s"
			"<EndTime>%s</EndTime>%s"
			"</Query>%s",
			Line, Line, Line, sn, Line, deviceid, Line, Line, Line, aram_type, Line, szStartTime, Line, szEndTime, Line, Line);
	}
	nMsgLen = sip_set_body(szMsg, sizeof(szMsg), szBody, SIP_APPLICATION_XML);
	return send_msg(addr, szMsg, nMsgLen);
}

int SipService::send_message_catalog(const char *addr, char *deviceid)
{
	char szTo[SIP_HEADER_LEN] = { 0 };
	sip_build_msg_to(szTo, sizeof(szTo), addr, deviceid);

	char szMsg[SIP_MSG_LEN] = { 0 };
	int nMsgLen = sip_build_msg_request(szMsg, sizeof(szMsg), "MESSAGE", NULL, NULL, 20, NULL, szTo, NULL, NULL);
	char szBody[SIP_BODY_LEN] = { 0 };
	sprintf(szBody,
		"<?xml version=\"1.0\"?>%s"
		"<Query>%s"
		"<CmdType>Catalog</CmdType>%s"
		"<SN>%d</SN>%s"
		"<DeviceID>%s</DeviceID>%s"
		"</Query>%s",
		Line, Line, Line, sip_build_randsn(), Line, deviceid, Line, Line);
	nMsgLen = sip_set_body(szMsg, sizeof(szMsg), szBody, SIP_APPLICATION_XML);
	return send_msg(addr, szMsg, nMsgLen);
}

int SipService::response_catalog_root(session_info * pSession, int sumnum, int type)
{
	acl::string strMethod = "MESSAGE";
	if (type == 1)
		strMethod = "NOTIFY";
	size_t nPos = 10;
	acl::string sCivilCode = m_conf.local_code;
	sCivilCode = sCivilCode.left(nPos);
	while (sCivilCode.rncompare("00", 2) == 0)
	{
		nPos -= 2;
		sCivilCode = sCivilCode.left(nPos);
	}
	char szGroupID[21] = { 0 };
	if (!convert_grouping(m_conf.local_code, szGroupID))
		strcpy(szGroupID, m_conf.local_code);
	acl::string strAddr;
	strAddr.format("sip:%s@%s:%d", pSession->deviceid, m_conf.local_addr, m_conf.local_port);
	char szFrom[SIP_HEADER_LEN] = { 0 };
	sprintf(szFrom, "sip:%s@%s:%d", pSession->deviceid, m_conf.local_addr, m_conf.local_port);
	char szMsg[SIP_MSG_LEN] = { 0 };
	int nMsgLen = sip_build_msg_request(szMsg, sizeof(szMsg), strMethod.c_str(), strAddr.c_str(), NULL, pSession->cseq + 1, szFrom, pSession->user_from, pSession->ftag, pSession->ttag);
	if (type == 1)
	{
		char szSubState[SIP_HEADER_LEN] = { 0 };
		sprintf(szSubState, "active;expires=%d;retry-after=0", pSession->expires);
		nMsgLen = sip_set_header(szMsg, sizeof(szMsg), SIP_H_SUBCRIPTION_STATE, szSubState);
		nMsgLen = sip_set_header(szMsg, sizeof(szMsg), SIP_H_EVENT, "presence");
	}
	char szBody[SIP_BODY_LEN] = { 0 };
	sprintf(szBody,
		"<?xml version=\"1.0\"?>%s"
		"<Response>%s"
		"<CmdType>Catalog</CmdType>%s"
		"<SN>%s</SN>%s"
		"<DeviceID>%s</DeviceID>%s"
		"<SumNum>%d</SumNum>%s"
		"<DeviceList Num=\"1\">%s"
		"<Item>%s"
		"<DeviceID>%s</DeviceID>%s"
		"<Name>%s</Name>%s"
		"<Manufacturer>XINYI</Manufacturer>%s"
		"<Model>XINYI</Model>%s"
		"<Owner>XINYI</Owner>%s"
		"<CivilCode>%s</CivilCode>%s"
		"<ParentID>%s</ParentID>%s"
		"<SafetyWay>0</SafetyWay>%s"
		"<RegisterWay>1</RegisterWay>%s"
		"<Secrecy>0</Secrecy>%s"
		"<IPAddress>%s</IPAddress>%s"
		"<Port>%d</Port>%s"
		"</Item>%s"
		"</DeviceList>%s"
		"</Response>%s",
		Line, Line, Line, pSession->sn, Line, pSession->deviceid, Line, sumnum, Line, Line, Line, szGroupID, Line, m_conf.srv_name, Line, Line, Line, Line,
		sCivilCode.c_str(), Line, pSession->deviceid, Line, Line, Line, Line, m_conf.local_addr, Line, m_conf.local_port, Line, Line, Line, Line);
	nMsgLen = sip_set_body(szMsg, sizeof(szMsg), szBody, SIP_APPLICATION_XML);
	send_msg(pSession->user_addr, szMsg, nMsgLen);
	pSession->rec_count++;
	pSession->session_time = time(NULL);
	return 0;
}

int SipService::response_catalog_node(session_info * pSession, CatalogInfo * pInfo, int sumnum, int type)
{
	acl::string strMethod = "MESSAGE";
	if (type == 1)
		strMethod = "NOTIFY";
	acl::string strAddr;
	strAddr.format("sip:%s@%s:%d", pSession->deviceid, m_conf.local_addr, m_conf.local_port);
	char szFrom[SIP_HEADER_LEN] = { 0 };
	sprintf(szFrom, "sip:%s@%s:%d", pSession->deviceid, m_conf.local_addr, m_conf.local_port);
	char szMsg[SIP_MSG_LEN] = { 0 };
	int nMsgLen = sip_build_msg_request(szMsg, sizeof(szMsg), strMethod.c_str(), strAddr.c_str(), NULL, pSession->cseq + 1, szFrom, pSession->user_from, pSession->ftag, pSession->ttag);
	if (type == 1)
	{
		char szSubState[SIP_HEADER_LEN] = { 0 };
		sprintf(szSubState, "active;expires=%d;retry-after=0", pSession->expires);
		nMsgLen = sip_set_header(szMsg, sizeof(szMsg), SIP_H_SUBCRIPTION_STATE, szSubState);
		nMsgLen = sip_set_header(szMsg, sizeof(szMsg), SIP_H_EVENT, "presence");
	}
	
	char szParentID[21] = { 0 };
	if (pInfo->szParentID && pInfo->szParentID[0] != '\0')
		strcpy(szParentID, pInfo->szParentID);
	else
		convert_grouping(m_conf.local_code, szParentID);
	char szBody[SIP_BODY_LEN] = { 0 };
	if (sip_get_user_role(pInfo->szDeviceID) != SIP_ROLE_CHANNEL)
	{
		sprintf(szBody,
			"<?xml version=\"1.0\"?>%s"
			"<Response>%s"
			"<CmdType>Catalog</CmdType>%s"
			"<SN>%s</SN>%s"
			"<DeviceID>%s</DeviceID>%s"
			"<SumNum>%d</SumNum>%s"
			"<DeviceList Num=\"1\">%s"
			"<Item>%s"
			"<DeviceID>%s</DeviceID>%s"
			"<Name>%s</Name>%s"
			"<Manufacturer>XINYI</Manufacturer>%s"
			"<Model>XINYI</Model>%s"
			"<Owner>XINYI</Owner>%s"
			"<CivilCode>%s</CivilCode>%s"
			"<Block>%s</Block>%s"
			"<ParentID>%s</ParentID>%s"
			"<SafetyWay>0</SafetyWay>%s"
			"<RegisterWay>1</RegisterWay>%s"
			"<Secrecy>0</Secrecy>%s"
			"<IPAddress>%s</IPAddress>%s"
			"<Port>%d</Port>%s"
			"</Item>%s"
			"</DeviceList>%s"
			"</Response>%s",
			Line, Line, Line, pSession->sn, Line, pSession->deviceid, Line, sumnum, Line, Line, Line, pInfo->szDeviceID, Line, acl_u2a(pInfo->szName).c_str(), Line, Line, Line, Line,
			pInfo->szCivilCode, Line, pInfo->szBlock, Line, szParentID, Line, Line, Line, Line, m_conf.local_addr, Line, m_conf.local_port, Line, Line, Line, Line);
		
	}
	else
	{
		sprintf(szBody,
			"<?xml version=\"1.0\"?>%s"
			"<Response>%s"
			"<CmdType>Catalog</CmdType>%s"
			"<SN>%s</SN>%s"
			"<DeviceID>%s</DeviceID>%s"
			"<SumNum>%d</SumNum>%s"
			"<DeviceList Num=\"1\">%s"
			"<Item>%s"
			"<DeviceID>%s</DeviceID>%s"
			"<Name>%s</Name>%s"
			"<Manufacturer>XINYI</Manufacturer>%s"
			"<Model>XINYI</Model>%s"
			"<Owner>XINYI</Owner>%s"
			"<CivilCode>%s</CivilCode>%s"
			"<Block>%s</Block>%s"
			"<Address>%s</Address>%s"
			"<Parental>0</Parental>%s"
			"<ParentID>%s</ParentID>%s"
			"<SafetyWay>0</SafetyWay>%s"
			"<RegisterWay>1</RegisterWay>%s"
			"<Secrecy>0</Secrecy>%s"
			"<IPAddress>%s</IPAddress>%s"
			"<Port>%d</Port>%s"
			"<Password></Password>%s"
			"<Status>%s</Status>%s"
			"<Longitude>%s</Longitude>%s"
			"<Latitude>%s</Latitude>%s"
			"<Info>%s"
			"<PTZType>%d</PTZType>%s"
			"</Info>%s"
			"</Item>%s"
			"</DeviceList>%s"
			"</Response>%s",
			Line, Line, Line, pSession->sn, Line, pSession->deviceid, Line, sumnum, Line, Line, Line, pInfo->szDeviceID, Line, acl_u2a(pInfo->szName).c_str(), Line, Line, Line, Line,
			pInfo->szCivilCode, Line, pInfo->szBlock, Line, pInfo->szAddress, Line, Line, szParentID, Line, Line, Line, Line, m_conf.local_addr, Line, m_conf.local_port, Line,
			Line, pInfo->szStatus, Line, pInfo->szLongitude, Line, pInfo->szLatitude, Line, Line, pInfo->nPTZType, Line, Line, Line, Line, Line);
	}
	nMsgLen = sip_set_body(szMsg, sizeof(szMsg), szBody, SIP_APPLICATION_XML);
	send_msg(pSession->user_addr, szMsg, nMsgLen);
	pSession->rec_count++;
	pSession->session_time = time(NULL);
	if (pSession->rec_count >= sumnum && type==0)
	{
		__msglocker.lock();
		sesmap_iter miter = __msgmap.find(pSession->sn);
		if (miter != __msgmap.end())
			miter = __msgmap.erase(miter);
		__msglocker.unlock();
		__subcatlocker.lock();
		miter = __subcatmap.find(pSession->userid);
		if (miter != __subcatmap.end())
			miter = __subcatmap.erase(miter);
		__subcatlocker.unlock();
		delete pSession;
		pSession = NULL;
	}
	return 0;
}

void SipService::send_position(const char * addr, char * deviceid)
{
	char szBuf[SIP_HEADER_LEN] = { 0 };
	char szMsg[SIP_MSG_LEN] = { 0 };
	acl::string strSn;
	strSn.format("%d", sip_build_randsn());
	acl::string strTo = sip_build_msg_to(szBuf, sizeof(szBuf), deviceid, addr);
	acl::string strCallId = sip_build_msg_call_id(szBuf, sizeof(szBuf));
	int nMsgLen = sip_build_msg_request(szMsg, sizeof(szMsg), "MESSAGE", NULL, strCallId.c_str(), 20, NULL, strTo.c_str(), NULL, NULL);
	acl::string strBody;
	strBody.format(
		"<?xml version=\"1.0\"?>%s"
		"<Query>%s"
		"<CmdType>MobilePosition</CmdType>%s"
		"<SN>%s</SN>%s"
		"<DeviceID>%s</DeviceID>%s"
		"<Interval>5</Interval>%s"
		"</Query>%s",
		Line, Line, Line, strSn.c_str(), Line, deviceid, Line, Line, Line, Line);
	nMsgLen = sip_set_body(szMsg, sizeof(szMsg), strBody.c_str(), SIP_APPLICATION_XML);
	send_msg(addr, szMsg, nMsgLen);
}


