//=========== Copyright (c) 2003, Tim Johnson. All rights reserved. ===========
// Defines complete basic rendering interface. Along with Texture and Mesh
//
//
// FIXME: Floating point math for canvas scaling sucks. at 0,0 Jer says it draws at 1,1 pixels when scaled
// smaller.
//
// TODO: Convert old occlusion queries over to the new class system
// TODO: Use RenderBase for everything possible
//=============================================================================
#include "stdafx.h"
#include "dxstdafx.h"
#include <d3d9.h>
#include "TextureManager.h"
#include "BatchRenderer.h"
#include "HDR.h"
#include "Editor.h"
#include "GUISystem.h"
#include "Graphics\DShowTextures.h"
#include "PostProcess.h"
#include "Profiler.h"
#include "ShadowMapping.h"
#include <stdio.h>
#include <shlobj.h>
#include <dsound.h>


LPDIRECT3DSTATEBLOCK9 states, defaultStates;
#define FMT_3DC MAKEFOURCC('A', 'T', 'I', '2') // ATI 3Dc

// Temporary global config list until we assign engine
CConfigManager* g_pCMList;

//--------------------------------------------------------------------------------------
// Defines
//--------------------------------------------------------------------------------------
extern vector<StaticModel*> modelCache; // So we can release D3DQUERY events from models

RenderBase::RenderBase(){
	RenderDevice::Instance()->RegisterCallback(this);
}

RenderBase::~RenderBase(){
	RenderDevice::Instance()->RemoveCallback(this);
}

//--------------------------------------------------------------------------------------
// Entry
//--------------------------------------------------------------------------------------
RenderDevice::~RenderDevice(){
}

RenderDevice::RenderDevice()
{
	m_bDeviceLost = true;
}

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void RenderDevice::UpdateViewport()
{
    D3DVIEWPORT9 vp;
    RenderWrap::dev->GetViewport(&vp);
	//if(vp.MinZ != RenderDevice::Instance()->MinViewportZ || vp.MaxZ != RenderDevice::Instance()->MaxViewportZ)
	{
		vp.MinZ = RenderDevice::Instance()->MinViewportZ;
		vp.MaxZ = RenderDevice::Instance()->MaxViewportZ;
		RenderWrap::dev->SetViewport(&vp);
	}
}

//--------------------------------------------------------------------------------------
void RenderDevice::ResetAllStates()
{
    states->Apply();
    defaultStates->Apply();
    RenderWrap::dev->SetTextureStageState(0,D3DTSS_COLORARG1,D3DTA_TEXTURE);
    RenderWrap::dev->SetTextureStageState(0,D3DTSS_COLORARG2,D3DTA_DIFFUSE);
	RenderWrap::dev->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_MODULATE);
	RenderWrap::dev->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_MODULATE);
    RenderWrap::dev->SetTextureStageState(0,D3DTSS_ALPHAARG1,D3DTA_TEXTURE);
    RenderWrap::dev->SetTextureStageState(0,D3DTSS_ALPHAARG2,D3DTA_DIFFUSE);
}

//--------------------------------------------------------------------------------------
// Expose settings dialog to game
//--------------------------------------------------------------------------------------
CD3DSettingsDlg* RenderDevice::GetSettingsDialog(){
	return DXUTGetSettingsDialog();
}

//-----------------------------------------------------------------------------
// Grab/release frame sync objects to prevent mouse lag
//-----------------------------------------------------------------------------
IDirect3DQuery9 *g_pFrameSyncQueryObject = NULL;

void AllocFrameSyncObjects( void )
{
	HRESULT hr = RenderWrap::dev->CreateQuery( D3DQUERYTYPE_EVENT, &g_pFrameSyncQueryObject );
	if( hr == D3DERR_NOTAVAILABLE )
	{
		Warning( "D3DQUERYTYPE_EVENT not available on this driver\n" );
		assert( g_pFrameSyncQueryObject == NULL );
	}
	else
	{
		assert( hr == D3D_OK );
		assert( g_pFrameSyncQueryObject );
		g_pFrameSyncQueryObject->Issue( D3DISSUE_END );
	}
}

void RenderDevice::UnbindTextures()
{
    for(int i=0;i<DXUTGetDeviceCaps()->MaxSimultaneousTextures;i++)
        RenderWrap::dev->SetTexture(i,0);
}

void FreeFrameSyncObjects( void )
{
	if( !g_pFrameSyncQueryObject )
	{
		return;
	}
	g_pFrameSyncQueryObject->Release();
	g_pFrameSyncQueryObject = NULL;
}

void DefaultStates()
{
	// Set miscellaneous fixed render states
	RenderWrap::SetRS( D3DRS_ZENABLE, D3DZB_TRUE );
	RenderWrap::SetRS( D3DRS_CULLMODE, D3DCULL_CCW);// CCW - cull counter-clockwise crap
	RenderWrap::SetRS( D3DRS_ZWRITEENABLE, TRUE);
	RenderWrap::SetRS( D3DRS_ZFUNC, D3DCMP_LESSEQUAL);

	RenderWrap::SetRS( D3DRS_ALPHABLENDENABLE, FALSE );
	RenderWrap::SetRS( D3DRS_SRCBLEND, D3DBLEND_ONE);
	RenderWrap::SetRS( D3DRS_DESTBLEND, D3DBLEND_INVDESTCOLOR   );

	float zero = 0.0f;
	float one = 1.0f;
	DWORD dZero = *((DWORD*)(&zero));
	DWORD dOne = *((DWORD*)(&one));

	// Set default values for all dx render states.
	RenderWrap::SetRS( D3DRS_FILLMODE, D3DFILL_SOLID );
	RenderWrap::SetRS( D3DRS_SHADEMODE, D3DSHADE_GOURAUD );
	RenderWrap::SetRS( D3DRS_LASTPIXEL, TRUE );
	RenderWrap::SetRS( D3DRS_CULLMODE, D3DCULL_CCW );
	RenderWrap::SetRS( D3DRS_DITHERENABLE, FALSE );
	RenderWrap::SetRS( D3DRS_FOGENABLE, FALSE );
	RenderWrap::SetRS( D3DRS_SPECULARENABLE, FALSE );
	RenderWrap::SetRS( D3DRS_FOGCOLOR, 0 );
	RenderWrap::SetRS( D3DRS_FOGTABLEMODE, D3DFOG_NONE );
	RenderWrap::SetRS( D3DRS_FOGSTART, dZero );
	RenderWrap::SetRS( D3DRS_FOGEND, dOne );
	RenderWrap::SetRS( D3DRS_FOGDENSITY, dZero );
	RenderWrap::SetRS( D3DRS_RANGEFOGENABLE, FALSE );
	RenderWrap::SetRS( D3DRS_STENCILENABLE, FALSE);
	RenderWrap::SetRS( D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP );
	RenderWrap::SetRS( D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP );
	RenderWrap::SetRS( D3DRS_STENCILPASS, D3DSTENCILOP_KEEP );
	RenderWrap::SetRS( D3DRS_STENCILFUNC, D3DCMP_ALWAYS );
	RenderWrap::SetRS( D3DRS_STENCILREF, 0 );
	RenderWrap::SetRS( D3DRS_STENCILMASK, 0xFFFFFFFF );
	RenderWrap::SetRS( D3DRS_STENCILWRITEMASK, 0xFFFFFFFF );
	RenderWrap::SetRS( D3DRS_TEXTUREFACTOR, 0xFFFFFFFF );
	RenderWrap::SetRS( D3DRS_WRAP0, 0 );
	RenderWrap::SetRS( D3DRS_WRAP1, 0 );
	RenderWrap::SetRS( D3DRS_WRAP2, 0 );
	RenderWrap::SetRS( D3DRS_WRAP3, 0 );
	RenderWrap::SetRS( D3DRS_WRAP4, 0 );
	RenderWrap::SetRS( D3DRS_WRAP5, 0 );
	RenderWrap::SetRS( D3DRS_WRAP6, 0 );
	RenderWrap::SetRS( D3DRS_WRAP7, 0 );
	RenderWrap::SetRS( D3DRS_CLIPPING, TRUE );
	RenderWrap::SetRS( D3DRS_LIGHTING, FALSE );
	RenderWrap::SetRS( D3DRS_AMBIENT, 0 );
	RenderWrap::SetRS( D3DRS_FOGVERTEXMODE, D3DFOG_NONE);
	RenderWrap::SetRS( D3DRS_COLORVERTEX, TRUE );
	RenderWrap::SetRS( D3DRS_LOCALVIEWER, TRUE );
	RenderWrap::SetRS( D3DRS_NORMALIZENORMALS, FALSE );
	RenderWrap::SetRS( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1 );
	RenderWrap::SetRS( D3DRS_SPECULARMATERIALSOURCE, D3DMCS_COLOR2 );
	RenderWrap::SetRS( D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL );
	RenderWrap::SetRS( D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL );
	RenderWrap::SetRS( D3DRS_VERTEXBLEND, D3DVBF_DISABLE );
	RenderWrap::SetRS( D3DRS_CLIPPLANEENABLE, 0 );
	RenderWrap::SetRS( D3DRS_POINTSIZE, dOne );
	RenderWrap::SetRS( D3DRS_POINTSIZE_MIN, dOne );
	RenderWrap::SetRS( D3DRS_POINTSPRITEENABLE, FALSE );
	RenderWrap::SetRS( D3DRS_POINTSCALEENABLE, FALSE );
	RenderWrap::SetRS( D3DRS_POINTSCALE_A, dOne );
	RenderWrap::SetRS( D3DRS_POINTSCALE_B, dZero );
	RenderWrap::SetRS( D3DRS_POINTSCALE_C, dZero );
	RenderWrap::SetRS( D3DRS_MULTISAMPLEANTIALIAS, TRUE );
	RenderWrap::SetRS( D3DRS_MULTISAMPLEMASK, 0xFFFFFFFF );
	RenderWrap::SetRS( D3DRS_PATCHEDGESTYLE, D3DPATCHEDGE_DISCRETE );
	RenderWrap::SetRS( D3DRS_DEBUGMONITORTOKEN, D3DDMT_ENABLE );
	float sixtyFour = 64.0f;
	RenderWrap::SetRS( D3DRS_POINTSIZE_MAX, *((DWORD*)(&sixtyFour)));
	RenderWrap::SetRS( D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE );
	RenderWrap::SetRS( D3DRS_TWEENFACTOR, dZero );
	RenderWrap::SetRS( D3DRS_BLENDOP, D3DBLENDOP_ADD );
	RenderWrap::SetRS( D3DRS_POSITIONDEGREE, D3DDEGREE_CUBIC );
	RenderWrap::SetRS( D3DRS_NORMALDEGREE, D3DDEGREE_LINEAR );
	RenderWrap::SetRS( D3DRS_SCISSORTESTENABLE, FALSE);
	RenderWrap::SetRS( D3DRS_SLOPESCALEDEPTHBIAS, dZero );
	RenderWrap::SetRS( D3DRS_ANTIALIASEDLINEENABLE, TRUE );
	RenderWrap::SetRS( D3DRS_MINTESSELLATIONLEVEL, dOne );
	RenderWrap::SetRS( D3DRS_MAXTESSELLATIONLEVEL, dOne );
	RenderWrap::SetRS( D3DRS_ADAPTIVETESS_X, dZero );
	RenderWrap::SetRS( D3DRS_ADAPTIVETESS_Y, dZero );
	RenderWrap::SetRS( D3DRS_ADAPTIVETESS_Z, dOne );
	RenderWrap::SetRS( D3DRS_ADAPTIVETESS_W, dZero );
	RenderWrap::SetRS( D3DRS_ENABLEADAPTIVETESSELLATION, FALSE );
	RenderWrap::SetRS( D3DRS_TWOSIDEDSTENCILMODE, FALSE );
	RenderWrap::SetRS( D3DRS_CCW_STENCILFAIL, 0x00000001);
	RenderWrap::SetRS( D3DRS_CCW_STENCILZFAIL, 0x00000001 );
	RenderWrap::SetRS( D3DRS_CCW_STENCILPASS, 0x00000001 );
	RenderWrap::SetRS( D3DRS_CCW_STENCILFUNC, 0x00000008 );
	RenderWrap::SetRS( D3DRS_COLORWRITEENABLE1, 0x0000000f );
	RenderWrap::SetRS( D3DRS_COLORWRITEENABLE2, 0x0000000f );
	RenderWrap::SetRS( D3DRS_COLORWRITEENABLE3, 0x0000000f);
	RenderWrap::SetRS( D3DRS_BLENDFACTOR, 0);//0xffffffff );
	RenderWrap::SetRS( D3DRS_SRGBWRITEENABLE, 0);
	RenderWrap::SetRS( D3DRS_DEPTHBIAS, dZero );
	RenderWrap::SetRS( D3DRS_WRAP8, 0 );
	RenderWrap::SetRS( D3DRS_WRAP9, 0 );
	RenderWrap::SetRS( D3DRS_WRAP10, 0 );
	RenderWrap::SetRS( D3DRS_WRAP11, 0 );
	RenderWrap::SetRS( D3DRS_WRAP12, 0 );
	RenderWrap::SetRS( D3DRS_WRAP13, 0 );
	RenderWrap::SetRS( D3DRS_WRAP14, 0 );
	RenderWrap::SetRS( D3DRS_WRAP15, 0 );
	RenderWrap::SetRS( D3DRS_SEPARATEALPHABLENDENABLE, FALSE );
	RenderWrap::SetRS( D3DRS_SRCBLENDALPHA, D3DBLEND_ONE );
	RenderWrap::SetRS( D3DRS_DESTBLENDALPHA, D3DBLEND_ZERO );
	RenderWrap::SetRS( D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD );
}

