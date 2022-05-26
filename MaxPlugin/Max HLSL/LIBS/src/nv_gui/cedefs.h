/*********************************************************************NVMH4****
Path:  NVSDK\Common\src\nv_gui
File:  cedefs.h

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
//	File:		cedefs.h
//	Version:	1.0.0.0
//	Created:	21-Feb-1999
//
//	Author:		Stcherbatchenko Andrei
//	E-mail:		windfall@gmx.de
//
//	Global definitions for Crystal Edit - syntax coloring text editor.
//
//	You are free to use or modify this code to the following restrictions:
//	- Acknowledge me somewhere in your about box, simple "Parts of code by.."
//	will be enough. If you can't (or don't want to), contact me personally.
//	- LEAVE THIS HEADER INTACT
////////////////////////////////////////////////////////////////////////////

#ifndef CEDEFS_H__INCLUDED
#define CEDEFS_H__INCLUDED

#if defined(CE_DLL_BUILD) && defined(CE_FROM_DLL)
#error Define CE_DLL_BUILD or CE_FROM_DLL, but not both
#endif

#ifndef CRYSEDIT_CLASS_DECL
	//#if defined(CE_DLL_BUILD)
	//	#define CRYSEDIT_CLASS_DECL		__declspec(dllexport)
	//#elif defined(CE_FROM_DLL)
	//	#define CRYSEDIT_CLASS_DECL		__declspec(dllimport)
	//#else
// TIM: No DLLs
		#define CRYSEDIT_CLASS_DECL
	//#endif
#endif

#ifndef CE_INLINE
	//#if defined(CE_DLL_BUILD) || defined(CE_FROM_DLL)
	//	#define CE_INLINE
	//#else
		#define CE_INLINE inline
	//#endif
#endif

#endif
