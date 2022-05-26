/**********************************************************************
DESCRIPTION:	Class for the prefab interface
**********************************************************************/

#ifndef __PREFABINTERFACE__H
#define __PREFABINTERFACE__H


class PrefabInterface {
public:
	PrefabInterface();
	~PrefabInterface();

	void ShowDialog();
	void CloseDialog();
	void Init();

	Interface *ip;
	HWND hwnd; // Dialog 

	//  settings
	int		 curFolder;
	int		 curFile;
	string   rootFolder;

	static void SelectionSetChanged(void* param, NotifyInfo* info);
	void SavePrefab();
	void MakePrefab(string filename, INode* node);
	vector<FileList> folders;
	vector<FileList> files;
	void PopulateFolders();
	void PopulateFiles(int i);
	void FileSelected(int index);
	void CommitSettings();
	void GetSettings();
};

extern PrefabInterface thePrefab;

#endif // __PREFABINTERFACE__H