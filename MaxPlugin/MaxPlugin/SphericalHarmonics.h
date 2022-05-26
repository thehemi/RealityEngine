//=========== Copyright (c) 2004, Tim Johnson. All rights reserved. ===========
//
// Interface to assign spherical harmonics data to meshes or mesh groups
//
//=============================================================================
#pragma once
#ifndef __SPHERICALHARMONICS__H
#define __SPHERICALHARMONICS__H

#define TRACKER_CLASS_ID Class_ID(0x590f67de, 0x35fc6770)


class SphericalHarmonics {
public:
	void SaveNodeSettings();

	SphericalHarmonics();
	~SphericalHarmonics(){}

	void ShowDialog();
	void CloseDialog();
	void Init();

	ExclList inBlockers, outBlockers; // Indoor/outdoor blocker lists
	Interface *ip;
	HWND hwnd;	
	ISpinnerControl *spin;
	bool m_bWindowClosed;
	bool m_bRefreshedGUI;

	bool NodeSelected();
	void WriteConfig();
	static void SelectionSetChanged(void *param, NotifyInfo *info);
	void ProcessSelectedNodes();
	void EditBlockers(bool indoors);
	void UpdateGlobalProperties(NodeData& data);
	void EnableCustomUI(bool bEnable);

	vector<INode*> curNodes;
	vector<NodeData> curData;

	// Reference handling (to notify if mesh is updated and needs recompiling)
	/*vector<ReferenceTarget*> m_References;
	void			SetReference(int i, RefTargetHandle rtarg);
	RefTargetHandle GetReference(int i);
	int				NumRefs();
	RefResult		NotifyRefChanged(class Interval cpc , RefTargetHandle hTarget, unsigned long & PartID,unsigned int _Message);
*/
	SClass_ID		SuperClassID() {return REF_MAKER_CLASS_ID;}
	Class_ID 		ClassID() {return TRACKER_CLASS_ID;}
	virtual			TCHAR* GetName(){ return _T("Node Tracker");}


	//***************************************************************************
	// Derived methods
	//From ITrackViewNode...
	void AddNode(ITrackViewNode *node, TCHAR *name, Class_ID cid, int pos=TVNODE_APPEND){}
	void AddController(Control *c, TCHAR *name, Class_ID cid, int pos=TVNODE_APPEND) {}
	int FindItem(Class_ID cid) {return 0;}
	void RemoveItem(int i) {}
	void RemoveItem(Class_ID cid) {}
	Control *GetController(int i) {return NULL;}
	Control *GetController(Class_ID cid) {return NULL;}
	ITrackViewNode *GetNode(int i) {return NULL;}
	ITrackViewNode *GetNode(Class_ID cid) {return NULL;}
	int NumItems() {return 0;}
	void SwapPositions(int i1, int i2) {}

	TCHAR *GetName(int i) { return _T("DummyTVN"); }
	void SetName(int i,TCHAR *name) {}

	void RegisterTVNodeNotify(TVNodeNotify *notify) {DebugPrint("Register\n");}
	void UnRegisterTVNodeNotify(TVNodeNotify *notify) {}
	void HideChildren(BOOL chide) {}
};

extern SphericalHarmonics theSH;

#endif //__SPHERICALHARMONICS__H