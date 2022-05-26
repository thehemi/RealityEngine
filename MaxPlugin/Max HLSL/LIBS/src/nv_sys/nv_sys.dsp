# Microsoft Developer Studio Project File - Name="nv_sys" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=nv_sys - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "nv_sys.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "nv_sys.mak" CFG="nv_sys - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "nv_sys - Win32 Debug Multithreaded DLL" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "nv_sys - Win32 Release Multithreaded DLL" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "nv_sys - Win32 Release Multithreaded DLL with PDB" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "nv_sys - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "nv_sys - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "nv_sys"
# PROP Scc_LocalPath "..\.."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "nv_sys - Win32 Debug Multithreaded DLL"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug Multithreaded DLL"
# PROP BASE Intermediate_Dir "Debug Multithreaded DLL"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug Multithreaded DLL"
# PROP Intermediate_Dir "Debug Multithreaded DLL"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\..\inc" /I "..\..\inc\nv_sys" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NV_SYS_EXPORTS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\inc" /I "..\..\inc\nv_sys" /D "_DEBUG" /D "SYS_BUILD" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NV_SYS_EXPORTS" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib nv_math_mtdll.lib /nologo /dll /debug /machine:I386 /implib:"..\..\implib\debug\nv_sys_mtdll.lib" /pdbtype:sept /libpath:"..\..\lib\debug"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "nv_sys - Win32 Release Multithreaded DLL"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release Multithreaded DLL"
# PROP BASE Intermediate_Dir "Release Multithreaded DLL"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release Multithreaded DLL"
# PROP Intermediate_Dir "Release Multithreaded DLL"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "..\..\inc" /I "..\..\inc\nv_sys" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NV_SYS_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\inc" /I "..\..\inc\nv_sys" /D "NDEBUG" /D "SYS_BUILD" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NV_SYS_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib nv_math_mtdll.lib /nologo /dll /machine:I386  /implib:"..\..\implib\release\nv_sys_mtdll.lib" /libpath:"..\..\lib\release"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "nv_sys - Win32 Release Multithreaded DLL with PDB"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release Multithreaded DLL with PDB"
# PROP BASE Intermediate_Dir "Release Multithreaded DLL with PDB"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release Multithreaded DLL with PDB"
# PROP Intermediate_Dir "Release Multithreaded DLL with PDB"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "..\..\inc" /I "..\..\inc\nv_sys" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NV_SYS_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O2 /I "..\..\inc" /I "..\..\inc\nv_sys" /D "NDEBUG" /D "SYS_BUILD" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NV_SYS_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /machine:I386 /out:"..\..\lib\release\nv_sys_mtdll.dll" /implib:"..\..\lib\release\nv_sys_mtdll.lib.lib"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib nv_math_mtdll.lib /nologo /dll /pdb:"..\..\dll\release_PDB\nv_sys_mtdll.pdb" /debug /machine:I386 /out:"..\..\dll\release_PDB\nv_sys_mtdll.dll" /implib:"..\..\implib\release_PDB\nv_sys_mtdll.lib" /libpath:"..\..\lib\release_PDB"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "nv_sys - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "nv_sys___Win32_Debug"
# PROP BASE Intermediate_Dir "nv_sys___Win32_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\inc" /I "..\..\inc\nv_sys" /D "_DEBUG" /D "SYS_BUILD" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NV_SYS_EXPORTS" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\inc" /I "..\..\inc\nv_sys" /D "_DEBUG" /D "SYS_BUILD" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NV_SYS_EXPORTS" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib nv_math_mtdll.lib /nologo /dll /debug /machine:I386 /out:"..\..\dll\debug\nv_sys_mtdll.dll" /implib:"..\..\implib\debug\nv_sys_mtdll.lib" /pdbtype:sept /libpath:"..\..\lib\debug"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib nv_math.lib /nologo /dll /debug /machine:I386 /out:"..\..\dll\debug\nv_sys.dll" /implib:"..\..\implib\debug\nv_sys.lib" /pdbtype:sept /libpath:"..\..\lib\debug"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "nv_sys - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "nv_sys___Win32_Release"
# PROP BASE Intermediate_Dir "nv_sys___Win32_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "..\..\inc" /I "..\..\inc\nv_sys" /D "NDEBUG" /D "SYS_BUILD" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NV_SYS_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\..\inc" /I "..\..\inc\nv_sys" /D "NDEBUG" /D "SYS_BUILD" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NV_SYS_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib nv_math_mtdll.lib /nologo /dll /machine:I386 /out:"..\..\dll\release\nv_sys_mtdll.dll" /implib:"..\..\implib\release\nv_sys_mtdll.lib" /libpath:"..\..\lib\release"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib nv_math.lib /nologo /dll /machine:I386 /out:"..\..\dll\release\nv_sys.dll" /implib:"..\..\implib\release\nv_sys.lib" /libpath:"..\..\lib\release"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "nv_sys - Win32 Debug Multithreaded DLL"
# Name "nv_sys - Win32 Release Multithreaded DLL"
# Name "nv_sys - Win32 Release Multithreaded DLL with PDB"
# Name "nv_sys - Win32 Debug"
# Name "nv_sys - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\nv_sys.cpp
# End Source File
# Begin Source File

SOURCE=.\nv_sys.rc
# End Source File
# Begin Source File

SOURCE=.\nvconnectionmanager.cpp
# End Source File
# Begin Source File

SOURCE=.\nvconnectionparameter.cpp
# End Source File
# Begin Source File

SOURCE=.\nvcreatorarray.cpp
# End Source File
# Begin Source File

SOURCE=.\nveffecttemplateparameter.cpp
# End Source File
# Begin Source File

SOURCE=.\nvlog.cpp
# End Source File
# Begin Source File

SOURCE=.\nvobjectsemantics.cpp
# End Source File
# Begin Source File

SOURCE=.\nvparameterlist.cpp
# End Source File
# Begin Source File

SOURCE=.\nvsystem.cpp
# End Source File
# Begin Source File

SOURCE=.\NVType.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD BASE CPP /Yc"stdafx.h"
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\inc\nv_sys\invclone.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_sys\invconnectionmanager.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_sys\invconnectionparameter.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_sys\invcreator.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_sys\invcreatorarray.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_sys\inveffecttemplateparameter.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_sys\invinterpolator.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_sys\invlog.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_sys\invobject.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_sys\invobjectsemantics.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_sys\invparameterlist.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_sys\invproperties.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_sys\invsystem.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_sys\nv_sys.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_sys\nv_typedef.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_sys\nvcgfxtype.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_sys\nvconnectionmanager.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_sys\nvconnectionparameter.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_sys\nvcreatorarray.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_sys\nveffecttemplateparameter.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_sys\NVException.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_sys\nvinterpolator.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_sys\nvlog.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_sys\nvobjectsemantics.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_sys\nvparameterlist.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_sys\nvsystem.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_sys\NVType.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
