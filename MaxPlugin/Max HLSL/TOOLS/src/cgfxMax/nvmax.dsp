# Microsoft Developer Studio Project File - Name="nvmax" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=nvmax - Win32 Debug Multithreaded DLL
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "nvmax.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "nvmax.mak" CFG="nvmax - Win32 Debug Multithreaded DLL"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "nvmax - Win32 Hybrid" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "nvmax - Win32 Release Multithreaded DLL" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "nvmax - Win32 Debug Multithreaded DLL" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "nvmax - Win32 Release Multithreaded DLL with PDB" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "nvmax"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "nvmax - Win32 Hybrid"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Hybrid"
# PROP BASE Intermediate_Dir "Hybrid"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Hybrid"
# PROP Intermediate_Dir "Hybrid"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NVMAX_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MD /W3 /Gm /GX /ZI /Od /I "..\..\..\LIBS\inc" /I "$(CG_INC_PATH)" /I "..\..\inc" /I "..\..\..\libs\inc\shared" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NVMAX_EXPORTS" /D "DEVIL_MTDLL" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib d3dx8.lib winmm.lib vfw32.lib bmm.lib core.lib geom.lib gfx.lib mesh.lib maxutil.lib maxscrpt.lib manipsys.lib paramblk2.lib ole32.lib dxerr8.lib comctl32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 nv_math_hybrid.lib d3dx9.lib dxerr9.lib nv_plugin_mtdll.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib winmm.lib vfw32.lib bmm.lib core.lib geom.lib gfx.lib mesh.lib maxutil.lib maxscrpt.lib manipsys.lib paramblk2.lib ole32.lib comctl32.lib /nologo /dll /debug /machine:I386 /out:"..\..\bin\hybrid\cgfxMax\plugins\nvcgfx.dlm" /pdbtype:sept /libpath:"..\..\..\LIBS\lib\debug" /libpath:"$(CG_LIB_PATH)" /libpath:"../../../LIBS/implib/debug"
# SUBTRACT LINK32 /verbose
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy                                 ..\..\..\libs\dll\debug\nv_gui_mtdll.dll                              ..\..\bin\hybrid\cgfxMax\plugins                             	copy                          ..\..\..\libs\dll\debug\nv_renderdevice_mtdll.dll                          ..\..\bin\hybrid\cgfxMax\plugins                         	copy                          ..\..\..\libs\dll\debug\nv_renderdevice9_mtdll.dll                          ..\..\bin\hybrid\cgfxMax\plugins                         	copy                          ..\..\..\libs\dll\debug\nv_fx_mtdll.dll                          ..\..\bin\hybrid\cgfxMax\plugins                         	copy             ..\..\..\libs\dll\debug\nv_sys_mtdll.dll             ..\..\bin\hybrid\cgfxMax            	copy                              default.fx                              ..\..\bin\hybrid\cgfxMax\plugins                             	copy                      nvplugins.inf                      ..\..\bin\hybrid\cgfxMax\plugins                     	copy                                 ..\..\..\libs\dll\debug\Devil_mtdll.dll                              ..\..\bin\hybrid\cgfxMax                             	copy \
                                 ..\..\..\libs\dll\debug\Devilu_mtdll.dll                                ..\..\bin\hybrid\cgfxMax                              	copy                                ..\..\bin\hybrid\cgfxMax\plugins\*.*                                $(MAXPATH)\plugins\                             	copy                                ..\..\bin\hybrid\cgfxMax\*.*                               $(MAXPATH)\ 
# End Special Build Tool

