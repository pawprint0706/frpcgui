
// frpcguiDlg.h : 헤더 파일
//

#pragma once
#include "afxwin.h"
#include <string>

// 사용자 정의 메시지
#define WM_USER_TRAY_ICON (WM_USER + 100)	// 트레이 아이콘 제어용
#define WM_USER_HIDE_WINDOW (WM_USER + 101)	// 창 숨기기용
#define WM_USER_LOG_LINE (WM_USER + 102)	// 로그라인 수신용

// 포트 재시도 최대 횟수
#define MAX_RETRIES 30

// CfrpcguiDlg 대화 상자
class CfrpcguiDlg : public CDialogEx
{
	// 생성입니다.
public:
	CfrpcguiDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FRPCGUI_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;
	NOTIFYICONDATA m_trayIconData; // 트레이 아이콘 데이터

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	// 멤버변수 선언
	CString m_deviceName;	// 장치명(프록시 이름)
	CString m_serverAddr;	// 서버 주소
	int m_serverPort;		// 서버 포트
	CString m_token;		// 인증 토큰
	int m_localPort;		// 로컬 포트
	int m_remotePort;		// 원격 포트
	CEdit m_edit_output;	// 출력창 컨트롤
	BOOL m_autoStart;		// 자동연결체크 컨트롤
	TCHAR m_szTargetFile[MAX_PATH];		// frpc.exe 파일 경로
	PROCESS_INFORMATION m_frpcProcess;	// 실행된 frpc 프로세스
	HANDLE m_hChildStd_OUT_Rd;			// stdout 리다이렉트 파이프 핸들(Read)
	HANDLE m_hChildStd_OUT_Wr;			// stdout 리다이렉트 파이프 핸들(Write)
	CWinThread* m_pLogThread;			// 로그 읽기용 쓰레드 핸들
	CEvent m_stopEvent;		// 스레드 정지 요청 이벤트
	int m_retryCount;		// 포트 재시도 관련 변수
	BOOL m_successStarted;	// 프록시 시작 성공 여부

	// 멤버함수 선언
	afx_msg LRESULT OnTrayIconNotify(WPARAM wParam, LPARAM lParam); // 트레이 아이콘
	afx_msg LRESULT OnHideWindow(WPARAM wParam, LPARAM lParam);		// 창 숨기기
	afx_msg void OnContextMenuShowWindow();	// 트레이 컨텍스트 메뉴의 창보이기
	afx_msg void OnContextMenuExit();		// 트레이 컨텍스트 메뉴의 종료
	afx_msg void OnDestroy();	// 종료 처리
	afx_msg void OnClose();		// 닫기 처리
	void LoadConf();			// 설정파일 읽기
	void SaveConf();			// 설정파일 저장
	afx_msg void OnBnClickedButtonStart();	// 시작 버튼
	afx_msg void OnBnClickedButtonStop();	// 중지 버튼
	std::wstring Trim(const std::wstring & str);	// WhiteSpace 제거 함수
	void XorData(BYTE * pData, DWORD dwSize, const BYTE * pKey, size_t keyLen);	// XOR 복호화 함수
	BOOL ExtractAndRunEncryptedExeFromResource();	// 리소스에서 XOR 암호화된 데이터를 로드하는 함수
	BOOL StartFrpcProcess(BOOL isRetry);	// frpc 프로세스 시작 함수
	void StopFrpcProcess();					// frpc 프로세스 중지 함수
	static UINT LogReaderThread(LPVOID pParam);		// frpc 출력 읽기 쓰레드
	afx_msg LRESULT OnLogLine(WPARAM wParam, LPARAM lParam);		// 로그 출력
	int GetValidPortOrRandom(int desiredPort);		// 포트 유효성 검사 및 랜덤 포트 생성 함수
	void GenerateFrpcIni(int remotePort, const CString& proxyName);	// frpc.ini 생성 함수
	CString GenerateProxyName();	// 랜덤 프록시 이름 생성 함수
	void CfrpcguiDlg::AddLogMessage(const CString& strMessage);		// 로그 메시지 출력 함수
};
