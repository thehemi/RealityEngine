# Microsoft Developer Studio Project File - Name="DX9SDKSampleFramework" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=DX9SDKSampleFramework - Win32 Debug Multithreaded DLL
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "DX9SDKSampleFramework.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "DX9SDKSampleFramework.mak" CFG="DX9SDKSampleFramework - Win32 Debug Multithreaded DLL"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DX9SDKSampleFramework - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "DX9SDKSampleFramework - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "DX9SDKSampleFramework - Win32 Release Multithreaded" (based on "Win32 (x86) Static Library")
!MESSAGE "DX9SDKSampleFramework - Win32 Debug Multithreaded" (based on "Win32 (x86) Static Library")
!MESSAGE "DX9SDKSampleFramework - Win32 Debug Multithreaded DLL" (based on "Win32 (x86) Static Library")
!MESSAGE "DX9SDKSampleFramework - Win32 Release Multithreaded DLL" (based on "Win32 (x86) Static Library")
!MESSAGE "DX9SDKSampleFramework - Win32 Release Multithreaded DLL with PDB" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "DX9SDKSampleFramework"
# PROP Scc_LocalPath "..\.."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "DX9SDKSampleFramework - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\..\inc\DX9SDKSampleFramework" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\lib\release\DX9SDKSampleFramework.lib"

!ELSEIF  "$(CFG)" == "DX9SDKSampleFramework - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\inc\DX9SDKSampleFramework" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\lib\debug\DX9SDKSampleFramework.lib"

!ELSEIF  "$(CFG)" == "DX9SDKSampleFramework - Win32 Release Multithreaded"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release Multithreaded"
# PROP BASE Intermediate_Dir "Release Multithreaded"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release Multithreaded"
# PROP Intermediate_Dir "Release Multithreaded"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /I "..\..\inc\DX9SDKSampleFramework" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\inc\DX9SDKSampleFramework" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\lib\release\DX9SDKSampleFramework.lib"
# ADD LIB32 /nologo /out:"..\..\lib\release\DX9SDKSampleFramework_mt.lib"

!ELSEIF  "$(CFG)" == "DX9SDKSampleFramework - Win32 Debug Multithreaded"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug Multithreaded"
# PROP BASE Intermediate_Dir "Debug Multithreaded"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug Multithreaded"
# PROP Intermediate_Dir "Debug Multithreaded"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\inc\DX9SDKSampleFramework" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FR /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\..\inc\DX9SDKSampleFramework" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\lib\debug\DX9SDKSampleFramework.lib"
# ADD LIB32 /nologo /out:"..\..\lib\debug\DX9SDKSampleFramework_mt.lib"

!ELSEIF  "$(CFG)" == "DX9SDKSampleFramework - Win32 Debug Multithreaded DLL"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug Multithreaded DLL"
# PROP BASE Intermediate_Dir "Debug Multithreaded DLL"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug Multithreaded DLL"
# PROP Intermediate_Dir "Debug Multithreaded DLL"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\inc\DX9SDKSampleFramework" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FR /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\inc\DX9SDKSampleFramework" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\lib\debug\DX9SDKSampleFramework.lib"
# ADD LIB32 /nologo /out:"..\..\lib\debug\DX9SDKSampleFramework_mtdll.lib"

!ELSEIF  "$(CFG)" == "DX9SDKSampleFramework - Win32 Release Multithreaded DLL"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release Multithreaded DLL"
# PROP BASE Intermediate_Dir "Release Multithreaded DLL"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release Multithreaded DLL"
# PROP Intermediate_Dir "Release Multithreaded DLL"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /I "..\..\inc\DX9SDKSampleFramework" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\inc\DX9SDKSampleFramework" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\lib\release\DX9SDKSampleFramework.lib"
# ADD LIB32 /nologo /out:"..\..\lib\release\DX9SDKSampleFramework_mtdll.lib"

!ELSEIF  "$(CFG)" == "DX9SDKSampleFramework - Win32 Release Multithreaded DLL with PDB"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "DX9SDKSampleFramework___Win32_Release_Multithreaded_DLL_with_PDB"
# PROP BASE Intermediate_Dir "DX9SDKSampleFramework___Win32_Release_Multithreaded_DLL_with_PDB"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseMTDLLPDB"
# PROP Intermediate_Dir "ReleaseMTDLLPDB"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "..\..\inc\DX9SDKSampleFramework" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O2 /I "..\..\inc\DX9SDKSampleFramework" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\lib\release\DX9SDKSampleFramework_mtdll.lib"
# ADD LIB32 /nologo /out:"..\..\lib\release_PDB\DX9SDKSampleFramework_mtdll.lib"

!ENDIF 

# Begin Target

# Name "DX9SDKSampleFramework - Win32 Release"
# Name "DX9SDKSampleFramework - Win32 Debug"
# Name "DX9SDKSampleFramework - Win32 Release Multithreaded"
# Name "DX9SDKSampleFramework - Win32 Debug Multithreaded"
# Name "DX9SDKSampleFramework - Win32 Debug Multithreaded DLL"
# Name "DX9SDKSampleFramework - Win32 Release Multithreaded DLL"
# Name "DX9SDKSampleFramework - Win32 Release Multithreaded DLL with PDB"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\d3dapp.cpp
# End Source File
# Begin Source File

SOURCE=.\d3denumeration.cpp
# End Source File
# Begin Source File

SOURCE=.\d3dfile.cpp
# End Source File
# Begin Source File

SOURCE=.\d3dfont.cpp
# End Source File
# Begin Source File

SOURCE=.\d3dsaver.cpp
# End Source File
# Begin Source File

SOURCE=.\d3dsettings.cpp
# End Source File
# Begin Source File

SOURCE=.\d3dutil.cpp
# End Source File
# Begin Source File

SOURCE=.\dxutil.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\inc\DX9SDKSampleFramework\d3dapp.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\DX9SDKSampleFramework\d3denumeration.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\DX9SDKSampleFramework\d3dfile.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\DX9SDKSampleFramework\d3dfont.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\DX9SDKSampleFramework\d3dsaver.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\DX9SDKSampleFramework\d3dsettings.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\DX9SDKSampleFramework\d3dutil.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\DX9SDKSampleFramework\dxutil.h
# End Source File
# End Group
# End Target
# End Project
