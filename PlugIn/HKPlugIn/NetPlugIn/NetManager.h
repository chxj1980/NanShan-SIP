#pragma once



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