!ELSEIF  "$(CFG)" == "nvmax - Win32 Release Multithreaded DLL"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "nvmax___Win32_Release_Multithreaded_DLL"
# PROP BASE Intermediate_Dir "nvmax___Win32_Release_Multithreaded_DLL"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Win32_Release_Multithreaded_DLL"
# PROP Intermediate_Dir "Win32_Release_Multithreaded_DLL"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "..\..\..\nvsdk\common\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NVMAX_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\..\LIBS\inc" /I "$(CG_INC_PATH)" /I "..\..\inc" /I "..\..\..\libs\inc\shared" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NVMAX_EXPORTS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib d3dx8.lib winmm.lib vfw32.lib bmm.lib core.lib geom.lib gfx.lib mesh.lib maxutil.lib maxscrpt.lib manipsys.lib paramblk2.lib ole32.lib dxerr8.lib comctl32.lib nv_math_mtdll.lib opengl32.lib glu32.lib /nologo /dll /machine:I386 /out:"d:\3dsmax5\plugins\nvcgfx.dlm" /libpath:"..\..\..\nvsdk\common\lib\debug"
# ADD LINK32 nv_math_mtdll.lib d3dx9.lib dxerr9.lib nv_plugin_mtdll.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib winmm.lib vfw32.lib bmm.lib core.lib geom.lib gfx.lib mesh.lib maxutil.lib maxscrpt.lib manipsys.lib paramblk2.lib ole32.lib comctl32.lib /nologo /dll /machine:I386 /out:"..\..\bin\release\cgfxMax\plugins\nvcgfx.dlm" /libpath:"..\..\..\LIBS\lib\release" /libpath:"$(CG_LIB_PATH)" /libpath:"../../../LIBS/implib/release"
# SUBTRACT LINK32 /debug
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy                                 ..\..\..\libs\dll\release\nv_gui_mtdll.dll                              ..\..\bin\release\cgfxMax\plugins                             	copy                          ..\..\..\libs\dll\release\nv_fx_mtdll.dll                          ..\..\bin\release\cgfxMax\plugins                         	copy                          ..\..\..\libs\dll\release\nv_renderdevice_mtdll.dll                          ..\..\bin\release\cgfxMax\plugins                         	copy                          ..\..\..\libs\dll\release\nv_renderdevice9_mtdll.dll                          ..\..\bin\release\cgfxMax\plugins                         	copy             ..\..\..\libs\dll\release\nv_sys_mtdll.dll             ..\..\bin\release\cgfxMax            	copy                              default.fx                              ..\..\bin\release\cgfxMax\plugins                             	copy                      nvplugins.inf                      ..\..\bin\release\cgfxMax\plugins                     	copy                                 ..\..\..\libs\dll\release\Devil_mtdll.dll                              ..\..\bin\release\cgfxMax                             	copy \
                                  ..\..\..\libs\dll\release\Devilu_mtdll.dll                                ..\..\bin\release\cgfxMax                              	copy                                ..\..\bin\release\cgfxMax\plugins\*.*                               $(MAXPATH)\plugins\                              	copy                                ..\..\bin\release\cgfxMax\*.*                              $(MAXPATH)\ 
# End Special Build Tool

!ELSEIF  "$(CFG)" == "nvmax - Win32 Debug Multithreaded DLL"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "nvmax___Win32_Debug_Multithreaded_DLL"
# PROP BASE Intermediate_Dir "nvmax___Win32_Debug_Multithreaded_DLL"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Win32_Debug_Multithreaded_DLL"
# PROP Intermediate_Dir "Win32_Debug_Multithreaded_DLL"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /Gm /GX /ZI /Od /I "..\..\..\nvsdk\common\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NVMAX_EXPORTS" /FR /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\..\LIBS\inc" /I "$(CG_INC_PATH)" /I "..\..\inc" /I "..\..\..\libs\inc\shared" /D "WIN32" /D "_DEBUG" /D "_DEBUGMAX" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NVMAX_EXPORTS" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib d3dx8.lib winmm.lib vfw32.lib bmm.lib core.lib geom.lib gfx.lib mesh.lib maxutil.lib maxscrpt.lib manipsys.lib paramblk2.lib ole32.lib dxerr8.lib comctl32.lib nv_math_hybrid.lib opengl32.lib glu32.lib nvparse_hybrid.lib /nologo /dll /debug /machine:I386 /out:"d:\3dsmax5\plugins\nvcgfx.dlm" /pdbtype:sept /libpath:"..\..\..\nvsdk\common\lib\debug" /libpath:"..\..\..\nvsdk\opengl\lib\debug"
# SUBTRACT BASE LINK32 /verbose
# ADD LINK32 nv_math_mtdll.lib d3dx9.lib dxerr9.lib nv_plugin_mtdll.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib winmm.lib vfw32.lib bmm.lib core.lib geom.lib gfx.lib mesh.lib maxutil.lib maxscrpt.lib manipsys.lib paramblk2.lib ole32.lib comctl32.lib /nologo /dll /debug /machine:I386 /out:"..\..\bin\debug\cgfxMax\plugins\nvcgfx.dlm" /pdbtype:sept /libpath:"..\..\..\LIBS\lib\debug" /libpath:"$(CG_LIB_PATH)" /libpath:"../../../LIBS/implib/debug"
# SUBTRACT LINK32 /verbose
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy                                 ..\..\..\libs\dll\debug\nv_gui_mtdll.dll                              ..\..\bin\debug\cgfxMax\plugins                             	copy                          ..\..\..\libs\dll\debug\nv_fx_mtdll.dll                          ..\..\bin\debug\cgfxMax\plugins                         	copy                          ..\..\..\libs\dll\debug\nv_renderdevice_mtdll.dll                          ..\..\bin\debug\cgfxMax\plugins                         	copy                          ..\..\..\libs\dll\debug\nv_renderdevice9_mtdll.dll                          ..\..\bin\debug\cgfxMax\plugins                         	copy             ..\..\..\libs\dll\debug\nv_sys_mtdll.dll             ..\..\bin\debug\cgfxMax            	copy                              default.fx                              ..\..\bin\debug\cgfxMax\plugins                             	copy                      nvplugins.inf                      ..\..\bin\debug\cgfxMax\plugins                     	copy                                 ..\..\..\libs\dll\debug\Devil_mtdll.dll                              ..\..\bin\debug\cgfxMax\                             	copy \
                                 ..\..\..\libs\dll\debug\Devilu_mtdll.dll                                ..\..\bin\debug\cgfxMax\                              	copy                                ..\..\bin\debug\cgfxMax\plugins\*.*                               $(MAXPATHDEBUG)\plugins\                              	copy                                ..\..\bin\debug\cgfxMax\*.*                               $(MAXPATHDEBUG)\ 
