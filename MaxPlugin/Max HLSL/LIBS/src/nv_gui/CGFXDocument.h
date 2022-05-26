/*********************************************************************NVMH4****
Path:  NVSDK\Common\src\nv_gui
File:  CGFXDocument.h

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

#if !defined(AFX_CGFXDOCUMENT_H__746AA399_1CDC_492D_9AEC_A603BB15F871__INCLUDED_)
#define AFX_CGFXDOCUMENT_H__746AA399_1CDC_492D_9AEC_A603BB15F871__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CGFXDocument.h : header file
//
#include "CCrystalTextBuffer.h"
/////////////////////////////////////////////////////////////////////////////
// CCGFXDocument document

class CCGFXDocument : public CDocument
{
protected:
	CCGFXDocument();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CCGFXDocument)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCGFXDocument)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
	virtual void DeleteContents();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CCGFXDocument();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

public:
    class CSampleTextBuffer : public CCrystalTextBuffer
    {
    private:
        CCGFXDocument *m_pOwnerDoc;
    public:
        CSampleTextBuffer(CCGFXDocument *pDoc) { m_pOwnerDoc = pDoc; };

        virtual void SetModified(BOOL bModified = TRUE)
            { m_pOwnerDoc->SetModifiedFlag(bModified); };
    };

    CSampleTextBuffer m_xTextBuffer;
	LOGFONT m_lf;

	// Generated message map functions
protected:
	//{{AFX_MSG(CCGFXDocument)
	afx_msg void OnReadOnly();
	afx_msg void OnUpdateReadOnly(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CGFXDOCUMENT_H__746AA399_1CDC_492D_9AEC_A603BB15F871__INCLUDED_)
