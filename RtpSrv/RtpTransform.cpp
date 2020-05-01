#include "stdafx.h"
#include "RtpTransform.h"


RtpTransform::RtpTransform(ChannelInfo *pChn, void* rtp)
{
	rtpsrv = (RtpService *)rtp;
	running = true;
	activate = false;
	connectd = false;
	rtp_seq = 1;
	rtp_v_timestamp = GetTickCount();
	rtp_a_timestamp = GetTickCount();
	rtp_time = 0;
	rtcp_time = time(0);
	invite_type = 0;
	media_type = SIP_MEDIA_VIDEO;
	playload = 0;
	stream_handle = 0;
	memset(rtp_callid, 0, sizeof(rtp_callid));
	memset(rtp_deviceid, 0, sizeof(rtp_deviceid));
	memset(rtp_data, 0, sizeof(rtp_data));
	memset(ps_data, 0, sizeof(ps_data));
	memset(rtp_stream_type, 0, sizeof(rtp_stream_type));
	memset(rtp_stream_ssrc, 0, sizeof(rtp_stream_ssrc));
	set_channelinfo(pChn);
}


RtpTransform::~RtpTransform()
{

}

void * RtpTransform::run()
{
	while (running)
	{
		if (stream_handle < 0)		//取流失败
		{
			printf("[%s][%d]媒体流取流失败!\n", rtp_deviceid, invite_type);
			cls_client();	//通知所有会话关闭
			stream_handle = 0;
		}
		if (rtp_time > 0 && time(0) - rtp_time > 10 && activate) //收流超时
		{
			printf("[%s][%d]媒体流接收超时!\n", rtp_deviceid,invite_type);
			cls_client();	//通知所有会话关闭
		}
		sleep(1);
	}
	//统一由stop()控制delete
	delete this;
	return nullptr;
}

void RtpTransform::stop()
{
	running = false;
}

void RtpTransform::dormant()
{
	if (stream_handle > 0)
	{
		if (invite_type == SIP_INVITE_PLAY)
		{
			stopCaptureStream(stream_handle);
		}
		disconnectDVR(stream_handle);
		stream_handle = 0;
	}
	rtp_time = 0;
	connectd = false;
	activate = false;
}

bool RtpTransform::get_status()
{
	return connectd;
}

int RtpTransform::get_playload()
{
	return playload;
}

char * RtpTransform::get_stream_type()
{
	return rtp_stream_type;
}

char * RtpTransform::get_stream_ssrc()
{
	return rtp_stream_ssrc;
}

void RtpTransform::set_stream_ssrc(const char * ssrc)
{
	memset(rtp_stream_ssrc, 0, sizeof(rtp_stream_ssrc));
	strcpy(rtp_stream_ssrc, ssrc);
}

void RtpTransform::set_channelinfo(ChannelInfo * pChn)
{
	memset(&chninfo, 0, sizeof(ChannelInfo));
	memcpy(&chninfo, pChn, sizeof(ChannelInfo));
	strcpy(rtp_stream_type, chninfo.szManufacturer);
	stream_handle = connectDVR(chninfo.szIPAddress, chninfo.nPort, chninfo.szUser, chninfo.szPwd, chninfo.nChannel, chninfo.szModel, connectd);
	//printf("stream_handle %p\n", stream_handle);
	//activate = connectd ? true : false;
}

void RtpTransform::set_playinfo(const char * callid, const char * deviceid, int playtype)
{
	strcpy(rtp_callid, callid);
	strcpy(rtp_deviceid, deviceid);
	invite_type = playtype;
	if (connectd && stream_handle>0)
	{
		MediaInf mInf = { 0 };
		if (GetMediaInfo(stream_handle, mInf))
		{
			playload = atoi(mInf.fileformat);
			memcpy(rtp_stream_type, mInf.codeType, strlen(mInf.codeType));
		}
	}
}

void RtpTransform::set_mediainfo(addr_info * addr)
{
	if (invite_type == SIP_INVITE_PLAY)
	{
		if (stream_handle > 0 && !activate)
		{
			if (strcmp(chninfo.szModel,"HK") == 0)
                activate = captureStream(stream_handle, stream_cb, this, STREAM_CALLBACK_STANDARD);
			else
                activate = captureStream(stream_handle, stream_cb, this, STREAM_CALLBACK_FRAME);
		}

        if (!activate)
        {
            cls_client();
        }
	}
}

