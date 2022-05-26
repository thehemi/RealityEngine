//=========== Copyright (c) 2004, Tim Johnson. All rights reserved. ===========
// Created Tuesday, January 18th, 2003
// ExportInterface for Evolution engine
//
//=============================================================================
#include "stdafx.h"
#include "ExportInterface.h"
#include "GlobalMain.h"
#include "dxfile.h"
#include "globalsettings.h"
#include "resource.h"
#include "MapSettings.h"

ExportInterface theExport;
 
//----------------------------------------------------------------------------------
// Desc: Helper
//----------------------------------------------------------------------------------
void GetPathAndName(string file, string& path, string& name){
	CStr fld = file.c_str();
	int pth = fld.last('\\');
	if(fld.last('/') > pth)
		pth = fld.last('/');

	path = fld.Substr(0,pth);
	name = fld.Substr(pth+1,fld.length()-pth);
}



//----------------------------------------------------------------------------------
// Desc: GUI Save
//----------------------------------------------------------------------------------
void ExportInterface::Save(bool bCompile, ExportType type){
	// Get the path/name based on the type
	string uncompiledExt, compiledExt, uncompiledPath, compiledPath, name;

	bool saveSelected = false;

	switch(type){
		case TYPE_LEVEL:
			saveSelected = Button_GetCheck(GetDlgItem(hwnd,IDC_SAVESELECTED_LEVEL));
			uncompiledPath       = GetUncompiledLevelPath();
			//compiledPath  = GetCompiledLevelPath();
			uncompiledExt = LEVEL_UEXT;
			compiledExt   = LEVEL_EXT;
			name = GetLevelFilename();
			break;
		case TYPE_MODEL:
			saveSelected = Button_GetCheck(GetDlgItem(hwnd,IDC_SAVESELECTED_MODEL));
			uncompiledPath = GetUncompiledModelPath();
			//compiledPath  = GetCompiledModelPath();
			uncompiledExt = MODEL_UEXT;
			compiledExt   = MODEL_EXT;
			name = GetModelFilename();
			break;
		case TYPE_ANIMATION:
			saveSelected = Button_GetCheck(GetDlgItem(hwnd,IDC_SAVESELECTED_ANIMATION));
			uncompiledPath = GetAnimationPath();
			//compiledPath  = GetAnimationPath();
			uncompiledExt = ANIM_EXT;
			compiledExt   = ANIM_EXT;
			name = GetAnimationFilename();
			break;
	}

	DWORD format = DXFILEFORMAT_TEXT;
	//if(Button_GetCheck(GetDlgItem(hwnd,IDC_BINARY)))
	//	format = DXFILEFORMAT_BINARY;
	if(Button_GetCheck(GetDlgItem(hwnd,IDC_BINARYCOMPRESSED)))
		format = DXFILEFORMAT_COMPRESSED;

	bool looping = Button_GetCheck(GetDlgItem(hwnd,IDC_ISLOOPING));

	Save(hwnd,name,bCompile,uncompiledPath,type,saveSelected,false,(type==TYPE_ANIMATION),looping,format);
}

