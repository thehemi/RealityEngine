//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
/// Embodies water, with scene refraction and reflection
///
/// Author: Tim Johnson
/// Based on http://graphics.cs.lth.se/theses/projects/projgrid/projgrid-hq.pdf
///
/// TODO: Fix damn sun scaling relative to skybox
///
/// FIXME: HDR Flips won't work on rendertargets
///
/// NOTE: If you start seeing white at intersections, the clipping plane needs rising
/// NOTE: If you start seeing refractions before an object is underwater, the clipping plane needs lowering ;-)
//====================================================================================
#include "stdafx.h"
#include "surface.h"
#include "Software_Noisemaker.h"
#include "WaterSurface.h"
#include "BatchRenderer.h"
#include "HDR.h"
#include "Profiler.h"
#include "SkyController.h"

static float s_height = 0;
WaterSurface* WaterSurface::instance = 0;

//--------------------------------------------------------------------------------------
// Initialize the water
//--------------------------------------------------------------------------------------
WaterSurface::WaterSurface(World* world) : Actor(world)
{
    instance = this;
	m_HasIncludeExcludeList = true;
	bInitialized = false;
	StaticObject = true;
	surf = 0; 
	m_CurPreset = m_LastPreset = m_NewTextureSize = m_LastGridX = -1;
    MaxTideHeight = 10;
	Initialize(64,256);
}


