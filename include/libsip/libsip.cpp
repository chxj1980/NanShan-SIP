#include "libsip.h"

//#include "acl_cpp/lib_acl.hpp"
//#include "lib_acl.h"


static char sipuser[21];
static char sipaddr[17];
static int	sipport;


bool msg_is_request(const char *msg, const char *head)
{
	if (strncasecmp(msg, head, strlen(head)) == 0)
		return true;
	return false;
}
int msg_is_response(const char *msg)
{
	const char *head = "SIP/2.0 ";
	size_t hlen = strlen(head);
	acl::string strMsg = msg;
	if (strMsg.ncompare(head, hlen) == 0)
	{
		acl::string strStatus;
		if (strMsg.substr(strStatus, hlen, 3) > 0)
			return atoi(strStatus.c_str());
	}
	return 0;
}
bool msg_is_status_1xx(const char *msg)
{
	int status = msg_is_response(msg);
	if (status >= 100 && status < 200)
		return true;
	return false;
}
bool msg_is_status_2xx(const char *msg)
{
	int status = msg_is_response(msg);
	if (status >= 200 && status < 300)
		return true;
	return false;
}
bool msg_is_status_3xx(const char *msg)
{
	int status = msg_is_response(msg);
	if (status >= 300 && status < 400)
		return true;
	return false;
}
bool msg_is_status_4xx(const char *msg)
{
	int status = msg_is_response(msg);
	if (status >= 400 && status < 500)
		return true;
	return false;
}
bool msg_is_status_5xx(const char *msg)
{
	int status = msg_is_response(msg);
	if (status >= 500 && status < 600)
		return true;
	return false;
}
bool msg_is_register(const char *msg)
{
	return msg_is_request(msg, "REGISTER");
}
bool msg_is_invite(const char *msg)
{
	return msg_is_request(msg, "INVITE");
}
bool msg_is_ack(const char *msg)
{
	return msg_is_request(msg, "ACK");
}
bool msg_is_bye(const char *msg)
{
	return msg_is_request(msg, "BYE");
}
bool msg_is_cancel(const char *msg)
{
	return msg_is_request(msg, "CANCEL");
}
bool msg_is_info(const char *msg)
{
	return msg_is_request(msg, "INFO");
}
bool msg_is_subscribe(const char *msg)
{
	return msg_is_request(msg, "SUBSCRIBE");
}
bool msg_is_notify(const char *msg)
{
	return msg_is_request(msg, "NOTIFY");
}
bool msg_is_message(const char *msg)
{
	return msg_is_request(msg, "MESSAGE");
}
bool msg_is_options(const char *msg)
{
	return msg_is_request(msg, "OPTIONS");
}

void sip_init(const char * sipcode, const char * ip, int port)
{
	memset(sipuser, 0, sizeof(sipuser));
	memset(sipaddr, 0, sizeof(sipaddr));
	strcpy(sipuser, sipcode);
	strcpy(sipaddr, ip);
	sipport = port;
}

void sip_sleep(int usec)
{
#ifdef _WIN32
	Sleep(usec);
#else
	usleep(usec);
#endif
}

time_t sip_get_totalseconds(char * time)
{
	int year, month, day, hour, min, sec;
	if (strlen(time) == 19)
	{
		time[10] = ' ';
		sscanf(time, "%04d-%02d-%02d %02d:%02d:%02d", &year, &month, &day, &hour, &min, &sec);
	}
	else if (strlen(time) == 14)
		sscanf(time, "%04d%02d%02d%02d%02d%02d", &year, &month, &day, &hour, &min, &sec);
	else
		return 0;
	tm tinfo;
	tinfo.tm_year = year - 1900;
	tinfo.tm_mon = month - 1;
	tinfo.tm_mday = day;
	tinfo.tm_hour = hour;
	tinfo.tm_min = min;
	tinfo.tm_sec = sec;
	tinfo.tm_isdst = 0;
	return mktime(&tinfo);
}

const char *sip_get_formattime(char *outstr, int outlen, char * time)
{
	int year, month, day, hour, min, sec;
	if (strlen(time) == 19)
		sscanf(time, "%04d-%02d-%02d %02d:%02d:%02d", &year, &month, &day, &hour, &min, &sec);
	else if (strlen(time) == 14)
		sscanf(time, "%04d%02d%02d%02d%02d%02d", &year, &month, &day, &hour, &min, &sec);
	else
		return 0;
	acl::string strTime;
	strTime.format("%04d-%02d-%02dT%02d:%02d:%02d", year, month, day, hour, min, sec);
	if (strTime.size() < outlen)
	{
		memset(outstr, 0, outlen);
		strcpy(outstr, strTime.c_str());
		return outstr;
	}
	return 0;
}

const char *sip_get_generaltime(char *outstr, int outlen, char * time)
{
	acl::string strTime = time;
	strTime.replace('T', ' ');
	if (strTime.size() < outlen)
	{
		memset(outstr, 0, outlen);
		strcpy(outstr, strTime.c_str());
		return outstr;
	}
	return 0;
}

int sip_build_randsn()
{
	char szRandnum[SIP_RAND_LEN] = { 0 };
	acl::string strRand = sip_build_randnum(szRandnum, sizeof(szRandnum));
	acl::string strSn = strRand.right(4);
	return atoi(strSn.c_str());
}

const char *sip_build_randnum(char *outstr, int outlen)
{
	static bool bSrand = false;
	static int  nRid = 0;
	unsigned int nNum = 0;
	unsigned int nOp = 100000000;
	if (!bSrand)
	{
		srand((unsigned int)time(NULL));
		bSrand = true;
	}
	acl::string strNum = "";
	strNum.format("%u", ((rand() % 10 + 1)*nOp + (rand() % nOp)*(rand() % nOp) % nOp)+nRid++);
	if (nRid >= 10000000)
		nRid = 0;
	if (strNum.size() < outlen)
	{
		memset(outstr, 0, outlen);
		strcpy(outstr, strNum.c_str());
		return outstr;
	}
	return 0;
}

const char *sip_build_sdp_ssrc(char *outstr, int outlen, SIP_INVITE_TYPE type, SIP_PROTOCOL_TYPE prot)
{
	//acl::string strUser(sipuser+3,4);
	acl::string strSsrc;
	if (type >= SIP_INVITE_TALK)
		strSsrc.format("2000%d%d%04d", type, (int)prot, sip_build_randsn());
	else if (type == SIP_INVITE_PLAY || type == SIP_INVITE_PLAY_SDK)
		strSsrc.format("0000%d%d%04d", type, (int)prot, sip_build_randsn());
	else
		strSsrc.format("1000%d%d%04d", type, (int)prot, sip_build_randsn());
	if (strSsrc.size() > outlen)
	{
		memset(outstr, 0, outlen);
		memcpy(outstr, strSsrc.c_str(), 10);
		return outstr;
	}
	else
	{
		memset(outstr, 0, outlen);
		strcpy(outstr, strSsrc.c_str());
		return outstr;
	}
}

const char *sip_build_nonce(char *outstr, int outlen)
{
	char szOutNum[33] = { 0 };
	acl::string strNum = sip_build_randnum(outstr, outlen);
	acl::md5::md5_string(strNum.c_str(), strNum.size(), NULL, 0, szOutNum, sizeof(szOutNum));
	acl::string strOutNum = szOutNum;
	if (outlen > 16)
	{
		memset(outstr, 0, outlen);
		strcpy(outstr, strOutNum.left(16).c_str());
		return outstr;
	}
	return 0;
}

const char * sip_build_scale(char * outstr, int outlen, float scale)
{
	acl::string strScale;
	if (scale - (float)(int)scale == 0.0) //¿ì½ø
		strScale.format("%.1f", scale);
	else                                  //Âý·Å
		strScale.format("%g", scale);
	if (strScale.size() < outlen)
	{
		memset(outstr, 0, outlen);
		strcpy(outstr, strScale.c_str());
		return outstr;
	}
	return 0;
}

const char *sip_build_sdp_time(char *outstr, int outlen, const char * starttime, const char * endtime)
{
	acl::string strTime;
	strTime.format("%llu %llu", sip_get_totalseconds((char *)starttime), sip_get_totalseconds((char *)endtime));
	if (strTime.size() < outlen)
	{
		memset(outstr, 0, outlen);
		strcpy(outstr, strTime.c_str());
		return outstr;
	}
	return 0;
}

