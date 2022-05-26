/*********************************************************************NVMH4****
Path:  NVSDK\Common\src\nv_gui
File:  GridDlg.cpp

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


For vector/matrix editing in nv_gui


******************************************************************************/

// GridDlg.cpp : implementation file
//

#include "stdafx.h"
#include "nv_gui\gui.h"
#include "GridDlg.h"
#include "GridCellNumeric.h"
#include <sstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGridDlg dialog


CGridDlg::CGridDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGridDlg::IDD, pParent),
    m_Grid(0, 0, 0, 0),
    m_x(0),
    m_y(0)
{
	//{{AFX_DATA_INIT(CGridDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CGridDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGridDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGridDlg, CDialog)
	//{{AFX_MSG_MAP(CGridDlg)
	ON_WM_CREATE()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGridDlg message handlers

int CGridDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
    if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
    
    SetWindowText(m_strTitle.c_str());

    CRect rc;
    GetClientRect(rc);	

    m_Grid.SetAutoSizeStyle(GVS_DEFAULT);
    m_Grid.SetEditable(TRUE);
    m_Grid.SetCompareFunction(CGridCtrl::pfnCellNumericCompare);
    m_Grid.SetDefaultCellType(RUNTIME_CLASS(CGridCellNumeric));
    m_Grid.Create(rc, this, 101);
        
	return 0;
}

void CGridDlg::OnSize(UINT nType, int cx, int cy) 
{
   
    CDialog::OnSize(nType, cx, cy);
}

#define CELL_WIDTH 70
#define CELL_HEIGHT 25
BOOL CGridDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
    // Set up the grid dimensions
    SetDimensions(m_x, m_y);

    // Get the client rect we need.
    CRect rcNewWin;
    rcNewWin.SetRect(0, 0, m_x * CELL_WIDTH, m_y * CELL_HEIGHT);

    // Calculate a window size based on the client
    CRect rcAdjusted = rcNewWin;
    AdjustWindowRectEx(rcAdjusted, WS_POPUP | WS_CAPTION | WS_SYSMENU, FALSE, WS_EX_TOOLWINDOW);

    CRect rcWinRect(0,0,0,0);
    if (GetParent())
    {
        GetParent()->GetWindowRect(&rcWinRect);
    }

    // Size it...
    SetWindowPos(&wndTopMost, rcWinRect.left, rcWinRect.top, rcAdjusted.Width(), rcAdjusted.Height(), SWP_NOZORDER | SWP_NOMOVE);

    // Make the grid fill the client
    m_Grid.SetWindowPos(&wndTopMost, 0, 0, rcNewWin.Width(), rcNewWin.Height(), SWP_NOZORDER | SWP_NOMOVE | SWP_NOZORDER);

    // Tell the grid to expand to fill the area
    m_Grid.ExpandToFit(TRUE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CGridDlg::SetDimensions(unsigned int x, unsigned int y)
{
    if (m_x != x || m_y != y)
    {
        m_x = x;
        m_y = y;
        m_Grid.SetRowCount(y);
        m_Grid.SetColumnCount(x);
        m_Floats.resize(m_x * m_y);

        if (IsWindow(m_Grid.GetSafeHwnd()))
            m_Grid.ExpandToFit(TRUE);
    }

}

void CGridDlg::SetValue(unsigned int x, unsigned int y, float fVal)
{
    assert(((y * m_x) + x) < m_Floats.size());
    m_Floats[(y * m_x) + x] = fVal;
    
    std::ostringstream strStream;
    strStream << fVal;
    m_Grid.SetItemText(y,x, strStream.str().c_str());
}

float CGridDlg::GetValue(unsigned int x, unsigned int y)
{
    assert(((y * m_x) + x) < m_Floats.size());

    CGridCellBase* pCell = m_Grid.GetCell(y, x);
    
    float fValue;
    _stscanf(pCell->GetText(), _T("%f"), &fValue);

    m_Floats[(y * m_x) + x] = fValue;
    return fValue;
}

void CGridDlg::SetTitle(const char* pszTitle)
{
    m_strTitle = pszTitle;
}

BOOL CGridDlg::PreCreateWindow(CREATESTRUCT& cs) 
{

	// TODO: Add your specialized code here and/or call the base class
	return CDialog::PreCreateWindow(cs);
}
