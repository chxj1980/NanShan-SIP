#include "StdAfx.h"
#include "PSPackage.h"

//I帧:PS + SYS + PSM + PES
//P帧:PS + PES

PSPackage::PSPackage(void)
{
}

PSPackage::~PSPackage(void)
{
}

void bits_write(bits_buffer_t *p_buffer,int i_count,unsigned long i_bits)
{
	while( i_count > 0 )
	{
		i_count--;

		if( ( i_bits >> i_count )&0x01 )
		{
			p_buffer->p_data[p_buffer->i_data] |= p_buffer->i_mask;
		}
		else
		{
			p_buffer->p_data[p_buffer->i_data] &= ~p_buffer->i_mask;
		}
		p_buffer->i_mask >>= 1;
		if( p_buffer->i_mask == 0 )
		{
			p_buffer->i_data++;
			p_buffer->i_mask = 0x80;
		}
	}
}

int find_nal_unit(unsigned char *buf, int size, int* nal_start, int* nal_end)
{
	int i = 0;
	// find start
	*nal_start = 0;
	*nal_end = 0;

	while (buf[i] != 0 || buf[i+1] != 0 || buf[i+2] != 0 || buf[i+3] != 0x01)
	{
		i++; // skip leading zero
		if (i + 4 >= size) { return 0; } // did not find nal start
	}
	*nal_start = i;
	i += 4;
	//( next_bits( 24 ) != 0x000000 && next_bits( 24 ) != 0x000001 )
	while ((buf[i] != 0 || buf[i+1] != 0 || buf[i+2] != 0 || buf[i+3] != 0x01))
	{
		i++;
		if (i - *nal_start > 1024) { *nal_end = size; return 0; }
		// FIXME the next line fails when reading a nal that ends exactly at the end of the data
		if (i + 4 >= size) { *nal_end = size; return 0; } // did not find nal end, stream ended first
	}
	*nal_end = i;
	return (*nal_end - *nal_start);
}

int make_ps_header(unsigned char *p_data, unsigned long pts)
{
	//unsigned long lScrExt = 0;			//DTS 
	unsigned long s64Scr = pts;		//PTS 时间戳,每帧3600递增
	unsigned long lScrExt = (s64Scr) % 100;

	bits_buffer_t   bitsBuffer;
	bitsBuffer.i_size = PS_HDR_LEN;
	bitsBuffer.i_data = 0;  
	bitsBuffer.i_mask = 0x80; // 二进制：10000000 这里是为了后面对一个字节的每一位进行操作，避免大小端夸字节字序错乱  
	bitsBuffer.p_data = (unsigned char *)p_data;
	memset(bitsBuffer.p_data, 0, PS_HDR_LEN);  
	bits_write(&bitsBuffer, 32, 0x000001BA);			/*start codes*/  
	bits_write(&bitsBuffer, 2,  1);						/*marker bits '01b'*/  
	bits_write(&bitsBuffer, 3,  (s64Scr>>30)&0x07);		/*System clock [32..30]*/  
	bits_write(&bitsBuffer, 1,  1);						/*marker bit*/  
	bits_write(&bitsBuffer, 15, (s64Scr>>15)&0x7FFF);   /*System clock [29..15]*/  
	bits_write(&bitsBuffer, 1,  1);						/*marker bit*/  
	bits_write(&bitsBuffer, 15, s64Scr&0x7fff);         /*System clock [14..0]*/ 
	bits_write(&bitsBuffer, 1,  1);						/*marker bit*/
	bits_write(&bitsBuffer, 9, lScrExt & 0x01ff);       /*System clock extension */
	bits_write(&bitsBuffer, 1,  1);						/*marker bit*/  
	bits_write(&bitsBuffer, 22, (255)&0x3fffff);		/*bit rate(n units of 50 bytes per second.)*/  
	bits_write(&bitsBuffer, 2,  3);						/*marker bits '11'*/  
	bits_write(&bitsBuffer, 5,  0x1f);					/*reserved(reserved for future use)*/  
	bits_write(&bitsBuffer, 3,  0);						/*stuffing length*/  
	return PS_HDR_LEN;
}