const char *sip_build_authorization_response(char *outstr, int outlen, const char * username, const char * realm, const char * password, const char * nonce, const char * uri)
{
	acl::string strAuth1,strAuth2,strAuth3;
	strAuth1.format("%s:%s:%s", username, realm, password);
	strAuth2.format("REGISTER:%s", uri);
	char szAuth1[33] = { 0 };
	acl::md5::md5_string(strAuth1.c_str(), strAuth1.size(), NULL, 0, szAuth1, sizeof(szAuth1));
	char szAuth2[33] = { 0 };
	acl::md5::md5_string(strAuth2.c_str(), strAuth2.size(), NULL, 0, szAuth2, sizeof(szAuth2));
	strAuth3.format("%s:%s:%s", szAuth1, nonce, szAuth2);
	char szRes[33] = { 0 };
	acl::md5::md5_string(strAuth3.c_str(), strAuth3.size(), NULL, 0, szRes, sizeof(szRes));
	if (outlen >= 33)
	{
		memset(outstr, 0, outlen);
		strcpy(outstr, szRes);
		return outstr;
	}
	return 0;
}

const char * sip_bulid_msg_via(char * outstr, int outlen, const char * ip, int port)
{
	acl::string strVia;
	strVia.format("SIP/2.0/UDP %s:%d;branch=z9hG4bK%s", ip, port, sip_build_randnum(outstr, outlen));
	if (strVia.size() < outlen)
	{
		memset(outstr, 0, outlen);
		strcpy(outstr, strVia.c_str());
		return outstr;
	}
	return 0;
}

const char *sip_build_msg_from(char *outstr, int outlen)
{
	acl::string strFrom;
	strFrom.format("sip:%s@%s:%d", sipuser, sipaddr, sipport);
	if (strFrom.size() < outlen)
	{
		memset(outstr, 0, outlen);
		strcpy(outstr, strFrom.c_str());
		return outstr;
	}
	return 0;
}

const char *sip_build_msg_to(char *outstr, int outlen, const char * user, const char * dst)
{
	acl::string strTo;
	strTo.format("sip:%s@%s", user, dst);
	if (strTo.size() < outlen)
	{
		memset(outstr, 0, outlen);
		strcpy(outstr, strTo.c_str());
		return outstr;
	}
	return 0;
}

const char *sip_build_msg_call_id(char *outstr, int outlen, unsigned int id)
{
	acl::string strCallId;
	if (id > 0)
		strCallId.format("%u@%s", id, sipaddr);
	else
		strCallId.format("%s@%s", sip_build_randnum(outstr, outlen), sipaddr);
	if (strCallId.size() < outlen)
	{
		memset(outstr, 0, outlen);
		strcpy(outstr, strCallId.c_str());
		return outstr;
	}
	return 0;
}

const char *sip_build_msg_subject(char *outstr, int outlen, const char * deviceid, const char * ssrc, const char * terminalid)
{
	acl::string strSsrc = ssrc;
	if (ssrc == NULL || ssrc[0] == '\0')
	{
		strSsrc = "0000000000";
	}
	acl::string strSub;
	if (terminalid == NULL || terminalid[0] == '\0')
		strSub.format("%s:%s,%s:1", deviceid, strSsrc.c_str(), sipuser);
	else if (atoi(terminalid) < 10)
		strSub.format("%s:%s,%s:%d", deviceid, strSsrc.c_str(), sipuser, atoi(terminalid));
	else
		strSub.format("%s:%s,%s:%s", deviceid, strSsrc.c_str(), sipuser, terminalid);
	if (strSub.size() < outlen)
	{
		memset(outstr, 0, outlen);
		strcpy(outstr, strSub.c_str());
		return outstr;
	}
	return 0;
}

int sip_build_msg_request(char *outstr, int outlen, const char * method, const char * dst, const char * callid, int cseq, const char * from, const char * to, const char * ftag, const char * ttag)
{
	acl::string strReq;
	acl::string strDst = dst;
	acl::string strCallId = callid;
	acl::string strFrom = from;
	acl::string strTo = to;
	acl::string strFTag = ftag;
	if ((dst == NULL || dst[0] == '\0') && (to == NULL || to[0] == '\0'))
	{
		return 0;
	}
	if (dst == NULL || dst[0] == '\0')
	{
		strDst = to;
	}
	if (callid == NULL || callid[0] == '\0')
	{
		strCallId = sip_build_msg_call_id(outstr, outlen);
	}
	if (from == NULL || from[0] == '\0')
	{
		strFrom = sip_build_msg_from(outstr, outlen);
	}
	if (ftag == NULL || ftag[0] == '\0')
	{
		strFTag = sip_build_randnum(outstr, outlen);
	}
	if (to == NULL || to[0] == '\0')
	{
		to = strFrom.c_str();
	}
	if (ttag == NULL || ttag[0] == '\0')
	{
		strTo.format("<%s>", to);
	}
	else
	{
		strTo.format("<%s>;tag=%s", to, ttag);
	}
	strReq.format("%s %s SIP/2.0%s"
		"Via: SIP/2.0/UDP %s:%d;branch=z9hG4bK%s%s"
		"From: <%s>;tag=%s%s"
		"To: %s%s"
		"Call-ID: %s%s"
		"CSeq: %d %s%s"
		"Max-Forwards: 70%s"
		"User-Agent: %s%s"
		"Content-Length: 0%s%s",
		method, strDst.c_str(), Line,
		sipaddr, sipport, sip_build_randnum(outstr, outlen), Line,
		strFrom.c_str(), strFTag.c_str(), Line,
		strTo.c_str(), Line,
		strCallId.c_str(), Line,
		cseq, method, Line,
		Line, SIP_USER_AGENT, Line, Line, Line);
	if (strReq.size() < outlen)
	{
		memset(outstr, 0, outlen);
		strcpy(outstr, strReq.c_str());
		return (int)strReq.size();
	}
	return 0;
}

int sip_build_msg_answer(char *outstr, int outlen, const char *msg, const char *status)
{
	acl::string strVia = sip_get_header(outstr, outlen, msg, SIP_H_VIA, false);
	acl::string strFrom = sip_get_header(outstr, outlen, msg, SIP_H_FROM, false);
	acl::string strTo = sip_get_header(outstr, outlen, msg, SIP_H_TO);
	acl::string strCallID = sip_get_header(outstr, outlen, msg, SIP_H_CALL_ID, false);
	acl::string strCSeq = sip_get_header(outstr, outlen, msg, SIP_H_CSEQ, false);
	if (strVia == "" || strFrom == "" || strTo == "" || strCallID == "" || strCSeq == "")
	{
		return 0;
	}
	char *sTag = strTo.rfind("tag=");
	if (sTag == NULL)
		strTo.format_append(";tag=%s%s", sip_build_randnum(outstr, outlen), Line);
	else
		strTo.append(Line);

	memset(outstr, 0, outlen);
	acl_sprintf(outstr,
		"SIP/2.0 %s%s"
		"Via: %s"
		"From: %s"
		"To: %s"
		"Call-ID: %s"
		"CSeq: %s"
		"User-Agent: %s%s"
		"Content-Length: 0%s%s",
		status, Line, strVia.c_str(), strFrom.c_str(), strTo.c_str(), strCallID.c_str(), strCSeq.c_str(), SIP_USER_AGENT, Line, Line, Line);
	return (int)strlen(outstr);
}

int sip_build_invite_answer(char * outstr, int outlen, const char * status, const char * via, const char * from, const char * to, const char * ttag, const char *callid, const char *deviceid, int cseq)
{
	acl::string strAnswer;
	strAnswer.format(
		"SIP/2.0 %s%s"
		"Via: %s%s"
		"From: %s%s"
		"To: %s;tag=%s%s"
		"Call-ID: %s%s"
		"CSeq: %d INVITE%s"
		"User-Agent: %s%s"
		"Contact: <%s@%s:%d>%s"
		"Content-Length: 0%s%s",
		status, Line, via, Line, from, Line, to, ttag, Line, callid, Line, cseq, Line, SIP_USER_AGENT, Line, deviceid, sipaddr, sipport, Line, Line, Line);
	if (strAnswer.size() < outlen)
	{
		memset(outstr, 0, outlen);
		strcpy(outstr, strAnswer.c_str());
		return (int)strAnswer.size();
	}
	return 0;
}

