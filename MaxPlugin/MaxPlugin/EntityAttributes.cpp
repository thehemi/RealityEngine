//=========== Copyright (c) 2004, Tim Johnson. All rights reserved. ===========
//
// Handles the data assignment of entity arributes
//
// TODO: Get 'Reference' clone working, or find a way to disable it from viewport options
//=============================================================================
#include "stdafx.h"
#include "CustAttrib.h"
#include "ICustAttribContainer.h"
#include "GlobalSettings.h"

#define SIMPLE_CLASS_ID Class_ID(0x4c45146a, 0x68114b4a)

class SimpleCustAttrib : public CustAttrib
{
public:
	HWND		 hPanel;
	char*		 m_Data;
	int			 m_DataSize;

	SimpleCustAttrib();
	~SimpleCustAttrib();

	virtual RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID,  RefMessage message){return REF_SUCCEED;}

	int	NumParamBlocks() { return 0; }					// return number of ParamBlocks in this instance
	IParamBlock2* GetParamBlock(int i) { return NULL; } // return i'th ParamBlock
	IParamBlock2* GetParamBlockByID(BlockID id) { return NULL; } // return id'd ParamBlock

	int NumRefs() { return 0;}
	virtual RefTargetHandle GetReference(int i) { return NULL; }
	virtual void SetReference(int i, RefTargetHandle rtarg){}

	virtual	int NumSubs()  { return 0; }
	virtual	Animatable* SubAnim(int i) { return NULL; }
	virtual TSTR SubAnimName(int i){ return "";} 

	void BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev);
	void EndEditParams(IObjParam *ip, ULONG flags, Animatable *next);
	SClass_ID		SuperClassID() {return CUST_ATTRIB_CLASS_ID;}
	Class_ID 		ClassID() {return SIMPLE_CLASS_ID;}

	ReferenceTarget *Clone(RemapDir &remap = NoRemap());
	virtual bool CheckCopyAttribTo(ICustAttribContainer *to) { return true; }

	virtual TCHAR* GetName(){ return "Entity Attributes";}
	void DeleteThis() { delete this;}

	//From ReferenceMaker...
	IOResult Load(ILoad *iload);
	IOResult Save(ISave *isave);
};

// This Node will also allow loading and saving of the data stored in the class
class SimpleCustAttribClassDesc:public ClassDesc2 {
public:
	BOOL NeedsToSave(){ return TRUE; }
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return new SimpleCustAttrib();}
	const TCHAR *	ClassName() {return _T("Entity Attributes");}
	SClass_ID		SuperClassID() {return CUST_ATTRIB_CLASS_ID;}
	Class_ID		ClassID() {return SIMPLE_CLASS_ID;}
	const TCHAR* 	Category() {return GetString(IDS_CLASS_CATEGORY);}
	HINSTANCE		HInstance()	{ return g_hInstance; }
};

static SimpleCustAttribClassDesc theSimpleCustAttribClassDesc;
ClassDesc2* GetSimpleCustAttribDesc() {return &theSimpleCustAttribClassDesc;}

//-------------------------------------------------------
// Auto-called on scene save
//-------------------------------------------------------
#define CHUNK_ENTITYDATA 0
IOResult SimpleCustAttrib::Save(ISave *isave)
{
	ULONG numBlocksWritten;
	isave->BeginChunk(CHUNK_ENTITYDATA);
	isave->Write(m_Data,m_DataSize,&numBlocksWritten);
	isave->EndChunk();
	return IO_OK;

}
//-------------------------------------------------------
// Auto-called on scene load
//-------------------------------------------------------
IOResult SimpleCustAttrib::Load(ILoad *iload)
{
	ULONG numBlocksRead;
	IOResult res;
	//float val;

	SAFE_DELETE_ARRAY(m_Data);

	while( (res=iload->OpenChunk())==IO_OK ) {
		switch (iload->CurChunkID()) {
			case CHUNK_ENTITYDATA:{
				m_DataSize = iload->CurChunkLength();
				m_Data	   = new char[m_DataSize];
				res = iload->Read(m_Data, m_DataSize, &numBlocksRead);
				break;
			}
		}
		iload->CloseChunk();
		if (res!=IO_OK)  
			return res;
	}

	return IO_OK;
}

