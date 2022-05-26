//=========== (C) Copyright 2003, Tim Johnson. All rights reserved. ===========
//
// Desc: High dynamic range lighting effects 
//       using floating point textures.
//
// The algorithms described in this sample are based very closely on the 
// lighting effects implemented in Masaki Kawase's Rthdribl sample and the tone
// mapping process described in the whitepaper "Tone Reproduction for Digital 
// Images"
//
// Real-Time High Dynamic Range Image-Based Lighting (Rthdribl)
// Masaki Kawase
// http://www.daionet.gr.jp/~masa/rthdribl/ 
//
// "Photographic Tone Reproduction for Digital Images"
// Erik Reinhard, Mike Stark, Peter Shirley and Jim Ferwerda
// http://www.cs.utah.edu/~reinhard/cdrom/ 
//
//=============================================================================
#include "stdafx.h"
#include "HDR.h"
#include "dxstdafx.h"

D3DFORMAT fpFormat = D3DFMT_A16B16G16R16F;
D3DFORMAT g_LuminanceFormat;

//#define DEBUm_VS   // Uncomment this line to debug vertex shaders 
//#define DEBUm_PS   // Uncomment this line to debug pixel shaders 
IDirect3DDevice9*       m_pd3dDevice = NULL;    // D3D Device object

const DWORD ScreenVertex::FVF = D3DFVF_XYZRHW | D3DFVF_TEX1;


//-----------------------------------------------------------------------------
// The 'old' target shaders can use to do alpha blending
//-----------------------------------------------------------------------------
PDIRECT3DTEXTURE9 HDRSystem::GetColorBuffer(){
	if(m_bEnabled)
		return m_pTexScene[!m_CurTarget];
	else
		return m_pLDRTexture;
}

//-----------------------------------------------------------------------------
// The current target shaders can use
//-----------------------------------------------------------------------------
PDIRECT3DTEXTURE9 HDRSystem::GetCurrentBuffer(){
if(m_bEnabled)
	return m_pTexScene[m_CurTarget];
else
	return m_pLDRTexture;
}

//-----------------------------------------------------------------------------
// Flips targets for ping-pong alpha blending
//-----------------------------------------------------------------------------
HRESULT HDRSystem::FlipTargets(bool copy){
	
	if(m_bEnabled)
	{
	// Flip target
	m_CurTarget = !m_CurTarget;

	PDIRECT3DSURFACE9 prevHDR = pSurfHDR;
	m_pTexScene[m_CurTarget]->GetSurfaceLevel(0, &pSurfHDR);

	// NOTE: Surface copy is ~5% faster than pixel shader (prolly just state overhead)
	if(copy)
		D3DXLoadSurfaceFromSurface(pSurfHDR,0,0,prevHDR,0,0,D3DTEXF_NONE,0);

	// Setup HDR render target
	RenderWrap::dev->SetRenderTarget(0, pSurfHDR);

    RenderDevice::Instance()->UpdateViewport();

	SAFE_RELEASE(prevHDR);
	}
	else
	{
		PDIRECT3DSURFACE9 pSurfLDRBuffer = NULL;
		PDIRECT3DSURFACE9 pSurfLDR = NULL; 

		m_pLDRTexture->GetSurfaceLevel(0, &pSurfLDRBuffer);
	    m_pd3dDevice->GetRenderTarget(0, &pSurfLDR);
	    D3DXLoadSurfaceFromSurface(pSurfLDRBuffer,0,0,pSurfLDR,0,0,D3DTEXF_NONE,0);

		SAFE_RELEASE( pSurfLDRBuffer );
	    SAFE_RELEASE( pSurfLDR );
	}

	return S_OK;
}
void HDRSystem::CopyToLDRTarget()
{
		PDIRECT3DSURFACE9 pSurfLDRBuffer = NULL;
		PDIRECT3DSURFACE9 pSurfLDR = NULL; 

		m_pLDRTexture->GetSurfaceLevel(0, &pSurfLDRBuffer);
	    m_pd3dDevice->GetRenderTarget(0, &pSurfLDR);
	    D3DXLoadSurfaceFromSurface(pSurfLDRBuffer,0,0,pSurfLDR,0,0,D3DTEXF_NONE,0);

		SAFE_RELEASE( pSurfLDRBuffer );
	    SAFE_RELEASE( pSurfLDR );
}
PDIRECT3DTEXTURE9 HDRSystem::GetLDRColorBuffer(){return m_pLDRTexture;}


//-----------------------------------------------------------------------------
// Desc: Returns a singleton instance
//-----------------------------------------------------------------------------
HDRSystem* HDRSystem::Instance () 
{
	static HDRSystem inst;
	return &inst;
}


//-----------------------------------------------------------------------------
// Name: HDRSystem()
// Desc: Application constructor. Sets attributes for the app.
//-----------------------------------------------------------------------------
HDRSystem::HDRSystem()
{
	m_IsLDR					= true;
	m_pEffect				= NULL;
	m_dwCropWidth           = 0;
	m_dwCropHeight          = 0;
	m_bEnabled				= FALSE;
	m_pTexAdaptedLuminanceCur  = NULL;
	m_pTexAdaptedLuminanceLast = NULL;

	m_pTexBrightPass        = NULL;
	m_pTexBloomSource       = NULL;
	m_pTexStarSource        = NULL;
	m_pLDRTexture = NULL;

	m_bAdaptationInvalid    = FALSE;

	ZeroMemory( m_apTexStar, sizeof(m_apTexStar) );
	ZeroMemory( m_apTexToneMap, sizeof(m_apTexToneMap) );
	ZeroMemory( m_apTexBloom, sizeof(m_apTexBloom) );

	ResetOptions();
}




//-----------------------------------------------------------------------------
// Constants and custom types
//-----------------------------------------------------------------------------
#define MAX_SAMPLES           16      // Maximum number of texture grabs

#define NUM_LIGHTS            2       // Number of lights in the scene
                                 
#define NUM_TONEMAP_TEXTURES  4       // Number of stages in the 4x4 down-scaling 
                                      // of average luminance textures
#define NUM_STAR_TEXTURES     12      // Number of textures used for the star
                                      // post-processing effect
#define NUM_BLOOM_TEXTURES    3       // Number of textures used for the bloom
                                      // post-processing effect
          


//-----------------------------------------------------------------------------
// Name: ResetOptions()
// Desc: Reset all user-controlled options to default values
//-----------------------------------------------------------------------------
void HDRSystem::ResetOptions()
{
    m_fKeyValue  = 0.18f;              // Middle gray key value for tone mapping (basically exposure)
	m_bToneMap	 = true;               // True when scene is to be tone mapped            
	m_bBlueShift = false;             // True when blue shift is to be factored in
	m_bEnabled   = false;
    
    m_eGlareType = (EGLARELIBTYPE)Engine::Instance()->MainConfig->GetInt("HDRGlareType");
	m_GlareDef.Initialize( m_eGlareType );
}


//--------------------------------------------------------------------------------------
// Called during device initialization, this code checks the device for some 
// minimum set of capabilities, and rejects those that don't pass by returning false.
//--------------------------------------------------------------------------------------
bool HDRSystem::IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, 
                                  D3DFORMAT BackBufferFormat, bool bWindowed )
{
	if(!m_bEnabled)
		return true;

    // Skip backbuffer formats that don't support alpha blending
    IDirect3D9* pD3D = DXUTGetD3DObject(); 
    if( FAILED( pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal, pCaps->DeviceType,
                    AdapterFormat, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING, 
                    D3DRTYPE_TEXTURE, BackBufferFormat ) ) )
        return false;

    if(pCaps->PixelShaderVersion < D3DPS_VERSION(2,0))
        return false;

    if(pCaps->VertexShaderVersion < D3DVS_VERSION(2,0))
        return false;

    // No fallback yet, so need to support D3DFMT_A16B16G16R16F render target
    if( FAILED( pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal, pCaps->DeviceType,
                    AdapterFormat, D3DUSAGE_RENDERTARGET, 
                    D3DRTYPE_TEXTURE, fpFormat) ) )
    {
        return false;
    }

    // No fallback yet, so need to support D3DFMT_R32F or D3DFMT_R16F render target
    if( FAILED( pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal, pCaps->DeviceType,
                    AdapterFormat, D3DUSAGE_RENDERTARGET, 
                    D3DRTYPE_TEXTURE, D3DFMT_R32F) ) )
    {
        if( FAILED( pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal, pCaps->DeviceType,
                        AdapterFormat, D3DUSAGE_RENDERTARGET, 
                        D3DRTYPE_TEXTURE, D3DFMT_R16F) ) )
            return false;
    }


     // Need to support post-pixel processing
    if( FAILED( pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal, pCaps->DeviceType,
        AdapterFormat, D3DUSAGE_RENDERTARGET | D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING, 
        D3DRTYPE_SURFACE, BackBufferFormat ) ) )
    {
        return false;
    }

    return true;
}


