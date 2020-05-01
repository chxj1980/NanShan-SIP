#include "BaseSocket.h"


std::map<std::string, ACL_SOCKET> __tcp_socklist;
acl::locker clnt_locker;
acl::locker read_locker;

class recv_udp_thread : public acl::thread
{
public:
	recv_udp_thread(CBaseSocket *pSocket)
	{
		m_socket = pSocket;
	}
	~recv_udp_thread(){}
protected:
	virtual void *run()
	{
		if (m_socket)
			m_socket->on_read_udp();
		delete this;
		return 0;
	}
private:
	CBaseSocket *m_socket;
};

//接收上级TCP信令线程
class recv_tcp_superior_thread : public acl::thread
{
public:
	recv_tcp_superior_thread(ACL_SOCKET sock, struct sockaddr_in *sock_addr, CBaseSocket *pSocket)
	{
		m_connectd = false;
		memset(&m_sock_addr, 0, sizeof(m_sock_addr));
		memcpy(&m_sock_addr, sock_addr, sizeof(m_sock_addr));
		m_sock = sock;
		m_socket = pSocket;
	}
	~recv_tcp_superior_thread() {}
protected:
	virtual void *run()
	{
		int nRecvLen = 0;
		char szBuffer[MAX_BUF_SIZE] = { 0 };
		while (true)
		{
			if (!m_connectd)
			{
				if (acl_timed_connect(m_sock, (sockaddr *)&m_sock_addr, sizeof(m_sock_addr), 6) == 0)
					m_connectd = true;
			}
			memset(szBuffer, 0, sizeof(szBuffer));
			nRecvLen = recv(m_sock, szBuffer, sizeof(szBuffer), 0);
			if (nRecvLen > 0)
			{
				socket_stream stream(m_sock, SOCK_STREAM);
				stream.set_peer(&m_sock_addr);
				if (m_socket)
					m_socket->on_read_tcp(&stream, szBuffer, nRecvLen);
			}
			else
				m_connectd = false;
		}
		delete this;
		return 0;
	}
private:
	bool m_connectd;
	ACL_SOCKET m_sock;
	struct sockaddr_in m_sock_addr;
	CBaseSocket *m_socket;
};

//接收下级TCP信令线程
class recv_tcp_lower_thread : public acl::thread	
{
public:
	recv_tcp_lower_thread(ACL_SOCKET sock, struct sockaddr_in *sock_addr, CBaseSocket *pSocket)
	{
		m_sock = sock;
		memcpy(&m_sock_addr, sock_addr, sizeof(m_sock_addr));
		m_socket = pSocket;
	}
	~recv_tcp_lower_thread() {}
protected:
	virtual void *run()
	{
		int nRecvLen = 0;
		char szBuffer[MAX_BUF_SIZE] = { 0 };
		while (true)
		{
			memset(szBuffer, 0, sizeof(szBuffer));
			int nRecvLen = recv(m_sock, szBuffer, sizeof(szBuffer), 0);
			if (nRecvLen > 0)
			{
				socket_stream stream(m_sock, SOCK_STREAM);
				stream.set_peer(&m_sock_addr);
				if (m_socket)
					m_socket->on_read_tcp(&stream, szBuffer, nRecvLen);
			}
			else
				break;
		}
		char szAddr[24] = { 0 };
		sprintf(szAddr, "%s:%d", inet_ntoa(m_sock_addr.sin_addr), ntohs(m_sock_addr.sin_port));
		__tcp_socklist[szAddr] = ACL_SOCKET_INVALID;
		acl_socket_close(m_sock);
		delete this;
		return 0;
	}
private:
	ACL_SOCKET m_sock;
	struct sockaddr_in m_sock_addr;
	CBaseSocket *m_socket;
};

CBaseSocket::CBaseSocket()
{
	m_running = true;
	m_socket_udp = ACL_SOCKET_INVALID;
	m_socket_tcp = ACL_SOCKET_INVALID;
}


CBaseSocket::~CBaseSocket()
{
	
}