int RtpTransform::add_client(const char * callid, rtp_sender * sender)
{
	_client_locker.lock();
	_client_list[callid] = sender;
	_client_locker.unlock();
	if (sender && stream_handle>0 && connectd)
	{
		SetStreamSaveTime(stream_handle, sender->sendssrc);
	}
	return (int)_client_list.size();
}

int RtpTransform::del_client(const char * callid)
{
	_client_locker.lock();
	std::map<std::string, rtp_sender *>::iterator itor = _client_list.find(callid);
	if (itor != _client_list.end())
	{
		_client_list.erase(itor);
	}
	_client_locker.unlock();
	return (int)_client_list.size();
}

int RtpTransform::client_size()
{
	return (int)_client_list.size();
}

bool RtpTransform::tranfer_data(unsigned char * buffer, size_t bufsize, int datatype)
{
	bool bSendOK = false;
	rtp_time = time(0);
	if (_client_list.size() == 0)
		return true;
	_client_locker.lock();
	std::map<std::string, rtp_sender *>::iterator itor = _client_list.begin();
	while (itor != _client_list.end())
	{
		if (!running)
		{
			_client_locker.unlock();
			return false;
		}
		rtp_sender *pSend = itor->second;
		if (pSend)
		{
			if (datatype == DATATYPE_STREAM)
				bSendOK = pSend->send_data(buffer, bufsize) == -1 ? false : true;
			else
			{
				if (pSend->sendaddr.transfrom)
					bSendOK = tranfer_ps_data(buffer, bufsize, datatype, pSend);
				else
					bSendOK = tranfer_es_data(buffer, bufsize, datatype, pSend);
			}
			if (!bSendOK)
			{
				//printf("接收端[%s]异常中断!\n", pSend->sendaddr.addr);
				rtpsrv->session_over(itor->first.c_str(), SESSION_RECVER_BYE);
				itor = _client_list.erase(itor);
				if (_client_list.size() == 0)
					break;
				else
					continue;
			}
			itor++;
		}
		else
		{
			itor = _client_list.erase(itor);
		}
	}
	_client_locker.unlock();
	return true;
}

bool RtpTransform::tranfer_ps_data(unsigned char * buffer, size_t bufsize, int datatype, rtp_sender * sender)
{
	unsigned char *pBuffer = NULL;
	unsigned long cur_timestamp = 0;
	unsigned int rtp_data_len = 0;
	unsigned int ps_data_len = 0;
	int ps_stream_id = PS_STREAMID_VIDEO;
	int ps_stream_type = 0;
	bool iframe = false;
	if (datatype == DATATYPE_FRAME_A)
	{
		rtp_v_timestamp += 320;
		cur_timestamp = rtp_v_timestamp;
	}
	else
	{
		rtp_a_timestamp += 3600;
		cur_timestamp = rtp_a_timestamp;
	}
	if (playload == SIP_STREAM_PS)
	{
		ps_data_len = (unsigned int)bufsize;
		pBuffer = buffer;
	}
	else
	{
		if (playload == SIP_STREAM_H264)
			ps_stream_type = PS_STREAMTYPE_H264;
		else if (playload == SIP_STREAM_H265)
			ps_stream_type = PS_STREAMTYPE_H265;
		else if (playload == SIP_STREAM_SVAC)
			ps_stream_type = PS_STREAMTYPE_SVAC;
		else
			return false;
		if (datatype == DATATYPE_FRAME_I)
			iframe = true;
		if (datatype == DATATYPE_FRAME_A)
			ps_stream_id = PS_STREAMID_AUDIO;
		if (media_type == SIP_MEDIA_AUDIO)
		{
			ps_stream_id = PS_STREAMID_AUDIO;
			ps_stream_type = PS_STREAMTYPE_G711;
		}
		ps_data_len = ps.Packing(ps_data, buffer, (int)bufsize, ps_stream_id, ps_stream_type, iframe, cur_timestamp);
		pBuffer = ps_data;
	}
	while (ps_data_len > 0)
	{
		if (ps_data_len > RTP_MAX_PACKET_SIZE)
		{
			ps.RTPFormat(rtp_data, SIP_STREAM_PS, 0, rtp_seq++, cur_timestamp, atoi(rtp_stream_ssrc));
			memcpy(rtp_data + RTP_HDR_LEN, pBuffer, RTP_MAX_PACKET_SIZE);
			if (sender->send_data(rtp_data, RTP_HDR_LEN + RTP_MAX_PACKET_SIZE) == -1)
				return false;
			pBuffer += RTP_MAX_PACKET_SIZE;
			ps_data_len -= RTP_MAX_PACKET_SIZE;
		}
		else
		{
			ps.RTPFormat(rtp_data, SIP_STREAM_PS, 1, rtp_seq++, cur_timestamp, atoi(rtp_stream_ssrc));
			memcpy(rtp_data + RTP_HDR_LEN, pBuffer, ps_data_len);
			if (sender->send_data(rtp_data, RTP_HDR_LEN + ps_data_len) == -1)
				return false;
			ps_data_len = 0;
		}
	}
	return true;
}