int sip_build_msg_register(char *outstr, int outlen, const char * to, const char *auth, int cseq, int expires, const char *callid, const char *ftag)
{
	int nLen = sip_build_msg_request(outstr, outlen, "REGISTER", to, callid, cseq, NULL, NULL, ftag, NULL);
	acl::string strReq;
	char szFrom[SIP_HEADER_LEN] = { 0 };
	acl::string strCotact;
	strCotact.format("<%s>", sip_build_msg_from(szFrom, sizeof(szFrom)));
	nLen = sip_set_header(outstr, outlen, SIP_H_CONTACT, strCotact.c_str());
	if (auth)
	{
		nLen = sip_set_header(outstr, outlen, SIP_H_AUTHORIZATION, auth);
	}
	acl::string strExpires;
	strExpires.format("%d", expires);
	nLen = sip_set_header(outstr, outlen, SIP_H_EXPIRES, strExpires.c_str());
	return nLen;
}

int sip_build_msg_keepalive(char *outstr, int outlen, const char * to)
{
	int nLen = sip_build_msg_request(outstr, outlen, "MESSAGE", NULL, NULL, 3, NULL, to, NULL, NULL);
	acl::string strBody;
	strBody.format(
		"<?xml version=\"1.0\"?>%s"
		"<Notify>%s"
		"<CmdType>Keepalive</CmdType>%s"
		"<SN>%d</SN>%s"
		"<DeviceID>%s</DeviceID>%s"
		"<Status>OK</Status>%s"
		"</Notify>%s",
		Line, Line, Line, sip_build_randsn(), Line, sipuser, Line, Line, Line);
	nLen = sip_set_body(outstr, outlen, strBody.c_str(), SIP_APPLICATION_XML);
	return nLen;
}

int sip_build_msg_invite(char *outstr, int outlen, const char * to, const char * subject, int cseq, const char * callid, const char * ftag, const char *ttag)
{
	int nLen = sip_build_msg_request(outstr, outlen, "INVITE", NULL, callid, cseq, NULL, to, ftag, ttag);
	nLen = sip_set_header(outstr, outlen, SIP_H_SUBJECT, subject);
	char szFrom[SIP_HEADER_LEN] = { 0 };
	acl::string strCotact;
	strCotact.format("<%s>", sip_build_msg_from(szFrom, sizeof(szFrom)));
	nLen = sip_set_header(outstr, outlen, SIP_H_CONTACT, strCotact.c_str());
	return nLen;
}

int sip_build_msg_sdp(char *outstr, int outlen, const char * user, const char * uri, const char * time, const char * recvip, int recvport, int type, int drct, int prot, int speed, int media, int playload, char *stream)
{
	acl::string strTime = time;
	if (strTime == "")
		strTime = "0 0";
	acl::string strSdp;
	acl::string strType = "Play";
	if (type == SIP_INVITE_PLAYBACK)
		strType = "Playback";
	else if (type == SIP_INVITE_DOWNLOAD)
		strType = "Download";
	else if (type == SIP_INVITE_TALK)
		strType = "Talk";
	else if (type == SIP_INVITE_MEDIA)
		strType = "MediaServer";
	strSdp.format(
		"v=0%s"
		"o=%s 0 0 IN IP4 %s%s"
		"s=%s%s"
		"u=%s%s"
		"c=IN IP4 %s%s"
		"t=%s%s",
		Line, user, recvip, Line, strType.c_str(), Line, uri, Line, recvip, Line, strTime.c_str(), Line);
	acl::string strDrct = "sendonly";
	if (drct == SIP_RECVONLY)
		strDrct = "recvonly";
	else if (drct == SIP_SENDRECV)
		strDrct = "sendrecv";
	//acl::string strStream = "PS";
	//if (stream == SIP_STREAM_HIKVISION)
	//	strStream = "HIKVISON";
	//else if (stream == SIP_STREAM_HIKVISION)
	//	strStream = "HIKVISON";
	//else if (stream == SIP_STREAM_DAHUA)
	//	strStream = "DAHUA";
	//else if (stream == SIP_STREAM_MPEG4)
	//	strStream = "MPEG4";
	//else if (stream == SIP_STREAM_H264)
	//	strStream = "H264";
	//else if (stream == SIP_STREAM_SVAC)
	//	strStream = "SVAC";
	//else if (stream == SIP_STREAM_H265)
	//	strStream = "H265";
	//else if (stream == SIP_STREAM_PCMA)
	//	strStream = "PCMA";
	//else if (stream == SIP_STREAM_SVACA)
	//	strStream = "SVACA";
	//else if (stream == SIP_STREAM_G723)
	//	strStream = "G723";
	//else if (stream == SIP_STREAM_G729)
	//	strStream = "G729";
	//else if (stream == SIP_STREAM_G722)
	//	strStream = "G722";
	acl::string strProt = "TCP/RTP/AVP";
	if (prot == SIP_TRANSFER_UDP)
		strProt = "RTP/AVP";
	if (media == SIP_MEDIA_VIDEO)
	{
		if (playload == 0)
		{
			strSdp.format_append(
				"m=video %d %s 96 98 97%s"
				"a=%s%s"
				"a=rtpmap:96 PS/90000%s"
				"a=rtpmap:98 H264/90000%s"
				"a=rtpmap:97 MPEG4/90000%s",
				recvport, strProt.c_str(), Line, strDrct.c_str(), Line, Line, Line, Line);
		}
		else
		{
			strSdp.format_append(
				"m=video %d %s %d%s"
				"a=%s%s"
				"a=rtpmap:%d %s/90000%s",
				recvport, strProt.c_str(), playload, Line, strDrct.c_str(), Line, playload, stream, Line);
		}
	}
	else
	{
		strSdp.format_append(
			"m=audio %d %s %d%s"
			"a=%s%s"
			"a=rtpmap:%d %s/8000%s",
			recvport, strProt.c_str(), playload, Line, strDrct.c_str(), Line, playload, stream, Line);
	}
	if (prot == SIP_TRANSFER_TCP_ACTIVE || prot == SIP_TRANSFER_TCP_PASSIVE)
	{
		acl::string strSetup = "active";
		if (prot == SIP_TRANSFER_TCP_PASSIVE)
			strSetup = "passive";
		strSdp.format_append(
			"a=setup:%s%s"
			"a=connection:new%s",
			strSetup.c_str(), Line, Line);
	}
	if (speed > 0)
	{
		strSdp.format_append("a=downloadspeed:%d%s", speed, Line);
	}
	if (strSdp.size() < outlen)
	{
		memset(outstr, 0, outlen);
		strcpy(outstr, strSdp.c_str());
		return (int)strSdp.size();
	}
	return 0;
}

int sip_build_msg_ack(char *outstr, int outlen, const char * msg)
{
	acl::string strVia = sip_get_header(outstr, outlen, msg, "Via: ", false);
	acl::string strFrom = sip_get_header(outstr, outlen, msg, "From: ", false);
	acl::string strTo = sip_get_header(outstr, outlen, msg, "To: ", false);
	acl::string strDst = sip_get_to_uri(outstr, outlen, msg);
	acl::string strCallID = sip_get_header(outstr, outlen, msg, "Call-ID: ", false);
	int nCSeq = sip_get_cseq_num(msg);
	if (strVia == "" || strFrom == "" || strTo == "" || strCallID == "" || nCSeq == 0)
	{
		return 0;
	}
	acl::string strAck;
	strAck.format(
		"ACK %s SIP/2.0%s"
		"Via: %s"
		"From: %s"
		"To: %s"
		"Call-ID: %s"
		"CSeq: %d ACK%s"
		"Max-Forwards: 70%s"
		"User-Agent: %s%s"
		"Content-Length: 0%s%s",
		strDst.c_str(), Line, strVia.c_str(), strFrom.c_str(), strTo.c_str(), strCallID.c_str(), nCSeq, Line, Line, SIP_USER_AGENT, Line, Line, Line);
	if (strAck.size() < outlen)
	{
		memset(outstr, 0, outlen);
		strcpy(outstr, strAck.c_str());
		return (int)strAck.size();
	}
	return 0;
}

int sip_build_msg_ack(char * outstr, int outlen, const char * to, int cseq, const char * callid, const char * ftag, const char * ttag)
{
	return sip_build_msg_request(outstr, outlen, "ACK", NULL, callid, cseq, NULL, to, ftag, ttag);
}

int sip_build_msg_bye(char *outstr, int outlen, const char * to, int cseq, const char * callid, const char * ftag, const char * ttag, char *from)
{
	return sip_build_msg_request(outstr, outlen, "BYE", NULL, callid, cseq, from, to, ftag, ttag);
}

