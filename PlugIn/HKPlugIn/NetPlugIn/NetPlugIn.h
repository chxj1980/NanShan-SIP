#pragma once
#include "HCNetSDK.h"
#include "IDVRHandler_x64.h"
#ifdef _PS_ANALYZE_USE
#include "ps_stream_analysis.h"
#endif

class NetPlugIn :
	public IDVRHandler
{
public:
	NetPlugIn(void);
	~NetPlugIn(void);
public:
	virtual bool connectDVR(char* sIP,int nPort,char* sUser,char* sPwd,int nChannel);
    virtual bool connectDVREx(PCONNECT_PARAM pCParam){return false;}
	virtual void disconnectDVR();
    virtual void freeLoginInfo(){return;}
    virtual LONG checkDVR(char* sIP, int nPort, char* sUser, char* sPwd,CLInfo pInfo[],int nMaxCount){return -1;}
    virtual LONG checkDVREx(PCONNECT_PARAM pCParam,CLInfo pInfo[],int nMaxCount){return -1;}
    virtual void setDVR(char* sIP, int nPort, char* sUser, char* sPwd,CLInfo pInfo[],int nInfoCount){return;}
    virtual void setDVREx(PCONNECT_PARAM pCParam,CLInfo pInfo[],int nInfoCount){return;};
    virtual void releaseHandler(){delete this;}
    virtual bool captureStream(LPCAPTURESTREAMCALLBACK lpCallBack, void* pUser, int nGetVideoType);
    virtual bool captureStreamEx(PCAPTURE_PARAM pParam){return false;}
    virtual bool stopCaptureStream();
    virtual int getRecordFileEx(int nType,char* startTime,char* endTime,RECORDFILE pRecordFile[],int nMaxFileCount){return -1;}
    virtual RECORDFILE* getRecordFile(int nType,char* startTime,char* endTime,int* nFileCount){return NULL;}
    virtual intptr_t downloadRecordFile(char* fileName,char* startTime, char* endTime,int nFileSize,char* saveFile,LPDOWNLOADRECORDFILECALLBACK lpCallBack,void* pUser){return -1;}
    virtual intptr_t downloadRecordFileEx(RECORDFILE RecordFile,char* saveFile,LPDOWNLOADRECORDFILECALLBACK lpCallBack,void* pUser){return -1;}
    virtual bool stopDownloadRecordFile(intptr_t ldownFileHandle){return false;}
    virtual intptr_t replayRecordFile(HWND hWnd,char* fileName,char* startTime, char* endTime,int nFileSize){return -1;}
    virtual intptr_t replayRecordFileEx(HWND hWnd,RECORDFILE RecordFile){return -1;}
    virtual bool controlReplayRecordFile(intptr_t lReplayHandle,int nCmd,int nInValue,int* outValue){return false;}
    virtual bool stopReplayRecordFile(intptr_t lReplayHandle){return false;}
    virtual bool startTalk(){return false;}
    virtual bool stopTalk(){return false;}
    virtual bool controlPTZ(int nCmd, bool bEnable){return false;}
    virtual bool presetPTZ(int nCmd, int nIndex);
    virtual bool showSettingPane(){return false;}
    virtual bool alarm_startCapture(int nPort,LPALARMCALLBACK lpCallBack){return false;}
    virtual bool alarm_stop(){return false;}
    virtual int log_QueryEx(LOGQUERYTYPE nQueryType, LOGERRORTYPE nErrorType,char* startTime,char* endTime,LOGINFO pLogInfo[],int nMaxLogCount){return -1;}
    virtual LOGINFO* log_Query(LOGQUERYTYPE nQueryType, LOGERRORTYPE nErrorType,char* startTime,char* endTime,int* nLogCount){return NULL;}
    virtual bool setVideoEffect(DWORD dwBrightValue,DWORD dwContrastValue, DWORD dwSaturationValue,DWORD dwHueValue){return false;}
    virtual bool getVideoEffect(DWORD *pBrightValue,DWORD *pContrastValue, DWORD *pSaturationValue,DWORD *pHueValue){return false;}
    virtual bool setDVRConfig(DWORD dwCommand,LONG lChannel,LPVOID lpInBuffer,DWORD dwInBufferSize){return false;}
    virtual bool getDVRConfig(DWORD dwCommand,LONG lChannel,LPVOID lpOutBuffer,DWORD dwOutBufferSize,LPDWORD lpBytesReturned){return false;}
    virtual bool playBackSaveData(intptr_t lReplayHandle,char *sFileName){return false;}
    virtual bool stopPlayBackSave(intptr_t lReplayHandle){return false;}
    virtual int getDownloadPos(intptr_t lFileHandle){return -1;}
    virtual bool getDVRWorkState(char dvrWorkState[],int arrayLen){return false;}
    virtual bool getDVRIPByResolveSvr(char *sServerIP,int wServerPort,char *sDVRName,int wDVRNameLen,char *sDVRSerialNumber,int wDVRSerialLen,char* sGetIP){return false;}
    virtual bool setConnectTime(DWORD dwWaitTime,DWORD dwTryTimes){return false;}
    virtual bool startListen (char *sLocalIP,int wLocalPort){return false;}
    virtual bool stopListen(){return false;}
    virtual bool serialSend(char* pBuf,DWORD nLength,char* pBuf2,DWORD nLen2){return false;}
    virtual bool serialSendWithCallBack(char* pBuf,DWORD nLength,int nCameraID){return false;}
    virtual bool buildSerialSend(LONG lSerialPort,LPSERIALDATACALLBACK pSerialCallBack,bool bOpen){return false;}
    virtual void makeKey(){return;}
    virtual bool clientAudioStart(){return false;}
    virtual bool clientAudioStop(){return false;}
    virtual bool addDVR(){return false;}
    virtual bool delDVR(){return false;}
    virtual bool setVoiceComClientVolume(WORD wVolume){return false;}
    virtual bool controlPTZSpeed(int nCmd, bool bEnable,int nSpeed);
    virtual int getDecoderAddress(LONG lChannelIndex){return -1;}
    virtual bool controlPTZSpeedWithChannel(int nCmd, bool bEnable, int nSpeed, int nChannel);
    virtual bool presetPTZWithChannel(int nCmd, int nIndex, int nChannel);
    virtual bool controlPTZSpeedWithChannelEx(PCONTROL_PARAM pParam){return false;}
    virtual bool getPlayBackOsdTime(intptr_t  lReplayHandle, char *pOsdTime){return false;}
    virtual void setCurrentChannel(int nChannel){return;}
    virtual bool playBackCaptureFile(intptr_t lReplayHandle, char *pFileName){return false;}
    virtual bool setCapturePictureMode(DWORD dwCaptureMode){return false;}
    virtual intptr_t captureFileStream(LPCAPTURESTREAMCALLBACK lpCallBack, void* pUser, int nStreamType, RECORDFILE FileInfo, char* sType){return -1;}
    virtual LONG getCameraInfo(int *nSize){return -1;}
    virtual int setPlayWindow(HWND palyWindow[], int nMaxCount){return -1;}
    virtual bool startPlayerByCamera(HWND hwndPlay, int nCameraID){return false;}
    virtual bool  stopPlayerByCamera(HWND hwndPlay, int nCameraID){return false;}
    virtual void setCurrentCameraCode(char* sCameraCode);
    virtual bool startRecord(char* saveFile){return false;}
    virtual bool stopRecord(){return false;}

    virtual bool initialPlayer(int nPort,int nDecodeType,BYTE* buff, int nSize, int nStreamType){return false;}
    virtual bool initialPlayerEx(PINITPY_PARAM pIParam){return false;}
    virtual bool startPlayer(HWND hWnd){return false;}
    virtual bool startPlayer2(HWND hWnd, LPDRAWCBCALLLBACK lpDrawCBFun, void *pUser){return false;}
    virtual bool startPlayer4Standard(HWND hWnd,LPDecCBFun lpDecCBFun, int nCameraID){return false;}
    virtual bool startPlayer4StandardEx(PSTARTPYS_PARAM pParam){return false;}
    virtual bool addData(BYTE* buff, int nSize){return false;}
    virtual bool stopPlayer(){return false;}
    virtual bool locPlay_OpenFile(int nPort,int nDecodeType,HWND hWnd,char* fileName){return false;}
    virtual bool locPlay_OpenFileEx(PLOCPY_OPENF_PARAM_PARAM pParam){return false;}
    virtual bool locPlay_Control(int nCmd,int nInValue,int* outValue){return false;}
    virtual bool locPlay_Stop(){return false;}
    virtual bool convertToBmpFile(char * pBuf,LONG nSize,LONG nWidth,LONG nHeight,LONG nType,char *sFileName){return false;}
    virtual bool oneByOneBack(LONG nPort){return false;}
    virtual bool oneByOne(LONG nPort){return false;}
    virtual bool setPlayedTimeEx(LONG nPort,DWORD nTime){return false;}
    virtual bool setFileRefCallBack(LONG nPort,void (__stdcall *pFileRefDone) (DWORD nPort,void *nUser),void *nUser){return false;}
    virtual bool setDisplayCallBack(LONG nPort, void (__stdcall *pFileRefDone)(long nPort,char * pBuf,long nSize,long nWidth,long nHeight,long nStamp,long nType,void *nReserved)){return false;}
    virtual bool capturePicture(char* sFilePath){return false;}
    virtual void reSetDDrawDevice(HWND hWnd){return;}
    virtual bool controlCameraSound(bool bOpenSound){return false;}
    virtual intptr_t openVideoFile(char *sFileName){return -1;}
    virtual int readVideoFile(char *pBuff,int nLen, int *pOutStreamType){return -1;}
    virtual bool closeVideoFile(){return false;}
    virtual bool controlPlay(int nCmd,int param, int *outValue){return false;}
    virtual void ClearPlayBuffer(){return;}
    virtual void pusePlayDecode(bool bPuse){return;}
	virtual bool SetCropArea(bool bFlag, int x1, int y1, int x2, int y2){return false;}
	virtual bool SetDecodeParam(int nCmd, bool bFlag, int x1, int y1, int x2, int y2){return false;}
	virtual bool SetYUVCallBack(LPYUVCALLBACK pCallBack){return false;}
	virtual bool SetStreamSaveTime(int nSaveTime);
	virtual bool PlayBackSaveStream(HWND hWnd){return false;}
	virtual bool StopPlayBackSaveStream(){return false;}
	virtual bool SetRenderMode(int nMode){return false;}
	virtual bool SetPosByFrameNum(DWORD nFrameNum){return false;}
	virtual bool GetVideoCut(DWORD beginFrame, DWORD endFrame, LPSTR filename){return false;}
	virtual bool StopCutVideo(){return false;}
	virtual bool SetFileEndCallback(void *pUser, pFileOverCB pCallBack){return false;}
	virtual bool GetMediaInfo(MediaInf &mInf);
	virtual bool GetDecodeParam(int nCmd, int &x1, int &y1, int &x2, int &y2){return false;}
	virtual bool queryPorperySupport(XYPROPROPERTY propery){return true;}
    virtual DWORD getLastError(){return m_nError;}

public:
	static void __stdcall fStdDataCB(LONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void *pUser);
	static void __stdcall fPlayESCB(LONG lPreviewHandle, NET_DVR_PACKET_INFO_EX *pstruPackInfo, void* pUser);

	void rtp_format(bool mark=false);
public:
	long					m_pHKHandle;
	int						m_nError;
	long					m_nPlayId;
	int						m_nChannel;
	int						m_nProto;
	char					m_szCameraID[128];

	LPCAPTURESTREAMCALLBACK m_pStreamCb;
	void *					m_pUser;
	bool					m_bFirstData;
	FILE *					m_file;
	unsigned char			*m_pBuffer;
	long					m_nBufflen;
	int						m_nKeyFrame;
	UINT					m_vTimeStamp;
	UINT					m_aTimeStamp;
	CRITICAL_SECTION		m_slock;
public:
	
	int						m_nCount;
	bool					m_bFirstAudio;

#ifdef _PS_ANALYZE_USE
	EasyAACEncoder_Handle	m_hEncHandle;
	ps_stream_analysis		*m_hAnalysis;
#endif
	int						m_nGetVideoType;
	int						m_nVideoType;
	int						m_nFrameRate;
	BYTE					m_szPrevPack[10240];
	int						m_nPrevPackSize;
	BYTE					m_szRtpPack[1500];
	int						m_nRtp_ssrc;
	unsigned int			m_nRtp_cseq;
	unsigned int			m_nRtp_time;

	DWORD					m_dwPTZCommand;
};


