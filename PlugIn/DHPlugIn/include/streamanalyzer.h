/*************************************************************************
**                           streamAnalyzer 

**      (c) Copyright 1992-2004, ZheJiang Dahua Technology Stock Co.Ltd.
**                         All Rights Reserved
**
**	File  Name		: StreamAnalyzer.h
**	Description		: streamAnalyzer header
**	Modification	: 2010/07/13	yeym	Create the file
**************************************************************************/
#ifndef __STREAM_ANALYZER_H
#define __STREAM_ANALYZER_H
#define INOUT
#define IN
#define OUT


#ifndef _WIN32
#include <unistd.h>
#include <time.h>
#define __stdcall
#else
#include <windows.h>
#endif

#ifndef NULL
#define NULL
#endif

#ifdef SUPPORT_TYPE_DEFINE
#include "platform.h"
#include "platformsdk.h"
#else

#if !defined(uint8)
typedef unsigned char       uint8;
#endif
#if !defined(uint16)
typedef unsigned short     uint16;
#endif
#if !defined(uint32)
typedef unsigned int        uint32;
#endif
#if !defined(int32)
typedef int                 int32;
#endif
#endif



#ifdef WIN32
#ifdef STREAMANALYZER_EXPORTS
#define ANALYZER_API __declspec(dllexport)
#else
#define ANALYZER_API __declspec(dllimport)
#endif
#else
#define ANALYZER_API
#endif

#ifndef NOERROR
#define NOERROR	0
#endif

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////
// ֡����
typedef enum _ANA_MEDIA_TYPE
{
	FRAME_TYPE_UNKNOWN = 0,			//֡���Ͳ���֪
	FRAME_TYPE_VIDEO,				//֡��������Ƶ֡
	FRAME_TYPE_AUDIO,				//֡��������Ƶ֡
	FRAME_TYPE_DATA					//֡����������֡
}ANA_MEDIA_TYPE;

// ������
typedef enum _FRAME_SUB_TYPE
{
	TYPE_DATA_INVALID = -1,				//������Ч
	TYPE_VIDEO_I_FRAME = 0 ,			// I֡
	TYPE_VIDEO_P_FRAME,				// P֡
	TYPE_VIDEO_B_FRAME,				// B֡
	TYPE_VIDEO_S_FRAME,				// S֡
	TYPE_WATERMARK_TEXT,			//ˮӡ����ΪTEXT����
	TYPE_WATERMARK_JPEG,			//ˮӡ����ΪJPEG����
	TYPE_WATERMARK_BMP,				//ˮӡ����ΪBMP����
	TYPE_DATA_INTL,					//���ܷ���֡
	TYPE_VIDEO_JPEG_FRAME,
	TYPE_DATA_ITS,				//its��Ϣ֡
	TYPE_DATA_GPS,
	TYPE_DATA_INTLEX,
    TYPE_DATA_RAW = 255
}FRAME_SUB_TYPE;						

// ��������
typedef enum _ENCODE_TYPE
{
	ENCODE_VIDEO_UNKNOWN = 0,		//��Ƶ�����ʽ����֪
	ENCODE_VIDEO_MPEG4 ,			//��Ƶ�����ʽ��MPEG4
	ENCODE_VIDEO_HI_H264,			//��Ƶ�����ʽ�Ǻ�˼H264
	ENCODE_VIDEO_JPEG,				//��Ƶ�����ʽ�Ǳ�׼JPEG
	ENCODE_VIDEO_DH_H264,			//��Ƶ�����ʽ�Ǵ�����H264
	ENCODE_VIDEO_INVALID,			//��Ƶ�����ʽ��Ч

	ENCODE_AUDIO_PCM = 7,			//��Ƶ�����ʽ��PCM8
	ENCODE_AUDIO_G729,				//��Ƶ�����ʽ��G729
	ENCODE_AUDIO_IMA,				//��Ƶ�����ʽ��IMA
	ENCODE_PCM_MULAW,				//��Ƶ�����ʽ��PCM MULAW
	ENCODE_AUDIO_G721,				//��Ƶ�����ʽ��G721
	ENCODE_PCM8_VWIS,				//��Ƶ�����ʽ��PCM8_VWIS
	ENCODE_MS_ADPCM,				//��Ƶ�����ʽ��MS_ADPCM
	ENCODE_AUDIO_G711A,				//��Ƶ�����ʽ��G711A
	ENCODE_AUDIO_AMR,				//��Ƶ�����ʽ��AMR
	ENCODE_AUDIO_PCM16,				//��Ƶ�����ʽ��PCM16
	ENCODE_AUDIO_G711U = 22,		//��Ƶ�����ʽ��G711U
	ENCODE_AUDIO_G723,				//��Ƶ�����ʽ��G723
	ENCODE_AUDIO_AAC = 26,			//��Ƶ�����ʽ��AAC
	ENCODE_AUDIO_TALK = 30,
	ENCODE_VIDEO_H263,
	ENCODE_VIDEO_PACKET
}ENCODE_TYPE;
		
