//=========== Copyright (c) 2003, Tim Johnson. All rights reserved. ===========
// Static Model class
//=============================================================================
#include "stdafx.h"
#include "Collision.h"
#include "BatchRenderer.h"
#include "Frame.h"
#include "Serializer.h"
#include "Collision\CollisionRoutines.h"
#include "Compiler\Compiler.h"
#include "LODManager.h"

// Holds all loaded models, even instances, to avoid duplicate loading of the same files
vector<StaticModel*> modelCache;

//-----------------------------------------------------------------------------
// Desc: 
//-----------------------------------------------------------------------------
StaticModel::StaticModel(){
    m_VisibleDistanceRatio = 500;
    m_LODRatio = 500;
	m_LoadScale				= 1;
	CustomFlags				= 0;
	m_pFrameRoot			= NULL;
	m_pInstances			= NULL;
	m_OcclusionState		= 0;
	m_OcclusionQuery		= NULL;
	m_StaticLights			= 0;
	bExportable = false;
}

StaticModel::~StaticModel(){ 
	Destroy(); 
}

Material* StaticModel::FindMaterial(string name){
	return m_pFrameRoot->FindMat(name);
}

//-----------------------------------------------------------------------------
// Desc: 
//-----------------------------------------------------------------------------
void StaticModel::Destroy(){
	ResourceManager::Instance()->Remove(this);

	if(!m_pInstances)
		return; // Was never loaded

	SAFE_RELEASE(m_OcclusionQuery);

	// Delete from cache list
	vector_erase(modelCache,this);

	// Delete from list of m_Instances
	vector_erase((*m_pInstances),this);

	// Last model must delete all data
	if(m_pInstances->size() == 0){
		if(m_pFrameRoot)
			m_pFrameRoot->FreeContents();
	}

	// Delete non-shared data
	SAFE_DELETE(m_pFrameRoot);
}

// HACK: This is barely used
void StaticModel::InitAfterLoad(){
	// Create instance list if we haven't yet been linked to one (i.e. we're the first instance)
	if(!m_pInstances){
		m_pInstances = new vector<StaticModel*>;
		m_pInstances->push_back(this);
	}
}

//-----------------------------------------------------------------------------
// Desc: 
//-----------------------------------------------------------------------------
void StaticModel::Scale(Vector scale)
{
    if(m_pFrameRoot)
      m_pFrameRoot->Scale(scale);
}

//-----------------------------------------------------------------------------
// Desc: 
//-----------------------------------------------------------------------------
void StaticModel::Load(const char* name, bool DeepCopy){
	// Safety checking to stop reloading
	if(IsLoaded())
		return;

	Destroy();

	// Check cache if a new deep copy wasn't requested
	if(!DeepCopy)
    {
		string fullname = name;

		// Get index of actual name, path stripped
		int i1 = fullname.find_last_of("/");
		int i2 = fullname.find_last_of("\\");
		int i3 = (i1>i2?i1:i2);
		if(i3 == -1)
			i3 = 0;
		else
			i3++; // so we go one ahead of /

		string stripped_name = fullname.substr(i3);
		// Remove the .ext too, because it might be a cached dds now
		stripped_name = stripped_name.substr(0,stripped_name.rfind(".")+1);
		ToLowerCase(stripped_name);

		for(int i=0;i<modelCache.size();i++){
			if(modelCache[i]->m_FileName.rfind(stripped_name) != -1 && modelCache[i]->m_pFrameRoot){
				modelCache[i]->CreateNewInstance(this); // Whee, we found a cached copy
				return;
			}
		}
	}

	m_FileName = name;

	// Store the proper filename string for easy comparison
	ToLowerCase(m_FileName);

	Serializer load;
	load.LoadModel(m_FileName,this);
	if(!this->m_pFrameRoot)
		return;

	Compiler c;
	if(!Engine::Instance()->MainConfig->GetBool("PRTAutoCompile")) 
          c.bCompilePRT = false;
	c.CompileModel((Model*)this);

	ResourceManager::Instance()->Add((Model*)this);

	if(m_pFrameRoot)
    {
		m_pFrameRoot->UpdateMatrices(m_RootTransform);
		InitAfterLoad();

		// We must add all models, even instances, to the cache, so that
		// if one copy is deleted we can still look up a fellow instance
		modelCache.push_back(this);
	}

	// Create instance list if we haven't yet been linked to one (i.e. we're the first instance)
	if(!m_pInstances)
    {
		m_pInstances = new vector<StaticModel*>;
		m_pInstances->push_back(this);
	}
}

