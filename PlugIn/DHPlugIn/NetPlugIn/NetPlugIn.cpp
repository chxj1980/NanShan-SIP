#include "StdAfx.h"
#include "NetPlugIn.h"
#include "NetManager.h"

NetManager gNetManager;

#define AUDIO_PCM		0
#define AUDIO_G722		1
#define AUDIO_G711A		2
#define AUDIO_G711U		3
#define AUDIO_AAC		4
#define AUDIO_AACLC		5
#define AUDIO_AACLD		6

#define DH_HEAD_LEN		40
#define DH_END_LEN		3
#define DH_FRAME_A		0xF0
#define DH_FRAME_EX		0XF1
#define DH_FRAME_P		0xFC
#define DH_FRAME_I		0xFD


NetPlugIn::NetPlugIn(void)
{
	CLIENT_Init(NULL,NULL);
	m_pDHHandle = -1;
	m_nPlayId = -1;
	m_nError = 0;
	memset(m_szCameraID,0,sizeof(m_szCameraID));
	m_pStreamCb = NULL;
	m_pUser = NULL;
	m_bFirstData = false;
	m_nCount = 0;
	m_pBuffer = new unsigned char[1024*1024];
	m_nBufSize = 0;
	m_vTimeStamp = 0;
	m_aTimeStamp = 0;
	m_nGetVideoType = 0;
	m_nVideoType = 98;
	m_nFrameLen = 0;

	m_dwCmd = 0;

#ifdef _DH_ANALYZE_USE
	m_bANA = true;
	m_pANAHandle = false;
	m_hEncHandle = NULL;
#endif // _DH_ANALYZE_USE

}

NetPlugIn::~NetPlugIn(void)
{
	if (m_pBuffer)
	{
		delete[] m_pBuffer;
		m_pBuffer = NULL;
	}
	//CLIENT_Cleanup();
}

bool NetPlugIn::connectDVR(char* sIP,int nPort,char* sUser,char* sPwd,int nChannel)
{
	NET_DEVICEINFO devinfo = {0};
	int nError = 0;
	m_pDHHandle = CLIENT_LoginEx(sIP,nPort,sUser,sPwd,0,NULL,&devinfo,&nError);
	if (m_pDHHandle == 0)
	{
		printf("登陆大华设备失败!\n");
		return false;
	}
	printf("登陆大华设备成功,设备类型:%d\n",devinfo.byDVRType);

	m_nChannel = nChannel-1;
	return true;
}

void NetPlugIn::disconnectDVR()
{
	if (m_pDHHandle != -1)
	{
		stopCaptureStream();
		//CLIENT_Logout(m_pDHHandle);
		m_pDHHandle = -1;
	}
}

void NetPlugIn::setCurrentCameraCode(char* sCameraCode)
{
	m_nChannel = 0;
	memset(m_szCameraID,0,sizeof(m_szCameraID));
	strcpy(m_szCameraID,sCameraCode);
}

//nGetVideoType: 0 - 原始流; 1 - 裸码流(帧); 2 - 国标流(RTP-PS)
bool NetPlugIn::captureStream(LPCAPTURESTREAMCALLBACK lpCallBack, void* pUser, int nGetVideoType)
{
	if (m_pDHHandle == -1)
	{
		return false;
	}
	m_nGetVideoType = nGetVideoType;
	m_pUser = pUser;
	m_pStreamCb = lpCallBack;

	m_nPlayId = CLIENT_RealPlayEx(m_pDHHandle, m_nChannel, 0, DH_RType_Realplay);
	if (m_nPlayId == 0)
	{
		printf("播放大华设备实时视频时失败!,错误码：%d\n",CLIENT_GetLastError());
		m_nPlayId = -1;
		return false;
	}

	if (!CLIENT_SetRealDataCallBack(m_nPlayId,fRealDataCB,(LDWORD)this))
	{
		printf("设置大华设备回调函数失败!\n");
		return false;
	}
	
	printf("播放大华设备实时视频成功\n");

#ifdef _DH_ANALYZE_USE
	if (ANA_CreateStream(0,&m_pANAHandle) != DH_NOERROR)
	{
		printf("启用流分析库失败!\n");
		return false;
	}
	printf("启用流分析库成功!\n");
#endif
	return true;
}

