//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
/// DLL entrypoints for exe hooks
///
//====================================================================================
#pragma once
#ifndef GAMEH_H
#define GAMEH_H

#ifdef __cplusplus    /// If used by C++ code, 
extern "C" {          /// we need to export the C interface
#endif

	/// Functions exported from Game DLL for use within Reality.exe and Reality Builder.exe
	LRESULT GAME_API Game_HandleMessages( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	void GAME_API Game_Shutdown();
	void GAME_API Game_Initialize(HWND topHwnd, HWND ChildHwnd, HINSTANCE hInst, string cmdLine, bool isDedicated);
	void GAME_API Game_Tick();
	bool GAME_API Game_Ready();
	int  GAME_API Game_MaxTicksPerSecond();
	void GAME_API Game_NewMap(const char* mapname);
	void GAME_API Game_RunConsoleCommand(string CommandLine);

#ifdef __cplusplus
}
#endif

#endif