# End Special Build Tool

!ELSEIF  "$(CFG)" == "nvmax - Win32 Release Multithreaded DLL with PDB"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "nvmax___Win32_Release_Multithreaded_DLL_with_PDB"
# PROP BASE Intermediate_Dir "nvmax___Win32_Release_Multithreaded_DLL_with_PDB"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "nvmax___Win32_Release_Multithreaded_DLL_with_PDB"
# PROP Intermediate_Dir "nvmax___Win32_Release_Multithreaded_DLL_with_PDB"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /Zi /O2 /I "..\..\..\LIBS\inc" /I "$(CG_INC_PATH)" /I "..\..\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NVMAX_EXPORTS" /FR /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O2 /I "..\..\..\LIBS\inc" /I "$(CG_INC_PATH)" /I "..\..\inc" /I "..\..\..\libs\inc\shared" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NVMAX_EXPORTS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 nv_math_mtdll.lib d3dx9.lib dxerr9.lib nv_plugin_mtdll.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib winmm.lib vfw32.lib bmm.lib core.lib geom.lib gfx.lib mesh.lib maxutil.lib maxscrpt.lib manipsys.lib paramblk2.lib ole32.lib comctl32.lib /nologo /dll /debug /machine:I386 /out:"..\..\bin\release\cgfxMax\plugins\nvcgfx.dlm" /libpath:"..\..\..\LIBS\lib\release" /libpath:"$(CG_LIB_PATH)" /libpath:"../../../LIBS/implib/release"
# ADD LINK32 nv_math_mtdll.lib d3dx9.lib dxerr9.lib nv_plugin_mtdll.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib winmm.lib vfw32.lib bmm.lib core.lib geom.lib gfx.lib mesh.lib maxutil.lib maxscrpt.lib manipsys.lib paramblk2.lib ole32.lib comctl32.lib /nologo /dll /debug /machine:I386 /out:"..\..\bin\release_PDB\cgfxMax\plugins\nvcgfx.dlm" /libpath:"..\..\..\LIBS\lib\release_PDB" /libpath:"$(CG_LIB_PATH)" /libpath:"../../../LIBS/implib/release_PDB"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy                                 ..\..\..\libs\dll\release_PDB\nv_gui_mtdll.dll                              ..\..\bin\release_PDB\cgfxMax\plugins                             	copy                          ..\..\..\libs\dll\release_PDB\nv_fx_mtdll.dll                          ..\..\bin\release_PDB\cgfxMax\plugins                         	copy                          ..\..\..\libs\dll\release_PDB\nv_renderdevice_mtdll.dll                          ..\..\bin\release_PDB\cgfxMax\plugins                         	copy                          ..\..\..\libs\dll\release_PDB\nv_renderdevice9_mtdll.dll                          ..\..\bin\release_PDB\cgfxMax\plugins                         	copy             ..\..\..\libs\dll\release_PDB\nv_sys_mtdll.dll             ..\..\bin\release_PDB\cgfxMax            	copy                              default.fx                              ..\..\bin\release_PDB\cgfxMax\plugins                             	copy                      nvplugins.inf                      ..\..\bin\release_PDB\cgfxMax\plugins                     	copy                                 ..\..\..\libs\dll\release\Devil_mtdll.dll \
                             ..\..\bin\release_PDB\cgfxMax                              	copy                                   ..\..\..\libs\dll\release\Devilu_mtdll.dll                               ..\..\bin\release_PDB\cgfxMax                              	copy                                ..\..\bin\release_PDB\cgfxMax\plugins\*.*                               $(MAXPATH)\plugins\                              	copy                                ..\..\bin\release_PDB\cgfxMax\*.*                              $(MAXPATH)\ 
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "nvmax - Win32 Hybrid"
# Name "nvmax - Win32 Release Multithreaded DLL"
# Name "nvmax - Win32 Debug Multithreaded DLL"
# Name "nvmax - Win32 Release Multithreaded DLL with PDB"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\examples\cg_regression.ms
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\cgfxdatabridge.cpp
# ADD CPP /Yu"pch.h"
# End Source File
# Begin Source File

