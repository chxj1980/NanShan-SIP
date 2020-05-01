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
	//virtual void on_read(acl::socket_stream* stream);											//���¼�
	virtual void on_read(socket_stream* sock_stream, const char *data, size_t dlen);			//���¼�
	virtual void proc_on_init();																//�����ʼ��
	virtual void proc_on_exit();																//�����˳�
public:
	void on_handle(socket_stream* sock_stream, const char *data, size_t dlen);					//��Ϣ����
	//response
	void on_register_success(socket_stream* stream, const char *data, size_t dlen);				//ע��ɹ�
	void on_register_failure(socket_stream* stream, const char *data, size_t dlen);				//ע��ʧ��
	void on_bye_success(socket_stream* stream, const char *data, size_t dlen);						//�رճɹ�
	void on_bye_failure(socket_stream* stream, const char *data, size_t dlen);						//�ر�ʧ��
	void on_notify_success(socket_stream* stream, const char *data, size_t dlen);					//֪ͨ�ɹ�
	void on_notify_failure(socket_stream* stream, const char *data, size_t dlen);					//֪ͨʧ��
	void on_message_success(socket_stream* stream, const char *data, size_t dlen);					//��ѯ�ɹ�
	void on_message_failure(socket_stream* stream, const char *data, size_t dlen);					//��ѯʧ��
	//request
	void on_options_request(socket_stream* stream, const char *data, size_t dlen);					//������ѯ
	void on_invite_request(socket_stream* stream, const char *data, size_t dlen);					//�㲥����
	void on_cancel_request(socket_stream* stream, const char *data, size_t dlen);					//�㲥ȡ��
	void on_ack_request(socket_stream* stream, const char *data, size_t dlen);						//�㲥ȷ��
	void on_bye_request(socket_stream* stream, const char *data, size_t dlen);						//�㲥�ر�
	void on_info_request(socket_stream* stream, const char *data, size_t dlen);					//�طſ���
	void on_subscribe_request(socket_stream* stream, const char *data, size_t dlen);				//��������
	void on_message_request(socket_stream* stream, const char *data, size_t dlen);					//��ѯ��Ϣ
	void on_msg_ptzcmd_request(socket_stream* stream, const char *data, size_t dlen);				//��̨����
	void on_msg_teleboot_request(socket_stream* stream, const char *data, size_t dlen);			//�豸Զ������
	void on_msg_recordcmd_request(socket_stream* stream, const char *data, size_t dlen);			//�豸�ֶ�¼��
	void on_msg_guardcmd_request(socket_stream* stream, const char *data, size_t dlen);			//��������
	void on_msg_alarmcmd_request(socket_stream* stream, const char *data, size_t dlen);			//������λ
	void on_msg_ifamecmd_request(socket_stream* stream, const char *data, size_t dlen);			//ǿ��I֡
	void on_msg_dragzoomin_request(socket_stream* stream, const char *data, size_t dlen);			//����Ŵ�
	void on_msg_dragzoomout_request(socket_stream* stream, const char *data, size_t dlen);			//������С
	void on_msg_homeposition_request(socket_stream* stream, const char *data, size_t dlen);		//��������
	void on_msg_config_request(socket_stream* stream, const char *data, size_t dlen);				//��������
	void on_msg_preset_request(socket_stream* stream, const char *data, size_t dlen);				//Ԥ��λ��ѯ
	void on_msg_devicestate_request(socket_stream* stream, const char *data, size_t dlen);			//�豸״̬��ѯ
	void on_msg_deviceinfo_request(socket_stream* stream, const char *data, size_t dlen);			//�豸��Ϣ��ѯ
	void on_msg_recordinfo_request(socket_stream* stream, const char *data, size_t dlen);			//¼���ѯ
	void on_msg_alarm_request(socket_stream* stream, const char *data, size_t dlen);				//������ѯ
	void on_msg_configdownload_request(socket_stream* stream, const char *data, size_t dlen);		//���ò�ѯ
	void on_msg_mobileposition_request(socket_stream* stream, const char *data, size_t dlen);		//GPS��ѯ
	//notify
	void on_msg_mediastatus_notify(socket_stream* stream, const char *data, size_t dlen);			//ý��������֪ͨ
	//�Ự����
	void scanf_command();
	void register_manager();
	void session_manager();
	void session_over(const char *callid, int session_type);
	void message_manager();
	void subscribe_manager();
	//ý��������
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
	//������Ϣ 
	bool send_msg(const char *addr, const char *msg, size_t mlen);
	//���͹ر�
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

