//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
// Generates noise in hardware. Requires shaders 3.0
//
//====================================================================================
#include "stdafx.h"
#include "noisemaker.h"
#include <math.h>
#include <mmsystem.h>


noisemaker::noisemaker(const LPDIRECT3DDEVICE9 device, int sizeX, int sizeY, int octaves, int animlength, parameterhandler *prm)
{
	this->device		= device;
	this->animlength	= 256; //animlength;		// must be power of two.. add checking later
	this->octaves		= octaves;
	patches_x			= 32;
	patches_y			= 128;
	this->sizeX = sizeX;
	this->sizeY = sizeY;
	this->prm = prm;
	create_vertexbuffer();
	prepare_textures();
	LoadEffect();
}

noisemaker::~noisemaker()
{
	rendered_texture->Release();
	rendered_normalmap->Release();
	RenderToSurface->Release();
	RenderToNormalmap->Release();
	rendered_texture_surface->Release();
	VB->Release();
	IB->Release();
	source_texture->Release();
	rendered_normalmap_surface->Release();
	anim_effect->Release();
	normalmap_effect->Release();
}

void noisemaker::create_vertexbuffer()
{
	NOISEVERTEX *anim;

	// Create the vertex buffer.
	device->CreateVertexBuffer( (patches_x+1)*(patches_y+1)*sizeof(NOISEVERTEX),
		D3DUSAGE_WRITEONLY, D3DFVF_NOISEVERTEX,
		D3DPOOL_DEFAULT, &VB, NULL );

	// Fill the vertex buffer	
	int i = 0; 
	if( !FAILED( VB->Lock( 0, 0, (void**)&anim, 0 ) ) ){
		for(int v=0; v<(patches_y+1); v++){
			for(int u=0; u<(patches_x+1); u++)
			{
				anim[i++].position = D3DXVECTOR3((float)u/patches_x,(float)v/patches_y,0);
			}
		}
		VB->Unlock();
	}

	device->CreateIndexBuffer(	sizeof(unsigned int) * 6 * (patches_x)*(patches_y),
		D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX32,	D3DPOOL_DEFAULT,&IB,NULL);

	unsigned int *indexbuffer;
	if( !FAILED( IB->Lock(0,0,(void**)&indexbuffer,0 ) ) )
	{
		i = 0;
		for(int v=0; v<patches_y; v++){
			for(int u=0; u<patches_x; u++){
				// face 1 |/
				indexbuffer[i++]	= v*(patches_x+1) + u;
				indexbuffer[i++]	= v*(patches_x+1) + u + 1;
				indexbuffer[i++]	= (v+1)*(patches_x+1) + u;

				// face 2 /|
				indexbuffer[i++]	= (v+1)*(patches_x+1) + u;
				indexbuffer[i++]	= v*(patches_x+1) + u + 1;
				indexbuffer[i++]	= (v+1)*(patches_x+1) + u + 1;
			}
		}
		IB->Unlock();
	}

	this->offsets = new tc[animlength];
	for( i=0; i<animlength; i++)
	{
		offsets[i].x = (float) rand()/RAND_MAX;
		offsets[i].y = (float) rand()/RAND_MAX;
	}

	// Create the quad vertex buffer
	
	device->CreateVertexBuffer( 6*sizeof(NOISEVERTEX),
		D3DUSAGE_WRITEONLY, D3DFVF_NOISEVERTEX,
		D3DPOOL_DEFAULT, &quad, NULL );
	
	NOISEVERTEX *qV;

	if( !FAILED( quad->Lock( 0, 0, (void**)&qV, 0 ) ) ){
	
		qV[0].position = D3DXVECTOR3(-1,-1,0);
		qV[1].position = D3DXVECTOR3(+1,-1,0);
		qV[2].position = D3DXVECTOR3(-1,+1,0);
		qV[3].position = D3DXVECTOR3(-1,+1,0);
		qV[4].position = D3DXVECTOR3(+1,-1,0);
		qV[5].position = D3DXVECTOR3(+1,+1,0);		

		quad->Unlock();
	}

}

