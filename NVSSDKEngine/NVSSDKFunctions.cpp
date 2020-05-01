#include "stdafx.h"
#pragma warning(disable : 4273)
extern "C" intptr_t connectDVR(char* sIP, int nPort, char* sUser, char* sPwd, int nChannel,char* sType , BOOL &BRes)
{
    return gNVSHandlerMgr.connectDVR(sIP,nPort,sUser,sPwd,nChannel,sType , BRes);
}

extern "C" intptr_t connectDVREx(PCONNECT_PARAM pCParam, BOOL &BRes)
{
    return gNVSHandlerMgr.connectDVREx(pCParam ,BRes);
}

extern "C" intptr_t getIdentify(char* sType)
{
    return gNVSHandlerMgr.getIdentify(sType);
}

extern "C" void disconnectDVR(intptr_t identify)
{
    gNVSHandlerMgr.disconnect(identify);
}

extern "C" void freeLoginInfo(intptr_t identify)
{
    gNVSHandlerMgr.freeLoginInfo(identify);
}
extern "C" LONG checkDVR(char* sIP, int nPort, char* sUser, char* sPwd,char* sType,CLInfo pInfo[],int nMaxCount)
{
    return gNVSHandlerMgr.checkDVR(sIP,nPort,sUser,sPwd,sType, pInfo, nMaxCount);
}

extern "C" LONG checkDVREx(PCONNECT_PARAM pCParam,CLInfo pInfo[],int nMaxCount)
{
    return gNVSHandlerMgr.checkDVREx(pCParam , pInfo , nMaxCount);
}

extern "C" void setDVR(char* sIP, int nPort, char* sUser, char* sPwd,char* sType,CLInfo pInfo[],int nInfoCount)
{
    gNVSHandlerMgr.setDVR(sIP,nPort,sUser,sPwd,sType,pInfo,nInfoCount);
}

extern "C" void setDVREx(PCONNECT_PARAM pCParam,CLInfo pInfo[],int nInfoCount)
{
    gNVSHandlerMgr.setDVREx(pCParam,pInfo,nInfoCount);
}

extern "C" bool captureStream(intptr_t identify, LPCAPTURESTREAMCALLBACK lpCallBack, void* pUser, int nGetVideoType)
{
    return gNVSHandlerMgr.captureStream(identify,lpCallBack, pUser,nGetVideoType);
}

extern "C" bool captureStreamEx(intptr_t identify , PCAPTURE_PARAM pParam)
{
    return gNVSHandlerMgr.captureStreamEx(identify,pParam);
}

extern "C" bool stopCaptureStream(intptr_t identify)
{
    return gNVSHandlerMgr.stopCaptureStream(identify);
}

extern "C" bool capturePicture(HWND hWnd,int nWidth,int nHeight,char* sFilePath)
{
    return gNVSHandlerMgr.capturePicture(hWnd,nWidth,nHeight,sFilePath);
}

extern "C" RECORDFILE* getRecordFile(intptr_t identify,RECORDQUERYTYPE nType,char* startTime,char* endTime,int* nFileCount)
{
    return gNVSHandlerMgr.getRecordFile(identify, nType, startTime, endTime, nFileCount);
}

extern "C" intptr_t downloadRecordFile(intptr_t identify,char* fileName,char* startTime, char* endTime,int nFileSize,char* saveFile,LPDOWNLOADRECORDFILECALLBACK lpCallBack,void* pUser)
{
    return gNVSHandlerMgr.downloadRecordFile(identify, fileName, startTime,  endTime, nFileSize, saveFile, lpCallBack, pUser);
}

extern "C" intptr_t downloadRecordFileEx(intptr_t identify ,RECORDFILE RecordFile,char* saveFile,LPDOWNLOADRECORDFILECALLBACK lpCallBack,void* pUser)
{
    return gNVSHandlerMgr.downloadRecordFileEx(identify, RecordFile, saveFile, lpCallBack, pUser);
}

