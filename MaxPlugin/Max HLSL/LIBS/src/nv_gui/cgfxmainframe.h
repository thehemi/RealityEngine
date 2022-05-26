/*********************************************************************NVMH4****
Path:  NVSDK\Common\src\nv_gui
File:  cgfxmainframe.h

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

#if !defined(AFX_CGFXMAINFRAME_H__B3529A72_BA9B_408C_822F_CC19FB007D4A__INCLUDED_)
#define AFX_CGFXMAINFRAME_H__B3529A72_BA9B_408C_822F_CC19FB007D4A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// cgfxmainframe.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCGFXMainFrame frame

class CCGFXMainFrame : public CFrameWnd
{
	DECLARE_DYNCREATE(CCGFXMainFrame)
public:
	CCGFXMainFrame();           // protected constructor used by dynamic creation

// Attributes
public:
	CStatusBar          m_wndStatusBar;
    BOOL                m_bCreated;
    
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCGFXMainFrame)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CCGFXMainFrame();

	// Generated message map functions
	//{{AFX_MSG(CCGFXMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnMove(int x, int y);
	//}}AFX_MSG
	LRESULT OnIdleUpdateCmdUI(WPARAM wParam, LPARAM);

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CGFXMAINFRAME_H__B3529A72_BA9B_408C_822F_CC19FB007D4A__INCLUDED_)