enum
{
	DH_ERROR_NOFIND_HEADER = -20, 	//������Ϣ����,��û��֡ͷ
	DH_ERROR_FILE, 					//�ļ�����ʧ��
	DH_ERROR_MM, 					//�ڴ����ʧ��
	DH_ERROR_NOOBJECT, 				//��������Ӧ�Ķ���
	DH_ERROR_ORDER, 				//�ӿڵ��ô�����ȷ
	DH_ERROR_TIMEOUT, 				//����ʱ
	DH_ERROR_EXPECPT_MODE, 			//�ӿڵ��ò���ȷ
	DH_ERROR_PARAMETER, 			//��������
	DH_ERROR_NOKNOWN, 				//����ԭ��δ��
	DH_ERROR_NOSUPPORT, 			//���ṩʵ��
	DH_ERROR_OVER,
	DH_ERROR_LOCKTIMEOUT,
	DH_NOERROR = NOERROR 			//û�д���
};


typedef enum _STREAM_TYPE
{
	DH_STREAM_UNKNOWN = 0,
		DH_STREAM_MPEG4,		
		DH_STREAM_DHPT =3,
		DH_STREAM_NEW,
		DH_STREAM_HB,
		DH_STREAM_AUDIO,
		DH_STREAM_PS,
	DH_STREAM_DHSTD,
	DH_STREAM_ASF,
	DH_STREAM_3GPP,
	DH_STREAM_RAW,	
	DH_STREAM_TS,
}STREAM_TYPE;

enum
{
	E_STREAM_NOERROR = 0,		//����У������
	E_STREAM_TIMESTAND,			//ʱ�����δʵ��
	E_STREAM_LENGTH,			//������Ϣ����
	E_STREAM_HEAD_VERIFY,		//δʵ��
	E_STREAM_VERIFY,			//����У��ʧ��
	E_STREAM_HEADER,			//����û��֡ͷ
	E_STREAM_NOKNOWN,			//����֪����δʵ��
	E_STREAM_LOSTFRAME,
	E_STREAM_WATERMARK,
	E_STREAM_CONTEXT,
	E_STREAM_NOSUPPORT,
	E_STREAM_BODY
};

enum
{
	DEINTERLACE_PAIR = 0,
	DEINTERLACE_SINGLE,
	DEINTERLACE_NONE,
};
////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct _DHTIME								
{
	uint32 second		:6;					//	��	1-60		
	uint32 minute		:6;					//	��	1-60		
	uint32 hour			:5;					//	ʱ	1-24		
	uint32 day			:5;					//	��	1-31		
	uint32 month		:4;					//	��	1-12		
	uint32 year			:6;					//	��	2000-2063	
}DHTIME,*pDHTIME;
// ֡��Ϣ
typedef struct __ANA_FRAME_INFO
{
	ANA_MEDIA_TYPE		nType;			// ֡����
	FRAME_SUB_TYPE	nSubType;			// ������
	STREAM_TYPE		nStreamType;		// ���ݴ��Э�����ͣ�DHAV��
	ENCODE_TYPE		nEncodeType;		// ��������

	uint8*	        pHeader;			// ����֡ͷ������ָ��
	uint32			nLength;			// ����֡ͷ�����ݳ���
	uint8*	        pFrameBody;			// ������ָ��
	uint32			nBodyLength;		// �����ݳ���
	uint32			dwFrameNum;			// ֡��ţ���֡�ж�

	uint32			nDeinterlace;		// �⽻��
	uint8			nFrameRate;			// ֡��
	uint8			nMediaFlag;			// �������ͱ�ǣ�h264������(0����������2����˼����)
	uint16			nWidth;				// �ֱ���
	uint16			nHeight;

	uint16			nSamplesPerSec;		// ������
	uint8			nBitsPerSample;		// λ��
	uint8			nChannels;

	// ʱ������
	uint16			nYear;	
	uint16			nMonth;
	uint16			nDay;
	uint16			nHour;
	uint16			nMinute;
	uint16			nSecond;
	uint32			dwTimeStamp;		// ʱ��� mktime������ֵ
	uint16			nMSecond;			// ����
	uint16			nReserved;
	uint32			Reserved[3];		//
	uint16			sReverved[2];		
	uint32			bValid;				// 
} ANA_FRAME_INFO;