void noisemaker::prepare_textures()
{
	// load source noise texture
	if( FAILED( D3DXCreateTextureFromFile( device, "textures/noise-uniform.png", &source_texture ) ) )
	{		
			MessageBox(NULL, "Could not find noise texture", "projgriddemo", MB_OK);		
	}
	HRESULT hr = device->CreateTexture(sizeX,sizeY,1,D3DUSAGE_RENDERTARGET, D3DFMT_R32F, D3DPOOL_DEFAULT, &(this->rendered_texture),NULL);
	
	D3DSURFACE_DESC desc;

	rendered_texture->GetSurfaceLevel( 0, &rendered_texture_surface );
	rendered_texture_surface->GetDesc( &desc );

	D3DXCreateRenderToSurface( device, 
		desc.Width, 
		desc.Height, 
		desc.Format, 
		FALSE, 
		D3DFMT_UNKNOWN, 
		&RenderToSurface );

	hr = device->CreateTexture(sizeX,sizeY,1,D3DUSAGE_RENDERTARGET, D3DFMT_A16B16G16R16, D3DPOOL_DEFAULT, &(this->rendered_normalmap),NULL);
	rendered_normalmap->GetSurfaceLevel( 0, &rendered_normalmap_surface );
	rendered_normalmap_surface->GetDesc( &desc );


	D3DXCreateRenderToSurface( device, 
		desc.Width, 
		desc.Height, 
		desc.Format, 
		FALSE, 
		D3DFMT_UNKNOWN, 
		&RenderToNormalmap );

	// create all the intermediary texture octaves
	for(int i=0; i<octaves; i++)
	{
		hr = device->CreateTexture(128,128,0,D3DUSAGE_RENDERTARGET, D3DFMT_A16B16G16R16, D3DPOOL_DEFAULT, &(this->noise_octaves[i]),NULL);	//D3DFMT_R32F
		noise_octaves[i]->GetSurfaceLevel( 0, &noise_octave_surface[i] );
	}
		
	noise_octave_surface[0]->GetDesc( &desc );

	D3DXCreateRenderToSurface( device, 
		desc.Width, 
		desc.Height, 
		desc.Format, 
		FALSE, 
		D3DFMT_UNKNOWN, 
		&RenderToOctaves );

}

void noisemaker::render_noise_octaves()
{
	
	DWORD itime = timeGetTime() & ((1<<23) - 1);

	
	double	r_timemulti = prm->params[p_fAnimspeed].fData;

	for(int i=0; i<octaves; i++)
	{
		if ( RenderToOctaves->BeginScene(noise_octave_surface[i],NULL) )
			{
			noise_octaves_effect->Begin(NULL,NULL);
			noise_octaves_effect->BeginPass(0);
	
			device->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
			device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);	
			device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
			device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
			unsigned int image[3];
			float amount[3];
			double dImage, fraction = modf((double)itime*r_timemulti,&dImage);
			int iImage = (int)dImage;
			amount[0] = pow(sin((fraction+2)*PI/3),2)/1.5;
			amount[1] = pow(sin((fraction+1)*PI/3),2)/1.5;
			amount[2] = pow(sin((fraction)*PI/3),2)/1.5;
			image[0] = (iImage) & (animlength-1);
			image[1] = (iImage+1) & (animlength-1);
			image[2] = (iImage+2) & (animlength-1);

			noise_octaves_effect->SetFloat("a",amount[0]);
			noise_octaves_effect->SetFloat("b",amount[1]);
			noise_octaves_effect->SetFloat("c",amount[2]);
			noise_octaves_effect->SetFloat("mapsize",128.0f);
			noise_octaves_effect->SetFloat("srcsize",512.0f);
			noise_octaves_effect->SetFloat("ratio",0.25f);			

			noise_octaves_effect->SetTexture("NoiseTex",source_texture);

			noise_octaves_effect->SetVector("tc_offset_a", &D3DXVECTOR4(offsets[image[0]].x,offsets[image[0]].y,0,1));
			noise_octaves_effect->SetVector("tc_offset_b", &D3DXVECTOR4(offsets[image[1]].x,offsets[image[1]].y,0,1));
			noise_octaves_effect->SetVector("tc_offset_c", &D3DXVECTOR4(offsets[image[2]].x,offsets[image[2]].y,0,1));
			noise_octaves_effect->CommitChanges();
			device->SetStreamSource( 0, quad, 0, sizeof(NOISEVERTEX) );
			device->SetFVF( D3DFVF_NOISEVERTEX );		
			device->DrawPrimitive( D3DPT_TRIANGLELIST, 0, 2);
			r_timemulti *= prm->params[p_fTimemulti].fData;			
			
			// end rendertarget
			noise_octaves_effect->EndPass();
			noise_octaves_effect->End();
			RenderToOctaves->EndScene(0);
			}
		}	
		
}

