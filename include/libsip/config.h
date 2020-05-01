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

#define SIP_ROLE_CLIENT		0		//�ͻ���
#define SIP_ROLE_SUPERIOR	1		//�ϼ�SIP
#define SIP_ROLE_LOWER		2		//�¼�SIP
#define SIP_ROLE_MEDIA		3		//RTP��ý��
#define SIP_ROLE_SENDER		4		//�豸
#define SIP_ROLE_DEVICE		5		//Ӳ��¼���
#define SIP_ROLE_CHANNEL	6		//��ͷ
#define SIP_ROLE_DECODER	7		//������
#define SIP_ROLE_MONITOR	8		//��ʾͨ��
#define SIP_ROLE_DIRECT		9		//ֱ��
#define SIP_ROLE_INVALID	10		//����


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

enum session_flow		//�Ự����
{
	SESSION_MEDIA_REQ,		//ý���������
	SESSION_MEDIA_OK,		//ý������Ӧ
	SESSION_SENDER_REQ,	//������������
	SESSION_SENDER_OK,	//�������ͻ�Ӧ
	SESSION_MEDIA_ACK,		//ý�����ȷ��
	SESSION_SENDER_ACK,	//��������ȷ��
	SESSION_RECVER_REQ,		//������������
	SESSION_RECVER_OK,		//�������ջ�Ӧ
	SESSION_SMEDIA_REQ,		//ý��������
	SESSION_SMEDIA_OK,		//ý������Ӧ
	SESSION_SMEDIA_REP,		//ý�����ַ���Ӧ
	SESSION_RECVER_ACK,		//��������ȷ��
	SESSION_SMEDIA_ACK,		//ý����ȷ��
	SESSION_RECVER_BYE,		//�������չر�
	SESSION_SMEDIA_BYE,		//ý�����ر�
	SESSION_MEDIA_BYE,		//ý�����ر�
	SESSION_SENDER_BYE,	//�������͹ر�
	SESSION_END				//�Ự����

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
	char addr[24];				//��ַIP:ADDR
	char ssrc[11];				//ý������ʶ
	bool online;				//�Ƿ����ӳɹ�
	int tranfer;				//����Э������
	int transfrom;				//�����Ƿ�ת��
	time_t timeout;				//����ʱ��
};

struct invite_info
{
	char deviceid[21];			//�豸����
	char addr[24];				//�豸��ַ
	char media_addr[24];		//ý������ַ
	char user_addr[24];			//�û���ַ
	char user_via[128];			//�û�����VIA
	char user_from[128];			//�û�����FROM
	char user_to[128];			//�û�����TO
	char ftag[128];				//�Ự�����ʶ
	char ttag[128];				//�ỰӦ���ʶ
	char callid[128];			//�ỰID
	char ssrc[11];				//ý������ʶ
	char terminalid[21];		//�ն˱��
	char terminal_addr[24];		//�ն˵�ַ
	recv_info receiver;			//��������Ϣ
	bool carrysdp;				//�Ƿ�Я��SDP
	char invite_uri[256];		//�㲥��ͷ����
	char invite_time[32];		//�㲥ʱ��
	int  invite_type;			//�㲥����
	int  downloadspeed;			//�����ٶ�
	int  cseq;					//�Ự���к�
	int  transfrom;				//�����Ƿ�ת��
};