//-----------------------------------------------------------------------------
// Desc: Strips offset from frame meshes
//-----------------------------------------------------------------------------
void StaticModel::RemoveSceneOffset(){
	if(m_pFrameRoot)
		m_pFrameRoot->RemoveSceneOffset();
}


//-----------------------------------------------------------------------------
// A simple variant of SetTransform()
//-----------------------------------------------------------------------------
void StaticModel::SetTransform(Matrix& rotOffset, Vector& locOffset){
	Matrix mat = rotOffset;
	mat.m3 = locOffset;
	SetTransform(mat);
}

//-----------------------------------------------------------------------------
// Desc: Transforms the entire hierarchy, keeping relative offsets that were 
// saved with the file
//-----------------------------------------------------------------------------
void StaticModel::SetTransform(Matrix& transform){
	if(!m_pFrameRoot)
		return;

	m_RootTransform = transform;

	// We do this immediately in case we aren't an animated model
	// Animated models call this every frame
	m_pFrameRoot->UpdateMatrices(m_RootTransform);

	if(!m_pFrameRoot)
		m_BBox = BBox();
	else // Uses combined matrices, so result is already in world space
		m_BBox = m_pFrameRoot->GetWorldBBox();
}

//-----------------------------------------------------------------------------
// Desc: Clones a model without copying the shared data
//-----------------------------------------------------------------------------
void StaticModel::CreateNewInstance(StaticModel* model){
	*model = *this;
	model->m_OcclusionQuery = NULL; // We want a unique query for this instance

	// Clone the frame hierarchy
	if(m_pFrameRoot)
		model->m_pFrameRoot = m_pFrameRoot->Clone(false);

	// Add instance to shared list
	m_pInstances->push_back(model);

	// We must add all models, even instances, to the cache, so that
	// if one copy is deleted we can still look up a fellow instance
	modelCache.push_back(model);
}

//-----------------------------------------------------------------------------
// Desc: World box for an element
// FIXME: Add multi-element boxes instead of this
//-----------------------------------------------------------------------------
BBox StaticModel::GetWorldBBox()
{ 
	return m_BBox;
}

//-----------------------------------------------------------------------------
// Desc: Clears lights from model cache
//-----------------------------------------------------------------------------
void StaticModel::ClearLights()
{	// Only erase dynamic lights
	if(m_TouchingLights.size())
		m_TouchingLights.erase(m_TouchingLights.begin()+m_StaticLights,m_TouchingLights.end());
}

void StaticModel::Update(){}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
Mesh* StaticModel::ChooseLOD(Camera* cam, ModelFrame* pFrame)
{
	float dist = (cam->Location - pFrame->CombinedTransformationMatrix[3]).Length();
	// First see if it's in range at all
	if(dist > m_VisibleDistanceRatio*LODManager::Instance()->VisibleRange)
		return 0;

/*	clamp(dist,0.0f,endDist);
	float distFraction = (dist / endDist);
	
	int lod = lods.size() * distFraction;
	if(lod >= lods.size())
		lod = lods.size()-1;

	pFrame->m_CurrentLOD*/
	return pFrame->GetMesh();
}

