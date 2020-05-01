#pragma once
#include "RtpThread.h"
#include "PSPackage.h"
#include "NVSSDKFunctions.h"

#define STREAM_CALLBACK_SOURCE		0  //原始数据回调
#define STREAM_CALLBACK_FRAME		1  //帧数据回调
#define STREAM_CALLBACK_STANDARD	2  //RTP+PS回调

#define DATATYPE_STREAM	 2	//媒体流数据
#define DATATYPE_FRAME_A 4	//音频数据
#define DATATYPE_FRAME_I 5  //关键帧数据
#define DATATYPE_FRAME_P 6	//非关键帧数据



class RtpTransform : public acl::thread
{
public:
	RtpTransform(ChannelInfo *pChn, void* rtp);
	~RtpTransform();
protected:
	virtual void *run();
public:
	void stop();
	void dormant();
	bool get_status();
	int get_playload();
	char *get_stream_type();
	char *get_stream_ssrc();
	void set_stream_ssrc(const char *ssrc);
    //connect dvr
	void set_channelinfo(ChannelInfo *pChn);
	void set_playinfo(const char *callid, const char *deviceid, int playtype);
    //capture stream
	void set_mediainfo(addr_info *addr);
	int client_size();
	int add_client(const char *callid, rtp_sender * sender);
	int del_client(const char *callid);
	void cls_client();
	bool tranfer_data(unsigned char *buffer, size_t bufsize, int datatype=DATATYPE_FRAME_P);
	bool tranfer_ps_data(unsigned char *buffer, size_t bufsize, int datatype, rtp_sender *sender);
	bool tranfer_es_data(unsigned char *buffer, size_t bufsize, int datatype, rtp_sender *sender);
	bool ctronl_ptz(int nType, int nCmd, int nIndex, int nSpeed);
public:
	static void CALLBACK stream_cb(intptr_t identify, int nDataType, BYTE* pBuffer, int nBufferSize, void* pUser);
private:
	ChannelInfo chninfo;
	PSPackage ps;
	RtpService *rtpsrv;
	bool running;
	bool activate;
	BOOL connectd;
	unsigned int rtp_seq;
	unsigned long rtp_v_timestamp;
	unsigned long rtp_a_timestamp;
	time_t rtp_time;
	time_t rtcp_time;

	intptr_t stream_handle;
	
	int invite_type;
	int media_type;
	int playload;
	char rtp_stream_type[32];
	char rtp_stream_ssrc[11];
	char rtp_callid[64];
	char rtp_deviceid[21];

	unsigned char rtp_data[1500];
	unsigned char ps_data[1024*1024];
public:
	std::map<std::string, rtp_sender *> _client_list;
	acl::locker							_client_locker;
};