SOURCE=.\connections.cpp
# ADD CPP /Yu"pch.h"
# End Source File
# Begin Source File

SOURCE=.\DllEntry.cpp
# ADD CPP /Yu"pch.h"
# End Source File
# Begin Source File

SOURCE=.\Lighting.cpp
# ADD CPP /Yu"pch.h"
# End Source File
# Begin Source File

SOURCE=.\material.cpp
# ADD CPP /Yu"pch.h"
# End Source File
# Begin Source File

SOURCE=.\material.def
# End Source File
# Begin Source File

SOURCE=.\material.rc
# End Source File
# Begin Source File

SOURCE=..\..\..\LIBS\src\nv_dds_common\nv_dds_common.cpp
# SUBTRACT CPP /YX
# End Source File
# Begin Source File

SOURCE=.\nvplugins.inf
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\pch.cpp
# ADD CPP /Yc"pch.h"
# End Source File
# Begin Source File

SOURCE=.\RenderMesh.cpp
# ADD CPP /Yu"pch.h"
# End Source File
# Begin Source File

SOURCE=.\sctex.cpp
# ADD CPP /Yu"pch.h"
# End Source File
# Begin Source File

SOURCE=.\shaderinfo.cpp
# ADD CPP /Yu"pch.h"
# End Source File
# Begin Source File

SOURCE=.\shaderinfodata.cpp
# ADD CPP /Yu"pch.h"
# End Source File
# Begin Source File

SOURCE=.\TextureMgr.cpp
# ADD CPP /Yu"pch.h"
# End Source File
# Begin Source File

SOURCE=.\tweakables.cpp
# ADD CPP /Yu"pch.h"
# End Source File
# Begin Source File

SOURCE=.\Utility.cpp
# ADD CPP /Yu"pch.h"
# End Source File
# Begin Source File

SOURCE=.\vertexshader.cpp
# ADD CPP /Yu"pch.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\cgfxdatabridge.h
# End Source File
# Begin Source File

SOURCE=.\connectionpblock.h
# End Source File
# Begin Source File

SOURCE=.\connections.h
# End Source File
# Begin Source File

SOURCE=.\effectmgr.h
# End Source File
# Begin Source File

SOURCE=.\filekey.h
# End Source File
# Begin Source File

SOURCE=.\filesearch.h
# End Source File
# Begin Source File

SOURCE=.\icgfx.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\cgfxMax\icgfxdatabridge.h
# End Source File
# Begin Source File

SOURCE=.\Lighting.h
# End Source File
# Begin Source File

SOURCE=.\material.h
# End Source File
# Begin Source File

SOURCE=..\..\..\LIBS\inc\nv_dds_common\nv_dds_common.h
# End Source File
# Begin Source File

SOURCE=.\nvtexture.h
# End Source File
# Begin Source File

SOURCE=.\pch.h
# End Source File
# Begin Source File

SOURCE=.\RenderMesh.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\scenemgr.h
# End Source File
# Begin Source File

SOURCE=.\sctex.h
# End Source File
# Begin Source File

SOURCE=.\shaderinfo.h
# End Source File
# Begin Source File

SOURCE=.\TextureMgr.h
# End Source File
# Begin Source File

SOURCE=.\tweakables.h
# End Source File
# Begin Source File

SOURCE=.\Utility.h
# End Source File
# Begin Source File

SOURCE=.\veroverrides.h
# End Source File
# Begin Source File

SOURCE=.\vertexshader.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\tweakabl.bmp
# End Source File
# Begin Source File

SOURCE=.\tweakabl_disabled.bmp
# End Source File
# Begin Source File

SOURCE=.\tweakabl_mask.bmp
# End Source File
# End Group
# End Target
# End Project
