// Cry Havoc.cpp : Defines the entry point for the application.
//
#define _WIN32_WINNT 0x0501
#include <windows.h>
#include "resource.h"
#include <string>
using namespace std;

#define MAX_LOADSTRING 100
//#include "..\GameDLL\Game.h"

//#define SHOW_SPLASH

// Library entrypoints for static linking
typedef void ( *TGame_Initialize)(HWND hwnd, HWND childHwnd, HINSTANCE hInst, string cmdLine, bool isDedicated);
typedef void ( *TGame_Shutdown)();
typedef LRESULT ( *TGame_HandleMessages)(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
typedef void ( *TGame_Tick)();
typedef bool ( *TGame_Ready)();

TGame_Initialize		Game_Initialize;
TGame_Shutdown			Game_Shutdown;
TGame_HandleMessages	Game_HandleMessages;
TGame_Tick				Game_Tick;
TGame_Ready				Game_Ready;

// Global Variables:
HINSTANCE hInst;								// current instance
HWND GMainWindowHWND;
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK SplashBannerProc(HWND hdlg,UINT wm,WPARAM wp,LPARAM lp);
 
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	HACCEL hAccel;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_HELIXCORE, szWindowClass, MAX_LOADSTRING);
	if(!MyRegisterClass(hInstance)){
		return FALSE;
	}

#ifdef _DEBUG
	TCHAR* Game = "GAMED.DLL";
#else
	TCHAR* Game = "GAME.DLL";
#endif

	BOOL b = SetCurrentDirectory(".\\System\\");
	// Load DLL dynamically
	HINSTANCE m_hHandle = ::LoadLibrary(Game);
	if(!m_hHandle){
		m_hHandle = ::LoadLibrary(Game);
		if(!m_hHandle){
#ifdef _DEBUG
			MessageBox(0,"Could not find or load GameD.DLL (Debug) in System\\ or current directory!\n If the file is there it may be an incorrect version or may have thrown an internal error during initialization, the .log file in System may have more information",0,0);
#else
			MessageBox(0,"Could not find or load Game.DLL in System\\ or current directory!.\n If the file is there it may be an incorrect version or may have thrown an internal error during initialization, the .log file in System may have more information",0,0);
#endif
			return 0;
		}
	}
 
	// Load dynamically linked routines
	Game_Initialize		= (TGame_Initialize)::GetProcAddress(m_hHandle,("Game_Initialize"));
	Game_HandleMessages = (TGame_HandleMessages)::GetProcAddress(m_hHandle,("Game_HandleMessages"));
	Game_Shutdown		= (TGame_Shutdown)::GetProcAddress(m_hHandle,("Game_Shutdown"));
	Game_Tick			= (TGame_Tick)::GetProcAddress(m_hHandle,("Game_Tick"));
	Game_Ready		    = (TGame_Ready)::GetProcAddress(m_hHandle,("Game_Ready"));

	if(!Game_Initialize || !Game_HandleMessages || !Game_Shutdown || !Game_Tick || !Game_Ready){
		MessageBox(0,"Entrypoint not initialized. Your Game.DLL may be wrong",0,0);
		return 0;
	}


	// Perform application initialization:
	if (!InitInstance (hInstance, SW_HIDE)) 
	{
		return FALSE;
	}

    /// CoInitializeEx does not work with GetDXVer
	CoInitialize(0);//CoInitializeEx( NULL, COINIT_MULTITHREADED );

	// Create and show Splash Banner for display during loading
#ifdef SHOW_SPLASH
	HWND dlg = CreateDialog(hInstance,
            MAKEINTRESOURCE(IDD_LOADING),0,
            SplashBannerProc);
	ShowWindow(dlg, SW_SHOW); // Splash Banner
	UpdateWindow(dlg);
#endif

	// Associate file types with application
	// AssociateFileTypes(hInstance);
	hAccel = LoadAccelerators(hInstance, (LPCTSTR)IDC_HELIXCORE);

    // Start the engine!
    string cmdLine = lpCmdLine; 
    Game_Initialize(GMainWindowHWND,GMainWindowHWND,hInstance,cmdLine,false);

	
	// Hide banner
#ifdef SHOW_SPLASH
	 DestroyWindow(dlg);
