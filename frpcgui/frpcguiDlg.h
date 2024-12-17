
// frpcguiDlg.h : ��� ����
//

#pragma once
#include "afxwin.h"


// CfrpcguiDlg ��ȭ ����
class CfrpcguiDlg : public CDialogEx
{
// �����Դϴ�.
public:
	CfrpcguiDlg(CWnd* pParent = NULL);	// ǥ�� �������Դϴ�.

// ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FRPCGUI_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �����Դϴ�.


// �����Դϴ�.
protected:
	HICON m_hIcon;
	NOTIFYICONDATA m_trayIconData; // Ʈ���� ������ ������

	// ������ �޽��� �� �Լ�
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	// ������� ����
	CString m_deviceName;	// ��ġ��(���Ͻ� �̸�)
	CString m_serverAddr;	// ���� �ּ�
	int m_serverPort;		// ���� ��Ʈ
	CString m_token;		// ���� ��ū
	int m_localPort;		// ���� ��Ʈ
	int m_remotePort;		// ���� ��Ʈ
	CEdit m_edit_output;	// ���â ��Ʈ��
	BOOL m_autoStart;		// �ڵ�����üũ ��Ʈ��

	// ����Լ� ����
	afx_msg LRESULT OnTrayIconNotify(WPARAM wParam, LPARAM lParam);
	afx_msg void OnContextMenuShowWindow();
	afx_msg void OnContextMenuExit();
	afx_msg void OnDestroy();
	afx_msg void OnClose();
	void LoadConf();
	void SaveConf();
	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnBnClickedButtonStop();
};