void noisemaker::LoadEffect(){

	char *errortext;
	LPD3DXBUFFER errors;
	// load the perlin noise effect
	D3DXCreateEffectFromFile(device, "noisemakermk2.fx", 
		NULL, NULL, 0, NULL, &anim_effect, &errors );
	if (errors != NULL){
		errortext = (char*) errors->GetBufferPointer();
		MessageBox(NULL, errortext, "Textures.exe", MB_OK);		
	}
	D3DXHANDLE hTechnique;
	anim_effect->FindNextValidTechnique(NULL, &hTechnique);    
	anim_effect->SetTechnique(hTechnique);

	// and the normalmap effect
	D3DXCreateEffectFromFile(device, "normalmapgenerator.fx", 
		NULL, NULL, 0, NULL, &normalmap_effect, &errors );
	if (errors != NULL){
		errortext = (char*) errors->GetBufferPointer();
		MessageBox(NULL, errortext, "Textures.exe", MB_OK);		
	}	
	normalmap_effect->FindNextValidTechnique(NULL, &hTechnique);    
	normalmap_effect->SetTechnique(hTechnique);

	// and the noise_octaves effect
	D3DXCreateEffectFromFile(device, "noise_octaves.fx", 
		NULL, NULL, 0, NULL, &noise_octaves_effect, &errors );
	if (errors != NULL){
		errortext = (char*) errors->GetBufferPointer();
		MessageBox(NULL, errortext, "Textures.exe", MB_OK);		
	}	
	noise_octaves_effect->FindNextValidTechnique(NULL, &hTechnique);    
	noise_octaves_effect->SetTechnique(hTechnique);

}

void noisemaker::render(const D3DXMATRIXA16 *mProjector)
{
	this->mProjector = *mProjector;

	if (RenderToSurface->BeginScene(rendered_texture_surface,NULL))
	{

		DWORD itime = timeGetTime() & ((1<<23) - 1);

		device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
		device->Clear(0, NULL, D3DCLEAR_TARGET , D3DCOLOR_XRGB(0,0,0), 1.0f, 0 );
		anim_effect->Begin(NULL,NULL);
		anim_effect->BeginPass(0);

		double r_scale	= prm->params[p_fScale].fData,
			r_timemulti = prm->params[p_fAnimspeed].fData,
			r_strength = 1.0f,
			r_falloff = prm->params[p_fFalloff].fData;
		// normalize r_strength
		float sum=0;
		for(int i=0; i<octaves; i++)
		{
			sum += r_strength;
			r_strength *= r_falloff;
		}
		r_strength = 1.0f / sum;

		for(int i=0; i<octaves; i++)
		{
			unsigned int image[3];
			float amount[3];
			double dImage, fraction = modf((double)itime*r_timemulti,&dImage);
			int iImage = (int)dImage;
			amount[0] = pow(sin((fraction+2)*PI/3),2)/1.5;
			amount[1] = pow(sin((fraction+1)*PI/3),2)/1.5;
			amount[2] = pow(sin((fraction)*PI/3),2)/1.5;
			image[0] = (iImage) & (animlength-1);
			image[1] = (iImage+1) & (animlength-1);
			image[2] = (iImage+2) & (animlength-1);


			anim_effect->SetFloat("a",amount[0]);
			anim_effect->SetFloat("b",amount[1]);
			anim_effect->SetFloat("c",amount[2]);
			anim_effect->SetFloat("scale",r_scale);
			anim_effect->SetFloat("strength",r_strength);

			anim_effect->SetVector("tc_offset_a", &D3DXVECTOR4(offsets[image[0]].x,offsets[image[0]].y,0,1));
			anim_effect->SetVector("tc_offset_b", &D3DXVECTOR4(offsets[image[1]].x,offsets[image[1]].y,0,1));
			anim_effect->SetVector("tc_offset_c", &D3DXVECTOR4(offsets[image[2]].x,offsets[image[2]].y,0,1));

			anim_effect->SetTexture("NoiseTex",source_texture);
			anim_effect->SetMatrix("mProjector",mProjector);
			anim_effect->CommitChanges();
			device->SetStreamSource( 0, VB, 0, sizeof(NOISEVERTEX) );
			device->SetFVF( D3DFVF_NOISEVERTEX );
			device->SetIndices( IB );
			device->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 	
				0,			
				0,
				(patches_x+1)*(patches_y+1),
				0,
				2*patches_x*patches_y-1 );

			r_scale *= 2;
			r_timemulti *= 1.5;
			r_strength *= r_falloff;		
		}
		RenderToSurface->EndScene(0);
		device->SetTexture(0,NULL);
		device->SetTexture(1,NULL);
		device->SetTexture(2,NULL);
		anim_effect->EndPass();
		anim_effect->End();
		device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
	}
}

