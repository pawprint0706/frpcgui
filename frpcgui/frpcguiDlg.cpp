
// frpcguiDlg.cpp : ���� ����
//

#include "stdafx.h"
#include "frpcgui.h"
#include "frpcguiDlg.h"
#include "afxdialogex.h"
#include <vector>
#include <map>
#include <sstream>
#include <VersionHelpers.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ���� ���α׷� ������ ���Ǵ� CAboutDlg ��ȭ �����Դϴ�.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

														// �����Դϴ�.
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


// CfrpcguiDlg ��ȭ ����


CfrpcguiDlg::CfrpcguiDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_FRPCGUI_DIALOG, pParent)
	, m_deviceName(_T("��ġ�̸��Է�"))
	, m_serverAddr(_T("127.0.0.1"))
	, m_serverPort(7000)
	, m_token(_T("12345"))
	, m_localPort(9500)
	, m_remotePort(9999)
	, m_autoStart(FALSE)
	, m_retryCount(0)
	, m_successStarted(FALSE)
	, m_stopEvent(TRUE, FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_frpcProcess.hProcess = NULL;
	m_frpcProcess.hThread = NULL;
	m_hChildStd_OUT_Rd = NULL;
	m_hChildStd_OUT_Wr = NULL;
}


void CfrpcguiDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_DEVICE_NAME, m_deviceName);
	DDV_MaxChars(pDX, m_deviceName, 256);
	DDX_Text(pDX, IDC_EDIT_SERVER_ADDRESS, m_serverAddr);
	DDV_MaxChars(pDX, m_serverAddr, 256);
	DDX_Text(pDX, IDC_EDIT_SERVER_PORT, m_serverPort);
	DDV_MinMaxInt(pDX, m_serverPort, 1, 65535);
	DDX_Text(pDX, IDC_EDIT_AUTH_TOKEN, m_token);
	DDV_MaxChars(pDX, m_token, 256);
	DDX_Text(pDX, IDC_EDIT_LOCAL_PORT, m_localPort);
	DDV_MinMaxInt(pDX, m_localPort, 1, 65535);
	DDX_Text(pDX, IDC_EDIT_REMOTE_PORT, m_remotePort);
	DDV_MinMaxInt(pDX, m_remotePort, 1, 65535);
	DDX_Control(pDX, IDC_EDIT_OUTPUT, m_edit_output);
	DDX_Check(pDX, IDC_CHECK_AUTO_START, m_autoStart);
}


BEGIN_MESSAGE_MAP(CfrpcguiDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_MESSAGE(WM_USER_TRAY_ICON, &CfrpcguiDlg::OnTrayIconNotify) // ����� ���� �޽���
	ON_COMMAND(ID_CONTEXTMENU_SHOW_WINDOW, &CfrpcguiDlg::OnContextMenuShowWindow)
	ON_COMMAND(ID_CONTEXTMENU_EXIT, &CfrpcguiDlg::OnContextMenuExit)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_START, &CfrpcguiDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CfrpcguiDlg::OnBnClickedButtonStop)
	ON_MESSAGE(WM_USER_LOG_LINE, &CfrpcguiDlg::OnLogLine)
END_MESSAGE_MAP()


// CfrpcguiDlg �޽��� ó����

BOOL CfrpcguiDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// �ý��� �޴��� "����..." �޴� �׸��� �߰��մϴ�.

	// IDM_ABOUTBOX�� �ý��� ��� ������ �־�� �մϴ�.
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

	// �� ��ȭ ������ �������� �����մϴ�.  ���� ���α׷��� �� â�� ��ȭ ���ڰ� �ƴ� ��쿡��
	//  �����ӿ�ũ�� �� �۾��� �ڵ����� �����մϴ�.
	SetIcon(m_hIcon, TRUE);			// ū �������� �����մϴ�.
	SetIcon(m_hIcon, FALSE);		// ���� �������� �����մϴ�.

									// Ʈ���� ������ ����
	m_trayIconData.cbSize = sizeof(NOTIFYICONDATA);
	m_trayIconData.hWnd = this->GetSafeHwnd();
	m_trayIconData.uID = 1; // Ʈ���� ������ ID
	m_trayIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	m_trayIconData.uCallbackMessage = WM_USER_TRAY_ICON; // ����� ���� �޽���
	m_trayIconData.hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));
	wcscpy_s(m_trayIconData.szTip, _T("FRP GUI Client"));
	// Ʈ���� ������ ǥ��
	Shell_NotifyIcon(NIM_ADD, &m_trayIconData);
	
	// �������� �ҷ�����
	LoadConf();

	// ��ư ��Ʈ�� ��Ȱ��ȭ
	GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(FALSE);

	// ���� �ε� ���� �� auto_start ���� TRUE�� �ڵ� ���� �õ�
	if (m_autoStart)
	{
		// OnBnClickedButtonStart() ȣ��
		PostMessage(WM_COMMAND, MAKEWPARAM(IDC_BUTTON_START, BN_CLICKED), (LPARAM)GetDlgItem(IDC_BUTTON_START)->m_hWnd);
	}

	return TRUE;  // ��Ŀ���� ��Ʈ�ѿ� �������� ������ TRUE�� ��ȯ�մϴ�.
}

void CfrpcguiDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// ��ȭ ���ڿ� �ּ�ȭ ���߸� �߰��� ��� �������� �׸�����
//  �Ʒ� �ڵ尡 �ʿ��մϴ�.  ����/�� ���� ����ϴ� MFC ���� ���α׷��� ��쿡��
//  �����ӿ�ũ���� �� �۾��� �ڵ����� �����մϴ�.

void CfrpcguiDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // �׸��⸦ ���� ����̽� ���ؽ�Ʈ�Դϴ�.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Ŭ���̾�Ʈ �簢������ �������� ����� ����ϴ�.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// �������� �׸��ϴ�.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}


// ����ڰ� �ּ�ȭ�� â�� ���� ���ȿ� Ŀ���� ǥ�õǵ��� �ý��ۿ���
//  �� �Լ��� ȣ���մϴ�.
HCURSOR CfrpcguiDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


LRESULT CfrpcguiDlg::OnTrayIconNotify(WPARAM wParam, LPARAM lParam)
{
	if (wParam == m_trayIconData.uID)
	{
		CMenu menu;
		CPoint point;
		switch (lParam)
		{
		case WM_LBUTTONDBLCLK: // �´���Ŭ�� �̺�Ʈ �߻� ��
			OnContextMenuShowWindow();
			break;
		case WM_RBUTTONUP: // ��Ŭ�� �̺�Ʈ �߻� ��
			// Ŀ�� ��ġ ã��
			GetCursorPos(&point);
			// ���ؽ�Ʈ �޴� �ε�
			if (menu.LoadMenu(IDR_MENU_CONTEXT_MENU))
			{
				CMenu* pSubMenu = menu.GetSubMenu(0);
				if (pSubMenu)
				{
					// SetForegroundWindow�� ȣ���� �˾� �޴��� ������ �ʵ��� ����
					SetForegroundWindow();

					// Ʈ���� �޴� ǥ�� �� ���õ� ��� ID ��ȯ
					int nCmd = pSubMenu->TrackPopupMenu(TPM_RETURNCMD | TPM_RIGHTBUTTON, point.x, point.y, this);

					// ��ȯ�� ��� ID�� ���� �ڵ鷯 ����
					switch (nCmd)
					{
					case ID_CONTEXTMENU_SHOW_WINDOW:
						OnContextMenuShowWindow();
						break;
					case ID_CONTEXTMENU_EXIT:
						OnContextMenuExit();
						break;
					default:
						break;
					}
				}
			}
			break;
		default:
			break;
		}
	}
	return 0;
}


void CfrpcguiDlg::OnContextMenuShowWindow()
{
	// ���� ������ ����
	ShowWindow(SW_RESTORE);
	// ��Ŀ�� ����
	SetForegroundWindow();
}


