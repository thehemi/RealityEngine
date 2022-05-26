/*********************************************************************NVMH4****
Path:  NVSDK\Common\src\nv_gui
File:  DynamicDlg.cpp

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

The main dialog that handles display if INVGUIItem objects.  Calls on events
and builds the hierarchy.


The guiitem classes are maintained by the client app (although most clients just
use the default implementations in nvguidata.h

The tree items are the graphical representations of the gui items, and are synced
to them as and when appropriate.

******************************************************************************/

// DynamicDlg.cpp : implementation file
//

#include "stdafx.h"
#include "nv_gui\gui.h"
#include "nv_gui\dynamicdlg.h"

using namespace nv_gui;
using namespace nv_sys;
using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDynamicDlg dialog

#define GetAValue(rgba)      ((BYTE)((rgba)>>24) & 0xFF)


INVGUI* INVGUI::CreateInstance(){
	void* pObj;
	CDynamicDlg::CreateNVObject(NULL,IID_INVGUI,&pObj);
	//return new CDynamicDlg();
	return (INVGUI*)pObj;
}

CDynamicDlg::CDynamicDlg(CWnd* pParent)
	: CDialog(CDynamicDlg::IDD, pParent),
	m_dwRefCount(1),
	m_pEventSink(NULL),
	m_pGUIData(auto_ptr<INVGUIData>(new NVGUIData))
{
	//{{AFX_DATA_INIT(CDynamicDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Register document templates
}

bool INTCALLTYPE CDynamicDlg::CreateNVObject(const nv_sys::INVCreator* pCreator, const NVGUID& InterfaceID, void** ppObj)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CDynamicDlg* pDynamicDlg = new CDynamicDlg();
	if (!pDynamicDlg)
		return false;

	if (pDynamicDlg->GetInterface(InterfaceID, ppObj))
	{
		// Create the panel
		pDynamicDlg->Create(IDD_PANEL, NULL);
		//CRect r;
		//pDynamicDlg->GetWindowRect
	//	pDynamicDlg->SetWindowPos(0,0,0,400,400,0);

		// Tell the app
	  	theApp.AddDialog(pDynamicDlg);

		pDynamicDlg->Release();
		return true;
	}

	delete pDynamicDlg;
	return false;
}

void CDynamicDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDynamicDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDynamicDlg, CDialog)
	//{{AFX_MSG_MAP(CDynamicDlg)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CLOSE()
  	ON_WM_DESTROY()
    ON_COMMAND(ID_READONLYATTRIBUTES,OnReadOnlyAttributes)
    ON_UPDATE_COMMAND_UI(ID_READONLYATTRIBUTES,OnUpdateReadOnlyAttributes)
    ON_COMMAND(ID_EXPANDALL,OnExpandAttributes)
    ON_UPDATE_COMMAND_UI(ID_EXPANDALL,OnUpdateExpandAttributes)
  	ON_WM_WINDOWPOSCHANGED()
    //}}AFX_MSG_MAP
    ON_NOTIFY(PTN_ITEMCHANGED, IDC_PROPERTYTREE, OnItemChanged)
    ON_NOTIFY(PTN_FILEEDIT, IDC_PROPERTYTREE, OnFileEdit)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDynamicDlg message handlers
/*
BOOL CDynamicDlg::PreCreateWindow(CREATESTRUCT& cs) 
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    cs.lpszClass = AfxRegisterWndClass( CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW, 
                                        AfxGetApp()->LoadIcon(IDC_ARROW),    
                                        (HBRUSH) (COLOR_WINDOW + 1), 
                                        AfxGetApp()->LoadIcon(IDR_CGFXMAINFRAME));
    cs.dwExStyle = WS_EX_TOOLWINDOW;
    cs.hMenu = LoadMenu(AfxGetResourceHandle(),MAKEINTRESOURCE(IDR_CGFXMENU));

	return CDialog::PreCreateWindow(cs);
}
*/
BOOL CDynamicDlg::OnInitDialog() 
{
	if (!CDialog::OnInitDialog())
		return FALSE;

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		 | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_CGFXMAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return FALSE; // fail to create
	}

	SetupToolBar(m_wndToolBar, IDB_CGFX_TOOLBAR, IDB_CGFX_TOOLBAR_DISABLED, IDB_CGFX_TOOLBAR, 
			m_ToolBarImages, m_ToolBarImagesDisabled, m_ToolBarImagesHot);

	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() | CBRS_FLYBY);

	DWORD dwStyle;
	CRect rc;
    
    SetIcon(LoadIcon(AfxGetResourceHandle(),MAKEINTRESOURCE(IDI_CONNECTION)),FALSE);

	CRect rcClientStart;
	CRect rcClientNow;
	GetClientRect(rcClientStart);
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 
				   0, reposQuery, rcClientNow);
	
	// Now move all the controls so they are in the same relative
	// position within the remaining client area as they would be
	// with no control bars.
	CPoint ptOffset(rcClientNow.left - rcClientStart.left,
					rcClientNow.top - rcClientStart.top); 

	CRect  rcChild;					
	CWnd* pwndChild = GetWindow(GW_CHILD);
	while (pwndChild)
	{                               
		pwndChild->GetWindowRect(rcChild);
		ScreenToClient(rcChild);
		rcChild.OffsetRect(ptOffset);
		pwndChild->MoveWindow(rcChild, FALSE);
		pwndChild = pwndChild->GetNextWindow();
	}

	// Adjust the dialog window dimensions
	CRect rcWindow;
	GetWindowRect(rcWindow);
	rcWindow.right += rcClientStart.Width() - rcClientNow.Width();
	rcWindow.bottom += rcClientStart.Height() - rcClientNow.Height();
	MoveWindow(rcWindow, FALSE);

	// Create CPropTree control
	// Init the control's size to cover the entire client area
	// PTS_NOTIFY - CPropTree will send notification messages to the parent window
	dwStyle = WS_CHILD|WS_VISIBLE|PTS_NOTIFY;
	GetClientRect(rc);
    CRect but_rect;
    m_wndToolBar.GetWindowRect(but_rect);
    rc.top += but_rect.Height();
	m_Tree.Create(dwStyle, rc, this, IDC_PROPERTYTREE);

	// And position the control bars
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
		

    EnableToolTips(FALSE);
    return TRUE;
}

