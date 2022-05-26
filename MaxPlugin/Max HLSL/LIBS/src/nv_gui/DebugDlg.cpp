/*********************************************************************NVMH4****
Path:  NVSDK\Common\src\nv_debug
File:  DebugDlg.cpp

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


A very simple listbox console for easy viewing of debug/console messages.


******************************************************************************/

// DebugDlg.cpp : implementation file
//

#include "stdafx.h"
#include "nv_gui\gui.h"
#include "nv_gui\DebugDlg.h"


using namespace nv_sys;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDebugDlg dialog

namespace nv_gui
{
//DECLARE_NVOBJECT(CDebugDlg, CLSID_NVDebugConsole, "gui.debugconsole", "Debug Console");

CDebugDlg::CDebugDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDebugDlg::IDD, pParent),
	m_dwRefCount(1)
{
	//{{AFX_DATA_INIT(CDebugDlg)
	//}}AFX_DATA_INIT
}

CDebugDlg::~CDebugDlg()
{
}


void CDebugDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDebugDlg)
	DDX_Control(pDX, IDC_LISTBOX, m_ListBox);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDebugDlg, CDialog)
	//{{AFX_MSG_MAP(CDebugDlg)
	ON_WM_SIZE()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDebugDlg message handlers

void CDebugDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	if (IsWindow(m_ListBox.m_hWnd))
	{
		CRect rcDlg;
		GetClientRect(rcDlg);

		CRect rcLB;
		m_ListBox.GetClientRect(rcLB);
		
		ClientToScreen((LPPOINT)&rcDlg);
		m_ListBox.ClientToScreen((LPPOINT)&rcLB);

		int xoffset = rcLB.left - rcDlg.left;
		int yoffset = rcLB.top - rcDlg.top;

		m_ListBox.SetWindowPos(NULL, rcLB.left, rcLB.top, cx - (xoffset * 2), cy - (yoffset * 2), SWP_NOMOVE | SWP_NOZORDER);
	}	
}

bool INTCALLTYPE CDebugDlg::CreateNVObject(const nv_sys::INVCreator* pCreator, const NVGUID& InterfaceID, void** ppObj)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CDebugDlg* pDebugDlg = new CDebugDlg();
	if (!pDebugDlg)
		return false;

	if (pDebugDlg->GetInterface(InterfaceID, ppObj))
	{
		// Create the panel
		pDebugDlg->Create(IDD_DEBUGDLG, NULL);

		// Tell the app
	  	theApp.AddDialog(pDebugDlg);

		pDebugDlg->Release();
		return true;
	}

	delete pDebugDlg;
	return false;
}

// IDebugConsole
unsigned long CDebugDlg::AddRef()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	m_dwRefCount++;
	return m_dwRefCount;
}

unsigned long CDebugDlg::Release()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	unsigned long RefNew = --m_dwRefCount;

	if (RefNew == 0)
	{
		if (m_hWnd)
		{
			DestroyWindow();
		}
		theApp.RemoveDialog(this);
		delete this;
	}
		
	return RefNew;
}

bool CDebugDlg::GetInterface(const NVGUID& guid, void** ppObj)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (EqualNVGUID(guid, IID_INVObject))
	{
		*ppObj = static_cast<INVDebugConsole*>(this);
	}
	else if (EqualNVGUID(guid, IID_INVDebugConsole))
	{		
		*ppObj = static_cast<INVDebugConsole*>(this);
	}
	else
	{
		return false;
	}
		
	static_cast<INVDebugConsole*>(this)->AddRef();
	return true;
}

bool CDebugDlg::OutputDebug(const char* pszChar)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (IsWindow(m_ListBox.m_hWnd))
	{
		m_ListBox.AddString(pszChar);
		m_ListBox.SetTopIndex(m_ListBox.GetCount() - 1);
	}

	OutputDebugString(pszChar);
	OutputDebugString("\n");

	// If there's a debug log, put the string in it.
	if (m_dbgLog.is_open())
	{
		m_strStream << pszChar << std::endl << std::ends;
		m_dbgLog << m_strStream.str();
		m_dbgLog.flush();

		m_strStream.freeze(false);
		m_strStream.seekp(0);
	}

	return true;
}

bool CDebugDlg::SetVisible(bool bShow)
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

bool CDebugDlg::IsVisible()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (IsWindowVisible() == TRUE)
		return true;

	return false;
}

void CDebugDlg::OnClose() 
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// TODO: Add your message handler code here and/or call default
	ShowWindow(SW_HIDE);
}

bool CDebugDlg::SetLogFile(const char* pszLogFile)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (!pszLogFile)
		m_strLogFile = "";
	else
	{
		m_strLogFile = pszLogFile;
		m_dbgLog.open(pszLogFile, std::ios::out);	// Open a log file for debug messages
	}


	return true;
}

bool CDebugDlg::SetTitle(const char* pszTitle)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	SetWindowText(pszTitle);

	return true;
}

BOOL CDebugDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	

	// NOTE: This font is always present, and does not go out of scope.
	CFont *pFont = CFont::FromHandle((HFONT)::GetStockObject(SYSTEM_FIXED_FONT));
	m_ListBox.SetFont(pFont);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


}; // namespace nv_gui