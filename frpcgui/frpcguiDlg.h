
// frpcguiDlg.h : 헤더 파일
//

#pragma once
#include "afxwin.h"
#include <string>


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
	TCHAR m_szTargetFile[MAX_PATH];	// frpc.exe 파일 경로
	PROCESS_INFORMATION m_frpcProcess;	// 실행된 frpc 프로세스

	// 멤버함수 선언
	afx_msg LRESULT OnTrayIconNotify(WPARAM wParam, LPARAM lParam); // 트레이 아이콘
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
};
