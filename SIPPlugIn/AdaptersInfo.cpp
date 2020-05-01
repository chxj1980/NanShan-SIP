#include "stdafx.h"
#include "AdaptersInfo.h"

#ifdef _WIN32
#include <WinSock2.h>
#include <IPHlpApi.h>
#pragma comment(lib, "Iphlpapi.lib")


AdaptersInfo::AdaptersInfo()
{

}


AdaptersInfo::~AdaptersInfo()
{

}


bool AdaptersInfo::getIp(char *ip)
{
	int nNetMediaType = 0;
	PIP_ADAPTER_INFO pAdapterInfo;
	PIP_ADAPTER_INFO pAdapter = NULL;
	ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
	pAdapterInfo = (PIP_ADAPTER_INFO)malloc(ulOutBufLen);
	DWORD dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen);
	if (dwRetVal == ERROR_BUFFER_OVERFLOW)
	{
		free(pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO *)malloc(ulOutBufLen);
		dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen);
	}
	if (dwRetVal == NO_ERROR)
	{
		pAdapter = pAdapterInfo;
		while (pAdapter)
		{
			int nNetIndex = pAdapter->Index;
			char *sNetName = pAdapter->AdapterName;

			int nNetCardType = 0;	// 0 虚拟网卡  1 物理网卡 2 虚拟物理网卡
			int nNetSubType = 1;    // 0 无线网卡  1 有线网卡
			HKEY hNetKey = NULL;
			DWORD dwNetKey = 0;
			DWORD dwSize = sizeof(dwNetKey);
			DWORD dwType = REG_DWORD;
			char szNetKey[256] = { 0 };
			//sprintf(szNetKey, "SYSTEM\\ControlSet001\\Control\\Class\\{4d36e972-e325-11ce-bfc1-08002be10318}\\%04d", nNetIndex);
			//if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szNetKey, 0, KEY_READ, &hNetKey) != ERROR_SUCCESS)
			//{
			//	pAdapter = pAdapter->Next;
			//	continue;
			//}
			//if (RegQueryValueExA(hNetKey, "Characteristics", 0, &dwType, (LPBYTE)&dwNetKey, &dwSize) != ERROR_SUCCESS)
			//{
			//	pAdapter = pAdapter->Next;
			//	continue;
			//}
			//if ((dwNetKey & 0x4) == 0x4)
			//{
			//	nNetCardType = 1;
			//}

			//RegCloseKey(hNetKey);
			//dwNetKey = 0;
			//dwSize = sizeof(dwNetKey);
			//dwType = REG_DWORD;
			//memset(szNetKey, 0, sizeof(szNetKey));
			sprintf(szNetKey, "SYSTEM\\CurrentControlSet\\Control\\Network\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\%s\\Connection", sNetName);
			if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szNetKey, 0, KEY_READ, &hNetKey) != ERROR_SUCCESS)
			{
				pAdapter = pAdapter->Next;
				continue;
			}
			if (RegQueryValueExA(hNetKey, "MediaSubType", 0, &dwType, (LPBYTE)&dwNetKey, &dwSize) == ERROR_SUCCESS)
			{
				//RegCloseKey(hNetKey);
				//BYTE szNetTypeKey[256] = { 0 };
				//dwSize = sizeof(szNetTypeKey);
				//dwType = REG_SZ;
				//memset(szNetKey, 0, sizeof(szNetKey));
				//sprintf(szNetKey, "SYSTEM\\ControlSet001\\Control\\Class\\{4d36e972-e325-11ce-bfc1-08002be10318}\\%04d\\Ndi\\Interfaces", nNetIndex);
				//if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szNetKey, 0, KEY_READ, &hNetKey) != ERROR_SUCCESS)
				//{
				//	pAdapter = pAdapter->Next;
				//	continue;
				//}
				//if (RegQueryValueEx(hNetKey, "LowerRange", 0, &dwType, szNetTypeKey, &dwSize) == ERROR_SUCCESS)
				//{
				//	if (strstr((char *)szNetTypeKey, "wlan") != NULL || strstr((char *)szNetTypeKey, "wifi") != NULL)
				//	{
				//		nNetSubType = 0;
				//	}
				//}
				if (dwNetKey == 2)
				{
					nNetSubType = 0;
				}
			}
			char szNetTypeKey[256] = { 0 };
			dwSize = sizeof(szNetTypeKey);
			dwType = REG_SZ;
			if (RegQueryValueExA(hNetKey, "PnPInstanceId", 0, &dwType, (LPBYTE)szNetTypeKey, &dwSize) == ERROR_SUCCESS)
			{
				if (strncmp(szNetTypeKey, "PCI", strlen("PCI")) == 0)
				{
					nNetCardType = 1;
				}
				else if (strncmp(szNetTypeKey, "XEN\\VIF", strlen("PCI")) == 0)
				{
					nNetCardType = 2;
				}
				else if (strncmp(szNetTypeKey, "XEN\\PIF", strlen("PCI")) == 0)
				{
					nNetCardType = 1;
				}
			}
			RegCloseKey(hNetKey);
			if (nNetCardType == 1)	//是物理网卡
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
					if (nNetMediaType == 1)	//是有线连接
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
#endif

