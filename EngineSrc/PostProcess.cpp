//=========== Copyright (c) 2004, Artificial Studios. All rights reserved. ===========
// Name: PostProcess
// Desc: Applies various pixel shaders "filters" to final scene render, before HDR is applied
//====================================================================================
#include "stdafx.h"
#include "PostProcess.h"
#include "HDR.h"
#include "dxstdafx.h"


//-----------------------------------------------------------------------------
// Desc: Returns a singleton instance
//-----------------------------------------------------------------------------
PostProcess* PostProcess::Instance () 
{
	static PostProcess inst;
	return &inst;
}

//-----------------------------------------------------------------------------
// Name: PostProcess()
// Desc: Application constructor. Sets attributes for the app.
//-----------------------------------------------------------------------------
PostProcess::PostProcess()
{
	m_pDownScaledScene8x[0] = NULL;
	m_pDownScaledScene8x[1] = NULL;
	m_pDownScaledScene4x[0] = NULL;
	m_pDownScaledScene4x[1] = NULL;
	m_pDownScaledScene2x[0] = NULL;
	m_pDownScaledScene2x[1] = NULL;
	m_CurDownScaledScene8x = 0;
	m_CurDownScaledScene4x = 0;
	m_CurDownScaledScene2x = 0;

	FXConfig = NULL;
	m_bUsingDOF = false;
}

//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has been 
// reset, which will happen after a lost device scenario. This is the best location to 
// create D3DPOOL_DEFAULT resources since these resources need to be reloaded whenever 
// the device is lost. Resources created here should be released in the OnLostDevice 
// callback. 
//--------------------------------------------------------------------------------------
HRESULT PostProcess::OnResetDevice()
{
	const D3DSURFACE_DESC* pBackBufferDesc = DXUTGetBackBufferSurfaceDesc();
	m_dwCropWidth = pBackBufferDesc->Width - pBackBufferDesc->Width % 8;
	m_dwCropHeight = pBackBufferDesc->Height - pBackBufferDesc->Height % 8;

	if(FXConfig)
		CreateFXFromConfigFile(*FXConfig);
	else
		PostProcessEffect_Permanent::ReloadPermanentEffects();

	return S_OK;
}

//--------------------------------------------------------------------------------------
// HDR-only effects
//--------------------------------------------------------------------------------------
void PostProcess::PostRenderHDR()
{
	if(HDRSystem::Instance()->m_bEnabled == false || !EffectsActive() || !m_bUsingDOF)
		return;

	PostRenderFunc();
}

bool DoneFullScaleEffect;

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void PostProcess::PostRenderFunc()
{
	DoneFullScaleEffect = false;

	LPDIRECT3DSTATEBLOCK9 states;
	RenderWrap::dev->CreateStateBlock(D3DSBT_ALL,&states);

	RenderDevice::Instance()->ResetAllStates();

	RenderWrap::SetRS( D3DRS_ALPHABLENDENABLE, FALSE );
	RenderWrap::SetRS( D3DRS_FOGENABLE, FALSE );

	RenderWrap::dev->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
	RenderWrap::dev->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );

	for(int i = 0; i < PostProcessEffects.size();i++)
	{
		if(PostProcessEffects[i]->Active)
			PostProcessEffects[i]->RenderEffect();
		else if(PostProcessEffects[i]->ActiveOneFrame)
		{
			PostProcessEffects[i]->ActiveOneFrame = false;
			PostProcessEffects[i]->RenderEffect();
		}
	}

	states->Apply();
	SAFE_RELEASE(states);
}
//--------------------------------------------------------------------------------------
// LDR version
//--------------------------------------------------------------------------------------
void PostProcess::PostRender()
{
	HDRSystem::Instance()->CopyToLDRTarget();

	if(!EffectsActive() || m_bUsingDOF)
		return;

	PostRenderFunc();
}
bool PostProcess::EffectsActive()
{ 
	for(int i=0;i<PostProcessEffects.size();i++)
		if(PostProcessEffects[i]->Active || PostProcessEffects[i]->ActiveOneFrame) 
			return true;


	return false;
}

