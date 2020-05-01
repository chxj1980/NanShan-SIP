#pragma once

typedef struct PacketHeader
{
	char iFrameType; //0 视频， 1 音频
	char iMediaType; //0 非关键帧, 1 关键帧, 2 AAC
	unsigned int nTimeStamp;
}PH,*LPPH;

class NetManager
{
public:
	NetManager(void);
public:
	~NetManager(void);
public:
	int connect(const char *ip,int port,const char *user,const char *pwd);
	void disconnect();
private:
	int nCount;
	int nHandler;

	CRITICAL_SECTION m_lock;
};
