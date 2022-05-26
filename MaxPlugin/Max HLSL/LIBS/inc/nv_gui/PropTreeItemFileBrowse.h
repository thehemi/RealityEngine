/*********************************************************************NVMH4****
Path:  NVSDK\Common\include\nv_gui
File:  PropTreeItemFileBrowse.h

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

#if !defined(AFX_PROPTREEITEMFILEBROWSE_H__642536B1_1162_4F99_B09D_9B1BD2CF88B6__INCLUDED_)
#define AFX_PROPTREEITEMFILEBROWSE_H__642536B1_1162_4F99_B09D_9B1BD2CF88B6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PropTreeItemFileBrowse.h : header file
//
//  Copyright (C) 1998-2001 Scott Ramsay
//	sramsay@gonavi.com
//	http://www.gonavi.com
//
//  This material is provided "as is", with absolutely no warranty expressed
//  or implied. Any use is at your own risk.
// 
//  Permission to use or copy this software for any purpose is hereby granted 
//  without fee, provided the above notices are retained on all copies.
//  Permission to modify the code and to distribute modified code is granted,
//  provided the above notices are retained, and a notice that the code was
//  modified is included with the above copyright notice.
// 
//	If you use this code, drop me an email.  I'd like to know if you find the code
//	useful.

#include "PropTreeItem.h"
#include "fileedit.h"

/////////////////////////////////////////////////////////////////////////////
// CPropTreeItemFileBrowse window

class CPropTreeItemFileBrowse : public CWnd, public CPropTreeItem
{
// Construction
public:
	CPropTreeItemFileBrowse(nv_gui::INVGUIEvent* pEventSink);
	virtual ~CPropTreeItemFileBrowse();

// Attributes
public:
	// The attribute area needs drawing
	virtual void DrawAttribute(CDC* pDC, const RECT& rc);

	// Retrieve the item's attribute value
	virtual LPARAM GetItemValue();

	// Set the item's attribute value
	virtual void SetItemValue(LPARAM lParam);

	// Called when attribute area has changed size
	virtual void OnMove();

	// Called when the item needs to refresh its data
	virtual void OnRefresh();

	// Called when the item needs to commit its changes
	virtual void OnCommit();

    // Called when the item doesn't need to commit its changes
    virtual void OnDontCommit();

	// Called to activate the item
	virtual void OnActivate();

	
	enum ValueFormat
	{
		ValueFormatText,
		ValueFormatNumber,
		ValueFormatFloatPointer
	};

	// Set to specifify format of SetItemValue/GetItemValue
	void SetValueFormat(ValueFormat nFormat);
		
	// Set to TRUE for to use a password edit control
	void SetAsPassword(BOOL bPassword);

	void SetTruncateDisplay(BOOL bTruncate);

    void SetPreview(BOOL bPreview);

	void SetExtensionString(const char* pszExtensions);


    LRESULT CPropTreeItemFileBrowse::OnDefId(WPARAM wParam, LPARAM lParam);
    
protected:
	CString	    m_pszExtensions;
    CString     m_sInitialDir;
	BOOL		m_bTruncateDisplay;
    BOOL		m_bPreview;
	CString		m_sEdit;
	float		m_fValue;

	ValueFormat m_nFormat;
	BOOL		m_bPassword;

    BOOL        m_bInBrowseDialog;
    BOOL        m_bInEditDialog;

    CEditFile   m_Edit;
    BOOL        m_bEdit;
    CButton     m_bBrowseButton;
    CButton     m_bEditButton;

    CBitmap     m_Bitmap;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPropTreeItemFileBrowse)
	//}}AFX_VIRTUAL

// Implementation
public:

	// Generated message map functions
protected:
    BOOL        GetPath(const CString & sFullPathName, CString & path);
    void        UpdateBitmapPreview();
    void        LoadDefault();

	//{{AFX_MSG(CPropTreeItemFileBrowse)
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnKillFocus( CWnd* );
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnEditKillfocus();
    afx_msg void OnBrowse();
    afx_msg void OnEdit();
    //}}AFX_MSG

	DECLARE_MESSAGE_MAP()
public:
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROPTREEITEMFILEBROWSE_H__642536B1_1162_4F99_B09D_9B1BD2CF88B6__INCLUDED_)