void CfrpcguiDlg::OnContextMenuExit()
{
	// ���� frpc ���μ��� ���� ���� Ȯ��
	BOOL isFrpcRunning = FALSE;
	if (m_frpcProcess.hProcess != NULL && WaitForSingleObject(m_frpcProcess.hProcess, 0) == WAIT_TIMEOUT) {
		isFrpcRunning = TRUE;
	}

	if (isFrpcRunning)
	{
		// ���Ͻ� ���� ���̹Ƿ� ��� �޽��� ���
		int result = MessageBox(_T("���� ������ �������ϴ�. �����Ͻðڽ��ϱ�?"), _T("���� Ȯ��"), MB_YESNO | MB_ICONQUESTION);
		if (result == IDYES)
		{
			// ���Ͻ� ����
			StopFrpcProcess();

			// frpc.exe ���� ���� �õ�
			if (!DeleteFile(m_szTargetFile)) {
				AddLogMessage(_T("frpc.exe ���� ���� ���� - ��� �� ��õ�"));
				Sleep(500);
				DeleteFile(m_szTargetFile);
			}

			// ���� ���� ����
			SaveConf();

			// Ʈ���� ������ ����
			Shell_NotifyIcon(NIM_DELETE, &m_trayIconData);

			// ���α׷� ����
			PostQuitMessage(0);
		}
		else
		{
			// ����ڰ� '�ƴϿ�' ���� �� ���� ���
			return;
		}
	}
	else
	{
		// frpc �̽��� �����̸� �ٷ� ����
		SaveConf();
		Shell_NotifyIcon(NIM_DELETE, &m_trayIconData);
		PostQuitMessage(0);
	}
}


void CfrpcguiDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	// ���α׷� ���� �� Ʈ���� ������ ����
	Shell_NotifyIcon(NIM_DELETE, &m_trayIconData);
}


void CfrpcguiDlg::OnClose()
{
	// ���� ������ �����
	ShowWindow(SW_HIDE);
}


void CfrpcguiDlg::LoadConf()
{
	TCHAR currentDir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, currentDir);
	CString path;
	path.Format(_T("%s\\frpc.ini"), currentDir);

	CFile file;
	if (!file.Open(path, CFile::modeRead | CFile::typeBinary)) {
		// ������ ���ų� �б� ���� �� �⺻�� ����
		m_serverAddr = _T("127.0.0.1");
		m_serverPort = 7000;
		m_token = _T("12345");
		m_autoStart = FALSE;
		m_deviceName = _T("��ġ�̸��Է�");
		m_localPort = 9500;
		m_remotePort = 9999;
		UpdateData(FALSE);
		return;
	}

	ULONGLONG fileSize = file.GetLength();
	std::vector<BYTE> buffer((size_t)fileSize + 1, 0);
	file.Read(buffer.data(), (UINT)fileSize);
	file.Close();

	// UTF-8 �� WideChar ��ȯ
	int wideLen = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)buffer.data(), (int)fileSize, NULL, 0);
	if (wideLen <= 0) {
		// ��ȯ ���� �� �⺻�� ����
		m_serverAddr = _T("127.0.0.1");
		m_serverPort = 7000;
		m_token = _T("12345");
		m_autoStart = FALSE;
		m_deviceName = _T("��ġ�̸��Է�");
		m_localPort = 9500;
		m_remotePort = 9999;
		UpdateData(FALSE);
		return;
	}

	std::wstring wtext;
	wtext.resize(wideLen);
	MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)buffer.data(), (int)fileSize, &wtext[0], wideLen);

	// INI �Ľ�: [����], key=value
	std::map<std::wstring, std::map<std::wstring, std::wstring>> iniData;
	{
		std::wstringstream ss(wtext);
		std::wstring line;
		std::wstring currentSection;
		while (std::getline(ss, line)) {
			line = Trim(line);
			if (line.empty() || line[0] == L';') continue; // �� �� Ȥ�� �ּ�
			if (line.front() == L'[' && line.back() == L']') {
				currentSection = line.substr(1, line.size() - 2);
			}
			else {
				size_t pos = line.find(L'=');
				if (pos != std::wstring::npos) {
					std::wstring key = Trim(line.substr(0, pos));
					std::wstring val = Trim(line.substr(pos + 1));
					iniData[currentSection][key] = val;
				}
			}
		}
	}

	// �� �б�(common)
	m_serverAddr = iniData[L"common"][L"server_addr"].c_str();
	m_serverPort = _wtoi(iniData[L"common"][L"server_port"].c_str());
	m_token = iniData[L"common"][L"token"].c_str();
	m_autoStart = _wtoi(iniData[L"common"][L"auto_start"].c_str()) ? TRUE : FALSE;

	// common �̿� ù ������ ��ġ �����̶� ����
	for (auto& sec : iniData) {
		if (sec.first != L"common") {
			m_deviceName = sec.first.c_str();
			m_localPort = _wtoi(sec.second[L"local_port"].c_str());
			m_remotePort = _wtoi(sec.second[L"remote_port"].c_str());
			break;
		}
	}

	// �ҷ��� ���� ��Ʈ�ѿ� ǥ��
	UpdateData(FALSE);
}

