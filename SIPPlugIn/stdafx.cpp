// stdafx.cpp : ֻ������׼�����ļ���Դ�ļ�
// SIPPlugIn.pch ����ΪԤ����ͷ
// stdafx.obj ������Ԥ����������Ϣ

#include "stdafx.h"

// TODO: �� STDAFX.H �������κ�����ĸ���ͷ�ļ���
//�������ڴ��ļ�������
#include <WinSock2.h>
#include <IPHlpApi.h>
#pragma comment(lib, "Iphlpapi.lib")
#pragma comment (lib,"Advapi32.lib")

extern "C" IDVRHandler *getHandler()
{
	SIPHandler* pHandler = new SIPHandler();
	return pHandler;
}

bool getIp(char *ip)
{
	int nNetMediaType = 0;
	PIP_ADAPTER_INFO pAdapterInfo;
	PIP_ADAPTER_INFO pAdapter = NULL;
	ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
	pAdapterInfo = (PIP_ADAPTER_INFO)malloc(ulOutBufLen);
	DWORD dwRetVal = ::GetAdaptersInfo(pAdapterInfo, &ulOutBufLen);
	if (dwRetVal == ERROR_BUFFER_OVERFLOW)
	{
		free(pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO *)malloc(ulOutBufLen);
		dwRetVal = ::GetAdaptersInfo(pAdapterInfo, &ulOutBufLen);
	}
	if (dwRetVal == NO_ERROR)
	{
		pAdapter = pAdapterInfo;
		while (pAdapter)
		{
			int nNetIndex = pAdapter->Index;
			char *sNetName = pAdapter->AdapterName;

			int nNetCardType = 0;	// 0 ��������  1 �������� 2 ������������
			int nNetSubType = 1;    // 0 ��������  1 ��������
			HKEY hNetKey = NULL;
			DWORD dwNetKey = 0;
			DWORD dwSize = sizeof(dwNetKey);
			DWORD dwType = REG_DWORD;
			char szNetKey[256] = { 0 };
			sprintf(szNetKey, "SYSTEM\\CurrentControlSet\\Control\\Network\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\%s\\Connection", sNetName);
			if (::RegOpenKeyEx(HKEY_LOCAL_MACHINE, szNetKey, 0, KEY_READ, &hNetKey) != ERROR_SUCCESS)
			{
				pAdapter = pAdapter->Next;
				continue;
			}
			if (::RegQueryValueExA(hNetKey, "MediaSubType", 0, &dwType, (LPBYTE)&dwNetKey, &dwSize) == ERROR_SUCCESS)
			{
				if (dwNetKey == 2)
				{
					nNetSubType = 0;
				}
			}
			char szNetTypeKey[256] = { 0 };
			dwSize = sizeof(szNetTypeKey);
			dwType = REG_SZ;
			if (::RegQueryValueExA(hNetKey, "PnPInstanceId", 0, &dwType, (LPBYTE)szNetTypeKey, &dwSize) == ERROR_SUCCESS)
			{
				if (strncmp(szNetTypeKey, "PCI", strlen("PCI")) == 0)
				{
					nNetCardType = 1;
				}
				else if (strncmp(szNetTypeKey, "XEN\\VIF", strlen("XEN\\VIF")) == 0)
				{
					nNetCardType = 2;
				}
				else if (strncmp(szNetTypeKey, "XEN\\PIF", strlen("XEN\\PIF")) == 0)
				{
					nNetCardType = 1;
				}
			}
			::RegCloseKey(hNetKey);
			if (nNetCardType == 1)	//����������
			{
				if (strcmp(pAdapter->IpAddressList.IpAddress.String, "0.0.0.0") == 0 || strstr(pAdapter->IpAddressList.IpAddress.String, "169.254") != NULL)
				{
					pAdapter = pAdapter->Next;
					continue;
				}
				if (ip)
				{
					strcpy(ip, pAdapter->IpAddressList.IpAddress.String);
					nNetMediaType = nNetSubType;
					if (nNetMediaType == 1)	//����������
					{
						return true;
					}
				}
			}
			else if (nNetCardType == 2)
			{
				if (ip)
				{
					strcpy(ip, pAdapter->IpAddressList.IpAddress.String);
				}
			}
			pAdapter = pAdapter->Next;
		}// end while
	}
	else
	{
		printf("Call to GetAdaptersInfo failed.\n");
		free(pAdapterInfo);
		return false;
	}
	free(pAdapterInfo);
	return true;
}