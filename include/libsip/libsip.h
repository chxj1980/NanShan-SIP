#pragma once
#include "config.h"

#include <time.h>
#include <math.h>


/********ö������**************************************/

typedef enum sip_event_type		//��Ϣ�¼�����
{
	SIP_EVENT_UNKNOWN,		//δ֪�¼�
	SIP_REGISTER_REQUEST,	//ע������
	SIP_REGISTER_SUCCESS,   //ע��ɹ�
	SIP_REGISTER_FAILURE,   //ע��ʧ��
	SIP_INVITE_REQUEST,		//�㲥����
	SIP_INVITE_PROCEEDING,	//�㲥�ȴ�
	SIP_INVITE_SUCCESS,		//�㲥�ɹ�
	SIP_INVITE_FAILURE,		//�㲥ʧ��
	SIP_ACK_REQUEST,		//�㲥ȷ��	
	SIP_BYE_REQUEST,		//�ر�����
	SIP_BYE_SUCCESS,		//�رճɹ�
	SIP_BYE_FAILURE,		//�ر�ʧ��
	SIP_CANCEL_REQUEST,		//ȡ������
	SIP_CANCEL_SUCCESS,		//ȡ���ɹ�	
	SIP_CANCEL_FAILURE,		//ȡ��ʧ��
	SIP_INFO_REQUEST,		//��������
	SIP_INFO_SUCCESS,		//���Ƴɹ�
	SIP_INFO_FAILURE,		//����ʧ��
	SIP_SUBSCRIBE_REQUEST,	//��������
	SIP_SUBSCRIBE_SUCCESS,	//���ĳɹ�
	SIP_SUBSCRIBE_FAILURE,	//����ʧ��
	SIP_NOTIFY_EVENT,		//֪ͨ��Ϣ
	SIP_NOTIFY_SUCCESS,		//֪ͨ�ɹ�
	SIP_NOTIFY_FAILURE,		//֪ͨʧ��
	SIP_MESSAGE_EVENT,		//��Ϣ����
	SIP_MESSAGE_SUCCESS,	//��Ϣ�ɹ�
	SIP_MESSAGE_FAILURES,	//��Ϣʧ��
	SIP_OPTIONS_REQUEST,	//��ѯ����
	SIP_OPTIONS_RESPONSE	//��ѯ��Ӧ

}SIP_EVENT_TYPE;

typedef enum sip_message_type	//��Ϣ��������
{
	SIP_MESSAGE_UNKNOWN,
	SIP_CONTROL_PTZCMD,			//��̨����
	SIP_CONTROL_TELEBOOT,		//Զ������(�豸)
	SIP_CONTROL_RECORDCMD,		//�ֶ�¼��(�豸)
	SIP_CONTROL_GUARDCMD,		//����/����
	SIP_CONTROL_ALARMCMD,		//������λ
	SIP_CONTROL_IFAMECMD,		//ǿ��I֡
	SIP_CONTROL_DRAGZOOMIN,		//����Ŵ�
	SIP_CONTROL_DRAGZOOMOUT,	//������С
	SIP_CONTROL_HOMEPOSITION,	//��������
	SIP_CONTROL_CONFIG,			//���ÿ���
	SIP_RESPONSE_CONTROL,		//����Ӧ��
	SIP_QUERY_PRESETQUERY,		//Ԥ��λ��ѯ
	SIP_RESPONSE_PRESETQUERY,	//Ԥ��λ��Ӧ
	SIP_QUERY_DEVICESTATUS,		//�豸״̬��ѯ
	SIP_RESPONSE_DEVICESTATUS,	//�豸״̬��Ӧ
	SIP_QUERY_CATALOG,			//Ŀ¼��Ϣ��ѯ
	SIP_RESPONSE_CATALOG,		//Ŀ¼��Ϣ��Ӧ
	SIP_QUERY_DEVICEINFO,		//�豸��Ϣ��ѯ
	SIP_RESPONSE_DEVICEINFO,	//�豸��Ϣ��Ӧ
	SIP_QUERY_RECORDINFO,		//¼��Ŀ¼����
	SIP_RESPONSE_RECORDINFO,	//Ŀ¼������Ӧ
	SIP_QUERY_ALARM,			//������ѯ
	SIP_RESPONSE_QUERY_ALARM,	//������ѯ��Ӧ
	SIP_QUERY_CONFIGDOWNLOAD,	//�豸���ò�ѯ
	SIP_RESPONSE_CONFIGDOWNLOAD,//�豸���û�Ӧ
	SIP_QUERY_MOBILEPOSITION,	//��λ�����ѯ
	SIP_NOTIFY_MOBILEPOSITION,	//��λ�����Ӧ
	SIP_NOTIFY_KEEPALIVE,		//����֪ͨ
	SIP_NOTIFY_MEDIASTATUS,		//ý��������֪ͨ
	SIP_NOTIFY_ALARM,			//����֪ͨ
	SIP_RESPONSE_NOTIFY_ALARM,	//����֪ͨ��Ӧ

	SIP_NOTIFY_BROADCAST,		//�����㲥֪ͨ
	SIP_RESPONSE_BROADCAST		//�����㲥Ӧ��
	
}SIP_MESSAGE_TYPE;

