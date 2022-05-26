/**********************************************************************
TIM: Deprecated. See NVMax DllEntry
 **********************************************************************/
#include "stdafx.h"

//extern ClassDesc2* GetGlobalMenuDesc();
//extern ClassDesc2* GetDummyTVNDesc();
//extern ClassDesc2* GetSimpleCustAttribDesc();

//HINSTANCE hInstance;
//int controlsInit = FALSE;
ConfigFile theConfig;


void Error(const char *fmt, ...){
	va_list		argptr;
	char		msg[8000];

	va_start (argptr,fmt);
	vsprintf (msg,fmt,argptr);
	va_end (argptr);

	string sMsg = "Error: ";
	sMsg += msg;
	sMsg += "\n";

	MessageBox(0,sMsg.c_str(),"Error",MB_ICONSTOP);
}

void Warning(const char *fmt, ...){
	va_list		argptr;
	char		msg[8000];

	va_start (argptr,fmt);
	vsprintf (msg,fmt,argptr);
	va_end (argptr);

	string sMsg = "Warning: ";
	sMsg += msg;
	sMsg += "\n";

	MessageBox(0,sMsg.c_str(),"Warning",MB_ICONSTOP);
}

// Dummy function so we can use ConfigFile
bool ResolvePath(string& str, bool LastDirectoriesMustReallyMatch){
	return true;
}

