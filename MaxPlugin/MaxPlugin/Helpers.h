#pragma once

#define LEVEL_UEXT ".xml"
#define LEVEL_EXT  ".xml"
#define MODEL_UEXT ".xml"
#define MODEL_EXT ".xml"
#define ANIM_EXT ".anm"

void SetText(HWND hwnd, string str, int ID);
string GetText(HWND hwnd, int ID);
string BrowseForDir(HWND hwnd, string title);
string BrowseForFile(HWND hwnd, string heading, char* filter = "Program Files\0*.exe\0" );


// File
#define MAX_FILES 500
void trimLeft(string &value);
void trimRight(string &value);

typedef struct FileListTag {
	char name[512];
	char path[512];
	long size;
} FileList;

void recurseDir( const char * path, vector<FileList>& fileList, bool files = true, const char* filter = "*.*" );
void remove_whitespace (char *the_string) ;
void remove_quotes (char *the_string);

void CreateMyTooltip (HWND hwnd, const char* strTT);
void SetDlgItemUrl(HWND hdlg,int id,const char *url);

extern HWND sheetHwnd;
BOOL HandlePropertySheet(LPARAM lParam, bool* bEnabled = NULL);

BOOL HandleFocus(WPARAM wParam);