
// PlayDemoDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "PlayDemo.h"
#include "PlayDemoDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CPlayDemoDlg 对话框



CPlayDemoDlg::CPlayDemoDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_PLAYDEMO_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPlayDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PLAYWND, m_PlayWnd);
	DDX_Control(pDX, IDC_SLIDER1, m_slider);
	DDX_Control(pDX, IDC_PLAYWND2, m_PlayWnd2);
	DDX_Control(pDX, IDC_PLAYWND3, m_PlayWnd3);
	DDX_Control(pDX, IDC_PLAYWND4, m_PlayWnd4);
}

BEGIN_MESSAGE_MAP(CPlayDemoDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDCANCEL, &CPlayDemoDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_REG, &CPlayDemoDlg::OnBnClickedReg)
	ON_BN_CLICKED(IDC_STARTPLAY, &CPlayDemoDlg::OnBnClickedStartplay)
	ON_BN_CLICKED(IDC_STOPPLAY, &CPlayDemoDlg::OnBnClickedStopplay)
	ON_BN_CLICKED(IDC_REGION, &CPlayDemoDlg::OnBnClickedRegion)
	ON_BN_CLICKED(IDC_POSITION, &CPlayDemoDlg::OnBnClickedPosition)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_BN_CLICKED(IDC_QUERYREC, &CPlayDemoDlg::OnBnClickedQueryrec)
	ON_BN_CLICKED(IDC_PLAYBACK, &CPlayDemoDlg::OnBnClickedPlayback)
	ON_BN_CLICKED(IDC_TEARDWON, &CPlayDemoDlg::OnBnClickedTeardwon)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_GPS, &CPlayDemoDlg::OnBnClickedGps)
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(ID__SplitScreen, &CPlayDemoDlg::SplitScreen)
	ON_COMMAND(ID__SingleSrceen, &CPlayDemoDlg::SingleSrceen)
END_MESSAGE_MAP()


// CPlayDemoDlg 消息处理程序

BOOL CPlayDemoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	memset(&m_PlayRect, 0, sizeof(CRect));
	m_Menu.LoadMenu(IDR_MENU1);

	// TODO: 在此添加额外的初始化代码
	//SetDlgItemText(IDC_ADDR, "10.200.66.100");
	//SetDlgItemText(IDC_PORT, "7050");
	//SetDlgItemText(IDC_USER, "44030500012000000002");
	//SetDlgItemText(IDC_PWD, "xinyi2513");
	//SetDlgItemText(IDC_DEVICEID, "44030500011310973156"); //44030501001310122752 44030504001310197384

	SetDlgItemText(IDC_ADDR, "10.200.67.147");
	SetDlgItemText(IDC_PORT, "5066");
	SetDlgItemText(IDC_USER, "44030500002000000006");
	SetDlgItemText(IDC_PWD, "xinyi2513");
	SetDlgItemText(IDC_DEVICEID, "44030500091320000001"); //44030501001310122752 44030504001310197384 44030500011320000005

	CTime sTime, eTime;
	((CDateTimeCtrl *)GetDlgItem(IDC_STIME))->GetTime(sTime);
	((CDateTimeCtrl *)GetDlgItem(IDC_ETIME))->GetTime(sTime);
	CTimeSpan sSpan(0, 1, 30, 0);
	CTimeSpan eSpan(0, 0, 30, 0);
	CTime sTime2 = sTime - sSpan;
	CTime eTime2 = eTime - eSpan;
	((CDateTimeCtrl *)GetDlgItem(IDC_STIME))->SetTime(&sTime2);
	((CDateTimeCtrl *)GetDlgItem(IDC_ETIME))->SetTime(&eTime2);

	((CEdit *)GetDlgItem(IDC_SPEED))->SetWindowText("5");

	m_PlayWnd2.ShowWindow(SW_HIDE);
	m_PlayWnd3.ShowWindow(SW_HIDE);
	m_PlayWnd4.ShowWindow(SW_HIDE);
	m_slider.ShowWindow(SW_HIDE);

	m_bRegion = FALSE;
	m_bPosition = FALSE;
	m_bMouseMove = FALSE;
	m_nPlayWndIndex = 0;
	m_nLeft = 0;
	m_nTop = 0;
	m_nRight = 0;
	m_nBottom = 0;
	m_px = 0;
	m_py = 0;
	m_hModule = NULL;
	m_pHandler = NULL;
	m_pHandler2 = NULL;
	m_pHandler3 = NULL;
	m_pHandler4 = NULL;
	m_nPlaybackId = 0;
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CPlayDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CPlayDemoDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CPlayDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CPlayDemoDlg::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	m_Menu.DestroyMenu();
	CDialogEx::OnCancel();
}


