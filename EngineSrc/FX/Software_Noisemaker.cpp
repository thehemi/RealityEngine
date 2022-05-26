//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
// Generates noise on CPU
//
//====================================================================================
#include "stdafx.h"
#include "Software_Noisemaker.h"
#include <xmmintrin.h>
#include <math.h>
#include <mmsystem.h>

//--------------------------------------------------------------------------------------
bool packednoise = true;
 
//--------------------------------------------------------------------------------------
// Takes x/y tessellation for grid
//--------------------------------------------------------------------------------------
Software_Noisemaker::Software_Noisemaker(int sX, int sY)
{
	this->sizeX = sX;
	this->sizeY = sY;
	time = 0.0;
	last_time = timeGetTime();
	octaves = 0;	// don't want to have the noise accessed before it's calculated

	f_sizeX = (float) sizeX;
	f_sizeY = (float) sizeY;	

	// reset normals
	vertices	= new SOFTWARESURFACEVERTEX[sizeX*sizeY];	
	for(int v=0; v<sizeY; v++)
	{
		for(int u=0; u<sizeX; u++)
		{
			vertices[v*sizeX + u].nx =	0.0f;
			vertices[v*sizeX + u].ny =	1.0f;
			vertices[v*sizeX + u].nz =	0.0f;
			vertices[v*sizeX + u].tu = (float)u/(sizeX-1);
			vertices[v*sizeX + u].tv = (float)v/(sizeY-1);
		}
	}	
	this->init_noise();	
	#ifndef CPU_NORMALS
	this->load_effects();
	this->init_textures();
	#endif
}

//--------------------------------------------------------------------------------------
// Resizes x/y tess
//--------------------------------------------------------------------------------------
void Software_Noisemaker::resize(int sX, int sY)
{
	this->sizeX = sX;
	this->sizeY = sY;

	f_sizeX = (float) sizeX;
	f_sizeY = (float) sizeY;

	// reset normals
	SAFE_DELETE(vertices);
	vertices	= new SOFTWARESURFACEVERTEX[sizeX*sizeY];	
	for(int v=0; v<sizeY; v++)
	{
		for(int u=0; u<sizeX; u++)
		{
			vertices[v*sizeX + u].nx =	0.0f;
			vertices[v*sizeX + u].ny =	1.0f;
			vertices[v*sizeX + u].nz =	0.0f;
			vertices[v*sizeX + u].tu = (float)u/(sizeX-1);
			vertices[v*sizeX + u].tv = (float)v/(sizeY-1);
		}
	}
}

//--------------------------------------------------------------------------------------
// Generate an array with noise in
//--------------------------------------------------------------------------------------
void Software_Noisemaker::init_noise()
{	
	// create noise (uniform)
	float tempnoise[n_size_sq*noise_frames];
	for(int i=0; i<(n_size_sq*noise_frames); i++)
	{
		//this->noise[i] = rand()&0x0000FFFF;		
		float temp = (float) rand()/RAND_MAX;		
		tempnoise[i] = 4*(temp - 0.5f);	
	}	

	for(int frame=0; frame<noise_frames; frame++)
	{
		for(int v=0; v<n_size; v++)
		{
			for(int u=0; u<n_size; u++)
			{	
				/*float temp = 0.25f * (tempnoise[frame*n_size_sq + v*n_size + u] +
									  tempnoise[frame*n_size_sq + v*n_size + ((u+1)&n_size_m1)] + 
									  tempnoise[frame*n_size_sq + ((v+1)&n_size_m1)*n_size + u] +
									  tempnoise[frame*n_size_sq + ((v+1)&n_size_m1)*n_size + ((u+1)&n_size_m1)]);*/
				int v0 = ((v-1)&n_size_m1)*n_size,
					v1 = v*n_size,
					v2 = ((v+1)&n_size_m1)*n_size,
					u0 = ((u-1)&n_size_m1),
					u1 = u,
					u2 = ((u+1)&n_size_m1),					
					f  = frame*n_size_sq;
				float temp = (1.0f/14.0f) * (	tempnoise[f + v0 + u0] + tempnoise[f + v0 + u1] + tempnoise[f + v0 + u2] +
										tempnoise[f + v1 + u0] + 6.0f*tempnoise[f + v1 + u1] + tempnoise[f + v1 + u2] +
										tempnoise[f + v2 + u0] + tempnoise[f + v2 + u1] + tempnoise[f + v2 + u2]);
									  
				this->noise[frame*n_size_sq + v*n_size + u] = noise_magnitude*temp;
			}
		}
	}	
	
}