//--------------------------------------------------------------------------------------
// Callbacks
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// This callback function will be called once at the beginning of every frame. This is the
// best location for your application to handle updates to the scene, but is not 
// intended to contain actual rendering calls, which should instead be placed in the 
// OnFrameRender callback.  
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime ){
	HDRSystem::Instance()->OnFrameMove();
	// Game should be called here
	// NOTE/TODO: I've unhooked time vars. If I update, I must rehook
}

//--------------------------------------------------------------------------------------
// Triggers OnFrameRender()
//--------------------------------------------------------------------------------------
void RenderDevice::DoRendering(){
	DXUTRender3DEnvironment();
}

//--------------------------------------------------------------------------------------
// Triggers OnFrameRender()
//--------------------------------------------------------------------------------------
void RenderDevice::FlipHDRTargets()
{
	if(HDRSystem::Instance()->m_bEnabled)
		HDRSystem::Instance()->FlipTargets();
}

//--------------------------------------------------------------------------------------
// This callback function will be called at the end of every frame to perform all the 
// rendering calls for the scene, and it will also be called if the window needs to be 
// repainted. After this function has returned, the sample framework will call 
// IDirect3DDevice9::Present to display the contents of the next buffer in the swap chain
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime )
{
	RenderDevice::Instance()->RenderCallback();
}


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
bool RenderDevice::IsDeviceLost()
{
	return m_bDeviceLost;
}

//--------------------------------------------------------------------------------------
// Does target support alpha blending
// FIXME: I'm forcing alpha blending off if any post-process effects are active
// because depth of field needs the alpha channel. This is an evil hack though
//--------------------------------------------------------------------------------------
bool RenderDevice::TargetSupportsBlending()
{
    return PixelShaderVersion>=3 || HDRSystem::Instance()->m_IsLDR;
}

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void RenderDevice::RenderCallback(){
	GetCanvas()->statNum = 0; // Used by canvas PrintStat
	m_Vertices = 0;
	m_DrawCalls = 0;
	m_Triangles = 0;

	StartMiniTimer();

	// We allow stalling of the pipeline to reduce input lag
	if(m_bHardwareSync && g_pFrameSyncQueryObject){
		BOOL bDummy;
		HRESULT hr;
		// FIXME: Could install a callback into the materialsystem to do something while waiting for
		// the frame to finish (update sound, etc.)
		do
		{
			hr = g_pFrameSyncQueryObject->GetData( &bDummy, sizeof( bDummy ), D3DGETDATA_FLUSH );
		} while( hr == S_FALSE );
		// FIXME: Need to check for hr==D3DERR_DEVICELOST here.
		assert( hr != D3DERR_DEVICELOST );
		assert( hr == S_OK );
		g_pFrameSyncQueryObject->Issue( D3DISSUE_END );
	}


	// Center mouse (rather excessive, needed to be sure)
	POINT ptCursor;
	GetCursorPos( &ptCursor );
	if( IsFullscreen() )
		ScreenToClient( DXUTGetHWND(), &ptCursor );

	// Center cursor if not shown
	if( m_bCenterCursor ){

		POINT ptCenter;
		ptCenter.x = GetViewportX()*0.5f;
		ptCenter.y = GetViewportY()*0.5f;
		ClientToScreen(DXUTGetHWND(),&ptCenter);
		if(!IsFullscreen()){
			// Only center the cursor if it's moved
			if(ptCursor.x != ptCenter.x || ptCursor.y != ptCenter.y)
				SetCursorPos(ptCenter.x,ptCenter.y);
		}
		else{
			RenderWrap::dev->SetCursorPosition( ptCenter.x, ptCenter.y, 0L );
		}
	}
	RenderWrap::dev->SetCursorPosition( ptCursor.x, ptCursor.y, 0L );

	// Set up HDR target, etc
	HDRSystem::Instance()->PreRender();

	// NOTE: Stencil ops are evil. Research heavily before touching them
	RenderWrap::dev->Clear( 0L, NULL, D3DCLEAR_ZBUFFER|((ClearScreen||Editor::Instance()->GetEditorMode())?D3DCLEAR_TARGET:0)/*|D3DCLEAR_STENCIL*/,ClearColor, 1.0f, 0L );

	DXASSERT(RenderWrap::dev->BeginScene());

	Profiler::Get()->PresentMS += StopMiniTimer();

	// Issue the game callback here...
	assert(m_FrameRenderCallback);
	m_FrameRenderCallback();


	states->Apply();

	// NOTE: It is *critical* we reset these states before continuing
	// If the render callback left them in a wrong state we could get video corruption
	// when presenting
	RenderWrap::dev->SetVertexShader(NULL);
	RenderWrap::dev->SetFVF((D3DFVF_XYZ|D3DFVF_DIFFUSE |D3DFVF_TEX1));
	RenderWrap::dev->SetPixelShader(NULL);
	RenderWrap::SetRS(D3DRS_FOGENABLE,FALSE);
	for(int i=0;i<8;i++)
		RenderWrap::ClearTextureLevel(i);

	// Reset PresentMS here, because we want the next frame to record the following timing:
    if(Profiler::Get()->NumFrames == 1)
        Profiler::Get()->PresentMS = 0;

	StartMiniTimer();
	RenderWrap::dev->EndScene();
	HDRSystem::Instance()->PostRenderClean();

	// Record time to present
	Profiler::Get()->PresentMS += StopMiniTimer();
}

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void RenderDevice::Update(Camera* ActiveCamera){
	TextureManager::UpdateAnimatedTextures();
}

