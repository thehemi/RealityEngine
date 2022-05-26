/*********************************************************************NVMH4****
Path:  NVSDK\Common\include\nv_gui
File:  bitmapctrl.h

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

#if !defined(AFX_BITMAPCTRL_H__B866456F_60CA_4046_83FE_046B42748DBA__INCLUDED_)
#define AFX_BITMAPCTRL_H__B866456F_60CA_4046_83FE_046B42748DBA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// bitmapctrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CBitmapCtrl window

class CBitmapCtrl : public CWnd
{
// Construction
public:
	CBitmapCtrl();

// Attributes
public:
    int         m_Width;
    int         m_Height;
    int         m_Depth;
    CString     m_Format;
    CString     m_Compression;
    BOOL        m_Cube;
    int         m_NumMipMaps;
    int         m_NumBitsPerPixels;
protected:
    CBitmap     m_Bitmap;
    CString     m_Filename;
// Operations
public:
    void        SetBitmapFile(const char * filename);
    void        LoadDefault();
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBitmapCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBitmapCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBitmapCtrl)
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BITMAPCTRL_H__B866456F_60CA_4046_83FE_046B42748DBA__INCLUDED_)
