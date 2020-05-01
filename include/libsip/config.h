#pragma once

#include "acl_cpp/lib_acl.hpp"
#include "lib_acl.h"

#include <direct.h>
#include <string>
#include <vector>
#include <queue>
#include <list>
#include <map>

#ifdef	_WIN32
#define	snprintf _snprintf
#define Line	"\r\n"
#else
#define Line	"\n"
#endif


#define DB_NONE		0
#define DB_ORACLE	1
#define DB_MYSQL	2
#define DB_SQLITE	3

#define LOG_INFO	0
#define LOG_WARN	1
#define LOG_ERROR	2
#define LOG_CLOSE	3

#define	SIP_RECVBUF_MAX		8192
#define SIP_REGISTERTIME	10
#define SIP_ALIVETIME		60
#define SIP_ALIVETIMEOUT	130
#define SIP_SESSIONOVER		6

#define SIP_ROLE_CLIENT		0		//客户端
#define SIP_ROLE_SUPERIOR	1		//上级SIP
#define SIP_ROLE_LOWER		2		//下级SIP
#define SIP_ROLE_MEDIA		3		//RTP流媒体
#define SIP_ROLE_SENDER		4		//设备
#define SIP_ROLE_DEVICE		5		//硬盘录像机
#define SIP_ROLE_CHANNEL	6		//镜头
#define SIP_ROLE_DECODER	7		//解码器
#define SIP_ROLE_MONITOR	8		//显示通道
#define SIP_ROLE_DIRECT		9		//直连
#define SIP_ROLE_INVALID	10		//其他


class config
{
public:
	config();
	~config();
public:
	bool read();
	void log(int level, const char *fmt, ...);
private:
	int log_level;
public:
	char cwd[256];
	char local_addr[16];
	char local_code[24];
	char default_pwd[64];
	char srv_name[64];
	char log_path[256];
	char log_date[64];
	char db_addr[64];
	char db_name[64];
	char db_user[64];
	char db_pwd[64];

	char kfk_brokers[256];
	char kfk_topic[128];
	char kfk_group[128];

	int local_port;
	int db_driver;
	int db_pool;
	int srv_pool;
	int srv_role;
	int clnt_tranfer;
	int rtp_port;
	int rtp_check_ssrc;
	int rec_type;
	int specify_declib;

	int public_mapping;
	char public_addr[24];
	char client_addrs[512];
	char no_enter_addrs[512];

	char rtp_send_port[16];
	char rtp_recv_port[16];
	char reg_list[1024];

};

enum session_flow		//会话流程
{
	SESSION_MEDIA_REQ,		//媒体服务请求
	SESSION_MEDIA_OK,		//媒体服务回应
	SESSION_SENDER_REQ,	//码流发送请求
	SESSION_SENDER_OK,	//码流发送回应
	SESSION_MEDIA_ACK,		//媒体服务确认
	SESSION_SENDER_ACK,	//码流发送确认
	SESSION_RECVER_REQ,		//码流接收请求
	SESSION_RECVER_OK,		//码流接收回应
	SESSION_SMEDIA_REQ,		//媒体流请求
	SESSION_SMEDIA_OK,		//媒体流回应
	SESSION_SMEDIA_REP,		//媒体流分发回应
	SESSION_RECVER_ACK,		//码流接收确认
	SESSION_SMEDIA_ACK,		//媒体流确认
	SESSION_RECVER_BYE,		//码流接收关闭
	SESSION_SMEDIA_BYE,		//媒体流关闭
	SESSION_MEDIA_BYE,		//媒体服务关闭
	SESSION_SENDER_BYE,	//码流发送关闭
	SESSION_END				//会话结束

};

struct record_info
{
	int nType;
	char sFileName[256];
	char startTime[20];
	char endTime[20];
	int  nSize;
};

struct recv_info
{
	char receiverid[21];
	char recvip[16];
	int  recvport;
	int  recvprotocol;
};

struct addr_info
{
	char addr[24];				//地址IP:ADDR
	char ssrc[11];				//媒体流标识
	bool online;				//是否连接成功
	int tranfer;				//传输协议类型
	int transfrom;				//码流是否转换
	time_t timeout;				//连接时间
};

struct invite_info
{
	char deviceid[21];			//设备编码
	char addr[24];				//设备地址
	char media_addr[24];		//媒体服务地址
	char user_addr[24];			//用户地址
	char user_via[128];			//用户请求VIA
	char user_from[128];			//用户请求FROM
	char user_to[128];			//用户请求TO
	char ftag[128];				//会话请求标识
	char ttag[128];				//会话应答标识
	char callid[128];			//会话ID
	char ssrc[11];				//媒体流标识
	char terminalid[21];		//终端编号
	char terminal_addr[24];		//终端地址
	recv_info receiver;			//收流端信息
	bool carrysdp;				//是否携带SDP
	char invite_uri[256];		//点播镜头参数
	char invite_time[32];		//点播时间
	int  invite_type;			//点播类型
	int  downloadspeed;			//下载速度
	int  cseq;					//会话序列号
	int  transfrom;				//码流是否转换
};