SIP_EVENT_TYPE sip_get_event_type(const char *msg)
{
	acl::string strMsg = msg;
	char szMedtod[SIP_HEADER_LEN] = { 0 };
	acl::string strMethod = sip_get_cseq_method(szMedtod, sizeof(szMedtod), strMsg.c_str());
	if (msg_is_options(strMsg.c_str()))
		return SIP_OPTIONS_REQUEST;
	else if (msg_is_register(strMsg.c_str()))
		return SIP_REGISTER_REQUEST;
	else if (msg_is_invite(strMsg.c_str()))
		return SIP_INVITE_REQUEST;
	else if (msg_is_ack(strMsg.c_str()))
		return SIP_ACK_REQUEST;
	else if (msg_is_bye(strMsg.c_str()))
		return SIP_BYE_REQUEST;
	else if (msg_is_cancel(strMsg.c_str()))
		return SIP_CANCEL_REQUEST;
	else if (msg_is_info(strMsg.c_str()))
		return SIP_INFO_REQUEST;
	else if (msg_is_subscribe(strMsg.c_str()))
		return SIP_SUBSCRIBE_REQUEST;
	else if (msg_is_notify(strMsg.c_str()))
		return SIP_NOTIFY_EVENT;
	else if (msg_is_message(strMsg.c_str()))
		return SIP_MESSAGE_EVENT;
	else if (msg_is_status_1xx(strMsg.c_str()))
	{
		if (strMethod.compare("INVITE", false) == 0)
			return SIP_INVITE_PROCEEDING;
	}
	else if (msg_is_status_2xx(strMsg.c_str()))
	{
		if (strMethod.compare("OPTIONS", false) == 0)
			return SIP_OPTIONS_RESPONSE;
		if (strMethod.compare("REGISTER", false) == 0)
			return SIP_REGISTER_SUCCESS;
		if (strMethod.compare("INVITE", false) == 0)
			return SIP_INVITE_SUCCESS;
		if (strMethod.compare("BYE", false) == 0)
			return SIP_BYE_SUCCESS;
		if (strMethod.compare("CANCEL", false) == 0)
			return SIP_CANCEL_SUCCESS;
		if (strMethod.compare("INFO", false) == 0)
			return SIP_INFO_SUCCESS;
		if (strMethod.compare("SUBSCRIBE", false) == 0)
			return SIP_SUBSCRIBE_SUCCESS;
		if (strMethod.compare("NOTIFY", false) == 0)
			return SIP_NOTIFY_SUCCESS;
		if (strMethod.compare("MESSAGE", false) == 0)
			return SIP_MESSAGE_SUCCESS;

	}
	else if (msg_is_status_4xx(strMsg.c_str()) || msg_is_status_5xx(strMsg.c_str()))
	{
		if (strMethod.compare("REGISTER",false) == 0)
			return SIP_REGISTER_FAILURE;
		if (strMethod.compare("INVITE", false) == 0)
			return SIP_INVITE_FAILURE;
		if (strMethod.compare("BYE", false) == 0)
			return SIP_BYE_FAILURE;
		if (strMethod.compare("CANCEL", false) == 0)
			return SIP_CANCEL_FAILURE;
		if (strMethod.compare("INFO", false) == 0)
			return SIP_INFO_FAILURE;
		if (strMethod.compare("SUBSCRIBE", false) == 0)
			return SIP_SUBSCRIBE_FAILURE;
		if (strMethod.compare("NOTIFY", false) == 0)
			return SIP_NOTIFY_FAILURE;
		if (strMethod.compare("MESSAGE", false) == 0)
			return SIP_MESSAGE_FAILURES;
	}
	return SIP_EVENT_UNKNOWN;
}

SIP_MESSAGE_TYPE sip_get_message_type(const char *msg)
{
	acl::string strMsg = msg;
	if (strMsg.find("<Control>", false))
	{
		if (strMsg.find("<PTZCmd>", false))			//PTZType
			return SIP_CONTROL_PTZCMD;
		else if (strMsg.find("<TeleBoot>", false))	//Boot
			return SIP_CONTROL_TELEBOOT;
		else if (strMsg.find("<RecordCmd>", false))	//recordType:Record/StopRecord
			return SIP_CONTROL_RECORDCMD;
		else if (strMsg.find("<GuardCmd>", false))	//guardType:SetGuard/ResetGuard
			return SIP_CONTROL_GUARDCMD;
		else if (strMsg.find("<AlarmCmd>", false))	//ResetAlarm
			return SIP_CONTROL_ALARMCMD;
		else if (strMsg.find("<IFameCmd>", false))
			return SIP_CONTROL_IFAMECMD;
		else if (strMsg.find("<DragZoomIn>", false))
			return SIP_CONTROL_DRAGZOOMIN;
		else if (strMsg.find("<DragZoomOut>", false))
			return SIP_CONTROL_DRAGZOOMOUT;
		else if (strMsg.find("<HomePosition>", false))
			return SIP_CONTROL_HOMEPOSITION;
		else if (strMsg.find("<BasicParam>", false))
			return SIP_CONTROL_CONFIG;
		else if (strMsg.find("<SAVCEncodeConfig>", false))
			return SIP_CONTROL_CONFIG;
		else if (strMsg.find("<SAVCDecodeConfig>", false))
			return SIP_CONTROL_CONFIG;
	}
	else if (strMsg.find("<Query>", false))
	{
		if (strMsg.find(">DeviceStatus<", false))
			return SIP_QUERY_DEVICESTATUS;
		else if (strMsg.find(">Catalog<", false))
			return SIP_QUERY_CATALOG;
		else if (strMsg.find(">DeviceInfo<", false))
			return SIP_QUERY_DEVICEINFO;
		else if (strMsg.find(">RecordInfo<", false))
			return SIP_QUERY_RECORDINFO;
		else if (strMsg.find(">Alarm<", false))
			return SIP_QUERY_ALARM;
		else if (strMsg.find(">ConfigDownload<", false)) //BasicParam/VideoParam/SVACEncodeConfig/SVACDecodeConfig
			return SIP_QUERY_CONFIGDOWNLOAD;
		else if (strMsg.find(">PresetQuery<", false))
			return SIP_QUERY_PRESETQUERY;
		else if (strMsg.find(">MobilePosition<", false))
			return SIP_QUERY_MOBILEPOSITION;
	}
	else if (strMsg.find("<Notify>", false))
	{
		if (strMsg.find(">Keepalive<", false))
			return SIP_NOTIFY_KEEPALIVE;
		else if (strMsg.find(">Alarm<", false))
			return SIP_NOTIFY_ALARM;
		else if (strMsg.find(">MediaStatus<", false))
			return SIP_NOTIFY_MEDIASTATUS;
		else if (strMsg.find(">Broadcast<", false))
			return SIP_NOTIFY_BROADCAST;
		else if (strMsg.find(">MobilePositon<", false))
			return SIP_NOTIFY_MOBILEPOSITION;
	}
	else if (strMsg.find("<Response>", false))
	{
		if (strMsg.find(">DeviceControl<", false))	
			return SIP_RESPONSE_CONTROL;
		else if (strMsg.find(">Alarm<", false))
			return SIP_RESPONSE_NOTIFY_ALARM;
		else if (strMsg.find(">Catalog<", false))
			return SIP_RESPONSE_CATALOG;
		else if (strMsg.find(">DeviceInfo<", false))
			return SIP_RESPONSE_DEVICEINFO;
		else if (strMsg.find(">DeviceStatus<", false))
			return SIP_RESPONSE_DEVICESTATUS;
		else if (strMsg.find(">RecordInfo<", false))
			return SIP_RESPONSE_RECORDINFO;
		else if (strMsg.find(">ConfigDwonload<", false))
			return SIP_RESPONSE_CONFIGDOWNLOAD;
		else if (strMsg.find(">PresetQuery<", false))
			return SIP_RESPONSE_PRESETQUERY;
		else if (strMsg.find(">Broadcast<", false))
			return SIP_RESPONSE_BROADCAST;
	}
	return SIP_MESSAGE_UNKNOWN;
}

SIP_CONTENT_TYPE sip_get_content_type(const char *msg)
{
	char szMsg[SIP_HEADER_LEN] = { 0 };
	acl::string strContentType = sip_get_header(szMsg, sizeof(szMsg), msg, SIP_H_CONTENT_TYPE);
	if (strContentType != "")
	{
		if (strContentType.find("xml", false))
			return SIP_APPLICATION_XML;
		else if (strContentType.find("sdp", false))
			return SIP_APPLICATION_SDP;
		else if (strContentType.find("rtsp", false))
			return SIP_APPLICATION_RTSP;
	}
	return SIP_APPLICATION_UNKNOWN;
}