int make_sys_header(unsigned char *p_data)
{
	bits_buffer_t   bitsBuffer;  
	bitsBuffer.i_size = SYS_HDR_LEN;  
	bitsBuffer.i_data = 0;  
	bitsBuffer.i_mask = 0x80;  
	bitsBuffer.p_data = (unsigned char *)p_data;
	memset(bitsBuffer.p_data, 0, SYS_HDR_LEN);  
	/*system header*/  
	bits_write( &bitsBuffer, 32, 0x000001BB);   /*start code*/  
	bits_write( &bitsBuffer, 16, SYS_HDR_LEN-6);/*header_length 表示次字节后面的长度，后面的相关头也是次意思*/  
	bits_write( &bitsBuffer, 1,  1);            /*marker_bit*/  
	bits_write( &bitsBuffer, 22, 50000);        /*rate_bound*/  
	bits_write( &bitsBuffer, 1,  1);            /*marker_bit*/  
	bits_write( &bitsBuffer, 6,  1);            /*audio_bound*/  
	bits_write( &bitsBuffer, 1,  0);            /*fixed_flag */  
	bits_write( &bitsBuffer, 1,  1);            /*CSPS_flag */  
	bits_write( &bitsBuffer, 1,  1);            /*system_audio_lock_flag*/  
	bits_write( &bitsBuffer, 1,  1);            /*system_video_lock_flag*/  
	bits_write( &bitsBuffer, 1,  1);            /*marker_bit*/  
	bits_write( &bitsBuffer, 5,  1);            /*video_bound*/  
	bits_write( &bitsBuffer, 1,  0);            /*dif from mpeg1*/  
	bits_write( &bitsBuffer, 7,  0x7F);         /*reserver*/  
	/*audio stream bound*/  
	bits_write( &bitsBuffer, 8,  0xC0);         /*stream_id*/  
	bits_write( &bitsBuffer, 2,  3);            /*marker_bit */  
	bits_write( &bitsBuffer, 1,  0);            /*PSTD_buffer_bound_scale*/  
	bits_write( &bitsBuffer, 13, 512);          /*PSTD_buffer_size_bound*/  
	/*video stream bound*/  
	bits_write( &bitsBuffer, 8,  0xE0);         /*stream_id*/  
	bits_write( &bitsBuffer, 2,  3);            /*marker_bit */  
	bits_write( &bitsBuffer, 1,  1);            /*PSTD_buffer_bound_scale*/  
	bits_write( &bitsBuffer, 13, 2048);         /*PSTD_buffer_size_bound*/  
	return SYS_HDR_LEN;  
}

int make_psm_header(unsigned char *pData, unsigned char stream_id, unsigned char stream_type)
{
	bits_buffer_t   bitsBuffer;  
	bitsBuffer.i_size = PSM_HDR_LEN;   
	bitsBuffer.i_data = 0;  
	bitsBuffer.i_mask = 0x80;  
	bitsBuffer.p_data = pData;
	memset(bitsBuffer.p_data, 0, PSM_HDR_LEN);  
	bits_write(&bitsBuffer, 24,0x000001);   /*start code*/  
	bits_write(&bitsBuffer, 8, 0xBC);       /*map stream id*/  
	bits_write(&bitsBuffer, 16,18);         /*program stream map length*/   
	bits_write(&bitsBuffer, 1, 1);          /*current next indicator */  
	bits_write(&bitsBuffer, 2, 3);          /*reserved*/  
	bits_write(&bitsBuffer, 5, 0);          /*program stream map version*/  
	bits_write(&bitsBuffer, 7, 0x7F);       /*reserved */  
	bits_write(&bitsBuffer, 1, 1);          /*marker bit */  
	bits_write(&bitsBuffer, 16,0);          /*programe stream info length*/  
	bits_write(&bitsBuffer, 16, 8);         /*elementary stream map length  is*/  
	if (stream_id == PS_STREAMID_AUDIO) /*audio*/
	{
		bits_write(&bitsBuffer, 8, stream_type);       /*stream_type*/
		bits_write(&bitsBuffer, 8, 0xC0);       /*elementary_stream_id*/  
	}
	else /*video*/
	{
		bits_write(&bitsBuffer, 8, stream_type);       /*stream_type*/
		bits_write(&bitsBuffer, 8, 0xE0);       /*elementary_stream_id*/
	}
	bits_write(&bitsBuffer, 16, 0);         /*elementary_stream_info_length */  
	/*crc (2e b9 0f 3d)*/  
	bits_write(&bitsBuffer, 8, 0x45);       /*crc (24~31) bits*/  
	bits_write(&bitsBuffer, 8, 0xBD);       /*crc (16~23) bits*/  
	bits_write(&bitsBuffer, 8, 0xDC);       /*crc (8~15) bits*/  
	bits_write(&bitsBuffer, 8, 0xF4);       /*crc (0~7) bits*/  
	return PSM_HDR_LEN;  
}