void CPlayDemoDlg::OnBnClickedReg()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_pHandler)
	{
		return;
	}
	CString sIp, sUser, sPwd, sDeviceID;
	int nPort;
	GetDlgItemText(IDC_ADDR, sIp);
	nPort = GetDlgItemInt(IDC_PORT);
	GetDlgItemText(IDC_USER, sUser);
	GetDlgItemText(IDC_PWD, sPwd);
	GetDlgItemText(IDC_DEVICEID, sDeviceID);

	char szPath[256] = { 0 };
	GetModuleFileName(NULL, szPath, sizeof(szPath));

	(strrchr(szPath, '\\'))[1] = '\0';
	strcat(szPath, "PlugIn\\SIPPlugIn\\SIPNetPlugIn.dll");

	m_hModule = LoadLibraryEx(szPath, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
	if (m_hModule == NULL)
	{
		AfxMessageBox("加载SIPNetPlugIn失败!");
		return;
	}
	typedef IDVRHandler*  (*getHandlerFun)();
	getHandlerFun getHandler = (getHandlerFun)GetProcAddress(m_hModule, "getHandler");
	if (getHandler == NULL)
	{
		AfxMessageBox("加载getHandler函数失败!");
		return;
	}
	m_pHandler = getHandler();
	m_pHandler2 = getHandler();
	m_pHandler3 = getHandler();
	m_pHandler4 = getHandler();
	if (m_pHandler == NULL)
	{
		AfxMessageBox("调用getHandler函数失败!");
		return;
	}
	if (!m_pHandler->connectDVR(sIp.GetBuffer(), nPort, sUser.GetBuffer(), sPwd.GetBuffer(), 1))
	{

	}
	m_pHandler2->connectDVR(sIp.GetBuffer(), nPort, sUser.GetBuffer(), sPwd.GetBuffer(), 1);
	m_pHandler3->connectDVR(sIp.GetBuffer(), nPort, sUser.GetBuffer(), sPwd.GetBuffer(), 1);
	m_pHandler4->connectDVR(sIp.GetBuffer(), nPort, sUser.GetBuffer(), sPwd.GetBuffer(), 1);
	m_pHandler->setCurrentCameraCode(sDeviceID.GetBuffer());
	sIp.ReleaseBuffer();
	sUser.ReleaseBuffer();
	sPwd.ReleaseBuffer();
	sDeviceID.ReleaseBuffer();
}

void CPlayDemoDlg::DrawCb(intptr_t identify, intptr_t lRealHandle, DWORD hdc, void *pUser)
{
	if (hdc == 400)
	{
		CPlayDemoDlg *pDlg = (CPlayDemoDlg *)pUser;
		if (pDlg)
		{
			CString strMsg;
			strMsg.Format("播放结果,ErrorCode:%d!\n", identify);
			pDlg->SetWindowText(strMsg);
		}
	}
}

