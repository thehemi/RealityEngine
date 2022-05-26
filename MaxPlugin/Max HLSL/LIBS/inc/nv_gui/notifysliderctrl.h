/*********************************************************************NVMH4****
Path:  NVSDK\Common\include\nv_gui
File:  notifysliderctrl.h

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

#if !defined(AFX_NOTIFYSLIDERCTRL_H__4C37832F_4738_49F4_BB73_6CD1CE912D81__INCLUDED_)
#define AFX_NOTIFYSLIDERCTRL_H__4C37832F_4738_49F4_BB73_6CD1CE912D81__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// notifysliderctrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNotifySliderCtrl window

class CNotifySliderCtrl : public CSliderCtrl
{
// Construction
public:
	CNotifySliderCtrl();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNotifySliderCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CNotifySliderCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CNotifySliderCtrl)
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NOTIFYSLIDERCTRL_H__4C37832F_4738_49F4_BB73_6CD1CE912D81__INCLUDED_)
