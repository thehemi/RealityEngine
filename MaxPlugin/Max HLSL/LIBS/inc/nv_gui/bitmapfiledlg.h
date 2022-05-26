/*********************************************************************NVMH4****
Path:  NVSDK\Common\include\nv_gui
File:  bitmapfiledlg.h

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

#if !defined(AFX_BITMAPFILEDLG_H__D8B5FECD_5EAD_4FC8_A748_59A2337A34FB__INCLUDED_)
#define AFX_BITMAPFILEDLG_H__D8B5FECD_5EAD_4FC8_A748_59A2337A34FB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// bitmapfiledlg.h : header file
//
#include "bitmapctrl.h"

/////////////////////////////////////////////////////////////////////////////
// CBitmapFileDlg dialog

class CBitmapFileDlg : public CFileDialog
{
	DECLARE_DYNAMIC(CBitmapFileDlg)

public:
	CBitmapFileDlg(BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
		LPCTSTR lpszDefExt = NULL,
		LPCTSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		LPCTSTR lpszFilter = NULL,
		CWnd* pParentWnd = NULL,
        BOOL bPreview = FALSE);

    virtual BOOL    OnInitDialog();
    virtual int     DoModal();
    virtual void    OnFileNameChange();

protected:
    CBitmapCtrl     m_edit;
    BOOL            m_bPreview;

    void            SetBitmapFile(const char * filename);

	//{{AFX_MSG(CBitmapFileDlg)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BITMAPFILEDLG_H__D8B5FECD_5EAD_4FC8_A748_59A2337A34FB__INCLUDED_)
