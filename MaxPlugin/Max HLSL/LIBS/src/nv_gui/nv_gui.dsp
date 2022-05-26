# Microsoft Developer Studio Project File - Name="nv_gui" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=nv_gui - Win32 Debug Multithreaded DLL
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "nv_gui.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "nv_gui.mak" CFG="nv_gui - Win32 Debug Multithreaded DLL"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "nv_gui - Win32 Debug Multithreaded DLL" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "nv_gui - Win32 Release Multithreaded DLL" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "nv_gui - Win32 Release Multithreaded DLL with PDB" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "nv_gui"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "nv_gui - Win32 Debug Multithreaded DLL"

# PROP BASE Use_MFC 1
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "nv_gui___Win32_Debug_Multithreaded_DLL"
# PROP BASE Intermediate_Dir "nv_gui___Win32_Debug_Multithreaded_DLL"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "nv_gui___Win32_Debug_Multithreaded_DLL"
# PROP Intermediate_Dir "nv_gui___Win32_Debug_Multithreaded_DLL"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NV_GUI_EXPORTS" /D "_WINDLL" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NV_GUI_EXPORTS" /D "_WINDLL" /D "_AFXDLL" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 nv_math.lib /nologo /dll /debug /machine:I386 /out:"..\..\dll\debug\nv_gui.dll" /implib:"..\..\lib\debug\nv_gui.lib" /pdbtype:sept /libpath:"..\..\lib\debug"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 nv_math_mtdll.lib /nologo /dll /debug /machine:I386 /out:"..\..\dll\debug\nv_gui_mtdll.dll" /implib:"..\..\implib\debug\nv_gui_mtdll.lib" /pdbtype:sept /libpath:"..\..\lib\debug" /libpath:"..\..\implib\debug"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "nv_gui - Win32 Release Multithreaded DLL"

# PROP BASE Use_MFC 1
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "nv_gui___Win32_Release_Multithreaded_DLL"
# PROP BASE Intermediate_Dir "nv_gui___Win32_Release_Multithreaded_DLL"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "nv_gui___Win32_Release_Multithreaded_DLL"
# PROP Intermediate_Dir "nv_gui___Win32_Release_Multithreaded_DLL"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /I "..\..\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NV_GUI_EXPORTS" /D "_WINDLL" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NV_GUI_EXPORTS" /D "_WINDLL" /D "_AFXDLL" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 nv_math.lib /nologo /dll /machine:I386 /out:"..\..\dll\release\nv_gui.dll" /implib:"..\..\lib\release\nv_gui.lib" /libpath:"..\..\lib\release"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 nv_math_mtdll.lib /nologo /dll /machine:I386 /out:"..\..\dll\release\nv_gui_mtdll.dll" /implib:"..\..\implib\release\nv_gui_mtdll.lib" /libpath:"..\..\lib\release" /libpath:"..\..\implib\release"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "nv_gui - Win32 Release Multithreaded DLL with PDB"