typedef	struct  __ANA_INDEX_INFO
{
	uint32	filePos;						//�ؼ�֡���ļ��е�ƫ��
	uint32	dwFrameNum;						//�ؼ�֡֡��
	uint32 dwFrameLen;						//�ؼ�֡֡��
	uint32 frameRate;						//֡��
	uint32	frameTime;						//�ؼ�֡ʱ��
}ANA_INDEX_INFO;

//������Ϣ�ṹ
#define  MAX_TRACKPOINT_NUM 10

typedef struct _ANA_IVS_POINT
{
	uint16 		x; 
	uint16 		y; 
	uint16		xSize;
	uint16		ySize;
	//�켣����������Ӿ��ε����ģ�����X��Y��XSize��YSize�������������Ӿ������꣨left��top��right��bottom����
	//RECT=(X-XSize, Y-YSize, X+XSize, Y+YSize)
	
}ANA_IVS_POINT; 

typedef struct _ANA_IVS_OBJ
{
	uint32				obj_id;						// ����id
	uint32				enable;						 // 0 ��ʾɾ������  1��ʾ����켣��Ϣ��Ч
	ANA_IVS_POINT 	track_point[MAX_TRACKPOINT_NUM]; 
	uint32				trackpt_num;				// �����������track_point��Ч����
}ANA_IVS_OBJ;

typedef struct _ANA_IVS_PrePos
{
	uint32				nPresetCount;			//Ԥ�õ���Ϣ��������λ: 1�ֽ�
	uint8*				pPresetPos;				//Ԥ�õ���Ϣָ��
}ANA_IVS_PrePos;

typedef enum  _IVS_METHOD
{
	IVS_track,									//��������֡�����ƶ��켣��Ϣ
	IVS_Preset									//������̨Ԥ�õ���Ϣ
}IVS_METHOD;

enum CHECK_ERROR_LEVEL
{
	CHECK_NO_LEVEL = 0,
	CHECK_PART_LEVEL,
	CHECK_COMPLETE_LEVEL
};

enum
{
	SAMPLE_FREQ_4000 = 1,
	SAMPLE_FREQ_8000,
	SAMPLE_FREQ_11025,
	SAMPLE_FREQ_16000,
	SAMPLE_FREQ_20000,
	SAMPLE_FREQ_22050,
	SAMPLE_FREQ_32000,
	SAMPLE_FREQ_44100,
	SAMPLE_FREQ_48000
};

typedef	void*		    ANA_HANDLE;
typedef ANA_HANDLE*		PANA_HANDLE;

/************************************************************************
 ** �ӿڶ���
 ***********************************************************************/

//------------------------------------------------------------------------
// ����: ANA_CreateStream
// ����: ��������������
// ����: dwSize��Ϊ�ڲ��������ĳ��ȣ����Ϊ0���ڲ�����Ӧ��pHandle�����ص��������������
// ����:<0 ��ʾʧ�ܣ�==0 ��ʾ�ɹ���
//------------------------------------------------------------------------
ANALYZER_API int32	__stdcall ANA_CreateStream(IN int dwSize,OUT PANA_HANDLE pHandle);

//------------------------------------------------------------------------
// ����: ANA_Destroy
// ����: ��������������
// ����: hHandle��ͨ��ANA_CreateStream ����ANA_CreateFile���صľ����
// ����:��
//------------------------------------------------------------------------
ANALYZER_API void	__stdcall ANA_Destroy(IN ANA_HANDLE hHandle);

