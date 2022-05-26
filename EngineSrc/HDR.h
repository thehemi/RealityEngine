//=========== (C) Copyright 2003, Artificial Studios. All rights reserved. ==================
/// Name: HDRSystem
/// \brief Applies HDR shaders postprocess fx onto scene Render Target 
/// includes Bloom, Spectral Flares, Tone Mapping, & Blue Shift
//===========================================================================================

#pragma once

#include "GlareDefD3D.h"

//-----------------------------------------------------------------------------
/// Defines, constants, and custom types
//-----------------------------------------------------------------------------
#define MAX_SAMPLES           16      /// Maximum number of texture grabs
//#define NUM_LIGHTS          2       /// Number of lights in the scene
//#define EMISSIVE_COEFFICIENT  39.78f  /// Emissive color multiplier for each lumen of light intensity                                    
#define NUM_TONEMAP_TEXTURES  4       /// Number of stages in the 4x4 down-scaling  of average luminance textures
#define NUM_STAR_TEXTURES     12      /// Number of textures used for the star post-processing effect
#define NUM_BLOOM_TEXTURES    3       /// Number of textures used for the bloom post-processing effect



/// Screen quad vertex format
struct ScreenVertex
{
	D3DXVECTOR4 p; /// position
	D3DXVECTOR2 t; /// texture coordinate

	static const DWORD FVF;
};


//-----------------------------------------------------------------------------
/// \brief Applies HDR shaders postprocess fx onto scene Render Target 
/// includes Bloom, Spectral Flares, Tone Mapping, & Blue Shift
//-----------------------------------------------------------------------------
class ENGINE_API HDRSystem
{
public:
	static HDRSystem* Instance () ;

    /// Toggle ldr status of target
    void SetLDR(bool ldr);

	bool	IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, bool bWindowed );
	HRESULT OnCreateDevice(const D3DSURFACE_DESC* DisplayMode );
	HRESULT OnResetDevice();
	void	OnLostDevice();
	void	OnDestroyDevice();
	void	OnFrameMove();
	HRESULT PreRender();
	void	PostRender();
	HRESULT FlipTargets(bool copy = true);
	void	CopyToLDRTarget();

	PDIRECT3DTEXTURE9 GetColorBuffer(); /// Previous pass target for HDR shaders doing alpha blending
	PDIRECT3DTEXTURE9 GetLDRColorBuffer(); /// LDR RT specifically for LDR fx
	PDIRECT3DTEXTURE9 GetCurrentBuffer(); /// Current pass target for PostProcess shaders
	HRESULT PostRenderClean();

	float		m_fBlueShift;			  /// Blue shift coefficient
	float		m_fMinLum;				  /// Minimum luminance sample, to cap the maximum darkness the viewer will adapt to
	float		m_fMaxLum;				  /// Maximum luminance sample, to cap the maximum brightness the viewer will adapt to
	float       m_fKeyValue;              /// Middle gray key value for tone mapping (basically exposure)
	bool        m_bToneMap;               /// True when scene is to be tone mapped            
	bool        m_bDetailedStats;         /// True when state variables should be rendered
	bool        m_bDrawHelp;              /// True when help instructions are to be drawn
	bool        m_bBlueShift;             /// True when blue shift is to be factored in
	bool        m_bAdaptationInvalid;     /// True when adaptation level needs refreshing
	bool		m_bEnabled;
	EGLARELIBTYPE     m_eGlareType;       /// Enumerated glare type

	/// Currently LDR?
	bool		m_IsLDR;
	bool IsReady(){ return m_pEffect != NULL; }

	HRESULT GetTextureCoords( PDIRECT3DTEXTURE9 pTexSrc, RECT* pRectSrc, PDIRECT3DTEXTURE9 pTexDest, RECT* pRectDest, CoordRect* pCoords );
	/// Sample offset calculation. These offsets are passed to corresponding
	/// pixel shaders.
	HRESULT GetSampleOffsets_GaussBlur5x5(DWORD dwD3DTexWidth, DWORD dwD3DTexHeight, D3DXVECTOR2* avTexCoordOffset, D3DXVECTOR4* avSampleWeights, FLOAT fMultiplier = 1.0f );
	HRESULT GetSampleOffsets_Bloom(DWORD dwD3DTexSize, float afTexCoordOffset[15], D3DXVECTOR4* avColorWeight, float fDeviation, FLOAT fMultiplier=1.0f);    
	HRESULT GetSampleOffsets_Star(DWORD dwD3DTexSize, float afTexCoordOffset[15], D3DXVECTOR4* avColorWeight, float fDeviation);    
	HRESULT GetSampleOffsets_DownScale4x4( DWORD dwWidth, DWORD dwHeight, D3DXVECTOR2 avSampleOffsets[] );
	HRESULT GetSampleOffsets_DownScale2x2( DWORD dwWidth, DWORD dwHeight, D3DXVECTOR2 avSampleOffsets[] );

    int				  m_CurTarget;			  /// Current target, 0 or 1

