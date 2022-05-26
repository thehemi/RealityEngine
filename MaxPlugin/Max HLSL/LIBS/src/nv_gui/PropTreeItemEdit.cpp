/*********************************************************************NVMH4****
Path:  NVSDK\Common\src\nv_gui
File:  PropTreeItemEdit.cpp

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

// PropTreeItemEdit.cpp : implementation file
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
#include "nv_gui\PropTreeItemEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPropTreeItemEdit

CPropTreeItemEdit::CPropTreeItemEdit(nv_gui::INVGUIEvent* pEventSink) :
	CPropTreeItem(pEventSink),
	m_sEdit(_T("")),
	m_nFormat(ValueFormatText),
	m_bPassword(FALSE),
    m_bEdit(FALSE),
	m_fValue(0.0f),
    m_bUseRange(TRUE)
{
    SetRangeParams(0.0f,10.0f,1);
}

CPropTreeItemEdit::~CPropTreeItemEdit()
{
}


BEGIN_MESSAGE_MAP(CPropTreeItemEdit, CWnd)
	//{{AFX_MSG_MAP(CPropTreeItemEdit)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
    ON_WM_KILLFOCUS()
    ON_EN_KILLFOCUS(100, OnEditKillfocus)
    ON_WM_HSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropTreeItemEdit message handlers

void CPropTreeItemEdit::DrawAttribute(CDC* pDC, const RECT& rc)
{
	ASSERT(m_pProp!=NULL);

	pDC->SelectObject(IsReadOnly() ? m_pProp->GetNormalFont() : m_pProp->GetBoldFont());
	pDC->SetTextColor(IsReadOnly() ? m_pProp->GetReadOnlyColor() : m_pProp->GetColor());
	pDC->SetBkMode(TRANSPARENT);

	CRect r = rc;

	TCHAR ch;

	// can't use GetPasswordChar(), because window may not be created yet
	ch = (m_bPassword) ? '*' : '\0';

	if (ch)
	{
		CString s;

		s = m_sEdit;
		for (LONG i=0; i<s.GetLength();i++)
			s.SetAt(i, ch);

		pDC->DrawText(s, r, DT_SINGLELINE|DT_VCENTER);
	}
	else
	{
		pDC->DrawText(m_sEdit, r, DT_SINGLELINE|DT_VCENTER);
	}

    if (IsActivated())
    {
        r = rc;
        r.right = (m_nFormat == ValueFormatFloatPointer && m_bUseRange) ? r.left + 100 : r.right;
        r.DeflateRect(0,1,1,1);
        DrawFocusRect(pDC->GetSafeHdc(),r);
    }
}



void CPropTreeItemEdit::SetAsPassword(BOOL bPassword)
{
	m_bPassword = bPassword;
}


void CPropTreeItemEdit::SetValueFormat(ValueFormat nFormat)
{
	m_nFormat = nFormat;
}

void CPropTreeItemEdit::SetAsRange(BOOL bRange)
{
    m_bUseRange = bRange;
}

void CPropTreeItemEdit::SetRangeParams(float min, float max, float delta)
{
    m_fMinValue = min;
    m_fMaxValue = max;
    m_fDeltaValue = 50;

    m_fMultiplier = m_fDeltaValue / (m_fMaxValue - m_fMinValue);
}

LPARAM CPropTreeItemEdit::GetItemValue()
{
	switch (m_nFormat)
	{
		case ValueFormatNumber:
			return _ttoi(m_sEdit);

		case ValueFormatFloatPointer:
			_stscanf(m_sEdit, _T("%f"), &m_fValue);
			return (LPARAM)&m_fValue;
	}

	return (LPARAM)(LPCTSTR)m_sEdit;
}


void CPropTreeItemEdit::SetItemValue(LPARAM lParam)
{
	switch (m_nFormat)
	{
		case ValueFormatNumber:
			m_sEdit.Format(_T("%d"), lParam);
			return;

		case ValueFormatFloatPointer:
			{
				TCHAR tmp[MAX_PATH];
				m_fValue = *(float*)lParam;
				if(m_fValue > m_fMaxValue)
					SetRangeParams(m_fMinValue,m_fValue,1);
				_stprintf(tmp, _T("%f"), m_fValue);
				m_sEdit = tmp;
			}
			return;
	}

	if (lParam==0L)
	{
		TRACE0("CPropTreeItemEdit::SetItemValue - Invalid lParam value\n");
		return;
	}

	m_sEdit = (LPCTSTR)lParam;

    if (m_nFormat == ValueFormatFloatPointer && m_bUseRange && ::IsWindow(m_Slider.GetSafeHwnd()))
        m_Slider.SetPos((int)(m_fValue * m_fMultiplier));
}


void CPropTreeItemEdit::OnMove()
{
	if (IsWindow(m_hWnd))
    {
        SetWindowPos(NULL, m_rc.left, m_rc.top, m_rc.Width(), m_rc.Height(), SWP_NOZORDER|SWP_NOACTIVATE);
        if (m_nFormat == ValueFormatFloatPointer && m_bUseRange)
        {
            CRect rect;
            GetClientRect(rect);
            rect.left = rect.left + 100;
            m_Slider.SetWindowPos(NULL,rect.left, 0,rect.Width(),rect.Height(),SWP_NOZORDER|SWP_NOACTIVATE);
        }
        CRect r(m_rc);
        r.left = 0;
        r.right = (m_nFormat == ValueFormatFloatPointer && m_bUseRange) ? 100 : m_rc.Width();
        r.top = 0;
        r.bottom = m_rc.Height();
        m_Edit.SetWindowPos(NULL,0, 0, r.right, r.bottom,SWP_NOZORDER|SWP_NOACTIVATE);
    }
}


void CPropTreeItemEdit::OnRefresh()
{
	if (IsWindow(m_hWnd))
		SetWindowText(m_sEdit);
}

void CPropTreeItemEdit::OnDontCommit()
{
   	ShowWindow(SW_HIDE);
}

void CPropTreeItemEdit::OnCommit()
{
	// hide edit control
    m_Edit.SetWindowText(m_sEdit);
    m_Edit.ShowWindow(SW_HIDE);
    if (m_nFormat == ValueFormatFloatPointer && m_bUseRange)
        m_Slider.ShowWindow(SW_HIDE);
    m_bEdit = FALSE;
   	ShowWindow(SW_HIDE);

	// store edit text for GetItemValue
	GetWindowText(m_sEdit);
    //Invalidate(TRUE);
}


void CPropTreeItemEdit::OnActivate()
{
	// Check if the edit control needs creation
    CRect r(m_rc);
    const int FloatEditSize = 100;
    r.left = 0;
    r.right = (m_nFormat == ValueFormatFloatPointer && m_bUseRange) ? 100 : m_rc.Width();
    r.top = 0;
    r.bottom = m_rc.Height();
	if (!IsWindow(m_hWnd))
	{
        LPCTSTR pszClassName;
        DWORD dwStyle = WS_CHILD|WS_VISIBLE;

        pszClassName = AfxRegisterWndClass(CS_VREDRAW|CS_HREDRAW, LoadCursor(NULL, IDC_ARROW), 0);

        CreateEx(0, pszClassName, _T(""), dwStyle, m_rc, m_pProp->GetCtrlParent(), 0);
        SendMessage(WM_SETFONT, (WPARAM)m_pProp->GetNormalFont()->m_hObject);

        dwStyle = WS_CHILD|ES_AUTOHSCROLL;
        m_Edit.Create(dwStyle, r, this, 100);
        m_Edit.SendMessage(WM_SETFONT, (WPARAM)m_pProp->GetNormalFont()->m_hObject);

        if (m_nFormat == ValueFormatFloatPointer && m_bUseRange)
        {
            CRect rect;
            GetClientRect(rect);
            rect.left = rect.left + 100;
            m_Slider.Create(WS_CHILD|WS_VISIBLE|TBS_HORZ|TBS_AUTOTICKS,rect, this, 101);
        }
	}
    CRect rect;
    GetClientRect(rect);
    SetWindowPos(NULL, m_rc.left, m_rc.top, m_rc.Width(), m_rc.Height(), SWP_NOZORDER|SWP_SHOWWINDOW);

    if (m_nFormat == ValueFormatFloatPointer && m_bUseRange)
    {
        m_Slider.SetRange((int)(m_fMinValue * m_fMultiplier),(int)(m_fMaxValue * m_fMultiplier));
        m_Slider.SetTic(m_fMultiplier);
        m_Slider.SetPos((int)(m_fValue * m_fMultiplier));
        m_Slider.ShowWindow(SW_SHOW);
        rect.left = rect.left + 100;
        m_Slider.SetWindowPos(NULL,rect.left, 0,rect.Width(),rect.Height(),SWP_NOZORDER|SWP_NOACTIVATE);
    }

    m_Edit.SetPasswordChar((TCHAR)(m_bPassword ? '*' : 0));
	m_Edit.SetWindowText(m_sEdit);
	m_Edit.SetSel(0, -1);
    m_Edit.SetWindowPos(NULL,0, 0, r.right, r.bottom,SWP_NOZORDER|SWP_NOACTIVATE);

    SetFocus();
	if (m_pEventSink)
		m_pEventSink->OnSetFocus();
}

void CPropTreeItemEdit::OnKillFocus( CWnd* pWnd)
{
    if (pWnd != &m_Edit && 
        pWnd != &m_Slider)
    {
        OnRefresh();
        DontCommitChanges();
    }
}

void CPropTreeItemEdit::OnMouseMove(UINT, CPoint point) 
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

void CPropTreeItemEdit::OnLButtonDown(UINT nFlags, CPoint point) 
{
    CRect rect;
    m_Edit.GetWindowRect(rect);
    ScreenToClient(rect);
    if (rect.PtInRect(point) && IsActivated())
    {
        if (m_nFormat == ValueFormatFloatPointer && m_bUseRange)
        {
            m_Slider.ModifyStyle(0,WS_DISABLED);
            m_Slider.ShowWindow(SW_SHOW);
        }

        m_Edit.ShowWindow(SW_SHOW);
        m_Edit.SetFocus();
        m_bEdit = TRUE;
    }
}

void CPropTreeItemEdit::OnEditKillfocus()
{
    float val;
    CString tmp;
    m_Edit.GetWindowText(tmp);

    if (m_nFormat == ValueFormatFloatPointer && m_bUseRange)
    {
        if (sscanf((LPCSTR)tmp,"%f",&val))
        {
			// If user has manually adjusted var outside of range, let them do it, and change our range markers
            if (val < m_fMinValue )
				SetRangeParams(val,m_fMaxValue,1);
            if (val > m_fMaxValue)
               SetRangeParams(m_fMinValue,val,1);
            m_sEdit.Format(_T("%f"),val);
        }
        else
            m_Edit.SetWindowText(m_sEdit);
    }
    else
        m_sEdit = tmp;

    SetWindowText(m_sEdit);
    if (m_nFormat == ValueFormatFloatPointer && m_bUseRange)
    {
        m_Slider.ModifyStyle(WS_DISABLED,0);
        m_Slider.ShowWindow(SW_SHOW);
    }
    
    CommitChanges();
    if (m_pEventSink)
		m_pEventSink->OnKillFocus();
}

void CPropTreeItemEdit::OnSliderKillfocus()
{
    float val = m_Slider.GetPos();
    val /= m_fMultiplier;
    SetItemValue((LPARAM)&val);
    m_Edit.SetWindowText(m_sEdit);
    SetWindowText(m_sEdit);
    CommitChanges();
	if (m_pEventSink)
		m_pEventSink->OnKillFocus();
}

void CPropTreeItemEdit::OnHScroll(UINT nSBCode,UINT nPos,CScrollBar* pScrollBar)
{
    if (m_Slider.GetSafeHwnd() == pScrollBar->GetSafeHwnd()) 
    {
        float val = m_Slider.GetPos();
        val /= m_fMultiplier;
        SetItemValue((LPARAM)&val);
        m_Edit.SetWindowText(m_sEdit);
        SetWindowText(m_sEdit);
        CRect rect;
        m_Edit.GetWindowRect(rect);
        CWnd::GetParent()->ScreenToClient(rect);
        CWnd::GetParent()->InvalidateRect(rect,FALSE);
	    m_pProp->SendNotify(PTN_ITEMCHANGED, this);
    }
    CWnd::OnHScroll(nSBCode, nPos, pScrollBar);
}