//------------------------------------------------------------------------
// ����: ANA_InputData
// ����: ����������
// ����: hHandle��ͨ��ANA_CreateStream���صľ����pBuffer����������ַ��dwSize�����������ȡ�
// ����:>=0��ʾ�ɹ���DH_ERROR_NOFIND_HEADER �������Ӧ�ÿ��Ժ��ԡ�
//------------------------------------------------------------------------
ANALYZER_API int32	__stdcall ANA_InputData(IN ANA_HANDLE hHandle, IN uint8* pBuffer, IN int dwSize);

//------------------------------------------------------------------------
// ����: ANA_GetNextFrame
// ����: ������������ó���Ƶ֮֡������֡��Ϣ����䵽pstFrameInfo����
// ����: hHandle��ͨ��ANA_CreateStream����ANA_CreateFile���صľ����pstFrameInfo �ⲿANA_FRAME_INFO��һ���ṹ��ַ��
// ����:(-1��ʧ�ܣ�0���ɹ���1���������ѿ�)
//------------------------------------------------------------------------
ANALYZER_API int32	__stdcall ANA_GetNextFrame(IN ANA_HANDLE hHandle, OUT ANA_FRAME_INFO* pstFrameInfo);

// ��ȡ�����������л���ʣ������ hHandle��ANA_CreateStream���صľ�� pData���ⲿ������ pSize�����pData==NULL�ͷ����ڲ��������ĳ��ȣ�pSize���ڴ�ֵΪ
//------------------------------------------------------------------------
// ����: ANA_ClearBuffer
// ����: ��������������ڲ��Ļ��塣
// ����: hHandle��ͨ��ANA_CreateStream���صľ��
// ����:>=0��ʾ�ɹ�
//------------------------------------------------------------------------
ANALYZER_API int	__stdcall ANA_ClearBuffer(IN ANA_HANDLE hHandle);
//------------------------------------------------------------------------
// ����: ANA_GetRemainData
// ����: ��ȡ�����������л���ʣ������
// ����: hHandle��ͨ��ANA_CreateStream���صľ����pData���ⲿ������ pSize�����pData==NULL�ͷ����ڲ��������ĳ��ȣ�pSize���ڴ�ֵΪ���س���pData�ĳ��ȡ�
// ����:>=0��ʾ�ɹ���
//------------------------------------------------------------------------
ANALYZER_API int32	__stdcall ANA_GetRemainData(IN ANA_HANDLE hHandle, IN uint8* pData, INOUT int* pSize);

//------------------------------------------------------------------------
// ����: ANA_GetLastError
// ����: ������������������
// ����: hHandle��ͨ��ANA_CreateStream���صľ����
// ����: ����ֵ��
//------------------------------------------------------------------------
ANALYZER_API int32	__stdcall ANA_GetLastError(IN ANA_HANDLE hHandle);

//------------------------------------------------------------------------
// ����: ANA_ParseIvsFrame
// ����: ����IVS����֡��
// ����: pstFrameInfo ����֡����Ϣ buffer������֡�Ļ����ַ��len ����֡�ĳ��ȡ�type�ж���Ѳ��������̨��
// ����: 0��ʾ�ɹ���
//------------------------------------------------------------------------
ANALYZER_API int32	__stdcall ANA_ParseIvsFrame(IN ANA_FRAME_INFO* pstFrameInfo,INOUT unsigned char* buffer,IN int len,IN IVS_METHOD type);
//------------------------------------------------------------------------
// ����: ANA_CreateFileIndexEx
// ����: ���ڴ����ļ�������
// ����: hHandle��ANA_CreateFile���صķ����������tv ������ʱ�����ƣ�Ĭ��Ϊ���޵ȴ���ֱ��������ɻ�ʧ�ܡ�flag ==0 ֻ������I֡��������ֵ���������е�֡��
// ����: >0����ʾ��������DH_ERROR_TIMEOUT Ӧ���ٴε���ANA_WaitForCreateIndexComplete��������ֵ��ʾʧ�ܡ�
//------------------------------------------------------------------------
ANALYZER_API int32	__stdcall ANA_CreateFileIndexEx(IN ANA_HANDLE hHandle,IN struct timeval* tv = NULL,IN int flag = 0);
//------------------------------------------------------------------------
// ����: ANA_WaitForCreateIndexComplete
// ����: ���ڵȴ��ļ��������ء�
// ����: hHandle��ANA_CreateFile���صķ����������
// ����: >0����ʾ��������������ֵ��ʾʧ�ܡ�
//------------------------------------------------------------------------
ANALYZER_API int32	__stdcall ANA_WaitForCreateIndexComplete(IN ANA_HANDLE hHandle);
//------------------------------------------------------------------------
// ����: ANA_CreateFile
// ����: �����ļ���������
// ����:filePath:ȫ·���ļ��� pHandle�����ص��������������
// ����:0 ��ʾ�ɹ���
//------------------------------------------------------------------------
ANALYZER_API int32	__stdcall ANA_CreateFile(const char* filePath,OUT PANA_HANDLE pHandle);

