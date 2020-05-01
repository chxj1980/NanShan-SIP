// RtpSrv.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "RtpService.h"

int main()
{
	RtpService srv;
	char szAddr[256] = { 0 };
	sprintf(szAddr, "%s:%d", srv.m_conf.local_addr, srv.m_conf.local_port);

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN);
	printf("RtpService %s@%s has been started!\n", srv.m_conf.local_code, szAddr);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0x07);

	srv.run_alone(szAddr);

    return 0;
}

