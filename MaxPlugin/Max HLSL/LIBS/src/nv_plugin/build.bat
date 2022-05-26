@echo off
if "%BUILD_TOOLS_DIR%"=="" set BUILD_TOOLS_DIR=d:\nv\tools

set SAVEPATH=%PATH%
set SAVEMSVCDIR=%MSVCDIR%
set SAVEINCLUDE=%INCLUDE%
set SAVELIB=%LIB%
set SAVEDDKDIR=%DDKDIR%
set SAVESDKDIR=%SDKDIR%

set MSVCDIR=%BUILD_TOOLS_DIR%\msvc60sp3
set DDKDIR=%BUILD_TOOLS_DIR%\ddk
set SDKDIR=%BUILD_TOOLS_DIR%\sdk
set PATH=%MSVCDIR%\msdev;%MSVCDIR%\bin;%PATH%
set INCLUDE=%MSVCDIR%\include;%MSVCDIR%\mfc\include
set LIB=%MSVCDIR%\lib

%MSVCDIR%\msdev\msdev.exe nv_plugin.dsp /CLEAN /MAKE "nv_plugin - Win32 Release Multithreaded DLL with PDB" /REBUILD /USEENV
%MSVCDIR%\msdev\msdev.exe nv_plugin.dsp /CLEAN /MAKE "nv_plugin - Win32 Debug Multithreaded DLL" /REBUILD /USEENV

set PATH=%SAVEPATH%
set MSVCDIR=%SAVEMSVCDIR%
set INCLUDE=%SAVEINCLUDE%
set LIB=%SAVELIB%
set DDKDIR=%SAVEDDKDIR%
set SDKDIR=%SAVESDKDIR%

set SAVEPATH=
set SAVEMSVCDIR=
set SAVEINCLUDE=
set SAVELIB=
set SAVEDDKDIR=
set SAVESDKDIR=