bool NetPlugIn::GetMediaInfo(MediaInf & mInf)
{
	char szOutBuffer[102400] = { 0 };
	int error = 0;
	if (!CLIENT_GetNewDevConfig(m_pDHHandle, CFG_CMD_ENCODE, m_nChannel, szOutBuffer, sizeof(szOutBuffer), &error, 3000))
		return false;
	CFG_ENCODE_INFO einfo = { 0 };
	if (!CLIENT_ParseData(CFG_CMD_ENCODE, szOutBuffer, &einfo, sizeof(einfo), NULL))
		return false;
	if (einfo.stuMainStream[0].stuVideoFormat.emCompression == VIDEO_FORMAT_H264)
	{
		strcpy(mInf.fileformat, "98");
		strcpy(mInf.codeType, "H264");
	}
	else if (einfo.stuMainStream[0].stuVideoFormat.emCompression == VIDEO_FORMAT_H265)
	{
		strcpy(mInf.fileformat, "108");
		strcpy(mInf.codeType, "H265");
	}
	else if (einfo.stuMainStream[0].stuVideoFormat.emCompression == VIDEO_FORMAT_SVAC)
	{
		strcpy(mInf.fileformat, "99");
		strcpy(mInf.codeType, "SVAC");
	}
	else
		return false;
	mInf.bit_rate = einfo.stuMainStream[0].stuVideoFormat.nBitRate;
	mInf.v_width = einfo.stuMainStream[0].stuVideoFormat.nWidth;
	mInf.v_height = einfo.stuMainStream[0].stuVideoFormat.nHeight;
	mInf.frame_rate = (int)einfo.stuMainStream[0].stuVideoFormat.nFrameRate;
	m_nVideoType = atoi(mInf.fileformat);
	if (einfo.stuMainStream[0].stuAudioFormat.emCompression == 0)
		sprintf(mInf.colorType, "%d/%d/%d/%d", AUDIO_G711A, einfo.stuMainStream[0].stuAudioFormat.nChannelsNum, einfo.stuMainStream[0].stuAudioFormat.nFrequency, einfo.stuMainStream[0].stuAudioFormat.nDepth);
	else if (einfo.stuMainStream[0].stuAudioFormat.emCompression == 0)
		sprintf(mInf.colorType, "%d/%d/%d/%d", AUDIO_PCM, einfo.stuMainStream[0].stuAudioFormat.nChannelsNum, einfo.stuMainStream[0].stuAudioFormat.nFrequency, einfo.stuMainStream[0].stuAudioFormat.nDepth);
	else if (einfo.stuMainStream[0].stuAudioFormat.emCompression == 0)
		sprintf(mInf.colorType, "%d/%d/%d/%d", AUDIO_G711U, einfo.stuMainStream[0].stuAudioFormat.nChannelsNum, einfo.stuMainStream[0].stuAudioFormat.nFrequency, einfo.stuMainStream[0].stuAudioFormat.nDepth);
	else if (einfo.stuMainStream[0].stuAudioFormat.emCompression == 0)
		sprintf(mInf.colorType, "%d/%d/%d/%d", AUDIO_AAC, einfo.stuMainStream[0].stuAudioFormat.nChannelsNum, einfo.stuMainStream[0].stuAudioFormat.nFrequency, einfo.stuMainStream[0].stuAudioFormat.nDepth);
	return true;
}