// find every pixel of the default background color in the specified
// bitmap and set each one to the user's button color.
void ReplaceBackgroundColor (CBitmap& ioBM)
{
	// figure out how many pixels there are in the bitmap
	BITMAP		bmInfo;

	VERIFY (ioBM.GetBitmap (&bmInfo));

	// add support for additional bit depths if you choose
	VERIFY (bmInfo.bmBitsPixel == 24);
	VERIFY (bmInfo.bmWidthBytes == (bmInfo.bmWidth * 3));

	const UINT		numPixels (bmInfo.bmHeight * bmInfo.bmWidth);

	// get a pointer to the pixels
	DIBSECTION  ds;

	VERIFY (ioBM.GetObject (sizeof (DIBSECTION), &ds) == sizeof (DIBSECTION));

	RGBTRIPLE*		pixels = reinterpret_cast<RGBTRIPLE*>(ds.dsBm.bmBits);
	VERIFY (pixels != NULL);

	// get the user's preferred button color from the system
	const COLORREF		buttonColor (::GetSysColor (COLOR_BTNFACE));
	const RGBTRIPLE		userBackgroundColor = {
	GetBValue (buttonColor), GetGValue (buttonColor), GetRValue (buttonColor)};

	// search through the pixels, substituting the user's button
	// color for any pixel that has the magic background color
	for (UINT i = 0; i < numPixels; ++i)
	{
		if (pixels [i].rgbtBlue == kBackgroundColor.rgbtBlue 
		&& pixels [i].rgbtGreen == kBackgroundColor.rgbtGreen 
		&& pixels [i].rgbtRed == kBackgroundColor.rgbtRed)
		{
			pixels [i] = userBackgroundColor;
		}
	}
}

// create an image list for the specified BMP resource
void MakeImageList (UINT inBitmapID, UINT inMaskBitmapID, CImageList& outImageList, int ImageWidth, int ImageHeight, int NumImages)
{
    CBitmap		bm;
    CBitmap		bmMask;
	// if we use CBitmap::LoadBitmap() to load the bitmap, the colors
	// will be reduced to the bit depth of the main screen and we won't
	// be able to access the pixels directly. To avoid those problems,
	// we'll load the bitmap as a DIBSection instead and attach the
	// DIBSection to the CBitmap.
	VERIFY (bm.Attach (::LoadImage (::AfxFindResourceHandle(
	MAKEINTRESOURCE (inBitmapID), RT_BITMAP),
	MAKEINTRESOURCE (inBitmapID), IMAGE_BITMAP, 0, 0,
	(LR_DEFAULTSIZE| LR_CREATEDIBSECTION))));

    if (inMaskBitmapID)
    {
	    VERIFY (bmMask.Attach (::LoadImage (::AfxFindResourceHandle(
	    MAKEINTRESOURCE (inMaskBitmapID), RT_BITMAP),
	    MAKEINTRESOURCE (inMaskBitmapID), IMAGE_BITMAP, 0, 0,
	    (LR_DEFAULTSIZE | LR_CREATEDIBSECTION))));
    }

    if (inMaskBitmapID)
    {
 	    // create a 24 bit image list with the same dimensions and number
	    // of buttons as the toolbar
	    VERIFY (outImageList.Create (
	    ImageWidth, ImageHeight, kToolBarBitDepth|ILC_MASK, NumImages, 0));
       // attach the bitmap to the image list
        VERIFY (outImageList.Add (&bm, &bmMask) != -1);
    }
    else
    {
	    // replace the specified color in the bitmap with the user's
	    // button color
	    ReplaceBackgroundColor (bm);
	    // create a 24 bit image list with the same dimensions and number
	    // of buttons as the toolbar
	    VERIFY (outImageList.Create (
	    ImageWidth, ImageHeight, kToolBarBitDepth, NumImages, 0));
        // attach the bitmap to the image list
        VERIFY (outImageList.Add (&bm, RGB(0, 0, 0)) != -1);
    }
}