//--------------------------------------------------------------------------------------
// Initialize the water
//--------------------------------------------------------------------------------------
bool WaterSurface::Initialize(int gridSize, int texSize)
{
    if(Engine::Instance()->IsDedicated())
        return true;

    m_ForceOcclusionTest = true;
    m_IsExcludeList = false;
    Location = Vector(0,0,0);
	bExportable = true;
	CollisionFlags = CF_BBOX;
	GhostObject = true;
    m_HasIncludeExcludeList = true;

	m_bReflect = true;
	m_bRefract = true;
	m_Name = "Water Surface";
	bInitialized = true;
	m_LastGridX = gridSize;
	m_NewTextureSize = texSize;
	m_GridVertsX = gridSize;
	m_GridVertsY = gridSize*2;

	surf = new Surface();

	/*
	add_parameter(p_fSunPosAlpha,	"Sun location horizontal",	dtFloat);
	add_parameter(p_fSunPosTheta,	"Sun location vertical",	dtFloat);		
	add_parameter(p_fSunShininess,	"Sun shininess",			dtFloat);
	add_parameter(p_fSunStrength,	"Sun strength",				dtFloat);
	*/

	surf->fLODbias = 0.0f;
	surf->bDisplayTargets, false;
	surf->fElevation=	7.0f;

	surf->bAsPoints = false;
	surf->bDisplayTargets = false;
	surf->bDiffuseRefl = false;
	surf->bReflRefrStrength=0.1f;
	surf->fOpacity = 0.0f;
	
	surf->Initialize(&D3DXVECTOR3(0,0,0),&D3DXVECTOR3(0,1,0),m_GridVertsX,m_GridVertsY, texSize);

	//
	// Set default parameters
	//
	surf->m_NoiseMaker->fStrength=	0.9f;
	surf->m_NoiseMaker->bDisplace=	true;
	surf->m_NoiseMaker->iOctaves = 6;
	surf->m_NoiseMaker->fScale = 0.38f;
	surf->m_NoiseMaker->fFalloff = 0.607f;
	surf->m_NoiseMaker->fAnimspeed=	1.4f;
	surf->m_NoiseMaker->fTimemulti=	1.27f;
	surf->m_NoiseMaker->bPaused = false;
	surf->m_NoiseMaker->fDampeningRadius = 0;
 
#ifdef CPU_NORMALS
	surf->m_NoiseMaker->bSmooth = 	false;
#else
	surf->m_NoiseMaker->bSmooth = 	true;
#endif
	

	LoadPresent(1);
	
	EditorVars.clear();
	EditorVars.push_back(EditorVar("Reflection",&m_bReflect," Water - Main","Reflect scene in water? Setup via include lists"));
	EditorVars.push_back(EditorVar("Refraction",&m_bRefract," Water - Main","Refract items under water? Auto-renders objects underwater"));
	EditorVars.push_back(EditorVar("Water Preset",&m_CurPreset," Water - Main","Index from 1 to 6 for preconfigured settings (won't update vars unless you reselect)"));
	EditorVars.push_back(EditorVar("Tessellation",&m_GridVertsX," Water - Main","Perspective grid tessellation for waves. Pow2!"));
	EditorVars.push_back(EditorVar("Texture size",&m_NewTextureSize," Water - Main","For reflections/refractions. Pow2!"));

	EditorVars.push_back(EditorVar("Noise strength",&surf->m_NoiseMaker->fStrength,"Water","Physical height/Intensity of waves"));
	EditorVars.push_back(EditorVar("Toggle displacement",&surf->m_NoiseMaker->bDisplace,"Water","Physical wave displacement on/off"));
	EditorVars.push_back(EditorVar("Smooth heightmap",&surf->m_NoiseMaker->bSmooth,"Water","Fixes distant wave artifacts"));
	EditorVars.push_back(EditorVar("Reflection/Refraction Offsetting",&surf->bReflRefrStrength,"Water","How much to distort image based on waves"));
	EditorVars.push_back(EditorVar("Opacity",&surf->fOpacity,"Water","Opacity of refractions"));
	
	EditorVars.push_back(EditorVar("Octaves",&surf->m_NoiseMaker->iOctaves,"Water","Wave octaves. Don't mess with this much!"));
	EditorVars.push_back(EditorVar("Noise scale",&surf->m_NoiseMaker->fScale,"Water","How fine/small are the waves"));
	EditorVars.push_back(EditorVar("Noise falloff",&surf->m_NoiseMaker->fFalloff,"Water","Falloff for waves, higher=finer"));

	EditorVars.push_back(EditorVar("Animation speed",&surf->m_NoiseMaker->fAnimspeed,"Water","Overall speed"));
	EditorVars.push_back(EditorVar("Animation multi",&surf->m_NoiseMaker->fTimemulti,"Water","Time multiplier for octaves"));
	EditorVars.push_back(EditorVar("Pause animation",&surf->m_NoiseMaker->bPaused,"Water"));

	EditorVars.push_back(EditorVar("Water Color",&surf->fWaterColour,"Water"));
	EditorVars.push_back(EditorVar("Projector elevation",&surf->fElevation,"Water","Only alter if you have distortion artifacts"));
	EditorVars.push_back(EditorVar("Pause animation",&surf->m_NoiseMaker->bPaused,"Water"));
	EditorVars.push_back(EditorVar("Wave Dampening Radius",&surf->m_NoiseMaker->fDampeningRadius,"Water","Radius in meters around (0,0,0) to suppress waves"));
	
	EditorVars.push_back(EditorVar("Mipmap LOD Bias",&surf->fLODbias,"Water","Only alter if you have aliasing artifacts"));
	//EditorVars.push_back(EditorVar("Diffuse sky reflection",&surf->bDiffuseRefl,"Water"));
	
	EditorVars.push_back(EditorVar("Render as points",&surf->bAsPoints,"Water Debugging/Visualization aid"));
	//EditorVars.push_back(EditorVar("Display rendertargets",&surf->bDisplayTargets,"Water Debugging"));

    EditorVars.push_back(EditorVar("Max Tide Height",&MaxTideHeight,"Water","Maximum height of tide. 0=No tide"));
	
	return true;
}

