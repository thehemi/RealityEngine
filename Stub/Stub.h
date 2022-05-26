#pragma once
extern void ShowEditor();


// Library entrypoints for static linking
typedef void ( *TGame_Initialize)(HWND topHwnd, HWND childHwnd, HINSTANCE hInst, string startMap, bool isDedicated);
typedef void ( *TGame_Shutdown)();
typedef LRESULT ( *TGame_HandleMessages)(HWND hWnd, unsigned int uMsg, WPARAM wParam, LPARAM lParam);
typedef void ( *TGame_Tick)();
typedef bool ( *TGame_Ready)();
typedef void ( *TGame_ReloadSky)();
typedef void ( *TGame_NewMap)(const char* mapfile);
typedef void ( *TGame_RunConsoleCommand)(string commandLine);
typedef int  ( *TGame_MaxTicksPerSecond)();


extern TGame_Initialize		Game_Initialize;
extern TGame_Shutdown			Game_Shutdown;
extern TGame_HandleMessages	Game_HandleMessages;
extern TGame_Tick				Game_Tick;
extern TGame_Ready				Game_Ready;
extern TGame_ReloadSky			Game_ReloadSky;
extern TGame_NewMap				Game_NewMap;
extern TGame_RunConsoleCommand	Game_RunConsoleCommand;
extern TGame_MaxTicksPerSecond  Game_MaxTicksPerSecond;

// Stub call
bool LoadLibraries(HWND topHwnd, HWND childHwnd, HINSTANCE hInstance, string cmdLine, bool isDedicated=false);
bool LibsInitialized();





