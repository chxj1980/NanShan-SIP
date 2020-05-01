
#ifndef WriteLog_h__
#define WriteLog_h__
#include <string>
#include<fstream>


#ifdef _WIN32
#include <tchar.h>

#else

#include <pthread.h>
typedef pthread_mutex_t   CRITICAL_SECTION;
#define EnterCriticalSection	pthread_mutex_lock
#define LeaveCriticalSection	pthread_mutex_unlock
#define DeleteCriticalSection   pthread_mutex_destroy
#define InitializeCriticalSection(a) pthread_mutex_init(a, NULL)

#define _stat stat
#define Sleep(x) usleep((x)*1000)

#include <stdarg.h>
#define  _vsnprintf vsnprintf

#define _T(x)  x

#endif

#define LOG_INFO    1
#define LOG_WARNING 2
#define LOG_ERROR   3
using namespace std;
class WriteLog
{
public:
    WriteLog(void);
    ~WriteLog(void);

public:
    bool SetFileName(string sFileName);

    //设置日志文件的大小
    void SetFileSize(int iFileSize) {m_iSize = iFileSize;};

    //设置日志输出的最低级别
    void SetLogLevel(int iLevel){m_nLogLevel = iLevel;};

    //日志输出接口
    bool LogInfo(int nLevel, string strContent);
    bool LogInfoEx(int nLevel, const char* sLogText , ...);

private:
    int     m_nLogLevel;
    bool    m_bRebuild;
    //CFile   m_fileLog;
    int     m_nWritefileLogTimes;
    string  m_sFileName;
    int     m_iSize;
    fstream m_fileLog;

public:
    static CRITICAL_SECTION m_mutex;
    static string FormStr(const char* sStr , ...);
};


class ForLock
{
public:
    ForLock(){InitializeCriticalSection(&WriteLog::m_mutex);};
    ~ForLock(){DeleteCriticalSection(&WriteLog::m_mutex);};
};

#endif // WriteLog_h__
