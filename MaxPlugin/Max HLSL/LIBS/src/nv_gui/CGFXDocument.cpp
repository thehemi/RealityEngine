/*********************************************************************NVMH4****
Path:  NVSDK\Common\src\nv_gui
File:  CGFXDocument.cpp

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

// CGFXDocument.cpp : implementation file
//

#include "stdafx.h"
#include "nv_gui\gui.h"
#include "CGFXDocument.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCGFXDocument

IMPLEMENT_DYNCREATE(CCGFXDocument, CDocument)

CCGFXDocument::CCGFXDocument() : m_xTextBuffer(this)
{
	//	Initialize LOGFONT structure
	memset(&m_lf, 0, sizeof(m_lf));
	m_lf.lfWeight = FW_NORMAL;
	m_lf.lfCharSet = ANSI_CHARSET;
	m_lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	m_lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	m_lf.lfQuality = DEFAULT_QUALITY;
	m_lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	strcpy(m_lf.lfFaceName, "Courier");
}

CCGFXDocument::~CCGFXDocument()
{
}


BEGIN_MESSAGE_MAP(CCGFXDocument, CDocument)
	//{{AFX_MSG_MAP(CCGFXDocument)
	ON_COMMAND(ID_READ_ONLY, OnReadOnly)
	ON_UPDATE_COMMAND_UI(ID_READ_ONLY, OnUpdateReadOnly)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCGFXDocument diagnostics

#ifdef _DEBUG
void CCGFXDocument::AssertValid() const
{
	CDocument::AssertValid();
}

void CCGFXDocument::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CCGFXDocument serialization

BOOL CCGFXDocument::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

    m_xTextBuffer.InitNew();
	return TRUE;
}

void CCGFXDocument::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CCGFXDocument commands

void CCGFXDocument::DeleteContents() 
{
	CDocument::DeleteContents();
    m_xTextBuffer.FreeAll();
}

BOOL CCGFXDocument::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;
	
	return m_xTextBuffer.LoadFromFile(lpszPathName);
}

BOOL CCGFXDocument::OnSaveDocument(LPCTSTR lpszPathName) 
{
	return m_xTextBuffer.SaveToFile(lpszPathName);
}

void CCGFXDocument::OnReadOnly() 
{
	if (! m_xTextBuffer.GetReadOnly())
	{
		m_xTextBuffer.SetReadOnly(TRUE);
		AfxMessageBox("Document now read-only!");
	}
	else
		m_xTextBuffer.SetReadOnly(FALSE);
}

void CCGFXDocument::OnUpdateReadOnly(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_xTextBuffer.GetReadOnly());
}
