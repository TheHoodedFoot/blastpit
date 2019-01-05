// blastpit.h : main header file for the blastpit DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CblastpitApp
// See blastpit.cpp for the implementation of this class
//

class CblastpitApp : public CWinApp
{
public:
	CblastpitApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