//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has been 
// created, which will happen during application initialization and windowed/full screen 
// toggles. This is the best location to create D3DPOOL_MANAGED resources since these 
// resources need to be reloaded whenever the device is destroyed. Resources created  
// here should be released in the OnDestroyDevice callback. 
//--------------------------------------------------------------------------------------
HRESULT HDRSystem::OnCreateDevice(const D3DSURFACE_DESC* blah){
	if(!m_bEnabled)
		return S_OK;
	HRESULT hr;

	D3DCAPS9 Caps;
	RenderWrap::dev->GetDeviceCaps(&Caps);
	D3DDISPLAYMODE DisplayMode;
    RenderWrap::dev->GetDisplayMode( 0, &DisplayMode );
	// IsDeviceAcceptable already ensured that one of D3DFMT_R16F or D3DFMT_R32F is available.
    if( FAILED( RenderWrap::d3d->CheckDeviceFormat( Caps.AdapterOrdinal, Caps.DeviceType,
                    DisplayMode.Format, D3DUSAGE_RENDERTARGET, 
                    D3DRTYPE_TEXTURE, D3DFMT_R16F ) ) )
        g_LuminanceFormat = D3DFMT_R32F;
    else
        g_LuminanceFormat = D3DFMT_R16F;

	DWORD dwShaderFlags = 0;
#ifdef DEBUm_VS
	dwShaderFlags |= D3DXSHADER_FORCE_VS_SOFTWARE_NOOPT;
#endif
#ifdef DEBUm_PS
	dwShaderFlags |= D3DXSHADER_FORCE_PS_SOFTWARE_NOOPT;
#endif


	// Load the effect file
	string Path = "HDRLighting.fx";
	FindMedia(Path,"Shaders");
	hr = D3DXCreateEffectFromFile(RenderWrap::dev, Path.c_str(), NULL, NULL, dwShaderFlags, NULL, &m_pEffect, NULL);
	if(FAILED(hr))
		return hr;

	return S_OK;
}



//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has been 
// reset, which will happen after a lost device scenario. This is the best location to 
// create D3DPOOL_DEFAULT resources since these resources need to be reloaded whenever 
// the device is lost. Resources created here should be released in the OnLostDevice 
// callback. 
//--------------------------------------------------------------------------------------
HRESULT HDRSystem::OnResetDevice()
{
	  HRESULT hr;
		m_pd3dDevice = RenderWrap::dev;
	    const D3DSURFACE_DESC* pBackBufferDesc = DXUTGetBackBufferSurfaceDesc();

		// A8R8G8B8 color buffer for RT fx in LDR mode
		hr = m_pd3dDevice->CreateTexture(pBackBufferDesc->Width, pBackBufferDesc->Height, 
			1, D3DUSAGE_RENDERTARGET, 
			D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, 
			&m_pLDRTexture, NULL);

	if( FAILED(hr) )
			return hr;

	if(!m_bEnabled)
		return S_OK;

    int i=0; // loop variable

    if( m_pEffect )
        V_RETURN( m_pEffect->OnResetDevice() );

    // Crop the scene texture so width and height are evenly divisible by 8.
    // This cropped version of the scene will be used for post processing effects,
    // and keeping everything evenly divisible allows precise control over
    // sampling points within the shaders.
    m_dwCropWidth = pBackBufferDesc->Width - pBackBufferDesc->Width % 8;
    m_dwCropHeight = pBackBufferDesc->Height - pBackBufferDesc->Height % 8;

    // Create the HDR scene textures
	for(i=0;i<2;i++){
		hr = m_pd3dDevice->CreateTexture(pBackBufferDesc->Width, pBackBufferDesc->Height, 
										1, D3DUSAGE_RENDERTARGET, fpFormat, 
										D3DPOOL_DEFAULT, &m_pTexScene[i], NULL);
		if( FAILED(hr) )
			return hr;
	}

    // Scaled version of the HDR scene texture
    hr = m_pd3dDevice->CreateTexture(m_dwCropWidth / 4, m_dwCropHeight / 4, 
                                     1, D3DUSAGE_RENDERTARGET, 
                                     fpFormat, D3DPOOL_DEFAULT, 
                                     &m_pTexSceneScaled, NULL);
    if( FAILED(hr) )
        return hr;
    

    // Create the bright-pass filter texture. 
    // Texture has a black border of single texel thickness to fake border 
    // addressing using clamp addressing
    hr = m_pd3dDevice->CreateTexture(m_dwCropWidth / 4 + 2, m_dwCropHeight / 4 + 2, 
                                     1, D3DUSAGE_RENDERTARGET, 
                                     D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, 
                                     &m_pTexBrightPass, NULL);
    if( FAILED(hr) )
        return hr;

    
    
    // Create a texture to be used as the source for the star effect
    // Texture has a black border of single texel thickness to fake border 
    // addressing using clamp addressing
    hr = m_pd3dDevice->CreateTexture(m_dwCropWidth / 4 + 2, m_dwCropHeight / 4 + 2, 
                                     1, D3DUSAGE_RENDERTARGET, 
                                     D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, 
                                     &m_pTexStarSource, NULL);
    if( FAILED(hr) )
        return hr;

    
    
    // Create a texture to be used as the source for the bloom effect
    // Texture has a black border of single texel thickness to fake border 
    // addressing using clamp addressing
    hr = m_pd3dDevice->CreateTexture(m_dwCropWidth / 8 + 2, m_dwCropHeight / 8 + 2, 
                                     1, D3DUSAGE_RENDERTARGET, 
                                     D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, 
                                     &m_pTexBloomSource, NULL);
    if( FAILED(hr) )
        return hr;

    // Create a 2 textures to hold the luminance that the user is currently adapted
    // to. This allows for a simple simulation of light adaptation.
    hr = m_pd3dDevice->CreateTexture(1, 1, 1, D3DUSAGE_RENDERTARGET,
                                     g_LuminanceFormat, D3DPOOL_DEFAULT,
                                     &m_pTexAdaptedLuminanceCur, NULL);
    if( FAILED(hr) )
        return hr;
    hr = m_pd3dDevice->CreateTexture(1, 1, 1, D3DUSAGE_RENDERTARGET,
                                     g_LuminanceFormat, D3DPOOL_DEFAULT,
                                     &m_pTexAdaptedLuminanceLast, NULL);
    if( FAILED(hr) )
        return hr;


    // For each scale stage, create a texture to hold the intermediate results
    // of the luminance calculation
    for(i=0; i < NUM_TONEMAP_TEXTURES; i++)
    {
        int iSampleLen = 1 << (2*i);

        hr = m_pd3dDevice->CreateTexture(iSampleLen, iSampleLen, 1, D3DUSAGE_RENDERTARGET, 
                                         g_LuminanceFormat, D3DPOOL_DEFAULT, 
                                         &m_apTexToneMap[i], NULL);
        if( FAILED(hr) )
            return hr;
    }


    // Create the temporary blooming effect textures
    // Texture has a black border of single texel thickness to fake border 
    // addressing using clamp addressing
    for( i=1; i < NUM_BLOOM_TEXTURES; i++ )
    {
        hr = m_pd3dDevice->CreateTexture(m_dwCropWidth/8 + 2, m_dwCropHeight/8 + 2, 
                                        1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, 
                                        D3DPOOL_DEFAULT, &m_apTexBloom[i], NULL);
        if( FAILED(hr) )
            return hr;
    }

    // Create the final blooming effect texture
    hr = m_pd3dDevice->CreateTexture( m_dwCropWidth/8, m_dwCropHeight/8,
                                      1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, 
                                      D3DPOOL_DEFAULT, &m_apTexBloom[0], NULL);
    if( FAILED(hr) )
        return hr;
                              

    // Create the star effect textures
    for( i=0; i < NUM_STAR_TEXTURES; i++ )
    {
        hr = m_pd3dDevice->CreateTexture( m_dwCropWidth /4, m_dwCropHeight / 4,
                                          1, D3DUSAGE_RENDERTARGET,
                                          fpFormat, D3DPOOL_DEFAULT,
                                          &m_apTexStar[i], NULL );

        if( FAILED(hr) )
            return hr;
    }

    // Textures with borders must be cleared since scissor rect testing will
    // be used to avoid rendering on top of the border
    ClearTexture( m_pTexAdaptedLuminanceCur );
    ClearTexture( m_pTexAdaptedLuminanceLast );
    ClearTexture( m_pTexBloomSource );
    ClearTexture( m_pTexBrightPass );
    ClearTexture( m_pTexStarSource );

    for( i=0; i < NUM_BLOOM_TEXTURES; i++ )
    {
        ClearTexture( m_apTexBloom[i] );
    }

    // Set effect file variables
	D3DXMATRIX mProjection = *(D3DXMATRIX*)&RenderWrap::GetProjection();
    m_pEffect->SetMatrix("g_mProjection", &mProjection);
    m_pEffect->SetFloat( "g_fBloomScale", 1.0f );
    m_pEffect->SetFloat( "g_fStarScale", 0.5f );

    // Setup the camera's projection parameters
    float fAspectRatio = ((FLOAT)m_dwCropWidth) / m_dwCropHeight;
	RenderDevice::Instance()->m_fAspect = fAspectRatio;
    
	return hr;
}


//-----------------------------------------------------------------------------
// Name: ClearTexture()
// Desc: Helper function for RestoreDeviceObjects to clear a texture surface
//-----------------------------------------------------------------------------
HRESULT HDRSystem::ClearTexture( LPDIRECT3DTEXTURE9 pTexture )
{
    HRESULT hr = S_OK;
    PDIRECT3DSURFACE9 pSurface = NULL;

    hr = pTexture->GetSurfaceLevel( 0, &pSurface );
    if( SUCCEEDED(hr) )
        m_pd3dDevice->ColorFill( pSurface, NULL, D3DCOLOR_ARGB(0, 0, 0, 0) );

    SAFE_RELEASE( pSurface );
    return hr;
}

