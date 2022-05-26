#include "stdafx.h"


string GetText(HWND hwnd, int ID){
	HWND hItem = GetDlgItem(hwnd,ID);
	char text[256];
	GetWindowText(hItem,text,256);
	return text;
}

void SetText(HWND hwnd, string str, int ID){
	HWND hItem = GetDlgItem(hwnd,ID);
	SetWindowText(hItem,str.c_str());
}


string BrowseForDir(HWND hwnd, string title){
	char folder[MAX_PATH];
	GetCOREInterface()->ChooseDirectory(hwnd,(TCHAR*)title.c_str(),folder,NULL);
	return folder;
}



string BrowseForFile(HWND hwnd, string heading, char* filter){
	string location;
	TCHAR szFileName[MAX_PATH];
	szFileName[0] = _T('\0');

	OPENFILENAME ofn;

	memset(&ofn,0,sizeof(ofn));
	ofn.lStructSize       = sizeof(ofn);
	ofn.hwndOwner         = hwnd;
	ofn.lpstrFilter       = filter;
	ofn.nFilterIndex      = 1;
	ofn.lpstrFile         = szFileName;
	ofn.lpstrTitle		  = heading.c_str();
	ofn.nMaxFile          = sizeof(szFileName);
	ofn.Flags			  = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	if(!GetOpenFileName(&ofn))
		return "";
	location = szFileName;
	return location;
}


BOOL GetFileName(HWND hwnd, LPSTR pszFileName, BOOL bSave)
{
	OPENFILENAME ofn;

	ZeroMemory(&ofn, sizeof(ofn));
	pszFileName[0] = 0;

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = "Script Files (*.py)\0*.py\0All Files (*.*)\0*.*\0\0";
	ofn.lpstrFile = pszFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrDefExt = "py";

	if(bSave)
	{
		ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY |
			OFN_OVERWRITEPROMPT;
		if(!GetSaveFileName(&ofn))
			return FALSE;
	}
	else
	{
		ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
		if(!GetOpenFileName(&ofn))
			return FALSE;
	} return TRUE;
}


// FIXME: Why are we using this when there's a good one in shared.cpp?
void recurseDir( const char * path, vector<FileList>& fileList, bool files, const char* filter  )
{
	HANDLE handle;
	WIN32_FIND_DATA findData;

	char filespec[512];
	char newpath[512];

	//(void) strlower( path );

	strcpy( filespec, path );
	strcat(filespec,"\\");
	strcat( filespec, filter );

	findData.dwFileAttributes = 0;
	handle = FindFirstFile( filespec, &findData );

	do
	{
		if ( findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
		{
			if ( findData.cFileName[0] != '.' )
			{
				strcpy( newpath, path );
				strcat( newpath, findData.cFileName );
				strcat( newpath, "\\" );
				if(!files){
					FileList item;
					strcpy( item.name, findData.cFileName );
					strcpy( item.path, path );
					item.size = findData.nFileSizeLow;
					fileList.push_back(item);
				}

				recurseDir( newpath, fileList );
			}
		}
		else if(findData.dwFileAttributes != NULL)
		{
			if ( fileList.size() >= MAX_FILES )
			{
				printf( "Reached maximum file limit %d\n", MAX_FILES );
				return;
			}
			if(files){
				FileList item;
				strcpy( item.name, findData.cFileName );
				strcpy( item.path, path );
				item.size = findData.nFileSizeLow;
				fileList.push_back(item);
		 }
		}
	}
	while( handle && FindNextFile( handle, &findData ) );

	return;
} 



/* CREATE A TOOLTIP CONTROL OVER THE ENTIRE WINDOW AREA */
void CreateMyTooltip (HWND hwnd, const char* strTT)
{
	// struct specifying control classes to register
	INITCOMMONCONTROLSEX iccex; 
	HWND hwndTT;                 // handle to the ToolTip control
	// struct specifying info about tool in ToolTip control
	TOOLINFO ti;
	unsigned int uid = 0;       // for ti initialization
	LPTSTR lptstr = (char*) strTT;
	RECT rect;                  // for client area coordinates

	/* INITIALIZE COMMON CONTROLS */
	iccex.dwICC = ICC_WIN95_CLASSES;
	iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	InitCommonControlsEx(&iccex);

	/* CREATE A TOOLTIP WINDOW */
	hwndTT = CreateWindowEx(WS_EX_TOPMOST,
		TOOLTIPS_CLASS,
		NULL,
		WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,		
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		hwnd,
		NULL,
		g_hInstance,
		NULL
		);

	SetWindowPos(hwndTT,
		HWND_TOPMOST,
		0,
		0,
		0,
		0,
		SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	/* GET COORDINATES OF THE MAIN CLIENT AREA */
	GetClientRect (hwnd, &rect);

	/* INITIALIZE MEMBERS OF THE TOOLINFO STRUCTURE */
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_SUBCLASS;
	ti.hwnd = hwnd;
	ti.hinst = g_hInstance;
	ti.uId = uid;
	ti.lpszText = lptstr;
	// ToolTip control will cover the whole window
	ti.rect.left = rect.left;    
	ti.rect.top = rect.top;
	ti.rect.right = rect.right;
	ti.rect.bottom = rect.bottom;

	/* SEND AN ADDTOOL MESSAGE TO THE TOOLTIP CONTROL WINDOW */
	SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);	
} 



//----------------------------------------------------------------------------------
// Stop max from stealing our thunder
//----------------------------------------------------------------------------------
BOOL HandleFocus(WPARAM wParam){
	switch (HIWORD (wParam)) {
		case EN_SETFOCUS: 
			DisableAccelerators();
			return TRUE;

		case EN_KILLFOCUS: 
			EnableAccelerators();
			return TRUE;
	}
	return FALSE;
}
