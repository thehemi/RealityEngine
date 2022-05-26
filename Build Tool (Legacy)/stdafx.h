// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <stdio.h>

void Error(const char *fmt, ...);
void Warning(const char *fmt, ...);
void LogPrintf(const char *fmt, ...);
void SetProgress(float percent);

#include <tchar.h>
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <windows.h>
#include <wchar.h>
#include <mmsystem.h>
#include <commctrl.h> // for InitCommonControls() 

#include <d3d9.h>
#include <d3dx9.h>
#include <dxerr9.h>
#include <d3dx9.h>
#include <dxfile.h>
#include "..\shared\Shared.h"
#include "..\MaxPlugin\ExportLib\xcustom.h"
#include "structs.h"
#include "Helpers.h"
using namespace std;

#include "PRTMesh.h"
#include "PRTSimulator.h"

#include "XImport.h"
#include "Compiler.h"
#include "Exporter.h"
#include "BuildTool.h"
#include "NVMeshMender\NVMeshMender.h" // For AddTangentData
#include "NVTriStrip\NvTriStrip.h"

#define LOG_NAME    "BuildTool.log"
#define CONFIG_NAME "BuildTool.ini"



//Vector AsD3DPoint(Vector& p);
//void ConvertMaxMatrix(Matrix& MaxMatrix);
