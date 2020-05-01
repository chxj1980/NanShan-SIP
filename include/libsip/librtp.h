#pragma once

#define RTP_UDP		0
#define RTP_TCP_A	1
#define RTP_TCP_P	2

#define RTP_CONN_TIMEOUT	5000
#define RTP_ACPT_TIMEOUT	6000
#define RTP_RECV_TIMEOUT	15000

typedef void *	RTP_STREAM;
typedef void (__stdcall *media_stream_cb)(int bufType, unsigned char *buffer, unsigned int bufsize, void *user);

int			sip_get_media_port(int start_port=6000);
void		sip_free_media_port(int media_port);
RTP_STREAM	sip_start_stream(const char *media_ip, int &media_port, int media_mode, const char *dst, void *session, media_stream_cb stream_cb=0, void * user=0);
void		sip_set_stream_cb(RTP_STREAM handle, media_stream_cb stream_cb, void *user);
void		sip_stop_stream(RTP_STREAM handle);