void CPlayDemoDlg::OnBnClickedStartplay()
{
	// TODO: 在此添加控件通知处理程序代码
	m_slider.ShowWindow(SW_HIDE);
	OnBnClickedReg();
	CString sDeviceID;
	GetDlgItemText(IDC_DEVICEID, sDeviceID);

	if (m_nPlayWndIndex == 0)
	{
		m_pHandler->setCurrentCameraCode(sDeviceID.GetBuffer());
		m_pHandler->startPlayer2(m_PlayWnd.GetSafeHwnd(), DrawCb, this);
		m_pHandler->startPlayerByCamera(m_PlayWnd.GetSafeHwnd(), 12345);
	}
	else if (m_nPlayWndIndex == 1)
	{
		m_pHandler2->setCurrentCameraCode(sDeviceID.GetBuffer());
		m_pHandler2->startPlayer2(m_PlayWnd2.GetSafeHwnd(), DrawCb, this);
		m_pHandler2->startPlayerByCamera(m_PlayWnd2.GetSafeHwnd(), 12345);
	}
			
	else if (m_nPlayWndIndex == 2)
	{
		m_pHandler3->setCurrentCameraCode(sDeviceID.GetBuffer());
		m_pHandler3->startPlayer2(m_PlayWnd3.GetSafeHwnd(), DrawCb, this);
		m_pHandler3->startPlayerByCamera(m_PlayWnd3.GetSafeHwnd(), 12345);
	}
			
	else if (m_nPlayWndIndex == 3)
	{
		m_pHandler4->setCurrentCameraCode(sDeviceID.GetBuffer());
		m_pHandler4->startPlayer2(m_PlayWnd4.GetSafeHwnd(), DrawCb, this);
		m_pHandler4->startPlayerByCamera(m_PlayWnd4.GetSafeHwnd(), 12345);
	}	
}


void CPlayDemoDlg::OnBnClickedStopplay()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_nPlayWndIndex == 0)
	{
		m_pHandler->stopPlayerByCamera(m_PlayWnd.GetSafeHwnd(), 12345);
	}
	else if (m_nPlayWndIndex == 1)
	{
		m_pHandler2->stopPlayerByCamera(m_PlayWnd2.GetSafeHwnd(), 12345);
	}

	else if (m_nPlayWndIndex == 2)
	{
		m_pHandler3->stopPlayerByCamera(m_PlayWnd3.GetSafeHwnd(), 12345);
	}

	else if (m_nPlayWndIndex == 3)
	{
		m_pHandler4->stopPlayerByCamera(m_PlayWnd4.GetSafeHwnd(), 12345);
	}
	Invalidate();
}


void CPlayDemoDlg::OnBnClickedRegion()
{
	// TODO: 在此添加控件通知处理程序代码
	//Invalidate();//父窗体清屏
	//CDC *pDC = GetDC(); //子窗体填屏
	//CRect rc;
	//GetDlgItem(IDC_PLAYWND)->GetWindowRect(&rc);
	//ScreenToClient(&rc);
	//pDC->FillSolidRect(&rc, RGB(0, 0, 0));
	//ReleaseDC(pDC);
	if (!m_bRegion)
	{
		m_bPosition = FALSE;
		m_bRegion = TRUE;
	}
	else
	{
		CRect ct;
		GetDlgItem(IDC_PLAYWND)->GetClientRect(&ct);
		unsigned char nParam[4];
		if (m_pHandler)
		{
			nParam[0] = ct.left / 10;
			nParam[1] = ct.top / 10;
			nParam[2] = ct.right / 10;
			nParam[3] = ct.bottom / 10;
			m_pHandler->controlPTZSpeed(CAMERA_SETP, false, *((int *)nParam));
			m_pHandler->controlPTZSpeed(CAMERA_SETN, false, *((int *)nParam));
		}
		m_bRegion = FALSE;
	}
}


void CPlayDemoDlg::OnBnClickedPosition()
{
	// TODO: 在此添加控件通知处理程序代码
	//Invalidate();//父窗体清屏
	//CDC *pDC = GetDC(); //子窗体填屏
	//CRect rc;
	//GetDlgItem(IDC_PLAYWND)->GetWindowRect(&rc);
	//ScreenToClient(&rc);
	//pDC->FillSolidRect(&rc, RGB(0, 0, 0));
	//ReleaseDC(pDC);
	if (!m_bPosition)
	{
		m_bRegion = FALSE;
		m_bPosition = TRUE;
	}
	else
	{
		CRect ct;
		GetDlgItem(IDC_PLAYWND)->GetClientRect(&ct);
		unsigned char nParam[4];
		if (m_pHandler)
		{
			nParam[0] = ct.left / 10;
			nParam[1] = ct.top / 10;
			nParam[2] = ct.right / 10;
			nParam[3] = ct.bottom / 10;
			m_pHandler->controlPTZSpeed(CAMERA_3D_FIRST, false, *((int *)nParam));
			m_pHandler->controlPTZSpeed(CAMERA_3D_SECOND, false, *((int *)nParam));
		}
		m_bPosition = FALSE;
	}
}


void CPlayDemoDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	ClientToScreen(&point);
	CRect rect;
	GetDlgItem(IDC_PLAYWND)->GetWindowRect(&rect);
	if (rect.PtInRect(point))
	{
		m_nLeft = point.x - rect.left;
		m_nTop = point.y - rect.top;
		m_px = point.x;
		m_py = point.y;
		m_bMouseMove = TRUE;
		m_nPlayWndIndex = 0;
	}
	GetDlgItem(IDC_PLAYWND2)->GetWindowRect(&rect);
	if (rect.PtInRect(point))
	{
		m_nLeft = point.x - rect.left;
		m_nTop = point.y - rect.top;
		m_px = point.x;
		m_py = point.y;
		m_bMouseMove = TRUE;
		m_nPlayWndIndex = 1;
	}
	GetDlgItem(IDC_PLAYWND3)->GetWindowRect(&rect);
	if (rect.PtInRect(point))
	{
		m_nLeft = point.x - rect.left;
		m_nTop = point.y - rect.top;
		m_px = point.x;
		m_py = point.y;
		m_bMouseMove = TRUE;
		m_nPlayWndIndex = 2;
	}
	GetDlgItem(IDC_PLAYWND4)->GetWindowRect(&rect);
	if (rect.PtInRect(point))
	{
		m_nLeft = point.x - rect.left;
		m_nTop = point.y - rect.top;
		m_px = point.x;
		m_py = point.y;
		m_bMouseMove = TRUE;
		m_nPlayWndIndex = 3;
	}
	CDialogEx::OnLButtonDown(nFlags, point);
}


void CPlayDemoDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	ClientToScreen(&point);
	CRect rect;
	GetDlgItem(IDC_PLAYWND)->GetWindowRect(&rect);
	if (rect.PtInRect(point))
	{
		m_nRight = point.x - rect.left;
		m_nBottom = point.y - rect.top;
		m_bMouseMove = FALSE;

		CRect ct;
		GetDlgItem(IDC_PLAYWND)->GetClientRect(&ct);
		unsigned char nParam[4];
		if (m_bRegion)
		{
			if (m_pHandler)
			{
				nParam[0] = ct.left / 10;
				nParam[1] = ct.top / 10;
				nParam[2] = ct.right / 10;
				nParam[3] = ct.bottom / 10;
				m_pHandler->controlPTZSpeed(CAMERA_SETP, false, *((int *)nParam));
				nParam[0] = m_nLeft / 10;
				nParam[1] = m_nTop / 10;
				nParam[2] = m_nRight / 10;
				nParam[3] = m_nBottom / 10;
				m_pHandler->controlPTZSpeed(CAMERA_SETN, false, *((int *)nParam));
			}
		}
		else if (m_bPosition)
		{
			nParam[0] = ct.left / 10;
			nParam[1] = ct.top / 10;
			nParam[2] = ct.right / 10;
			nParam[3] = ct.bottom / 10;
			m_pHandler->controlPTZSpeed(CAMERA_3D_FIRST, false, *((int *)nParam));
			nParam[0] = m_nLeft / 10;
			nParam[1] = m_nTop / 10;
			nParam[2] = m_nRight / 10;
			nParam[3] = m_nBottom / 10;
			m_pHandler->controlPTZSpeed(CAMERA_3D_SECOND, false, *((int *)nParam));
		}

	
	}
	CDialogEx::OnLButtonUp(nFlags, point);
}


void CPlayDemoDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	ClientToScreen(&point);
	CRect rect, prect;
	GetDlgItem(IDC_PLAYWND)->GetWindowRect(&rect);
	if (rect.PtInRect(point) && m_bMouseMove)
	{
		prect.left = m_px < point.x ? m_px : point.x;
		prect.right = m_px < point.x ? point.x : m_px;
		prect.top = m_py < point.y ? m_py : point.y;
		prect.bottom = m_py < point.y ? point.y : m_py;
		
		ScreenToClient(&prect);

		if (m_bRegion)
		{
			CPen pen(PS_SOLID, 1, RGB(255, 0, 0));
			CDC *dc = GetDC();
			CPen *oldpen = dc->SelectObject(&pen);
			MoveToEx(dc->m_hDC, prect.left, prect.top, NULL);
			LineTo(dc->m_hDC, prect.right, prect.top);
			LineTo(dc->m_hDC, prect.right, prect.bottom);
			LineTo(dc->m_hDC, prect.left, prect.bottom);
			LineTo(dc->m_hDC, prect.left, prect.top);
			dc->SelectObject(oldpen);
			ReleaseDC(dc);
		}
		else if (m_bPosition)
		{
			CPen pen(PS_SOLID, 1, RGB(0, 0, 255));
			CDC *dc = GetDC();
			CPen *oldpen = dc->SelectObject(&pen);
			MoveToEx(dc->m_hDC, prect.left, prect.top, NULL);
			LineTo(dc->m_hDC, prect.right, prect.top);
			LineTo(dc->m_hDC, prect.right, prect.bottom);
			LineTo(dc->m_hDC, prect.left, prect.bottom);
			LineTo(dc->m_hDC, prect.left, prect.top);
			dc->SelectObject(oldpen);
			ReleaseDC(dc);
		}
	}
	CDialogEx::OnMouseMove(nFlags, point);
}


void CPlayDemoDlg::OnBnClickedQueryrec()
{
	// TODO: 在此添加控件通知处理程序代码
	CTime sdate, stime, edate, etime;
	((CDateTimeCtrl *)GetDlgItem(IDC_SDATE))->GetTime(sdate);
	((CDateTimeCtrl *)GetDlgItem(IDC_STIME))->GetTime(stime);
	((CDateTimeCtrl *)GetDlgItem(IDC_ETIME))->GetTime(edate);
	((CDateTimeCtrl *)GetDlgItem(IDC_EDATE))->GetTime(etime);
	CString sBeginTime, sEndTime;
	sBeginTime.Format("%s %s", sdate.Format("%Y-%m-%d"), stime.Format("%H:%M:%S"));
	sEndTime.Format("%s %s", edate.Format("%Y-%m-%d"), etime.Format("%H:%M:%S"));
	if (m_pHandler)
	{
		RECORDFILE recfile[100] = { 0 };
		int nCount = m_pHandler->getRecordFileEx(0, sBeginTime.GetBuffer(), sEndTime.GetBuffer(), recfile, 100);
		if (nCount <= 0)
		{
			AfxMessageBox("没有录像文件！");
		}
		CString sMsg;
		sMsg.Format("搜索到录像文件%d个！", nCount);
		AfxMessageBox(sMsg);
	}
}


void CPlayDemoDlg::OnBnClickedPlayback()
{
	// TODO: 在此添加控件通知处理程序代码

	OnBnClickedReg();
	CString sDeviceID;
	GetDlgItemText(IDC_DEVICEID, sDeviceID);

	CTime sdate, stime, edate, etime;
	((CDateTimeCtrl *)GetDlgItem(IDC_SDATE))->GetTime(sdate);
	((CDateTimeCtrl *)GetDlgItem(IDC_STIME))->GetTime(stime);
	((CDateTimeCtrl *)GetDlgItem(IDC_ETIME))->GetTime(edate);
	((CDateTimeCtrl *)GetDlgItem(IDC_EDATE))->GetTime(etime);
	CString sBeginTime, sEndTime;
	sBeginTime.Format("%s %s", sdate.Format("%Y-%m-%d"), stime.Format("%H:%M:%S"));
	sEndTime.Format("%s %s", edate.Format("%Y-%m-%d"), etime.Format("%H:%M:%S"));
	if (m_pHandler)
	{
		m_pHandler->setCurrentCameraCode(sDeviceID.GetBuffer());
		m_nPlaybackId = m_pHandler->replayRecordFile(m_PlayWnd, NULL, sBeginTime.GetBuffer(), sEndTime.GetBuffer(), 0);
		if (m_nPlaybackId <=0 )
		{
			AfxMessageBox("回放录像文件失败！");
		}
		SetTimer(m_nPlaybackId, 1000, NULL);
		__time64_t nTatolTime = etime.GetTime() - stime.GetTime();
		m_slider.SetRange(0, nTatolTime);
		m_slider.ShowWindow(SW_SHOW);
		sDeviceID.ReleaseBuffer();
		sBeginTime.ReleaseBuffer();
		sEndTime.ReleaseBuffer();
	}
}


