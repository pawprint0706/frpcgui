
// frpcgui.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// CfrpcguiApp:
// �� Ŭ������ ������ ���ؼ��� frpcgui.cpp�� �����Ͻʽÿ�.
//

class CfrpcguiApp : public CWinApp
{
public:
	CfrpcguiApp();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern CfrpcguiApp theApp;