void CfrpcguiDlg::SaveConf()
{
	// ��Ʈ���� ���� ��������
	UpdateData(TRUE);

	// INI ���� ���� (�����ڵ� ���)
	std::wstringstream ss;
	ss << L"[common]\r\n";
	ss << L"server_addr=" << (LPCTSTR)m_serverAddr << L"\r\n";
	ss << L"server_port=" << m_serverPort << L"\r\n";
	ss << L"token=" << (LPCTSTR)m_token << L"\r\n";
	ss << L"auto_start=" << (m_autoStart ? 1 : 0) << L"\r\n";

	if (!m_deviceName.IsEmpty()) {
		ss << L"[" << (LPCTSTR)m_deviceName << L"]\r\n";
		ss << L"type=tcp\r\n";
		ss << L"local_port=" << m_localPort << L"\r\n";
		ss << L"remote_port=" << m_remotePort << L"\r\n";
	}

	std::wstring wtext = ss.str();

	// �����ڵ� �� UTF-8 ��ȯ (BOM ����)
	int mbLen = WideCharToMultiByte(CP_UTF8, 0, wtext.c_str(), (int)wtext.size(), NULL, 0, NULL, NULL);
	std::vector<char> mbBuf(mbLen, 0);
	WideCharToMultiByte(CP_UTF8, 0, wtext.c_str(), (int)wtext.size(), mbBuf.data(), mbLen, NULL, NULL);

	TCHAR currentDir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, currentDir);
	CString path;
	path.Format(_T("%s\\frpc.ini"), currentDir);

	CFile file;
	if (file.Open(path, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary)) {
		file.Write(mbBuf.data(), (UINT)mbLen);
		file.Close();
	}
}


void CfrpcguiDlg::OnBnClickedButtonStart()
{
	// ��Ʈ���� ���� ��������
	UpdateData(TRUE);
	// frpc�� �̹� ���� ������ ����
	if (m_frpcProcess.hProcess != NULL && WaitForSingleObject(m_frpcProcess.hProcess, 0) == WAIT_TIMEOUT)
	{
		MessageBox(_T("frpc�� �̹� ���� ���Դϴ�."), _T("���"));
		return;
	}
	// �Է� �� ����
	if (m_serverAddr.Trim().IsEmpty())
	{
		MessageBox(_T("���� �ּҸ� �Է��ϼ���."), _T("����"));
		return;
	}
	// �������� ����
	SaveConf();
	// ExtractAndRunEncryptedExeFromResource() ȣ�� ��,
	// ���� �� StartFrpcProcess() ȣ��
	if (!ExtractAndRunEncryptedExeFromResource())
	{
		AddLogMessage(_T("frpc.exe ��ȣȭ/���� ����"));
		return;
	}
	// ��ư ��Ʈ�� ��Ȱ��ȭ
	GetDlgItem(IDC_BUTTON_START)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(TRUE);
	// �̺�Ʈ �ʱ�ȭ �� ��õ� ���� ����
	m_stopEvent.ResetEvent();
	m_retryCount = 0;
	m_successStarted = FALSE;

	AddLogMessage(_T("frpc ���� �õ� ��..."));

	if (!StartFrpcProcess(FALSE))
	{
		AddLogMessage(_T("frpc ���μ��� ���� ����"));
		// ��ư ��Ʈ�� ��Ȱ��ȭ
		GetDlgItem(IDC_BUTTON_START)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(FALSE);
	}
}


void CfrpcguiDlg::OnBnClickedButtonStop()
{
	// ���μ��� ���� ��û
	StopFrpcProcess();

	// frpc.exe ���� �õ�
	if (!DeleteFile(m_szTargetFile))
	{
		AddLogMessage(_T("frpc.exe ���� ����. ��� ��� �� ��õ��մϴ�."));
		Sleep(1000);
		if (!DeleteFile(m_szTargetFile))
		{
			AddLogMessage(_T("frpc.exe ���� ���� - ���� ��� ������ �� ����."));
		}
	}

	// ���μ��� ���� ó�� ��
	GetDlgItem(IDC_BUTTON_START)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_DEVICE_NAME)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT_SERVER_ADDRESS)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT_SERVER_PORT)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT_AUTH_TOKEN)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT_LOCAL_PORT)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT_REMOTE_PORT)->EnableWindow(TRUE);

	AddLogMessage(_T("frpc ���μ����� �����Ǿ����ϴ�."));
}