//--------------------------------------------------------------------------------------
// Update noise tables on CPU
//--------------------------------------------------------------------------------------
void Software_Noisemaker::calc_noise()
{
	octaves = min(iOctaves, max_octaves);		

	// calculate the strength of each octave
	float sum=0.0f;
	for(int i=0; i<octaves; i++)
	{
		f_multitable[i] = powf(fFalloff,1.0f*i);
		sum += f_multitable[i];
	}

	{
	for(int i=0; i<octaves; i++)
	{
		f_multitable[i] /= sum;
	}}
	
	{
	for(int i=0; i<octaves; i++)
	{
		multitable[i] = scale_magnitude*f_multitable[i];
	}}
	

	DWORD this_time = timeGetTime();
	double itime = this_time - last_time;
	static double lp_itime=0.0;	
	last_time = this_time;
	itime *= 0.001 * fAnimspeed ;
	lp_itime = 0.99*lp_itime + 0.01 * itime;
	if ( !bPaused )
		time += lp_itime;			

	
	double	r_timemulti = 1.0;

	for(int o=0; o<octaves; o++)
	{		
		unsigned int image[3];
		int amount[3];
		double dImage, fraction = modf(time*r_timemulti,&dImage);
		int iImage = (int)dImage;
		amount[0] = scale_magnitude*f_multitable[o]*(pow(sin((fraction+2)*PI/3),2)/1.5);
		amount[1] = scale_magnitude*f_multitable[o]*(pow(sin((fraction+1)*PI/3),2)/1.5);
		amount[2] = scale_magnitude*f_multitable[o]*(pow(sin((fraction)*PI/3),2)/1.5);
		image[0] = (iImage) & noise_frames_m1;
		image[1] = (iImage+1) & noise_frames_m1;
		image[2] = (iImage+2) & noise_frames_m1;
		{	
			for(int i=0; i<n_size_sq; i++)
			{
				o_noise[i + n_size_sq*o] =	(	((amount[0] * noise[i + n_size_sq * image[0]])>>scale_decimalbits) + 
												((amount[1] * noise[i + n_size_sq * image[1]])>>scale_decimalbits) + 
												((amount[2] * noise[i + n_size_sq * image[2]])>>scale_decimalbits));
			}
		}

		r_timemulti *= fTimemulti;
	}

	if(packednoise)
	{
		int octavepack = 0;
		for(int o=0; o<octaves; o+=n_packsize)
		{
			for(int v=0; v<np_size; v++)
			for(int u=0; u<np_size; u++)
			{
				p_noise[v*np_size+u+octavepack*np_size_sq] = o_noise[(o+3)*n_size_sq + (v&n_size_m1)*n_size + (u&n_size_m1)];
				p_noise[v*np_size+u+octavepack*np_size_sq] += mapsample( u, v, 3, o);
				p_noise[v*np_size+u+octavepack*np_size_sq] += mapsample( u, v, 2, o+1);
				p_noise[v*np_size+u+octavepack*np_size_sq] += mapsample( u, v, 1, o+2);				
			}
			octavepack++;

			/*for(int v=0; v<20; v++)
			for(int u=0; u<20; u++)
				p_noise[v*np_size+u] = 1000;*/
			// debug box
			
		}
	}
}

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
inline int Software_Noisemaker::mapsample(int u, int v, int upsamplepower, int octave)
{
	int magnitude = 1<<upsamplepower;
	int pu = u >> upsamplepower;
	int pv = v >> upsamplepower;	
	int fu = u & (magnitude-1);
	int fv = v & (magnitude-1);
	int fu_m = magnitude - fu;
	int fv_m = magnitude - fv;

	int o = fu_m*fv_m*o_noise[octave*n_size_sq + ((pv)&n_size_m1)*n_size + ((pu)&n_size_m1)] +
			fu*fv_m*o_noise[octave*n_size_sq + ((pv)&n_size_m1)*n_size + ((pu+1)&n_size_m1)] +
			fu_m*fv*o_noise[octave*n_size_sq + ((pv+1)&n_size_m1)*n_size + ((pu)&n_size_m1)] +
			fu*fv*o_noise[octave*n_size_sq + ((pv+1)&n_size_m1)*n_size + ((pu+1)&n_size_m1)];

	return o >> (upsamplepower+upsamplepower);
}

Software_Noisemaker::~Software_Noisemaker()
{
	delete [] vertices;
}


