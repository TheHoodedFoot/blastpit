
// ccLaserDlg.h : header file
//

#pragma once
#include "lmosactxctrl1.h"


// CccLaserDlg dialog
class CccLaserDlg : public CDialog
{
// Construction
public:
	CccLaserDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_CCLASER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CLmosactxctrl1 ccControl;
	afx_msg void OnBnClickedButton1();
};