SIP_RECORD_TYPE sip_get_record_type(const char * type)
{
	if (strcasecmp(type, "time") == 0)
		return SIP_RECORDTYPE_TIME;
	else if (strcasecmp(type, "move") == 0)
		return SIP_RECORDTYPE_MOVE;
	else if (strcasecmp(type, "alarm") == 0)
		return SIP_RECORDTYPE_ALARM;
	else if (strcasecmp(type, "manual") == 0)
		return SIP_RECORDTYPE_MANUAL;
	else
		return SIP_RECORDTYPE_ALL;
}

acl::string get_msg_substr(const char *msg, const char *begin, const char *end)
{
	acl::string strMsg = msg;
	size_t blen = strlen(begin);
	char *sBegin = strMsg.find(begin, false);
	if (sBegin)
	{
		acl::string strSubstr = sBegin + blen;
		char *sEnd = strSubstr.find(end, false);
		if (sEnd)
		{
			sEnd[0] = '\0';
			return strSubstr;
		}
	}
	return "";
}

acl::string get_msg_substr2(const char *msg, const char *head, const char *begin, const char *end)
{
	size_t hlen = strlen(head);
	size_t blen = strlen(begin);
	char *pHead = (char *)strstr(msg, head);
	if (pHead == NULL)
	{
		return "";
	}
	pHead += hlen;
	char *pBegin = (char *)strstr(pHead, begin);
	if (pBegin == NULL)
	{
		return "";
	}
	pBegin += blen;
	acl::string strSubstr = pBegin;
	char *sEnd = strSubstr.find(end);
	if (sEnd == NULL)
	{
		return "";
	}
	sEnd[0] = '\0';
	return strSubstr;
}

const char *sip_get_localtime(char *outstr, int outlen, time_t spansec)
{
	time_t ctime = time(NULL);
	ctime += spansec;
	tm *ctm = localtime(&ctime);
	acl::string strDate;
	strDate.format("%04d-%02d-%02dT%02d:%02d:%02d", ctm->tm_year+1900, ctm->tm_mon+1, ctm->tm_mday, ctm->tm_hour, ctm->tm_min, ctm->tm_sec);
	if (strDate.size() < outlen)
	{
		memset(outstr, 0, outlen);
		strcpy(outstr, strDate.c_str());
		return outstr;
	}
	return 0;
}

int sip_get_user_role(const char * user)
{
	acl::string strUser = user;
	if (strUser.size() != 20)
		return SIP_ROLE_INVALID;
	acl::string strType;
	strUser.substr(strType,10, 3);
	int nRoleType = atoi(strType.c_str());
	if (nRoleType >= 300)
	{
		return SIP_ROLE_CLIENT;
	}
	else if (nRoleType >= 200 && nRoleType < 300)
	{
		if (nRoleType == 202)
		{
			return SIP_ROLE_MEDIA;
		}
		else if (nRoleType == 210)
		{
			return SIP_ROLE_CLIENT;
		}
		else if (nRoleType == 255)
		{
			return SIP_ROLE_DIRECT;
		}
		return SIP_ROLE_SENDER;
	}
	else if (nRoleType > 130 && nRoleType <= 140)
	{
		if (nRoleType == 133)
		{
			return SIP_ROLE_MONITOR;
		}
		return SIP_ROLE_CHANNEL;
	}
	else if (nRoleType > 110 && nRoleType <= 130)
	{
		if (nRoleType == 114)
		{
			return SIP_ROLE_DECODER;
		}
		return SIP_ROLE_DEVICE;
	}
	return SIP_ROLE_INVALID;
}

const char *sip_get_header(char *outstr, int outlen, const char *msg, const char *head, bool nonl)
{
	acl::string strMsg = msg;
	char *sFnd = strMsg.find(head, false);
	if (sFnd != NULL)
	{	
		sFnd += strlen(head);
		acl::string strFnd = sFnd;
		acl::string strHead;
		if (strFnd.scan_line(strHead, nonl))
		{
			if (strHead.size() < outlen)
			{
				memset(outstr, 0, outlen);
				strcpy(outstr, strHead.c_str());
				return outstr;
			}
		}
	}
	return 0;
}

int sip_get_status_code(const char * msg)
{
	acl::string strMsg = msg;
	acl::string strCode;
	strMsg.substr(strCode,8,3);
	int nStatus = atoi(strMsg.c_str());
	return nStatus > 0 ? nStatus : 400;
}

const char * sip_get_from_uri(char * outstr, int outlen, const char * msg)
{
	acl::string strValue = get_msg_substr(msg, "From: <", ">");
	if (strlen(strValue.c_str()) < outlen)
	{
		memset(outstr, 0, outlen);
		strcpy(outstr, strValue.c_str());
		return outstr;
	}
	return 0;
}

const char *sip_get_from_user(char *outstr, int outlen, const char * msg)
{
	acl::string strValue = get_msg_substr(msg, "From: <sip:", "@");
	if (strlen(strValue.c_str()) < outlen)
	{
		memset(outstr, 0, outlen);
		strcpy(outstr, strValue.c_str());
		return outstr;
	}
	return 0;
}

const char *sip_get_from_addr(char *outstr, int outlen, const char * msg)
{
	acl::string strValue = get_msg_substr2(msg, "From: ", "@", ">");
	if (strlen(strValue.c_str()) < outlen)
	{
		memset(outstr, 0, outlen);
		strcpy(outstr, strValue.c_str());
		return outstr;
	}
	return 0;
}

const char *sip_get_from_tag(char *outstr, int outlen, const char * msg)
{
	acl::string strFrom = sip_get_header(outstr, outlen, msg, SIP_H_FROM);
	char *sFnd = strFrom.rfind("tag=");
	if (sFnd)
	{
		sFnd += strlen("tag=");
		if (strlen(sFnd) < outlen)
		{
			memset(outstr, 0, outlen);
			strcpy(outstr, sFnd);
			return outstr;
		}
	}
	return 0;
}

const char *sip_get_to_uri(char *outstr, int outlen, const char * msg)
{
	acl::string strValue = get_msg_substr(msg, "To: <", ">");
	if (strlen(strValue.c_str()) < outlen)
	{
		memset(outstr, 0, outlen);
		strcpy(outstr, strValue.c_str());
		return outstr;
	}
	return 0;
}

const char *sip_get_to_user(char *outstr, int outlen, const char * msg)
{
	acl::string strValue = get_msg_substr(msg, "To: <sip:", "@");
	if (strlen(strValue.c_str()) < outlen)
	{
		memset(outstr, 0, outlen);
		strcpy(outstr, strValue.c_str());
		return outstr;
	}
	return 0;
}

const char *sip_get_to_addr(char *outstr, int outlen, const char * msg)
{
	acl::string strValue = get_msg_substr2(msg, "To: ", "@", ">");
	if (strlen(strValue.c_str()) < outlen)
	{
		memset(outstr, 0, outlen);
		strcpy(outstr, strValue.c_str());
		return outstr;
	}
	return 0;
}

const char *sip_get_to_tag(char *outstr, int outlen, const char * msg)
{
	acl::string strTo = sip_get_header(outstr, outlen, msg, SIP_H_TO);
	char *sFnd = strTo.rfind("tag=");
	if (sFnd)
	{
		sFnd += strlen("tag=");
		if (strlen(sFnd) < outlen)
		{
			memset(outstr, 0, outlen);
			strcpy(outstr, sFnd);
			return outstr;
		}
	}
	return 0;
}

int sip_get_cseq_num(const char * msg)
{
	return atoi(get_msg_substr(msg, "CSeq: ", " ").c_str());
}

const char *sip_get_cseq_method(char *outstr, int outlen, const char * msg)
{
	acl::string strCseq = sip_get_header(outstr, outlen, msg, SIP_H_CSEQ);
	char *sFnd = strCseq.rfind(" ");
	if (sFnd)
	{
		if (strlen(sFnd + 1) < outlen)
		{
			memset(outstr, 0, outlen);
			strcpy(outstr, sFnd + 1);
			return outstr;
		}
	}
	return 0;
}

const char *sip_get_expires(char *outstr, int outlen, const char * msg)
{
	acl::string strExpires = sip_get_header(outstr, outlen, msg, SIP_H_EXPIRES);
	if (strExpires == "")
	{
		strExpires = sip_get_header(outstr, outlen, msg, "expires=");
		int nPos = strExpires.find(';');
		if (nPos > 0)
		{
			strExpires = strExpires.left(nPos);
		}
	}
	if (strExpires.size() < outlen)
	{
		memset(outstr, 0, outlen);
		strcpy(outstr, strExpires.c_str());
		return outstr;
	}
	return 0;
}

const char *sip_get_authorization_username(char *outstr, int outlen, const char * msg)
{
	acl::string strValue = get_msg_substr(msg, "username=\"", "\"");
	if (strlen(strValue.c_str()) < outlen)
	{
		memset(outstr, 0, outlen);
		strcpy(outstr, strValue.c_str());
		return outstr;
	}
	return 0;
}