struct session_info
{
	char deviceid[21];			//�豸����
	char addr[24];				//�豸��ַ
	char media_user[21];		//ý��������
	char media_addr[24];		//ý������ַ
	char user_addr[24];			//�û���ַ
	char userid[21];			//�û����
	char user_via[128];			//�û�����VIA
	char user_from[128];			//�û�����FROM
	char user_to[128];			//�û�����TO
	int user_role;				//�û���ɫ
	char ftag[128];				//�û������ʶ
	char ttag[128];				//�豸Ӧ���ʶ
	char callid[64];			//�ỰID
	char ssrc[11];				//�Ựý������ʶ
	char terminalid[21];		//�ն˱��
	char terminal_addr[24];		//�ն˵�ַ
	char terminal_ttag[24];		//�ն�Ӧ���ʶ
	char sn[12];				//�������к�
	char starttime[20];			//��ʼʱ��
	char endtime[20];			//����ʱ��
	char invite_time[32];		//�㲥ʱ��
	int  invite_type;			//�㲥����
	int  cseq;					//�Ự���к�
	int  info_cseq;				//�������к�
	float info_scale;			//�����ٶ�ֵ
	char alarm_method[24];		//��������
	int  alarm_type;			//����������
	int  event_type;			//��������
	int	expires;				//������Чʱ��
	recv_info receiver;			//��������Ϣ
	void *stream_cb;			//�����ص�
	void *stream_user;			//�����û�
	void *rtp_stream;			//�����߳�
	record_info *rec_info;		//¼���ļ�ָ��
	int	rec_count;				//¼���ļ���
	bool rec_finish;
	bool transfrom_sdk;			//�Ƿ�SDKȡ��
	bool clsmedia;				//�Ƿ��ͷ���				
	time_t mediaover;			//ý��������ʱ��
	time_t session_time;		//�Ự����ʱ��
	session_flow flow;			//�Ự����״̬
};

typedef std::list<session_info*>				session_list;
typedef session_list::iterator					seslist_iter;
typedef std::map<std::string, session_info*>	session_map;
typedef session_map::iterator					sesmap_iter;

struct register_info
{
	char addr[24];				//ע���ַ
	char mapping_addr[24];		//ӳ���ַ
	char user[21];				//ע�����
	char pwd[24];				//ע������
	char name[64];				//ע������
	int sn;						//����ID
	int role;					//ע���ɫ			
	int expires;				//ע��ʱЧ
	int tranfer;				//ý�崫��Э��
	int protocol;				//�����Э��
	int tcpport;				//�����Ӧ��TCP�˿�
	bool online;				//�Ƿ�����
	bool vaild;					//�Ƿ���Ч
	time_t heartbeat;			//��ǰ����ʱ��
	time_t alivetime;			//��ǰ���ʱ��
	session_list *slist;		//�����Ự
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
	int	 nStatus;		//0-������; 1-����
	int  nPTZType;		//1-���; 2-����; 3-�̶�ǹ��; 4-ң��ǹ��
	int  nChannel;
};

struct CatalogInfo
{
	int nID;					//���ݿ�ID
	char szDeviceID[21];		//��ͷ���
	char szName[128];			//��ͷ����
	char szManufacturer[64];	//��ͷ����
	char szModel[64];			//��ͷ�ͺ�
	char szOwner[64];			//��ͷ����
	char szCivilCode[21];		//��������
	char szBlock[64];			//��������
	char szAddress[128];		//��װ��ַ
	int nParental;				//�Ƿ�Ϊ���豸
	char szParentID[21];		//��ID
	int nSatetyWay;				//���ȫģʽ
	int nRegisterWay;			//ע�᷽ʽ
	char szCertNum[24];			//֤�����к�
	int nCertifiable;			//֤����Ч��ʶ
	int nErrCode;				//֤����Чԭ����
	char szEndTime[24];			//֤����ֹ��Ч��
	int nSecrecy;				//��������
	char szIPAddress[16];		//IP��ַ
	int nPort;					//�˿�
	char szPassword[64];		//����
	char szStatus[12];			//����״̬
	char szLongitude[12];		//����
	char szLatitude[12];		//γ��
	int nPTZType;				//���������
	int nPositionType;			//λ������
	int nRoomType;				//���⡢����
	int nUseType;				//��;����
	int nSupplyLightType;		//��������
	int nDirectionType;			//���ӷ�λ����
	char szResolution[32];		//֧�ֵķֱ���
	char szBusinessGroupID[21];	//ҵ����� ID 
	int nDownloadSpeed;			//���ر���
	int nSVCpaceSupportMode;	//�����������
	int nSVCTimeSupportMode;	//ʱ���������
	char szServiceID[21];		//����������
	int nChanID;				//ͨ��ID
};

typedef std::vector<CatalogInfo>				catalog_list;
typedef std::queue<CatalogInfo*>				catalog_que;

typedef std::map<std::string, register_info>	register_map;
typedef register_map::iterator					regmap_iter;

typedef std::vector<acl::string>				register_list;
typedef register_list::iterator					reglist_iter;

typedef std::map<std::string, ChannelInfo>		channel_map;
typedef channel_map::iterator					chnmap_iter;

