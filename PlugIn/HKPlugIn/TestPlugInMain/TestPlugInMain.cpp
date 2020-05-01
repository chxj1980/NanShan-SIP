// TestPlugInMain.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "IDVRHandler_x64.h"

typedef IDVRHandler*  (*getHandlerFun)();

void CALLBACK stream_cb(intptr_t identify, int nDataType, BYTE* pBuffer, int nBufferSize, void* pUser)
{
	if (nBufferSize > 0)
	{
		printf("data_size:%d %02x %02x %02x %02x %02x type:%d\n", nBufferSize, pBuffer[0], pBuffer[1], pBuffer[2], pBuffer[3], pBuffer[4], nDataType);
	}
}

int main()
{
	char szDllPath[256] = { 0 };
	GetModuleFileName(NULL, szDllPath, sizeof(szDllPath));
	(strrchr(szDllPath, '\\'))[1] = '\0';
	strcat(szDllPath, "HKNetPlugIn.dll");
	HMODULE hMod = LoadLibraryEx(szDllPath,NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
	if (hMod == NULL)
	{
		printf("加载%s失败\n", szDllPath);
		system("pause");
		return 0;
	}
	getHandlerFun getHandler = (getHandlerFun)GetProcAddress(hMod, "getHandler");
	IDVRHandler *pHandler = getHandler();
	//if (!pHandler->connectDVR("10.200.198.152", 8000, "admin", "xinyi@12345", 1))
	//if (!pHandler->connectDVR("10.201.11.27", 8000, "admin", "admin12345", 1))
	if (!pHandler->connectDVR("10.201.133.67", 8000, "admin", "hk666666", 1))
	{
		printf("连接设备失败\n");
		system("pause");
		return 0;
	}
	MediaInf mInf = { 0 };
	pHandler->GetMediaInfo(mInf);
	pHandler->captureStream(stream_cb, NULL, 0);
	while (true)
	{
		int cmd = getchar();
		if (cmd == 'q')
		{
			break;
		}
	}
	pHandler->stopCaptureStream();
	pHandler->disconnectDVR();
	FreeLibrary(hMod);
	system("pause");
    return 0;
}

