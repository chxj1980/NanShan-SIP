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
	virtual void on_read(socket_stream* sock_stream, const char *data, size_t dlen);		//���¼�
	virtual void proc_on_init();															//�����ʼ��
	virtual void proc_on_exit();															//�����˳�
public:
	void on_handle(socket_stream* stream, const char *data, size_t dlen);							//��Ϣ����
	//response
	void on_options_response(socket_stream* stream, const char *data, size_t dlen);				//������Ӧ
	void on_register_success(socket_stream* stream, const char *data, size_t dlen);				//ע��ɹ�
	void on_register_failure(socket_stream* stream, const char *data, size_t dlen);				//ע��ʧ��
	void on_invite_proceeding(socket_stream* stream, const char *data, size_t dle);				//�㲥�ȴ�
	void on_invite_success(socket_stream* stream, const char *data, size_t dlen);					//�㲥�ɹ�
	void on_invite_failure(socket_stream* stream, const char *data, size_t dlen);					//�㲥ʧ��
	void on_cancel_success(socket_stream* stream, const char *data, size_t dlen);					//ȡ���ɹ�
	void on_cancel_failure(socket_stream* stream, const char *data, size_t dlen);					//ȡ��ʧ��
	void on_bye_success(socket_stream* stream, const char *data, size_t dlen);						//�رճɹ�
	void on_bye_failure(socket_stream* stream, const char *data, size_t dlen);						//�ر�ʧ��
	void on_info_success(socket_stream* stream, const char *data, size_t dlen);					//�طſ��Ƴɹ�
	void on_info_failure(socket_stream* stream, const char *data, size_t dlen);					//�طſ���ʧ��
	void on_subscribe_success(socket_stream* stream, const char *data, size_t dlen);				//���ĳɹ�
	void on_subscribe_failure(socket_stream* stream, const char *data, size_t dlen);				//����ʧ��
	void on_notify_success(socket_stream* stream, const char *data, size_t dlen);					//֪ͨ�ɹ�
	void on_notify_failure(socket_stream* stream, const char *data, size_t dlen);					//֪ͨʧ��
	void on_message_success(socket_stream* stream, const char *data, size_t dlen);					//��ѯ�ɹ�
	void on_message_failure(socket_stream* stream, const char *data, size_t dlen);					//��ѯʧ��
	void on_msg_control_response(socket_stream* stream, const char *data, size_t dlen);			//���ƻ�Ӧ
	void on_msg_preset_response(socket_stream* stream, const char *data, size_t dlen);				//Ԥ��λ��Ӧ
	void on_msg_devicestate_response(socket_stream* stream, const char *data, size_t dlen);		//�豸״̬��Ӧ
	void on_msg_catalog_response(socket_stream* stream, const char *data, size_t dlen);			//Ŀ¼��ѯ��Ӧ
	void on_msg_deviceinfo_response(socket_stream* stream, const char *data, size_t dlen);			//�豸��Ϣ��Ӧ
	void on_msg_recordinfo_response(socket_stream* stream, const char *data, size_t dlen);			//¼���ѯ��Ӧ
	void on_msg_alarm_response(socket_stream* stream, const char *data, size_t dlen);				//����֪ͨ��Ӧ
	void on_msg_configdownload_response(socket_stream* stream, const char *data, size_t dlen);		//���ò�ѯ��Ӧ
	void on_msg_broadcast_response(socket_stream* stream, const char *data, size_t dlen);			//�����㲥��Ӧ
	//request
	void on_options_request(socket_stream* stream, const char *data, size_t dlen);					//������ѯ
	void on_register_request(socket_stream* stream, const char *data, size_t dlen);				//ע������
	void on_invite_request(socket_stream* stream, const char *data, size_t dlen);					//�㲥����
	void on_cancel_request(socket_stream* stream, const char *data, size_t dlen);					//�㲥ȡ��
	void on_ack_request(socket_stream* stream, const char *data, size_t dlen);						//�㲥ȷ��
	void on_bye_request(socket_stream* stream, const char *data, size_t dlen);						//�㲥�ر�
	void on_info_request(socket_stream* stream, const char *data, size_t dlen);					//�طſ���
	void on_subscribe_request(socket_stream* stream, const char *data, size_t dlen);				//��������
	void on_notify_request(socket_stream* stream, const char *data, size_t dlen);					//֪ͨ��Ϣ
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
	void on_msg_catalog_request(socket_stream* stream, const char *data, size_t dlen);				//Ŀ¼��ѯ
	void on_msg_deviceinfo_request(socket_stream* stream, const char *data, size_t dlen);			//�豸��Ϣ��ѯ
	void on_msg_recordinfo_request(socket_stream* stream, const char *data, size_t dlen);			//¼���ѯ
	void on_msg_alarm_request(socket_stream* stream, const char *data, size_t dlen);				//������ѯ
	void on_msg_configdownload_request(socket_stream* stream, const char *data, size_t dlen);		//���ò�ѯ
	void on_msg_mobileposition_request(socket_stream* stream, const char *data, size_t dlen);		//GPS��ѯ
	//notify
	void on_msg_mobileposition_notify(socket_stream* stream, const char *data, size_t dlen);		//GPS֪ͨ
	void on_msg_keepalive_notify(socket_stream* stream, const char *data, size_t dlen);			//����֪ͨ
	void on_msg_mediastatus_notify(socket_stream* stream, const char *data, size_t dlen);			//ý��������֪ͨ
	void on_msg_alarm_notify(socket_stream* stream, const char *data, size_t dlen);				//����֪ͨ
	void on_msg_broadcast_request(socket_stream* stream, const char *data, size_t dlen);			//�����㲥֪ͨ
	//�Ự����
	void scanf_command();
	void register_manager();
	void session_manager();
	void message_manager();
	void subscribe_manager();
	void catalog_manager();
	//������
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
	//��ѯ�û�Ȩ��
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
	//������Ϣ 
	bool send_msg(const char *addr, const char *msg, size_t mlen);
	//ת��������Ϣ
	bool transfer_msg(const char *deviceid, const char *msg, size_t mlen);
	//ת����Ӧ��Ϣ
	bool response_msg(session_info *pInfo, const char *msg, size_t mlen);
	//����ע����Ϣ
	void send_register(const char *ip, int port, const char *user, const char *pwd, int expires=36000);
	//���͵㲥��Ϣ return invite_cid
	int send_invite(invite_info *info, media_stream_cb stream_cb=NULL, void *user=NULL);
	//���͹ر���Ϣ
	void send_bye(int invite_cid, bool clsrtp=true, bool force=false);
	void send_bye_by_cid(const char *cid, bool clsrtp=true, bool force=false);
	//������̨����
	void send_ptzcmd(const char *addr, char *deviceid, unsigned char cmd, unsigned char param1, unsigned short param2=0, SIP_PTZ_RECT *param=NULL);
	//����¼���ѯ
	int send_recordinfo(const char *addr, char *deviceid, int rectype, char *starttime, char *endtime, record_info *recordinfo, int recordsize);
	//���ͻطſ���
	bool send_info(int invite_cid, unsigned char cmd, unsigned char param);
	int send_info_by_session(session_info *pSession, unsigned char cmd, unsigned char param);
	//���Ͷ�����Ϣ
	int send_subscribe(const char *addr, char *deviceid, int sn, int event_type=0, int aram_type=0, int expires=3600);
	//����Ŀ¼��Ϣ
	int send_message_catalog(const char *addr, char *deviceid);
	//��ӦĿ¼��Ϣ
	int response_catalog_root(session_info *pSession, int sumnum, int type=0);
	int response_catalog_node(session_info *pSession, CatalogInfo *pInfo, int sumnum, int type=0);

	//����GPS��ѯ
	void send_position(const char *addr, char *deviceid);
public:
	config	m_conf;
	
};

