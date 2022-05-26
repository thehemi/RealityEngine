//=========== Copyright (c) 2004, Tim Johnson. All rights reserved. ===========
//
// Uses paramblock2 to hold global settings in a .max file
// Also manages global references (objects this plugin is tracking)
// NOTE: Global references are actually initialized in EntityAttributes along with the node data
//
//=============================================================================
#include "stdafx.h"
#include "globalsettings.h"
#include "MapSettings.h"

//----------------------------------------------------------------------------------
// Chunk is just entire param block
// Increment this any time we change version
//----------------------------------------------------------------------------------
#define CHUNK_SCENEDATA 6
#define CHUNK_REFCOUNT	107

GlobalSettings* theGlobalSettings = NULL;

//----------------------------------------------------------------------------------
// It also allows a utilities parameters to be displayed via a TrackViewNode.  
// This Node will also allow loading and saving of the Utilities data stored in the Paramblock2
//----------------------------------------------------------------------------------
class GlobalSettingsClassDesc:public ClassDesc2 {
public:
	BOOL NeedsToSave(){ return TRUE; }
	int 			IsPublic() {return 1;}

	void *			Create(BOOL loading = FALSE) 
	{
		// Need a new global settings each time a file is loaded
		if(theGlobalSettings) 
			delete theGlobalSettings;
		theGlobalSettings = new GlobalSettings;
		return theGlobalSettings;
	}
	const TCHAR *	ClassName() {return _T("Reality_DummyTVNode");}
	SClass_ID		SuperClassID() {return REF_MAKER_CLASS_ID;}
	Class_ID		ClassID() {return DUMMYTVN_CLASSID;}
	const TCHAR* 	Category() {return GetString(IDS_CLASS_CATEGORY);}
	HINSTANCE		HInstance()	{ return g_hInstance; }
};
static GlobalSettingsClassDesc theGlobalSettingsClassDesc;
ClassDesc2* GetDummyTVNDesc() {return &theGlobalSettingsClassDesc;}


//----------------------------------------------------------------------------------
// Why do we need this?
//----------------------------------------------------------------------------------
enum { SuperUtility_params }; //Needed to define the ParamBlock2
static ParamBlockDesc2 SuperUtilityBlockDesc (
	//required block spec
	SuperUtility_params, _T("params"),  0, &theGlobalSettingsClassDesc, P_AUTO_CONSTRUCT + P_AUTO_UI,
	//auto-construct block refno
	PBLOCK_REFNO,
	//rollout specification
	0, 0, 0, 0, NULL,
	end
);

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
GlobalSettings::GlobalSettings()
{
	node = NULL;
	theGlobalSettingsClassDesc.MakeAutoParamBlocks( this );
	m_References.push_back(NULL);
}

//----------------------------------------------------------------------------------
// Creates node in scene to store data
//----------------------------------------------------------------------------------
bool GlobalSettings::InitGlobalSettings()
{
	//Allow the ClassDesc to create and display our dialog boxes
	ITrackViewNode *root = GetCOREInterface()->GetTrackViewRootNode();

	if (root) {
		if (root->FindItem(DUMMYTVN_CLASSID) == -1) {
			// There's no SuperUtility tag here, create one.
			try{
				root->AddNode((ITrackViewNode*)this, _T("Evo_Global_Settings"), DUMMYTVN_CLASSID);
			}
			catch(...){
				MessageBox(0,"(Reality Tools) An error occured trying to read the map properties. Your .max file is corrupt. \nThis is due to a problem with the old map data stored in .max files\nSave the data in your file to another scene.","Error",MB_ICONSTOP);
			}
			return false;
		}
	}
	return true;
}

//----------------------------------------------------------------------------------
// Clears node in scene to store data
//----------------------------------------------------------------------------------
bool GlobalSettings::ClearGlobalSettings()
{
	//Allow the ClassDesc to create and display our dialog boxes
	ITrackViewNode *root = GetCOREInterface()->GetTrackViewRootNode();

	if (root) 
	{
		int index = root->FindItem(DUMMYTVN_CLASSID);
		if(index != -1){
			root->RemoveItem(index);
			return true;
		}
	}
	return false;
}