extern "C" bool stopDownloadRecordFile(intptr_t identify, intptr_t ldownFileHandle)
{
    return gNVSHandlerMgr.stopDownloadRecordFile(identify,ldownFileHandle);
}

extern "C" int getRecordFileEx(intptr_t identify,RECORDQUERYTYPE nType,char* startTime,char* endTime,RECORDFILE pRecordFile[],int nMaxFileCount)
{
    return gNVSHandlerMgr.getRecordFileEx(identify,nType, startTime, endTime, pRecordFile,nMaxFileCount);
}

extern "C" intptr_t replayRecordFile(intptr_t identify, HWND hWnd,char* fileName,char* startTime, char* endTime,int nFileSize)
{
    return gNVSHandlerMgr.replayRecordFile(identify,hWnd, fileName, startTime, endTime,nFileSize);
}

extern "C" intptr_t replayRecordFileEx(intptr_t identify, HWND hWnd,RECORDFILE RecordFile)
{
    return gNVSHandlerMgr.replayRecordFileEx(identify,hWnd, RecordFile);
}

extern "C" bool controlReplayRecordFile(intptr_t identify, intptr_t lReplayHandle,int nCmd,int nInValue,int* outValue)
{
    return gNVSHandlerMgr.controlReplayRecordFile(identify, lReplayHandle,nCmd, nInValue, outValue);
}
extern "C" bool stopReplayRecordFile(intptr_t identify, intptr_t lReplayHandle)
{
    return gNVSHandlerMgr.stopReplayRecordFile(identify,lReplayHandle);
}

extern "C" bool startTalk(intptr_t identify)
{
    return gNVSHandlerMgr.startTalk(identify);
}
extern "C" bool stopTalk(intptr_t identify)
{
    return gNVSHandlerMgr.stopTalk(identify);
}

extern "C" bool controlPTZ(intptr_t identify,int nCmd, bool bEnable)
{
    return gNVSHandlerMgr.controlPTZ(identify, nCmd, bEnable);
}

extern "C" bool presetPTZ(intptr_t identify, int nCmd, int nIndex)
{
    return gNVSHandlerMgr.presetPTZ(identify, nCmd, nIndex);
}

//显示设置界面
extern "C" bool showSettingPane(intptr_t identify)
{
    return gNVSHandlerMgr.showSettingPane(identify);
}

extern "C" intptr_t initialPlayer(char* sType,int nPort,int nDecodeType,BYTE* buff, int nSize, int nStreamType)
{
    return gNVSHandlerMgr.initialPlayer(sType,nPort, nDecodeType,buff,nSize, nStreamType);
}

extern "C" intptr_t initialPlayerEx(char* sType ,PINITPY_PARAM pIParam)
{
    return gNVSHandlerMgr.initialPlayerEx(sType , pIParam);
}

extern "C" bool startPlayer(intptr_t identify,HWND hWnd)
{
    return gNVSHandlerMgr.startPlayer(identify,hWnd);
}
extern "C" bool startPlayer2(intptr_t identify,HWND hWnd,  LPDRAWCBCALLLBACK lpDrawCBFun, void *pUser)
{
    return gNVSHandlerMgr.startPlayer2(identify,  hWnd,  lpDrawCBFun, pUser);
}
//2007-04-23
extern "C" bool startPlayer4Standard(intptr_t identify,HWND hWnd,LPDecCBFun lpDecCBFun, int nCameraID)
{
    return gNVSHandlerMgr.startPlayer4Standard(identify,hWnd,lpDecCBFun, nCameraID);
}

extern "C" bool startPlayer4StandardEx(intptr_t identify, PSTARTPYS_PARAM pParam)
{
    return gNVSHandlerMgr.startPlayer4StandardEx(identify,pParam);
}

extern "C" bool addData(intptr_t identify,BYTE* buff, int nSize)
{
    return gNVSHandlerMgr.addData(identify,buff,nSize);
}

