
// CityGML2IFC.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CCityGML2IFCApp:
// See CityGML2IFC.cpp for the implementation of this class
//

class CCityGML2IFCApp : public CWinApp
{
public:
	CCityGML2IFCApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CCityGML2IFCApp theApp;
