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

NetPlugIn::NetPlugIn(void)
{
	char szSdkPath[256] = { 0 };
	GetModuleFileName(NULL, szSdkPath, sizeof(szSdkPath));
	(strrchr(szSdkPath, '\\'))[0] = '\0';
	strcat(szSdkPath, "\\PlugIn\\HKPlugIn");
	NET_DVR_SetSDKInitCfg(NET_SDK_INIT_CFG_SDK_PATH, szSdkPath);
	NET_DVR_Init();
	NET_DVR_SetLogToFile();
	InitializeCriticalSection(&m_slock);
	m_pHKHandle = -1;
	m_nPlayId = -1;
	m_nError = 0;
	memset(m_szCameraID,0,sizeof(m_szCameraID));
	m_pStreamCb = NULL;
	m_pUser = NULL;
	m_bFirstData = false;
	m_nCount = 0;
	m_pBuffer = new unsigned char[1024*1024];
	m_nBufflen = 0;
	m_vTimeStamp = 0;
	m_aTimeStamp = 0;
	m_file = NULL;

	m_bFirstAudio = false;

#ifdef _PS_ANALYZE_USE
	m_hEncHandle = NULL;
	m_hAnalysis = NULL;
#endif
	
	m_nGetVideoType = 0;
	m_nVideoType = 96;
	m_nFrameRate = 25;
	memset(m_szPrevPack,0,sizeof(m_szPrevPack));
	m_nPrevPackSize = 0;
	memset(m_szRtpPack,0,sizeof(m_szRtpPack));
	m_nRtp_ssrc = 0;
	m_nRtp_cseq = 1;
	m_nRtp_time = GetTickCount();

	m_dwPTZCommand = 0;
}

NetPlugIn::~NetPlugIn(void)
{
	if (m_pBuffer)
	{
		delete[] m_pBuffer;
		m_pBuffer = NULL;
	}
	stopCaptureStream();
	DeleteCriticalSection(&m_slock);
	NET_DVR_Cleanup();
	printf("�ͷź�����ͷ\n");
}

bool NetPlugIn::connectDVR(char* sIP,int nPort,char* sUser,char* sPwd,int nChannel)
{
	NET_DVR_USER_LOGIN_INFO login = {0};
	NET_DVR_DEVICEINFO_V40 devinfo = {0};
	strcpy(login.sDeviceAddress,sIP);
	login.wPort = nPort;
	strcpy(login.sUserName,sUser);
	strcpy(login.sPassword,sPwd);
	m_pHKHandle = NET_DVR_Login_V40(&login,&devinfo);
	if (m_pHKHandle == -1)
	{
		printf("��½�����豸ʧ��!\n");
		return false;
	}
	printf("��½�����豸�ɹ�[%s:%d %s %s %d]\n", sIP, nPort, sUser, sPwd, nChannel);
	m_nProto = devinfo.struDeviceV30.byMainProto;
	m_nChannel = nChannel;

	//m_file = fopen("test.mp4","wb");

	return true;
}

void NetPlugIn::disconnectDVR()
{
	if (m_pHKHandle >= 0)
	{
		stopCaptureStream();
		NET_DVR_Logout(m_pHKHandle);
		m_pHKHandle = -1;
		printf("�ǳ������豸�ɹ�!\n");
	}
}

void NetPlugIn::setCurrentCameraCode(char* sCameraCode)
{
	if (sCameraCode != NULL && sCameraCode[0] != '\0')
	{
		//m_nChannel = atoi(sCameraCode);
		m_nChannel = 1;
		memset(m_szCameraID,0,sizeof(m_szCameraID));
		strcpy(m_szCameraID,sCameraCode);
	}
}