//--------------------------------------------------------------------------------------
// Calculates the physical displacement for the mesh
//--------------------------------------------------------------------------------------
bool Software_Noisemaker::render_geometry(const D3DXMATRIXA16 *m)
{
	this->calc_noise();

	float magnitude = n_dec_magn * fScale;
	float inv_magnitude_sq = 1.0f/(fScale*fScale);

	D3DXMATRIXA16 m_inv;
	D3DXMatrixInverse( &m_inv, NULL, m );
	D3DXVec3TransformNormal( &e_u, &D3DXVECTOR3(1,0,0), m);
	D3DXVec3TransformNormal( &e_v, &D3DXVECTOR3(0,1,0), m);
	D3DXVec3Normalize( &e_u, &e_u );
	D3DXVec3Normalize( &e_v, &e_v );


	t_corners0 = this->calc_worldpos(D3DXVECTOR2(0.0f,0.0f),m);
	t_corners1 = this->calc_worldpos(D3DXVECTOR2(+1.0f,0.0f),m);
	t_corners2 = this->calc_worldpos(D3DXVECTOR2(0.0f,+1.0f),m);
	t_corners3 = this->calc_worldpos(D3DXVECTOR2(+1.0f,+1.0f),m);

	D3DXMATRIXA16 surface_to_world;


	float	du = 1.0f/float(sizeX-1),
		dv = 1.0f/float(sizeY-1),
		u,v=0.0f;
	D3DXVECTOR4 result;
	int i=0;
	for(int iv=0; iv<sizeY; iv++)
	{
		u = 0.0f;		
		for(int iu=0; iu<sizeX; iu++)
		{				

			//result = (1.0f-v)*( (1.0f-u)*t_corners0 + u*t_corners1 ) + v*( (1.0f-u)*t_corners2 + u*t_corners3 );				
			result.x = (1.0f-v)*( (1.0f-u)*t_corners0.x + u*t_corners1.x ) + v*( (1.0f-u)*t_corners2.x + u*t_corners3.x );				
			result.z = (1.0f-v)*( (1.0f-u)*t_corners0.z + u*t_corners1.z ) + v*( (1.0f-u)*t_corners2.z + u*t_corners3.z );				
			result.w = (1.0f-v)*( (1.0f-u)*t_corners0.w + u*t_corners1.w ) + v*( (1.0f-u)*t_corners2.w + u*t_corners3.w );				

			float divide = 1.0f/result.w;				
			result.x *= divide;
			result.z *= divide;

			vertices[i].x = result.x;
			vertices[i].z = result.z;
			//vertices[i].y = GetHeightAt(magnitude*result.x, magnitude*result.z, octaves);
			vertices[i].y = get_height_dual(magnitude*result.x, magnitude*result.z );

			// TIM: Multiply by dist for falloff model
			// Only if dampening is lower than actual wave
			float damp = vertices[i].y*(Vector(result.x,0,result.z).Length()/(fDampeningRadius+0.1f));
			if(fabsf(damp) < fabsf(vertices[i].y))
            {
				vertices[i].y = damp;
            }

			i++;
			u += du;
		}
		v += dv;			
	}

	// smooth the heightdata
	if(bSmooth)
	{
		//for(int n=0; n<3; n++)
		for(int v=1; v<(sizeY-1); v++)
		{
			for(int u=1; u<(sizeX-1); u++)
			{				
				vertices[v*sizeX + u].y =	0.2f * (vertices[v*sizeX + u].y +
					vertices[v*sizeX + (u+1)].y + 
					vertices[v*sizeX + (u-1)].y + 
					vertices[(v+1)*sizeX + u].y + 
					vertices[(v-1)*sizeX + u].y);															
			}
		}
	}

	if(!bDisplace)
	{
		// reset height to 0
		for(int u=0; u<(sizeX*sizeY); u++)
		{
			vertices[u].y = 0;
		}

	}


	#ifdef CPU_NORMALS
	calc_normals();	
	#else
	this->upload_noise();
	#endif	

	return true;
}


//--------------------------------------------------------------------------------------
// check the point of intersection with the plane (0,1,0,0) and return the position in homogenous coordinates 
//--------------------------------------------------------------------------------------
D3DXVECTOR4 Software_Noisemaker::calc_worldpos(D3DXVECTOR2 uv, const D3DXMATRIXA16 *m)
{	
	// this is hacky.. this does take care of the homogenous coordinates in a correct way, 
	// but only when the plane lies at y=0
	D3DXVECTOR4	origin(uv.x,uv.y,-1,1);
	D3DXVECTOR4	direction(uv.x,uv.y,1,1);

	D3DXVec4Transform( &origin, &origin, m );
	D3DXVec4Transform( &direction, &direction, m );
	direction -= origin;    

	float	l = -origin.y / direction.y;	// assumes the plane is y=0

	D3DXVECTOR4 worldPos = origin + direction*l;    
	return worldPos;
}

