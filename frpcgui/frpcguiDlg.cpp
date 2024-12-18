
// frpcguiDlg.cpp : 구현 파일
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
	ON_MESSAGE(WM_USER_TRAY_ICON, &CfrpcguiDlg::OnTrayIconNotify) // 사용자 정의 메시지
	ON_COMMAND(ID_CONTEXTMENU_SHOW_WINDOW, &CfrpcguiDlg::OnContextMenuShowWindow)
	ON_COMMAND(ID_CONTEXTMENU_EXIT, &CfrpcguiDlg::OnContextMenuExit)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_START, &CfrpcguiDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CfrpcguiDlg::OnBnClickedButtonStop)
	ON_MESSAGE(WM_USER_LOG_LINE, &CfrpcguiDlg::OnLogLine)
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

									// 트레이 아이콘 설정
	m_trayIconData.cbSize = sizeof(NOTIFYICONDATA);
	m_trayIconData.hWnd = this->GetSafeHwnd();
	m_trayIconData.uID = 1; // 트레이 아이콘 ID
	m_trayIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	m_trayIconData.uCallbackMessage = WM_USER_TRAY_ICON; // 사용자 정의 메시지
	m_trayIconData.hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));
	wcscpy_s(m_trayIconData.szTip, _T("FRP GUI Client"));
	// 트레이 아이콘 표시
	Shell_NotifyIcon(NIM_ADD, &m_trayIconData);
	
	// 설정파일 불러오기
	LoadConf();

	// 버튼 컨트롤 비활성화
	GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(FALSE);

	// 설정 로딩 끝난 후 auto_start 값이 TRUE면 자동 실행 시도
	if (m_autoStart)
	{
		// OnBnClickedButtonStart() 호출
		PostMessage(WM_COMMAND, MAKEWPARAM(IDC_BUTTON_START, BN_CLICKED), (LPARAM)GetDlgItem(IDC_BUTTON_START)->m_hWnd);
	}

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
	if (wParam == m_trayIconData.uID)
	{
		CMenu menu;
		CPoint point;
		switch (lParam)
		{
		case WM_LBUTTONDBLCLK: // 좌더블클릭 이벤트 발생 시
			OnContextMenuShowWindow();
			break;
		case WM_RBUTTONUP: // 우클릭 이벤트 발생 시
			// 커서 위치 찾기
			GetCursorPos(&point);
			// 컨텍스트 메뉴 로드
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
			break;
		default:
			break;
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
	// 현재 frpc 프로세스 실행 상태 확인
	BOOL isFrpcRunning = FALSE;
	if (m_frpcProcess.hProcess != NULL && WaitForSingleObject(m_frpcProcess.hProcess, 0) == WAIT_TIMEOUT) {
		isFrpcRunning = TRUE;
	}

	if (isFrpcRunning)
	{
		// 프록시 실행 중이므로 경고 메시지 출력
		int result = MessageBox(_T("현재 연결이 끊어집니다. 종료하시겠습니까?"), _T("종료 확인"), MB_YESNO | MB_ICONQUESTION);
		if (result == IDYES)
		{
			// 프록시 종료
			StopFrpcProcess();

			// frpc.exe 파일 삭제 시도
			if (!DeleteFile(m_szTargetFile)) {
				AddLogMessage(_T("frpc.exe 파일 삭제 실패 - 잠시 후 재시도"));
				Sleep(500);
				DeleteFile(m_szTargetFile);
			}

			// 설정 파일 저장
			SaveConf();

			// 트레이 아이콘 제거
			Shell_NotifyIcon(NIM_DELETE, &m_trayIconData);

			// 프로그램 종료
			PostQuitMessage(0);
		}
		else
		{
			// 사용자가 '아니오' 선택 시 종료 취소
			return;
		}
	}
	else
	{
		// frpc 미실행 상태이면 바로 종료
		SaveConf();
		Shell_NotifyIcon(NIM_DELETE, &m_trayIconData);
		PostQuitMessage(0);
	}
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


void CfrpcguiDlg::LoadConf()
{
	TCHAR currentDir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, currentDir);
	CString path;
	path.Format(_T("%s\\frpc.ini"), currentDir);

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

	// UTF-8 → WideChar 변환
	int wideLen = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)buffer.data(), (int)fileSize, NULL, 0);
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
	MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)buffer.data(), (int)fileSize, &wtext[0], wideLen);

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

	// 유니코드 → UTF-8 변환 (BOM 없이)
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
	// 컨트롤의 값을 가져오기
	UpdateData(TRUE);
	// frpc가 이미 실행 중인지 조사
	if (m_frpcProcess.hProcess != NULL && WaitForSingleObject(m_frpcProcess.hProcess, 0) == WAIT_TIMEOUT)
	{
		MessageBox(_T("frpc가 이미 실행 중입니다."), _T("경고"));
		return;
	}
	// 입력 값 검증
	if (m_serverAddr.Trim().IsEmpty())
	{
		MessageBox(_T("서버 주소를 입력하세요."), _T("오류"));
		return;
	}
	// 설정파일 저장
	SaveConf();
	// ExtractAndRunEncryptedExeFromResource() 호출 후,
	// 성공 시 StartFrpcProcess() 호출
	if (!ExtractAndRunEncryptedExeFromResource())
	{
		AddLogMessage(_T("frpc.exe 복호화/생성 실패"));
		return;
	}
	// 버튼 컨트롤 비활성화
	GetDlgItem(IDC_BUTTON_START)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(TRUE);
	// 이벤트 초기화 및 재시도 변수 리셋
	m_stopEvent.ResetEvent();
	m_retryCount = 0;
	m_successStarted = FALSE;

	AddLogMessage(_T("frpc 시작 시도 중..."));

	if (!StartFrpcProcess(FALSE))
	{
		AddLogMessage(_T("frpc 프로세스 시작 실패"));
		// 버튼 컨트롤 비활성화
		GetDlgItem(IDC_BUTTON_START)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(FALSE);
	}
}