void CPlayDemoDlg::OnBnClickedTeardwon()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_pHandler)
	{
		m_pHandler->stopReplayRecordFile(m_nPlaybackId);
		KillTimer(m_nPlaybackId);
		m_nPlaybackId = 0;
	}
	Invalidate();
}


void CPlayDemoDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (nIDEvent == m_nPlaybackId)
	{
		if (m_pHandler)
		{
			int nOutValue = 0;
			m_pHandler->controlReplayRecordFile(m_nPlaybackId, REPLAY_CTRL_PLAYGETTIME, 0, &nOutValue);
			m_slider.SetPos(nOutValue);
		}
	}
	CDialogEx::OnTimer(nIDEvent);
}


BOOL CPlayDemoDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if (pMsg->message == WM_LBUTTONUP)
	{
		if (m_nCmd > 0)
		{
			UINT nSpeed = GetDlgItemInt(IDC_SPEED);
			if (m_pHandler)
				m_pHandler->controlPTZSpeed(m_nCmd, true, nSpeed);
			m_nCmd = 0;
		}
		if (pMsg->hwnd == m_slider.m_hWnd)
		{
			if (m_pHandler)
			{
				int nTimePos = m_slider.GetPos();
				int nOutValue = 0;
				m_pHandler->controlReplayRecordFile(m_nPlaybackId, REPLAY_CTRL_PLAYSETTIME, nTimePos, &nOutValue);
			}
		}
	}
	else if (pMsg->message == WM_LBUTTONDOWN)
	{
		if (pMsg->hwnd == GetDlgItem(IDC_UP)->m_hWnd)
		{
			UINT nSpeed = GetDlgItemInt(IDC_SPEED);
			m_nCmd = CAMERA_PAN_UP;
			if (m_pHandler)
				m_pHandler->controlPTZSpeed(CAMERA_PAN_UP, false, nSpeed);
		}
		else if (pMsg->hwnd == GetDlgItem(IDC_DOWN)->m_hWnd)
		{
			UINT nSpeed = GetDlgItemInt(IDC_SPEED);
			m_nCmd = CAMERA_PAN_DOWN;
			if (m_pHandler)
				m_pHandler->controlPTZSpeed(CAMERA_PAN_DOWN, false, nSpeed);
		}
		else if (pMsg->hwnd == GetDlgItem(IDC_LEFT)->m_hWnd)
		{
			UINT nSpeed = GetDlgItemInt(IDC_SPEED);
			m_nCmd = CAMERA_PAN_LEFT;
			if (m_pHandler)
				m_pHandler->controlPTZSpeed(CAMERA_PAN_LEFT, false, nSpeed);
		}
		else if (pMsg->hwnd == GetDlgItem(IDC_RIGHT)->m_hWnd)
		{
			UINT nSpeed = GetDlgItemInt(IDC_SPEED);
			m_nCmd = CAMERA_PAN_RIGHT;
			if (m_pHandler)
				m_pHandler->controlPTZSpeed(CAMERA_PAN_RIGHT, false, nSpeed);
		}
		else if (pMsg->hwnd == GetDlgItem(IDC_LU2)->m_hWnd)
		{
			UINT nSpeed = GetDlgItemInt(IDC_SPEED);
			m_nCmd = CAMERA_PAN_LU;
			if (m_pHandler)
				m_pHandler->controlPTZSpeed(CAMERA_PAN_LU, false, nSpeed);
		}
		else if (pMsg->hwnd == GetDlgItem(IDC_RU2)->m_hWnd)
		{
			UINT nSpeed = GetDlgItemInt(IDC_SPEED);
			m_nCmd = CAMERA_PAN_RU;
			if (m_pHandler)
				m_pHandler->controlPTZSpeed(CAMERA_PAN_RU, false, nSpeed);
		}
		else if (pMsg->hwnd == GetDlgItem(IDC_LD2)->m_hWnd)
		{
			UINT nSpeed = GetDlgItemInt(IDC_SPEED);
			m_nCmd = CAMERA_PAN_LD;
			if (m_pHandler)
				m_pHandler->controlPTZSpeed(CAMERA_PAN_LD, false, nSpeed);
		}
		else if (pMsg->hwnd == GetDlgItem(IDC_RD2)->m_hWnd)
		{
			UINT nSpeed = GetDlgItemInt(IDC_SPEED);
			m_nCmd = CAMERA_PAN_RD;
			if (m_pHandler)
				m_pHandler->controlPTZSpeed(CAMERA_PAN_RD, false, nSpeed);
		}
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}


