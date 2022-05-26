//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
// Generates noise in hardware. Requires shaders 3.0
//
//====================================================================================

#ifndef _c_noisemaker_
#define _c_noisemaker_

#include <d3dx9.h>

#include "parameterhandler.h"

struct tc{
	float x, y;
};

struct NOISEVERTEX
{
	D3DXVECTOR3	position;
};

#define D3DFVF_NOISEVERTEX (D3DFVF_XYZ)

class noisemaker{
public:
	noisemaker(const LPDIRECT3DDEVICE9 device, int sizeX, int sizeY, int octaves, int animlength, parameterhandler *prm);	
	~noisemaker();
	void render(const D3DXMATRIXA16 *mProjector);
	void render_projected_noise(const D3DXMATRIXA16 *mProjector);
	void render_normalmap();
	void render_noise_octaves();
	LPDIRECT3DTEXTURE9			rendered_texture;
	LPDIRECT3DTEXTURE9			rendered_normalmap;
	LPD3DXRENDERTOSURFACE		RenderToSurface;
	LPD3DXRENDERTOSURFACE		RenderToNormalmap;
	LPDIRECT3DSURFACE9			rendered_texture_surface;
	LPDIRECT3DTEXTURE9			noise_octaves[16];
private:
	D3DXMATRIXA16 mProjector;
	void prepare_textures();
	void create_vertexbuffer();
	void LoadEffect();
	int sizeX, sizeY, sourcesize, animlength, octaves, patches_x, patches_y; 
	float falloff;
	LPDIRECT3DDEVICE9			device;
	LPDIRECT3DVERTEXBUFFER9		VB,quad;
	LPDIRECT3DINDEXBUFFER9		IB;
	LPDIRECT3DTEXTURE9			source_texture;
	
	LPDIRECT3DSURFACE9			noise_octave_surface[16];
	LPDIRECT3DSURFACE9			backbuffer, rendertarget;

	LPD3DXRENDERTOSURFACE		RenderToOctaves;
	LPDIRECT3DSURFACE9			rendered_normalmap_surface;
	LPD3DXEFFECT				anim_effect, normalmap_effect, noise_octaves_effect;

	tc *offsets;
	parameterhandler *prm;
};

#endif