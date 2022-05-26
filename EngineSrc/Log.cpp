#include "stdafx.h"
#include "Log.h"
#include <cstdarg>
#include <fstream>
using namespace std;



ofstream file;
string prefix;
int prefixLength[20];
int prefixCount = 0;

bool initialized = false;

void EngineLog::Init(char* FileName){
    if(Engine::Instance()->IsDedicated())
        file.open(FileName,ios::app);
    else
	    file.open(FileName);
}

void EngineLog::Exit(){
	file.close();
}

void EngineLog::PushPrefix(char* aPrefix){
	if(prefixCount >= 19)
		return;

	prefix += aPrefix;
	prefixLength[prefixCount++] = strlen(aPrefix);
}

void EngineLog::PopPrefix(){
	if(prefixCount == 0)
		return;

	prefix = prefix.substr(0,prefix.length() - prefixLength[--prefixCount]);
}

void EngineLog::Out (const char *fmt, ...)
{
	char msg[8000];
	va_list		argptr;

	va_start (argptr,fmt);
	vsprintf (msg,fmt,argptr);
	va_end (argptr);
	file << prefix << msg << endl;
	file.flush();
	//engine.gfx.console.Out(msg);
}
