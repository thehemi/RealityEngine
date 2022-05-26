//--------------------------------------------------------------------------------------
/// File: DxStdAfx.h
//
/// Desc: Header file that is the standard includes for the DirectX SDK samples
//
/// Copyright (c) Artificial Studios. All rights reserved.
//--------------------------------------------------------------------------------------
#pragma once
#ifndef DXSDK_STDAFX_H
#define DXSDK_STDAFX_H

#ifdef ENGINE_EXPORTS
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif

#ifdef _BUILDTOOL
#define ENGINE_API
#endif

#define VC_EXTRALEAN		/// Exclude rarely-used stuff from Windows headers
#ifndef STRICT
#define STRICT
#endif

/// Works with Windows 2000 and later and Windows 98 or later
#undef _WIN32_IE
#undef WINVER
#undef _WIN32_WINDOWS
#undef _WIN32_WINNT
#define WINVER         0x0500 
#define _WIN32_WINDOWS 0x0410 
#define _WIN32_WINNT   0x0500 

#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <assert.h>
#include <wchar.h>
//#include <mmsystem.h>
#include <commctrl.h> /// for InitCommonControls() 
#include <shellapi.h> /// for ExtractIcon()
#include <new.h>      /// for placement new
#include <math.h>      
#include <limits.h>      


/// Enable extra D3D debugging in debug builds if using the debug DirectX runtime.  
/// This makes D3D objects work well in the debugger watch window, but slows down 
/// performance slightly.
#if defined(DEBUG) | defined(_DEBUG)
#define D3D_DEBUG_INFO
#endif

/// Direct3D includes
#include <d3d9.h>
#include <d3dx9.h>
#include <dxerr9.h>

/// DirectSound includes
//#include <mmsystem.h>
//#include <mmreg.h>
//#include <dsound.h>

#include "DXUT.h"
#include "DXUTmisc.h"
#include "DXUTenum.h"
#include "DXUTmesh.h"
#include "DXUTgui.h"
#include "DXUTsettingsDlg.h"
//#include "DXUTSound.h"

#if defined(DEBUG) | defined(_DEBUG)
    #define V(x)           { hr = x; if( FAILED(hr) ) { DXUTTrace( __FILE__, (DWORD)__LINE__, hr, L#x, true ); } }
    #define V_RETURN(x)    { hr = x; if( FAILED(hr) ) { return DXUTTrace( __FILE__, (DWORD)__LINE__, hr, L#x, true ); } }
#else
    #define V(x)           { hr = x; }
    #define V_RETURN(x)    { hr = x; if( FAILED(hr) ) { return hr; } }
#endif

#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }

#endif /// !defined(DXSDK_STDAFX_H)
