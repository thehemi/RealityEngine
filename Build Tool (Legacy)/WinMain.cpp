// Helix Core.cpp : Defines the entry point for the application.
//
#include "stdafx.h"
#include "BuildTool.h"
#include <mmsystem.h>
#include "windowsx.h"
#include <objbase.h>
#include <string>
#include "commdlg.h"
#include "commctrl.h"
#include "richedit.h"
#include "resource.h"

void SetDlgItemUrl(HWND hdlg,int id,const char *url); 


using namespace std;
#define MAX_LOADSTRING 100

#define INPUT_STRING    "Uncompiled maps (*.xml) \0*.xml\0Uncompiled models (*.mdu) \0*.mdu\0\0"
#define OUTPUT_MAP		"Maps (*.mpc) \0*.mpc\0\0"
#define OUTPUT_MODEL    "Models (*.mdc) \0*.mdc\0\0"

// Global Variables:
string cmdLine;
HWND gHwnd;
BuildTool build;
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK SplashBannerProc(HWND hdlg,UINT wm,WPARAM wp,LPARAM lp);

#define PostCommand(hd,id,h,cmd) PostMessage \
	(hd,WM_COMMAND, \
	GET_WM_COMMAND_MPS(id,h,cmd))

// when posting a menu selection, hwnd=0 & cmd=0
#define PostMenu(hd,id) PostCommand(hd,id,0,0)

// when posting a button click, hwnd=hwnd of control
#define PostButton(hd,id) PostCommand(hd,id, \
	GetDlgItem(hd,id),BN_CLICKED)



