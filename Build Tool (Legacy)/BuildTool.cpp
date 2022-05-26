// MapCompiler.cpp : Defines the entry point for the console application.
//
// Pipeline:
// Importer class
// SIMPLE File data structure class

// Exporter class
// Easy question:
// How would the exporter stand up to outputting multiple files, model files, or other filetypes??
#include "stdafx.h"
#include <MMSystem.h>
#include "BuildTool.h"
#include "resource.h"
#include "windowsx.h"
#include "XML\XMLImport.h"
  
 
void AddToPane(string str);

// Warning/Error logging stuff
ofstream logFile;

HRESULT InitD3D(HWND hWnd);
void CleanupD3D();

vector<string> errors;
vector<string> warnings;

extern HWND gHwnd;

void Error(const char *fmt, ...){
	va_list		argptr;
	char		msg[8000];
	
	va_start (argptr,fmt);
	vsprintf (msg,fmt,argptr);
	va_end (argptr);

	string sMsg = "Error: ";
	sMsg += msg;
	sMsg += "\n";

	MessageBox(gHwnd,msg,"Build Error",MB_ICONSTOP);
	AddToPane(sMsg);

	errors.push_back(sMsg);

	logFile << sMsg;
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

	AddToPane(sMsg);

	warnings.push_back(sMsg);
	logFile << sMsg;
}

void LogPrintf(const char *fmt, ...){
	va_list		argptr;
	char		msg[8000];

	va_start (argptr,fmt);
	vsprintf (msg,fmt,argptr);
	va_end (argptr);

	AddToPane(msg);

	logFile << msg;
	OutputDebugString(msg);
}

extern HINSTANCE hInst;

string GetLocation(string in){
	char filePath[300];
	GetModuleFileName(hInst,filePath,sizeof(filePath));
	string str = filePath;
	str = str.substr(0,str.find_last_of("\\")+1);
	str += in;
	return str;
}


void BuildTool::Initialize(HWND hWnd, string cmdLine)
{
	RenderNodeSize  = 100; // 500M
	strips			= false;
	logFile.open(GetLocation(LOG_NAME).c_str());

	ifstream config(GetLocation(CONFIG_NAME).c_str());
	if(config){
		config >> inFile;
		config >> outFile;
		bool quitWhenDone;
		config >> quitWhenDone;
		Button_SetCheck(GetDlgItem(hWnd,IDC_EXITONSUCCESS),quitWhenDone);
	}
	config.close();

	// Parse arguments..
	if(cmdLine.find(".m") != -1){
		inFile = "";
		outFile = "";
		LogPrintf("Command Line: %s\n",cmdLine.c_str());

		// Parse optional -flags
		if(cmdLine.find("-nostrips") != -1)
			strips = false;

		if(cmdLine.find("-nodesize")!=-1){
			string size = cmdLine.substr(cmdLine.find("-nodesize:")+1);
			if(size.length()){
				RenderNodeSize = strtod(size.c_str(),0);
			}
			else{
				Warning("There must not be a space in -nodesize:size. Ignoring!");
			}
		}

		model = cmdLine.find(".mdu") != -1;

		// No quotes around filename, perhaps we can guess where to insert them
		if(cmdLine.find("\"") == -1 && (cmdLine.find(":\\") !=-1))
		{
			int start = cmdLine.find(":\\") !=-1;
			int end = cmdLine.find(".mdu");
			if(end == -1)
				end = cmdLine.find(".xml");
			end += 4;

			cmdLine.insert(end,"\"");
			cmdLine.insert(start-1,"\"");
		}

		// Extract input/output file based on extension. Filenames must be in "quotes"
		if(cmdLine.find(".xml\"")!= -1 || cmdLine.find(".xml\"") != -1){
			inFile = cmdLine.substr(0,cmdLine.find("u\"")+1);
			inFile = inFile.substr(inFile.find("\"")+1);
		}
		if(cmdLine.find(".mdc\"") !=-1 || cmdLine.find(".mpc\"") != -1){
			outFile = cmdLine.substr(0,cmdLine.find("c\"")+1);
			outFile = outFile.substr(outFile.find_last_of("\"")+1);
		}

		if(inFile.length() == 0){
			Error("No valid input file specified! Remember to use \"quotes\" around input and output filenames, and only files with the correct extensions will be parsed\n");
		}
		// If outfile is not set, use input filename + extension
		else if(outFile.length() == 0){
			if(model)
				outFile = inFile.substr(0,inFile.find_last_of(".")) + ".mdc";
			else
				outFile = inFile.substr(0,inFile.find_last_of(".")) + ".mpc";
		}
	}

	LogPrintf("Initializing D3D9...");
	InitD3D(hWnd);
	LogPrintf("done.\n");

	// Did we get a valid command line input? If so, build it!
	//if(inFile.length() && cmdLine.length())
	//	Build(inFile,outFile);
}

void BuildTool::Shutdown(){
	logFile.close();
	ofstream config(GetLocation(CONFIG_NAME).c_str());
	if(config){
		config << inFile << endl;
		config << outFile << endl;
		config << (bool)Button_GetCheck(GetDlgItem(gHwnd,IDC_EXITONSUCCESS)) << endl;
	}
	config.close();
	CleanupD3D();
}

bool BuildTool::Build(string _inFile, string _outFile){
	warnings.resize(0);
	errors.resize(0);

	inFile	= _inFile;
	outFile = _outFile;
	LogPrintf("--- Importing ---\n");
	DWORD start = timeGetTime();
	bool success;

	SetProgress(0);
	XMLSystem xml;
	
	if(!xml.Load(inFile)){
		SetProgress(0);
		return false;
	}
	SetProgress(20);

	float importTime = ((timeGetTime() - start) / 1000.f);
	LogPrintf("--- Importing took %f seconds\n",(timeGetTime() - start) / 1000.f);
	LogPrintf("--- Compiling ---\n");
	start = timeGetTime();

/*	ModelFile modelFile;
	LevelFile levelFile;
	Compiler compiler(strips,RenderNodeSize);
	if(model)
		compiler.CompileModelFile(&xImport.sceneRoot,modelFile);
	else
		compiler.CompileLevelFile(&xImport.sceneRoot,levelFile);


	LogPrintf("--- Compiling took %f seconds\n",(timeGetTime() - start) / 1000.f);

	LogPrintf("--- Exporting ---\n");

	Exporter e;

	if(model)
		e.ExportModelFile(outFile,modelFile);
	else
		e.ExportLevelFile(outFile,levelFile);

	SetProgress(100);

	if(warnings.size()||errors.size()){
		LogPrintf("\nThere were errors or warnings, please review them.\n");
		//for(int i=0;i<warnings.size();i++)
		//	LogPrintf("%s\n",warnings[i].c_str());
		//for(int i=0;i<errors.size();i++)
		//	LogPrintf("%s\n",errors[i].c_str());
	}

	LogPrintf("--- Completed with %d warnings and %d errors\n", warnings.size(), errors.size());
*/
#ifdef _DEBUG
	return 0; // always make window stay open
#else
	return (errors.size()+warnings.size() == 0);
#endif
}