std::wstring CfrpcguiDlg::Trim(const std::wstring& str)
{
	const std::wstring whitespace = L" \t\r\n";
	size_t start = str.find_first_not_of(whitespace);
	if (start == std::wstring::npos) return L"";
	size_t end = str.find_last_not_of(whitespace);
	return str.substr(start, end - start + 1);
}


void CfrpcguiDlg::XorData(BYTE* pData, DWORD dwSize, const BYTE* pKey, size_t keyLen)
{
	for (DWORD i = 0; i < dwSize; i++)
	{
		pData[i] ^= pKey[i % keyLen];
	}
}


BOOL CfrpcguiDlg::ExtractAndRunEncryptedExeFromResource()
{
	// ���� ���� ������ ��ġ�� ��� ���
	TCHAR szModulePath[MAX_PATH] = { 0 };
	if (GetModuleFileName(NULL, szModulePath, MAX_PATH) == 0) {
		AddLogMessage(_T("���� ���� ��θ� ���� �� �����ϴ�."));
		return FALSE;
	}

	// ���ϸ� �κ� �����Ͽ� ���丮 ��θ� �����
	PathRemoveFileSpec(szModulePath);

	// ���� ���ϸ��� "frpc.exe"�� ����
	ZeroMemory(m_szTargetFile, MAX_PATH);
	_tcscpy_s(m_szTargetFile, szModulePath);
	PathAppend(m_szTargetFile, _T("frpc.exe"));

	// frpc.exe ������ �̹� �����Ѵٸ� ���� �õ�
	if (PathFileExists(m_szTargetFile))
	{
		if (!DeleteFile(m_szTargetFile))
		{
			AddLogMessage(_T("���� frpc.exe ���� ���� ����. �۾� �����ڿ��� frpc.exe ���μ����� �������ּ���."));
			return FALSE;
		}
	}

	// ������ ���� üũ
	LPCTSTR pResourceName;
	if (IsWindows7OrGreater())
	{
		if (!IsWindows8OrGreater())
		{
			// ������ 7�� ���
			pResourceName = MAKEINTRESOURCE(IDR_FRPC_ENC_WIN7);
		}
		else
		{
			// ������ 8 �̻��� ���
			pResourceName = MAKEINTRESOURCE(IDR_FRPC_ENC);
		}
	}
	else
	{
		// ������ Vista, XP �� (�������� ����)
		pResourceName = MAKEINTRESOURCE(IDR_FRPC_ENC_WIN7);
	}

	// ���ҽ����� ������ �ε�
	HRSRC hRes = FindResource(AfxGetResourceHandle(), pResourceName, RT_RCDATA);
	if (!hRes)
	{
		AddLogMessage(_T("��ȣȭ�� ���ҽ��� ã�� �� �����ϴ�."));
		return FALSE;
	}
	DWORD dwSize = SizeofResource(AfxGetResourceHandle(), hRes);
	HGLOBAL hResData = LoadResource(AfxGetResourceHandle(), hRes);
	if (!hResData)
	{
		AddLogMessage(_T("���ҽ� �ε� ����"));
		return FALSE;
	}

	BYTE* pEncData = (BYTE*)LockResource(hResData);
	if (!pEncData)
	{
		AddLogMessage(_T("���ҽ� ��� ����"));
		return FALSE;
	}
	// ���ҽ� �����͸� �޸𸮿� ����
	std::vector<BYTE> buffer(pEncData, pEncData + dwSize);

	// XOR Ű (20����Ʈ ���� ���ڿ�)
	const BYTE key[] = "LfZA*j@v%&6ZnDF8xjr%";

	// XOR ��ȣȭ
	// sizeof(key)-1�� ����� ����: key�� �� ���� ����('\0')�� �����Ƿ� ���� ��ȿ�� Ű ���̴� 20����Ʈ.
	XorData(buffer.data(), dwSize, key, sizeof(key) - 1);

	// frpc.exe ������ ���� ���丮�� ����
	HANDLE hFile = CreateFile(m_szTargetFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		AddLogMessage(_T("frpc.exe ���� ���� ����"));
		return FALSE;
	}

	DWORD dwWritten = 0;
	BOOL bWrite = WriteFile(hFile, buffer.data(), dwSize, &dwWritten, NULL);
	CloseHandle(hFile);

	if (!bWrite || dwWritten != dwSize) {
		AddLogMessage(_T("frpc.exe ���� ���� ����"));
		return FALSE;
	}

	return TRUE;
}


