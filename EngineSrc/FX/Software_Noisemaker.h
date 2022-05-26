//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
// Generates noise on CPU
//
//
//====================================================================================
#ifndef _Software_Noisemaker_
#define _Software_Noisemaker_

//#define CPU_NORMALS

//--------------------------------------------------------------------------------------
// Noise defines
//--------------------------------------------------------------------------------------
#define n_bits				5
#define n_size				(1<<(n_bits-1))
#define n_size_m1			(n_size - 1)
#define n_size_sq			(n_size*n_size)
#define n_size_sq_m1		(n_size_sq - 1)

#define n_packsize			4

#define np_bits				(n_bits+n_packsize-1)
#define np_size				(1<<(np_bits-1))
#define np_size_m1			(np_size-1)
#define np_size_sq			(np_size*np_size)
#define np_size_sq_m1		(np_size_sq-1)

#define n_dec_bits			12
#define n_dec_magn			4096
#define n_dec_magn_m1		4095

#define max_octaves			32

#define noise_frames		256
#define noise_frames_m1		(noise_frames-1)

#define noise_decimalbits	15
#define noise_magnitude		(1<<(noise_decimalbits-1))

#define scale_decimalbits	15
#define scale_magnitude		(1<<(scale_decimalbits-1))

// TODO: Make these configurable!!
#define nmapsize_x	256
#define nmapsize_y	512


struct SOFTWARESURFACEVERTEX
{
	float x,y,z;
	float nx,ny,nz;
	float tu,tv;
};

#define D3DFVF_SOFTWARESURFACEVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1)

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
class Software_Noisemaker : public RenderBase 
{
public:
	/// Editable parameters
	float fFalloff;
	float fScale;
	int	  iOctaves;
	bool  bDisplace;
	bool  bPaused;
	float fTimemulti;
	float fAnimspeed;
	bool  bSmooth;
	float fStrength;
	float fDampeningRadius;

	Software_Noisemaker(int sizeX, int sizeY);
	~Software_Noisemaker();
	void resize(int sizeX, int sizeY);
	bool render_geometry(const D3DXMATRIXA16 *m);
	D3DXVECTOR4 calc_worldpos(D3DXVECTOR2 uv, const D3DXMATRIXA16 *m);
	float GetHeightAt(int, int, int);
	float get_height_dual(int, int);
	float GetHeightAt(float, float);
	SOFTWARESURFACEVERTEX *vertices;
	void calc_normals();	

private:

	virtual HRESULT OnResetDevice(){ init_textures(); return S_OK; };
	virtual void	OnLostDevice(){ FreeTextures(); };
	virtual void	OnDestroyDevice(){};

	void init_noise();
	void calc_noise();
	float readtexel_nearest(int u, int v);
	int readtexel_linear(int u, int v, int);
	int readtexel_linear_dual(int u, int v, int);
	int mapsample(int u, int v, int level, int octave);

	int sizeX, sizeY;	/// framebuffer size
	float f_sizeX, f_sizeY;
	float *framebuffer;
	int noise[n_size_sq*noise_frames];
	int o_noise[n_size_sq*max_octaves];
	int p_noise[np_size_sq*(max_octaves>>(n_packsize-1))];	
	int *r_noise;
	int octaves;
	D3DXVECTOR3 e_u, e_v;
	int multitable[max_octaves];
	DWORD last_time;
	float f_multitable[max_octaves];	
	double time;

	/// remember these
	D3DXVECTOR4 t_corners0,t_corners1,t_corners2,t_corners3;

	/// textures & stuff for the GPU normalmap calculation
public:
	LPDIRECT3DTEXTURE9	packed_noise_texture[2];
	LPDIRECT3DTEXTURE9	heightmap,normalmap;
	LPDIRECT3DSURFACE9	depthstencil;
	Shader		hmap_effect, nmap_effect;

	void FreeTextures();
	void init_textures();
	void upload_noise();
	void generate_normalmap();	
	void load_effects();

};

inline float Software_Noisemaker::readtexel_nearest(int u, int v)
{
	int lu, lv;
	lu = (u>>n_dec_bits)&n_size_m1;
	lv = (v>>n_dec_bits)&n_size_m1;	
	return noise[lv*n_size + lu];
}


inline int Software_Noisemaker::readtexel_linear_dual(int u, int v, int o)
{
	int iu, iup, iv, ivp, fu, fv;
	iu = (u>>n_dec_bits)&np_size_m1;
	iv = ((v>>n_dec_bits)&np_size_m1)*np_size;
	
	iup = ((u>>n_dec_bits) + 1)&np_size_m1;
	ivp = (((v>>n_dec_bits) + 1)&np_size_m1)*np_size;
	
	fu = u & n_dec_magn_m1;
	fv = v & n_dec_magn_m1;
		
	int ut01 = ((n_dec_magn-fu)*r_noise[iv + iu] + fu*r_noise[iv + iup])>>n_dec_bits;
	int ut23 = ((n_dec_magn-fu)*r_noise[ivp + iu] + fu*r_noise[ivp + iup])>>n_dec_bits;
	int ut = ((n_dec_magn-fv)*ut01 + fv*ut23) >> n_dec_bits;
	return ut;
}

inline float Software_Noisemaker::get_height_dual(int u, int v)
{	
	int value=0;	
	r_noise = p_noise;	/// pointer to the current noise source octave
	int hoct = octaves / n_packsize;
	for(int i=0; i<hoct; i++)
	{		
		value += readtexel_linear_dual(u,v,0);
		u = u << n_packsize;
		v = v << n_packsize;
		r_noise += np_size_sq;
	}		
	return value*fStrength/noise_magnitude;
}

#endif