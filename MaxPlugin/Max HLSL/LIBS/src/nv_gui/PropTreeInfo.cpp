/*********************************************************************NVMH4****
Path:  NVSDK\Common\src\nv_gui
File:  PropTreeInfo.cpp

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

// PropTreeInfo.cpp : implementation file
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
#include "nv_gui\PropTreeInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPropTreeInfo

CPropTreeInfo::CPropTreeInfo() :
	m_pProp(NULL)
{
}

CPropTreeInfo::~CPropTreeInfo()
{
}


BEGIN_MESSAGE_MAP(CPropTreeInfo, CStatic)
	//{{AFX_MSG_MAP(CPropTreeInfo)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropTreeInfo message handlers

void CPropTreeInfo::SetPropOwner(CPropTree* pProp)
{
	m_pProp = pProp;
}

void CPropTreeInfo::OnPaint() 
{
	CPaintDC dc(this);
	CRect rc;

	GetClientRect(rc);

	dc.SelectObject(GetSysColorBrush(COLOR_BTNFACE));
	dc.PatBlt(rc.left, rc.top, rc.Width(), rc.Height(), PATCOPY);

	dc.DrawEdge(&rc, BDR_SUNKENOUTER, BF_RECT);
	rc.DeflateRect(4, 4);

	ASSERT(m_pProp!=NULL);

	CPropTreeItem* pItem = m_pProp->GetFocusedItem();

	if (!m_pProp->IsWindowEnabled())
		dc.SetTextColor(GetSysColor(COLOR_GRAYTEXT));
	else
		dc.SetTextColor(GetSysColor(COLOR_BTNTEXT));

	dc.SetBkMode(TRANSPARENT);
	dc.SelectObject(m_pProp->GetBoldFont());

	CString txt;

	if (!pItem)
		txt.LoadString(IDS_NOITEMSEL);
	else
		txt = pItem->GetLabelText();

	CRect ir;
	ir = rc;

	// draw label
	dc.DrawText(txt, &ir, DT_SINGLELINE|DT_CALCRECT);
	dc.DrawText(txt, &ir, DT_SINGLELINE);

	ir.top = ir.bottom;
	ir.bottom = rc.bottom;
	ir.right = rc.right;

	if (pItem)
		txt = pItem->GetInfoText();
	else
		txt.LoadString(IDS_SELFORINFO);

	dc.SelectObject(m_pProp->GetNormalFont());
	dc.DrawText(txt, &ir, DT_WORDBREAK);
}