void CDynamicDlg::SetupToolBar(CToolBar& ToolBar, 
							  UINT BarImageID,
							  UINT BarDisabledImageID,
							  UINT BarHotImageID, 
							  CImageList& Image,
							  CImageList& ImageDisabled,
							  CImageList& ImageHot)
{

	// make high-color image lists for each of the bitmaps
	MakeImageList (BarImageID, 0, Image, kImageWidth, kImageHeight, kNumImages);
	MakeImageList (BarDisabledImageID, 0, ImageDisabled, kImageWidth, kImageHeight, kNumImages);
	MakeImageList (BarHotImageID, 0, ImageHot, kImageWidth, kImageHeight, kNumImages);

	// get the toolbar control associated with the CToolbar object
	CToolBarCtrl&	barCtrl = ToolBar.GetToolBarCtrl();

	// attach the image lists to the toolbar control
	barCtrl.SetImageList(&Image);
	barCtrl.SetDisabledImageList(&ImageDisabled);
	barCtrl.SetHotImageList(&ImageHot);
}

void CDynamicDlg::OnUpdateReadOnlyAttributes( CCmdUI* pCmdUI )
{
    pCmdUI->SetCheck(m_Tree.IsFilter(0));
}

void CDynamicDlg::OnReadOnlyAttributes()
{
    m_Tree.Filter(0,0);
}

void CDynamicDlg::OnExpandAttributes()
{
    CPropTreeItem* pItem = m_Tree.GetSelectedItem();
    if (pItem)
    {
        if (pItem->IsExpanded())
            m_Tree.ExpandAll(pItem,FALSE);
        else
            m_Tree.ExpandAll(pItem,TRUE);
        m_Tree.Invalidate();
    }
        
}

void CDynamicDlg::OnUpdateExpandAttributes( CCmdUI* pCmdUI )
{
    CPropTreeItem* pItem = m_Tree.GetSelectedItem();
    if (pItem)
    {
        if (pItem->GetChild())
            pCmdUI->Enable(TRUE);
        else
            pCmdUI->Enable(FALSE);
        pCmdUI->SetCheck(pItem->IsExpanded());
    }
    else
        pCmdUI->Enable(FALSE);
}

void CDynamicDlg::SetAppName(const char * pAppName)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    m_AppName = pAppName;
    ((CNVGUIApp*)AfxGetApp())->SetAppName(pAppName);

    // Custom position/size...
    if (((CNVGUIApp*)AfxGetApp())->ReadWindowPosRegKey("CGFXGUI_WindowPosData",GetSafeHwnd()) == FALSE)
        ((CNVGUIApp*)AfxGetApp())->WriteWindowPosRegKey("CGFXGUI_WindowPosData",GetSafeHwnd());
}

void CDynamicDlg::SetParentWindow(HWND hWnd)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    if (hWnd)
    {
        SetParent(CWnd::FromHandle(hWnd));
        ((CNVGUIApp*)AfxGetApp())->m_hParent = hWnd;
    }
}

unsigned long CDynamicDlg::AddRef()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	m_dwRefCount++;
	return m_dwRefCount;
}

unsigned long CDynamicDlg::Release()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	unsigned long RefNew = --m_dwRefCount;
	if (RefNew == 0)
	{
		if (m_hWnd)
		{
			ClearGUI();
			DestroyWindow();
		}
		theApp.RemoveDialog(this);
		delete this;
	}
	return RefNew;
}

bool CDynamicDlg::GetInterface(const NVGUID& guid, void** ppObj)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (EqualNVGUID(guid, IID_INVObject))
	{
		*ppObj = static_cast<INVGUI*>(this);
	}
	else if (EqualNVGUID(guid, IID_INVGUI))
	{		
		*ppObj = static_cast<INVGUI*>(this);
	}
	else
	{
		return false;
	}
		
	static_cast<INVGUI*>(this)->AddRef();
	return true;
}

bool CDynamicDlg::AddItem(nv_gui::INVGUIItem* pItem)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	m_pGUIData->AddItem(pItem);

	CPropTreeItem* pNewItem = SetupItem(m_Tree.GetRootItem(), pItem);
	if (pNewItem)
		pNewItem->Expand(TRUE);

	return true;
}

bool CDynamicDlg::AddChild(nv_gui::INVGUIItem* pParent, nv_gui::INVGUIItem* pChild)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (!pParent || !pChild)
		return false;

	tmapGUIItemTreeItem::iterator itrItem = m_mapGUIItemTreeItem.find(pParent);
	if (itrItem == m_mapGUIItemTreeItem.end())
	{
		assert(!"Couldn't find Parent item in AddChild!");
		return false;
	}

    pParent->AddChild(pChild);
	SetupItem(itrItem->second, pChild);

	return true;
}

