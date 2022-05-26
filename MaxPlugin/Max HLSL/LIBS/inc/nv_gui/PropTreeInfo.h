/*********************************************************************NVMH4****
Path:  NVSDK\Common\include\nv_gui
File:  PropTreeInfo.h

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

#if !defined(AFX_PROPTREEINFO_H__22BD9C18_A68C_4BB8_B7FC_C4A7DA0E1EBF__INCLUDED_)
#define AFX_PROPTREEINFO_H__22BD9C18_A68C_4BB8_B7FC_C4A7DA0E1EBF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PropTreeInfo.h : header file
//
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

class CPropTree;

/////////////////////////////////////////////////////////////////////////////
// CPropTreeInfo window

class CPropTreeInfo : public CStatic
{
// Construction
public:
	CPropTreeInfo();

// Attributes
public:
	// CPropTree class that this class belongs
	void SetPropOwner(CPropTree* pProp);

protected:
	CPropTree*		m_pProp;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPropTreeInfo)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPropTreeInfo();

	// Generated message map functions
protected:
	//{{AFX_MSG(CPropTreeInfo)
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROPTREEINFO_H__22BD9C18_A68C_4BB8_B7FC_C4A7DA0E1EBF__INCLUDED_)