typedef enum sip_invite_type	//�㲥����
{
	SIP_INVITE_PLAY,					//ʵʱ�㲥
	SIP_INVITE_PLAYBACK,				//��ʷ�㲥
	SIP_INVITE_DOWNLOAD,				//���ص㲥
	SIP_INVITE_PLAY_SDK,				//ʵʱ�㲥SDK
	SIP_INVITE_PLAYBACK_SDK,			//��ʷ�㲥SDK
	SIP_INVITE_DOWNLOAD_SDK,			//���ص㲥SDK
	SIP_INVITE_TALK,					//�����Խ�
	SIP_INVITE_TALK_SDK,				//�����Խ�SDK
	SIP_INVITE_MEDIA,					//ý��������
	SIP_INVITE_MEDIASERVER,				//ý��������
	
}SIP_INVITE_TYPE;

typedef enum sip_content_type	//��Ϣ������
{
	SIP_APPLICATION_UNKNOWN,
	SIP_APPLICATION_XML,	//XML
	SIP_APPLICATION_SDP,	//SDP
	SIP_APPLICATION_RTSP	//RTSP

}SIP_CONTENT_TYPE;

typedef enum sip_direction_type	//ý������������
{
	SIP_DIRECTION_UNKNOWN,
	SIP_RECVONLY,			//ֻ����
	SIP_SENDONLY,			//ֻ����
	SIP_SENDRECV			//���պͷ���

}SIP_DIRECTION_TYPE;

typedef enum sip_protocol_type
{
	SIP_TRANSFER_UDP,			//UDP����
	SIP_TRANSFER_TCP_ACTIVE,	//TCP��������
	SIP_TRANSFER_TCP_PASSIVE,	//TCP��������
	SIP_TRANSFER_SDK,			//SDK����
	SIP_TRANSFER_SDK_STANDARDING//SDK��׼������
}SIP_PROTOCOL_TYPE;

typedef enum sip_media_type
{
	SIP_MEDIA_VIDEO,		//��Ƶ��
	SIP_MEDIA_AUDIO			//��Ƶ��
}SIP_MEDIA_TYPE;

typedef enum sip_stream_type
{
	SIP_STREAM_GRG,			//
	SIP_STREAM_HIKVISION,	//
	SIP_STREAM_DAHUA,		//
	SIP_STREAM_HUAWEI,		//
	SIP_STREAM_G723,		//4
	SIP_STREAM_PCMA,		//8
	SIP_STREAM_G722,		//9
	SIP_STREAM_G729,		//18
	SIP_STREAM_SVACA,		//20
	SIP_STREAM_PS=96,		//96
	SIP_STREAM_MPEG4,		//97
	SIP_STREAM_H264,		//98
	SIP_STREAM_SVAC,		//99
	SIP_STREAM_H265=108		//108
	
}SIP_STREAM_TYPE;

typedef enum sip_encode_type
{
	SIP_F_MPEG4=1,
	SIP_F_H264,
	SIP_F_SVAC,
	SIP_F_3GP,
	SIP_F_H265
}SIP_ENCODE_TYPE;