//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has been 
// reset, which will happen after a lost device scenario. This is the best location to 
// create D3DPOOL_DEFAULT resources since these resources need to be reloaded whenever 
// the device is lost. Resources created here should be released in the OnLostDevice 
// callback. 
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc ){
	return RenderDevice::Instance()->OnResetDevice(pd3dDevice,pBackBufferSurfaceDesc);
}

HRESULT RenderDevice::OnResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc ){
	SetDeviceStats();
	LogPrintf("OnResetDevice()...");

    // Set viewport here so it's captured by stateblock and used from now on
    MaxViewportZ = 1;
    // Leave some buffer room for weapon drawing
    MinViewportZ = 0.05f;

    D3DVIEWPORT9 vp;
    RenderWrap::dev->GetViewport(&vp);
    vp.MinZ = MinViewportZ;
    vp.MaxZ = MaxViewportZ;
    RenderWrap::dev->SetViewport(&vp);

	// Capture the current device states
	RenderWrap::dev->CreateStateBlock(D3DSBT_ALL,&states);

	RenderWrap::Restore();

   // Test for 3DC
    // Stupid NVIDIA cards are returning support as R8G8B8, grrr!!
  /* LPDIRECT3DTEXTURE9 p3DcTexture = NULL;
   if (FAILED (RenderWrap::dev->CreateTexture (256,256, 1,
                                          D3DUSAGE_DYNAMIC, (D3DFORMAT)FMT_3DC,
                                          D3DPOOL_DEFAULT , &p3DcTexture, NULL)))
   {
       m_b3DC = false;
   }
   else
   {
       m_b3DC = true;
       SAFE_RELEASE(p3DcTexture);
       LogPrintf("Card supports 3DC compression!");
   }*/

   m_b3DC = false;

	// Capture default states
	RenderWrap::dev->BeginStateBlock();
		// States
		DefaultStates();
		// Settings
		SetSRGB(GetSRGB());
		SetGammaLevel(true,1,1,GetGamma());
		SetAnisotropyLevel(GetAnisotropyLevel());
	RenderWrap::dev->EndStateBlock(&defaultStates);

	GetCanvas()->Height = pBackBufferSurfaceDesc->Height;
	GetCanvas()->Width  = pBackBufferSurfaceDesc->Width;
    ShaderManager::Instance()->D3DRestore();
	GetCanvas()->RestoreDeviceObjects();

	AllocFrameSyncObjects();
	TextureManager::RestoreAll();
	TextureManager::maxTextures = DXUTGetDeviceCaps()->MaxSimultaneousTextures;

	
	ShaderManager::Instance()->UpdateRenderSettings();

	// Set default aspect. HDRSystem may override this
	m_fAspect = float(GetViewportX()) / float(GetViewportY());;
	HDRSystem::Instance()->OnResetDevice();
	PostProcess::Instance()->OnResetDevice();
	
	// MaterialManager Needs to init after all renderstates are set
	MaterialManager::Instance()->Initialize(); 
	BatchRenderer::Instance()->Initialize();

	for(int i=0;i<m_Callbacks.size();i++){
		m_Callbacks[i]->OnResetDevice();
	}

	// GUI
	GUISystem::Instance()->OnResize();

	// Recreate queries
	for(int i=0;i<m_Queries.size();i++){
		DXASSERT(RenderWrap::dev->CreateQuery(D3DQUERYTYPE_OCCLUSION,&m_Queries[i]->query));
	}

	m_bDeviceLost = false;
	LogPrintf("...completed.");
	return S_OK;
}

//--------------------------------------------------------------------------------------
// Create occlusion query
//--------------------------------------------------------------------------------------
void OcclusionQuery::Create()
{
    pending = false;
    if(query)
        return;

	HRESULT hr = RenderWrap::dev->CreateQuery(D3DQUERYTYPE_OCCLUSION,&query);
	if(hr == D3DERR_NOTAVAILABLE){
		return;
	}
    RenderDevice::Instance()->m_Queries.push_back(this);
}

//--------------------------------------------------------------------------------------
// Free occlusion query
//--------------------------------------------------------------------------------------
void OcclusionQuery::Free()
{
	SAFE_RELEASE(query);

	for(int i=0;i<RenderDevice::Instance()->m_Queries.size();i++)
    {
		if(RenderDevice::Instance()->m_Queries[i] == this)
        {
			RenderDevice::Instance()->m_Queries.erase(RenderDevice::Instance()->m_Queries.begin() + i);
			return;
		}
	}
}
//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
DWORD OcclusionQuery::GetPixels()
{
	DWORD pixels;
    HRESULT hr;
	while((hr=query->GetData(&pixels,sizeof(DWORD),D3DGETDATA_FLUSH )) == S_FALSE);
    if(hr == D3DERR_DEVICELOST)
        hr=hr;
    pending = false;
	return pixels;
}
//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void OcclusionQuery::Begin(){
    assert(!pending); // Queries can't be pending before you start a new one
	query->Issue(D3DISSUE_BEGIN);
}
//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void OcclusionQuery::End(){
	query->Issue(D3DISSUE_END);
    pending = true;
}

//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has 
// entered a lost state and before IDirect3DDevice9::Reset is called. Resources created
// in the OnResetDevice callback should be released here, which generally includes all 
// D3DPOOL_DEFAULT resources. See the "Lost Devices" section of the documentation for 
// information about lost devices.
//--------------------------------------------------------------------------------------
void CALLBACK OnLostDevice(){
	RenderDevice::Instance()->OnLostDevice();
}

void RenderDevice::OnLostDevice(){
	LogPrintf("OnLostDevice()...");
	m_bDeviceLost = true;

	SAFE_RELEASE(states);
	SAFE_RELEASE(defaultStates);

	FreeFrameSyncObjects();

	TextureManager::OnLostDevice();
	ShaderManager::Instance()->D3DInvalidate();
	RenderDevice::Instance()->GetCanvas()->InvalidateDeviceObjects();
	HDRSystem::Instance()->PostRenderClean(); // In case surfaces are still held
	HDRSystem::Instance()->OnLostDevice();
	PostProcess::Instance()->OnLostDevice();

	// Release any queries models may be holding
	for(int i=0;i<modelCache.size();i++){
		SAFE_RELEASE(modelCache[i]->m_OcclusionQuery);
	}

	// Release any queries
	for(int i=0;i<RenderDevice::Instance()->m_Queries.size();i++){
		SAFE_RELEASE(RenderDevice::Instance()->m_Queries[i]->query);
	}

	BatchRenderer::Instance()->InvalidateDeviceObjects();

	for(int i=0;i<m_Callbacks.size();i++){
		m_Callbacks[i]->OnLostDevice();
	}

	LogPrintf("...completed");
}

//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has been 
// created, which will happen during application initialization and windowed/full screen 
// toggles. This is the best location to create D3DPOOL_MANAGED resources since these 
// resources need to be reloaded whenever the device is destroyed. Resources created  
// here should be released in the OnDestroyDevice callback. 
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc )
{
    HRESULT hr;
	LogPrintf("OnCreateDevice()...");
	RenderWrap::dev = pd3dDevice;
	RenderWrap::d3d = DXUTGetD3DObject();

    //
    // Obtain the CConfigManager object for the 3D device created.
    // Point g_pCM to the CConfigManager object.
    D3DCAPS9 Caps;
    pd3dDevice->GetDeviceCaps( &Caps );
    Engine::Instance()->ConfigManager = g_pCMList + Caps.AdapterOrdinal;

    CConfigManager* pCM = Engine::Instance()->ConfigManager;
    // Verify requirement and prompt the user if any requirement is not met.
    hr = pCM->VerifyRequirements();
    if( FAILED( hr ) )
        return hr;

    // Check for unsupported display device.
/*    if( pCM->cf_UnsupportedCard )
    {
        ::MessageBoxW( NULL, L"The display device is not supported by this application. "
                            L"The program will now exit.", L"Reality Engine", MB_OK|MB_ICONERROR );
        return E_FAIL;
    }

    // Check for invalid display driver
    if( pCM->cf_InvalidDriver )
    {
        ::MessageBoxW( NULL, L"The display driver detected is incompatible with this application. "
                            L"The program will now exit.", L"Reality Engine", MB_OK|MB_ICONERROR );
        return E_FAIL;
    }

    // Check for old display driver
    if( pCM->cf_OldDriver )
    {
        ::MessageBoxW( NULL, L"The display driver is out-of-date. We recommend that you "
                            L"install the latest driver for this display device. Click OK to continue.",
                            L"Reality Engine", MB_OK|MB_ICONWARNING );
    }

    // Check for known prototype card
    if( pCM->cf_PrototypeCard )
    {
        ::MessageBoxW( NULL, L"A prototype display card is detected.  We recommend using "
                            L"only supported retail display cards to run this application.",
                            L"Reality Engine", MB_OK|MB_ICONWARNING );
    }

    // Check for invalid sound driver
    if( pCM->cf_InvalidSoundDriver )
    {
        ::MessageBoxW( NULL, L"The sound driver detected is incompatible with this application. "
                            L"The program will now exit.", L"Reality Engine", MB_OK|MB_ICONERROR );
        return E_FAIL;
    }

    // Check for old sound driver
    if( pCM->cf_OldSoundDriver )
    {
        ::MessageBoxW( NULL, L"The sound driver is out-of-date. We recommend that you "
                            L"install the latest driver for this sound device.",
                            L"Reality Engine", MB_OK|MB_ICONWARNING );
    }
*/
    // Go through the list of resolution and remove those that are excluded for this card.
 /*   CD3DEnumAdapterInfo* pAdapterInfo = DXUTGetEnumeration()->GetAdapterInfo( Caps.AdapterOrdinal );
    for( int idm = pAdapterInfo->displayModeList.GetSize() - 1; idm >= 0; --idm )
    {
        D3DDISPLAYMODE DisplayMode = pAdapterInfo->displayModeList.GetAt( idm );

        if( DisplayMode.Width > pCM->cf_MaximumResolution )
            pAdapterInfo->displayModeList.Remove( idm );
    }
*/
	LogPrintf("CreateDevice. Format=%d Usage=%d Type=%d Pool=%d", 
		(int)pBackBufferSurfaceDesc->Format,(int)pBackBufferSurfaceDesc->Usage,(int)pBackBufferSurfaceDesc->Type,
		(int)pBackBufferSurfaceDesc->Pool);

	RenderDevice::Instance()->SetDeviceStats();
	TextureManager::Initialize(pBackBufferSurfaceDesc);

	RenderDevice::Instance()->GetCanvas()->InitDeviceObjects();
	ShaderManager::Instance()->D3DInitialize();
	HDRSystem::Instance()->OnCreateDevice(pBackBufferSurfaceDesc);

	LogPrintf("...completed");
	return S_OK;
}