//-----------------------------------------------------------------------------
// Name: DrawFullScreenQuad
// Desc: Draw a properly aligned quad covering the entire render target
//-----------------------------------------------------------------------------
void PostProcess::DrawFullScreenQuad(float fLeftU, float fTopV, float fRightU, float fBottomV)
{
	D3DSURFACE_DESC dtdsdRT;
	PDIRECT3DSURFACE9 pSurfRT;

	// Acquire render target width and height
	RenderWrap::dev->GetRenderTarget(0, &pSurfRT);
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

	RenderWrap::dev->SetRenderState(D3DRS_ZENABLE, FALSE);
	RenderWrap::dev->SetFVF(ScreenVertex::FVF);
	RenderWrap::dev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, svQuad, sizeof(ScreenVertex));
	RenderWrap::dev->SetRenderState(D3DRS_ZENABLE, TRUE);
}

//-----------------------------------------------------------------------------
// Loads effects from a config definition
//-----------------------------------------------------------------------------
void PostProcess::CreateFXFromConfigFile(ConfigFile& config)
{
	PostProcess::Instance()->RemoveAllEffects();

	if(RenderDevice::Instance()->PixelShaderVersion < 2)
		return;

	FXConfig = &config;

	for(int i = 0; i < 10; i++)
	{
		if(config.KeyExists("Effect" + ToStr(i),"PostProcessFX"))
		{
			string val = config.GetString("Effect" + ToStr(i),"PostProcessFX");
			string shader = val.substr(0,val.find(","));
			string technique = val.substr(val.find(",")+1);

			if(technique.find("DOF") == -1 || HDRSystem::Instance()->m_bEnabled)
				AddPostProcessEffect(shader,technique);
		}
	}

	PostProcessEffect_Permanent::ReloadPermanentEffects();
}

//-----------------------------------------------------------------------------
void PostProcess::OnDestroyDevice()
{
	RemoveAllEffects();
}
//-----------------------------------------------------------------------------
void PostProcess::OnLostDevice()
{
	RemoveAllEffects();
}