void CPlayDemoDlg::OnBnClickedGps()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_pHandler)
	{
		m_pHandler->alarm_startCapture(2, NULL);
	}
}


void CPlayDemoDlg::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (m_PlayRect.Width() == 0 && m_PlayRect.Height() == 0)
	{
		GetDlgItem(IDC_PLAYWND)->GetWindowRect(&m_PlayRect);
	}
	ClientToScreen(&point);
	if (m_PlayRect.PtInRect(point))
	{
		m_Menu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_VERTICAL, point.x, point.y, this);
	}
	CDialogEx::OnRButtonDown(nFlags, point);
}


void CPlayDemoDlg::SplitScreen()
{
	// TODO: 在此添加命令处理程序代码
	if (!m_bSplit)
	{
		CRect Rect;
		GetDlgItem(IDC_PLAYWND)->GetWindowRect(&Rect);
		ScreenToClient(&Rect);
		WORD iWidth, iHeight;
		iWidth = Rect.Width();
		iHeight = Rect.Height();
		m_PlayWnd.MoveWindow(Rect.left, Rect.top, iWidth / 2, iHeight / 2);
		m_PlayWnd.ShowWindow(SW_SHOW);
		m_PlayWnd2.MoveWindow(Rect.left + (iWidth / 2) + 1, Rect.top, iWidth / 2, iHeight / 2);
		m_PlayWnd2.ShowWindow(SW_SHOW);
		m_PlayWnd3.MoveWindow(Rect.left, Rect.top + (iHeight / 2), iWidth / 2, iHeight / 2);
		m_PlayWnd3.ShowWindow(SW_SHOW);
		m_PlayWnd4.MoveWindow(Rect.left + (iWidth / 2) + 1, Rect.top + (iHeight / 2), iWidth / 2, iHeight / 2);
		m_PlayWnd4.ShowWindow(SW_SHOW);
		Invalidate();
		UpdateWindow();
		m_bSplit = true;
	}
}


void CPlayDemoDlg::SingleSrceen()
{
	// TODO: 在此添加命令处理程序代码
	if (m_bSplit)
	{
		CRect Rect;
		GetDlgItem(IDC_PLAYWND)->GetWindowRect(&Rect);
		ScreenToClient(&Rect);
		WORD iWidth, iHeight;
		iWidth = Rect.Width() * 2;
		iHeight = Rect.Height() * 2;
		m_PlayWnd.MoveWindow(Rect.left, Rect.top, iWidth, iHeight);
		m_PlayWnd.ShowWindow(SW_SHOW);
		m_PlayWnd2.ShowWindow(SW_HIDE);
		m_PlayWnd3.ShowWindow(SW_HIDE);
		m_PlayWnd4.ShowWindow(SW_HIDE);
		Invalidate();
		UpdateWindow();
		m_bSplit = false;
	}
}