//nGetVideoType: 0 - ԭʼ��; 1 - ������(֡); 2 - ������(RTP-PS)
bool NetPlugIn::captureStream(LPCAPTURESTREAMCALLBACK lpCallBack, void* pUser, int nGetVideoType)
{
	if (m_pHKHandle == -1)
	{
		return false;
	}
	m_nGetVideoType = nGetVideoType;

	m_pUser = pUser;
	m_pStreamCb = lpCallBack;

	NET_DVR_PREVIEWINFO info = {0};
	info.lChannel = m_nChannel;
	info.bBlocked = 0;
	info.byProtoType = 0;
	m_nPlayId = NET_DVR_RealPlay_V40(m_pHKHandle, &info, NULL, NULL);
	if (m_nPlayId == -1)
	{
		printf("���ź����豸ʵʱ��Ƶʱʧ��!,�����룺%d\n",NET_DVR_GetLastError());
		m_nPlayId = -1;

		return false;
	}
#ifdef _PS_ANALYZE_USE
	m_hAnalysis = new ps_stream_analysis();
	m_hAnalysis->set_stream_cb(lpCallBack, pUser);
	NET_DVR_SetRealDataCallBackEx(m_nPlayId, fStdDataCB, m_hAnalysis);
#else
	if (m_nGetVideoType == 1)
		NET_DVR_SetESRealPlayCallBack(m_nPlayId, fPlayESCB, this);
	else
		NET_DVR_SetRealDataCallBackEx(m_nPlayId, fStdDataCB, this);
#endif
	
	printf("���ź����豸ʵʱ��Ƶ[%d]�ɹ�\n", m_nPlayId);
	return true;
}