//--------------------------------------------------------------------------------------
// Restores windowed gamma to the value before we changed it
//--------------------------------------------------------------------------------------
void RenderDevice::RestoreGamma(){
	if(!DXUTIsWindowed())
		RenderWrap::dev->SetGammaRamp(0,D3DSGR_CALIBRATE,(D3DGAMMARAMP*)&originalGamma);
	else{
		HDC hDC;
		hDC = GetDC(NULL);
		SetDeviceGammaRamp(hDC, &originalGamma);
		ReleaseDC(NULL, hDC);
	}
}

//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has 
// been destroyed, which generally happens as a result of application termination or 
// windowed/full screen toggles. Resources created in the OnCreateDevice callback 
// should be released here, which generally includes all D3DPOOL_MANAGED resources. 
//--------------------------------------------------------------------------------------
void CALLBACK OnDestroyDevice()
{
	RenderDevice::Instance()->OnDestroyDevice();
}

void RenderDevice::OnDestroyDevice(){
	LogPrintf("OnDestroyDevice()...");
	m_bDeviceLost = true;
	// Restore gamma before we lose the device, possibly forever
	RestoreGamma();

	ShaderManager::Instance()->D3DDelete();
	TextureManager::FreeAll();
	HDRSystem::Instance()->OnDestroyDevice();
	PostProcess::Instance()->OnDestroyDevice();

	GetCanvas()->DeleteDeviceObjects();

	for(int i=0;i<m_Callbacks.size();i++){
		m_Callbacks[i]->OnDestroyDevice();
	}

	RenderWrap::dev = 0;

	LogPrintf("...completed");
}


//--------------------------------------------------------------------------------------
// Returns true if a particular depth-stencil format can be created and used with
// an adapter format and backbuffer format combination.
BOOL IsDepthFormatOk( D3DFORMAT DepthFormat,
                      D3DFORMAT AdapterFormat,
                      D3DFORMAT BackBufferFormat )
{
    // Verify that the depth format exists
    HRESULT hr = DXUTGetD3DObject()->CheckDeviceFormat( D3DADAPTER_DEFAULT,
                                                        D3DDEVTYPE_HAL,
                                                        AdapterFormat,
                                                        D3DUSAGE_DEPTHSTENCIL,
                                                        D3DRTYPE_SURFACE,
                                                        DepthFormat );
    if( FAILED( hr ) ) return FALSE;

    // Verify that the backbuffer format is valid
    hr = DXUTGetD3DObject()->CheckDeviceFormat( D3DADAPTER_DEFAULT,
                                                D3DDEVTYPE_HAL,
                                                AdapterFormat,
                                                D3DUSAGE_RENDERTARGET,
                                                D3DRTYPE_SURFACE,
                                                BackBufferFormat );
    if( FAILED( hr ) ) return FALSE;

    // Verify that the depth format is compatible
    hr = DXUTGetD3DObject()->CheckDepthStencilMatch( D3DADAPTER_DEFAULT,
                                                     D3DDEVTYPE_HAL,
                                                     AdapterFormat,
                                                     BackBufferFormat,
                                                     DepthFormat );

    return SUCCEEDED(hr);
}

//--------------------------------------------------------------------------------------
// Called during device initialization, this code checks the device for some 
// minimum set of capabilities, and rejects those that don't pass by returning false.
//--------------------------------------------------------------------------------------
bool CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, 
                                  D3DFORMAT BackBufferFormat, bool bWindowed )
{
	RenderWrap::d3d = DXUTGetD3DObject();

	if(!HDRSystem::Instance()->IsDeviceAcceptable(pCaps,AdapterFormat,BackBufferFormat,bWindowed))
    {
        // No HDR support, so disable
        if(pCaps->DeviceType == D3DDEVTYPE_HAL)
            HDRSystem::Instance()->m_bEnabled = false;
        else
            return false;
	}

	 // Must support stencil buffer
    // TIM: Stencil buffer not used, can cause problems on FX cards if S8 is used and
    // StencilEnable = false. Missing triangles and such
    /*if( !IsDepthFormatOk( D3DFMT_D24S8,
                          AdapterFormat,
                          BackBufferFormat ) &&
        !IsDepthFormatOk( D3DFMT_D24X4S4,
                          AdapterFormat,
                          BackBufferFormat ) &&
        !IsDepthFormatOk( D3DFMT_D15S1,
                          AdapterFormat,
                          BackBufferFormat ) &&
        !IsDepthFormatOk( D3DFMT_D24FS8,
                          AdapterFormat,
                          BackBufferFormat ) )
        return false;
*/

   return true;
}

//--------------------------------------------------------------------------------------
// This callback function is called immediately before a device is created to allow the 
// application to modify the device settings. The supplied pDeviceSettings parameter 
// contains the settings that the framework has selected for the new device, and the 
// application can make any desired changes directly to this structure.  Note however that 
// the sample framework will not correct invalid device settings so care must be taken 
// to return valid device settings, otherwise IDirect3D9::CreateDevice() will fail.  
//--------------------------------------------------------------------------------------
void CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, const D3DCAPS9* pCaps )
{
	pDeviceSettings->pp.EnableAutoDepthStencil = TRUE;

/*
	// This requires a stencil buffer.
    if( IsDepthFormatOk( D3DFMT_D24S8,
                         pDeviceSettings->AdapterFormat,
                         pDeviceSettings->pp.BackBufferFormat ) )
        pDeviceSettings->pp.AutoDepthStencilFormat = D3DFMT_D24S8;
    else
    if( IsDepthFormatOk( D3DFMT_D24X4S4,
                         pDeviceSettings->AdapterFormat,
                         pDeviceSettings->pp.BackBufferFormat ) )
        pDeviceSettings->pp.AutoDepthStencilFormat = D3DFMT_D24X4S4;
    else
    if( IsDepthFormatOk( D3DFMT_D24FS8,
                         pDeviceSettings->AdapterFormat,
                         pDeviceSettings->pp.BackBufferFormat ) )
        pDeviceSettings->pp.AutoDepthStencilFormat = D3DFMT_D24FS8;
    else
    if( IsDepthFormatOk( D3DFMT_D15S1,
                         pDeviceSettings->AdapterFormat,
                         pDeviceSettings->pp.BackBufferFormat ) )
        pDeviceSettings->pp.AutoDepthStencilFormat = D3DFMT_D15S1;

*/
	// No PURE device, thanks
	pDeviceSettings->BehaviorFlags &= ~D3DCREATE_PUREDEVICE;

	//if(Editor::Instance()->GetEditorMode())
		pDeviceSettings->BehaviorFlags |= D3DCREATE_MULTITHREADED;

	// Does user want SWVP?
	if(RenderDevice::Instance()->m_bUseSWVP){
		pDeviceSettings->BehaviorFlags &= ~D3DCREATE_HARDWARE_VERTEXPROCESSING;
        pDeviceSettings->BehaviorFlags &= ~D3DCREATE_PUREDEVICE;                            
        pDeviceSettings->BehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}
}


