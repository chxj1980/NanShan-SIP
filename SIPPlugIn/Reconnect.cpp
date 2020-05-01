#include "stdafx.h"
#include "Reconnect.h"


Reconnect::Reconnect(SIPHandler *pParam)
{
	pHandle = pParam;
}


Reconnect::~Reconnect()
{
}

void * Reconnect::run()
{
	if (pHandle)
	{
		if (pHandle->m_pStramCallback)
		{
			LPCAPTURESTREAMCALLBACK pCb = pHandle->m_pStramCallback;
			void *pUser = pHandle->m_pStreamUser;
			pHandle->stopCaptureStream();
			pHandle->captureStream(pCb, pUser, 0);
		}
#ifdef _PLAY_PLUGIN
		HWND hWnd = pHandle->m_hWnd;
		int nChannel = pHandle->m_nChannel;
		LPDRAWCBCALLLBACK pCb = pHandle->m_pDrawCallback;
		void *pUser = pHandle->m_pDrawUser;
		pHandle->stopPlayerByCamera(hWnd, nChannel);
		pHandle->startPlayer2(hWnd, pCb, pUser);
		pHandle->startPlayerByCamera(hWnd, nChannel);
#endif
	}
	delete this;
	return NULL;
}