bool CDynamicDlg::ExpandAll()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	tmapGUIItemTreeItem::iterator itrItem = m_mapGUIItemTreeItem.begin();
	while (itrItem != m_mapGUIItemTreeItem.end())
	{
        itrItem->second->Expand(TRUE);
        itrItem++;
	}
    return true;
}

bool CDynamicDlg::RemoveChildren(nv_gui::INVGUIItem* pParent)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	tmapGUIItemTreeItem::iterator itrGUIItem;
	tmapTreeItemGUIItem::iterator itrTreeItem;
	
	itrGUIItem = m_mapGUIItemTreeItem.find(pParent);
	if (itrGUIItem == m_mapGUIItemTreeItem.end())
	{
		assert(!"Couldn't find GUI Item in RemoveChildren");
		return false;
	}

	CPropTreeItem* pIter = itrGUIItem->second->GetChild();
	CPropTreeItem* pNext = pIter;
	while (pNext)
	{
		pNext = pIter->GetSibling();
		m_Tree.DeleteItem(pIter);

		itrTreeItem = m_mapTreeItemGUIItem.find(pIter);
		if (itrTreeItem == m_mapTreeItemGUIItem.end())
		{
			assert(!"Couldn't find tree item in removechildren");
			return false;
		}

		itrGUIItem = m_mapGUIItemTreeItem.find(itrTreeItem->second);
		if (itrGUIItem == m_mapGUIItemTreeItem.end())
		{
			assert(!"Couldnt' find gui item in removechildren");
			return false;
		}

		// Remove the mapping for this child
		m_mapGUIItemTreeItem.erase(itrGUIItem);
		m_mapTreeItemGUIItem.erase(itrTreeItem);

		pIter = pNext;
	}

	pParent->RemoveChildren();

	return true;
}

bool CDynamicDlg::RemoveItem(nv_gui::INVGUIItem* pItem)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	tmapGUIItemTreeItem::iterator itrGUIItem = m_mapGUIItemTreeItem.find(pItem);
	if (itrGUIItem == m_mapGUIItemTreeItem.end())
	{
		assert(!"Couldn't find GUI ITem in RemoveItem");
		return false;
	}
	tmapTreeItemGUIItem::iterator itrTreeItem = m_mapTreeItemGUIItem.find(itrGUIItem->second);
	if (itrTreeItem == m_mapTreeItemGUIItem.end())
	{
		assert(!"Couldn't find tree item in RemoveItem");
		return false;
	}

	m_Tree.DeleteItem(itrGUIItem->second);
	if (pItem->GetParent())
	{
		pItem->GetParent()->RemoveChild(pItem);
	}

	// Remove the mapping
	m_mapGUIItemTreeItem.erase(itrGUIItem);
	m_mapTreeItemGUIItem.erase(itrTreeItem);
	return true;
}

bool CDynamicDlg::RefreshGUI(INVGUIItem* pItem)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	tmapGUIItemTreeItem::iterator itrGUIItem = m_mapGUIItemTreeItem.find(pItem);
	if (itrGUIItem == m_mapGUIItemTreeItem.end())
	{
		assert(!"Couldn't find tree item in RefreshGUI");
		return false;
	}

	// Sync the tree item to the gui item
	SyncData(itrGUIItem->first, itrGUIItem->second, COPY_GUIITEM_TO_TREEITEM);
    /*
    CWnd * wnd = GetFocus();
    if (wnd && (wnd->GetSafeHwnd() != GetSafeHwnd())&&
        GetParent()->GetSafeHwnd() != GetSafeHwnd())
    {
        CRect rect;
        wnd->GetWindowRect(rect);
        m_Tree.ScreenToClient(rect);

        m_Tree.InvalidateRect(0,FALSE);
        m_Tree.ValidateRect(rect);
	    m_Tree.UpdateWindow();
    }
    else*/
        m_Tree.Invalidate();

	return true;
}

bool CDynamicDlg::ClearGUI()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	m_Tree.DeleteAllItems();
	m_mapTreeItemGUIItem.clear();
	m_mapGUIItemTreeItem.clear();

	// Kill the data itemsm, being careful here.
	while (m_pGUIData->GetNumItems() != 0)
	{
		INVGUIItem* pItem = m_pGUIData->GetItem(0);
		pItem->RemoveChildren();
		m_pGUIData->RemoveItem(pItem);
	}

	return true;
}

void CDynamicDlg::OnClose() 
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

    ShowWindow(SW_HIDE);
    ((CNVGUIApp*)AfxGetApp())->WriteWindowPosRegKey("CGFXGUI_WindowPosData",GetSafeHwnd());
}

void CDynamicDlg::OnDestroy()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	// Have to post a message so we don't get into a race condition
	if (m_pEventSink)
	{
		m_pEventSink->PostMessage(EVENT_CLOSEPANEL, 0);
	}
}

bool CDynamicDlg::SetVisible(bool bShow)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (IsWindow(m_hWnd))
	{
		if (bShow)
            ShowWindow(SW_SHOW);
		else
			ShowWindow(SW_HIDE);
	}

	return true;
}

