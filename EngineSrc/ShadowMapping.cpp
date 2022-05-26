//=========== (C) Copyright 2003, Tim Johnson. All rights reserved. ===========
//	Shadow Mapping!
//
// TODO: Renderdevice registration system, no static callbacks need
// must derive all classes from abstract class
//=============================================================================
#include "stdafx.h"
#include "ShadowMapping.h"
#include "dxstdafx.h"
#include "BatchRenderer.h"
#include "LODManager.h"

void SplitByBox(BBox& box, vector<Face>& faces, vector<Face>& boxFaces);
static const bool bHardwareShadowMapping = true;

LPDIRECT3DVOLUMETEXTURE9      ShadowMap::m_pJitterTexture = 0;
static const int JITTER_SIZE = 32;
static const int JITTER_SAMPLES = 8;


//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has been 
// reset, which will happen after a lost device scenario. This is the best location to 
// create D3DPOOL_DEFAULT resources since these resources need to be reloaded whenever 
// the device is lost. Resources created here should be released in the OnLostDevice 
// callback. 
//--------------------------------------------------------------------------------------
void ShadowMap::Initialize(int size)
{
	m_Size = size;
	HRESULT hr;
/*
    if(!m_pJitterTexture)
    {
        DXASSERT(RenderWrap::dev->CreateVolumeTexture(JITTER_SIZE, JITTER_SIZE, JITTER_SAMPLES*JITTER_SAMPLES/2, 1, 
            D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pJitterTexture, NULL));

        // Build the jitter texture
        D3DLOCKED_BOX lb;
        m_pJitterTexture->LockBox(0, &lb, NULL, 0);

        unsigned char *data = (unsigned char *)lb.pBits;

        for (int i = 0; i<JITTER_SIZE; i++) {
            for (int j = 0; j<JITTER_SIZE; j++) {
                float rot_offset = ((float)rand() / RAND_MAX - 1) * 2 * 3.1415926f;

                for (int k = 0; k<JITTER_SAMPLES*JITTER_SAMPLES/2; k++) {

                    int x, y;
                    float v[4];

                    x = k % (JITTER_SAMPLES / 2);
                    y = (JITTER_SAMPLES - 1) - k / (JITTER_SAMPLES / 2);

                    v[0] = (float)(x * 2 + 0.5f) / JITTER_SAMPLES;
                    v[1] = (float)(y + 0.5f) / JITTER_SAMPLES;
                    v[2] = (float)(x * 2 + 1 + 0.5f) / JITTER_SAMPLES;
                    v[3] = v[1];
                    
                    // jitter
                    v[0] += ((float)rand() * 2 / RAND_MAX - 1) / JITTER_SAMPLES;
                    v[1] += ((float)rand() * 2 / RAND_MAX - 1) / JITTER_SAMPLES;
                    v[2] += ((float)rand() * 2 / RAND_MAX - 1) / JITTER_SAMPLES;
                    v[3] += ((float)rand() * 2 / RAND_MAX - 1) / JITTER_SAMPLES;

                    // warp to disk
                    float d[4];
                    d[0] = sqrtf(v[1]) * cosf(2 * 3.1415926f * v[0] + rot_offset);
                    d[1] = sqrtf(v[1]) * sinf(2 * 3.1415926f * v[0] + rot_offset);
                    d[2] = sqrtf(v[3]) * cosf(2 * 3.1415926f * v[2] + rot_offset);
                    d[3] = sqrtf(v[3]) * sinf(2 * 3.1415926f * v[2] + rot_offset);

                    d[0] = (d[0] + 1.0) / 2.0;
                    data[k*lb.SlicePitch + j*lb.RowPitch + i*4 + 0] = (unsigned char)(d[0] * 255);
                    d[1] = (d[1] + 1.0) / 2.0;
                    data[k*lb.SlicePitch + j*lb.RowPitch + i*4 + 1] = (unsigned char)(d[1] * 255);
                    d[2] = (d[2] + 1.0) / 2.0;
                    data[k*lb.SlicePitch + j*lb.RowPitch + i*4 + 2] = (unsigned char)(d[2] * 255);
                    d[3] = (d[3] + 1.0) / 2.0;
                    data[k*lb.SlicePitch + j*lb.RowPitch + i*4 + 3] = (unsigned char)(d[3] * 255);
                }
            }
        }
        m_pJitterTexture->UnlockBox(0);
    }
*/

    
    // Create the depth-stencil buffer to be used with the shadow map
    // We do this to ensure that the depth-stencil buffer is large
    // enough and has correct multisample type/quality when rendering
    // the shadow map.  The default depth-stencil buffer created during
    // device creation will not be large enough if the user resizes the
    // window to a very small size.  Furthermore, if the device is created
    // with multisampling, the default depth-stencil buffer will not
    // work with the shadow map texture because texture render targets
    // do not support multisample.
    DXUTDeviceSettings d3dSettings = DXUTGetDeviceSettings();

    D3DFORMAT zFormat = D3DFMT_D24S8;
    D3DFORMAT colorFormat = D3DFMT_A8R8G8B8;
    //
    // NVIDIA hardware shadow-mapping, create depth texture instead of surface
    //
    if(RenderDevice::Instance()->PixelShaderVersion >= 3 && bHardwareShadowMapping)
    {
         
        if( (zFormat == D3DFMT_D16) ||
            (zFormat == D3DFMT_D15S1) )
            colorFormat = D3DFMT_R5G6B5;

        if(FAILED(RenderWrap::dev->CreateTexture(m_Size, m_Size, 1, D3DUSAGE_DEPTHSTENCIL, D3DFMT_D24S8, 
        D3DPOOL_DEFAULT, &m_pTexDepthShadow, NULL)))
            return;

        DXASSERT( m_pTexDepthShadow->GetSurfaceLevel( 0, &m_pDSShadow ));
    }
    else
    {
	    V( RenderWrap::dev->CreateDepthStencilSurface( m_Size,m_Size,
                                                        DXUTGetDeviceSettings().pp.AutoDepthStencilFormat,
                                                        D3DMULTISAMPLE_NONE,
                                                        0,
                                                        TRUE,
                                                        &m_pDSShadow,
                                                        NULL ) );
    }

	 // Create the shadow map texture
	if(RenderDevice::Instance()->PixelShaderVersion < 2){
		V( RenderWrap::dev->CreateTexture( m_Size, m_Size,
											1, D3DUSAGE_RENDERTARGET,
											D3DFMT_X8R8G8B8,
											D3DPOOL_DEFAULT,
											&m_pShadowMap,
											NULL ) );
	}
	else{
		V( RenderWrap::dev->CreateTexture( m_Size, m_Size,
											1, D3DUSAGE_RENDERTARGET,
											D3DFMT_R32F,
											D3DPOOL_DEFAULT,
											&m_pShadowMap,
											NULL ) );
	}

    // Capture nothing, so not garbage if the renderer doesn't capture anything at runtime (out of view, whatever)
    BeginCapture();
    EndCapture();
}

