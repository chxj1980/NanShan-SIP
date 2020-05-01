#include "stdafx.h"
#include "NVSHandlerMgr.h"

#include <fcntl.h>

//#include "xImage.h"

#pragma warning(disable : 4311)
#pragma warning(disable : 4312)

#ifdef _WIN32
//#ifdef _DEBUG
//#define NETPLUGIN "NetPlugind.dll"
//#define PLAYPLUGIN "PlayPlugind.dll"
//#else
#define NETPLUGIN "NetPlugin.dll"
#define PLAYPLUGIN "PlayPlugin.dll"
//#endif
#else
#define NETPLUGIN "NetPlugin.so"
#define PLAYPLUGIN "PlayPlugin.so"
#endif

NVSHandlerMgr::NVSHandlerMgr(void)
{
    strncpy(m_sSystemPath,"C:\\NVSLog\\SDKEngine",sizeof(m_sSystemPath));
	MKDIR("C:\\NVSLog");
    MKDIR(m_sSystemPath);

#ifdef _WIN32
    m_log.SetFileName("C:\\NVSLog\\SDKEngine\\NVSSDKEngine.log");
#else
    m_log.SetFileName("/usr/local/NVSLog/NVSSDKEngine.log");
#endif
    m_log.SetFileSize(1024);//1M大小
    m_log.SetLogLevel(LOG_INFO);

    InitializeCriticalSection(&m_Lock);
}
NVSHandlerMgr::~NVSHandlerMgr(void)
{
    map<string,HINSTANCE>::iterator itb = m_mDllFunction.begin();
    map<string,HINSTANCE>::iterator ite = m_mDllFunction.end();

    for(;itb!=ite;itb++)
    {
        if((*itb).second != NULL)
            FreeLibrary(itb->second);
    }
    m_mDllFunction.clear();

    DeleteCriticalSection(&m_Lock);
}

intptr_t NVSHandlerMgr::getIdentify(char* sType)
{
    EnterCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO , "getIdentify in......");
    string sDll = sType;
    IDVRHandler* pHandler = NULL;

	sDll += PLAYPLUGIN;//pzm add 20131008 组合串*Play
    m_log.LogInfoEx(LOG_INFO , "DLL FileName is %s" , sDll.c_str());

	int nRet = XSLoadLibrary(sType ,sDll);
	if(nRet < 0)
	{
		LeaveCriticalSection(&m_Lock);
		return nRet;
	}
    LeaveCriticalSection(&m_Lock);

    GetDVRHandler pGetDVRHandler = (GetDVRHandler)::GetProcAddress(m_mDllFunction[sDll],"getHandler");
    if(pGetDVRHandler == NULL)
    {
        //LeaveCriticalSection(&m_Lock);
        m_log.LogInfoEx(LOG_ERROR , "获取导出函数句柄失败");
        return -2;
    }

    m_log.LogInfoEx(LOG_INFO , "获取导出函数句柄成功");

    pHandler = pGetDVRHandler();
    if(NULL == pHandler)
    {
        //LeaveCriticalSection(&m_Lock);

        m_log.LogInfoEx(LOG_ERROR , "获取DVR句柄失败");
        return -2;
    }

    m_log.LogInfoEx(LOG_INFO , "获取DVR句柄成功");

    m_log.LogInfoEx(LOG_INFO ,"getIdentify Out...");
    return reinterpret_cast<intptr_t>(pHandler);
}

void NVSHandlerMgr::freeLoginInfo(intptr_t identify)
{
    EnterCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO ,"freeLoginInfo In...");
    IDVRHandler* pHandler = (IDVRHandler*)identify;

    if(NULL == pHandler)
    {
        m_log.LogInfoEx(LOG_ERROR , "传递的句柄为空");
        LeaveCriticalSection(&m_Lock);
        return;
    }
    //pHandler->freeLoginInfo();
    pHandler->releaseHandler();
    m_log.LogInfoEx(LOG_INFO ,"freeLoginInfo Out...");
    LeaveCriticalSection(&m_Lock);
}

intptr_t NVSHandlerMgr::connectDVR(char* sIP, int nPort, char* sUser, char* sPwd, int nChannel, char* sType, BOOL &BRes)
{
    EnterCriticalSection(&m_Lock);
    BRes = FALSE;
    m_log.LogInfoEx(LOG_INFO ,"connectDVR In...");

    string sDll = sType;
    IDVRHandler* pHandler = NULL;
    char strLog[256]={0};

    sDll += NETPLUGIN;
	m_log.LogInfoEx(LOG_INFO , "DLL FileName is %s" , sDll.c_str());

	int nRet = XSLoadLibrary(sType ,sDll);
	if(nRet < 0)
	{
		LeaveCriticalSection(&m_Lock);
		return nRet;
	}
    LeaveCriticalSection(&m_Lock);

    GetDVRHandler pGetDVRHandler = (GetDVRHandler)::GetProcAddress(m_mDllFunction[sDll],"getHandler");
    if(pGetDVRHandler == NULL)
    {
        m_log.LogInfoEx(LOG_ERROR , "获取导出函数句柄失败");
        // LeaveCriticalSection(&m_Lock);
        return -2;
    }
    m_log.LogInfoEx(LOG_INFO , "获取导出函数句柄成功");
	
    pHandler = pGetDVRHandler();
    if(NULL == pHandler)
    {
        m_log.LogInfoEx(LOG_ERROR , "获取DVR句柄失败");
        return -2;
    }
    m_log.LogInfoEx(LOG_INFO , "获取DVR句柄成功");

    if(!pHandler->connectDVR(sIP,nPort,sUser,sPwd,nChannel))
    {
        pHandler->releaseHandler();
        pHandler = NULL;
        m_log.LogInfoEx(LOG_ERROR , "连接DVR失败");
        return -3;
    }
    m_log.LogInfoEx(LOG_INFO , "连接DVR成功");
    BRes = TRUE;
    return reinterpret_cast<intptr_t>(pHandler);
}

void NVSHandlerMgr::disconnect(intptr_t identify)
{
    EnterCriticalSection(&m_Lock);

    m_log.LogInfoEx(LOG_INFO ,"disconnect In...");

    IDVRHandler* pHandler = (IDVRHandler*)identify;

	printf("disconnect:%p\n", pHandler);

    if(NULL == pHandler)
    {
        m_log.LogInfoEx(LOG_ERROR , "传递的句柄为空");
        LeaveCriticalSection(&m_Lock);
        return;
    }
    pHandler->disconnectDVR();
    pHandler->releaseHandler();
    pHandler = NULL;

    m_log.LogInfoEx(LOG_INFO , "断开DVR连接成功");

    LeaveCriticalSection(&m_Lock);
}

LONG NVSHandlerMgr::checkDVR(char* sIP, int nPort, char* sUser, char* sPwd,char* sType,CLInfo pInfo[],int nMaxCount)
{
    EnterCriticalSection(&m_Lock);
    string sDll = sType;

    m_log.LogInfoEx(LOG_INFO , "checkDVR in......");

    sDll += NETPLUGIN;

	m_log.LogInfoEx(LOG_INFO , "DLL FileName is %s" , sDll.c_str());

	int nRet = XSLoadLibrary(sType ,sDll);
	if(nRet < 0)
	{
		LeaveCriticalSection(&m_Lock);
		return nRet;
	}

    GetDVRHandler pGetDVRHandler = (GetDVRHandler)::GetProcAddress(m_mDllFunction[sDll],"getHandler");
    if(pGetDVRHandler == NULL)
    {
        m_log.LogInfoEx(LOG_ERROR , "获取导出函数句柄失败");
        LeaveCriticalSection(&m_Lock);
        return -3;
    }
    m_log.LogInfoEx(LOG_INFO , "获取导出函数句柄成功");

    IDVRHandler* pHandler = pGetDVRHandler();
    if(NULL == pHandler)
    {
        m_log.LogInfoEx(LOG_ERROR , "获取DVR句柄失败");
        LeaveCriticalSection(&m_Lock);
        return -4;
    }
    m_log.LogInfoEx(LOG_INFO , "获取DVR句柄成功");

    LONG lResult = pHandler->checkDVR(sIP,nPort,sUser,sPwd, pInfo, nMaxCount);
    m_log.LogInfoEx(LOG_INFO , "checkDVR返回值：%d", lResult);
    pHandler->releaseHandler();
    pHandler = NULL;
    m_log.LogInfoEx(LOG_INFO , "checkDVR out....");
    LeaveCriticalSection(&m_Lock);
    return lResult;
}

void NVSHandlerMgr::setDVR(char* sIP, int nPort, char* sUser, char* sPwd,char* sType,CLInfo pInfo[],int nInfoCount)
{
    EnterCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO , "setDVR In...");
    string sDll = sType;

    sDll += NETPLUGIN;

	m_log.LogInfoEx(LOG_INFO , "DLL FileName is %s" , sDll.c_str());

	int nRet = XSLoadLibrary(sType ,sDll);
	if(nRet < 0)
	{
		LeaveCriticalSection(&m_Lock);
		return;
	}

    GetDVRHandler pGetDVRHandler = (GetDVRHandler)::GetProcAddress(m_mDllFunction[sDll],"getHandler");
    if(pGetDVRHandler == NULL)
    {
        m_log.LogInfoEx(LOG_ERROR , "获取导出函数句柄失败");
        LeaveCriticalSection(&m_Lock);
        return;
    }
    m_log.LogInfoEx(LOG_INFO , "获取导出函数句柄成功");

    IDVRHandler* pHandler = pGetDVRHandler();
    if(NULL == pHandler)
    {
        m_log.LogInfoEx(LOG_ERROR , "获取DVR句柄失败");

        LeaveCriticalSection(&m_Lock);
        return;
    }
    m_log.LogInfoEx(LOG_INFO , "获取DVR句柄成功");

    pHandler->setDVR(sIP,nPort,sUser,sPwd,pInfo,nInfoCount);
    pHandler->releaseHandler();
    pHandler = NULL;
    m_log.LogInfoEx(LOG_INFO , "setDVR out...");
    LeaveCriticalSection(&m_Lock);
    return;
}

bool NVSHandlerMgr::captureStream(intptr_t identify, LPCAPTURESTREAMCALLBACK lpCallBack, void* pUser, int nGetVideoType)
{
    EnterCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO , "captureStream in...");
    IDVRHandler* pHandler = (IDVRHandler*)identify;

    if(NULL == pHandler)
    {
        m_log.LogInfoEx(LOG_WARNING , "传递的句柄为空");

        LeaveCriticalSection(&m_Lock);
        return false;
    }
    bool bOK = pHandler->captureStream(lpCallBack,pUser, nGetVideoType);
    if (!bOK)
    {
        m_log.LogInfoEx(LOG_ERROR , "调用插件捕获流失败 ");
    }
    else
    {
        m_log.LogInfoEx(LOG_INFO , "调用插件捕获流成功 ");
    }

	printf("captureStream:%p\n", pHandler);

    m_log.LogInfoEx(LOG_INFO , "captureStream out...");
    LeaveCriticalSection(&m_Lock);

    return bOK;
}

bool NVSHandlerMgr::stopCaptureStream(intptr_t identify)
{
    EnterCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO , "stopCaptureStream In...");
    IDVRHandler* pHandler = (IDVRHandler*)identify;

	printf("stopCaptureStream:%p\n", pHandler);

    if(NULL == pHandler)
    {
        m_log.LogInfoEx(LOG_ERROR , "传递的句柄为空");
        LeaveCriticalSection(&m_Lock);
        return false;
    }
    bool bOK = pHandler->stopCaptureStream();
    m_log.LogInfoEx(LOG_WARNING , "stopCaptureStream返回值：%d", bOK);
    m_log.LogInfoEx(LOG_INFO , "stopCaptureStream Out...");
    LeaveCriticalSection(&m_Lock);
    return bOK;
}


bool NVSHandlerMgr::capturePicture(HWND hWnd,int nWidth1,int nHeight1,char* sFilePath)
{
    // 	RECT rc;
    // 	GetClientRect(hWnd,&rc);
    // 	int nWidthSrc = rc.right - rc.left;
    // 	int nHeightSrc = rc.bottom -rc.top;
    // 	HDC hDC = ::GetDC(hWnd);
    // 	HBITMAP hBitmap = ::CreateCompatibleBitmap(hDC,nWidthSrc,nHeightSrc);
    // 	HDC hMemDC;
    // 	hMemDC = ::CreateCompatibleDC(hDC);
    // 	HBITMAP hOld = (HBITMAP)::SelectObject(hMemDC,hBitmap);
    // 	::BitBlt(hMemDC,0,0,nWidthSrc,nHeightSrc,hDC,0,0,SRCCOPY);
    // 	::SelectObject(hMemDC,hOld);
    //
    // 	CxImage xImage;
    // 	xImage.CreateFromHBITMAP(hBitmap);
    // 	xImage.Resample2(nWidth1,nHeight1);
    // 	HBITMAP hNewBitmap = xImage.MakeBitmap();
    //
    // 	CreateBMPFile(sFilePath,hNewBitmap,hDC);
    // 	::DeleteObject(hNewBitmap);
    // 	::DeleteObject(hBitmap);
    // 	::DeleteDC(hMemDC);
    // 	::ReleaseDC(hWnd,hDC);
    return true;
}

