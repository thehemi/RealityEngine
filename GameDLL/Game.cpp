//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
/// DLL entrypoints for exe hooks
/// Interface used by shell exe to control game
///
/// Author: Tim Johnson
//====================================================================================
#include "stdafx.h"
#include "Game.h"
#include "GameEngine.h"

//--------------------------------------------------------------------------------------
/// Global module handle
HANDLE gModule;

//--------------------------------------------------------------------------------------
BOOL APIENTRY DllMain( HANDLE hModule, 
					  DWORD  ul_reason_for_call, 
					  LPVOID lpReserved
					  )
{
	gModule = hModule;
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		gModule = gModule;
		break;
	case DLL_PROCESS_DETACH:
		gModule = gModule;
		break;
		break;
	}
	return TRUE;
}

//--------------------------------------------------------------------------------------
LRESULT Game_HandleMessages( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ){
	return g_Game.HandleMessages(hWnd,uMsg,wParam,lParam);
}

//--------------------------------------------------------------------------------------
void Game_Shutdown(){
	g_Game.Shutdown();
	//	#ifdef _DEBUG
	//	_CrtDumpMemoryLeaks();
	//	#endif
}

//--------------------------------------------------------------------------------------
void Game_Initialize(HWND topHwnd, HWND childHwnd, HINSTANCE hInst, string cmdLine, bool isDedicated){
	g_Game.Initialize(topHwnd, childHwnd,hInst,cmdLine,isDedicated);
}

//--------------------------------------------------------------------------------------
void Game_Tick(){g_Game.Tick();}

//--------------------------------------------------------------------------------------
bool Game_Ready(){
	return TRUE;
};

//--------------------------------------------------------------------------------------
void Game_NewMap(const char* mapname)
{
	Server::Instance()->BeginHosting("Reality Builder",mapname,false);
}

//--------------------------------------------------------------------------------------
void Game_RunConsoleCommand(string CommandLine)
{
	string a = g_Game.m_Console.shell.ExecuteSymbol(CommandLine);
	if(!a.size())
		a = "Unknown Command.";
	g_Game.m_Console.Printf(a.c_str());
}

//--------------------------------------------------------------------------------------
int Game_MaxTicksPerSecond(){
	return 0; 
}

//--------------------------------------------------------------------------------------
float GetSeconds(){
	return GSeconds;
}

//--------------------------------------------------------------------------------------
float GetDeltaTime(){
	return GDeltaTime;
}