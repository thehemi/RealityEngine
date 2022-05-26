// TODO: Re-implement in new framework



#include "stdafx.h"

/*
#include "modstack.h" // Object creation

using namespace std;

void NOTE(const char *fmt, ...){
	va_list		argptr;
	char		msg[8000];
	
	va_start (argptr,fmt);
	vsprintf (msg,fmt,argptr);
	va_end (argptr);
	OutputDebugString(msg);
}

//--------------------------------------------------------------
// Gets passed every node, checks to see if it's a light node, 
// if it is, it adds it to the list
//--------------------------------------------------------------
bool IsExportableLight(INode* node);
bool CollectLights(INode* node){
	if(IsExportableLight(node)  && !(psc->m_bSaveSelection && !node->Selected())){
		psc->splitLights.push_back(node);
	}

	// For each child of this node, we recurse into ourselves 
	// until no more children are found.
	for (int c = 0; c < node->NumberOfChildren(); c++) {
		if (!CollectLights(psc, node->GetChildNode(c)))
			return FALSE;
	}
	
	return TRUE;
}




BOOL CalcNewBooleanOp(Mesh** meshOut, Mesh & mesh1, Mesh & mesh2, int op, Matrix3 *tm1=NULL, Matrix3 *tm2=NULL, int whichinv=0) {
	MNMesh m1(mesh1);
	MNMesh m2(mesh2);
	if(tm1) m1.Transform(*tm1);
	if(tm2) m2.Transform(*tm2);


	// This faults:
	m1.PrepForBoolean();

	m2.PrepForBoolean();

	// "Face XX has no size" <-- How do we fix this??
	m1.CheckAllData();
	if(!m1.BooleanCut(m2,op))
		return false;
	
	m1.CheckAllData();

	if(whichinv==0) m1.Transform(Inverse(*tm1));
	//if(whichinv==1) mOut.Transform(Inverse(*tm2));

	m1.PrepForPipeline();

	delete *meshOut;
	*meshOut = new Mesh;
	m1.OutToTri(**meshOut);

	return TRUE;
}



// Slice mesh by box of w/h/l=radius, and put into inside/outside meshes
bool SliceByBox(float radius, Point3 center, Mesh** mesh, Matrix3& meshTM){

	float size = radius*2; // This gives the box the closest sphere fit

	// Create a new object through the CreateInstance() API
	Object *obj = (Object*)psc->m_pInterface->CreateInstance(
		GEOMOBJECT_CLASS_ID,
		Class_ID(BOXOBJ_CLASS_ID,0));
	assert(obj);
	// Get a hold of the parameter block
	IParamArray *iCylParams = obj->GetParamBlock();
	assert(iCylParams);

	// Set the values
	int id;
	id = obj->GetParamBlockIndex(BOXOBJ_LENGTH);
	assert(id>=0);
	iCylParams->SetValue(id,TimeValue(0),size);

	id = obj->GetParamBlockIndex(BOXOBJ_WIDTH);
	assert(id>=0);
	iCylParams->SetValue(id,TimeValue(0),size);

	id = obj->GetParamBlockIndex(BOXOBJ_HEIGHT);
	assert(id>=0);
	iCylParams->SetValue(id,TimeValue(0),size);

	id = obj->GetParamBlockIndex(BOXOBJ_WSEGS);
	assert(id>=0);
	iCylParams->SetValue(id,TimeValue(0),1);

	id = obj->GetParamBlockIndex(BOXOBJ_LSEGS);
	assert(id>=0);
	iCylParams->SetValue(id,TimeValue(0),1);

	id = obj->GetParamBlockIndex(BOXOBJ_HSEGS);
	assert(id>=0);
	iCylParams->SetValue(id,TimeValue(0),1);

	// Create a derived object that references the box
	IDerivedObject *dobj = CreateDerivedObject(obj);

	INode *node = psc->m_pInterface->CreateObjectNode(dobj);
	// Name the node and make the name unique.
	TSTR name(_T("Boolean"));
	psc->m_pInterface->MakeNameUnique(name);
	node->SetName(name);

	// Center where light is
	Matrix3 tm;
	tm.IdentityMatrix();
	tm.SetTrans(center-Point3(0.f,0.f,radius));
	node->SetNodeTM(0,tm);

	// Get Mesh object for boolean
	int needDel;
	const ObjectState &pWorldObjState = node->EvalWorldState(0);
	TriObject* tri = GetTriObjectFromObjRef(pWorldObjState.obj, &needDel);

	Mesh* booleanMesh = &tri->GetMesh();

	Matrix3 tm1 = meshTM; // Object
	Matrix3 tm2 = node->GetNodeTM(0);    // Cutter box

	// Don't do these, they erase texture coordinates and stuff for some reason
//	(*mesh)->RemoveIllegalFaces();
//	(*mesh)->RemoveDegenerateFaces();
//	(*mesh)->DeleteIsoVerts();
//	(*mesh)->DeleteIsoMapVerts();

	NOTE("Doing boolean...");
	bool worked = CalcNewBooleanOp(mesh,**mesh,*booleanMesh,BOOLOP_CUT_REFINE,&tm1,&tm2);
	NOTE("done!\n");

	psc->m_pInterface->DeleteNode(node);

	if(needDel)
		delete tri;

	if(!worked){
		Error("SplitMeshForLights() failed for mesh '%s'. This means the object probably has mesh errors, and failed the boolean operation. It will be skipped for now, but should be STL Checked and any bad faces repaired.",node->GetName());
		return false;
	}

	return true;
}




bool CheckBoundingSphereAgainstBox(float radius, Point3& center, Box3& box){
	float d = 0;
	for(int i=0;i<3;i++){
		if(center[i] < box.pmin[i])
			d = d+ (( center[i] - box.pmin[i])*( center[i] - box.pmin[i]));
		else if (center[i] > box.pmax[i])
			d = d+ ((center[i] - box.pmax[i])*(center[i] - box.pmax[i]));
	}
	if( d > radius*radius)
		return false; // disjoint

	return true; // overlap
}




bool GetLightData(INode* dalight,float& radius, Point3& pos){
	ObjectState os =dalight->EvalWorldState(0);
	if (!os.obj) {
		MessageBox(0,"ERROR LIGHT INVALID",0,0);
	}
	GenLight* light = (GenLight*)os.obj;
	LightState ls;
	light->EvalLightState(0, FOREVER, &ls);

	Matrix3 tm = dalight->GetNodeTM(0);
	float scale = tm.GetRow(0).x;
	if(fabsf(scale)>0.01f)
		radius = ls.attenEnd*scale;
	else
		radius = ls.attenEnd;

	if(fabsf(radius) < 0.01f && ls.type == OMNI_LIGHT){
		char msg[256];
		sprintf(msg,"Warning: Light '%s' has radius of '%f'. Skipping!",dalight->GetName(),radius);
		return false;
	}


	// Light pos
	Matrix3 transform = dalight->GetObjectTM(0);
	pos = transform.GetTrans();


	if(ls.type == OMNI_LIGHT)
		return true;
	else if(ls.type == FSPOT_LIGHT || ls.type == TSPOT_LIGHT){
		// HACK!!! For spotlight
		//radius = 50000;
		return true;
	}
	
	return false;
}

//
//
//
Mesh* SplitMeshForLights(INode* InNode, Mesh* InMesh){
	Mesh* NewMesh = new Mesh(*InMesh);

	// Collect lights if not done already
	if(psc->splitLights.size() == 0)
		CollectLights(psc,psc->rootNode);

	NOTE("\tSplitting mesh '%s' by %d lights...\n",InNode->GetName(),psc->splitLights.size());

	int checked = 0;

	// Snip mesh by all lights
	// Collect a mesh part for each light segment in the 'out' vector
	// Snip the single 'outside' part by all the lights
	for(int i=0;i<psc->splitLights.size();i++){
		
		// Get light
		float radius;
		Point3 pos;
		if(!GetLightData(psc->splitLights[i],radius,pos))
			continue; // Skip non-omnis
		
		// Get bbox, and check for intersection
		// TODO: Make triangle-accurate test against light box
		// Will give a decent speed boost
		Matrix3 tm = InNode->GetObjTMAfterWSM(0);
		Box3 box;
		int nv = InMesh->getNumVerts();
		box.Init();
		for (int j=0; j<nv; j++) 
			box += tm*InMesh->getVert(j);

		// FIXME: Use bbox/bbox, not sphere/bbox, as this is how volume maps work (??)
		if(!CheckBoundingSphereAgainstBox(radius,pos,box))
			continue;

		NOTE("light %d/%d. rad=%f verts=%d name=%s...\n",i,psc->splitLights.size(),radius,nv,psc->splitLights[i]->GetName());

		checked++;

		if(!SliceByBox(psc, radius,pos,&NewMesh,InNode->GetNodeTM(0))){
			SAFE_DELETE(NewMesh);
			return InMesh;
		}
	}

	return NewMesh;
}
*/