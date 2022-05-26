//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
// Patch surface, for moving water
// Uses algorithm from http://graphics.cs.lth.se/theses/projects/projgrid/projgrid-hq.pdf
//
//====================================================================================
#ifndef _surface_
#define _surface_
#include "Software_Noisemaker.h"


//--------------------------------------------------------------------------------------
// A structure for our custom vertex type
struct CUSTOMVERTEX
{
	FLOAT x, y, z;      // The untransformed, 3D position for the vertex
	DWORD color;        // The vertex color
};

struct SURFACEVERTEX
{
	D3DXVECTOR3	position;	
	float displacement;
};

struct DISPLACEMENT
{
	DWORD displacement;
};

enum rendermode
{
	RM_POINTS = 0,
	RM_WIREFRAME,
	RM_SOLID
};

//--------------------------------------------------------------------------------------
// Our custom FVF, which describes our custom vertex structure
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)
#define D3DFVF_SURFACEVERTEX (D3DFVF_XYZ|D3DFVF_TEX1) //|D3DFVF_TEX1
#define D3DFVF_DISPLACEMENT (D3DFVF_DIFFUSE)
#define D3DFVF_SURFACEVERTEX_AND_DISPLACEMENT (D3DFVF_XYZ|D3DFVF_DIFFUSE)

//--------------------------------------------------------------------------------------
// Patch surface, for moving water
//--------------------------------------------------------------------------------------
class Surface : public RenderBase {
public:
	/// Editable parameters
	FloatColor	fWaterColour;
	float	fLODbias;
	float	bReflRefrStrength;
	float	fOpacity;
	bool	bDiffuseRefl;
	float	fElevation;
	bool	bAsPoints;
	bool	bDisplayTargets;
	
	int m_TargetSize;

	Surface();
	~Surface();
	/// Init surface
	void Initialize(const D3DXVECTOR3* position, const D3DXVECTOR3* normal,int m_GridX, int m_GridY, int tex_size);
	/// Update the surface before rendering
	bool Prepare(Camera* cam);
	/// Render surface with attached targets. Targets should be filled by this point
	bool Render(World* pWorld);
	/// Cutter plane, rarely used
	void RenderCutter();
	/// Just for stats
	void CalcEfficiency();
	/// Points/wireframe/solid
	void SetRenderMode(int rendermode);
	/// Change tessellation
	void SetGridSize(int size_x, int size_y);
	/// Change reflection/refraction targets
	void SetTextureSize(int size);
	/// Change displacement
	void SetDisplacementAmplitude(float amplitude);
	/// Height at position in water, for displacement
	float GetHeightAt(float,float);
	
	float efficiency;
	D3DXMATRIXA16 range;
	D3DXPLANE	plane, upper_bound, lower_bound;
	/// Textures for reflection/refraction
	LPDIRECT3DTEXTURE9			m_RefractionTex, m_ReflectionTex;
	/// Z-Buffer and stencil for reflection/refraction
	LPDIRECT3DSURFACE9			depthstencil;

	/// The camera that does the actual projection
	Camera						*m_ProjectingCamera;	
	/// The camera whose frustum the projection is created for
	/// also the camera the geometry is transformed through when rendering.
	Camera						m_ObservingCamera;		
	/// Class for generating noise													
	Software_Noisemaker *m_NoiseMaker;
    
    /// Update water height
    void SetPosition(const D3DXVECTOR3* pos);
     /// Water pos (height)
    D3DXVECTOR3 m_Pos;

private:
   

	virtual HRESULT OnResetDevice(){ InitializeData(); return S_OK; };
	virtual void	OnLostDevice(){ FreeData(); };
	virtual void	OnDestroyDevice(){};

	/// D3D stuff, uses DEFAULT so needs recreating
	bool InitializeData(bool buffersOnly = false, bool targetsOnly = false);
	/// Free D3D stuff
	void FreeData();
	/// Used for projective transform
	bool GetMinMax(D3DXMATRIXA16 *range);
	void SetupMatrices(const Camera *camera_view);
	/// Frustum calc
	bool WithinFrustum(const D3DXVECTOR3 *pos);
	/// Loads shaders
	void LoadEffect();
	bool initialized, boxfilter;
protected:	
	/// Temp stuff
	D3DXVECTOR3	normal, u, v, pos;
	/// Current tessellation
	int			m_GridX, m_GridY;
	/// Current render mode
	int			rendermode;
	/// Result of plane/frustum check
	bool		m_bPlaneInFrustum;
		
	/// Vertices for water
	LPDIRECT3DVERTEXBUFFER9		m_SurfVertices;	
	/// Indices for water
	LPDIRECT3DINDEXBUFFER9		m_SurfIndices;
	/// All our textures
	LPDIRECT3DTEXTURE9			m_Surf_texture, m_Surf_fresnel, underwater_fresnel, noise2D;
	Shader						m_Surf_software_effect, underwater_software_effect;
};

#endif