extern "C" bool stopPlayer(intptr_t identify)
{
    return gNVSHandlerMgr.stopPlayer(identify);
}

extern "C" intptr_t locPlay_OpenFile(char* sType,int nPort,int nDecodeType,int hWnd,char* fileName)
{
    return gNVSHandlerMgr.locPlay_OpenFile(sType,nPort,nDecodeType,(HWND)hWnd,fileName);
}

extern "C" intptr_t locPlay_OpenFileEx(char* sType , PLOCPY_OPENF_PARAM_PARAM pParam)
{
    return gNVSHandlerMgr.locPlay_OpenFileEx(sType,pParam);
}

extern "C" bool locPlay_Control(intptr_t identify,int nCmd,int nInValue,int* outValue)
{
    return gNVSHandlerMgr.locPlay_Control(identify, nCmd, nInValue, outValue);
}

extern "C" bool locPlay_Stop(intptr_t identify)
{
    return gNVSHandlerMgr.locPlay_Stop(identify);
}

extern "C" intptr_t alarm_startCapture(char* sType,int nPort,LPALARMCALLBACK lpCallBack)
{
    return gNVSHandlerMgr.alarm_startCapture(sType,nPort,lpCallBack);
}

extern "C" bool alarm_stop(intptr_t identify)
{
    return gNVSHandlerMgr.alarm_stop(identify);
}
extern "C" int log_QueryEx(intptr_t identify,int nQueryType, LOGQUERYTYPE nErrorType,char* startTime,char* endTime,LOGINFO pLogInfo[],int nMaxLogCount)
{
    return gNVSHandlerMgr.log_QueryEx(identify, nQueryType, nErrorType, startTime, endTime, pLogInfo,nMaxLogCount);
}
extern "C" LOGINFO* log_Query(intptr_t identify,int nQueryType, LOGQUERYTYPE nErrorType,char* startTime,char* endTime,int* nLogCount)
{
    return gNVSHandlerMgr.log_Query(identify, nQueryType, nErrorType, startTime, endTime, nLogCount);
}