RECORDFILE* NVSHandlerMgr::getRecordFile(intptr_t identify,RECORDQUERYTYPE nType,char* startTime,char* endTime,int* nFileCount)
{
    m_log.LogInfoEx(LOG_INFO , "getRecordFile In...");
    m_log.LogInfoEx(LOG_INFO ,"getRecordFile In...");

    RECORDFILE* pReturn = NULL;
    IDVRHandler* pHandler = (IDVRHandler*)identify;

    if(NULL == pHandler)
    {
        m_log.LogInfoEx(LOG_ERROR , "传递的句柄为空");
        return false;
    }
    pReturn = pHandler->getRecordFile(nType, startTime, endTime, nFileCount);
    if (NULL == pReturn)
    {
        m_log.LogInfoEx(LOG_ERROR , "获取录像文件列表失败");
    }
    else
    {
        m_log.LogInfoEx(LOG_INFO , "获取录像文件列表成功");
    }
    m_log.LogInfoEx(LOG_INFO , "getRecordFile Out...");
    return pReturn;
}

int NVSHandlerMgr::getRecordFileEx(intptr_t identify,RECORDQUERYTYPE nType,char* startTime,char* endTime,RECORDFILE pRecordFile[],int nMaxFileCount)
{
    m_log.LogInfoEx(LOG_INFO , "getRecordEx In...");
    IDVRHandler* pHandler = (IDVRHandler*)identify;
    if(NULL == pHandler)
    {
        m_log.LogInfoEx(LOG_ERROR , "传递的句柄为空");
        return false;
    }
    int nReturn = pHandler->getRecordFileEx(nType, startTime, endTime, pRecordFile,nMaxFileCount);
    m_log.LogInfoEx(LOG_WARNING , "获取录像文件列表返回值：%d", nReturn);
    m_log.LogInfoEx(LOG_INFO , "getRecordFileEx Out...");
    return nReturn;
}

intptr_t NVSHandlerMgr::downloadRecordFile(intptr_t identify,char* fileName,char* startTime, char* endTime,int nFileSize,char* saveFile,LPDOWNLOADRECORDFILECALLBACK lpCallBack,void* pUser)
{
    m_log.LogInfoEx(LOG_INFO , "downloadRecordFile In...");

    IDVRHandler* pHandler = (IDVRHandler*)identify;

    if(NULL == pHandler)
    {
        m_log.LogInfoEx(LOG_ERROR , "传递的句柄为空");
        return false;
    }
    std::string strLog;
	if (fileName)
	{
		strLog = fileName;
		strLog += " ";
	}
	if (startTime)
	{
		strLog += startTime;
		strLog += " ";
	}
	if (endTime)
	{
		strLog += endTime;
		strLog += " ";
	}
    strLog+=saveFile;
    m_log.LogInfoEx(LOG_INFO , "下载文件名称：%s", strLog.c_str());

	intptr_t ldownFile = pHandler->downloadRecordFile(fileName, startTime, endTime, nFileSize, saveFile, lpCallBack, pUser);
    //LeaveCriticalSection(&m_Lock);
    if (ldownFile!=-1)
    {
        m_log.LogInfoEx(LOG_INFO , "下载成功");
    }
    else
    {
        m_log.LogInfoEx(LOG_ERROR , "下载失败");
    }

    m_log.LogInfoEx(LOG_INFO , "downloadRecordFile Out...");
    return ldownFile;
}

bool NVSHandlerMgr::stopDownloadRecordFile(intptr_t identify, intptr_t ldownFileHandle)
{
    //EnterCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO , "stopDownloadRecordFile In...");
    IDVRHandler* pHandler = (IDVRHandler*)identify;

    if(NULL == pHandler)
    {
        m_log.LogInfoEx(LOG_ERROR , "stopDownloadRecordFile 传递的句柄为空");
        //LeaveCriticalSection(&m_Lock);
        return false;
    }

    bool bOK = pHandler->stopDownloadRecordFile(ldownFileHandle);
    m_log.LogInfoEx(LOG_WARNING , "stopDownloadRecordFile返回值：%d", bOK);
    //LeaveCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO , "stopDownloadRecordFile Out...");
    return bOK;
}

intptr_t NVSHandlerMgr::replayRecordFile(intptr_t identify, HWND hWnd,char* fileName,char* startTime, char* endTime,int nFileSize)
{
    //EnterCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO , "replayRecordFile In...");
    IDVRHandler* pHandler = (IDVRHandler*)identify;

    if(NULL == pHandler)
    {
        m_log.LogInfoEx(LOG_ERROR , "传递的句柄为空");
        //LeaveCriticalSection(&m_Lock);
        return false;
    }
	intptr_t lReturn = pHandler->replayRecordFile(hWnd, fileName, startTime, endTime, nFileSize);
    m_log.LogInfoEx(LOG_WARNING , "replayRecordFile返回值：%d", lReturn);
    m_log.LogInfoEx(LOG_INFO , "replayRecordFile Out...");
    //LeaveCriticalSection(&m_Lock);
    return lReturn;
}

bool NVSHandlerMgr::controlReplayRecordFile(intptr_t identify, intptr_t lReplayHandle,int nCmd,int nInValue,int* outValue)
{
    //EnterCriticalSection(&m_Lock);
    IDVRHandler* pHandler = (IDVRHandler*)identify;

    if(NULL == pHandler)
    {
        m_log.LogInfoEx(LOG_ERROR , "传递的句柄为空");
        //LeaveCriticalSection(&m_Lock);
        return false;
    }

    bool bOK = pHandler->controlReplayRecordFile(lReplayHandle,nCmd, nInValue, outValue);

    //LeaveCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO , "controlReplayRecordFile Out...");
    return bOK;
}

bool NVSHandlerMgr::stopReplayRecordFile(intptr_t identify, intptr_t lReplayHandle )
{
    //EnterCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO , "stopReplayRecordFile in...");
    IDVRHandler* pHandler = (IDVRHandler*)identify;

    if(NULL == pHandler)
    {
        m_log.LogInfoEx(LOG_ERROR , "传递的句柄为空");
        //LeaveCriticalSection(&m_Lock);
        return false;
    }

    bool bOK = pHandler->stopReplayRecordFile(lReplayHandle);
    m_log.LogInfoEx(LOG_INFO , "stopReplayRecordFile Out...");
    //LeaveCriticalSection(&m_Lock);
    return bOK;
}

bool NVSHandlerMgr::startTalk(intptr_t identify)
{
    //EnterCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO , "startTalk In...");
    IDVRHandler* pHandler = (IDVRHandler*)identify;

    if(NULL == pHandler)
    {
        m_log.LogInfoEx(LOG_ERROR , "传递的句柄为空");
        //LeaveCriticalSection(&m_Lock);
        return false;
    }

    bool bOK = pHandler->startTalk();
    m_log.LogInfoEx(LOG_INFO , "startTalk Out...");
    //LeaveCriticalSection(&m_Lock);
    return bOK;
}

bool NVSHandlerMgr::stopTalk(intptr_t identify)
{
    //EnterCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO , "stopTalk In...");
    IDVRHandler* pHandler = (IDVRHandler*)identify;
    if(NULL == pHandler)
    {
        m_log.LogInfoEx(LOG_INFO , "传递的句柄为空");
        //LeaveCriticalSection(&m_Lock);
        return false;
    }

    bool bOK = pHandler->stopTalk();
    m_log.LogInfoEx(LOG_INFO , "stopTalk Out...");
    //LeaveCriticalSection(&m_Lock);
    return bOK;
}

bool NVSHandlerMgr::controlPTZ(intptr_t identify,int nCmd, bool bEnable)
{
    m_log.LogInfoEx(LOG_INFO , "controlPTZ In...");
    //EnterCriticalSection(&m_Lock);
    IDVRHandler* pHandler = (IDVRHandler*)identify;

    if(NULL == pHandler)
    {
        m_log.LogInfoEx(LOG_ERROR , "传递的句柄为空");
        //LeaveCriticalSection(&m_Lock);
        return false;
    }

    m_log.LogInfoEx(LOG_WARNING , "PTZ用户标志[%d]开始PTZ控制,控制指令[%d],控制状态[%d]", identify, nCmd, bEnable);
    bool bOK = pHandler->controlPTZ(nCmd,bEnable);
    //LeaveCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO , "controlPTZ Out...");
    return bOK;
}

bool NVSHandlerMgr::controlPTZSpeed(intptr_t identify,int nCmd, bool bEnable,int nSpeed)
{
    EnterCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO , "controlPTZSpeed In...");
    IDVRHandler* pHandler = (IDVRHandler*)identify;

    if(NULL == pHandler)
    {
        m_log.LogInfoEx(LOG_ERROR , "传递的参数为空");
        LeaveCriticalSection(&m_Lock);
        return false;
    }

    m_log.LogInfoEx(LOG_WARNING , "用户标志[%d]调用PTZ控制,控制指令[%d],控制速度[%d],控制状态[%d]", identify, nCmd, nSpeed, bEnable);
    try
    {
        bool bOK = pHandler->controlPTZSpeed(nCmd,bEnable,nSpeed);
        LeaveCriticalSection(&m_Lock);
        m_log.LogInfoEx(LOG_INFO , "controlPTZSpeed Out...");
        return bOK;
    }
    catch(...)
    {
        m_log.LogInfoEx(LOG_ERROR , "发生未知异常");
        LeaveCriticalSection(&m_Lock);
    }
    return false;
}

bool NVSHandlerMgr::presetPTZ(intptr_t identify, int nCmd, int nIndex)
{
    m_log.LogInfoEx(LOG_INFO , "presetPTZ In...");
    //EnterCriticalSection(&m_Lock);
    IDVRHandler* pHandler = (IDVRHandler*)identify;

    if(NULL == pHandler)
    {
        m_log.LogInfoEx(LOG_ERROR , "传递的句柄为空");
        //LeaveCriticalSection(&m_Lock);
        return false;
    }

    bool bOK = pHandler->presetPTZ(nCmd,nIndex);
    //LeaveCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO , "presetPTZ Out...");
    return bOK;
}

bool NVSHandlerMgr::showSettingPane(intptr_t identify)
{
    //EnterCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO , "presetPTZ Out...");
    m_log.LogInfoEx(LOG_INFO ,"showSettingPane In...");
    IDVRHandler* pHandler = (IDVRHandler*)identify;

    if(NULL == pHandler)
    {
        m_log.LogInfoEx(LOG_ERROR , "传递的句柄为空");
        //LeaveCriticalSection(&m_Lock);
        return false;
    }

    bool bOK = pHandler->showSettingPane();
    m_log.LogInfoEx(LOG_INFO , "showSettingPane Out...");
    //LeaveCriticalSection(&m_Lock);
    return bOK;
}

intptr_t NVSHandlerMgr::initialPlayer(char* sType,int nPort,int nDecodeType,BYTE* buff, int nSize, int nStreamType)
{
    //EnterCriticalSection(&m_Lock);
    //m_log.LogInfoEx(LOG_INFO , "initialPlayer InOut...");
    m_log.LogInfoEx(LOG_INFO ,"initialPlayer In...");
    string sDll = sType;

    sDll += PLAYPLUGIN;

	m_log.LogInfoEx(LOG_INFO , "DLL FileName is %s" , sDll.c_str());

	int nRet = XSLoadLibrary(sType ,sDll);
	if(nRet < 0)
	{
		return nRet;
	}

    m_log.LogInfoEx(LOG_INFO , "动态库句柄：%d", m_mDllFunction[sDll]);
    GetDVRHandler pGetDVRHandler = (GetDVRHandler)::GetProcAddress(m_mDllFunction[sDll],"getHandler");
    if(pGetDVRHandler == NULL)
    {
        m_log.LogInfoEx(LOG_ERROR , "获取导出函数句柄失败");
        return 0;
    }
    m_log.LogInfoEx(LOG_INFO , "获取导出函数句柄成功");

    IDVRHandler* pHandler = pGetDVRHandler();
    if(NULL == pHandler)
    {
        m_log.LogInfoEx(LOG_ERROR , "获取DVR句柄失败");

        return 0;
    }
    m_log.LogInfoEx(LOG_INFO , "获取DVR句柄成功");

    if(pHandler->initialPlayer(nPort, nDecodeType, buff, nSize, nStreamType))
    {
        m_log.LogInfoEx(LOG_INFO , "播放初始化成功 ");
	
        return reinterpret_cast<intptr_t>(pHandler);
    }
    else
    {
        m_log.LogInfoEx(LOG_ERROR , "播放初始化失败 ");
    }

    pHandler->releaseHandler();
    pHandler = NULL;
    m_log.LogInfoEx(LOG_INFO , "initialPlayer Out...");
    return 0;
}