//���� ����RTP-SSRC
bool NetPlugIn::SetStreamSaveTime(int nSaveTime)	
{
	m_nRtp_ssrc = nSaveTime;
	return true;
}
//���� ��ȡ��Ƶѹ������
bool NetPlugIn::GetMediaInfo(MediaInf & mInf)
{
	NET_DVR_COMPRESSIONCFG_V30 compression_cfg = { 0 };
	compression_cfg.dwSize = sizeof(compression_cfg);
	DWORD lpSize = 0;
	if (NET_DVR_GetDVRConfig(m_pHKHandle, NET_DVR_GET_COMPRESSCFG_V30, m_nChannel, &compression_cfg, compression_cfg.dwSize, &lpSize))
	{
		
		if (compression_cfg.struNormHighRecordPara.byVideoEncType == 1)
		{
			//strcpy(mInf.fileformat, "98");
			//strcpy(mInf.codeType, "H264");
			strcpy(mInf.fileformat, "96");
			strcpy(mInf.codeType, "PS");
		}
		else if (compression_cfg.struNormHighRecordPara.byVideoEncType == 9)
		{
			strcpy(mInf.fileformat, "99");
			strcpy(mInf.codeType, "SVAC");
		}
		else if (compression_cfg.struNormHighRecordPara.byVideoEncType == 10)
		{
			strcpy(mInf.fileformat, "108");
			strcpy(mInf.codeType, "H265");
		}
		else
		{
			strcpy(mInf.fileformat, "96");
			strcpy(mInf.codeType, "PS");
		}
		if (compression_cfg.struNormHighRecordPara.dwVideoBitrate == 15)
			mInf.frame_rate = 512;
		else if (compression_cfg.struNormHighRecordPara.dwVideoBitrate == 16)
			mInf.frame_rate = 640;
		else if (compression_cfg.struNormHighRecordPara.dwVideoBitrate == 17)
			mInf.frame_rate = 768;
		else if (compression_cfg.struNormHighRecordPara.dwVideoBitrate == 18)
			mInf.frame_rate = 896;
		else if (compression_cfg.struNormHighRecordPara.dwVideoBitrate == 19)
			mInf.frame_rate = 1024;
		else if (compression_cfg.struNormHighRecordPara.dwVideoBitrate == 20)
			mInf.frame_rate = 1280;
		else if (compression_cfg.struNormHighRecordPara.dwVideoBitrate == 21)
			mInf.frame_rate = 1536;
		else if (compression_cfg.struNormHighRecordPara.dwVideoBitrate == 22)
			mInf.frame_rate = 1792;
		else if (compression_cfg.struNormHighRecordPara.dwVideoBitrate == 23)
			mInf.frame_rate = 2048;
		else if (compression_cfg.struNormHighRecordPara.dwVideoBitrate == 24)
			mInf.frame_rate = 3072;
		else if (compression_cfg.struNormHighRecordPara.dwVideoBitrate == 25)
			mInf.frame_rate = 4096;
		else if (compression_cfg.struNormHighRecordPara.dwVideoBitrate == 26)
			mInf.frame_rate = 8192;
		else
			mInf.frame_rate = 2048;
		if (compression_cfg.struNormHighRecordPara.dwVideoFrameRate == 10)
			mInf.frame_rate = 10;
		else if (compression_cfg.struNormHighRecordPara.dwVideoFrameRate == 11)
			mInf.frame_rate = 12;
		else if (compression_cfg.struNormHighRecordPara.dwVideoFrameRate == 12)
			mInf.frame_rate = 16;
		else if (compression_cfg.struNormHighRecordPara.dwVideoFrameRate == 13)
			mInf.frame_rate = 20;
		else if (compression_cfg.struNormHighRecordPara.dwVideoFrameRate == 14)
			mInf.frame_rate = 15;
		else if (compression_cfg.struNormHighRecordPara.dwVideoFrameRate == 15)
			mInf.frame_rate = 18;
		else if (compression_cfg.struNormHighRecordPara.dwVideoFrameRate == 16)
			mInf.frame_rate = 22;
		else if (compression_cfg.struNormHighRecordPara.dwVideoFrameRate == 17)
			mInf.frame_rate = 25;
		else if (compression_cfg.struNormHighRecordPara.dwVideoFrameRate == 18)
			mInf.frame_rate = 30;
		else if (compression_cfg.struNormHighRecordPara.dwVideoFrameRate == 22)
			mInf.frame_rate = 50;
		else
			mInf.frame_rate = 25;
		if (compression_cfg.struNormHighRecordPara.byResolution == 3)
		{
			mInf.v_width = 704;
			mInf.v_height = 576;
		}
		else if (compression_cfg.struNormHighRecordPara.byResolution == 19)
		{
			mInf.v_width = 1280;
			mInf.v_height = 720;
		}
		else if (compression_cfg.struNormHighRecordPara.byResolution == 27)
		{
			mInf.v_width = 1920;
			mInf.v_height = 1080;
		}
		else if (compression_cfg.struNormHighRecordPara.byResolution == 63)
		{
			mInf.v_width = 4096;
			mInf.v_height = 2160;
		}
		else if (compression_cfg.struNormHighRecordPara.byResolution == 64)
		{
			mInf.v_width = 3840;
			mInf.v_height = 2160;
		}
		else if (compression_cfg.struNormHighRecordPara.byResolution == 70)
		{
			mInf.v_width = 2560;
			mInf.v_height = 1440;
		}
		else
		{
			mInf.v_width = 1920;
			mInf.v_height = 1080;
		}
		m_nFrameRate = mInf.frame_rate;
		m_nVideoType = atoi(mInf.fileformat);
		int nAudioType, nAudioChannels, nAudioFrequency, nAudioDepth;
		if (compression_cfg.struNormHighRecordPara.byAudioEncType == 8)
			nAudioType = AUDIO_PCM;
		else if (compression_cfg.struNormHighRecordPara.byAudioEncType == 0)
			nAudioType = AUDIO_G722;
		else if (compression_cfg.struNormHighRecordPara.byAudioEncType == 2)
			nAudioType = AUDIO_G711A;
		else if (compression_cfg.struNormHighRecordPara.byAudioEncType == 1)
			nAudioType = AUDIO_G711U;
		else if (compression_cfg.struNormHighRecordPara.byAudioEncType == 7)
			nAudioType = AUDIO_AAC;
		else if (compression_cfg.struNormHighRecordPara.byAudioEncType == 12)
			nAudioType = AUDIO_AACLC;
		else if (compression_cfg.struNormHighRecordPara.byAudioEncType == 13)
			nAudioType = AUDIO_AACLD;
		nAudioChannels = 1;
		if (compression_cfg.struNormHighRecordPara.byAudioSamplingRate == 1)
			nAudioFrequency = 16000;
		else if (compression_cfg.struNormHighRecordPara.byAudioSamplingRate == 2)
			nAudioFrequency = 32000;
		else if (compression_cfg.struNormHighRecordPara.byAudioSamplingRate == 3)
			nAudioFrequency = 48000;
		else if (compression_cfg.struNormHighRecordPara.byAudioSamplingRate == 4)
			nAudioFrequency = 44100;
		else if (compression_cfg.struNormHighRecordPara.byAudioSamplingRate == 5)
			nAudioFrequency = 8000;
		else
			nAudioFrequency = 16000;
		nAudioDepth = 16;
		sprintf(mInf.colorType, "%d/%d/%d/%d", nAudioType, nAudioChannels, nAudioFrequency, nAudioDepth);
		return true;
	}
	return false;
}