//--------------------------------------------------------------------------------------
// Calculate vertex normals for noise map
//--------------------------------------------------------------------------------------
void Software_Noisemaker::calc_normals()
{
	for(int v=1; v<(sizeY-1); v++)
	{
		for(int u=1; u<(sizeX-1); u++)
		{
			D3DXVECTOR3 vec1(	vertices[v*sizeX + u + 1].x-vertices[v*sizeX + u - 1].x,
				vertices[v*sizeX + u + 1].y-vertices[v*sizeX + u - 1].y, 
				vertices[v*sizeX + u + 1].z-vertices[v*sizeX + u - 1].z);

			D3DXVECTOR3 vec2(	vertices[(v+1)*sizeX + u].x - vertices[(v-1)*sizeX + u].x,
				vertices[(v+1)*sizeX + u].y - vertices[(v-1)*sizeX + u].y,
				vertices[(v+1)*sizeX + u].z - vertices[(v-1)*sizeX + u].z);
			D3DXVECTOR3 normal;
			D3DXVec3Cross( &normal, &vec2, &vec1 );
			vertices[v*sizeX + u].nx = normal.x;
			vertices[v*sizeX + u].ny = normal.y;
			vertices[v*sizeX + u].nz = normal.z;

		}
	}
}

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
inline int Software_Noisemaker::readtexel_linear(int u, int v, int offset)
{
	int iu, iup, iv, ivp, fu, fv;
	iu = (u>>n_dec_bits)&n_size_m1;
	iv = ((v>>n_dec_bits)&n_size_m1)*n_size;

	iup = (((u>>n_dec_bits) + 1)&n_size_m1);
	ivp = (((v>>n_dec_bits) + 1)&n_size_m1)*n_size;

	fu = u & n_dec_magn_m1;
	fv = v & n_dec_magn_m1;
	/*float f_fu = (float) fu / n_dec_magn;
	float f_fv = (float) fv / n_dec_magn;*/
	/*float ut01 = ((n_dec_magn_m1-fu)*o_noise[iv + iu + offset] + fu*o_noise[iv + iup + offset]);
	float ut23 = ((n_dec_magn_m1-fu)*o_noise[ivp + iu + offset] + fu*o_noise[ivp + iup + offset]);*/

	int ut01 = ((n_dec_magn-fu)*o_noise[offset + iv + iu] + fu*o_noise[offset + iv + iup]) >> n_dec_bits;
	int ut23 = ((n_dec_magn-fu)*o_noise[offset + ivp + iu] + fu*o_noise[offset + ivp + iup]) >> n_dec_bits ;
	int ut = ((n_dec_magn-fv)*ut01 + fv*ut23) >> n_dec_bits;
	return ut;
}

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
inline float Software_Noisemaker::GetHeightAt(int u, int v, int octaves)
{	
	int value=0;	
	//r_noise = o_noise;	// pointer to the current noise source octave
	for(int i=0; i<octaves; i++)
	{		
		value += readtexel_linear(u,v,i*n_size_sq);
		u = u << 1;
		v = v << 1;
		//r_noise += n_size_sq;
	}		
	return (float)(value)*fStrength / noise_magnitude;
}

float Software_Noisemaker::GetHeightAt(float u, float v)
{
	float magnitude = n_dec_magn * fScale;
	//return GetHeightAt(magnitude*u, magnitude*v, octaves);
	return get_height_dual(magnitude*u, magnitude*v);
}

// Little function to encapsulate
//
bool CreateMaps(int x, int y, LPDIRECT3DTEXTURE9& heightmap, LPDIRECT3DTEXTURE9& normalmap, D3DFORMAT fmt)
{
    if(FAILED(RenderWrap::dev->CreateTexture(x,y,1,D3DUSAGE_RENDERTARGET, fmt, D3DPOOL_DEFAULT, &(heightmap),NULL)))
        return false;

    if(FAILED(RenderWrap::dev->CreateTexture(x,y,1,D3DUSAGE_AUTOGENMIPMAP|D3DUSAGE_RENDERTARGET, fmt, D3DPOOL_DEFAULT, &(normalmap),NULL)))
        return false;

    return true;
}