bool NVSHandlerMgr::startPlayer(intptr_t identify, HWND hWnd)
{
    //EnterCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO , "startPlayer In...");
    IDVRHandler* pHandler = reinterpret_cast<IDVRHandler*>(identify);

    if(NULL == pHandler)
    {
        m_log.LogInfoEx(LOG_ERROR , "传递的句柄为空");
        //LeaveCriticalSection(&m_Lock);
        return false;
    }

    bool bOK = pHandler->startPlayer(hWnd);
    m_log.LogInfoEx(LOG_INFO , "startPlayer Out...");
    //LeaveCriticalSection(&m_Lock);
    return bOK;
}

bool NVSHandlerMgr::startPlayer2(intptr_t identify, HWND hWnd, LPDRAWCBCALLLBACK lpDrawCBFun, void *pUser)
{
    m_log.LogInfoEx(LOG_INFO , "startPlayer2 In...");
    IDVRHandler* pHandler = (IDVRHandler*)identify;

    if(NULL == pHandler)
    {
        m_log.LogInfoEx(LOG_ERROR , "传递的句柄为空");
        return false;
    }

    bool bOK = pHandler->startPlayer2(hWnd, lpDrawCBFun, pUser);
    m_log.LogInfoEx(LOG_INFO , "startPlayer2 Out...");
    return bOK;

}

// 2007-04-23增加
bool NVSHandlerMgr::startPlayer4Standard(intptr_t identify,HWND hWnd,LPDecCBFun lpDecCBFun, int nCameraID)
{
    m_log.LogInfoEx(LOG_INFO , "startPlayer4Standard in...");
    IDVRHandler* pHandler = (IDVRHandler*)identify;

    if(NULL == pHandler)
    {
        m_log.LogInfoEx(LOG_ERROR , "传递的句柄为空");
        return false;
    }

    bool bOK = pHandler->startPlayer4Standard(hWnd,lpDecCBFun, nCameraID);
    m_log.LogInfoEx(LOG_INFO , "startPlayer4Standard Out...");
    return bOK;
}

bool NVSHandlerMgr::addData(intptr_t identify,BYTE* buff, int nSize)
{
    //EnterCriticalSection(&m_Lock);
    IDVRHandler* pHandler = (IDVRHandler*)identify;

    if(NULL == pHandler)
    {
        m_log.LogInfoEx(LOG_ERROR , "传递的句柄为空");
        //LeaveCriticalSection(&m_Lock);
        return false;
    }

    bool bOK = pHandler->addData(buff,nSize);
    //LeaveCriticalSection(&m_Lock);
    return bOK;
}

bool NVSHandlerMgr::stopPlayer(intptr_t identify)
{
    m_log.LogInfoEx(LOG_INFO , "stopPlayer In...");
    IDVRHandler* pHandler = (IDVRHandler*)identify;

    if(NULL == pHandler)
    {
        m_log.LogInfoEx(LOG_ERROR , "传递的句柄为空");
        return false;
    }
    pHandler->stopPlayer();
    pHandler->releaseHandler();
    pHandler = 0;
    m_log.LogInfoEx(LOG_INFO , "stopPlayer Out...");
    return true;
}

intptr_t NVSHandlerMgr::locPlay_OpenFile(char* sType,int nPort,int nDecodeType,HWND hWnd,char* fileName)
{
    //EnterCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO , "locPlay_OpenFile In...");
    string sDll = sType;

    sDll += PLAYPLUGIN;
	m_log.LogInfoEx(LOG_INFO , "DLL FileName is %s" , sDll.c_str());

	int nRet = XSLoadLibrary(sType ,sDll);
	if(nRet < 0)
	{
		return nRet;
	}
    
    GetDVRHandler pGetDVRHandler = (GetDVRHandler)::GetProcAddress(m_mDllFunction[sDll],"getHandler");
    if(pGetDVRHandler == NULL)
    {
        //LeaveCriticalSection(&m_Lock);
        m_log.LogInfoEx(LOG_ERROR , "获取导出函数失败");
        return 0;
    }

    IDVRHandler* pHandler = pGetDVRHandler();
    if(NULL == pHandler)
    {
        //LeaveCriticalSection(&m_Lock);
        m_log.LogInfoEx(LOG_ERROR , "获取DVR句柄失败");
        return 0;
    }
    if(pHandler->locPlay_OpenFile(nPort,nDecodeType, hWnd, fileName))
    {
        //LeaveCriticalSection(&m_Lock);
        m_log.LogInfoEx(LOG_INFO , "打开文件成功,文件名称: %s", fileName);
        return reinterpret_cast<intptr_t>(pHandler);
    }
    pHandler->releaseHandler();
    pHandler = NULL;
	m_log.LogInfoEx(LOG_ERROR , "打开文件失败,文件名称: %s", fileName);
    m_log.LogInfoEx(LOG_INFO , "locPlay_OpenFile out....");
    //LeaveCriticalSection(&m_Lock);
    return 0;
}

bool NVSHandlerMgr::locPlay_Control(intptr_t identify,int nCmd,int nInValue,int* outValue)
{
    //EnterCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO , "locPlay_Control In....");
    IDVRHandler* pHandler = (IDVRHandler*)identify;

    if(NULL == pHandler)
    {
        //LeaveCriticalSection(&m_Lock);
        m_log.LogInfoEx(LOG_ERROR , "传递的句柄为空");
        return false;
    }
    bool bOK = pHandler->locPlay_Control(nCmd, nInValue, outValue);
	m_log.LogInfoEx(LOG_INFO , "locPlay_Control out....bOK:%d",bOK);
    //LeaveCriticalSection(&m_Lock);
    return bOK;
}

bool NVSHandlerMgr::locPlay_Stop(intptr_t identify)
{
    //EnterCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO , "locPlay_Stop In....");
    IDVRHandler* pHandler = (IDVRHandler*)identify;

    if(NULL == pHandler)
    {
        //LeaveCriticalSection(&m_Lock);
        m_log.LogInfoEx(LOG_ERROR, "传递的句柄为空");
        return false;
    }
    pHandler->locPlay_Stop();
    pHandler->releaseHandler();
    pHandler = 0;
    m_log.LogInfoEx(LOG_INFO , "locPlay_Stop out....");
    //LeaveCriticalSection(&m_Lock);
    return true;
}

intptr_t NVSHandlerMgr::alarm_startCapture(char* sType,int nPort,LPALARMCALLBACK lpCallBack)
{
    //EnterCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO , "alarm_startCapture In....");
    string sDll = sType;

    sDll += NETPLUGIN;
	m_log.LogInfoEx(LOG_INFO , "DLL FileName is %s" , sDll.c_str());

	int nRet = XSLoadLibrary(sType ,sDll);
	if(nRet < 0)
	{
		return nRet;
	}

    GetDVRHandler pGetDVRHandler = (GetDVRHandler)::GetProcAddress(m_mDllFunction[sDll],"getHandler");
    if(pGetDVRHandler == NULL)
    {
        //LeaveCriticalSection(&m_Lock);
        m_log.LogInfoEx(LOG_ERROR , "获取导出函数getHandler句柄失败");
        return 0;
    }

    IDVRHandler* pHandler = pGetDVRHandler();
    if(NULL == pHandler)
    {
        //LeaveCriticalSection(&m_Lock);
        m_log.LogInfoEx(LOG_ERROR , "获取DVR句柄失败");
        return 0;
    }
    if(pHandler->alarm_startCapture(nPort, lpCallBack))
    {
        //LeaveCriticalSection(&m_Lock);
        m_log.LogInfoEx(LOG_INFO , "开始报警捕获成功 ");
        return reinterpret_cast<intptr_t>(pHandler);
    }
    m_log.LogInfoEx(LOG_ERROR , "开始报警捕获失败，端口为：%d", nPort);

    pHandler->releaseHandler();
    pHandler = NULL;
    m_log.LogInfoEx(LOG_INFO , "alarm_startCapture out....");

    //LeaveCriticalSection(&m_Lock);
    return 0;
}

bool NVSHandlerMgr::alarm_stop(intptr_t identify)
{
    //EnterCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO , "alarm_stop In....");
    IDVRHandler* pHandler = (IDVRHandler*)identify;

    if(NULL == pHandler)
    {
        //LeaveCriticalSection(&m_Lock);
        m_log.LogInfoEx(LOG_ERROR , "传递的句柄为空");
        return false;
    }
    pHandler->alarm_stop();
    pHandler->releaseHandler();
    pHandler = NULL;
    //LeaveCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO , "alarm_stop out....");
    return true;
}

int NVSHandlerMgr::log_QueryEx(intptr_t identify,LOGQUERYTYPE nQueryType, LOGERRORTYPE nErrorType,char* startTime,char* endTime,LOGINFO pLogInfo[],int nMaxLogCount)
{
    //EnterCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO , "log_QueryEx In....");
    IDVRHandler* pHandler = (IDVRHandler*)identify;

    if(NULL == pHandler)
    {
        //LeaveCriticalSection(&m_Lock);
        m_log.LogInfoEx(LOG_ERROR , "传递的句柄为空");
        return 0;
    }
    int nReturn = pHandler->log_QueryEx(nQueryType, nErrorType, startTime, endTime, pLogInfo,nMaxLogCount);
    m_log.LogInfoEx(LOG_INFO , "log_QueryEx out....");
    //LeaveCriticalSection(&m_Lock);
    return nReturn;
}

LOGINFO* NVSHandlerMgr::log_Query(intptr_t identify,LOGQUERYTYPE nQueryType, LOGERRORTYPE nErrorType,char* startTime,char* endTime,int* nLogCount)
{
    m_log.LogInfoEx(LOG_INFO , "log_Query In....");
    LOGINFO* pReturn = 0;
    //EnterCriticalSection(&m_Lock);
    IDVRHandler* pHandler = (IDVRHandler*)identify;

    if(NULL == pHandler)
    {
        //LeaveCriticalSection(&m_Lock);
        m_log.LogInfoEx(LOG_ERROR , "传递的句柄为空");
        return false;
    }
    pReturn = pHandler->log_Query(nQueryType, nErrorType, startTime, endTime, nLogCount);
    m_log.LogInfoEx(LOG_INFO , "log_Query out....");
    //LeaveCriticalSection(&m_Lock);
    return pReturn;
}

// 2007-1-28增加
bool NVSHandlerMgr::setVideoEffect(intptr_t identify,DWORD dwBrightValue,DWORD dwContrastValue, DWORD dwSaturationValue,DWORD dwHueValue)
{
    EnterCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO , "setVideoEffect In....");
    IDVRHandler* pHandler = (IDVRHandler*)identify;

    if(NULL == pHandler)
    {
        m_log.LogInfoEx(LOG_ERROR , "传递的句柄为空");
        LeaveCriticalSection(&m_Lock);
        return false;
    }
    m_log.LogInfoEx(LOG_INFO , "BrightValue=%d, ContrastValue=%d, SaturationValue=%d, HueValue=%d"
        , dwBrightValue, dwContrastValue, dwSaturationValue, dwHueValue);

    bool bOK = pHandler->setVideoEffect(dwBrightValue,dwContrastValue,dwSaturationValue,dwHueValue);

    m_log.LogInfoEx(LOG_INFO , "setVideoEffect out....");
    LeaveCriticalSection(&m_Lock);
    return bOK;
}

bool NVSHandlerMgr::getVideoEffect(intptr_t identify,DWORD *pBrightValue,DWORD *pContrastValue, DWORD *pSaturationValue,DWORD *pHueValue)
{
    EnterCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO , "getVideoEffect In...");
    IDVRHandler* pHandler = (IDVRHandler*)identify;

    if(NULL == pHandler)
    {
        m_log.LogInfoEx(LOG_ERROR , "传递的句柄为空");
        LeaveCriticalSection(&m_Lock);
        return false;
    }
    bool bOK = pHandler->getVideoEffect(pBrightValue,pContrastValue,pSaturationValue,pHueValue);
    m_log.LogInfoEx(LOG_INFO , "getVideoEffect Out...");
    LeaveCriticalSection(&m_Lock);
    return bOK;
}

bool NVSHandlerMgr::setDVRConfig(intptr_t identify, DWORD dwCommand,LONG lChannel,LPVOID lpInBuffer,DWORD dwInBufferSize)
{
    EnterCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO , "setDVRConfig In...");
    IDVRHandler* pHandler = (IDVRHandler*)identify;

    if(NULL == pHandler)
    {
        m_log.LogInfoEx(LOG_ERROR , "传递的句柄为空");
        LeaveCriticalSection(&m_Lock);
        return false;
    }
    bool bOK = pHandler->setDVRConfig(dwCommand,lChannel,lpInBuffer,dwInBufferSize);
    m_log.LogInfoEx(LOG_INFO , "setDVRConfig Out...");
    LeaveCriticalSection(&m_Lock);
    return bOK;
}

bool NVSHandlerMgr::getDVRConfig(intptr_t identify, DWORD dwCommand,LONG lChannel,LPVOID lpOutBuffer,DWORD dwOutBufferSize,LPDWORD lpBytesReturned)
{
    EnterCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO ,"getDVRConfig In...");
    IDVRHandler* pHandler = (IDVRHandler*)identify;

    if(NULL == pHandler)
    {
        m_log.LogInfoEx(LOG_ERROR , "传递的句柄为空");
        LeaveCriticalSection(&m_Lock);
        return false;
    }
    bool bOK = pHandler->getDVRConfig(dwCommand,lChannel,lpOutBuffer,dwOutBufferSize,lpBytesReturned);
    m_log.LogInfoEx(LOG_INFO ,"getDVRConfig Out...");
    LeaveCriticalSection(&m_Lock);
    return bOK;
}

