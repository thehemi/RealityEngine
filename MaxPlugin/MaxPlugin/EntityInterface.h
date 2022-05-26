//=========== Copyright (c) 2004, Tim Johnson. All rights reserved. ===========
//
// A utility to assign game classes to max objects
//
//=============================================================================
#pragma once
#ifndef __ENTITYINTERFACE__H
#define __ENTITYINTERFACE__H

class EntityInterface{
public:

	void AssignToNode(INode* node, string theClass);

	EntityInterface();
	~EntityInterface(){}

	void ShowDialog();
	void CloseDialog();
	void Init();

	Interface *ip;
	HWND hwnd;	

	ISpinnerControl *spin;
	int curClassSelection;

	bool NodeSelected();

	static void SelectionSetChanged(void *param, NotifyInfo *info);

	void UpdateClass(int iIndex);
	void SetScriptsFolder();
	void LoadScript();
	void GetAppData();
	void PutAppData();
	void EditScript();
	void CreateObject(bool spawnAtCenter = false);
	void ProcessScripts(string folder);
	void ProcessSelectedNodes();
	void PopulateListView(NodeData& cd);
	void LoadAllScripts(bool reload);
	int  GetClassIndex(const char* name);

	vector<INode*> curNodes;
	string scriptsFolder;

	vector<NodeData> classes;
};

extern EntityInterface theEntityInterface;

#endif //__ENTITYINTERFACE__H