//------------------------------------------------------------------------
// ����: ANA_GetNextAudio
// ����: �������������л�ȡAUDIO֡���ݡ�
// ����:pHandle�����ص�������������� pstFrameInfo��֡�ṹ��Ϣ��ַ
// ����:-1��ʧ�ܣ�0���ɹ���1���������ѿ�
//------------------------------------------------------------------------
ANALYZER_API int32	__stdcall ANA_GetNextAudio(IN ANA_HANDLE hHandle, OUT ANA_FRAME_INFO* pstFrameInfo);

//------------------------------------------------------------------------
// ����: ANA_ParseFile
// ����: ��ʼ�����ļ���
// ����:handle��ͨ������ANA_CreateFile���ص��������������
// ����:<0��ʧ�ܣ�>=0 ��ʾ�ɹ���
//------------------------------------------------------------------------
ANALYZER_API int32	__stdcall ANA_ParseFile(IN ANA_HANDLE handle);
//------------------------------------------------------------------------
// ����: ANA_GetIndexByOffset
// ����: ��ʼ��ȡ������Ϣ
// ����:handle��ͨ������ANA_CreateFile���ص����������������offset ƫ��λ�ã���0��ʼ��pIndex �ⲿ�������ṹ��ַ��
// ����:<0��ʧ�ܣ�>=0 ��ʾ�ɹ���
//------------------------------------------------------------------------
ANALYZER_API int32	__stdcall ANA_GetIndexByOffset(IN ANA_HANDLE handle,IN int offset,OUT ANA_INDEX_INFO* pIndex);
//------------------------------------------------------------------------
// ����: ANA_GetMediaFrame
// ����: �������������л�ȡ���е�֡���ݡ�
// ����:pHandle�����ص�������������� pstFrameInfo��֡�ṹ��Ϣ��ַ
// ����:-1��ʧ�ܣ�0���ɹ���1���������ѿ�
//------------------------------------------------------------------------
ANALYZER_API int32	__stdcall ANA_GetMediaFrame(IN ANA_HANDLE hHandle, OUT ANA_FRAME_INFO* pstFrameInfo);
//------------------------------------------------------------------------
// ����: ANA_Reset
// ����: �������������⡣
// ����:pHandle��ANA_CreateStream���ص�������������� nLevel 1��ʾ�����ڲ����壬0��ʾ���¿�ʼ�������������������ڲ����塣
// ����:0���ɹ�
//------------------------------------------------------------------------
ANALYZER_API int32	__stdcall ANA_Reset(IN ANA_HANDLE hHandle,IN int nLevel);

//------------------------------------------------------------------------
// ����: ANA_EnableError
// ����: ������������������Դ���֡�ļ�����ȣ������ã�Ĭ����CHECK_PART_LEVEL��
// ����:pHandle��ANA_CreateStream���ص�������������� nEnableLevel��CHECK_ERROR_LEVEL �������е�һ��ֵ
// ����:0���ɹ�
//------------------------------------------------------------------------
ANALYZER_API int32	__stdcall ANA_EnableError(IN ANA_HANDLE hHandle,IN int nEnableLevel);


#ifdef __cplusplus
}
#endif


#endif // __STREAM_ANALYZER_H



