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
	//nStreamType: 0 -�Խ��� 96 -H264 97 -H265
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
	unsigned char *			m_szFrameData;	//��ǰ֡����
	unsigned int			m_nFrameSize;	//��ǰ֡�����С
	unsigned int			m_nTimestamp;	//��ǰ֡ʱ���
	int						m_nfreq;		//ʱ��Ƶ��
	int						m_nStreamType;	//��������
	LPCAPTURESTREAMCALLBACK m_pStreamCb;	//�����ص�
	void*					m_pStreamUser;	//�����ص��û�
	ACL_FIFO *				m_pFifo;		//�����������
	acl::locker				m_lockerFifo;	//������
	Long					m_pCodec;		//ת����
	int						m_nUsec;		//֡����
	bool					m_bFirstData;	//��һ��
	bool					m_bStop;		//ֹͣ����
	FILE *					m_file;
};