//回调裸码流 音频：4；I帧：5; P帧：6;
void NetPlugIn::fRealDataCB(LLONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, LDWORD dwUser)
{
	NetPlugIn* pHandler = (NetPlugIn*)dwUser;
	if (!pHandler || !pBuffer || dwBufSize == 0)
	{
		return;
	}
	//if (pHandler->m_file)
	//{
	//	fwrite(szData,nDataLen,1,pHandler->m_file);
	//	fwrite("\r\n",strlen("\r\n"),1,pHandler->m_file);
	//}
	BYTE *p = pBuffer;
	if (pHandler->m_nGetVideoType == 0)
	{
		if (!pHandler->m_bFirstData)
		{
			printf("开始发送媒体数据...\n");
			if (pHandler->m_pStreamCb)
				pHandler->m_pStreamCb(lRealHandle, 1, NULL, 0, pHandler->m_pUser);
			pHandler->m_bFirstData = true;
		}
		if (pHandler->m_pStreamCb)
			pHandler->m_pStreamCb(lRealHandle, 2, pBuffer, dwBufSize, pHandler->m_pUser);
	}
	else if (pHandler->m_nGetVideoType == 1)
	{

		if (p[0] == 0x44 && p[1] == 0x48 && p[2] == 0x41 && p[3] == 0x56) //大华包头
		{
			int nPackEndLen = (int)p[11];
			int nPackLen = (int)p[12] + ((int)p[13] << 8) + ((int)p[14] << 16) + ((int)p[15] << 24);
			
			if (p[4] == DH_FRAME_A)
			{
				if (pHandler->m_nBufSize > 0)
				{
					if (pHandler->m_pStreamCb)
						pHandler->m_pStreamCb(pHandler->m_nVideoType, 5, pHandler->m_pBuffer, pHandler->m_nFrameLen, pHandler->m_pUser);
					pHandler->m_nBufSize = 0;
				}
				pHandler->m_nFrameLen = nPackLen - DH_HEAD_LEN - nPackEndLen - DH_END_LEN;
				if (pHandler->m_pStreamCb)
					pHandler->m_pStreamCb(pHandler->m_nVideoType, 4, pBuffer+DH_HEAD_LEN, pHandler->m_nFrameLen, pHandler->m_pUser);
			}
			else if (p[4] == DH_FRAME_P)
			{
				if (pHandler->m_nBufSize > 0)
				{
					if (pHandler->m_pStreamCb)
						pHandler->m_pStreamCb(pHandler->m_nVideoType, 5, pHandler->m_pBuffer, pHandler->m_nFrameLen, pHandler->m_pUser);
					pHandler->m_nBufSize = 0;
				}
				pHandler->m_nFrameLen = nPackLen - DH_HEAD_LEN - nPackEndLen - DH_END_LEN;
				if (pHandler->m_pStreamCb)
					pHandler->m_pStreamCb(pHandler->m_nVideoType, 6, pBuffer + DH_HEAD_LEN, pHandler->m_nFrameLen, pHandler->m_pUser);
			}
			else if (p[4] == DH_FRAME_I)
			{
				pHandler->m_nFrameLen = nPackLen - DH_HEAD_LEN - nPackEndLen - DH_END_LEN;
				memcpy(pHandler->m_pBuffer, pBuffer + DH_HEAD_LEN, dwBufSize - DH_HEAD_LEN);
				pHandler->m_nBufSize += dwBufSize - DH_HEAD_LEN;
			}
			else
			{
				if (pHandler->m_nBufSize > 0)
				{
					if (pHandler->m_pStreamCb)
						pHandler->m_pStreamCb(pHandler->m_nVideoType, 5, pHandler->m_pBuffer, pHandler->m_nFrameLen, pHandler->m_pUser);
					pHandler->m_nBufSize = 0;
				}
			}
		}
		else
		{
			memcpy(pHandler->m_pBuffer+pHandler->m_nBufSize, pBuffer, dwBufSize);
			pHandler->m_nBufSize += dwBufSize;
		}
	}
	//int nLen = 0;
	//nLen = (int)p[12] + ((int)p[13] << 8) + ((int)p[14] << 16) + ((int)p[15] << 24);
	//if (p[0] == 0x44 && p[1] == 0x48 && p[2] == 0x41 && p[3] == 0x56 && p[4] == 0xF0)
	//{
	//	printf("A:(%d)[%d][%d] %02x %02x %02x %02x %02x | %c %c %c %c %c\n", dwBufSize, p[11], nLen, p[41], p[42], p[43], p[44], p[45], 
	//		p[dwBufSize-8], p[dwBufSize-7], p[dwBufSize-6], p[dwBufSize-5], p[dwBufSize-4]);
	//}
	//else if (p[0] == 0x44 && p[1] == 0x48 && p[2] == 0x41 && p[3] == 0x56 && p[4] == 0xF1)
	//{
	//	printf("B:(%d)[%d][%d] %02x %02x %02x %02x %02x |  %c %c %c %c %c\n", dwBufSize, p[11], nLen, p[41], p[42], p[43], p[44], p[45], 
	//		p[dwBufSize - 8], p[dwBufSize - 7], p[dwBufSize - 6], p[dwBufSize - 5], p[dwBufSize - 4]);
	//}
	//else if (p[0] == 0x44 && p[1] == 0x48 && p[2] == 0x41 && p[3] == 0x56 && p[4] == 0xFC)
	//{
	//	printf("P:(%d)[%d][%d] %02x %02x %02x %02x %02x |  %c %c %c %c %c\n", dwBufSize, p[11], nLen, p[41], p[42], p[43], p[44], p[45],
	//		p[dwBufSize - 8], p[dwBufSize - 7], p[dwBufSize - 6], p[dwBufSize - 5], p[dwBufSize - 4]);
	//}
	//else if (p[0] == 0x44 && p[1] == 0x48 && p[2] == 0x41 && p[3] == 0x56 && p[4] == 0xFD)
	//{
	//	printf("I:(%d)[%d][%d] %02x %02x %02x %02x %02x |  %c %c %c %c %c\n", dwBufSize, p[11], nLen, p[41], p[42], p[43], p[44], p[45],
	//		p[dwBufSize - 8], p[dwBufSize - 7], p[dwBufSize - 6], p[dwBufSize - 5], p[dwBufSize - 4]);
	//}
	//else
	//{
	//	printf("*:(%d) %02x %02x %02x %02x %02x |  %c %c %c %c %c\n", dwBufSize, p[0], p[1], p[2], p[3], p[4],
	//		p[dwBufSize - 8], p[dwBufSize - 7], p[dwBufSize - 6], p[dwBufSize - 5], p[dwBufSize - 4]);
	//}
}