//--------------------------------------------------------------------------------------
// One-time init, called by engine
//--------------------------------------------------------------------------------------
bool RenderDevice::Initialize(HINSTANCE hInst, HWND hWnd, string cmdLine)
{
	// Set the callback functions. These functions allow the framework to notify
    // the application about device changes, user input, and windows messages.  The 
    // callbacks are optional so you need only set callbacks for events you're interested 
    // in. However, if you don't handle the device reset/lost callbacks then the 
    // framework won't be able to reset your device since the application must first 
    // release all device resources before resetting.  Likewise, if you don't handle the 
    // device created/destroyed callbacks then the sample framework won't be able to 
    // recreate your device resources.
	DXUTSetCallbackDeviceCreated( ::OnCreateDevice );
	DXUTSetCallbackDeviceReset( ::OnResetDevice );
	DXUTSetCallbackDeviceLost( ::OnLostDevice );
	DXUTSetCallbackDeviceDestroyed( ::OnDestroyDevice );
    //DXUTSetCallbackMsgProc( MsgProc );
    //DXUTSetCallbackKeyboard( KeyboardProc );
    DXUTSetCallbackFrameRender( OnFrameRender );
    DXUTSetCallbackFrameMove( OnFrameMove );

	// Let game configure the settings GUI
//    CD3DSettingsDlg* pSettingsDialog = DXUTGetSettingsDialog();
//    pSettingsDialog->SetCallbackModify( m_SettingsUI );
    // Show the cursor and clip it when in full screen
    DXUTSetCursorSettings( true, true );

	// Initialize the sample framework and create the desired Win32 window and Direct3D 
    // device for the application. Calling each of these functions is optional, but they
    // allow you to set several options which control the behavior of the framework.
    DXUTInit( true, false, false ); // Parse the command line, don't handle the default hotkeys, and don't show msgboxes
    DXUTSetWindow( hWnd, hWnd, hWnd, false );

     //
    // Initialize config information for all available 3D devices after DXUTInit is called
    // because we would like to do this as soon as we have a D3D object.
    IDirect3D9 *pD3D = DXUTGetD3DObject();
    assert( pD3D != NULL );

     // Allocate the CM array to hold all devices on the system.
    g_pCMList = new CConfigManager[pD3D->GetAdapterCount()];

    // Obtain sound information (device GUID)
    GUID guidDeviceId;
    GetDeviceID( &DSDEVID_DefaultPlayback, &guidDeviceId );

    // Initialize CM objects, one for each 3D device
    for( DWORD dev = 0; dev < pD3D->GetAdapterCount(); ++dev )
    {
        D3DADAPTER_IDENTIFIER9 id;
        D3DCAPS9 Caps;
        pD3D->GetAdapterIdentifier( dev, 0, &id );
        pD3D->GetDeviceCaps( dev, D3DDEVTYPE_HAL, &Caps );
        g_pCMList[dev].Initialize( L"DeviceConfigs.ini", id, Caps, guidDeviceId );
        // Propagate safe mode flag
        g_pCMList[dev].cf_SafeMode = Engine::Instance()->SafeMode;
    }

     // Make ConfigManager point to the first element in g_pCMList by default.
     // OnCreateDevice will make it point to the correct element later.
    Engine::Instance()->ConfigManager = g_pCMList;

	LoadConfig();

	// Command-line overrides
	if(cmdLine.find("-hdr") != -1)
	{
		HDRSystem::Instance()->m_bEnabled = true;
	}

	// FIXME: Should be in LoadConfig, but we need some of these vars here only
	ConfigFile* Config = Engine::Instance()->MainConfig;
	bool FullScreen		= Config->GetBool("FullScreen");
	int  SizeX			= Config->GetInt("FullScreenWidth");
	int  SizeY			= Config->GetInt("FullScreenHeight");
	int  ColorDepth		= Config->GetInt("FullScreenColorDepth");
	bool VSync			= Config->GetBool("FullScreenVSync");

	if(Editor::Instance()->GetEditorMode()){
		FullScreen = false; // Editor never uses fullscreen
	}

	// Resize window to size requested in config (if size wasn't 0,0)
	if(!FullScreen){
		SizeX = Config->GetInt("WindowedWidth");
		SizeY = Config->GetInt("WindowedHeight");
		 // If 0,0, or in editor mode, use window default size
		if((!SizeX && !SizeY) || Editor::Instance()->GetEditorMode()){
			RECT rect;
			//GetWindowRect(hWnd,&rect);
			// RECT rcWindowClient;
			GetClientRect( hWnd, &rect );

			SizeX = rect.right  - rect.left;
			SizeY = rect.bottom - rect.top;
			//SetWindowPos (hWnd, NULL, 0, 0, width, height, SWP_NOZORDER | SWP_NOMOVE);
		}
	}

	LogPrintf("\tRenderDevice: Creating D3D Device");
	HRESULT hr = DXUTCreateDevice( D3DADAPTER_DEFAULT, !FullScreen, SizeX, SizeY, IsDeviceAcceptable, ModifyDeviceSettings );
	if(FAILED(hr))
	{
		if(hr == D3DERR_OUTOFVIDEOMEMORY)
			Error("CreateDevice failed because you are out of available video memory.\nClose any other 3D applications or editors running, check in the task list for an old instance of the game running\nand if all else fails restart. \nIf you persistently see this error you may not have enough available video memory on your card, \nin which case you should edit the config to disable HDR and lower the screen resolution");
		else
			Error("CreateDevice failed. Checklist:\n1. If you have any tweakers or overclocking tools installed, like Rage3D please remove them!"
			"\n2. Try setting HDREnabled to false in the config. Older cards can have problems with this\n3. Get the latest video card drivers from www.ati.com or www.nvidia.com\n4."
			"Make sure you have the latest DirectX (9.0c) from www.microsoft.com/directx\n5. No luck? E-mail tim@artificialstudios.com incluing the below error code,"
			" which may give clues to the problem.\n Error code was %s",DXGetErrorString9(hr));
	}

    VertexFormats::Instance()->Initialize();
	// Make it initialize
	DXUTSetShowSettingsDialog(true);
	DXUTSetShowSettingsDialog(false);
	return SUCCEEDED(hr);
}

//--------------------------------------------------------------------------------------
// Load config into device
//--------------------------------------------------------------------------------------
void RenderDevice::LoadConfig(){
	ConfigFile* Config = Engine::Instance()->MainConfig;
	// Load everything from the Config->
	m_bIgnoreGamma		= Config->GetBool("IgnoreGamma");
	m_OcclusionTesting	= Config->GetBool("OcclusionTesting");
	ClearScreen			= Config->GetBool("ClearScreen");
	CompressScreenshots	= Config->GetBool("CompressScreenshots");
	ClearColor			= Config->GetColor("ClearColor");
	Gamma				= Config->GetFloat("Gamma");
	UseSRGB				= Config->GetBool("EnableSRGB");
	Brightness			= Config->GetFloat("Brightness");
	Contrast			= Config->GetFloat("Contrast");
	AnisotropyLevel		= Config->GetInt("AnisotropyLevel");
	dynamicLights		= Config->GetBool("DynamicLights");
	m_bUseSWVP			= Config->GetBool("UseSWVP");
	m_ShadowMapScale	= Config->GetFloat("ShadowMapScale");
    m_bShadows          = Config->GetBool("RenderShadows");
	TextureManager::FrameTime = Config->GetFloat("AnimationFrameTime");
    m_bCompressPRT      = Config->GetBool("CompressPRTMaps");
	HDRSystem::Instance()->m_fKeyValue	= Config->GetFloat("HDRExposure");
	HDRSystem::Instance()->m_fMinLum	= Config->GetFloat("HDRMinLum");
	HDRSystem::Instance()->m_fMaxLum	= Config->GetFloat("HDRMaxLum");
	HDRSystem::Instance()->m_bEnabled	= Config->GetBool("HDREnabled");
	HDRSystem::Instance()->m_bToneMap	= Config->GetBool("ToneMapping");
	HDRSystem::Instance()->m_bBlueShift	= Config->GetBool("BlueShift");
	HDRSystem::Instance()->m_fBlueShift	= Config->GetFloat("BlueShiftCoef");

//	BatchRenderer::Instance()->m_bZPass = Config->GetBool("DoZPass");
	m_bHardwareSync = Config->GetBool("ReduceInputLag");

	if(Engine::Instance()->MainConfig->GetString("PSEmulationVersion").find("NONE") == -1)
		if(Engine::Instance()->MainConfig->GetString("PSEmulationVersion").find("ps_1_") != -1)
			HDRSystem::Instance()->m_bEnabled = false;

	TextureManager::CompressNormalMaps = Config->GetBool("CompressNormalMaps");
	TextureManager::TextureSizePercent = Config->GetInt("TextureSizePercent");
}

//--------------------------------------------------------------------------------------
// Save config from device
//--------------------------------------------------------------------------------------
void RenderDevice::SaveConfig(){
	ConfigFile* cfg = Engine::Instance()->MainConfig;
	// Don't save fullscreen flag in editor mode (always false)
	if(!Editor::Instance()->GetEditorMode()) 
		cfg->SetBool("FullScreen",IsFullscreen());
	cfg->SetBool("DynamicLights",GetDynamicLights());
	cfg->SetFloat("Gamma",Gamma);
	cfg->SetFloat("Brightness",Brightness);
	cfg->SetFloat("Contrast",Contrast);
	cfg->SetInt("AnisotropyLevel",AnisotropyLevel);
	cfg->SetBool("VSync",GetVSync());
	cfg->SetBool("EnableSRGB",UseSRGB);
	cfg->SetFloat("HDRExposure",HDRSystem::Instance()->m_fKeyValue);
	cfg->SetFloat("HDRMinLum",HDRSystem::Instance()->m_fMinLum);
	cfg->SetFloat("HDRMaxLum",HDRSystem::Instance()->m_fMaxLum);
	cfg->SetBool("ToneMapping",HDRSystem::Instance()->m_bToneMap);
	cfg->SetBool("BlueShift",HDRSystem::Instance()->m_bBlueShift);
	cfg->SetFloat("BlueShiftCoef",HDRSystem::Instance()->m_fBlueShift);
	cfg->SetFloat("ShadowMapScale",m_ShadowMapScale);
    cfg->SetBool("CompressPRTMaps",m_bCompressPRT);
    cfg->SetBool("RenderShadows",m_bShadows);
	
	cfg->SetBool("OcclusionTesting",m_OcclusionTesting);

	if(IsFullscreen()){
		cfg->SetInt("FullScreenWidth",GetViewportX());
		cfg->SetInt("FullScreenHeight",GetViewportY());
		cfg->SetInt("FullScreenColorDepth",GetViewportColorDepth());
	}
}