//--------------------------------------------------------------------------------------
// This callback function will be called once at the beginning of every frame. This is the
// best location for your application to handle updates to the scene, but is not 
// intended to contain actual rendering calls, which should instead be placed in the 
// OnFrameRender callback.  
//--------------------------------------------------------------------------------------
void HDRSystem::OnFrameMove()
{
	if(!m_bEnabled)
		return;

    // Set the flag to refresh the user's simulated adaption level.
    // Frame move is not called when the scene is paused or single-stepped. 
    // If the scene is paused, the user's adaptation level needs to remain
    // unchanged.
    m_bAdaptationInvalid = true;

    // Set frame shader constants
    m_pEffect->SetBool("g_bEnableToneMap", m_bToneMap);
    m_pEffect->SetBool("g_bEnableBlueShift", m_bBlueShift);
}


//-----------------------------------------------------------------------------
// Clears surface handles each frame (doesn't delete them, just decreases refs)
//-----------------------------------------------------------------------------
HRESULT HDRSystem::PostRenderClean(){
	if(!m_bEnabled)
		return S_OK;

	// Release surfaces
	SAFE_RELEASE(pSurfHDR);
	SAFE_RELEASE(pSurfLDR);
	return S_OK;
}


void HDRSystem::SetLDR(bool ldr)
{
    if(!m_bEnabled)
		return;

    if(ldr)
        RenderWrap::dev->SetRenderTarget(0,pSurfLDR);
    else
        RenderWrap::dev->SetRenderTarget(0, pSurfHDR);
}

//-----------------------------------------------------------------------------
// Sets the initial FP target and stores the old LDR target
//-----------------------------------------------------------------------------
HRESULT HDRSystem::PreRender()
{
	if(!m_bEnabled)
		return S_OK;

	m_IsLDR = false;
	HRESULT hr = S_OK;

	// Keep old LDR target for later
	RenderWrap::dev->GetRenderTarget(0, &pSurfLDR);
	m_CurTarget = 1; // so it flips on to #0
	FlipTargets(false); 
    

	return S_OK;
}

//--------------------------------------------------------------------------------------
// This callback function will be called at the end of every frame to perform all the 
// rendering calls for the scene, and it will also be called if the window needs to be 
// repainted. After this function has returned, the sample framework will call 
// IDirect3DDevice9::Present to display the contents of the next buffer in the swap chain
//--------------------------------------------------------------------------------------
void HDRSystem::PostRender()
{
	if(!m_bEnabled)
		return;

	HRESULT hr;
   
	RenderWrap::SetRS( D3DRS_ALPHABLENDENABLE, FALSE );
	RenderWrap::SetRS( D3DRS_FOGENABLE, FALSE );
	//DWORD srgb = RenderWrap::SetRS( D3DRS_SRGBWRITEENABLE, FALSE );

	// HDR needs SRGB off
	for(int i=0;i<4;i++)
		RenderWrap::dev->SetSamplerState( i, D3DSAMP_SRGBTEXTURE, FALSE);

	// Create a scaled copy of the scene
	Scene_To_SceneScaled();

	// Setup tone mapping technique
	if( m_bToneMap )
		MeasureLuminance();

	// If FrameMove has been called, the user's adaptation level has also changed
	// and should be updated
	// TIM: Added m_bToneMap to stop adaption when tone mapping is disabled
	if( m_bToneMap && m_bAdaptationInvalid )
	{
		// Clear the update flag
		m_bAdaptationInvalid = false;

		// Calculate the current luminance adaptation level
		CalculateAdaptation();
	}

	// Now that luminance information has been gathered, the scene can be bright-pass filtered
	// to remove everything except bright lights and reflections.
	SceneScaled_To_BrightPass();

	// Blur the bright-pass filtered image to create the source texture for the star effect
	BrightPass_To_StarSource();

	// Scale-down the source texture for the star effect to create the source texture
	// for the bloom effect
	StarSource_To_BloomSource();

	// Render post-process lighting effects
	RenderBloom();
	RenderStar();

	// Draw the high dynamic range scene texture to the low dynamic range
	// back buffer. As part of this final pass, the scene will be tone-mapped
	// using the user's current adapted luminance, blue shift will occur
	// if the scene is determined to be very dark, and the post-process lighting
	// effect textures will be added to the scene.
	UINT uiPassCount, uiPass;

	// TIM: Hacky way to ensure we run optimized shaders with no runtime branching
	if(m_bToneMap && m_bBlueShift)
		V( m_pEffect->SetTechnique("FinalScenePass_ToneShift") );
	if(!m_bToneMap && !m_bBlueShift)
		V( m_pEffect->SetTechnique("FinalScenePass") );
	if(m_bToneMap && !m_bBlueShift)
		V( m_pEffect->SetTechnique("FinalScenePass_Tone") );
	if(!m_bToneMap && m_bBlueShift)
		V( m_pEffect->SetTechnique("FinalScenePass_Shift") );

	V( m_pEffect->SetFloat("ADAPTION_RATE", 10.0f));
	V( m_pEffect->SetFloat("g_fMiddleGray", m_fKeyValue) );
	V( m_pEffect->SetFloat("g_fMinLum", m_fMinLum) );
	V( m_pEffect->SetFloat("g_fMaxLum", m_fMaxLum) );
	V( m_pEffect->SetFloat("g_fBlueShift", m_fBlueShift) );

	V( m_pd3dDevice->SetRenderTarget(0, pSurfLDR) );
	V( m_pd3dDevice->SetTexture( 0, m_pTexScene[m_CurTarget] ) );
	V( m_pd3dDevice->SetTexture( 1, m_apTexBloom[0] ) );
	V( m_pd3dDevice->SetTexture( 2, m_apTexStar[0] ) );
	V( m_pd3dDevice->SetTexture( 3, m_pTexAdaptedLuminanceCur ) );
	V( m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT ) );
	V( m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT ) );
	V( m_pd3dDevice->SetSamplerState( 1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR) );
	V( m_pd3dDevice->SetSamplerState( 1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR ) );
	V( m_pd3dDevice->SetSamplerState( 2, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR) );
	V( m_pd3dDevice->SetSamplerState( 2, D3DSAMP_MINFILTER, D3DTEXF_LINEAR ) );
	V( m_pd3dDevice->SetSamplerState( 3, D3DSAMP_MAGFILTER, D3DTEXF_POINT ) );
	V( m_pd3dDevice->SetSamplerState( 3, D3DSAMP_MINFILTER, D3DTEXF_POINT ) );

     

	V( m_pEffect->Begin(&uiPassCount, 0) );

	for (uiPass = 0; uiPass < uiPassCount; uiPass++)
	{
		V( m_pEffect->BeginPass(uiPass) );

		DrawFullScreenQuad( 0.0f, 0.0f, 1.0f, 1.0f );

		V( m_pEffect->EndPass() );
	}

	V( m_pEffect->End() );

	RenderWrap::SetRS( D3DRS_FOGENABLE, TRUE );
	RenderWrap::SetRS( D3DRS_ALPHABLENDENABLE, TRUE );

	m_IsLDR = true;
}




