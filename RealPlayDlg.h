// RealPlayDlg.h : header file
//

#if !defined(AFX_REALPLAYDLG_H__4C5D1B67_AC1D_499C_A88A_C0B663F739D0__INCLUDED_)
#define AFX_REALPLAYDLG_H__4C5D1B67_AC1D_499C_A88A_C0B663F739D0__INCLUDED_

#define  PI  3.14159265358979323846264338327950288419716939937510
/*#define */ 


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GeneralDef.h"
#include "PTZButton.h"
#include "afxwin.h"
#include "cv.h"
#include "highgui.h"
#include <ctype.h>

/////////////////////////////////////////////////////////////////////////////
// CRealPlayDlg dialog

class CRealPlayDlg : public CDialog
{
// Construction
public:
	CRealPlayDlg(CWnd* pParent = NULL);	// standard constructor

public:
	BOOL m_bTrackRun;
	BOOL m_bIsOnCruise;
	void InitDecoderReferCtrl();
	void GetDecoderCfg();
	void StopRecord();
	void StartRecord();
	int GetPTZSpeed();
	LONG GetPlayHandle();
	int GetCurChanIndex();
	void DbPlayChannel(int ChanIndex);
	void CreateDeviceTree();
	void StartPlay(int iChanIndex);
	void StopPlay();
	BOOL DoLogin();
	void CALLBACK DecCBFun();
	afx_msg void CRealPlayDlg::IpcameraMove();
	afx_msg void CRealPlayDlg::CountPoint();
	afx_msg double CRealPlayDlg::ComputeAngle(CPoint nowpoint);
	afx_msg double CRealPlayDlg::PointLegth(CPoint pa, CPoint pb);
	
	void DoGetDeviceResoureCfg();
	BOOL m_bIsLogin;
	BOOL m_bIsPlaying;
	BOOL m_bIsPlaying1;
	BOOL m_bIsRecording;
	HTREEITEM m_hDevItem;
	LOCAL_DEVICE_INFO m_struDeviceInfo;
	LOCAL_DEVICE_INFO m_struDeviceInfo1;

	int m_iCurChanIndex;               //当前通道在数组中索引
	LONG m_lPlayHandle;
	LONG m_lPlayHandle1;


	//下列按钮的顺序一定要和CPTZButton中的处理一致
	CPTZButton m_btnPtzUp;
	CPTZButton m_btnPtzDown;
	CPTZButton m_btnPtzLeft;
	CPTZButton m_btnPtzRight;
	CPTZButton m_btnZoomIn;//调焦拉近,放大
	CPTZButton m_btnZoomOut;//调焦拉远,缩小
	CPTZButton m_btnFocusNear;//聚焦拉近,放大
	CPTZButton m_btnFocusFar;//聚焦拉远,缩小
	CPTZButton m_btnIrisOpen;//光圈放大
	CPTZButton m_btnIrisClose;//光圈缩小
	CPTZButton m_btnPtzUpleft; //左上
	CPTZButton m_btnPtzUpright; //右上
	CPTZButton m_btnPtzDownleft; //左下
	CPTZButton m_btnPtzDownright; //右下

	BOOL m_bAutoOn;       //自动开
    BOOL m_bLightOn;      //灯光开
    BOOL m_bWiperOn;      //雨刷开
	BOOL m_bFanOn;        //风扇开
	BOOL m_bHeaterOn;     //加热开
	BOOL m_bAuxOn1;        //辅助设备1开
	BOOL m_bAuxOn2;        //辅助设备2开
// Dialog Data
	//{{AFX_DATA(CRealPlayDlg)
	enum { IDD = IDD_REALPLAY_DIALOG };
	CComboBox	m_coJpgQuality;
	CComboBox	m_coJpgSize;
	CComboBox	m_coPicType;
	CComboBox	m_comboSeq;
	CComboBox	m_comboPreset;
	CComboBox	m_comboPTZSpeed;
	CTreeCtrl	m_ctrlTreeChan;
	CIPAddressCtrl	m_ctrlDevIp;
	UINT	m_nDevPort;
	CString	m_csUser;
	CString	m_csPWD;
	
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRealPlayDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CRealPlayDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButtonLogin();
	afx_msg void OnDblclkTreeChan(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangedTreeChan(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButtonRecord();
	afx_msg void OnButtonCapture();
	afx_msg void OnButtonPlay();
	afx_msg void OnSelchangeComboPreset();
	afx_msg void OnButtonPresetGoto();
	afx_msg void OnButtonPresetSet();
	afx_msg void OnButtonPresetDel();
	afx_msg void OnButtonSeqGoto();
	afx_msg void OnButtonSeqSet();
	afx_msg void OnButtonTrackRun();
	afx_msg void OnButtonTrackStart();
	afx_msg void OnButtonTrackStop();
	afx_msg void OnBtnPtzAuto();
	afx_msg void OnBtnFanPwron();
	afx_msg void OnSelchangeComboPicType();
	afx_msg void OnClose();
	afx_msg void ShowEndImage();

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnIpnFieldchangedIpaddressDev(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButton1();
	CStatic m_showRectangle;
protected:
	afx_msg LRESULT OnShowEndImage(WPARAM wParam, LPARAM lParam);

public:

	void ReSize(void);
	POINT old;
	bool flag_First;
	POINT OldPoint[1000]; //获取现在对话框的大小  
	afx_msg void OnSize(UINT nType, int cx, int cy);

	afx_msg void OnBnClickedButton2();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnBnClickedButton3();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
protected:
	afx_msg LRESULT OnTestShowipc(WPARAM wParam, LPARAM lParam);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_REALPLAYDLG_H__4C5D1B67_AC1D_499C_A88A_C0B663F739D0__INCLUDED_)
