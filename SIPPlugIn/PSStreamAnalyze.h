#pragma once

#include "libsip.h"
#include "NVSSDKDef.h"
#include "libHEVCtoAVC.h"
#ifdef _HEVC2AVC_USE
#pragma comment(lib, "libHEVCtoAVC.lib")
#endif

#define RTP_BUF_SIZE		1024*100
#define FRAME_BUF_SIZE		1024*1024*2

class PSStreamAnalyze :
	public acl::thread
{
public:
	//nStreamType: 0 -自解析 96 -H264 97 -H265
	PSStreamAnalyze(int nSteamType, LPCAPTURESTREAMCALLBACK pCb, void *pUser);
	~PSStreamAnalyze();
public:
	void add_data(unsigned char *pBuffer, unsigned int nBufferSize);
	void stop();
protected:
	virtual void *run();
private:
	void analyze_h264(unsigned char *pBuffer, unsigned int nBufferSize);
	void analyze_h265(unsigned char *pBuffer, unsigned int nBufferSize);
	static void __stdcall VideoCodingFrameCb(unsigned char * pBuf, long nSize, long tv_sec, long tv_usec, void* pUser);
private:
	unsigned char *			m_szFrameData;	//当前帧缓冲
	unsigned int			m_nFrameSize;	//当前帧缓冲大小
	unsigned int			m_nTimestamp;	//当前帧时间戳
	int						m_nfreq;		//时钟频率
	int						m_nStreamType;	//码流类型
	LPCAPTURESTREAMCALLBACK m_pStreamCb;	//码流回调
	void*					m_pStreamUser;	//码流回调用户
	ACL_FIFO *				m_pFifo;		//码流缓冲队列
	acl::locker				m_lockerFifo;	//队列锁
	Long					m_pCodec;		//转码句柄
	int						m_nUsec;		//帧速率
	bool					m_bFirstData;	//第一包
	bool					m_bStop;		//停止解析
	FILE *					m_file;
};


