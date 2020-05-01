#pragma once

#define PS_HDR_LEN  14  
#define SYS_HDR_LEN 18  
#define PSM_HDR_LEN 24  
//#define PES_HDR_LEN 19  //有dts
#define PES_HDR_LEN 14 
#define RTP_HDR_LEN 12
#define RTP_VERSION	2

#define PS_STREAMID_VIDEO 0xE0
#define PS_STREAMID_AUDIO 0xC0

#define PS_STREAMTYPE_MPEG4 0x10
#define PS_STREAMTYPE_H264 0x1B
#define PS_STREAMTYPE_SVAC 0x80
#define PS_STREAMTYPE_H265 0x24
#define PS_STREAMTYPE_G711 0x90
#define PS_STREAMTYPE_G722 0x92
#define PS_STREAMTYPE_G723 0x93
#define PS_STREAMTYPE_G729 0x99
#define PS_STREAMTYPE_SVACA 0x9B

#define RTP_MAX_PACKET_SIZE 1400 //gb28181_send_rtp_pack
#define PS_PES_PAYLOAD_SIZE 5106 //gb28181_streampackageForH264

#define RTP_VERSION 2  //RTP协议的版本号 当前为2

typedef struct bits_buffer
{
	int i_size;
	int i_data;
	unsigned char i_mask;
	unsigned char *p_data;

}bits_buffer_t;

typedef struct frame_info
{
	int i_streamid;					//码流类型
	int i_frame;					//是否关键帧
	int  i_framelen;				//帧长度
	unsigned char *p_framedata;		//帧数据,需要用户分配空间
	unsigned long i_pts;			//时间戳
}frame_info_t;

class PSPackage
{
public:
	PSPackage(void);
public:
	~PSPackage(void);
public:
	unsigned char *RTPFormat(unsigned char *pData, int playload, int marker, unsigned short cseq, unsigned long curpts, unsigned int ssrc);
	unsigned int Packing(unsigned char *pOutBuffer, unsigned char* pInBuffer, int nInSize, int nStreamID, int nStreamType, bool bIFrame, unsigned long nTimestamp);
};
