/*********************************************************************NVMH4****
Path:  NVSDK\Common\src\nv_gui
File:  PropTreeItemStatic.cpp

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

// PropTreeItemStatic.cpp
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

#include "nv_gui\PropTreeItemStatic.h"


CPropTreeItemStatic::CPropTreeItemStatic(nv_gui::INVGUIEvent* pEventSink) :
	CPropTreeItem(pEventSink),
		m_sAttribute(_T(""))
		
{
}


CPropTreeItemStatic::~CPropTreeItemStatic()
{
}


void CPropTreeItemStatic::DrawAttribute(CDC* pDC, const RECT& rc)
{
	ASSERT(m_pProp!=NULL);

	pDC->SelectObject(m_pProp->GetNormalFont());
	pDC->SetTextColor(IsReadOnly() ? m_pProp->GetReadOnlyColor() : m_pProp->GetColor());
	pDC->SetBkMode(TRANSPARENT);

	CRect r = rc;
	pDC->DrawText(m_sAttribute, r, DT_SINGLELINE|DT_VCENTER);
}


LPARAM CPropTreeItemStatic::GetItemValue()
{
	return (LPARAM)(LPCTSTR)m_sAttribute;
}


void CPropTreeItemStatic::SetItemValue(LPARAM lParam)
{
	if (lParam==0L)
	{
		TRACE0("CPropTreeItemStatic::SetItemValue() - Invalid lParam value\n");
		return;
	}

	m_sAttribute = (LPCTSTR)lParam;
}
