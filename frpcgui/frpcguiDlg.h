
// frpcguiDlg.h : ��� ����
//

#pragma once
#include "afxwin.h"
#include <string>

// ����� ���� �޽���
#define WM_USER_TRAY_ICON (WM_USER + 100)	// Ʈ���� ������ �����
#define WM_USER_HIDE_WINDOW (WM_USER + 101)	// â ������
#define WM_USER_LOG_LINE (WM_USER + 102)	// �α׶��� ���ſ�

// ��Ʈ ��õ� �ִ� Ƚ��
#define MAX_RETRIES 30

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
	TCHAR m_szTargetFile[MAX_PATH];		// frpc.exe ���� ���
	PROCESS_INFORMATION m_frpcProcess;	// ����� frpc ���μ���
	HANDLE m_hChildStd_OUT_Rd;			// stdout �����̷�Ʈ ������ �ڵ�(Read)
	HANDLE m_hChildStd_OUT_Wr;			// stdout �����̷�Ʈ ������ �ڵ�(Write)
	CWinThread* m_pLogThread;			// �α� �б�� ������ �ڵ�
	CEvent m_stopEvent;		// ������ ���� ��û �̺�Ʈ
	int m_retryCount;		// ��Ʈ ��õ� ���� ����
	BOOL m_successStarted;	// ���Ͻ� ���� ���� ����

	// ����Լ� ����
	afx_msg LRESULT OnTrayIconNotify(WPARAM wParam, LPARAM lParam); // Ʈ���� ������
	afx_msg LRESULT OnHideWindow(WPARAM wParam, LPARAM lParam);		// â �����
	afx_msg void OnContextMenuShowWindow();	// Ʈ���� ���ؽ�Ʈ �޴��� â���̱�
	afx_msg void OnContextMenuExit();		// Ʈ���� ���ؽ�Ʈ �޴��� ����
	afx_msg void OnDestroy();	// ���� ó��
	afx_msg void OnClose();		// �ݱ� ó��
	void LoadConf();			// �������� �б�
	void SaveConf();			// �������� ����
	afx_msg void OnBnClickedButtonStart();	// ���� ��ư
	afx_msg void OnBnClickedButtonStop();	// ���� ��ư
	std::wstring Trim(const std::wstring & str);	// WhiteSpace ���� �Լ�
	void XorData(BYTE * pData, DWORD dwSize, const BYTE * pKey, size_t keyLen);	// XOR ��ȣȭ �Լ�
	BOOL ExtractAndRunEncryptedExeFromResource();	// ���ҽ����� XOR ��ȣȭ�� �����͸� �ε��ϴ� �Լ�
	BOOL StartFrpcProcess(BOOL isRetry);	// frpc ���μ��� ���� �Լ�
	void StopFrpcProcess();					// frpc ���μ��� ���� �Լ�
	static UINT LogReaderThread(LPVOID pParam);		// frpc ��� �б� ������
	afx_msg LRESULT OnLogLine(WPARAM wParam, LPARAM lParam);		// �α� ���
	int GetValidPortOrRandom(int desiredPort);		// ��Ʈ ��ȿ�� �˻� �� ���� ��Ʈ ���� �Լ�
	void GenerateFrpcIni(int remotePort, const CString& proxyName);	// frpc.ini ���� �Լ�
	CString GenerateProxyName();	// ���� ���Ͻ� �̸� ���� �Լ�
	void CfrpcguiDlg::AddLogMessage(const CString& strMessage);		// �α� �޽��� ��� �Լ�
};