//--------------------------------------------------------------------------------------
// Preset water configuration
//--------------------------------------------------------------------------------------
void WaterSurface::LoadPresent(int n)
{
	m_CurPreset = n;
	switch(n)
	{
	case 1:
		surf->m_NoiseMaker->fScale = 0.38f;
		surf->m_NoiseMaker->fStrength=	0.9f;
		surf->m_NoiseMaker->fFalloff = 0.607f;
		surf->fWaterColour = FloatColor(0.07f,0.11f,0.11f);
		surf->fLODbias = 0.0f;
		surf->bAsPoints = false;
		surf->bDiffuseRefl = false;
		break;
	case 2:
		surf->m_NoiseMaker->fScale = 0.38f;
		surf->m_NoiseMaker->fStrength=	4.0f;
		surf->m_NoiseMaker->fFalloff = 0.47f;
		surf->fWaterColour = FloatColor(0.13f,0.19f,0.22f);
		surf->fLODbias = 4.5f;
		surf->bAsPoints = false;
		surf->bDiffuseRefl = false;
		break;
	case 3:
		surf->m_NoiseMaker->fScale = 0.38f;
		surf->m_NoiseMaker->fStrength=	7.0f;
		surf->m_NoiseMaker->fFalloff = 0.53f;
		surf->fWaterColour = Vector(0.12f,0.22f,0.29f);
		surf->fLODbias = 10.0f;
		surf->bAsPoints = false;
		surf->bDiffuseRefl = true;
		break;
	case 4:
		surf->m_NoiseMaker->fScale = 0.38f;
		surf->m_NoiseMaker->fStrength=	4.0f;
		surf->m_NoiseMaker->fFalloff = 0.47f;
		surf->fWaterColour = FloatColor(0.13f,0.19f,0.22f);
		surf->fLODbias = 4.5f;
		surf->bAsPoints = true;
		surf->bDiffuseRefl = true;
		break;
	case 5:
		surf->m_NoiseMaker->fScale = 0.197f;
		surf->m_NoiseMaker->fStrength=	12.9f;
		surf->m_NoiseMaker->fFalloff = 0.467f;
		surf->fWaterColour = Vector(0.12f,0.20f,0.24f);
		surf->fLODbias = 	0.0f;
		surf->bAsPoints = false;
		surf->bDiffuseRefl = true;
		break;
	case 6:
		surf->m_NoiseMaker->fScale = 0.38f;
		surf->m_NoiseMaker->fStrength=	11.3f;
		surf->m_NoiseMaker->fFalloff = 0.56f;
		surf->fWaterColour = FloatColor(0.17f,0.27f,0.26f);
		surf->fLODbias = 	0.0f;
		surf->bAsPoints = false;
		surf->bDiffuseRefl = false;
		break;
	}
}


//--------------------------------------------------------------------------------------
float getheight(float x, float z)
{
	//	return displacement*0.5*(1+sin(0.15*x)*sin(z));
	return 0;
}
//--------------------------------------------------------------------------------------
void set_clipplane_height(float height)
{
	float plane[4];

	RenderWrap::dev->GetClipPlane(0,plane);

	plane[3] = height;

	RenderWrap::dev->SetClipPlane(0,plane);
}
//--------------------------------------------------------------------------------------
enum rs_mode {
	rsm_normal=0,
	rsm_refraction,
	rsm_reflection,
	rsm_reflection_backface,
};

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void WaterSurface::Tick()
{
    CollisionBox = BBox(Vector(-9999,Location.y-5,-9999),Vector(9999,Location.y+5,9999));
}

