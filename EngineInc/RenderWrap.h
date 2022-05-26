//=========== Copyright (c) 2003, Tim Johnson. All rights reserved. ===========
/// Low-level Functionality for graphics rendering classes
//
//=============================================================================
#ifndef RENDERWRAP_INCLUDED
#define RENDERWRAP_INCLUDED
#include "d3dcustom.h" /// Fuck it, include D3D


enum GfxComp
{
  GFX_NEVER         = 1,
  GFX_LESS          = 2,
  GFX_EQUAL         = 3,
  GFX_LESSEQUAL		= 4,
  GFX_GREATER       = 5,
  GFX_NOTEQUAL		= 6,
  GFX_GREATEREQUAL	= 7,
  GFX_ALWAYS        = 8,
};

enum GfxBlend
{
  GFX_ZERO          = 1,
  GFX_ONE           = 2,
  GFX_SRCCOLOR		= 3,
  GFX_INVSRCCOLOR	= 4,
  GFX_SRCALPHA		= 5,
  GFX_INVSRCALPHA	= 6,
  GFX_DESTCOLOR		= 9,
  GFX_INVDESTCOLOR	= 10,
};

/*
/// It's always exponential now
enum GfxFogMode {
	GFX_FOGNONE = 0,
	GFX_FOGEXP = 1,
	GFX_FOGEXP2 = 2,
	GFX_FOGLINEAR = 3,
};
*/

enum GfxCull
{
  GFX_NONE  = 1,
  GFX_CW	= 2,
  GFX_CCW	= 3,
};

//--------------------------------------------------------------------------------------
/// Low-level Functionality for graphics rendering classes
//--------------------------------------------------------------------------------------
class ENGINE_API RenderWrap {
public:

	static void Restore();

	static Matrix BuildView(Vector& pos, Vector& dir, float roll=0);
	static Matrix BuildProjection(float fov, float nearclip, float farclip);
	static struct IDirect3DDevice9* dev;
	static struct IDirect3D9 *d3d;

	static void DrawPrim(bool strips, int numVerts, void* verts, int vertsize);

	static void SetBlending(GfxBlend src, GfxBlend dest);
	static void DisableBlending();


	static DWORD SetRS(D3DRENDERSTATETYPE  state, DWORD val); /// returns current RS
	static DWORD GetRS(D3DRENDERSTATETYPE  state);

	static DWORD SetTSS(int stage, DWORD what, DWORD val); /// returns current TSS
	static DWORD GetTSS(int stage, DWORD what);

	static void SetSS(int stage, D3DSAMPLERSTATETYPE what, DWORD val);
	static DWORD GetSS(int stage, D3DSAMPLERSTATETYPE what);

	static void SetWorld(const Matrix& world);
	static Matrix GetWorld();

	static void SetView(Matrix& view);
	static Matrix GetView();

	static Matrix GetViewProjection();

	static void SetProjection(Matrix& projection);
	static Matrix GetProjection();

	static void ClearTextureLevel(int level);

	static void SetMaterial(COLOR rgb);
	static void SetMaterial(void* mat);
};



#endif
