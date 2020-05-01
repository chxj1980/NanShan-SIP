#pragma once
#include "libsip.h"
#include "BaseSocket.h"

//class RtpService : public acl::master_udp
class RtpService : public CBaseSocket
{
public:
	RtpService();
	~RtpService();
protected:
	//virtual void on_read(acl::socket_stream* stream);											//读事件
	virtual void on_read(socket_stream* sock_stream, const char *data, size_t dlen);			//读事件
	virtual void proc_on_init();																//服务初始化
	virtual void proc_on_exit();																//服务退出
public:
	void on_handle(socket_stream* sock_stream, const char *data, size_t dlen);					//消息处理
	//response
	void on_register_success(socket_stream* stream, const char *data, size_t dlen);				//注册成功
	void on_register_failure(socket_stream* stream, const char *data, size_t dlen);				//注册失败
	void on_bye_success(socket_stream* stream, const char *data, size_t dlen);						//关闭成功
	void on_bye_failure(socket_stream* stream, const char *data, size_t dlen);						//关闭失败
	void on_notify_success(socket_stream* stream, const char *data, size_t dlen);					//通知成功
	void on_notify_failure(socket_stream* stream, const char *data, size_t dlen);					//通知失败
	void on_message_success(socket_stream* stream, const char *data, size_t dlen);					//查询成功
	void on_message_failure(socket_stream* stream, const char *data, size_t dlen);					//查询失败
	//request
	void on_options_request(socket_stream* stream, const char *data, size_t dlen);					//能力查询
	void on_invite_request(socket_stream* stream, const char *data, size_t dlen);					//点播请求
	void on_cancel_request(socket_stream* stream, const char *data, size_t dlen);					//点播取消
	void on_ack_request(socket_stream* stream, const char *data, size_t dlen);						//点播确认
	void on_bye_request(socket_stream* stream, const char *data, size_t dlen);						//点播关闭
	void on_info_request(socket_stream* stream, const char *data, size_t dlen);					//回放控制
	void on_subscribe_request(socket_stream* stream, const char *data, size_t dlen);				//订阅请求
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
	void on_msg_deviceinfo_request(socket_stream* stream, const char *data, size_t dlen);			//设备信息查询
	void on_msg_recordinfo_request(socket_stream* stream, const char *data, size_t dlen);			//录像查询
	void on_msg_alarm_request(socket_stream* stream, const char *data, size_t dlen);				//报警查询
	void on_msg_configdownload_request(socket_stream* stream, const char *data, size_t dlen);		//配置查询
	void on_msg_mobileposition_request(socket_stream* stream, const char *data, size_t dlen);		//GPS查询
	//notify
	void on_msg_mediastatus_notify(socket_stream* stream, const char *data, size_t dlen);			//媒体流结束通知
	//会话管理
	void scanf_command();
	void register_manager();
	void session_manager();
	void session_over(const char *callid, int session_type);
	void message_manager();
	void subscribe_manager();
	//媒体流管理
	void rtp_init(const char *send_ports, const char *recv_ports);
	void *rtp_get_send_port(int tranfer);
	void rtp_free_send_port(int port, int tranfer);
	void *rtp_get_recv_thd(int tranfer);
	void rtp_stop_recv_send(const char *callid, const char *deviceid, int playtype);
	void *rtp_get_sdk_thd(ChannelInfo *pChn, void* rtp);
	void rtp_stop_sdk_send(const char *callid, const char *deviceid, int playtype);
	void rtp_cls_recv_send(const char *callid, const char *deviceid, int playtype);
	void rtp_free_recv_port(int port, int tranfer);

	bool on_invite_transfrom_sdk(socket_stream* stream, int nPlayType, int nNetType, char *szDeviecID, char *szCallID, char *szFtag, char *szTtag, char *szBody, char *szMsg);
public:
	//发送消息 
	bool send_msg(const char *addr, const char *msg, size_t mlen);
	//发送关闭
	bool send_bye(const char *callid);
	//
	bool ptzcmd_parse(const char *ptzcmd, int *nCmd, int *nType, int *nIndex, int *nSpeed);
private:
	bool			m_running;
	char			*m_szRecvBuf;
public:
	config			m_conf;
	int				m_nBeginSendPort;
	int				m_nEndSendPort;
	int				m_nBeginRecvPort;
	int				m_nEndRecvPort;
};