typedef enum sip_resolution_type //�ֱ�������
{
	SIP_F_QCIF=1,
	SIP_F_CIF,
	SIP_F_4CIF,
	SIP_F_D1,
	SIP_F_720P,
	SIP_F_1080P,
	SIP_F_2K,
	SIP_F_4K
}SIP_RESOLUTION_TYPE;

typedef enum sip_bitrate_type
{
	SIP_F_CBR=1,
	SIP_F_VBR
}SIP_BITRATE_TYPE;

typedef enum sip_voice_type
{
	SIP_F_G711=1,
	SIP_F_G723,
	SIP_F_G729,
	SIP_F_G722
}SIP_VOICE_TYPE;

typedef enum sip_bitsize_type
{
	SIP_F_5_3KBPS=1,		//g723
	SIP_F_6_3KBPS,			//g723
	SIP_F_8KBPS,			//g729
	SIP_F_16KBPS,			//g722/g711
	SIP_F_24KBPS,			//g722
	SIP_F_32KBPS,			//g722
	SIP_F_48KBPS,			//g722
	SIP_F_64KBPS			//g711
}SIP_BITSIZE_TYPE;

typedef enum sip_samples_type
{
	SIP_F_8KHZ=1,			//g711/g723/g729
	SIP_F_14KHZ,			//g722
	SIP_F_16KHZ,			//g722
	SIP_F_32KHZ				//g722
}SIP_SAMPLES_TYPE;

typedef enum sip_info_type		//��������
{
	SIP_INFO_UNKNOWN,
	SIP_INFO_PLAY,			//�طſ���
	SIP_INFO_PAUSE,			//�ط���ͣ
	SIP_INFO_TEARDOWN		//����ֹͣ
}SIP_INFO_TYPE;

typedef enum sip_playback_info	
{
	SIP_PLAYBACK_TEARDOWN,
	SIP_PLAYBACK_PAUSE,
	SIP_PLAYBACK_RESUME,
	SIP_PLAYBACE_FAST,
	SIP_PLAYBACK_SLOW,
	SIP_PLAYBACK_NORMAL,
	SIP_PLAYBACK_SETTIME
}SIP_PLAYBACK_INFO;

/********��������**************************************/
typedef enum sip_alarm_method
{
	SIP_ALARM_ALL,			//ȫ��
	SIP_ALARM_PHONE,		//�绰���� 
	SIP_ALARM_DEVICE,       //�豸����
	SIP_ALARM_SMS,			//���ű���
	SIP_ALARM_GPS,			//GPS����
	SIP_ALARM_VIDEO,		//��Ƶ����
	SIP_ALARM_DEVBROKEN,	//�豸���ϱ���
	SIP_ALARM_OTHER,		//��������
	SIP_ALARM_PEOPLE,		//���˱���
	SIP_ALARM_VEHICLE		//��������
}SIP_ARARM_METHOD;

typedef enum sip_alarm_device_type	//�豸��������
{
	SIP_ALARM_LOSE = 1,		//�豸��ʧ
	SIP_ALARM_DISASSEMBLY,	//���𱨾�
	SIP_ALARM_DISKFULL,		//������
	SIP_ALARM_HIGHTTEMP,	//�豸����
	SIP_ALARM_LOWTEMP		//�豸����
	
}SIP_ALARM_DEVICE_TYPE;

typedef enum sip_alarm_video_type	//��Ƶ��������
{
	SIP_ALARM_BLOCK,		//��Ƶ�ڵ�
	SIP_ALARM_MANUAL,		//�˹�����
	SIP_ALARM_MOVING,		//�ƶ����
	SIP_ALARM_LEAVEOVER,	//������
	SIP_ALARM_REMOVAL,		//�����Ƴ�
	SIP_ALARM_TRIPPINGLINE, //Խ�߼��
	SIP_ALARM_INVASION,		//���ּ��
	SIP_ALARM_RETROGRADE,	//���м��
	SIP_ALARM_WANDERING,	//�ǻ����
	SIP_ALARM_FLOWCOUNT,	//����ͳ��
	SIP_ALARM_CROWDED,		//��Ա�ܶ�
	SIP_ALARM_ANOMALY,		//��Ƶ�쳣
	SIP_ALARM_FASTMOVING	//�����ƶ�

}SIP_ALARM_VIDEO_TYPE;