const char *sip_get_authorization_realm(char *outstr, int outlen, const char * msg)
{
	acl::string strValue = get_msg_substr(msg, "realm=\"", "\"");
	if (strlen(strValue.c_str()) < outlen)
	{
		memset(outstr, 0, outlen);
		strcpy(outstr, strValue.c_str());
		return outstr;
	}
	return 0;
}

const char *sip_get_authorization_nonce(char *outstr, int outlen, const char * msg)
{
	acl::string strValue = get_msg_substr(msg, "nonce=\"", "\"");
	if (strlen(strValue.c_str()) < outlen)
	{
		memset(outstr, 0, outlen);
		strcpy(outstr, strValue.c_str());
		return outstr;
	}
	return 0;
}

const char *sip_get_authorization_uri(char *outstr, int outlen, const char * msg)
{
	acl::string strValue = get_msg_substr(msg, "uri=\"", "\"");
	if (strlen(strValue.c_str()) < outlen)
	{
		memset(outstr, 0, outlen);
		strcpy(outstr, strValue.c_str());
		return outstr;
	}
	return 0;
}

const char *sip_get_authorization_response(char *outstr, int outlen, const char * msg)
{
	acl::string strValue = get_msg_substr(msg, "response=\"", "\"");
	if (strlen(strValue.c_str()) < outlen)
	{
		memset(outstr, 0, outlen);
		strcpy(outstr, strValue.c_str());
		return outstr;
	}
	return 0;
}

const char *sip_get_authorization_algorithm(char *outstr, int outlen, const char * msg)
{
	acl::string strValue = get_msg_substr(msg, "algorithm=\"", "\"");
	if (strlen(strValue.c_str()) < outlen)
	{
		memset(outstr, 0, outlen);
		strcpy(outstr, strValue.c_str());
		return outstr;
	}
	return 0;
}

const char *sip_get_www_authenticate_realm(char *outstr, int outlen, const char * msg)
{
	acl::string strValue = get_msg_substr(msg, "realm=\"", "\"");
	if (strlen(strValue.c_str()) < outlen)
	{
		memset(outstr, 0, outlen);
		strcpy(outstr, strValue.c_str());
		return outstr;
	}
	return 0;
}

const char *sip_get_www_authenticate_nonce(char *outstr, int outlen, const char * msg)
{
	acl::string strValue = get_msg_substr(msg, "nonce=\"", "\"");
	if (strlen(strValue.c_str()) < outlen)
	{
		memset(outstr, 0, outlen);
		strcpy(outstr, strValue.c_str());
		return outstr;
	}
	return 0;
}

const char *sip_get_subject_deviceid(char *outstr, int outlen, const char * msg)
{
	acl::string strSubject = sip_get_header(outstr, outlen, msg, SIP_H_SUBJECT);
	if (strSubject == "")
		return "";
	int nPos = strSubject.find(':');
	if (nPos < 0)
		return "";
	acl::string strDevice = strSubject.left(nPos);
	if (strDevice.size() < outlen)
	{
		memset(outstr, 0, outlen);
		strcpy(outstr, strDevice.c_str());
		return outstr;
	}
	return 0;
}

int sip_get_subject_playtype(const char * msg)
{
	char outstr[SIP_HEADER_LEN] = { 0 };
	int outlen = (int)sizeof(outstr);
	acl::string strSubject = sip_get_header(outstr, outlen, msg, SIP_H_SUBJECT);
	if (strSubject == "")
		return 0;
	int nBeginPos = strSubject.find(':');
	if (nBeginPos < 0)
		return 0;
	acl::string strSsrc;
	strSubject.substr(strSsrc, nBeginPos + 1, 1);
	return atoi(strSsrc.c_str());
}

int sip_get_subject_tranfer(const char * msg)
{
	char outstr[SIP_HEADER_LEN] = { 0 };
	int outlen = (int)sizeof(outstr);
	acl::string strSubject = sip_get_header(outstr, outlen, msg, SIP_H_SUBJECT);
	if (strSubject == "")
		return 0;
	int nBeginPos = strSubject.find(':');
	if (nBeginPos < 0)
		return 0;
	acl::string strSsrc;
	strSubject.substr(strSsrc, nBeginPos + 6, 1);
	return atoi(strSsrc.c_str());
}

const char * sip_get_subject_msid(char * outstr, int outlen, const char * msg)
{
	acl::string strSubject = sip_get_header(outstr, outlen, msg, SIP_H_SUBJECT);
	if (strSubject == "")
		return "";
	char *p = strSubject.rfind(",");
	if (p)
	{
		char szID[SIP_HEADER_LEN] = { 0 };
		strcpy(szID, p + 1);
		(strchr(szID, ':'))[0] = '\0';
		if (strlen(szID) < outlen)
		{
			memset(outstr, 0, outlen);
			strcpy(outstr, szID);
			return outstr;
		}
	}
	return 0;
}

const char *sip_get_subject_receiverid(char *outstr, int outlen, const char * msg)
{
	acl::string strSubject = sip_get_header(outstr, outlen, msg, SIP_H_SUBJECT);
	if (strSubject == "")
		return "";
	char *p = strSubject.rfind(":");
	if (strlen(p+1) < outlen)
	{
		memset(outstr, 0, outlen);
		strcpy(outstr, p + 1);
		return outstr;
	}
	return 0;
}

int sip_get_content_length(const char * msg)
{
	acl::string strMsg = msg;
	char szContent[SIP_HEADER_LEN] = { 0 };
	sip_get_header(szContent, sizeof(szContent), strMsg.c_str(), SIP_H_CONTENT_LENGTH);
	if (szContent[0] != '\0')
	{
		return atoi(szContent);
	}
	return 0;
}

SIP_CONTENT_TYPE sip_get_body(const char * msg, char *outstr, int outlen)
{
	acl::string strMsg = msg;
	char *sBody = NULL;
	SIP_CONTENT_TYPE content_type = sip_get_content_type(msg);
	switch (content_type)
	{
	case SIP_APPLICATION_XML:
		sBody = strMsg.find("<?xml version=");
		break;
	case SIP_APPLICATION_SDP:
		sBody = strMsg.find("v=");
		break;
	case SIP_APPLICATION_RTSP:
		sBody = strMsg.find("RTSP/1.0");
		break;
	default:
		return SIP_APPLICATION_UNKNOWN;
	}
	memset(outstr, 0, outlen);
	if (sBody && strlen(sBody) < outlen)
	{
		strcpy(outstr, sBody);
		return content_type;
	}
	return SIP_APPLICATION_UNKNOWN;
}