void NetPlugIn::fStdDataCB(LONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void *pUser)
{
#ifdef _PS_ANALYZE_USE
	ps_stream_analysis *pAnalysis = (ps_stream_analysis *)pUser;
#else
	NetPlugIn *pAnalysis = (NetPlugIn *)pUser;
#endif
	if (pAnalysis == NULL || pBuffer == NULL || dwBufSize <= 0)
	{
		return;
	}
	if (pAnalysis->m_nGetVideoType == 0)  //�ص�ԭʼ����
	{
		if (dwDataType == NET_DVR_SYSHEAD)
		{
#ifdef _PS_ANALYZE_USE
			if (pAnalysis)
				pAnalysis->start();
#else
			if (pAnalysis->m_pStreamCb)
				pAnalysis->m_pStreamCb(lRealHandle, dwDataType, pBuffer, dwBufSize, pAnalysis->m_pUser);
#endif
		}
		else if (dwDataType == NET_DVR_STREAMDATA)
		{
#if _PS_ANALYZE_USE
			if (pAnalysis)
				pAnalysis->push_data(pBuffer, dwBufSize);
#else
			if (pAnalysis->m_pStreamCb)
				pAnalysis->m_pStreamCb(lRealHandle, dwDataType, pBuffer, dwBufSize, pAnalysis->m_pUser);
#endif
		}
	}
	else                                   //�ص�RTP-PS
	{
		if (pBuffer[0] != 0 && pBuffer[1] != 0 && pBuffer[2] != 1)	//����ͷ
			return;
		if (pBuffer[0] == 0 && pBuffer[1] == 0 && pBuffer[2] == 1 && pBuffer[3] == 0xBD) //����˽�а�
			return;
		if (pAnalysis->m_nPrevPackSize == 0)	//�Ȼ���һ��
		{
			memcpy(pAnalysis->m_szPrevPack, pBuffer, dwBufSize);
			pAnalysis->m_nPrevPackSize = dwBufSize;
		}
		else
		{
			int nPackSize = pAnalysis->m_nPrevPackSize;
			BYTE *pPackData = pAnalysis->m_szPrevPack;
			while (nPackSize > 0)
			{
				if (nPackSize > 1400)
				{
					pAnalysis->rtp_format();
					memcpy(pAnalysis->m_szRtpPack + 12, pPackData, 1400);
					if (pAnalysis->m_pStreamCb)
						pAnalysis->m_pStreamCb(96, 2, pAnalysis->m_szRtpPack, 1412, pAnalysis->m_pUser);
					pPackData += 1400;
					nPackSize -= 1400;
				}
				else
				{
					if (pBuffer[0] == 0 && pBuffer[1] == 0 && pBuffer[2] == 1 && pBuffer[3] == 0xBA) //֡��ͷ
					{
						pAnalysis->rtp_format(true);
						pAnalysis->m_nRtp_time += (90000 / pAnalysis->m_nFrameRate);
					}
					else
						pAnalysis->rtp_format();
					memcpy(pAnalysis->m_szRtpPack + 12, pPackData, nPackSize);
					if (pAnalysis->m_pStreamCb)
						pAnalysis->m_pStreamCb(96, 2, pAnalysis->m_szRtpPack, nPackSize+12, pAnalysis->m_pUser);
					nPackSize = 0;
				}
			}
			memcpy(pAnalysis->m_szPrevPack, pBuffer, dwBufSize);
			pAnalysis->m_nPrevPackSize = dwBufSize;
		}
	}
}

