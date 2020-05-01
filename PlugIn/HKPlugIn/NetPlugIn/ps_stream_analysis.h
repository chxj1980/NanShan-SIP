#pragma once
#ifdef _PS_ANALYZE_USE
#include "idvrhandler_x64.h"
#include "EasyAACEncoderAPI.h"
#pragma comment (lib,"libEasyAACEncoder.lib")
#include <queue>
#ifndef byte
typedef unsigned char byte;
#endif

#define	PS_BUF_SZIE		1024*1024
#define	FRAME_BUF_SZIE	1024*1024*2

class ps_stream_analysis
{
public:
	ps_stream_analysis();
	~ps_stream_analysis();
public:
	void set_stream_cb(LPCAPTURESTREAMCALLBACK streamcb, void *user, int streamtype=96);
	void push_data(byte *data, int datalen);
	void start();
	void stop();
	void analysis();
	void analyze_h264(byte *pBuffer, int nBufferSize);
	void analyze_h265(byte *pBuffer, int nBufferSize);
	static DWORD WINAPI analysis_thrd(LPVOID lpThreadParameter);
private:
	LPCAPTURESTREAMCALLBACK m_streamcb;
	void *					m_user;
	CRITICAL_SECTION		m_lock;
	std::queue<byte *>		m_quedata;
	int						m_streamtype;
	byte *					m_szFrameBuf;
	int						m_nFrameBufSize;
	bool					m_bFirstData;
	bool					m_bFirstAudio;
	unsigned int			m_nTimestamp;
	unsigned int			m_nTimeAudio;
	time_t					m_tStreamTimeout;
	void *					m_hEncHandle;
	bool					m_bStop;
};

#endif

