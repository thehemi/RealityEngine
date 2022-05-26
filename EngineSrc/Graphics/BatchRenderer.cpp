//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
// BatchRenderer.cpp - Core Render Loop. Renders the most basic building blocks: BatchItems
// The renderer heavily batches and sorts all data
// 
// High-Level Optimizations: All outdoors batches use same light, prt data
// 
// TODO: Handle shader array indices changing shader (Skinning, SH, etc) better. 
//
// TODO: Make all double-pass alpha renders just use alpha-blend after XXX meters
//
//
// Re-Written: Sep 2004 (Tim Johnson)
//====================================================================================
#include <stdafx.h>
#include "BatchRenderer.h"
#include "HDR.h"
#include "ShadowMapping.h"
#include "Shader.h"
#include "Profiler.h"
#include "LODManager.h"
#include "SkyController.h"

// How often is PRT refreshed for meshes?
// The longer the time, the more we can spread the calcuations out.
// NOTE: Max update time will be g_PRTRefreshTime*2 due to the way calculations are spaced over frames
static const float g_PRTRefreshTime = 5.0f;
// Used for z-sorting
static Matrix s_ViewProjection; 
static Vector s_CamPos;

Technique* m_BlackTech;

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
// Compare light arrays, for batching
//-----------------------------------------------------------------------------
inline bool LightsMatch(vector<Light*>& l1, vector<Light*>& l2)
{
    if(l1.size() != l2.size())
        return false;

    for(int i=0;i<l1.size();i++)
        for(int j=0;j<l2.size();j++)
            if(l1[i] != l2[j])
                return false;
    return true;
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

	/// Alpha tree has special requirements 
	bool m_IsAlphaTree;

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
	void Blend(BatchItem* item, bool IsFirstPass);
public:

	/// Sets the tree sorting order
	void SetSorting(vector<Type>& sortTypes)
	{
		assert(depth == 0);
		subType = sortTypes[depth];
		nodeTypes = sortTypes;
	}

	/// Constructor
	SortTree(){ subType = T_UnInitialized; groupItem = 0; depth = 0; m_IsAlphaTree = false; }
    /// Destructor
    ~SortTree(){ Clear(); }

	/// Clear the tree
	void Clear()
	{
		groupItem = NULL;
		for(int i=0;i<items.size();i++)
		{
			items[i]->refCount--;
			if(items[i]->refCount == 0)
				delete items[i];
		}
		items.resize(0);

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
        // Calc sorting information for alpha items
        if(item->tech->m_AlphaBlend && m_IsAlphaTree && subType == T_RenderGroupZOrdered)
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
            item->Z_Value += leastZ;
        }

		// This item may be in a sub-group, but add it as the groupItem so queries can fail here
		// rather than at the final node
		groupItem = item; 

		// This is an end-node with no further grouping, add
		if(subType == T_RenderGroup)
		{
			// Add to group
			item->refCount++;
			items.push_back(item);
			return;
		}
		else if(subType == T_RenderGroupZOrdered)
		{
			// Add to group
			item->refCount++;
			items.push_back(item);
			return;
		}
		// If this node has a subType, find the correct subNode to pass the type off to
		// We check the groupItem to ensure this
		else if(subType == T_Light)
		{
			for(int i=0;i<nodes.size();i++){
				if(!nodes[i]->groupItem || LightsMatch(nodes[i]->groupItem->lights,item->lights)){
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
                if(!nodes[i]->groupItem || nodes[i]->groupItem->tech->Handle == item->tech->Handle){
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
		node->m_IsAlphaTree = m_IsAlphaTree;
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
	static int			m_CurNumBones;
	static int			m_CurBlend;
	static Light*		m_LastLight;
	static int			m_CurFirstPass;
    static Mesh*        m_LastMesh;
    static vector<Light*>*		m_CurLights;
	static vector<Light*>*		m_FlippedLights;
};

//-----------------------------------------------------------------------------
// SortTree Static Members
//-----------------------------------------------------------------------------
Shader*		SortTree::m_CurShader;
Material*	SortTree::m_CurMaterial;
int			SortTree::m_CurNumBones;
D3DXHANDLE	SortTree::m_CurTechnique;
int			SortTree::m_CurBlend;
Light*		SortTree::m_LastLight;
int			SortTree::m_CurFirstPass;
Mesh*       SortTree::m_LastMesh;
vector<Light*>*		SortTree::m_CurLights;
vector<Light*>*		SortTree::m_FlippedLights;


//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void SortTree::DoRendering()
{
	// Flush states
	m_CurShader		= 0;
	m_CurTechnique	= 0;
	m_CurMaterial	= 0;
	m_CurLights		= 0;
	m_CurNumBones	= -1;
	m_CurBlend		= -1;
	m_LastLight		= 0;
	m_CurFirstPass  = -1;
    m_FlippedLights = 0;
    m_LastMesh      = 0;

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

		// A change of technique flushes the following states
		//m_CurMaterial	= 0;
		//m_CurLights		= 0;
		m_CurNumBones   = -1;
		//m_CurBlend		= -1;
		//m_LastLight		= 0;
		m_CurFirstPass  = -1;
        //m_FlippedLights = 0;
        //m_LastMesh      = 0;
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
		m_CurLights		= 0;
		m_CurNumBones   = -1;
		m_CurBlend		= -1;
		m_LastLight		= 0;
		m_CurFirstPass  = -1;
        m_LastMesh      = 0;
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
    if(!item->lights.size())
		return;

    if(!m_CurLights || !LightsMatch(*m_CurLights,item->lights))
	{
		Profiler::Get()->LightChanges++;
		m_CurShader->SetLightShaderConstants(item->lights);
		m_CurLights = &item->lights;
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
	if(m_CurTechnique != item->tech->Handle && item->tech->Handle)
	{
		Profiler::Get()->TechniqueChanges++;
		EndTechnique();
		
		m_CurShader->SetTechnique(item->tech->Handle);
		m_CurShader->Begin();
		m_CurShader->BeginPass(0);
		m_CurTechnique = item->tech->Handle;
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
		
		// This is required on X800 or we see state issues (mostly with HDR). Why???
        // NOTE: Above comment is outdated, there was another reason for doing this on any card...
        if(item->bone_matrices.size())
        {
		    m_CurShader->CommitChanges();
		    EndTechnique();
		    m_CurShader->SetTechnique(item->tech->Handle);
		    m_CurShader->Begin();
		    m_CurShader->BeginPass(0);
            ChangeLight(item);
        }

        // Set here, so EndTechnique doesn't reset
        m_CurNumBones = item->bone_matrices.size();
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
// Blending function. Figures out blending states for multipass
//-----------------------------------------------------------------------------
void SortTree::Blend(BatchItem* item, bool FirstPass)
{
    if(!FirstPass) // Enable if multipass. Never disable, because we let shader decide the final alpha state
	    RenderWrap::SetRS(D3DRS_ALPHABLENDENABLE,TRUE); 
    // If doing double-blend trick, force alpha off for the alpha-test pass
    else if(!m_IsAlphaTree && item->tech->m_AlphaTest && item->tech->m_AlphaBlend)
    {
        // TIM: Necessary because alpha gets turned on in first pass, which it does
        RenderWrap::SetRS(D3DRS_ALPHABLENDENABLE,FALSE);
    }

    RenderWrap::SetRS( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );

	if(m_IsAlphaTree)
	{
		// Default alpha states for blend pass, where we do double-pass trick
		// See (GD) [Algorithms] grass
		// Func is flipped, so now only less than REF get rendered
        if(item->tech->m_AlphaTest && item->tech->m_AlphaBlend)
		    RenderWrap::SetRS( D3DRS_ALPHAFUNC, D3DCMP_LESS );
		// Don't render already rendered pixels
		RenderWrap::SetRS( D3DRS_ZFUNC,D3DCMP_LESS);
	}

	if(!FirstPass)
	{ 
        RenderWrap::SetRS(D3DRS_DESTBLEND,D3DBLEND_ONE);

		// 1:1 for passes > 0 
		if(item->tech->m_AlphaBlend) // 1:Alpha if it contains alpha and it's a multi-blend
			RenderWrap::SetRS(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA); 
		else
			RenderWrap::SetRS(D3DRS_SRCBLEND,D3DBLEND_ONE);
	}
}

//-----------------------------------------------------------------------------
// Core item rendering loop, for already batched items
//-----------------------------------------------------------------------------
void SortTree::RenderAll(Type sortFlags)
{
    bool bSupportsBlending = RenderDevice::Instance()->TargetSupportsBlending();
	bool bDidGlobalRTCopy = false;

	for(int i=0;i<items.size();i++)
	{
		BatchItem* item = items[i];

		// Custom per-item callback
		if(BatchRenderer::Instance()->m_Callback)
			if(!BatchRenderer::Instance()->m_Callback(item))
				continue;

        // Hack, skip double-pass blending in HDR on cards that can't do it
        if(m_IsAlphaTree && HDRSystem::Instance()->m_bEnabled && RenderDevice::Instance()->PixelShaderVersion < 3
            && item->tech->m_AlphaTest && item->tech->m_AlphaBlend)
        {
            continue;
        }

		// Update any states that may have changed, with redundancy filtering
		// NOTE: Order is important due to dependencies (params depend on shader, etc)
		ChangeShader(item);
        ChangeMaterial(item);
		ChangeTechnique(item);
		ChangeLight(item);
        if(item->world->m_bSetSHStates)
		    ChangeTransform(item);
		ChangeSkinning(item);

        if(SkyController::Instance && item->mesh->UsingPRT() && item->mesh != m_LastMesh && item->lights.size() && item->lights[0]->m_Method != LIGHT_METHOD_PERPIXEL)
		{

            Vector sunDir = SkyController::Instance->AmbientLight->GetCurrentState().Direction;
            FloatColor skyColor = SkyController::Instance->AmbientLight->GetCurrentState().Diffuse*SkyController::Instance->AmbientLight->GetCurrentState().Intensity;
			// Update if sky color has changed significantly, for example lightnign flash, time jump
            // etc
            bool colorChanged = fabsf(item->mesh->m_LastColor.r-skyColor.r) > 0.02f
            || fabsf(item->mesh->m_LastColor.g-skyColor.g) > 0.02f || fabsf(item->mesh->m_LastColor.b-skyColor.b) > 0.02f;
             
            colorChanged = colorChanged || fabsf(sunDir.Dot(item->mesh->m_LastSunDir)) > 0.1f;
            // Update PRT data
            if(colorChanged || !item->StaticObject)
			{
				if(item->StaticObject)
					item->world->UpdateStaticPRT(item->mesh);
				else
					item->world->UpdateDynamicPRT(item->mesh,item->transform,item->lights);

                item->mesh->m_LastColor = skyColor;
                item->mesh->m_LastSunDir = sunDir;
            }

            m_LastMesh = item->mesh;
		}

        // Must set SH states for every mesh
        if(item->world->m_bSetSHStates)
            item->mesh->SetSHStates(m_CurShader,item->subset);

		bool FirstPass = (m_IsAlphaTree && item->pFirstItem->alphaPasses<1) || (!m_IsAlphaTree && item->pFirstItem->passes<1);
        //FirstPass = FirstPass && !item->matRef->m_Shader->OverridesEngineMultipass();

        // Does card need to do hacky target blend
        bool bTargetBlend = !m_IsAlphaTree && !FirstPass && !bSupportsBlending;
        bTargetBlend = bTargetBlend || (item->matRef->m_Shader->OverridesEngineMultipass() && !bSupportsBlending);

        // Must do flip target instead of alpha blend on multipass if card doesn't support target blending
        // or shader wants a RenderTarget switch
        bool bForceFlip = item->matRef->m_Shader->m_Flags & Shader::ReadsColorBuffer; // && HDRSystem::Instance()->m_bEnabled;

        bool bJustFlipped = false;
        if(item->matRef->m_Shader->m_Flags & Shader::ReadsGlobalColorBuffer && !bDidGlobalRTCopy)
        {
            if(item->world->m_bDoFlips)
                HDRSystem::Instance()->FlipTargets();
            Profiler::Get()->FlipTargets++;
            bDidGlobalRTCopy = true;
        }
        // Need to flip target i.e. light changed
        else if((m_FlippedLights != m_CurLights && bTargetBlend) || bForceFlip) 
		{
			  if(item->world->m_bDoFlips)
				    HDRSystem::Instance()->FlipTargets();
				Profiler::Get()->FlipTargets++;
				m_FlippedLights = m_CurLights;
				bJustFlipped = true;
		}

#ifdef STRICT_TEST
        if(!bTargetBlend && item->lights.size() && item->lights[0]->m_bShadowProjector)
            Error("Non-blended shadow pass! %s %d",item->matRef->m_Name.c_str(),m_IsAlphaTree);
#endif

        // This is called on cards that don't support HDR alpha _OR_ when a fliptarget is requested by a shader
        // (so it's almost never called on 6800s and above)
        // (1) If we flip to the opposite target texture (so texture is updated)
        // (2) If blending modes change

        // Update only color buffer if force flip only, otherwise do full update, including HDR states
		if(item->matRef->m_Shader->m_Flags & Shader::ReadsLDRBuffer)
		{
            if(item->world->m_bDoFlips)
			    m_CurShader->SetColorBuffer(HDRSystem::Instance()->GetLDRColorBuffer());
		}
        else if(bForceFlip || item->matRef->m_Shader->m_Flags & Shader::ReadsGlobalColorBuffer)
		{
            if(item->world->m_bDoFlips)
                m_CurShader->SetColorBuffer(HDRSystem::Instance()->GetColorBuffer());
		}
        else if((!bSupportsBlending && ((int)bTargetBlend != m_CurBlend || m_IsAlphaTree)) || bJustFlipped)
		{
			Profiler::Get()->SetHDRCalls++;
			m_CurShader->SetHDR(HDRSystem::Instance()->m_bEnabled || bForceFlip,bTargetBlend,!m_IsAlphaTree);
			m_CurBlend = bTargetBlend;
		}

		// Fog with filtering
		if(m_CurFirstPass != FirstPass)
		{
			// Do fog on first pass`
			if(FirstPass)
				m_CurShader->SetFog(1.f/item->world->GetFogDistance(),item->world->GetFogColor());
			else
				m_CurShader->SetFog(1.f/item->world->GetFogDistance(),FloatColor());
			m_CurFirstPass = FirstPass;
		}

		m_CurShader->CommitChanges();
	
		if(!HDRSystem::Instance()->m_bEnabled && (item->matRef->m_Shader->m_Flags & Shader::ReadsGlobalColorBuffer || item->matRef->m_Shader->m_Flags & Shader::ReadsColorBuffer || item->matRef->m_Shader->m_Flags & Shader::ReadsLDRBuffer))
		{
		    for(int i=0;i<4;i++)
			    RenderWrap::dev->SetSamplerState( i, D3DSAMP_SRGBTEXTURE, FALSE);
		}

		// Restore material states
        RenderWrap::dev->SetRenderState(D3DRS_SRCBLEND,item->tech->m_SrcBlend);
	    RenderWrap::dev->SetRenderState(D3DRS_DESTBLEND,item->tech->m_DestBlend);
        // If using RT, don't enable alpha blending
	    RenderWrap::dev->SetRenderState(D3DRS_ALPHABLENDENABLE,item->tech->m_AlphaBlend&&!bForceFlip);
	    RenderWrap::dev->SetRenderState(D3DRS_ALPHATESTENABLE,item->tech->m_AlphaTest);
        // Set Zwrites
        RenderWrap::SetRS( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );
        RenderWrap::dev->SetRenderState(D3DRS_ZWRITEENABLE,FirstPass && !m_IsAlphaTree);
        RenderWrap::dev->SetRenderState( D3DRS_ZFUNC,D3DCMP_LESSEQUAL);

		// Set blending.
        if(bSupportsBlending && !m_CurShader->OverridesEngineMultipass())
	    	Blend(item,FirstPass);

		// Draw!
        if(item->world->m_bDrawMeshes)
            item->mesh->DrawSubset(item->subset);

		Profiler::Get()->NumDraws++;
		if(m_IsAlphaTree)
			item->pFirstItem->alphaPasses++;
		else
			item->pFirstItem->passes++;
	}
}



SortTree m_MainTree;
SortTree m_AlphaTree;


//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void BatchRenderer::InvalidateDeviceObjects()
{

}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
BatchRenderer::BatchRenderer()
{
	m_Callback = 0;
	m_SystemMaterial = NULL;
	m_bZPass = FALSE;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void BatchRenderer::DrawBatches(vector<BatchItem*>& items)
{
    // Default alpha states
	RenderWrap::SetRS( D3DRS_ALPHAREF , 0xFF);
	RenderWrap::SetRS( D3DRS_ZWRITEENABLE , TRUE);
	RenderWrap::SetRS( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );
	RenderWrap::SetRS( D3DRS_SRCBLEND,D3DBLEND_SRCALPHA); 
	RenderWrap::SetRS( D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA); 
	RenderWrap::SetRS( D3DRS_ZFUNC,D3DCMP_LESS);

    vector<SortTree::Type> sortType;
    sortType.push_back(SortTree::T_RenderGroup);

    SortTree tree;
    tree.Clear();
	tree.SetSorting(sortType);

    SortTree alphaTree;
    alphaTree.Clear();
	alphaTree.SetSorting(sortType);

    for(int i=0;i<items.size();i++)
        QueueBatchItem(items[i],&tree,&alphaTree);
    tree.DoRendering();
    alphaTree.DoRendering();
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
    sortType.push_back(SortTree::T_Shader);
    sortType.push_back(SortTree::T_Technique);
	sortType.push_back(SortTree::T_Light);
	sortType.push_back(SortTree::T_Material);
	sortType.push_back(SortTree::T_RenderGroup);
	m_MainTree.SetSorting(sortType);

	m_AlphaTree.m_IsAlphaTree = true;
	m_AlphaTree.Clear();
	sortType.clear();
	sortType.push_back(SortTree::T_Shader);
	//sortType.push_back(SortTree::T_Technique);
	//sortType.push_back(SortTree::T_Material);
	sortType.push_back(SortTree::T_RenderGroupZOrdered);
	m_AlphaTree.SetSorting(sortType);
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void BatchRenderer::QueueBatchItem(BatchItem* item)
{		
    QueueBatchItem(item,&m_MainTree,&m_AlphaTree);
}

//-----------------------------------------------------------------------------
// Add another item to our queue. Sorts transparent and opaque queues
//-----------------------------------------------------------------------------
void BatchRenderer::QueueBatchItem(BatchItem* item, SortTree* tree, SortTree* alphaTree)
{
    if(!m_BlackTech)
        m_BlackTech = m_SystemMaterial->GetTechnique("black");

    // Which lights can be collapsed into the same pass?
    // Loop over all lights, if it uses a different pass then add to a new batch
    //
    // TODO: Support better grouped collapsing. This just checks previous
    // light parsed, not all groups created. 
    // So if we had: SH, PerPixel, SH, PerPixel We'd get 4 batches instead of 2 ;-(
    //
    //
    LPCSTR lastHandle = 0;
    BatchItem* lastItem = item;
    for(int i=0;i<item->lights.size();i++)
    {
        // PRT on? Also Treat baked vertex lighting as PRT
        bool PRTEnabled		= (item->mesh->UsingPRT()|| item->mesh->UsingColorVertex()) && item->lights[i]->m_Method == LIGHT_METHOD_SH;
        //
        bool lightMapping = item->mesh->HasLightMapping() && item->lights[i]->m_Method == LIGHT_METHOD_SH;
        Technique* t = item->matRef->GetTechnique(item->lights[i]->m_Type,PRTEnabled,item->lights[i]->m_bShadowProjector,lightMapping);
        
        if(
            // If technique+light not supported, don't render
            !t || 
            // If shadows turned off and this is a shadow, don't render
            (item->lights[i]->m_bShadowProjector && !RenderDevice::Instance()->GetShadows()) ||
			// if the light isn't actually visible for some reason (maybe IsHidden or Intensity == 0)
			!item->lights[i]->IsVisible()
			)
        {
            item->lights.erase(item->lights.begin()+i);
            i--;
            continue;
        }
        //
        // Create a new batch if technique has changed, or max lights has been exceeded
        //
        if(i>0 && (t->Handle != lastHandle || (lastItem->lights.size()+1 > t->MaxLights)))
        {
            // This needs a new batch
            BatchItem* item2 = new BatchItem;
            *item2 = *item;

            item2->tech	= t;
            item2->lights.resize(1);
            item2->lights[0] = item->lights[i];

            //
            // NOTE: This assumes foolishly that shadow projectors will not cast light
            //
			if((!item2->tech->m_AlphaBlend || item2->tech->m_AlphaTest) && !item->lights[i]->m_bShadowProjector)
		        tree->Add(item2);

//            bool noDoubleBlend = item2->tech->m_AlphaBlend && item2->tech->m_AlphaTest && RenderDevice::Initialize()->GetAlpha

	        if(item2->tech->m_AlphaBlend || item->lights[i]->m_bShadowProjector)
	        {
		        alphaTree->Add(item2);
	        }

            // Light has new batch, so remove from current batch
            item->lights.erase(item->lights.begin()+i);
            i--;
            lastItem = item2;
        }
        // Matches last item, so add to its light list
        else if(lastItem!=item)
        {
            lastItem->lights.push_back(item->lights[i]);
            item->lights.erase(item->lights.begin()+i);
            i--;
        }
        // Not grouped, so this will be item's core technique
        else
        {
            item->tech = t;
        }
        
        // Doesn't need a new batch, continue looping, keep note of last handle
        lastHandle = t->Handle;
    }

    // All invalid or unlit items will use black/null shader
    if(!item->lights.size())
    {
        item->tech = m_BlackTech;
        item->matRef = m_SystemMaterial;
    }

    if((!item->tech->m_AlphaBlend || item->tech->m_AlphaTest)  && (!item->lights.size() || !item->lights[0]->m_bShadowProjector))
        tree->Add(item);

    if(item->tech->m_AlphaBlend || (!item->lights.size() || item->lights[0]->m_bShadowProjector))
    {
        alphaTree->Add(item);
    }

}

//-----------------------------------------------------------------------------
// Desc: Sorts batches and sends them down the pipeline
//
//-----------------------------------------------------------------------------
void BatchRenderer::PrepareQueuedBatches(Camera* cam, World* world)
{
    s_CamPos = cam->Location;
	s_ViewProjection =  cam->view * cam->projection;

    m_BlackTech = m_SystemMaterial->GetTechnique("black");
    // Reapply HDR after messing with shadows
   // HDRSystem::Instance()->FlipTargets(false);
}


//-----------------------------------------------------------------------------
// Desc: Does core rendernig of batches
//
//-----------------------------------------------------------------------------
void BatchRenderer::RenderQueuedBatches(World* world)
{
	// Default alpha states
	RenderWrap::SetRS( D3DRS_ALPHAREF , 0xFF);
	RenderWrap::SetRS( D3DRS_ZWRITEENABLE , TRUE);
	RenderWrap::SetRS( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );
	RenderWrap::SetRS( D3DRS_SRCBLEND,D3DBLEND_SRCALPHA); 
	RenderWrap::SetRS( D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA); 
	RenderWrap::SetRS( D3DRS_ZFUNC,D3DCMP_LESS);

	m_MainTree.DoRendering();
    m_AlphaTree.DoRendering(); 
	RenderWrap::SetRS( D3DRS_ZFUNC,D3DCMP_LESSEQUAL);
}


//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void BatchRenderer::RenderFinalShaders(){
	
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
void BatchRenderer::ClearAll()
{
	m_MainTree.Clear();
	m_AlphaTree.Clear();
}