int  NVSHandlerMgr::convertToBmpFile(char* sType,char * pBuf,LONG nSize,LONG nWidth,LONG nHeight,LONG nType,char *sFileName)
{
    //EnterCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO ,"convertToBmpFile In...");
    string sDll = sType;

    sDll += PLAYPLUGIN;
	m_log.LogInfoEx(LOG_INFO , "DLL FileName is %s" , sDll.c_str());

	int nRet = XSLoadLibrary(sType ,sDll);
	if(nRet < 0)
	{
		return nRet;
	}

    GetDVRHandler pGetDVRHandler = (GetDVRHandler)::GetProcAddress(m_mDllFunction[sDll],"getHandler");
    if(pGetDVRHandler == NULL)
    {
        //LeaveCriticalSection(&m_Lock);
        m_log.LogInfoEx(LOG_ERROR , "获取导出函数getHandler句柄失败");
        return -1;
    }
    m_log.LogInfoEx(LOG_INFO ,"convertToBmpFile", "获取导出函数getHandler句柄成功");

    IDVRHandler* pHandler = pGetDVRHandler();
    if(NULL == pHandler)
    {
        //LeaveCriticalSection(&m_Lock);
        m_log.LogInfoEx(LOG_ERROR , "获取DVR句柄失败");
        return -3;
    }
    m_log.LogInfoEx(LOG_INFO ,"convertToBmpFile", "获取DVR句柄成功");

    if(pHandler->convertToBmpFile(pBuf,nSize,nWidth,nHeight,nType,sFileName))
    {
        //LeaveCriticalSection(&m_Lock);
        m_log.LogInfoEx(LOG_INFO , "成功");
        return 0;
    }
    pHandler->releaseHandler();
    pHandler = NULL;
    //LeaveCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_ERROR ,"convertToBmpFile", "失败");
    m_log.LogInfoEx(LOG_INFO ,"convertToBmpFile Out...");

    return -4;
}

int  NVSHandlerMgr::oneByOneBack(char* sType,LONG nPort)
{
    //EnterCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO ,"oneByOneBack In...");
    string sDll = sType;

    sDll += PLAYPLUGIN;
	m_log.LogInfoEx(LOG_INFO , "DLL FileName is %s" , sDll.c_str());

	int nRet = XSLoadLibrary(sType ,sDll);
	if(nRet < 0)
	{
		return nRet;
	}

    GetDVRHandler pGetDVRHandler = (GetDVRHandler)::GetProcAddress(m_mDllFunction[sDll],"getHandler");
    if(pGetDVRHandler == NULL)
    {
        //LeaveCriticalSection(&m_Lock);
        m_log.LogInfoEx(LOG_ERROR , "获取导出函数句柄失败");
        return -1;
    }
    m_log.LogInfoEx(LOG_INFO ,"获取导出函数句柄成功");

    IDVRHandler* pHandler = pGetDVRHandler();
    if(NULL == pHandler)
    {
        //LeaveCriticalSection(&m_Lock);
        m_log.LogInfoEx(LOG_ERROR , "获取DVR句柄失败");
        return -3;
    }
    if(pHandler->oneByOneBack(nPort))
    {
        //LeaveCriticalSection(&m_Lock);
        m_log.LogInfoEx(LOG_INFO , "成功");
        return 0;
    }
    m_log.LogInfoEx(LOG_ERROR , "失败");
    pHandler->releaseHandler();
    pHandler = NULL;
    //LeaveCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO ,"oneByOneBack Out...");
    return -4;
}

int  NVSHandlerMgr::oneByOne(char* sType,LONG nPort)
{
    //EnterCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO ,"oneByOne In...");
    string sDll = sType;

    sDll += PLAYPLUGIN;
	m_log.LogInfoEx(LOG_INFO , "DLL FileName is %s" , sDll.c_str());

	int nRet = XSLoadLibrary(sType ,sDll);
	if(nRet < 0)
	{
		return nRet;
	}

    GetDVRHandler pGetDVRHandler = (GetDVRHandler)::GetProcAddress(m_mDllFunction[sDll],"getHandler");
    if(pGetDVRHandler == NULL)
    {
        //LeaveCriticalSection(&m_Lock);
        m_log.LogInfoEx(LOG_ERROR , "获取导出函数句柄失败");
        return -1;
    }
    m_log.LogInfoEx(LOG_INFO , "获取导出函数句柄成功");

    IDVRHandler* pHandler = pGetDVRHandler();
    if(NULL == pHandler)
    {
        //LeaveCriticalSection(&m_Lock);
        m_log.LogInfoEx(LOG_ERROR , "获取DVR句柄失败");
        return -3;
    }
    m_log.LogInfoEx(LOG_INFO , "获取DVR句柄成功");

    if(pHandler->oneByOne(nPort))
    {
        //LeaveCriticalSection(&m_Lock);
        m_log.LogInfoEx(LOG_INFO , "本地播放向前一帧成功 ");
        return 0;
    }
    m_log.LogInfoEx(LOG_ERROR , "本地播放向前一帧失败 ");
    pHandler->releaseHandler();
    pHandler = NULL;
    //LeaveCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO , "Out...");
    return -4;
}

int  NVSHandlerMgr::setPlayedTimeEx(char* sType,LONG nPort,DWORD nTime)
{
    //EnterCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO ,"setPlayedTimeEx In...");
    string sDll = sType;

    sDll += PLAYPLUGIN;
	m_log.LogInfoEx(LOG_INFO , "DLL FileName is %s" , sDll.c_str());

	int nRet = XSLoadLibrary(sType ,sDll);
	if(nRet < 0)
	{
		return nRet;
	}

    GetDVRHandler pGetDVRHandler = (GetDVRHandler)::GetProcAddress(m_mDllFunction[sDll],"getHandler");
    if(pGetDVRHandler == NULL)
    {
        //LeaveCriticalSection(&m_Lock);
        m_log.LogInfoEx(LOG_ERROR , "获取导出函数getHandler句柄失败");
        return -1;
    }
    m_log.LogInfoEx(LOG_INFO , "获取导出函数getHandler句柄成功");

    IDVRHandler* pHandler = pGetDVRHandler();
    if(NULL == pHandler)
    {
        //LeaveCriticalSection(&m_Lock);
        m_log.LogInfoEx(LOG_ERROR , "获取DVR句柄失败");
        return -3;
    }
    m_log.LogInfoEx(LOG_INFO , "获取DVR句柄成功");

    if(pHandler->setPlayedTimeEx(nPort,nTime))
    {
        //LeaveCriticalSection(&m_Lock);
        m_log.LogInfoEx(LOG_INFO , "设置本地回放播放的时间进度成功 ");
        return 0;
    }
    m_log.LogInfoEx(LOG_ERROR , "设置本地回放播放的时间进度失败 ");
    pHandler->releaseHandler();
    pHandler = NULL;
    //LeaveCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO ,"setPlayedTimeEx Out...");
    return -4;
}
int  NVSHandlerMgr::setFileRefCallBack(char* sType,LONG nPort,void (__stdcall *pFileRefDone) (DWORD nPort,void *nUser),void *nUser)
{
    //EnterCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO ,"setFileRefCallBack In...");
    string sDll = sType;

    sDll += PLAYPLUGIN;
	m_log.LogInfoEx(LOG_INFO , "DLL FileName is %s" , sDll.c_str());

	int nRet = XSLoadLibrary(sType ,sDll);
	if(nRet < 0)
	{
		return nRet;
	}

    GetDVRHandler pGetDVRHandler = (GetDVRHandler)::GetProcAddress(m_mDllFunction[sDll],"getHandler");
    if(pGetDVRHandler == NULL)
    {
        //LeaveCriticalSection(&m_Lock);
        m_log.LogInfoEx(LOG_ERROR , "获取导出函数getHandler句柄失败");
        return -1;
    }
    m_log.LogInfoEx(LOG_INFO , "获取导出函数getHandler句柄成功");

    IDVRHandler* pHandler = pGetDVRHandler();
    if(NULL == pHandler)
    {
        //LeaveCriticalSection(&m_Lock);
        m_log.LogInfoEx(LOG_ERROR , "获取DVR句柄失败");
        return -3;
    }
    m_log.LogInfoEx(LOG_INFO , "获取DVR句柄成功");

    if(pHandler->setFileRefCallBack(nPort,pFileRefDone,nUser))
    {
        //LeaveCriticalSection(&m_Lock);
        m_log.LogInfoEx(LOG_INFO , "本地播放前回调函数设置成功 ");
        return 0;
    }
    m_log.LogInfoEx(LOG_ERROR , "本地播放前回调函数设置失败 ");
    pHandler->releaseHandler();
    pHandler = NULL;
    //LeaveCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO ,"setFileRefCallBack Out...");
    return -4;
}

int  NVSHandlerMgr::setDisplayCallBack(char* sType,LONG nPort, void (__stdcall *pFileRefDone)(long nPort,char * pBuf,long nSize,long nWidth,long nHeight,long nStamp,long nType,void *nReserved))
{
    //EnterCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO ,"setDisplayCallBack In...");
    string sDll = sType;

    sDll += PLAYPLUGIN;
	m_log.LogInfoEx(LOG_INFO , "DLL FileName is %s" , sDll.c_str());

	int nRet = XSLoadLibrary(sType ,sDll);
	if(nRet < 0)
	{
		return nRet;
	}

    GetDVRHandler pGetDVRHandler = (GetDVRHandler)::GetProcAddress(m_mDllFunction[sDll],"getHandler");
    if(pGetDVRHandler == NULL)
    {
        //LeaveCriticalSection(&m_Lock);
        m_log.LogInfoEx(LOG_ERROR , "获取导出函数getHandler句柄失败 ");
        return -1;
    }
    m_log.LogInfoEx(LOG_ERROR , "获取导出函数getHandler句柄成功 ");

    IDVRHandler* pHandler = pGetDVRHandler();
    if(NULL == pHandler)
    {
        //LeaveCriticalSection(&m_Lock);
        m_log.LogInfoEx(LOG_ERROR , "获取DVR句柄失败 ");
        return -3;
    }
    m_log.LogInfoEx(LOG_ERROR , "获取DVR句柄成功");

    if(pHandler->setDisplayCallBack(nPort,pFileRefDone))
    {
        //LeaveCriticalSection(&m_Lock);
        m_log.LogInfoEx(LOG_INFO , "设置本地解帧回调函数成功");
        return 0;
    }
    m_log.LogInfoEx(LOG_ERROR , "设置本地解帧回调函数失败");
    pHandler->releaseHandler();
    pHandler = NULL;
    //LeaveCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO ,"setDisplayCallBack Out...");
    return -4;
}

bool NVSHandlerMgr::playBackSaveData(intptr_t identify, intptr_t lReplayHandle,char *sFileName)
{
    EnterCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO ,"playBackSaveData In...");
    IDVRHandler* pHandler = (IDVRHandler*)identify;

    if(NULL == pHandler)
    {
        m_log.LogInfoEx(LOG_ERROR , "传递的句柄为空");
        LeaveCriticalSection(&m_Lock);
        return false;
    }
    bool bOK = pHandler->playBackSaveData(lReplayHandle,sFileName);
    m_log.LogInfoEx(LOG_INFO ,"playBackSaveData Out...");
    LeaveCriticalSection(&m_Lock);
    return bOK;
}

bool NVSHandlerMgr::stopPlayBackSave(intptr_t identify, intptr_t lReplayHandle)
{
    EnterCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO ,"stopPlayBackSave In...");
    IDVRHandler* pHandler = (IDVRHandler*)identify;

    if(NULL == pHandler)
    {
        m_log.LogInfoEx(LOG_ERROR , "传递的句柄为空");
        LeaveCriticalSection(&m_Lock);
        return false;
    }

    bool bOK = pHandler->stopPlayBackSave(lReplayHandle);
    m_log.LogInfoEx(LOG_INFO ,"stopPlayBackSave Out...");
    LeaveCriticalSection(&m_Lock);
    return bOK;
}

int NVSHandlerMgr::getDownloadPos(intptr_t identify, intptr_t lFileHandle)
{
    EnterCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO ,"getDownloadPos In...");
    IDVRHandler* pHandler = (IDVRHandler*)identify;

    if(NULL == pHandler)
    {
        m_log.LogInfoEx(LOG_ERROR , "传递的句柄为空");
        LeaveCriticalSection(&m_Lock);
        return -1;
    }
    int nPos = pHandler->getDownloadPos(lFileHandle);
    m_log.LogInfoEx(LOG_INFO ,"setDownloadPos Out...");
    LeaveCriticalSection(&m_Lock);
    return nPos;
}