typedef enum sip_alarm_devbroken_type
{
	SIP_ALARM_DISKBROKEN = 1, //���̹���
	SIP_ALARM_FANBROKEN		  //���ȹ���

}SIP_ALARM_DEVBORKEN_TYPE;

typedef enum sip_alarm_people_type
{
	SIP_ALARM_FACE,				//�������
	SIP_ALARM_HUMANBODY			//������

}SIP_ALARM_PEOPLE_TYPE;

typedef enum sip_alarm_vehicle_type
{
	SIP_ALARM_CAR,
	SIP_ALARM_NOMOTO

}SIP_ALARM_VEHICLE_TYPE;

typedef enum sip_alarm_invasion_event
{
	SIP_ALARM_EVENT_ENTERAREA = 1, //��������
	SIP_ALARM_EVENT_LEAVEAREA	   //�뿪����

}SIP_ALARM_INVASION_EVENT;

typedef enum sip_record_type
{
	SIP_RECORDTYPE_ALL,				//ȫ��¼��
	SIP_RECORDTYPE_TIME,			//��ʱ¼��
	SIP_RECORDTYPE_MOVE,			//���¼��
	SIP_RECORDTYPE_ALARM,			//�澯¼��
	SIP_RECORDTYPE_MANUAL			//�ֶ�¼��

}SIP_RECORD_TYPE;

typedef struct sip_sdp_info
{
	char o_user[21];				//ý�������
	char o_ip[16];					//ý�����IP
	int s_type;						//�㲥����
	int	m_type;						//ý������
	int	m_port;						//ý�����˿�
	int m_protocol;					//����Э��
	int	a_direct;					//���䷽��
	int	a_playload;					//ý�����
	int a_downloadspeed;			//�����ٶ�
	size_t t_begintime;				//��ʼʱ��
	size_t t_endtime;				//����ʱ��
	size_t a_filesize;				//�ļ���С
	char a_streamtype[11];			//��������
	char y_ssrc[11];				//ý������ʶ
	char u_uri[128];				//�ļ�·��
	char f_parm[128];				//�������

}SIP_SDP_INFO;

typedef struct sip_ptz_rect
{
	unsigned short left;			
	unsigned short top;
	unsigned short right;
	unsigned short bottom;
	unsigned short width;
	unsigned short height;
}SIP_PTZ_RECT;

#define SIP_USER_AGENT			"XINYI"

#define SIP_CSEQ_KEEPALIVE		3

#define SIP_RAND_LEN			64
#define SIP_HEADER_LEN			128
#define SIP_AUTH_LEN			256
#define SIP_BODY_LEN			8192
#define SIP_MSG_LEN				10240

/********ָ����**************************************/
#define SIP_PTZCMD_NONE				0xFF
#define SIP_PTZCMD_STOP				0x00		//ֹͣ
#define SIP_PTZCMD_UP				0x08		//����
#define SIP_PTZCMD_DOWN				0x04		//����
#define SIP_PTZCMD_LEFT				0x02		//����
#define SIP_PTZCMD_RIGHT			0x01		//����
#define SIP_PTZCMD_LU				0x0A		//����
#define SIP_PTZCMD_LD				0x06		//����
#define SIP_PTZCMD_RU				0x09		//����
#define SIP_PTZCMD_RD				0x05		//����
#define SIP_PTZCMD_ZOOM_IN			0x10		//�Ŵ�
#define SIP_PTZCMD_ZOOM_OUT			0x20		//��С

#define SIP_PTZCMD_FI_IRIS_IN		0x48		//��Ȧ�Ŵ�
#define SIP_PTZCMD_FI_IRIS_OUT		0x44		//��Ȧ��С
#define SIP_PTZCMD_FI_FOCUS_IN		0x42		//�۽�����
#define SIP_PTZCMD_FI_FOCUS_OUT		0x41		//�ۼ���Զ
#define SIP_PTZCMD_FI_STOP			0x40		//����ֹͣ

