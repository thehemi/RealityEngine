//=========== Copyright (c) 2004, Artificial Studios. All rights reserved. ===========
/// Name: PostProcess
/// Desc: Applies various pixel shaders "filters" to final scene render, before HDR is applied
//====================================================================================

#pragma once

//-----------------------------------------------------------------------------
/// A stackable post-proces effect that contains the shaders and handles rendering of the effect; this is full screen rez
//-----------------------------------------------------------------------------
class ENGINE_API PostProcessEffect
{
public:
	Material* Mat;
	D3DXHANDLE TechniqueHandle;
	bool Active;
	bool ActiveOneFrame;
	string shaderFile;
	string techniqueName;
	PostProcessEffect(string ShaderFile, string TechniqueName);
	virtual ~PostProcessEffect(){SAFE_RELEASE(Mat);}
	virtual void RenderEffect();
	D3DXHANDLE m_DownScaled8xParam;
	D3DXHANDLE m_DownScaled4xParam;
	D3DXHANDLE m_DownScaled2xParam;
	float m_RTScale;
	CoordRect m_CoordRect;
	bool m_WantsBloomX;
	bool m_WantsBloomY;
	bool m_WantsGaussBlur;
	D3DXVECTOR2 avBloomOffsets[16];
	D3DXVECTOR4 avBloomWeights[16];
};

//-----------------------------------------------------------------------------
/// A stackable post-proces effect that contains the shaders and handles rendering of the effect; this is a permanent effect that always stays on the stack until it is Released()
//-----------------------------------------------------------------------------
class ENGINE_API PostProcessEffect_Permanent : public PostProcessEffect
{
public:
	/// contains information to allow re-initialization of the effect across world loads or lostdevice
	struct PostProcessEffect_Permanent_Info
	{
		string ShaderFile;
		string TechniqueName;
		int orderingPreference;
		bool StartActive;
	};
	static vector<PostProcessEffect_Permanent_Info> PostProcessEffect_Permanent_Infos;
	static void ReloadPermanentEffects();
	PostProcessEffect_Permanent(string ShaderFile, string TechniqueName,bool StartActive,int orderingPreference = -1);
	PostProcessEffect_Permanent(string ShaderFile, string TechniqueName,int orderingPreference,bool StartActive,bool dontAddToList);
	void Release();
};

//-----------------------------------------------------------------------------
/// Manages & applies various pixel shader filters to post scene render, before final HDR is processed
//-----------------------------------------------------------------------------
class ENGINE_API PostProcess
{
	friend class PostProcessEffect;
public:
    /// Is DOF on? Thus needs color.a = z?
    bool m_bUsingDOF;

	/// Singleton
	static PostProcess* Instance () ;

	/// Sets Render Targets screen resolution 
	HRESULT OnResetDevice();
	/// Frees Render Target textures
	void	OnDestroyDevice();
	void	OnLostDevice();

	/// Renders the stacked Post Process FX
	void	PostRender();
    /// Renders stacked FX that need HDR
    void    PostRenderHDR();
	
	/// Adds a new PostProcessEffect to the stack based on a given shader file and technique. Scaling is % of screen resolution of texture to render effect, can be .25, .5, or 1, and is stored in appropriate sampler. OrderingPreference allows effects to be inserted at a specified index in the stack.
	PostProcessEffect* AddPostProcessEffect(string ShaderFile, string TechniqueName, bool StartActive = true, int OrderingPreference = -1);
	/// Adds an already-created PostProcessEffect to the stack. Scaling is % of screen resolution of texture to render effect, can be .25, .5, or 1, and is stored in appropriate sampler. OrderingPreference allows effects to be inserted at a specified index in the stack.
	PostProcessEffect* AddPostProcessEffect(PostProcessEffect* ppEffect, bool StartActive = true, int OrderingPreference = -1);
	/// Finds a stacked Post-Process Effect that uses the given shader file and technique, if any. Useful for targeted deletion of specific PostProcess FX.
	PostProcessEffect* FindPostProcessEffect(string ShaderFile, string TechniqueName);
	/// Removes a given PostProcess effect from the stack, if the effect exists on it. This does not delete the PostProcessEffect object itself however.
	void RemovePostProcessEffect(PostProcessEffect* ppEffect);
	/// Removes AND Deletes all effects on the stack, useful for totally resetting state of PostProcess FX system in between World loads.
	void RemoveAllEffects();
	/// Loads up a set of PostProcess FX defined in a config file, usually a level configuration file
	void CreateFXFromConfigFile(ConfigFile& config);
    /// Check for active effects
    bool EffectsActive();

	DWORD m_dwCropWidth;
	DWORD m_dwCropHeight;

	void DrawFullScreenQuad(float fLeftU, float fTopV, float fRightU, float fBottomV);

protected:
	PDIRECT3DTEXTURE9 m_pDownScaledScene8x[2];
	int m_CurDownScaledScene8x;

	PDIRECT3DTEXTURE9 m_pDownScaledScene4x[2];
	int m_CurDownScaledScene4x;

	PDIRECT3DTEXTURE9 m_pDownScaledScene2x[2];
	int m_CurDownScaledScene2x;

	PDIRECT3DTEXTURE9 GetCurrentDownScaled8xBuffer(){return m_pDownScaledScene8x[m_CurDownScaledScene8x];}
	PDIRECT3DTEXTURE9 GetCurrentDownScaled4xBuffer(){return m_pDownScaledScene4x[m_CurDownScaledScene4x];}
	PDIRECT3DTEXTURE9 GetCurrentDownScaled2xBuffer(){return m_pDownScaledScene2x[m_CurDownScaledScene2x];}

	void DrawFullScreenQuad(CoordRect c) { DrawFullScreenQuad( c.fLeftU, c.fTopV, c.fRightU, c.fBottomV ); }

    void PostRenderFunc();
	PostProcess();

private:
	vector<PostProcessEffect*> PostProcessEffects;
	ConfigFile* FXConfig;
};