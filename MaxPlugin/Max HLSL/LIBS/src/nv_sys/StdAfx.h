// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__3D66C79B_E3FB_4267_A588_81E4E7719237__INCLUDED_)
#define AFX_STDAFX_H__3D66C79B_E3FB_4267_A588_81E4E7719237__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#pragma warning(disable:4244)
#pragma warning(disable:4018)
#pragma warning(disable:4927)
#pragma warning(disable:4101)

#include "nv_sys.h"


// Private interfaces
#include "inveffecttemplateparameter.h"

// Implementation classes
#include "nvsystem.h"
#include "nvcreatorarray.h"
#include "nvconnectionparameter.h"
#include "nvconnectionmanager.h"
#include "nvcgfxtype.h"
#include "nvinterpolator.h"
#include "nveffecttemplateparameter.h"
#include "nvparameterlist.h"

// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__3D66C79B_E3FB_4267_A588_81E4E7719237__INCLUDED_)
