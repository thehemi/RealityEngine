/*********************************************************************NVMH4****
Path:  NVSDK\Common\src\nv_gui
File:  PropTreeItemFileBrowse.cpp

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

// PropTreeItemFileBrowse.cpp : implementation file
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
#include "nv_gui\PropTreeItemFileBrowse.h"
#include "nv_gui\gui.h"
#include "nv_gui\bitmapfiledlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPropTreeItemFileBrowse

CPropTreeItemFileBrowse::CPropTreeItemFileBrowse(nv_gui::INVGUIEvent* pEventSink) :
	CPropTreeItem(pEventSink),
	m_sEdit(_T("")),
	m_nFormat(ValueFormatText),
	m_bPassword(FALSE),
    m_bInBrowseDialog(FALSE),
    m_bInEditDialog(FALSE),
    m_bEdit(FALSE),
	m_fValue(0.0f),
	m_bTruncateDisplay(FALSE),
	m_pszExtensions("All Files (*.*)|*.*||"),
    m_bPreview(FALSE)
{
}

CPropTreeItemFileBrowse::~CPropTreeItemFileBrowse()
{
    if (m_Bitmap.m_hObject)
        m_Bitmap.DeleteObject();
}


BEGIN_MESSAGE_MAP(CPropTreeItemFileBrowse, CWnd)
	//{{AFX_MSG_MAP(CPropTreeItemFileBrowse)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
    ON_WM_KILLFOCUS()
    ON_EN_KILLFOCUS(100, OnEditKillfocus)
    ON_WM_CREATE()
    ON_BN_CLICKED(101, OnBrowse)
    ON_BN_CLICKED(102, OnEdit)
    //}}AFX_MSG_MAP
//	ON_WM_DESTROY()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropTreeItemFileBrowse message handlers
void CPropTreeItemFileBrowse::SetTruncateDisplay(BOOL bTruncate)
{
	m_bTruncateDisplay = bTruncate;
}

void CPropTreeItemFileBrowse::SetPreview(BOOL bPreview)
{
    m_bPreview = bPreview;
}

void CPropTreeItemFileBrowse::SetExtensionString(const char* pszExtensions)
{
	m_pszExtensions = pszExtensions;
}

void CPropTreeItemFileBrowse::DrawAttribute(CDC* pDC, const RECT& rc)
{
	ASSERT(m_pProp!=NULL);
    CBitmap * pOldBitmap;
    CDC memdc;

	pDC->SelectObject(IsReadOnly() ? m_pProp->GetNormalFont() : m_pProp->GetBoldFont());
	pDC->SetTextColor(IsReadOnly() ? m_pProp->GetReadOnlyColor() : m_pProp->GetColor());
	pDC->SetBkMode(TRANSPARENT);

    CRect r = rc;
    if (m_bPreview)
        r.left += r.Height() + 3;

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
		if (strrchr(m_sEdit,'\\') != NULL)
		{
			pDC->DrawText(strrchr(m_sEdit, '\\') + 1, r, DT_SINGLELINE|DT_VCENTER);
		}
		else
		{
			pDC->DrawText(m_sEdit, r, DT_SINGLELINE|DT_VCENTER);
		}

	}
    
    if (m_bPreview && m_Bitmap.m_hObject)
    {
        memdc.CreateCompatibleDC( pDC );
	    
        pOldBitmap = memdc.SelectObject( &m_Bitmap );

        CSize size = m_Bitmap.GetBitmapDimension();

        r = rc;
        r.right = r.left + r.Height();
        pDC->StretchBlt(r.left,r.top,r.Width(), r.Height(), &memdc, 0, 0, size.cx, size.cy, SRCCOPY);

        memdc.SelectObject( pOldBitmap );
    }

    if (IsActivated())
    {
        r = rc;
        r.right -= 80;
        if (m_bPreview)
            r.left += r.Height() + 1;
        r.DeflateRect(0,1,1,1);
        DrawFocusRect(pDC->GetSafeHdc(),r);
    }
}



void CPropTreeItemFileBrowse::SetAsPassword(BOOL bPassword)
{
	m_bPassword = bPassword;
}


void CPropTreeItemFileBrowse::SetValueFormat(ValueFormat nFormat)
{
	m_nFormat = nFormat;
}

LPARAM CPropTreeItemFileBrowse::GetItemValue()
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


void CPropTreeItemFileBrowse::SetItemValue(LPARAM lParam)
{
	if (lParam==0L)
	{
		TRACE0("CPropTreeItemFileBrowse::SetItemValue - Invalid lParam value\n");
		return;
	}

	switch (m_nFormat)
	{
		case ValueFormatNumber:
			m_sEdit.Format(_T("%d"), lParam);
			return;

		case ValueFormatFloatPointer:
			{
				TCHAR tmp[MAX_PATH];
				m_fValue = *(float*)lParam;
				_stprintf(tmp, _T("%f"), m_fValue);
				m_sEdit = tmp;
			}
			return;
		default:
			break;
	}

	m_sEdit = (LPCTSTR)lParam;
    UpdateBitmapPreview();
}


void CPropTreeItemFileBrowse::OnMove()
{
	if (IsWindow(m_hWnd))
    {
        SetWindowPos(NULL, m_rc.left, m_rc.top, m_rc.Width(), m_rc.Height(), SWP_NOZORDER|SWP_NOACTIVATE);
        CRect rect;
        GetClientRect(rect);
        m_bBrowseButton.SetWindowPos(NULL,rect.right - 40, 0,40,rect.Height(),SWP_NOZORDER|SWP_NOACTIVATE);
        m_bEditButton.SetWindowPos(NULL,rect.right - 80, 0,40,rect.Height(),SWP_NOZORDER|SWP_NOACTIVATE);
        CRect r(m_rc);
        r.left = 0;
        r.right = m_rc.Width() - 80;
        r.top = 0;
        r.bottom = m_rc.Height();
        m_Edit.SetWindowPos(NULL,0, 0, r.right, r.bottom,SWP_NOZORDER|SWP_NOACTIVATE);
    }
		
}

void CPropTreeItemFileBrowse::LoadDefault() 
{
	if (m_Bitmap.m_hObject)
		m_Bitmap.DeleteObject();
  	VERIFY (m_Bitmap.Attach (::LoadImage (::AfxFindResourceHandle(
	MAKEINTRESOURCE (IDB_DEFAULT_SMALL), RT_BITMAP),
	MAKEINTRESOURCE (IDB_DEFAULT_SMALL), IMAGE_BITMAP, 0, 0,
	(LR_DEFAULTSIZE| LR_CREATEDIBSECTION))));
    m_Bitmap.SetBitmapDimension(20,20);
}

void CPropTreeItemFileBrowse::UpdateBitmapPreview()
{
	ilInit();
   	ilGetError(); // temporary till il init bug is fixed.
	
    // Generate the main image name to use.
	unsigned int imgId;	
	ilGenImages(1, &imgId);
	
    // Bind this image name.
	ilBindImage(imgId);

    // Set origin location:
	ilEnable(IL_ORIGIN_SET);
    ilSetInteger(IL_KEEP_DXTC_DATA,IL_TRUE);
	ilOriginFunc(IL_ORIGIN_UPPER_LEFT);

	// Loads the image specified by File into the ImgId image.
	char* const pName = (char* const)(LPCSTR)m_sEdit;
	if(pName)
	{
		ilLoadImage((char* const)(LPCSTR)m_sEdit);
		if (ilGetError() != IL_NO_ERROR)
		{
			LoadDefault();
			return;
		}
	}
	else
	{
		LoadDefault();
		return;
	}

	// Check if its a cube map:
    if (ilGetInteger(IL_NUM_IMAGES) < 0)
    {
        LoadDefault();
        return;
    }

	ilBindImage(imgId);
    ilConvertImage(IL_BGRA,IL_UNSIGNED_BYTE);
    iluImageParameter(ILU_FILTER,ILU_BILINEAR);
    int resize = m_rc.Height() >= 16 ? m_rc.Height() : 16;
    iluScale(resize,resize,1);
		
    if (ilGetError() != IL_NO_ERROR)
    {
        LoadDefault();
        return;
    }

	void * pData = ilGetData();

	if (m_Bitmap.m_hObject)
		m_Bitmap.DeleteObject();

    CClientDC dc(CWnd::FromHandle(NULL));
    BITMAPINFO bmInfo;
    memset(&bmInfo,0,sizeof(BITMAPINFO));
    bmInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmInfo.bmiHeader.biWidth = ilGetInteger(IL_IMAGE_WIDTH);
    bmInfo.bmiHeader.biHeight = ilGetInteger(IL_IMAGE_HEIGHT);
    bmInfo.bmiHeader.biPlanes = 1;
    bmInfo.bmiHeader.biBitCount = 32;
    bmInfo.bmiHeader.biCompression = BI_RGB;
    bmInfo.bmiHeader.biSizeImage = ((((bmInfo.bmiHeader.biWidth * bmInfo.bmiHeader.biBitCount) + 31) & ~31) >> 3) * bmInfo.bmiHeader.biHeight;

    HBITMAP hBitmap = ::CreateDIBitmap(dc.GetSafeHdc(),&bmInfo.bmiHeader,CBM_INIT,pData,&bmInfo,DIB_RGB_COLORS);
    m_Bitmap.Attach(hBitmap);
    m_Bitmap.SetBitmapDimension(bmInfo.bmiHeader.biWidth,bmInfo.bmiHeader.biHeight);

	// Reset origin location:
	ilDisable(IL_ORIGIN_SET);

    ilDeleteImages(1,&imgId);
}

void CPropTreeItemFileBrowse::OnRefresh()
{
	if (IsWindow(m_hWnd) && m_bEdit==FALSE)
		SetWindowText(m_sEdit);
}

void CPropTreeItemFileBrowse::OnDontCommit()
{
   	ShowWindow(SW_HIDE);
}

void CPropTreeItemFileBrowse::OnCommit()
{
	// hide edit control
    m_Edit.SetWindowText(m_sEdit);
    m_Edit.ShowWindow(SW_HIDE);
    m_bEdit = FALSE;
   	ShowWindow(SW_HIDE);
	// store edit text for GetItemValue
	GetWindowText(m_sEdit);
    if (m_bPreview)
        UpdateBitmapPreview();
    Invalidate(TRUE);
}


void CPropTreeItemFileBrowse::OnActivate()
{
	// Check if the edit control needs creation
    CRect r(m_rc);
    r.left = 0;
    r.right = m_rc.Width() - 80;
    r.top = 0;
    r.bottom = m_rc.Height();
	if (!IsWindow(m_hWnd))
	{
        LPCTSTR pszClassName;

		pszClassName = AfxRegisterWndClass(CS_VREDRAW|CS_HREDRAW, LoadCursor(NULL, IDC_ARROW), 0);
		
		DWORD dwStyle = WS_CHILD|WS_VISIBLE;

		CreateEx(0, pszClassName, _T(""), dwStyle, m_rc, m_pProp->GetCtrlParent(), 0);

		SendMessage(WM_SETFONT, (WPARAM)m_pProp->GetNormalFont()->m_hObject);
        
        dwStyle = WS_CHILD|ES_AUTOHSCROLL;
        m_Edit.Create(dwStyle, r, this, 100);
        m_Edit.SendMessage(WM_SETFONT, (WPARAM)m_pProp->GetNormalFont()->m_hObject);
        
        CRect rect;
        GetClientRect(rect);
        rect.left = rect.right - 40;
        m_bBrowseButton.Create("File...",WS_VISIBLE|WS_CHILD|WS_TABSTOP|BS_PUSHBUTTON,rect, this, 101);
        m_bBrowseButton.SendMessage(WM_SETFONT, (WPARAM)m_pProp->GetNormalFont()->m_hObject);
        
        rect.right = rect.left;
        rect.left = rect.right - 40;
        m_bEditButton.Create("Edit...",WS_VISIBLE|WS_CHILD|WS_TABSTOP|BS_PUSHBUTTON,rect, this, 102);
        m_bEditButton.SendMessage(WM_SETFONT, (WPARAM)m_pProp->GetNormalFont()->m_hObject);
    }
    CRect rect;
    GetClientRect(rect);
    SetWindowPos(NULL, m_rc.left, m_rc.top, m_rc.Width(), m_rc.Height(), SWP_NOZORDER|SWP_SHOWWINDOW);

    m_bBrowseButton.ModifyStyle(WS_DISABLED,0);
    m_bEditButton.ModifyStyle(WS_DISABLED,0);

    m_bBrowseButton.ShowWindow(SW_SHOW);
    m_bBrowseButton.SetWindowPos(NULL,rect.right - 40, 0,40,rect.Height(),SWP_NOZORDER|SWP_NOACTIVATE);
    m_bEditButton.ShowWindow(SW_SHOW);
    m_bEditButton.SetWindowPos(NULL,rect.right - 80, 0,40,rect.Height(),SWP_NOZORDER|SWP_NOACTIVATE);

    m_Edit.SetPasswordChar((TCHAR)(m_bPassword ? '*' : 0));
	m_Edit.SetWindowText(m_sEdit);
	m_Edit.SetSel(0, -1);
    m_Edit.SetWindowPos(NULL,0, 0, r.right, r.bottom,SWP_NOZORDER|SWP_NOACTIVATE);

    SetFocus();
	if (m_pEventSink)
		m_pEventSink->OnSetFocus();

}

void CPropTreeItemFileBrowse::OnMouseMove(UINT, CPoint point) 
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

void CPropTreeItemFileBrowse::OnLButtonDown(UINT nFlags, CPoint point) 
{
    CRect rect;
    m_Edit.GetWindowRect(rect);
    ScreenToClient(rect);
    if (rect.PtInRect(point) && IsActivated())
    {
        m_bBrowseButton.ModifyStyle(0,WS_DISABLED);
        m_bEditButton.ModifyStyle(0,WS_DISABLED);
        m_bBrowseButton.RedrawWindow();
        m_bEditButton.RedrawWindow();

        m_Edit.ShowWindow(SW_SHOW);
        m_Edit.SetFocus();
        m_bEdit = TRUE;
    }
}

void CPropTreeItemFileBrowse::OnEditKillfocus()
{
 	if (!m_bInBrowseDialog && !m_bInEditDialog )
    {
        m_Edit.GetWindowText(m_sEdit);
        SetWindowText(m_sEdit);
        CommitChanges();
		if (m_pEventSink)
			m_pEventSink->OnKillFocus();
    }
}

BOOL CPropTreeItemFileBrowse::GetPath(const CString & sFullPathName, CString & sPath)
{
    int idx = sFullPathName.ReverseFind('\\');
    if (idx < 0)
        idx = sFullPathName.ReverseFind('/');
    if (idx < 0)
        return FALSE;

    sPath = sFullPathName;
    sPath.SetAt(idx,'\0');
    return TRUE;
}

void CPropTreeItemFileBrowse::OnBrowse()
{
    m_bInBrowseDialog = TRUE;
	ASSERT(m_pProp!=NULL);
	m_pProp->DisableInput();

    CBitmapFileDlg dlg(TRUE,NULL,NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,(LPCSTR)m_pszExtensions,AfxGetMainWnd(),m_bPreview);

    CString defExt(m_pszExtensions);
    int idx = m_pszExtensions.Find('|');
    if (idx >= 0)
    {
        defExt.Delete(0,idx+1);
        idx = defExt.Find('.');
        defExt.Delete(0,idx+1);
        idx = defExt.Find('|');
        defExt.SetAt(idx,'\0');
        const char * path = ((nv_gui::CNVGUIApp*)AfxGetApp())->GetInitialPath((LPCSTR)defExt);
        if (path)
            dlg.m_ofn.lpstrInitialDir = path;
    }

    if (dlg.DoModal() == IDOK)
    {
        m_sEdit = dlg.GetPathName();
        GetPath(m_sEdit,m_sInitialDir);
        ((nv_gui::CNVGUIApp*)AfxGetApp())->SetInitialPath(dlg.GetFileExt(),(LPCSTR)m_sInitialDir);
        m_Edit.SetWindowText(m_sEdit);
        SetWindowText(m_sEdit);
		CommitChanges();
    }
    else
    {
        DontCommitChanges();
    }
    
    m_pProp->DisableInput(FALSE);
    m_bInBrowseDialog = FALSE;
}

void CPropTreeItemFileBrowse::OnEdit()
{
    m_bInEditDialog = TRUE;
	DontCommitChanges();
    
    m_pProp->DisableInput(FALSE);
  	m_pProp->SendNotify(PTN_FILEEDIT, this);
    m_bInEditDialog = FALSE;
}


int CPropTreeItemFileBrowse::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	return 0;
}

void CPropTreeItemFileBrowse::OnKillFocus( CWnd* pWnd)
{
    if (pWnd != &m_Edit && 
        pWnd != &m_bBrowseButton &&
        pWnd != &m_bEditButton )
    {
        OnRefresh();
        DontCommitChanges();
    }
            
}

//"CgFX (*.fx)\0*.fx\0All Files (*.*)\0;*.*\0"