# PROP BASE Use_MFC 1
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "nv_gui___Win32_Release_Multithreaded_DLL_with_PDB"
# PROP BASE Intermediate_Dir "nv_gui___Win32_Release_Multithreaded_DLL_with_PDB"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseMTDLLPDB"
# PROP Intermediate_Dir "ReleaseMTDLLPDB"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /Zi /O2 /I "..\..\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NV_GUI_EXPORTS" /D "_WINDLL" /D "_AFXDLL" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O2 /I "..\..\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NV_GUI_EXPORTS" /D "_WINDLL" /D "_AFXDLL" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 nv_math_mtdll.lib /nologo /dll /debug /machine:I386 /out:"..\..\dll\release\nv_gui_mtdll.dll" /implib:"..\..\lib\release\nv_gui_mtdll.lib" /libpath:"..\..\lib\release"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 nv_math_mtdll.lib /nologo /dll /pdb:"../../dll/release_PDB/nv_gui_mtdll.pdb" /debug /machine:I386 /out:"..\..\dll\release_PDB\nv_gui_mtdll.dll" /implib:"..\..\implib\release_PDB\nv_gui_mtdll.lib" /libpath:"..\..\lib\release_PDB" /libpath:"..\..\implib\release_PDB"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "nv_gui - Win32 Debug Multithreaded DLL"
# Name "nv_gui - Win32 Release Multithreaded DLL"
# Name "nv_gui - Win32 Release Multithreaded DLL with PDB"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\BitmapCtrl.cpp
# ADD BASE CPP /Yu"stdafx.h"
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\bitmapfiledlg.cpp
# ADD BASE CPP /Yu"stdafx.h"
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\CCrystalEditView.cpp
# ADD BASE CPP /Yu"stdafx.h"
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\CCrystalTextBuffer.cpp
# ADD BASE CPP /Yu"stdafx.h"
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\CCrystalTextView.cpp
# ADD BASE CPP /Yu"stdafx.h"
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\CCrystalTextView2.cpp
# ADD BASE CPP /Yu"stdafx.h"
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\CEditReplaceDlg.cpp
# ADD BASE CPP /Yu"stdafx.h"
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\CFindTextDlg.cpp
# ADD BASE CPP /Yu"stdafx.h"
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\CGFXDocument.cpp
# ADD BASE CPP /Yu"stdafx.h"
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\cgfxmainframe.cpp
# ADD BASE CPP /Yu"stdafx.h"
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\CGFXView.cpp
# ADD BASE CPP /Yu"stdafx.h"
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\ColorPickerDlg.cpp
# ADD BASE CPP /Yu"stdafx.h"
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\DebugDlg.cpp
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\DIB.cpp
# ADD BASE CPP /Yu"stdafx.h"
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\dlgbars.cpp
# ADD BASE CPP /Yu"stdafx.h"
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\DynamicDlg.cpp
# ADD BASE CPP /Yu"stdafx.h"
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\fileedit.cpp
# ADD BASE CPP /Yu"stdafx.h"
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\GridCell.cpp
# ADD BASE CPP /Yu"stdafx.h"
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\GridCellBase.cpp
# ADD BASE CPP /Yu"stdafx.h"
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\GridCellNumeric.cpp
# ADD BASE CPP /Yu"stdafx.h"
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\GridCtrl.cpp
# ADD BASE CPP /Yu"stdafx.h"
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\GridDlg.cpp
# ADD BASE CPP /Yu"stdafx.h"
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\GridDropTarget.cpp
# ADD BASE CPP /Yu"stdafx.h"
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\gui.cpp
# ADD BASE CPP /Yu"stdafx.h"
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\gui.rc
# End Source File
# Begin Source File

SOURCE=.\gui6.def
# End Source File
# Begin Source File

SOURCE=.\gui6hybrid.def
# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\InPlaceEdit.cpp
# ADD BASE CPP /Yu"stdafx.h"
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\notifysliderctrl.cpp
# ADD BASE CPP /Yu"stdafx.h"
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\nv_gui.cpp
# ADD BASE CPP /Yu"stdafx.h"
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\PropTree.cpp
# ADD BASE CPP /Yu"stdafx.h"
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\PropTreeInfo.cpp
# ADD BASE CPP /Yu"stdafx.h"
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\PropTreeItem.cpp
# ADD BASE CPP /Yu"stdafx.h"
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\PropTreeItemColor.cpp
# ADD BASE CPP /Yu"stdafx.h"
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\PropTreeItemCombo.cpp
# ADD BASE CPP /Yu"stdafx.h"
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\PropTreeItemEdit.cpp
# ADD BASE CPP /Yu"stdafx.h"
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\PropTreeItemFileBrowse.cpp
# ADD BASE CPP /Yu"stdafx.h"
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\PropTreeItemGrid.cpp
# ADD BASE CPP /Yu"stdafx.h"
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\PropTreeItemStatic.cpp
# ADD BASE CPP /Yu"stdafx.h"
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\PropTreeList.cpp
# ADD BASE CPP /Yu"stdafx.h"
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD BASE CPP /Yc"stdafx.h"
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\TitleTip.cpp
# ADD BASE CPP /Yu"stdafx.h"
# ADD CPP /Yu"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\inc\nv_gui\bitmapctrl.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_gui\bitmapfiledlg.h
# End Source File
# Begin Source File