bool sip_get_sdp_info(const char * sdp, sip_sdp_info * info)
{
	memset(info, 0, sizeof(sip_sdp_info));
	acl::string strSdp = sdp;
	acl::string strInfo;
	while (strSdp.scan_line(strInfo, true, NULL))
	{
		if (strInfo.ncompare("o=",strlen("o=")) == 0)
		{
			acl::string strO = strInfo.right(1);
			int nPos = strO.find(' ');
			acl::string strUser = strO.left(nPos);
			strcpy(info->o_user, strUser.c_str());
			char *p = strO.find("IP4 ");
			if (p)
			{
				p += strlen("IP4 ");
				strcpy(info->o_ip, p);
			}
		}
		else if (strInfo.ncompare("c=", strlen("c=")) == 0)
		{
			char *p = strInfo.find("IP4 ");
			if (p)
			{
				p += strlen("IP4 ");
				strcpy(info->o_ip, p);
			}
		}
		else if (strInfo.ncompare("s=", strlen("s=")) == 0)
		{
			acl::string strS = strInfo.right(1);
			if (strS.compare("Play", false) == 0)
				info->s_type = SIP_INVITE_PLAY;
			else if (strS.compare("Playback", false) == 0)
				info->s_type = SIP_INVITE_PLAYBACK;
			else if (strS.compare("Download", false) == 0)
				info->s_type = SIP_INVITE_DOWNLOAD;
			else if (strS.compare("Talk", false) == 0)
				info->s_type = SIP_INVITE_TALK;
			else
				info->s_type = SIP_INVITE_MEDIA;
		}
		else if (strInfo.ncompare("t=", strlen("t=")) == 0)
		{
			acl::string strT = strInfo.right(1);
			sscanf(strT.c_str(), "%lld %lld", &info->t_begintime, &info->t_endtime);
		}
		else if (strInfo.ncompare("m=", strlen("m=")) == 0)
		{
			acl::string strM = strInfo.right(1);
			std::vector<acl::string> vecM = strM.split2(" ");
			if (vecM.size() < 3)
			{
				memset(info, 0, sizeof(sip_sdp_info));
				return false;
			}
			info->m_type = SIP_MEDIA_VIDEO;
			if (vecM[0].compare("audio") == 0)
			{
				info->m_type = SIP_MEDIA_AUDIO;
			}
			info->m_port = atoi(vecM[1].c_str());
			info->m_protocol = SIP_TRANSFER_UDP;
			if (vecM[2].find("TCP",false))
			{
				if (info->m_protocol == 0)
				{
					info->m_protocol = SIP_TRANSFER_TCP_ACTIVE;
				}
			}
		}
		else if (strInfo.ncompare("a=", strlen("a=")) == 0)
		{
			acl::string strA = strInfo.right(1);
			if (strA.compare("sendonly", false) == 0)
			{
				info->a_direct = SIP_SENDONLY;
			}
			else if (strA.compare("recvonly", false) == 0)
			{
				info->a_direct = SIP_RECVONLY;
			}
			else if (strA.compare("sendrecv", false) == 0)
			{
				info->a_direct = SIP_SENDRECV;
			}
			else
			{
				if (strA.ncompare("rtpmap:", strlen("rtpmap:")) == 0)
				{
					strA = strA.right(strlen("rtpmap:")-1);
					if (info->a_playload == 0)
					{
						info->a_playload = atoi(strA.c_str());
					}
					if (info->a_streamtype[0] == '\0')
					{
						acl::string strType = get_msg_substr(strA.c_str(), " ", "/");
						strcpy(info->a_streamtype, strType.c_str());
					}
				}
				else if (strA.ncompare("setup:", strlen("setup:")) == 0)
				{
					strA = strA.right(strlen("setup:")-1);
					if (strA.compare("passive", false) == 0)
					{
						info->m_protocol = SIP_TRANSFER_TCP_PASSIVE;
					}
				}
				else if (strA.ncompare("downloadspeed:", strlen("downloadspeed:")) == 0)
				{
					strA = strA.right(strlen("downloadspeed:")-1);
					info->a_downloadspeed = atoi(strA.c_str());
				}
				else if (strA.ncompare("filesize:", strlen("filesize:")) == 0)
				{
					strA = strA.right(strlen("filesize:")-1);
					long long lSize = atoll(strA.c_str());
					if (lSize > 0)
					{
						info->a_filesize = (size_t)lSize;
					}
				}
			}
		}
		else if (strInfo.ncompare("y=", strlen("y=")) == 0)
		{
			acl::string strY = strInfo.right(1);
			strcpy(info->y_ssrc, strY.c_str());
		}
		else if (strInfo.ncompare("u=", strlen("u=")) == 0)
		{
			acl::string strU = strInfo.right(1);
			strcpy(info->u_uri, strU.c_str());
		}
		else if (strInfo.ncompare("f=", strlen("f=")) == 0)
		{
			acl::string strF = strInfo.right(1);
			strcpy(info->f_parm, strF.c_str());
		}
		strInfo.clear();
	}
	return true;
}

int sip_add_via(char * msg, int msglen, const char * via)
{
	acl::string strHeader;
	strHeader.format("Via: %s%s", via, Line);
	acl::string strMsg(msg, msglen);
	char *sFnd = strMsg.find("Via: SIP/2.0/UDP ");
	if (sFnd == NULL)
		return 0;
	acl::string strAfterBody = sFnd;
	sFnd[0] = '\0';
	strMsg.format("%s%s%s", strMsg.c_str(), strHeader.c_str(), strAfterBody.c_str());
	if (strlen(strMsg.c_str()) < msglen)
	{
		memset(msg, 0, msglen);
		strcpy(msg, strMsg.c_str());
		return (int)strlen(strMsg.c_str());
	}
	return 0;
}

int sip_set_start_line_uri(char * msg, int msglen, const char * uri)
{
	acl::string strMsg(msg, msglen);
	char *sFnd = strMsg.find("sip:");
	if (sFnd == NULL)
		return 0;
	char *sEnd = strstr(sFnd, "SIP/2.0");
	if (sEnd == NULL)
		return 0;
	sFnd[0] = '\0';
	strMsg.format("%s%s %s", strMsg.c_str(), uri, sEnd);
	if (strlen(strMsg.c_str()) < msglen)
	{
		memset(msg, 0, msglen);
		strcpy(msg, strMsg.c_str());
		return (int)strlen(strMsg.c_str());
	}
	return 0;
}

int sip_set_via_addr(char * msg, int msglen, const char * addr)
{
	acl::string strMsg(msg, msglen);
	char *sFnd = strMsg.find("Via: SIP/2.0/UDP ");
	if (sFnd == NULL)
		return 0;
	sFnd += strlen("Via: SIP/2.0/UDP ");
	char *sEnd = strchr(sFnd, ';');
	if (sEnd == NULL)
		return 0;
	sFnd[0] = '\0';
	strMsg.format("%s%s%s", strMsg.c_str(), addr, sEnd);
	if (strlen(strMsg.c_str()) < msglen)
	{
		memset(msg, 0, msglen);
		strcpy(msg, strMsg.c_str());
		return (int)strlen(strMsg.c_str());
	}
	return 0;
}

int sip_set_from_uri(char * msg, int msglen, const char * uri)
{
	acl::string strMsg(msg, msglen);
	char *sFnd = strMsg.find("From: <");
	if (sFnd == NULL)
		return 0;
	sFnd += strlen("From: <");
	char *sEnd = strchr(sFnd, '>');
	if (sEnd == NULL)
		return 0;
	sFnd[0] = '\0';
	strMsg.format("%s%s%s", strMsg.c_str(), uri, sEnd);
	if (strlen(strMsg.c_str()) < msglen)
	{
		memset(msg, 0, msglen);
		strcpy(msg, strMsg.c_str());
		return (int)strlen(strMsg.c_str());
	}
	return 0;
}

int sip_set_to_uri(char * msg, int msglen, const char * uri)
{
	acl::string strMsg(msg, msglen);
	char *sFnd = strMsg.find("To: <");
	if (sFnd == NULL)
		return 0;
	sFnd += strlen("To: <");
	char *sEnd = strchr(sFnd, '>');
	if (sEnd == NULL)
		return 0;
	sFnd[0] = '\0';
	strMsg.format("%s%s%s", strMsg.c_str(), uri, sEnd);
	if (strlen(strMsg.c_str()) < msglen)
	{
		memset(msg, 0, msglen);
		strcpy(msg, strMsg.c_str());
		return (int)strlen(strMsg.c_str());
	}
	return 0;
}

int sip_set_user_agent(char * msg, int msglen, const char * name)
{
	acl::string strMsg(msg, msglen);
	char szAgent[SIP_HEADER_LEN] = { 0 };
	if (sip_get_header(szAgent, sizeof(szAgent), strMsg.c_str(), SIP_H_USER_AGENT))
	{
		if (strcmp(szAgent, name) == 0)
			return (int)strlen(msg);
	}
	return sip_set_header(msg, msglen, SIP_H_USER_AGENT, name);
}

int sip_set_header(char * msg, int msglen, const char * head, const char * value)
{
	acl::string strMsg(msg,msglen);
	acl::string strHeader;
	strHeader.format("%s%s%s", head, value, Line);
	char *sFnd = strMsg.find(head);
	if (sFnd)
	{
		acl::string strAfterBody = sFnd;
		sFnd[0] = '\0';
		sFnd = strAfterBody.find(Line);
		if (sFnd == NULL)
		{
			return 0;
		}
		sFnd += strlen(Line);
		strAfterBody = sFnd;
		strMsg.format("%s%s%s", strMsg.c_str(), strHeader.c_str(), strAfterBody.c_str());
	}
	else
	{
		sFnd = strMsg.find("Content-Length: ");
		if (sFnd == NULL)
		{
			return 0;
		}
		acl::string strAfterBody = sFnd;
		sFnd[0] = '\0';
		strMsg = strMsg.c_str();
		strMsg.append(strHeader);
		strMsg.append(strAfterBody);
	}
	if (strlen(strMsg.c_str()) < msglen)
	{
		memset(msg, 0, msglen);
		strcpy(msg, strMsg.c_str());
		return (int)strlen(strMsg.c_str());
	}
	return 0;
}

int sip_set_content_length(char * msg, int msglen, int length)
{
	acl::string strLength;
	strLength.format("%d", length);
	return sip_set_header(msg, msglen, SIP_H_CONTENT_LENGTH, strLength.c_str());
}

