/*********************************************************************NVMH4****
Path:  NVSDK\Common\src\nv_gui
File:  cgfxmainframe.cpp

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

// cgfxmainframe.cpp : implementation file
//

#include "stdafx.h"
#include "nv_gui\gui.h"
#include "cgfxmainframe.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace nv_gui;

/////////////////////////////////////////////////////////////////////////////
// CCGFXMainFrame

IMPLEMENT_DYNCREATE(CCGFXMainFrame, CFrameWnd)

CCGFXMainFrame::CCGFXMainFrame() :
m_bCreated(FALSE)
{
}

CCGFXMainFrame::~CCGFXMainFrame()
{
}

static UINT indicators[] =
{
	ID_SEPARATOR,               // status line indicator 
	ID_EDIT_INDICATOR_POSITION,	 
	ID_EDIT_INDICATOR_COL,
	ID_EDIT_INDICATOR_CRLF,
	ID_INDICATOR_OVR,	
	ID_EDIT_INDICATOR_READ,
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

BEGIN_MESSAGE_MAP(CCGFXMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CCGFXMainFrame)
	ON_WM_CREATE()
    ON_WM_CLOSE()
    ON_WM_SIZE()
    ON_WM_MOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCGFXMainFrame message handlers

BOOL CCGFXMainFrame::PreCreateWindow(CREATESTRUCT& cs) 
{
    cs.lpszClass = AfxRegisterWndClass( CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW, 
                                        AfxGetApp()->LoadIcon(IDC_ARROW),    
                                        (HBRUSH) (COLOR_WINDOW + 1), 
                                        AfxGetApp()->LoadIcon(IDR_CGFXMAINFRAME));
    cs.dwExStyle = WS_EX_TOOLWINDOW;
	return CFrameWnd::PreCreateWindow(cs);
}

int CCGFXMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	
    if (((CNVGUIApp*)AfxGetApp())->ReadWindowPosRegKey("CGFXEDITOR_WindowPosData",GetSafeHwnd()) == FALSE)
		((CNVGUIApp*)AfxGetApp())->WriteWindowPosRegKey("CGFXEDITOR_WindowPosData",GetSafeHwnd());

    ShowWindow(SW_SHOW);
    m_bCreated = TRUE;
    return 0;
}

void CCGFXMainFrame::OnClose()
{
	// Note: only queries the active document
	CDocument* pDocument = GetActiveDocument();
	if (pDocument != NULL && !pDocument->CanCloseFrame(this))
	{
		// document can't close right now -- don't close it
		return;
	}

    if (pDocument != NULL)
        pDocument->OnCloseDocument();

    //CFrameWnd::OnClose();
}

void CCGFXMainFrame::OnSize( UINT nType, int cx, int cy)
{
    CFrameWnd::OnSize(nType, cx, cy);
    if (m_bCreated)
        ((CNVGUIApp*)AfxGetApp())->WriteWindowPosRegKey("CGFXEDITOR_WindowPosData",GetSafeHwnd());
}

void CCGFXMainFrame::OnMove( int x, int y)
{
    CFrameWnd::OnMove(x, y);
    if (m_bCreated)
        ((CNVGUIApp*)AfxGetApp())->WriteWindowPosRegKey("CGFXEDITOR_WindowPosData",GetSafeHwnd());
}
