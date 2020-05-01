#include "stdafx.h"
#include "PSStreamAnalyze.h"
#include "h264_sps_parse.h"

PSStreamAnalyze::PSStreamAnalyze(int nSteamType, LPCAPTURESTREAMCALLBACK pCb, void * pUser)
{
	m_nStreamType = nSteamType;
	m_pStreamCb = pCb;
	m_pStreamUser = pUser;
	
	m_nTimestamp = 0;
	m_nfreq = 3600;
	m_pFifo = acl_fifo_new();
	m_pCodec = NULL;
	m_nUsec = 0;
	m_bFirstData = true;
	m_bStop = false;

	m_file = NULL;
}

PSStreamAnalyze::~PSStreamAnalyze()
{
	
}

void PSStreamAnalyze::add_data(unsigned char * pBuffer, unsigned int nBufferSize)
{
	m_lockerFifo.lock();
	if (!m_bStop && m_pFifo)
	{
		if (m_nStreamType == -1)
		{
			m_lockerFifo.unlock();
			return;
		}
		unsigned char *pBuf = (unsigned char *)acl_mymalloc(nBufferSize + sizeof(unsigned int));
		memcpy(pBuf, &nBufferSize, sizeof(unsigned int));
		memcpy(pBuf + sizeof(unsigned int), pBuffer, nBufferSize);
		acl_fifo_push(m_pFifo, pBuf);
	}
	m_lockerFifo.unlock();
}

void PSStreamAnalyze::stop()
{
	m_bStop = true;
}

