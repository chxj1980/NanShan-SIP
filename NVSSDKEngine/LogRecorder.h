#ifndef CLOG_RECORDER_H_
#define CLOG_RECORDER_H_

#include <log4cxx\log4cxx.h>
#include <log4cxx\propertyconfigurator.h>
#include <log4cxx\xml\domconfigurator.h>
#include <log4cxx\helpers\loglog.h>

using namespace std;
using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace log4cxx::xml;

/**
* 权限级别：分别对应Log4cxx中的：TRACE, DEBUG, INFO, WARN, ERROR, FATAL
* 使用建议：1.常用的权限级别为DEBUG, INFO, WARN, ERROR
*			2.对于在调试时与系统上线试运行阶段需要记录的信息，采用DEBUG级别
*			3.对于在系统在运行时也需要记录的日志信息，采用INFO级别
*			4.对于可能存在问题的地方，采用WARN级别，并输出可能导致出现问题的信息
*			5.对于发生错误的信息，采用WARN级别
*/
enum LoggerLevel{TRACELEVEL, DEBUGLEVEL, INFOLEVEL, WARNLEVEL, ERRORLEVEL, FATALLEVEL};

class CLogRecorder
{
public:
	CLogRecorder(){ m_logger = NULL; }
	~CLogRecorder() {}

	// 初始化Logger
	bool InitLogger(const char *pszLogConfigPath, const char *pszLoggerName, const char *pszModuleName);
	// 日志记录的基函数，所有其他函数都通过它来实现
	void WriteLog(const char *pszFunction, const char *pszLogContent, LoggerLevel nLevel=INFOLEVEL);
	// 普通的日志记录函数
	void WriteDebugLog(const char *pszFunction, const char *pszLogContent);
	void WriteInfoLog(const char *pszFunction, const char *pszLogContent);
	void WriteWarnLog(const char *pszFunction, const char *pszLogContent);
	void WriteErrorLog(const char *pszFunction, const char *pszLogContent);
	void WriteFatalLog(const char *pszFunction, const char *pszLogContent);
	
	// 扩展的日志记录函数，支持可变参数，更加方便和快捷，建议使用此组函数
	void WriteLogEx(LoggerLevel nLevel, const char *pszFunction, const char *pszFormat, ...);
	void WriteDebugLogEx(const char *pszFunction,const char *pszFormat, ...);
	void WriteInfoLogEx(const char *pszFunction, const char *pszFormat, ...);
	void WriteWarnLogEx(const char *pszFunction, const char *pszFormat, ...);
	void WriteErrorLogEx(const char *pszFunction, const char *pszForamt, ...);
	void WriteFatalLogEx(const char *pszFunction, const char *pszFormat, ...);

private:
	char m_szModuleName[MAX_PATH];	// 模块名字，日志的记录以模块为单位
	LoggerPtr m_logger;	// 日志记录器的名称
};

#endif	// CLOG_RECORDER_H_