bool CDynamicDlg::IsVisible()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (m_hWnd && IsWindowVisible())
		return true;

	return false;
}

bool CDynamicDlg::AddEventSink(INVGUIEvent* pSink)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	m_pEventSink = pSink;

	return true;
}

bool CDynamicDlg::SetImageList(HIMAGELIST hImageList,HIMAGELIST hImageListDisabled)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
    m_Tree.SetImageList(*CImageList::FromHandle(hImageList), *CImageList::FromHandle(hImageListDisabled));
    return true;
}

unsigned int CDynamicDlg::SetStateImageIdx(INVGUIItem::ItemState state, unsigned int idx)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    return m_Tree.SetStateImageIdx((CPropTreeItem::ItemState)state, idx);
}

unsigned int CDynamicDlg::GetStateImageIdx(INVGUIItem::ItemState state)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return m_Tree.GetStateImageIdx((CPropTreeItem::ItemState)state);
}

BOOL CDynamicDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return CDialog::OnCommand(wParam, lParam);
}

void CDynamicDlg::OnSize(UINT nType, int cx, int cy) 
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CDialog::OnSize(nType, cx, cy);
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);

    if (::IsWindow(m_Tree.GetSafeHwnd()))
    {
        CRect but_rect;
        m_wndToolBar.GetWindowRect(but_rect);
        m_Tree.SetWindowPos(NULL, 0, but_rect.Height(), cx, cy - but_rect.Height(), SWP_NOZORDER);	
    }
}

void CDynamicDlg::OnWindowPosChanged( WINDOWPOS* lpwndpos )
{
    CDialog::OnWindowPosChanged(lpwndpos);
    if (m_dwRefCount && ::IsWindow(m_Tree.GetSafeHwnd()))
        ((CNVGUIApp*)AfxGetApp())->WriteWindowPosRegKey("CGFXGUI_WindowPosData",GetSafeHwnd());
}