void ShadowMap::FreeResources()
{
    SAFE_RELEASE(m_pJitterTexture);
    SAFE_RELEASE(m_pDSShadow);
    SAFE_RELEASE(m_pShadowMap);
    SAFE_RELEASE(m_pTexDepthShadow);
}

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
LPDIRECT3DTEXTURE9 ShadowMap::GetMap()
{ 
    if(RenderDevice::Instance()->PixelShaderVersion >= 3 && bHardwareShadowMapping)
        return m_pTexDepthShadow ; 
    else
        return m_pShadowMap;
}

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void ShadowMap::BeginCapture(){
	//
    // Render the shadow map
    //
    DXASSERT( RenderWrap::dev->GetRenderTarget( 0, &m_pOldRT ) );
    LPDIRECT3DSURFACE9 pShadowSurf;
    DXASSERT( m_pShadowMap->GetSurfaceLevel( 0, &pShadowSurf ));

    RenderWrap::dev->SetRenderTarget( 0, pShadowSurf );
    SAFE_RELEASE( pShadowSurf );

    DXASSERT( RenderWrap::dev->GetDepthStencilSurface( &m_pOldDS ));
    RenderWrap::dev->SetDepthStencilSurface( m_pDSShadow );

	DXASSERT(RenderWrap::dev->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
                          0xffffffff, 1.0f, 0L ) );
}


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void ShadowMap::EndCapture(){
	if( m_pOldDS )
    {
        RenderWrap::dev->SetDepthStencilSurface( m_pOldDS );
        m_pOldDS->Release();
    }
    RenderWrap::dev->SetRenderTarget( 0, m_pOldRT );
    SAFE_RELEASE( m_pOldRT );
}