//--------------------------------------------------------------------------------------
// When the game closes
//--------------------------------------------------------------------------------------
void RenderDevice::Shutdown(){
	m_bDeviceLost = true;
	Sleep(50); // Let any threads get this new device state

	// Save the current video settings to file
	SaveConfig();
	RenderDevice::Instance()->GetCanvas()->Cleanup();
	ShaderManager::Instance()->Shutdown();
	MaterialManager::Instance()->Shutdown();

	CTextureRenderer::CleanupDShow();
	DXUTShutdown();
    VertexFormats::Instance()->Shutdown();
	LogPrintf("Shutdown()");
}

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
HRESULT RenderDevice::EndHDR()
{
	// Post-process scene, add bloom, etc
	if(HDRSystem::Instance()->m_bEnabled){
		StartMiniTimer();

		LPDIRECT3DSTATEBLOCK9 states2;
		RenderWrap::dev->CreateStateBlock(D3DSBT_ALL,&states2);
		states2->Capture();

		states->Apply();
		HDRSystem::Instance()->PostRender();
		// Do LDR stuff like occlusion tests now, because it's cheaper on LDR surface, and there were bugs with HDR tests
		BatchRenderer::Instance()->RenderLDR();
        Profiler::Get()->RenderPostMS += StopMiniTimer();

		states2->Apply();
		SAFE_RELEASE(states2);
	}

	BatchRenderer::Instance()->ClearAll();

	return S_OK;
} 

//-----------------------------------------------------------------------------
// Name: GetVideoModes()
// Desc: List of video modes for application selection boxes
//-----------------------------------------------------------------------------
vector<DeviceMode> RenderDevice::GetDeviceModes(){
	vector<DeviceMode> modes;
	 // Working variables
	CD3DEnumAdapterInfo* pAdapter = DXUTGetEnumeration()->GetAdapterInfo(DXUTGetDeviceSettings().AdapterOrdinal);
	
	 // Add a list of modes (res x width x height )
	for(int m=0; m<pAdapter->displayModeList.GetSize(); m++ )
	{
		D3DDISPLAYMODE* mode = (D3DDISPLAYMODE*)&pAdapter->displayModeList.GetAt(m);
		
		DeviceMode Mode;
		// Add Resolution
		Mode.SizeX = mode->Width;
		Mode.SizeY = mode->Height;

		// Make sure it's unique
		bool copy = false;
		for(int i=0;i<modes.size();i++){
			if(modes[i].SizeY == Mode.SizeY
				&& modes[i].SizeX == Mode.SizeX)
				copy = true;
		}
		if(!copy)
			modes.push_back(Mode);
	}

	return modes;
}

//--------------------------------------------------------------------------------------
// Set Gamma 2.2 usage
//--------------------------------------------------------------------------------------
void RenderDevice::SetSRGB(bool sRGB){
	UseSRGB = sRGB;
	RenderWrap::dev->SetRenderState(D3DRS_SRGBWRITEENABLE, sRGB);

	for(int i=0;i<DXUTGetDeviceCaps()->MaxSimultaneousTextures;i++){
		RenderWrap::dev->SetSamplerState( i, D3DSAMP_SRGBTEXTURE,  sRGB);// Default, effects override!sRGB );
	}

	ShaderManager::Instance()->UpdateRenderSettings();
}

//--------------------------------------------------------------------------------------
// Sets the stats for the device, done at initdevobjects so every class has access to the device info
// as soon as it's set
//--------------------------------------------------------------------------------------
void RenderDevice::SetDeviceStats(){
	if(!(DXUTGetDeviceCaps()->TextureCaps & D3DPTEXTURECAPS_CUBEMAP)){
		Error("Sorry, but your card is below the minimum required specifications. You really should upgrade.");
	}

	// Output some device stats
	#define STAT_OUT(b,x) if(b) LogPrintf((sup + x).c_str()); else LogPrintf((nosup + x).c_str())

	// Get original gamma ramp
	HDC hDC;
	hDC = GetDC(NULL);
	GetDeviceGammaRamp(GetDC(NULL),&originalGamma);
	ReleaseDC(NULL, hDC);

	LogPrintf("SetDeviceStats()");
	string sup = "Supports ";
	string nosup = "Does not support ";

	STAT_OUT((DXUTGetDeviceCaps()->RasterCaps & D3DPRASTERCAPS_ANISOTROPY),"Anisotropic filtering");
	STAT_OUT((DXUTGetDeviceCaps()->TextureCaps & D3DPTEXTURECAPS_POW2),"Power-of-2 textures");
	STAT_OUT((DXUTGetDeviceCaps()->TextureCaps & D3DPTEXTURECAPS_VOLUMEMAP_POW2),"3D textures");

	// Get Pixel/Vertex shader versions
	PixelShaderVersion = D3DSHADER_VERSION_MAJOR(DXUTGetDeviceCaps()->PixelShaderVersion);
	PixelShaderVersion +=  D3DSHADER_VERSION_MINOR(DXUTGetDeviceCaps()->PixelShaderVersion)/10.f;
	VertexShaderVersion =  D3DSHADER_VERSION_MAJOR(DXUTGetDeviceCaps()->VertexShaderVersion);
	VertexShaderVersion		  += D3DSHADER_VERSION_MINOR(DXUTGetDeviceCaps()->VertexShaderVersion)/10.f;

	PixelShaderString  = D3DXGetPixelShaderProfile(RenderWrap::dev);
	VertexShaderString = D3DXGetVertexShaderProfile(RenderWrap::dev);

	if(PixelShaderString == "ps_2_0")  
	{  
		//check for the ps/vs2.0 variants based on a unique quality, how many ps instructions they support  
		if(DXUTGetDeviceCaps()->PS20Caps.NumInstructionSlots >= 1024)  
		{  
			PixelShaderString = "ps_2_a";  
			VertexShaderString = "vs_2_a";  
		}  
		else if(DXUTGetDeviceCaps()->PS20Caps.NumInstructionSlots >= 512)   
		{  
			PixelShaderString = "ps_2_b";  
			VertexShaderString = "vs_2_a";  
		}  
	}

	// Are we debugging with a fake shader version?
	string pEmulation = Engine::Instance()->MainConfig->GetString("PSEmulationVersion");
	if(pEmulation.find("NONE") == -1)
	{
		PixelShaderString = pEmulation;
		if(pEmulation.find("ps_1_") != -1)
			PixelShaderVersion = 1.1;
		if(pEmulation.find("ps_2_") != -1)
			PixelShaderVersion = 2.0;
		if(pEmulation.find("ps_3_") != -1)
			PixelShaderVersion = 3.0;
	}

	string vEmulation = Engine::Instance()->MainConfig->GetString("VSEmulationVersion");
	if(vEmulation.find("NONE") == -1)
	{
		VertexShaderString = vEmulation;
		if(vEmulation.find("vs_1_") != -1)
			VertexShaderVersion = 1.1;
		if(vEmulation.find("vs_2_") != -1)
			VertexShaderVersion = 2.0;
		if(vEmulation.find("vs_3_") != -1)
			VertexShaderVersion = 3.0;
	}

	// macro strings shouldn't go below ps 2.0, since they're used only for compiler variance among ps2.0 shaders
	// so that ps2.0 or greater cards can compile ideally optimized shaders. ps/vs 1.1 won't even use the system.
	if(PixelShaderVersion < 2.0)
		PixelShaderString = "ps_2_0";

	if(VertexShaderVersion < 2.0)
		VertexShaderString = "vs_2_0";

	// Output support
	if(PixelShaderVersion) 
		LogPrintf("Supports pixel shaders %f",PixelShaderVersion);
	else 
		LogPrintf("Does not support pixel shaders! Falling back to fixed function pipeline");
	if(VertexShaderVersion) 
		LogPrintf("Supports vertex shaders %f",VertexShaderVersion);
	else 
		LogPrintf("Does not support vertex shaders! Shaders will run in software");

	FFP =  (PixelShaderVersion < 1.1);

	// Check to see if device supports visibility query
	LPDIRECT3DQUERY9 query = NULL;
	if (D3DERR_NOTAVAILABLE == RenderWrap::dev->CreateQuery (D3DQUERYTYPE_OCCLUSION, &query))
	{
		m_OcclusionTesting = FALSE;
		LogPrintf("Warning: Occlusion queries are NOT supported. Disabling occlusion optimizations.");
	}
	SAFE_RELEASE(query);

		// Check for SRGB support
	HRESULT hr= DXUTGetD3DObject()->CheckDeviceFormat(DXUTGetDeviceCaps()->AdapterOrdinal, DXUTGetDeviceCaps()->DeviceType, 
		DXUTGetDeviceSettings().AdapterFormat,D3DUSAGE_RENDERTARGET|D3DUSAGE_QUERY_SRGBWRITE,D3DRTYPE_TEXTURE ,DXUTGetDeviceSettings().pp.BackBufferFormat);
	if(FAILED(hr)){
		UseSRGB = false;
		LogPrintf("Info: SRGB gamma correction is not supported");
	}
	D3DADAPTER_IDENTIFIER9 adapter = DXUTGetEnumeration()->GetAdapterInfo(DXUTGetDeviceSettings().AdapterOrdinal)->AdapterIdentifier;
	DWORD Product	 = HIWORD(adapter.DriverVersion.HighPart);
	DWORD Version	 = LOWORD(adapter.DriverVersion.HighPart);
	DWORD SubVersion = HIWORD(adapter.DriverVersion.LowPart);
	DWORD Build		 = LOWORD(adapter.DriverVersion.LowPart);
	LogPrintf("Driver is %s %s, version %d.%d.%d.%d",adapter.Description,adapter.Driver,Product,Version,SubVersion,Build);
	LogPrintf("Display mode is: %dx%d",GetViewportX(),GetViewportY());
	LogPrintf("Backbuffer is: %s", ToAnsi(DXUTD3DFormatToString(DXUTGetDeviceSettings().pp.BackBufferFormat,true)).c_str());
    LogPrintf("DepthStencil is: %d %s",(int)DXUTGetDeviceSettings().pp.EnableAutoDepthStencil, ToAnsi(DXUTD3DFormatToString(DXUTGetDeviceSettings().pp.AutoDepthStencilFormat,true)).c_str());
    LogPrintf("PresentInterval is %d SwapEffect is %d", (int)DXUTGetDeviceSettings().pp.PresentationInterval,(int)DXUTGetDeviceSettings().pp.SwapEffect);
    LogPrintf("Display is: %s", ToAnsi(DXUTD3DFormatToString(DXUTGetDeviceSettings().AdapterFormat,true)).c_str());
	LogPrintf("Max texture size: %dx%d",DXUTGetDeviceCaps()->MaxTextureWidth,DXUTGetDeviceCaps()->MaxTextureHeight);
	LogPrintf("Device is: %s",GetDeviceString().c_str());
}

