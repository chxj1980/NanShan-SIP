#pragma once
#include "config.h"

#include <time.h>
#include <math.h>


/********枚举类型**************************************/

typedef enum sip_event_type		//消息事件类型
{
	SIP_EVENT_UNKNOWN,		//未知事件
	SIP_REGISTER_REQUEST,	//注册请求
	SIP_REGISTER_SUCCESS,   //注册成功
	SIP_REGISTER_FAILURE,   //注册失败
	SIP_INVITE_REQUEST,		//点播请求
	SIP_INVITE_PROCEEDING,	//点播等待
	SIP_INVITE_SUCCESS,		//点播成功
	SIP_INVITE_FAILURE,		//点播失败
	SIP_ACK_REQUEST,		//点播确认	
	SIP_BYE_REQUEST,		//关闭请求
	SIP_BYE_SUCCESS,		//关闭成功
	SIP_BYE_FAILURE,		//关闭失败
	SIP_CANCEL_REQUEST,		//取消请求
	SIP_CANCEL_SUCCESS,		//取消成功	
	SIP_CANCEL_FAILURE,		//取消失败
	SIP_INFO_REQUEST,		//控制请求
	SIP_INFO_SUCCESS,		//控制成功
	SIP_INFO_FAILURE,		//控制失败
	SIP_SUBSCRIBE_REQUEST,	//订阅请求
	SIP_SUBSCRIBE_SUCCESS,	//订阅成功
	SIP_SUBSCRIBE_FAILURE,	//订阅失败
	SIP_NOTIFY_EVENT,		//通知消息
	SIP_NOTIFY_SUCCESS,		//通知成功
	SIP_NOTIFY_FAILURE,		//通知失败
	SIP_MESSAGE_EVENT,		//消息请求
	SIP_MESSAGE_SUCCESS,	//消息成功
	SIP_MESSAGE_FAILURES,	//消息失败
	SIP_OPTIONS_REQUEST,	//查询功能
	SIP_OPTIONS_RESPONSE	//查询响应

}SIP_EVENT_TYPE;

typedef enum sip_message_type	//消息请求类型
{
	SIP_MESSAGE_UNKNOWN,
	SIP_CONTROL_PTZCMD,			//云台控制
	SIP_CONTROL_TELEBOOT,		//远程重启(设备)
	SIP_CONTROL_RECORDCMD,		//手动录像(设备)
	SIP_CONTROL_GUARDCMD,		//布防/撤防
	SIP_CONTROL_ALARMCMD,		//报警复位
	SIP_CONTROL_IFAMECMD,		//强制I帧
	SIP_CONTROL_DRAGZOOMIN,		//拉框放大
	SIP_CONTROL_DRAGZOOMOUT,	//拉框缩小
	SIP_CONTROL_HOMEPOSITION,	//守望控制
	SIP_CONTROL_CONFIG,			//配置控制
	SIP_RESPONSE_CONTROL,		//控制应答
	SIP_QUERY_PRESETQUERY,		//预置位查询
	SIP_RESPONSE_PRESETQUERY,	//预置位回应
	SIP_QUERY_DEVICESTATUS,		//设备状态查询
	SIP_RESPONSE_DEVICESTATUS,	//设备状态回应
	SIP_QUERY_CATALOG,			//目录信息查询
	SIP_RESPONSE_CATALOG,		//目录信息回应
	SIP_QUERY_DEVICEINFO,		//设备信息查询
	SIP_RESPONSE_DEVICEINFO,	//设备信息回应
	SIP_QUERY_RECORDINFO,		//录像目录检索
	SIP_RESPONSE_RECORDINFO,	//目录检索回应
	SIP_QUERY_ALARM,			//报警查询
	SIP_RESPONSE_QUERY_ALARM,	//报警查询回应
	SIP_QUERY_CONFIGDOWNLOAD,	//设备配置查询
	SIP_RESPONSE_CONFIGDOWNLOAD,//设备配置回应
	SIP_QUERY_MOBILEPOSITION,	//点位坐标查询
	SIP_NOTIFY_MOBILEPOSITION,	//点位坐标回应
	SIP_NOTIFY_KEEPALIVE,		//心跳通知
	SIP_NOTIFY_MEDIASTATUS,		//媒体流结束通知
	SIP_NOTIFY_ALARM,			//报警通知
	SIP_RESPONSE_NOTIFY_ALARM,	//报警通知回应

	SIP_NOTIFY_BROADCAST,		//语音广播通知
	SIP_RESPONSE_BROADCAST		//语音广播应答
	
}SIP_MESSAGE_TYPE;

typedef enum sip_invite_type	//点播类型
{
	SIP_INVITE_PLAY,					//实时点播
	SIP_INVITE_PLAYBACK,				//历史点播
	SIP_INVITE_DOWNLOAD,				//下载点播
	SIP_INVITE_PLAY_SDK,				//实时点播SDK
	SIP_INVITE_PLAYBACK_SDK,			//历史点播SDK
	SIP_INVITE_DOWNLOAD_SDK,			//下载点播SDK
	SIP_INVITE_TALK,					//语音对讲
	SIP_INVITE_TALK_SDK,				//语音对讲SDK
	SIP_INVITE_MEDIA,					//媒体流接收
	SIP_INVITE_MEDIASERVER,				//媒体流结束
	
}SIP_INVITE_TYPE;