bool NVSHandlerMgr::getDVRWorkState(intptr_t identify,char dvrWorkState[],int arrayLen)
{
    EnterCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO ,"getDVRWorkState In...");
    IDVRHandler* pHandler = (IDVRHandler*)identify;

    if(NULL == pHandler)
    {
        m_log.LogInfoEx(LOG_ERROR , "传递的句柄为空");
        LeaveCriticalSection(&m_Lock);
        return false;
    }
    bool bOK = pHandler->getDVRWorkState(dvrWorkState,arrayLen);
    m_log.LogInfoEx(LOG_INFO ,"getDVRWorkState Out...");
    LeaveCriticalSection(&m_Lock);
    return bOK;
}

bool NVSHandlerMgr::getDVRIPByResolveSvr(intptr_t identify,char *sServerIP,int wServerPort,char *sDVRName,int wDVRNameLen,char *sDVRSerialNumber,int wDVRSerialLen,char* sGetIP)
{
    EnterCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO ,"getDVRIPByResolveSvr In...");
    IDVRHandler* pHandler = (IDVRHandler*)identify;

    if(NULL == pHandler)
    {
        m_log.LogInfoEx(LOG_ERROR , "传递的句柄为空");
        LeaveCriticalSection(&m_Lock);
        return false;
    }
    bool bOK = pHandler->getDVRIPByResolveSvr(sServerIP,wServerPort,sDVRName,wDVRNameLen,sDVRSerialNumber,wDVRSerialLen,sGetIP);
    m_log.LogInfoEx(LOG_INFO ,"getDVRIPByResolveSvr Out...");
    LeaveCriticalSection(&m_Lock);
    return bOK;
}

bool NVSHandlerMgr::setConnectTime(intptr_t identify,DWORD dwWaitTime,DWORD dwTryTimes)
{
    EnterCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO ,"setConnectTime In...");
    IDVRHandler* pHandler = (IDVRHandler*)identify;

    if(NULL == pHandler)
    {
        m_log.LogInfoEx(LOG_ERROR , "传递的句柄为空");

        LeaveCriticalSection(&m_Lock);
        return false;
    }
    bool bOK = pHandler->setConnectTime(dwWaitTime,dwTryTimes);
    m_log.LogInfoEx(LOG_INFO ,"setConnectTime Out...");
    LeaveCriticalSection(&m_Lock);
    return bOK;
}

bool NVSHandlerMgr::startListen (intptr_t identify,char *sLocalIP,int wLocalPort)
{
    EnterCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO ,"startListen In...");
    IDVRHandler* pHandler = (IDVRHandler*)identify;

    if(NULL == pHandler)
    {
        m_log.LogInfoEx(LOG_ERROR ,  "传递的句柄为空");
        LeaveCriticalSection(&m_Lock);
        return false;
    }
    bool bOK = pHandler->startListen(sLocalIP,wLocalPort);
    m_log.LogInfoEx(LOG_INFO ,"startListen Out...");
    LeaveCriticalSection(&m_Lock);
    return bOK;
}

bool NVSHandlerMgr::stopListen(intptr_t identify)
{
    EnterCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO ,"stopListen In...");
    IDVRHandler* pHandler = (IDVRHandler*)identify;

    if(NULL == pHandler)
    {
        m_log.LogInfoEx(LOG_ERROR , "传递的句柄为空");
        LeaveCriticalSection(&m_Lock);
        return false;
    }
    bool bOK = pHandler->stopListen();
    m_log.LogInfoEx(LOG_INFO ,"stopListen Out...");
    LeaveCriticalSection(&m_Lock);
    return bOK;
}

bool NVSHandlerMgr::serialSend(intptr_t identify,char* pBuf,DWORD nLength,char* pBuf2,DWORD nLen2)
{
    EnterCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO ,"serialSend In...");
    IDVRHandler* pHandler = (IDVRHandler*)identify;

    if(NULL == pHandler)
    {
        m_log.LogInfoEx(LOG_ERROR ,"传递的句柄为空");
        LeaveCriticalSection(&m_Lock);
        return false;
    }
    m_log.LogInfoEx(LOG_WARNING , "指令1=%s, 指令2=%s", pBuf, pBuf2);
    bool bOK = pHandler->serialSend(pBuf,nLength,pBuf2,nLen2);
    m_log.LogInfoEx(LOG_ERROR ,"serialSend Out...");
    LeaveCriticalSection(&m_Lock);
    return bOK;
}

bool NVSHandlerMgr::serialSendWithCallBack(intptr_t identify,char* pBuf,DWORD nLength,int nCameraID)
{
    EnterCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO ,"serialSendWithCallBack In...");
    IDVRHandler* pHandler = (IDVRHandler*)identify;
    if(NULL == pHandler)
    {
        m_log.LogInfoEx(LOG_ERROR , "传递的句柄为空");
        LeaveCriticalSection(&m_Lock);
        return false;
    }

    m_log.LogInfoEx(LOG_WARNING , "指令=%s", pBuf);
    bool bOK = pHandler->serialSendWithCallBack(pBuf,nLength,nCameraID);
    m_log.LogInfoEx(LOG_INFO ,"serialSendWithCallBack Out...");
    LeaveCriticalSection(&m_Lock);
    return bOK;
}

bool NVSHandlerMgr::buildSerialSend(intptr_t identify,LONG lSerialPort,LPSERIALDATACALLBACK pSerialCallBack,bool bOpen)
{
    EnterCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO ,"buildSerialSend In...");
    IDVRHandler* pHandler = (IDVRHandler*)identify;
    if(NULL == pHandler)
    {
        m_log.LogInfoEx(LOG_ERROR , "传递的句柄为空");
        LeaveCriticalSection(&m_Lock);
        return false;
    }
    bool bOK = pHandler->buildSerialSend(lSerialPort,pSerialCallBack,bOpen);
    m_log.LogInfoEx(LOG_INFO ,"buildSerialSend Out...");
    LeaveCriticalSection(&m_Lock);
    return bOK;
}

bool NVSHandlerMgr::clientAudioStart(intptr_t identify)
{
    IDVRHandler* pHandler = (IDVRHandler*)identify;

    if(NULL == pHandler)
    {
        return false;
    }
    bool bOK = pHandler->clientAudioStart();
    return bOK;
}
bool NVSHandlerMgr::clientAudioStop(intptr_t identify)
{
    IDVRHandler* pHandler = (IDVRHandler*)identify;
    if(NULL == pHandler)
    {
        return false;
    }

    bool bOK = pHandler->clientAudioStop();
    return bOK;
}
bool NVSHandlerMgr::addDVR(intptr_t identify)
{
    IDVRHandler* pHandler = (IDVRHandler*)identify;

    if(NULL == pHandler)
    {
        return false;
    }
    bool bOK = pHandler->addDVR();
    return bOK;
}
bool NVSHandlerMgr::delDVR(intptr_t identify)
{
    IDVRHandler* pHandler = (IDVRHandler*)identify;

    if(NULL == pHandler)
    {
        return false;
    }
    bool bOK = pHandler->delDVR();
    return bOK;
}
bool NVSHandlerMgr::setVoiceComClientVolume(intptr_t identify,WORD wVolume)
{
    IDVRHandler* pHandler = (IDVRHandler*)identify;

    if(NULL == pHandler)
    {
        return false;
    }
    bool bOK = pHandler->setVoiceComClientVolume(wVolume);
    return bOK;
}

//2007-05-23 增加
int NVSHandlerMgr::getDecoderAddress(intptr_t identify,LONG lChannelIndex)
{
    //EnterCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO ,"getDecoderAddress In...");
    IDVRHandler* pHandler = (IDVRHandler*)identify;

    if(NULL == pHandler)
    {
        //LeaveCriticalSection(&m_Lock);
        m_log.LogInfoEx(LOG_ERROR , "传递的句柄为空");

        return false;
    }
    int nReturn = pHandler->getDecoderAddress(lChannelIndex);
    m_log.LogInfoEx(LOG_INFO ,"getDecoderAddress Out...");
    //LeaveCriticalSection(&m_Lock);
    return nReturn;
}

bool NVSHandlerMgr::capturePictureEx(intptr_t identify, char* sFilePath)
{
    m_log.LogInfoEx(LOG_INFO ,"capturePictureEx In...");
    IDVRHandler* pHandler = (IDVRHandler*)identify;

    if(NULL == pHandler)
    {
        m_log.LogInfoEx(LOG_ERROR , "传递的句柄为空");
        return false;
    }
    m_log.LogInfoEx(LOG_INFO ,"capturePictureEx Out...");
    return pHandler->capturePicture(sFilePath);
}

void NVSHandlerMgr::makeKey(intptr_t identify)
{
    IDVRHandler* pHandler = (IDVRHandler*)identify;

    if(NULL == pHandler)
        return;
    pHandler->makeKey();
}

void NVSHandlerMgr::reSetDDrawDevice(intptr_t identify, HWND hWnd)
{
    IDVRHandler* pHandler = (IDVRHandler*)identify;
    if(NULL == pHandler)
        return;
    pHandler->reSetDDrawDevice(hWnd);
}

bool NVSHandlerMgr::controlCameraSound(intptr_t identify, bool bOpenSound)
{
	 m_log.LogInfoEx(LOG_INFO ,"controlCameraSound In1...");
    IDVRHandler* pHandler = (IDVRHandler*)identify;
    if(NULL == pHandler)
        return false;
     m_log.LogInfoEx(LOG_INFO ,"controlCameraSound In2...");
	return pHandler->controlCameraSound(bOpenSound);
}

bool NVSHandlerMgr::controlPTZSpeedWithChannel(intptr_t identify,int nCmd, bool bEnable, int nSpeed, int nChannel)
{
    IDVRHandler* pHandler = (IDVRHandler*)identify;
    if(NULL == pHandler)
        return false;
    return pHandler->controlPTZSpeedWithChannel( nCmd,  bEnable,  nSpeed,  nChannel);

}

bool NVSHandlerMgr::presetPTZWithChannel(intptr_t identify, int nCmd, int nIndex, int nChannel)
{
    IDVRHandler* pHandler = (IDVRHandler*)identify;
    if(NULL == pHandler)
        return false;
    return pHandler->presetPTZWithChannel(  nCmd,  nIndex,  nChannel);

}

bool NVSHandlerMgr::getPlayBackOsdTime(intptr_t identify , intptr_t  lReplayHandle, char *pOsdTime)
{
    IDVRHandler* pHandler = (IDVRHandler*)identify;
    if(NULL == pHandler)
        return false;
    return pHandler->getPlayBackOsdTime(lReplayHandle,pOsdTime);

}

void NVSHandlerMgr::setCurrentChannel(intptr_t identify ,int nChannel)
{
    IDVRHandler* pHandler = (IDVRHandler*)identify;
    if(NULL == pHandler)
        return;

    pHandler->setCurrentChannel(nChannel);
}

DWORD NVSHandlerMgr::getLastError(intptr_t identify)
{
    IDVRHandler* pHandler = (IDVRHandler*)identify;
    if(NULL == pHandler)
        return 0;
    return pHandler->getLastError();
}
bool NVSHandlerMgr::playBackCaptureFile(intptr_t identify , intptr_t  lReplayHandle, char *pFileName)
{
    IDVRHandler* pHandler = (IDVRHandler*)identify;
    if(NULL == pHandler)
        return false;
    return pHandler->playBackCaptureFile(lReplayHandle,pFileName);
}

bool NVSHandlerMgr::setCapturePictureMode(intptr_t identify ,DWORD dwCaptureMode)
{
    IDVRHandler* pHandler = (IDVRHandler*)identify;
    if (NULL == pHandler)
        return false;
    return pHandler->setCapturePictureMode(dwCaptureMode);
}
intptr_t NVSHandlerMgr::getCameraInfo(intptr_t identify ,int *nSize)
{
    IDVRHandler* pHandler = (IDVRHandler*)identify;
    if(NULL == pHandler)
        return 0;
    return pHandler->getCameraInfo(nSize);
}

int NVSHandlerMgr::setPlayWindow(intptr_t identify ,HWND palyWindow[], int nMaxCount)
{
    IDVRHandler* pHandler = (IDVRHandler*)identify;
    if(NULL == pHandler)
        return false;
    return pHandler->setPlayWindow(palyWindow,nMaxCount);
}


bool NVSHandlerMgr::startPlayerByCamera(intptr_t identify, HWND hwndPlay, int nCameraID)
{
    m_log.LogInfoEx(LOG_INFO ,"startPlayerByCamera In...");
    IDVRHandler* pHandler = (IDVRHandler*)identify;
    if(NULL == pHandler)
        return false;
    m_log.LogInfoEx(LOG_INFO ,"startPlayerByCamera Out...");
    return pHandler->startPlayerByCamera(hwndPlay,nCameraID);
}

bool  NVSHandlerMgr::stopPlayerByCamera(intptr_t identify, HWND hwndPlay, int nCameraID)
{
    IDVRHandler* pHandler = (IDVRHandler*)identify;
    if(NULL == pHandler)
        return false;

	//add by zjf
	map<string, intptr_t>::iterator itCID = m_mapCameraIDToHandler.begin();
	while(itCID != m_mapCameraIDToHandler.end())
	{
		if(itCID->second == identify)
		{
			m_mapCameraIDToHandler.erase(itCID);             //删除Handler
			break;
		}
		else
		{
			itCID ++;
		}
	}
	//add over

    pHandler->stopPlayerByCamera(hwndPlay,nCameraID);

	return true;
}

