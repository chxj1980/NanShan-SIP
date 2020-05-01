#include "stdafx.h"
#include "WriteLog.h"
#include "string"
#include <time.h>
#include <sys/stat.h>

using namespace std;

ForLock GL_ForLock;//专用来初始化互斥量
CRITICAL_SECTION WriteLog::m_mutex;

WriteLog::WriteLog(void)
{
    m_nLogLevel = -1;
    m_bRebuild = false;
    m_nWritefileLogTimes = 0;
    m_iSize = 1024*1;

}

WriteLog::~WriteLog(void)
{
    m_fileLog.close();
}


bool WriteLog::LogInfo(int nLevel, string strContent)
{
    if (nLevel<m_nLogLevel)
    {
        return false;
    }


    try //////////////////////////////////////////
    {

        time_t t = time(0);
        char tmp[64];
        strftime( tmp, sizeof(tmp), "%Y-%m-%d %X >>",localtime(&t));
        char tmp2[64];
        strftime(tmp2, sizeof(tmp2),"%Y%m%d%H",localtime(&t));

        string sLog = "";
        sLog += tmp;

        switch(nLevel)
        {
        case LOG_INFO:
            sLog += "INFO:";
            break;
        case LOG_WARNING:
            sLog += "WARNING:";
            break;
        case LOG_ERROR:
            sLog += "ERROR:";
            break;
        default:
            sLog += "UNKNOW:";
            break;
        }
        sLog += strContent;

        EnterCriticalSection(&m_mutex);

        //重建文件
        if (m_bRebuild)
        {
            m_fileLog.close();
            string sOldFileName = m_sFileName;

            int iPos = (int)sOldFileName.find_last_of(".");
            string sTmp = sOldFileName.substr(0 , iPos);
            sTmp += tmp2;
            sTmp += sOldFileName.substr(iPos , sOldFileName.length() - iPos);
            sOldFileName = sTmp;
            ::rename(m_sFileName.c_str() , sOldFileName.c_str());

            m_bRebuild = false ;
        }

        if (!m_fileLog.is_open())
        {
            m_fileLog.open(m_sFileName.c_str() ,  ios::out |ios::app);
            if (!m_fileLog.is_open())
            {
                LeaveCriticalSection(&m_mutex);
                return false;
            }
        }

        int nLength = sLog.length();
        if (nLength>0)
        {
            if (!(sLog.find_last_of('\n') == sLog.length()-1))
            {
                sLog += "\n";
            }
            nLength = (int)sLog.length();
            m_fileLog.write(sLog.c_str(),nLength);
            m_fileLog.flush();
        }


        m_nWritefileLogTimes++;

        //////检查文件大小
        if (m_nWritefileLogTimes> 100)//每写一百次检查一次
        {
            m_nWritefileLogTimes = 0 ;

            int iSize;
            struct _stat FileInfo;
            int nRet = _stat(m_sFileName.c_str(),&FileInfo);
			if(nRet != 0)
			{
				m_bRebuild = true;
			}
			else
			{
				iSize = FileInfo.st_size;

				if (iSize > m_iSize*1024)
				{
					m_bRebuild = true;
				}
			}
        }

    }///////////////////////////////////////////////
    catch( ... )
    {
        ::Sleep(100);
    }

    LeaveCriticalSection(&m_mutex);
    return true;
}

bool WriteLog::LogInfoEx(int nLevel, const char* sLogText , ...)
{
	if (nLevel < m_nLogLevel)
	{
		return false;
	}
    //拼接参数
    va_list arlist;
    va_start(arlist, sLogText);
    char sTmp[4096];
    memset(sTmp , 0 , 1024);
    _vsnprintf(sTmp, sizeof(sTmp), sLogText, arlist);
    va_end(arlist);

    string sLog = _T(sTmp);
    return LogInfo(nLevel , sLog);
}

bool WriteLog::SetFileName( string sFileName )
{
    m_sFileName = sFileName;

    if (m_fileLog.is_open())
    {
        m_fileLog.close();
    }
    m_fileLog.open(m_sFileName.c_str() ,  ios::out |ios::app);
    if (!m_fileLog.is_open())
    {
        return false;
    }
    return true;
}
