/*********************************************************************NVMH4****
Path:  NVSDK\Common\src\nv_gui
File:  PropTreeItemColor.cpp

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

// PropTreeItemColor.cpp : implementation file
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
#include "nv_gui\PropTree.h"
#include "Resource.h"
#include "nv_gui\PropTreeItemColor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

typedef struct _ColorTableEntry
{
	COLORREF	color;
	RECT		rcSpot;
} ColorTableEntry;

static ColorTableEntry _crColors[] = 
{
    {RGB(0x00, 0x00, 0x00)},
    {RGB(0xA5, 0x2A, 0x00)},
    {RGB(0x00, 0x40, 0x40)},
    {RGB(0x00, 0x55, 0x00)},
    {RGB(0x00, 0x00, 0x5E)},
    {RGB(0x00, 0x00, 0x8B)},
    {RGB(0x4B, 0x00, 0x82)},
    {RGB(0x28, 0x28, 0x28)},

    {RGB(0x8B, 0x00, 0x00)},
    {RGB(0xFF, 0x68, 0x20)},
    {RGB(0x8B, 0x8B, 0x00)},
    {RGB(0x00, 0x93, 0x00)},
    {RGB(0x38, 0x8E, 0x8E)},
    {RGB(0x00, 0x00, 0xFF)},
    {RGB(0x7B, 0x7B, 0xC0)},
    {RGB(0x66, 0x66, 0x66)},

    {RGB(0xFF, 0x00, 0x00)},
    {RGB(0xFF, 0xAD, 0x5B)},
    {RGB(0x32, 0xCD, 0x32)}, 
    {RGB(0x3C, 0xB3, 0x71)},
    {RGB(0x7F, 0xFF, 0xD4)},
    {RGB(0x7D, 0x9E, 0xC0)},
    {RGB(0x80, 0x00, 0x80)},
    {RGB(0x7F, 0x7F, 0x7F)},

    {RGB(0xFF, 0xC0, 0xCB)},
    {RGB(0xFF, 0xD7, 0x00)},
    {RGB(0xFF, 0xFF, 0x00)},    
    {RGB(0x00, 0xFF, 0x00)},
    {RGB(0x40, 0xE0, 0xD0)},
    {RGB(0xC0, 0xFF, 0xFF)},
    {RGB(0x48, 0x00, 0x48)},
    {RGB(0xC0, 0xC0, 0xC0)},

    {RGB(0xFF, 0xE4, 0xE1)},
    {RGB(0xD2, 0xB4, 0x8C)},
    {RGB(0xFF, 0xFF, 0xE0)},
    {RGB(0x98, 0xFB, 0x98)},
    {RGB(0xAF, 0xEE, 0xEE)},
    {RGB(0x68, 0x83, 0x8B)},
    {RGB(0xE6, 0xE6, 0xFA)},
    {RGB(0xFF, 0xFF, 0xFF)}
};

static void ColorBox(CDC* pDC, CPoint pt, COLORREF clr, BOOL bHover)
{
	CBrush br(clr);

	CBrush* obr = pDC->SelectObject(&br);

	pDC->PatBlt(pt.x, pt.y, 13, 13, PATCOPY);
	pDC->SelectObject(obr);

	CRect rc;
	rc.SetRect(pt.x - 2, pt.y - 2, pt.x + 15, pt.y + 15);

	pDC->DrawEdge(&rc, (bHover) ? BDR_SUNKENOUTER : BDR_RAISEDINNER, BF_RECT);
}



static LONG FindSpot(CPoint point)
{
	for (LONG i=0; i<40; i++)
	{
		if (PtInRect(&_crColors[i].rcSpot, point))
			return i;
	}

	return -1;
}


/////////////////////////////////////////////////////////////////////////////
// CPropTreeItemColor

COLORREF* CPropTreeItemColor::s_pColors = NULL;

CPropTreeItemColor::CPropTreeItemColor(nv_gui::INVGUIEvent* pEventSink) :
	CPropTreeItem(pEventSink),
	m_cColor(0),
	m_cPrevColor(0),
	m_nSpot(-1),
	m_bButton(FALSE),
	m_bInDialog(FALSE)
{
}

CPropTreeItemColor::~CPropTreeItemColor()
{
}


BEGIN_MESSAGE_MAP(CPropTreeItemColor, CWnd)
	//{{AFX_MSG_MAP(CPropTreeItemColor)
	ON_WM_KILLFOCUS()
	ON_WM_CLOSE()
	ON_WM_MOUSEMOVE()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPropTreeItemColor message handlers

void CPropTreeItemColor::SetDefaultColorsList(COLORREF* pColors)
{
	s_pColors = pColors;
}


void CPropTreeItemColor::DrawAttribute(CDC* pDC, const RECT& rc)
{
	ASSERT(m_pProp!=NULL);

	CRect r(rc);
	
	pDC->SelectObject(m_pProp->GetNormalFont());
    pDC->SetTextColor(IsReadOnly() ? m_pProp->GetReadOnlyColor() : m_pProp->GetColor());

	r.top += 1;
	r.right = r.left + r.Height() - 1;

	CBrush br(m_cColor & 0xFFFFFF);
	CBrush* pold = pDC->SelectObject(&br);
	pDC->PatBlt(r.left, r.top, r.Width(), r.Height(), PATCOPY);
	pDC->SelectObject(pold);

	if (m_bButton)
		pDC->DrawEdge(&r, BDR_SUNKENOUTER, BF_RECT);
	else
		pDC->DrawEdge(&r, BDR_RAISEDINNER, BF_RECT);

	CString s;

	r = rc;
	r.left += r.Height();
	s = _T("RGB (");
	pDC->DrawText(s, r, DT_SINGLELINE|DT_VCENTER);
    CRect r_test;
    pDC->DrawText(s, r_test, DT_CALCRECT);
    pDC->SelectObject(IsReadOnly() ? m_pProp->GetNormalFont() : m_pProp->GetBoldFont());
    s.Format(_T("%d, %d, %d"), GetRValue(m_cColor),GetGValue(m_cColor), GetBValue(m_cColor));
    r.left += r_test.Width();
    pDC->DrawText(s, r, DT_SINGLELINE|DT_VCENTER);
    pDC->DrawText(s, r_test, DT_CALCRECT);
    r.left += r_test.Width();
    s = _T(")");
    pDC->SelectObject(m_pProp->GetNormalFont());
    pDC->DrawText(s, r, DT_SINGLELINE|DT_VCENTER);

}


LPARAM CPropTreeItemColor::GetItemValue()
{
	return m_cColor;
}


void CPropTreeItemColor::SetItemValue(LPARAM lParam)
{
	m_cColor = static_cast<COLORREF>(lParam);
}


void CPropTreeItemColor::OnMove()
{
}


void CPropTreeItemColor::OnRefresh()
{
}


void CPropTreeItemColor::OnCommit()
{
	if (IsWindow(m_hWnd))
		ShowWindow(SW_HIDE);
}


void CPropTreeItemColor::OnActivate()
{
	CRect r;

	m_cPrevColor = m_cColor;
    m_bButton = FALSE;

	r = m_rc;

	ASSERT(m_pProp!=NULL);
	m_pProp->GetCtrlParent()->ClientToScreen(r);

	if (!IsWindow(m_hWnd))
	{
		LPCTSTR pszClassName;

		pszClassName = AfxRegisterWndClass(CS_VREDRAW|CS_HREDRAW, LoadCursor(NULL, IDC_ARROW), (HBRUSH)(COLOR_BTNFACE + 1));
		
		DWORD dwStyle = WS_POPUP|WS_DLGFRAME;

		CreateEx(WS_EX_TRANSPARENT, pszClassName, _T(""), dwStyle, r, m_pProp->GetCtrlParent(), 0);
	}

	SetWindowPos(NULL, r.left, r.top, r.Width() + 1, r.Height(), SWP_NOZORDER|SWP_SHOWWINDOW);
	SetFocus();
	CPoint point;

	GetCursorPos(&point);
	ScreenToClient(&point);
	OnLButtonDown(0, point);
}


void CPropTreeItemColor::OnKillFocus(CWnd* pNewWnd) 
{
	CWnd::OnKillFocus(pNewWnd);
	
	if (!m_bInDialog)
		CommitChanges();
    m_bButton = FALSE;
}

void CPropTreeItemColor::OnClose() 
{
	CommitChanges();
    m_bButton = FALSE;
}


void CPropTreeItemColor::OnMouseMove(UINT, CPoint point) 
{
	BOOL bButton;
	
	CRect r(0,0,m_rc.Width(), m_rc.Height());

  	r.top += 1;
	r.right = r.left + m_rc.Height() - 1;

    bButton = r.PtInRect(point);

	if (bButton!=m_bButton)
	{
		m_bButton = bButton;
	}
}


BOOL CPropTreeItemColor::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	if (nHitTest==HTCLIENT)
	{
		CPoint point;

		GetCursorPos(&point);
		ScreenToClient(&point);

		if (FindSpot(point)!=-1 || m_rcButton.PtInRect(point))
		{
			SetCursor(LoadCursor(AfxGetResourceHandle(), MAKEINTRESOURCE(IDC_FPOINT)));
			return TRUE;
		}

	}

	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}


void CPropTreeItemColor::OnLButtonDown(UINT, CPoint point) 
{
	BOOL bButton;
	
	CRect r(0,0,m_rc.Width(), m_rc.Height());

  	r.top += 1;
	r.right = r.left + m_rc.Height() - 1;

	bButton = r.PtInRect(point);

	if (bButton!=m_bButton)
	{
		m_bButton = bButton;
	}
	if (m_bButton)
	{
		CHOOSECOLOR cc;
		COLORREF clr[16];

		ZeroMemory(&cc, sizeof(CHOOSECOLOR));
		cc.Flags = CC_FULLOPEN|CC_ANYCOLOR|CC_RGBINIT;
		cc.lStructSize = sizeof(CHOOSECOLOR);
		cc.hwndOwner = m_hWnd;
		cc.rgbResult = m_cColor;
		cc.lpCustColors = s_pColors ? s_pColors : clr;

		memset(clr, 0xff, sizeof(COLORREF) * 16);
		clr[0] = m_cColor;

		m_bInDialog = TRUE;

		ASSERT(m_pProp!=NULL);
		m_pProp->DisableInput();

		ShowWindow(SW_HIDE);

		if (ChooseColor(&cc))
			m_cColor = cc.rgbResult;

		m_pProp->DisableInput(FALSE);
		CommitChanges();
		m_bButton = FALSE;
	}
}
