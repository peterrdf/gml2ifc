
// CityGML2IFCDlg.h : header file
//

#pragma once

#include "engine.h"
#include "ifcengine.h"
#include "gisengine.h"

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#include <string>
#include <bitset>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <time.h>
#include <map>
using namespace std;

// CCityGML2IFCDlg dialog
class CCityGML2IFCDlg : public CDialogEx
{

private: // Members

	CWinThread* m_pThread;

	wstring m_strRootFolder;

protected: // Methods
	
	static UINT ThreadProc(LPVOID pParam);
	void ExportFile(const wstring& strInputFile);
	void ExportFiles(const fs::path& pthInputFolder);

// Construction
public:
	CCityGML2IFCDlg(CWnd* pParent = nullptr);	// standard constructor
	~CCityGML2IFCDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CITYGML2IFC_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	CString m_strInputFile;
	afx_msg void OnBnClickedButtonInputFile();
	CEdit m_edtProgress;
	afx_msg void OnBnClickedButtonClose();
};
