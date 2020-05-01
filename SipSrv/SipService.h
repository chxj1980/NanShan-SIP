#pragma once
#include "libsip.h"
#include "librtp.h"
#include "BaseSocket.h"
#ifdef _DBLIB_USE
#include "DatabaseClient.h"
#endif
#ifdef _KAFKA_USE
#include "Consumer.h"
#endif


//class SipService : public acl::master_udp
class SipService : public CBaseSocket
{
public:
	SipService();
	~SipService();
protected:
	//virtual void on_read(acl::socket_stream* stream);	
	virtual void on_read(socket_stream* sock_stream, const char *data, size_t dlen);		//读事件
	virtual void proc_on_init();															//服务初始化
	virtual void proc_on_exit();															//服务退出
public:
	void on_handle(socket_stream* stream, const char *data, size_t dlen);							//消息处理
	//response
	void on_options_response(socket_stream* stream, const char *data, size_t dlen);				//能力回应
	void on_register_success(socket_stream* stream, const char *data, size_t dlen);				//注册成功
	void on_register_failure(socket_stream* stream, const char *data, size_t dlen);				//注册失败
	void on_invite_proceeding(socket_stream* stream, const char *data, size_t dle);				//点播等待
	void on_invite_success(socket_stream* stream, const char *data, size_t dlen);					//点播成功
	void on_invite_failure(socket_stream* stream, const char *data, size_t dlen);					//点播失败
	void on_cancel_success(socket_stream* stream, const char *data, size_t dlen);					//取消成功
	void on_cancel_failure(socket_stream* stream, const char *data, size_t dlen);					//取消失败
	void on_bye_success(socket_stream* stream, const char *data, size_t dlen);						//关闭成功
	void on_bye_failure(socket_stream* stream, const char *data, size_t dlen);						//关闭失败
	void on_info_success(socket_stream* stream, const char *data, size_t dlen);					//回放控制成功
	void on_info_failure(socket_stream* stream, const char *data, size_t dlen);					//回放控制失败
	void on_subscribe_success(socket_stream* stream, const char *data, size_t dlen);				//订阅成功
	void on_subscribe_failure(socket_stream* stream, const char *data, size_t dlen);				//订阅失败
	void on_notify_success(socket_stream* stream, const char *data, size_t dlen);					//通知成功
	void on_notify_failure(socket_stream* stream, const char *data, size_t dlen);					//通知失败
	void on_message_success(socket_stream* stream, const char *data, size_t dlen);					//查询成功
	void on_message_failure(socket_stream* stream, const char *data, size_t dlen);					//查询失败
	void on_msg_control_response(socket_stream* stream, const char *data, size_t dlen);			//控制回应
	void on_msg_preset_response(socket_stream* stream, const char *data, size_t dlen);				//预置位回应
	void on_msg_devicestate_response(socket_stream* stream, const char *data, size_t dlen);		//设备状态回应
	void on_msg_catalog_response(socket_stream* stream, const char *data, size_t dlen);			//目录查询回应
	void on_msg_deviceinfo_response(socket_stream* stream, const char *data, size_t dlen);			//设备信息回应
	void on_msg_recordinfo_response(socket_stream* stream, const char *data, size_t dlen);			//录像查询回应
	void on_msg_alarm_response(socket_stream* stream, const char *data, size_t dlen);				//报警通知回应
	void on_msg_configdownload_response(socket_stream* stream, const char *data, size_t dlen);		//配置查询回应
	void on_msg_broadcast_response(socket_stream* stream, const char *data, size_t dlen);			//语音广播回应
	//request
	void on_options_request(socket_stream* stream, const char *data, size_t dlen);					//能力查询
	void on_register_request(socket_stream* stream, const char *data, size_t dlen);				//注册请求
	void on_invite_request(socket_stream* stream, const char *data, size_t dlen);					//点播请求
	void on_cancel_request(socket_stream* stream, const char *data, size_t dlen);					//点播取消
	void on_ack_request(socket_stream* stream, const char *data, size_t dlen);						//点播确认
	void on_bye_request(socket_stream* stream, const char *data, size_t dlen);						//点播关闭
	void on_info_request(socket_stream* stream, const char *data, size_t dlen);					//回放控制
	void on_subscribe_request(socket_stream* stream, const char *data, size_t dlen);				//订阅请求
	void on_notify_request(socket_stream* stream, const char *data, size_t dlen);					//通知消息
	void on_message_request(socket_stream* stream, const char *data, size_t dlen);					//查询消息
	void on_msg_ptzcmd_request(socket_stream* stream, const char *data, size_t dlen);				//云台控制
	void on_msg_teleboot_request(socket_stream* stream, const char *data, size_t dlen);			//设备远程重启
	void on_msg_recordcmd_request(socket_stream* stream, const char *data, size_t dlen);			//设备手动录像
	void on_msg_guardcmd_request(socket_stream* stream, const char *data, size_t dlen);			//布防撤防
	void on_msg_alarmcmd_request(socket_stream* stream, const char *data, size_t dlen);			//报警复位
	void on_msg_ifamecmd_request(socket_stream* stream, const char *data, size_t dlen);			//强制I帧
	void on_msg_dragzoomin_request(socket_stream* stream, const char *data, size_t dlen);			//拉框放大
	void on_msg_dragzoomout_request(socket_stream* stream, const char *data, size_t dlen);			//拉框缩小
	void on_msg_homeposition_request(socket_stream* stream, const char *data, size_t dlen);		//守望控制
	void on_msg_config_request(socket_stream* stream, const char *data, size_t dlen);				//配置设置
	void on_msg_preset_request(socket_stream* stream, const char *data, size_t dlen);				//预置位查询
	void on_msg_devicestate_request(socket_stream* stream, const char *data, size_t dlen);			//设备状态查询
	void on_msg_catalog_request(socket_stream* stream, const char *data, size_t dlen);				//目录查询
	void on_msg_deviceinfo_request(socket_stream* stream, const char *data, size_t dlen);			//设备信息查询
	void on_msg_recordinfo_request(socket_stream* stream, const char *data, size_t dlen);			//录像查询
	void on_msg_alarm_request(socket_stream* stream, const char *data, size_t dlen);				//报警查询
	void on_msg_configdownload_request(socket_stream* stream, const char *data, size_t dlen);		//配置查询
	void on_msg_mobileposition_request(socket_stream* stream, const char *data, size_t dlen);		//GPS查询
	//notify
	void on_msg_mobileposition_notify(socket_stream* stream, const char *data, size_t dlen);		//GPS通知
	void on_msg_keepalive_notify(socket_stream* stream, const char *data, size_t dlen);			//心跳通知
	void on_msg_mediastatus_notify(socket_stream* stream, const char *data, size_t dlen);			//媒体流结束通知
	void on_msg_alarm_notify(socket_stream* stream, const char *data, size_t dlen);				//报警通知
	void on_msg_broadcast_request(socket_stream* stream, const char *data, size_t dlen);			//语音广播通知
	//会话管理
	void scanf_command();
	void register_manager();
	void session_manager();
	void message_manager();
	void subscribe_manager();
	void catalog_manager();
	//工具类
	bool is_no_enter_ip(const char *ip);
	bool build_ssrc(char * ssrc, int slen, int type);
	bool convert_grouping(const char *deviceid, char *groupid);
	void client_locker(bool lock);
	void register_update(const char *deviceid, char *outMappingAddr);
	void register_slist_erase(const char *addr, const char *callid);
	void register_slist_erase_by_cid(const char *callid);
	void register_slist_clear(const char *addr);
	session_info *register_slist_find(const char *addr, const char *deviceid);
	register_info *register_get_mediaservice(const char *deviceid);
	register_info *register_get_info(const char *addr);
	bool is_superior_addr(const char *addr);
	bool is_public_addr(const char *addr);
	bool is_client_addr(const char *addr);
	//查询用户权限
	bool get_user_allow_all(const char *user);
	bool get_user_allow_catalog(const char *user, catalog_list &catlist);
	bool get_user_allow_device(const char *user, const char *deviceid);

#ifdef _DBLIB_USE
	DatabaseClient  m_db;
#endif
private:
	acl::thread		*m_consumer_thd;
	bool			m_running;
	char			*m_szRecvBuf;
	std::vector<std::string> m_no_enter_list;
public:
	//发送消息 
	bool send_msg(const char *addr, const char *msg, size_t mlen);
	//转发请求消息
	bool transfer_msg(const char *deviceid, const char *msg, size_t mlen);
	//转发回应消息
	bool response_msg(session_info *pInfo, const char *msg, size_t mlen);
	//发送注册消息
	void send_register(const char *ip, int port, const char *user, const char *pwd, int expires=36000);
	//发送点播消息 return invite_cid
	int send_invite(invite_info *info, media_stream_cb stream_cb=NULL, void *user=NULL);
	//发送关闭消息
	void send_bye(int invite_cid, bool clsrtp=true, bool force=false);
	void send_bye_by_cid(const char *cid, bool clsrtp=true, bool force=false);
	//发送云台控制
	void send_ptzcmd(const char *addr, char *deviceid, unsigned char cmd, unsigned char param1, unsigned short param2=0, SIP_PTZ_RECT *param=NULL);
	//发送录像查询
	int send_recordinfo(const char *addr, char *deviceid, int rectype, char *starttime, char *endtime, record_info *recordinfo, int recordsize);
	//发送回放控制
	bool send_info(int invite_cid, unsigned char cmd, unsigned char param);
	int send_info_by_session(session_info *pSession, unsigned char cmd, unsigned char param);
	//发送订阅消息
	int send_subscribe(const char *addr, char *deviceid, int sn, int event_type=0, int aram_type=0, int expires=3600);
	//发送目录消息
	int send_message_catalog(const char *addr, char *deviceid);
	//回应目录消息
	int response_catalog_root(session_info *pSession, int sumnum, int type=0);
	int response_catalog_node(session_info *pSession, CatalogInfo *pInfo, int sumnum, int type=0);

	//发送GPS查询
	void send_position(const char *addr, char *deviceid);
public:
	config	m_conf;
	
};