//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
static ShadowEngine inst;
ShadowEngine* ShadowEngine::Instance () 
{
	return &inst;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
ShadowEngine::ShadowEngine()
{
    MaxVisibleShadows = 20;
    MaxShadowDistance = 40;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
HRESULT ShadowEngine::OnResetDevice()
{
    ShadowsEnabled = Engine::Instance()->MainConfig->GetBool("DropShadows");
    m_pShadowMaps.resize(MaxVisibleShadows);
    for(int i=0;i<MaxVisibleShadows;i++)
    {
        m_pShadowMaps[i] = new ShadowTarget;

        DXASSERT( RenderWrap::dev->CreateTexture( 256, 256,
            1, D3DUSAGE_RENDERTARGET,
            D3DFMT_A8R8G8B8,
            D3DPOOL_DEFAULT,
            &m_pShadowMaps[i]->map,
            NULL ) );
    }
    if(!m_BlobShadow.IsLoaded())
        m_BlobShadow.Load("shadow_test.tga");

    // After a reset all RTS need to be flushed from actors, but we don't have an actors
    // array (can't trust world list if editor is holding actors)
    // Therefore we check all actors against this time and flush
    m_LastReset = GSeconds;
    return S_OK;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ShadowEngine::OnLostDevice()
{
    for(int i=0;i<m_pShadowMaps.size();i++)
        SAFE_DELETE(m_pShadowMaps[i]);
    m_pShadowMaps.clear();
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void ShadowEngine::RenderShadowItem(ModelFrame* pFrame, Shader* m_CurShader)
{
	if(!pFrame)
		return;

	RenderShadowItem(pFrame->pFrameFirstChild,m_CurShader);
	RenderShadowItem(pFrame->pFrameSibling,m_CurShader);

	if(!pFrame->GetMesh())
		return;

	// TODO: Add back LOD support
    Mesh*	mesh		= pFrame->GetMesh();//ChooseLOD(camera,pFrame);

    for(int i=0;i<mesh->m_AttribTable.size();i++)
    {
        // TODO: Simply make shadow lighter if subset has alpha
        // If subset uses alpha blending it will not be written to the shadow map
        //if(mesh->GetMaterial(mesh->m_AttribTable[i].AttribId)->GetTechnique(LIGHT_SPOT,mesh->UsingPRT(),false,false)->m_AlphaBlend)
        //    continue;

		m_CurShader->SetWorld(pFrame->CombinedTransformationMatrix);

		//Set skinning if this is a possible skeletal mesh
		if(pFrame->bone_matrices.size())
			mesh->SetSkinningRenderStates(m_CurShader,mesh->m_AttribTable[i],pFrame->bone_matrices);
		else
			m_CurShader->SetSkinning(0,0,NULL);

		//m_CurrentShader->GetEffect()->SetTexture( "tSpotlight", NULL); 
		m_CurShader->CommitChanges();
		RenderWrap::SetRS(D3DRS_ALPHABLENDENABLE,FALSE); 
		RenderWrap::SetRS(D3DRS_ZWRITEENABLE,TRUE);
        mesh->DrawSubset(mesh->m_AttribTable[i]);
	}
}

//-----------------------------------------------------------------------------
// Shadow Mapping
//-----------------------------------------------------------------------------
void ShadowEngine::RenderShadowMaps(World* world, Camera* cam)
{
	RenderDevice::Instance()->SetDefaultStates();

    // Unbind all textures, in case shadow map was used by earlier pass
    RenderDevice::Instance()->UnbindTextures();
    //
    // Render any shadow maps held by world lights
    //
    for(int i=0;i<world->m_Lights.size();i++)
    {
        if(world->m_Lights[i]->m_ShadowMapItems.size())
            RenderShadowMap(world->m_Lights[i],cam);
    }

    // Shadows messes with the states some how
    RenderDevice::Instance()->SetDefaultStates();
}


//-----------------------------------------------------------------------------
// Shadow Mapping
//-----------------------------------------------------------------------------
void ShadowEngine::RenderShadowMap(Light* light, Camera* cam)
{
	if(RenderDevice::Instance()->PixelShaderVersion < 2 || light->m_ShadowMapItems.size() == 0 || 
        !RenderDevice::Instance()->GetShadows())  
		return;

    // Don't update shadow map if light is out of range
    float dist = (cam->Location - light->Location).Length();
    if(dist > light->VisibleDistanceRatio*LODManager::Instance()->VisibleRange)
		return;

	light->m_ShadowMap->BeginCapture();
	Matrix oldProj = RenderWrap::GetProjection();
	Matrix oldView = RenderWrap::GetView();
	RenderWrap::SetProjection(light->GetProjection());
	RenderWrap::SetView(light->GetView());//*(Matrix*)&mLightView);//

    Shader* m_CurShader = BatchRenderer::Instance()->m_SystemMaterial->m_Shader;
	m_CurShader->SetTechnique("RenderShadow");
	//m_CurShader->SetHDR(false,false);
	RenderWrap::SetRS( D3DRS_FOGENABLE, FALSE );
	int passes		= m_CurShader->Begin();
	m_CurShader->EndPass(); // In case
	m_CurShader->BeginPass(0);

    // NVIDIA 6800+: Make sure color writes and alpha test is off
    if(RenderDevice::Instance()->PixelShaderVersion > 3)
    {
        // Set new, funky viewport
        D3DVIEWPORT9 newViewport;
        newViewport.X = 0;
        newViewport.Y = 0;
        newViewport.Width  = light->m_ShadowMap->GetSize();
        newViewport.Height = light->m_ShadowMap->GetSize();
        newViewport.MinZ = 0.f;
        newViewport.MaxZ = 1.0f;
        RenderWrap::dev->SetViewport(&newViewport);

        // Depth bias
        float  m_fDepthBias = 0.0004f;
        float  m_fBiasSlope = 5.0f;
        RenderWrap::dev->SetRenderState(D3DRS_DEPTHBIAS, *(DWORD*)&m_fDepthBias);
        RenderWrap::dev->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, *(DWORD*)&m_fBiasSlope);

        RenderWrap::SetRS( D3DRS_COLORWRITEENABLE, 0);
    }
    RenderWrap::SetRS( D3DRS_ALPHATESTENABLE, FALSE);

	
	// Render all batch items
	// Items should be grouped by material, so we'll use redundant state filtering
	// to make sure we only set materials as they change
    for(int c=0;c<light->m_ShadowMapItems.size();c++)
    {
        RenderShadowItem(light->m_ShadowMapItems[c]->m_pFrameRoot,m_CurShader);
	}

    RenderWrap::SetRS(D3DRS_COLORWRITEENABLE,0xFFFFFFFF);

    RenderDevice::Instance()->UpdateViewport();
	m_CurShader->EndPass();
	m_CurShader->End(); // End last m_CurShader
	light->m_ShadowMap->EndCapture();
	RenderWrap::SetProjection(oldProj);
	RenderWrap::SetView(oldView);

    RenderWrap::SetRS(D3DRS_COLORWRITEENABLE,0xFFFFFFFF);
    RenderWrap::SetRS( D3DRS_FOGENABLE, FALSE );

    //depth bias
    float fTemp = 0.0f;
    RenderWrap::dev->SetRenderState(D3DRS_DEPTHBIAS, *(DWORD*)&fTemp);
    RenderWrap::dev->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, *(DWORD*)&fTemp);
}


//-----------------------------------------------------------------------------
// Sorts by distance to camera
//-----------------------------------------------------------------------------
Vector s_CamLocation;
bool sort_by_dist(const Actor* p1, const Actor* p2)
{
    return (p1->Location-s_CamLocation).Length() < (p2->Location-s_CamLocation).Length();
}

//-----------------------------------------------------------------------------
// Clamps light pos from being too steep
//-----------------------------------------------------------------------------
Vector ShadowEngine::GetBiasedLightPos(Actor* a)
{
    Vector		 vAt   = a->Location;
    Vector		 vFrom = a->m_CachedShadow->lightPos;
    Vector delta = (vAt - vFrom);
    float len = delta.Length();
    if(len > 100)
    {
        len = 100;
        delta = delta.Normalized() * 100;
    }

    delta += Vector(0,-len*2,0);
    return a->Location - delta;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
D3DXMATRIX ShadowEngine::GetView(Actor* a)
{
    D3DXMATRIX MatLight;
    D3DXVECTOR3 vUp( 0.0f, 1.0f, 0.0f );
    Vector		 vAt   = a->Location;
    Vector		 vFrom = GetBiasedLightPos(a);

    D3DXMatrixLookAtLH( (D3DXMATRIX*)&MatLight,(D3DXVECTOR3*)&vFrom, (D3DXVECTOR3*)&vAt, &vUp );
    return MatLight;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void ShadowEngine::RenderDropShadows(World* world, Camera* cam)
{
    if(!ShadowsEnabled)
        return;

    RenderDevice::Instance()->ResetAllStates();
    // Gather shadowed actors within radius
    vector<Actor*> actors;
    for(int i=0;i<world->m_Actors.size();i++)
    {
        Actor* a = world->m_Actors[i];
        if(a->m_ShadowType != Actor::None && a->MyModel && !a->IsHidden)
        {
            if((a->Location-cam->Location).Length() < 60)
                actors.push_back(a);
        }  
    }

    s_CamLocation = cam->Location;
    // Sort so nearest shadows have priority in the limited number of shadow slots
    std::sort(actors.begin(),actors.end(),sort_by_dist);

    // Clamp to the nearest number of shadows that we can render
    if(actors.size()>MaxVisibleShadows)
        actors.resize(MaxVisibleShadows);

    // Mark all used RTs this frame
    for(int i=0;i<actors.size();i++)
    {
        if(actors[i]->m_CachedShadow && actors[i]->m_CachedShadow->rtLastUpdated > m_LastReset && actors[i]->m_CachedShadow->rt 
            && actors[i] == actors[i]->m_CachedShadow->rt->lastOwner )
        {
            actors[i]->m_CachedShadow->rt->fLastUsed = GSeconds;
        }
    }

    // We just tagged the used RTs, so now we know the unused ones...
    vector<ShadowTarget*> rts;
    for(int i=0;i<m_pShadowMaps.size();i++)
    {
        if(m_pShadowMaps[i]->fLastUsed < GSeconds)
            rts.push_back(m_pShadowMaps[i]);
    }


    Shader* shader = BatchRenderer::Instance()->m_SystemMaterial->m_Shader;
    D3DXMATRIX matProj = *(D3DXMATRIX*)&(RenderWrap::GetProjection());
	D3DXMATRIX matView = *(D3DXMATRIX*)&(RenderWrap::GetView());
    RenderWrap::dev->SetPixelShader(0);
	RenderWrap::SetWorld(Matrix());
    
    //
    // Render all shadows
    //
    shader->SetTechnique("Black_PS11");
    shader->Begin();
    shader->BeginPass(0);

    LPDIRECT3DSURFACE9 old;
    RenderWrap::dev->GetRenderTarget(0,&old);
    LPDIRECT3DSURFACE9 pOldDS = NULL;
    RenderWrap::dev->GetDepthStencilSurface( &pOldDS );
    RenderWrap::dev->SetDepthStencilSurface( 0 );
    LPDIRECT3DSURFACE9 pShadowSurf=0;

   // Vector Direction(0.2f,-0.8f,0.2f);
  //  Direction.Normalize();
  //  Vector Offset = -Direction*5;

    for(int i=0;i<actors.size();i++)
    {
        Actor* a = actors[i];

        if(!a->m_CachedShadow)
            a->m_CachedShadow = new CachedShadow;

        // This is just for shadow maps
        if(a->m_ShadowType == Actor::Blob)
            continue;

        // Get nearest light
        // TODO: Do this only every second or so
        // NOTE: Never touch pointer on subsequent frames - light may be deleted
		a->m_CachedShadow->srcLight = a->MyModel->FindStrongestLight();

        Vector lightLoc = a->m_CachedShadow->srcLight->Location;
        if(a->m_CachedShadow->srcLight->m_Type == LIGHT_DIR)
            lightLoc = a->Location + Vector(300,400,300);

        if(!a->m_CachedShadow->srcLight)
            continue;
        
        // See if shadow doesn't need re-rendering...
        if(
            // Is RT still bound to this actor?
            // !!STATEMENT ORDER VITAL. If device reset the rt pointer will be garbage!!
            a->m_CachedShadow->rtLastUpdated > m_LastReset && a->m_CachedShadow->rt 
            && a == a->m_CachedShadow->rt->lastOwner 
            // Is animated?
            && a->MyModel->GetNumAnimations()==0
            // Has cached yet?
            && a->m_CachedShadow->m_ShadowPolys.size()
            // Has rotated?
            && a->PrevRotation[1] == a->Rotation[1] && a->PrevRotation[0] == a->Rotation[0] && a->PrevRotation[2] == a->Rotation[2] 
            // Has light pos changed?
            && a->m_CachedShadow->lightPos == lightLoc)
        {
            continue;
        }

        a->m_CachedShadow->lightPos = lightLoc;

        // RT was re-allocated or not allocated, find a new one
        if(a->m_CachedShadow->rtLastUpdated < m_LastReset || !a->m_CachedShadow->rt || a->m_CachedShadow->rt->lastOwner != a)
        {
            a->m_CachedShadow->rt = rts.back();
            a->m_CachedShadow->rt->lastOwner = a;
            rts.resize(rts.size()-1);
        }

        a->m_CachedShadow->rtLastUpdated = GSeconds;

        a->PrevRotation = a->Rotation;

        // Create new projection
        // This is ortho so we retain maximum resolution without getting
        // a shadow with a projection as if the light was right behind the object
        Vector size = a->MyModel->GetWorldBBox().max - a->MyModel->GetWorldBBox().min;
        float maxSize = MAX(size.x,size.y);
        maxSize = MAX(maxSize,size.z)*1.7f;

        float   fDist   = (GetBiasedLightPos(a)-a->Location).Length();

        // Use ortho if light is sufficiently far away
        if(fDist > 80)
        {
            D3DXMatrixOrthoLH( &a->m_CachedShadow->proj, maxSize, maxSize, 0.01f, 400);
        }
        // Otherwise use skewed perspective
        else
        {
            if(fDist < maxSize) fDist = maxSize;
            float   fRadius = maxSize;
            float	fAngle = 2.0f * asinf(fRadius / fDist);
		    float	n = fDist - fRadius * 2.f;
		    float	f = fDist + fRadius * 2.f;
		    if (n < 0.001f) { n = 0.001f; }
		    D3DXMatrixPerspectiveFovLH(&a->m_CachedShadow->proj, fAngle/2, 1.0f, n, f);
        }


        //D3DXMatrixPerspectiveFovLH( &a->m_CachedShadow->proj, D3DX_PI/4, 640.0f/480.0f, 0.01f, 10000 );
       // D3DXMatrixPerspectiveLH( &a->m_CachedShadow->proj, maxSize/14, maxSize/14, 0.01f, 100);
        
        RenderWrap::SetProjection(*(Matrix*)&a->m_CachedShadow->proj);

        // View matrix just looks down on camera from Offset
        D3DXMATRIX MatLight = GetView(a);
        RenderWrap::SetView(*(Matrix*)&MatLight);

        D3DXMATRIX matViewProj;
        D3DXMatrixMultiply( &matViewProj, &MatLight, &a->m_CachedShadow->proj ); 
        shader->GetEffect()->SetMatrix("mViewProjection",&matViewProj);

        DXASSERT(a->m_CachedShadow->rt->map->GetSurfaceLevel( 0, &pShadowSurf ));

        RenderWrap::dev->SetRenderTarget(0,pShadowSurf);
        DXASSERT(RenderWrap::dev->Clear( 0L, NULL, D3DCLEAR_TARGET,0xffffffff, 1.0f, 0L ) );

        // Never fail, always draw
        RenderWrap::SetRS(D3DRS_ZFUNC,D3DCMP_ALWAYS);
        RenderWrap::SetRS(D3DRS_ZWRITEENABLE,FALSE);
        RenderShadowItem(a->MyModel->m_pFrameRoot,shader);
		for(int r = 0; r < a->dropShadowPartners.size();r++)
		{
			if(!a->dropShadowPartners[r]->IsHidden)
				RenderShadowItem(a->dropShadowPartners[r]->MyModel->m_pFrameRoot,shader);
		}
        SAFE_RELEASE(pShadowSurf);
    }


    shader->EndPass();
    shader->End();
    RenderWrap::dev->SetRenderTarget(0,old);
    RenderWrap::dev->SetDepthStencilSurface( pOldDS );
    SAFE_RELEASE(old);
    SAFE_RELEASE(pOldDS);
    RenderDevice::Instance()->UpdateViewport();
    RenderWrap::SetView(*(Matrix*)&matView);
    RenderWrap::SetProjection(*(Matrix*)&matProj);

    // Depth bias to avoid z-fighting
    float  m_fDepthBias = -0.00002f;
    float  m_fBiasSlope = 1.0f;
    RenderWrap::dev->SetRenderState(D3DRS_DEPTHBIAS, *(DWORD*)&m_fDepthBias);
   // RenderWrap::dev->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, *(DWORD*)&m_fBiasSlope);

    RenderWrap::SetRS(D3DRS_ZWRITEENABLE,FALSE);
    RenderWrap::SetRS(D3DRS_ALPHABLENDENABLE,TRUE);
    RenderWrap::SetRS(D3DRS_SRCBLEND,D3DBLEND_ZERO);
    RenderWrap::SetRS(D3DRS_DESTBLEND,D3DBLEND_SRCCOLOR);
    RenderWrap::SetRS(D3DRS_ZFUNC,D3DCMP_LESSEQUAL);

    // Set special texture matrix for shadow mapping
    float fOffsetX = 0.5f + (0.5f / (float)256); // Width
    float fOffsetY = 0.5f + (0.5f / (float)256); // Height
    unsigned int range = 1;
    float fBias    = -0.001f * (float)range;
    D3DXMATRIX texScaleBiasMat( 0.5f,     0.0f,     0.0f,         0.0f,
        0.0f,    -0.5f,     0.0f,         0.0f,
        0.0f,     0.0f,     (float)range, 0.0f,
        fOffsetX, fOffsetY, fBias,        1.0f );
    shader->GetEffect()->SetMatrix("mTexProj",&texScaleBiasMat);

    RenderWrap::SetWorld(Matrix());

	D3DXMATRIX matViewProj=matProj, matViewInv;
	D3DXMatrixMultiply( &matViewProj, &matView, &matProj ); 

    shader->SetTechnique("ShadowProject");
    shader->Begin();
    shader->BeginPass(0);
    RenderWrap::dev->SetFVF(FVF_VERTEX);
    RenderWrap::dev->SetVertexDeclaration(VertexFormats::Instance()->FindFormat(sizeof(Vertex))->decl);

    Matrix mWorld;
    shader->GetEffect()->SetMatrix("mWorld",(D3DXMATRIX*)&mWorld);
    shader->GetEffect()->SetMatrix("mWorldViewProjection",&matViewProj);
    D3DXHANDLE spotHandle  = shader->GetEffect()->GetParameterByName(0,"tSpotlight");
    D3DXHANDLE lightHandle = shader->GetEffect()->GetParameterByName(0,"LightProjection");
    D3DXHANDLE shadowBrightHandle = shader->GetEffect()->GetParameterByName(0,"ShadowBrightness");

    for(int i=0;i<actors.size();i++)
    {
        Actor* a = actors[i];

        if(!a->m_CachedShadow->srcLight)
            continue;

        BBox box = a->MyModel->GetWorldBBox();
        box.max += Vector(1.0f,0,1.0f);
        box.min -= Vector(1.0f,5,1.0f);

        //
        // Gather polygons if actor moves
        //
        if(a->m_CachedShadow->m_ShadowPolys.size() == 0 || (a->PrevLocation-a->Location).Length() > 0.25f)
        {
            a->m_CachedShadow->m_ShadowPolys.resize(0);
            world->GatherRenderingPolys(a,a->m_CachedShadow->m_ShadowPolys,Vector(0,-1,0),box);
            a->PrevLocation = a->Location;
            a->m_CachedShadow->m_LastMovedTime = GSeconds;
        }
        //
        // If it's just stopped moving, do an accurate clipping of the polygons
        // the shadow touches
        // This is really slow, so we only do it once when an object has completely
        // come to rest
        else if(a->m_CachedShadow->m_LastMovedTime > 0 && GSeconds-a->m_CachedShadow->m_LastMovedTime > 2)
        {
            vector<Face> faces;
            faces.resize(a->m_CachedShadow->m_ShadowPolys.size()/3);
            for(int i=0;i<a->m_CachedShadow->m_ShadowPolys.size();i+=3)
            {
                Face& f = faces[i/3];
                f.v[0] = a->m_CachedShadow->m_ShadowPolys[i];
                f.v[1] = a->m_CachedShadow->m_ShadowPolys[i+1];
                f.v[2] = a->m_CachedShadow->m_ShadowPolys[i+2];
            }
            
            vector<Face> newFaces;
            SplitByBox(box,faces,newFaces);

            a->m_CachedShadow->m_ShadowPolys.resize(newFaces.size()*3);
            for(int i=0;i<newFaces.size();i++)
            {
                a->m_CachedShadow->m_ShadowPolys[i*3] = newFaces[i].v[0];
                a->m_CachedShadow->m_ShadowPolys[i*3+1] = newFaces[i].v[1];
                a->m_CachedShadow->m_ShadowPolys[i*3+2] = newFaces[i].v[2];
            }
            a->m_CachedShadow->m_LastMovedTime = -1;
        }

        // View matrix just looks down on camera from Offset
        D3DXMATRIX MatLight = GetView(a);
        D3DXMatrixMultiply(&MatLight,&MatLight,&a->m_CachedShadow->proj);

        // Turn length into brightness falloff
        float len = (cam->Location-a->Location).Length();
        len /= 60;
        len = MAX(0,len-0.5f);
 
        // Set all effects to shader
        shader->GetEffect()->SetMatrix(lightHandle,&MatLight);
        if(a->m_ShadowType == Actor::Drop)
            shader->GetEffect()->SetTexture(spotHandle,a->m_CachedShadow->rt->map);
        else if(a->m_ShadowType == Actor::Blob)
            shader->GetEffect()->SetTexture(spotHandle,m_BlobShadow.GetTexture());
        shader->GetEffect()->SetFloat(shadowBrightHandle,0.7f + len);
        shader->GetEffect()->CommitChanges();

        // Draw!
        if(a->m_CachedShadow->m_ShadowPolys.size())
        {
            if(FAILED(RenderWrap::dev->DrawPrimitiveUP(D3DPT_TRIANGLELIST,
                a->m_CachedShadow->m_ShadowPolys.size()/3, &a->m_CachedShadow->m_ShadowPolys[0], sizeof(Vertex))))
                Error("BatchRenderer::DrawPrimitiveUP() failed. Make sure you aren't trying to draw outside of Begin/EndRender()\nVerts: %d",a->m_CachedShadow->m_ShadowPolys.size());

        }
    }

    shader->EndPass();
    shader->End();

    //depth bias
    float fTemp = 0.0f;
    RenderWrap::dev->SetRenderState(D3DRS_DEPTHBIAS, *(DWORD*)&fTemp);
    RenderWrap::dev->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, *(DWORD*)&fTemp);
}