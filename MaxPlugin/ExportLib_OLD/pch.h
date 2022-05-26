//-----------------------------------------------------------------------------
// File: Pch.h
//
// Desc: Header file to precompile
//
// Copyright (C) 1998-2000 Microsoft Corporation. All Rights Reserved.
//-----------------------------------------------------------------------------
#pragma once
#ifndef __PCH__H
#define __PCH__H

#include <initguid.h>
#include <string>
#include <vector>
#include <iostream>
using namespace std;

#include "Max.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"
#include <assert.h>
#include <io.h>
#include <fcntl.h>
#include <direct.h>
#include <commdlg.h>
#include "phyexp.h"
#include "d3dx9.h"
#include "dxfile.h"
#include "d3d9types.h"
#include "stdmat.h"
//#include "Max_Mem.h"
#include "exportxfile.h"
#include "xcustom.h"
#include <dxerr9.h>

extern HINSTANCE g_hInstance;

void Error(const char *fmt, ...);
#define DXASSERT(x) {HRESULT hr;if(FAILED(hr=(x))){ Error(" Error: %s, in: "###x,DXGetErrorString9(hr));}}

#endif