CPropTreeItem* CDynamicDlg::SetupItem(CPropTreeItem* pParent, INVGUIItem* pGUIItem)
{
	CPropTreeItem* pTreeItem = NULL;

	switch(pGUIItem->GetType())
	{
		case GUITYPE_INT:
		{
			CPropTreeItemEdit* pEdit = new CPropTreeItemEdit(m_pEventSink);
			pTreeItem = m_Tree.InsertItem(pEdit, pParent);
			pEdit->SetValueFormat(CPropTreeItemEdit::ValueFormatNumber);
		}
		break;

		case GUITYPE_DWORD:
		{
			CPropTreeItemEdit* pEdit = new CPropTreeItemEdit(m_pEventSink);
			pTreeItem = m_Tree.InsertItem(pEdit, pParent);
			pEdit->SetValueFormat(CPropTreeItemEdit::ValueFormatNumber);
		}
		break;

		case GUITYPE_FLOAT:
		{
			CPropTreeItemEdit* pEdit = new CPropTreeItemEdit(m_pEventSink);
			pTreeItem = m_Tree.InsertItem(pEdit, pParent);
			pEdit->SetValueFormat(CPropTreeItemEdit::ValueFormatFloatPointer);
		}
		break;

		case GUITYPE_SLIDER:
		{
			pTreeItem = m_Tree.InsertItem(new CPropTreeItem(m_pEventSink), pParent);
		}
		break;

		case GUITYPE_VECTOR2:
		{
			CPropTreeItemGrid* pGrid = new CPropTreeItemGrid(m_pEventSink);
			pGrid->SetDimensions(1, 2);
            pGrid->SetTitle(pGUIItem->GetName());

			pTreeItem = m_Tree.InsertItem(pGrid, pParent);
		}
		break;

		case GUITYPE_VECTOR3:
		{
			CPropTreeItemGrid* pGrid = new CPropTreeItemGrid(m_pEventSink);
			pGrid->SetDimensions(1, 3);
            pGrid->SetTitle(pGUIItem->GetName());
			pTreeItem = m_Tree.InsertItem(pGrid, pParent);
		}
		break;

		case GUITYPE_VECTOR4:
		{
			CPropTreeItemGrid* pGrid = new CPropTreeItemGrid(m_pEventSink);
			pGrid->SetDimensions(1, 4);
            pGrid->SetTitle(pGUIItem->GetName());
			pTreeItem = m_Tree.InsertItem(pGrid, pParent);
		}
		break;

		case GUITYPE_MATRIX:
		{
			INVGUIItem_Matrix* pMatrix = static_cast<INVGUIItem_Matrix*>(pGUIItem);
			CPropTreeItemGrid* pGrid = new CPropTreeItemGrid(m_pEventSink);
			pGrid->SetDimensions(pMatrix->GetRows(), pMatrix->GetColumns());
            pGrid->SetTitle(pGUIItem->GetName());
			pTreeItem = m_Tree.InsertItem(pGrid, pParent);
		}
		break;

		case GUITYPE_COLOR:
		{
			pTreeItem = m_Tree.InsertItem(new CPropTreeItemColor(m_pEventSink), pParent);
		}
		break;

		case GUITYPE_BRANCH:
		{
			pTreeItem = m_Tree.InsertItem(new CPropTreeItem(m_pEventSink), pParent);
		}
		break;

		case GUITYPE_TEXT:
		{
			pTreeItem = m_Tree.InsertItem(new CPropTreeItemStatic(m_pEventSink), pParent);
		}
		break;

		case GUITYPE_BOOL:
		{
			CPropTreeItemCombo* pCombo = new CPropTreeItemCombo(m_pEventSink);
			pTreeItem = m_Tree.InsertItem(pCombo, pParent);
			pCombo->CreateComboBoxBool();	// create the ComboBox control and auto fill with TRUE/FALSE values
		}
		break;

        case GUITYPE_LISTBOX:
        {
            CPropTreeItemCombo* pCombo = new CPropTreeItemCombo(m_pEventSink);
            pTreeItem = m_Tree.InsertItem(pCombo, pParent);
            pCombo->CreateComboBox(WS_CHILD | WS_VSCROLL | CBS_DROPDOWNLIST);
            INVGUIItem_ListBox* pListBox = static_cast<INVGUIItem_ListBox*>(pGUIItem);
            
            for (int i = 0; i < pListBox->GetNumStrings(); i++)
            {
                pCombo->AddString(pListBox->GetString(i));
                pCombo->SetItemData(i, i);
                pCombo->SetCurSel(pListBox->GetIndex());
            }
        }
        break;

		case GUITYPE_FILEPATH:
		{
			CPropTreeItemFileBrowse* pBrowse = new CPropTreeItemFileBrowse(m_pEventSink);
            INVGUIItem_FilePath* pFilePath = static_cast<INVGUIItem_FilePath*>(pGUIItem);
			pBrowse->SetTruncateDisplay(pFilePath->IsTruncatedDisplay() ? TRUE : FALSE);
			pBrowse->SetExtensionString(pFilePath->GetExtensionString());
            pBrowse->SetPreview(pFilePath->IsPreview());

			pTreeItem = m_Tree.InsertItem(pBrowse, pParent);
		}
		break;

		default: 
		{
			// Just create a default
			pTreeItem = m_Tree.InsertItem(new CPropTreeItem(m_pEventSink), pParent);
			assert(!"Unknown GUI Item type!");
		}
		break;
	}

	// Fill in the standard stuff
	if (pTreeItem)
	{
        if(pGUIItem->IsImage())
            pTreeItem->HasImage(TRUE,pGUIItem->GetImageIdx());
        
        pTreeItem->SetState(pGUIItem->GetState());
        
		// Copy the info in the GUI item we were supplied with to the tree item
		SyncData(pGUIItem, pTreeItem, COPY_GUIITEM_TO_TREEITEM);

		// Remember where it is.
		pTreeItem->SetLabelText(_T(pGUIItem->GetName()));
		pTreeItem->SetInfoText(_T(pGUIItem->GetInfoText()));
		m_mapTreeItemGUIItem[pTreeItem] = pGUIItem;
		m_mapGUIItemTreeItem[pGUIItem] = pTreeItem;
		return pTreeItem;
	}

	assert(!"Failed in SetupItem");
	return NULL;
}

bool CDynamicDlg::PreTranslateMessage(WORD hWnd, WORD Message, WORD wParam, DWORD lParam) 
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    MSG msg;
    msg.message = Message;
    msg.hwnd = (HWND)hWnd;
    msg.lParam = lParam;
    msg.wParam = wParam;
    msg.pt = CPoint(0, 0);
    msg.time = 0;

    // If it's one of ours, handle it.
    if (theApp.PreTranslateMessage(&msg))
       return true;

    return false;
}

void CDynamicDlg::OnFileEdit(NMHDR* pNotifyStruct, LRESULT* plResult)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

    LPNMPROPTREE pNMPropTree = (LPNMPROPTREE)pNotifyStruct;

	if (pNMPropTree->pItem)
	{
        CPropTreeItemFileBrowse * pPropFileEdit = (CPropTreeItemFileBrowse *)pNMPropTree->pItem;
#ifdef _WIN32
        std::string arg = "\"";
        arg += (LPCSTR)pPropFileEdit->GetItemValue();
        arg += "\"";
	    SHELLEXECUTEINFO sinfo;
	    ::ZeroMemory(&sinfo,sizeof(SHELLEXECUTEINFO));
	    sinfo.cbSize = sizeof(SHELLEXECUTEINFO);
	    sinfo.fMask |= SEE_MASK_NOCLOSEPROCESS|SEE_MASK_FLAG_NO_UI;
        sinfo.lpVerb = "open";
	    sinfo.lpFile = arg.c_str();
        sinfo.nShow = SW_SHOW;
        const char * extension = strrchr((LPCSTR)pPropFileEdit->GetItemValue(),'.');

        // We want to let the user define its preferred text editor to compose .fx files.
        // If none can be found, we prompt our very own editor as a fallback only if we are trying 
        // to edit an .
	    if (ShellExecuteEx(&sinfo) == FALSE)
        {
            if (extension && strncmp(extension,".fx", 3) == 0)
            {
                CDocument * pDoc = ((CNVGUIApp*)AfxGetApp())->OpenDocumentFile((LPCSTR)pPropFileEdit->GetItemValue());
                if (pDoc)
                {
                    AfxGetApp()->GetMainWnd()->ShowWindow(SW_SHOW);
                    AfxGetApp()->GetMainWnd()->UpdateWindow();
                }
            }
            else
            {
                LPVOID lpMsgBuf;
                FormatMessage(  FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                                FORMAT_MESSAGE_FROM_SYSTEM | 
                                FORMAT_MESSAGE_IGNORE_INSERTS,
                                NULL,
                                GetLastError(),
                                0, // Default language
                                (LPTSTR) &lpMsgBuf,
                                0,
                                NULL );
                // Display the string.
                MessageBox( (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );
                // Free the buffer.
                LocalFree( lpMsgBuf );
            }
        }
#endif
    }
    *plResult = 0;
}

