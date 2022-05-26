// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__8329AAF5_825A_4808_BFD1_42598279F902__INCLUDED_)
#define AFX_STDAFX_H__8329AAF5_825A_4808_BFD1_42598279F902__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#pragma warning (disable : 4786)

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxole.h>         // MFC OLE classes
#include <afxodlgs.h>       // MFC OLE dialog classes
#include <afxdisp.h>        // MFC Automation classes
#endif // _AFX_NO_OLE_SUPPORT


#ifndef _AFX_NO_DB_SUPPORT
#include <afxdb.h>			// MFC ODBC database classes
#endif // _AFX_NO_DB_SUPPORT

#ifndef _AFX_NO_DAO_SUPPORT
#include <afxdao.h>			// MFC DAO database classes
#endif // _AFX_NO_DAO_SUPPORT

#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxtempl.h>
#include <afxpriv.h>
#include <string>
#include <vector>
#include <map>
#include <list>
#include <set>

typedef std::vector<float> tvecFloats;

#include <shared\nv_common.h>
#include <nv_gui\nv_gui.h>
#include <nv_gui\nvguicommon.h>
#include <nv_math\nv_math.h>
#include <il/il.h>
#include <il/ilu.h>
#include <nv_gui\PropTree.h>
#include <nv_gui\dlgbars.h>
#include "resource.h"



//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__8329AAF5_825A_4808_BFD1_42598279F902__INCLUDED_)
