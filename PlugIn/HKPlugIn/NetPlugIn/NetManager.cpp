#include "StdAfx.h"
#include "NetManager.h"

NetManager::NetManager(void)
{
	nCount = 0;
	nHandler = -1;
	InitializeCriticalSection(&m_lock);
	
}

NetManager::~NetManager(void)
{
	DeleteCriticalSection(&m_lock);
}

int NetManager::connect(const char *ip,int port,const char *user,const char *pwd)
{
	EnterCriticalSection(&m_lock);
	if (nHandler != -1)
	{
		nCount++;
		printf("##Login Users :%d\n",nCount);
		LeaveCriticalSection(&m_lock);
		return nHandler;
	}
	//��ʼ��

	//��½


	nCount++;
	printf("��½�ɹ�! User[%d]\n", nCount);
	LeaveCriticalSection(&m_lock);
	return nHandler;
}

void NetManager::disconnect()
{
	EnterCriticalSection(&m_lock);
	if (nCount > 0)
	{
		nCount--;
		printf("##Logout Users:%d\n",nCount);
		if (nCount <=0)
		{
			printf("�˳��ɹ�! Users[%d]\n",nCount);
			nCount = 0;
			nHandler = -1;
		}
	}
	LeaveCriticalSection(&m_lock);
}
