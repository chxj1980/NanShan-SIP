#pragma once

//#include "ISIPDVRHandler.h";
#include "IDVRHandler_x64.h"
#include "WriteLog.h"

//#include "LogRecorder.h"

typedef IDVRHandler* (*GetDVRHandler)();

#include <map>
#include <string>
using namespace std;



class NVSHandlerMgr
{
public:
    NVSHandlerMgr(void);
    ~NVSHandlerMgr(void);

protected:
    map<string,HINSTANCE> m_mDllFunction;
    CRITICAL_SECTION m_Lock;
public:
	intptr_t connectDVR(char* sIP, int nPort, char* sUser, char* sPwd, int nChannel, char* sType , BOOL &BRes);
	intptr_t connectDVREx(PCONNECT_PARAM pCParam, BOOL &BRes);
    void disconnect(intptr_t identify);
    // -由于增加接口需要设置DVR上通道的名称和检测这个DVR是否能够登陆上，加以下两个接口（郑峰）-
    //检测DVR
    LONG checkDVR(char* sIP, int nPort, char* sUser, char* sPwd,char* sType,CLInfo pInfo[],int nMaxCount);

    LONG checkDVREx(PCONNECT_PARAM pCParam,CLInfo pInfo[],int nMaxCount);
    //设置信息
    void setDVR(char* sIP, int nPort, char* sUser, char* sPwd,char* sType,CLInfo pInfo[],int nInfoCount);

    void setDVREx(PCONNECT_PARAM pCParam,CLInfo pInfo[],int nInfoCount);
    // -----------------------------------------------------------------------------------------

    bool captureStream(intptr_t identify, LPCAPTURESTREAMCALLBACK lpCallBack, void* pUser, int nGetVideoType);

    bool captureStreamEx(intptr_t identify, PCAPTURE_PARAM pParam);

    bool stopCaptureStream(intptr_t identify);

    bool capturePicture(HWND hWnd,int nWidth,int nHeight,char* sFilePath);

    int getRecordFileEx(intptr_t identify,RECORDQUERYTYPE nType,char* startTime,char* endTime,RECORDFILE pRecordFile[],int nMaxFileCount);
    RECORDFILE* getRecordFile(intptr_t identify,RECORDQUERYTYPE nType,char* startTime,char* endTime,int* nFileCount);

	intptr_t downloadRecordFile(intptr_t identify,char* fileName,char* startTime, char* endTime,int nFileSize,char* saveFile,LPDOWNLOADRECORDFILECALLBACK lpCallBack,void* pUser);

	intptr_t downloadRecordFileEx(intptr_t identify,RECORDFILE RecordFile,char* saveFile,LPDOWNLOADRECORDFILECALLBACK lpCallBack,void* pUser);

    bool stopDownloadRecordFile(intptr_t identify, intptr_t ldownFileHandle);

	intptr_t replayRecordFile(intptr_t identify,HWND hWnd,char* fileName,char* startTime, char* endTime,int nFileSize);

	intptr_t replayRecordFileEx(intptr_t identify, HWND hWnd,RECORDFILE RecordFile);

    bool controlReplayRecordFile(intptr_t identify, intptr_t lReplayHandle,int nCmd,int nInValue,int* outValue);

    bool stopReplayRecordFile(intptr_t identify, intptr_t lReplayHandle);

    bool startTalk(intptr_t identify);

    bool stopTalk(intptr_t identify);

    bool controlPTZ(intptr_t identify,int nCmd, bool bEnable);

    bool presetPTZ(intptr_t identify, int nCmd, int nIndex);

    bool showSettingPane(intptr_t identify);

    intptr_t initialPlayer(char* sType,int nPort,int nDecodeType,BYTE* buff, int nSize, int nStreamType);

	intptr_t initialPlayerEx(char* sType , PINITPY_PARAM pIParam);

    bool startPlayer(intptr_t identify,HWND hWnd);

    bool startPlayer2(intptr_t identify, HWND hWnd, LPDRAWCBCALLLBACK lpDrawCBFun, void *pUser);

    //2007-04-23
    bool startPlayer4Standard(intptr_t identify, HWND hWnd, LPDecCBFun lpDecCBFun, int nCameraID);

    bool startPlayer4StandardEx(intptr_t identify, PSTARTPYS_PARAM pParam);

    bool addData(intptr_t identify,BYTE* buff, int nSize);

    bool stopPlayer(intptr_t identify);

	intptr_t locPlay_OpenFile(char* sType,int nPort,int nDecodeType, HWND hWnd,char* fileName);

	intptr_t locPlay_OpenFileEx(char* sType , PLOCPY_OPENF_PARAM_PARAM pParam);

    bool locPlay_Control(intptr_t identify,int nCmd,int nInValue,int* outValue);

    bool locPlay_Stop(intptr_t identify);