int sip_set_body(char * msg, int msglen, const char * body, SIP_CONTENT_TYPE type)
{
	acl::string sContentType;
	if (type == SIP_APPLICATION_XML)
		sContentType = "Application/MANSCDP+XML";
	else if (type == SIP_APPLICATION_SDP)
		sContentType = "APPLICATION/SDP";
	else if (type == SIP_APPLICATION_RTSP)
		sContentType = "Application/MANSRTSP";
	else
		return msglen;
	size_t nBodylen = strlen(body);
	int nLen = sip_set_header(msg, msglen, SIP_H_CONTENT_TYPE, sContentType.c_str());
	acl::string strMsg(msg, nLen);
	char *sBodylen = strMsg.rfind("0");
	sBodylen[0] = '\0';
	acl::string strReq;
	strReq.format("%s%d%s%s%s", strMsg.c_str(), nBodylen, Line, Line, body);
	if (strReq.size() < msglen)
	{
		memset(msg, 0, msglen);
		strcpy(msg, strReq.c_str());
		return (int)strReq.size();
	}
	return 0;
}

int sip_set_xml_sn(char * body, int bodylen, const char * sn)
{
	acl::string strMsg(body, bodylen);
	char *sFnd = strMsg.find("<SN>");
	if (sFnd == NULL)
		return 0;
	sFnd += strlen("<SN>");
	char *sEnd = strstr(sFnd, "</SN>");
	if (sEnd == NULL)
		return 0;
	sFnd[0] = '\0';
	strMsg.format("%s%s%s", strMsg.c_str(), sn, sEnd);
	if (strlen(strMsg.c_str()) < bodylen)
	{
		memset(body, 0, bodylen);
		strcpy(body, strMsg.c_str());
		return (int)strlen(strMsg.c_str());
	}
	return 0;
}

int sip_set_sdp_ip(char *body, int bodylen, const char * ip)
{
	acl::string strMsg(body, bodylen);
	acl::string strAfter,strEnd;
	char *sFnd = strMsg.find("IN IP4 ");
	if (sFnd == NULL)
		return 0;
	sFnd += strlen("IN IP4 ");
	char *sEnd = strstr(sFnd, Line);
	strEnd = sEnd;
	sFnd[0] = '\0';
	sFnd = strEnd.find("IN IP4 ");
	if (sFnd)
	{
		sFnd += strlen("IN IP4 ");
		strAfter = sFnd;
		sFnd[0] = '\0';
		strMsg.format("%s%s%s", strMsg.c_str(), ip, strEnd.c_str());
		sEnd = strAfter.find(Line);
	}
	strMsg.format("%s%s%s", strMsg.c_str(), ip, sEnd);
	if (strlen(strMsg.c_str()) < bodylen)
	{
		memset(body, 0, bodylen);
		strcpy(body, strMsg.c_str());
		return (int)strlen(strMsg.c_str());
	}
	return 0;
}

int sip_set_sdp_s(char * body, int bodylen, SIP_INVITE_TYPE type)
{
	acl::string strMsg(body, bodylen);
	acl::string strType;
	switch (type)
	{
	case SIP_INVITE_PLAY:
		strType.format("s=Play%s", Line);
		break;
	case SIP_INVITE_PLAYBACK:
		strType.format("s=Playback%s", Line);
		break;
	case SIP_INVITE_DOWNLOAD:
		strType.format("s=Download%s", Line);
		break;
	case SIP_INVITE_TALK:
		strType.format("s=Talk%s", Line);
		break;
	case SIP_INVITE_MEDIASERVER:
		strType.format("s=MediaServer%s", Line);
		break;
	}
	char *sFnd = strMsg.find("s=");
	if (sFnd)
	{
		acl::string strAfter = sFnd;
		sFnd[0] = '\0';
		sFnd = strAfter.find(Line);
		if (sFnd)
		{
			sFnd += strlen(Line);
			strAfter = sFnd;
			strMsg.format("%s%s%s", strMsg.c_str(), strType.c_str(), strAfter.c_str());
		}
	}
	if (strlen(strMsg.c_str()) < bodylen)
	{
		memset(body, 0, bodylen);
		strcpy(body, strMsg.c_str());
		return (int)strlen(strMsg.c_str());
	}
	return 0;
}

int sip_set_sdp_t(char * body, int bodylen, const char * time)
{
	acl::string strMsg(body, bodylen);
	acl::string strTime;
	strTime.format("t=%s%s", time, Line);
	char *sFnd = strMsg.find("t=");
	if (sFnd)
	{
		acl::string strAfter = sFnd;
		sFnd[0] = '\0';
		sFnd = strAfter.find(Line);
		if (sFnd)
		{
			sFnd += strlen(Line);
			strAfter = sFnd;
			strMsg.format("%s%s%s", strMsg.c_str(), strTime.c_str(), strAfter.c_str());
		}
	}
	if (strlen(strMsg.c_str()) < bodylen)
	{
		memset(body, 0, bodylen);
		strcpy(body, strMsg.c_str());
		return (int)strlen(strMsg.c_str());
	}
	return 0;
}

int sip_set_sdp_y(char *msg, int msglen, const char * ssrc)
{
	acl::string strSsrc;
	strSsrc.format("y=%s%s", ssrc, Line);
	acl::string strMsg(msg, msglen);
	char *sFnd = strMsg.find(SIP_H_CONTENT_LENGTH, false);
	if (sFnd)
	{
		int nBodyLen = sip_get_content_length(msg);
		int nMsgLen = sip_set_content_length(msg, msglen, nBodyLen + (int)strlen(strSsrc));
		strMsg = msg;
	}
	sFnd = strMsg.find("y=");
	if (sFnd)
	{
		acl::string strAfter = sFnd;
		sFnd[0] = '\0';
		sFnd = strAfter.find(Line);
		if (sFnd)
		{
			sFnd += strlen(Line);
			strAfter = sFnd;
			strMsg.format("%s%s%s", strMsg.c_str(), strSsrc.c_str(), strAfter.c_str());
		}
	}
	else
	{
		strMsg.format("%s%s", strMsg.c_str(), strSsrc.c_str());
	}
	if (strlen(strMsg.c_str()) < msglen)
	{
		memset(msg, 0, msglen);
		strcpy(msg, strMsg.c_str());
		return (int)strlen(strMsg.c_str());
	}
	return 0;
}

int sip_set_sdp_f(char * msg, int msglen, int encode, int resolution, int frame, int bitrate, int kbps, int voice, int bitsize, int samples)
{
	acl::string strEncode, strResolution, strFrame, strBitrate, strKbps, strVoice, strBitszie, srtSamples, strF;
	if (encode > 0)
		strEncode.format("%d", encode);
	if (resolution > 0)
		strResolution.format("%d", resolution);
	if (frame > 0)
		strFrame.format("%d", frame);
	if (bitrate > 0)
		strBitrate.format("%d", bitrate);
	if (kbps > 0)
		strKbps.format("%d", kbps);
	if (voice > 0)
		strVoice.format("%d", voice);
	if (bitsize > 0)
		strBitszie.format("%d", bitsize);
	if (samples > 0)
		srtSamples.format("%d", samples);
	acl::string strSdp(msg, msglen);
	strF.format("f=v/%s/%s/%s/%s/%sa/%s/%s/%s%s", strEncode.c_str(), strResolution.c_str(), strFrame.c_str(), strBitrate.c_str(), strKbps.c_str(), strVoice.c_str(), strBitszie.c_str(), srtSamples.c_str(), Line);
	char *sFnd = strSdp.find(SIP_H_CONTENT_LENGTH, false);
	if (sFnd)
	{
		int nBodyLen = sip_get_content_length(msg);
		int nMsgLen = sip_set_content_length(msg, msglen, nBodyLen + (int)strlen(strF));
		strSdp = msg;
	}
	sFnd = strSdp.find("f=");
	if (sFnd)
	{
		acl::string strAfter = sFnd;
		sFnd[0] = '\0';
		sFnd = strAfter.find(Line);
		if (sFnd)
		{
			sFnd += strlen(Line);
			strAfter = sFnd;
			strSdp.format("%s%s%s", strSdp.c_str(), strF.c_str(), strAfter.c_str());
		}
	}
	else
	{
		strSdp.format("%s%s", strSdp.c_str(), strF.c_str());
	}
	if (strlen(strSdp.c_str()) < msglen)
	{
		memset(msg, 0, msglen);
		strcpy(msg, strSdp.c_str());
		return (int)strlen(strSdp.c_str());
	}
	return 0;
}