extern "C"
{
    // 2007-1-28 增加接口如下
    bool setVideoEffect(intptr_t identify,DWORD dwBrightValue,DWORD dwContrastValue, DWORD dwSaturationValue,DWORD dwHueValue)
    {
        return gNVSHandlerMgr.setVideoEffect(identify,dwBrightValue,dwContrastValue,dwSaturationValue,dwHueValue);
    }
    bool getVideoEffect(intptr_t identify,DWORD *pBrightValue,DWORD *pContrastValue, DWORD *pSaturationValue,DWORD *pHueValue)
    {
        return gNVSHandlerMgr.getVideoEffect(identify,pBrightValue,pContrastValue,pSaturationValue,pHueValue);
    }
    bool setDVRConfig(intptr_t identify, DWORD dwCommand,LONG lChannel,LPVOID lpInBuffer,DWORD dwInBufferSize)
    {
        return gNVSHandlerMgr.setDVRConfig(identify,dwCommand,lChannel,lpInBuffer,dwInBufferSize);
    }
    bool getDVRConfig(intptr_t identify, DWORD dwCommand,LONG lChannel,LPVOID lpOutBuffer,DWORD dwOutBufferSize,LPDWORD lpBytesReturned)
    {
        return gNVSHandlerMgr.getDVRConfig(identify,dwCommand,lChannel,lpOutBuffer,dwOutBufferSize,lpBytesReturned);
    }
    int  convertToBmpFile(char * sType,char * pBuf,LONG nSize,LONG nWidth,LONG nHeight,LONG nType,char *sFileName)
    {
        return gNVSHandlerMgr.convertToBmpFile(sType,pBuf,nSize,nWidth,nHeight,nType,sFileName);
    }
    int  oneByOneBack(char * sType,LONG nPort)
    {
        return gNVSHandlerMgr.oneByOneBack(sType,nPort);
    }
    int  oneByOne(char * sType,LONG nPort)
    {
        return gNVSHandlerMgr.oneByOne(sType,nPort);
    }
    int  setPlayedTimeEx(char* sType,LONG nPort,DWORD nTime)
    {
        return gNVSHandlerMgr.setPlayedTimeEx(sType,nPort,nTime);
    }
    int  setFileRefCallBack(char* sType,LONG nPort,void (__stdcall *pFileRefDone) (DWORD nPort,void *nUser),void *nUser)
    {
        return gNVSHandlerMgr.setFileRefCallBack(sType,nPort,pFileRefDone,nUser);
    }
    int  setDisplayCallBack(char* sType,LONG nPort, void (__stdcall *pFileRefDone)(long nPort,char * pBuf,long nSize,long nWidth,long nHeight,long nStamp,long nType,void *nReserved))
    {
        return gNVSHandlerMgr.setDisplayCallBack(sType,nPort,pFileRefDone);
    }
    bool playBackSaveData(intptr_t identify, intptr_t lReplayHandle,char *sFileName)
    {
        return gNVSHandlerMgr.playBackSaveData(identify,lReplayHandle,sFileName);
    }
    bool stopPlayBackSave(intptr_t identify, intptr_t lReplayHandle)
    {
        return gNVSHandlerMgr.stopPlayBackSave(identify,lReplayHandle);
    }
    int getDownloadPos(intptr_t identify, intptr_t lFileHandle)
    {
        return gNVSHandlerMgr.getDownloadPos(identify,lFileHandle);
    }
    int getDownloadPosEx(intptr_t, intptr_t, int *, int *, int *, int *, int *)
    {
        return -1;
    }
    bool getDVRWorkState(intptr_t identify,char dvrWorkState[],int arrayLen)
    {
        return gNVSHandlerMgr.getDVRWorkState(identify,dvrWorkState,arrayLen);
    }
    bool getDVRIPByResolveSvr(intptr_t identify,char *sServerIP,int wServerPort,char *sDVRName,int wDVRNameLen,char *sDVRSerialNumber,int wDVRSerialLen,char* sGetIP)
    {
        return gNVSHandlerMgr.getDVRIPByResolveSvr(identify,sServerIP,wServerPort,sDVRName,wDVRNameLen,sDVRSerialNumber,wDVRSerialLen,sGetIP);
    }
    bool setConnectTime(intptr_t identify,DWORD dwWaitTime,DWORD dwTryTimes)
    {
        return gNVSHandlerMgr.setConnectTime(identify,dwWaitTime,dwTryTimes);
    }
    bool startListen (intptr_t identify,char *sLocalIP,int wLocalPort)
    {
        return gNVSHandlerMgr.startListen(identify,sLocalIP,wLocalPort);
    }
    bool stopListen(intptr_t identify)
    {
        return gNVSHandlerMgr.stopListen(identify);
    }
    bool serialSend(intptr_t identify,char* pBuf,DWORD nLength,char* pBuf2,DWORD nLen2)
    {
        return gNVSHandlerMgr.serialSend(identify,pBuf,nLength,pBuf2,nLen2);
    }
    bool serialSendWithCallBack(intptr_t identify,char* pBuf,DWORD nLength,int nCameraID)
    {
        return gNVSHandlerMgr.serialSendWithCallBack(identify,pBuf,nLength,nCameraID);
    }
    bool buildSerialSend(intptr_t identify,LONG lSerialPort,LPSERIALDATACALLBACK pSerialCallBack,bool bOpen)
    {
        return gNVSHandlerMgr.buildSerialSend(identify,lSerialPort,pSerialCallBack,bOpen);
    }
    bool clientAudioStart(intptr_t identify)
    {
        return gNVSHandlerMgr.clientAudioStart(identify);
    }
    bool clientAudioStop(intptr_t identify)
    {
        return gNVSHandlerMgr.clientAudioStop(identify);
    }
    bool addDVR(intptr_t identify)
    {
        return gNVSHandlerMgr.addDVR(identify);
    }
    bool delDVR(intptr_t identify)
    {
        return gNVSHandlerMgr.delDVR(identify);
    }
    bool setVoiceComClientVolume(intptr_t identify,WORD wVolume)
    {
        return gNVSHandlerMgr.setVoiceComClientVolume(identify,wVolume);
    }
    bool controlPTZSpeed(intptr_t identify,int nCmd, bool bEnable,int nSpeed)
    {
        return gNVSHandlerMgr.controlPTZSpeed(identify,nCmd,bEnable,nSpeed);
    }

    //2007-05-23 增加
    int getDecoderAddress(intptr_t identify, LONG lChannelIndex)
    {
        return gNVSHandlerMgr.getDecoderAddress(identify,lChannelIndex);
    }

    bool capturePictureEx(intptr_t identify, char* sFilePath)
    {
        return gNVSHandlerMgr.capturePictureEx(identify,sFilePath);
    }
    void makeKey(intptr_t identify)
    {
        return gNVSHandlerMgr.makeKey(identify);
    }

    void reSetDDrawDevice(intptr_t identify, HWND hWnd)
    {
        gNVSHandlerMgr.reSetDDrawDevice(identify, hWnd);
    }
    //2008-03-31 增加
    bool controlCameraSound(intptr_t identify, bool bOpenSound)
    {
        return gNVSHandlerMgr.controlCameraSound(identify, bOpenSound);
    }

    bool controlPTZSpeedWithChannel(intptr_t identify,int nCmd, bool bEnable, int nSpeed, int nChannel)
    {
        return gNVSHandlerMgr.controlPTZSpeedWithChannel(identify, nCmd,  bEnable,  nSpeed,  nChannel);
    }

    bool controlPTZSpeedWithChannelEx(intptr_t identify, PCONTROL_PARAM pParam)
    {
        return gNVSHandlerMgr.controlPTZSpeedWithChannelEx(identify, pParam);
    }

    bool presetPTZWithChannel(intptr_t identify, int nCmd, int nIndex, int nChannel)
    {
        return gNVSHandlerMgr.presetPTZWithChannel( identify,  nCmd,  nIndex,  nChannel);
    }

    bool getPlayBackOsdTime(intptr_t identify , intptr_t  lReplayHandle, char *pOsdTime)
    {
        return gNVSHandlerMgr.getPlayBackOsdTime(identify, lReplayHandle, pOsdTime);
    }

    void setCurrentChannel(intptr_t identify ,int nChannel)
    {
        gNVSHandlerMgr.setCurrentChannel(identify,nChannel);
    }

    DWORD getLastError(intptr_t identify)
    {
        return gNVSHandlerMgr.getLastError(identify);
    }

    bool playBackCaptureFile(intptr_t identify , intptr_t  lReplayHandle, char *pFileName)
    {
        return gNVSHandlerMgr.playBackCaptureFile(identify,lReplayHandle,pFileName);
    }

    bool setCapturePictureMode(intptr_t identify ,DWORD dwCaptureMode)
    {
        return gNVSHandlerMgr.setCapturePictureMode(identify,dwCaptureMode);
    }

	intptr_t getCameraInfo(intptr_t identify ,int *nSize)
    {
        return gNVSHandlerMgr.getCameraInfo(identify ,nSize);
    }

    int setPlayWindow(intptr_t identify ,HWND palyWindow[], int nMaxCount)
    {
        return gNVSHandlerMgr.setPlayWindow(identify ,palyWindow, nMaxCount);
    }

    bool  startPlayerByCamera(intptr_t identify, HWND hwndPlay, int nCameraID)
    {
        return gNVSHandlerMgr.startPlayerByCamera(identify, hwndPlay, nCameraID);
    }

    bool  stopPlayerByCamera(intptr_t identify, HWND hwndPlay, int nCameraID)
    {
        return gNVSHandlerMgr.stopPlayerByCamera(identify, hwndPlay, nCameraID);
    }
    bool queryPorperySupport(intptr_t identify, XYPROPROPERTY propery)
    {
        return gNVSHandlerMgr.queryPorperySupport(identify, propery);
    }
    void setCurrentCameraCode(intptr_t identify ,char* sCameraCode)
    {
        gNVSHandlerMgr.setCurrentCameraCode(identify,sCameraCode);
    }
	intptr_t openVideoFile(intptr_t identify, char *sFileName)
    {
        return 	gNVSHandlerMgr.openVideoFile(identify, sFileName);
    }
    int readVideoFile(intptr_t identify, char *pBuff,int nLen, int *pOutStreamType)
    {
        return 	gNVSHandlerMgr.readVideoFile(identify, pBuff, nLen, pOutStreamType);
    }
    bool closeVideoFile(intptr_t identify)
    {
        return 	gNVSHandlerMgr.closeVideoFile(identify);
    }
    bool controlPlay(intptr_t identify, int nCmd, int param, int *outValue)
    {
        return 	gNVSHandlerMgr.controlPlay(identify, nCmd, param, outValue);
    }

	intptr_t captureFileStream(intptr_t identify,LPCAPTURESTREAMCALLBACK lpCallBack, void* pUser, int nStreamType, RECORDFILE FileInfo, char* sType)
    {
        return 	gNVSHandlerMgr.captureFileStream(identify,  lpCallBack, pUser, nStreamType, FileInfo, sType);
    }
    void ClearPlayBuffer(intptr_t identify)
    {
        return 	gNVSHandlerMgr.ClearPlayBuffer(identify);
    }
    void pusePlayDecode(intptr_t identify,bool bPuse)
    {
        return gNVSHandlerMgr.pusePlayDecode(identify,bPuse);
    }
    bool startRecord(intptr_t identify, char* saveFile)
    {
        return gNVSHandlerMgr.startRecord(identify,  saveFile);
    }
    bool stopRecord(intptr_t identify)
    {
        return gNVSHandlerMgr.stopRecord(identify);
    }
	bool SetCropArea(intptr_t identify, bool bFlag, int x1, int y1, int x2, int y2)
	{
		return gNVSHandlerMgr.SetCropArea(identify, bFlag, x1, y1, x2, y2);
	}
	intptr_t GetIdentify(char * sCameraID, bool bFlag)
	{
		return gNVSHandlerMgr.getIdentify(sCameraID, bFlag);
	}
	bool SetDecodeParam(intptr_t identify, int nCmd, bool bFlag, int x1, int y1, int x2, int y2)
	{
		return gNVSHandlerMgr.SetDecodeParam(identify, nCmd, bFlag, x1, y1, x2, y2);
	}
	bool SetYUVCallBack(intptr_t identify, LPYUVCALLBACK pCallBack)
	{
		return gNVSHandlerMgr.SetYUVCallBack(identify, pCallBack);
	}
	bool SetStreamSaveTime(intptr_t identify, int nSaveTime)
	{
		return gNVSHandlerMgr.SetStreamSaveTime(identify, nSaveTime);
	}
	bool PlayBackSaveStream(intptr_t identify, HWND hWnd)
	{
		return gNVSHandlerMgr.PlayBackSaveStream(identify, hWnd);
	}
	bool StopPlayBackSaveStream(intptr_t identify)
	{
		return gNVSHandlerMgr.StopPlayBackSaveStream(identify);
	}
	bool SetRenderMode(intptr_t identify, int nMode)
	{
		return gNVSHandlerMgr.SetRenderMode(identify, nMode);
	}

	 bool SetPosByFrameNum(intptr_t identify,DWORD nFrameNum)
	{
		 return gNVSHandlerMgr.SetPosByFrameNum(identify, nFrameNum);
	}

	bool GetVideoCut(intptr_t identify, DWORD beginFrame, DWORD endFrame, LPSTR filename)
	{
		return gNVSHandlerMgr.GetVideoCut(identify, beginFrame, endFrame, filename);
	}

	bool StopCutVideo(intptr_t identify)
	{
		return gNVSHandlerMgr.StopCutVideo(identify);
	}

	bool SetFileEndCallback(intptr_t identify, void *pUser,pFileOverCB pCallBack)
	{
		 return gNVSHandlerMgr.SetFileEndCallback(identify,pUser, pCallBack);
	}

	bool GetMediaInfo(intptr_t identify,MediaInf &mInf)
	{
		return gNVSHandlerMgr.GetMediaInfo(identify, mInf);
	}

	bool GetLightParam(LONG nPort,OUT int *gray1,OUT int *gray2)
	{
		return gNVSHandlerMgr.GetLightParam(nPort,gray1, gray2);
	}

	bool GetDecodeParam(intptr_t identify, int nCmd, int &x1, int &y1, int &x2, int &y2)
	{
		return gNVSHandlerMgr.GetDecodeParam(identify, nCmd, x1, y1, x2, y2);
	}

	//bool Xsip_init(intptr_t identify, pNVSSipCallback pCB, void *pUser)
	//{
	//	return gNVSHandlerMgr.Xsip_init( identify,  pCB, pUser);
	//}

	//bool startPlayCamera(intptr_t identify, char* sCameraCode, LPREALSTREAMCALLBACK lpCallBack, void* pUser)
	//{
	//	return gNVSHandlerMgr.startPlayCamera(identify,sCameraCode,lpCallBack,pUser);
	//}

	//bool stopPlayCamera(intptr_t identify)
	//{
	//	return gNVSHandlerMgr.stopPlayCamera( identify);
	//}

	//bool startPlayMonitor(intptr_t identify,char* sCameraCode, char *sMonitorCode)
	//{
	//	return gNVSHandlerMgr.startPlayMonitor( identify,sCameraCode, sMonitorCode);
	//}

	//bool stopPlayMonitor(intptr_t identify)
	//{
	//	return gNVSHandlerMgr.stopPlayMonitor( identify);
	//}

	//bool getRecordFileBySource(intptr_t identify,int nType, char* startTime, char* endTime, int nRecordSource)
	//{
	//	return gNVSHandlerMgr.getRecordFileBySource( identify, nType, startTime, endTime, nRecordSource);
	//}

	//intptr_t downloadRecordFileBySource(intptr_t identify,char* fileName,char* startTime, char* endTime,int nFileSize,char* saveFile,LPDOWNLOADRECORDFILECALLBACK lpCallBack,void* pUser, int nRecordSource)
	//{
	//	return gNVSHandlerMgr.downloadRecordFileBySource(identify, fileName, startTime,  endTime, nFileSize, saveFile,lpCallBack, pUser, nRecordSource);
	//}

	//intptr_t replayRecordFileWithCB(intptr_t identify,char* pFileName,char* pStartTime, char* pEndTime,int nFileSize,LPREPLAYCALLBACK lpCallBack,void* pUser)
	//{
	//	return gNVSHandlerMgr.replayRecordFileWithCB(identify,pFileName,pStartTime, pEndTime, nFileSize, lpCallBack, pUser);
	//}

	//bool presetPTZWithName(intptr_t identify,int nCmd, int nParam, int nGroup, char* pPosName)
	//{
	//	return gNVSHandlerMgr.presetPTZWithName(identify,nCmd,nParam, nGroup, pPosName);
	//}

	//bool StartMatrixPatrol(intptr_t identify,char * pMatrixPlan, char * pCameraGroup, int nPatrolTime)
	//{
	//	return gNVSHandlerMgr.StartMatrixPatrol(identify,pMatrixPlan,pCameraGroup, nPatrolTime);
	//}

	//bool StopMatrixPatrol(intptr_t identify,char * pMatrixPlan)
	//{
	//	return gNVSHandlerMgr.StopMatrixPatrol(identify,pMatrixPlan);
	//}
};

