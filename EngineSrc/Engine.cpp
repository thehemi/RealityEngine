//=========== Copyright (c) 2003, Tim Johnson. All rights reserved. ===========
// Engine.cpp : Defines the entry point for the DLL application.
//
//=============================================================================
#include <crtdbg.h>
#include "stdafx.h"
#include "dxstdafx.h"
#include <iostream>
#include <stdlib.h>
#include "Log.h"
#include "ScriptSystem.h"
#include <crtdbg.h>
#include "mdump.h"
#include "..\Engine\resource.h" // IDD_ERROR
#include "shellapi.h"
#include "windowsversion.h"
#include "Graphics\TextureManager.h"
#include "Collision.h"
#include "classmap.h"
#include "Editor.h"
#include "Server.h"
#include "Client.h"
#include "WaterSurface.h"
#include "Profiler.h"
#include "GUISystem.h"
#include "LODManager.h"
#include "SkyController.h"
#include "IndoorVolume.h"
#include <shlobj.h>


MiniDumper dump("Reality Engine");

ENGINE_API float GDeltaTime = 0;
ENGINE_API float GSeconds   = 0;

ENGINE_API int   DynamicLightsRendered;
ENGINE_API int   StaticLightsRendered;


string EngineVersionInfo = _U("Engine Version info not loaded!");

EngineLog GLog;
string GAppName;
HINSTANCE GHInstance;
HANDLE gModule;

LARGE_INTEGER qwTicksPerSec;

// Hacky: Must register all engine classes when we're 100% sure _registry is loaded
map<string, Factory*> _registry;
vector<HashKey> Factory::HashKeys;

REGISTER_FACTORY(Actor);
REGISTER_FACTORY(Prefab);
REGISTER_FACTORY(Light);
REGISTER_FACTORY(WaterSurface);
REGISTER_FACTORY(SkyController);
REGISTER_FACTORY(IndoorVolume);

/// Singleton
Profiler* Profiler::Get()
{
	static Profiler p;
	return &p;
}


//-----------------------------------------------------------------------------
// DLL entrypoint
//-----------------------------------------------------------------------------
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{ 
	gModule = hModule;
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
		static bool init = false;
		if(!init){
			
		}
		init = true;
		}
	break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}

//-----------------------------------------------------------------------------
void SetExceptionHandling(bool enable)
{
    if(enable)
        ::SetUnhandledExceptionFilter( dump.TopLevelFilter );
    else
        ::SetUnhandledExceptionFilter( 0 );
}

//-----------------------------------------------------------------------------
// Mini Timer
//-----------------------------------------------------------------------------
LARGE_INTEGER	StartTime[10];
int		stack = 0;
//bool	timerStarted = false;

/// Returns MS since PC started
/*double GetMS()
{
	LARGE_INTEGER timer_count;
	QueryPerformanceCounter(&timer_count);
	return ((timer_count.QuadPart*1000.0f)/(double)qwTicksPerSec.QuadPart);
}*/

ENGINE_API void StartMiniTimer(){
	//if(!timerStarted){ DXUTTimer(TIMER_START); timerStarted = true; }
	if(stack >= 9 || stack < 0) 
		Error("Too many nested StartMiniTimer() calls");

	QueryPerformanceCounter(&StartTime[stack++]);
}

ENGINE_API float StopMiniTimer(){
	LARGE_INTEGER end;
	QueryPerformanceCounter(&end);
	return ((double)end.QuadPart - (double)StartTime[--stack].QuadPart) / (double)qwTicksPerSec.QuadPart;
}

ENGINE_API float GetGSeconds()
{
return DXUTGetGlobalTimer()->GetAbsoluteTime();
}

string g_RootPath;
//-----------------------------------------------------------------------------
/// Searches directory path structure
//-----------------------------------------------------------------------------
struct Paths {
	vector<Paths> m_SubDirectories;
	string m_Path;

