// AlarmSubSrv.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"



#include "SipService.h"


void msg_cb(int64_t offset, char *message, size_t msglen, void *user)
{
	acl::charset_conv conv;
	acl::string msgout;
	conv.convert("utf-8", "gb2312", message, msglen, &msgout);
	printf("offset:%I64d msglen:%d\n%s\n", offset, (int)msgout.size(), msgout.c_str());
}


int main()
{
	SipService srv;
	char szAddr[256] = { 0 };
	sprintf(szAddr, "%s:%d", srv.m_conf.local_addr, srv.m_conf.local_port);

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY|FOREGROUND_GREEN);
	printf("SipService %s@%s has been started!\n", srv.m_conf.local_code, szAddr);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0x07);

	srv.run_alone(szAddr);

    return 0;
}