//----------------------------------------------------------------------------------
// Long save. No GUI-inputs, so can be called externally
// Calls the exporter and optionally the compiler
//----------------------------------------------------------------------------------
void ExportInterface::Save(HWND hwnd, string name, bool bCompile, string path, ExportType type, bool bSaveSelected, bool bSavePrefabs, bool bAnimation, bool bLooping, DWORD format){
	INode* root = ip->GetRootNode();
	//FindRootNode(&theGlobalMenu,&root);

	if(! (FILE_ATTRIBUTE_DIRECTORY & GetFileAttributes(path.c_str())) )
	{
		// Not a valid dir, prompt user
		if(type == TYPE_ANIMATION)
		{
			path = BrowseForDir(hwnd,"Locate directory for animations...");
			SetAnimationPath(path);
		}
		if(type == TYPE_MODEL)
		{
			path = BrowseForDir(hwnd,"Locate directory for models...");
			SetUncompiledModelPath(path);
		}
		if(type == TYPE_LEVEL)
		{
			path = BrowseForDir(hwnd,"Locate directory for maps...");
			SetUncompiledLevelPath(path);
		}
	}

	// setup dialog options
	DlgOptions.m_bSaveAnimationData = bAnimation;
	DlgOptions.m_bLoopingAnimationData = bLooping;
	//DlgOptions.sceneData = *theMap.GetSceneProperties();

	string uEXT = ".xml";
	if(type == TYPE_ANIMATION)
		uEXT = ".anm";

	// Build uncompiled file with extension
	string uFile = path;
	if(uFile[uFile.length()-1] != '\\' && uFile[uFile.length()-1] != '/')
		uFile += "\\";
	uFile += name + "\\";
	CreateDirectory(uFile.c_str(),0);
	uFile += name;
	uFile += uEXT;

	ShowWindow(hwnd,SW_HIDE); // Must hide during progress bar (3dsmax rule)
	ExportFile(uFile.c_str(),root, ip,TRUE, bSaveSelected, DlgOptions);
	ShowWindow(hwnd,SW_SHOW);

	// Compile too? ...
	/*
	if(bCompile){
		string cName, cPath;

		string compiler = GetCompilerPath();
		if(compiler.length() == 0){
			// The export pane may not be open, read config
			compiler = theConfig.GetString("CompilerPath");
			// Config is blank too, prompt user
			if(compiler.length() == 0){
				compiler = BrowseForFile(hwnd,"Locate compiler...");
				 theConfig.SetString("CompilerPath",compiler);
			}
		}
		GetPathAndName(compiler, cPath, cName );

		string options = "\"" + uFile + "\"";
		//options += " " + strOptions;
		//options += " -out:";
		string cmdFile = compiledPath;
		if(cmdFile[cmdFile.length()-1] != '\\' && cmdFile[cmdFile.length()-1] != '/')
			cmdFile += "\\";
		cmdFile += name;
		cmdFile += cEXT;

		options += " \"" + cmdFile + "\"";
		int err = (int)ShellExecute(hwnd,"open",cName.c_str(),options.c_str(),cPath.c_str(),SW_SHOWNORMAL);
		if(err <= 32){
			string str = "Couldn't open ";
			str += compiler;
			str += ", please locate it again...";
			MessageBox(hwnd,str.c_str(),"Error",MB_OK | MB_ICONSTOP);

			SetCompilerPath(BrowseForFile(hwnd,"Locate compiler..."));
			GetPathAndName(compiler, cPath, cName );
			
			int err = (int)ShellExecute(hwnd,"open",cName.c_str(),options.c_str(),cPath.c_str(),SW_SHOWNORMAL);
			if(err <= 32){
				MessageBox(hwnd,"Still couldn't open it for some reason, giving up","Error",MB_OK | MB_ICONSTOP);
			}
		}
	}*/
}

//----------------------------------------------------------------------------------
// Misc
// TODO: Remove all these, they barely save any time now 
// we have the new Get/SetText macros
//----------------------------------------------------------------------------------
string ExportInterface::GetGamePath(){
	return GetText(hwnd,IDC_GAME_LOCATION);
}

void ExportInterface::SetGamePath(string str){
	SetText(hwnd,str,IDC_GAME_LOCATION);
}

string ExportInterface::GetCompilerPath(){
	return GetText(hwnd,IDC_COMPILER_LOCATION);
}

void ExportInterface::SetCompilerPath(string str){
	//SetText(hwnd,str,IDC_COMPILER_LOCATION);
}

//----------------------------------------------------------------------------------
// Animation widget functions
//----------------------------------------------------------------------------------
string ExportInterface::GetAnimationPath(){
	return GetText(hwnd,IDC_ANIMATION_PATH);
}

void ExportInterface::SetAnimationPath(string str){
	SetText(hwnd,str,IDC_ANIMATION_PATH);
}

string ExportInterface::GetAnimationFilename(){
	return GetText(hwnd,IDC_ANIMATION_FILENAME);
}

void ExportInterface::SetAnimationFilename(string str){
	SetText(hwnd,str,IDC_ANIMATION_FILENAME);
}
//----------------------------------------------------------------------------------
// Model widget functions
//----------------------------------------------------------------------------------
string ExportInterface::GetCompiledModelPath(){
	return GetText(hwnd,IDC_COMPILED_PATH);
}

string ExportInterface::GetUncompiledModelPath(){
	return GetText(hwnd,IDC_UNCOMPILED_PATH);
}

string ExportInterface::GetModelFilename(){
	return GetText(hwnd,IDC_MODEL_FILENAME);
}

void ExportInterface::SetCompiledModelPath(string str){
	//SetText(hwnd,str,IDC_COMPILED_PATH);
}

void ExportInterface::SetUncompiledModelPath(string str){
	SetText(hwnd,str,IDC_UNCOMPILED_PATH);
}

void ExportInterface::SetModelFilename(string str){
	SetText(hwnd,str,IDC_MODEL_FILENAME);
}

//----------------------------------------------------------------------------------
// Level widget functions
//----------------------------------------------------------------------------------
string ExportInterface::GetCompiledLevelPath(){
	return GetText(hwnd,IDC_COMPILED_PATH2);
}

string ExportInterface::GetUncompiledLevelPath(){
	return GetText(hwnd,IDC_UNCOMPILED_PATH2);
}

string ExportInterface::GetLevelFilename(){
	return GetText(hwnd,IDC_LEVEL_FILENAME);
}

void ExportInterface::SetCompiledLevelPath(string str){
	//SetText(hwnd,str,IDC_COMPILED_PATH2);
}

