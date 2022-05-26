//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
/// Shadow Mapping!
///
/// Author: Tim Johnson
//====================================================================================
#pragma once
#ifndef SHADOWMAPPING_INCLUDED
#define SHADOWMAPPING_INCLUDED

//--------------------------------------------------------------------------------------
/// Encapsulates functionality for a single shadow map
//--------------------------------------------------------------------------------------
class ShadowMap : public RenderBase {
private:
	int m_Size;
	LPDIRECT3DSURFACE9		m_pOldDS;
	LPDIRECT3DSURFACE9		m_pOldRT;
	/// Texture to which the shadow map is rendered
	LPDIRECT3DTEXTURE9      m_pShadowMap;    
	/// Depth-stencil buffer for rendering to shadow map
	LPDIRECT3DSURFACE9      m_pDSShadow;     
    /// Texture of depth-stencil
    LPDIRECT3DTEXTURE9      m_pTexDepthShadow;
    /// Jitter texture!
    static LPDIRECT3DVOLUMETEXTURE9      m_pJitterTexture;
public:
	float GetSize(){ return m_Size;}

	ShadowMap(){m_pTexDepthShadow=0;}
    ~ShadowMap(){ FreeResources(); }

    void FreeResources();

    LPDIRECT3DVOLUMETEXTURE9 GetJitterMap(){ return m_pJitterTexture; }
	LPDIRECT3DTEXTURE9 GetMap();
	void Initialize(int size);
	/// Begin/End capturing scene to shadow map
	void BeginCapture();
	void EndCapture();

	virtual HRESULT OnResetDevice(){Initialize(m_Size); return S_OK;};
	virtual void	OnLostDevice(){FreeResources();};
};


/// RenderTargets for drop shadows
struct ShadowTarget
{
    /// Render Target
    LPDIRECT3DTEXTURE9 map;
    /// Allows actors to clarify wether they were the last to update it
    Actor*              lastOwner;
    /// Being used this frame? (GSeconds)
    float               fLastUsed;
    /// DTor
    ~ShadowTarget(){ SAFE_RELEASE(map); }
    /// Ctor
    ShadowTarget(){ map=0;}
};


/// Cached data for a single shadow
struct CachedShadow
{
    /// Light causing shadow
    Light*      srcLight;
    /// Light projection
    D3DXMATRIX  proj;
    /// Light position
    Vector      lightPos;
    /// Render target shadow is bound to
    ShadowTarget*   rt;
    /// Cached world polygons under drop-shadow
    vector<Vertex>  m_ShadowPolys;
    /// Needed to know when to update shadow polygons
    float    m_LastMovedTime;
    /// Last time RT was updated
    float    rtLastUpdated;

    CachedShadow(){ rt=0; srcLight=0; }
};

//--------------------------------------------------------------------------------------
/// Shadow Engine - Handles per-item drop shadows
//--------------------------------------------------------------------------------------
class ShadowEngine : public RenderBase
{
protected:
    /// Last time device was reset, flushing all shadows 
    float m_LastReset;
    /// Array of all shadow targets
    vector<ShadowTarget*> m_pShadowMaps;
    /// Texture for blob drop shadow
    Texture m_BlobShadow;
    ///
    void RenderShadowItem(ModelFrame* pFrame, Shader* m_CurShader);
    /// Renders to a shadow map for a light, based on the lights shadow items
	void RenderShadowMap(Light* light, Camera* cam);
    ///
    D3DXMATRIX GetView(Actor* a);
    ///
    Vector GetBiasedLightPos(Actor* a);
    virtual HRESULT OnResetDevice();
	virtual void	OnLostDevice();
public:
    /// Enable shadows
    bool ShadowsEnabled;
    /// Singleton
	static  ShadowEngine* Instance();
    /// Ctor
    ShadowEngine();
    /// Maximum shadows visible at any one time
    int MaxVisibleShadows;
    /// Maximum distance before a shadow is faded out
    int MaxShadowDistance;
    /// Renders all per-actor drop shadows, which are special case per-item shadows
    void RenderDropShadows(World* world, Camera* cam);
    /// Render to all shadow maps (doesn't draw actual shadows)
    void RenderShadowMaps(World* world, Camera* cam);
};

//----------------------------------------------------------------------------------
// Desc: Face class used during shadow splitting
//----------------------------------------------------------------------------------
struct Face
{
    int user1, user2;
	Vertex v[3];
	Face(){ user1=user2=0;  }
};

#endif