void SetProgress(float percent){
	int progress = percent * 10; // Progress bar is 0 to 1000
	SendMessage(GetDlgItem(gHwnd,IDC_PROGRESS), PBM_SETPOS, (WPARAM) progress, 0); 

	// Use this as an opportunity to
	// process windows messages while deep in processing
	MSG msg;
	while(PeekMessage(&msg,gHwnd,0,0,PM_REMOVE)){
		if(!IsDialogMessage(gHwnd,&msg)){
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

string GetText(int ID){
	HWND hItem = GetDlgItem(gHwnd,ID);
	char text[125000];
	GetWindowText(hItem,text,125000);
	return text;
}

void SetText(int ID, string str){
	HWND hItem = GetDlgItem(gHwnd,ID);
	SetWindowText(hItem,str.c_str());
}

// Allows us to keep at the end of the edit box
int line = 1;
void SelectLine(HWND hwndEdit, int Line)
{
	// Start of line to select
	int StartPos = SendMessage(hwndEdit, EM_LINEINDEX, Line, 0);

	// Start of next line
	int EndPos = SendMessage(hwndEdit, EM_LINEINDEX, Line+1, 0);
	// End of line to select
	// (the line ends with "\r\n", so set the end of the
	// selection just before "\r\n"
	EndPos -= 2;

	// Set the selection
	// The caret will be at the end of the selected line
	SendMessage(hwndEdit, EM_SETSEL, StartPos, EndPos);
	// If you want the caret to be at the start of the line
	// use the folowing code instead:
	// SendMessage(hwndEdit, EM_SETSEL, EndPos, StartPos);

	// Ensure the caret is visible
	SendMessage(hwndEdit, EM_SCROLLCARET, 0, 0);
	SetFocus(hwndEdit);
}


// Unused.
// Doesn't work for multiple selections
// Makes sense, since SetText() refills buffer. Need to use RTF format
// Bah, not worth it
/*void ColorWord(int start, int end, int colour)
{
	UINT flags = SCF_SELECTION ;
	CHARFORMAT2 cf ;
	cf.cbSize = sizeof(cf) ;
	//cf.dwMask = CFM_COLOR ; 
	cf.dwMask = CFM_BOLD;
	cf.dwEffects = rand()%2 == 1?CFE_BOLD:0;
	cf.crTextColor = colour ;
	//SendMessage(GetDlgItem(gHwnd,IDC_STATUS), EM_SETSEL, start, end);
	CHARRANGE cr;
	cr.cpMax = end;
	cr.cpMin = start;
	SendMessage(GetDlgItem(gHwnd,IDC_STATUS),EM_EXSETSEL,0,(LPARAM)(CHARRANGE *)&cr);

	LRESULT ret = SendMessage(GetDlgItem(gHwnd,IDC_STATUS), EM_SETCHARFORMAT, flags, (long)&cf) ;
}*/



void AddToPane(string str){
	// Turn \n into \r\n
	for(int i=0;i<str.length();i++){
		if(str[i] == '\n'){
			str.insert(i,"\r");
			i++;
			line++;
		}
	}

	string oldStr = GetText(IDC_STATUS);
	SetText(IDC_STATUS,oldStr + str);

	// Move to the end of the edit box
	SelectLine(GetDlgItem(gHwnd,IDC_STATUS),line);

	// Use this as an opportunity to
	// process windows messages while deep in processing
	MSG msg;
	while(PeekMessage(&msg,gHwnd,0,0,PM_REMOVE)){
		if(!IsDialogMessage(gHwnd,&msg)){
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

//-----------------------------------------------------------------------------
// Name: AssociateFileTypes()
// Desc: Allows the application to associate files with itself
// For example associating a map file means you can directly open the map
// instead of launching the application
//-----------------------------------------------------------------------------
/*#define MYCLASS_NAME    "EvoBuildTool"
#define MYCLASS_DESC    "Evolution Build Tools"
#define COMMANDKEY      "\\shell\\open\\command"

void AssociateFileTypes(HINSTANCE hinstance){
	// Only associate on first run (if there's no log)
	FILE* f = fopen(LOG_NAME,"r");
	if(!f)
		return;
	fclose(f);

	// Format:
	// root\.ext = filetypename
	// root\filetypename = desc
	// root\filetypename\DefaultIcon = "somepath",0
	// root\filetypename\shell\open\command = exe path

	//class name
	char filePath[300];
	GetModuleFileName(hinstance,filePath,sizeof(filePath));

	char buffer[300];
	sprintf(buffer,"\"%s\"",filePath);

	strcat(buffer,"\"%1\""); // Command-line option to trigger compile

	RegSetValue(HKEY_CLASSES_ROOT,MYCLASS_NAME"\\shell\\open\\command",REG_SZ,buffer,strlen(buffer));
	RegSetValue(HKEY_CLASSES_ROOT,MYCLASS_NAME,REG_SZ,MYCLASS_DESC,strlen(MYCLASS_DESC));

	// Register each file type this application can launch
	RegSetValue(HKEY_CLASSES_ROOT,".MPU",REG_SZ,MYCLASS_NAME,strlen(MYCLASS_NAME));
	RegSetValue(HKEY_CLASSES_ROOT,".MDU",REG_SZ,MYCLASS_NAME,strlen(MYCLASS_NAME));

	// How identify icon resource?
	char newBuf[300];
	sprintf(newBuf,"%s,%d",filePath,0);
	RegSetValue(HKEY_CLASSES_ROOT,MYCLASS_NAME"\\DefaultIcon",REG_SZ,newBuf,strlen(newBuf));
}*/



string Browse(string heading, const char* filter, bool isSave, const char* defExt = NULL){
	string location;
	TCHAR szFileName[MAX_PATH];
	szFileName[0] = _T('\0');

	OPENFILENAME ofn;
 
	memset(&ofn,0,sizeof(ofn));
	ofn.lpstrDefExt		  = defExt;
	ofn.lStructSize       = sizeof(ofn);
	ofn.hwndOwner         = gHwnd;
	ofn.lpstrFilter       = filter;
	ofn.nFilterIndex      = 1;
	ofn.lpstrFile         = szFileName;
	ofn.lpstrTitle		  = heading.c_str();
	ofn.nMaxFile          = sizeof(szFileName);
	ofn.Flags			  = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;
	if(isSave)
		GetSaveFileName(&ofn);
	else
		GetOpenFileName(&ofn);

	location = szFileName;
	return location;
}

void BrowseMulti(string heading, const char* filter, vector<string>& files){
	string location;
	TCHAR szFileName[MAX_PATH*200]; // Allow up to 200 files
	szFileName[0] = _T('\0');

	OPENFILENAME ofn;

	memset(&ofn,0,sizeof(ofn));
	//ofn.lpstrDefExt		  = defExt;
	ofn.lStructSize       = sizeof(ofn);
	ofn.hwndOwner         = gHwnd;
	ofn.lpstrFilter       = filter;
	ofn.nFilterIndex      = 1;
	ofn.lpstrFile         = szFileName;
	ofn.lpstrTitle		  = heading.c_str();
	ofn.nMaxFile          = sizeof(szFileName);
	ofn.Flags			  = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER | OFN_ALLOWMULTISELECT;
	GetOpenFileName(&ofn);

	string path = ofn.lpstrFile;
	path += "\\";
	while(*ofn.lpstrFile!=NULL)
	{        
		ofn.lpstrFile+=lstrlen(ofn.lpstrFile)+1;
		if(*ofn.lpstrFile!=NULL)
			files.push_back(path + ofn.lpstrFile);
	}
}

bool isModel;

//----------------------------------------------------------------------------------
// Desc: Lets user choose input file. Auto-saves extension
// Fills output box with most likely output filename
//----------------------------------------------------------------------------------
void LocateInput(){
	string file = Browse("Locate file to build...",INPUT_STRING,false);

	if(file.length() == 0)
		return;

	build.model = file.find(".mdu") != -1;

	SetText(IDC_INPUT,file);

	// Set output box with most likely filename+path
	string outFile = file.substr(0,file.find_last_of("."));
	if(build.model)
		outFile += ".mdc";
	else
		outFile += ".mpc";
	SetText(IDC_OUTPUT, outFile);

	build.inFile = file;
	build.outFile = outFile;
}

//----------------------------------------------------------------------------------
// Desc: Lets user choose output file. Picks right extension, and updates box
//----------------------------------------------------------------------------------
void SelectOutput(){
	string file;
	
	if(build.model)
		file = Browse("Save as...",OUTPUT_MODEL,true,".mdc");
	else
		file = Browse("Save as...",OUTPUT_MAP,true,".mpc");

	if(file.length() == 0)
		return;

	SetText(IDC_OUTPUT,file);

	build.outFile = file;
}

//----------------------------------------------------------------------------------
// Desc: Loads widgets from BuildTool initial settings
//----------------------------------------------------------------------------------
void LoadWidgets(){
	SetText(IDC_OUTPUT,build.outFile);
	SetText(IDC_INPUT,build.inFile);
}

BOOL CALLBACK BuildDlgProc
(
 HWND hWnd,
 UINT message,
 WPARAM wParam,
 LPARAM lParam
 ) 
{
	gHwnd = hWnd;

	switch(message) 
	{
	case WM_INITDIALOG:
		SendMessage(GetDlgItem(hWnd,IDC_PROGRESS), PBM_SETSTEP, (WPARAM) 1, 0); 
		SendMessage(GetDlgItem(hWnd,IDC_PROGRESS), PBM_SETRANGE, 0, MAKELPARAM(0, 1000)); 

		SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON)));
		SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON)));

		build.Initialize(hWnd, cmdLine);
		SetText(IDC_NODESIZE,ToStr(build.RenderNodeSize));
		LoadWidgets();
		SetDlgItemUrl(hWnd,IDC_HYPERLINK,"http://www.helixcore.com/cgi-bin/twiki/bin/view/Main/PluginDocumentation");
		SetDlgItemUrl(hWnd,IDC_EMAIL,"mailto:tim@helixcore.com?subject=Compiler Problem?&body=Please attach the file you were trying to compile, along with the compiler log and any other relevant information");
		// Post instead of call, so INIT gets a chance to complete
		if(cmdLine.size())
			PostButton(hWnd,IDC_BUILD);

		Button_SetCheck(GetDlgItem(hWnd,IDC_ENABLEPRT),TRUE);
		return TRUE;

	
	case WM_DESTROY:
		build.Shutdown();
		PostQuitMessage(0);
		return TRUE;

	case WM_QUIT:
	case WM_CLOSE:
		build.Shutdown();
		DestroyWindow(hWnd);
		return TRUE;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
			/*
		case IDC_CONFIGUREPRT:
		{
			CPRTOptionsDlg dlg;
            bool bResult = dlg.Show();
		}
		*/
		case IDC_QUIT:
			build.Shutdown();
			DestroyWindow(hWnd);
			break;
		case IDC_BROWSEINPUT:
			LocateInput();
			break;
		case IDC_BROWSEOUTPUT:
			SelectOutput();
			break;
		case IDC_BUILD:
			if(GetText(IDC_INPUT).length() == 0)
				LocateInput();
			if(GetText(IDC_OUTPUT).length() == 0)
				SelectOutput();

			build.RenderNodeSize = atoi(GetText(IDC_NODESIZE).c_str());
			build.m_PRTEnabled   = Button_GetCheck(GetDlgItem(hWnd,IDC_ENABLEPRT));

			if(build.Build(GetText(IDC_INPUT),GetText(IDC_OUTPUT))){
				bool closeOnSuccess = Button_GetCheck(GetDlgItem(hWnd,IDC_EXITONSUCCESS));
				if(closeOnSuccess)
					PostMessage(hWnd, WM_QUIT,0,0);
			}
			break;
		case IDC_BATCHBUILD:
			{
				build.RenderNodeSize = atoi(GetText(IDC_NODESIZE).c_str());
				build.m_PRTEnabled   = Button_GetCheck(GetDlgItem(hWnd,IDC_ENABLEPRT));

				vector<string> files;
				BrowseMulti("Select file(s)...",INPUT_STRING,files);
				// Build all files...
				for(int i=0;i<files.size();i++){
					build.model = files[i].find(".mdu") != -1;
					// Set output box with most likely filename+path
					string outFile = files[i].substr(0,files[i].find_last_of("."));
					if(build.model)
						outFile += ".mdc";
					else
						outFile += ".mpc";

					if(build.Build(files[i],outFile)){
						bool closeOnSuccess = Button_GetCheck(GetDlgItem(hWnd,IDC_EXITONSUCCESS));
						if(closeOnSuccess)
							exit(0);
					}
				}
			}
			
			break;


		default:
			break;
		}
	}
	return FALSE;
}

