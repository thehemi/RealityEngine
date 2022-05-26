/*********************************************************************NVMH4****
Path:  NVSDK\Common\src\nv_gui
File:  GridDlg.h

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

#if !defined(AFX_GRIDDLG_H__02F55749_3A73_4096_9604_319D7EC1811F__INCLUDED_)
#define AFX_GRIDDLG_H__02F55749_3A73_4096_9604_319D7EC1811F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GridDlg.h : header file
//

#include "nv_gui\GridCtrl.h"
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CGridDlg dialog

class CGridDlg : public CDialog
{
// Construction
public:
	CGridDlg(CWnd* pParent = NULL);   // standard constructor

    void SetDimensions(unsigned int x, unsigned int y);
    void SetValue(unsigned int x, unsigned int y, float fVal);
    float GetValue(unsigned int x, unsigned int y);
    void SetTitle(const char* pszTitle);

// Dialog Data
	//{{AFX_DATA(CGridDlg)
	enum { IDD = IDD_GRID };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGridDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
    CGridCtrl m_Grid;
   	tvecFloats	m_Floats;
    unsigned int m_x;
    unsigned int m_y;
    std::string m_strTitle;

	// Generated message map functions
	//{{AFX_MSG(CGridDlg)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GRIDDLG_H__02F55749_3A73_4096_9604_319D7EC1811F__INCLUDED_)