//----------------------------------------------------------------------------------
// Auto-called on scene save
//----------------------------------------------------------------------------------
IOResult GlobalSettings::Save(ISave *isave)
{
	try{
	ULONG numBlocksWritten;
	isave->BeginChunk(CHUNK_SCENEDATA);
	theMap.CommitSettings(); // Fill theMap.sceneData
	isave->Write(&theMap.sceneData,sizeof(SceneProperties),&numBlocksWritten);
	isave->EndChunk();

	// Write ref count!
	isave->BeginChunk(CHUNK_REFCOUNT);
	int numReferences = m_References.size();
	isave->Write(&numReferences,sizeof(int),&numBlocksWritten);
	isave->EndChunk();
	}
	catch(...){
		MessageBox(0,"GlobalSettings::Save() failed. Report to tim@artificialstudios.com","Error",0);
	}
	return IO_OK;

}

//----------------------------------------------------------------------------------
// Auto-called on scene load
//----------------------------------------------------------------------------------
IOResult GlobalSettings::Load(ILoad *iload)
{
	ULONG numBlocksRead;
	IOResult res;
	//float val;

	while( (res=iload->OpenChunk())==IO_OK ) {
		switch (iload->CurChunkID()) {
			case CHUNK_REFCOUNT:
			{
				int numReferences;
				res = iload->Read(&numReferences,sizeof(int),&numBlocksRead);
				// Expand & Init all references to NULL, except for first, which is pblock, already initialized
				m_References.resize(numReferences);
				for(int i=1;i<m_References.size();i++) 
					m_References[i] = NULL;
			}
			break;
			case CHUNK_SCENEDATA:
				res = iload->Read(&theMap.sceneData, sizeof(SceneProperties), &numBlocksRead);
				theMap.ApplySettings(); // Load into window
			break;
		}
		iload->CloseChunk();
		if (res!=IO_OK)  return res;
	}

	return IO_OK;
}

//----------------------------------------------------------------------------------
// Number of references we have made
//----------------------------------------------------------------------------------
int GlobalSettings::NumRefs() {
	return m_References.size();
}

//----------------------------------------------------------------------------------
// Return the n'th reference
//----------------------------------------------------------------------------------
RefTargetHandle GlobalSettings::GetReference(int i) {
	if(i == 0)
		return pblock;

	if(i < m_References.size())
		return m_References[i];
	else
		return NULL;
}

//----------------------------------------------------------------------------------
// Set the n'th reference.
//----------------------------------------------------------------------------------
void GlobalSettings::SetReference(int i, RefTargetHandle rtarg) {
	if(i == 0){
		pblock = (IParamBlock2*)rtarg;
		return;
	}

	if(i < m_References.size())
		m_References[i] = rtarg;
	else {
		assert(i == m_References.size());
		m_References.push_back(rtarg);
	}
}