void ExportInterface::SetUncompiledLevelPath(string str){
	SetText(hwnd,str,IDC_UNCOMPILED_PATH2);
}

void ExportInterface::SetLevelFilename(string str){
	SetText(hwnd,str,IDC_LEVEL_FILENAME);
}

//----------------------------------------------------------------------------------
// Desc: Read states from global config
//----------------------------------------------------------------------------------
BOOL ExportInterface::ReadConfig()
{
	SceneProperties* props = theMap.GetSceneProperties();

	// These are read from the scene properties:
	SetModelFilename(props->modelfile);
	SetLevelFilename(props->mapfile);

	// Misc
	SetCompilerPath(theConfig.GetString("CompilerPath"));
	SetGamePath(theConfig.GetString("GamePath"));
	// Animation
	SetAnimationFilename(theConfig.GetString("AnimationFilename"));
	SetAnimationPath(theConfig.GetString("AnimationPath"));
	// Model
	SetUncompiledModelPath(theConfig.GetString("UncompiledModelPath"));
	SetCompiledModelPath(theConfig.GetString("CompiledModelPath"));
	
	// Level
	SetUncompiledLevelPath(theConfig.GetString("UncompiledLevelPath"));
	SetCompiledLevelPath(theConfig.GetString("CompiledLevelPath"));

	Button_SetCheck(GetDlgItem(hwnd,IDC_SAVESELECTED_MODEL),theConfig.GetBool("SaveSelected"));
	Button_SetCheck(GetDlgItem(hwnd,IDC_ISLOOPING),theConfig.GetBool("IsLooping"));
	if(theConfig.GetBool("SaveBinary"))
		Button_SetCheck(GetDlgItem(hwnd,IDC_BINARYCOMPRESSED),TRUE);
	else
		Button_SetCheck(GetDlgItem(hwnd,IDC_TEXT),TRUE);

	

	return TRUE;
}

//----------------------------------------------------------------------------------
// Desc: Write states to global config
//----------------------------------------------------------------------------------
void ExportInterface::WriteConfig()
{
	SceneProperties* props = theMap.GetSceneProperties();
	// These are written to the scene properties:
	strcpy(props->modelfile,GetModelFilename().c_str());
	strcpy(props->mapfile,GetLevelFilename().c_str());

	// Misc
	//theConfig.SetString("CompilerPath",GetCompilerPath());
	theConfig.SetString("GamePath",GetGamePath());
	// Animation
	theConfig.SetString("AnimationFilename",GetAnimationFilename());
	theConfig.SetString("AnimationPath",GetAnimationPath());
	// Model
	theConfig.SetString("UncompiledModelPath",GetUncompiledModelPath());
	//theConfig.SetString("CompiledModelPath",GetCompiledModelPath());
	theConfig.SetString("ModelFilename",GetModelFilename());
	// Level
	theConfig.SetString("UncompiledLevelPath",GetUncompiledLevelPath());
	//theConfig.SetString("CompiledLevelPath",GetCompiledLevelPath());
	theConfig.SetString("LevelFilename",GetLevelFilename());

	theConfig.SetBool("SaveSelected",Button_GetCheck(GetDlgItem(hwnd,IDC_SAVESELECTED_MODEL)));
	theConfig.SetBool("IsLooping",Button_GetCheck(GetDlgItem(hwnd,IDC_ISLOOPING)));
//	theConfig.SetBool("SaveBinary",Button_GetCheck(GetDlgItem(hwnd,IDC_BINARYCOMPRESSED)));
}

