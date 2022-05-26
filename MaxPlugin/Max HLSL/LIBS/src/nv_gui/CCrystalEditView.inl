/*********************************************************************NVMH4****
Path:  NVSDK\Common\src\nv_gui
File:  CCrystalEditView.inl

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

////////////////////////////////////////////////////////////////////////////
//	File:		CCrystalEditView.inl
//	Version:	1.0.0.0
//	Created:	29-Dec-1998
//
//	Author:		Stcherbatchenko Andrei
//	E-mail:		windfall@gmx.de
//
//	Inline functions of Crystal Edit classes
//
//	You are free to use or modify this code to the following restrictions:
//	- Acknowledge me somewhere in your about box, simple "Parts of code by.."
//	will be enough. If you can't (or don't want to), contact me personally.
//	- LEAVE THIS HEADER INTACT
////////////////////////////////////////////////////////////////////////////

#ifndef __CCrystalEditView_INL_INCLUDED
#define __CCrystalEditView_INL_INCLUDED

#include "CCrystalEditView.h"

CE_INLINE BOOL CCrystalEditView::GetOverwriteMode() const
{
	return m_bOvrMode;
}

CE_INLINE void CCrystalEditView::SetOverwriteMode(BOOL bOvrMode /*= TRUE*/)
{
	m_bOvrMode = bOvrMode;
}

CE_INLINE BOOL CCrystalEditView::GetDisableBSAtSOL() const
{
	return m_bDisableBSAtSOL;
}

CE_INLINE BOOL CCrystalEditView::GetAutoIndent() const
{
	return m_bAutoIndent;
}

CE_INLINE void CCrystalEditView::SetAutoIndent(BOOL bAutoIndent)
{
	m_bAutoIndent = bAutoIndent;
}

#endif