#endif


	ShowWindow(GMainWindowHWND,nCmdShow);

	// Now we're ready to receive and process Windows messages.
    bool bGotMsg;
    MSG  msg;
    msg.message = WM_NULL;
    PeekMessage( &msg, NULL, 0U, 0U, PM_NOREMOVE );

    while( WM_QUIT != msg.message  )
    {
        // Use PeekMessage() so we can use idle time to render the scene. 
        bGotMsg = ( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) != 0 );

        if( bGotMsg )
        {
            // Translate and dispatch the message
            if( hAccel == NULL || msg.hwnd == NULL || 
                0 == TranslateAccelerator( msg.hwnd, hAccel, &msg ) )
            {
                TranslateMessage( &msg );
                DispatchMessage( &msg );
            }
        }
        else
        {
            if( !Game_Ready() )
            {
                // Yield some CPU time to other processes
                Sleep( 100 ); 
            }

             // Update the engine during idle time (no messages are waiting)
			Game_Tick();
        }
    }

	FreeLibrary(m_hHandle);
	CoUninitialize();
	return (int) msg.wParam;
}


//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_HELIXCORE);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(BLACK_BRUSH);//(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCTSTR)IDC_HELIXCORE;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_HELIXCORE);

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HANDLE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	hInst = hInstance; // Store instance handle in our global variable

	// Set the window's initial style
	DWORD    m_dwWindowStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | 
		WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_VISIBLE;

	hWnd = CreateWindow(szWindowClass, szTitle, m_dwWindowStyle,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	GMainWindowHWND = hWnd;

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}






//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message) 
	{
	case WM_CREATE:
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam); 
		wmEvent = HIWORD(wParam); 
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_CLOSE:
        Game_Shutdown();
        // This is the 'proper' way to force quit without causing any exceptions
		SendMessage(hWnd,WM_SYSCOMMAND,SC_CLOSE,0);
		exit(0);
        break;

	case WM_QUIT:
	case WM_DESTROY:
		Game_Shutdown();
		PostQuitMessage(0);
		break;
	}

	// Let the engine process some messages
	return Game_HandleMessages(hWnd,message,wParam,lParam);
}

// Message handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}


//-----------------------------------------------------------------------------
// Name: DlgProc()
// Desc: Initial splash banner
//-----------------------------------------------------------------------------
BOOL CALLBACK SplashBannerProc(HWND hdlg,UINT wm,WPARAM wp,LPARAM lp)
{
	RECT r;
	HWND h;
    BOOL handled=TRUE;
	switch(wm){
	case WM_CLOSE:
		EndDialog(hdlg,0);
		PostQuitMessage(0);
		break;

	case WM_INITDIALOG:
		h = GetDesktopWindow();
		GetWindowRect(h,&r);
		
		// Banner sizing and positioning - must be changed to match resource size
		SetWindowPos(hdlg,0,(r.right/2)-(540/2),(r.bottom/2)-(180/2),540,180, SWP_NOSIZE);
		break;
	default: handled=FALSE;
	}
	return(handled);
}



//-----------------------------------------------------------------------------
// Name: AssociateFileTypes()
// Desc: Allows the application to associate files with itself
// For example associating a map file means you can directly open the map
// instead of launching the application
//-----------------------------------------------------------------------------
/*
#define MYCLASS_NAME    "HelixCore"
#define MYCLASS_DESC    "Cry Havoc"
#define COMMANDKEY      "\\shell\\open\\command"

void AssociateFileTypes(HINSTANCE hinstance){

	// Format:
	// root\.ext = filetypename
	// root\filetypename = desc
	// root\filetypename\DefaultIcon = "somepath",0
	// root\filetypename\shell\open\command = exe path

	//class name
	char filePath[300];
	GetModuleFileName(hinstance,filePath,sizeof(filePath));
	Log::Out("Path is: %s", filePath);

	char buffer[300];
	sprintf(buffer,"\"%s\"",filePath);

	strcat(buffer,"-map %1"); // Command-line option to trigger map load

	RegSetValue(HKEY_CLASSES_ROOT,MYCLASS_NAME"\\shell\\open\\command",
			REG_SZ,buffer,strlen(buffer));
	RegSetValue(HKEY_CLASSES_ROOT,MYCLASS_NAME,
			REG_SZ,MYCLASS_DESC,strlen(MYCLASS_DESC));

	// Register each file type this application can launch

	RegSetValue(HKEY_CLASSES_ROOT,".MSH",REG_SZ,MYCLASS_NAME,
				strlen(MYCLASS_NAME));


	// How identify icon resource?
	char newBuf[300];
	sprintf(newBuf,"%s,%d",filePath,0);
	RegSetValue(HKEY_CLASSES_ROOT,MYCLASS_NAME"\\DefaultIcon",REG_SZ,newBuf,
				strlen(newBuf));
}

*/