//视频流ID:0xE0 时钟频率:90000
//音频流ID:0XC0 时钟频率:8000
int make_pes_header(unsigned char *pData, int stream_id, int payload_len, unsigned long pts)
{
	//unsigned long dts = pts / 3;
	bits_buffer_t   bitsBuffer;  
	bitsBuffer.i_size = PES_HDR_LEN;  
	bitsBuffer.i_data = 0;  
	bitsBuffer.i_mask = 0x80;  
	bitsBuffer.p_data = pData;
	memset(bitsBuffer.p_data, 0, PES_HDR_LEN);
	payload_len = payload_len + PES_HDR_LEN - 6;
	/*system header*/  
	bits_write( &bitsBuffer, 24,0x000001);			/*start code*/  
	bits_write( &bitsBuffer, 8, (stream_id));		/*streamID*/
	bits_write( &bitsBuffer, 16,(payload_len));		/*packet_len*/ //该字节后的长度加帧数据长度
	bits_write( &bitsBuffer, 2, 2 );				/*'10'*/  
	bits_write( &bitsBuffer, 2, 0 );				/*scrambling_control*/  
	bits_write( &bitsBuffer, 1, 0 );				/*priority*/  
	bits_write( &bitsBuffer, 1, 0 );				/*data_alignment_indicator*/  
	bits_write( &bitsBuffer, 1, 0 );				/*copyright*/  
	bits_write( &bitsBuffer, 1, 0 );				/*original_or_copy*/  
	bits_write( &bitsBuffer, 1, 1 );				/*PTS_flag*/  
	bits_write( &bitsBuffer, 1, 0 );				/*DTS_flag*/ 
	//bits_write( &bitsBuffer, 1, 1 );				/*DTS_flag*/ //有dts
	bits_write( &bitsBuffer, 1, 0 );				/*ESCR_flag*/  
	bits_write( &bitsBuffer, 1, 0 );				/*ES_rate_flag*/  
	bits_write( &bitsBuffer, 1, 0 );				/*DSM_trick_mode_flag*/  
	bits_write( &bitsBuffer, 1, 0 );				/*additional_copy_info_flag*/  
	bits_write( &bitsBuffer, 1, 0 );				/*PES_CRC_flag*/  
	bits_write( &bitsBuffer, 1, 0 );				/*PES_extension_flag*/  
	bits_write( &bitsBuffer, 8, 5 );				/*header_data_length*/   // 以下可选字段和填充字节所占用的总字节数
	//bits_write( &bitsBuffer, 8, 10);              /*header_data_length*/   //有dts

	/*PTS,DTS*/   
	bits_write( &bitsBuffer, 4, 2 );                    /*'0010'*/ 
	//bits_write( &bitsBuffer, 4, 3 );                    /*'0011'*/  
	bits_write( &bitsBuffer, 3, ((pts)>>30)&0x07 );     /*PTS[32..30]*/  
	bits_write( &bitsBuffer, 1, 1 );  
	bits_write( &bitsBuffer, 15,((pts)>>15)&0x7FFF);    /*PTS[29..15]*/  
	bits_write( &bitsBuffer, 1, 1 );  
	bits_write( &bitsBuffer, 15,(pts)&0x7FFF);          /*PTS[14..0]*/  
	bits_write( &bitsBuffer, 1, 1 );  
	//bits_write( &bitsBuffer, 4, 1 );                    /*'0001'*/  
	//bits_write( &bitsBuffer, 3, ((dts)>>30)&0x07 );     /*DTS[32..30]*/  
	//bits_write( &bitsBuffer, 1, 1 );  
	//bits_write( &bitsBuffer, 15,((dts)>>15)&0x7FFF);    /*DTS[29..15]*/  
	//bits_write( &bitsBuffer, 1, 1 );  
	//bits_write( &bitsBuffer, 15,(dts)&0x7FFF);          /*DTS[14..0]*/  
	//bits_write( &bitsBuffer, 1, 1 );  
	return PES_HDR_LEN;
}