//-------------------------------------------------------
// Simple
//-------------------------------------------------------
SimpleCustAttrib::SimpleCustAttrib()
{
	m_DataSize  = 0;
	m_Data		= NULL;
	//SimpleAttribDesc.MakeAutoParamBlocks(this);
}

SimpleCustAttrib::~SimpleCustAttrib(){
	SAFE_DELETE_ARRAY(m_Data);
}

void SimpleCustAttrib::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev){
	//SimpleAttribDesc.BeginEditParams(ip,this,flags,prev);
}

void SimpleCustAttrib::EndEditParams(IObjParam *ip, ULONG flags, Animatable *next){
	//SimpleAttribDesc.EndEditParams(ip,this,flags,next);
}

ReferenceTarget *SimpleCustAttrib::Clone(RemapDir &remap){
	SimpleCustAttrib *pnew = new SimpleCustAttrib;

	// Copy over data
	pnew->m_Data	 = new char[m_DataSize];
	pnew->m_DataSize = m_DataSize;
	memcpy(pnew->m_Data,m_Data,m_DataSize);

	BaseClone(this, pnew, remap);
	return pnew;
}


//---------------------------
// GetModifier
//
// Returns the modifier that's on the node with the specified
// class IDs, if there is one.
Modifier* GetModifier(INode* node){
	IDerivedObject* dobj = NULL;
	int i;
	{
		//assert(scid == OSM_CLASS_ID);
		Object* obj = node->GetObjectRef();

		if(!obj)
			return NULL;

		if (obj->ClassID() == Class_ID(XREFOBJ_CLASS_ID, 0))
		{
			// if it's an xref, retarget at the actual
			// pipeline object.
			obj = (Object*)((IXRefObject*)obj)->GetReference(0);
		}

		if (obj && obj->SuperClassID() == GEN_DERIVOB_CLASS_ID)
		{
			dobj = (IDerivedObject*)obj;
		}
	}

	// Look through the derived objects attached from this object
	// "down" to find the modifier.
	while (dobj &&
		(dobj->SuperClassID() == GEN_DERIVOB_CLASS_ID))
	{
		int numMods = dobj->NumModifiers();
		Modifier* mod;

		for (i = 0 ; i < numMods ; ++i)
		{
			mod = dobj->GetModifier(i);

			if (mod)// && (mod->ClassID() == cid))
			{
#ifdef _DEBUG
				//assert(mod->SuperClassID() == scid);
#endif
				return mod;
			}
		}

		// We're being hopeful here. Maybe not really
		// a DObj, but we'll see in the loop check.
		dobj = (IDerivedObject*)dobj->GetObjRef();
	}

	return NULL;
}

int FindAttribute(ICustAttribContainer * cc){
	for(int i=0;i<cc->GetNumCustAttribs();i++)
	{
		CustAttrib * ca = cc->GetCustAttrib(i);
		if(!strcmp(ca->GetName(),"Entity Attributes"))
		{
			return i;
		}
	}

	return -1; // Not loaded
}

//-------------------------------------------------------
// Gets CA, and optionally creates if doesn't yet exist
//-------------------------------------------------------
SimpleCustAttrib* GetObjectCA(INode* node, bool bCreate){
	BaseObject *obj = node->GetObjectRef();

	if(!obj)
		return NULL;

	BaseObject* obj2 = GetModifier(node);
	if(obj2)
		obj = obj2;

	//If there is no modifier we could just bail out here but instead we apply
	//the CA to the base object
	ICustAttribContainer* cc = obj->GetCustAttribContainer();
	if(!cc){
		if(!bCreate)
			return NULL;

		obj->AllocCustAttribContainer();
		cc = obj->GetCustAttribContainer();
	}

	//  Is our CA already installed ?
	int index = FindAttribute(cc);
	if(index != -1)
		return (SimpleCustAttrib*)cc->GetCustAttrib(index); // Already loaded, return it

	if(!bCreate)
		return NULL;

	// Create the new CA
	SimpleCustAttrib  * ca = new SimpleCustAttrib();
	cc->AppendCustAttrib(ca);
	return ca;
}
//-------------------------------------------------------
//
//-------------------------------------------------------
void GetData(INode* node, char*& data, int& dataSize){
	SimpleCustAttrib* a = GetObjectCA(node,false);
	if(a){
		data = a->m_Data;
		dataSize = a->m_DataSize;
	}
	else{
		data		= 0;
		dataSize	= 0;
	}
}

