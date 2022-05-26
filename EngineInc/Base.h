//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
/// Name:Base.h: Shared engine header
///
/// Author: Tim Johnson
//====================================================================================
#pragma once

/*----------------------------------------------------------------------------
	API.
----------------------------------------------------------------------------*/
/// The following ifdef block is the standard way of creating macros which make exporting 
/// from a DLL simpler. All files within this DLL are compiled with the ENGINE_EXPORTS
/// symbol defined on the command line. this symbol should not be defined on any project
/// that uses this DLL. This way any other project whose source files include this file see 
/// ENGINE_API functions as being imported from a DLL, whereas this DLL sees symbols
/// defined with this macro as being exported.

#ifdef ENGINE_EXPORTS
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif

/// Report error and terminate program. Returns S_OK to shut up functions. Will never really return.
ENGINE_API HRESULT Error(const TCHAR *fmt, ...);
/// Report warning without terminating program (stops program until user responds).
ENGINE_API void Warning(const TCHAR *fmt, ...);
/// Serious warnings, always show MB
ENGINE_API void SeriousWarning(const TCHAR *fmt, ...);
/// Output to global log file
ENGINE_API void LogPrintf(LogLevel level, const TCHAR *fmt, ...);
/// Output to global log file
ENGINE_API void LogPrintf(const TCHAR *fmt, ...);
/// Output to global log file
ENGINE_API void LogPrintf(LogLevel level, const TCHAR *fmt, ...);
//
ENGINE_API bool ResolvePathW(string& str, bool LastDirectoriesMustReallyMatch = false);

/// Report error and terminate program. Returns S_OK to shut up functions. Will never really return.
ENGINE_API HRESULT Error(const CHAR *fmt, ...);
/// Report warning without terminating program (stops program until user responds).
ENGINE_API void Warning(const CHAR *fmt, ...);
/// Report a message to the user for debug-only builds
ENGINE_API void Debug(const CHAR *fmt, ...);
/// Output to global log file
ENGINE_API void LogPrintf(LogLevel level, const CHAR *fmt, ...);
/// Output to global log file
ENGINE_API void LogPrintf(const CHAR *fmt, ...);
/// Prefix all log entrys with text until LogPopPrefix is called
ENGINE_API void LogPushPrefix(const CHAR* prefix);
/// Remove last prefix
ENGINE_API void LogPopPrefix();
/// Disable/enable exception handler
ENGINE_API void SetExceptionHandling(bool enable);


/// Given a file or file+path will find the valid location by searching appropriate directory location
/// Returns success result. Modifies input string to valid location
/// If not resolved, string will not be touched
ENGINE_API bool FindMedia(string& resource, const char* location, bool LastDirsMustMatch = false);

ENGINE_API void StartMiniTimer();
ENGINE_API float StopMiniTimer();
//alternate method to get application time, equivalent to GSeconds but accurate to when you call it.
//however, this is slow, so don't do it often. 
//Preferrably use GSeconds variable, which is updated every frame in Engine::Update()
ENGINE_API float GetGSeconds();

ENGINE_API int PrintStackWalk( TCHAR * pc, int iPcLen, EXCEPTION_POINTERS *pExPtrs);
ENGINE_API int PrintStackToString( TCHAR * pc, int iPcLen );
/*-----------------------------------------------------------------------------
	Global variables.
-----------------------------------------------------------------------------*/
ENGINE_API extern float	GDeltaTime; /// Time between frames. 1.f/GDeltaTime = FPS
ENGINE_API extern float	GSeconds;   /// Time since application launch