bool NetPlugIn::stopCaptureStream()
{
	//if (m_file)
	//{
	//	fclose(m_file);
	//}
	if (m_nPlayId != -1)
	{

		if (m_pDHHandle)
		{
			CLIENT_SetRealDataCallBack(m_nPlayId, NULL, NULL);
			CLIENT_StopRealPlayEx(m_nPlayId);
#ifdef _DH_ANALYZE_USE
			if (m_pANAHandle)
			{
				ANA_ClearBuffer(m_pANAHandle);
				ANA_Destroy(m_pANAHandle);
				m_pANAHandle = NULL;
			}
			if (m_hEncHandle)
			{
				Easy_AACEncoder_Release(m_hEncHandle);
				m_hEncHandle = NULL;
			}
#endif
			m_nPlayId = -1;
			m_pStreamCb = NULL;
			m_pUser = NULL;
			m_bFirstData = false;
			m_nCount = 0;
			m_vTimeStamp = 0;
		}
	}
	return true;
}

bool NetPlugIn::controlPTZSpeedWithChannel(int nCmd, bool bEnable, int nSpeed, int nChannel)
{
	if (m_pDHHandle == -1)
	{
		return false;
	}
	
	LONG  lParam1 = 0, lParam2 = 0, lParam3 = 0;
	BOOL bStop = bEnable ? false : true;
	if (!bEnable)
	{
		switch (nCmd)
		{
		case CAMERA_PAN_UP:				//云台向上
			m_dwCmd = DH_PTZ_UP_CONTROL;
			lParam2 = nSpeed;
			bStop = bEnable ? false : true;
			break;
		case CAMERA_PAN_DOWN:			//云台向下
			m_dwCmd = DH_PTZ_DOWN_CONTROL;
			lParam2 = nSpeed;
			bStop = bEnable ? false : true;
			break;
		case CAMERA_PAN_LEFT:			//云台向左
			m_dwCmd = DH_PTZ_LEFT_CONTROL;
			lParam2 = nSpeed;
			bStop = bEnable ? false : true;
			bStop = bEnable ? false : true;
			break;
		case CAMERA_PAN_RIGHT:			//云台向右
			m_dwCmd = DH_PTZ_RIGHT_CONTROL;
			lParam2 = nSpeed;
			bStop = bEnable ? false : true;
			break;
		case CAMERA_PAN_LU:
			m_dwCmd = DH_EXTPTZ_LEFTTOP;
			lParam1 = nSpeed;
			lParam2 = nSpeed;
			bStop = bEnable ? false : true;
			break;
		case CAMERA_PAN_LD:
			m_dwCmd = DH_EXTPTZ_LEFTDOWN;
			lParam1 = nSpeed;
			lParam2 = nSpeed;
			bStop = bEnable ? false : true;
			break;
		case CAMERA_PAN_RU:
			m_dwCmd = DH_EXTPTZ_RIGHTTOP;
			lParam1 = nSpeed;
			lParam2 = nSpeed;
			bStop = bEnable ? false : true;
			break;
		case CAMERA_PAN_RD:
			m_dwCmd = DH_EXTPTZ_RIGHTDOWN;
			lParam1 = nSpeed;
			lParam2 = nSpeed;
			bStop = bEnable ? false : true;
			break;
		case CAMERA_FOCUS_IN:		//焦点前调
			m_dwCmd = DH_PTZ_FOCUS_ADD_CONTROL;
			lParam2 = nSpeed;
			bStop = bEnable ? false : true;
			break;
		case CAMERA_FOCUS_OUT:		//焦点后调
			m_dwCmd = DH_PTZ_FOCUS_DEC_CONTROL;
			lParam2 = nSpeed;
			bStop = bEnable ? false : true;
			break;
		case CAMERA_IRIS_IN:        //光圈调大
			m_dwCmd = DH_PTZ_APERTURE_ADD_CONTROL;
			lParam2 = nSpeed;
			bStop = bEnable ? false : true;
			break;
		case CAMERA_IRIS_OUT:       //光圈调小
			m_dwCmd = DH_PTZ_APERTURE_DEC_CONTROL;
			lParam2 = nSpeed;
			bStop = bEnable ? false : true;
			break;
		case CAMERA_ZOOM_IN:        //倍率调大
			m_dwCmd = DH_PTZ_ZOOM_ADD_CONTROL;
			lParam2 = nSpeed;
			bStop = bEnable ? false : true;
			break;
		case CAMERA_ZOOM_OUT:       //倍率调小
			m_dwCmd = DH_PTZ_ZOOM_DEC_CONTROL;
			lParam2 = nSpeed;

			break;
		case CAMERA_LIGHT_CTRL:     //灯控制
		case CAMERA_HEATER_CTRL:    //加热器控制
			m_dwCmd = DH_EXTPTZ_LIGHTCONTROL;
			lParam1 = bEnable ? 1 : 0;
			break;
		case CAMERA_BRUSH_CTRL:     //雨刷控制
			m_dwCmd = DH_PTZ_LAMP_CONTROL;
			lParam1 = bEnable ? 1 : 0;
			break;
		case CAMERA_AUX_CTRL:       //辅助设备控制
			if (bEnable)
				m_dwCmd = DH_EXTPTZ_AUXIOPEN;
			else
				m_dwCmd = DH_EXTPTZ_AUXICLOSE;
			lParam1 = nSpeed;
		case CAMERA_AUTO_TURN:      //自动旋转
			if (bEnable)
				m_dwCmd = DH_EXTPTZ_STARTLINESCAN;
			else
				m_dwCmd = DH_EXTPTZ_CLOSELINESCAN;
		default:
			return false;
		}
	}
	return CLIENT_DHPTZControlEx2(m_pDHHandle, nChannel, m_dwCmd, lParam1, lParam2, lParam3, bStop) ? true : false;
}

