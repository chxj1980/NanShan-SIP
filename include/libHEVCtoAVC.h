#pragma once
#ifdef LIBHEVCTOAVC_EXPORTS
#define HEVCTOAVC extern "C" _declspec(dllexport)
#else
#define HEVCTOAVC extern "C" _declspec(dllimport)
#endif


#ifdef _WIN64
typedef LONGLONG Long;
#else
typedef long Long;
#endif

/*   H265 Converted to H264   */

#define CODEC_TYPE_H264		1
#define CODEC_TYPE_H265		2


//0.H264���ݻص�
typedef void(__stdcall* VideoCodingFrameCBFun)(unsigned char * pBuf, long nSize, long tv_sec, long tv_usec, void* pUser);

//1.��������ʼ��:�ɹ����ر�������� ʧ�ܷ���0 (ÿ��ת��ǰ����)
//
HEVCTOAVC Long hevc2avc_init(int codec_type=2, int src_width=0, int src_height=0, int dst_width=0, int dst_height=0, int dst_bitrate=0);

//2.����ת������ݻص�
HEVCTOAVC bool hevc2avc_setcb(Long codec, VideoCodingFrameCBFun cb, void *pUserData);

//3.��ʼת�루ÿ������������һ֡H265��Ƶ������
HEVCTOAVC bool hevc2avc_input(Long codec, unsigned char *inFrameBuff, unsigned int inFrameLen);

//4.�������ͷ�
HEVCTOAVC bool hevc2avc_uninit(Long codec);

