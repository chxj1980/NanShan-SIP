#include "stdafx.h"
#include "SipThread.h"


scanf_thread::scanf_thread(void *pUser)
{
	m_pSip = (SipService *)pUser;
}
scanf_thread::~scanf_thread()
{
	m_pSip = NULL;
}
void *scanf_thread::run()
{
	if (m_pSip)
		m_pSip->scanf_command();
	delete this;
	return NULL;
}

//任务处理线程池
sip_thread_pool::sip_thread_pool()
{
}
sip_thread_pool::~sip_thread_pool()
{
}
bool sip_thread_pool::thread_on_init()
{
	//printf("thread_on_init curtid: %lu\n", acl::thread::thread_self());
	return true;
}
void sip_thread_pool::thread_on_exit()
{
	//printf("thread_on_exit curtid: %lu\n", acl::thread::thread_self());
}

//任务处理子线程
sip_handle_thread::sip_handle_thread(socket_stream* sock_stream, const char *data, size_t dlen, void *sip)
{
	m_pSip = (SipService *)sip;
	memset(&m_sock_stream, 0, sizeof(m_sock_stream));
	memcpy(&m_sock_stream, sock_stream, sizeof(m_sock_stream));
	memset(m_pData, 0, SIP_RECVBUF_MAX);
	memcpy(m_pData, data, dlen);
	m_nDlen = dlen;
}
sip_handle_thread::~sip_handle_thread()
{
	m_pSip = NULL;
}
void *sip_handle_thread::run()
{
	if (m_pSip)
	{
		m_pSip->on_handle(&m_sock_stream, m_pData, m_nDlen);
	}
	delete this;
	return NULL;
}

//注册管理线程
regmnger_thread::regmnger_thread(void *pUser)
{
	m_pSip = (SipService *)pUser;
}
regmnger_thread::~regmnger_thread()
{
	m_pSip = NULL;
}
void *regmnger_thread::run()
{
	if (m_pSip)
		m_pSip->register_manager();
	delete this;
	return NULL;
}

//会话管理线程
sesmnger_thread::sesmnger_thread(void *pUser)
{
	m_pSip = (SipService *)pUser;
}
sesmnger_thread::~sesmnger_thread()
{
	m_pSip = NULL;
}
void *sesmnger_thread::run()
{
	if (m_pSip)
		m_pSip->session_manager();
	delete this;
	return NULL;
}

//消息管理线程
msgmnger_thread::msgmnger_thread(void *pUser)
{
	m_pSip = (SipService *)pUser;
}
msgmnger_thread::~msgmnger_thread()
{
	m_pSip = NULL;
}
void *msgmnger_thread::run()
{
	if (m_pSip)
		m_pSip->message_manager();
	delete this;
	return NULL;
}

//目录推送线程
catalog_thread::catalog_thread(SipService *sip, session_info *session, int type)
{
	m_pSip = sip;
	m_pSession = session;
	m_nType = type;
}
catalog_thread::~catalog_thread()
{
	m_pSip = NULL;
	m_pSession = NULL;
}
void * catalog_thread::run()
{
	if (m_pSip && m_pSession)
	{
		size_t nSumNum = 0;
		std::vector<CatalogInfo> CatalogList;
		if (m_pSip->get_user_allow_all(m_pSession->userid)) //查询目录权限
		{
#ifdef _DBLIB_USE
			if (strcmp(m_pSession->deviceid, m_pSip->m_conf.local_code) == 0) //查询所有目录
			{
				m_pSip->m_db.GetUserCatalog(CatalogList);
				nSumNum = CatalogList.size();
				if (nSumNum > 0)
					m_pSip->response_catalog_root(m_pSession, (int)nSumNum + 1, m_nType);
			}
			else //查询区域目录
			{
				m_pSip->m_db.GetUserCatalog(CatalogList, m_pSession->deviceid);
				nSumNum = CatalogList.size();
			}
			for (size_t i = 0; i < CatalogList.size(); i++)
			{
				m_pSip->response_catalog_node(m_pSession, &CatalogList[i], (int)nSumNum + 1, m_nType);
				sip_sleep(10);
			}
#endif
		}
		else
		{
			if (m_pSip->get_user_allow_catalog(m_pSession->userid, CatalogList))
			{
				nSumNum = CatalogList.size();
				m_pSip->response_catalog_root(m_pSession, (int)nSumNum + 1, m_nType);
				for (size_t i = 0; i < CatalogList.size(); i++)
				{
					m_pSip->response_catalog_node(m_pSession, &CatalogList[i], (int)nSumNum + 1, m_nType);
					sip_sleep(10);
				}
			}
		}
	}
	delete this;
	return NULL;
}

//目录更新线程
catalog_update::catalog_update(SipService * sip)
{
	m_pSip = sip;
}
catalog_update::~catalog_update()
{
	m_pSip = NULL;
}
void * catalog_update::run()
{
	if (m_pSip)
		m_pSip->catalog_manager();
	delete this;
	return NULL;
}