bool NVSHandlerMgr::queryPorperySupport(intptr_t identify, XYPROPROPERTY propery)
{
    IDVRHandler* pHandler = (IDVRHandler*)identify;
    if(NULL == pHandler)
        return false;
    return pHandler->queryPorperySupport(propery);
}

void NVSHandlerMgr::setCurrentCameraCode(intptr_t identify,char* sCameraCode)
{
    IDVRHandler* pHandler = (IDVRHandler*)identify;
    if(NULL == pHandler)
    {
        return;
    }
	m_mapCameraIDToHandler[sCameraCode] = identify;           //add by zjf, 保存播放镜头的Handler
    pHandler->setCurrentCameraCode(sCameraCode);
}
intptr_t NVSHandlerMgr::openVideoFile(intptr_t identify, char *sFileName)
{
    IDVRHandler* pHandler = (IDVRHandler*)identify;
    if(NULL == pHandler)
        return false;
    return pHandler->openVideoFile(sFileName);
}
int NVSHandlerMgr::readVideoFile(intptr_t identify, char *pBuff,int nLen, int *pOutStreamType)
{
    IDVRHandler* pHandler = (IDVRHandler*)identify;
    if(NULL == pHandler)
        return false;
    return pHandler->readVideoFile(pBuff, nLen, pOutStreamType);
}
bool NVSHandlerMgr::closeVideoFile(intptr_t identify )
{
    IDVRHandler* pHandler = (IDVRHandler*)identify;
    if(NULL == pHandler)
        return false;
    return pHandler->closeVideoFile();
}
bool NVSHandlerMgr::controlPlay(intptr_t identify, int nCmd,int param,int *outValue)
{
    IDVRHandler* pHandler = (IDVRHandler*)identify;
    if(NULL == pHandler)
        return false;
    return pHandler->controlPlay( nCmd, param, outValue);
}

intptr_t NVSHandlerMgr::captureFileStream(intptr_t identify,LPCAPTURESTREAMCALLBACK lpCallBack, void* pUser, int nStreamType, RECORDFILE FileInfo, char* sType)
{
    IDVRHandler* pHandler = (IDVRHandler*)identify;
    if(NULL == pHandler)
        return false;
    return pHandler->captureFileStream( lpCallBack, pUser, nStreamType, FileInfo, sType);
}

bool NVSHandlerMgr::startRecord(intptr_t identify, char* saveFile)
{
    IDVRHandler* pHandler = (IDVRHandler*)identify;
    if(NULL == pHandler)
        return false;
    return pHandler->startRecord( saveFile);
}

bool NVSHandlerMgr::stopRecord(intptr_t identify)
{
    IDVRHandler* pHandler = (IDVRHandler*)identify;
    if(NULL == pHandler)
        return false;
    return pHandler->stopRecord();
}
void NVSHandlerMgr::ClearPlayBuffer(intptr_t identify)
{
    IDVRHandler* pHandler = (IDVRHandler*)identify;
    if(NULL == pHandler)
        return ;
    pHandler->ClearPlayBuffer();

}

void NVSHandlerMgr::pusePlayDecode(intptr_t identify,bool bPuse)
{
    IDVRHandler* pHandler = (IDVRHandler*)identify;
    if(NULL == pHandler)
        return ;
    pHandler->pusePlayDecode(bPuse);

}

void NVSHandlerMgr::WriteSysLog(std::string sLongText)
{
    //    char strtime[23];
    //    SYSTEMTIME t;
    //    ::GetLocalTime(&t);
    //    sprintf_s(strtime,sizeof(strtime) ,"%02d-%02d-%02d %02d:%02d:%02d ",t.wYear,t.wMonth,t.wDay,t.wHour,t.wMinute,t.wSecond);

    //    string sTotalLog = strtime;
    //    sTotalLog += sLongText;
    //    sTotalLog += "\r\n";
    //    DWORD nWritten;
    //    ::WriteFile(m_hFileHandle, sTotalLog.c_str(), (DWORD)sTotalLog.length(), &nWritten, NULL);
}

string NVSHandlerMgr::GetFilePath(const char *sType, char *pDir)
{
	if(sType != NULL)
	{
		char strLog[512]={0};
		char sCurName[MAX_PATH] = {0};
		char szFullPath[MAX_PATH] = {0};
		char szDir[MAX_PATH] = {0};
		char szDrive[MAX_PATH] = {0};
		char szPlugPath[MAX_PATH]={0};

		::GetModuleFileName(NULL,szFullPath,MAX_PATH); 
		if(pDir != NULL)
		{
			strcpy_s(szFullPath,MAX_PATH,pDir);
		}
		_splitpath_s(szFullPath,szDrive,sizeof(szDrive),szDir,sizeof(szDir),NULL,NULL,NULL,NULL);
		sprintf_s(sCurName, sizeof(char)*256, "%s%sPlugIn\\%sPlugIn\\",szDrive,szDir, sType);
		m_log.LogInfoEx(LOG_INFO , "GetFilePath end :%s .",sCurName);
		return sCurName;
	}

    char sFilePath[256] = {0};

    if ( GetModuleFileName( NULL, sFilePath, 256 ) == 0 )
    {
        return false;
    }
    string sFileName = sFilePath;

#ifdef _WIN32
    int nLen = (int)sFileName.find_last_of("\\");
    if(nLen < 0)
    {
        return "ERROR";
    }

    sFileName = sFileName.substr(0,nLen+1);
#else
    sFileName += "/";
#endif
    //sFileName += "LogConfig.properties";
    return sFileName;
}

intptr_t NVSHandlerMgr::connectDVREx(PCONNECT_PARAM pCParam , BOOL &BRes)
{
    EnterCriticalSection(&m_Lock);

    BRes = FALSE;
    m_log.LogInfoEx(LOG_INFO ,"connectDVR In...");

    string sDll = pCParam->sType;
	m_log.LogInfoEx(LOG_INFO , "DLL FileName is %s" , sDll.c_str());
    IDVRHandler* pHandler = NULL;
    char strLog[256]={0};

    sDll += NETPLUGIN;
	int nRet = XSLoadLibrary((char *)pCParam->sType ,sDll);
	if(nRet < 0)
	{
		LeaveCriticalSection(&m_Lock);
		return nRet;
	}

    //if(m_mDllFunction[sDll] == NULL)
    //{
    //    //        char sCurName[256] = {0};
    //    //        char szFullPath[MAX_PATH] = {0};
    //    //        char szDir[MAX_PATH] = {0};
    //    //        char szDrive[MAX_PATH] = {0};
    //    //        ::GetModuleFileName(NULL,szFullPath,MAX_PATH);
    //    //        _splitpath_s(szFullPath,szDrive,sizeof(szDrive),szDir,sizeof(szDir),NULL,NULL,NULL,NULL);
    //    //        sprintf_s(sCurName,sizeof(char)*256,"%s%s%s",szDrive,szDir,sType);

    //    string sFilePath = GetFilePath(pCParam->sType);
    //    sFilePath += sDll;
    //    //库加载
    //    HINSTANCE hHandle = ::LoadLibrary(sFilePath.c_str());
    //    if(NULL == hHandle)
    //    {
    //        m_log.LogInfoEx(LOG_INFO ,"加载当前目录动态库%s失败，开始加载系统目录动态库%s", sFilePath.c_str(), pCParam->sType);
    //        hHandle = ::LoadLibrary(pCParam->sType);
    //        if(NULL == hHandle)
    //        {
    //            m_log.LogInfoEx(LOG_ERROR ,"加载系统目录动态库%s失败 ", pCParam->sType);
    //            m_log.LogInfoEx(LOG_INFO ,"connectDVR Out...");
    //            LeaveCriticalSection(&m_Lock);
    //            return -2;
    //        }
    //        m_log.LogInfoEx(LOG_INFO , "加载系统目录动态库%s成功 ", pCParam->sType);
    //    }
    //    else
    //        m_log.LogInfoEx(LOG_INFO , "加载当前目录动态库%s成功 ", sFilePath.c_str());
    //    m_mDllFunction[sDll] = hHandle;
    //}
    LeaveCriticalSection(&m_Lock);

    GetDVRHandler pGetDVRHandler = (GetDVRHandler)::GetProcAddress(m_mDllFunction[sDll],"getHandler");
    if(pGetDVRHandler == NULL)
    {
        m_log.LogInfoEx(LOG_ERROR ,"获取导出函数句柄失败 ");
        // LeaveCriticalSection(&m_Lock);
        return -2;
    }
    m_log.LogInfoEx(LOG_INFO ,"获取导出函数句柄成功 ");

    pHandler = pGetDVRHandler();
    if(NULL == pHandler)
    {
        //LeaveCriticalSection(&m_Lock);
        m_log.LogInfoEx(LOG_ERROR , "获取DVR句柄失败 ");
        return -2;
    }
    m_log.LogInfoEx(LOG_INFO , "获取DVR句柄成功 ");

    if(!pHandler->connectDVREx(pCParam))
    {
        pHandler->releaseHandler();
        pHandler = NULL;
        m_log.LogInfoEx(LOG_ERROR , "连接DVR失败 ");
        //LeaveCriticalSection(&m_Lock);
        return -3;
    }
    m_log.LogInfoEx(LOG_INFO , "连接DVR成功 ");
    m_log.LogInfoEx(LOG_INFO ,"connectDVR Out...");
    BRes = TRUE;
    return reinterpret_cast<intptr_t>(pHandler);
}

intptr_t NVSHandlerMgr::downloadRecordFileEx(intptr_t identify,RECORDFILE RecordFile,char* saveFile,LPDOWNLOADRECORDFILECALLBACK lpCallBack,void* pUser)
{
    IDVRHandler* pHandler = (IDVRHandler*)identify;

    if(NULL == pHandler)
    {

        return false;
    }


	intptr_t ldownFile = pHandler->downloadRecordFileEx(RecordFile, saveFile, lpCallBack, pUser);

    return ldownFile;
}

LONG NVSHandlerMgr::checkDVREx(PCONNECT_PARAM pCParam,CLInfo pInfo[],int nMaxCount)
{
    EnterCriticalSection(&m_Lock);
    string sDll = pCParam->sType;

    sDll += NETPLUGIN;
	m_log.LogInfoEx(LOG_INFO , "DLL FileName is %s" , sDll.c_str());

	int nRet = XSLoadLibrary((char *)pCParam->sType ,sDll);
	if(nRet < 0)
	{
		LeaveCriticalSection(&m_Lock);
		return nRet;
	}
    //if(m_mDllFunction[sDll] == NULL)
    //{

    //    string sFilePath = GetFilePath(pCParam->sType);
    //    sFilePath += sDll;
    //    //库加载
    //    HINSTANCE hHandle = ::LoadLibrary(sFilePath.c_str());

    //    if(NULL == hHandle)
    //    {
    //        hHandle = ::LoadLibrary(pCParam->sType);
    //        if(NULL == hHandle)
    //        {
    //            LeaveCriticalSection(&m_Lock);
    //            return -2;
    //        }
    //    }
    //    else
    //        m_log.LogInfoEx(LOG_INFO , "加载当前目录动态库%s成功", pCParam->sType);
    //    m_mDllFunction[sDll] = hHandle;
    //}

    GetDVRHandler pGetDVRHandler = (GetDVRHandler)::GetProcAddress(m_mDllFunction[sDll],"getHandler");
    if(pGetDVRHandler == NULL)
    {
        m_log.LogInfoEx(LOG_ERROR , "获取导出函数句柄失败");
        LeaveCriticalSection(&m_Lock);
        return -3;
    }
    m_log.LogInfoEx(LOG_INFO , "获取导出函数句柄成功");

    IDVRHandler* pHandler = pGetDVRHandler();
    if(NULL == pHandler)
    {
        m_log.LogInfoEx(LOG_ERROR , "获取DVR句柄失败");
        LeaveCriticalSection(&m_Lock);
        return -4;
    }
    m_log.LogInfoEx(LOG_INFO , "获取DVR句柄成功");

    LONG lResult = pHandler->checkDVREx(pCParam, pInfo, nMaxCount);
    m_log.LogInfoEx(LOG_WARNING , "checkDVR返回值：%d", lResult);
    pHandler->releaseHandler();
    pHandler = NULL;
    m_log.LogInfoEx(LOG_INFO ,"checkDVR Out...");
    LeaveCriticalSection(&m_Lock);
    return lResult;
}

