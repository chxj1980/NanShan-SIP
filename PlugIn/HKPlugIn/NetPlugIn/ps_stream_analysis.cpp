#include "stdafx.h"
#ifdef _PS_ANALYZE_USE
#include "ps_stream_analysis.h"

ps_stream_analysis::ps_stream_analysis()
{
	InitializeCriticalSection(&m_lock);
	m_streamcb = NULL;
	m_user = NULL;
	m_bFirstData = true;
	m_bFirstAudio = true;
	m_nTimestamp = 0;
	m_nTimeAudio = 0;
	m_tStreamTimeout = 0;
	m_hEncHandle = NULL;
	m_bStop = true;
}


ps_stream_analysis::~ps_stream_analysis()
{
}

void ps_stream_analysis::set_stream_cb(LPCAPTURESTREAMCALLBACK streamcb, void * user, int streamtype)
{
	m_streamcb = streamcb;
	m_user = user;
	m_streamtype = streamtype;
}

DWORD ps_stream_analysis::analysis_thrd(LPVOID lpThreadParameter)
{
	ps_stream_analysis *pThis = (ps_stream_analysis *)lpThreadParameter;
	if (pThis)
	{
		pThis->analysis();
	}
	return 0;
}

void ps_stream_analysis::start()
{
	if (m_bStop)
	{
		m_bStop = false;
		HANDLE hThrd = CreateThread(NULL, 0, analysis_thrd, this, 0, NULL);
		CloseHandle(hThrd);
	}
}

void ps_stream_analysis::stop()
{
	m_bStop = true;
	m_streamcb = NULL;
	m_user = NULL;
}