//-----------------------------------------------------------------------------
// Name: MeasureLuminance()
// Desc: Measure the average log luminance in the scene.
//-----------------------------------------------------------------------------
HRESULT HDRSystem::MeasureLuminance()
{
    HRESULT hr = S_OK;
    UINT uiPassCount, uiPass;
    int i, x, y, index;
    D3DXVECTOR2 avSampleOffsets[MAX_SAMPLES];

    
    DWORD dwCurTexture = NUM_TONEMAP_TEXTURES-1;

    // Sample log average luminance
    PDIRECT3DSURFACE9 apSurfToneMap[NUM_TONEMAP_TEXTURES] = {0};

    // Retrieve the tonemap surfaces
    for( i=0; i < NUM_TONEMAP_TEXTURES; i++ )
    {
        hr = m_apTexToneMap[i]->GetSurfaceLevel( 0, &apSurfToneMap[i] );
        if( FAILED(hr) )
            goto LCleanReturn;
    }

    D3DSURFACE_DESC desc;
    m_apTexToneMap[dwCurTexture]->GetLevelDesc( 0, &desc );

    
    // Initialize the sample offsets for the initial luminance pass.
    float tU, tV;
    tU = 1.0f / (3.0f * desc.Width);
    tV = 1.0f / (3.0f * desc.Height);
    
    index=0;
    for( x = -1; x <= 1; x++ )
    {
        for( y = -1; y <= 1; y++ )
        {
            avSampleOffsets[index].x = x * tU;
            avSampleOffsets[index].y = y * tV;

            index++;
        }
    }

    
    // After this pass, the m_apTexToneMap[NUM_TONEMAP_TEXTURES-1] texture will contain
    // a scaled, grayscale copy of the HDR scene. Individual texels contain the log 
    // of average luminance values for points sampled on the HDR texture.
    m_pEffect->SetTechnique("SampleAvgLum");
    m_pEffect->SetValue("g_avSampleOffsets", avSampleOffsets, sizeof(avSampleOffsets));
    
    m_pd3dDevice->SetRenderTarget(0, apSurfToneMap[dwCurTexture]);
    m_pd3dDevice->SetTexture(0, m_pTexSceneScaled);
    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetSamplerState( 1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetSamplerState( 1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
    
    
    hr = m_pEffect->Begin(&uiPassCount, 0);
    if( FAILED(hr) )
        goto LCleanReturn;

    for (uiPass = 0; uiPass < uiPassCount; uiPass++)
    {
        m_pEffect->BeginPass(uiPass);

        // Draw a fullscreen quad to sample the RT
        DrawFullScreenQuad(0.0f, 0.0f, 1.0f, 1.0f);

        m_pEffect->EndPass();
    }

    m_pEffect->End();
    dwCurTexture--;
    
    // Initialize the sample offsets for the iterative luminance passes
    while( dwCurTexture > 0 )
    {
        m_apTexToneMap[dwCurTexture+1]->GetLevelDesc( 0, &desc );
        GetSampleOffsets_DownScale4x4( desc.Width, desc.Height, avSampleOffsets );
    

        // Each of these passes continue to scale down the log of average
        // luminance texture created above, storing intermediate results in 
        // m_apTexToneMap[1] through m_apTexToneMap[NUM_TONEMAP_TEXTURES-1].
        m_pEffect->SetTechnique("ResampleAvgLum");
        m_pEffect->SetValue("g_avSampleOffsets", avSampleOffsets, sizeof(avSampleOffsets));

        m_pd3dDevice->SetRenderTarget(0, apSurfToneMap[dwCurTexture]);
        m_pd3dDevice->SetTexture(0, m_apTexToneMap[dwCurTexture+1]);
        m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
        m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
    
        
        hr = m_pEffect->Begin(&uiPassCount, 0);
        if( FAILED(hr) )
            goto LCleanReturn;
        
        for (uiPass = 0; uiPass < uiPassCount; uiPass++)
        {
            m_pEffect->BeginPass(uiPass);

            // Draw a fullscreen quad to sample the RT
            DrawFullScreenQuad(0.0f, 0.0f, 1.0f, 1.0f);

            m_pEffect->EndPass();
        }

        m_pEffect->End();
        dwCurTexture--;
    }

    // Downsample to 1x1
    m_apTexToneMap[1]->GetLevelDesc( 0, &desc );
    GetSampleOffsets_DownScale4x4( desc.Width, desc.Height, avSampleOffsets );
    
 
    // Perform the final pass of the average luminance calculation. This pass
    // scales the 4x4 log of average luminance texture from above and performs
    // an exp() operation to return a single texel cooresponding to the average
    // luminance of the scene in m_apTexToneMap[0].
    m_pEffect->SetTechnique("ResampleAvgLumExp");
    m_pEffect->SetValue("g_avSampleOffsets", avSampleOffsets, sizeof(avSampleOffsets));
    
    m_pd3dDevice->SetRenderTarget(0, apSurfToneMap[0]);
    m_pd3dDevice->SetTexture(0, m_apTexToneMap[1]);
    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
    
     
    hr = m_pEffect->Begin(&uiPassCount, 0);
    if( FAILED(hr) )
        goto LCleanReturn;

    for (uiPass = 0; uiPass < uiPassCount; uiPass++)
    {
        m_pEffect->BeginPass(uiPass);

        // Draw a fullscreen quad to sample the RT
        DrawFullScreenQuad(0.0f, 0.0f, 1.0f, 1.0f);

        m_pEffect->EndPass();
    }

    m_pEffect->End();
 

    hr = S_OK;
LCleanReturn:
    for( i=0; i < NUM_TONEMAP_TEXTURES; i++ )
    {
        SAFE_RELEASE( apSurfToneMap[i] );
    }

    return hr;
}




//-----------------------------------------------------------------------------
// Name: CalculateAdaptation()
// Desc: Increment the user's adapted luminance
//-----------------------------------------------------------------------------
HRESULT HDRSystem::CalculateAdaptation()
{
    HRESULT hr = S_OK;
    UINT uiPass, uiPassCount;

    // Swap current & last luminance
    PDIRECT3DTEXTURE9 pTexSwap = m_pTexAdaptedLuminanceLast;
    m_pTexAdaptedLuminanceLast = m_pTexAdaptedLuminanceCur;
    m_pTexAdaptedLuminanceCur = pTexSwap;
    
    PDIRECT3DSURFACE9 pSurfAdaptedLum = NULL;
    hr = m_pTexAdaptedLuminanceCur->GetSurfaceLevel(0, &pSurfAdaptedLum);
    if( FAILED(hr) )
        return hr;

    // This simulates the light adaptation that occurs when moving from a 
    // dark area to a bright area, or vice versa. The m_pTexAdaptedLuminance
    // texture stores a single texel cooresponding to the user's adapted 
    // level.
    m_pEffect->SetTechnique("CalculateAdaptedLum");
    m_pEffect->SetFloat("g_fElapsedTime", DXUTGetElapsedTime());
    
    m_pd3dDevice->SetRenderTarget(0, pSurfAdaptedLum);
    m_pd3dDevice->SetTexture(0, m_pTexAdaptedLuminanceLast);
    m_pd3dDevice->SetTexture(1, m_apTexToneMap[0]);
    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
    m_pd3dDevice->SetSamplerState( 1, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
    m_pd3dDevice->SetSamplerState( 1, D3DSAMP_MINFILTER, D3DTEXF_POINT );

    
    hr = m_pEffect->Begin(&uiPassCount, 0);
    if( FAILED(hr) )
        return hr;
    
    for (uiPass = 0; uiPass < uiPassCount; uiPass++)
    {
        m_pEffect->BeginPass(uiPass);

        // Draw a fullscreen quad to sample the RT
        DrawFullScreenQuad(0.0f, 0.0f, 1.0f, 1.0f);

        m_pEffect->EndPass();
    }
    
    m_pEffect->End();


    SAFE_RELEASE( pSurfAdaptedLum );
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: RenderStar()
// Desc: Render the blooming effect
//-----------------------------------------------------------------------------
HRESULT HDRSystem::RenderStar()
{
    HRESULT hr = S_OK;
    UINT uiPassCount, uiPass;
    int i, d, p, s; // Loop variables

    LPDIRECT3DSURFACE9 pSurfStar = NULL;
    hr = m_apTexStar[0]->GetSurfaceLevel( 0, &pSurfStar );
    if( FAILED(hr) )
        return hr;

    // Clear the star texture
    m_pd3dDevice->ColorFill( pSurfStar, NULL, D3DCOLOR_ARGB(0, 0, 0, 0) );
    SAFE_RELEASE( pSurfStar );

    // Avoid rendering the star if it's not being used in the current glare
    if( m_GlareDef.m_fGlareLuminance <= 0.0f ||
        m_GlareDef.m_fStarLuminance <= 0.0f )
    {
        return S_OK ;
    }

    // Initialize the constants used during the effect
    const CStarDef& starDef = m_GlareDef.m_starDef ;
    const float fTanFoV = atanf(D3DX_PI/8) ;
    const D3DXVECTOR4 vWhite( 1.0f, 1.0f, 1.0f, 1.0f );
    static const s_maxPasses = 3 ;
    static const int nSamples = 8 ;
    static D3DXVECTOR4 s_aaColor[s_maxPasses][8] ;
    static const D3DXCOLOR s_colorWhite(0.63f, 0.63f, 0.63f, 0.0f) ;
    
    D3DXVECTOR4 avSampleWeights[MAX_SAMPLES];
    D3DXVECTOR2 avSampleOffsets[MAX_SAMPLES];
        
    m_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE) ;
    m_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE) ;

    PDIRECT3DSURFACE9 pSurfSource = NULL;
    PDIRECT3DSURFACE9 pSurfDest = NULL;

    // Set aside all the star texture surfaces as a convenience
    PDIRECT3DSURFACE9 apSurfStar[NUM_STAR_TEXTURES] = {0};
    for( i=0; i < NUM_STAR_TEXTURES; i++ )
    {
        hr = m_apTexStar[i]->GetSurfaceLevel( 0, &apSurfStar[i] );
        if( FAILED(hr) )
            goto LCleanReturn;
    }

    // Get the source texture dimensions
    hr = m_pTexStarSource->GetSurfaceLevel( 0, &pSurfSource );
    if( FAILED(hr) )
        goto LCleanReturn;

    D3DSURFACE_DESC desc;
    hr = pSurfSource->GetDesc( &desc );
    if( FAILED(hr) )
        goto LCleanReturn;

    SAFE_RELEASE( pSurfSource );

    float srcW;
    srcW = (FLOAT) desc.Width;
    float srcH;
    srcH= (FLOAT) desc.Height;

    
   
    for (p = 0 ; p < s_maxPasses ; p ++)
    {
        float ratio;
        ratio = (float)(p + 1) / (float)s_maxPasses ;
        
        for (s = 0 ; s < nSamples ; s ++)
        {
            D3DXCOLOR chromaticAberrColor ;
            D3DXColorLerp(&chromaticAberrColor,
                &( CStarDef::GetChromaticAberrationColor(s) ),
                &s_colorWhite,
                ratio) ;

            D3DXColorLerp( (D3DXCOLOR*)&( s_aaColor[p][s] ),
                &s_colorWhite, &chromaticAberrColor,
                m_GlareDef.m_fChromaticAberration ) ;
        }
    }

    float radOffset;
    radOffset = m_GlareDef.m_fStarInclination + starDef.m_fInclination ;
    

    PDIRECT3DTEXTURE9 pTexSource;


    // Direction loop
    for (d = 0 ; d < starDef.m_nStarLines ; d ++)
    {
        CONST STARLINE& starLine = starDef.m_pStarLine[d] ;

        pTexSource = m_pTexStarSource;
        
        float rad;
        rad = radOffset + starLine.fInclination ;
        float sn, cs;
        sn = sinf(rad), cs = cosf(rad) ;
        D3DXVECTOR2 vtStepUV;
        vtStepUV.x = sn / srcW * starLine.fSampleLength ;
        vtStepUV.y = cs / srcH * starLine.fSampleLength ;
        
        float attnPowScale;
        attnPowScale = (fTanFoV + 0.1f) * 1.0f *
                       (160.0f + 120.0f) / (srcW + srcH) * 1.2f ;

        // 1 direction expansion loop
        m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE) ;
        
        int iWorkTexture;
        iWorkTexture = 1 ;
        for (p = 0 ; p < starLine.nPasses ; p ++)
        {
            
            if (p == starLine.nPasses - 1)
            {
                // Last pass move to other work buffer
                pSurfDest = apSurfStar[d+4];
            }
            else {
                pSurfDest = apSurfStar[iWorkTexture];
            }

            // Sampling configration for each stage
            for (i = 0 ; i < nSamples ; i ++)
            {
                float lum;
                lum = powf( starLine.fAttenuation, attnPowScale * i );
                
                avSampleWeights[i] = s_aaColor[starLine.nPasses - 1 - p][i] *
                                lum * (p+1.0f) * 0.5f ;
                                
                
                // Offset of sampling coordinate
                avSampleOffsets[i].x = vtStepUV.x * i ;
                avSampleOffsets[i].y = vtStepUV.y * i ;
                if ( fabs(avSampleOffsets[i].x) >= 0.9f ||
                     fabs(avSampleOffsets[i].y) >= 0.9f )
                {
                    avSampleOffsets[i].x = 0.0f ;
                    avSampleOffsets[i].y = 0.0f ;
                    avSampleWeights[i] *= 0.0f ;
                }
                
            }

            
            m_pEffect->SetTechnique("Star");
            m_pEffect->SetValue("g_avSampleOffsets", avSampleOffsets, sizeof(avSampleOffsets));
            m_pEffect->SetVectorArray("g_avSampleWeights", avSampleWeights, nSamples);
            
            m_pd3dDevice->SetRenderTarget( 0, pSurfDest );
            m_pd3dDevice->SetTexture( 0, pTexSource );
            m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
            m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
    
            
            hr = m_pEffect->Begin(&uiPassCount, 0);
            if( FAILED(hr) )
                return hr;
            
            for (uiPass = 0; uiPass < uiPassCount; uiPass++)
            {
                m_pEffect->BeginPass(uiPass);

                // Draw a fullscreen quad to sample the RT
                DrawFullScreenQuad(0.0f, 0.0f, 1.0f, 1.0f);

                m_pEffect->EndPass();
            }
            
            m_pEffect->End();

            // Setup next expansion
            vtStepUV *= nSamples ;
            attnPowScale *= nSamples ;

            // Set the work drawn just before to next texture source.
            pTexSource = m_apTexStar[iWorkTexture];

            iWorkTexture += 1 ;
            if (iWorkTexture > 2) {
                iWorkTexture = 1 ;
            }

        }
    }


    pSurfDest = apSurfStar[0];

    
    for( i=0; i < starDef.m_nStarLines; i++ )
    {
        m_pd3dDevice->SetTexture( i, m_apTexStar[i+4] );
        m_pd3dDevice->SetSamplerState( i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
        m_pd3dDevice->SetSamplerState( i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );

        avSampleWeights[i] = vWhite * 1.0f / (FLOAT) starDef.m_nStarLines;
    }

    CHAR strTechnique[256];
    _snprintf( strTechnique, 256, "MergeTextures_%d", starDef.m_nStarLines );
    strTechnique[255] = 0;

    m_pEffect->SetTechnique(strTechnique);

    m_pEffect->SetVectorArray("g_avSampleWeights", avSampleWeights, starDef.m_nStarLines);
    
    m_pd3dDevice->SetRenderTarget( 0, pSurfDest );
    
    hr = m_pEffect->Begin(&uiPassCount, 0);
    if( FAILED(hr) )
        goto LCleanReturn;
    
    for (uiPass = 0; uiPass < uiPassCount; uiPass++)
    {
        m_pEffect->BeginPass(uiPass);

        // Draw a fullscreen quad to sample the RT
        DrawFullScreenQuad(0.0f, 0.0f, 1.0f, 1.0f);

        m_pEffect->EndPass();
    }
    
    m_pEffect->End();

    
    hr = S_OK;
LCleanReturn:
    for( i=0; i < NUM_STAR_TEXTURES; i++ )
    {
        SAFE_RELEASE( apSurfStar[i] );
    }

    return hr;
}




//-----------------------------------------------------------------------------
// Name: RenderBloom()
// Desc: Render the blooming effect
//-----------------------------------------------------------------------------
HRESULT HDRSystem::RenderBloom()
{
    HRESULT hr = S_OK;
    UINT uiPassCount, uiPass;
    int i=0;


    D3DXVECTOR2 avSampleOffsets[MAX_SAMPLES];
    FLOAT       afSampleOffsets[MAX_SAMPLES];
    D3DXVECTOR4 avSampleWeights[MAX_SAMPLES];

    PDIRECT3DSURFACE9 pSurfScaledHDR;
    m_pTexSceneScaled->GetSurfaceLevel(0, &pSurfScaledHDR);

    PDIRECT3DSURFACE9 pSurfBloom;
    m_apTexBloom[0]->GetSurfaceLevel(0, &pSurfBloom);

    PDIRECT3DSURFACE9 pSurfHDR;
    m_pTexScene[m_CurTarget]->GetSurfaceLevel(0, &pSurfHDR);  
    
    PDIRECT3DSURFACE9 pSurfTempBloom;
    m_apTexBloom[1]->GetSurfaceLevel(0, &pSurfTempBloom);

    PDIRECT3DSURFACE9 pSurfBloomSource;
    m_apTexBloom[2]->GetSurfaceLevel(0, &pSurfBloomSource);

    // Clear the bloom texture
    m_pd3dDevice->ColorFill( pSurfBloom, NULL, D3DCOLOR_ARGB(0, 0, 0, 0) );

    if (m_GlareDef.m_fGlareLuminance <= 0.0f ||
        m_GlareDef.m_fBloomLuminance <= 0.0f)
    {
        hr = S_OK;
        goto LCleanReturn;
    }

    if( m_GlareDef.m_fGlareLuminance <= 0.0f ||
        m_GlareDef.m_fBloomLuminance <= 0.0f )
    {
        return S_OK ;
    }

    RECT rectSrc;
    GetTextureRect( m_pTexBloomSource, &rectSrc );
    InflateRect( &rectSrc, -1, -1 );

    RECT rectDest;
    GetTextureRect( m_apTexBloom[2], &rectDest );
    InflateRect( &rectDest, -1, -1 );

    CoordRect coords;
    GetTextureCoords( m_pTexBloomSource, &rectSrc, m_apTexBloom[2], &rectDest, &coords );
   
    D3DSURFACE_DESC desc;
    hr = m_pTexBloomSource->GetLevelDesc( 0, &desc );
    if( FAILED(hr) )
        return hr;


    m_pEffect->SetTechnique("GaussBlur5x5");

    hr = GetSampleOffsets_GaussBlur5x5( desc.Width, desc.Height, avSampleOffsets, avSampleWeights, 1.0f );

    m_pEffect->SetValue("g_avSampleOffsets", avSampleOffsets, sizeof(avSampleOffsets));
    m_pEffect->SetValue("g_avSampleWeights", avSampleWeights, sizeof(avSampleWeights));
   
    m_pd3dDevice->SetRenderTarget(0, pSurfBloomSource );
    m_pd3dDevice->SetTexture( 0, m_pTexBloomSource );
    m_pd3dDevice->SetScissorRect( &rectDest );
    m_pd3dDevice->SetRenderState( D3DRS_SCISSORTESTENABLE, TRUE );
    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
       
    
    hr = m_pEffect->Begin(&uiPassCount, 0);
    if( FAILED(hr) )
        goto LCleanReturn;
    
    for (uiPass = 0; uiPass < uiPassCount; uiPass++)
    {
        m_pEffect->BeginPass(uiPass);

        // Draw a fullscreen quad to sample the RT
        DrawFullScreenQuad( coords );

        m_pEffect->EndPass();
    }
    m_pEffect->End();
    m_pd3dDevice->SetRenderState( D3DRS_SCISSORTESTENABLE, FALSE );

    hr = m_apTexBloom[2]->GetLevelDesc( 0, &desc );
    if( FAILED(hr) )
        return hr;

    hr = GetSampleOffsets_Bloom( desc.Width, afSampleOffsets, avSampleWeights, 3.0f, 2.0f );
    for( i=0; i < MAX_SAMPLES; i++ )
    {
        avSampleOffsets[i] = D3DXVECTOR2( afSampleOffsets[i], 0.0f );
    }
     

    m_pEffect->SetTechnique("Bloom");
    m_pEffect->SetValue("g_avSampleOffsets", avSampleOffsets, sizeof(avSampleOffsets));
    m_pEffect->SetValue("g_avSampleWeights", avSampleWeights, sizeof(avSampleWeights));
   
    m_pd3dDevice->SetRenderTarget(0, pSurfTempBloom);
    m_pd3dDevice->SetTexture( 0, m_apTexBloom[2] );
    m_pd3dDevice->SetScissorRect( &rectDest );
    m_pd3dDevice->SetRenderState( D3DRS_SCISSORTESTENABLE, TRUE );
    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
       
    
    m_pEffect->Begin(&uiPassCount, 0);
    for (uiPass = 0; uiPass < uiPassCount; uiPass++)
    {
        m_pEffect->BeginPass(uiPass);

        // Draw a fullscreen quad to sample the RT
        DrawFullScreenQuad( coords );

        m_pEffect->EndPass();
    }
    m_pEffect->End();
    m_pd3dDevice->SetRenderState( D3DRS_SCISSORTESTENABLE, FALSE );
    

    hr = m_apTexBloom[1]->GetLevelDesc( 0, &desc );
    if( FAILED(hr) )
        return hr;

    hr = GetSampleOffsets_Bloom( desc.Height, afSampleOffsets, avSampleWeights, 3.0f, 2.0f );
    for( i=0; i < MAX_SAMPLES; i++ )
    {
        avSampleOffsets[i] = D3DXVECTOR2( 0.0f, afSampleOffsets[i] );
    }

    
    GetTextureRect( m_apTexBloom[1], &rectSrc );
    InflateRect( &rectSrc, -1, -1 );

    GetTextureCoords( m_apTexBloom[1], &rectSrc, m_apTexBloom[0], NULL, &coords );

    
    m_pEffect->SetTechnique("Bloom");
    m_pEffect->SetValue("g_avSampleOffsets", avSampleOffsets, sizeof(avSampleOffsets));
    m_pEffect->SetValue("g_avSampleWeights", avSampleWeights, sizeof(avSampleWeights));
    
    m_pd3dDevice->SetRenderTarget(0, pSurfBloom);
    m_pd3dDevice->SetTexture(0, m_apTexBloom[1]);
   m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
       
   
    hr = m_pEffect->Begin(&uiPassCount, 0);
    if( FAILED(hr) )
        goto LCleanReturn;

    for (uiPass = 0; uiPass < uiPassCount; uiPass++)
    {
        m_pEffect->BeginPass(uiPass);

        // Draw a fullscreen quad to sample the RT
        DrawFullScreenQuad( coords );

        m_pEffect->EndPass();
    }

    m_pEffect->End();
   
  
    hr = S_OK;

LCleanReturn:
    SAFE_RELEASE( pSurfBloomSource );
    SAFE_RELEASE( pSurfTempBloom );
    SAFE_RELEASE( pSurfBloom );
    SAFE_RELEASE( pSurfHDR );
    SAFE_RELEASE( pSurfScaledHDR );
    
    return hr;
}



//-----------------------------------------------------------------------------
// Name: DrawFullScreenQuad
// Desc: Draw a properly aligned quad covering the entire render target
//-----------------------------------------------------------------------------
void HDRSystem::DrawFullScreenQuad(float fLeftU, float fTopV, float fRightU, float fBottomV)
{
    D3DSURFACE_DESC dtdsdRT;
    PDIRECT3DSURFACE9 pSurfRT;

    // Acquire render target width and height
    m_pd3dDevice->GetRenderTarget(0, &pSurfRT);
    pSurfRT->GetDesc(&dtdsdRT);
    pSurfRT->Release();

    // Ensure that we're directly mapping texels to pixels by offset by 0.5
    // For more info see the doc page titled "Directly Mapping Texels to Pixels"
    FLOAT fWidth5 = (FLOAT)dtdsdRT.Width - 0.5f;
    FLOAT fHeight5 = (FLOAT)dtdsdRT.Height - 0.5f;

    // Draw the quad
    ScreenVertex svQuad[4];

    svQuad[0].p = D3DXVECTOR4(-0.5f, -0.5f, 0.5f, 1.0f);
    svQuad[0].t = D3DXVECTOR2(fLeftU, fTopV);

    svQuad[1].p = D3DXVECTOR4(fWidth5, -0.5f, 0.5f, 1.0f);
    svQuad[1].t = D3DXVECTOR2(fRightU, fTopV);

    svQuad[2].p = D3DXVECTOR4(-0.5f, fHeight5, 0.5f, 1.0f);
    svQuad[2].t = D3DXVECTOR2(fLeftU, fBottomV);

    svQuad[3].p = D3DXVECTOR4(fWidth5, fHeight5, 0.5f, 1.0f);
    svQuad[3].t = D3DXVECTOR2(fRightU, fBottomV);

    m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
    m_pd3dDevice->SetFVF(ScreenVertex::FVF);
    m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, svQuad, sizeof(ScreenVertex));
    m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
}


//-----------------------------------------------------------------------------
// Name: GetTextureRect()
// Desc: Get the dimensions of the texture
//-----------------------------------------------------------------------------
HRESULT HDRSystem::GetTextureRect( PDIRECT3DTEXTURE9 pTexture, RECT* pRect )
{
    HRESULT hr = S_OK;

    if( pTexture == NULL || pRect == NULL )
        return E_INVALIDARG;

    D3DSURFACE_DESC desc;
    hr = pTexture->GetLevelDesc( 0, &desc );
    if( FAILED(hr) )
        return hr;

    pRect->left = 0;
    pRect->top = 0;
    pRect->right = desc.Width;
    pRect->bottom = desc.Height;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: GetTextureCoords()
// Desc: Get the texture coordinates to use when rendering into the destination
//       texture, given the source and destination rectangles
//-----------------------------------------------------------------------------
HRESULT HDRSystem::GetTextureCoords( PDIRECT3DTEXTURE9 pTexSrc, RECT* pRectSrc, 
                          PDIRECT3DTEXTURE9 pTexDest, RECT* pRectDest, CoordRect* pCoords )
{
    HRESULT hr = S_OK;
    D3DSURFACE_DESC desc;
    float tU, tV;

    // Validate arguments
    if( pTexSrc == NULL || pTexDest == NULL || pCoords == NULL )
        return E_INVALIDARG;

    // Start with a default mapping of the complete source surface to complete 
    // destination surface
    pCoords->fLeftU = 0.0f;
    pCoords->fTopV = 0.0f;
    pCoords->fRightU = 1.0f; 
    pCoords->fBottomV = 1.0f;

    // If not using the complete source surface, adjust the coordinates
    if( pRectSrc != NULL )
    {
        // Get destination texture description
        hr = pTexSrc->GetLevelDesc( 0, &desc );
        if( FAILED(hr) )
            return hr;

        // These delta values are the distance between source texel centers in 
        // texture address space
        tU = 1.0f / desc.Width;
        tV = 1.0f / desc.Height;

        pCoords->fLeftU += pRectSrc->left * tU;
        pCoords->fTopV += pRectSrc->top * tV;
        pCoords->fRightU -= (desc.Width - pRectSrc->right) * tU;
        pCoords->fBottomV -= (desc.Height - pRectSrc->bottom) * tV;
    }

    // If not drawing to the complete destination surface, adjust the coordinates
    if( pRectDest != NULL )
    {
        // Get source texture description
        hr = pTexDest->GetLevelDesc( 0, &desc );
        if( FAILED(hr) )
            return hr;

        // These delta values are the distance between source texel centers in 
        // texture address space
        tU = 1.0f / desc.Width;
        tV = 1.0f / desc.Height;

        pCoords->fLeftU -= pRectDest->left * tU;
        pCoords->fTopV -= pRectDest->top * tV;
        pCoords->fRightU += (desc.Width - pRectDest->right) * tU;
        pCoords->fBottomV += (desc.Height - pRectDest->bottom) * tV;
    }

    return S_OK;
}
  



//-----------------------------------------------------------------------------
// Name: Scene_To_SceneScaled()
// Desc: Scale down m_pTexScene[m_CurTarget] by 1/4 x 1/4 and place the result in 
//       m_pTexSceneScaled
//-----------------------------------------------------------------------------
HRESULT HDRSystem::Scene_To_SceneScaled()
{
    HRESULT hr = S_OK;
    D3DXVECTOR2 avSampleOffsets[MAX_SAMPLES];
    
   
    // Get the new render target surface
    PDIRECT3DSURFACE9 pSurfScaledScene = NULL;
    hr = m_pTexSceneScaled->GetSurfaceLevel( 0, &pSurfScaledScene );
    if( FAILED(hr) )
        goto LCleanReturn;

    const D3DSURFACE_DESC* pBackBufferDesc = DXUTGetBackBufferSurfaceDesc();

    // Create a 1/4 x 1/4 scale copy of the HDR texture. Since bloom textures
    // are 1/8 x 1/8 scale, border texels of the HDR texture will be discarded 
    // to keep the dimensions evenly divisible by 8; this allows for precise 
    // control over sampling inside pixel shaders.
    m_pEffect->SetTechnique("DownScale4x4");

    // Place the rectangle in the center of the back buffer surface
    RECT rectSrc;
    rectSrc.left = (pBackBufferDesc->Width - m_dwCropWidth) / 2;
    rectSrc.top = (pBackBufferDesc->Height - m_dwCropHeight) / 2;
    rectSrc.right = rectSrc.left + m_dwCropWidth;
    rectSrc.bottom = rectSrc.top + m_dwCropHeight;

    // Get the texture coordinates for the render target
    CoordRect coords;
    GetTextureCoords( m_pTexScene[m_CurTarget], &rectSrc, m_pTexSceneScaled, NULL, &coords );

    // Get the sample offsets used within the pixel shader
    GetSampleOffsets_DownScale4x4( pBackBufferDesc->Width, pBackBufferDesc->Height, avSampleOffsets );
    m_pEffect->SetValue("g_avSampleOffsets", avSampleOffsets, sizeof(avSampleOffsets));
    
    m_pd3dDevice->SetRenderTarget( 0, pSurfScaledScene );
    m_pd3dDevice->SetTexture( 0, m_pTexScene[m_CurTarget] );
    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );
   
    UINT uiPassCount, uiPass;       
    hr = m_pEffect->Begin(&uiPassCount, 0);
    if( FAILED(hr) )
        goto LCleanReturn;

    for (uiPass = 0; uiPass < uiPassCount; uiPass++)
    {
        m_pEffect->BeginPass(uiPass);

        // Draw a fullscreen quad
        DrawFullScreenQuad( coords );

        m_pEffect->EndPass();
    }

    m_pEffect->End();
    

    hr = S_OK;
LCleanReturn:
    SAFE_RELEASE( pSurfScaledScene );
    return hr;
}




//-----------------------------------------------------------------------------
// Name: SceneScaled_To_BrightPass
// Desc: Run the bright-pass filter on m_pTexSceneScaled and place the result
//       in m_pTexBrightPass
//-----------------------------------------------------------------------------
HRESULT HDRSystem::SceneScaled_To_BrightPass()
{
    HRESULT hr = S_OK;

    D3DXVECTOR2 avSampleOffsets[MAX_SAMPLES];
    D3DXVECTOR4 avSampleWeights[MAX_SAMPLES];

    
    // Get the new render target surface
    PDIRECT3DSURFACE9 pSurfBrightPass;
    hr = m_pTexBrightPass->GetSurfaceLevel( 0, &pSurfBrightPass );
    if( FAILED(hr) )
        goto LCleanReturn;

    
    D3DSURFACE_DESC desc;
    m_pTexSceneScaled->GetLevelDesc( 0, &desc );

    // Get the offsets to be used within the GaussBlur5x5 pixel shader
    hr = GetSampleOffsets_GaussBlur5x5( desc.Width, desc.Height, avSampleOffsets, avSampleWeights );
    m_pEffect->SetValue("g_avSampleOffsets", avSampleOffsets, sizeof(avSampleOffsets) );
    m_pEffect->SetValue("g_avSampleWeights", avSampleWeights, sizeof(avSampleWeights) );
    

    // Get the rectangle describing the sampled portion of the source texture.
    // Decrease the rectangle to adjust for the single pixel black border.
    RECT rectSrc;
    GetTextureRect( m_pTexSceneScaled, &rectSrc );
    InflateRect( &rectSrc, -1, -1 );

    // Get the destination rectangle.
    // Decrease the rectangle to adjust for the single pixel black border.
    RECT rectDest;
    GetTextureRect( m_pTexBrightPass, &rectDest );
    InflateRect( &rectDest, -1, -1 );

    // Get the correct texture coordinates to apply to the rendered quad in order 
    // to sample from the source rectangle and render into the destination rectangle
    CoordRect coords;
    GetTextureCoords( m_pTexSceneScaled, &rectSrc, m_pTexBrightPass, &rectDest, &coords );

    // The bright-pass filter removes everything from the scene except lights and
    // bright reflections
    m_pEffect->SetTechnique("BrightPassFilter");

    m_pd3dDevice->SetRenderTarget( 0, pSurfBrightPass );
    m_pd3dDevice->SetTexture( 0, m_pTexSceneScaled );
    m_pd3dDevice->SetTexture( 1, m_pTexAdaptedLuminanceCur );
    m_pd3dDevice->SetRenderState( D3DRS_SCISSORTESTENABLE, TRUE );
    m_pd3dDevice->SetScissorRect( &rectDest );
    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
    m_pd3dDevice->SetSamplerState( 1, D3DSAMP_MINFILTER, D3DTEXF_POINT );
    m_pd3dDevice->SetSamplerState( 1, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
       
    UINT uiPass, uiPassCount;
    hr = m_pEffect->Begin(&uiPassCount, 0);
    if( FAILED(hr) )
        goto LCleanReturn;
    
    for (uiPass = 0; uiPass < uiPassCount; uiPass++)
    {
        m_pEffect->BeginPass(uiPass);

        // Draw a fullscreen quad to sample the RT
        DrawFullScreenQuad( coords );

        m_pEffect->EndPass();
    }
    
    m_pEffect->End();
    m_pd3dDevice->SetRenderState( D3DRS_SCISSORTESTENABLE, FALSE );

    hr = S_OK;
LCleanReturn:
    SAFE_RELEASE( pSurfBrightPass );
    return hr;
}




//-----------------------------------------------------------------------------
// Name: BrightPass_To_StarSource
// Desc: Perform a 5x5 gaussian blur on m_pTexBrightPass and place the result
//       in m_pTexStarSource. The bright-pass filtered image is blurred before
//       being used for star operations to avoid aliasing artifacts.
//-----------------------------------------------------------------------------
HRESULT HDRSystem::BrightPass_To_StarSource()
{
    HRESULT hr = S_OK;

    D3DXVECTOR2 avSampleOffsets[MAX_SAMPLES];
    D3DXVECTOR4 avSampleWeights[MAX_SAMPLES];

    // Get the new render target surface
    PDIRECT3DSURFACE9 pSurfStarSource;
    hr = m_pTexStarSource->GetSurfaceLevel( 0, &pSurfStarSource );
    if( FAILED(hr) )
        goto LCleanReturn;

    
    // Get the destination rectangle.
    // Decrease the rectangle to adjust for the single pixel black border.
    RECT rectDest;
    GetTextureRect( m_pTexStarSource, &rectDest );
    InflateRect( &rectDest, -1, -1 );

    // Get the correct texture coordinates to apply to the rendered quad in order 
    // to sample from the source rectangle and render into the destination rectangle
    CoordRect coords;
    GetTextureCoords( m_pTexBrightPass, NULL, m_pTexStarSource, &rectDest, &coords );

    // Get the sample offsets used within the pixel shader
    D3DSURFACE_DESC desc;
    hr = m_pTexBrightPass->GetLevelDesc( 0, &desc );
    if( FAILED(hr) )
        return hr;

    
    GetSampleOffsets_GaussBlur5x5( desc.Width, desc.Height, avSampleOffsets, avSampleWeights );
    m_pEffect->SetValue("g_avSampleOffsets", avSampleOffsets, sizeof(avSampleOffsets));
    m_pEffect->SetValue("g_avSampleWeights", avSampleWeights, sizeof(avSampleWeights));
    
    // The gaussian blur smooths out rough edges to avoid aliasing effects
    // when the star effect is run
    m_pEffect->SetTechnique("GaussBlur5x5");

    m_pd3dDevice->SetRenderTarget( 0, pSurfStarSource );
    m_pd3dDevice->SetTexture( 0, m_pTexBrightPass );
    m_pd3dDevice->SetScissorRect( &rectDest );
    m_pd3dDevice->SetRenderState( D3DRS_SCISSORTESTENABLE, TRUE );
    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );
   
    UINT uiPassCount, uiPass;       
    hr = m_pEffect->Begin(&uiPassCount, 0);
    if( FAILED(hr) )
        goto LCleanReturn;

    for (uiPass = 0; uiPass < uiPassCount; uiPass++)
    {
        m_pEffect->BeginPass(uiPass);

        // Draw a fullscreen quad
        DrawFullScreenQuad( coords );

        m_pEffect->EndPass();
    }

    m_pEffect->End();
    m_pd3dDevice->SetRenderState( D3DRS_SCISSORTESTENABLE, FALSE );

  
    hr = S_OK;
LCleanReturn:
    SAFE_RELEASE( pSurfStarSource);
    return hr;
}




//-----------------------------------------------------------------------------
// Name: StarSource_To_BloomSource
// Desc: Scale down m_pTexStarSource by 1/2 x 1/2 and place the result in 
//       m_pTexBloomSource
//-----------------------------------------------------------------------------
HRESULT HDRSystem::StarSource_To_BloomSource()
{
    HRESULT hr = S_OK;

    D3DXVECTOR2 avSampleOffsets[MAX_SAMPLES];
    
    // Get the new render target surface
    PDIRECT3DSURFACE9 pSurfBloomSource;
    hr = m_pTexBloomSource->GetSurfaceLevel( 0, &pSurfBloomSource );
    if( FAILED(hr) )
        goto LCleanReturn;

    
    // Get the rectangle describing the sampled portion of the source texture.
    // Decrease the rectangle to adjust for the single pixel black border.
    RECT rectSrc;
    GetTextureRect( m_pTexStarSource, &rectSrc );
    InflateRect( &rectSrc, -1, -1 );

    // Get the destination rectangle.
    // Decrease the rectangle to adjust for the single pixel black border.
    RECT rectDest;
    GetTextureRect( m_pTexBloomSource, &rectDest );
    InflateRect( &rectDest, -1, -1 );

    // Get the correct texture coordinates to apply to the rendered quad in order 
    // to sample from the source rectangle and render into the destination rectangle
    CoordRect coords;
    GetTextureCoords( m_pTexStarSource, &rectSrc, m_pTexBloomSource, &rectDest, &coords );

    // Get the sample offsets used within the pixel shader
    D3DSURFACE_DESC desc;
    hr = m_pTexBrightPass->GetLevelDesc( 0, &desc );
    if( FAILED(hr) )
        return hr;

    GetSampleOffsets_DownScale2x2( desc.Width, desc.Height, avSampleOffsets );
    m_pEffect->SetValue("g_avSampleOffsets", avSampleOffsets, sizeof(avSampleOffsets));

    // Create an exact 1/2 x 1/2 copy of the source texture
    m_pEffect->SetTechnique("DownScale2x2");

    m_pd3dDevice->SetRenderTarget( 0, pSurfBloomSource );
    m_pd3dDevice->SetTexture( 0, m_pTexStarSource );
    m_pd3dDevice->SetScissorRect( &rectDest );
    m_pd3dDevice->SetRenderState( D3DRS_SCISSORTESTENABLE, TRUE );
    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );
   
    UINT uiPassCount, uiPass;       
    hr = m_pEffect->Begin(&uiPassCount, 0);
    if( FAILED(hr) )
        goto LCleanReturn;

    for (uiPass = 0; uiPass < uiPassCount; uiPass++)
    {
        m_pEffect->BeginPass(uiPass);

        // Draw a fullscreen quad
        DrawFullScreenQuad( coords );

        m_pEffect->EndPass();
    }

    m_pEffect->End();
    m_pd3dDevice->SetRenderState( D3DRS_SCISSORTESTENABLE, FALSE );


    
    hr = S_OK;
LCleanReturn:
    SAFE_RELEASE( pSurfBloomSource);
    return hr;
}