	bool FindFile(string& file){
		// Check our path
		string test = (m_Path + "\\" + file);
		if( access( test.c_str(), 0) == 0){
			file = test;
			return true;
		}

		// Check all our subdirectories
		for(int i=0;i<m_SubDirectories.size();i++){
			if(m_SubDirectories[i].FindFile(file))
				return true;
		}
		return false;
	}

	// This only checks root paths, so we should strip the path specifier
	Paths* FindPath(const string& path){
		for(int i=0;i<m_SubDirectories.size();i++){
			if(CompareIgnoreCase(m_SubDirectories[i].m_Path.substr(g_RootPath.length()+1),path))
				return &m_SubDirectories[i];
		}

		return NULL;
	}
};

Paths g_Paths;

//-----------------------------------------------------------------------------
// BuildSearchPaths
//
//-----------------------------------------------------------------------------
void BuildSearchPaths(Paths* currentPath)
{
   string path = currentPath->m_Path;
   if(path.length())
	   path += "\\";

   string searchPath = path + _U("*.*");
   WIN32_FIND_DATA findData;
   HANDLE handle = FindFirstFile( searchPath.c_str(), &findData );

   do
   {
	   if ( findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && findData.cFileName[0] != '.' )
	   {
		   string dirPath = path + string(findData.cFileName);
		   // Create a new search path and enumerate it
		   Paths p;
		   p.m_Path = dirPath;
		   currentPath->m_SubDirectories.push_back(p);
		   BuildSearchPaths( &currentPath->m_SubDirectories.back() );
	   }
   }
   while( handle && FindNextFile( handle, &findData ) );
   return;
} 



//-----------------------------------------------------------------------------
// Finds a resource file at a specified location (folder)
//-----------------------------------------------------------------------------
ENGINE_API bool FindMedia(string& resource, const char* location, bool LastDirsMustMatch)
{
	if(resource.length() == 0)
		return false;
	// Must reset dir so we resolve the correct relative path
	ResetCurrentDirectory(); 
	// See if resource is found
	if(access( resource.c_str(), 0) == 0)
		return true;

	string filename = resource;

	if(!LastDirsMustMatch){
		// Strip path
		if(filename.find(_U("\\"))!=-1)
			filename = filename.substr(filename.find_last_of(_U("\\"))+1);
		else if(filename.find(_U("/"))!=-1)
			filename = filename.substr(filename.find_last_of(_U("/"))+1);
	}

	// Find the search path, then check it to see if the file exists there
	Paths* p = g_Paths.FindPath(string(location));
	if(p && p->FindFile(filename)){
		resource = filename;
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Name: Log
//-----------------------------------------------------------------------------
ENGINE_API void LogPrintf(const TCHAR *fmt, ...){
	if(Engine::Instance()->LogLevel() == LOG_OFF)
		return;

	va_list		argptr;
	TCHAR		msg[8000];
	va_start (argptr,fmt);
	vsprintf (msg,fmt,argptr);
	va_end (argptr);

	Editor::Instance()->m_Log.push_back(string(msg)+"\n");
	GLog.Out(msg);
	_tcscat(msg,_U("\r\n"));
	if(Engine::Instance()->IsDedicated()){
		printf(msg);
	}
	_tcscat(msg,_U("\t\t"));
	OutputDebugString(msg);
}

//-----------------------------------------------------------------------------
// Name: Log with level filtering
//-----------------------------------------------------------------------------
ENGINE_API void LogPrintf(LogLevel level, const TCHAR *fmt, ...){
	va_list		argptr;
	TCHAR		msg[8000];
	va_start (argptr,fmt);
	vsprintf (msg,fmt,argptr);
	va_end (argptr);

	if(level <= Engine::Instance()->LogLevel())
		LogPrintf(msg);
}

ENGINE_API void LogPopPrefix(){
	GLog.PopPrefix();
}

ENGINE_API void LogPushPrefix(const TCHAR* prefix){
	GLog.PushPrefix((TCHAR*)prefix);
}



//-----------------------------------------------------------------------------
// Name: DlgProc()
// Desc: Initial splash banner
//-----------------------------------------------------------------------------
TCHAR* errorMsg;
BOOL CALLBACK ErrorProc(HWND hdlg,UINT wm,WPARAM wp,LPARAM lp)
{
	BOOL handled=TRUE;
	switch(wm){
	case WM_CLOSE:
		EndDialog(hdlg,0);
		exit(0);
		return TRUE;
	case WM_INITDIALOG:
		SetDlgItemText (hdlg, IDC_EDIT,errorMsg );
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wp) == IDOK || LOWORD(wp) == IDCANCEL) 
		{
			EndDialog(hdlg, LOWORD(wp));
			return TRUE;
		}
		if(LOWORD(wp) == IDSENDERROR){
			//MessageBox(0,"To send an error
			//ShellExecuteA(hdlg,_U("open"),_U("Helix Tools.exe"),0,GetDir(),SW_SHOWNORMAL);
			//system("Helix Tools.exe");
			return TRUE;
		}
		return FALSE;

	default: handled=FALSE;
	}

	return(handled);
}