//--------------------------------------------------------------------------------------
// Let the device process some windows messages itself. For example resize
//--------------------------------------------------------------------------------------
LRESULT RenderDevice::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ){
	return DXUTStaticWndProc(hWnd,uMsg,wParam,lParam);
}

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
bool RenderDevice::SetDisplayMode(int NewSizeX, int NewSizeY, int NewColorDepth, bool NewFullScreen, bool NewVSync){
	if(NewSizeX == GetViewportX() && NewSizeY == GetViewportY() && NewColorDepth == GetViewportColorDepth() && NewFullScreen == IsFullscreen() && NewVSync == GetVSync())
		return false;

	// TODO: Steal from DXUTSettings
	DXUTDeviceSettings settings = DXUTGetDeviceSettings();
	return SUCCEEDED( DXUTCreateDeviceFromSettings(&settings,FALSE) );
}


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void RenderDevice::SetAnisotropyLevel(int level){
	AnisotropyLevel = level;

	for(int i = 0;i<DXUTGetDeviceCaps()->MaxSimultaneousTextures; i++){
		//RenderWrap::dev->SetSamplerState( i, D3DSAMP_DMAPOFFSET, 0x100);

		if(level > 1&&(DXUTGetDeviceCaps()->RasterCaps & D3DPRASTERCAPS_ANISOTROPY)){
			AnisotropyLevel = (DXUTGetDeviceCaps()->MaxAnisotropy>level?level:DXUTGetDeviceCaps()->MaxAnisotropy);
			D3DTextureFilter = D3DTEXF_ANISOTROPIC;
			RenderWrap::dev->SetSamplerState( i, D3DSAMP_MAXANISOTROPY  , AnisotropyLevel);
			RenderWrap::dev->SetSamplerState( i, D3DSAMP_MAGFILTER ,     D3DTEXF_LINEAR     );
			RenderWrap::dev->SetSamplerState( i, D3DSAMP_MINFILTER,      D3DTEXF_ANISOTROPIC     );
			RenderWrap::dev->SetSamplerState( i, D3DSAMP_MIPFILTER ,	 D3DTEXF_LINEAR   );
		}
		else{
			AnisotropyLevel = 1;
			D3DTextureFilter = D3DTEXF_LINEAR;
			RenderWrap::dev->SetSamplerState( i, D3DSAMP_MAXANISOTROPY , AnisotropyLevel );
			RenderWrap::dev->SetSamplerState( i, D3DSAMP_MAGFILTER ,	 D3DTEXF_LINEAR   );
			RenderWrap::dev->SetSamplerState( i, D3DSAMP_MINFILTER,   D3DTEXF_LINEAR   );
			RenderWrap::dev->SetSamplerState( i, D3DSAMP_MIPFILTER ,	 D3DTEXF_LINEAR	  );
		}
	}

	ShaderManager::Instance()->UpdateRenderSettings();
}

//--------------------------------------------------------------------------------------
/* Faster, less accurate rounding */
//--------------------------------------------------------------------------------------
double fast_round(double dbl)
{
  return double((long)(dbl+0.5));
}

//--------------------------------------------------------------------------------------
// 0.5 is normal. values from 0 to 1
//--------------------------------------------------------------------------------------
void RenderDevice::SetGammaLevel(bool Linear, float contrast, float brightness, float gamma)
{
	if(m_bIgnoreGamma)
		return;
	Gamma = gamma;
	Contrast = contrast;
	Brightness = brightness;
	D3DGAMMARAMP GammaRamp;


	if(!Linear){
		//
		// Get these code from shaderX book.
		// contrast,brightness,gamma are float values.
		//
		for(int i=0; i<256; ++i)
		{
			int x=fast_round((contrast+0.5f)*pow(i/255.f,1.0f/gamma)*65535.f +
				(brightness-0.5f)*32768.f -
				contrast*32768.f+16384.f);
			// clamp to (0,65535)
			WORD val=x<0 ? 0 : x<65535 ? x : 65535;

			GammaRamp.red[i] = val;
			GammaRamp.green[i] = val;
			GammaRamp.blue[i] = val;
		}
	}
	else{
		float brightness = pow(2.0, (0.5 - Gamma) * 4.0);
		// Set up gamma ramps
		for (int i=0; i<256; i++)
		{
			if(!UseSRGB){
				int bright = (int)(pow((float)(i / 255.0), brightness) * 65535.0);
				GammaRamp.red[i] = GammaRamp.green[i] = GammaRamp.blue[i] = bright;
				// GammaRamp.red[i] = GammaRamp.green[i] = GammaRamp.blue[i]    = (WORD) (pow ( ((float) i) / 255.0f, 0.4545f) * 65535);
			}
			else
				GammaRamp.red[i] = GammaRamp.green[i] = GammaRamp.blue[i] = (WORD) ((((float) i) / 255.0f) * 65535);
		}
	}


	// This simple Win32 API call works when not in exclusive mode, but doesn't
	// work on all hardware.  Use this when in windowed mode.
	if(!IsFullscreen())
	{
		HDC hDC;
		hDC = GetDC(NULL);
		SetDeviceGammaRamp(hDC, &GammaRamp);
		ReleaseDC(NULL, hDC);
	}
	else
		// This is the proper way to do it, using DirectDraw.  Works on more
		// hardware than the above but requires exclusive mode.
		RenderWrap::dev->SetGammaRamp(0,D3DSGR_CALIBRATE,&GammaRamp);
}


//--------------------------------------------------------------------------------------
//
// Misc Functions...
//
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// FIXME: Can't change HDR at runtime because of the one-time fp init
//--------------------------------------------------------------------------------------
void RenderDevice::SetHDR(bool enable){
	/*if(enable && !m_HDREnabled){
		// If not initialized yet, do so now
		if(!HDRSystem::Instance()->IsReady()){
			HDRSystem::Instance()->InitDeviceObjects();
			HDRSystem::Instance()->RestoreDeviceObjects();
		}
	}*/
	// Just set the config, as we can't really set the value at runtime
	Engine::Instance()->MainConfig->SetBool("HDREnabled",enable);
	//HDRSystem::Instance()->m_bEnabled = enable;
	//m_HDREnabled					  = enable;
}


//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
bool RenderDevice::GetHDR(){
	return HDRSystem::Instance()->m_bEnabled;
}

//-----------------------------------------------------------------------------
// Exposure Key
//-----------------------------------------------------------------------------
void RenderDevice::SetHDRExposure(float exposure){
	HDRSystem::Instance()->m_fKeyValue = exposure;
}

//-----------------------------------------------------------------------------
// Exposure Key
//-----------------------------------------------------------------------------
float RenderDevice::GetHDRExposure(){
	return HDRSystem::Instance()->m_fKeyValue;
}

//-----------------------------------------------------------------------------
// Tone Mapping
//-----------------------------------------------------------------------------
void RenderDevice::SetToneMapping(bool toneMap){
	HDRSystem::Instance()->m_bToneMap = toneMap;
}

//-----------------------------------------------------------------------------
// Tone Mapping
//-----------------------------------------------------------------------------
bool RenderDevice::GetToneMapping(){
	return HDRSystem::Instance()->m_bToneMap;
}

//-----------------------------------------------------------------------------
// Blue-Shift
//-----------------------------------------------------------------------------
void RenderDevice::SetBlueShift(bool blueShift){
	HDRSystem::Instance()->m_bBlueShift = blueShift;
}

//-----------------------------------------------------------------------------
// Blue-Shift
//-----------------------------------------------------------------------------
bool RenderDevice::GetBlueShift(){
	return HDRSystem::Instance()->m_bBlueShift;
}

//-----------------------------------------------------------------------------
// Blue-Shift Coefficient
//-----------------------------------------------------------------------------
void RenderDevice::SetBlueShiftCoefficient(float blueShift){
	HDRSystem::Instance()->m_fBlueShift = blueShift;
}