//----------------------------------------------------------------------------------
// Desc: Export Dialog
//----------------------------------------------------------------------------------
BOOL CALLBACK ExportDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	string temp;
	BOOL ret;
	switch (msg) {
		case WM_INITDIALOG:{
			theExport.hwnd = hWnd;
			CenterWindow(hWnd,GetParent(hWnd));
			theExport.ReadConfig();
			return TRUE;
		}
			break;

		case WM_DESTROY:
		case WM_CLOSE:
			theExport.CloseDialog();
			break;

		case WM_NOTIFY :
			ret = HandlePropertySheet(lParam);
			if(!ret && theExport.hwnd){
				// Window is probably changing (but maybe not closing), so let's store the settings
				theExport.WriteConfig();
			}
			return ret;

		case WM_COMMAND:
			if(HandleFocus(wParam))
				return TRUE;
			switch (LOWORD(wParam)) {
				//----------------------------------------------------------------------------------
				// Misc Widgets
				//----------------------------------------------------------------------------------
				case IDC_BROWSEFORCOMPILER:
					temp = BrowseForFile(hWnd,"Locate compiler...");
					if(temp.length())
						theExport.SetCompilerPath(temp);
					break;
				case IDC_BROWSEFORGAME:
					temp = BrowseForFile(hWnd,"Locate game...");
					if(temp.length())
						theExport.SetGamePath(temp);
					break;
				//-------------------------------------
				//
				// Animation widgets
				//
				//-------------------------------------
				case IDC_SAVEANIMATION:
					theExport.Save(false,ExportInterface::TYPE_ANIMATION);
					//theExport.CloseDialog();
					break;
				case IDC_BROWSE_ANIMATION:
					temp = BrowseForDir(hWnd,"Locate game...");
					if(temp.length())
						theExport.SetAnimationPath(temp);
				break;

				case IDC_ANIMATION_PATH:
				case IDC_ANIMATION_FILENAME:
					break;

				//-------------------------------------
				//
				// Model widgets
				//
				//-------------------------------------
				case IDC_BROWSE_COMPILED:
					temp = BrowseForDir(hWnd,"Choose directory for compiled models..");
					if(temp.length())
						theExport.SetCompiledModelPath(temp);
					break;
				case IDC_BROWSE_UNCOMPILED:
					temp = BrowseForDir(hWnd,"Choose directory for uncompiled models..");
					if(temp.length())
						theExport.SetUncompiledModelPath(temp);
					break;
				case IDC_UNCOMPILED_PATH:
				case IDC_COMPILED_PATH:
				case IDC_MODEL_FILENAME:
					break;
				case IDC_SAVEMODEL:
					theExport.Save(false,ExportInterface::TYPE_MODEL);
					//theExport.CloseDialog();
					break;
				case IDC_SAVEANDCOMPILE:
					theExport.Save(true,ExportInterface::TYPE_MODEL);
					//theExport.CloseDialog();
					break;

				//-------------------------------------
				//
				// Level widgets
				//
				//-------------------------------------
				case IDC_BROWSE_COMPILED2:
					temp = BrowseForDir(hWnd,"Choose directory for compiled levels..");
					if(temp.length())
						theExport.SetCompiledLevelPath(temp);
					break;
				case IDC_BROWSE_UNCOMPILED2:
					temp = BrowseForDir(hWnd,"Choose directory for uncompiled levels..");
					if(temp.length())
						theExport.SetUncompiledLevelPath(temp);
					break;
				//----------------------------------------------------------------------------------
				// User clicks Build & Run
				//----------------------------------------------------------------------------------
				case IDC_TEST:
					{
					// Compile before testing.
					theExport.Save(true,ExportInterface::TYPE_LEVEL);

					string gName, gPath;

					if(theExport.GetGamePath().length() == 0){
						theExport.SetGamePath(BrowseForFile(hWnd, "Locate game..."));
					}
					GetPathAndName(theExport.GetGamePath(), gPath, gName );

					string cmdLine = "map:";
					cmdLine += theExport.GetLevelFilename();
					cmdLine += LEVEL_EXT;

					int err = (int)ShellExecute(hWnd,"open",gName.c_str(),cmdLine.c_str(),gPath.c_str(),SW_SHOWNORMAL);
					if(err <= 32){
						string str = "Couldn't open ";
						str += theExport.GetGamePath();
						str += ", please locate it again...";
						MessageBox(hWnd,str.c_str(),"Error",MB_OK | MB_ICONSTOP);

						theExport.SetGamePath(BrowseForFile(hWnd,"Locate game..."));
						GetPathAndName(theExport.GetGamePath(), gPath, gName );

						int err = (int)ShellExecute(hWnd,"open",gName.c_str(),cmdLine.c_str(),gPath.c_str(),SW_SHOWNORMAL);
						if(err <= 32){
							MessageBox(hWnd,"Still couldn't open it for some reason, giving up","Error",MB_OK | MB_ICONSTOP);
						}
					}
					}
					//theExport.CloseDialog();
					break;

				case IDC_UNCOMPILED_PATH2:
				case IDC_COMPILED_PATH2:
				case IDC_LEVEL_FILENAME:
					break;
				// It's a level
				case IDC_SAVELEVEL:
					theExport.Save(false,ExportInterface::TYPE_LEVEL);
					break;
				case IDC_SAVEANDCOMPILE2:
					theExport.Save(true,ExportInterface::TYPE_LEVEL);
					//theExport.CloseDialog();
					break;

			}
			break;

		default:
			break;
	}
	return FALSE;
}	

ExportInterface::ExportInterface()
{
	ip = NULL;	
	hwnd = NULL;
}

ExportInterface::~ExportInterface()
{
}

void ExportInterface::Init(){
	ip = GetCOREInterface();
}

//----------------------------------------------------------------------------------
// Desc: Closes the dialog
//----------------------------------------------------------------------------------
void ExportInterface::CloseDialog() 
{
	if(!hwnd) // We've already closed once
		return;

	WriteConfig();
	//EndDialog(hwnd,0);
	hwnd = NULL;
}