#define SIP_PTZCMD_SET				0x81		//Ԥ��λ����
#define SIP_PTZCMD_GOTO				0x82		//Ԥ��λ��ת
#define SIP_PTZCMD_DELETE			0x83		//Ԥ��λɾ��
#define SIP_PTZCMD_CRUISE_ADD		0x84		//Ѳ�������
#define	SIP_PTZCMD_CRUISE_DEL		0x85		//Ѳ����ɾ��
#define SIP_PTZCMD_CRUISE_SPEED		0x86		//Ѳ���ٶ�
#define SIP_PTZCMD_CRUISE_DTIME		0x87		//Ѳ��ͣ��ʱ��
#define SIP_PTZCMD_CRUISE_RUN		0x88		//Ѳ����ʼ

#define SIP_PTZCMD_AUTO_RUN			0x89		//�Զ�ɨ�迪ʼ
#define SIP_PTZCMD_AUTO_LBORDER		0x89		//�Զ�ɨ����߽�
#define SIP_PTZCMD_AUTO_RBORDER		0x89		//�Զ�ɨ���ұ߽�
#define SIP_PTZCMD_AUTO_SPEED		0x8A		//�Զ�ɨ���ٶ�

#define SIP_PTZCMD_AUXIOPEN			0x8C		//�������ش�(��ˢ)
#define SIP_PTZCMD_AUXICLOSE		0x8D		//�������عر�

#define SIP_PTZCMD_DRAGZOOM_IN		0x76
#define SIP_PTZCMD_DRAGZOOM_OUT		0X77
#define SIP_PTZCMD_INITIAL			0x78
#define SIP_PTZCMD_UNINITIAL		0X79

#define SIP_PTZCMD_LOCK				0xC0
#define SIP_PTZCMD_UNLOCK			0X80
/***********************************************************/
#define SIP_H_VIA				"Via: "
#define SIP_H_ROUTE				"Route: "
#define SIP_H_FROM				"From: "
#define SIP_H_TO				"To: "
#define SIP_H_CONTACT			"Contact: "
#define SIP_H_CSEQ				"CSeq: "
#define SIP_H_CALL_ID			"Call-ID: "
#define SIP_H_SUBJECT			"Subject: "
#define SIP_H_MAX_FORWARDS		"Max-Forwards: "
#define SIP_H_CONTENT_TYPE		"Content-Type: "
#define SIP_H_CONTENT_LENGTH	"Content-Length: "
#define SIP_H_WWW_AUTHENTICATE	"WWW-Authenticate: "
#define SIP_H_AUTHORIZATION		"Authorization: "
#define SIP_H_EXPIRES			"Expires: "
#define SIP_H_DATE				"Date: "
#define SIP_H_EVENT				"Event: "
#define SIP_H_SUBCRIPTION_STATE	"Subscription-State: "
#define SIP_H_USER_AGENT		"User-Agent: "
#define SIP_H_ACCEPT			"Accept: "
#define SIP_H_ALLOW				"Allow: "
#define SIP_H_SUPPORTED			"Supported: "


#define SIP_STATUS_100			"100 Trying"								//��������
#define SIP_STATUS_101			"101 Dialog Establishement"					//�Ự����
#define SIP_STATUS_200			"200 OK"									//�Ự�ɹ�
#define SIP_STATUS_201			"201 Canceling"								//�Ựȡ��
#define SIP_STATUS_400			"400 Bad Request"							//�������
#define SIP_STATUS_401			"401 Unauthorized"							//δ��֤
#define SIP_STATUS_403			"403 Forbidden"								//��ֹ����
#define SIP_STATUS_404			"404 Not Found"								//δ�ҵ�
#define	SIP_STATUS_405			"405 Method Not Allowed"					//��֧�ֵ�����
#define SIP_STATUS_408			"408 Request Timeout"						//�Ự��ʱ
#define SIP_STATUS_457			"457 Invalid Range"							//��Ч��Χ
#define SIP_STATUS_481			"481 Dialog/Transaction Does Not Exist"		//û��ƥ��ĻỰ		
#define SIP_STATUS_482			"482 Loop Detected"							//ѭ������
#define SIP_STATUS_486			"486 Busy Here"								//�豸æ
#define SIP_STATUS_487			"487 Request Terminated"					//������ֹ
#define SIP_STATUS_488			"488 Not Acceptable Here"					//SDP����
#define SIP_STATUS_500			"500 Server Internal Error"					//�����ڲ�����
#define SIP_STATUS_502			"502 Bad Gateway"							//��Ч��Ӧ��
#define SIP_STATUS_503			"503 Service Unavailable"					//����������
#define SIP_STATUS_551			"551 Option not supported"					//��֧�ֵĲ���


void			 sip_sleep(int usec);
void			 sip_init(const char *sipcode, const char *ip, int port);
int				 sip_build_randsn();
const char *	 sip_build_randnum(char *outstr, int outlen);
const char *	 sip_build_nonce(char *outstr, int outlen);
const char *	 sip_build_scale(char *outstr, int outlen, float scale);
const char *	 sip_build_sdp_ssrc(char *outstr, int outlen, SIP_INVITE_TYPE type, SIP_PROTOCOL_TYPE prot= SIP_TRANSFER_TCP_ACTIVE);
const char *	 sip_build_sdp_time(char *outstr, int outlen, const char *starttime, const char *endtime);
const char *	 sip_build_authorization_response(char *outstr, int outlen, const char *username, const char *realm, const char *password, const char *nonce, const char *uri);
const char *	 sip_bulid_msg_via(char *outstr, int outlen, const char *ip, int port);
const char *	 sip_build_msg_from(char *outstr, int outlen);
const char *	 sip_build_msg_to(char *outstr, int outlen, const char*user, const char *dst);
const char *	 sip_build_msg_call_id(char *outstr, int outlen, unsigned int id=0);
const char *	 sip_build_msg_subject(char *outstr, int outlen, const char *deviceid, const char *ssrc, const char *terminalid=NULL);
int				 sip_build_msg_request(char *outstr, int outlen, const char *method, const char *dst, const char *callid, int cseq, const char *from, const char *to, const char *ftag, const char*ttag);
int				 sip_build_msg_answer(char *outstr, int outlen, const char *msg, const char *status);
int				 sip_build_invite_answer(char * outstr, int outlen, const char * status, const char * via, const char * from, const char * to, const char * ttag, const char *callid, const char *deviceid, int cseq);
int				 sip_build_msg_register(char *outstr, int outlen, const char *to, const char *auth, int cseq, int expires, const char *callid=NULL, const char *ftag=NULL);
int				 sip_build_msg_keepalive(char *outstr, int outlen, const char *to);
int				 sip_build_msg_invite(char *outstr, int outlen, const char *to, const char *subject, int cseq, const char *callid = NULL, const char *ftag = NULL, const char *ttag=NULL);
// user ������ID uri �豸ID:�豸���Ӳ��� time ������ֹʱ��(��_��) recvip ������IP recvport �������˿� type �㲥���� drct �������� prot ����Э�� speed �����ٶ� media ������ playload �������� stream ���볧��
int				 sip_build_msg_sdp(char *outstr, int outlen, const char *user, const char *uri, const char *time, const char *recvip, int recvport, int type, int drct, int prot=0, int speed=0, int media=0, int playload=0, char *stream=0);
int				 sip_build_msg_ack(char *outstr, int outlen, const char *msg);
int				 sip_build_msg_ack(char *outstr, int outlen, const char *to, int cseq, const char *callid, const char *ftag, const char *ttag);
int				 sip_build_msg_bye(char *outstr, int outlen, const char *to, int cseq, const char *callid, const char *ftag, const char*ttag, char *from=NULL);

SIP_EVENT_TYPE	 sip_get_event_type(const char *msg);
SIP_MESSAGE_TYPE sip_get_message_type(const char *msg);
SIP_CONTENT_TYPE sip_get_content_type(const char *msg);
SIP_RECORD_TYPE	 sip_get_record_type(const char *type);

