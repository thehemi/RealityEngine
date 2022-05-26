/*********************************************************************NVMH4****
Path:  NVSDK\Common\src\nv_gui
File:  notifysliderctrl.cpp

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

// notifysliderctrl.cpp : implementation file
//

#include "stdafx.h"
#include "nv_gui\notifysliderctrl.h"
#include "nv_gui\PropTreeItemEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNotifySliderCtrl

CNotifySliderCtrl::CNotifySliderCtrl()
{
}

CNotifySliderCtrl::~CNotifySliderCtrl()
{
}


BEGIN_MESSAGE_MAP(CNotifySliderCtrl, CSliderCtrl)
	//{{AFX_MSG_MAP(CNotifySliderCtrl)
	ON_WM_KILLFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNotifySliderCtrl message handlers

void CNotifySliderCtrl::OnKillFocus(CWnd* pNewWnd) 
{
	CSliderCtrl::OnKillFocus(pNewWnd);
	
	((CPropTreeItemEdit*)GetParent())->OnSliderKillfocus();
	
}