bool CBaseSocket::run_alone(const char * addrs, const char * path, unsigned int timeout)
{
	int nRecvTimeout = timeout * 1000;
	int nBufferSize = MAX_BUF_SIZE;
	char szAddr[24] = { 0 };
	strcpy(szAddr, addrs);
	char *pos = strchr(szAddr, ':');
	if (!pos)
		return false;
	int nSrvPort = atoi(pos + 1);
	pos[0] = '\0';
	struct sockaddr_in srv_addr;
	memset(&srv_addr, 0, sizeof(srv_addr));
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_addr.S_un.S_addr = inet_addr(szAddr);
	srv_addr.sin_port = htons(nSrvPort);

	m_socket_udp = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	m_socket_tcp = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	setsockopt(m_socket_udp, SOL_SOCKET, SO_RCVTIMEO, (char *)&nRecvTimeout, sizeof(nRecvTimeout));
	setsockopt(m_socket_udp, SOL_SOCKET, SO_SNDBUF, (char*)&nBufferSize, sizeof(nBufferSize));
	setsockopt(m_socket_udp, SOL_SOCKET, SO_RCVBUF, (char*)&nBufferSize, sizeof(nBufferSize));

	setsockopt(m_socket_tcp, SOL_SOCKET, SO_RCVTIMEO, (char *)&nRecvTimeout, sizeof(nRecvTimeout));
	setsockopt(m_socket_tcp, SOL_SOCKET, SO_SNDBUF, (char*)&nBufferSize, sizeof(nBufferSize));
	setsockopt(m_socket_tcp, SOL_SOCKET, SO_RCVBUF, (char*)&nBufferSize, sizeof(nBufferSize));

	if (bind(m_socket_udp, (sockaddr *)&srv_addr, sizeof(srv_addr)) == -1)
	{
		printf("绑定UDP地址[%s]失败!\n", addrs);
		return false;
	}
	if (bind(m_socket_tcp, (sockaddr *)&srv_addr, sizeof(srv_addr)) == -1)
	{
		printf("绑定TCP地址[%s]失败!\n", addrs);
		return false;
	}
	listen(m_socket_tcp, 20);

	recv_udp_thread *recv_udp_thd = new recv_udp_thread(this);
	recv_udp_thd->set_detachable(true);
	recv_udp_thd->start();

	proc_on_init();

	struct sockaddr_in clnt_addr;
	int addr_size = sizeof(clnt_addr);
	while (m_running)
	{
		memset(&clnt_addr, 0, sizeof(clnt_addr));
		ACL_SOCKET clnt_socket = acl_sane_accept(m_socket_tcp, (sockaddr *)&clnt_addr, &addr_size);
		if (clnt_socket != ACL_SOCKET_INVALID)
		{
			recv_tcp_lower_thread *recv_tcp_thd = new recv_tcp_lower_thread(clnt_socket, &clnt_addr, this);
			recv_tcp_thd->set_detachable(true);
			recv_tcp_thd->start();

			char szAddr[24] = { 0 };
			sprintf(szAddr, "%s:%d", inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port));
			__tcp_socklist[szAddr] = clnt_socket;
		}
	}

	proc_on_exit();

	return true;
}

void CBaseSocket::stop()
{
	m_running = false;
}

bool CBaseSocket::on_tcp_bind(const char * addr, int port)
{
	int nRecvTimeout = 30000;
	int nBufferSize = MAX_BUF_SIZE;
	char szAddr[24] = { 0 };
	strcpy(szAddr, addr);
	char *pos = strchr(szAddr, ':');
	if (!pos)
		return false;
	int nPort = atoi(pos + 1);
	pos[0] = '\0';
	struct sockaddr_in sock_addr;
	memset(&sock_addr, 0, sizeof(sock_addr));
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_addr.S_un.S_addr = inet_addr(szAddr);
	sock_addr.sin_port = htons(nPort);

	struct sockaddr_in srv_addr;
	memset(&srv_addr, 0, sizeof(srv_addr));
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	srv_addr.sin_port = htons(port);
	ACL_SOCKET socket_tcp = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	setsockopt(socket_tcp, SOL_SOCKET, SO_RCVTIMEO, (char *)&nRecvTimeout, sizeof(nRecvTimeout));
	setsockopt(socket_tcp, SOL_SOCKET, SO_SNDBUF, (char*)&nBufferSize, sizeof(nBufferSize));
	setsockopt(socket_tcp, SOL_SOCKET, SO_RCVBUF, (char*)&nBufferSize, sizeof(nBufferSize));
	if (bind(socket_tcp, (sockaddr *)&srv_addr, sizeof(srv_addr)) == -1)
	{
		printf("绑定TCP端口[%d]失败!\n", port);
		return false;
	}
	recv_tcp_superior_thread *recv_tcp_thd = new recv_tcp_superior_thread(socket_tcp, &sock_addr, this);
	recv_tcp_thd->set_detachable(true);
	recv_tcp_thd->start();

	__tcp_socklist[addr] = socket_tcp;
	return true;
}

