/*********************************************************************NVMH4****
Path:  NVSDK\Common\include\nv_gui
File:  dlgbars.h

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

/*****************************************************************************
  DLGBARS.H

  Purpose: 
  	Interface for CDlgToolBar, a special type of CToolBar which does not
  	expect a parent frame window to be available, and CDlgStatusBar, which
  	does the same for CStatusBars.  This allows the control bars
  	to be used in applications where the main window is a dialog bar.

  Functions:
    CDlgToolBar::CDlgToolBar()          -- constructor
    CDlgToolBar::~CDlgToolBar()         -- destructor
    CDlgToolBar::OnIdleUpdateCmdUI()    -- WM_IDLEUPDATECMDUI handler
    
    CDlgStatusBar::CDlgStatusBar()      -- constructor
    CDlgStatusBar::~CDlgStatusBar()     -- destructor
    CDlgStatusBar::OnIdleUpdateCmdUI()	-- WM_IDLEUPDATECMDUI handler

  Development Team:
	Mary Kirtland
  Ported to 32-bit by:
    Mike Hedley	
  Written by Microsoft Product Support Services, Premier ISV Support
  Copyright (c) 1996 Microsoft Corporation. All rights reserved.
\****************************************************************************/

#ifndef __DLGBARS_H__
	#define __DLGBARS_H__

class CDlgToolBar : public CToolBar 
{   
// Construction
public:
	CDlgToolBar();
   
// Implementation
public:   
	virtual ~CDlgToolBar();

protected:                
	// Generated message map functions
	//{{AFX_MSG(CDlgToolBar)
	afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM wParam, LPARAM);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};        

class CDlgStatusBar : public CStatusBar 
{   
// Construction
public:
	CDlgStatusBar();
   
// Implementation
public:   
	virtual ~CDlgStatusBar();

protected:                
	// Generated message map functions
	//{{AFX_MSG(CDlgStatusBar)
	afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM wParam, LPARAM);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};



#endif //__DLGBARS_H__