//--------------------------------------------------------------------------------------
// Initialize noise textures, and noise normal textures, and a depth buffer
//--------------------------------------------------------------------------------------
void Software_Noisemaker::init_textures()
{
	// the noise textures. currently two of them (= 8 levels)
	RenderWrap::dev->CreateTexture(np_size,np_size,0,D3DUSAGE_DYNAMIC, D3DFMT_L16, D3DPOOL_DEFAULT, &(this->packed_noise_texture[0]),NULL);	
	RenderWrap::dev->CreateTexture(np_size,np_size,0,D3DUSAGE_DYNAMIC, D3DFMT_L16, D3DPOOL_DEFAULT, &(this->packed_noise_texture[1]),NULL);

    // Try one format, fall back to other format
	if(!CreateMaps(nmapsize_x,nmapsize_y,heightmap,normalmap,D3DFMT_A16B16G16R16))
        if(!CreateMaps(nmapsize_x,nmapsize_y,heightmap,normalmap,D3DFMT_A16B16G16R16F))
            if(!CreateMaps(nmapsize_x,nmapsize_y,heightmap,normalmap,D3DFMT_A8R8G8B8))
                SeriousWarning("Your card does not support any compatible render-target needed for water rendering");
  
	// create z/stencil-buffer
	RenderWrap::dev->CreateDepthStencilSurface( nmapsize_x, nmapsize_y,D3DFMT_D24S8, D3DMULTISAMPLE_NONE, 0, true, &depthstencil, NULL );
}

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void Software_Noisemaker::FreeTextures()
{
	SAFE_RELEASE(packed_noise_texture[0]);
	SAFE_RELEASE(packed_noise_texture[1]);
	SAFE_RELEASE(heightmap);
	SAFE_RELEASE(normalmap);
	SAFE_RELEASE(depthstencil);
}

//--------------------------------------------------------------------------------------
// if CPU_NORMALS is not defined this is called to write to the texture
//--------------------------------------------------------------------------------------
void Software_Noisemaker::upload_noise()
{
	D3DLOCKED_RECT locked;
	unsigned short *data;
	int tempdata[np_size_sq];
	for(int t=0; t<2; t++)
	{
		int offset = np_size_sq*t;
		// upload the first level
		packed_noise_texture[t]->LockRect( 0, &locked, NULL, D3DLOCK_DISCARD );
		data = (unsigned short*)locked.pBits;
		for(int i=0; i<np_size_sq; i++)
			data[i] = 32768+p_noise[i+offset];
		packed_noise_texture[t]->UnlockRect( 0 );

		int c = packed_noise_texture[t]->GetLevelCount();

		// calculate the second level, and upload it
		HRESULT hr = packed_noise_texture[t]->LockRect( 1, &locked, NULL, 0 );
		data = (unsigned short*)locked.pBits;		
		int sz = np_size>>1;
		for(int v=0; v<sz; v++){
			for(int u=0; u<sz; u++)
			{				
				tempdata[v*np_size + u] = (p_noise[((v<<1))*np_size + (u<<1)+offset] + p_noise[((v<<1))*np_size + (u<<1) + 1+offset] +
										   p_noise[((v<<1)+1)*np_size + (u<<1)+offset] + p_noise[((v<<1)+1)*np_size + (u<<1) + 1+offset])>>2;
				data[v*sz+u] = 32768+tempdata[v*np_size + u];
			}
		}

		packed_noise_texture[t]->UnlockRect( 1 );		
		
		for(int j=2; j<c; j++)
		{
			hr = packed_noise_texture[t]->LockRect( j, &locked, NULL, 0 );
			data = (unsigned short*)locked.pBits;
			int pitch = (locked.Pitch)>>1;
			sz = np_size>>j;			
			for(int v=0; v<sz; v++){
				for(int u=0; u<sz; u++)
				{
					tempdata[v*np_size + u] =	(tempdata[((v<<1))*np_size + (u<<1)] + tempdata[((v<<1))*np_size + (u<<1) + 1] +
												tempdata[((v<<1)+1)*np_size + (u<<1)] + tempdata[((v<<1)+1)*np_size + (u<<1) + 1])>>2;
					data[v*pitch+u] = 32768+tempdata[v*np_size + u];
				}
			}		
			packed_noise_texture[t]->UnlockRect( j );
		}
	}
}


