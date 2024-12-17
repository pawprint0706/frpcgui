
// frpcguiDlg.cpp : ���� ����
//

#include "stdafx.h"
#include "frpcgui.h"
#include "frpcguiDlg.h"
#include "afxdialogex.h"
#include <vector>
#include <map>
#include <string>
#include <sstream>

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
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
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
	ON_MESSAGE(WM_USER + 1, &CfrpcguiDlg::OnTrayIconNotify) // ����� ���� �޽���
	ON_COMMAND(ID_CONTEXTMENU_SHOW_WINDOW, &CfrpcguiDlg::OnContextMenuShowWindow)
	ON_COMMAND(ID_CONTEXTMENU_EXIT, &CfrpcguiDlg::OnContextMenuExit)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_START, &CfrpcguiDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CfrpcguiDlg::OnBnClickedButtonStop)
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

	// TODO: ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.

	// Ʈ���� ������ ����
	m_trayIconData.cbSize = sizeof(NOTIFYICONDATA);
	m_trayIconData.hWnd = this->GetSafeHwnd();
	m_trayIconData.uID = 1; // Ʈ���� ������ ID
	m_trayIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	m_trayIconData.uCallbackMessage = WM_USER + 1; // ����� ���� �޽���
	m_trayIconData.hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));
	wcscpy_s(m_trayIconData.szTip, _T("FRP GUI Client"));
	// Ʈ���� ������ ǥ��
	Shell_NotifyIcon(NIM_ADD, &m_trayIconData);

	// �������� �ҷ�����
	LoadConf();

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
	if (lParam == WM_RBUTTONUP) // ��Ŭ�� �̺�Ʈ �߻� ��
	{
		CPoint point;
		GetCursorPos(&point);

		// ���ؽ�Ʈ �޴� �ε�
		CMenu menu;
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
	// �������� ����
	SaveConf();
	// Ʈ���� ������ ����
	Shell_NotifyIcon(NIM_DELETE, &m_trayIconData);
	// ���α׷� ����
	PostQuitMessage(0);
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


// WhiteSpace ����
static std::wstring Trim(const std::wstring& str)
{
	const std::wstring whitespace = L" \t\r\n";
	size_t start = str.find_first_not_of(whitespace);
	if (start == std::wstring::npos) return L"";
	size_t end = str.find_last_not_of(whitespace);
	return str.substr(start, end - start + 1);
}


void CfrpcguiDlg::LoadConf()
{
	TCHAR currentDir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, currentDir);
	CString path;
	path.Format(_T("%s\\frpc.ini"), currentDir);

	// ���� �б� (CP949�� ���ڵ��� �ؽ�Ʈ)
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

	// CP949 �� WideChar ��ȯ
	int wideLen = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)buffer.data(), (int)fileSize, NULL, 0);
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
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)buffer.data(), (int)fileSize, &wtext[0], wideLen);

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

	// �����ڵ� �� CP949 ��ȯ
	int mbLen = WideCharToMultiByte(CP_ACP, 0, wtext.c_str(), (int)wtext.size(), NULL, 0, NULL, NULL);
	std::vector<char> mbBuf(mbLen, 0);
	WideCharToMultiByte(CP_ACP, 0, wtext.c_str(), (int)wtext.size(), mbBuf.data(), mbLen, NULL, NULL);

	// ���� ����
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
	// �������� ����
	SaveConf();
}


void CfrpcguiDlg::OnBnClickedButtonStop()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
}