//-----------------------------------------------------------------------------
// Blue-Shift Coefficient
//-----------------------------------------------------------------------------
float RenderDevice::GetBlueShiftCoefficient(){
	return HDRSystem::Instance()->m_fBlueShift;
}

//-----------------------------------------------------------------------------
// Min Lum
//-----------------------------------------------------------------------------
void RenderDevice::SetMinLuminance(float min){
	HDRSystem::Instance()->m_fMinLum = min;
}

//-----------------------------------------------------------------------------
// Min Lum
//-----------------------------------------------------------------------------
float RenderDevice::GetMinLuminance(){
	return HDRSystem::Instance()->m_fMinLum;
}

//-----------------------------------------------------------------------------
// Max Lum
//-----------------------------------------------------------------------------
void RenderDevice::SetMaxLuminance(float max){
	HDRSystem::Instance()->m_fMaxLum = max;
}

//-----------------------------------------------------------------------------
// Max Lum
//-----------------------------------------------------------------------------
float RenderDevice::GetMaxLuminance(){
	return HDRSystem::Instance()->m_fMaxLum;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void RenderDevice::ReloadShaders(){
	ShaderManager::Instance()->ReloadShaders();
}


int RenderDevice::CountUsedTextureMB(){
	//UINT i = RenderWrap::dev->GetAvailableTextureMem();
	return ((TextureManager::textureMemory/1024)/1024);
}

// Returns a singleton instance
RenderDevice* RenderDevice::Instance () 
{
    static RenderDevice inst;
    return &inst;
}

void RenderDevice::SetWireframe(bool wireframe){
	RenderWrap::dev->SetRenderState(D3DRS_FILLMODE,wireframe?D3DFILL_WIREFRAME:D3DFILL_SOLID);
}

bool RenderDevice::GetCompressNormalMaps() { return TextureManager::CompressNormalMaps; }
void RenderDevice::SetCompressNormalMaps(bool enable){ TextureManager::CompressNormalMaps = enable; }

int  RenderDevice::GetTextureSizePercent() { return TextureManager::TextureSizePercent; }
void RenderDevice::SetTextureSizePercent(int percent){ 
	TextureManager::TextureSizePercent = percent; 
	TextureManager::UpdateAllLODs();
}


//--------------------------------------------------------------------------------------
// Snaps a shot based on a File Name and Path
//	NOTE: Do not include .bmp with string in
//
// This is just for thumbnails
//--------------------------------------------------------------------------------------
void RenderDevice::SaveThumbnail(const char* file)
{
    string fileNameAndPath = file;
	ResetCurrentDirectory();
	LPDIRECT3DDEVICE9 pDev = RenderWrap::dev;

	HRESULT hr;

	// get display dimensions
	// this will be the dimensions of the front buffer
	D3DDISPLAYMODE mode;
	if (FAILED(hr=pDev->GetDisplayMode(0,&mode)))
		Error("GetDisplayMode() failed");

	// create the image surface to store the front buffer image
	// note that call to GetFrontBuffer will always convert format to A8R8G8B8
	LPDIRECT3DSURFACE9 surf;

	// read the front buffer into the image surface
	if (FAILED(hr=pDev->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&surf))) {
		surf->Release();
		SeriousWarning("GetBackBufferData() failed");
        return;
    }

    fileNameAndPath.append(".bmp");

    // Temporarily save buffer. We must do this because I can't figure out how to read
    // the buffer into another surface!
    DXASSERT(D3DXSaveSurfaceToFile(fileNameAndPath.c_str(),D3DXIFF_BMP,surf,NULL,NULL));
   
     // New scaled surface for thumbnail
    LPDIRECT3DSURFACE9 surf2;
	DXASSERT(pDev->CreateOffscreenPlainSurface(140,105,D3DFMT_A8R8G8B8,D3DPOOL_SYSTEMMEM ,&surf2,0));
    DXASSERT(D3DXLoadSurfaceFromFile(surf2,0,0,fileNameAndPath.c_str(),0,D3DX_DEFAULT,NULL,0));
	DXASSERT(D3DXSaveSurfaceToFile(fileNameAndPath.c_str(),D3DXIFF_BMP,surf2,NULL,NULL));

	// release the image surface
	SAFE_RELEASE(surf);
    SAFE_RELEASE(surf2);
}



void RenderDevice::TakeScreenshot(){
	ResetCurrentDirectory();
	LPDIRECT3DDEVICE9 pDev = RenderWrap::dev;

	HRESULT hr;

	// get display dimensions
	// this will be the dimensions of the front buffer
	D3DDISPLAYMODE mode;
	if (FAILED(hr=pDev->GetDisplayMode(0,&mode)))
		Error("GetDisplayMode() failed");

	// create the image surface to store the front buffer image
	// note that call to GetFrontBuffer will always convert format to A8R8G8B8
	LPDIRECT3DSURFACE9 surf;

	// read the front buffer into the image surface
	if (FAILED(hr=pDev->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&surf))) {
		surf->Release();
		SeriousWarning("GetBackBufferData() failed");
        return;
    }

	// Get screenshot directory
	string shotPath = Engine::Instance()->MainConfig->GetString("SearchPath") + "\\Screenshots\\";
	// Make sure directory exists
	CreateDirectory(shotPath.c_str(),NULL);
	// write the entire surface to the requested file
	char fileName[512];
	int i = 0;
	do{
		if(CompressScreenshots)
			sprintf(fileName,"%s\\ScreenShot%d.jpg",shotPath.c_str(),++i);
		else
			sprintf(fileName,"%s\\ScreenShot%d.bmp",shotPath.c_str(),++i);
	}
	while(FileExists(fileName));

	if(CompressScreenshots)
	{
		DXASSERT(D3DXSaveSurfaceToFile(fileName,D3DXIFF_JPG,surf,NULL,0));
	}
	else
	{
		DXASSERT(D3DXSaveSurfaceToFile(fileName,D3DXIFF_BMP,surf,NULL,0));
	}

	LogPrintf("Screenshot saved as %s",fileName);

	// release the image surface
	surf->Release();
}


bool RenderDevice::IsFullscreen(){
	return !DXUTIsWindowed();
}

bool RenderDevice::GetVSync(){
	return m_bUseVSync;
}

//-----------------------------------------------------------------------------
// TIM: Commented out conditionals, as there appear to be fringe cases
// where we need to set both
//-----------------------------------------------------------------------------
void RenderDevice::ShowCursor(bool bShow, bool CenterCursor){
	//if(m_bWindowed){
		if(!bShow)
			while(::ShowCursor(FALSE) >= 0);
		else
			while(::ShowCursor(TRUE) < 0);
	//}
	//else{
		RenderWrap::dev->ShowCursor( bShow );
	//}
	m_bCursorVisible = bShow;
	m_bCenterCursor  = CenterCursor;
}


bool RenderDevice::IsCursorVisible(){
	return m_bCursorVisible;
}

int RenderDevice::DynamicLightsDrawn(){
	// TODO: This
//	int num = Renderer::dynamicLightsDrawn;
//	Renderer::dynamicLightsDrawn = 0;
//	return num;
	return 0;
}

int RenderDevice::TrisPerFrame(){
	return m_Triangles;
}
int RenderDevice::VertsPerFrame(){
	return m_Vertices;
}
int RenderDevice::MeshesPerFrame(){
	return m_DrawCalls;
}

string RenderDevice::GetDeviceString(){
	if(!RenderWrap::dev)
		return "NOT LOADED";
	return ToAnsi(wstring(DXUTGetDeviceStats()));
}

bool RenderDevice::ReadyToRender(){ 
	// TODO: Me
	return true;//( !GetDXUTState().GetDeviceLost() && !GetDXUTState().GetRenderingPaused() && GetDXUTState().GetActive() );
}

//-----------------------------------------------------------------------------
// Name: CacheAllTextures
// Desc: Caches textures to video memory
//-----------------------------------------------------------------------------
void RenderDevice::PreCacheLoadedTextures(){
	return;
	for(int i=0;i<TextureManager::textures.size();i++){
		if(TextureManager::textures[i].texture->IsValid())
			TextureManager::textures[i].texture->GetTexture()->PreLoad();
	}
}


int RenderDevice::GetViewportX(){ return DXUTGetDeviceSettings().pp.BackBufferWidth; }
int RenderDevice::GetViewportY(){ return DXUTGetDeviceSettings().pp.BackBufferHeight; }
int RenderDevice::GetViewportColorDepth(){
	if( DXUTGetDeviceSettings().pp.BackBufferFormat == D3DFMT_X8R8G8B8 ||
		DXUTGetDeviceSettings().pp.BackBufferFormat == D3DFMT_A8R8G8B8 ||
		DXUTGetDeviceSettings().pp.BackBufferFormat == D3DFMT_R8G8B8 ||
		DXUTGetDeviceSettings().pp.BackBufferFormat == D3DFMT_A2R10G10B10)
	{
		return 32;
	}
	return 16;
}

Canvas* RenderDevice::GetCanvas(){
	return Canvas::Instance();
}




//--------------------------------------------------------------------------------------
// Just in case...
//--------------------------------------------------------------------------------------
void RenderDevice::SetDefaultStates()
{
	defaultStates->Apply();
}

void	RenderDevice::SetDropShadows(bool enable)
{
	ShadowEngine::Instance()->ShadowsEnabled = enable;
}
bool	RenderDevice::GetDropShadows()
{
return ShadowEngine::Instance()->ShadowsEnabled;
}