	intptr_t alarm_startCapture(char* sType,int nPort,LPALARMCALLBACK lpCallBack);

    bool alarm_stop(intptr_t identify);

    int log_QueryEx(intptr_t identify,LOGQUERYTYPE nQueryType, LOGERRORTYPE nErrorType,char* startTime,char* endTime,LOGINFO pLogInfo[],int nMaxLogCount);

    LOGINFO* log_Query(intptr_t identify,LOGQUERYTYPE nQueryType, LOGERRORTYPE nErrorType,char* startTime,char* endTime,int* nLogCount);

    bool setVideoEffect(intptr_t identify,DWORD dwBrightValue,DWORD dwContrastValue, DWORD dwSaturationValue,DWORD dwHueValue);
    bool getVideoEffect(intptr_t identify,DWORD *pBrightValue,DWORD *pContrastValue, DWORD *pSaturationValue,DWORD *pHueValue);
    bool setDVRConfig(intptr_t identify,DWORD dwCommand,LONG lChannel,LPVOID lpInBuffer,DWORD dwInBufferSize);
    bool getDVRConfig(intptr_t identify,DWORD dwCommand,LONG lChannel,LPVOID lpOutBuffer,DWORD dwOutBufferSize,LPDWORD lpBytesReturned);
    int  convertToBmpFile(char* sType,char * pBuf,LONG nSize,LONG nWidth,LONG nHeight,LONG nType,char *sFileName);
    int  oneByOneBack(char* sType,LONG nPort);
    int  oneByOne(char* sType,LONG nPort);
    int  setPlayedTimeEx(char* sType,LONG nPort,DWORD nTime);
    int  setFileRefCallBack(char* sType,LONG nPort,void (__stdcall *pFileRefDone) (DWORD nPort,void *nUser),void *nUser);
    int  setDisplayCallBack(char* sType,LONG nPort, void (__stdcall *pFileRefDone)(long nPort,char * pBuf,long nSize,long nWidth,long nHeight,long nStamp,long nType,void *nReserved));
    bool playBackSaveData(intptr_t identify, intptr_t lReplayHandle,char *sFileName);
    bool stopPlayBackSave(intptr_t identify, intptr_t lReplayHandle);
    int getDownloadPos(intptr_t identify, intptr_t lFileHandle);
    bool getDVRWorkState(intptr_t identify,char dvrWorkState[],int arrayLen);
    bool getDVRIPByResolveSvr(intptr_t identify,char *sServerIP,int wServerPort,char *sDVRName,int wDVRNameLen,char *sDVRSerialNumber,int wDVRSerialLen,char* sGetIP);
    bool setConnectTime(intptr_t identify,DWORD dwWaitTime,DWORD dwTryTimes);
    bool startListen (intptr_t identify,char *sLocalIP,int wLocalPort);
    bool stopListen(intptr_t identify);
    bool serialSend(intptr_t identify,char* pBuf,DWORD nLength,char* pBuf2,DWORD nLen2);
    bool serialSendWithCallBack(intptr_t identify,char* pBuf,DWORD nLength,int nCameraID);
    bool buildSerialSend(intptr_t identify,LONG lSerialPort,LPSERIALDATACALLBACK pSerialCallBack,bool bOpen);
    //启动PC 端声音捕获
    bool clientAudioStart(intptr_t identify);
    //停止PC 端声音捕获
    bool clientAudioStop(intptr_t identify);
    //添加一台DVR 到可以接收PC 机声音的组里
    bool addDVR(intptr_t identify);
    //删除已经加到组的DVR
    bool delDVR(intptr_t identify);
    bool setVoiceComClientVolume(intptr_t identify,WORD wVolume);
    bool controlPTZSpeed(intptr_t identify,int nCmd, bool bEnable,int nSpeed);
    void makeKey(intptr_t identify);
    //2007-05-23 增加
    int getDecoderAddress(intptr_t identify,LONG lChannelIndex);
    //2007-05-25 增加
    bool capturePictureEx(intptr_t identify, char* sFilePath);
    //2008-03-10 增加
    void reSetDDrawDevice(intptr_t identify, HWND hWnd);
    //2008-03-31 增加
    bool controlCameraSound(intptr_t identify, bool bOpenSound);
    bool controlPTZSpeedWithChannel(intptr_t identify,int nCmd, bool bEnable, int nSpeed, int nChannel);
    bool controlPTZSpeedWithChannelEx(intptr_t identify, PCONTROL_PARAM pParam);
    bool presetPTZWithChannel(intptr_t identify, int nCmd, int nIndex, int nChannel);
    bool getPlayBackOsdTime(intptr_t identify , intptr_t  lReplayHandle, char *pOsdTime);
    void setCurrentChannel(intptr_t identify ,int nChannel);
    DWORD getLastError(intptr_t identify);
    bool playBackCaptureFile(intptr_t identify , intptr_t  lReplayHandle, char *pFileName);
    bool setCapturePictureMode(intptr_t identify ,DWORD dwCaptureMode);
	intptr_t getCameraInfo(intptr_t identify ,int *nSize);
    int setPlayWindow(intptr_t identify ,HWND palyWindow[], int nMaxCount);
    bool startPlayerByCamera(intptr_t identify, HWND hwndPlay, int nCameraID);
    bool  stopPlayerByCamera(intptr_t identify, HWND hwndPlay, int nCameraID);
    bool queryPorperySupport(intptr_t identify, XYPROPROPERTY propery);
    void setCurrentCameraCode(intptr_t identify,char* sCameraCode);
	intptr_t openVideoFile(intptr_t identify, char *sFileName);
    int readVideoFile(intptr_t identify, char *pBuff,int nLen, int *pOutStreamType);
    bool closeVideoFile(intptr_t identify);
    bool controlPlay(intptr_t identify, int nCmd,int param, int *outValue);
	intptr_t captureFileStream(intptr_t identify, LPCAPTURESTREAMCALLBACK lpCallBack, void* pUser, int nStreamType, RECORDFILE FileInfo, char* sType);
    void ClearPlayBuffer(intptr_t identify);
    void pusePlayDecode(intptr_t identify,bool bPuse);
    bool startRecord(intptr_t identify, char* saveFile);
    bool stopRecord(intptr_t identify);
public:
    void WriteSysLog(std::string sLongText);
    //int filecopy(const char* source,const char* dest);
    //bool getCurrentDirectory(char* pDirectory);
	intptr_t getIdentify(char* sType);
    void freeLoginInfo(intptr_t identify);
private:
    HANDLE m_hFileHandle ;
    char m_sSystemPath[256];
    //CLogRecorder m_cLogRecorder;
    WriteLog m_log;