//-----------------------------------------------------------------------------
// Name: Render()
// Desc: Draws everything
//-----------------------------------------------------------------------------
void WaterSurface::Render(Camera* cam)
{	
	if(IsHidden)
		return;

    s_height = Location.y;
    StartMiniTimer();

    if(SkyController::Instance && !IsSelected)
    {
        // Get fraction of day from 0 to 1
        float frac = SkyController::Instance->GetDayTimeMinutes() / 1440;
        bool rising = true;
        // 4 tides cycles a day of course.
        if(frac < 0.25)
        {
            rising = true;
        }
        else if(frac < 0.5)
        {
            frac -= .25f;
            rising = false;
        }
        else if(frac < 0.75)
        {
            frac -= .5f;
            rising = true;
        }
        else
        {
            frac -= 0.75;
            rising = false;
        }

        // Put into correct time scale
        float scale = GDeltaTime * SkyController::Instance->GetMinutesPerGameSecond()/(1440/4);

        // Now frac will always be 0-0.25, so multiply by 4 then the max tide height
        if(rising)
            Location.y += frac*4*MaxTideHeight*scale;
        else
            Location.y -= frac*4*MaxTideHeight*scale;
    }

    surf->SetPosition(&D3DXVECTOR3(0,Location.y,0));
    
	// If exclude list has changed, update
	if(m_ExcludeListHandles.size() != m_ExcludeList.size())
	{
		m_ExcludeListHandles.clear();

		for(int i=0;i<MyWorld->m_Actors.size();i++)
		{
			Model* model = MyWorld->m_Actors[i]->MyModel;
			// For exclude list, assume everything is reflected
			if(model && m_IsExcludeList)
				model->CustomFlags |= FLAG_REFLECTED;
			// For include list, assume not reflected, and remove flag
			if(model && !m_IsExcludeList)
				model->CustomFlags = model->CustomFlags&~FLAG_REFLECTED;
		}

		for(int k=0;k<m_ExcludeList.size();k++)
		{
			Model* found = 0;
			for(int i=0;i<MyWorld->m_Actors.size();i++)
			{
				if(MyWorld->m_Actors[i]->m_Name == m_ExcludeList.at(k))
				{
					found = MyWorld->m_Actors[i]->MyModel;
				}
			}
			if(found)
			{
				m_ExcludeListHandles.push_back(found);
			}

			// Set flag on model if include list and found
			if(found && !m_IsExcludeList)
				found->CustomFlags |= FLAG_REFLECTED;

			if(!found)
			{
				m_ExcludeList.erase(m_ExcludeList.begin() + k);
				k--;
			}
		}
	}

	RenderDevice::Instance()->SetDefaultStates();

	// See if preset has changed
	if(m_LastPreset != m_CurPreset)
	{
        if(m_LastPreset != -1)
		    LoadPresent(m_CurPreset);
		m_LastPreset = m_CurPreset;
	}

	// See if grid size has changed. We assume Y = X*2
	if(m_LastGridX != m_GridVertsX)
	{
        if(m_LastGridX != -1)
		    surf->SetGridSize(m_GridVertsX,m_GridVertsX*2);
		m_LastGridX = m_GridVertsX;
	}

	// See if texture has changed
	if(m_NewTextureSize != surf->m_TargetSize)
	{
        if(m_NewTextureSize != -1)
		    surf->SetTextureSize(m_NewTextureSize);
	}

	surf->Prepare(cam);

    Profiler::Get()->WaterCPUMS += StopMiniTimer();

    StartMiniTimer();
	// render reflections/refractions into textures
	//bool wasEnabled = HDRSystem::Instance()->m_bEnabled;
	//HDRSystem::Instance()->m_bEnabled = false;
	RenderRefractedScene(cam);
	RenderReflectedScene(cam);		
	//HDRSystem::Instance()->m_bEnabled = wasEnabled;

    FloatColor col = surf->fWaterColour;
    if(SkyController::Instance)
        col = col * SkyController::Instance->AmbientLight->GetCurrentState().Diffuse*SkyController::Instance->AmbientLight->GetCurrentState().Intensity;

    // Clear color must be same as water
    RenderDevice::Instance()->SetClearColor(col.DWORDColor());
	
    if (surf->bAsPoints)
		RenderWrap::dev->Clear( 0, NULL,D3DCLEAR_ZBUFFER|D3DCLEAR_TARGET, D3DCOLOR_XRGB(255,255,255), 1.0f, 0 );
	//else
    RenderDevice::Instance()->UpdateViewport();
	
    // Messing with depth stencils requires we clear the Z-Buffer in DX Debug mode, or z-sorting is wrong
    // Why????
    //RenderWrap::dev->Clear( 0, NULL,D3DCLEAR_ZBUFFER|D3DCLEAR_TARGET, 0 , 1.0f, 0 );

	// Begin the scene		
	RenderDevice::Instance()->SetDefaultStates();

	// set rendering states		
	RenderWrap::dev->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
	RenderWrap::dev->SetRenderState(D3DRS_ZWRITEENABLE,true);
	RenderWrap::dev->SetRenderState(D3DRS_ZFUNC,D3DCMP_LESSEQUAL);
	RenderWrap::dev->SetRenderState(D3DRS_ALPHABLENDENABLE,false);		

	// render sea
	RenderWrap::dev->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE  );	
	surf->Render(MyWorld);	

	RenderWrap::dev->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW  );	
	RenderDevice::Instance()->SetDefaultStates();

    Profiler::Get()->WaterRenderMS += StopMiniTimer();
}