protected:
	HDRSystem();

	PDIRECT3DSURFACE9 pSurfLDR; /// Low dynamic range surface for final output
	PDIRECT3DSURFACE9 pSurfHDR; /// High dynamic range surface to store 
	/// intermediate floating point color values

	/// Post-processing source textures creation
	HRESULT Scene_To_SceneScaled();
	HRESULT SceneScaled_To_BrightPass();
	HRESULT BrightPass_To_StarSource();
	HRESULT StarSource_To_BloomSource();


	/// Post-processing helper functions
	HRESULT GetTextureRect( PDIRECT3DTEXTURE9 pTexture, RECT* pRect );


	/// Tone mapping and post-process lighting effects
	HRESULT MeasureLuminance();
	HRESULT CalculateAdaptation();
	HRESULT RenderStar();
	HRESULT RenderBloom();

	HRESULT ClearTexture( LPDIRECT3DTEXTURE9 pTexture );

	VOID    ResetOptions();
	VOID    DrawFullScreenQuad(CoordRect c) { DrawFullScreenQuad( c.fLeftU, c.fTopV, c.fRightU, c.fBottomV ); }
	VOID    DrawFullScreenQuad(float fLeftU, float fTopV, float fRightU, float fBottomV);

	static inline float GaussianDistribution( float x, float y, float rho );

private:

	PDIRECT3DTEXTURE9 m_pTexScene[2];         /// HDR render targets containing the scene
	PDIRECT3DTEXTURE9 m_pLDRTexture; /// LDR texture for LDR RT fx
	
	PDIRECT3DTEXTURE9 m_pTexSceneScaled;      /// Scaled copy of the HDR scene
	PDIRECT3DTEXTURE9 m_pTexBrightPass;       /// Bright-pass filtered copy of the scene
	PDIRECT3DTEXTURE9 m_pTexAdaptedLuminanceCur;  /// The luminance that the user is currenly adapted to
	PDIRECT3DTEXTURE9 m_pTexAdaptedLuminanceLast; /// The luminance that the user is currenly adapted to
	PDIRECT3DTEXTURE9 m_pTexStarSource;       /// Star effect source texture
	PDIRECT3DTEXTURE9 m_pTexBloomSource;      /// Bloom effect source texture

	PDIRECT3DTEXTURE9 m_apTexBloom[NUM_BLOOM_TEXTURES];     /// Blooming effect working textures
	PDIRECT3DTEXTURE9 m_apTexStar[NUM_STAR_TEXTURES];       /// Star effect working textures
	PDIRECT3DTEXTURE9 m_apTexToneMap[NUM_TONEMAP_TEXTURES]; /// Log average luminance samples 
	/// from the HDR render target

	LPD3DXEFFECT      m_pEffect;          /// Effect file

	LPD3DXMESH        m_pWorldMesh;       /// Mesh to contain world objects
	LPD3DXMESH        m_pmeshSphere;      /// Representation of point light

	CGlareDef         m_GlareDef;         /// Glare defintion

	/*
	D3DXVECTOR4       m_avLightPosition[NUM_LIGHTS];    /// Light positions in world space
	D3DXVECTOR4       m_avLightIntensity[NUM_LIGHTS];   /// Light floating point intensities
	int               m_nLightLogIntensity[NUM_LIGHTS]; /// Light intensities on a log scale
	int               m_nLightMantissa[NUM_LIGHTS];     /// Mantissa of the light intensity
	*/
	DWORD m_dwCropWidth;    /// Width of the cropped scene texture
	DWORD m_dwCropHeight;   /// Height of the cropped scene texture
};