//�ص������� ��Ƶ��4��I֡��5; P֡��6;
void NetPlugIn::fPlayESCB(LONG lPreviewHandle, NET_DVR_PACKET_INFO_EX * pstruPackInfo, void * pUser)
{
	NetPlugIn *pAnalysis = (NetPlugIn *)pUser;
	if (pstruPackInfo && pAnalysis && pAnalysis->m_pStreamCb)
	{
		if (pstruPackInfo->dwPacketType == 1)
			pAnalysis->m_pStreamCb(pAnalysis->m_nVideoType, 5, pstruPackInfo->pPacketBuffer, pstruPackInfo->dwPacketSize, pAnalysis->m_pUser);
		else if (pstruPackInfo->dwPacketType == 3)
			pAnalysis->m_pStreamCb(pAnalysis->m_nVideoType, 6, pstruPackInfo->pPacketBuffer, pstruPackInfo->dwPacketSize, pAnalysis->m_pUser);
		else if (pstruPackInfo->dwPacketType == 10)
			pAnalysis->m_pStreamCb(pAnalysis->m_nVideoType, 4, pstruPackInfo->pPacketBuffer, pstruPackInfo->dwPacketSize, pAnalysis->m_pUser);
	}
}

void NetPlugIn::rtp_format(bool mark)
{
	m_szRtpPack[0] = 0x80;
	if (mark)
		m_szRtpPack[1] = 0xE0;
	else
		m_szRtpPack[1] = 0x60;
	m_szRtpPack[2] = m_nRtp_cseq >> 8;
	m_szRtpPack[3] = m_nRtp_cseq & 0xff;
	m_nRtp_cseq++;
	m_szRtpPack[4] = m_nRtp_time >> 24;
	m_szRtpPack[5] = m_nRtp_time >> 16;
	m_szRtpPack[6] = m_nRtp_time >> 8;
	m_szRtpPack[7] = m_nRtp_time & 0xff;
	m_szRtpPack[8] = m_nRtp_ssrc  >> 24;
	m_szRtpPack[9] = m_nRtp_ssrc  >> 16;
	m_szRtpPack[10] = m_nRtp_ssrc >> 8;
	m_szRtpPack[11] = m_nRtp_ssrc & 0xff;
}

bool NetPlugIn::stopCaptureStream()
{
	if (m_file)
	{
		fclose(m_file);
	}
	if (m_nPlayId >= 0)
	{
		//printf("�رպ�����ͷ[%d]\n", m_nPlayId);
		//EnterCriticalSection(&m_slock);
		if (m_pHKHandle >= 0)
		{
			if (m_nGetVideoType == 1)
				NET_DVR_SetESRealPlayCallBack(m_nPlayId, NULL, this);
			else
				NET_DVR_SetRealDataCallBackEx(m_nPlayId, NULL, this);
			NET_DVR_StopRealPlay(m_nPlayId);
		}
#ifdef _PS_ANALYZE_USE
		if (m_hAnalysis)
		{
			m_hAnalysis->stop();
			m_hAnalysis = NULL;
		}
		if (m_hEncHandle)
		{	
			Easy_AACEncoder_Release(m_hEncHandle);
			m_hEncHandle = NULL;
		}
#endif
		m_pStreamCb = NULL;
		m_pUser = NULL;
		m_bFirstData = false;
		m_bFirstAudio = false;
		m_nCount = 0;
		m_nBufflen = 0;
		m_vTimeStamp = 0;
		m_nPlayId = -1;

		m_nGetVideoType = 0;
		m_nRtp_ssrc = 0;
		m_nRtp_cseq = 1;
		m_nRtp_time = GetTickCount();
		//LeaveCriticalSection(&m_slock);
		//printf("�رպ�����ͷ�ɹ�\n");
	}
	return true;
}