//--------------------------------------------------------------------------------------
// Normals for the displaced water
//--------------------------------------------------------------------------------------
void Software_Noisemaker::generate_normalmap( )
{
#ifndef CPU_NORMALS
	HRESULT hr;
	
	// do the heightmap thingy
	LPDIRECT3DSURFACE9 target,bb,old_depthstencil;
	hr = RenderWrap::dev->GetRenderTarget(0, &bb );
	hr = heightmap->GetSurfaceLevel( 0,&target );
	RenderWrap::dev->GetDepthStencilSurface( &old_depthstencil );	

	hr = RenderWrap::dev->SetRenderTarget( 0, target );
	SAFE_RELEASE(target);
	RenderWrap::dev->SetDepthStencilSurface( depthstencil );


	//hr = RenderWrap::dev->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW  );	
	//RenderWrap::dev->SetStreamSource( 0, m_SurfVertices, 0, sizeof(SOFTWARESURFACEVERTEX) );
	hr = RenderWrap::dev->SetFVF( D3DFVF_SOFTWARESURFACEVERTEX);			
	//RenderWrap::dev->SetIndices(surf->surf_indicies);
	hr = hmap_effect.GetEffect()->Begin(NULL,NULL);
	hmap_effect.GetEffect()->BeginPass(0);				
	hmap_effect.GetEffect()->SetFloat("scale", fScale);
	
	hmap_effect.GetEffect()->SetTexture("noise0",packed_noise_texture[0]);
	hmap_effect.GetEffect()->SetTexture("noise1",packed_noise_texture[1]);
	
	hmap_effect.GetEffect()->CommitChanges();
	//RenderWrap::dev->Clear( 0, NULL,D3DCLEAR_TARGET, D3DCOLOR_XRGB(255,128,28), 1.0f, 0 );
	RenderWrap::dev->SetRenderState( D3DRS_ZENABLE, D3DZB_FALSE );
	RenderWrap::dev->DrawIndexedPrimitive(	D3DPT_TRIANGLELIST, 0,	0, sizeX*sizeY, 0, 2*(sizeX-1)*(sizeY-1) );			
	hmap_effect.GetEffect()->EndPass();
	hmap_effect.GetEffect()->End();
	
	// calculate normalmap

	hr = normalmap->GetSurfaceLevel( 0,&target );
	hr = RenderWrap::dev->SetRenderTarget( 0, target );
	SAFE_RELEASE(target);
	hr = nmap_effect.GetEffect()->Begin(NULL,NULL);
	nmap_effect.GetEffect()->BeginPass(0);				
	nmap_effect.GetEffect()->SetFloat("inv_mapsize_x", 1.0f/nmapsize_x);
	nmap_effect.GetEffect()->SetFloat("inv_mapsize_y", 1.0f/nmapsize_y);
	nmap_effect.GetEffect()->SetVector("corner00", &t_corners0 );
	nmap_effect.GetEffect()->SetVector("corner01", &t_corners1 );
	nmap_effect.GetEffect()->SetVector("corner10", &t_corners2 );
	nmap_effect.GetEffect()->SetVector("corner11", &t_corners3 );
	nmap_effect.GetEffect()->SetFloat("amplitude", 2*fStrength);
	nmap_effect.GetEffect()->SetTexture("hmap",heightmap);
	nmap_effect.GetEffect()->CommitChanges();
	RenderWrap::dev->DrawIndexedPrimitive(	D3DPT_TRIANGLELIST, 0,	0, sizeX*sizeY, 0, 2*(sizeX-1)*(sizeY-1) );			
    nmap_effect.GetEffect()->EndPass();
	nmap_effect.GetEffect()->End();

	// restore the device
	RenderWrap::dev->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE );

	RenderWrap::dev->SetRenderTarget( 0, bb );
	RenderWrap::dev->SetDepthStencilSurface( old_depthstencil );
	SAFE_RELEASE(bb);
	SAFE_RELEASE(old_depthstencil);

#endif
}

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void Software_Noisemaker::load_effects()
{
	D3DXHANDLE hTechnique;

	// load effect
	hmap_effect.Load("../shaders/fx/v2_heightmapgen.fx");
	hmap_effect.GetEffect()->FindNextValidTechnique(NULL, &hTechnique);    
	hmap_effect.GetEffect()->SetTechnique(hTechnique);

	nmap_effect.Load("../shaders/fx/v2_normalmapgen.fx");
	nmap_effect.GetEffect()->FindNextValidTechnique(NULL, &hTechnique);    
	nmap_effect.GetEffect()->SetTechnique(hTechnique);
}