int APIENTRY WinMain(HINSTANCE hInstance,
					 HINSTANCE hPrevInstance,
					 LPTSTR    lpCmdLine,
					 int       nCmdShow)
{
	hInst = hInstance;
	cmdLine = lpCmdLine;
	// Associate file types with application
	// AssociateFileTypes(hInstance);

	HMODULE hModule = LoadLibrary("riched20.dll") ;
	if(!hModule)
		MessageBox(0,"Couldn't load riched20.dll",0,0);

	// Initialize global strings
	LoadString(hInstance, IDS_COMPILER, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDS_COMPILER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	//AssociateFileTypes(hInstance);

	CreateDialog(hInstance,MAKEINTRESOURCE(IDD_BUILD),NULL,BuildDlgProc);
	ShowWindow(gHwnd,SW_SHOW);

	/*DialogBoxParam(hInstance, 
		MAKEINTRESOURCE(IDD_BUILD), 
		NULL, 
		BuildDlgProc, 
		(LPARAM)lpCmdLine);*/

	MSG msg;

	// Now we're ready to recieve and process Windows messages.
	BOOL bGotMsg;
	msg.message = WM_NULL;
	PeekMessage( &msg, NULL, 0U, 0U, PM_NOREMOVE );
	while( WM_QUIT != msg.message  )
	{
		
		// Use PeekMessage() if the app is active, so we can use idle time to
		// render the scene. Else, use GetMessage() to avoid eating CPU time.
		//if( Game_Ready() )
		//	bGotMsg = PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE );
		//else
			bGotMsg = GetMessage( &msg, NULL, 0U, 0U );

		//if(IsDialogMessage(msg.hwnd,&msg))
		//	continue;

		if( bGotMsg )
		{
			// Translate and dispatch the message
			//if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

	FreeLibrary(hModule);
	//CoUninitialize();
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
	wcex.lpfnWndProc	= (WNDPROC)BuildDlgProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_ICON);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(BLACK_BRUSH);//(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCTSTR)IDS_COMPILER;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_ICON);

	return RegisterClassEx(&wcex);
}