void NVSHandlerMgr::setDVREx(PCONNECT_PARAM pCParam,CLInfo pInfo[],int nInfoCount)
{
    EnterCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO ,"setDVR In...");
    string sDll = pCParam->sType;

    sDll += NETPLUGIN;
	m_log.LogInfoEx(LOG_INFO , "DLL FileName is %s" , sDll.c_str());

	int nRet = XSLoadLibrary((char *)pCParam->sType ,sDll);
	if(nRet < 0)
	{
		LeaveCriticalSection(&m_Lock);
		return;
	}

    GetDVRHandler pGetDVRHandler = (GetDVRHandler)::GetProcAddress(m_mDllFunction[sDll],"getHandler");
    if(pGetDVRHandler == NULL)
    {
        m_log.LogInfoEx(LOG_ERROR , "获取导出函数句柄失败");
        LeaveCriticalSection(&m_Lock);
        return;
    }
    m_log.LogInfoEx(LOG_INFO , " 获取导出函数句柄成功");

    IDVRHandler* pHandler = pGetDVRHandler();
    if(NULL == pHandler)
    {
        m_log.LogInfoEx(LOG_ERROR , "获取DVR句柄失败");
        LeaveCriticalSection(&m_Lock);
        return;
    }
    m_log.LogInfoEx(LOG_INFO , "获取DVR句柄成功");

    pHandler->setDVREx(pCParam,pInfo,nInfoCount);
    pHandler->releaseHandler();
    pHandler = NULL;
    m_log.LogInfoEx(LOG_INFO ,"setDVR Out...");
    LeaveCriticalSection(&m_Lock);
    return;
}

bool NVSHandlerMgr::captureStreamEx(intptr_t identify, PCAPTURE_PARAM pParam)
{
    EnterCriticalSection(&m_Lock);
    m_log.LogInfoEx(LOG_INFO ,"captureStream In...");
    IDVRHandler* pHandler = (IDVRHandler*)identify;

    if(NULL == pHandler)
    {
        m_log.LogInfoEx(LOG_ERROR , "传递的句柄为空");
        LeaveCriticalSection(&m_Lock);
        return false;
    }
    bool bOK = pHandler->captureStreamEx(pParam);
    if (!bOK)
    {
        m_log.LogInfoEx(LOG_ERROR , "调用插件捕获流失败 ");
    }
    else
    {
        m_log.LogInfoEx(LOG_INFO , "调用插件捕获流成功 ");
    }

    m_log.LogInfoEx(LOG_INFO ,"captureStream Out...");
    LeaveCriticalSection(&m_Lock);

    return bOK;
}

intptr_t NVSHandlerMgr::replayRecordFileEx(intptr_t identify, HWND hWnd,RECORDFILE RecordFile)
{
    IDVRHandler* pHandler = (IDVRHandler*)identify;

    if(NULL == pHandler)
    {

        return false;
    }
	intptr_t lReturn = pHandler->replayRecordFileEx(hWnd,RecordFile);

    return lReturn;
}

intptr_t NVSHandlerMgr::initialPlayerEx(char* sType , PINITPY_PARAM pIParam)
{
    string sDll = sType;

    sDll += PLAYPLUGIN;
	m_log.LogInfoEx(LOG_INFO , "DLL FileName is %s" , sDll.c_str());

	int nRet = XSLoadLibrary(sType ,sDll);
	if(nRet < 0)
	{
		return nRet;
	}

    m_log.LogInfoEx(LOG_INFO  , "动态库句柄：%d", m_mDllFunction[sDll]);
    GetDVRHandler pGetDVRHandler = (GetDVRHandler)::GetProcAddress(m_mDllFunction[sDll],"getHandler");
    if(pGetDVRHandler == NULL)
    {
        m_log.LogInfoEx(LOG_ERROR , "获取导出函数句柄失败");
        return 0;
    }
    m_log.LogInfoEx(LOG_INFO , "获取导出函数句柄成功");

    IDVRHandler* pHandler = pGetDVRHandler();
    if(NULL == pHandler)
    {
        m_log.LogInfoEx(LOG_ERROR , "获取DVR句柄失败");
        return 0;
    }
    m_log.LogInfoEx(LOG_INFO , "获取DVR句柄成功");

    if(pHandler->initialPlayerEx(pIParam))
    {
        m_log.LogInfoEx(LOG_INFO , "播放初始化成功 ");
        return reinterpret_cast<intptr_t>(pHandler);
    }
    else
    {
        m_log.LogInfoEx(LOG_ERROR , "播放初始化失败 ");
    }

    pHandler->releaseHandler();
    pHandler = NULL;
    m_log.LogInfoEx(LOG_INFO ,"initialPlayer Out...");
    return 0;
}

bool NVSHandlerMgr::startPlayer4StandardEx(intptr_t identify, PSTARTPYS_PARAM pParam)
{
    IDVRHandler* pHandler = (IDVRHandler*)identify;

    if(NULL == pHandler)
    {
        return false;
    }

    bool bOK = pHandler->startPlayer4StandardEx(pParam);
    return bOK;
}

intptr_t NVSHandlerMgr::locPlay_OpenFileEx(char* sType , PLOCPY_OPENF_PARAM_PARAM pParam)
{
    string sDll = sType;

    sDll += PLAYPLUGIN;

	m_log.LogInfoEx(LOG_INFO , "DLL FileName is %s" , sDll.c_str());

	int nRet = XSLoadLibrary(sType ,sDll);
	if(nRet < 0)
	{
		return nRet;
	}

    GetDVRHandler pGetDVRHandler = (GetDVRHandler)::GetProcAddress(m_mDllFunction[sDll],"getHandler");
    if(pGetDVRHandler == NULL)
    {
        //LeaveCriticalSection(&m_Lock);
        m_log.LogInfoEx(LOG_ERROR , "获取导出函数失败");
        return 0;
    }

    IDVRHandler* pHandler = pGetDVRHandler();
    if(NULL == pHandler)
    {
        //LeaveCriticalSection(&m_Lock);
        m_log.LogInfoEx(LOG_ERROR , "获取DVR句柄失败");
        return 0;
    }
    if(pHandler->locPlay_OpenFileEx(pParam))
    {
        //LeaveCriticalSection(&m_Lock);
        m_log.LogInfoEx(LOG_INFO , "打开文件成功,文件名称: %s", pParam->fileName);
        return reinterpret_cast<intptr_t>(pHandler);
    }
    pHandler->releaseHandler();
    pHandler = NULL;
    m_log.LogInfoEx(LOG_ERROR , "打开文件失败,文件名称: %s", pParam->fileName);
    m_log.LogInfoEx(LOG_INFO ,"locPlay_OpenFile Out...");
    //LeaveCriticalSection(&m_Lock);
    return 0;
}

bool NVSHandlerMgr::controlPTZSpeedWithChannelEx(intptr_t identify, PCONTROL_PARAM pParam)
{
    IDVRHandler* pHandler = (IDVRHandler*)identify;
    if(NULL == pHandler)
        return false;
    return pHandler->controlPTZSpeedWithChannelEx( pParam);
}