typedef enum sip_content_type	//消息体类型
{
	SIP_APPLICATION_UNKNOWN,
	SIP_APPLICATION_XML,	//XML
	SIP_APPLICATION_SDP,	//SDP
	SIP_APPLICATION_RTSP	//RTSP

}SIP_CONTENT_TYPE;

typedef enum sip_direction_type	//媒体流方向类型
{
	SIP_DIRECTION_UNKNOWN,
	SIP_RECVONLY,			//只接收
	SIP_SENDONLY,			//只发送
	SIP_SENDRECV			//接收和发送

}SIP_DIRECTION_TYPE;

typedef enum sip_protocol_type
{
	SIP_TRANSFER_UDP,			//UDP传输
	SIP_TRANSFER_TCP_ACTIVE,	//TCP主动传输
	SIP_TRANSFER_TCP_PASSIVE,	//TCP被动传输
	SIP_TRANSFER_SDK,			//SDK传输
	SIP_TRANSFER_SDK_STANDARDING//SDK标准化传输
}SIP_PROTOCOL_TYPE;

typedef enum sip_media_type
{
	SIP_MEDIA_VIDEO,		//视频流
	SIP_MEDIA_AUDIO			//音频流
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

typedef enum sip_resolution_type //分辨率类型
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

typedef enum sip_info_type		//控制类型
{
	SIP_INFO_UNKNOWN,
	SIP_INFO_PLAY,			//回放控制
	SIP_INFO_PAUSE,			//回放暂停
	SIP_INFO_TEARDOWN		//回旋停止
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

/********报警类型**************************************/
typedef enum sip_alarm_method
{
	SIP_ALARM_ALL,			//全部
	SIP_ALARM_PHONE,		//电话报警 
	SIP_ALARM_DEVICE,       //设备报警
	SIP_ALARM_SMS,			//短信报警
	SIP_ALARM_GPS,			//GPS报警
	SIP_ALARM_VIDEO,		//视频报警
	SIP_ALARM_DEVBROKEN,	//设备故障报警
	SIP_ALARM_OTHER,		//其它报警
	SIP_ALARM_PEOPLE,		//行人报警
	SIP_ALARM_VEHICLE		//车辆报警
}SIP_ARARM_METHOD;

typedef enum sip_alarm_device_type	//设备报警类型
{
	SIP_ALARM_LOSE = 1,		//设备丢失
	SIP_ALARM_DISASSEMBLY,	//防拆报警
	SIP_ALARM_DISKFULL,		//磁盘满
	SIP_ALARM_HIGHTTEMP,	//设备高温
	SIP_ALARM_LOWTEMP		//设备低温
	
}SIP_ALARM_DEVICE_TYPE;

typedef enum sip_alarm_video_type	//视频报警类型
{
	SIP_ALARM_BLOCK,		//视频遮挡
	SIP_ALARM_MANUAL,		//人工报警
	SIP_ALARM_MOVING,		//移动侦测
	SIP_ALARM_LEAVEOVER,	//遗留物
	SIP_ALARM_REMOVAL,		//物体移除
	SIP_ALARM_TRIPPINGLINE, //越线检测
	SIP_ALARM_INVASION,		//入侵检测
	SIP_ALARM_RETROGRADE,	//逆行检测
	SIP_ALARM_WANDERING,	//徘徊检测
	SIP_ALARM_FLOWCOUNT,	//流量统计
	SIP_ALARM_CROWDED,		//人员密度
	SIP_ALARM_ANOMALY,		//视频异常
	SIP_ALARM_FASTMOVING	//快速移动

}SIP_ALARM_VIDEO_TYPE;

typedef enum sip_alarm_devbroken_type
{
	SIP_ALARM_DISKBROKEN = 1, //磁盘故障
	SIP_ALARM_FANBROKEN		  //风扇故障

}SIP_ALARM_DEVBORKEN_TYPE;

typedef enum sip_alarm_people_type
{
	SIP_ALARM_FACE,				//人脸检测
	SIP_ALARM_HUMANBODY			//人体检测

}SIP_ALARM_PEOPLE_TYPE;

typedef enum sip_alarm_vehicle_type
{
	SIP_ALARM_CAR,
	SIP_ALARM_NOMOTO

}SIP_ALARM_VEHICLE_TYPE;

typedef enum sip_alarm_invasion_event
{
	SIP_ALARM_EVENT_ENTERAREA = 1, //进入区域
	SIP_ALARM_EVENT_LEAVEAREA	   //离开区域

}SIP_ALARM_INVASION_EVENT;

typedef enum sip_record_type
{
	SIP_RECORDTYPE_ALL,				//全部录像
	SIP_RECORDTYPE_TIME,			//定时录像
	SIP_RECORDTYPE_MOVE,			//侦测录像
	SIP_RECORDTYPE_ALARM,			//告警录像
	SIP_RECORDTYPE_MANUAL			//手动录像

}SIP_RECORD_TYPE;

typedef struct sip_sdp_info
{
	char o_user[21];				//媒体服务编号
	char o_ip[16];					//媒体服务IP
	int s_type;						//点播类型
	int	m_type;						//媒体类型
	int	m_port;						//媒体服务端口
	int m_protocol;					//传输协议
	int	a_direct;					//传输方向
	int	a_playload;					//媒体编码
	int a_downloadspeed;			//下载速度
	size_t t_begintime;				//开始时间
	size_t t_endtime;				//结束时间
	size_t a_filesize;				//文件大小
	char a_streamtype[11];			//编码名称
	char y_ssrc[11];				//媒体流标识
	char u_uri[128];				//文件路径
	char f_parm[128];				//编码参数

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

/********指令码**************************************/
#define SIP_PTZCMD_NONE				0xFF
#define SIP_PTZCMD_STOP				0x00		//停止
#define SIP_PTZCMD_UP				0x08		//向上
#define SIP_PTZCMD_DOWN				0x04		//向下
#define SIP_PTZCMD_LEFT				0x02		//向左
#define SIP_PTZCMD_RIGHT			0x01		//向右
#define SIP_PTZCMD_LU				0x0A		//左上
#define SIP_PTZCMD_LD				0x06		//左下
#define SIP_PTZCMD_RU				0x09		//右上
#define SIP_PTZCMD_RD				0x05		//右下
#define SIP_PTZCMD_ZOOM_IN			0x10		//放大
#define SIP_PTZCMD_ZOOM_OUT			0x20		//缩小

#define SIP_PTZCMD_FI_IRIS_IN		0x48		//光圈放大
#define SIP_PTZCMD_FI_IRIS_OUT		0x44		//光圈缩小
#define SIP_PTZCMD_FI_FOCUS_IN		0x42		//聚焦拉近
#define SIP_PTZCMD_FI_FOCUS_OUT		0x41		//聚集拉远
#define SIP_PTZCMD_FI_STOP			0x40		//调节停止

#define SIP_PTZCMD_SET				0x81		//预置位设置
#define SIP_PTZCMD_GOTO				0x82		//预置位跳转
#define SIP_PTZCMD_DELETE			0x83		//预置位删除
#define SIP_PTZCMD_CRUISE_ADD		0x84		//巡航点添加
#define	SIP_PTZCMD_CRUISE_DEL		0x85		//巡航点删除
#define SIP_PTZCMD_CRUISE_SPEED		0x86		//巡航速度
#define SIP_PTZCMD_CRUISE_DTIME		0x87		//巡航停顿时间
#define SIP_PTZCMD_CRUISE_RUN		0x88		//巡航开始

#define SIP_PTZCMD_AUTO_RUN			0x89		//自动扫描开始
#define SIP_PTZCMD_AUTO_LBORDER		0x89		//自动扫描左边界
#define SIP_PTZCMD_AUTO_RBORDER		0x89		//自动扫描右边界
#define SIP_PTZCMD_AUTO_SPEED		0x8A		//自动扫描速度

#define SIP_PTZCMD_AUXIOPEN			0x8C		//辅助开关打开(雨刷)
#define SIP_PTZCMD_AUXICLOSE		0x8D		//辅助开关关闭

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


#define SIP_STATUS_100			"100 Trying"								//尝试连接
#define SIP_STATUS_101			"101 Dialog Establishement"					//会话建立
#define SIP_STATUS_200			"200 OK"									//会话成功
#define SIP_STATUS_201			"201 Canceling"								//会话取消
#define SIP_STATUS_400			"400 Bad Request"							//请求错误
#define SIP_STATUS_401			"401 Unauthorized"							//未认证
#define SIP_STATUS_403			"403 Forbidden"								//禁止访问
#define SIP_STATUS_404			"404 Not Found"								//未找到
#define	SIP_STATUS_405			"405 Method Not Allowed"					//不支持的请求
#define SIP_STATUS_408			"408 Request Timeout"						//会话超时
#define SIP_STATUS_457			"457 Invalid Range"							//无效范围
#define SIP_STATUS_481			"481 Dialog/Transaction Does Not Exist"		//没有匹配的会话		
#define SIP_STATUS_482			"482 Loop Detected"							//循环请求
#define SIP_STATUS_486			"486 Busy Here"								//设备忙
#define SIP_STATUS_487			"487 Request Terminated"					//请求终止
#define SIP_STATUS_488			"488 Not Acceptable Here"					//SDP错误
#define SIP_STATUS_500			"500 Server Internal Error"					//服务内部错误
#define SIP_STATUS_502			"502 Bad Gateway"							//无效的应答
#define SIP_STATUS_503			"503 Service Unavailable"					//服务器过载
#define SIP_STATUS_551			"551 Option not supported"					//不支持的操作


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
// user 接收流ID uri 设备ID:设备连接参数 time 播放起止时间(秒_秒) recvip 接收流IP recvport 接收流端口 type 点播类型 drct 收流类型 prot 收流协议 speed 下载速度 media 流类型 playload 编码类型 stream 编码厂商
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

//nonl 去掉换行符 true 是 false 否
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