static int s_mode;

//--------------------------------------------------------------------------------------
// Custom rendering callback
//--------------------------------------------------------------------------------------
bool RefractItem(BatchItem* item)
{
	// If in refraction mode, only get objects touching or below the water
	if(s_mode == rsm_refraction && item->box.min.y > s_height)
		return false;
	// If not included, return false
	if(s_mode == rsm_reflection && !(item->model->CustomFlags & FLAG_REFLECTED))
		return false;
	// Cancel out this pass so it doesn't do additive in subsequent passes
	item->pFirstItem->passes--;
	return true;
}

//--------------------------------------------------------------------------------------
// Renders scene objects
// rsm_reflection = render objects above water
// rsm_refraction = render objects below water
//--------------------------------------------------------------------------------------
void render_scene(int mode, World* world)
{
	s_mode = mode;
	// If in refraction mode, only get objects touching or below the water
	BatchRenderer::Instance()->m_Callback = RefractItem;

	BatchRenderer::Instance()->RenderQueuedBatches(world);

	// Restore callback
	BatchRenderer::Instance()->m_Callback = 0;
}


//--------------------------------------------------------------------------------------
// Refracted scene is rendered to m_RefractionTex
//--------------------------------------------------------------------------------------
void WaterSurface::RenderRefractedScene(Camera* cam)
{
    LPDIRECT3DSURFACE9 oldDepth;
	RenderWrap::dev->GetDepthStencilSurface(&oldDepth);

	// set rendertarget
	LPDIRECT3DSURFACE9 target,bb;
	RenderWrap::dev->GetRenderTarget(0, &bb );
	surf->m_RefractionTex->GetSurfaceLevel( 0,&target );
	RenderWrap::dev->SetRenderTarget(0, target);

    RenderWrap::dev->SetDepthStencilSurface( surf->depthstencil );

	/*RenderWrap::dev->Clear( 0, NULL,D3DCLEAR_ZBUFFER, 0, 1.0f, 0 );
	surf->render_cutter();
	RenderWrap::dev->SetRenderState( D3DRS_ZFUNC, D3DCMP_GREATEREQUAL  );	*/

    FloatColor col = surf->fWaterColour;
    if(SkyController::Instance)
        col = col * SkyController::Instance->AmbientLight->GetCurrentState().Diffuse*SkyController::Instance->AmbientLight->GetCurrentState().Intensity;

    RenderWrap::dev->Clear( 0, NULL,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, col.DWORDColor(), 1.0f, 0 );
	
	D3DXMATRIX scale;	
	D3DXMatrixScaling( &scale, 1, 0.75, 1 );
    // Compensate for Y-scaling if the water is at any height but 0
    scale._42 = surf->m_Pos.y*0.25f;
	// squach the scene
	ShaderManager::Instance()->SetWorldWarp(*(Matrix*)&scale);
	RenderWrap::dev->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW  );	

	// add a clip-plane as well	

	// Calculate a clip plane to avoid rendering below-surface objects
	D3DXPLANE   planeNew;
    D3DXPLANE   plane;
    // Create plane to cut off rendering above water (causes refraction errors). Allow 1.5f for waves
    D3DXPlaneFromPointNormal(&plane,&D3DXVECTOR3(0,surf->m_Pos.y+1.5f,0),&D3DXVECTOR3(0,-1,0));
	D3DXPlaneNormalize(&plane, &plane);
	D3DXMATRIX matrix = *(D3DXMATRIX*)&(cam->viewProjection);
	// D3DXPlaneTransform needs inverse transpose
	D3DXMatrixInverse(&matrix, NULL, &matrix);
	D3DXMatrixTranspose(&matrix, &matrix);
	D3DXPlaneTransform(&planeNew, &plane, &matrix);

	RenderWrap::dev->SetClipPlane(0,planeNew);
	RenderWrap::dev->SetRenderState( D3DRS_CLIPPLANEENABLE, 1);

    // Disable fog for underwater items
    float fog = MyWorld->GetFogDistance();
    MyWorld->SetFog(MyWorld->GetFogColor(),99999999);
	if(m_bRefract)
		render_scene(rsm_refraction,MyWorld);
    MyWorld->SetFog(MyWorld->GetFogColor(),fog);

	// restore
	RenderWrap::dev->SetRenderState( D3DRS_ZFUNC, D3DCMP_LESSEQUAL );
	RenderWrap::dev->SetRenderState( D3DRS_CLIPPLANEENABLE, 0);
	ShaderManager::Instance()->SetWorldWarp(Matrix());
	RenderWrap::dev->SetRenderTarget(0, bb);

    RenderWrap::dev->SetDepthStencilSurface( oldDepth );
	SAFE_RELEASE(oldDepth);


	SAFE_RELEASE(target);
	SAFE_RELEASE(bb);
}


