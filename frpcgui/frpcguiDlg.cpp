
// frpcguiDlg.cpp : 구현 파일
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


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
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


// CfrpcguiDlg 대화 상자



CfrpcguiDlg::CfrpcguiDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_FRPCGUI_DIALOG, pParent)
	, m_deviceName(_T("장치이름입력"))
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
	ON_MESSAGE(WM_USER + 1, &CfrpcguiDlg::OnTrayIconNotify) // 사용자 정의 메시지
	ON_COMMAND(ID_CONTEXTMENU_SHOW_WINDOW, &CfrpcguiDlg::OnContextMenuShowWindow)
	ON_COMMAND(ID_CONTEXTMENU_EXIT, &CfrpcguiDlg::OnContextMenuExit)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_START, &CfrpcguiDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CfrpcguiDlg::OnBnClickedButtonStop)
END_MESSAGE_MAP()


// CfrpcguiDlg 메시지 처리기

BOOL CfrpcguiDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
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

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.

	// 트레이 아이콘 설정
	m_trayIconData.cbSize = sizeof(NOTIFYICONDATA);
	m_trayIconData.hWnd = this->GetSafeHwnd();
	m_trayIconData.uID = 1; // 트레이 아이콘 ID
	m_trayIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	m_trayIconData.uCallbackMessage = WM_USER + 1; // 사용자 정의 메시지
	m_trayIconData.hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));
	wcscpy_s(m_trayIconData.szTip, _T("FRP GUI Client"));
	// 트레이 아이콘 표시
	Shell_NotifyIcon(NIM_ADD, &m_trayIconData);

	// 설정파일 불러오기
	LoadConf();

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
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

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CfrpcguiDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CfrpcguiDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

LRESULT CfrpcguiDlg::OnTrayIconNotify(WPARAM wParam, LPARAM lParam)
{
	if (lParam == WM_RBUTTONUP) // 우클릭 이벤트 발생 시
	{
		CPoint point;
		GetCursorPos(&point);

		// 컨텍스트 메뉴 로드
		CMenu menu;
		if (menu.LoadMenu(IDR_MENU_CONTEXT_MENU))
		{
			CMenu* pSubMenu = menu.GetSubMenu(0);
			if (pSubMenu)
			{
				// SetForegroundWindow를 호출해 팝업 메뉴가 닫히지 않도록 설정
				SetForegroundWindow();

				// 트레이 메뉴 표시 후 선택된 명령 ID 반환
				int nCmd = pSubMenu->TrackPopupMenu(TPM_RETURNCMD | TPM_RIGHTBUTTON, point.x, point.y, this);

				// 반환된 명령 ID에 따라 핸들러 실행
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
	// 메인 윈도우 복원
	ShowWindow(SW_RESTORE);
	// 포커스 설정
	SetForegroundWindow();
}

void CfrpcguiDlg::OnContextMenuExit()
{
	// 설정파일 저장
	SaveConf();
	// 트레이 아이콘 제거
	Shell_NotifyIcon(NIM_DELETE, &m_trayIconData);
	// 프로그램 종료
	PostQuitMessage(0);
}



void CfrpcguiDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	// 프로그램 종료 시 트레이 아이콘 제거
	Shell_NotifyIcon(NIM_DELETE, &m_trayIconData);
}


void CfrpcguiDlg::OnClose()
{
	// 메인 윈도우 숨기기
	ShowWindow(SW_HIDE);
}


// WhiteSpace 제거
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

	// 파일 읽기 (CP949로 인코딩된 텍스트)
	CFile file;
	if (!file.Open(path, CFile::modeRead | CFile::typeBinary)) {
		// 파일이 없거나 읽기 실패 시 기본값 설정
		m_serverAddr = _T("127.0.0.1");
		m_serverPort = 7000;
		m_token = _T("12345");
		m_autoStart = FALSE;
		m_deviceName = _T("장치이름입력");
		m_localPort = 9500;
		m_remotePort = 9999;
		UpdateData(FALSE);
		return;
	}

	ULONGLONG fileSize = file.GetLength();
	std::vector<BYTE> buffer((size_t)fileSize + 1, 0);
	file.Read(buffer.data(), (UINT)fileSize);
	file.Close();

	// CP949 → WideChar 변환
	int wideLen = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)buffer.data(), (int)fileSize, NULL, 0);
	if (wideLen <= 0) {
		// 변환 실패 시 기본값 설정
		m_serverAddr = _T("127.0.0.1");
		m_serverPort = 7000;
		m_token = _T("12345");
		m_autoStart = FALSE;
		m_deviceName = _T("장치이름입력");
		m_localPort = 9500;
		m_remotePort = 9999;
		UpdateData(FALSE);
		return;
	}

	std::wstring wtext;
	wtext.resize(wideLen);
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)buffer.data(), (int)fileSize, &wtext[0], wideLen);

	// INI 파싱: [섹션], key=value
	std::map<std::wstring, std::map<std::wstring, std::wstring>> iniData;
	{
		std::wstringstream ss(wtext);
		std::wstring line;
		std::wstring currentSection;
		while (std::getline(ss, line)) {
			line = Trim(line);
			if (line.empty() || line[0] == L';') continue; // 빈 줄 혹은 주석
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

	// 값 읽기(common)
	m_serverAddr = iniData[L"common"][L"server_addr"].c_str();
	m_serverPort = _wtoi(iniData[L"common"][L"server_port"].c_str());
	m_token = iniData[L"common"][L"token"].c_str();
	m_autoStart = _wtoi(iniData[L"common"][L"auto_start"].c_str()) ? TRUE : FALSE;

	// common 이외 첫 섹션을 장치 섹션이라 가정
	for (auto& sec : iniData) {
		if (sec.first != L"common") {
			m_deviceName = sec.first.c_str();
			m_localPort = _wtoi(sec.second[L"local_port"].c_str());
			m_remotePort = _wtoi(sec.second[L"remote_port"].c_str());
			break;
		}
	}

	// 불러온 값을 컨트롤에 표시
	UpdateData(FALSE);
}

void CfrpcguiDlg::SaveConf()
{
	// 컨트롤의 값을 가져오기
	UpdateData(TRUE);

	// INI 내용 빌드 (유니코드 기반)
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

	// 유니코드 → CP949 변환
	int mbLen = WideCharToMultiByte(CP_ACP, 0, wtext.c_str(), (int)wtext.size(), NULL, 0, NULL, NULL);
	std::vector<char> mbBuf(mbLen, 0);
	WideCharToMultiByte(CP_ACP, 0, wtext.c_str(), (int)wtext.size(), mbBuf.data(), mbLen, NULL, NULL);

	// 파일 쓰기
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
	// 설정파일 저장
	SaveConf();
}


void CfrpcguiDlg::OnBnClickedButtonStop()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}
