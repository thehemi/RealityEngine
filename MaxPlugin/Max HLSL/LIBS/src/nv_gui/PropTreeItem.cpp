/*********************************************************************NVMH4****
Path:  NVSDK\Common\src\nv_gui
File:  PropTreeItem.cpp

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

// PropTreeItem.cpp
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

#include "nv_gui\PropTreeItem.h"

#define PROPTREEITEM_DEFHEIGHT			21			// default heigt of an item
#define PROPTREEITEM_SPACE				5			// default horz spacing
#define PROPTREEITEM_EXPANDBOX			9			// size of the expand box
#define PROPTREEITEM_CHECKBOX			14			// size of the check box
#define PROPTREEITEM_EXPANDCOLUMN		16			// width of the expand column
#define PNINDENT						16			// child level indent

#define PROPTREEITEM_EXPANDBOXHALF		(PROPTREEITEM_EXPANDBOX/2)


/////////////////////////////////////////////////////////////////////////////
// drawing helper functions
//

// draw a dotted horizontal line
static void _DotHLine(HDC hdc, LONG x, LONG y, LONG w)
{
	for (; w>0; w-=2, x+=2)
		SetPixel(hdc, x, y, GetSysColor(COLOR_BTNSHADOW));
}


// draw the plus/minus button
static void _DrawExpand(HDC hdc, LONG x, LONG y, BOOL bExpand, BOOL bFill)
{
	HPEN hPen;
	HPEN oPen;
	HBRUSH oBrush;

	hPen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW));
	oPen = (HPEN)SelectObject(hdc, hPen);
	oBrush = (HBRUSH)SelectObject(hdc, GetStockObject(bFill ? WHITE_BRUSH : NULL_BRUSH));

	Rectangle(hdc, x, y, x + PROPTREEITEM_EXPANDBOX, y + PROPTREEITEM_EXPANDBOX);
	SelectObject(hdc, GetStockObject(BLACK_PEN));

	if (!bExpand)
	{
		MoveToEx(hdc, x + PROPTREEITEM_EXPANDBOXHALF, y + 2, NULL);
		LineTo(hdc, x + PROPTREEITEM_EXPANDBOXHALF, y + PROPTREEITEM_EXPANDBOX - 2);
	}

	MoveToEx(hdc, x + 2, y + PROPTREEITEM_EXPANDBOXHALF, NULL);
	LineTo(hdc, x + PROPTREEITEM_EXPANDBOX - 2, y + PROPTREEITEM_EXPANDBOXHALF);

	SelectObject(hdc, oPen);
	SelectObject(hdc, oBrush);
	DeleteObject(hPen);
}

/////////////////////////////////////////////////////////////////////////////
// CPropTreeItem
//

CPropTreeItem::CPropTreeItem(nv_gui::INVGUIEvent* pEventSink) :
	m_pProp(NULL),
	m_pEventSink(pEventSink),
	m_sLabel(_T("")),
	m_sInfo(_T("")),
	m_loc(0,0),
	m_rc(0,0,0,0),
	m_lParam(0),
	m_nCtrlID(0),
	m_dwState(0),
	m_bCommitOnce(FALSE),
	m_rcExpand(0,0,0,0),
	m_rcCheckbox(0,0,0,0),
	m_pParent(NULL),
	m_pSibling(NULL),
	m_pChild(NULL),
	m_pVis(NULL),
    m_ImageIdx(-1),
    m_dwItemState(0)
{
}


CPropTreeItem::~CPropTreeItem()
{
}


BOOL CPropTreeItem::IsExpanded()
{
	return (m_dwState & TreeItemExpanded) ? TRUE : FALSE;
}


BOOL CPropTreeItem::IsSelected()
{
	return (m_dwState & TreeItemSelected) ? TRUE : FALSE;
}


BOOL CPropTreeItem::IsChecked()
{
	return (m_dwState & TreeItemChecked) ? TRUE : FALSE;
}


BOOL CPropTreeItem::IsReadOnly()
{
	return (m_dwItemState & kReadOnly) ? TRUE : FALSE;
}


BOOL CPropTreeItem::IsActivated()
{
	return (m_dwState & TreeItemActivated) ? TRUE : FALSE;
}


void CPropTreeItem::Select(BOOL bSelect)
{
	if (bSelect)
		m_dwState |= TreeItemSelected;
	else
		m_dwState &= ~TreeItemSelected;
}


void CPropTreeItem::Expand(BOOL bExpand)
{
	if (bExpand)
		m_dwState |= TreeItemExpanded;
	else
		m_dwState &= ~TreeItemExpanded;
}


void CPropTreeItem::Check(BOOL bCheck)
{
	if (bCheck)
		m_dwState |= TreeItemChecked;
	else
		m_dwState &= ~TreeItemChecked;
}


void CPropTreeItem::ReadOnly(BOOL bReadOnly)
{
	if (bReadOnly)
		m_dwItemState |= kReadOnly;
	else
		m_dwItemState &= ~kReadOnly;
}


BOOL CPropTreeItem::IsCheckBox()
{
	return (m_dwState & TreeItemCheckbox) ? TRUE : FALSE;
}


void CPropTreeItem::HasCheckBox(BOOL bCheckbox)
{
	if (bCheckbox)
		m_dwState |= TreeItemCheckbox;
	else
		m_dwState &= ~TreeItemCheckbox;
}

void CPropTreeItem::HasImage(BOOL bImage, UINT idx)
{
    if (bImage)
		m_dwState |= TreeItemImage;
	else
		m_dwState &= ~TreeItemImage;
    m_ImageIdx = idx;
}

void CPropTreeItem::SetState(DWORD state)
{
    m_dwItemState = state;
}

DWORD CPropTreeItem::GetState()
{
    return m_dwItemState;
}

bool CPropTreeItem::IsState(ItemState state)
{
    return (m_dwItemState & state) ? true : false;
}

UINT CPropTreeItem::GetImageIdx()
{
    return m_ImageIdx;
}

BOOL CPropTreeItem::IsImage()
{
    return (m_dwState & TreeItemImage) ? TRUE : FALSE;
}

BOOL CPropTreeItem::HitExpand(const POINT& pt)
{
	return m_rcExpand.PtInRect(pt);
}


BOOL CPropTreeItem::HitCheckBox(const POINT& pt)
{
	return m_rcCheckbox.PtInRect(pt);
}


BOOL CPropTreeItem::IsRootLevel()
{
	ASSERT(m_pProp!=NULL);
	return GetParent() == m_pProp->GetRootItem();
}


LONG CPropTreeItem::GetTotalHeight()
{
	CPropTreeItem* pItem;
	LONG nHeight;

	nHeight = GetHeight();

	if (IsExpanded())
	{
		for (pItem = GetChild(); pItem; pItem = pItem->GetSibling())
        {
            if ((m_pProp && m_pProp->ShowReadOnlyAttributes()) || pItem->IsReadOnly() == FALSE || pItem->GetChild())
                nHeight += pItem->GetTotalHeight();
        }
			
	}

	return nHeight;
}


void CPropTreeItem::SetLabelText(LPCTSTR sLabel)
{
	m_sLabel = sLabel;
}


LPCTSTR CPropTreeItem::GetLabelText()
{
	return m_sLabel;
}


void CPropTreeItem::SetInfoText(LPCTSTR sInfo)
{
	m_sInfo = sInfo;
}


LPCTSTR CPropTreeItem::GetInfoText()
{
	return m_sInfo;
}


void CPropTreeItem::SetCtrlID(UINT nCtrlID)
{
	m_nCtrlID = nCtrlID;
}


UINT CPropTreeItem::GetCtrlID()
{
	return m_nCtrlID;
}


LONG CPropTreeItem::GetHeight()
{
	return PROPTREEITEM_DEFHEIGHT;
}


LPARAM CPropTreeItem::GetItemValue()
{
	// no items are assocatied with this type
	return 0L;
}


void CPropTreeItem::SetItemValue(LPARAM)
{
	// no items are assocatied with this type
}


void CPropTreeItem::OnMove()
{
	// no attributes, do nothing
}


void CPropTreeItem::OnRefresh()
{
	// no attributes, do nothing
}


void CPropTreeItem::OnCommit()
{
	// no attributes, do nothing
}

void CPropTreeItem::OnDontCommit()
{
	// no attributes, do nothing
}

void CPropTreeItem::Activate()
{
    m_dwState |= TreeItemActivated;

	m_bCommitOnce = FALSE;

	OnActivate();
}

void CPropTreeItem::DontCommitChanges()
{
    m_dwState &= ~TreeItemActivated;

	ASSERT(m_pProp!=NULL);

    OnDontCommit();
}

void CPropTreeItem::CommitChanges()
{
    m_dwState &= ~TreeItemActivated;

	if (m_bCommitOnce)
		return;

	m_bCommitOnce = TRUE;

	ASSERT(m_pProp!=NULL);

	OnCommit();

	m_pProp->SendNotify(PTN_ITEMCHANGED, this);
	m_pProp->RefreshItems(this);
}


void CPropTreeItem::OnActivate()
{
	// no attributes, do nothing
}


void CPropTreeItem::SetPropOwner(CPropTree* pProp)
{
	m_pProp = pProp;
}


const POINT& CPropTreeItem::GetLocation()
{
	return m_loc;
}


CPropTreeItem* CPropTreeItem::GetParent()
{
	return m_pParent;
}


CPropTreeItem* CPropTreeItem::GetSibling()
{
	return m_pSibling;
}


CPropTreeItem* CPropTreeItem::GetChild()
{
	return m_pChild;
}


CPropTreeItem* CPropTreeItem::GetNextVisible()
{
	return m_pVis;
}


void CPropTreeItem::SetParent(CPropTreeItem* pParent)
{
	m_pParent = pParent;
}


void CPropTreeItem::SetSibling(CPropTreeItem* pSibling)
{
	m_pSibling = pSibling;
}


void CPropTreeItem::SetChild(CPropTreeItem* pChild)
{
	m_pChild = pChild;
}


void CPropTreeItem::SetNextVisible(CPropTreeItem* pVis)
{
	m_pVis = pVis;
}


LONG CPropTreeItem::DrawItem(CDC* pDC, const RECT& rc, LONG x, LONG y)
{
	CPoint pt;
	LONG nTotal, nCol, ey;
	CRect drc, ir;

	ASSERT(m_pProp!=NULL);

	// Add TreeItem the list of visble items
	m_pProp->AddToVisibleList(this);

	// store the item's location
	m_loc = CPoint(x, y);

	// store the items rectangle position
	m_rc.SetRect(m_pProp->GetOrigin().x + PROPTREEITEM_SPACE, m_loc.y, rc.right, m_loc.y + GetHeight()-1);
	m_rc.OffsetRect(0, -m_pProp->GetOrigin().y);

	// init temp drawing variables
	nTotal = GetHeight();
	ey = (nTotal >> 1) - (PROPTREEITEM_EXPANDBOX >> 1) - 2;

	bool bCheck = false;
    bool bImage = false;

	// convert item coordinates to screen coordinates
	pt = m_loc;
	pt.y -= m_pProp->GetOrigin().y;
	nCol = m_pProp->GetOrigin().x;

	if (IsRootLevel())
		drc.SetRect(pt.x + PROPTREEITEM_EXPANDCOLUMN, pt.y, rc.right, pt.y + nTotal);
	else
		drc.SetRect(pt.x + PROPTREEITEM_EXPANDCOLUMN, pt.y, nCol, pt.y + nTotal);

	// root level items are shaded
	if (IsRootLevel() || (IsReadOnly() && !IsRootLevel()))
	{
		HGDIOBJ hOld = pDC->SelectObject(GetSysColorBrush(COLOR_BTNFACE));
		pDC->PatBlt(rc.left, drc.top, rc.right - rc.left + 1, drc.Height(), PATCOPY);
		pDC->SelectObject(hOld);
	}

	// calc/draw expand box position
	if (GetChild())
	{
		m_rcExpand.left = PROPTREEITEM_EXPANDCOLUMN/2 - PROPTREEITEM_EXPANDBOXHALF;
		m_rcExpand.top = m_loc.y + ey;
		m_rcExpand.right = m_rcExpand.left + PROPTREEITEM_EXPANDBOX - 1;
		m_rcExpand.bottom = m_rcExpand.top + PROPTREEITEM_EXPANDBOX - 1;

		ir = m_rcExpand;
		ir.OffsetRect(0, -m_pProp->GetOrigin().y);
		_DrawExpand(pDC->m_hDC, ir.left, ir.top, IsExpanded(), !IsRootLevel());
	}
	else
		m_rcExpand.SetRectEmpty();

	// calc/draw check box position
	if (IsCheckBox())
	{
		bCheck = true;

		ir.left = drc.left + PROPTREEITEM_SPACE;
		ir.top = m_loc.y + ey;

		ir.right = ir.left + PROPTREEITEM_CHECKBOX;
		ir.bottom = ir.top + PROPTREEITEM_CHECKBOX;

		m_rcCheckbox = ir;
	}
	else
		m_rcCheckbox.SetRectEmpty();

	// calc/draw image position
	if (IsImage())
	{
		bImage = true;

		ir.left = drc.left + PROPTREEITEM_SPACE;
		ir.top = m_loc.y + ey;

		ir.right = ir.left + PROPTREEITEM_CHECKBOX;
		ir.bottom = ir.top + PROPTREEITEM_CHECKBOX;

		m_rcImage = ir;
	}
	else
		m_rcImage.SetRectEmpty();

    HRGN hRgn = NULL;

	// create a clipping region for the label
	if (!IsRootLevel())
	{
		hRgn = CreateRectRgn(drc.left, drc.top, drc.right, drc.bottom);
		SelectClipRgn(pDC->m_hDC, hRgn);
	}

	// calc label position
	ir = drc;
	ir.left += PROPTREEITEM_SPACE;

	// offset the label text if item has a check box
	if (bCheck)
		OffsetRect(&ir, PROPTREEITEM_CHECKBOX + PROPTREEITEM_SPACE * 2, 0);

	// offset the label text if item has an image
	if (bImage)
		OffsetRect(&ir, PROPTREEITEM_CHECKBOX + PROPTREEITEM_SPACE * 2, 0);

    // draw label
	if (!m_sLabel.IsEmpty())
	{
		if (IsRootLevel())
			pDC->SelectObject(CPropTree::GetBoldFont());
		else
			pDC->SelectObject(CPropTree::GetNormalFont());

        if (IsState(kReadOnly))
            pDC->SetTextColor(GetSysColor(COLOR_GRAYTEXT));
        else
		    pDC->SetTextColor(GetSysColor(COLOR_BTNTEXT));
		pDC->SetBkMode(TRANSPARENT);
		pDC->DrawText(m_sLabel, &ir, DT_SINGLELINE|DT_VCENTER|DT_CALCRECT);

		// draw the text highlighted if selected
		if (IsSelected())
		{
			HGDIOBJ oPen = pDC->SelectObject(GetStockObject(NULL_PEN));
			HGDIOBJ oBrush = pDC->SelectObject(GetSysColorBrush(COLOR_HIGHLIGHT));
			
			CRect dr;
			dr = drc;
			dr.left = PROPTREEITEM_EXPANDCOLUMN;
			
			pDC->Rectangle(&dr);
			
			pDC->SelectObject(oPen);
			pDC->SelectObject(oBrush);

			pDC->SetTextColor(GetSysColor(COLOR_BTNHIGHLIGHT));
		}

		// check if we need to draw the text as disabled
		if (!m_pProp->IsWindowEnabled())
			pDC->SetTextColor(GetSysColor(COLOR_GRAYTEXT));

		pDC->DrawText(m_sLabel, &ir, DT_SINGLELINE|DT_VCENTER);
	}

	// draw check box frame
	if (IsCheckBox())
	{
		ir = m_rcCheckbox;
		ir.OffsetRect(0, -m_pProp->GetOrigin().y);
		pDC->DrawFrameControl(&ir, DFC_BUTTON, DFCS_BUTTONCHECK | (IsChecked() ? DFCS_CHECKED : 0));
	}

	// draw the image
	if (IsImage())
	{
		ir = m_rcImage;
		ir.OffsetRect(-1, -m_pProp->GetOrigin().y-2);
        CImageList * imglist = NULL;
        if (IsState(kReadOnly))
            imglist = &m_pProp->GetImageListDisabled();
        else
            imglist = &m_pProp->GetImageList();
		if (imglist->m_hImageList)
			imglist->Draw(pDC,m_ImageIdx,ir.TopLeft(),ILD_TRANSPARENT);
	}

    if (IsState(kUnset) && IsState(kReadOnly) == FALSE)
	{
		ir = m_rcImage;
		ir.OffsetRect(-1, -m_pProp->GetOrigin().y-2);
        CImageList & imglist = m_pProp->GetImageList();
		if (imglist.m_hImageList)
			imglist.Draw(pDC,m_pProp->GetStateImageIdx(kUnset),ir.TopLeft(),ILD_TRANSPARENT);
	}

    // remove clip region
	if (hRgn)
	{
		SelectClipRgn(pDC->m_hDC, NULL);
		DeleteObject(hRgn);
	}

	// draw horzontal sep
	_DotHLine(pDC->m_hDC, PROPTREEITEM_EXPANDCOLUMN, pt.y + nTotal - 1, rc.right - PROPTREEITEM_EXPANDCOLUMN + 1);

	// draw separators
	if (!IsRootLevel())
	{
		// column sep
		CPen pn1(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW));
		CPen* pOld;

		pOld = pDC->SelectObject(&pn1);
		pDC->MoveTo(nCol, drc.top);
		pDC->LineTo(nCol, drc.bottom);

		CPen pn2(PS_SOLID, 1, GetSysColor(COLOR_BTNHIGHLIGHT));
		pDC->SelectObject(&pn2);
		pDC->MoveTo(nCol + 1, drc.top);
		pDC->LineTo(nCol + 1, drc.bottom);

		pDC->SelectObject(pOld);
	}

	// draw attribute
	if (!IsRootLevel())
	{
		// create clip region
		hRgn = CreateRectRgn(m_rc.left, m_rc.top, m_rc.right, m_rc.bottom);
		SelectClipRgn(pDC->m_hDC, hRgn);
		
		DrawAttribute(pDC, m_rc);

		SelectClipRgn(pDC->m_hDC, NULL);
		DeleteObject(hRgn);
	}

	// draw children
	if (GetChild() && IsExpanded())
	{
		y += nTotal;

		CPropTreeItem* pNext;

		for (pNext = GetChild(); pNext; pNext = pNext->GetSibling())
		{
            if ((m_pProp && m_pProp->ShowReadOnlyAttributes()) || pNext->IsReadOnly() == FALSE  || pNext->GetChild())
            {
			    LONG nHeight = pNext->DrawItem(pDC, rc, x + (IsRootLevel() ? 0 : PNINDENT), y);
			    nTotal += nHeight;
			    y += nHeight;
            }
		}
	}

	return nTotal;
}


void CPropTreeItem::DrawAttribute(CDC*, const RECT&)
{
	// no attributes are assocatied with this type
}