//-----------------------------------------------------------------------------
// Desc: Queues the frame for drawing
//-----------------------------------------------------------------------------
void StaticModel::Draw(Camera* camera, ModelFrame* pFrame, World* world, bool StaticObject, DrawFlags flags, Actor* Owner){
	if(!pFrame)
		return;

	Draw(camera,pFrame->pFrameFirstChild,world,StaticObject,flags,Owner);
	Draw(camera,pFrame->pFrameSibling,world,StaticObject,flags,Owner);

	if(!pFrame->GetMesh())
		return;
	
	// Which LOD to use?
	Mesh*	mesh		= ChooseLOD(camera,pFrame);
	if(!mesh) // Out of visible range
		return;
	
	// Find lights touching
	if(flags & DRAW_UPDATELIGHTING){
		bool dynamicEnabled = RenderDevice::Instance()->GetDynamicLights();
		//REMOVED: Clear any dynamic lights in case the lighting has already been updated this frame
		//Update();

		//Clear all lights in case the lighting has already been updated this frame
		m_TouchingLights.clear();

		for(int i=0;i<world->m_Lights.size();i++){
			Light* l = world->m_Lights[i];
			//manual model draws don't shadow
			if(l->IsHidden || l->m_bShadowProjector || (!dynamicEnabled && l->IsDynamic()) || (Owner && l->Outside != Owner->Outside && l->Inside != Owner->Inside))
				continue;

			if(IsBoxIntersectingSphere(m_BBox.min,m_BBox.max,l->GetCurrentState().Position,l->GetCurrentState().Range))
			{
				if(Owner)
				{
					Actor* a = Owner;
					Light* theLight = l;

			bool bAdd = true;
			// If m_AllowIncludeExclude or is a prefab, check the exclude list
			// NOTE: Checking IsPrefab is redundancy -- m_AllowIncludeExclude is true for prefabs

			bool IsExcluded = false;

			if(a->Inside != theLight->Inside && a->Outside != theLight->Outside)
				bAdd = false;
			else //if(a->MyModel->m_AllowIncludeExclude)
				if(a->m_IsExcludeList)
				{
				//bAdd = !theLight->IsExcluded(a->MyModel);
					IsExcluded = theLight->IsExcluded(a);
					bAdd = !IsExcluded;
				}

				if(bAdd)
					m_TouchingLights.push_back(theLight);
				}
				else
					m_TouchingLights.push_back(l);
			}
		}
	}

	// Reset draw calls for mesh
	// FIXME: This is wrong when using lods
	mesh->m_DrawCalls = 0;

    vector<BatchItem*> immediateItems;
	for(int i=0;i<mesh->m_AttribTable.size();i++)
    {
        BatchItem* item = new BatchItem;

        // Does segment have frustum checking box?
        if(i < pFrame->m_WorldAttribBoxes.size())
        {
            BBox& box = pFrame->m_WorldAttribBoxes[i];
            // Cam check
            if(!camera->BoxInFrustum(box))
            {
                delete item;
                continue;
            }

            // Light check
            for(int j=0;j<m_TouchingLights.size();j++)
            {
                if(IsBoxIntersectingSphere(box.min,box.max,m_TouchingLights[j]->GetCurrentState().Position,m_TouchingLights[j]->GetCurrentState().Range))
                    item->lights.push_back(m_TouchingLights[j]);
            }
        }
        else
            item->lights	    = m_TouchingLights;

		item->pFirstItem    = item;
		item->model			= (Model*)this;
		item->box			= m_BBox;
		item->mesh			= mesh;
		item->subset		= mesh->m_AttribTable[i];
        item->bone_matrices = pFrame->bone_matrices;
		// If it's skinned, the material indexes are in the bone combination buffer, not the attribute buffer
		// This is so that we can have multiple attribute ids for different skinning sections that still
		// map to the same material id
        item->matRef = mesh->GetMaterial(item->subset.AttribId);
		// Fallbacks in case the materials or ids are messed up
		// This should never really happen, so if you're in a bug squashing mood you could set this to throw an assert
        if(!item->matRef) 
			item->matRef		= MaterialManager::Instance()->GetDefaultMaterial();

		item->world				= world;
		item->usesTransform		= true;
		item->transform			= pFrame->CombinedTransformationMatrix;
		// Hack: Bias Z per batch so two batches in same model don't fight each other
		item->Z_Value			= i*0.001f;
		item->StaticObject		= StaticObject;

        if(flags & DRAW_IMMEDIATE)
        {
            immediateItems.push_back(item);
        }
        else
		    BatchRenderer::Instance()->QueueBatchItem(item);
	}

    if(flags & DRAW_IMMEDIATE)
    {
        BatchRenderer::Instance()->DrawBatches(immediateItems);
    }
}

//-----------------------------------------------------------------------------
// Desc: Draws the entire model. Either immediately or later in the batch
//-----------------------------------------------------------------------------
void StaticModel::Draw(Camera* camera, World* world, bool StaticObject, DrawFlags flags, Actor* Owner){
    if(!camera->BoxInFrustum(m_BBox))
		 return;

	Draw(camera, m_pFrameRoot,world,StaticObject,flags, Owner);
}

//-----------------------------------------------------------------------------
// Desc: 
//-----------------------------------------------------------------------------
NODEHANDLE StaticModel::GetNodeHandle(string name, ModelFrame* ignore){
	if(!m_pFrameRoot)
		return NULL;

	return (ModelFrame*) m_pFrameRoot->Find(name,ignore);
}