//-----------------------------------------------------------------------------
// Add an effect with specified parameters
//-----------------------------------------------------------------------------
PostProcessEffect* PostProcess::AddPostProcessEffect(string ShaderFile, string TechniqueName, bool StartActive , int OrderingPreference)
{
	PostProcessEffect* ppEffect = new PostProcessEffect(ShaderFile,TechniqueName);

	ppEffect->Active = StartActive;

	if(OrderingPreference == -1)
		PostProcessEffects.push_back(ppEffect);
	else
		PostProcessEffects.insert(PostProcessEffects.begin() + OrderingPreference,ppEffect);

	return ppEffect;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
PostProcessEffect* PostProcess::AddPostProcessEffect(PostProcessEffect* ppEffect,bool StartActive , int OrderingPreference)
{
	ppEffect->Active = StartActive;

	if(OrderingPreference == -1)
		PostProcessEffects.push_back(ppEffect);
	else
		PostProcessEffects.insert(PostProcessEffects.begin() + OrderingPreference,ppEffect);

	return ppEffect;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void PostProcess::RemovePostProcessEffect(PostProcessEffect* ppEffect)
{
	vector<PostProcessEffect*>::iterator location = find(PostProcessEffects.begin(),PostProcessEffects.end(),ppEffect);
	if(location != PostProcessEffects.end())
		PostProcessEffects.erase(location);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void PostProcess::RemoveAllEffects()
{
	PostProcess::Instance()->m_bUsingDOF = false;
	for(int i = 0; i < PostProcessEffects.size();i++)
	{
		delete PostProcessEffects[i];
	}
	PostProcessEffects.clear();

	m_CurDownScaledScene8x = 0;
	SAFE_RELEASE(m_pDownScaledScene8x[0]);
	SAFE_RELEASE(m_pDownScaledScene8x[1]);
	m_CurDownScaledScene4x = 0;
	SAFE_RELEASE(m_pDownScaledScene4x[0]);
	SAFE_RELEASE(m_pDownScaledScene4x[1]);
	m_CurDownScaledScene2x = 0;
	SAFE_RELEASE(m_pDownScaledScene2x[0]);
	SAFE_RELEASE(m_pDownScaledScene2x[1]);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
PostProcessEffect* PostProcess::FindPostProcessEffect(string ShaderFile, string TechniqueName)
{
	for(int i = 0; i < PostProcessEffects.size();i++)
	{
		if(PostProcessEffects[i]->shaderFile == ShaderFile && PostProcessEffects[i]->techniqueName == TechniqueName)
			return PostProcessEffects[i];
	}
	return 0;
}

D3DXVECTOR2 avSampleOffsets2X[16];
D3DXVECTOR2 avSampleOffsets4X[16];
D3DXVECTOR2 avSampleOffsets8X[16];
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
PostProcessEffect::PostProcessEffect(string ShaderFile, string TechniqueName)
{
	Active = false;
	ActiveOneFrame = false;
	Mat = 0;

	if(RenderDevice::Instance()->PixelShaderVersion < 2.0)
		return;

	D3DFORMAT textureFormat = D3DFMT_A8R8G8B8;

	if(TechniqueName.find("DOF") != -1)
	{
		PostProcess::Instance()->m_bUsingDOF = true;
		textureFormat = D3DFMT_A16B16G16R16F;
	}

	shaderFile = ShaderFile;
	techniqueName = TechniqueName;
	Mat = new Material(ShaderFile + TechniqueName);
	Mat->m_ID = 0;
	Mat->Initialize(ShaderFile.c_str(),TechniqueName.c_str());

	TechniqueHandle = Mat->m_Shader->GetTechnique(TechniqueName);
	m_DownScaled8xParam = Mat->m_Shader->GetEffect()->GetParameterByName(NULL,"tDownScale8x");
	m_DownScaled4xParam = Mat->m_Shader->GetEffect()->GetParameterByName(NULL,"tDownScale4x");
	m_DownScaled2xParam = Mat->m_Shader->GetEffect()->GetParameterByName(NULL,"tDownScale2x");

	const D3DSURFACE_DESC* pBackBufferDesc = DXUTGetBackBufferSurfaceDesc();

	if(TechniqueName.find("_2xRT") != -1)
	{	
		m_RTScale = .5f;

		//create any appropriate scaled RT's
		if(!PostProcess::Instance()->m_pDownScaledScene2x[0])
		{
			// 1/2x Scaled version of the scene texture
			RenderWrap::dev->CreateTexture(PostProcess::Instance()->m_dwCropWidth / 2, PostProcess::Instance()->m_dwCropHeight / 2, 
				1, D3DUSAGE_RENDERTARGET, 
				textureFormat, D3DPOOL_DEFAULT, 
				&PostProcess::Instance()->m_pDownScaledScene2x[0], NULL);

			RenderWrap::dev->CreateTexture(PostProcess::Instance()->m_dwCropWidth / 2, PostProcess::Instance()->m_dwCropHeight / 2, 
				1, D3DUSAGE_RENDERTARGET, 
				textureFormat, D3DPOOL_DEFAULT, 
				&PostProcess::Instance()->m_pDownScaledScene2x[1], NULL);
		}

		// Place the rectangle in the center of the back buffer surface
		RECT rectSrc;
		rectSrc.left = (pBackBufferDesc->Width - PostProcess::Instance()->m_dwCropWidth) / 2;
		rectSrc.top = (pBackBufferDesc->Height - PostProcess::Instance()->m_dwCropHeight) / 2;
		rectSrc.right = rectSrc.left + PostProcess::Instance()->m_dwCropWidth;
		rectSrc.bottom = rectSrc.top + PostProcess::Instance()->m_dwCropHeight;

		// Get the texture coordinates for the render target
		HDRSystem::Instance()->GetTextureCoords(HDRSystem::Instance()->GetCurrentBuffer(), &rectSrc, PostProcess::Instance()->GetCurrentDownScaled2xBuffer(), NULL, &m_CoordRect );

		// Get the sample offsets used within the pixel shader
		HDRSystem::Instance()->GetSampleOffsets_DownScale2x2( pBackBufferDesc->Width, pBackBufferDesc->Height, avSampleOffsets2X );
	}
	else if(TechniqueName.find("_4xRT") != -1)
	{
		m_RTScale = .25f;

		if(!PostProcess::Instance()->m_pDownScaledScene4x[0])
		{
			// 1/4x Scaled version of the scene texture
			RenderWrap::dev->CreateTexture(PostProcess::Instance()->m_dwCropWidth / 4, PostProcess::Instance()->m_dwCropHeight / 4, 
				1, D3DUSAGE_RENDERTARGET, 
				textureFormat, D3DPOOL_DEFAULT, 
				&PostProcess::Instance()->m_pDownScaledScene4x[0], NULL);

			RenderWrap::dev->CreateTexture(PostProcess::Instance()->m_dwCropWidth / 4, PostProcess::Instance()->m_dwCropHeight / 4, 
				1, D3DUSAGE_RENDERTARGET, 
				textureFormat, D3DPOOL_DEFAULT, 
				&PostProcess::Instance()->m_pDownScaledScene4x[1], NULL);
		}

		// Place the rectangle in the center of the back buffer surface
		RECT rectSrc;
		rectSrc.left = (pBackBufferDesc->Width - PostProcess::Instance()->m_dwCropWidth) / 4;
		rectSrc.top = (pBackBufferDesc->Height - PostProcess::Instance()->m_dwCropHeight) / 4;
		rectSrc.right = rectSrc.left + PostProcess::Instance()->m_dwCropWidth;
		rectSrc.bottom = rectSrc.top + PostProcess::Instance()->m_dwCropHeight;

		// Get the texture coordinates for the render target
		HDRSystem::Instance()->GetTextureCoords(HDRSystem::Instance()->GetCurrentBuffer(), &rectSrc, PostProcess::Instance()->GetCurrentDownScaled4xBuffer(), NULL, &m_CoordRect );

		// Get the sample offsets used within the pixel shader
		HDRSystem::Instance()->GetSampleOffsets_DownScale4x4( pBackBufferDesc->Width, pBackBufferDesc->Height, avSampleOffsets4X );
	}
	else if(TechniqueName.find("_8xRT") != -1)
	{
		m_RTScale = .125f;

		if(!PostProcess::Instance()->m_pDownScaledScene8x[0])
		{
			// 1/8x Scaled version of the scene texture
			RenderWrap::dev->CreateTexture(PostProcess::Instance()->m_dwCropWidth / 8, PostProcess::Instance()->m_dwCropHeight / 8, 
				1, D3DUSAGE_RENDERTARGET, 
				textureFormat, D3DPOOL_DEFAULT, 
				&PostProcess::Instance()->m_pDownScaledScene8x[0], NULL);

			RenderWrap::dev->CreateTexture(PostProcess::Instance()->m_dwCropWidth / 8, PostProcess::Instance()->m_dwCropHeight / 8, 
				1, D3DUSAGE_RENDERTARGET, 
				textureFormat, D3DPOOL_DEFAULT, 
				&PostProcess::Instance()->m_pDownScaledScene8x[1], NULL);
		}

		// Place the rectangle in the center of the back buffer surface
		RECT rectSrc;
		rectSrc.left = (pBackBufferDesc->Width - PostProcess::Instance()->m_dwCropWidth) / 8;
		rectSrc.top = (pBackBufferDesc->Height - PostProcess::Instance()->m_dwCropHeight) / 8;
		rectSrc.right = rectSrc.left + PostProcess::Instance()->m_dwCropWidth;
		rectSrc.bottom = rectSrc.top + PostProcess::Instance()->m_dwCropHeight;

		// Get the texture coordinates for the render target
		HDRSystem::Instance()->GetTextureCoords(HDRSystem::Instance()->GetCurrentBuffer(), &rectSrc, PostProcess::Instance()->GetCurrentDownScaled8xBuffer(), NULL, &m_CoordRect );

		// Get the sample offsets used within the pixel shader
		HDRSystem::Instance()->GetSampleOffsets_DownScale4x4( pBackBufferDesc->Width, pBackBufferDesc->Height, avSampleOffsets8X );
	}
	else
	{
		m_RTScale = 1.0f;
		m_CoordRect.fLeftU = 0;
		m_CoordRect.fTopV = 0;
		m_CoordRect.fRightU = 1;
		m_CoordRect.fBottomV = 1;
	}
	m_WantsBloomX = false;
	m_WantsBloomY = false;
	m_WantsGaussBlur = false;
	if(TechniqueName.find("BloomX") != -1)
	{
		m_WantsBloomX = true;
		FLOAT       afSampleOffsets[MAX_SAMPLES];
		HDRSystem::Instance()->GetSampleOffsets_Bloom( PostProcess::Instance()->m_dwCropWidth * m_RTScale, afSampleOffsets, avBloomWeights, 3.0f, 2.0f );
		for(int i=0; i < MAX_SAMPLES; i++ )
		{
			avBloomOffsets[i] = D3DXVECTOR2( afSampleOffsets[i], 0.0f );
		}
	}
	else if(TechniqueName.find("BloomY") != -1)
	{
		m_WantsBloomY = true;
		FLOAT       afSampleOffsets[MAX_SAMPLES];
		HDRSystem::Instance()->GetSampleOffsets_Bloom( PostProcess::Instance()->m_dwCropWidth * m_RTScale, afSampleOffsets, avBloomWeights, 3.0f, 2.0f );
		for(int i=0; i < MAX_SAMPLES; i++ )
		{
			avBloomOffsets[i] = D3DXVECTOR2( 0.0f, afSampleOffsets[i] );
		}
	}
	else if(TechniqueName.find("GaussBlur") != -1)
	{
		m_WantsGaussBlur = true;
		HDRSystem::Instance()->GetSampleOffsets_GaussBlur5x5( PostProcess::Instance()->m_dwCropWidth * m_RTScale,PostProcess::Instance()->m_dwCropHeight * m_RTScale, avBloomOffsets, avBloomWeights);
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void PostProcessEffect::RenderEffect()
{
	if(RenderDevice::Instance()->PixelShaderVersion < 2.0)
		return;

	UINT uiPassCount, uiPass;

	Mat->m_Shader->SetTechnique(TechniqueHandle);

	if(PostProcess::Instance()->GetCurrentDownScaled8xBuffer() && m_DownScaled8xParam)
		Mat->m_Shader->GetEffect()->SetTexture(m_DownScaled8xParam, PostProcess::Instance()->GetCurrentDownScaled8xBuffer());
	if(PostProcess::Instance()->GetCurrentDownScaled4xBuffer() && m_DownScaled4xParam)
		Mat->m_Shader->GetEffect()->SetTexture(m_DownScaled4xParam, PostProcess::Instance()->GetCurrentDownScaled4xBuffer());
	if(PostProcess::Instance()->GetCurrentDownScaled2xBuffer() && m_DownScaled2xParam)
		Mat->m_Shader->GetEffect()->SetTexture(m_DownScaled2xParam, PostProcess::Instance()->GetCurrentDownScaled2xBuffer());

    //Mat->m_Shader->GetEffect()->SetFloat("g_fMiddleGray",HDRSystem::Instance()->m_fKeyValue);
	if(PostProcess::Instance()->m_bUsingDOF)
	{
		if(m_RTScale == 1.0f)
		{
			HDRSystem::Instance()->FlipTargets();
			Mat->m_Shader->SetColorBuffer(HDRSystem::Instance()->GetColorBuffer());
		}
		else
			Mat->m_Shader->SetColorBuffer(HDRSystem::Instance()->GetCurrentBuffer());
	}
	else
	{
		if(m_RTScale == 1.0f)
		{
			if(DoneFullScaleEffect)
				HDRSystem::Instance()->CopyToLDRTarget();
			DoneFullScaleEffect = true;
		}
		Mat->m_Shader->SetColorBuffer(HDRSystem::Instance()->GetLDRColorBuffer());
	}

	///SET THE (SCALED IF APPROPRIATE) RENDER TARGET
	PDIRECT3DSURFACE9 pSurfScaledScene = NULL;
	PDIRECT3DSURFACE9 CurrentRT = NULL; 

	if(m_RTScale == .125f)
	{
		RenderWrap::dev->GetRenderTarget(0, &CurrentRT);
		PostProcess::Instance()->m_CurDownScaledScene8x = !PostProcess::Instance()->m_CurDownScaledScene8x;
		PostProcess::Instance()->GetCurrentDownScaled8xBuffer()->GetSurfaceLevel( 0, &pSurfScaledScene );
		RenderWrap::dev->SetRenderTarget( 0, pSurfScaledScene );

		if(m_WantsBloomX || m_WantsBloomY || m_WantsGaussBlur)
		{
			Mat->m_Shader->GetEffect()->SetValue("g_avSampleOffsets", avBloomOffsets, sizeof(avBloomOffsets));
			Mat->m_Shader->GetEffect()->SetValue("g_avSampleWeights", avBloomWeights, sizeof(avBloomWeights));
		}
		else
			Mat->m_Shader->GetEffect()->SetValue("g_avSampleOffsets", avSampleOffsets8X, sizeof(avSampleOffsets8X));
	}
	else if(m_RTScale == .25f)
	{
		RenderWrap::dev->GetRenderTarget(0, &CurrentRT);
		PostProcess::Instance()->m_CurDownScaledScene4x = !PostProcess::Instance()->m_CurDownScaledScene4x;
		PostProcess::Instance()->GetCurrentDownScaled4xBuffer()->GetSurfaceLevel( 0, &pSurfScaledScene );
		RenderWrap::dev->SetRenderTarget( 0, pSurfScaledScene );
		if(m_WantsBloomX || m_WantsBloomY || m_WantsGaussBlur)
		{
			Mat->m_Shader->GetEffect()->SetValue("g_avSampleOffsets", avBloomOffsets, sizeof(avBloomOffsets));
			Mat->m_Shader->GetEffect()->SetValue("g_avSampleWeights", avBloomWeights, sizeof(avBloomWeights));
		}
		else
			Mat->m_Shader->GetEffect()->SetValue("g_avSampleOffsets", avSampleOffsets4X, sizeof(avSampleOffsets4X));
	}
	else if(m_RTScale == .5f)
	{
		RenderWrap::dev->GetRenderTarget(0, &CurrentRT);
		PostProcess::Instance()->m_CurDownScaledScene2x = !PostProcess::Instance()->m_CurDownScaledScene2x;
		PostProcess::Instance()->GetCurrentDownScaled2xBuffer()->GetSurfaceLevel( 0, &pSurfScaledScene );
		RenderWrap::dev->SetRenderTarget( 0, pSurfScaledScene );
		if(m_WantsBloomX || m_WantsBloomY || m_WantsGaussBlur)
		{
			Mat->m_Shader->GetEffect()->SetValue("g_avSampleOffsets", avBloomOffsets, sizeof(avBloomOffsets));
			Mat->m_Shader->GetEffect()->SetValue("g_avSampleWeights", avBloomWeights, sizeof(avBloomWeights));
		}
		else
			Mat->m_Shader->GetEffect()->SetValue("g_avSampleOffsets", avSampleOffsets2X, sizeof(avSampleOffsets2X));
	}
	else if(m_WantsBloomX || m_WantsBloomY || m_WantsGaussBlur)
	{
		Mat->m_Shader->GetEffect()->SetValue("g_avSampleOffsets", avBloomOffsets, sizeof(avBloomOffsets));
		Mat->m_Shader->GetEffect()->SetValue("g_avSampleWeights", avBloomWeights, sizeof(avBloomWeights));
	}

	Mat->m_Shader->CommitChanges();

	Mat->m_Shader->GetEffect()->Begin(&uiPassCount, 0 );
	for (uiPass = 0; uiPass < uiPassCount; uiPass++)
	{
		Mat->m_Shader->BeginPass(uiPass);

		// HDR needs SRGB off
		for(int i=0;i<4;i++)
			RenderWrap::dev->SetSamplerState( i, D3DSAMP_SRGBTEXTURE, FALSE);

		PostProcess::Instance()->DrawFullScreenQuad(m_CoordRect);
		Mat->m_Shader->EndPass();
	}

	Mat->m_Shader->End();


	// RESTORE THE PREVIOUS RT IF JUST RENDERED TO A SCALED RT
	if(CurrentRT)
		RenderWrap::dev->SetRenderTarget( 0, CurrentRT );

	SAFE_RELEASE(CurrentRT);
	SAFE_RELEASE(pSurfScaledScene);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
vector<PostProcessEffect_Permanent::PostProcessEffect_Permanent_Info> PostProcessEffect_Permanent::PostProcessEffect_Permanent_Infos;
void PostProcessEffect_Permanent::ReloadPermanentEffects()
{
	for(int i = 0; i < PostProcessEffect_Permanent_Infos.size(); i++)
	{
		new PostProcessEffect_Permanent(PostProcessEffect_Permanent_Infos[i].ShaderFile,PostProcessEffect_Permanent_Infos[i].TechniqueName,PostProcessEffect_Permanent_Infos[i].orderingPreference,PostProcessEffect_Permanent_Infos[i].StartActive,true);
	}
}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
PostProcessEffect_Permanent::PostProcessEffect_Permanent(string ShaderFile, string TechniqueName,bool StartActive, int orderingPreference) : PostProcessEffect( ShaderFile, TechniqueName)
{
	int index = PostProcessEffect_Permanent_Infos.size();
	PostProcessEffect_Permanent_Infos.resize(index + 1);
	PostProcessEffect_Permanent_Infos[index].ShaderFile = ShaderFile;
	PostProcessEffect_Permanent_Infos[index].TechniqueName = TechniqueName;
	PostProcessEffect_Permanent_Infos[index].orderingPreference = orderingPreference;
	PostProcessEffect_Permanent_Infos[index].StartActive = StartActive;
	PostProcess::Instance()->AddPostProcessEffect(this,StartActive,orderingPreference);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
PostProcessEffect_Permanent::PostProcessEffect_Permanent(string ShaderFile, string TechniqueName, int orderingPreference,bool StartActive, bool dontAddToList) : PostProcessEffect(ShaderFile, TechniqueName)
{
	PostProcess::Instance()->AddPostProcessEffect(this,StartActive,orderingPreference);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void PostProcessEffect_Permanent::Release()
{
	for(int i = 0; i < PostProcessEffect_Permanent_Infos.size(); i++)
	{
		if(PostProcessEffect_Permanent_Infos[i].ShaderFile == shaderFile && PostProcessEffect_Permanent_Infos[i].TechniqueName == techniqueName)
			PostProcessEffect_Permanent_Infos.erase(PostProcessEffect_Permanent_Infos.begin() + i);
	}
}