time_t			 sip_get_totalseconds(char *time);
const char *	 sip_get_formattime(char *outstr, int outlen, char *time);
const char *	 sip_get_generaltime(char *outstr, int outlen, char *time);
const char *	 sip_get_localtime(char *outstr, int outlen, time_t spansec=0);
int				 sip_get_user_role(const char *user);

//nonl ȥ�����з� true �� false ��
const char *	 sip_get_header(char *outstr, int outlen, const char *msg, const char *head, bool nonl=true);
int				 sip_get_status_code(const char *msg);
const char *	 sip_get_from_uri(char *outstr, int outlen, const char *msg);
const char *	 sip_get_from_user(char *outstr, int outlen, const char *msg);
const char *	 sip_get_from_addr(char *outstr, int outlen, const char *msg);
const char *	 sip_get_from_tag(char *outstr, int outlen, const char *msg);
const char *	 sip_get_to_uri(char *outstr, int outlen, const char *msg);
const char *	 sip_get_to_user(char *outstr, int outlen, const char *msg);
const char *	 sip_get_to_addr(char *outstr, int outlen, const char *msg);
const char *	 sip_get_to_tag(char *outstr, int outlen, const char *msg);
int				 sip_get_cseq_num(const char *msg);
const char *	 sip_get_cseq_method(char *outstr, int outlen, const char *msg);
//REGISTER HEADER
const char *	 sip_get_expires(char *outstr, int outlen, const char *msg);
const char *	 sip_get_authorization_username(char *outstr, int outlen, const char *msg);
const char *	 sip_get_authorization_realm(char *outstr, int outlen, const char *msg);
const char *	 sip_get_authorization_nonce(char *outstr, int outlen, const char *msg);
const char *	 sip_get_authorization_uri(char *outstr, int outlen, const char *msg);
const char *	 sip_get_authorization_response(char *outstr, int outlen, const char *msg);
const char *	 sip_get_authorization_algorithm(char *outstr, int outlen, const char *msg);
const char *	 sip_get_www_authenticate_realm(char *outstr, int outlen, const char *msg);
const char *	 sip_get_www_authenticate_nonce(char *outstr, int outlen, const char *msg);
//INVITE HEADER
const char *	 sip_get_subject_deviceid(char *outstr, int outlen, const char *msg);
int				 sip_get_subject_playtype(const char *msg);
int				 sip_get_subject_tranfer(const char *msg);
const char *	 sip_get_subject_msid(char *outstr, int outlen, const char *msg);
const char *	 sip_get_subject_receiverid(char *outstr, int outlen, const char *msg);

int				 sip_get_content_length(const char *msg);
SIP_CONTENT_TYPE sip_get_body(const char *msg, char *outstr, int outlen);
bool			 sip_get_sdp_info(const char *sdp, sip_sdp_info *info);

int				 sip_add_via(char *msg, int msglen, const char *via);
int				 sip_set_start_line_uri(char *msg, int msglen, const char *uri);
int				 sip_set_via_addr(char *msg, int msglen, const char *addr);
int				 sip_set_from_uri(char *msg, int msglen, const char *uri);
int				 sip_set_to_uri(char *msg, int msglen, const char *uri);
int				 sip_set_user_agent(char *msg, int msglen, const char *name);
int				 sip_set_header(char *msg, int msglen, const char *head, const char *value);
int				 sip_set_content_length(char *msg, int msglen, int length);

int				 sip_set_body(char *msg, int msglen, const char *body, SIP_CONTENT_TYPE type);
int				 sip_set_xml_sn(char *body, int bodylen, const char *sn);
int				 sip_set_sdp_ip(char *body, int bodylen, const char *ip);
int				 sip_set_sdp_s(char *body, int bodylen, SIP_INVITE_TYPE type);
int				 sip_set_sdp_t(char *body, int bodylen, const char *time);
int				 sip_set_sdp_y(char *body, int bodylen, const char *ssrc);
int				 sip_set_sdp_f(char *body, int bodylen, int encode, int resolution, int frame, int bitrate, int kbps, int voice, int bitsize, int samples);