void CBaseSocket::on_read_udp()
{
	int nRecvLen = 0;
	struct sockaddr_in clnt_addr;
	int addr_size = sizeof(clnt_addr);
	char szBuffer[MAX_BUF_SIZE] = { 0 };
	while (m_running)
	{
		read_locker.lock();
		memset(&clnt_addr, 0, addr_size);
		memset(szBuffer, 0, sizeof(szBuffer));
		nRecvLen = recvfrom(m_socket_udp, szBuffer, sizeof(szBuffer), 0, (sockaddr *)&clnt_addr, &addr_size);
		if (nRecvLen > 0)
		{
			if (!m_running)
			{
				read_locker.unlock();
				return;
			}
			socket_stream stream(m_socket_udp, SOCK_DGRAM);
			stream.set_peer(&clnt_addr);
			on_read(&stream, szBuffer, nRecvLen);
		}
		read_locker.unlock();
	}

}

void CBaseSocket::on_read_tcp(socket_stream * stream, const char *data, size_t dlen)
{
	on_read(stream, data, dlen);
}

int CBaseSocket::on_write(const char * addr, const char * data, size_t dlen)
{
	char szAddr[24] = { 0 };
	strcpy(szAddr, addr);
	char *pos = strchr(szAddr, ':');
	if (!pos)
		return 0;
	int nPort = atoi(pos + 1);
	pos[0] = '\0';
	struct sockaddr_in sock_addr;
	memset(&sock_addr, 0, sizeof(sock_addr));
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_addr.S_un.S_addr = inet_addr(szAddr);
	sock_addr.sin_port = htons(nPort);

	std::map<std::string, ACL_SOCKET>::iterator itor = __tcp_socklist.find(addr);
	if (itor != __tcp_socklist.end())
	{
		if (itor->second != ACL_SOCKET_INVALID)
			return send(itor->second, data, (int)dlen, 0);
		else
			return 0;
	}
	else
	{
		return sendto(m_socket_udp, data, (int)dlen, 0, (sockaddr *)&sock_addr, sizeof(sock_addr));
	}
}

socket_stream::socket_stream()
{
}

socket_stream::socket_stream(ACL_SOCKET sock, int sock_type)
{
	nSockType = sock_type;
	sock_stream = sock;
	memset(&sock_addr,0,sizeof(sock_addr));
	memset(szClinetAddr,0,sizeof(szClinetAddr));
	memset(szClientIp,0,sizeof(szClientIp));
}

socket_stream::~socket_stream()
{
	sock_stream = ACL_SOCKET_INVALID;
}

bool socket_stream::open(ACL_SOCKET sock, int sock_type)
{
	nSockType = sock_type;
	sock_stream = sock;
	return false;
}

bool socket_stream::set_peer(sockaddr_in * addr)
{
	if (!addr)
		return false;
	memcpy(&sock_addr, addr, sizeof(sock_addr));
	char *ip = inet_ntoa(addr->sin_addr);
	int port = ntohs(addr->sin_port);
	sprintf(szClinetAddr, "%s:%d", ip, port);
	sprintf(szClientIp, "%s", ip);
	return true;
}

const char * socket_stream::get_peer(bool full)
{
	if (full)
		return szClinetAddr;
	else
		return szClientIp;
}

int socket_stream::write(const void * data, size_t size)
{
	if (sock_stream == ACL_SOCKET_INVALID)
		return -1;
	if (nSockType == SOCK_DGRAM)
	{
		return sendto(sock_stream, (const char *)data, (int)size, 0, (sockaddr *)&sock_addr, sizeof(sock_addr));
	}
	else if (nSockType == SOCK_STREAM)
	{
		return send(sock_stream, (const char *)data, (int)size, 0);
	}
	return 0;
}