//-----------------------------------------------------------------------------
// This function calls GetLastError() to get the last error and converts
// the error number to a string so the user can display it.
//-----------------------------------------------------------------------------
TCHAR *GetLastErrorString (void)
{
  static LPVOID pErrorMessage;

  FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER |
		 FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
		 MAKELANGID (LANG_ENGLISH, SUBLANG_ENGLISH_US),
		 (LPTSTR)&pErrorMessage, 0, NULL);

  return (TCHAR *)pErrorMessage;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ShowErrorProc(const TCHAR* err){
	string sErr = err;
	findandreplace(sErr,_U("\n"),_U("@#"));
	findandreplace(sErr,_U("@#"),_U("\r\n"));
	errorMsg = (TCHAR*)sErr.c_str();

	// Create and show Splash Banner for display during loading
	int ret = DialogBox((HINSTANCE)gModule,MAKEINTRESOURCE(IDD_ERROR),Engine::Instance()->hWnd,ErrorProc);
	
	if(ret == -1)
		LogPrintf(_U("ERROR STRING: %s"),GetLastErrorString());
}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
ENGINE_API HRESULT Error(const TCHAR *fmt, ...){
	static int errorInProgress = 0;
	static string sMsg = "IMPOSSIBLE - FLAW IN ERROR HANDLER";
	static bool errorShown = false;

	static TCHAR		msg[8000];

	if(errorShown){
		SendMessage(Engine::Instance()->hWnd,WM_SYSCOMMAND,SC_CLOSE,0);
		exit(0);
	}

	LogPrintf(_U("Entering Error().. (%dst time)"),(int)errorInProgress);


	if(errorInProgress){
		int * p = NULL ;
        *p = 0 ; // Raise another exception to exit out of this mess
		return 0;
	}
	errorInProgress++;

	if(Editor::Instance()->GetEditorMode() && Editor::Instance()->m_World)
	{
		ResetCurrentDirectory();
		BOOL ret = CreateDirectory("..\\Maps\\Autorecover\\",NULL);
		ERROR_ALREADY_EXISTS;
		ERROR_PATH_NOT_FOUND;
		Editor::Instance()->SaveScene("..\\Maps\\Autorecover\\Autorecover.xml");
	}
	
	/*if(errorInProgress){
		// Another error occured trying to shut down from the last error.
		// At this point it is safe to call exit() without fully shutting down everything else

		// We display the error again since it probably wasn't displayed last time
		if(!errorShown)
			MessageBox(0,sMsg.c_str(),"2nd fatal error",0);
			//ShowErrorProc(sMsg.c_str());
		exit(0);
	}*/
	
	// IF this function itself crashes
	// it's probably because a string with c_str() or invalid object was passed
	
	
	if(!Engine::Instance()->IsDedicated())
		while(ShowCursor(TRUE)<0);


	va_list		argptr;
	va_start (argptr,fmt);
	vsprintf (msg,fmt,argptr);
	va_end (argptr);


	sMsg = EngineVersionInfo;
	if(!Engine::Instance()->IsDedicated()){
		sMsg += _U("\nGfx card: ");
		sMsg += RenderDevice::Instance()->GetDeviceString();
	}

	sMsg += _U("\n-------\n");
	sMsg += msg;
	if(Editor::Instance()->GetEditorMode())
		sMsg += "\nYour scene has been recovered and saved in 'Maps\\AutoRecover'";

	// MiniDumper adds the proper stack trace. So only force a stack dump
	// if it hasn't caught an exception and created the proper stack
	if(!MiniDumper::ExceptionInProgress){
		TCHAR pc[4000];
		PrintStackToString(pc,4000);

		sMsg += _U("\n\nStack Trace:\n");
		sMsg += pc;
	}

	try{
		LogPrintf(sMsg.c_str());
		//Engine::Instance()->Shutdown();
	}
	catch(...){
		sMsg+= _U("\n(Also, another error occured trying to shut down the engine or print to the log)");
	}

	if(Engine::Instance()->IsDedicated()){
		// 1. Open again with -crashed
		/*TCHAR strExePath[MAX_PATH];
		GetModuleFileName( NULL, strExePath, MAX_PATH );
		string param = GetCommandLine();
		if(param.find("-wait") == -1)
			param += " -wait";
		HINSTANCE hInst = ShellExecute( NULL, "open", strExePath , param.c_str(), NULL, SW_SHOW);
		if ((UINT) hInst < 32)
			MessageBox(0,"Game crashed, and it couldn't be relaunched for some reason. Please report this to tim@helixcore.com",0,0);
			*/
		// 2. Close game
		exit(0);
		//cout << "Error was fatal. Press any key to exit.." << endl;
		//getchar();
	}
	else{
		while(ShowCursor(TRUE)<0);
		ShowErrorProc(sMsg.c_str());
		errorShown = true;

		try{
			SendMessage(Engine::Instance()->hWnd,WM_SYSCOMMAND,SC_CLOSE,0);
			exit(0);
		}
		catch(...){
			exit(0);
		}
	}

	return S_OK;
};


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
ENGINE_API void SeriousWarning(const TCHAR *fmt, ...){
	va_list		argptr;
	TCHAR		msg[8000];
	va_start (argptr,fmt);
	vsprintf (msg,fmt,argptr);
	va_end (argptr);

    if(Engine::Instance()->IsDedicated())
    {
        LogPrintf(msg);
        return;
    }


	MessageBoxA(Engine::Instance()->hWnd,msg,"Serious Warning",MB_ICONSTOP);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
ENGINE_API void Warning(const TCHAR *fmt, ...){
	va_list		argptr;
	TCHAR		msg[8000];
	va_start (argptr,fmt);
	vsprintf (msg,fmt,argptr);
	va_end (argptr);

	string sMsg = _U("WARNING: ");
	sMsg += msg;

	if(Engine::Instance()->MainConfig->GetBool("SuppressWarnings") || Engine::Instance()->IsDedicated()){
		LogPrintf(sMsg.c_str());
		return;
	}

	// Routing through error system for fullscreen handling for now
	Error(sMsg.c_str());

	//LogPrintf(msg);

	//MessageBox(0,msg,"Warning",0);

};

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
ENGINE_API void Debug(const TCHAR *fmt, ...){
#ifdef _DEBUG
	va_list		argptr;
	TCHAR		msg[8000];
	
	va_start (argptr,fmt);
	vsprintf (msg,fmt,argptr);
	va_end (argptr);
	Warning(msg);
#endif
}


//-----------------------------------------------------------------------------
// Engine version resource
//-----------------------------------------------------------------------------
string GetFileVersionString(LPCTSTR lpszFilePath)
{
    BOOL bOK = FALSE;
    DWORD dwZero=0;
    DWORD dwVerSize = GetFileVersionInfoSize((LPTSTR)(LPCTSTR)lpszFilePath, &dwZero);


	if(dwVerSize == 0)
		Error(_U("Couldn't get version info from %s"),lpszFilePath);
    
	TCHAR* pData = new TCHAR[dwVerSize];

    if(GetFileVersionInfo((LPTSTR)(LPCTSTR)lpszFilePath, dwZero, dwVerSize, pData))
    {
		UINT nVersionValueLength;
		TCHAR* buf;
    /*    bOK = ::VerQueryValue(pData, "\\StringFileInfo\\%04X%04X\\FileVersion", (void**)&buf, &nVersionValueLength);
		if(!bOK)
			Error("VerQueryValue failed (was trying to get the engine dll version): %s",GetLastErrorString());
    */

		struct LANGANDCODEPAGE
		{
			WORD wLanguage;
			WORD wCodePage;
		} lpTranslate;

		string tmp;

		VS_FIXEDFILEINFO* lpvi;
		UINT iLen;
		//Read the Language ID & CodePage info
		bOK = VerQueryValue(pData, _U("\\VarFileInfo\\Translation"), (VOID **)&lpvi, &nVersionValueLength);
		if(!bOK) Error("VerQueryValue failed (was trying to get the engine dll version): %s",GetLastErrorString());

		lpTranslate = *(LANGANDCODEPAGE*)lpvi;

		//Now build a base string for getting the Version Info
		TCHAR LangID[128];
		sprintf(LangID,_U("%04x%04x"),lpTranslate.wLanguage, lpTranslate.wCodePage);
		string BaseString = _U("\\StringFileInfo\\") +ToStr(LangID);

		//Append the desired field from the version info to the base string,  get
		//the version info & display it
		tmp = BaseString + _U("\\FileVersion");
		bOK = VerQueryValue(pData, (LPSTR)tmp.c_str(), (LPVOID*)&buf, &iLen);
		if(!bOK || buf[0] == '\0' ) 
			Error(_U("2nd VerQueryValue failed (was trying to get the engine dll version): %s"),GetLastErrorString());
		
		string ret = buf;

		delete[] pData;

		return ret;
	}
	else
		Error(_U("GetFileVersionInfo failed with: %s"),GetLastErrorString());

    return "ERROR";
}

//-----------------------------------------------------------------------------
// Engine Ctor
//-----------------------------------------------------------------------------
Engine::Engine()
{	
	logLevel = LOG_HIGH;
	ResetCurrentDirectory();
    ConfigManager = 0;
}

//-----------------------------------------------------------------------------
// Engine initialization
//-----------------------------------------------------------------------------
void Engine::Initialize(ConfigFile& cfg, HWND topHwnd, HWND childHwnd, HINSTANCE AppHInst, const TCHAR* AppName, bool dedicated, string cmdLine){
	// Use QueryPerformanceFrequency() to get frequency of timer.  
    (QueryPerformanceFrequency( &qwTicksPerSec ) != 0);
	qwTicksPerSec.QuadPart /= 1000.0f;

	dedicatedMode = dedicated;
	/*_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG)|
                   _CRTDBG_LEAK_CHECK_DF);

    //dump to debugger
    _CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_DEBUG );
	_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG );
	_CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_DEBUG );*/
 
	//_CrtSetBreakAlloc(62487);
	//_CrtSetBreakAlloc(0x9a29130);
	//_CrtSetBreakAlloc(0x9a28f98);

	// Ensure appropraite default dirs exist
	CreateDirectory("..\\Materials\\",NULL);

	g_RootPath = cfg.GetString("SearchPath");
	g_Paths.m_Path = g_RootPath;
	BuildSearchPaths(&g_Paths);
	TopHwnd = topHwnd;
	hWnd = childHwnd;
	GHInstance = AppHInst;
	GAppName = AppName;
	GLog.Init((TCHAR*)(string(AppName) + string(_U(".log"))).c_str());

	MainConfig = new ConfigFile;
	*MainConfig = cfg;
	logLevel = (::LogLevel) MainConfig->GetInt("LogLevel");
	if(logLevel > LOG_HIGH)
		logLevel = LOG_HIGH;

#ifdef _DEBUG
	EngineVersionInfo = GetFileVersionString(_U("EngineD.dll"));
	LogPrintf("Engine (Debug mode): %s",EngineVersionInfo.c_str());
#else
	EngineVersionInfo = GetFileVersionString(_U("Engine.dll"));
	LogPrintf("Engine (Release mode): %s",EngineVersionInfo.c_str());
#endif

	LogPushPrefix(_U("Init:\t"));
	LogPrintf(getOSString().c_str());


	//--- INSTALL CUSTOM FONTS ---
	string font1 = MainConfig->GetString("CustomFont");
	if(FindMedia(font1,"System")){
		if(!AddFontResource(font1.c_str()))
			Error(_U("Installing the font %s failed."),font1.c_str());
		
		SendMessageTimeout(HWND_BROADCAST, WM_FONTCHANGE, 0, 0,SMTO_NORMAL,500,NULL);
	}

    SafeMode = false;

#ifndef _DEBUG // Safe mode is annoying when debugging
 WCHAR wszPath[MAX_PATH];
    if(!Engine::IsDedicated())
    {
        // Check for initialization flag from previous launch.
        // We use SHGetFolderPath with CSIDL_LOCAL_APPDATA to write to
        // a subdirectory of Documents and Settings\<username>\Local Settings\Application Data.
        // It is the best practice to write user-specific data under the user profile directory
        // because not every user on the system will always have write access to the application
        // executable directory.
        ::GetModuleFileNameW( NULL, wszPath, MAX_PATH );
        HRESULT hr = ::SHGetFolderPathW( NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, wszPath );
        if( SUCCEEDED( hr ) )
        {
            // Create the directory for this application. We don't care
            // if the directory already exists as long as it exists after
            // this point.
            lstrcatW( wszPath, L"\\ConfigSystem" );
            CreateDirectoryW( wszPath, NULL );

            // Check for launch.sta
            lstrcatW( wszPath, L"\\Launch.sta" );
            if( ::GetFileAttributesW( wszPath ) != INVALID_FILE_ATTRIBUTES )
            {
                // Launch.sta exists.  Most likely previous launch did not succeed.
                // Prompt the user to enter safe mode.
                if( IDYES == ::MessageBoxW( NULL, L"The application failed to initialize during "
                                                L"the previous launch.  It is recommended that "
                                                L"you run the application in safe mode.\n\n"
                                                L"Do you wish to run in safe mode?",
                                                L"ConfigSystem", MB_YESNO|MB_ICONQUESTION ) )
                    SafeMode = true;
            }
            else
            {
                // Create Launch.sta in the executable folder.
                HANDLE hFile = ::CreateFileW( wszPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
                if( hFile != INVALID_HANDLE_VALUE )
                    ::CloseHandle( hFile );
            }
        }
    }
#endif

	InputSys = 0;
	AudioSys = 0;
	RenderSys = 0;

	collider.Initialize();
	LODManager::Instance()->Initialize();
	// Python engine
	ScriptSystem::Instance()->Initialize();
	// Networking
	Client::Instance()->Initialize();
	Server::Instance()->Initialize();

	if(!dedicatedMode){
		if(cfg.GetString("RenderDevice") == "D3DRenderDevice"){
			RenderSys	= RenderDevice::Instance();
			LogPrintf("Initializing Rendering System: D3DRenderDevice");
			RenderSys->Initialize(AppHInst,hWnd,cmdLine);

			// PURE EVIL
			// HACK FOR RESIZE BUG
			//if(Editor::Instance()->GetEditorMode())
			//{
			//DXUTReset3DEnvironment();
			//}
		}

		if(cfg.GetString("AudioDevice") == "DXAudioSubsystem"){
			AudioSys	= AudioDevice::Instance();
			LogPrintf("Initializing Audio System: DXAudioSubsystem");
			AudioSys->Initialize(hWnd);
		}

		if(cfg.GetString("Input") == "DirectInput"){
			InputSys	= Input::Instance();
			LogPrintf("Initializing Input System: DirectInput");
			InputSys->Initialize(cfg,topHwnd,0);
		}

		if(RenderSys)
			GUISystem::Instance()->Initialize();
	}


	PhysicsEngine::Instance()->Initialize();

	LogPopPrefix();
	
	//Mostafa: Scripting
	ScriptEngine::Instance()->Intialize();

#ifndef _DEBUG
       // Program completed.  Remove Launch.sta
		if(!Engine::IsDedicated())       
			::DeleteFileW( wszPath );
#endif
}



Engine* Engine::Instance () 
{
    static Engine inst;
    return &inst;
}

//-----------------------------------------------------------------------------
// Engine tick
//-----------------------------------------------------------------------------
void Engine::Update(Camera* ActiveCamera){
	GDeltaTime = DXUTGetGlobalTimer()->GetAbsoluteTime() - GSeconds;
	GSeconds   = DXUTGetGlobalTimer()->GetAbsoluteTime();

	if(GDeltaTime > 0.5f)
		GDeltaTime = 0.5f;
	if(GDeltaTime < 0.001f)
		GDeltaTime = 0.001f;

	if(!InputSys) // No subsystems to update
		return;

	StartMiniTimer();
	if(InputSys)
		InputSys->Update();
	if(RenderSys)
		RenderSys->Update(ActiveCamera);

    Profiler::Get()->SubsystemMS += StopMiniTimer();
    if(AudioSys)
		AudioSys->Update(ActiveCamera);
}


//-----------------------------------------------------------------------------
// Engine shutdown
//-----------------------------------------------------------------------------
void Engine::Shutdown(){
	if(!InputSys)
		return; // Already shutdown

	LogPrintf(_U("\nEngine shutting down..."));

	if(RenderSys){
		GUISystem::Instance()->Shutdown();
		RenderSys->Shutdown();
		RenderSys = NULL;
	}

	// Hide window so errors can be displayed if fullscreen
	// Must only hide after killing rendersystem, so we don't release cycles
	// to the damn video texture thread
	ShowWindow(hWnd,SW_HIDE); 

	GLog.Exit();
	if(InputSys){
		InputSys->Shutdown();
		InputSys = NULL;
	}

	if(AudioSys){
		AudioSys->Shutdown();
		AudioSys = NULL;
	}
	

    //Mostafa: Scripting
	ScriptEngine::Instance()->Shutdown();
	ScriptSystem::Instance()->Shutdown();
	PhysicsEngine::Instance()->Destroy();

	//--- REMOVE CUSTOM FONTS ---
	string font1 = MainConfig->GetString("CustomFont");
	if(FindMedia(font1,"System")){
		RemoveFontResource(font1.c_str());
		SendMessageTimeout(HWND_BROADCAST, WM_FONTCHANGE, 0, 0,SMTO_NORMAL,500,NULL);
	}

	delete MainConfig;
	//assert(_CrtCheckMemory());
	LogPrintf("Engine Shutdown Complete!");
}

LRESULT Engine::HandleMessages( HWND hWnd, UINT uMsg, WPARAM wParam,
					LPARAM lParam ){

	if(!InputSys || !RenderSys)
		return DefWindowProc(hWnd,uMsg,wParam,lParam);

	InputSys->MsgProc(hWnd,uMsg,wParam,lParam);
	return RenderSys->MsgProc(hWnd,uMsg,wParam,lParam);
}


Engine::~Engine(){
	//if(RenderSys || AudioSys || InputSys)
	//	Error("There was an unexpected error.");
}


