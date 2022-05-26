//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
// BatchRenderer.cpp - Core Render Loop. Renders the most basic building blocks: BatchItems
// The renderer heavily batches and sorts all data
// 
// High-Level Optimizations: All outdoors batches use same light, prt data
// 
// TODO: Handle shader array indices changing shader (Skinning, SH, etc). 
// Figure this out when adding batches!!
//
// FIXME: Something funny with spotlights in warehouse. Removing lightfiltering fixes it.
//
// Re-Written: Sep 2004 (Tim Johnson)
//====================================================================================
#include <stdafx.h>
#include "BatchRenderer.h"
#include "HDR.h"
#include "ShadowMapping.h"
#include "Shader.h"
#include "Profiler.h"

// How often is PRT refreshed for meshes?
// The longer the time, the more we can spread the calcuations out.
// NOTE: Max update time will be g_PRTRefreshTime*2 due to the way calculations are spaced over frames
static const float g_PRTRefreshTime = 0;
// Used for z-sorting
static Matrix s_ViewProjection; 

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
BatchRenderer* BatchRenderer::Instance () 
{
	static BatchRenderer inst;
	return &inst;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
bool sort_by_z(const BatchItem* p1, const BatchItem* p2)
{
	return p1->Z_Value > p2->Z_Value;
}


//-----------------------------------------------------------------------------
// SortTrees allow arbitrary and efficient render sorting
// Their nodes can be configured arbitrarily. Here is an example tree:
//
// -Shader (Diffuse)
//   -Light (Omni01)
//     -Material (Floor)
//       -Transform (MainFloor)
//       -Transform (LowerFloor)
// -Shader (MixMap)
//   ...
//
//-----------------------------------------------------------------------------
class SortTree
{
public:
	typedef int Type;

	/// Each Node contains unique elements of one of the following types
	const static int T_RenderGroup	= (1<<0);		// No sub-type, group here
	const static int T_Pass			= (1<<1);
	const static int T_Transform	= (1<<2);
	const static int T_Light		= (1<<3);
	const static int T_Material		= (1<<4);
	const static int T_Technique	= (1<<5);
	const static int T_Shader		= (1<<6);
	const static int T_UnInitialized	= (1<<7);
	const static int T_RenderGroupZOrdered		= (1<<8);

protected:
	/// Grouping type for sub 'nodes'
	Type				subType;
	/// Nodes grouped by 'type'
	vector<SortTree*>	nodes;
	/// Node contents
	vector<BatchItem*>	items;
	/// Token BatchItem, specifying the instance type grouped here (such as mat pointer, light pointer, etc)
	BatchItem*			groupItem;   	   
	/// Keep this for initializing new tree nodes dynamically
	/// Defines sort order for tree. Use SetSorting() on root
	vector<Type>		nodeTypes;
	/// Depth of this node in tree, used for finding out type
	int					depth;

	/// Swap shader
	inline void ChangeShader(BatchItem* item);
	/// Swap material
	inline void ChangeMaterial(BatchItem* item);
	/// Swap light
	inline void ChangeLight(BatchItem* item);
	/// Swap technique
	inline void ChangeTechnique(BatchItem* item);
	/// Swap transform
	inline void ChangeTransform(BatchItem* item);
	/// Swap skinning
	inline void ChangeSkinning(BatchItem* item);
	/// Render all items
	inline void RenderAll(Type sortFlags);

	/// Ends technique, etc
	void EndTechnique();

	/// Render Recursively
	void Render(Type sortFlags=0);

	/// Blend for multipass
	void Blend(bool opaque, bool IsFirstPass);
public:

	/// Sets the tree sorting order
	void SetSorting(vector<Type>& sortTypes)
	{
		assert(depth == 0);
		subType = sortTypes[depth];
		nodeTypes = sortTypes;
	}

	/// Constructor
	SortTree(){ subType = T_UnInitialized; groupItem = 0; depth = 0; }

	/// Clear the tree
	void Clear()
	{
		groupItem = NULL;
		SAFE_DELETE_VECTOR(items);

		// Don't delete/resize nodes. This is to avoid flushing our nice little allocated tree
		// SAFE_DELETE_VECTOR(nodes);

		// End of the line. Clear sub-nodes in case this used to be a deeper tree, then return
		if(subType == T_RenderGroup || subType == T_RenderGroupZOrdered)
		{
			SAFE_DELETE_VECTOR(nodes);
			return;
		}

		// Initialize children
		for(int i=0;i<nodes.size();i++)
			nodes[i]->Clear();
	}

	/// Add an item to the tree (recursive), finds correct node
	void Add(BatchItem* item)
	{
		// This item may be in a sub-group, but add it as the groupItem so queries can fail here
		// rather than at the final node
		groupItem = item; 

		// This is an end-node with no further grouping, add
		if(subType == T_RenderGroup)
		{
			// Add to group
			items.push_back(item);
			return;
		}
		else if(subType == T_RenderGroupZOrdered)
		{
			// Add to group
			items.push_back(item);
			return;
		}
		// If this node has a subType, find the correct subNode to pass the type off to
		// We check the groupItem to ensure this
		else if(subType == T_Light)
		{
			for(int i=0;i<nodes.size();i++){
				if(!nodes[i]->groupItem || nodes[i]->groupItem->light == item->light){
					nodes[i]->Add(item);
					return;
				}
			}
		}
		else if(subType == T_Material)
		{
			for(int i=0;i<nodes.size();i++){
				if(!nodes[i]->groupItem || nodes[i]->groupItem->matRef == item->matRef){
					nodes[i]->Add(item);
					return;
				}
			}
		}
		else if(subType == T_Technique)
		{
			for(int i=0;i<nodes.size();i++){
				if(!nodes[i]->groupItem || nodes[i]->groupItem->technique == item->technique){
					nodes[i]->Add(item);
					return;
				}
			}
		}

		else if(subType == T_Shader)
		{
			for(int i=0;i<nodes.size();i++){
				if(!nodes[i]->groupItem || nodes[i]->groupItem->matRef->m_Shader == item->matRef->m_Shader){
					nodes[i]->Add(item);
					return;
				}
			}
		}

		// If we got here there is no appropriate node, so add one
		// NOTE: This should rarely be hit once the tree grows to peak size
		Profiler::Get()->TreeGrow++;
		SortTree* node  = new SortTree;
		node->depth		= depth+1;
		node->nodeTypes	= nodeTypes; 
		node->subType   = nodeTypes[depth+1];
		node->Add(item);
		nodes.push_back(node);
	}

	/// Rendering
	void DoRendering();

protected:
	// State-filtering parameters
	static D3DXHANDLE	m_CurTechnique;
	static Shader*		m_CurShader;
	static Material*	m_CurMaterial;
	static Light*		m_CurLight;
	static int			m_CurNumBones;
	static int			m_CurBlend;
	static Light*		m_LastLight;
	static int			m_CurFirstPass;
	static Light*		m_FlippedLight;

};

//-----------------------------------------------------------------------------
// SortTree Static Members
//-----------------------------------------------------------------------------
Shader*		SortTree::m_CurShader;
Material*	SortTree::m_CurMaterial;
int			SortTree::m_CurNumBones;
Light*		SortTree::m_CurLight;
D3DXHANDLE	SortTree::m_CurTechnique;
int			SortTree::m_CurBlend;
Light*		SortTree::m_LastLight;
int			SortTree::m_CurFirstPass;
Light*		SortTree::m_FlippedLight;


//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void SortTree::DoRendering()
{
	// Flush states
	m_CurShader		= 0;
	m_CurTechnique	= 0;
	m_CurMaterial	= 0;
	m_CurLight		= 0;
	m_CurNumBones	= -1;
	m_CurBlend		= -1;
	m_LastLight		= 0;
	m_CurFirstPass  = -1;

	Render(0);

	EndTechnique();
}


//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void SortTree::EndTechnique()
{
	if(m_CurTechnique && m_CurShader)
	{
		// Unbind to stop debugger whining when we switch targets
		m_CurShader->UnbindHDRTarget();
		m_CurShader->EndPass();
		m_CurShader->End();
	}
}

//-----------------------------------------------------------------------------
// Change to groupItem shader
//-----------------------------------------------------------------------------
void SortTree::ChangeShader(BatchItem* item)
{
	if(m_CurShader != item->matRef->m_Shader)
	{
		EndTechnique();
		Profiler::Get()->ShaderChanges++;

		m_CurShader = item->matRef->m_Shader;

		// A change of shader flushes the following states
		m_CurTechnique	= 0;
		m_CurMaterial	= 0;
		m_CurLight		= 0;
		m_CurNumBones   = -1;
		m_CurBlend		= -1;
		m_LastLight		= 0;
		m_CurFirstPass  = -1;
	}
}

//-----------------------------------------------------------------------------
// Change to groupItem material
//-----------------------------------------------------------------------------
void SortTree::ChangeMaterial(BatchItem* item)
{
	if(m_CurMaterial != item->matRef)
	{
		Profiler::Get()->MaterialChanges++;
		item->matRef->Apply();
		m_CurMaterial = item->matRef;
	}

}
//-----------------------------------------------------------------------------
// Change to groupItem light
//-----------------------------------------------------------------------------
void SortTree::ChangeLight(BatchItem* item)
{
	if(m_CurLight != item->light && item->light)
	{
		Profiler::Get()->LightChanges++;
		m_CurShader->SetLightShaderConstants(item->light);
		m_CurLight = item->light;
	}
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void SortTree::ChangeTransform(BatchItem* item)
{
	Profiler::Get()->TransformChanges++;
	m_CurShader->SetWorld(item->transform);
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void SortTree::ChangeTechnique(BatchItem* item)
{
	if(m_CurTechnique != item->technique && item->technique)
	{
		Profiler::Get()->TechniqueChanges++;
		EndTechnique();
		
		m_CurShader->SetTechnique(item->technique);
		m_CurShader->Begin();
		m_CurShader->BeginPass(0);
		m_CurTechnique = item->technique;
	}
}

//-----------------------------------------------------------------------------
// Recursive state-filtering render loop
//-----------------------------------------------------------------------------
void SortTree::Render(Type sortFlags)
{
	Profiler::Get()->SortRenders++;

	sortFlags |= subType;

	// This is a final node, just render all items
	if(subType == T_RenderGroup)
	{
		RenderAll(sortFlags);
	}
	else if(subType == T_RenderGroupZOrdered)
	{
		std::sort(items.begin(),items.end(),sort_by_z);
		RenderAll(sortFlags);
	}

	for(int i=0;i<nodes.size();i++)
	{
		if(!nodes[i]->groupItem)
			continue; // Empty node

		nodes[i]->Render(sortFlags);
	}
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void SortTree::ChangeSkinning(BatchItem* item)
{
	//Set skinning if this is a possible skeletal mesh
	if(m_CurNumBones != item->bone_matrices.size() || item->bone_matrices.size())
	{
		Profiler::Get()->SkinningChanges++;

		if(item->bone_matrices.size())
			item->mesh->SetSkinningRenderStates(item->matRef->m_Shader,item->subset,item->bone_matrices);
		else
			item->matRef->m_Shader->SetSkinning(0,0,NULL);
		m_CurNumBones = item->bone_matrices.size();

		// Have to restart technique after a skinning change (ugh)
		m_CurShader->CommitChanges();
		EndTechnique();
		m_CurShader->SetTechnique(item->technique);
		m_CurShader->Begin();
		m_CurShader->BeginPass(0);
	}
}

//-----------------------------------------------------------------------------
// Blending function. Figures out blending states for multipass
//-----------------------------------------------------------------------------
void SortTree::Blend(bool opaque, bool IsFirstPass){
	bool HDR = RenderDevice::Instance()->GetHDR();

	if(HDR)
		RenderWrap::SetRS(D3DRS_ALPHATESTENABLE,false);

	bool alpha = (!HDR && (!opaque || !IsFirstPass));

	if(m_CurShader->OverridesEngineMultipass()){
		// FIXME: Probably should be !IsFirstPass, but it's not necessary at the moment
		// as all shaders that use this are > the first pass.
		RenderWrap::SetRS(D3DRS_ZWRITEENABLE,FALSE);//!IsFirstPass);
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
	//if(!m_ZWrites)
	//	RenderWrap::SetRS(D3DRS_ZWRITEENABLE,FALSE);
}

//-----------------------------------------------------------------------------
// Core item rendering loop, for already batched items
//-----------------------------------------------------------------------------
void SortTree::RenderAll(Type sortFlags)
{
	for(int i=0;i<items.size();i++)
	{
		BatchItem* item = items[i];

		// Update any states that may have changed, with redundancy filtering
		// NOTE: Order is important due to dependencies (params depend on shader, etc)
		ChangeShader(item);
		ChangeTechnique(item);
		ChangeMaterial(item);
		ChangeLight(item);
		ChangeTransform(item);
		ChangeSkinning(item);

		if(item->mesh->UsingPRT())
		{
			// Update PRT data only every g_PRTRefreshTime
			if(item->mesh->m_UpdateTime < GSeconds-g_PRTRefreshTime)
			{
				item->world->UpdateStaticPRT(item->mesh);
				
				static float cycle = 0;
				item->mesh->m_UpdateTime = GSeconds + cycle;
				// Spread out updates one per frame
				cycle += GDeltaTime;
				if(cycle > g_PRTRefreshTime)
					cycle = 0;
			}

			// Must set shader constants every frame, regardless
			item->mesh->SetSHStates(m_CurShader,item->subset);
		}

		// TODO: Don't do this for unsorted alpha (double-pass and alpha-test)
		bool bAlphaBlend = !item->matRef->m_Opaque && HDRSystem::Instance()->m_bEnabled/* && 
			RenderDevice::Instance()->PixelShaderVersion < 3*/;

		// Must do target blend instead of alpha blend on multipass on HDR cards below ps3.0
		bool bTargetBlend = item->pFirstItem->passes && HDRSystem::Instance()->m_bEnabled && 
			RenderDevice::Instance()->PixelShaderVersion < 3;
		
		// Need to flip target i.e. light changed
		if(m_FlippedLight != m_CurLight || (bTargetBlend&&!m_CurBlend) || bAlphaBlend )
		{
			HDRSystem::Instance()->FlipTargets();
			Profiler::Get()->FlipTargets++;
			m_FlippedLight = m_CurLight;
		}

		if((int)bTargetBlend != m_CurBlend)
		{
			Profiler::Get()->SetHDRCalls++;
			m_CurShader->SetHDR(HDRSystem::Instance()->m_bEnabled,bTargetBlend);
			m_CurBlend = bTargetBlend;
		}

		// Fog with filtering
		if(m_CurFirstPass != (item->pFirstItem->passes==0))
		{
			if(item->pFirstItem->passes==0)
				m_CurShader->SetFog(1.f/item->world->GetFogDistance(),item->world->GetFogColor());
			else
				m_CurShader->SetFog(0,FloatColor());
			m_CurFirstPass = item->pFirstItem->passes==0;
		}

		
		m_CurShader->CommitChanges();

		// Restore previous states
		m_CurShader->RestorePassStates();
		// Set blending. Target blends do not have blending, so treat as first pass always (except for when alpha blending)
		Blend(item->matRef->m_Opaque,(item->pFirstItem->passes==0||(bTargetBlend&&!bAlphaBlend)));
		item->pFirstItem->passes++;

		item->mesh->DrawSubset(item->subset);
		Profiler::Get()->NumDraws++;
	}
}



SortTree m_MainTree;
SortTree m_AlphaTree;


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
BatchRenderer::BatchRenderer(){
	m_SystemMaterial = NULL;
	m_bZPass = FALSE;
}


//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void BatchRenderer::Initialize(){
	if(m_SystemMaterial)
		return; // Already initialize

	m_SystemMaterial = new Material;
	m_SystemMaterial->Initialize("System.fx","Black");

	m_MainTree.Clear();
	vector<SortTree::Type> sortType;
	sortType.push_back(SortTree::T_Light);
	sortType.push_back(SortTree::T_Material);
	
	// Grouping by material implies grouping by shader/technique, so these are redundant
	//sortType.push_back(SortTree::T_Shader);
	//sortType.push_back(SortTree::T_Technique);
	
	sortType.push_back(SortTree::T_RenderGroup);
	m_MainTree.SetSorting(sortType);

	m_AlphaTree.Clear();
	sortType.clear();
	//sortType.push_back(SortTree::T_Shader);
	//sortType.push_back(SortTree::T_Technique);
	//sortType.push_back(SortTree::T_Material);
	sortType.push_back(SortTree::T_RenderGroupZOrdered);
	m_AlphaTree.SetSorting(sortType);
}

//-----------------------------------------------------------------------------
// Add another item to our queue. Sorts transparent and opaque queues
//-----------------------------------------------------------------------------
void BatchRenderer::QueueBatchItem(BatchItem* item)
{
	// All unlit items will use black/null shader
	if(!item->light)
	{
		item->technique = m_SystemMaterial->m_Shader->GetCurrentTechnique();
		item->matRef = m_SystemMaterial;
	}

	// TODO: Support shadow projectors!!
	if(item->light && item->light->m_bShadowProjector)
		return;

	if(item->matRef->m_Opaque)
		m_MainTree.Add(item);
	// TODO: Only for sorted alpha, so don't include alpha test or alpha double-pass
	// because forcing z-sort without state batching is extremely painful
	else
	{
		// Get smallest bbox vert distance from camera, for z-sorting
		float leastZ = BIG_NUMBER;
		for(int i = 0; i < 8; i++)
		{
			// Transform center to view space for faster z-sorting
			float Z = (s_ViewProjection * item->box.GetVert(i)).z;
			if(Z < leastZ)
				leastZ = Z;
		}
		item->Z_Value = leastZ;

		m_AlphaTree.Add(item);
	}
}

//-----------------------------------------------------------------------------
// Desc: Sorts batches and sends them down the pipeline
//
//-----------------------------------------------------------------------------
void BatchRenderer::PrepareQueuedBatches(Camera* cam, World* world)
{
	s_ViewProjection =  cam->view * cam->projection;
}


//-----------------------------------------------------------------------------
// Desc: Does core rendernig of batches
//
//-----------------------------------------------------------------------------
void BatchRenderer::RenderQueuedBatches()
{
	// Default alpha states
	RenderWrap::SetRS( D3DRS_ALPHAREF , 0xFF);
	RenderWrap::SetRS( D3DRS_ZWRITEENABLE , TRUE);
	RenderWrap::SetRS( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );
	RenderWrap::SetRS( D3DRS_SRCBLEND,D3DBLEND_SRCALPHA); 
	RenderWrap::SetRS( D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA); 

	m_MainTree.DoRendering();
	m_MainTree.Clear();

	m_AlphaTree.DoRendering();
	m_AlphaTree.Clear();
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
void BatchRenderer::RenderLDR()
{
	
}

//-----------------------------------------------------------------------------
// Delete items
//-----------------------------------------------------------------------------
void BatchRenderer::ClearAll(){
	SAFE_DELETE_VECTOR(m_Batches);
	SAFE_DELETE_VECTOR(m_AlphaBatches);
}



//-----------------------------------------------------------------------------
// Renders pools,  grouped by m_CurrentShader
//-----------------------------------------------------------------------------
void BatchRenderer::RenderShadows()
{
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
void BatchRenderer::RenderShadowMap(Light* light)
{
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

		// Render here
	}

	m_FlushState = false;
}