// Called to tell us the user messed with the UI.
void CDynamicDlg::OnItemChanged(NMHDR* pNotifyStruct, LRESULT* plResult)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

    LPNMPROPTREE pNMPropTree = (LPNMPROPTREE)pNotifyStruct;

	if (pNMPropTree->pItem)
	{
		// Find the GUI Item.
		tmapTreeItemGUIItem::iterator itrItem = m_mapTreeItemGUIItem.find(pNMPropTree->pItem);
		if (itrItem != m_mapTreeItemGUIItem.end())
		{
			INVGUIItem* pGUIItem = itrItem->second;

			// Sync the GUI Item to the tree item
			SyncData(pGUIItem, (CPropTreeItem*)pNMPropTree->pItem, COPY_TREEITEM_TO_GUIITEM);

			// Tell the client something changed
			if (m_pEventSink)
				m_pEventSink->PostMessage(EVENT_UPDATEITEM, pGUIItem);
		}
	}

	*plResult = 0;
}

// Syncs a gui item to a tree item.
bool CDynamicDlg::SyncData(INVGUIItem* pGUIItem, CPropTreeItem* pTreeItem, tSyncDirection SyncDirection)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

    switch(pGUIItem->GetType())
	{
		case GUITYPE_SLIDER:
		{
			INVGUIItem_Slider* pSlider = static_cast<INVGUIItem_Slider*>(pGUIItem);
			if (SyncDirection == COPY_TREEITEM_TO_GUIITEM)
			{
				;
			}
			else
			{
				;
			}
		}
		break;

		case GUITYPE_DWORD:
		{
			INVGUIItem_Dword* pDword = static_cast<INVGUIItem_Dword*>(pGUIItem);
			if (SyncDirection == COPY_TREEITEM_TO_GUIITEM)
			{
				pDword->SetDword(pTreeItem->GetItemValue());
			}
			else
			{
				pTreeItem->SetItemValue(pDword->GetDword());
			}
		}
		break;

		case GUITYPE_INT:
		{
			INVGUIItem_Int* pInt = static_cast<INVGUIItem_Int*>(pGUIItem);
			if (SyncDirection == COPY_TREEITEM_TO_GUIITEM)
			{
				pInt->SetInt(pTreeItem->GetItemValue());
			}
			else
			{
				pTreeItem->SetItemValue(pInt->GetInt());
			}
		}
		break;

		case GUITYPE_VECTOR2:
		{
			INVGUIItem_Vector2* pVec = static_cast<INVGUIItem_Vector2*>(pGUIItem);
			if (SyncDirection == COPY_TREEITEM_TO_GUIITEM)
			{
				pVec->SetVector(*reinterpret_cast<vec2*>(pTreeItem->GetItemValue()));
			}
			else
			{
				vec2 Val = pVec->GetVector();
				pTreeItem->SetItemValue(reinterpret_cast<LPARAM>(&Val));
			}
		}
		break;
		case GUITYPE_VECTOR3:
		{
			INVGUIItem_Vector3* pVec = static_cast<INVGUIItem_Vector3*>(pGUIItem);
			if (SyncDirection == COPY_TREEITEM_TO_GUIITEM)
			{
				pVec->SetVector(*reinterpret_cast<vec3*>(pTreeItem->GetItemValue()));
			}
			else
			{
				vec3 Val = pVec->GetVector();
				pTreeItem->SetItemValue(reinterpret_cast<LPARAM>(&Val));
			}
		}
		break;
		case GUITYPE_VECTOR4:
		{
			INVGUIItem_Vector4* pVec = static_cast<INVGUIItem_Vector4*>(pGUIItem);
			if (SyncDirection == COPY_TREEITEM_TO_GUIITEM)
			{
				pVec->SetVector(*reinterpret_cast<vec4*>(pTreeItem->GetItemValue()));
			}
			else
			{
				vec4 Val = pVec->GetVector();
				pTreeItem->SetItemValue(reinterpret_cast<LPARAM>(&Val));
			}
		}
		break;

		case GUITYPE_MATRIX:
		{
			INVGUIItem_Matrix* pMat = static_cast<INVGUIItem_Matrix*>(pGUIItem);
			if (SyncDirection == COPY_TREEITEM_TO_GUIITEM)
			{
				pMat->SetArray(reinterpret_cast<float*>(pTreeItem->GetItemValue()));
			}
			else
			{
				pTreeItem->SetItemValue(reinterpret_cast<LPARAM>(pMat->GetArray()));
			}
		}
		break;

		case GUITYPE_FLOAT:
		{
			INVGUIItem_Float* pFloat = static_cast<INVGUIItem_Float*>(pGUIItem);
			if (SyncDirection == COPY_TREEITEM_TO_GUIITEM)
			{
				pFloat->SetFloat(*reinterpret_cast<float*>(pTreeItem->GetItemValue()));
			}
			else
			{
				float fVal = pFloat->GetFloat();
                float min,max,step;
                pFloat->GetMinMaxStep(min,max,step);
                pTreeItem->SetItemValue(reinterpret_cast<LPARAM>(&fVal));
				
                if (step != 0.0f)
                {
                    CPropTreeItemEdit * pTreeItemEdit = static_cast<CPropTreeItemEdit*>(pTreeItem);
                    pTreeItemEdit->SetRangeParams(min,max,step);
                }
			}
		}
		break;

		case GUITYPE_COLOR:
		{
			INVGUIItem_Color* pColor = static_cast<INVGUIItem_Color*>(pGUIItem);

			if (SyncDirection == COPY_TREEITEM_TO_GUIITEM)
			{
				COLORREF color_ref = (COLORREF)pTreeItem->GetItemValue();
				vec4 color((float)GetRValue(color_ref) / nv_scalar(255.0), 
							(float)GetGValue(color_ref) / nv_scalar(255.0), 
							(float)GetBValue(color_ref) / nv_scalar(255.0),
							(float)GetAValue(color_ref) / nv_scalar(255.0));
				pColor->SetColor(color);                
			}
			else
			{
				vec4 color = pColor->GetColor();
				COLORREF color_ref = RGB(color.x * 255.0f, color.y * 255.0f, color.z * 255.0f);
				color_ref |= ((DWORD)(color.w * 255.0f) & 0xFF) << 24;
				pTreeItem->SetItemValue((LPARAM)color_ref);
			}
		}
		break;
		
		case GUITYPE_BRANCH:
		{
			INVGUIItem_Branch* pBranch = static_cast<INVGUIItem_Branch*>(pGUIItem);
			if (SyncDirection == COPY_TREEITEM_TO_GUIITEM)
			{
				;
			}
			else
			{
				;
			}

		}
		break;

		case GUITYPE_TEXT:
		{
			INVGUIItem_Text* pText = static_cast<INVGUIItem_Text*>(pGUIItem);
			if (SyncDirection == COPY_TREEITEM_TO_GUIITEM)
			{
				pText->SetString(reinterpret_cast<const char*>(pTreeItem->GetItemValue()));
			}
			else
			{
				pTreeItem->SetItemValue((LPARAM)pText->GetString());;
			}
		}
		break;

		case GUITYPE_FILEPATH:
		{
			INVGUIItem_FilePath* pFilePath = static_cast<INVGUIItem_FilePath*>(pGUIItem);
			if (SyncDirection == COPY_TREEITEM_TO_GUIITEM)
			{
				pFilePath->SetPath(reinterpret_cast<const char*>(pTreeItem->GetItemValue()));
                pFilePath->SetState(pTreeItem->GetState());
			}
			else
			{
				pTreeItem->SetItemValue(reinterpret_cast<LPARAM>(pFilePath->GetPath()));
                pTreeItem->SetState(pFilePath->GetState());
			}			
		}
		break;
 
		case GUITYPE_BOOL:
		{
			INVGUIItem_Bool* pBool = static_cast<INVGUIItem_Bool*>(pGUIItem);
			if (SyncDirection == COPY_TREEITEM_TO_GUIITEM)
			{
				pBool->SetBool(pTreeItem->GetItemValue() ? true : false);
			}
			else
			{
				pTreeItem->SetItemValue(pBool->GetBool() ? TRUE : FALSE);
			}
		}
		break;

        case GUITYPE_LISTBOX:
        {
            INVGUIItem_ListBox* pListBox = static_cast<INVGUIItem_ListBox*>(pGUIItem);
            CPropTreeItemCombo* pCombo = reinterpret_cast<CPropTreeItemCombo*>(pTreeItem);
            if (SyncDirection == COPY_TREEITEM_TO_GUIITEM)
            {
                pListBox->SetIndex((int)pTreeItem->GetItemValue());
            }
            else
            {
                pTreeItem->SetItemValue((LPARAM)pListBox->GetIndex());
            }
        }
        break;

		default: 
		{
			assert(!"Unknown item!");
			return false;
		}
		break;
	}
	return true;

}