//-------------------------------------------------------
//
//-------------------------------------------------------
void SetData(INode* node, char* data, int dataSize){
	SimpleCustAttrib* a = GetObjectCA(node,true);
	SAFE_DELETE_ARRAY(a->m_Data);
	a->m_Data = new char[dataSize+20];
	a->m_DataSize = dataSize+20;
	memcpy(a->m_Data,data,dataSize);
}




//----------------------------------------------------------------------------------
// Desc: Helper to serialize a string from the app data
//----------------------------------------------------------------------------------
void ReadString(string& str, char** buf){
	int len = *(int*)(*buf);
	(*buf) += sizeof(int);
	str.resize(len);
	memcpy((char*)str.c_str(),(*buf),len);
	(*buf) += len;
}
//----------------------------------------------------------------------------------
// Desc: Helper to serialize a string from the app data
//----------------------------------------------------------------------------------
void WriteString(string& str,char** buf){
	int len = str.length();
	memcpy((*buf),&len,sizeof(int));
	(*buf) += sizeof(int);
	memcpy((*buf),str.c_str(),len);
	(*buf) += len;
}
//----------------------------------------------------------------------------------
// Desc: Helper to serialize an int
//----------------------------------------------------------------------------------
void WriteInt(int i, char** buf){
	memcpy((*buf),&i,sizeof(int));
	(*buf) += sizeof(int);
}
//----------------------------------------------------------------------------------
// Desc: Helper to serialize an int
//----------------------------------------------------------------------------------
int ReadInt(char** buf){
	int i;
	memcpy(&i,(*buf),sizeof(int));
	(*buf) += sizeof(int);
	return i;
}

//----------------------------------------------------------------------------------
// Read/Write an object
//----------------------------------------------------------------------------------
#define WriteObject(x) memcpy((void*)(buf),(void*)&(x),sizeof(x)); (buf) += sizeof(x);
#define ReadObject(x) memcpy((void*)&(x),(buf),sizeof(x)); (buf) += sizeof(x);


//-------------------------------------------------------
//
//-------------------------------------------------------
void ClearNodeData(INode* node){
	SimpleCustAttrib* a = GetObjectCA(node,true);
	SAFE_DELETE_ARRAY(a->m_Data);
	a->m_Data = NULL;
	a->m_DataSize = 0;
}



//----------------------------------------------------------------------------------
// Desc: Sets game object attributes for this node
//----------------------------------------------------------------------------------
void SetNodeData(INode* node, NodeData& data){
	char *buf, *buf2;
	buf = buf2 = new char[20240]; // 20K

	// Is this node on ref list?
	bool found = false;
	for(int i=0;i<theGlobalSettings->m_References.size();i++){
		if(node == theGlobalSettings->m_References[i])
			found = true;
	}

	if(!found){
		// Not yet referenced, so create reference tracker
		// Create the reference maker to get a notification when anything changes
		theGlobalSettings->MakeRefByID( FOREVER, theGlobalSettings->NumRefs(), node );
	}

	WriteInt(NODEDATA_VERSION,&buf);

	// Timestamps
	WriteObject(data.timeMoved);
	WriteObject(data.timeModified);

	WriteInt(data.script.bIncludeModel,&buf);
	WriteString(data.script.filename,&buf);
	WriteString(data.script.classname,&buf);
	WriteString(data.script.parentclass,&buf);

	// Params
	WriteInt(data.script.parameters.size(),&buf);
	for(int i=0;i<data.script.parameters.size();i++)
		WriteString(data.script.parameters[i],&buf);

	// Param values
	WriteInt(data.script.paramvalues.size(),&buf);
	for(int i=0;i<data.script.paramvalues.size();i++)
		WriteString(data.script.paramvalues[i],&buf);

	// SH Data...
	WriteObject(data.bSHEnabled);
	WriteString(data.receiverGroup,&buf);

	// Indoor Blockers
	WriteInt(data.inBlockers.size(),&buf);
	for(int i=0;i<data.inBlockers.size();i++)
		WriteString(data.inBlockers[i],&buf);

	// Outdoor Blockers
	WriteInt(data.outBlockers.size(),&buf);
	for(int i=0;i<data.outBlockers.size();i++)
		WriteString(data.outBlockers[i],&buf);

	WriteObject(data.shOptions);
	WriteObject(data.bUseCustom);

	int len = (buf - buf2);
	SetData(node,buf2,len);
	delete[] buf2; // Delete old buffer
}