struct session_info
{
	char deviceid[21];			//设备编码
	char addr[24];				//设备地址
	char media_user[21];		//媒体服务编码
	char media_addr[24];		//媒体服务地址
	char user_addr[24];			//用户地址
	char userid[21];			//用户编号
	char user_via[128];			//用户请求VIA
	char user_from[128];			//用户请求FROM
	char user_to[128];			//用户请求TO
	int user_role;				//用户角色
	char ftag[128];				//用户请求标识
	char ttag[128];				//设备应答标识
	char callid[64];			//会话ID
	char ssrc[11];				//会话媒体流标识
	char terminalid[21];		//终端编号
	char terminal_addr[24];		//终端地址
	char terminal_ttag[24];		//终端应答标识
	char sn[12];				//命令序列号
	char starttime[20];			//开始时间
	char endtime[20];			//结束时间
	char invite_time[32];		//点播时间
	int  invite_type;			//点播类型
	int  cseq;					//会话序列号
	int  info_cseq;				//控制序列号
	float info_scale;			//控制速度值
	char alarm_method[24];		//报警类型
	int  alarm_type;			//报警子类型
	int  event_type;			//订阅类型
	int	expires;				//订阅有效时间
	recv_info receiver;			//收流端信息
	void *stream_cb;			//码流回调
	void *stream_user;			//码流用户
	void *rtp_stream;			//码流线程
	record_info *rec_info;		//录像文件指针
	int	rec_count;				//录像文件数
	bool rec_finish;
	bool transfrom_sdk;			//是否SDK取流
	bool clsmedia;				//是否释放流				
	time_t mediaover;			//媒体流结束时间
	time_t session_time;		//会话请求时间
	session_flow flow;			//会话流程状态
};

typedef std::list<session_info*>				session_list;
typedef session_list::iterator					seslist_iter;
typedef std::map<std::string, session_info*>	session_map;
typedef session_map::iterator					sesmap_iter;

struct register_info
{
	char addr[24];				//注册地址
	char mapping_addr[24];		//映射地址
	char user[21];				//注册编码
	char pwd[24];				//注册密码
	char name[64];				//注册名称
	int sn;						//订阅ID
	int role;					//注册角色			
	int expires;				//注册时效
	int tranfer;				//媒体传输协议
	int protocol;				//信令传输协议
	int tcpport;				//服务对应的TCP端口
	bool online;				//是否在线
	bool vaild;					//是否有效
	time_t heartbeat;			//当前心跳时间
	time_t alivetime;			//当前存活时间
	session_list *slist;		//关联会话
};

struct ChannelInfo
{
	char szDeviceID[24];
	char szName[128];
	char szManufacturer[64];
	char szModel[64];
	char szSIPCode[24];
	char szUser[64];
	char szPwd[64];
	char szIPAddress[16];
	char szResolution[20];
	char szMediaAddress[24];
	int	 nPort;
	int	 nStatus;		//0-不在线; 1-在线
	int  nPTZType;		//1-球机; 2-半球; 3-固定枪机; 4-遥控枪机
	int  nChannel;
};

struct CatalogInfo
{
	int nID;					//数据库ID
	char szDeviceID[21];		//镜头编号
	char szName[128];			//镜头名称
	char szManufacturer[64];	//镜头厂商
	char szModel[64];			//镜头型号
	char szOwner[64];			//镜头归属
	char szCivilCode[21];		//行政区划
	char szBlock[64];			//所属警区
	char szAddress[128];		//安装地址
	int nParental;				//是否为父设备
	char szParentID[21];		//父ID
	int nSatetyWay;				//信令安全模式
	int nRegisterWay;			//注册方式
	char szCertNum[24];			//证书序列号
	int nCertifiable;			//证书有效标识
	int nErrCode;				//证书无效原因码
	char szEndTime[24];			//证书终止有效期
	int nSecrecy;				//保密属性
	char szIPAddress[16];		//IP地址
	int nPort;					//端口
	char szPassword[64];		//密码
	char szStatus[12];			//在线状态
	char szLongitude[12];		//经度
	char szLatitude[12];		//纬度
	int nPTZType;				//摄像机类型
	int nPositionType;			//位置类型
	int nRoomType;				//室外、室内
	int nUseType;				//用途属性
	int nSupplyLightType;		//补光属性
	int nDirectionType;			//监视方位属性
	char szResolution[32];		//支持的分辨率
	char szBusinessGroupID[21];	//业务分组 ID 
	int nDownloadSpeed;			//下载倍速
	int nSVCpaceSupportMode;	//空域编码能力
	int nSVCTimeSupportMode;	//时域编码能力
	char szServiceID[21];		//所属服务编号
	int nChanID;				//通道ID
};

typedef std::vector<CatalogInfo>				catalog_list;
typedef std::queue<CatalogInfo*>				catalog_que;

typedef std::map<std::string, register_info>	register_map;
typedef register_map::iterator					regmap_iter;

typedef std::vector<acl::string>				register_list;
typedef register_list::iterator					reglist_iter;

typedef std::map<std::string, ChannelInfo>		channel_map;
typedef channel_map::iterator					chnmap_iter;