    string GetFilePath(const char *sType = NULL, char *pDir = NULL);

	int   XSLoadLibrary(char* sType ,string sDll);
public:
//2013 0926 zjf add
	map<string, intptr_t> m_mapCameraIDToHandler;
	intptr_t getIdentify(char * sCameraID, bool bFlag);
	bool SetCropArea(intptr_t identify, bool bFlag, int x1, int y1, int x2, int y2);
	bool SetDecodeParam(intptr_t identify, int nCmd, bool bFlag, int x1, int y1, int x2, int y2);
	bool SetYUVCallBack(intptr_t identify, LPYUVCALLBACK pCallBack);

	bool SetStreamSaveTime(intptr_t identify, int nSaveTime);
	bool PlayBackSaveStream(intptr_t identify, HWND hWnd);
	bool StopPlayBackSaveStream(intptr_t identify);
	bool GetLightParam(LONG nPort,OUT int *gray1,OUT int *gray2);
//2013 0927 pzm add
	bool SetRenderMode(intptr_t identify, int nMode);

	bool SetPosByFrameNum(intptr_t identify,DWORD nFrameNum);

	bool GetVideoCut(intptr_t identify, DWORD beginFrame, DWORD endFrame, LPSTR filename);

	bool StopCutVideo(intptr_t identify);

	bool SetFileEndCallback(intptr_t identify, void *pUser, pFileOverCB pCallBack);

	bool GetMediaInfo(intptr_t identify,MediaInf &mInf);

	bool GetDecodeParam(intptr_t identify, int nCmd, int &x1, int &y1, int &x2, int &y2);

//V3.0 PZM
	//bool Xsip_init(intptr_t identify, pNVSSipCallback pCB, void *pUser);

	//bool startPlayCamera(intptr_t identify,char* sCameraCode, LPREALSTREAMCALLBACK lpCallBack = NULL, void* pUser = NULL);

	//bool stopPlayCamera(intptr_t identify);

	//bool startPlayMonitor(intptr_t identify,char* sCameraCode, char *sMonitorCode);

	//bool stopPlayMonitor(intptr_t identify);

	//bool getRecordFileBySource(intptr_t identify,int nType, char* startTime, char* endTime, int nRecordSource);

	//LONG downloadRecordFileBySource(intptr_t identify,char* fileName,char* startTime, char* endTime,int nFileSize,char* saveFile,LPDOWNLOADRECORDFILECALLBACK lpCallBack,void* pUser, int nRecordSource);
	//
	//LONG replayRecordFileWithCB(intptr_t identify,char* pFileName,char* pStartTime, char* pEndTime,int nFileSize,LPREPLAYCALLBACK lpCallBack,void* pUser);

	//bool presetPTZWithName(intptr_t identify,int nCmd, int nParam, int nGroup, char* pPosName);

	//bool StartMatrixPatrol(intptr_t identify,char * pMatrixPlan, char * pCameraGroup, int nPatrolTime);

	//bool StopMatrixPatrol(intptr_t identify,char * pMatrixPlan);
};