SOURCE=.\CCrystalEditView.h
# End Source File
# Begin Source File

SOURCE=.\CCrystalEditView.inl
# End Source File
# Begin Source File

SOURCE=.\CCrystalTextBuffer.h
# End Source File
# Begin Source File

SOURCE=.\CCrystalTextBuffer.inl
# End Source File
# Begin Source File

SOURCE=.\CCrystalTextView.h
# End Source File
# Begin Source File

SOURCE=.\CCrystalTextView.inl
# End Source File
# Begin Source File

SOURCE=.\cedefs.h
# End Source File
# Begin Source File

SOURCE=.\CEditReplaceDlg.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_gui\CellRange.h
# End Source File
# Begin Source File

SOURCE=.\CFindTextDlg.h
# End Source File
# Begin Source File

SOURCE=.\CGFXDocument.h
# End Source File
# Begin Source File

SOURCE=.\cgfxmainframe.h
# End Source File
# Begin Source File

SOURCE=.\CGFXView.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_gui\ColorPickerDlg.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_gui\DebugDlg.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_gui\DIB.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_gui\dlgbars.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_gui\DynamicDlg.h
# End Source File
# Begin Source File

SOURCE=.\editcmd.h
# End Source File
# Begin Source File

SOURCE=.\editreg.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_gui\fileedit.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_gui\GridCell.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_gui\GridCellBase.h
# End Source File
# Begin Source File

SOURCE=.\GridCellNumeric.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_gui\GridCtrl.h
# End Source File
# Begin Source File

SOURCE=.\GridDlg.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_gui\GridDropTarget.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_gui\gui.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_gui\InPlaceEdit.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_gui\invdebugconsole.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_gui\invgui.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_gui\invguidata.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_gui\MemDC.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_gui\notifysliderctrl.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_gui\nv_gui.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_gui\nvguicommon.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_gui\nvguidata.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_gui\PropTree.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_gui\PropTreeInfo.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_gui\PropTreeItem.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_gui\PropTreeItemColor.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_gui\PropTreeItemCombo.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_gui\PropTreeItemEdit.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_gui\PropTreeItemFileBrowse.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_gui\PropTreeItemGrid.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_gui\PropTreeItemStatic.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_gui\PropTreeList.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\nv_gui\TitleTip.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\CCHSB.bmp
# End Source File
# Begin Source File

SOURCE=.\res\ccrgb.bmp
# End Source File
# Begin Source File

SOURCE=.\res\CgFXToolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\CgFXToolbard.bmp
# End Source File
# Begin Source File

SOURCE=.\res\CgFXToolbare.bmp
# End Source File
# Begin Source File

SOURCE=.\res\connection.ico
# End Source File
# Begin Source File

SOURCE=.\res\default.bmp
# End Source File
# Begin Source File

SOURCE=.\res\fpoint.cur
# End Source File
# Begin Source File

SOURCE=.\res\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\mg_cur.cur
# End Source File
# Begin Source File

SOURCE=.\mg_icons.bmp
# End Source File
# Begin Source File

SOURCE=.\res\NeedImage.bmp
# End Source File
# Begin Source File

SOURCE=.\res\NoPreview.bmp
# End Source File
# Begin Source File

SOURCE=.\res\SampleDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\spliter.cur
# End Source File
# End Group
# End Target
# End Project
