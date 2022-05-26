/*********************************************************************NVMH4****
Path:  NVSDK\Common\src\nv_gui
File:  PropTreeItemGrid.cpp

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

// PropTreeItemFileBrowse.cpp : implementation file
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

#include "stdafx.h"
#include "nv_gui\proptree.h"
#include "nv_gui\PropTreeItemgrid.h"
#include "griddlg.h"
#include <sstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPropTreeItemGrid

CPropTreeItemGrid::CPropTreeItemGrid(nv_gui::INVGUIEvent* pEventSink) :
	CPropTreeItem(pEventSink),
	m_sEdit(_T("")),
	m_nFormat(ValueFormatText),
    m_bInBrowseDialog(FALSE),
    m_bEdit(FALSE)
{
}

CPropTreeItemGrid::~CPropTreeItemGrid()
{
}


BEGIN_MESSAGE_MAP(CPropTreeItemGrid, CWnd)
	//{{AFX_MSG_MAP(CPropTreeItemGrid)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
    ON_WM_KILLFOCUS()
    ON_EN_KILLFOCUS(100, OnEditKillfocus)
    ON_WM_CREATE()
    ON_BN_CLICKED(101, OnBrowse)
    //}}AFX_MSG_MAP
//	ON_WM_DESTROY()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropTreeItemGrid message handlers
void CPropTreeItemGrid::DrawAttribute(CDC* pDC, const RECT& rc)
{
	ASSERT(m_pProp!=NULL);

	pDC->SelectObject(IsReadOnly() ? m_pProp->GetNormalFont() : m_pProp->GetBoldFont());
    pDC->SetTextColor(IsReadOnly() ? m_pProp->GetReadOnlyColor() : m_pProp->GetColor());
	pDC->SetBkMode(TRANSPARENT);

    CRect r = rc;
    if (IsActivated())
        r.right -= 40;

    std::ostringstream strStream;
	tvecFloats::iterator itrFloats;
	itrFloats = m_Floats.begin();
	while (itrFloats != m_Floats.end())
	{
		strStream << *itrFloats;
		itrFloats++;

		if (itrFloats != m_Floats.end())
			strStream << ", ";
	}

	pDC->DrawText(strStream.str().c_str(), r, DT_SINGLELINE|DT_VCENTER);
}

void CPropTreeItemGrid::SetValueFormat(ValueFormat nFormat)
{
	m_nFormat = nFormat;
}

void CPropTreeItemGrid::OnMove()
{
	if (IsWindow(m_hWnd))
    {
        SetWindowPos(NULL, m_rc.left, m_rc.top, m_rc.Width(), m_rc.Height(), SWP_NOZORDER|SWP_NOACTIVATE);
        CRect rect;
        GetClientRect(rect);
        m_bBrowseButton.SetWindowPos(NULL,rect.right - 40, 0,40,rect.Height(),SWP_NOZORDER|SWP_NOACTIVATE);

        CRect r(m_rc);
        r.left = 0;
        r.right = m_rc.Width();

        if (IsActivated())
            r.right -= 40;

        r.top = 0;
        r.bottom = m_rc.Height();
        m_Edit.SetWindowPos(NULL,0, 0, r.right, r.bottom,SWP_NOZORDER|SWP_NOACTIVATE);
    }
		
}


void CPropTreeItemGrid::OnRefresh()
{
	if (IsWindow(m_hWnd) && m_bEdit==FALSE)
		SetWindowText(m_sEdit);
}

void CPropTreeItemGrid::OnDontCommit()
{
   	ShowWindow(SW_HIDE);
}

void CPropTreeItemGrid::OnCommit()
{
	// hide edit control
    //m_Edit.SetWindowText(m_sEdit);
    //m_Edit.ShowWindow(SW_HIDE);
    m_bEdit = FALSE;
   	ShowWindow(SW_HIDE);
	// store edit text for GetItemValue
	GetWindowText(m_sEdit);
    Invalidate(TRUE);
}


void CPropTreeItemGrid::OnActivate()
{
	// Check if the edit control needs creation
    CRect r(m_rc);
    r.left = 0;
    r.right = m_rc.Width();
    r.right -= 40;

    r.top = 0;
    r.bottom = m_rc.Height();
	if (!IsWindow(m_hWnd))
	{
        LPCTSTR pszClassName;

		pszClassName = AfxRegisterWndClass(CS_VREDRAW|CS_HREDRAW, LoadCursor(NULL, IDC_ARROW), 0);
		
		DWORD dwStyle = WS_CHILD|WS_VISIBLE;

		CreateEx(0, pszClassName, _T(""), dwStyle, m_rc, m_pProp->GetCtrlParent(), 0);

		SendMessage(WM_SETFONT, (WPARAM)m_pProp->GetNormalFont()->m_hObject);
        
        dwStyle = WS_CHILD|ES_AUTOHSCROLL;
        m_Edit.Create(dwStyle, r, this, 100);
        m_Edit.SendMessage(WM_SETFONT, (WPARAM)m_pProp->GetNormalFont()->m_hObject);
        
        CRect rect;
        GetClientRect(rect);
        rect.left = rect.right;
        rect.left -= 40;
        m_bBrowseButton.Create("Edit...",WS_VISIBLE|WS_CHILD|WS_TABSTOP|BS_PUSHBUTTON,rect, this, 101);
        m_bBrowseButton.SendMessage(WM_SETFONT, (WPARAM)m_pProp->GetNormalFont()->m_hObject);
    
    }
    CRect rect;
    GetClientRect(rect);
    SetWindowPos(NULL, m_rc.left, m_rc.top, m_rc.Width(), m_rc.Height(), SWP_NOZORDER|SWP_SHOWWINDOW);

    m_bBrowseButton.ModifyStyle(WS_DISABLED,0);
    m_bBrowseButton.ShowWindow(SW_SHOW);
    m_bBrowseButton.SetWindowPos(NULL,rect.right - 40, 0,40,rect.Height(),SWP_NOZORDER|SWP_NOACTIVATE);

    m_Edit.SetReadOnly(TRUE);
    m_Edit.SetPasswordChar(0);
	//m_Edit.SetWindowText(m_sEdit);
	//m_Edit.SetSel(0, -1);
    m_Edit.SetWindowPos(NULL,0, 0, r.right, r.bottom,SWP_NOZORDER|SWP_NOACTIVATE);


    SetFocus();
	if (m_pEventSink)
		m_pEventSink->OnSetFocus();

}

void CPropTreeItemGrid::OnMouseMove(UINT, CPoint point) 
{
	BOOL bEdit;

    CRect rect;
    m_Edit.GetWindowRect(rect);
    bEdit = rect.PtInRect(point);

	if (bEdit!=m_bEdit)
	{
        m_bEdit = bEdit;
		Invalidate(FALSE);
	}
}

void CPropTreeItemGrid::OnLButtonDown(UINT nFlags, CPoint point) 
{
    CRect rect;
    m_Edit.GetWindowRect(rect);
    ScreenToClient(rect);
    if (rect.PtInRect(point) && IsActivated())
    {
        m_bBrowseButton.ModifyStyle(WS_DISABLED,0);
        m_bBrowseButton.RedrawWindow();
        m_bBrowseButton.SetFocus();

        //m_Edit.ShowWindow(SW_SHOW);
        //m_Edit.SetFocus();
        //m_bEdit = TRUE;
    }
}

LPARAM CPropTreeItemGrid::GetItemValue()
{
	return reinterpret_cast<LPARAM>(&m_Floats[0]);
}


void CPropTreeItemGrid::SetItemValue(LPARAM lParam)
{
	memcpy(&m_Floats[0], reinterpret_cast<float*>(lParam), sizeof(float) * m_x * m_y);
}

void CPropTreeItemGrid::SetTitle(const char* pszTitle)
{
    m_strTitle = pszTitle;
}

void CPropTreeItemGrid::SetDimensions(unsigned int y, unsigned int x)
{
	m_x = x;
	m_y = y;
	m_Floats.resize(x * y);
}

void CPropTreeItemGrid::OnEditKillfocus()
{
 	if (!m_bInBrowseDialog)
    {
        //m_Edit.GetWindowText(m_sEdit);
        SetWindowText(m_sEdit);
        CommitChanges();
		if (m_pEventSink)
			m_pEventSink->OnKillFocus();
    }
}

void CPropTreeItemGrid::OnBrowse()
{
    m_bInBrowseDialog = TRUE;
	ASSERT(m_pProp!=NULL);
	m_pProp->DisableInput();

    CGridDlg GridDialog(CWnd::GetParent());
    GridDialog.SetDimensions(m_x, m_y);
    GridDialog.SetTitle(m_strTitle.c_str());

    unsigned int y, x;

    for (y = 0; y < m_y; y++)
	{
		for (x = 0; x < m_x; x++)
		{
			GridDialog.SetValue(x, y, m_Floats[(y * m_x) + x]);
		}
	}

    GridDialog.DoModal();

    for (y = 0; y < m_y; y++)
	{
		for (x = 0; x < m_x; x++)
		{
			 m_Floats[(y * m_x) + x] = GridDialog.GetValue(x, y);
		}
	}
    CommitChanges();

    m_pProp->DisableInput(FALSE);
    m_bInBrowseDialog = FALSE;
}

int CPropTreeItemGrid::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	return 0;
}

void CPropTreeItemGrid::OnKillFocus( CWnd* pWnd)
{
    if (pWnd != &m_Edit && 
        pWnd != &m_bBrowseButton)
    {
        OnRefresh();
        DontCommitChanges();
    }
            
}

