/*********************************************************************NVMH4****
Path:  NVSDK\Common\src\nv_gui
File:  PropTree.cpp

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

// PropTree.cpp : implementation file
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

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define PROPTREEITEM_EXPANDCOLUMN		16			// width of the expand column
#define PROPTREEITEM_COLRNG				5			// width of splitter

static const CString strOfficeFontName	= _T("Tahoma");
static const CString strDefaultFontName = _T("MS Sans Serif");

static int CALLBACK FontFamilyProcFonts(const LOGFONT FAR* lplf, const TEXTMETRIC FAR*, ULONG, LPARAM)
{
	ASSERT(lplf != NULL);
	CString strFont = lplf->lfFaceName;
	return strFont.CollateNoCase (strOfficeFontName) == 0 ? 0 : 1;
}

/////////////////////////////////////////////////////////////////////////////
// CPropTree

std::map<CPropTreeItem::ItemState,unsigned int> CPropTree::m_ItemStateIdx;


UINT CPropTree::s_nInstanceCount;
CFont* CPropTree::s_pNormalFont;
CFont* CPropTree::s_pBoldFont;
CPropTreeItem* CPropTree::s_pFound;
COLORREF CPropTree::m_ReadOnlyTextColor = RGB(64,64,64);
COLORREF CPropTree::m_TextColor = RGB(0,0,0);


CPropTree::CPropTree() :
	m_Root(NULL),
	m_bShowInfo(TRUE),
	m_nInfoHeight(50),
	m_pVisbleList(NULL),
	m_Origin(200,0),
	m_nLastUID(1),
	m_pFocus(NULL),
	m_bDisableInput(FALSE)
{
	m_Root.Expand();

	// init global resources only once
	if (!s_nInstanceCount)
		InitGlobalResources();
	s_nInstanceCount++;
}


CPropTree::~CPropTree()
{
	DeleteAllItems();

	s_nInstanceCount--;

	// free global resource when ALL CPropTrees are destroyed
	if (!s_nInstanceCount)
		FreeGlobalResources();
}


BEGIN_MESSAGE_MAP(CPropTree, CWnd)
	//{{AFX_MSG_MAP(CPropTree)
	ON_WM_CLOSE()
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ENABLE()
	ON_WM_SYSCOLORCHANGE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPropTree message handlers

// set the image list
void CPropTree::SetImageList(CImageList & images,CImageList & ImagesDisabled)
{
    m_Images.Attach(images.m_hImageList);
    m_ImagesDisabled.Attach(ImagesDisabled.m_hImageList);
}

CImageList & CPropTree::GetImageList()
{
    return m_Images;
}

CImageList & CPropTree::GetImageListDisabled()
{
    return m_ImagesDisabled;
}

void CPropTree::Filter(DWORD dwAdd, DWORD dwRemove)
{
    m_List.m_ShowReadOnlyAttributes = !m_List.m_ShowReadOnlyAttributes;
    m_List.UpdateResize();
	m_List.Invalidate();
	m_List.UpdateWindow();
}

BOOL CPropTree::IsFilter(DWORD dwTest)
{
    return m_List.m_ShowReadOnlyAttributes;
}

unsigned int CPropTree::SetStateImageIdx(CPropTreeItem::ItemState state, unsigned int idx)
{
    std::map<CPropTreeItem::ItemState,unsigned int>::iterator it = m_ItemStateIdx.find(state);
    if (it!= m_ItemStateIdx.end())
        (*it).second = idx;
    else
        m_ItemStateIdx.insert(std::map<CPropTreeItem::ItemState,unsigned int>::value_type(state,idx));
    return idx;
}

unsigned int CPropTree::GetStateImageIdx(CPropTreeItem::ItemState state)
{
    std::map<CPropTreeItem::ItemState,unsigned int>::iterator it = m_ItemStateIdx.find(state);
    if (it!= m_ItemStateIdx.end())
        return (*it).second;
    return -1;
}

const POINT& CPropTree::GetOrigin()
{
	return m_Origin;
}

void CPropTree::OnClose()
{
	m_Images.Detach();
    m_ImagesDisabled.Detach();
	CWnd::OnDestroy();
}

BOOL CPropTree::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	CWnd* pWnd = this;

	LPCTSTR pszCreateClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW, ::LoadCursor(NULL, IDC_ARROW));

	return pWnd->Create(pszCreateClass, _T(""), dwStyle, rect, pParentWnd, nID);
}


int CPropTree::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	DWORD dwStyle;
	CRect rc;

	GetClientRect(rc);

	// create CPropTreeList
	//

	dwStyle = WS_VISIBLE|WS_CHILD|WS_VSCROLL;

	if (!m_List.Create(dwStyle, rc, this, 100))
	{
		TRACE0("Failed to create CPropTreeList\n");
		return -1;
	}

	m_List.SetPropOwner(this);

	// create CPropTreeInfo
	//

	dwStyle &= ~WS_VSCROLL;

	if (!m_Info.Create(_T(""), dwStyle, rc, this))
	{
		TRACE0("Failed to create CPropTreeInfo\n");
		return -1;
	}

	m_Info.SetPropOwner(this);

	return 0;
}

BOOL CPropTree::ShowReadOnlyAttributes()
{
    return m_List.m_ShowReadOnlyAttributes;
}

CWnd* CPropTree::GetCtrlParent()
{
	return &m_List;
}


void CPropTree::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);
	ResizeChildWindows(cx, cy);
}


void CPropTree::ResizeChildWindows(int cx, int cy)
{
	if (m_bShowInfo)
	{
		if (IsWindow(m_List.m_hWnd))
			m_List.MoveWindow(0, 0, cx, cy - m_nInfoHeight);

		if (IsWindow(m_Info.m_hWnd))
			m_Info.MoveWindow(0, cy - m_nInfoHeight, cx, m_nInfoHeight);
	}
	else
	{
		if (IsWindow(m_List.m_hWnd))
			m_List.MoveWindow(0, 0, cx, cy);
	}
}


void CPropTree::InitGlobalResources()
{
	NONCLIENTMETRICS info;
	info.cbSize = sizeof(info);

	::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(info), &info, 0);

	LOGFONT lf;
	memset(&lf, 0, sizeof (LOGFONT));

	CWindowDC dc(NULL);
	lf.lfCharSet = (BYTE)GetTextCharsetInfo(dc.GetSafeHdc(), NULL, 0);

	lf.lfHeight = info.lfMenuFont.lfHeight;
	lf.lfWeight = info.lfMenuFont.lfWeight;
	lf.lfItalic = info.lfMenuFont.lfItalic;

	// check if we should use system font
	_tcscpy(lf.lfFaceName, info.lfMenuFont.lfFaceName);

	BOOL fUseSystemFont = (info.lfMenuFont.lfCharSet > SYMBOL_CHARSET);
	if (!fUseSystemFont)
	{
		// check for "Tahoma" font existance:
		if (::EnumFontFamilies(dc.GetSafeHdc(), NULL, FontFamilyProcFonts, 0)==0)
		{
			// Found! Use MS Office font!
			_tcscpy(lf.lfFaceName, strOfficeFontName);
		}
		else
		{
			// Not found. Use default font:
			_tcscpy(lf.lfFaceName, strDefaultFontName);
		}
	}

	s_pNormalFont = new CFont;
	s_pNormalFont->CreateFontIndirect(&lf);

	lf.lfWeight = FW_BOLD;
	s_pBoldFont = new CFont;
	s_pBoldFont->CreateFontIndirect(&lf);

    m_ReadOnlyTextColor = GetSysColor(COLOR_GRAYTEXT);
    m_TextColor = GetSysColor(COLOR_BTNTEXT);

}


void CPropTree::FreeGlobalResources()
{
	if (s_pNormalFont)
	{
		delete s_pNormalFont;
		s_pNormalFont = NULL;
	}

	if (s_pBoldFont)
	{
		delete s_pBoldFont;
		s_pBoldFont = NULL;
	}
}


CFont* CPropTree::GetNormalFont()
{
	return s_pNormalFont;
}


CFont* CPropTree::GetBoldFont()
{
	return s_pBoldFont;
}

const COLORREF & CPropTree::GetReadOnlyColor()
{
    return m_ReadOnlyTextColor;
}

const COLORREF & CPropTree::GetColor()
{
    return m_TextColor;
}


CPropTreeItem* CPropTree::GetFocusedItem()
{
	return m_pFocus;
}


CPropTreeItem* CPropTree::GetRootItem()
{
	return &m_Root;
}


void CPropTree::ClearVisibleList()
{
	m_pVisbleList = NULL;
}


CPropTreeItem* CPropTree::GetVisibleList()
{
	return m_pVisbleList;
}


void CPropTree::AddToVisibleList(CPropTreeItem* pItem)
{
	if (!pItem)
		return;

	// check for an empty visible list
	if (!m_pVisbleList)
		m_pVisbleList = pItem;
	else
	{
		// Add the new item to the end of the list
		CPropTreeItem* pNext;

		pNext = m_pVisbleList;
		while (pNext->GetNextVisible())
			pNext = pNext->GetNextVisible();

		pNext->SetNextVisible(pItem);
	}

	pItem->SetNextVisible(NULL);
}


BOOL CPropTree::EnumItems(CPropTreeItem* pItem, ENUMPROPITEMPROC proc, LPARAM lParam)
{
	if (!pItem || !proc)
		return FALSE;

	CPropTreeItem* pNext;

	// don't count the root item in any enumerations
	if (pItem!=&m_Root && !proc(this, pItem, lParam))
		return FALSE;

	// recurse thru all child items
	pNext = pItem->GetChild();

	while (pNext)
	{
		if (!EnumItems(pNext, proc, lParam))
			return FALSE;

		pNext = pNext->GetSibling();
	}

	return TRUE;
}


void CPropTree::SetOriginOffset(LONG nOffset)
{
	m_Origin.y = nOffset;
}

	
void CPropTree::UpdatedItems()
{
	if (!IsWindow(m_hWnd))
		return;

	ClearVisibleList();

	Invalidate();

	m_List.UpdateResize();
	m_List.Invalidate();
}


void CPropTree::DeleteAllItems()
{
	Delete(NULL);
	ClearVisibleList();
	UpdatedItems();
	m_nLastUID = 1; // reset uid counter
}


void CPropTree::DeleteItem(CPropTreeItem* pItem)
{
	Delete(pItem);
	UpdatedItems();
}


LONG CPropTree::GetColumn()
{
	return m_Origin.x;
}


void CPropTree::SetColumn(LONG nColumn)
{
	CRect rc;

	GetClientRect(rc);
	
	if (rc.IsRectEmpty())
		nColumn = __max(PROPTREEITEM_EXPANDCOLUMN, nColumn);
	else
		nColumn = __min(__max(PROPTREEITEM_EXPANDCOLUMN, nColumn), rc.Width() - PROPTREEITEM_EXPANDCOLUMN);

	m_Origin.x = nColumn;

	Invalidate();
}


void CPropTree::Delete(CPropTreeItem* pItem)
{
	if (pItem && pItem!=&m_Root && SendNotify(PTN_DELETEITEM, pItem))
		return;

	// passing in a NULL item is the same as calling DeleteAllItems
	if (!pItem)
		pItem = &m_Root;

	// delete children

	CPropTreeItem* pIter;
	CPropTreeItem* pNext;

	pIter = pItem->GetChild();
	while (pIter)
	{
		pNext = pIter->GetSibling();
		DeleteItem(pIter);
		pIter = pNext;
	}

	// unlink from tree
	if (pItem->GetParent())
	{
		if (pItem->GetParent()->GetChild()==pItem)
			pItem->GetParent()->SetChild(pItem->GetSibling());
		else
		{
			pIter = pItem->GetParent()->GetChild();
			while (pIter->GetSibling() && pIter->GetSibling()!=pItem)
				pIter = pIter->GetSibling();

			if (pIter->GetSibling())
				pIter->SetSibling(pItem->GetSibling());
		}
	}

	if (pItem!=&m_Root)
	{
		if (pItem==GetFocusedItem())
			SetFocusedItem(NULL);
		delete pItem;
	}
}


void CPropTree::SetFocusedItem(CPropTreeItem* pItem)
{
	m_pFocus = pItem;
	EnsureVisible(m_pFocus);

	if (!IsWindow(m_hWnd))
		return;

	Invalidate();
}


void CPropTree::ShowInfoText(BOOL bShow)
{
	m_bShowInfo = bShow;

	CRect rc;

	GetClientRect(rc);
	ResizeChildWindows(rc.Width(), rc.Height());
}


BOOL CPropTree::IsItemVisible(CPropTreeItem* pItem)
{
	if (!pItem)
		return FALSE;

	for (CPropTreeItem* pNext = m_pVisbleList; pNext; pNext = pNext->GetNextVisible())
	{
		if (pNext==pItem)
			return TRUE;
	}

	return FALSE;
}

void CPropTree::ExpandAll(CPropTreeItem* pItem, BOOL bExpand)
{
    RecEnsureVisible(pItem,bExpand);
}

void CPropTree::RecEnsureVisible(CPropTreeItem* pItem, BOOL bVisible)
{
    EnsureVisible(pItem,bVisible);
	// recurse thru all child items
	CPropTreeItem* pNext = pItem->GetChild();

	while (pNext)
	{
        RecEnsureVisible(pNext,bVisible);
		pNext = pNext->GetSibling();
	}
}

void CPropTree::EnsureVisible(CPropTreeItem* pItem,BOOL bVisible)
{
	if (!pItem)
		return;

	CPropTreeItem* pParent = NULL;
    if (bVisible)
    {
	    // item is not scroll visible (expand all parents)
	    if (!IsItemVisible(pItem))
	    {


		    pParent = pItem->GetParent();
		    while (pParent)
		    {
			    pParent->Expand();
			    pParent = pParent->GetParent();
		    }

		    UpdatedItems();
		    UpdateWindow();
	    }
        ASSERT(IsItemVisible(pItem));
    }
    else
    {
	    if (IsItemVisible(pItem))
	    {
            pItem->Expand(FALSE);
            pParent = pItem->GetParent();
		    UpdatedItems();
		    UpdateWindow();
	    }
    }

	CRect rc;

	m_List.GetClientRect(rc);
	rc.OffsetRect(0, m_Origin.y);
	rc.bottom -= pItem->GetHeight();

	CPoint pt;

    if (bVisible)
	    pt = pItem->GetLocation();
    else if (pParent)
        pt = pParent->GetLocation();
    else
        pt = CPoint(rc.left,rc.top);

	if (!rc.PtInRect(pt))
	{
		LONG oy;

		if (pt.y < rc.top)
			oy = pt.y;
		else
			oy = pt.y - rc.Height() + pItem->GetHeight();

		m_List.OnVScroll(SB_THUMBTRACK, oy, NULL);
	}
}


CPropTreeItem* CPropTree::InsertItem(CPropTreeItem* pItem, CPropTreeItem* pParent)
{
	if (!pItem)
		return NULL;

	if (!pParent)
		pParent = &m_Root;

	if (!pParent->GetChild())
		pParent->SetChild(pItem);
	else
	{
		// add to end of the sibling list
		CPropTreeItem* pNext;

		pNext = pParent->GetChild();
		while (pNext->GetSibling())
			pNext = pNext->GetSibling();

		pNext->SetSibling(pItem);
	}

	pItem->SetParent(pParent);
	pItem->SetPropOwner(this);

	// auto generate a default ID
	pItem->SetCtrlID(m_nLastUID++);

	SendNotify(PTN_INSERTITEM, pItem);

	UpdatedItems();

	return pItem;
}



LONG CPropTree::HitTest(const POINT& pt)
{
	POINT p = pt;

	CPropTreeItem* pItem;

	// convert screen to tree coordinates
	p.y += m_Origin.y;

	if ((pItem = FindItem(pt))!=NULL)
	{
		if (!pItem->IsRootLevel() && pt.x >= m_Origin.x - PROPTREEITEM_COLRNG && pt.x <= m_Origin.x + PROPTREEITEM_COLRNG)
			return HTCOLUMN;

		if (pt.x > m_Origin.x + PROPTREEITEM_COLRNG)
			return HTATTRIBUTE;

		if (pItem->HitExpand(p))
			return HTEXPAND;

		if (pItem->HitCheckBox(p))
			return HTCHECKBOX;

		return HTLABEL;
	}

	return HTCLIENT;
}


CPropTreeItem* CPropTree::FindItem(const POINT& pt)
{
	CPropTreeItem* pItem;

	CPoint p = pt;

	// convert screen to tree coordinates
	p.y += m_Origin.y;

	// search the visible list for the item
	for (pItem = m_pVisbleList; pItem; pItem = pItem->GetNextVisible())
	{
		CPoint ipt = pItem->GetLocation();
		if (p.y>=ipt.y && p.y<ipt.y + pItem->GetHeight())
			return pItem;
	}

	return NULL;
}


CPropTreeItem* CPropTree::FindItem(UINT nCtrlID)
{
	s_pFound = NULL;

	try{
		EnumItems(&m_Root, EnumFindItem, nCtrlID);
	}
	catch(...){
		::MessageBox(0,"FindItem crashed","",0);
	}

	return s_pFound;
}


BOOL CALLBACK CPropTree::EnumFindItem(CPropTree*, CPropTreeItem* pItem, LPARAM lParam)
{
	ASSERT(pItem!=NULL);

	if (pItem->GetCtrlID()==(UINT)lParam)
	{
		s_pFound = pItem;
		return FALSE;
	}

	return TRUE;
}


BOOL CPropTree::IsDisableInput()
{
	return m_bDisableInput;
}


void CPropTree::DisableInput(BOOL bDisable)
{
	m_bDisableInput = bDisable;

	CWnd* pWnd;

	if ((pWnd = GetParent())!=NULL)
		pWnd->EnableWindow(!bDisable);
}

CPropTreeItem* CPropTree::GetSelectedItem()
{
    CPropTreeItem* pItem = &m_Root;
	try{
	 EnumItems(pItem, EnumGetSelection, NULL);
	}
	catch(...){
		::MessageBox(0,"GetSelectedItem crashed","",0);
	}
    return s_pFound;
}

void CPropTree::SelectItems(CPropTreeItem* pItem, BOOL bSelect)
{
	if (!pItem)
		pItem = &m_Root;

	try{
		EnumItems(pItem, EnumSelectAll, (LPARAM)bSelect);
	}
	catch(...){
		::MessageBox(0,"SelectItems crashed","",0);
	}
}


CPropTreeItem* CPropTree::FocusFirst()
{
	CPropTreeItem *pold;

	pold = m_pFocus;

	SetFocusedItem(m_pVisbleList);

	if (m_pFocus)
	{
		SelectItems(NULL, FALSE);
		m_pFocus->Select();
	}

	if (pold!=m_pFocus)
		SendNotify(PTN_SELCHANGE, m_pFocus);

	return m_pFocus;
}


CPropTreeItem* CPropTree::FocusLast()
{
	CPropTreeItem* pNext;
	CPropTreeItem* pChange;

	pChange = m_pFocus;

	pNext = m_pVisbleList;

	if (pNext)
	{
		while (pNext->GetNextVisible())
			pNext = pNext->GetNextVisible();

		SetFocusedItem(pNext);

		if (m_pFocus)
		{
			SelectItems(NULL, FALSE);
			m_pFocus->Select();
		}
	}

	if (pChange!=m_pFocus)
		SendNotify(PTN_SELCHANGE, m_pFocus);

	return pNext;
}


CPropTreeItem* CPropTree::FocusPrev()
{
	CPropTreeItem* pNext;
	CPropTreeItem* pChange;

	pChange = m_pFocus;

	if (m_pFocus==NULL)
	{
		// get the last visible item
		pNext = m_pVisbleList;
		while (pNext && pNext->GetNextVisible())
			pNext = pNext->GetNextVisible();
	}
	else
	{
		pNext = m_pVisbleList;
		while (pNext && pNext->GetNextVisible()!=m_pFocus)
			pNext = pNext->GetNextVisible();
	}

	if (pNext)
		SetFocusedItem(pNext);
	
	if (m_pFocus)
	{
		SelectItems(NULL, FALSE);
		m_pFocus->Select();
	}

	if (pChange!=m_pFocus)
		SendNotify(PTN_SELCHANGE, m_pFocus);

	return pNext;
}


CPropTreeItem* CPropTree::FocusNext()
{
	CPropTreeItem* pNext;
	CPropTreeItem* pChange;

	pChange = m_pFocus;

	if (m_pFocus==NULL)
		pNext = m_pVisbleList;
	else
	if (m_pFocus->GetNextVisible())
		pNext = m_pFocus->GetNextVisible();
	else
		pNext = NULL;

	if (pNext)
		SetFocusedItem(pNext);

	if (m_pFocus)
	{
		SelectItems(NULL, FALSE);
		m_pFocus->Select();
	}

	if (pChange!=m_pFocus)
		SendNotify(PTN_SELCHANGE, m_pFocus);

	return pNext;
}


void CPropTree::UpdateMoveAllItems()
{
	try{
		EnumItems(&m_Root, EnumMoveAll);
	}
	catch(...){
		::MessageBox(0,"UpdateMoveAllItems crashed","",0);
	}
}


void CPropTree::RefreshItems(CPropTreeItem* pItem)
{
	if (!pItem)
		pItem = &m_Root;

	try{
		EnumItems(pItem, EnumRefreshAll);
	}
	catch(...){
		::MessageBox(0,"RefreshItems crashed","",0);
	}

	UpdatedItems();
}


BOOL CALLBACK CPropTree::EnumGetSelection(CPropTree*, CPropTreeItem* pItem, LPARAM)
{
	if (!pItem)
		return FALSE;

    if (pItem->IsSelected())
    {
        s_pFound = pItem;
        return FALSE;
    }
        
    return TRUE;
}

BOOL CALLBACK CPropTree::EnumSelectAll(CPropTree*, CPropTreeItem* pItem, LPARAM lParam)
{
	if (!pItem)
		return FALSE;

	pItem->Select((BOOL)lParam);

	return TRUE;
}


BOOL CALLBACK CPropTree::EnumRefreshAll(CPropTree*, CPropTreeItem* pItem, LPARAM)
{
	if (!pItem)
		return FALSE;

	pItem->OnRefresh();

	return TRUE;
}


BOOL CALLBACK CPropTree::EnumMoveAll(CPropTree*, CPropTreeItem* pItem, LPARAM)
{
	if (!pItem)
		return FALSE;

	pItem->OnMove();

	return TRUE;
}


LRESULT CPropTree::SendNotify(UINT nNotifyCode, CPropTreeItem* pItem)
{
	if (!IsWindow(m_hWnd))
		return 0L;

	if (!(GetStyle() & PTS_NOTIFY))
		return 0L;

	NMPROPTREE nmmp;
	LPNMHDR lpnm;

	lpnm = NULL;

	switch (nNotifyCode)
	{
		case PTN_INSERTITEM:
		case PTN_DELETEITEM:
		case PTN_DELETEALLITEMS:
		case PTN_ITEMCHANGED:
		case PTN_ITEMBUTTONCLICK:
		case PTN_SELCHANGE:
		case PTN_ITEMEXPANDING:
		case PTN_COLUMNCLICK:
		case PTN_PROPCLICK:
		case PTN_CHECKCLICK:
        case PTN_FILEEDIT:
			lpnm = (LPNMHDR)&nmmp;
			nmmp.pItem = pItem;
			break;
	}

	if (lpnm)
	{
		UINT id = (UINT)(::GetMenu(m_hWnd));
		lpnm->code = nNotifyCode;
		lpnm->hwndFrom = m_hWnd;
		lpnm->idFrom = id;
	
		return GetParent()->SendMessage(WM_NOTIFY, (WPARAM)id, (LPARAM)lpnm);
	}

	return 0L;
}


void CPropTree::OnEnable(BOOL bEnable) 
{
	CWnd::OnEnable(bEnable);
	Invalidate();
}


void CPropTree::OnSysColorChange() 
{
	CWnd::OnSysColorChange();
	
	Invalidate();	
}


BOOL CPropTree::IsSingleSelection()
{
	// right now only support single selection
	return TRUE;
}

