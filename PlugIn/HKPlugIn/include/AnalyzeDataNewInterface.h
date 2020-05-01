/*****************************************************
Copyright 2008-2011 Hikvision Digital Technology Co., Ltd.

FileName: AnalyzeDataNewInterface.h

Description: �����������½ӿ�

current version: 4.0.0.1
 	 
Modification History: 2010/5/27 ������
*****************************************************/

#ifndef _ANALYZEDATA_NEW_INTERFACE_H_
#define _ANALYZEDATA_NEW_INTERFACE_H_

#if defined(_WINDLL)
	#define ANALYZEDATA_API  extern "C" __declspec(dllexport) 
#else 
	#define ANALYZEDATA_API  extern "C" __declspec(dllimport) 
#endif

//���ݰ�����
#define FILE_HEAD			            0
#define VIDEO_I_FRAME		            1
#define VIDEO_B_FRAME		            2
#define VIDEO_P_FRAME		            3
#define AUDIO_PACKET		            10
#define PRIVT_PACKET                    11

//E֡���
#define HIK_H264_E_FRAME        	    (1 << 6)

//������		    
#define ERROR_NOOBJECT                  -1//��Ч���
#define ERROR_NO                        0//û�д���
#define ERROR_OVERBUF                   1//�������
#define ERROR_PARAMETER                 2//��������
#define ERROR_CALL_ORDER                3//����˳�����
#define ERROR_ALLOC_MEMORY              4//���뻺��ʧ��
#define ERROR_OPEN_FILE                 5//���ļ�ʧ��
#define ERROR_MEMORY                    11//�ڴ����
#define ERROR_SUPPORT                   12//��֧��
#define ERROR_UNKNOWN                   99//δ֪����

typedef struct _PACKET_INFO
{
	int		nPacketType;    /*������ 
						    0  - �ļ�ͷ
	                        1  -	I֡	
							2  -	B֡	
							3  -	P֡
							10 - ��Ƶ��
							11 -˽�а�*/

	char*	pPacketBuffer;//��ǰ֡���ݵĻ�������ַ
	DWORD	dwPacketSize;//���Ĵ�С
	
	int		nYear;	//ʱ�꣺��
	int		nMonth; //ʱ�꣺��
	int		nDay;//ʱ�꣺��
	int		nHour;//ʱ�꣺ʱ
	int		nMinute;//ʱ�꣺��
	int		nSecond;//ʱ�꣺��

	DWORD   dwTimeStamp;//ʱ���������

} PACKET_INFO;

typedef struct _PACKET_INFO_EX
{
	int		nPacketType;    /*������ 
						    0  - �ļ�ͷ
	                        1  -	I֡	
							2  -	B֡	
							3  -	P֡
							10 - ��Ƶ��
							11 -˽�а�*/

	char*	pPacketBuffer;//��ǰ֡���ݵĻ�������ַ
	DWORD	dwPacketSize;//���Ĵ�С
	
	int		nYear;	//ʱ�꣺��
	int		nMonth; //ʱ�꣺��
	int		nDay;//ʱ�꣺��
	int		nHour;//ʱ�꣺ʱ
	int		nMinute;//ʱ�꣺��
	int		nSecond;//ʱ�꣺��

	DWORD   dwTimeStamp;//ʱ�����32λ����λ����

	DWORD          dwFrameNum;//֡��
	DWORD          dwFrameRate;//֡��
	unsigned short uWidth;//���
	unsigned short uHeight;//�߶�
	DWORD		   dwTimeStampHigh;//ʱ�����32λ����λ����
	DWORD          dwFlag;//E֡��ǣ���E֡ dwFlag = 64,����Ϊ0
	DWORD          Reserved[5];//������

} PACKET_INFO_EX;

/***************************************************************************************
��  �ܣ� ��ʽ�򿪲���ȡ�������
���������DWORD dwSize --���÷��������С
          ��CIF����200KB; 
          ��4CIF����1MB
         PBYTE pHeader--ͷ����

�����������
����ֵ�� �������
****************************************************************************************/	
ANALYZEDATA_API HANDLE __stdcall HIKANA_CreateStreamEx(IN DWORD dwSize, IN PBYTE pHeader);
/***************************************************************************************
��  �ܣ� ���ٷ������
���������HANDLE hHandle -- �������
�����������
����ֵ�� ��
****************************************************************************************/
	
ANALYZEDATA_API void   __stdcall HIKANA_Destroy(IN HANDLE hHandle);
/***************************************************************************************
��  �ܣ��������������
���������HANDLE hHandle -�������
          PBYTE pBuffer-�������ݵ�ַ
		  DWORD dwSize-�������ݳ���
���������
����ֵ���ɹ����
****************************************************************************************/
	
ANALYZEDATA_API BOOL   __stdcall HIKANA_InputData(IN HANDLE hHandle, IN PBYTE pBuffer, IN DWORD dwSize);
/***************************************************************************************
��  �ܣ���SDK�������л�ȡPACKET_INFO������ʹ��AnalyzeDataInputData�������ݳɹ����뷴������HIKANA_GetOnePacketExֱ�����ط�0ֵ����ȷ����ȡ������������Ч���ݡ�
���������HANDLE hHandle -�������
���������PACKET_INFO* pstPacket -�����ⲿ�������ݵĽṹ
����ֵ���ɹ�����0�����򷵻ش�����
****************************************************************************************/

ANALYZEDATA_API int	   __stdcall HIKANA_GetOnePacketEx(IN HANDLE hHandle, OUT PACKET_INFO_EX* pstPacket);
/***************************************************************************************
��  �ܣ���ջ���
���������HANDLE hHandle -�������
�����������
����ֵ���ɹ����
****************************************************************************************/
ANALYZEDATA_API BOOL   __stdcall HIKANA_ClearBuffer(IN HANDLE hHandle);
/***************************************************************************************
��  �ܣ���ȡ���뻺���еĲ�������
���������HANDLE hHandle -�������
          PBYTE pData-�������ݵ�ַ
		  DWORD *pdwSize-�������ݴ�Сָ��
���������
����ֵ���ɹ����
****************************************************************************************/
ANALYZEDATA_API int	   __stdcall HIKANA_GetRemainData(IN HANDLE hHandle, IN PBYTE pData, OUT DWORD* pdwSize);
/***************************************************************************************
��  �ܣ���ȡ������
���������HANDLE hHandle -�������
�����������
����ֵ�����ش�����
****************************************************************************************/
ANALYZEDATA_API DWORD  __stdcall HIKANA_GetLastErrorH(IN HANDLE hHandle);

#endif