BOOL CfrpcguiDlg::StartFrpcProcess(BOOL isRetry)
{
	if (!isRetry)
	{
		// ���� ��õ� ī��Ʈ �ʱ�ȭ
		m_retryCount = 0;
		m_successStarted = FALSE;
		// ������ ���� ��û �̺�Ʈ ����
		m_stopEvent.ResetEvent();
	}

	// ���� remotePort ������ ��ȿ�� ��Ʈ �Ҵ�
	int remotePort = GetValidPortOrRandom(m_remotePort);
	if (remotePort == -1)
	{
		AddLogMessage(_T("���� ��Ʈ�� 1~65535 �������� �մϴ�."));
		return FALSE;
	}

	// ���Ͻ� �̸� ����
	CString proxyName = GenerateProxyName();
	// frpc.ini ����
	GenerateFrpcIni(remotePort, proxyName);

	// ������ frpc.ini �� �������� ��Ʈ�� �ٽ� ǥ��
	m_remotePort = remotePort;
	m_deviceName = proxyName;
	UpdateData(FALSE);

	// ���� ���μ��� ����
	StopFrpcProcess();

	// ������ ����
	SECURITY_ATTRIBUTES saAttr = { sizeof(SECURITY_ATTRIBUTES) };
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;

	if (!CreatePipe(&m_hChildStd_OUT_Rd, &m_hChildStd_OUT_Wr, &saAttr, 0))
	{
		AddLogMessage(_T("������ ���� ����"));
		return FALSE;
	}

	if (!SetHandleInformation(m_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
	{
		AddLogMessage(_T("SetHandleInformation ����"));
		return FALSE;
	}

	// frpc.exe ���
	TCHAR currentDir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, currentDir);
	CString frpcPath;
	frpcPath.Format(_T("%s\\frpc.exe"), currentDir);

	STARTUPINFO si = { sizeof(si) };
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	si.hStdOutput = m_hChildStd_OUT_Wr;
	si.hStdError = m_hChildStd_OUT_Wr;
	si.wShowWindow = SW_HIDE;

	PROCESS_INFORMATION pi = { 0 };

	TCHAR cmdLine[] = _T("frpc.exe -c frpc.ini");
	// �ܼ� â ���� ��׶��忡�� �����ϵ��� CREATE_NO_WINDOW �÷��� �߰�
	if (!CreateProcess(frpcPath, cmdLine, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, currentDir, &si, &pi))
	{
		AddLogMessage(_T("frpc ���� ����"));
		CloseHandle(m_hChildStd_OUT_Rd);
		CloseHandle(m_hChildStd_OUT_Wr);
		m_hChildStd_OUT_Rd = NULL;
		m_hChildStd_OUT_Wr = NULL;
		return FALSE;
	}

	// ���μ��� ���� ���� ��
	m_frpcProcess = pi;

	// �θ�� ���� �ڵ��� �ʿ� �����Ƿ� ��� ���� -> EOF ó�� ����
	if (m_hChildStd_OUT_Wr)
	{
		CloseHandle(m_hChildStd_OUT_Wr);
		m_hChildStd_OUT_Wr = NULL;
	}

	// �α� �б� ������ ����
	m_pLogThread = AfxBeginThread(LogReaderThread, this);

	return TRUE;
}


void CfrpcguiDlg::StopFrpcProcess()
{
	// ������ ���� ��û
	m_stopEvent.SetEvent();

	if (m_frpcProcess.hProcess != NULL)
	{
		// ���� ���� ����(����� ���ٸ� �ٷ� terminate)
		if (WaitForSingleObject(m_frpcProcess.hProcess, 1000) == WAIT_TIMEOUT)
		{
			// ���� ���μ����� ������� �ʾ����Ƿ� ���� ����
			TerminateProcess(m_frpcProcess.hProcess, 0);
			WaitForSingleObject(m_frpcProcess.hProcess, 2000);
		}

		CloseHandle(m_frpcProcess.hThread);
		CloseHandle(m_frpcProcess.hProcess);
		m_frpcProcess.hProcess = NULL;
		m_frpcProcess.hThread = NULL;
	}

	// �α� ������ ���� ���
	if (m_pLogThread != NULL)
	{
		WaitForSingleObject(m_pLogThread->m_hThread, 2000);
		// ������ �ڵ��� MFC���� �ڵ� ����
		m_pLogThread = NULL;
	}

	// ������ �ڵ� ����
	if (m_hChildStd_OUT_Rd) {
		CloseHandle(m_hChildStd_OUT_Rd);
		m_hChildStd_OUT_Rd = NULL;
	}
	if (m_hChildStd_OUT_Wr) {
		CloseHandle(m_hChildStd_OUT_Wr);
		m_hChildStd_OUT_Wr = NULL;
	}

	// �̺�Ʈ ���� (���� ������ ����)
	m_stopEvent.ResetEvent();
}


UINT CfrpcguiDlg::LogReaderThread(LPVOID pParam)
{
	CfrpcguiDlg* pDlg = (CfrpcguiDlg*)pParam;
	HANDLE hRead = pDlg->m_hChildStd_OUT_Rd;
	if (!hRead) return 0;

	std::string buffer;
	char buf[256];
	DWORD dwRead;

	while (WaitForSingleObject(pDlg->m_stopEvent, 10) != WAIT_OBJECT_0)
	{
		if (!ReadFile(hRead, buf, 255, &dwRead, NULL) || dwRead == 0)
		{
			// EOF or ���μ��� ����
			break;
		}

		buf[dwRead] = '\0';
		buffer.append(buf);

		// ����(\n, \r\n)�� �������� ���� �Ľ�
		size_t pos = 0;
		while ((pos = buffer.find_first_of("\r\n")) != std::string::npos)
		{
			std::string line = buffer.substr(0, pos);
			buffer.erase(0, buffer.find_first_not_of("\r\n", pos));

			// ������ ������� �� �����Ƿ� üũ
			if (!line.empty())
			{
				CString wLine = CA2T(line.c_str(), CP_ACP);
				pDlg->PostMessage(WM_USER_LOG_LINE, 0, (LPARAM)new CString(wLine));
			}
		}

		// Ȥ�� ������ ���� ���� ���, ���� ���� �̻��̸� ���
		if (buffer.size() > 1024)
		{
			// �ӽ÷� ������ �����͸� �� ���� ���
			CString wLine = CA2T(buffer.c_str(), CP_ACP);
			pDlg->PostMessage(WM_USER_LOG_LINE, 0, (LPARAM)new CString(wLine));
			buffer.clear();
		}
	}

	// ���� ������ ���
	if (!buffer.empty())
	{
		CString wLine = CA2T(buffer.c_str(), CP_ACP);
		pDlg->PostMessage(WM_USER_LOG_LINE, 0, (LPARAM)new CString(wLine));
	}

	return 0;
}


LRESULT CfrpcguiDlg::OnLogLine(WPARAM wParam, LPARAM lParam)
{
	CString* pLogLine = (CString*)lParam;
	CString line = *pLogLine;
	delete pLogLine;

	int len = m_edit_output.GetWindowTextLength();
	m_edit_output.SetSel(len, len);
	m_edit_output.ReplaceSel(line + _T("\r\n"));

	CString lowerLine = line;
	lowerLine.MakeLower();

	if (lowerLine.Find(_T("port already used")) != -1 || lowerLine.Find(_T("already in use")) != -1)
	{
		if (m_retryCount < MAX_RETRIES && WaitForSingleObject(m_stopEvent, 0) == WAIT_TIMEOUT)
		{
			AddLogMessage(_T("��Ʈ �浹 �߻�, �ٸ� ��Ʈ�� ��õ��մϴ�."));
			m_retryCount++;
			m_remotePort = 0; // ���� ��Ʈ �Ҵ�
			StopFrpcProcess();
			StartFrpcProcess(TRUE);
		}
		else
		{
			AddLogMessage(_T("��Ʈ ��õ� Ƚ���� �ʰ��߽��ϴ�. ���μ����� �����մϴ�."));
			StopFrpcProcess();
			GetDlgItem(IDC_BUTTON_START)->EnableWindow(TRUE);
			GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_DEVICE_NAME)->EnableWindow(TRUE);
			GetDlgItem(IDC_EDIT_SERVER_ADDRESS)->EnableWindow(TRUE);
			GetDlgItem(IDC_EDIT_SERVER_PORT)->EnableWindow(TRUE);
			GetDlgItem(IDC_EDIT_AUTH_TOKEN)->EnableWindow(TRUE);
			GetDlgItem(IDC_EDIT_LOCAL_PORT)->EnableWindow(TRUE);
			GetDlgItem(IDC_EDIT_REMOTE_PORT)->EnableWindow(TRUE);
		}
	}
	else if (lowerLine.Find(_T("proxy")) != -1 && lowerLine.Find(_T("already exists")) != -1)
	{
		AddLogMessage(_T("�ߺ��� ��ġ���Դϴ�. ��ġ���� �������ּ���."));
		StopFrpcProcess();
		GetDlgItem(IDC_BUTTON_START)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_DEVICE_NAME)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_SERVER_ADDRESS)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_SERVER_PORT)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_AUTH_TOKEN)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_LOCAL_PORT)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_REMOTE_PORT)->EnableWindow(TRUE);
	}
	else if (lowerLine.Find(_T("token in login doesn't match")) != -1)
	{
		AddLogMessage(_T("��ū�� ��ġ���� �ʽ��ϴ�. ��ū�� Ȯ�����ּ���."));
		StopFrpcProcess();
		GetDlgItem(IDC_BUTTON_START)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_DEVICE_NAME)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_SERVER_ADDRESS)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_SERVER_PORT)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_AUTH_TOKEN)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_LOCAL_PORT)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_REMOTE_PORT)->EnableWindow(TRUE);
	}
	else if (lowerLine.Find(_T("login to server success")) != -1)
	{
		AddLogMessage(_T("������ ���������� �����Ͽ����ϴ�."));
	}
	else if (lowerLine.Find(_T("start proxy success")) != -1)
	{
		m_successStarted = TRUE;
		AddLogMessage(_T("���Ͻð� ���������� ���۵Ǿ����ϴ�."));
		GetDlgItem(IDC_BUTTON_START)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_DEVICE_NAME)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_SERVER_ADDRESS)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_SERVER_PORT)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_AUTH_TOKEN)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_LOCAL_PORT)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_REMOTE_PORT)->EnableWindow(FALSE);
	}

	return 0;
}


