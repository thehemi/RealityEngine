//=========== Copyright (c) 2004, Tim Johnson. All rights reserved. ===========
// BatchRenderer.cpp - Renders the most basic engine building blocks -- BatchItems
// The renderer heavily batches and sorts all data
// Sorting is by Effect Shader+Technique, Material
//
// This is the single most important file of the engine in terms of efficiency
//
// NOTE: Don't share vars between shaders. Has n^2 complexity.
// TODO: Make sure getting occlusion query multiple times works. Optimize it to a cached value
//
// WARNING: Do not refer to pool.matRef.m_Shader when you want to access the current shader
// isntead use m_CurrentShader, or you will probably be using the WRONG shader
//
//
//=============================================================================
#include <stdafx.h>
#include "BatchRenderer.h"
#include "HDR.h"
#include "ShadowMapping.h"
 
static Matrix s_ViewProjection; // Used for z-sorting
static Vector s_CamPos;

bool bDoublePassAlpha = false;
bool bUpdatedStaticPRT = false;
World* RenderingForWorld = NULL;
static Light* lastLight = NULL;
static Light* PrimaryLight = NULL;

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
BatchRenderer* BatchRenderer::Instance () 
{
	static BatchRenderer inst;
	return &inst;
}

//-----------------------------------------------------------------------------
// Used to build scissor region
//-----------------------------------------------------------------------------
BBox BuildInteresction(BBox& b1, BBox &b2){
	BBox b;
	b.min.x = max(b1.min.x,b2.min.x);
	b.min.y = max(b1.min.y,b2.min.y);
	b.min.z = max(b1.min.z,b2.min.z);

	b.max.x = min(b1.max.x,b2.max.x);
	b.max.y = min(b1.max.y,b2.max.y);
	b.max.z = min(b1.max.z,b2.max.z);
	return b;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
bool sort_by_material(const BatchItem* p1, const BatchItem* p2)
{
	return p1->matRef < p2->matRef;
}
//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
bool sort_by_shader(const BatchItem* p1, const BatchItem* p2)
{
	return p1->matRef->m_Shader > p2->matRef->m_Shader;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
bool sort_by_z(const ShaderBatch* p1, const ShaderBatch* p2)
{
	return p1->items[0]->Z_Value > p2->items[0]->Z_Value;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void BatchRenderer::InvalidateDeviceObjects(){
	// Reset the queued batch lists, because they contain device-dependent info
	SAFE_DELETE_VECTOR(m_Batches);
	SAFE_DELETE_VECTOR(m_AlphaBatches);
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void BatchRenderer::Initialize(){
	if(m_SystemMaterial)
		return; // Already initialize

	m_SystemMaterial = new Material;
	m_SystemMaterial->Initialize("System.fx","Occlusion");
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
BatchRenderer::BatchRenderer(){
	m_SystemMaterial = NULL;
	m_bZPass = FALSE;
}

//-----------------------------------------------------------------------------
// Checks for shadow mapping, which must be done in the alpha pass
//-----------------------------------------------------------------------------
void BatchRenderer::CheckShadowMapping(BatchItem* item){
	// Check for shadow lights, which must go to the final alpha stage
	for(int i=0;i<item->lights.size();i++){
		Light* shadowLight = item->lights[i];
		if(shadowLight->m_Type == LIGHT_SPOT && shadowLight->m_bShadowProjector){
			// Strip from this light list and put on alpha list
			item->lights.erase(item->lights.begin() + i);
			// Create new item
			BatchItem* newItem = new BatchItem;
			*newItem = *item;
			newItem->lights.clear();
			// Add the light to our new item list
			newItem->lights.push_back(shadowLight);
			// Add to shadow batches
			//if(m_ShadowBatches.size() == 0)
			//	m_ShadowBatches.push_back(new ShaderBatch(newItem->matRef->m_Shader,newItem->matRef->m_ShaderFunction_Point));
			m_ShadowItems.items.push_back(newItem);

			i--; // We erased a light, back up
		}
	}
}

//-----------------------------------------------------------------------------
// Add item to batch. We'll use the 'Point' technique for caching purposes
//
// NOTE:
// GetTechnique(): Set mat PRT from mesh PRT. This will force GetTechnique() to return correct version
// because mats are shared between meshes
//-----------------------------------------------------------------------------
void BatchRenderer::AddToBatch(vector<ShaderBatch*>& batches, BatchItem* item){
	LPCSTR tech = item->matRef->GetTechnique(LIGHT_OMNI,false);
	bool found = false;
	for(int k=0;k<batches.size();k++){
		// Compare shader & Technique (Independently of the light, since we always compare the point versions)
		if(tech == batches[k]->technique && item->matRef->m_Shader == batches[k]->shader && (!item->m_SHBatchItem || batches[k]->SHtechnique))
		{
			CheckShadowMapping(item);
			// Add
			batches[k]->items.push_back(item);
			found = true;
			break;
		}
	}
	// No group with this m_CurrentShader yet, so create it
	if(!found){
		CheckShadowMapping(item);
		batches.push_back(new ShaderBatch(item->matRef->m_Shader,tech));

		//set pertinent SH technique
		if(item->m_SHBatchItem)
		{
			batches[batches.size() - 1]->SHtechnique = item->matRef->GetTechnique(LIGHT_OMNI,true,false);

			if(item->mesh->m_bPRTMultiPass || !batches[batches.size() - 1]->SHtechnique)
				item->m_SHBatchItem = false;
		}

		batches[batches.size() - 1]->items.push_back(item);
	}
}

//-----------------------------------------------------------------------------
// Add another item to our queue. Sorts transparent and opaque queues
//-----------------------------------------------------------------------------
void BatchRenderer::QueueBatchItem(BatchItem* item){
	// Tests for a light being on the list twice
	/*for(int i=0;i<item->lights.size();i++){
	Light* one = item->lights[i];
	for(int j=0;j<item->lights.size();j++){
	Light* two = item->lights[j];
	if(two == one && i != j)
	one = one;
	}
	}*/

	if(!item->matRef)
		return;

	item->m_SHBatchItem = item->mesh->UsingPRT();

	if(!item->matRef->m_Opaque/* && !item->matRef->m_AlphaTest*/){
		CheckShadowMapping(item);

		//get smallest bbox vert distance from camera, for z-sorting
		float leastZ = BIG_NUMBER;
		for(int i = 0; i < 8; i++)
		{
			// Transform center to view space for faster z-sorting
			float Z = (s_ViewProjection * item->box.GetVert(i)).z;
			if(Z < leastZ)
				leastZ = Z;
		}
		item->Z_Value = leastZ;

		// Create completely unique batch for each item, because we musn't sort alpha by anything but z
		m_AlphaBatches.push_back(new ShaderBatch(item->matRef->m_Shader,item->matRef->GetTechnique(LIGHT_OMNI,item->mesh->UsingPRT())));
		m_AlphaBatches.back()->items.push_back(item);

		//set pertinent SH technique
		if(item->m_SHBatchItem)
		{
			m_AlphaBatches[m_AlphaBatches.size() - 1]->SHtechnique = item->matRef->GetTechnique(LIGHT_OMNI,true,false);
			if(item->mesh->m_bPRTMultiPass || !m_AlphaBatches[m_AlphaBatches.size() - 1]->SHtechnique)
				item->m_SHBatchItem = false;
		}

	}
	else
		AddToBatch(m_Batches,item);
}


//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void BatchRenderer::RenderFinalShaders(){
	if(RenderDevice::Instance()->PixelShaderVersion > 1.4 && !HDRSystem::Instance()->m_bEnabled)
		RenderShadows();
	SAFE_DELETE_VECTOR(m_ShadowItems.items);
}

//-----------------------------------------------------------------------------
// LDR rendering
// Renders occlusion test boxes and alpha objects
//-----------------------------------------------------------------------------
void BatchRenderer::RenderLDR(){
	m_SystemMaterial->m_Shader->SetTechnique(m_SystemMaterial->m_Shader->GetTechnique("Occlusion"));
	m_SystemMaterial->m_Shader->SetWorld(Matrix());
	m_SystemMaterial->m_Shader->Begin();
	m_SystemMaterial->m_Shader->BeginPass(0);

	for(int i=0;i<m_Batches.size();i++){
		for(int j=0;j<m_Batches[i]->items.size();j++){
			BatchItem* item = m_Batches[i]->items[j];
			if(item->usesOcclusion && (*item->occlusionQuery)){
				(*item->occlusionQuery)->Issue(D3DISSUE_BEGIN);

				// Draw object if it's going to be cheaper than the box
				// Remember, fillrate is our main worry, so more vertices are better than
				// more pixels
				// NOTE: Commented out, as we need to set matrices, probably too expensive
				//if(item->subset.FaceCount <= 36){
				//  mat->m_Shader->SetWorld(item->worldmatrix);
				//	item->mesh->DrawSubset(item->subset);
				//  mat->m_Shader->SetWorld(Matrix());
				//else{
				// Increase box by small amount so geometry doesn't occlude its own box
				BBox box = item->box;
				box.min -= Vector(0.1f,0.1f,0.1f);
				box.max += Vector(0.1f,0.1f,0.1f);
				RenderDevice::Instance()->GetCanvas()->Cube(box);
				//}

				(*item->occlusionQuery)->Issue(D3DISSUE_END);
			}
		}
	}
	m_SystemMaterial->m_Shader->EndPass();
	m_SystemMaterial->m_Shader->End();

	// Enable states the previous shader may have altered
	RenderWrap::SetRS(D3DRS_FOGENABLE,FALSE);
	RenderWrap::SetRS(D3DRS_COLORWRITEENABLE,0xFFFFFFFF);
	RenderWrap::SetRS(D3DRS_ZWRITEENABLE,FALSE);

	// Good time to render alpha components
	RenderWrap::SetRS(D3DRS_ALPHABLENDENABLE,TRUE);
	// Default alpha states
	RenderWrap::SetRS( D3DRS_ALPHAREF , 0xFF);
	RenderWrap::SetRS( D3DRS_ZWRITEENABLE , TRUE);
	RenderWrap::SetRS( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );
	RenderWrap::SetRS( D3DRS_SRCBLEND,D3DBLEND_SRCALPHA); 
	RenderWrap::SetRS( D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA); 

	if(!RenderDevice::Instance()->GetHDR())
	{
		RenderBatches_SHSinglePass(m_AlphaBatches);
		RenderBatches(m_AlphaBatches);
	}

	RenderWrap::SetRS(D3DRS_ALPHABLENDENABLE,FALSE);

	RenderFinalShaders();
}

//-----------------------------------------------------------------------------
// Delete items
//-----------------------------------------------------------------------------
void BatchRenderer::ClearAll(){
	SAFE_DELETE_VECTOR(m_Batches);
	SAFE_DELETE_VECTOR(m_AlphaBatches);
}

//-----------------------------------------------------------------------------
// Desc: Sorts batches and sends them down the pipeline
//
//-----------------------------------------------------------------------------
void BatchRenderer::PrepareQueuedBatches(Camera* cam, World* world)
{
	PrimaryLight = NULL;
	RenderingForWorld = world;
	bUpdatedStaticPRT = false;

	DWORD val;
	RenderWrap::dev->GetRenderState(D3DRS_ZWRITEENABLE,&val);
	m_ZWrites = val;
	bDoublePassAlpha = Engine::Instance()->MainConfig->GetBool("AlphaDoublePass");
	//StartMiniTimer();

	StartMiniTimer(); // RenderMS time start

	s_ViewProjection =  cam->view * cam->projection;
	s_CamPos = cam->Location;

	// Sub-sort batch by material...
	for(int k=0;k<m_Batches.size();k++){
		std::sort(m_Batches[k]->items.begin(),m_Batches[k]->items.end(),sort_by_material);
	}

	// Primary-sort by z...
	std::sort(m_AlphaBatches.begin(),m_AlphaBatches.end(),sort_by_z);

	// Find most intense singlepass Light in scene for key lighting effects like specular highlights
	Vector colorVecTest;
	Vector colorVecPrimary;
	for(int i = 0; i < RenderingForWorld->m_Lights.size(); i++)
	{
		if(RenderingForWorld->m_Lights[i]->m_ForceSHMultiPass)
			continue;

		FloatColor col = RenderingForWorld->m_Lights[i]->GetCurrentState().Diffuse;
		colorVecTest.Set(col.r,col.g,col.b);

		if(!PrimaryLight || RenderingForWorld->m_Lights[i]->GetCurrentState().Intensity*colorVecTest.Length() > PrimaryLight->GetCurrentState().Intensity*colorVecPrimary.Length())
		{
			PrimaryLight = RenderingForWorld->m_Lights[i];
			FloatColor col = PrimaryLight->GetCurrentState().Diffuse;
			colorVecPrimary.Set(col.r,col.g,col.b);
		}
	}
	if(RenderingForWorld->m_Lights.size())
		PrimaryLight = RenderingForWorld->m_Lights[0];

	// Note how long sorting took
	//RenderCPUMS += StopMiniTimer()*1000;

	if(m_bZPass)
		DoZPass();

	// If we have done a Z-Pass, we use CMP_EQUAL, and don't write to Z again
	RenderWrap::SetRS(D3DRS_ZWRITEENABLE,!m_bZPass);
	if(m_bZPass)
		RenderWrap::SetRS(D3DRS_ZFUNC,D3DCMP_EQUAL);

	// Default alpha states
	RenderWrap::SetRS( D3DRS_ALPHAREF , 0xFF);
	RenderWrap::SetRS( D3DRS_ZWRITEENABLE , TRUE);
	RenderWrap::SetRS( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );
	RenderWrap::SetRS( D3DRS_SRCBLEND,D3DBLEND_SRCALPHA); 
	RenderWrap::SetRS( D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA); 

	// Render all shadow maps now
	bool bFound = false;
	for(int i=0;i<world->m_Lights.size();i++){
		if(world->m_Lights[i]->m_ShadowMap && !world->m_Lights[i]->IsHidden){
			bFound = true;
			RenderShadowMap(world->m_Lights[i]);
		}
	}

	// Must clear Z after changing depth/sencil for shadow maps
	// TODO: Find out why!!!
	if(bFound)
		RenderWrap::dev->Clear( 0L, NULL, D3DCLEAR_ZBUFFER, 0x000000ff, 1.0f, 0L );

	RenderMS += StopMiniTimer()*1000;
}


//-----------------------------------------------------------------------------
// Desc: Does core rendernig of batches
//
//-----------------------------------------------------------------------------
void BatchRenderer::RenderQueuedBatches()
{
	StartMiniTimer();
	RenderBatches_SHSinglePass(m_Batches,RenderDevice::Instance()->GetHDR());
	RenderBatches(m_Batches,0,RenderDevice::Instance()->GetHDR());

	// Render alpha blending batches here if in HDR mode, otherwise we render them in the LDR stage later
	if(RenderDevice::Instance()->GetHDR())
	{
		RenderShadows();
		RenderBatches_FlipTargets(m_AlphaBatches,true);
		bool foundAlphaBlendBatches = false;
		for(int i = 0; i < m_Batches.size();i++)
		{
			if(!m_Batches[i]->items[0]->matRef->m_AlphaTest)
				continue;
			foundAlphaBlendBatches = true;
			for(int p = 0; p < m_Batches[i]->items.size();p++)
			{
				m_Batches[i]->items[p]->rendered = false;
			}
		}
		if(foundAlphaBlendBatches)
		{
			HDRSystem::Instance()->FlipTargets();
			RenderBatches_SHSinglePass(m_Batches,true,false);
			RenderBatches(m_Batches,0,true,false);
		}
	}

	// Put Z writes back for all subsequent rendering
	RenderWrap::SetRS(D3DRS_ZWRITEENABLE,TRUE);
	RenderWrap::SetRS(D3DRS_ZFUNC,D3DCMP_LESSEQUAL);

	// If no HDR, render LDR now. Otherwise RenderDevice will do it a bit later
	if(!RenderDevice::Instance()->GetHDR())
		RenderLDR();

	RenderWrap::dev->SetPixelShader(NULL);
	RenderWrap::dev->SetVertexShader(NULL);

	m_ZWrites = true;

	RenderMS += StopMiniTimer()*1000;
}


//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void BatchRenderer::DoZPass(){
	/*
	RenderWrap::SetRS(D3DRS_ZWRITEENABLE,TRUE);
	RenderWrap::SetRS(D3DRS_ZFUNC,D3DCMP_LESSEQUAL);
	RenderWrap::SetRS(D3DRS_ALPHABLENDENABLE,FALSE);
	*/
}

//-----------------------------------------------------------------------------
// NOTE: Fillrate cost to rendering boxes.
// Occlusion Geometry =
// +Entire scene! Frame coherence!
//
// Occlude Testers =
// +Boxes for all prefabs
//
// NOTE: Query only checks occlusion at draw time, not retrieve time :(
// That means we must draw boxes for everything
//
// Method:
// 1. Render entire scene. After finished, render all prefab boxes with queries on
// 2. Next frame, read back queries. Invisible objects are skipped, 
// and have query bboxes executed in their place
// 3. Repeat.
//
// Future Work:
// Hierarchical boxes. Occlusion tests for second passes on all geometry.
// Sky occlusion test
//-----------------------------------------------------------------------------
bool BatchRenderer::CheckOcclusionTest(BatchItem* p){
	if(p->usesOcclusion){
		if((*p->occlusionQuery) == NULL){
			// First-time init
			DXASSERT(RenderWrap::dev->CreateQuery(D3DQUERYTYPE_OCCLUSION,p->occlusionQuery));
			return FALSE;
		}

		// Get query from last frame...

		// If camera is within the box, the pixel test will obviously say no pixels rendered
		// so just render the object if it's that close
		BBox box = p->box;
		box.min -= Vector(0.3f,0.3f,0.3f); // Expand box to cover clip plane
		box.max += Vector(0.3f,0.3f,0.3f); // Expand box to cover clip plane
		if(box.IsPointInBox(s_CamPos))
			return FALSE;

		DWORD pixels = 5;
		while((*p->occlusionQuery)->GetData(&pixels,sizeof(DWORD),D3DGETDATA_FLUSH ) == S_FALSE);
		// If item was invisible last frame, do not render. We'll draw it's bbox at the end of the frame
		if(pixels < 5){ // less than 5 pixels visible
			return TRUE; // Do NOT render model
		}
	}
	return FALSE;
}

//-----------------------------------------------------------------------------
// Renders pools,  grouped by m_CurrentShader
//-----------------------------------------------------------------------------
void BatchRenderer::RenderBatches_FlipTargets(vector<ShaderBatch*>& batches, bool bHDR)
{
	PrimaryLight = NULL;
	UINT		passes			= 0;
	Material* lastMat = NULL;

	//	RenderWrap::SetRS(D3DRS_ALPHABLENDENABLE,FALSE);
	//	RenderWrap::SetRS(D3DRS_ALPHATESTENABLE, FALSE);

	m_FlushState	 = false;

	for(int a=0;a<batches.size();a++)
	{
		assert(batches[a]->items.size());

		m_CurrentShader  = batches[a]->shader;

		m_CurrentShader->SetTechnique(batches[a]->technique);

		passes	= m_CurrentShader->Begin();

		// Render all batch items
		// Items should be grouped by material, so we'll use redundant state filtering
		// to make sure we only set materials as they change
		for(int c=0;c<batches[a]->items.size();c++)
		{	
			BatchItem* p	= batches[a]->items[c];

			if(p->skipMe)
				continue;
			//if(p->rendered) // Rendered already. Used so two worlds can share a queue (HACKY)
			//	continue;

			// HDR: Record batch passes here
			p->passes = passes * p->GetNumberOfPerPixelLights();

			// May be no lights, so we do one pass
			if(!p->passes && (!p->m_SHBatchItem || !p->HasAnySinglePassSHLights()))
				p->passes = 1;

			if(CheckOcclusionTest(p) == TRUE)
				continue;

			//Set skinning if this is a possible skeletal mesh
			if(p->bone_matrices.size())
				p->mesh->SetSkinningRenderStates(p->matRef->m_Shader,p->subset,p->bone_matrices);
			else
				p->matRef->m_Shader->SetSkinning(0,0,NULL);

			if(lastMat != p->matRef)
			{
				p->matRef->Apply();
				lastMat = p->matRef;
			}

			// Render one SH pass if single-pass SH mesh
			if(p->m_SHBatchItem && p->HasAnySinglePassSHLights())
			{
				HDRSystem::Instance()->FlipTargets();

				m_CurrentShader->End();
				m_CurrentShader->SetTechnique(batches[a]->SHtechnique);
				if(PrimaryLight)
				{
					m_CurrentShader->SetLightShaderConstants(NULL,PrimaryLight,PrimaryLight,true,true,false);
					lastLight = PrimaryLight;
				}
				UINT SPpasses	= m_CurrentShader->Begin();
				m_CurrentShader->SetHDR(bHDR,0);
				RenderingForWorld->UpdateMeshPRT(p->mesh,p->transform,RenderingForWorld->m_Lights,true,!bUpdatedStaticPRT,true);
				if(!bUpdatedStaticPRT)
					bUpdatedStaticPRT = true;
				p->mesh->SetSHStates(p->matRef->m_Shader,p->subset);
				// Render single-SH pass...
				for(int b=0;b<SPpasses;b++)
				{
					m_CurrentShader->SetHDR(bHDR,false);

					m_CurrentShader->SetWorld(p->transform);
					m_CurrentShader->BeginPass(b);

					RenderBatchItem(*p,true);

					m_CurrentShader->CommitChanges();
					m_CurrentShader->EndPass(); 
				}

				if(p->passes)
				{
					m_CurrentShader->End();
					m_CurrentShader->SetTechnique(batches[a]->technique);
					m_CurrentShader->Begin();
				}
			}

			int curPass = 0;
			while(curPass < p->passes)
			{
				//do all lights sequentially
				int lightNum = bHDR?curPass:-1;

				//for alpha batches we have to fliptarget between every pass
				HDRSystem::Instance()->FlipTargets();
				m_CurrentShader->SetHDR(bHDR,curPass>0 || (p->m_SHBatchItem && p->HasAnySinglePassSHLights()));

				// Render all passes...
				for(int b=0;b<passes;b++)
				{

					m_CurrentShader->BeginPass(b);

					// Render Item!
					//TODO: Filter
					RenderBatchItemLit(*p,(!p->m_SHBatchItem || !p->HasAnySinglePassSHLights()) && ((!bHDR && (b==0)) || (bHDR && b==0 && lightNum == 0 && curPass == 0)), lightNum);

					m_CurrentShader->CommitChanges();
					m_CurrentShader->EndPass(); 
				}

				// HACK: To stop rendered being set before all HDR passes drawn
				if(curPass < p->passes-1)
					p->rendered = false;
				else
					p->rendered = true;

				curPass++;
			}
		}
		m_CurrentShader->UnbindHDRTarget();
		m_CurrentShader->End();
	}

	// All state has been flushed, resume normal filtering
	m_CurrentShader = NULL;
}

//-----------------------------------------------------------------------------
// Renders SH pools
//-----------------------------------------------------------------------------
void BatchRenderer::RenderBatches_SHSinglePass(vector<ShaderBatch*>& batches, bool bHDR, bool bAlphaTestPass)
{
	PrimaryLight = NULL;
	UINT	passes	= 0;
	m_FlushState	= true;
	bool hasExtraPerPixelPasses = false;

	for(int a=0;a<batches.size();a++)
	{
		if(!batches[a]->SHtechnique)
			continue;

		//if HDR blending but not an alpha-test material batch, don't render it again
		if(!bAlphaTestPass && !batches[a]->items[0]->matRef->m_AlphaTest)
			continue;

		m_CurrentShader  = batches[a]->shader;
		m_CurrentShader->SetTechnique(batches[a]->SHtechnique);
		if(PrimaryLight)
		{
			m_CurrentShader->SetLightShaderConstants(NULL,PrimaryLight,PrimaryLight,true,true,false);
			lastLight = PrimaryLight;
		}
		passes	= m_CurrentShader->Begin();

		// Render all batch items
		// Items should be grouped by material, so we'll use redundant state filtering
		// to make sure we only set materials as they change
		for(int c=0;c<batches[a]->items.size();c++)
		{	
			BatchItem* p	= batches[a]->items[c];

			if(!p->m_SHBatchItem || !p->mesh->UsingPRT())
				continue;

			if(p->skipMe)
				continue;

			//if(p->rendered) // Rendered already. Used so two worlds can share a queue (HACKY)
			//	continue;

			if(!p->HasAnySinglePassSHLights())
				continue;

			// HDR: Record batch passes here
			p->passes = passes;
			// Set static SH coefficients for this mesh, if SH mesh
			RenderingForWorld->UpdateMeshPRT(p->mesh,p->transform,RenderingForWorld->m_Lights,true,!bUpdatedStaticPRT && p->StaticObject,p->StaticObject);
			if(!bUpdatedStaticPRT && p->StaticObject)
				bUpdatedStaticPRT = true;

			p->mesh->SetSHStates(p->matRef->m_Shader,p->subset);

			static Material* lastMat = NULL;
			// TODO: Flush state
			//if(lastMat != p->matRef)
			{
				p->matRef->Apply();
				lastMat = p->matRef;
			}

			// Render all passes...
			for(int b=0;b<passes;b++)
			{
				m_CurrentShader->SetHDR(bHDR,false,bAlphaTestPass);

				m_CurrentShader->SetWorld(p->transform);
				m_CurrentShader->BeginPass(b);

				RenderBatchItem(*p,true);

				m_CurrentShader->CommitChanges();
				m_CurrentShader->EndPass(); 
			}

			p->rendered = false;

			if(!hasExtraPerPixelPasses)
				hasExtraPerPixelPasses = p->GetNumberOfPerPixelLights();
		}
		m_CurrentShader->UnbindHDRTarget();
		m_CurrentShader->End();
		m_CurrentShader->SetTechnique(batches[a]->technique);
	}

	if(hasExtraPerPixelPasses)
		HDRSystem::Instance()->FlipTargets();

	// All state has been flushed, resume normal filtering
	m_FlushState = false;
	m_CurrentShader = NULL;
}

extern bool m_UseConstantTable;
//-----------------------------------------------------------------------------
// Renders pools,  grouped by m_CurrentShader
//-----------------------------------------------------------------------------
void BatchRenderer::RenderBatches(vector<ShaderBatch*>& batches, int curPass, bool bHDR, bool bAlphaTestPass){
	D3DXHANDLE currentTechnique = NULL; // Local for redundancy
	m_CurrentShader				= NULL; // Class var because it's useful elsewhere
	UINT		passes			= 0;
	int			highestPass		= 0;    // So we know if we should continue recursing

	for(int a=0;a<batches.size();a++){
		assert(batches[a]->items.size());
		// Skip pass if no batches go that high
		if(curPass > 0 && curPass >= batches[a]->maxPasses)
			continue;

		//if HDR blending but not an alpha-test material batch, don't render it again
		if(!bAlphaTestPass && !batches[a]->items[0]->matRef->m_AlphaTest)
			continue;


		// If technique or shader has changed, apply it
		if(currentTechnique != batches[a]->technique || m_CurrentShader != batches[a]->shader){
			if(m_CurrentShader){ // End last m_CurrentShader
				// Unbind to stop debugger whining when we switch targets
				if(bHDR)
					m_CurrentShader->UnbindHDRTarget();
				m_CurrentShader->CommitChanges();
				m_CurrentShader->EndPass();
				m_CurrentShader->End();
			}
			m_CurrentShader  = batches[a]->shader;
			currentTechnique = batches[a]->technique;
			m_CurrentShader->SetTechnique(currentTechnique);
			//m_CurrentShader->SetHDR(bHDR,curPass>0,bAlphaTestPass);
			passes			 = m_CurrentShader->Begin();
			m_FlushState	 = true;
		}

		// Render all batch items
		// Items should be grouped by material, so we'll use redundant state filtering
		// to make sure we only set materials as they change
		for(int c=0;c<batches[a]->items.size();c++){
			BatchItem* p	= batches[a]->items[c];
			//if(p->rendered) // Rendered already. Used so two worlds can share a queue (HACKY)
			//	continue;

			if(p->skipMe)
				continue;

			int numLights = p->GetNumberOfPerPixelLights();
			// HDR: Record batch passes here
			p->passes = passes * numLights;

			// May be no lights, so we do one per-pixel pass if not an SH batchitem
			if(numLights == 0)
			{
				if(!p->m_SHBatchItem || !p->HasAnySinglePassSHLights())
					p->passes = 1;
				else
				{
					//all done with this SH batchitem
					p->rendered = true;
					continue;
				}
			}

			// To speed up calculations, record highest pass and highest batch pass
			if(p->passes > highestPass)
				highestPass			 = p->passes;
			if(p->passes > batches[a]->maxPasses)
				batches[a]->maxPasses = p->passes;

			if(curPass >= p->passes)
				continue;

			if(CheckOcclusionTest(p) == TRUE)
				continue;

			// FILTERED: Set skinning if this is a possible skeletal mesh
			static DWORD NumBones = -1;
			if(p->bone_matrices.size()){
				p->mesh->SetSkinningRenderStates(p->matRef->m_Shader,p->subset,p->bone_matrices);
				NumBones = p->bone_matrices.size();
			}
			else if(NumBones !=0 || m_FlushState){
				p->matRef->m_Shader->SetSkinning(0,0,NULL);
				NumBones = 0;
			}

			// FILTERED: Change material, but check for redundant material changes, as we've
			// grouped by material already.
			static Material* lastMat = NULL;
			if(lastMat != p->matRef || m_FlushState){
				p->matRef->Apply();
				lastMat = p->matRef;
			}

			// Render all passes...
			for(int b=0;b<passes;b++){
				// With HDR, do one light at a time, otherwise do all lights in one pass
				int lightNum = bHDR?curPass:-1;

				// FILTERED: Set pass
				static DWORD lastPass = -1;
				if(lastPass != b || m_FlushState){
					// TODO: Would it help if this were called earlier?
					m_CurrentShader->CommitChanges();
					m_CurrentShader->EndPass(); 
					// Set blend to false, to make sure previous passes don't interfere,
					// as we're not restoring state
					RenderWrap::SetRS(D3DRS_ALPHABLENDENABLE,FALSE);
					RenderWrap::SetRS(D3DRS_ALPHATESTENABLE, FALSE);
					m_CurrentShader->BeginPass(b);
					lastPass = b;
				}

				//TODO: Filter
				//Always additive blend for SH meshes
				m_CurrentShader->SetHDR(bHDR,curPass>0 || (p->m_SHBatchItem && p->HasAnySinglePassSHLights()),bAlphaTestPass);

				// Render Item!
				RenderBatchItemLit(*p,(!p->m_SHBatchItem || !p->HasAnySinglePassSHLights()) && ((!bHDR && (b==0)) || (bHDR && b==0 && lightNum == 0 && curPass == 0)), lightNum);
			}

			// HACK: To stop rendered being set before all HDR passes drawn
			if(curPass < p->passes-1)
				p->rendered = false;
		}

		// All state has been flushed, resume normal filtering
		m_FlushState = false;
	}

	if(m_CurrentShader){
		// Unbind to stop debugger whining when we switch targets
		m_CurrentShader->UnbindHDRTarget();
		m_CurrentShader->CommitChanges();
		m_CurrentShader->EndPass();
		m_CurrentShader->End(); // End last m_CurrentShader
	}

	// (Recursive) If there are more passes to do, render them now
	if(bHDR && highestPass > curPass+1){
		HDRSystem::Instance()->FlipTargets();
		RenderBatches(batches,curPass+1,bHDR,bAlphaTestPass);
	}
}



//-----------------------------------------------------------------------------
// Renders pools,  grouped by m_CurrentShader
//-----------------------------------------------------------------------------
void BatchRenderer::RenderShadows(){
	if(m_ShadowItems.items.size() == 0)
		return;

	m_CurrentShader = m_SystemMaterial->m_Shader;
	m_CurrentShader->SetTechnique("ShadowScene");
	RenderWrap::SetRS( D3DRS_FOGENABLE, FALSE );
	int passes	= m_CurrentShader->Begin();
	m_CurrentShader->BeginPass(0);

	// Render all batch items
	// Items should be grouped by material, so we'll use redundant state filtering
	// to make sure we only set materials as they change
	while(m_ShadowItems.items.size())
	{
		if(m_ShadowItems.items[0]->matRef->m_Opaque) // Only shadow opaque
		{
			//RENDER GROUP BY LIGHT FOR EFFICIENT RT FLIPPING
			if(HDRSystem::Instance()->m_bEnabled)
				HDRSystem::Instance()->FlipTargets();
			else
			{
				RenderWrap::SetRS(D3DRS_ALPHABLENDENABLE,TRUE);
				RenderWrap::SetRS(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA); 
				RenderWrap::SetRS(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA); 
			}
			m_CurrentShader->SetHDR(HDRSystem::Instance()->m_bEnabled,HDRSystem::Instance()->m_bEnabled);

			for(int i = m_ShadowItems.items.size() - 1; i >= 0;  i--)
			{
				if(m_ShadowItems.items[i]->lights[0] == m_ShadowItems.items[0]->lights[0])
				{
					BatchItem* p = m_ShadowItems.items[i];

					if(p->skipMe)
						continue;

					m_FlushState		  = true;
					ShaderVar* old		  = p->matRef->m_Emissive;
					p->matRef->m_Emissive = NULL; // HACK: Stop emissive getting set by clearing it
					RenderBatchItemLitShadows(*p);
					p->matRef->m_Emissive = old;
					p->rendered			  = false;
					delete m_ShadowItems.items[i];
					m_ShadowItems.items.erase(m_ShadowItems.items.begin() + i);
				}
			}
		}
		else
		{
			delete m_ShadowItems.items[0];
			m_ShadowItems.items.erase(m_ShadowItems.items.begin());
		}
	}

	m_CurrentShader->GetEffect()->SetTexture( "tSpotlight", NULL); // Stop debugger whining
	m_CurrentShader->CommitChanges();
	m_CurrentShader->EndPass();
	m_CurrentShader->End();
	RenderWrap::SetRS( D3DRS_FOGENABLE, FALSE );
}


//-----------------------------------------------------------------------------
// Shadow Mapping
//-----------------------------------------------------------------------------
void BatchRenderer::RenderShadowMap(Light* light){
	if(RenderDevice::Instance()->PixelShaderVersion < 2)  
		return;

	light->m_ShadowMap->BeginCapture();
	Matrix oldProj = RenderWrap::GetProjection();
	Matrix oldView = RenderWrap::GetView();
	RenderWrap::SetProjection(light->GetProjection());
	RenderWrap::SetView(light->GetView());//*(Matrix*)&mLightView);//

	m_CurrentShader = m_SystemMaterial->m_Shader;
	m_CurrentShader->SetTechnique("RenderShadow");
	m_CurrentShader->SetHDR(false,false);
	RenderWrap::SetRS( D3DRS_FOGENABLE, FALSE );
	int passes		= m_CurrentShader->Begin();
	m_CurrentShader->EndPass(); // In case
	m_CurrentShader->BeginPass(0);
	
	// Render all batch items
	// Items should be grouped by material, so we'll use redundant state filtering
	// to make sure we only set materials as they change
	for(int c=0;c<m_ShadowItems.items.size();c++){
		BatchItem* p	= m_ShadowItems.items[c];

		if(p->skipMe)
			continue;

		if(p->lights[0] != light)
			continue;

		ShaderVar* e = p->matRef->m_Emissive;
		D3DXCOLOR* ec = NULL;
		if(e)
			ec = (D3DXCOLOR*)e->data;

		if(!p->matRef->m_Opaque || (e && ((ec->r+ec->g+ec->b) >= 3))) // Only shadow opaque and non-emissive Materials (components sum to >= 3 considered fully emissive)
			continue;

		// Don't use RenderBatchItem, since it's for lighting
		if(p->usesTransform)
			m_CurrentShader->SetWorld(p->transform);
		else
			m_CurrentShader->SetWorld(Matrix());

		//Set skinning if this is a possible skeletal mesh
		if(p->bone_matrices.size())
			p->mesh->SetSkinningRenderStates(m_CurrentShader,p->subset,p->bone_matrices);
		else
			m_CurrentShader->SetSkinning(0,0,NULL);


		m_CurrentShader->GetEffect()->SetTexture( "tSpotlight", NULL); 
		m_CurrentShader->CommitChanges();
		RenderWrap::SetRS(D3DRS_ALPHABLENDENABLE,FALSE); 
		RenderWrap::SetRS(D3DRS_ZWRITEENABLE,TRUE);
		p->mesh->DrawSubset(p->subset);
	}

	m_CurrentShader->EndPass();
	m_CurrentShader->End(); // End last m_CurrentShader
	RenderWrap::SetRS( D3DRS_FOGENABLE, FALSE );
	light->m_ShadowMap->EndCapture();
	RenderWrap::SetProjection(oldProj);
	RenderWrap::SetView(oldView);
}
//-----------------------------------------------------------------------------
// Renders a pool lit by all world lights that cast on it
//-----------------------------------------------------------------------------
void BatchRenderer::RenderBatchItemLit(BatchItem& pool, bool IsFirstPass, int lightNum){
	BBox bw = pool.box; 
	// FILTERED: Set world transform
	//static Matrix lastMat;
	static bool   m_IdentityMatrixSet = false;
	if(pool.usesTransform /*&& (m_FlushState || !(pool.transform == lastMat))*/){
		m_CurrentShader->SetWorld(pool.transform);
		m_IdentityMatrixSet = false;
	}
	else if(!m_IdentityMatrixSet || m_FlushState){
		m_CurrentShader->SetWorld(Matrix());
		m_IdentityMatrixSet = true;
	}

	// Range initially covers all lights touching this item
	int start = 0;
	int end   = pool.GetNumberOfPerPixelLights();

	// If we were given a specific light number, just render it
	if(lightNum != -1 && end){
		start = lightNum;
		end   = lightNum+1;
	}

	static bool LastPRTEnabled = false;

	// Render specified light(s)
	for(int i=start;i<end;i++)
	{
		Light* light = pool.GetPerPixelLight(i);
		if(!light)
			continue;

		bool PRTEnabled = (pool.mesh->UsingPRT() && light->m_Method == LIGHT_METHOD_SH);
		if(PRTEnabled)
		{
			vector<Light*> lights;
			lights.push_back(light);
			RenderingForWorld->UpdateMeshPRT(pool.mesh,pool.transform,lights,IsFirstPass,false,false);
			lights.clear();
			pool.mesh->SetSHStates(pool.matRef->m_Shader,pool.subset);
		}

		// Set light constants with state filtering
		if(m_FlushState || light != lastLight || LastPRTEnabled != PRTEnabled)
		{
			if(m_FlushState)
				lastLight = NULL; // Force shader to upadate too

			m_CurrentShader->SetLightShaderConstants(pool.matRef,light,lastLight,PRTEnabled,LastPRTEnabled);

			lastLight = light;
			LastPRTEnabled = PRTEnabled;
		}
		else
			m_CurrentShader->SetLightShaderConstants(pool.matRef,light,light,LastPRTEnabled,LastPRTEnabled,false);

		if(!light->IsHidden)
		{
			RenderBatchItem(pool,IsFirstPass);
			IsFirstPass = false;
		}
	}
	// If no lights cast on it, just render it (in the dark)
	// If we've already done a black/zpass, only render if it's self-illuminating
	if(IsFirstPass){
		ShaderVar* e = pool.matRef->m_Emissive;
		D3DXCOLOR* ec = NULL;
		if(e)
			ec = (D3DXCOLOR*)e->data;

		if(!m_bZPass || (e && ((ec->r+ec->g+ec->b) > 0))){
			// Change lighting to black
			m_CurrentShader->SetUnlit();
			// Clear light cache, so next shader restores light
			lastLight = NULL;
			// Render!
			RenderBatchItem(pool,IsFirstPass);
		}
	}
	m_FlushState = false;
}

//-----------------------------------------------------------------------------
// Renders a pool lit by all world lights that cast on it
//-----------------------------------------------------------------------------
void BatchRenderer::RenderBatchItemLitShadows(BatchItem& pool){
	BBox bw = pool.box; 

	if(pool.usesTransform)
		m_CurrentShader->SetWorld(pool.transform);

	//Set skinning if this is a possible skeletal mesh
	if(pool.bone_matrices.size())
		pool.mesh->SetSkinningRenderStates(m_CurrentShader,pool.subset,pool.bone_matrices);
	else
		m_CurrentShader->SetSkinning(0,0,NULL);


	// Range initially covers all lights touching this item
	int start = 0;
	int end   = pool.lights.size();

	// Render specified light(s)
	for(int i=start;i<end;i++)
	{

		Light* light = pool.lights[i];

		if(!light->m_bShadowProjector)
			continue;

		// Set light constants with state filtering
		//if(m_FlushState || light != lastLight)
		//{
		//	if(m_FlushState)
		//		lastLight = NULL; // Force shader to upadate too

		m_CurrentShader->SetLightShaderConstants(pool.matRef,light,lastLight,false,false);
		//	lastLight = light;
		//}
		//else
		//	m_CurrentShader->SetLightShaderConstants(pool.matRef,light,light,false,false,false);

		if(!light->IsHidden)
			RenderBatchItem(pool,HDRSystem::Instance()->m_bEnabled);
	}

	m_FlushState = false;
}

//-----------------------------------------------------------------------------
// Blending function. Figures out blending states for multipass
//-----------------------------------------------------------------------------
void BatchRenderer::Blend(bool opaque, bool IsFirstPass){
	bool HDR = RenderDevice::Instance()->GetHDR();

	if(HDR)
		RenderWrap::SetRS(D3DRS_ALPHATESTENABLE,false);

	bool alpha = (!HDR && (!opaque || !IsFirstPass));

	if(m_CurrentShader->OverridesEngineMultipass()){
		// FIXME: Probably should be !IsFirstPass, but it's not necessary at the moment
		// as all shaders that use this are > the first pass.
		RenderWrap::SetRS(D3DRS_ZWRITEENABLE,FALSE);//!IsFirstPass);
		//RenderWrap::SetRS(D3DRS_ZFUNC,D3DCMP_NEVER);
		//RenderWrap::SetRS(D3DRS_ALPHABLENDENABLE,TRUE);
		//RenderWrap::SetRS(D3DRS_SRCBLEND,D3DBLEND_ZERO); 
		//RenderWrap::SetRS(D3DRS_DESTBLEND,D3DBLEND_ONE); 
		return;
	}

	if(!IsFirstPass){ 
		// 1:1 for passes > 0 
		RenderWrap::SetRS(D3DRS_DESTBLEND,D3DBLEND_ONE); 

		if(!opaque) // 1:Alpha if it contains alpha and it's a multi-blend
			RenderWrap::SetRS(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA); 
		else
			RenderWrap::SetRS(D3DRS_SRCBLEND,D3DBLEND_ONE);
	}

	RenderWrap::SetRS(D3DRS_ALPHABLENDENABLE,alpha); 
	RenderWrap::SetRS(D3DRS_ZWRITEENABLE,!alpha);
	if(!m_ZWrites)
		RenderWrap::SetRS(D3DRS_ZWRITEENABLE,FALSE);
}

//-----------------------------------------------------------------------------
// Render a single item
//-----------------------------------------------------------------------------
void BatchRenderer::RenderBatchItem(BatchItem& pool, bool IsFirstPass){
	// TODO: Just disable this without checking, so users don't need
	// to set emissive to > 1 to trigger the code
	// Custom shaders can use this for their emissive portions
	// First pass has self-illum/ambient, second pass or light doesn't, or it would over-bright
	// So see if there's any emissive RGB contribution
	ShaderVar* e = pool.matRef->m_Emissive;
	if(e){
		D3DXCOLOR* ec = (D3DXCOLOR*)e->data;
		if((ec->r+ec->g+ec->b) > 0 && !IsFirstPass){
			m_CurrentShader->SetVar(*e,&D3DXCOLOR(0,0,0,0));
			e->data = ec;
		}
		else
			e = NULL;
	}

	// Set fog to black (off) for passes > 0
	if(IsFirstPass)
		m_CurrentShader->SetFog(1.f/pool.world->m_FogDistance,pool.world->m_FogColor);
	else
		m_CurrentShader->SetFog(0,FloatColor());

	m_CurrentShader->CommitChanges();
	//m_CurrentShader->EndPass();
	//m_CurrentShader->BeginPass(0);
	// Restore previous states. Could do this per item not per render?
	m_CurrentShader->RestorePassStates();

	Blend(pool.matRef->m_Opaque,IsFirstPass);

	// Check for alpha-test, meaning we may use the double-render trick
	DWORD val;
	RenderWrap::dev->GetRenderState( D3DRS_ALPHATESTENABLE, &val);
	if(val){
		// Alpha-Test can use z-writes, doesn't need ordering
		RenderWrap::SetRS( D3DRS_ZWRITEENABLE , TRUE);
	}

	// If alpha test on, do double-pass trick
	if(val && bDoublePassAlpha){
		// Technically this should be on in the shader, but 3dsmax doesn't mind,
		// so dumb artists often leave it disabled
		RenderWrap::SetRS( D3DRS_ALPHABLENDENABLE, TRUE);
		pool.mesh->DrawSubset(pool.subset);
		RenderWrap::SetRS( D3DRS_ALPHATESTENABLE, FALSE);
		RenderWrap::SetRS( D3DRS_ZWRITEENABLE , FALSE);
		pool.mesh->DrawSubset(pool.subset);
	}
	else{
		pool.mesh->DrawSubset(pool.subset);
	}

	// Set emissive back to old value, if used
	if(e) 
		m_CurrentShader->SetVar(*e);

	pool.rendered = true;
}