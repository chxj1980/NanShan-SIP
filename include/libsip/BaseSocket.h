#pragma once

#include "acl_cpp/lib_acl.hpp"
#include "lib_acl.h"

#define MAX_BUF_SIZE	8192


class socket_stream
{
public:
	socket_stream();
	socket_stream(ACL_SOCKET sock, int sock_type);
	~socket_stream();
public:
	int	nSockType;
private:
	ACL_SOCKET sock_stream;
	struct sockaddr_in sock_addr;
	char szClinetAddr[24];
	char szClientIp[16];
public:
	bool open(ACL_SOCKET sock, int sock_type);
	bool set_peer(struct sockaddr_in *addr);
	const char *get_peer(bool full = false);
	int write(const void* data, size_t size);
};


class CBaseSocket
{
public:
	CBaseSocket();
	~CBaseSocket();
public:
	bool run_alone(const char* addrs, const char* path = NULL, unsigned int timeout = 30);
	void stop();
public:
	bool on_tcp_bind(const char *addr, int port);
	void on_read_udp();
	void on_read_tcp(socket_stream *stream, const char *data, size_t dlen);
	int on_write(const char *addr, const char *data, size_t dlen);
protected:
	virtual void on_read(socket_stream *stream, const char *data, size_t dlen) = 0;
	virtual void proc_on_init() = 0;
	virtual void proc_on_exit() = 0;
private:
	bool m_running;
	ACL_SOCKET m_socket_udp;
	ACL_SOCKET m_socket_tcp;
};