void ps_stream_analysis::analysis()
{
	m_szFrameBuf = new byte[FRAME_BUF_SZIE];
	m_nFrameBufSize = 0;
	byte *szPesBuf = new byte[PS_BUF_SZIE];
	int	nPesBufCurSize = 0;
	byte *szAACbuf = new byte[2048];
	unsigned int nAAClen = 2048;
	
	while (!m_bStop && m_streamcb)
	{
		if (time(NULL) - m_tStreamTimeout > 10)
		{
			m_bStop = true;
			m_streamcb = NULL;
			m_user = NULL;
			continue;
		}
		if (m_quedata.empty())
		{
			Sleep(30);
			continue;
		}
		EnterCriticalSection(&m_lock);
		byte *pData = m_quedata.front();
		m_quedata.pop();
		LeaveCriticalSection(&m_lock);
		if (pData == nullptr)
		{
			continue;
		}
		int buffsize = 0;
		memcpy(&buffsize, pData, sizeof(int));
		byte *buffer = pData + sizeof(int);
		if (buffsize <= 0)
		{
			continue;
		}
		if (nPesBufCurSize + buffsize > PS_BUF_SZIE)
		{
			printf("己超出一个PES包最大缓冲:%d\n", nPesBufCurSize + (int)buffsize);
			nPesBufCurSize = 0;
		}
		if (buffsize < PS_BUF_SZIE)
		{
			memcpy(szPesBuf + nPesBufCurSize, buffer, buffsize);
			nPesBufCurSize += (int)buffsize;
		}
		delete[] pData;
		pData = NULL;
		if (m_streamtype == -1)
			continue;
		int i = 0;
		while (nPesBufCurSize > 9)
		{
			if (m_streamtype == 0)
			{
				if ((byte)szPesBuf[i] == 0 && (byte)szPesBuf[i + 1] == 0 && (byte)szPesBuf[i + 2] == 1 && (byte)szPesBuf[i + 3] == 0xBC)
				{
					int nPeslen = 6 + szPesBuf[i + 4] * 256 + szPesBuf[i + 5];
					int nSteamTypePos = 10 + szPesBuf[i + 8] * 256 + szPesBuf[i + 9] + 2;
					if (nPeslen <= nSteamTypePos)
					{
						i++;
						nPesBufCurSize--;
						continue;
					}
					if (nPeslen + i > nPesBufCurSize)
						break;
					if (szPesBuf[i + nSteamTypePos] == 0x1B)
						m_streamtype = 96;
					else if (szPesBuf[i + nSteamTypePos] == 0x24)
						m_streamtype = 98;
					else
						m_streamtype = -1;
					i += nPeslen;
					nPesBufCurSize -= nPeslen;
				}
				else
				{
					i++;
					nPesBufCurSize--;
					continue;
				}
			}
			if ((byte)szPesBuf[i] == 0 && (byte)szPesBuf[i + 1] == 0 && (byte)szPesBuf[i + 2] == 1 && ((byte)szPesBuf[i + 3] == 0xE0 || (byte)szPesBuf[i + 3] == 0xC0))
			{
				int nPeslen = 6 + szPesBuf[i + 4] * 256 + szPesBuf[i + 5];	//PES包长度
				int nPeshead = 9 + szPesBuf[i + 8];						//PES包头长度
				if (nPeslen <= nPeshead)
				{
					i++;
					nPesBufCurSize--;
					continue;
				}
				if (nPeslen  > nPesBufCurSize)
					break;
				if ((byte)szPesBuf[i + 3] == 0xC0) //处理音频
				{
					if (m_bFirstAudio)
					{
						m_bFirstAudio = false;
						m_nTimeAudio = GetTickCount();
						InitParam initPar;
						initPar.u32AudioSamplerate = 8000;
						initPar.u32PCMBitSize = 16;
						initPar.ucAudioChannel = 1;
						initPar.ucAudioCodec = Law_ALaw;
						m_hEncHandle = Easy_AACEncoder_Init(initPar);
						if (m_streamcb)
							m_streamcb(-1, 97, (BYTE*)"AAC", 0, m_user);
					}
					if (m_hEncHandle)
					{
						memset(szAACbuf, 0, 2048);
						int nBufLen = Easy_AACEncoder_Encode(m_hEncHandle, szPesBuf + i + nPeshead, nPeslen - nPeshead, szAACbuf, &nAAClen);
						if (nBufLen > 0)
						{
							if (m_streamcb)
								m_streamcb(m_nTimeAudio+=1024, 97, szAACbuf, nBufLen, m_user);
						}
					}
				}
				else if ((byte)szPesBuf[i + 3] == 0xE0)	//处理视频
				{
					if (m_bFirstData)
					{
						if (m_streamcb)
							m_streamcb(-1, 96, (byte*)"H264", 0, m_user);
						m_bFirstData = false;
						//m_nTimestamp = GetTickCount();
						printf("开始传送流媒体数据,StreamType:%d\n", m_streamtype);
					}
					if (m_streamtype == 96)
						analyze_h264(szPesBuf + i + nPeshead, nPeslen - nPeshead);
					else if (m_streamtype == 98)
					{
						//if (pCodec == 0)
						//	pCodec = hevc2avc_init(CODEC_TYPE_H265, 0, 0, 1920, 1080, 1024 * 1024 * 4);
						//if (pCodec == 0)
						//{
						//	printf("获取转码句柄失败！内存不足！\n");
						//	nStreamType = -1;
						//}
						analyze_h265(szPesBuf + i + nPeshead, nPeslen - nPeshead);
					}
				}
				i += nPeslen - 1;
				nPesBufCurSize -= nPeslen - 1;
			}
			i++;
			nPesBufCurSize--;
		}
		if (nPesBufCurSize > 0 && i > 0)
		{
			memcpy(szPesBuf, szPesBuf + i, nPesBufCurSize);
		}
	}
	m_bFirstData = true;
	m_bFirstAudio = true;
	m_nTimestamp = 0;
	m_nTimeAudio = 0;
	delete[] szAACbuf;
	szAACbuf = NULL;
	delete[] szPesBuf;
	szPesBuf = NULL;
	delete[] m_szFrameBuf;
	m_szFrameBuf = NULL;
	m_nFrameBufSize = 0;
	if (m_hEncHandle)
	{
		Easy_AACEncoder_Release(m_hEncHandle);
		m_hEncHandle = NULL;
	}
	EnterCriticalSection(&m_lock);
	while (!m_quedata.empty())
	{
		byte *pData = m_quedata.front();
		m_quedata.pop();
		delete[] pData;
		pData = NULL;
	}
	LeaveCriticalSection(&m_lock);
	DeleteCriticalSection(&m_lock);
	delete this;
}

