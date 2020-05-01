#pragma once
#ifdef _FOR_RTS_USE
#define _PS_ANALYZE_USE
#endif
#ifndef _PS_ANALYZE_USE
#include "idvrhandler_x64.h"
#else
#include "idvrhandler.h"
#define _HEVC2AVC_USE
#include "PSStreamAnalyze.h"
#endif


#ifdef XSIGHT_64
typedef intptr_t LLONG;
typedef HWND	 LHWND;
typedef void*	 LDWORD;
typedef void*	 LVOID;
#else
typedef LONG	LLONG;
typedef int		LHWND;
typedef long	LVOID;
typedef DWORD	LDWORD;
#endif // XSIGHT_64



class SIPHandler :
	public IDVRHandler
{
public:
	SIPHandler(void);
	~SIPHandler(void);
public:
	virtual bool connectDVR(char* sIP, int nPort, char* sUser, char* sPwd, int nChannel);
	virtual bool connectDVREx(PCONNECT_PARAM pCParam) { return false; }
	virtual void disconnectDVR();
	virtual void freeLoginInfo() { return; }
	virtual LONG checkDVR(char* sIP, int nPort, char* sUser, char* sPwd, CLInfo pInfo[], int nMaxCount) { return -1; }
	virtual LONG checkDVREx(PCONNECT_PARAM pCParam, CLInfo pInfo[], int nMaxCount) { return -1; }
	virtual void setDVR(char* sIP, int nPort, char* sUser, char* sPwd, CLInfo pInfo[], int nInfoCount) { return; }
	virtual void setDVREx(PCONNECT_PARAM pCParam, CLInfo pInfo[], int nInfoCount) { return; };
	virtual void releaseHandler() { delete this; }
	virtual bool captureStream(LPCAPTURESTREAMCALLBACK lpCallBack, void* pUser, int nGetVideoType);
	virtual bool captureStreamEx(PCAPTURE_PARAM pParam) { return false; }
	virtual bool stopCaptureStream();
	virtual int getRecordFileEx(int nType, char* startTime, char* endTime, RECORDFILE pRecordFile[], int nMaxFileCount);
	virtual RECORDFILE* getRecordFile(int nType, char* startTime, char* endTime, int* nFileCount);
	virtual LLONG downloadRecordFile(char* fileName, char* startTime, char* endTime, int nFileSize, char* saveFile, LPDOWNLOADRECORDFILECALLBACK lpCallBack, void* pUser);
	virtual LLONG downloadRecordFileEx(RECORDFILE RecordFile, char* saveFile, LPDOWNLOADRECORDFILECALLBACK lpCallBack, void* pUser) { return -1; }
	virtual bool stopDownloadRecordFile(LLONG ldownFileHandle);
	virtual LLONG replayRecordFile(LHWND hWnd, char* fileName, char* startTime, char* endTime, int nFileSize);
	virtual LLONG replayRecordFileEx(LHWND hWnd, RECORDFILE RecordFile) { return -1; }
	virtual bool controlReplayRecordFile(LLONG lReplayHandle, int nCmd, int nInValue, int* outValue);
	virtual bool stopReplayRecordFile(LLONG lReplayHandle);
	virtual bool startTalk() { return false; }
	virtual bool stopTalk() { return false; }
	virtual bool controlPTZ(int nCmd, bool bEnable) { return false; }
	virtual bool presetPTZ(int nCmd, int nIndex);
	virtual bool showSettingPane() { return false; }
	virtual bool alarm_startCapture(int nPort, LPALARMCALLBACK lpCallBack) { return false; }
	virtual bool alarm_stop() { return false; }
	virtual int log_QueryEx(LOGQUERYTYPE nQueryType, LOGERRORTYPE nErrorType, char* startTime, char* endTime, LOGINFO pLogInfo[], int nMaxLogCount) { return -1; }
	virtual LOGINFO* log_Query(LOGQUERYTYPE nQueryType, LOGERRORTYPE nErrorType, char* startTime, char* endTime, int* nLogCount) { return NULL; }
	virtual bool setVideoEffect(DWORD dwBrightValue, DWORD dwContrastValue, DWORD dwSaturationValue, DWORD dwHueValue) { return false; }
	virtual bool getVideoEffect(DWORD *pBrightValue, DWORD *pContrastValue, DWORD *pSaturationValue, DWORD *pHueValue) { return false; }
	virtual bool setDVRConfig(DWORD dwCommand, LONG lChannel, LPVOID lpInBuffer, DWORD dwInBufferSize);
	virtual bool getDVRConfig(DWORD dwCommand, LONG lChannel, LPVOID lpOutBuffer, DWORD dwOutBufferSize, LPDWORD lpBytesReturned);
	virtual bool playBackSaveData(LLONG lReplayHandle, char *sFileName);
	virtual bool stopPlayBackSave(LLONG lReplayHandle);
	virtual int getDownloadPos(LLONG lFileHandle);
	virtual bool getDVRWorkState(char dvrWorkState[], int arrayLen) { return false; }
	virtual bool getDVRIPByResolveSvr(char *sServerIP, int wServerPort, char *sDVRName, int wDVRNameLen, char *sDVRSerialNumber, int wDVRSerialLen, char* sGetIP) { return false; }
	virtual bool setConnectTime(DWORD dwWaitTime, DWORD dwTryTimes) { return false; }
	virtual bool startListen(char *sLocalIP, int wLocalPort) { return false; }
	virtual bool stopListen() { return false; }
	virtual bool serialSend(char* pBuf, DWORD nLength, char* pBuf2, DWORD nLen2) { return false; }
	virtual bool serialSendWithCallBack(char* pBuf, DWORD nLength, int nCameraID) { return false; }
	virtual bool buildSerialSend(LONG lSerialPort, LPSERIALDATACALLBACK pSerialCallBack, bool bOpen) { return false; }
	virtual void makeKey();
	virtual bool clientAudioStart() { return false; }
	virtual bool clientAudioStop() { return false; }
	virtual bool addDVR() { return false; }
	virtual bool delDVR() { return false; }
	virtual bool setVoiceComClientVolume(WORD wVolume) { return false; }
	virtual bool controlPTZSpeed(int nCmd, bool bEnable, int nSpeed);
	virtual int getDecoderAddress(LONG lChannelIndex) { return -1; }
	virtual bool controlPTZSpeedWithChannel(int nCmd, bool bEnable, int nSpeed, int nChannel);
	virtual bool presetPTZWithChannel(int nCmd, int nIndex, int nChannel);
	virtual bool controlPTZSpeedWithChannelEx(PCONTROL_PARAM pParam);
	virtual bool getPlayBackOsdTime(LLONG  lReplayHandle, char *pOsdTime) { return false; }
	virtual void setCurrentChannel(int nChannel) { return; }
	virtual bool playBackCaptureFile(LLONG lReplayHandle, char *pFileName);
	virtual bool setCapturePictureMode(DWORD dwCaptureMode) { return false; }
	virtual LLONG captureFileStream(LPCAPTURESTREAMCALLBACK lpCallBack, void* pUser, int nStreamType, RECORDFILE FileInfo, char* sType);
	virtual LONG getCameraInfo(int *nSize) { return -1; }
	virtual int setPlayWindow(HWND palyWindow[], int nMaxCount) { return -1; }
	virtual bool startPlayerByCamera(HWND hwndPlay, int nCameraID);
	virtual bool stopPlayerByCamera(HWND hwndPlay, int nCameraID);
	virtual void setCurrentCameraCode(char* sCameraCode);
	virtual bool startRecord(char* saveFile);
	virtual bool stopRecord();

	virtual bool initialPlayer(int nPort, int nDecodeType, BYTE* buff, int nSize, int nStreamType);
	virtual bool initialPlayerEx(PINITPY_PARAM pIParam) { return false; }
	virtual bool startPlayer(HWND hWnd);
	virtual bool startPlayer2(HWND hWnd, LPDRAWCBCALLLBACK lpDrawCBFun, void *pUser);
	virtual bool startPlayer4Standard(HWND hWnd, LPDecCBFun lpDecCBFun, int nCameraID);
	virtual bool startPlayer4StandardEx(PSTARTPYS_PARAM pParam) { return false; }
	virtual bool addData(BYTE* buff, int nSize);
	virtual bool stopPlayer();
	virtual bool locPlay_OpenFile(int nPort, int nDecodeType, LHWND hWnd, char* fileName) { return false; }
	virtual bool locPlay_OpenFileEx(PLOCPY_OPENF_PARAM_PARAM pParam) { return false; }
	virtual bool locPlay_Control(int nCmd, int nInValue, int* outValue) { return false; }
	virtual bool locPlay_Stop() { return false; }
	virtual bool convertToBmpFile(char * pBuf, LONG nSize, LONG nWidth, LONG nHeight, LONG nType, char *sFileName) { return false; }
	virtual bool oneByOneBack(LONG nPort) { return false; }
	virtual bool oneByOne(LONG nPort) { return false; }
	virtual bool setPlayedTimeEx(LONG nPort, DWORD nTime) { return false; }
	virtual bool setFileRefCallBack(LONG nPort, void(__stdcall *pFileRefDone) (DWORD nPort, LDWORD nUser), LDWORD nUser) { return false; }
	virtual bool setDisplayCallBack(LONG nPort, void(__stdcall *pFileRefDone)(long nPort, char * pBuf, long nSize, long nWidth, long nHeight, long nStamp, long nType, LVOID nReserved)) { return false; }
	virtual bool capturePicture(char* sFilePath);
	virtual void reSetDDrawDevice(HWND hWnd) { return; }
	virtual bool controlCameraSound(bool bOpenSound) { return false; }
	virtual LLONG openVideoFile(char *sFileName) { return -1; }
	virtual int readVideoFile(char *pBuff, int nLen, int *pOutStreamType) { return -1; }
	virtual bool closeVideoFile() { return false; }
	virtual bool controlPlay(int nCmd, int param, int *outValue) { return false; }
	virtual void ClearPlayBuffer() { return; }
	virtual void pusePlayDecode(bool bPuse) { return; }
	virtual bool SetCropArea(bool bFlag, int x1, int y1, int x2, int y2) { return false; }
	virtual bool SetDecodeParam(int nCmd, bool bFlag, int x1, int y1, int x2, int y2) { return false; }
	virtual bool SetYUVCallBack(LPYUVCALLBACK pCallBack) { return false; }
	virtual bool SetStreamSaveTime(int nSaveTime) { return false; }
	virtual bool PlayBackSaveStream(HWND hWnd) { return false; }
	virtual bool StopPlayBackSaveStream() { return false; }
	virtual bool SetRenderMode(int nMode) { return false; }
	virtual bool SetPosByFrameNum(DWORD nFrameNum) { return false; }
	virtual bool GetVideoCut(DWORD beginFrame, DWORD endFrame, LPSTR filename) { return false; }
	virtual bool StopCutVideo() { return false; }
	virtual bool SetFileEndCallback(void *pUser, pFileOverCB pCallBack) { return false; }
	virtual bool GetMediaInfo(MediaInf &mInf) { return false; }
	virtual bool GetDecodeParam(int nCmd, int &x1, int &y1, int &x2, int &y2) { return false; }
	virtual bool queryPorperySupport(XYPROPROPERTY propery) { return true; }
	virtual DWORD getLastError() { return -1; }
public:
	static void CALLBACK stream_cb(int bufType, unsigned char *buffer, unsigned int bufsize, void *user);
	void reconnect();
	bool sendKeyFrame(unsigned char * buffer, unsigned int bufsize);
public:
	int m_nWindlt;
	int m_nWindrb;
	int m_nSrcWidth;
	int m_nSrcHight;
	int m_nXl;
	int m_nYt;
	int m_nXr;
	int m_nYb;
	int m_nChannel;
	char m_szTerminalID[21];
	char m_szDeviceID[21];
	char m_szConnectKey[128];
	unsigned char *m_szFrameData;
	unsigned int m_nTimestamp;
	int	m_nFrameSize;
	unsigned int m_nStreamType;
	unsigned char			m_cPTZCmd;
	int						m_nInviteId;
	bool					m_bCloseSream;
	bool					m_bFirstPacket;
	bool					m_bMakeKeyFrame;
	FILE					*m_fRecord;
	LPCAPTURESTREAMCALLBACK m_pStramCallback;
	void					*m_pStreamUser;
	void					*m_pAnalyzeThrd;
#ifdef _PLAY_PLUGIN
	LPDecCBFun				m_pDecCallback;
	int						m_nDecUser;
	LPDRAWCBCALLLBACK		m_pDrawCallback;
	void					*m_pDrawUser;
	HWND					m_hWnd;
	HMODULE					m_hDecModule;
	IDVRHandler				*m_pDecHandle;
	typedef IDVRHandler * (*getHandlerFun)();
	getHandlerFun			m_getHandler;
#endif
	
};