bool NetPlugIn::controlPTZSpeed(int nCmd, bool bEnable,int nSpeed)
{
	return controlPTZSpeedWithChannel(nCmd,bEnable,nSpeed,m_nChannel);
}

bool NetPlugIn::presetPTZWithChannel(int nCmd, int nIndex, int nChannel)
{
	if (m_pDHHandle == -1)
	{
		return false;
	}
	DWORD dwCmd = 0;
	LONG  lParam1 = 0, lParam2 = 0, lParam3 = 0;
	BOOL bStop = false;
	switch(nCmd)
	{
	case PTZ_PRESET_SET:        //设置预置点
		dwCmd = DH_PTZ_POINT_SET_CONTROL;
		lParam2 = nIndex;
		break;
	case PTZ_PRESET_DELETE:     //删除预置点
		dwCmd = DH_PTZ_POINT_DEL_CONTROL;
		lParam2 = nIndex;
		break;
	case PTZ_PRESET_GOTO:       //转到预置点
		dwCmd = DH_PTZ_POINT_MOVE_CONTROL;
		lParam2 = nIndex;
		break;
	default:
		return false;
	}
	return CLIENT_DHPTZControlEx2(m_pDHHandle, nChannel, dwCmd, lParam1, lParam2, lParam3, bStop) ? true : false;
}

bool NetPlugIn::presetPTZ(int nCmd, int nIndex)
{
	return presetPTZWithChannel(nCmd, nIndex, m_nChannel);
}