bool NetPlugIn::controlPTZSpeedWithChannel(int nCmd, bool bEnable, int nSpeed, int nChannel)
{
	if (m_pHKHandle == -1)
	{
		return false;
	}
	if (!bEnable)
	{
		switch (nCmd)
		{
		case CAMERA_PAN_UP:			//��̨����
			m_dwPTZCommand = TILT_UP;
			break;
		case CAMERA_PAN_DOWN:		//��̨����
			m_dwPTZCommand = TILT_DOWN;
			break;
		case CAMERA_PAN_LEFT:		//��̨����
			m_dwPTZCommand = PAN_LEFT;
			break;
		case CAMERA_PAN_RIGHT:		//��̨����
			m_dwPTZCommand = PAN_RIGHT;
			break;
		case CAMERA_FOCUS_IN:		//����ǰ��
			m_dwPTZCommand = FOCUS_NEAR;
			break;
		case CAMERA_FOCUS_OUT:		//������
			m_dwPTZCommand = FOCUS_FAR;
			break;
		case CAMERA_IRIS_IN:        //��Ȧ����
			m_dwPTZCommand = IRIS_OPEN;
			break;
		case CAMERA_IRIS_OUT:       //��Ȧ��С
			m_dwPTZCommand = IRIS_CLOSE;
			break;
		case CAMERA_ZOOM_IN:        //���ʵ���
			m_dwPTZCommand = ZOOM_IN;
			break;
		case CAMERA_ZOOM_OUT:       //���ʵ�С
			m_dwPTZCommand = ZOOM_OUT;
			break;
		case CAMERA_LIGHT_CTRL:     //�ƿ���
			m_dwPTZCommand = LIGHT_PWRON;
			break;
		case CAMERA_BRUSH_CTRL:     //��ˢ����
			m_dwPTZCommand = WIPER_PWRON;
			break;
		case CAMERA_HEATER_CTRL:    //����������
			m_dwPTZCommand = HEATER_PWRON;
			break;
		case CAMERA_AUX_CTRL:       //�����豸����
			m_dwPTZCommand = AUX_PWRON1;
			break;
		case CAMERA_AUTO_TURN:      //�Զ���ת
			m_dwPTZCommand = PAN_AUTO;
			break;
		case CAMERA_PAN_LU:
			m_dwPTZCommand = UP_LEFT;
			break;
		case CAMERA_PAN_LD:
			m_dwPTZCommand = DOWN_LEFT;
			break;
		case CAMERA_PAN_RU:
			m_dwPTZCommand = UP_RIGHT;
			break;
		case CAMERA_PAN_RD:
			m_dwPTZCommand = DOWN_RIGHT;
			break;
		default:
			return false;
		}
	}
	DWORD dwStop = bEnable?1:0;
	BOOL bRet = NET_DVR_PTZControlWithSpeed_Other(m_pHKHandle, m_nChannel, m_dwPTZCommand, dwStop, nSpeed);
	if (!bRet)
	{
		DWORD nError = NET_DVR_GetLastError();
	}
	return bRet?true:false;
}

bool NetPlugIn::controlPTZSpeed(int nCmd, bool bEnable,int nSpeed)
{
	return controlPTZSpeedWithChannel(nCmd,bEnable,nSpeed,m_nChannel);
}

bool NetPlugIn::presetPTZWithChannel(int nCmd, int nIndex, int nChannel)
{
	if (m_pHKHandle == -1)
	{
		return false;
	}
	int dwPTZCommand = 0;
	switch(nCmd)
	{
	case PTZ_PRESET_SET:        //����Ԥ�õ�
		{
			dwPTZCommand = SET_PRESET;
			break;
		}
	case PTZ_PRESET_DELETE:     //ɾ��Ԥ�õ�
		{
			dwPTZCommand = CLE_PRESET;
			break;
		}
	case PTZ_PRESET_GOTO:       //ת��Ԥ�õ�
		{
			dwPTZCommand = GOTO_PRESET;
			break;
		}
	default:
		return false;
	}
	BOOL bRet = NET_DVR_PTZPreset_Other(m_pHKHandle, m_nChannel, dwPTZCommand, nIndex);
	return bRet?true:false;
}

bool NetPlugIn::presetPTZ(int nCmd, int nIndex)
{
	return presetPTZWithChannel(nCmd, nIndex, m_nChannel);
}