bool RtpTransform::tranfer_es_data(unsigned char * buffer, size_t bufsize, int datatype, rtp_sender * sender)
{
	unsigned long cur_timestamp = 0;
	unsigned int rtp_data_len = 0;
	if (datatype == DATATYPE_FRAME_A)
	{
		rtp_v_timestamp += 320;
		cur_timestamp = rtp_v_timestamp;
	}
	else
	{
		rtp_a_timestamp += 3600;
		cur_timestamp = rtp_a_timestamp;
	}
	unsigned char *pBuffer = buffer;
	while (bufsize > 0)
	{
		if (bufsize > RTP_MAX_PACKET_SIZE)
		{
			ps.RTPFormat(rtp_data, playload, 0, rtp_seq++, cur_timestamp, atoi(rtp_stream_ssrc));
			memcpy(rtp_data + RTP_HDR_LEN, pBuffer, RTP_MAX_PACKET_SIZE);
			if (sender->send_data(rtp_data, RTP_HDR_LEN + RTP_MAX_PACKET_SIZE) == -1)
				return false;
			pBuffer += RTP_MAX_PACKET_SIZE;
			bufsize -= RTP_MAX_PACKET_SIZE;
		}
		else
		{
			ps.RTPFormat(rtp_data, playload, 1, rtp_seq++, cur_timestamp, atoi(rtp_stream_ssrc));
			memcpy(rtp_data + RTP_HDR_LEN, pBuffer, bufsize);
			if (sender->send_data(rtp_data, RTP_HDR_LEN + bufsize) == -1)
				return false;
			rtp_data_len += RTP_MAX_PACKET_SIZE;
			bufsize = 0;
		}
		
	}
	return true;
}

bool RtpTransform::ctronl_ptz(int nType, int nCmd, int nIndex, int nSpeed)
{
	if (stream_handle>0 && activate)
	{
		if (nType == 0)
		{
			bool bStop = (nCmd == 0) ? true : false;
			return controlPTZSpeed(stream_handle, nCmd, bStop, nSpeed);
		}
		else
		{
			return presetPTZWithChannel(stream_handle, nCmd, nIndex, nSpeed);
		}
	}
	return false;
}

void RtpTransform::cls_client()
{
	_client_locker.lock();
	if (_client_list.size() > 0)
	{
		//printf("释放所有媒体流接收端\n");
		std::map<std::string, rtp_sender *>::iterator itor = _client_list.begin();
		while (itor != _client_list.end())
		{
			rtpsrv->session_over(itor->first.c_str(), SESSION_SMEDIA_BYE);
			itor++;
		}
		_client_list.clear();
	}
	_client_locker.unlock();
	activate = false;
}

void RtpTransform::stream_cb(intptr_t identify, int nDataType, BYTE * pBuffer, int nBufferSize, void * pUser)
{
	RtpTransform *pThd = (RtpTransform *)pUser;
	if (pThd == NULL || !pThd->activate)
		return;
	pThd->tranfer_data(pBuffer, nBufferSize, nDataType);
}