void ps_stream_analysis::analyze_h264(byte *pBuffer, int nBufferSize)
{
	//printf("frame packet:%x %x %x %x %x %x\n", (byte)pBuffer[0], (byte)pBuffer[1], (byte)pBuffer[2], (byte)pBuffer[3], (byte)pBuffer[4], (byte)pBuffer[5]);
	if ((byte)pBuffer[0] == 0 && (byte)pBuffer[1] == 0 && (byte)pBuffer[2] == 0 && (byte)pBuffer[3] == 1 && ((byte)pBuffer[4] & 0x1f) == 7) //SPS
	{
		if (m_nFrameBufSize > 0)
		{
			if (m_streamcb)
			{
				if (m_nTimestamp > 0)
					m_streamcb(m_nTimestamp += 3600, 2, m_szFrameBuf, m_nFrameBufSize, m_user);
				else
					m_streamcb(0, 2, m_szFrameBuf, m_nFrameBufSize, m_user);
			}
			//printf("frame packet:%x %x %x %x %x %x len:%d\n", (byte)szFrameData[0], (byte)szFrameData[1], (byte)szFrameData[2], (byte)szFrameData[3], (byte)szFrameData[4], (byte)szFrameData[5], nFrameSize);
			m_nFrameBufSize = 0;
		}
		if (m_nFrameBufSize + nBufferSize >= FRAME_BUF_SZIE)
			return;
		memcpy(m_szFrameBuf + m_nFrameBufSize, pBuffer, nBufferSize);
		m_nFrameBufSize += nBufferSize;
	}
	else if ((byte)pBuffer[0] == 0 && (byte)pBuffer[1] == 0 && (byte)pBuffer[2] == 0 && (byte)pBuffer[3] == 1 && ((byte)pBuffer[4] & 0x1f) == 8) //PPS
	{
		if (m_nFrameBufSize > 0)
		{
			if (m_streamcb)
			{
				if (m_nTimestamp > 0)
					m_streamcb(m_nTimestamp, 2, m_szFrameBuf, m_nFrameBufSize, m_user);
				else
					m_streamcb(0, 2, m_szFrameBuf, m_nFrameBufSize, m_user);
			}
			m_nFrameBufSize = 0;
		}
		if (m_nFrameBufSize + nBufferSize >= FRAME_BUF_SZIE)
		{
			printf("组帧PPS时超出缓冲[%d],丢弃!\n", m_nFrameBufSize + nBufferSize);
			return;
		}
		memcpy(m_szFrameBuf + m_nFrameBufSize, pBuffer, nBufferSize);
		m_nFrameBufSize += nBufferSize;
	}
	else if ((byte)pBuffer[0] == 0 && (byte)pBuffer[1] == 0 && (byte)pBuffer[2] == 0 && (byte)pBuffer[3] == 1 && ((byte)pBuffer[4] & 0x1f) == 5) //IDR
	{
		if (m_nFrameBufSize > 0)
		{
			if (m_streamcb)
			{
				if (m_nTimestamp > 0)
					m_streamcb(m_nTimestamp, 2, m_szFrameBuf, m_nFrameBufSize, m_user);
				else
					m_streamcb(0, 2, m_szFrameBuf, m_nFrameBufSize, m_user);
			}
			m_nFrameBufSize = 0;
		}
		if (m_nFrameBufSize + nBufferSize >= FRAME_BUF_SZIE)
		{
			printf("组帧IDR时超出缓冲[%d],丢弃!\n", m_nFrameBufSize + nBufferSize);
			m_nFrameBufSize = 0;
		}
		memcpy(m_szFrameBuf + m_nFrameBufSize, pBuffer, nBufferSize);
		m_nFrameBufSize += nBufferSize;
	}
	else if ((byte)pBuffer[0] == 0 && (byte)pBuffer[1] == 0 && (byte)pBuffer[2] == 0 && (byte)pBuffer[3] == 1 && ((byte)pBuffer[4] & 0x1f) == 1) //PDR
	{
		if (m_nFrameBufSize > 0)
		{
			if (m_streamcb)
			{
				if (m_nTimestamp > 0)
					m_streamcb(m_nTimestamp += 3600, 2, m_szFrameBuf, m_nFrameBufSize, m_user);
				else
					m_streamcb(0, 2, m_szFrameBuf, m_nFrameBufSize, m_user);
			}
			//printf("frame packet:%x %x %x %x %x %x len:%d\n", (byte)szFrameData[0], (byte)szFrameData[1], (byte)szFrameData[2], (byte)szFrameData[3], (byte)szFrameData[4], (byte)szFrameData[5], nFrameSize);
			m_nFrameBufSize = 0;
		}
		memcpy(m_szFrameBuf + m_nFrameBufSize, pBuffer, nBufferSize);
		m_nFrameBufSize += nBufferSize;

	}
	else if ((byte)pBuffer[0] == 0 && (byte)pBuffer[1] == 0 && (byte)pBuffer[2] == 0 && (byte)pBuffer[3] == 1 && ((byte)pBuffer[4] & 0x1f) == 6) //SEI
	{
		return;
	}
	else if ((byte)pBuffer[0] == 0 && (byte)pBuffer[1] == 0 && (byte)pBuffer[2] == 0 && (byte)pBuffer[3] == 1 && ((byte)pBuffer[4] & 0x1f) == 9) //EOF
	{
		return;
	}
	else
	{
		if (m_nFrameBufSize + nBufferSize >= FRAME_BUF_SZIE)
		{
			printf("组帧PDR时超出缓冲[%d],丢弃!\n", m_nFrameBufSize + nBufferSize);
			return;
		}
		memcpy(m_szFrameBuf + m_nFrameBufSize, pBuffer, nBufferSize);
		m_nFrameBufSize += nBufferSize;
	}
}

void ps_stream_analysis::analyze_h265(byte * pBuffer, int nBufferSize)
{
}

void ps_stream_analysis::push_data(byte * data, int datalen)
{
	if (!m_bStop && m_streamcb != nullptr)
	{
		EnterCriticalSection(&m_lock);
		byte *pData = new byte[datalen + sizeof(int)];
		memcpy(pData, &datalen, sizeof(int));
		memcpy(pData + sizeof(int), data, datalen);
		m_quedata.push(pData);
		LeaveCriticalSection(&m_lock);
	}
	m_tStreamTimeout = time(NULL);
}

#endif


