@echo off
if "%BUILD_TOOLS_DIR%"=="" set BUILD_TOOLS_DIR=d:\nv\tools

set SAVEPATH=%PATH%
set SAVEMSVCDIR=%MSVCDIR%
set SAVEINCLUDE=%INCLUDE%
set SAVELIB=%LIB%
set SAVEDDKDIR=%DDKDIR%
set SAVESDKDIR=%SDKDIR%
set SAVECGINC=%CG_INC_PATH%
set SAVECGLIB=%CG_LIB_PATH%

set MSVCDIR=%BUILD_TOOLS_DIR%\msvc60sp3
set DDKDIR=%BUILD_TOOLS_DIR%\ddk
set SDKDIR=%BUILD_TOOLS_DIR%\sdk
set PATH=%MSVCDIR%\msdev;%MSVCDIR%\bin;%PATH%
set INCLUDE=%DDKDIR%\winxp\inc\wxp;%DDKDIR%\winxp\inc\crt;%MSVCDIR%\include;%MSVCDIR%\mfc\include;..\..\inc;..\..\..\inc;%SDKDIR%\cg\include
set LIB0=%MSVCDIR%\lib;%MSVCDIR%\mfc\lib;%SDKDIR%\cg\lib

set LIB=%LIB0%;..\..\lib\release_PDB
%MSVCDIR%\msdev\msdev.exe nv_gui.dsw /CLEAN /MAKE "nv_gui - Win32 Release Multithreaded DLL with PDB" /REBUILD /USEENV

set LIB=%LIB0%;..\..\lib\debug
rem %MSVCDIR%\msdev\msdev.exe nv_gui.dsw /CLEAN /MAKE "nv_gui - Win32 Debug Multithreaded DLL" /REBUILD /USEENV

set PATH=%SAVEPATH%
set MSVCDIR=%SAVEMSVCDIR%
set INCLUDE=%SAVEINCLUDE%
set LIB=%SAVELIB%
set DDKDIR=%SAVEDDKDIR%
set SDKDIR=%SAVESDKDIR%
set CG_INC_PATH=%SAVECGINC%
set CG_LIB_PATH=%SAVECGLIB%

set SAVEPATH=
set SAVEMSVCDIR=
set SAVEINCLUDE=
set SAVELIB=
set SAVEDDKDIR=
set SAVESDKDIR=
set SAVECGINC=
set SAVECGLIB=
set LIB0=