//-----------------------------------------------------------------------------
// Get world matrix for node
//-----------------------------------------------------------------------------
Matrix StaticModel::GetNodeMatrix(NODEHANDLE node){
	ModelFrame* frame = (ModelFrame*)node;
	Matrix nodeMat;
	if(node == NULL)
		return nodeMat;

	nodeMat = frame->CombinedTransformationMatrix;

	return (nodeMat);
}

//-----------------------------------------------------------------------------
// Desc: Procedurally influence node matrices
//-----------------------------------------------------------------------------
void StaticModel::SetNodeInfluence(NODEHANDLE node, Matrix& newMat, float weight){
	if(node == NULL)
		return;
	((ModelFrame*)node)->CustomTransformationMatrix = newMat;
}

void StaticModel::DrawSimple(Camera* camera, ModelFrame* pFrame){
	if(!pFrame)
		return;

	DrawSimple(camera,pFrame->pFrameFirstChild);
	DrawSimple(camera,pFrame->pFrameSibling);

	if(!pFrame->GetMesh())
		return;

	// Decide mesh we use
	Mesh*	mesh		= ChooseLOD(camera,pFrame);
    if(!mesh)
        return;

    RenderWrap::SetWorld(pFrame->CombinedTransformationMatrix);
    LPDIRECT3DVERTEXDECLARATION9 oldDecl = mesh->m_pDeclaration;
    mesh->m_pDeclaration = VertexFormats::Instance()->FindFormat(sizeof(SimpleVertex))->decl;

	for(int i=0;i<mesh->m_AttribTable.size();i++)
    {
         mesh->DrawSubset(mesh->m_AttribTable[i]);
	}
    mesh->m_pDeclaration = oldDecl;
}

void StaticModel::DrawSimple(Camera* camera){
	DrawSimple(camera, m_pFrameRoot);
}

extern LPDIRECT3DSTATEBLOCK9 defaultStates;
int depth = 0;
//-----------------------------------------------------------------------------
// Draw box hierarchy showing all nodes. Used for skeletal visualization
//-----------------------------------------------------------------------------
void StaticModel::DrawHierarchy()
{
    depth = 0;
    defaultStates->Apply();
    RenderWrap::dev->SetTextureStageState(0,D3DTSS_COLORARG1,D3DTA_TFACTOR);
	RenderWrap::dev->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_SELECTARG1);

	DrawHierarchy(m_pFrameRoot,0);
}

//-----------------------------------------------------------------------------
// Draw box hierarchy showing all nodes. Used for skeletal visualization
//-----------------------------------------------------------------------------
void StaticModel::DrawHierarchy(ModelFrame* pFrame, ModelFrame* parent){
	if(!pFrame)
		return;
    depth+=5;

	DrawHierarchy(pFrame->pFrameFirstChild,pFrame);
	DrawHierarchy(pFrame->pFrameSibling,pFrame);

    RenderWrap::dev->SetRenderState(D3DRS_TEXTUREFACTOR,COLOR_RGBA(depth,depth,depth,255));
    if(parent)
    {
        LVertex line[2];
        line[0] = LVertex(parent->CombinedTransformationMatrix.m3,0xFFFFFFFF);
        line[1] = LVertex(pFrame->CombinedTransformationMatrix.m3,0xFFFFFFFF);
        Canvas::Instance()->DrawLines(2,line,sizeof(LVertex),false);
    }

    Vector size(0.02f,0.02f,0.02f);
    BBox box(pFrame->CombinedTransformationMatrix.m3-size,pFrame->CombinedTransformationMatrix.m3+size);

    Canvas::Instance()->Cube(box);
}

Light* StaticModel::FindStrongestLight()
{
	float minDistFrac = 999;
	Light* bestLight = 0;
	for(int i=0;i<m_TouchingLights.size();i++)
	{
		Light* light = m_TouchingLights[i];

		if(light->m_bShadowProjector || light->IsHidden || light->LifeTime != -1)
			continue;

		Vector v = light->Location - m_RootTransform.m3;
		// Get fraction as a factor of how far within the radius point is
		float distFrac = v.DotSelf() / (light->GetCurrentState().Range*light->GetCurrentState().Range);
		// If > 1, is out of range
		if(distFrac < 1)
		{
			// Bias by intensity
			distFrac /= light->GetCurrentState().Intensity;
			if(distFrac < minDistFrac)
			{
				minDistFrac = distFrac;
				bestLight = light;
			}
		}
	}

	return bestLight;
}