void CfrpcguiDlg::OnBnClickedButtonStop()
{
	// 프로세스 정지 요청
	StopFrpcProcess();

	// frpc.exe 삭제 시도
	if (!DeleteFile(m_szTargetFile))
	{
		AddLogMessage(_T("frpc.exe 삭제 실패. 잠시 대기 후 재시도합니다."));
		Sleep(1000);
		if (!DeleteFile(m_szTargetFile))
		{
			AddLogMessage(_T("frpc.exe 삭제 실패 - 파일 잠금 상태일 수 있음."));
		}
	}

	// 프로세스 종료 처리 후
	GetDlgItem(IDC_BUTTON_START)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_DEVICE_NAME)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT_SERVER_ADDRESS)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT_SERVER_PORT)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT_AUTH_TOKEN)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT_LOCAL_PORT)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT_REMOTE_PORT)->EnableWindow(TRUE);

	AddLogMessage(_T("frpc 프로세스가 중지되었습니다."));
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
	// 현재 실행 파일이 위치한 경로 얻기
	TCHAR szModulePath[MAX_PATH] = { 0 };
	if (GetModuleFileName(NULL, szModulePath, MAX_PATH) == 0) {
		AddLogMessage(_T("현재 실행 경로를 얻을 수 없습니다."));
		return FALSE;
	}

	// 파일명 부분 제거하여 디렉토리 경로만 남기기
	PathRemoveFileSpec(szModulePath);

	// 실행 파일명을 "frpc.exe"로 지정
	ZeroMemory(m_szTargetFile, MAX_PATH);
	_tcscpy_s(m_szTargetFile, szModulePath);
	PathAppend(m_szTargetFile, _T("frpc.exe"));

	// frpc.exe 파일이 이미 존재한다면 삭제 시도
	if (PathFileExists(m_szTargetFile))
	{
		if (!DeleteFile(m_szTargetFile))
		{
			AddLogMessage(_T("기존 frpc.exe 파일 삭제 실패. 작업 관리자에서 frpc.exe 프로세스를 종료해주세요."));
			return FALSE;
		}
	}

	// 윈도우 버전 체크
	LPCTSTR pResourceName;
	if (IsWindows7OrGreater())
	{
		if (!IsWindows8OrGreater())
		{
			// 윈도우 7의 경우
			pResourceName = MAKEINTRESOURCE(IDR_FRPC_ENC_WIN7);
		}
		else
		{
			// 윈도우 8 이상인 경우
			pResourceName = MAKEINTRESOURCE(IDR_FRPC_ENC);
		}
	}
	else
	{
		// 윈도우 Vista, XP 등 (지원하지 않음)
		pResourceName = MAKEINTRESOURCE(IDR_FRPC_ENC_WIN7);
	}

	// 리소스에서 데이터 로드
	HRSRC hRes = FindResource(AfxGetResourceHandle(), pResourceName, RT_RCDATA);
	if (!hRes)
	{
		AddLogMessage(_T("암호화된 리소스를 찾을 수 없습니다."));
		return FALSE;
	}
	DWORD dwSize = SizeofResource(AfxGetResourceHandle(), hRes);
	HGLOBAL hResData = LoadResource(AfxGetResourceHandle(), hRes);
	if (!hResData)
	{
		AddLogMessage(_T("리소스 로드 실패"));
		return FALSE;
	}

	BYTE* pEncData = (BYTE*)LockResource(hResData);
	if (!pEncData)
	{
		AddLogMessage(_T("리소스 잠금 실패"));
		return FALSE;
	}
	// 리소스 데이터를 메모리에 복사
	std::vector<BYTE> buffer(pEncData, pEncData + dwSize);

	// XOR 키 (20바이트 랜덤 문자열)
	const BYTE key[] = "LfZA*j@v%&6ZnDF8xjr%";

	// XOR 복호화
	// sizeof(key)-1을 사용한 이유: key는 널 종료 문자('\0')가 있으므로 실제 유효한 키 길이는 20바이트.
	XorData(buffer.data(), dwSize, key, sizeof(key) - 1);

	// frpc.exe 파일을 현재 디렉토리에 생성
	HANDLE hFile = CreateFile(m_szTargetFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		AddLogMessage(_T("frpc.exe 파일 생성 실패"));
		return FALSE;
	}

	DWORD dwWritten = 0;
	BOOL bWrite = WriteFile(hFile, buffer.data(), dwSize, &dwWritten, NULL);
	CloseHandle(hFile);

	if (!bWrite || dwWritten != dwSize) {
		AddLogMessage(_T("frpc.exe 파일 쓰기 실패"));
		return FALSE;
	}

	return TRUE;
}