//--------------------------------------------------------------------------------------
// Reflected scene is flipped along Y then rendered to m_ReflectionTex
//--------------------------------------------------------------------------------------
void WaterSurface::RenderReflectedScene(Camera* cam)
{
	LPDIRECT3DSURFACE9 oldDepth;
	RenderWrap::dev->GetDepthStencilSurface(&oldDepth);

	// set rendertarget
	LPDIRECT3DSURFACE9 target,bb;
	RenderWrap::dev->GetRenderTarget(0, &bb );
	surf->m_ReflectionTex->GetSurfaceLevel( 0,&target );
	RenderWrap::dev->SetRenderTarget(0, target);
	RenderWrap::dev->SetDepthStencilSurface( surf->depthstencil );

	SAFE_RELEASE(target);

	// alpha & z must be cleared
	RenderWrap::dev->Clear( 0, NULL,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0, 1.0f, 0 );

	if(m_bReflect)
	{
		// mirror the scene
		D3DXMATRIX scale;	
		D3DXMatrixScaling( &scale, 1, -1, 1 );
		ShaderManager::Instance()->SetWorldWarp(*(Matrix*)&scale);

		RenderWrap::dev->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW  );		

		// Calculate a clip plane to avoid rendering below-surface objects
		D3DXPLANE   planeNew;
		D3DXPLANE   plane(0,-1,0,0);
		D3DXPlaneNormalize(&plane, &plane);
		D3DXMATRIX matrix = *(D3DXMATRIX*)&(cam->viewProjection);
		// D3DXPlaneTransform needs inverse transpose
		D3DXMatrixInverse(&matrix, NULL, &matrix);
		D3DXMatrixTranspose(&matrix, &matrix);
		D3DXPlaneTransform(&planeNew, &plane, &matrix);

		RenderWrap::dev->SetClipPlane(0,planeNew);
		RenderWrap::dev->SetRenderState( D3DRS_CLIPPLANEENABLE, 1);

		render_scene(rsm_reflection,MyWorld);

		// restore
		RenderWrap::dev->SetRenderState( D3DRS_CLIPPLANEENABLE, 0);
		ShaderManager::Instance()->SetWorldWarp(Matrix());
	}

	RenderWrap::dev->SetRenderTarget(0, bb);
	RenderWrap::dev->SetDepthStencilSurface( oldDepth );
	SAFE_RELEASE(oldDepth);

	SAFE_RELEASE(bb);
}