//-----------------------------------------------------------------------------
// Name: GetSampleOffsets_DownScale4x4
// Desc: Get the texture coordinate offsets to be used inside the DownScale4x4
//       pixel shader.
//-----------------------------------------------------------------------------
HRESULT HDRSystem::GetSampleOffsets_DownScale4x4( DWORD dwWidth, DWORD dwHeight, D3DXVECTOR2 avSampleOffsets[] )
{
    if( NULL == avSampleOffsets )
        return E_INVALIDARG;

    float tU = 1.0f / dwWidth;
    float tV = 1.0f / dwHeight;

    // Sample from the 16 surrounding points. Since the center point will be in
    // the exact center of 16 texels, a 0.5f offset is needed to specify a texel
    // center.
    int index=0;
    for( int y=0; y < 4; y++ )
    {
        for( int x=0; x < 4; x++ )
        {
            avSampleOffsets[ index ].x = (x - 1.5f) * tU;
            avSampleOffsets[ index ].y = (y - 1.5f) * tV;
                                                      
            index++;
        }
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: GetSampleOffsets_DownScale2x2
// Desc: Get the texture coordinate offsets to be used inside the DownScale2x2
//       pixel shader.
//-----------------------------------------------------------------------------
HRESULT HDRSystem::GetSampleOffsets_DownScale2x2( DWORD dwWidth, DWORD dwHeight, D3DXVECTOR2 avSampleOffsets[] )
{
    if( NULL == avSampleOffsets )
        return E_INVALIDARG;

    float tU = 1.0f / dwWidth;
    float tV = 1.0f / dwHeight;

    // Sample from the 4 surrounding points. Since the center point will be in
    // the exact center of 4 texels, a 0.5f offset is needed to specify a texel
    // center.
    int index=0;
    for( int y=0; y < 2; y++ )
    {
        for( int x=0; x < 2; x++ )
        {
            avSampleOffsets[ index ].x = (x - 0.5f) * tU;
            avSampleOffsets[ index ].y = (y - 0.5f) * tV;
                                                      
            index++;
        }
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: GetSampleOffsets_GaussBlur5x5
// Desc: Get the texture coordinate offsets to be used inside the GaussBlur5x5
//       pixel shader.
//-----------------------------------------------------------------------------
HRESULT HDRSystem::GetSampleOffsets_GaussBlur5x5(DWORD dwD3DTexWidth,
                                                         DWORD dwD3DTexHeight,
                                                         D3DXVECTOR2* avTexCoordOffset,
                                                         D3DXVECTOR4* avSampleWeight,
                                                         FLOAT fMultiplier )
{
    float tu = 1.0f / (float)dwD3DTexWidth ;
    float tv = 1.0f / (float)dwD3DTexHeight ;

    D3DXVECTOR4 vWhite( 1.0f, 1.0f, 1.0f, 1.0f );
    
    float totalWeight = 0.0f;
    int index=0;
    for( int x = -2; x <= 2; x++ )
    {
        for( int y = -2; y <= 2; y++ )
        {
            // Exclude pixels with a block distance greater than 2. This will
            // create a kernel which approximates a 5x5 kernel using only 13
            // sample points instead of 25; this is necessary since 2.0 shaders
            // only support 16 texture grabs.
            if( abs(x) + abs(y) > 2 )
                continue;

            // Get the unscaled Gaussian intensity for this offset
            avTexCoordOffset[index] = D3DXVECTOR2( x * tu, y * tv );
            avSampleWeight[index] = vWhite * GaussianDistribution( (float)x, (float)y, 1.0f );
            totalWeight += avSampleWeight[index].x;

            index++;
        }
    }

    // Divide the current weight by the total weight of all the samples; Gaussian
    // blur kernels add to 1.0f to ensure that the intensity of the image isn't
    // changed when the blur occurs. An optional multiplier variable is used to
    // add or remove image intensity during the blur.
    for( int i=0; i < index; i++ )
    {
        avSampleWeight[i] /= totalWeight;
        avSampleWeight[i] *= fMultiplier;
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: GetSampleOffsets_Bloom
// Desc: Get the texture coordinate offsets to be used inside the Bloom
//       pixel shader.
//-----------------------------------------------------------------------------
HRESULT HDRSystem::GetSampleOffsets_Bloom( DWORD dwD3DTexSize,
                                                   float afTexCoordOffset[15],
                                                   D3DXVECTOR4* avColorWeight,
                                                   float fDeviation,
                                                   float fMultiplier )
{
    int i=0;
    float tu = 1.0f / (float)dwD3DTexSize;

    // Fill the center texel
    float weight = fMultiplier * GaussianDistribution( 0, 0, fDeviation );
    avColorWeight[0] = D3DXVECTOR4( weight, weight, weight, 1.0f );

    afTexCoordOffset[0] = 0.0f;
    
    // Fill the first half
    for( i=1; i < 8; i++ )
    {
        // Get the Gaussian intensity for this offset
        weight = fMultiplier * GaussianDistribution( (float)i, 0, fDeviation );
        afTexCoordOffset[i] = i * tu;

        avColorWeight[i] = D3DXVECTOR4( weight, weight, weight, 1.0f );
    }

    // Mirror to the second half
    for( i=8; i < 15; i++ )
    {
        avColorWeight[i] = avColorWeight[i-7];
        afTexCoordOffset[i] = -afTexCoordOffset[i-7];
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: GetSampleOffsets_Bloom
// Desc: Get the texture coordinate offsets to be used inside the Bloom
//       pixel shader.
//-----------------------------------------------------------------------------
HRESULT HDRSystem::GetSampleOffsets_Star(DWORD dwD3DTexSize,
                                                float afTexCoordOffset[15],
                                                D3DXVECTOR4* avColorWeight,
                                                float fDeviation)
{
    int i=0;
    float tu = 1.0f / (float)dwD3DTexSize;

    // Fill the center texel
    float weight = 1.0f * GaussianDistribution( 0, 0, fDeviation );
    avColorWeight[0] = D3DXVECTOR4( weight, weight, weight, 1.0f );

    afTexCoordOffset[0] = 0.0f;
    
    // Fill the first half
    for( i=1; i < 8; i++ )
    {
        // Get the Gaussian intensity for this offset
        weight = 1.0f * GaussianDistribution( (float)i, 0, fDeviation );
        afTexCoordOffset[i] = i * tu;

        avColorWeight[i] = D3DXVECTOR4( weight, weight, weight, 1.0f );
    }

    // Mirror to the second half
    for( i=8; i < 15; i++ )
    {
        avColorWeight[i] = avColorWeight[i-7];
        afTexCoordOffset[i] = -afTexCoordOffset[i-7];
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: GaussianDistribution
// Desc: Helper function for GetSampleOffsets function to compute the 
//       2 parameter Gaussian distrubution using the given standard deviation
//       rho
//-----------------------------------------------------------------------------
float HDRSystem::GaussianDistribution( float x, float y, float rho )
{
    float g = 1.0f / sqrtf( 2.0f * D3DX_PI * rho * rho );
    g *= expf( -(x*x + y*y)/(2*rho*rho) );

    return g;
}


//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has 
// entered a lost state and before IDirect3DDevice9::Reset is called. Resources created
// in the OnResetDevice callback should be released here, which generally includes all 
// D3DPOOL_DEFAULT resources. See the "Lost Devices" section of the documentation for 
// information about lost devices.
//--------------------------------------------------------------------------------------
void HDRSystem::OnLostDevice()
{
	SAFE_RELEASE(m_pLDRTexture);

	if(!m_bEnabled)
		return;
    int i=0;

    if( m_pEffect )
        m_pEffect->OnLostDevice();

	for(i=0;i<2;i++)
		SAFE_RELEASE(m_pTexScene[i]);
    SAFE_RELEASE(m_pTexSceneScaled);
    SAFE_RELEASE(m_pTexAdaptedLuminanceCur);
    SAFE_RELEASE(m_pTexAdaptedLuminanceLast);
    SAFE_RELEASE(m_pTexBrightPass);
    SAFE_RELEASE(m_pTexBloomSource);
    SAFE_RELEASE(m_pTexStarSource);
    
    for( i=0; i < NUM_TONEMAP_TEXTURES; i++)
    {
        SAFE_RELEASE(m_apTexToneMap[i]);
    }

    for( i=0; i < NUM_STAR_TEXTURES; i++ )
    {
        SAFE_RELEASE(m_apTexStar[i]);
    }

    for( i=0; i < NUM_BLOOM_TEXTURES; i++ )
    {
        SAFE_RELEASE(m_apTexBloom[i]);
    }
}


//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has 
// been destroyed, which generally happens as a result of application termination or 
// windowed/full screen toggles. Resources created in the OnCreateDevice callback 
// should be released here, which generally includes all D3DPOOL_MANAGED resources. 
//--------------------------------------------------------------------------------------
void HDRSystem::OnDestroyDevice()
{
    SAFE_RELEASE(m_pEffect);
}
