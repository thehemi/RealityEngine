/*********************************************************************NVMH4****
Path:  NVSDK\Common\src\nv_gui
File:  CFindTextDlg.h

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

////////////////////////////////////////////////////////////////////////////
//	File:		CFindTextDlg.h
//	Version:	1.0.0.0
//	Created:	29-Dec-1998
//
//	Author:		Stcherbatchenko Andrei
//	E-mail:		windfall@gmx.de
//
//	Declaration of the CFindTextDlg dialog, a part of Crystal Edit -
//	syntax coloring text editor.
//
//	You are free to use or modify this code to the following restrictions:
//	- Acknowledge me somewhere in your about box, simple "Parts of code by.."
//	will be enough. If you can't (or don't want to), contact me personally.
//	- LEAVE THIS HEADER INTACT
////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_CFINDTEXTDLG_H__F59009E3_7B01_11D2_8C4F_0080ADB86836__INCLUDED_)
#define AFX_CFINDTEXTDLG_H__F59009E3_7B01_11D2_8C4F_0080ADB86836__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "cedefs.h"
#include "editcmd.h"

class CCrystalTextView;


/////////////////////////////////////////////////////////////////////////////
// CFindTextDlg dialog

class CFindTextDlg : public CDialog
{
private:
	CCrystalTextView *m_pBuddy;

// Construction
public:
	CFindTextDlg(CCrystalTextView *pBuddy);

	CPoint m_ptCurrentPos;
// Dialog Data
	//{{AFX_DATA(CFindTextDlg)
	enum { IDD = IDD_EDIT_FIND };
	int		m_nDirection;
	BOOL	m_bMatchCase;
	CString	m_sText;
	BOOL	m_bWholeWord;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFindTextDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFindTextDlg)
	virtual void OnOK();
	afx_msg void OnChangeEditText();
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CFINDTEXTDLG_H__F59009E3_7B01_11D2_8C4F_0080ADB86836__INCLUDED_)