void * PSStreamAnalyze::run()
{
	//m_file = fopen("test_es.mp4", "ab+");
	m_szFrameData = new unsigned char[FRAME_BUF_SIZE];
	m_nFrameSize = 0;
	unsigned char szPesBuf[RTP_BUF_SIZE] = { 0 };
	int	nPesBufCurSize = 0;
	while (!m_bStop && m_pFifo)
	{
		int nPackets = acl_fifo_size(m_pFifo);
		if (nPackets == 0)
		{
			sip_sleep(30);
			continue;
		}
		m_lockerFifo.lock();
		unsigned char *pData = (unsigned char *)acl_fifo_pop(m_pFifo);
		m_lockerFifo.unlock();
		if (pData == NULL)
			continue;
		unsigned int buffsize = 0;
		memcpy(&buffsize, pData, sizeof(unsigned int));
		unsigned char *buffer = pData + sizeof(unsigned int);
		if (nPesBufCurSize + buffsize > RTP_BUF_SIZE)
		{
			printf("己超出一个PES包最大缓冲:%d\n", nPesBufCurSize + (int)buffsize);
			nPesBufCurSize = 0;
		}
		memcpy(szPesBuf + nPesBufCurSize, buffer, buffsize);
		nPesBufCurSize += (int)buffsize;
		acl_myfree(pData);
		pData = NULL;
		if (m_nStreamType == -1)
			continue;
		int i = 0;
		while (nPesBufCurSize > 9 && !m_bStop)
		{
			if (m_nStreamType == 0)
			{
				if ((BYTE)szPesBuf[i] == 0 && (BYTE)szPesBuf[i + 1] == 0 && (BYTE)szPesBuf[i + 2] == 1 && (BYTE)szPesBuf[i + 3] == 0xBC)
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
						m_nStreamType = 96;
					else if (szPesBuf[i + nSteamTypePos] == 0x24)
						m_nStreamType = 98;
					else
						m_nStreamType = -1;
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
			if ((BYTE)szPesBuf[i] == 0 && (BYTE)szPesBuf[i + 1] == 0 && (BYTE)szPesBuf[i + 2] == 1 && ((BYTE)szPesBuf[i + 3] == 0xE0 || (BYTE)szPesBuf[i + 3] == 0xC0))
			{
				int nPeslen = 6 + szPesBuf[i + 4] * 256 + szPesBuf[i + 5];	//PES包长度
				int nPeshead = 9 + szPesBuf[i + 8];							//PES包头长度
				if (nPeslen <= nPeshead)
				{
					i++;
					nPesBufCurSize--;
					continue;
				}
				if (nPeslen  > nPesBufCurSize)
					break;
				if ((BYTE)szPesBuf[i + 3] == 0xE0)		//视频流
				{
					if (m_bFirstData)
					{
						if (m_pStreamCb)
							m_pStreamCb(-1, 96, (BYTE*)"H264", 0, m_pStreamUser);
						m_bFirstData = false;
						m_nTimestamp = GetTickCount();
						printf("开始传送流媒体数据,StreamType:%d\n", m_nStreamType);
					}
					if (m_nStreamType == 96)
					{
						analyze_h264(szPesBuf + i + nPeshead, nPeslen - nPeshead);
					}
					else if (m_nStreamType == 98)
					{
#ifdef _HEVC2AVC_USE
						if (m_pCodec == NULL)
						{
							m_pCodec = hevc2avc_init(CODEC_TYPE_H265, 0, 0, 1920, 1080, 1024 * 1024 * 4);
							if (m_pCodec == NULL)
							{
								printf("获取转码句柄失败！内存不足！\n");
								m_nStreamType = -1;
								continue;
							}
							hevc2avc_setcb(m_pCodec, VideoCodingFrameCb, this);
						}
#endif
						analyze_h265(szPesBuf + i + nPeshead, nPeslen - nPeshead);
					}
				}
				else if ((BYTE)szPesBuf[i + 3] == 0xC0) {}//音频流
				i += nPeslen - 1;
				nPesBufCurSize -= nPeslen - 1;
			}
			i++;
			nPesBufCurSize--;
		}
		if (nPesBufCurSize > 0)
		{
			memcpy(szPesBuf, szPesBuf + i, nPesBufCurSize);
		}
	}
	//解析线程结束
	if (m_file)
		fclose(m_file);
	m_nUsec = 0;
	m_bFirstData = true;
#ifdef _HEVC2AVC_USE
	if (m_pCodec > 0)
	{
		hevc2avc_uninit(m_pCodec);
		m_pCodec = NULL;
	}
#endif
	if (m_szFrameData)
	{
		delete[] m_szFrameData;
		m_szFrameData = NULL;
	}
	m_nTimestamp = 0;
	m_nfreq = 0;
	m_nFrameSize = 0;
	m_lockerFifo.lock();
	if (m_pFifo)
	{
		acl_fifo_free(m_pFifo, acl_myfree_fn);
		m_pFifo = NULL;
	}
	m_lockerFifo.unlock();
	delete this;
	return NULL;
}

void PSStreamAnalyze::analyze_h264(unsigned char * pBuffer, unsigned int nBufferSize)
{
	if ((BYTE)pBuffer[0] == 0 && (BYTE)pBuffer[1] == 0 && (BYTE)pBuffer[2] == 0 && (BYTE)pBuffer[3] == 1 && ((BYTE)pBuffer[4] & 0x1f) == 7) //SPS
	{
		if (m_nFrameSize > 0)
		{
			if (m_pStreamCb)
				m_pStreamCb(m_nTimestamp += m_nfreq, DVR_DATA_STREAM, m_szFrameData, m_nFrameSize, m_pStreamUser);
			m_nFrameSize = 0;
		}
		memcpy(m_szFrameData + m_nFrameSize, pBuffer, nBufferSize);
		m_nFrameSize += nBufferSize;
		if (m_nUsec == 0)
		{
			int nWidth, nHeight;
			float fFps = 25;
			h264_sps_parse(pBuffer + 4, nBufferSize - 4, nWidth, nHeight, fFps);
			printf("当前媒体流分辨率为%d x %d,帧率为:%d\n", nWidth, nHeight, (int)fFps);
			m_nUsec = (int)(1000 / fFps - 10);
			m_nfreq = (int)(90000 / fFps);
		}
		sip_sleep(m_nUsec);

	}
	else if ((BYTE)pBuffer[0] == 0 && (BYTE)pBuffer[1] == 0 && (BYTE)pBuffer[2] == 0 && (BYTE)pBuffer[3] == 1 && ((BYTE)pBuffer[4] & 0x1f) == 8) //PPS
	{
		if (m_nFrameSize + nBufferSize >= FRAME_BUF_SIZE)
		{
			printf("组帧PPS时超出缓冲[%d],丢弃!\n", m_nFrameSize + nBufferSize);
			return;
		}
		memcpy(m_szFrameData + m_nFrameSize, pBuffer, nBufferSize);
		m_nFrameSize += nBufferSize;
	}
	else if ((BYTE)pBuffer[0] == 0 && (BYTE)pBuffer[1] == 0 && (BYTE)pBuffer[2] == 0 && (BYTE)pBuffer[3] == 1 && ((BYTE)pBuffer[4] & 0x1f) == 5) //IDR
	{
		if (m_nFrameSize + nBufferSize >= FRAME_BUF_SIZE)
		{
			printf("组帧IDR时超出缓冲[%d],丢弃!\n", m_nFrameSize + nBufferSize);
			m_nFrameSize = 0;
		}
		memcpy(m_szFrameData + m_nFrameSize, pBuffer, nBufferSize);
		m_nFrameSize += nBufferSize;
	}
	else if ((BYTE)pBuffer[0] == 0 && (BYTE)pBuffer[1] == 0 && (BYTE)pBuffer[2] == 0 && (BYTE)pBuffer[3] == 1 && ((BYTE)pBuffer[4] & 0x1f) == 1) //PDR
	{
		if (m_nFrameSize > 0)
		{
			if (m_pStreamCb)
				m_pStreamCb(m_nTimestamp += m_nfreq, DVR_DATA_STREAM, m_szFrameData, m_nFrameSize, m_pStreamUser);
			m_nFrameSize = 0;
		}
		memcpy(m_szFrameData + m_nFrameSize, pBuffer, nBufferSize);
		m_nFrameSize += nBufferSize;
		sip_sleep(m_nUsec);

	}
	else if ((BYTE)pBuffer[0] == 0 && (BYTE)pBuffer[1] == 0 && (BYTE)pBuffer[2] == 0 && (BYTE)pBuffer[3] == 1 && ((BYTE)pBuffer[4] & 0x1f) == 6) //SEI
	{
		return;
	}
	else if ((BYTE)pBuffer[0] == 0 && (BYTE)pBuffer[1] == 0 && (BYTE)pBuffer[2] == 0 && (BYTE)pBuffer[3] == 1 && ((BYTE)pBuffer[4] & 0x1f) == 9) //EOF
	{
		return;
	}
	else
	{
		if (m_nFrameSize + nBufferSize >= FRAME_BUF_SIZE)
		{
			printf("组帧PDR时超出缓冲[%d],丢弃!\n", m_nFrameSize + nBufferSize);
			return;
		}
		memcpy(m_szFrameData + m_nFrameSize, pBuffer, nBufferSize);
		m_nFrameSize += nBufferSize;
	}
}

void PSStreamAnalyze::analyze_h265(unsigned char * pBuffer, unsigned int nBufferSize)
{
	if ((BYTE)pBuffer[0] == 0 && (BYTE)pBuffer[1] == 0 && (BYTE)pBuffer[2] == 0 && (BYTE)pBuffer[3] == 1 && ((BYTE)pBuffer[4] & 0x7e) >> 1 == 32) //VPS
	{
		if (m_nFrameSize > 0)
		{
			if (m_file)
				fwrite(m_szFrameData, m_nFrameSize, 1, m_file);
#ifdef _HEVC2AVC_USE
			if (m_pCodec > 0)
				hevc2avc_input(m_pCodec, m_szFrameData, m_nFrameSize);
#else
			m_pStreamCb(0, DVR_DATA_STREAM, m_szFrameData, m_nFrameSize, m_pStreamUser);
#endif
			m_nFrameSize = 0;
		}
		memcpy(m_szFrameData + m_nFrameSize, pBuffer, nBufferSize);
		m_nFrameSize += nBufferSize;
	}
	else if ((BYTE)pBuffer[0] == 0 && (BYTE)pBuffer[1] == 0 && (BYTE)pBuffer[2] == 0 && (BYTE)pBuffer[3] == 1 && ((BYTE)pBuffer[4] & 0x7e) >> 1 == 33) //SPS
	{
		if (m_nFrameSize > 0)
		{
			if (m_file)
				fwrite(m_szFrameData, m_nFrameSize, 1, m_file);
#ifdef _HEVC2AVC_USE
			if (m_pCodec > 0)
				hevc2avc_input(m_pCodec, m_szFrameData, m_nFrameSize);
#else
			m_pStreamCb(0, DVR_DATA_STREAM, m_szFrameData, m_nFrameSize, m_pStreamUser);
#endif
			m_nFrameSize = 0;
		}
		//if (m_nFrameSize + nBufferSize >= FRAME_BUF_SIZE)
		//{
		//	printf("组帧SPS时超出缓冲[%d]\n", m_nFrameSize + nBufferSize);
		//	m_nFrameSize = 0;
		//}
		memcpy(m_szFrameData + m_nFrameSize, pBuffer, nBufferSize);
		m_nFrameSize += nBufferSize;
	}
	else if ((BYTE)pBuffer[0] == 0 && (BYTE)pBuffer[1] == 0 && (BYTE)pBuffer[2] == 0 && (BYTE)pBuffer[3] == 1 && ((BYTE)pBuffer[4] & 0x7e) >> 1 == 34) //PPS
	{
		if (m_nFrameSize > 0)
		{
			if (m_file)
				fwrite(m_szFrameData, m_nFrameSize, 1, m_file);
#ifdef _HEVC2AVC_USE
			if (m_pCodec > 0)
				hevc2avc_input(m_pCodec, m_szFrameData, m_nFrameSize);
#else
			m_pStreamCb(0, DVR_DATA_STREAM, m_szFrameData, m_nFrameSize, m_pStreamUser);
#endif
			m_nFrameSize = 0;
		}
		//if (m_nFrameSize + nBufferSize >= FRAME_BUF_SIZE)
		//{
		//	printf("组帧PPS时超出缓冲[%d]\n", m_nFrameSize + nBufferSize);
		//	return;
		//}
		memcpy(m_szFrameData + m_nFrameSize, pBuffer, nBufferSize);
		m_nFrameSize += nBufferSize;
	}
	else if ((BYTE)pBuffer[0] == 0 && (BYTE)pBuffer[1] == 0 && (BYTE)pBuffer[2] == 0 && (BYTE)pBuffer[3] == 1 && ((BYTE)pBuffer[4] & 0x7e) >> 1 == 19) //IDR
	{
		if (m_nFrameSize > 0)
		{
			if (m_file)
				fwrite(m_szFrameData, m_nFrameSize, 1, m_file);
#ifdef _HEVC2AVC_USE
			if (m_pCodec > 0)
				hevc2avc_input(m_pCodec, m_szFrameData, m_nFrameSize);
#else
			m_pStreamCb(0, DVR_DATA_STREAM, m_szFrameData, m_nFrameSize, m_pStreamUser);
#endif
			m_nFrameSize = 0;
		}
		//if (m_nFrameSize + nBufferSize >= FRAME_BUF_SIZE)
		//{
		//	printf("组帧IDR时超出缓冲[%d]\n", m_nFrameSize + nBufferSize);
		//	m_nFrameSize = 0;
		//}
		memcpy(m_szFrameData + m_nFrameSize, pBuffer, nBufferSize);
		m_nFrameSize += nBufferSize;
	}
	else if ((BYTE)pBuffer[0] == 0 && (BYTE)pBuffer[1] == 0 && (BYTE)pBuffer[2] == 0 && (BYTE)pBuffer[3] == 1 && ((BYTE)pBuffer[4] & 0x7e) >> 1 == 1) //PDR
	{
		if (m_nFrameSize > 0)
		{
			if (m_file)
				fwrite(m_szFrameData, m_nFrameSize, 1, m_file);
#ifdef _HEVC2AVC_USE
			if (m_pCodec > 0)
				hevc2avc_input(m_pCodec, m_szFrameData, m_nFrameSize);
#else
			m_pStreamCb(0, DVR_DATA_STREAM, m_szFrameData, m_nFrameSize, m_pStreamUser);
#endif
			m_nFrameSize = 0;
		}
		memcpy(m_szFrameData + m_nFrameSize, pBuffer, nBufferSize);
		m_nFrameSize += nBufferSize;
	}
	else if ((BYTE)pBuffer[0] == 0 && (BYTE)pBuffer[1] == 0 && (BYTE)pBuffer[2] == 0 && (BYTE)pBuffer[3] == 1 && ((BYTE)pBuffer[4] & 0x7e) >> 1 == 39) //SEI
	{
		//丢弃
	}
	else
	{
		if (m_nFrameSize + nBufferSize >= FRAME_BUF_SIZE)
		{
			printf("组帧PDR时超出缓冲[%d]\n", m_nFrameSize + nBufferSize);
			return;
		}
		memcpy(m_szFrameData + m_nFrameSize, pBuffer, nBufferSize);
		m_nFrameSize += nBufferSize;
	}
}

void PSStreamAnalyze::VideoCodingFrameCb(unsigned char * pBuf, long nSize, long tv_sec, long tv_usec, void * pUser)
{
	PSStreamAnalyze *pAnalyze = static_cast<PSStreamAnalyze*>(pUser);
	if (pAnalyze != nullptr && pAnalyze->m_pStreamCb != nullptr && pAnalyze->m_pStreamUser != nullptr)
	{
		pAnalyze->m_pStreamCb(0, DVR_DATA_STREAM, pBuf, nSize, pAnalyze->m_pStreamUser);
	}
}
