//=========== Copyright (c) 2004, Tim Johnson. All rights reserved. ===========
//
// ExportInterface for Reality engine
//
//=============================================================================
#ifndef __EVOEXPORT__H
#define __EVOEXPORT__H


class ExportInterface {
public:
	ExportInterface();
	~ExportInterface();

	void ShowDialog();
	void CloseDialog();
	void Init();

	Interface *ip;
	HWND hwnd; // Dialog panel
	SaveOptions DlgOptions;
	void UpdateDlgOptions();

	// Config
	void WriteConfig();
	BOOL ReadConfig();

	enum ExportType{
		TYPE_MODEL,
		TYPE_LEVEL,
		TYPE_ANIMATION,
	};

	void Save(HWND hwnd,string name, bool bCompile, string path, ExportType type, bool saveSelected, bool bSavePrefabs=false, bool bAnimation=false, bool bLooping=true, DWORD format = DXFILEFORMAT_COMPRESSED);
	void Save(bool bCompile, enum ExportType type);

	// Misc
	void SetGamePath(string str);
	string GetGamePath();
	void SetCompilerPath(string str);
	string GetCompilerPath();

	// Animation
	void SetAnimationPath(string str);
	string GetAnimationPath();
	void SetAnimationFilename(string str);
	string GetAnimationFilename();


	// Model
	void SetUncompiledModelPath(string str);
	string GetUncompiledModelPath();
	void SetCompiledModelPath(string str);
	string GetCompiledModelPath();
	string GetModelFilename();
	void SetModelFilename(string str);

	// Level
	void SetUncompiledLevelPath(string str);
	string GetUncompiledLevelPath();
	void SetCompiledLevelPath(string str);
	string GetCompiledLevelPath();
	string GetLevelFilename();
	void SetLevelFilename(string str);
};

extern ExportInterface theExport;

#endif // __EvoExport__H