void noisemaker::render_projected_noise(const D3DXMATRIXA16 *mProjector)
{
	this->mProjector = *mProjector;

	if (RenderToSurface->BeginScene(rendered_texture_surface,NULL))
	{

		anim_effect->Begin(NULL,NULL);
		anim_effect->BeginPass(0);

		float  r_scale	= prm->params[p_fScale].fData,			   
			   r_strength = 1.0f,
			   r_falloff = prm->params[p_fFalloff].fData;

		float scale[4], amplitude[4];

		// normalize r_strength
		float sum=0;
		for(int i=0; i<octaves; i++)
		{
			sum += r_strength;
			amplitude[i] = r_strength;
			r_strength *= r_falloff;
			scale[i] = r_scale;
			r_scale *= 2;
			
		}
		r_strength = 1.0f / sum;

					
		anim_effect->SetFloat("scale",r_scale);
		anim_effect->SetFloat("strength",r_strength);
		anim_effect->SetFloat("sum_inverse",0.5*r_strength);
		
		anim_effect->SetTexture("nOctave0",noise_octaves[0]);
		anim_effect->SetTexture("nOctave1",noise_octaves[1]);
		anim_effect->SetTexture("nOctave2",noise_octaves[2]);
		anim_effect->SetTexture("nOctave3",noise_octaves[3]);		
		anim_effect->SetFloatArray("scale",scale,4);
		anim_effect->SetFloatArray("amplitude",amplitude,4);
			
		anim_effect->SetMatrix("mProjector",mProjector);
		anim_effect->CommitChanges();
		device->SetStreamSource( 0, VB, 0, sizeof(NOISEVERTEX) );
		device->SetFVF( D3DFVF_NOISEVERTEX );
		device->SetIndices( IB );
		device->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 	
				0,			
				0,
				(patches_x+1)*(patches_y+1),
				0,
				2*patches_x*patches_y-1 );
		
		
		RenderToSurface->EndScene(0);
		anim_effect->EndPass();
		anim_effect->End();		
	}
}

void noisemaker::render_normalmap()
{

	if (RenderToNormalmap->BeginScene(rendered_normalmap_surface,NULL))
	{
		normalmap_effect->Begin(NULL,NULL);
		normalmap_effect->BeginPass(0);

		normalmap_effect->SetFloat("amplitude", prm->params[p_fStrength].fData);
		normalmap_effect->SetFloat("mapsize_x", this->sizeX);
		normalmap_effect->SetFloat("mapsize_y", this->sizeY);
		normalmap_effect->SetMatrix("mProjector",&mProjector);
		D3DXMATRIXA16 mInvProjector;
		D3DXMatrixInverse( &mInvProjector, NULL, &mProjector);
		normalmap_effect->SetMatrix("mInvProjector",&mInvProjector);
		
		device->Clear(0, NULL, D3DCLEAR_TARGET , D3DCOLOR_XRGB(0,0,0), 1.0f, 0 );
		normalmap_effect->CommitChanges();
		device->SetStreamSource( 0, VB, 0, sizeof(NOISEVERTEX) );
		device->SetFVF( D3DFVF_NOISEVERTEX );
		normalmap_effect->SetTexture("Heightmap",rendered_texture);
		device->SetIndices( IB );
		device->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0,
				(patches_x+1)*(patches_y+1), 0, 2*patches_x*patches_y-1 );
				
		normalmap_effect->EndPass();
		normalmap_effect->End();
		
		RenderToNormalmap->EndScene(0);
	}
}