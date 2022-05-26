/*********************************************************************NVMH4****
Path:  NVSDK\Common\include\nv_gui
File:  gui.h

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





******************************************************************************/

// nvgui.h : main header file for the CGFXPAN DLL
//

#if !defined(AFX_NVGUI_H__9B880535_D79E_4F59_8425_D99E5C53723A__INCLUDED_)
#define AFX_NVGUI_H__9B880535_D79E_4F59_8425_D99E5C53723A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "editcmd.h"
#include "resource.h"		// main symbols

#include <nv_gui\invgui.h>

namespace nv_gui
{

/////////////////////////////////////////////////////////////////////////////
// CNVGUIApp
// See nvgui.cpp for the implementation of this class
//
class CDynamicDlg;

class CNVGUIApp : public CWinApp
{
public:
	CNVGUIApp();

    CSingleDocTemplate* m_pDocTemplate;
    HWND                m_hParent;
    std::string         m_AppName;

    void                SetAppName(const char * pAppName);
    const char *        GetAppName();

    bool                ReadWindowPosRegKey(const char * KeyName, HWND hwnd);
    bool                WriteWindowPosRegKey(const char * KeyName, HWND hwnd);
	void				AddDialog(CDialog* pDlg);
	void				RemoveDialog(CDialog* pDlg);
	void				UpdateDialogs();
    
    // persistence of folder locations wrt file extensions
    const char *        GetInitialPath(const char * ext);
    bool                SetInitialPath(const char * ext, const char * path);

    bool                ReadInitialPathRegKey();
    bool                WriteInitialPathRegKey();

	std::set<CDialog*> m_setDlg;
    std::map<std::string, std::string> m_PathExtMap;
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNVGUIApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CNVGUIApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
extern CNVGUIApp theApp;

}; // namespace nvgui
/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CGFXPAN_H__9B880535_D79E_4F59_8425_D99E5C53723A__INCLUDED_)