BOOL CfrpcguiDlg::StartFrpcProcess(BOOL isRetry)
{
	if (!isRetry)
	{
		// 먼저 재시도 카운트 초기화
		m_retryCount = 0;
		m_successStarted = FALSE;
		// 스레드 정지 요청 이벤트 리셋
		m_stopEvent.ResetEvent();
	}

	// 현재 remotePort 값으로 유효한 포트 할당
	int remotePort = GetValidPortOrRandom(m_remotePort);
	if (remotePort == -1)
	{
		AddLogMessage(_T("원격 포트는 1~65535 범위여야 합니다."));
		return FALSE;
	}

	// 프록시 이름 생성
	CString proxyName = GenerateProxyName();
	// frpc.ini 생성
	GenerateFrpcIni(remotePort, proxyName);

	// 생성된 frpc.ini 를 기준으로 컨트롤 다시 표출
	m_remotePort = remotePort;
	m_deviceName = proxyName;
	UpdateData(FALSE);

	// 기존 프로세스 종료
	StopFrpcProcess();

	// 파이프 생성
	SECURITY_ATTRIBUTES saAttr = { sizeof(SECURITY_ATTRIBUTES) };
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;

	if (!CreatePipe(&m_hChildStd_OUT_Rd, &m_hChildStd_OUT_Wr, &saAttr, 0))
	{
		AddLogMessage(_T("파이프 생성 실패"));
		return FALSE;
	}

	if (!SetHandleInformation(m_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
	{
		AddLogMessage(_T("SetHandleInformation 실패"));
		return FALSE;
	}

	// frpc.exe 경로
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
	// 콘솔 창 없이 백그라운드에서 동작하도록 CREATE_NO_WINDOW 플래그 추가
	if (!CreateProcess(frpcPath, cmdLine, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, currentDir, &si, &pi))
	{
		AddLogMessage(_T("frpc 실행 실패"));
		CloseHandle(m_hChildStd_OUT_Rd);
		CloseHandle(m_hChildStd_OUT_Wr);
		m_hChildStd_OUT_Rd = NULL;
		m_hChildStd_OUT_Wr = NULL;
		return FALSE;
	}

	// 프로세스 실행 성공 후
	m_frpcProcess = pi;

	// 부모는 쓰기 핸들이 필요 없으므로 즉시 닫음 -> EOF 처리 가능
	if (m_hChildStd_OUT_Wr)
	{
		CloseHandle(m_hChildStd_OUT_Wr);
		m_hChildStd_OUT_Wr = NULL;
	}

	// 로그 읽기 스레드 시작
	m_pLogThread = AfxBeginThread(LogReaderThread, this);

	return TRUE;
}


void CfrpcguiDlg::StopFrpcProcess()
{
	// 스레드 종료 요청
	m_stopEvent.SetEvent();

	if (m_frpcProcess.hProcess != NULL)
	{
		// 정상 종료 유도(방법이 없다면 바로 terminate)
		if (WaitForSingleObject(m_frpcProcess.hProcess, 1000) == WAIT_TIMEOUT)
		{
			// 아직 프로세스가 종료되지 않았으므로 강제 종료
			TerminateProcess(m_frpcProcess.hProcess, 0);
			WaitForSingleObject(m_frpcProcess.hProcess, 2000);
		}

		CloseHandle(m_frpcProcess.hThread);
		CloseHandle(m_frpcProcess.hProcess);
		m_frpcProcess.hProcess = NULL;
		m_frpcProcess.hThread = NULL;
	}

	// 로그 스레드 종료 대기
	if (m_pLogThread != NULL)
	{
		WaitForSingleObject(m_pLogThread->m_hThread, 2000);
		// 스레드 핸들은 MFC에서 자동 정리
		m_pLogThread = NULL;
	}

	// 파이프 핸들 정리
	if (m_hChildStd_OUT_Rd) {
		CloseHandle(m_hChildStd_OUT_Rd);
		m_hChildStd_OUT_Rd = NULL;
	}
	if (m_hChildStd_OUT_Wr) {
		CloseHandle(m_hChildStd_OUT_Wr);
		m_hChildStd_OUT_Wr = NULL;
	}

	// 이벤트 리셋 (다음 시작을 위해)
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
			// EOF or 프로세스 종료
			break;
		}

		buf[dwRead] = '\0';
		buffer.append(buf);

		// 개행(\n, \r\n)을 기준으로 라인 파싱
		size_t pos = 0;
		while ((pos = buffer.find_first_of("\r\n")) != std::string::npos)
		{
			std::string line = buffer.substr(0, pos);
			buffer.erase(0, buffer.find_first_not_of("\r\n", pos));

			// 라인이 비어있을 수 있으므로 체크
			if (!line.empty())
			{
				CString wLine = CA2T(line.c_str(), CP_ACP);
				pDlg->PostMessage(WM_USER_LOG_LINE, 0, (LPARAM)new CString(wLine));
			}
		}

		// 혹시 개행이 전혀 없는 경우, 일정 길이 이상이면 출력
		if (buffer.size() > 1024)
		{
			// 임시로 누적된 데이터를 한 번에 출력
			CString wLine = CA2T(buffer.c_str(), CP_ACP);
			pDlg->PostMessage(WM_USER_LOG_LINE, 0, (LPARAM)new CString(wLine));
			buffer.clear();
		}
	}

	// 남은 데이터 출력
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
			AddLogMessage(_T("포트 충돌 발생, 다른 포트로 재시도합니다."));
			m_retryCount++;
			m_remotePort = 0; // 랜덤 포트 할당
			StopFrpcProcess();
			StartFrpcProcess(TRUE);
		}
		else
		{
			AddLogMessage(_T("포트 재시도 횟수를 초과했습니다. 프로세스를 중지합니다."));
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
		AddLogMessage(_T("중복된 장치명입니다. 장치명을 변경해주세요."));
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
		AddLogMessage(_T("토큰이 일치하지 않습니다. 토큰을 확인해주세요."));
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
		AddLogMessage(_T("서버에 성공적으로 접속하였습니다."));
	}
	else if (lowerLine.Find(_T("start proxy success")) != -1)
	{
		m_successStarted = TRUE;
		AddLogMessage(_T("프록시가 성공적으로 시작되었습니다."));
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
		// 랜덤 포트 할당
		srand((UINT)time(NULL));
		return rand() % (65535 - 10000) + 10000;
	}
	if (desiredPort >= 1 && desiredPort <= 65535)
		return desiredPort;
	return -1; // invalid
}


void CfrpcguiDlg::GenerateFrpcIni(int remotePort, const CString & proxyName)
{
	// 컨트롤의 값을 가져오기
	UpdateData(TRUE);

	CString iniText;
	iniText.Format(
		_T("[common]\r\nserver_addr=%s\r\nserver_port=%d\r\ntoken=%s\r\nauto_start=%d\r\n\r\n[%s]\r\ntype=tcp\r\nlocal_port=%d\r\nremote_port=%d\r\n"),
		m_serverAddr, m_serverPort, m_token, m_autoStart ? 1 : 0,
		proxyName,
		m_localPort, remotePort
	);

	// UTF-8로 저장
	// 유니코드 → UTF-8
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
		file.Write(mbBuf.data(), (UINT)(mbLen - 1)); // -1은 널문자 제외
		file.Close();
	}
}


CString CfrpcguiDlg::GenerateProxyName()
{
	CString name = m_deviceName;
	name.Trim();
	if (name.IsEmpty())
	{
		// 랜덤 생성
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

