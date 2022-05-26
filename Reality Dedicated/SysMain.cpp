// Helix Core.cpp : Defines the entry point for the application.
//
#include "stdafx.h"
#include "SysMain.h"
#include <objbase.h>
#include <string>
using namespace std;
#define MAX_LOADSTRING 100
#include "..\Stub\Stub.h"
#include <stdlib.h>
#include <tchar.h>

#define Max(x,y) (x>y?x:y)

void shutdown(){
	Game_Shutdown();
	CoUninitialize();
}


int main(int argc, _TCHAR* argv[]){
	string startMap;

    string commands;
	if(argc > 1){
		// Parse optional -flags
		for(int i=1;i<argc;i++){
            commands += argv[i];
            commands += " ";
			if(string(argv[i]).find("map:")!=-1){
				startMap = string(argv[i]).substr(string(argv[i]).find(":")+1);
			}
			if(string(argv[i]).find("-wait")!=-1){
				Sleep(5000);
			}
		}
	}

	// FIXME: This isn't getting called soon enough. Other dlls are unwinding and releasing
	// COM
	atexit(shutdown);

    CoInitialize(0); // So GetDXVer works
	//CoInitializeEx( NULL, COINIT_MULTITHREADED );

	char szWindowTitle[256];
	GetConsoleTitle(szWindowTitle, 128);
	// Get the handle to the console window.
	HWND hwndConsole = FindWindow(NULL, szWindowTitle);

	LoadLibraries(hwndConsole,hwndConsole,NULL,commands,true);

	int MaxTickRate = Game_MaxTicksPerSecond();

	float OldTime;
	while(true){
		//OldTime = GetSeconds();
		//DWORD time = timeGetTime();
		Game_Tick();

		// NOTE: NOT sleeping the deltaTime amount as it's making the framerates MORE
		// erratic (WHY???)
		//float secsTaken = (timeGetTime()-time)/1000.f;

		// Sleep while we wait for the next tick
		if( MaxTickRate>0.0 )
		{
			double Delta = (1.0/MaxTickRate);// - secsTaken;
			Sleep( Delta*1000.f );//Max(10,Delta*1000.f) );
		}
	}

	return 0;
}