unsigned char *PSPackage::RTPFormat(unsigned char *pData, int playload, int marker, unsigned short cseq, unsigned long curpts, unsigned int ssrc)
{
	bits_buffer_t   bitsBuffer;   
	bitsBuffer.i_size = RTP_HDR_LEN;  
	bitsBuffer.i_data = 0;  
	bitsBuffer.i_mask = 0x80;  
	bitsBuffer.p_data = pData;  
	memset(bitsBuffer.p_data, 0, RTP_HDR_LEN);  
	bits_write(&bitsBuffer, 2, RTP_VERSION);    /* rtp version  */  
	bits_write(&bitsBuffer, 1, 0);              /* rtp padding  */  
	bits_write(&bitsBuffer, 1, 0);              /* rtp extension  */  
	bits_write(&bitsBuffer, 4, 0);              /* rtp CSRC count */  
	bits_write(&bitsBuffer, 1, (marker));		/* rtp marker   */  
	bits_write(&bitsBuffer, 7, playload);		/* rtp payload type*/
	bits_write(&bitsBuffer, 16, (cseq));        /* rtp sequence      */  
	bits_write(&bitsBuffer, 32, (curpts));      /* rtp timestamp     */  
	bits_write(&bitsBuffer, 32, (ssrc));        /* rtp SSRC      */  
	return pData;
}

unsigned int PSPackage::Packing(unsigned char *pOutBuffer,unsigned char* pInBuffer,int nInSize,int nStreamID,int nStreamType,bool bIFrame,unsigned long nTimestamp)
{
	int nOutSize = 0;
	int nHeadLen = 0;
	
	nHeadLen = make_ps_header(pOutBuffer,nTimestamp);
	nOutSize += nHeadLen;
	if (bIFrame)
	{
		nHeadLen = make_sys_header(pOutBuffer + nOutSize);
		nOutSize += nHeadLen;
		nHeadLen = make_psm_header(pOutBuffer + nOutSize, (unsigned char)nStreamID, (unsigned char)nStreamType);
		nOutSize += nHeadLen;

		int nal_start, nal_end, nal_size;
		while (nal_size = find_nal_unit(pInBuffer, nInSize, &nal_start, &nal_end))
		{
			nHeadLen = make_pes_header(pOutBuffer+nOutSize, nStreamID, nal_size, nTimestamp);
			nOutSize += nHeadLen;
			memcpy(pOutBuffer + nOutSize, pInBuffer, nal_size);
			nOutSize += nal_size;
			pInBuffer += nal_size;
			nInSize -= nal_size;
		}
	}
	while (nInSize > 0)
	{
		//每次帧的长度不要超过ps_pes_paylad_size长(包括头)，过了就得分片进循环行发送
		// 添加pes头  
		if (nInSize > PS_PES_PAYLOAD_SIZE)
		{
			nHeadLen = make_pes_header(pOutBuffer+nOutSize,nStreamID, PS_PES_PAYLOAD_SIZE,nTimestamp);
			nOutSize += nHeadLen;
			memcpy(pOutBuffer+nOutSize, pInBuffer, PS_PES_PAYLOAD_SIZE);
			nOutSize += PS_PES_PAYLOAD_SIZE;
			pInBuffer += PS_PES_PAYLOAD_SIZE;
			nInSize -= PS_PES_PAYLOAD_SIZE;
		}
		else
		{
			nHeadLen = make_pes_header(pOutBuffer+nOutSize,nStreamID, nInSize,nTimestamp);
			nOutSize += nHeadLen;
			memcpy(pOutBuffer+nOutSize, pInBuffer, nInSize);
			nOutSize += nInSize;
			nInSize = 0;
		}
		
	}
	return nOutSize;
}

