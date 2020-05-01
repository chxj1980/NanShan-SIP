
// PlayDemoDlg.h : 头文件
//

#pragma once
#include "IDVRHandler_x64.h"
#include "afxwin.h"
#include "afxcmn.h"

// CPlayDemoDlg 对话框
class CPlayDemoDlg : public CDialogEx
{
// 构造
public:
	CPlayDemoDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PLAYDEMO_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	static void CALLBACK DrawCb(intptr_t identify, intptr_t lRealHandle, DWORD hdc, void *pUser);
	CStatic		m_PlayWnd;
	CStatic		m_PlayWnd2;
	CStatic		m_PlayWnd3;
	CStatic		m_PlayWnd4;
	CSliderCtrl m_slider;
	CRect		m_PlayRect;
	CMenu		m_Menu;
	BOOL m_bRegion;
	BOOL m_bPosition;
	BOOL m_bMouseMove;
	BOOL m_bSplit;
	int	 m_nLeft;
	int	 m_nTop;
	int  m_nRight;
	int  m_nBottom;
	int  m_nCmd;
	int	 m_nPlayWndIndex;
	LONG m_px;
	LONG m_py;
	HMODULE	m_hModule;
	IDVRHandler *m_pHandler;
	IDVRHandler *m_pHandler2;
	IDVRHandler *m_pHandler3;
	IDVRHandler *m_pHandler4;
	intptr_t m_nPlaybackId;

public:
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedReg();
	afx_msg void OnBnClickedStartplay();
	afx_msg void OnBnClickedStopplay();
	afx_msg void OnBnClickedRegion();
	afx_msg void OnBnClickedPosition();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnBnClickedQueryrec();
	afx_msg void OnBnClickedPlayback();
	afx_msg void OnBnClickedTeardwon();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedGps();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void SplitScreen();
	afx_msg void SingleSrceen();
};