//----------------------------------------------------------------------------------
// Important function, used to notify us when a key node changes, 
// so we know whether to recompile it (if incremental compiling is enabled)
//----------------------------------------------------------------------------------
RefResult GlobalSettings::NotifyRefChanged(class Interval cpc ,RefTargetHandle hTarget,unsigned long & PartID,unsigned int _Message)
{ 
	// If not INode, return
	if(!hTarget || hTarget->GetInterface(INODE_INTERFACE)==NULL)
		return REF_SUCCEED;

	static bool bGot = false;	// Got node data before stack collapse
	static INode* pGot = NULL;	// Node who's data we got
	static bool bChange = false;// Node modified
	static NodeData	data;
	static bool refDel = false;	// 'true' before clicking off object

	bool dummy = false;
	INode *node = (INode*)hTarget;

	switch ( _Message )
	{
		//This message is sent when a reference target is deleted. This allows the reference maker 
		//to handle this condition if it depends on the deleted item. 
		//For example this is sent when the item you are referencing is actually deleted and you need 
		//to set your pointer to the item to NULL.
	case REFMSG_TARGET_DELETED:
		for(int i=0;i<m_References.size();i++){
			if(hTarget == m_References[i]){
				m_References.erase(m_References.begin()+i);
				//m_References[i] = NULL;
			}
		}
		break;
		//This message is sent to ask a reference maker if it is okay to change the 
		//topology of a node. If any dependents have made topology-dependent modifiers, 
		//they should return REF_FAIL. A return of REF_SUCCEED means that the answer is 
		//YES, it is okay to change the topology. A return of REF_FAIL means that the 
		//answer is NO, it is not okay to change the topology.
	case REFMSG_IS_OK_TO_CHANGE_TOPOLOGY:
		dummy = true; 
		break;
		//This message is sent when a target has had a reference deleted.
	case REFMSG_REF_DELETED:
		dummy = true; 
		break;
		//This message is sent when a target has had a reference added.
	case REFMSG_REF_ADDED:
		dummy = true; 
		break;

		//Changes inside the object
	case REFMSG_CHANGE:
		{
			// Let's assume any change is a change worth recompiling for
			bChange = true;

			if(PartID & TEXMAP_CHANNEL)
				PartID = PART_TEXMAP;
			if(PartID & PART_GEOM)
				PartID = PART_GEOM;

			switch(PartID)
			{
				//This is a special partID sent by visibility controllers 
				//when they change the hidden in viewport state. 
			case PART_HIDESTATE:
				break;

				//This is passed in partID when the reference is to a node in the scene 
				//and its transformation matrix has changed. 
			case PART_TM:
				break;

				//This is sent if the object type changes.
			case PART_OBJECT_TYPE:
				break;

				//The vertices of the object
			case PART_TOPO:
				break;

				//The topology channel, i.e. the face or polygon structures.
				//Smoothing groups and materials are also part of this channel.
				//Edge visibility is also part of this channels since it is an
				//attribute of the face structure.
			case PART_GEOM:
				break;

				//The texture vertices and procedural mappings.
			case PART_TEXMAP:
				break;

				//The sub-object selection channel. An object's selection flows down 
				//the pipeline. What the selection is actually comprised of is up to 
				//the specific object type. For example, TriObjects have bits for face, 
				//edge and vertex selection. This channel is the actual BitArray used 
				//(like selLevel of the Mesh class).
			case PART_SELECT:
				bChange = false;
				break;

				//This is the current level of selection. Every object that flows down the 
				//pipeline is at a certain level that corresponds to the Sub-Object drop 
				//down in the MAX user interface. This channel indicates which level the 
				//object is at. This is also specific to the object type. 
				//There are 32 bits to represent the level of selection. 
				//When all the bits are 0, the object is at object level selection.
			case PART_SUBSEL_TYPE:
				bChange = false;
				break;

				//These are miscellaneous bits controlling the item's display.
				//These bits are specific to the type of object.
				//For the Mesh object these are the surface normal scale, display of
				//surface normals, edge visibility and display flags.
			case PART_DISPLAY:
				bChange = false;
				break;

				//This is the color per vertex channel. This is also used for the 
				//second texture mapping channel.
			case PART_VERTCOLOR:
				bChange = false;
				break;

			default:
				{
					int i = 0;
				}
				break;
			}
		} 
		break;

		// Selection changing on or off this node
	case REFMSG_TARGET_SELECTIONCHANGE:
		if(bChange && refDel && !bGot){
			NodeData d;
			if(GetNodeData(node,d)){
				GetSystemTime(&d.timeMoved);
				//GetSystemTime(&data.timeModified);
				SetNodeData(node,d);
				#ifdef _DEBUG
						//Beep(5000,55);
				#endif
			}
		}
		bChange = false;
		dummy = true; 
		break;

		//This message is sent by a node when it has a child linked to it or unlinked 
		//from it.
	case REFMSG_NODE_LINK:
		{
		}
		dummy = true; 
		break;

		//This message is sent by a node when its name has been changed. 
		//For example, the path controller displays the name of the node in the scene 
		//which it follows. It responds to this message by changing the name displayed 
		//in the UI.
	case REFMSG_NODE_NAMECHANGE:
	case REFMSG_SUBANIM_STRUCTURE_CHANGED:
		{
			int u = 0;
		}
		dummy = true; 
		break;

		//This message is sent by a derived object when a modifier is a added or deleted.
	case REFMSG_MODIFIER_ADDED:
		{
		}
		dummy = true; 
		break;

		//This message is sent when an animatable switches controllers for one 
		//of its parameters.
	case REFMSG_CONTROLREF_CHANGE:
		{
		}
		dummy = true; 
		break;

		//The selection set sends this notification when it receives a REFMSG_CHANGE from 
		//an item in the selection set. The selection set doesn't propagate the 
		//REFMSG_CHANGE message.
	case REFMSG_NODEINSELSET_CHANGED:
		{
		}
		dummy = true; 
		break;

		//This message is sent when a UV Generator changes symmetry, 
		//so interactive texture display updates.
	case REFMSG_UV_SYM_CHANGE:
		{
		}
		dummy = true; 
		break;

		//The first node that gets this message will fill in the TSTR which partID 
		//points to with its name and stop the message from propagating.
	case REFMSG_GET_NODE_NAME:
		{
		}
		dummy = true; 
		break;

		//This message is sent by the selection set whenever it has just deleted nodes.
	case REFMSG_SEL_NODES_DELETED:
		{
		}
		dummy = true; 
		break;

		//This message is sent before a reference target is pasted. It is sent by the target about to be replaced.
	case REFMSG_PRENOTIFY_PASTE:
		{
		}
		dummy = true; 
		break;

		//A texture map has been removed. This tells the Materials Editor to remove it 
		//from the viewport if it is active.
	case REFMSG_TEXMAP_REMOVED:
		{
			//Materials
		}
		dummy = true; 
		break;


		//This messages is sent by objects which contain shapes when the position 
		//changes.
	case REFMSG_CONTAINED_SHAPE_POS_CHANGE:
		dummy = true; 
		break;
		//This messages is sent by objects which contain shapes when 
		//the selection changes.
	case REFMSG_CONTAINED_SHAPE_SEL_CHANGE:
		dummy = true; 
		break;
		//Sent when a shape enters a state where it'll be changing a lot and it would be a good idea for anybody using it for mesh generation to suppress updates.
	case REFMSG_SHAPE_START_CHANGE:
		dummy = true; 
		break;
		//Sent to terminate the above state.
	case REFMSG_SHAPE_END_CHANGE:
		{
			//Splines
		}
		dummy = true; 
		break;

		//This message is sent by objects which contain shapes when the selection, 
		//or the position changes.
	case REFMSG_CONTAINED_SHAPE_GENERAL_CHANGE:
		{
		}
		dummy = true; 
		break;

		//This message is sent to dependents of the transform controllers of selected objects 
		//when the user ends a mouse transformation in the viewports (move/rotate/scale).
	case REFMSG_MOUSE_CYCLE_COMPLETED:
		{
				NodeData d;
				if(GetNodeData(node,d)){
					GetSystemTime(&d.timeMoved);
					SetNodeData(node,d);
#ifdef _DEBUG
					//Beep(5000,200);
#endif
				}
		}
		break;

		//Sent when objects are replaced from another scene (File->Replace). Other objects referencing the object that is replaced may want to perform some validity checking; this message is more specific than REFMSG_SUMANIM_STRUCTURE_CHANGED.
	case REFMSG_OBJECT_REPLACED :
		{
		}
		dummy = true; 
		break;


		//--------------------------------------------------------------------------------
		//------------------------------NOT INTERESTING MESSAGES -------------------------
		//--------------------------------------------------------------------------------


		//This message is sent by an object that provides branching in the history 
		//to notify it that the structure of the branches has changed.
	case REFMSG_BRANCHED_HISTORY_CHANGED:
		dummy = true; 
		break;
		//This method is used to see if this reference target depends on something. 
		//In MAX 2.0 and later, if the partID is nonzero, the dependency test will 
		//include child nodes. Otherwise, child nodes will not be considered dependents. 
		//See ReferenceTarget::BeginDependencyTest().
	case REFMSG_TEST_DEPENDENCY:
		dummy = true; 
		break;
		//A Parameter block sends this to its client to ask if it should display a 
		//distinct "Parameters" level in the track view hierarchy. 
		//A pointer to a boolean is passed in for PartID: set this to the desired 
		//answer. The default is NO -- in this case the message doesn't need 
		//to be responded to.
	case REFMSG_WANT_SHOWPARAMLEVEL:
		dummy = true; 
		break;
		//These messages are sent before and after a paste has been done. 
		//Sent as partID is a pointer to a data structure containing three RefTargetHandle's: 
		//the reference maker, the old target, and the new target. 
		//The message is sent to the reference maker initially.
	case REFMSG_BEFORE_PASTE:
		dummy = true; 
		break;
	case REFMSG_NOTIFY_PASTE:
		dummy = true; 
		break;
		//Sent by an unselected node to see if any selected nodes depend on it. 
		//The partID parameter points to a boolean. If a selected node receives 
		//this message it should set the boolean to true and return REF_STOP.
	case REFMSG_FLAG_NODES_WITH_SEL_DEPENDENTS:
		dummy = true; 
		break;
		//This tests for a cyclic reference. It will return REF_FAIL if there is a loop.
	case REFMSG_LOOPTEST:
		dummy = true; 
		break;

		//This is used by modifiers to indicate when they are beginning an edit. 
		//For example in SimpleMod::BeginEditParams() this message is sent.
	case REFMSG_BEGIN_EDIT:
		// Grab data so we don't lose it across stack collapses
		if(GetNodeData(node,data)){
			pGot = node;
			bGot = true;
		}
		break;

		//This is used by modifiers to indicate when they are ending an edit. 
		//For example in SimpleMod::EndEditParams() this message is sent. 
		//Typically what a modifier will do while it is being edited it will have 
		//its LocalValidity() return NEVER so that a cache is built before it. 
		//This will ensure it is more interactive while it is being edited. 
		//When this message is sent to indicate the edit is finished the system can 
		//discard the cache.
	case REFMSG_END_EDIT:
		{
			// Update node timestamp!
			if(bChange && bGot && pGot == node)
			{
				//GetSystemTime(&data.timeMoved);
				GetSystemTime(&data.timeModified);
				// NEVER create in middle
				SetNodeData(node,data);
#ifdef _DEBUG
				//Beep(5000,55);
#endif
			}
			bChange = false;
			bGot = false;
			pGot = NULL;
			break;
		}
		//This is used by modifiers to indicate that their apparatus (gizmo) 
		//is displayed. For example in SimpleMod::BeginEditParams() this message is sent.
	case REFMSG_MOD_DISPLAY_ON:
		dummy = true; 
		break;
		//This is used by modifiers to indicate that their apparatus (gizmo) 
		//is no longer displayed.
	case REFMSG_MOD_DISPLAY_OFF:
		dummy = true; 
		break;
		//This is sent by a modifier to cause its ModApp to call Eval() on the modifier. 
		//If a modifier wants its ModifyObject() method to be called it can send this 
		//message.
		//The PartID should contain the bits that specify which channels are to be 
		//evaluated, for example PART_GEOM|PART_TOPO or ALL_CHANNELS. 
		//The interval passed should be set to Interval(t, t), where t is the time the 
		//to evaluate. Note that before NotifyDependents() returns, ModifyObject() 
		//will be called.
	case REFMSG_MOD_EVAL:
		dummy = true; 
		break;
		//When an object receives this message it should do whatever it needs to do 
		//(usually select the appropriate sub-object) to make the dependent object 
		//be the object returned from Object::GetPipeBranch(). 
		//The partID will point to an INode pointer that will be filled in by the 
		//first node to receive this message. Thus, when an object that supports 
		//branching in the history receives this message it selects the target that 
		//sent the message.
	case REFMSG_SELECT_BRANCH:
		dummy = true; 
		break;
		//This messages is sent to dependents of the transform controllers of selected objects 
		//when the user begins a mouse transformation in the viewports (move/rotate/scale).
	case REFMSG_MOUSE_CYCLE_STARTED: 
		dummy = true; 
		break;
		//Sent by a node to other nodes (which depend on that node) when the user attempts to link another node to a node. The partID parameter contains a pointer to the new parent node.
	case REFMSG_CHECK_FOR_INVALID_BIND:
		dummy = true; 
		break;
		//Sent when a cache is dumped in the pipeline. A REFMSG_CHANGE message used to be sent, however that was misleading since the object itself didn't change even though any old object pointer has become invalid. For example, if a path controller depends on a spline object and that object dumps some caches in the pipeline, the path controller hasn't actually changed.
	case REFMSG_OBJECT_CACHE_DUMPED:
		dummy = true; 
		break;
		//Sent by Atmospheric Effects or Render Effects when they make or delete a reference to a node. When Atmospherics or Effects add or delete a gizmo they should send this message via NotifyDependents().
	case REFMSG_SFX_CHANGE:
		dummy = true; 
		break;
		//Used Internally
	case REFMSG_MODAPP_DELETING:
		dummy = true; 
		break;
	case REFMSG_EVAL:
		dummy = true; 
		break;
	case REFMSG_RESET_ORIGIN:
		dummy = true; 
		break;
	case REFMSG_FLAGDEPENDENTS:
		dummy = true; 
		break;
	case REFMSG_DISABLE:
		dummy = true; 
		break;
	case REFMSG_ENABLE:
		dummy = true; 
		break;
	case REFMSG_TURNON:
		dummy = true; 
		break;
	case REFMSG_TURNOFF:
		dummy = true; 
		break;
	case REFMSG_LOOKAT_TARGET_DELETED:
		dummy = true; 
		break;
	case REFMSG_INVALIDATE_IF_BG:
		dummy = true; 
		break;
	case REFMSG_OBJXREF_UPDATEMAT:
		dummy = true; 
		break;
	case REFMSG_OBJXREF_GETNODES:
		dummy = true; 
		break;
	case REFMSG_END_MODIFY_PARAMS:
	case REFMSG_BEGIN_MODIFY_PARAMS:
	case REFMSG_NODE_WSCACHE_UPDATED:
	case REFMSG_NODE_RENDERING_PROP_CHANGED:
		dummy = true;
		break;

	default:
		{
			dummy = true;
		}
		break;
	}

	if(_Message == REFMSG_REF_DELETED)
		refDel = true;
	else
		refDel = false;

	return REF_SUCCEED;
}