int CfrpcguiDlg::GetValidPortOrRandom(int desiredPort)
{
	if (desiredPort <= 0)
	{
		// ���� ��Ʈ �Ҵ�
		srand((UINT)time(NULL));
		return rand() % (65535 - 10000) + 10000;
	}
	if (desiredPort >= 1 && desiredPort <= 65535)
		return desiredPort;
	return -1; // invalid
}


void CfrpcguiDlg::GenerateFrpcIni(int remotePort, const CString & proxyName)
{
	// ��Ʈ���� ���� ��������
	UpdateData(TRUE);

	CString iniText;
	iniText.Format(
		_T("[common]\r\nserver_addr=%s\r\nserver_port=%d\r\ntoken=%s\r\nauto_start=%d\r\n\r\n[%s]\r\ntype=tcp\r\nlocal_port=%d\r\nremote_port=%d\r\n"),
		m_serverAddr, m_serverPort, m_token, m_autoStart ? 1 : 0,
		proxyName,
		m_localPort, remotePort
	);

	// UTF-8�� ����
	// �����ڵ� �� UTF-8
	int mbLen = WideCharToMultiByte(CP_UTF8, 0, iniText, -1, NULL, 0, NULL, NULL);
	std::vector<char> mbBuf(mbLen, 0);
	WideCharToMultiByte(CP_UTF8, 0, iniText, -1, mbBuf.data(), mbLen, NULL, NULL);

	TCHAR currentDir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, currentDir);
	CString path;
	path.Format(_T("%s\\frpc.ini"), currentDir);

	CFile file;
	if (file.Open(path, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary))
	{
		file.Write(mbBuf.data(), (UINT)(mbLen - 1)); // -1�� �ι��� ����
		file.Close();
	}
}


CString CfrpcguiDlg::GenerateProxyName()
{
	CString name = m_deviceName;
	name.Trim();
	if (name.IsEmpty())
	{
		// ���� ����
		const TCHAR chars[] = _T("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
		CString randStr;
		for (int i = 0; i < 10; i++)
		{
			randStr.AppendChar(chars[rand() % (sizeof(chars) / sizeof(TCHAR) - 1)]);
		}
		name = randStr;
	}
	return name;
}


void CfrpcguiDlg::AddLogMessage(const CString & strMessage)
{
	int len = m_edit_output.GetWindowTextLength();
	m_edit_output.SetSel(len, len);
	m_edit_output.ReplaceSel(strMessage + _T("\r\n"));
}

