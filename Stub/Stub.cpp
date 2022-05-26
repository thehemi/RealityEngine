// Cry Havoc.cpp : Defines the entry point for the application.
//
// This is for the game stub, do not put engine or app-specific code inside here!!!
//
#define _WIN32_WINNT 0x0501
#include <windows.h>
#include <string>
using namespace std;
#include "Stub.h"
#include "..\shared\shared.h"


TGame_Initialize		Game_Initialize;
TGame_Shutdown			Game_Shutdown;
TGame_HandleMessages	Game_HandleMessages;
TGame_Tick				Game_Tick;
TGame_Ready				Game_Ready;
TGame_ReloadSky			Game_ReloadSky;
TGame_NewMap			Game_NewMap;
TGame_RunConsoleCommand	Game_RunConsoleCommand;
TGame_MaxTicksPerSecond Game_MaxTicksPerSecond;


bool bInitialized = false;
bool LibsInitialized()
{
	return bInitialized;
}

static string AsLower(const string& s){
	string lower = s;
	for(int i = 0; i < lower.length(); i++)
		lower[i] = tolower(lower.c_str()[i]);
	return lower;
}

bool LoadLibraries(HWND topHwnd, HWND childHwnd, HINSTANCE hInstance, string cmdLine, bool isDedicated)
{
	
#ifdef _DEBUG
	TCHAR* Game = "GAMED.DLL";
#else
	TCHAR* Game = "GAME.DLL";
#endif

	char buf[512];
	GetCurrentDirectory(512,buf);
	string path = buf;

	if(AsLower(path).find("system") == -1 || !(AsLower(path).find("system") > path.length()-9))
		SetCurrentDirectory(".\\System\\");
	// Load DLL dynamically
	HINSTANCE m_hHandle = ::LoadLibrary(Game);
	if(!m_hHandle){
		m_hHandle = ::LoadLibrary(Game);
		if(!m_hHandle){
#ifdef _DEBUG
			MessageBoxA(0,"Could not find GameD.DLL (Debug) in System\\ or current directory!",0,0);
#else
			MessageBoxA(0,"Could not find Game.DLL in System\\ or current directory!.\n Check the file exists in System\\. If it does, just copy this exe to System\\ and run from there instead.",0,0);
#endif
			return false;
		}
	}

	// Load dynamically linked routines
	Game_Initialize		= (TGame_Initialize)::GetProcAddress(m_hHandle,("Game_Initialize"));
	Game_HandleMessages = (TGame_HandleMessages)::GetProcAddress(m_hHandle,("Game_HandleMessages"));
	Game_Shutdown		= (TGame_Shutdown)::GetProcAddress(m_hHandle,("Game_Shutdown"));
	Game_Tick			= (TGame_Tick)::GetProcAddress(m_hHandle,("Game_Tick"));
	Game_Ready		    = (TGame_Ready)::GetProcAddress(m_hHandle,("Game_Ready"));

	Game_ReloadSky		    = (TGame_ReloadSky)::GetProcAddress(m_hHandle,("Game_ReloadSky"));
	Game_NewMap				= (TGame_NewMap)::GetProcAddress(m_hHandle,("Game_NewMap"));
	Game_RunConsoleCommand	= (TGame_RunConsoleCommand)::GetProcAddress(m_hHandle,("Game_RunConsoleCommand"));
    Game_MaxTicksPerSecond  = (TGame_MaxTicksPerSecond)::GetProcAddress(m_hHandle,("Game_MaxTicksPerSecond"));

	if(!Game_Initialize || !Game_HandleMessages || !Game_Shutdown || !Game_Tick || !Game_Ready || !Game_MaxTicksPerSecond){
		MessageBoxA(0,"Entrypoint not initialized. Your Game.DLL may be wrong",0,0);
		return false;
	}

    // Start the engine!
	string startMap;
    if(cmdLine.find("map:") != -1)
		startMap = cmdLine.substr(cmdLine.find("map:")+4); 
    Game_Initialize(topHwnd, childHwnd,hInstance,startMap,isDedicated);

	bInitialized = true;
	return true;
}

