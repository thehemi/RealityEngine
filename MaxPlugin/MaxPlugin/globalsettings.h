//=========== Copyright (c) 2004, Tim Johnson. All rights reserved. ===========
//
// Uses paramblock2 to hold global settings in a .max file
//
//=============================================================================
#pragma once

#define	DUMMYTVN_CLASSID	Class_ID(0x2a751a0e, 0x319a6e1e)
#define PBLOCK_REFNO	0
#define TVN_PBLOCK_REFNO 0


/*	RefTargetHandle GetReference(int i) {return i==PBLOCK_REFNO? pblock:NULL;}
	void SetReference(int i, RefTargetHandle rtarg) {if(i==PBLOCK_REFNO) pblock=(IParamBlock2*)rtarg;}*/
//===========================================================================
//
// Class GlobalSettings
// An ITrackViewObject which serves as a "latch";
// It holds the paramblock for the utility, serving as the utility's link to the reference hierarchy
//===========================================================================
class GlobalSettings : public ITrackViewNode
{
public:		
	IParamBlock2 *pblock; //a reference to the the SuperUtility instance's ParamBlock pointer
	INode *node;

	//***************************************************************************
	// Implementation-specific methods
	GlobalSettings();
	bool InitGlobalSettings();
	bool ClearGlobalSettings();

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

	//From Animatable
	Class_ID ClassID() {return DUMMYTVN_CLASSID;}
	SClass_ID SuperClassID() {return REF_MAKER_CLASS_ID;}

	void DeleteThis() {delete this;}

	// Specifiy whether you want the node to be visible or not
	BOOL BypassTreeView() {return FALSE;}

	int	NumParamBlocks() {return 1;}
	IParamBlock2* GetParamBlock(int i) {return i==0? pblock:NULL;}
	IParamBlock2* GetParamBlockByID(short id) {return id==PBLOCK_REFNO? pblock:NULL;}

	int NumSubs()  { return 1; }
	Animatable* SubAnim(int i) { return i==0? pblock:NULL; }
	//TSTR SubAnimName(int i);  // get name of ith subanim

	//From ReferenceMaker...
	IOResult Load(ILoad *iload);
	IOResult Save(ISave *isave);

	// Reference handling (to notify if mesh is updated and needs recompiling)
	vector<ReferenceTarget*> m_References;
	void			SetReference(int i, RefTargetHandle rtarg);
	RefTargetHandle GetReference(int i);
	int				NumRefs();
	RefResult		NotifyRefChanged(class Interval cpc , RefTargetHandle hTarget, unsigned long & PartID,unsigned int _Message);
};

extern GlobalSettings* theGlobalSettings;