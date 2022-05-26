/*********************************************************************NVMH4****
Path:  NVSDK\Common\src\nv_debug
File:  DebugDlg.h

Copyright NVIDIA Corporation 2002
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED
*AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS
BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES
WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS,
BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS)
ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS
BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.



Comments:

A debug window dialog implementation of IDebugConsole.




******************************************************************************/

#if !defined(AFX_DEBUGDLG_H__D72B70F5_9A9D_46EC_8C74_17391267E7F6__INCLUDED_)
#define AFX_DEBUGDLG_H__D72B70F5_9A9D_46EC_8C74_17391267E7F6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DebugDlg.h : header file
// public
#include <iostream>
#pragma warning(disable: 4786)
#include <sstream>
#pragma warning(disable: 4786)
#include <iomanip>
#pragma warning(disable: 4786)
#include <strstream>
#pragma warning(disable: 4786)
#include <fstream>
#pragma warning(disable: 4786)

/////////////////////////////////////////////////////////////////////////////
// CDebugDlg dialog
namespace nv_gui
{

class CDebugDlg : public CDialog,  public INVDebugConsole
{
// Construction
public:
	CDebugDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDebugDlg();

	static bool INTCALLTYPE CreateNVObject(const nv_sys::INVCreator* pCreator, const nv_sys::NVGUID& InterfaceID, void** ppObj);

	// IDebugConsole
	virtual unsigned long INTCALLTYPE AddRef();
	virtual unsigned long INTCALLTYPE Release();
	virtual bool INTCALLTYPE GetInterface(const nv_sys::NVGUID& Interface, void** ppvObj);

    virtual bool INTCALLTYPE OutputDebug(const char* pszChar);
	virtual bool INTCALLTYPE SetVisible(bool bHide);
	virtual bool INTCALLTYPE IsVisible();
	virtual bool INTCALLTYPE SetLogFile(const char* pszLogFile);
	virtual bool INTCALLTYPE SetTitle(const char* pszTitle);


// Dialog Data
	//{{AFX_DATA(CDebugDlg)
	enum { IDD = IDD_DEBUGDLG };
	CListBox	m_ListBox;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDebugDlg)
	public:
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	unsigned long m_dwRefCount;
	CString m_strLogFile;
	std::ostrstream m_strStream;
	std::ofstream m_dbgLog;

	// Generated message map functions
	//{{AFX_MSG(CDebugDlg)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

}; //namespace nv_gui

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEBUGDLG_H__D72B70F5_9A9D_46EC_8C74_17391267E7F6__INCLUDED_)
