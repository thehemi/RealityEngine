/*********************************************************************NVMH4****
Path:  NVSDK\Common\src\nv_gui
File:  bitmapfiledlg.cpp

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

// bitmapfiledlg.cpp : implementation file
//

#include "stdafx.h"
#include "nv_gui\gui.h"
#include "nv_gui\bitmapfiledlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBitmapFileDlg

IMPLEMENT_DYNAMIC(CBitmapFileDlg, CFileDialog)

CBitmapFileDlg::CBitmapFileDlg(BOOL bOpenFileDialog, LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
		DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd, BOOL bPreview) :
CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd),
    m_bPreview(bPreview)

{
}


BEGIN_MESSAGE_MAP(CBitmapFileDlg, CFileDialog)
	//{{AFX_MSG_MAP(CBitmapFileDlg)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CBitmapFileDlg::OnInitDialog()
{
    if (m_ofn.Flags & OFN_EXPLORER && m_bPreview)
    {
        CWnd * pWnd = GetDlgItem(IDC_BITMAP_PREVIEW);
        CRect rect;
        pWnd->GetWindowRect(rect);
        ScreenToClient(rect);

        m_edit.Create(0,0,WS_CHILD|WS_VISIBLE,rect,this, 0);
        SetBitmapFile("<no file selected>");
    }
    return CFileDialog::OnInitDialog();
}

int CBitmapFileDlg::DoModal()
{
    if (m_ofn.Flags & OFN_EXPLORER && m_bPreview)
    {
        // Add the bitmap display control
        m_ofn.lpTemplateName = MAKEINTRESOURCE(IDD_BITMAP_PREVIEW_DLG);
        m_ofn.Flags |= OFN_ENABLETEMPLATE;
    }
    return CFileDialog::DoModal();
}

void CBitmapFileDlg::OnFileNameChange()
{
    if (m_bPreview)
        SetBitmapFile((LPCSTR)GetPathName());
}

void CBitmapFileDlg::SetBitmapFile(const char * filename)
{
    m_edit.SetWindowText(filename);
    m_edit.SetBitmapFile(filename);
    CString text;
    text.Format("%d",m_edit.m_Width);
    GetDlgItem(IDC_BITMAP_WIDTH)->SetWindowText(text);
    text.Format("%d",m_edit.m_Height);
    GetDlgItem(IDC_BITMAP_HEIGHT)->SetWindowText(text);
    text.Format("%d",m_edit.m_Depth);
    GetDlgItem(IDC_BITMAP_DEPTH)->SetWindowText(text);
    GetDlgItem(IDC_BITMAP_FORMAT)->SetWindowText(m_edit.m_Format);
    GetDlgItem(IDC_BITMAP_COMPRESSION)->SetWindowText(m_edit.m_Compression);
    text.Format("%s",m_edit.m_Cube ? "Yes" : "No");
    GetDlgItem(IDC_BITMAP_CUBE)->SetWindowText(text);
    text.Format("%d",m_edit.m_NumMipMaps);
    GetDlgItem(IDC_BITMAP_NUM_MIPMAPS)->SetWindowText(text);
    text.Format("%d",m_edit.m_NumBitsPerPixels);
    GetDlgItem(IDC_BITMAP_BITS_PER_PIXEL)->SetWindowText(text);
}

void CBitmapFileDlg::OnSize(UINT nType, int cx, int cy) 
{
	CFileDialog::OnSize(nType, cx, cy);
}