intptr_t NVSHandlerMgr::getIdentify(char * sCameraID, bool bFlag)
{    
	//zjf add 
	map<string, intptr_t>::iterator itCID = m_mapCameraIDToHandler.find(sCameraID);
	if(itCID == m_mapCameraIDToHandler.end())
	{
		return 0;
	}
	return itCID->second;           //取Handler
	//zjf add over

	if(NULL == m_mapCameraIDToHandler[sCameraID])
	{
		return 0;
	}
	/*HANDLE m_hSaveFile= ::CreateFile("C:\\RTPDecode.txt",GENERIC_WRITE,FILE_SHARE_WRITE,NULL,
	OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	SetFilePointer(m_hSaveFile,0,0,FILE_END);
	DWORD ndw = 0;
	char pLog[512] = {0};
	sprintf_s(pLog, "getIdentify: CameraID[%s], Flag[%d] [ID: %d]\r\n", sCameraID, bFlag, m_mapCameraIDToHandler[sCameraID]);    
	::WriteFile(m_hSaveFile, pLog, strlen(pLog), &ndw, NULL);
	CloseHandle(m_hSaveFile);*/
	return m_mapCameraIDToHandler[sCameraID];
}
bool NVSHandlerMgr::SetCropArea(intptr_t identify, bool bFlag, int x1, int y1, int x2, int y2)
{
	HANDLE m_hSaveFile= ::CreateFile("C:\\RTPDecode.txt",GENERIC_WRITE,FILE_SHARE_WRITE,NULL,
		OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	SetFilePointer(m_hSaveFile,0,0,FILE_END);
	DWORD ndw = 0;
	char pLog[512] = {0};
	sprintf_s(pLog, "NVSSDK: SetCropArea[id:%Id][flag: %d][%d %d %d %d]\r\n", identify, bFlag, x1, y1, x2, y2);    
	::WriteFile(m_hSaveFile, pLog, (int)strlen(pLog), &ndw, NULL);
	CloseHandle(m_hSaveFile);

	IDVRHandler* pHandler = (IDVRHandler*)identify;
	if(NULL == pHandler)
	{
		return false ;
	}
	return pHandler->SetCropArea(bFlag, x1, y1, x2, y2);
}
bool NVSHandlerMgr::SetDecodeParam(intptr_t identify, int nCmd, bool bFlag, int x1, int y1, int x2, int y2)
{
	IDVRHandler* pHandler = (IDVRHandler*)identify;
	if(NULL == pHandler)
	{
		return false;
	}
	return pHandler->SetDecodeParam(nCmd, bFlag, x1, y1, x2, y2);
}
bool NVSHandlerMgr::SetYUVCallBack(intptr_t identify, LPYUVCALLBACK pCallBack)
{
	IDVRHandler* pHandler = (IDVRHandler*)identify;
	if(NULL == pHandler)
	{
		return false;
	}
	return pHandler->SetYUVCallBack(pCallBack);
}
bool NVSHandlerMgr::SetStreamSaveTime(intptr_t identify, int nSaveTime)
{
	IDVRHandler* pHandler = (IDVRHandler*)identify;
	if(NULL == pHandler)
	{
		return false;
	}
	return pHandler->SetStreamSaveTime(nSaveTime);
}
bool NVSHandlerMgr::PlayBackSaveStream(intptr_t identify, HWND hWnd)
{
	IDVRHandler* pHandler = (IDVRHandler*)identify;
	if(NULL == pHandler)
	{
		return false;
	}
	return pHandler->PlayBackSaveStream(hWnd);
}
bool NVSHandlerMgr::StopPlayBackSaveStream(intptr_t identify)
{
	if(identify == 0)
	{
		return false;
	}
	IDVRHandler* pHandler = (IDVRHandler*)identify;
	if(NULL == pHandler)
	{
		return false;
	}
	return pHandler->StopPlayBackSaveStream();
}

//2013 0927 pzm add
bool NVSHandlerMgr::SetRenderMode(intptr_t identify, int nMode)
{
	if(identify == 0)
	{
		return false;
	}
	IDVRHandler* pHandler = (IDVRHandler*)identify;
	if(NULL == pHandler)
	{
		return false;
	}
	return pHandler->SetRenderMode(nMode);
}

bool NVSHandlerMgr::SetPosByFrameNum(intptr_t identify,DWORD nFrameNum)
{
	if(identify == 0)
	{
		return false;
	}
	IDVRHandler* pHandler = (IDVRHandler*)identify;
	if(NULL == pHandler)
	{
		return false;
	}
	return pHandler->SetPosByFrameNum(nFrameNum);
}


bool NVSHandlerMgr::GetVideoCut(intptr_t identify, DWORD beginFrame, DWORD endFrame, LPSTR filename)
{
	if(identify == 0)
	{
		return false;
	}
	IDVRHandler* pHandler = (IDVRHandler*)identify;
	if(NULL == pHandler)
	{
		return false;
	}
	return pHandler->GetVideoCut(beginFrame,  endFrame,  filename);
}

bool NVSHandlerMgr::StopCutVideo(intptr_t identify)
{
	if(identify == 0)
	{
		return false;
	}
	IDVRHandler* pHandler = (IDVRHandler*)identify;
	if(NULL == pHandler)
	{
		return false;
	}
	return pHandler->StopCutVideo();
}

bool NVSHandlerMgr::SetFileEndCallback(intptr_t identify, void *pUser,pFileOverCB pCallBack)
{
	if(identify == 0)
	{
		return false;
	}
	IDVRHandler* pHandler = (IDVRHandler*)identify;
	if(NULL == pHandler)
	{
		return false;
	}
	return pHandler->SetFileEndCallback( pUser,pCallBack);
}

bool NVSHandlerMgr::GetMediaInfo(intptr_t identify,MediaInf &mInf)
{
	if(identify == 0)
	{
		return false;
	}
	IDVRHandler* pHandler = (IDVRHandler*)identify;
	if(NULL == pHandler)
	{
		return false;
	}
	return pHandler->GetMediaInfo(mInf);
}

bool NVSHandlerMgr::GetLightParam(LONG nPort,OUT int *gray1,OUT int *gray2)
{
	return false;
	//m_log.LogInfoEx(LOG_INFO ,"GetLightParam In...");
	//string sDll = sType;

	//sDll += PLAYPLUGIN;
	//m_log.LogInfoEx(LOG_INFO , "DLL FileName is %s" , sDll.c_str());

	//int nRet = XSLoadLibrary(sType ,sDll);
	//if(nRet < 0)
	//{
	//	return nRet;
	//}

	//GetDVRHandler pGetDVRHandler = (GetDVRHandler)::GetProcAddress(m_mDllFunction[sDll],"getHandler");
	//if(pGetDVRHandler == NULL)
	//{
	//	//LeaveCriticalSection(&m_Lock);
	//	m_log.LogInfoEx(LOG_ERROR , "获取导出函数getHandler句柄失败 ");
	//	return -1;
	//}
	//m_log.LogInfoEx(LOG_ERROR , "获取导出函数getHandler句柄成功 ");

	//IDVRHandler* pHandler = pGetDVRHandler();
	//if(NULL == pHandler)
	//{
	//	//LeaveCriticalSection(&m_Lock);
	//	m_log.LogInfoEx(LOG_ERROR , "获取DVR句柄失败 ");
	//	return -3;
	//}
	//m_log.LogInfoEx(LOG_ERROR , "获取DVR句柄成功");

	//if(pHandler->GetLightParam(nPort,gray1,gray2))
	//{
	//	//LeaveCriticalSection(&m_Lock);
	//	m_log.LogInfoEx(LOG_INFO , "设置本地解帧回调函数成功");
	//	return 0;
	//}
	//m_log.LogInfoEx(LOG_ERROR , "设置本地解帧回调函数失败");
	//pHandler->releaseHandler();
	//pHandler = NULL;
	////LeaveCriticalSection(&m_Lock);
	//m_log.LogInfoEx(LOG_INFO ,"setDisplayCallBack Out...");
	//return -4;
}



bool NVSHandlerMgr::GetDecodeParam(intptr_t identify, int nCmd, int &x1, int &y1, int &x2, int &y2)
{
	if(identify == 0)
	{
		return false;
	}
	IDVRHandler* pHandler = (IDVRHandler*)identify;
	if(NULL == pHandler)
	{
		return false;
	}
	return pHandler->GetDecodeParam( nCmd,x1,y1,x2,y2);
}

int  NVSHandlerMgr::XSLoadLibrary(char* sType ,string sDll)
{
	if(m_mDllFunction[sDll] != NULL)
	{
		return 1;
	}

	if(m_mDllFunction[sDll] == NULL)
	{
		//库加载
		string sFilePath = GetFilePath(sType);
		sFilePath += sDll;

		m_log.LogInfoEx(LOG_INFO , "加载当前目录动态库%s开始。", sFilePath.c_str());
		HINSTANCE hHandle = ::LoadLibraryEx(sFilePath.c_str(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
		DWORD DError = GetLastError();

		if(NULL == hHandle)
		{
			m_log.LogInfoEx(LOG_ERROR , "加载当前目录动态库%s失败，开始加载系统目录动态库%s,错误码:%d .", sFilePath.c_str(), sType,DError);
			string sSys32FilePath = GetFilePath(sType,"C:/WINDOWS/System32/");
			sSys32FilePath += sDll;
			hHandle = ::LoadLibraryEx(sSys32FilePath.c_str(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
			if(NULL == hHandle) 
			{
				DError = GetLastError();
				m_log.LogInfoEx(LOG_ERROR , "开始加载系统目录动态库%s,错误码:%d .", sSys32FilePath.c_str(),DError);
				hHandle = ::LoadLibrary(sDll.c_str());//pzm add 20131008 加载组合串*Play/Net
			
				if(NULL == hHandle)
				{
					DError = GetLastError();
					m_log.LogInfoEx(LOG_ERROR , "加载系统目录动态库%s失败,错误码:%d .", sDll.c_str() ,DError);
					return -1;
				}
				else//sDll 加载成功
				{
					m_log.LogInfoEx(LOG_INFO , "加载系统目录动态库%s成功", sDll.c_str());
				}		
			}
			else//sType 加载成功
			{
				m_log.LogInfoEx(LOG_INFO , "加载系统目录动态库%s成功", sType);
			}
		}
		else//sFilePath 加载成功
		{
			m_log.LogInfoEx(LOG_INFO , "加载当前目录动态库%s成功", sFilePath.c_str());
		}
		m_mDllFunction[sDll] = hHandle;
	}
	return 1;
}

//bool NVSHandlerMgr::Xsip_init(intptr_t identify, pNVSSipCallback pCB, void *pUser)
//{
//	if(identify == 0)
//	{
//		//m_log.LogInfoEx(LOG_ERROR , "identify = :%d .", identify);
//		//return false;
//		EnterCriticalSection(&m_Lock);
//		BOOL BRes = FALSE;
//		m_log.LogInfoEx(LOG_INFO ,"connectDVR In...");
//
//		char *sType = "SIP";
//		string sDll = sType;
//		IDVRHandler* pHandler = NULL;
//		char strLog[256]={0};
//
//		sDll += NETPLUGIN;
//		m_log.LogInfoEx(LOG_INFO , "DLL FileName is %s" , sDll.c_str());
//
//		int nRet = XSLoadLibrary(sType ,sDll);
//		if(nRet < 0)
//		{
//			LeaveCriticalSection(&m_Lock);
//			return false;
//		}
//		LeaveCriticalSection(&m_Lock);
//
//		GetDVRHandler pGetDVRHandler = (GetDVRHandler)::GetProcAddress(m_mDllFunction[sDll],"getHandler");
//		if(pGetDVRHandler == NULL)
//		{
//			m_log.LogInfoEx(LOG_ERROR , "获取导出函数句柄失败");
//			return false;
//		}
//		m_log.LogInfoEx(LOG_INFO , "获取导出函数句柄成功");
//
//		pHandler = pGetDVRHandler();
//		if(NULL == pHandler)
//		{
//			m_log.LogInfoEx(LOG_ERROR , "获取DVR句柄失败");
//			return false;
//		}
//		m_log.LogInfoEx(LOG_INFO , "获取DVR句柄成功");
//		identify = reinterpret_cast<intptr_t>(pHandler);
//	}
//	ISIPDVRHandler* pHandler = (ISIPDVRHandler*)identify;
//	if(NULL == pHandler)
//	{
//		m_log.LogInfoEx(LOG_ERROR , "pHandler = :%d .", pHandler);
//		return false;
//	}
//	return pHandler->Xsip_init( pCB, pUser);
//}
//
//bool NVSHandlerMgr::startPlayCamera(intptr_t identify, char* sCameraCode, LPREALSTREAMCALLBACK lpCallBack, void* pUser)
//{
//	if(identify == 0)
//	{
//		m_log.LogInfoEx(LOG_ERROR , "identify = :%d .", identify);
//		return false;
//	}
//	ISIPDVRHandler* pHandler = (ISIPDVRHandler*)identify;
//	if(NULL == pHandler)
//	{
//		m_log.LogInfoEx(LOG_ERROR , "pHandler = :%d .", pHandler);
//		return false;
//	}
//	return pHandler->startPlayCamera(sCameraCode,lpCallBack,pUser);
//}
//
//bool NVSHandlerMgr::stopPlayCamera(intptr_t identify)
//{
//	if(identify == 0)
//	{
//		m_log.LogInfoEx(LOG_ERROR , "identify = :%d .", identify);
//		return false;
//	}
//	ISIPDVRHandler* pHandler = (ISIPDVRHandler*)identify;
//	if(NULL == pHandler)
//	{
//		m_log.LogInfoEx(LOG_ERROR , "pHandler = :%d .", pHandler);
//		return false;
//	}
//	return pHandler->stopPlayCamera();
//}
//
//bool NVSHandlerMgr::startPlayMonitor(intptr_t identify,char* sCameraCode, char *sMonitorCode)
//{
//	if(identify == 0)
//	{
//		m_log.LogInfoEx(LOG_ERROR , "identify = :%d .", identify);
//		return false;
//	}
//	ISIPDVRHandler* pHandler = (ISIPDVRHandler*)identify;
//	if(NULL == pHandler)
//	{
//		m_log.LogInfoEx(LOG_ERROR , "pHandler = :%d .", pHandler);
//		return false;
//	}
//	return pHandler->startPlayMonitor(sCameraCode, sMonitorCode);
//}
//
//bool NVSHandlerMgr::stopPlayMonitor(intptr_t identify)
//{
//	if(identify == 0)
//	{
//		m_log.LogInfoEx(LOG_ERROR , "identify = :%d .", identify);
//		return false;
//	}
//	ISIPDVRHandler* pHandler = (ISIPDVRHandler*)identify;
//	if(NULL == pHandler)
//	{
//		m_log.LogInfoEx(LOG_ERROR , "pHandler = :%d .", pHandler);
//		return false;
//	}
//	return pHandler->stopPlayMonitor();
//}
//
//
//bool NVSHandlerMgr::getRecordFileBySource(intptr_t identify,int nType, char* startTime, char* endTime, int nRecordSource)
//{
//	if(identify == 0)
//	{
//		m_log.LogInfoEx(LOG_ERROR , "identify = :%d .", identify);
//		return false;
//	}
//	ISIPDVRHandler* pHandler = (ISIPDVRHandler*)identify;
//	if(NULL == pHandler)
//	{
//		m_log.LogInfoEx(LOG_ERROR , "pHandler = :%d .", pHandler);
//		return false;
//	}
//	return pHandler->getRecordFileBySource( nType,  startTime,  endTime, nRecordSource);
//}
//
//LONG NVSHandlerMgr::downloadRecordFileBySource(intptr_t identify,char* fileName,char* startTime, char* endTime,int nFileSize,char* saveFile,LPDOWNLOADRECORDFILECALLBACK lpCallBack,void* pUser, int nRecordSource)
//{
//	if(identify == 0)
//	{
//		m_log.LogInfoEx(LOG_ERROR , "identify = :%d .", identify);
//		return false;
//	}
//	ISIPDVRHandler* pHandler = (ISIPDVRHandler*)identify;
//	if(NULL == pHandler)
//	{
//		m_log.LogInfoEx(LOG_ERROR , "pHandler = :%d .", pHandler);
//		return false;
//	}
//	return pHandler->downloadRecordFileBySource( fileName, startTime,  endTime, nFileSize, saveFile,lpCallBack, pUser, nRecordSource);
//}
//
//LONG NVSHandlerMgr::replayRecordFileWithCB(intptr_t identify,char* pFileName,char* pStartTime, char* pEndTime,int nFileSize,LPREPLAYCALLBACK lpCallBack,void* pUser)
//{
//	m_log.LogInfoEx(LOG_INFO , "replayRecordFileWithCB In...");
//	ISIPDVRHandler* pHandler = (ISIPDVRHandler*)identify;
//
//	if(NULL == pHandler)
//	{
//		m_log.LogInfoEx(LOG_ERROR , "传递的句柄为空");
//		return false;
//	}
//	LONG lReturn = pHandler->replayRecordFileWithCB(pFileName,pStartTime, pEndTime, nFileSize, lpCallBack, pUser);
//	m_log.LogInfoEx(LOG_WARNING , "replayRecordFileWithCB返回值：%d", lReturn);
//	m_log.LogInfoEx(LOG_INFO , "replayRecordFileWithCB Out...");
//	return lReturn;
//}
//
//
//bool NVSHandlerMgr::presetPTZWithName(intptr_t identify,int nCmd, int nParam, int nGroup, char* pPosName)
//{
//	m_log.LogInfoEx(LOG_INFO , "presetPTZWithName In...");
//	ISIPDVRHandler* pHandler = (ISIPDVRHandler*)identify;
//
//	if(NULL == pHandler)
//	{
//		m_log.LogInfoEx(LOG_ERROR , "传递的句柄为空");
//		return false;
//	}
//	bool lReturn = pHandler->presetPTZWithName(nCmd,nParam, nGroup, pPosName);
//	m_log.LogInfoEx(LOG_WARNING , "presetPTZWithName返回值：%d", lReturn);
//	m_log.LogInfoEx(LOG_INFO , "presetPTZWithName Out...");
//	return lReturn;
//}
//
//
//bool NVSHandlerMgr::StartMatrixPatrol(intptr_t identify,char * pMatrixPlan, char * pCameraGroup, int nPatrolTime)
//{
//	m_log.LogInfoEx(LOG_INFO , "StartMatrixPatrol In...");
//	ISIPDVRHandler* pHandler = (ISIPDVRHandler*)identify;
//
//	if(NULL == pHandler)
//	{
//		m_log.LogInfoEx(LOG_ERROR , "传递的句柄为空");
//		return false;
//	}
//	bool lReturn = pHandler->StartMatrixPatrol(pMatrixPlan,pCameraGroup, nPatrolTime);
//	m_log.LogInfoEx(LOG_WARNING , "StartMatrixPatrol返回值：%d", lReturn);
//	m_log.LogInfoEx(LOG_INFO , "StartMatrixPatrol Out...");
//	return lReturn;
//}
//
//bool NVSHandlerMgr::StopMatrixPatrol(intptr_t identify,char * pMatrixPlan)
//{
//	m_log.LogInfoEx(LOG_INFO , "StopMatrixPatrol In...");
//	ISIPDVRHandler* pHandler = (ISIPDVRHandler*)identify;
//
//	if(NULL == pHandler)
//	{
//		m_log.LogInfoEx(LOG_ERROR , "传递的句柄为空");
//		return false;
//	}
//	bool lReturn = pHandler->StopMatrixPatrol(pMatrixPlan);
//	m_log.LogInfoEx(LOG_WARNING , "StopMatrixPatrol返回值：%d", lReturn);
//	m_log.LogInfoEx(LOG_INFO , "StopMatrixPatrol Out...");
//	return lReturn;
//}