//----------------------------------------------------------------------------------
// Desc: Gets game object attributes for this node
//----------------------------------------------------------------------------------
bool GetNodeData(INode* node, NodeData& data){
	char* buf;
	int   bufSize;
	GetData(node,buf,bufSize);
	if(buf == NULL || bufSize == 0)
		return false;

	int version = ReadInt(&buf);

	if(version < NODEDATA_VERSION)
		return false;

	// Timestamps
	ReadObject(data.timeMoved);
	ReadObject(data.timeModified);

	data.script.bIncludeModel = ReadInt(&buf);
	ReadString(data.script.filename,&buf);
	ReadString(data.script.classname,&buf);
	ReadString(data.script.parentclass,&buf);

	// Params
	int params = ReadInt(&buf);
	data.script.parameters.resize(params);
	for(int i=0;i<params;i++)
		ReadString(data.script.parameters[i],&buf);

	// Param values
	int paramvalues = ReadInt(&buf);
	data.script.paramvalues.resize(paramvalues);
	for(int i=0;i<data.script.paramvalues.size();i++)
		ReadString(data.script.paramvalues[i],&buf);

	ReadObject(data.bSHEnabled);

	ReadString(data.receiverGroup,&buf);

	// Indoor Blockers
	data.inBlockers.resize(ReadInt(&buf));
	for(int i=0;i<data.inBlockers.size();i++)
		ReadString(data.inBlockers[i],&buf);

	// Outdoor Blockers
	data.outBlockers.resize(ReadInt(&buf));
	for(int i=0;i<data.outBlockers.size();i++)
		ReadString(data.outBlockers[i],&buf);

	ReadObject(data.shOptions);
	ReadObject(data.bUseCustom);

	return true;
}

string AsLower(const string& s);


//----------------------------------------------------------------------------------
// Desc: Sets a property for this node
//----------------------------------------------------------------------------------
bool SetNodeProperty(INode* node, string prop, string val){
	// Find property & Set it
	NodeData data;
	GetNodeData(node,data);
	for(int i=0;i<data.script.parameters.size();i++){
		if(AsLower(data.script.parameters[i]) == AsLower(prop)){
			data.script.paramvalues[i] = val;
			SetNodeData(node,data);
			return true;
		}
	}

	return false;
}


//----------------------------------------------------------------------------------
// Desc: Gets a property for this node
//----------------------------------------------------------------------------------
string GetNodeProperty(INode* node, string prop){
	// Find property & Set it
	NodeData data;
	GetNodeData(node,data);
	for(int i=0;i<data.script.parameters.size();i++){
		if(AsLower(data.script.parameters[i]) == AsLower(prop)){
			return data.script.paramvalues[i];
		}
	}
	return "";
}
/*
void DynPBlock::Remove()
{
	INode *node;
	HWND hWndList;
	Interface *ip = GetCOREInterface();

	if (ip->GetSelNodeCount() == 1) 
		node = ip->GetSelNode(0); 
	else 	
	{	
		node = NULL; 
		return;
	}
	BaseObject *obj = node->GetObjectRef();
	if(obj->SuperClassID() == GEN_DERIVOB_CLASS_ID )
	{
		IDerivedObject *pDerObj = (IDerivedObject *) obj;
		obj = pDerObj->GetModifier(0);
	}

	ICustAttribContainer* cc = obj->GetCustAttribContainer();
	if(cc!=NULL)
	{
		// Make sure we only remove ourselves and not other CAs
		hWndList = GetDlgItem(hPanel,IDC_CALIST);
		int curSel = ListBox_GetCurSel(hWndList);

		int index = CheckCCIsLoaded(cc,curSel);

		if(index >= 0) {
			theHold.Begin(); 
			cc->RemoveCustAttrib(index);
			theHold.Accept(_T("Remove Custom Attribute"));
		}
		else if (index == CC_NOT_LOADED)
			MessageBox(hPanel,"Nothing to Remove", strWarning, NULL);
		else 	if (index == CC_UNKNOWN_CUST_ATTRIB)
			MessageBox(hPanel,strUnknownCAFoundMsg, strWarning, NULL);
	}
}*/
