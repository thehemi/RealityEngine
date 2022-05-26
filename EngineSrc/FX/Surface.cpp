//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
// Patch surface, for moving water
// Uses algorithm from http://graphics.cs.lth.se/theses/projects/projgrid/projgrid-hq.pdf
//
//====================================================================================
#include "stdafx.h"
#include "surface.h"
#include <stdio.h>
#include "SkyController.h"

//--------------------------------------------------------------------------------------
// Misc helper functions

// add support for logical XOR 
#define log_xor || log_xor_helper() ||

struct log_xor_helper {
    bool value;
};

template<typename LEFT>
log_xor_helper &operator ||(const LEFT &left, log_xor_helper &xor) {
    xor.value = (bool)left;
    return xor;
}

template<typename RIGHT>
bool operator ||(const log_xor_helper &xor, const RIGHT &right) {
    return xor.value ^ (bool)right;
}

float gaussian(float x, float d)
{
	return exp(-x*x/(2*d*d))/sqrt(2*PI*d);
}

struct DVERTEX
{
	D3DXVECTOR3	position;
	float u,v;
};

#define D3DFVF_DVERTEX (D3DFVF_XYZ|D3DFVF_TEX1)


// render a small texture overlay
void debug_render_quad(const LPDIRECT3DTEXTURE9 texture, int n)
{
	LPDIRECT3DVERTEXBUFFER9		quad;

	// Create the quad vertex buffer
	
	RenderWrap::dev->CreateVertexBuffer( 6*sizeof(DVERTEX),
		D3DUSAGE_WRITEONLY, D3DFVF_DVERTEX,
		D3DPOOL_DEFAULT, &quad, NULL );
	
	DVERTEX *qV;

	int ix = n%4,
		iy = n/4;

	float	fx = (ix / 4.0f)*2.0f - 1.0f,
			fy = (iy / 4.0f)*2.0f - 1.0f,
			d = 2.0f / 4.0f;


	if( !FAILED( quad->Lock( 0, 0, (void**)&qV, 0 ) ) ){
	
		qV[0].position = D3DXVECTOR3(fx,	fy,		0);
		qV[1].position = D3DXVECTOR3(fx+d,	fy,		0);
		qV[2].position = D3DXVECTOR3(fx,	fy+d,	0);
		qV[3].position = D3DXVECTOR3(fx,	fy+d,	0);
		qV[4].position = D3DXVECTOR3(fx+d,	fy,		0);
		qV[5].position = D3DXVECTOR3(fx+d,	fy+d,	0);		

		qV[0].u = 0;	qV[0].v = 0;
		qV[1].u = +1;	qV[1].v = 0;
		qV[2].u = 0;	qV[2].v = +1;
		qV[3].u = 0;	qV[3].v = +1;
		qV[4].u = +1;	qV[4].v = 0;
		qV[5].u = +1;	qV[5].v = +1;

		quad->Unlock();
	}
	
	RenderWrap::dev->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
	RenderWrap::dev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);	
	RenderWrap::dev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	RenderWrap::dev->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

	RenderWrap::dev->SetFVF(D3DFVF_DVERTEX);
	RenderWrap::dev->SetStreamSource( 0, quad, 1, sizeof(DVERTEX) );
	RenderWrap::dev->SetTexture( 0, texture);

	D3DXMATRIXA16 id;
	D3DXMatrixIdentity( &id );
	RenderWrap::dev->SetTransform( D3DTS_VIEW, &id );
	RenderWrap::dev->SetTransform( D3DTS_PROJECTION, &id );
	
	RenderWrap::dev->DrawPrimitive( D3DPT_TRIANGLELIST, 0, 2);

	quad->Release();
	RenderWrap::dev->SetTexture( 0, NULL);
	RenderWrap::dev->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
	//RenderWrap::dev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);	
	//RenderWrap::dev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	RenderWrap::dev->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);

}

Surface::Surface(){
    m_ReflectionTex=0;
	m_RefractionTex=0;
	depthstencil=0;
}

//--------------------------------------------------------------------------------------
// Position of water plane
//--------------------------------------------------------------------------------------
void Surface::SetPosition(const D3DXVECTOR3* pos)
{
    D3DXPlaneFromPointNormal( &plane, pos, &normal);
    m_Pos = *pos;
}

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void Surface::Initialize(const D3DXVECTOR3 *pos, 
				 const D3DXVECTOR3 *n, 
				 int size_x, 
				 int size_y, 
				 int tex_size)
{
	m_TargetSize = tex_size,
	initialized = true;
	D3DXPlaneFromPointNormal( &plane, pos, n);
	D3DXVec3Normalize(&(this->normal), n);

    m_Pos = *pos;
	// calculate the u and v-vectors
	// take one of two vectors (the one further away from the normal) and force it into the plane
	D3DXVECTOR3 x;
	if(fabs( D3DXVec3Dot(&D3DXVECTOR3(1,0,0),&normal)) < fabs(D3DXVec3Dot(&D3DXVECTOR3(0,0,1),&normal))){
		x = D3DXVECTOR3(1,0,0);
	} else {
		x = D3DXVECTOR3(0,0,1);
	}
	u = x - normal*D3DXVec3Dot(&normal,&x);
	D3DXVec3Normalize(&u,&u);
	// get v (cross)
	D3DXVec3Cross(&v,&u,&normal);

	this->pos = *pos;
	this->m_GridX = size_x+1;
	this->m_GridY = size_y+1;
	this->m_ProjectingCamera = NULL;
	this->rendermode = RM_SOLID;
	this->boxfilter = false;


	SetDisplacementAmplitude(0.0f);

	if (!InitializeData()) initialized = false;		// init vertex & indexbuffers

	m_NoiseMaker = new Software_Noisemaker(m_GridX,m_GridY);

	LoadEffect();
}

//--------------------------------------------------------------------------------------
// Loads water/underwater shaders
//--------------------------------------------------------------------------------------
void Surface::LoadEffect(){
	D3DXHANDLE hTechnique;

	// same for m_Surf_software_effect
    
#ifdef CPU_NORMALS
	D3DXCreateEffectFromFile(RenderWrap::dev, "water_soft.fx", 
		NULL, NULL, 0, NULL, &m_Surf_software_effect, &errors );
#else 
	m_Surf_software_effect.Load("../shaders/fx/DynamicWater.fx");
	//D3DXCreateEffectFromFile(RenderWrap::dev, "../shaders/fx/water_R300.fx", 
	//	NULL, NULL, 0, NULL, &m_Surf_software_effect, &errors );
#endif

	m_Surf_software_effect.GetEffect()->FindNextValidTechnique(NULL, &hTechnique);    
	m_Surf_software_effect.GetEffect()->SetTechnique(hTechnique);

	
	//	underwater_software_effect
	underwater_software_effect.Load("../shaders/fx/underwater_soft.fx");
	underwater_software_effect.GetEffect()->FindNextValidTechnique(NULL, &hTechnique);    
	underwater_software_effect.GetEffect()->SetTechnique(hTechnique);
}

//--------------------------------------------------------------------------------------
// Prepare the vertex and indexbuffer with a uniform grid (dependant on the size parameter)	
//--------------------------------------------------------------------------------------
bool Surface::InitializeData(bool buffersOnly, bool targetsOnly)
{
	
	// Textures size must always be less than viewport, or rendering fails
	int texSize = m_TargetSize;
	while(texSize > RenderDevice::Instance()->GetViewportX() || texSize > RenderDevice::Instance()->GetViewportY())
		texSize /= 2;

    SAFE_RELEASE(m_ReflectionTex);
	SAFE_RELEASE(m_RefractionTex);
	SAFE_RELEASE(depthstencil);

	RenderWrap::dev->CreateTexture(texSize,texSize,1,D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_ReflectionTex,NULL);	
	RenderWrap::dev->CreateTexture(texSize,texSize,1,D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_RefractionTex,NULL);		
	RenderWrap::dev->CreateDepthStencilSurface( texSize, texSize,D3DFMT_D24S8, D3DMULTISAMPLE_NONE, 0, true, &depthstencil, NULL );

	if(targetsOnly)
	{	
		return true;
	}
	
	// create the vertexbuffer used in the softwaremode (it can be empty as it'll be memcpy-ed to)
	if( FAILED( RenderWrap::dev->CreateVertexBuffer( m_GridX*m_GridY*sizeof(SOFTWARESURFACEVERTEX),
		D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC, D3DFVF_SOFTWARESURFACEVERTEX,
		D3DPOOL_DEFAULT, &m_SurfVertices, NULL ) ) )
	{
		return false;
	}		

	// create/fill the indexbuffer

	if(	FAILED( RenderWrap::dev->CreateIndexBuffer(	sizeof(unsigned int) * 6 * (m_GridX-1)*(m_GridY-1),
		D3DUSAGE_WRITEONLY,			
		D3DFMT_INDEX32,	D3DPOOL_DEFAULT,&m_SurfIndices,NULL)))
	{
		return false;
	}
	unsigned int *indexbuffer;
	if( FAILED( m_SurfIndices->Lock(0,0,(void**)&indexbuffer,0 ) ) )
		return false;
	int i = 0;
	{
		for(int v=0; v<m_GridY-1; v++){
			for(int u=0; u<m_GridX-1; u++){
				// face 1 |/
				indexbuffer[i++]	= v*m_GridX + u;
				indexbuffer[i++]	= v*m_GridX + u + 1;
				indexbuffer[i++]	= (v+1)*m_GridX + u;

				// face 2 /|
				indexbuffer[i++]	= (v+1)*m_GridX + u;
				indexbuffer[i++]	= v*m_GridX + u + 1;
				indexbuffer[i++]	= (v+1)*m_GridX + u + 1;
			}
		}
	}
	m_SurfIndices->Unlock();

	if(buffersOnly)
		return true;

    string t1 = "../textures/core/water/fresnel_water_linear.bmp";
    string t2 = "../textures/core/water/reflection_underwater.bmp";
    string t3 = "../textures/core/water/XZnoise.png";
    FindMedia(t1,"Textures");
    FindMedia(t2,"Textures");
    FindMedia(t3,"Textures");

	if( FAILED( D3DXCreateTextureFromFile( RenderWrap::dev, t1.c_str() , &m_Surf_fresnel ) ) )
	{
		MessageBox(NULL, "Could not find fresnelmap", "Reality Engine", MB_OK);
		initialized = false;		
	}
	if( FAILED( D3DXCreateTextureFromFile( RenderWrap::dev, t2.c_str(), &underwater_fresnel ) ) )
	{
		MessageBox(NULL, "Could not find underwater fresnelmap", "Reality Engine", MB_OK);
		initialized = false;		
	}
	if( FAILED( D3DXCreateTextureFromFile( RenderWrap::dev, t3.c_str(), &noise2D ) ) )
	{
		MessageBox(NULL, "Could not find noise texture", "Reality Engine", MB_OK);
		initialized = false;		
	}

	if(!initialized)
		MessageBox(NULL, "Something went wrong in initialization of the class surface", "Reality Engine", MB_OK);


	return true;
}

//--------------------------------------------------------------------------------------
// Frees data
//--------------------------------------------------------------------------------------
void Surface::FreeData()
{
	// Buffers
	SAFE_RELEASE(m_SurfVertices);
	SAFE_RELEASE(m_SurfIndices);
	// Textures
	SAFE_RELEASE(m_Surf_fresnel);
	SAFE_RELEASE(underwater_fresnel);
	SAFE_RELEASE(noise2D);
	// Rendertargets
	SAFE_RELEASE(m_ReflectionTex);
	SAFE_RELEASE(m_RefractionTex);
	SAFE_RELEASE(depthstencil);
}

//--------------------------------------------------------------------------------------
// Only used for efficiency calc, can be removed
//--------------------------------------------------------------------------------------
bool Surface::WithinFrustum(const D3DXVECTOR3 *pos){
	D3DXVECTOR3 test;
	D3DXVec3TransformCoord(&test, pos, (D3DXMATRIX*)&(m_ObservingCamera.viewProjection));
	if((fabs(test.x) < 1.00001f)&&(fabs(test.y) < 1.00001f)&&(fabs(test.z) < 1.00001f))
		return true;
	return false;
}

float dispmulti(float dist){
	return largest(0, smallest(1, dist-1));
}


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
bool Surface::Prepare(Camera* cam)
{
	if(!initialized) return false;
	
	m_ObservingCamera = *cam;
	//m_ObservingCamera.Roll = -m_ObservingCamera.Roll;
	//m_ObservingCamera.Update();

	//this->SetupMatrices(this->m_ObservingCamera);		// obsolete with vertexshaders
	static int i=0;

    // Only update noise every other frame
    //if(i%2)
    {
	    m_bPlaneInFrustum = this->GetMinMax(&range);
    	
	    if (m_bPlaneInFrustum){
    		
		    m_NoiseMaker->render_geometry(&range);
    		
		    D3DVERTEXBUFFER_DESC pDesc;
		    SOFTWARESURFACEVERTEX *vertices;
		    HRESULT hr = m_SurfVertices->GetDesc( &pDesc );
    		
		    if( FAILED(m_SurfVertices->Lock( 0, 0, (void**) &vertices, D3DLOCK_DISCARD)))	
		    {
			    MessageBox(NULL, "Could not lock vertexbuffer", "Reality Engine", MB_OK);
		    }
		    else
		    {
			    int size = pDesc.Size;
			    memcpy(vertices, m_NoiseMaker->vertices, size);
			    m_SurfVertices->Unlock();
    			
		    }		
	    }
    }
    i++;
	

	return true;
}


//--------------------------------------------------------------------------------------
// Z-Buffer cutter. Unused
//--------------------------------------------------------------------------------------
void Surface::RenderCutter()
{
	if (m_bPlaneInFrustum)
	{		
		//RenderWrap::dev->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW  );
		RenderWrap::dev->SetStreamSource( 0, m_SurfVertices, 0, sizeof(SOFTWARESURFACEVERTEX) );
		RenderWrap::dev->SetFVF( D3DFVF_SOFTWARESURFACEVERTEX);			
		RenderWrap::dev->SetIndices(m_SurfIndices);
		RenderWrap::dev->DrawIndexedPrimitive(	D3DPT_TRIANGLELIST, 0,	0, m_GridX*m_GridY, 0, 2*(m_GridX-1)*(m_GridY-1) );			
	}
}

//--------------------------------------------------------------------------------------
// Renders water with reflections and all
//--------------------------------------------------------------------------------------
bool Surface::Render(World* pWorld)
{
    Matrix world;
    world.m3.y = m_Pos.y;
    Matrix viewProj = m_ObservingCamera.viewProjection;
    viewProj = world * viewProj;

    D3DXVECTOR4 viewPos(m_ObservingCamera.Location.x,m_ObservingCamera.Location.y,m_ObservingCamera.Location.z,1);

    if (m_bPlaneInFrustum)
	{		
		if(false){			
			// underwater pass
			RenderWrap::dev->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW  );
			RenderWrap::dev->SetStreamSource( 0, m_SurfVertices, 0, sizeof(SOFTWARESURFACEVERTEX) );
			RenderWrap::dev->SetFVF( D3DFVF_SOFTWARESURFACEVERTEX);			
			RenderWrap::dev->SetIndices(m_SurfIndices);

			underwater_software_effect.GetEffect()->Begin(NULL,NULL);
			underwater_software_effect.GetEffect()->BeginPass(0);			
			underwater_software_effect.GetEffect()->SetMatrix("mViewProj",(D3DXMATRIX*)&(viewProj));
			
			if(SkyController::Instance)
			{
				underwater_software_effect.GetEffect()->SetFloat("sun_alfa", SkyController::Instance->fSunPosAlpha);
				underwater_software_effect.GetEffect()->SetFloat("sun_theta", SkyController::Instance->fSunPosTheta);
				underwater_software_effect.GetEffect()->SetFloat("sun_shininess", SkyController::Instance->fSunShininess);
				underwater_software_effect.GetEffect()->SetFloat("sun_strength", SkyController::Instance->fSunStrength);
			}

			underwater_software_effect.GetEffect()->SetVector("watercolour", &D3DXVECTOR4(fWaterColour.r,fWaterColour.g,fWaterColour.b,1));

			underwater_software_effect.GetEffect()->SetFloat("LODbias", fLODbias );
			underwater_software_effect.GetEffect()->SetVector("view_position", &D3DXVECTOR4(m_ObservingCamera.Location.x,m_ObservingCamera.Location.y,m_ObservingCamera.Location.z,1));
			underwater_software_effect.GetEffect()->SetTexture("EnvironmentMap",SkyController::Instance->GetSkyBGTexture(0)->GetTexture());
			underwater_software_effect.GetEffect()->SetTexture("FresnelMap",underwater_fresnel);
			underwater_software_effect.GetEffect()->SetTexture("Normalmap",m_NoiseMaker->normalmap);
			underwater_software_effect.GetEffect()->CommitChanges();
			
			if ( bAsPoints )
				RenderWrap::dev->DrawPrimitive( D3DPT_POINTLIST, 0, m_GridX*m_GridY );
			else
				RenderWrap::dev->DrawIndexedPrimitive(	D3DPT_TRIANGLELIST, 0,	0, m_GridX*m_GridY, 0, 2*(m_GridX-1)*(m_GridY-1) );				
			
			underwater_software_effect.GetEffect()->EndPass();
			underwater_software_effect.GetEffect()->End();
		}
	else{
			// above water pass
			RenderWrap::dev->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE  );	
			RenderWrap::dev->SetStreamSource( 0, m_SurfVertices, 0, sizeof(SOFTWARESURFACEVERTEX) );
			RenderWrap::dev->SetFVF( D3DFVF_SOFTWARESURFACEVERTEX);			
			RenderWrap::dev->SetIndices(m_SurfIndices);

#ifndef CPU_NORMALS
			m_NoiseMaker->generate_normalmap();
#endif			

            RenderDevice::Instance()->UpdateViewport();
			RenderWrap::dev->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE  );
			m_Surf_software_effect.GetEffect()->Begin(NULL,NULL);
			m_Surf_software_effect.GetEffect()->BeginPass(0);			
			m_Surf_software_effect.GetEffect()->SetMatrix("mViewProj",(D3DXMATRIX*)&(viewProj));
			//m_Surf_software_effect.GetEffect()->SetMatrix("mView",(D3DXMATRIX*)&(m_ObservingCamera.view));
			/*m_Surf_software_effect.GetEffect()->SetFloat("sun_alfa", fSunPosAlpha);
			m_Surf_software_effect.GetEffect()->SetFloat("sun_theta", fSunPosTheta);*/

			if(SkyController::Instance)
			{
				float sa = SkyController::Instance->fSunPosAlpha, st = SkyController::Instance->fSunPosTheta;
				m_Surf_software_effect.GetEffect()->SetVector("sun_vec",&D3DXVECTOR4(cos(st)*sin(sa), sin(st), cos(st)*cos(sa),0));
				m_Surf_software_effect.GetEffect()->SetFloat("sun_shininess", SkyController::Instance->fSunShininess);			
				m_Surf_software_effect.GetEffect()->SetFloat("sun_strength", SkyController::Instance->fSunStrength);
				
				for(int i=0;i<4;i++)
				{
					m_Surf_software_effect.GetEffect()->SetFloat(("SkyMix"+ToStr(i)).c_str(),*(float*)SkyController::Instance->GetSkyMaterial()->m_Parameters[i]->data);
					m_Surf_software_effect.GetEffect()->SetTexture(("EnvironmentMap"+ToStr(i)).c_str(),SkyController::Instance->GetSkyBGTexture(i)->GetTexture());
				}
			}

			m_Surf_software_effect.GetEffect()->SetFloat("reflrefr_offset", bReflRefrStrength);	
			m_Surf_software_effect.GetEffect()->SetFloat("fOpacity", fOpacity);
			m_Surf_software_effect.GetEffect()->SetBool("diffuseSkyRef", bDiffuseRefl);

            m_Surf_software_effect.GetEffect()->SetVector("FogColor",(D3DXVECTOR4*)&pWorld->GetFogColor());
            m_Surf_software_effect.GetEffect()->SetFloat("FogFactor",1.f/pWorld->GetFogDistance());
			m_Surf_software_effect.GetEffect()->SetVector("watercolour", &D3DXVECTOR4(fWaterColour.r,fWaterColour.g,fWaterColour.b,1));
			m_Surf_software_effect.GetEffect()->SetFloat("LODbias", fLODbias );
			m_Surf_software_effect.GetEffect()->SetVector("view_position", &viewPos);
			m_Surf_software_effect.GetEffect()->SetTexture("FresnelMap",m_Surf_fresnel);
#ifndef CPU_NORMALS
			m_Surf_software_effect.GetEffect()->SetTexture("Heightmap",m_NoiseMaker->heightmap);
			m_Surf_software_effect.GetEffect()->SetTexture("Normalmap",m_NoiseMaker->normalmap);
#endif
			m_Surf_software_effect.GetEffect()->SetTexture("Refractionmap",m_RefractionTex);
			m_Surf_software_effect.GetEffect()->SetTexture("Reflectionmap",m_ReflectionTex);
 
			//m_Surf_software_effect.GetEffect()->SetTexture("noiseXZ",noise2D);
			m_Surf_software_effect.GetEffect()->CommitChanges();
			if ( bAsPoints )
				RenderWrap::dev->DrawPrimitive( D3DPT_POINTLIST, 0, m_GridX*m_GridY );
			else
				RenderWrap::dev->DrawIndexedPrimitive(	D3DPT_TRIANGLELIST, 0,	0, m_GridX*m_GridY, 0, 2*(m_GridX-1)*(m_GridY-1) );				

			 m_Surf_software_effect.GetEffect()->EndPass();

			m_Surf_software_effect.GetEffect()->End();
		} 
	}
    
	/*
#ifndef CPU_NORMALS
	if (bDisplayTargets)
	{
		debug_render_quad( m_NoiseMaker->packed_noise_texture[0], 0);
		debug_render_quad( m_NoiseMaker->packed_noise_texture[1], 1);
		debug_render_quad( m_NoiseMaker->heightmap, 2);
		debug_render_quad( m_NoiseMaker->normalmap, 3);
		debug_render_quad( m_RefractionTex, 4);
		debug_render_quad( m_ReflectionTex, 5);
	}
#endif
*/
	return true;
}


// set all parameters assuming position, forward & all the perspective shit are correct
void update_lookat(Camera* cam, Vector& forward)
{
	// perspective matrix
	//D3DXMatrixPerspectiveFovLH(&proj, fov, aspect, znear, zfar);
	// view matrix
	D3DXMatrixLookAtLH( (D3DXMATRIX*)&cam->view, (D3DXVECTOR3*)&cam->Location,(D3DXVECTOR3*)&(cam->Location+forward), &D3DXVECTOR3(0,1,0));

	// and finally the combined viewproj
	cam->viewProjection = cam->view*cam->projection;
	// and all the inverses
	/*D3DXMatrixInverse(&invproj,NULL,&proj);
	D3DXMatrixInverse(&invview,NULL,&view);
	D3DXMatrixInverse(&invviewproj,NULL,&viewproj);
	D3DXVec3TransformNormal(&right,&D3DXVECTOR3(1,0,0),&invview);*/
}


//--------------------------------------------------------------------------------------
// Get the matrix that defines the minimum rectangle in which the frustum is located
//--------------------------------------------------------------------------------------
bool Surface::GetMinMax(D3DXMATRIXA16 *range)
{
	SetDisplacementAmplitude(m_NoiseMaker->fStrength);
	float		x_min,y_min,x_max,y_max;
	D3DXVECTOR3 frustum[8],proj_points[24];		// frustum to check the camera against

	int n_points=0;
	int cube[] = {	0,1,	0,2,	2,3,	1,3,
		0,4,	2,6,	3,7,	1,5,
		4,6,	4,5,	5,7,	6,7};	// which frustum points are connected together?

	// transform frustum points to worldspace (should be done to the m_ObservingCamera because it's the interesting one)
    Matrix world;
    world.m3.y = m_Pos.y;
    Matrix viewProj = m_ObservingCamera.viewProjection;
    viewProj = world * viewProj;
    D3DXMATRIX invViewProjection = *(D3DXMATRIX*)&viewProj.Inverse();

	D3DXVec3TransformCoord(&frustum[0], &D3DXVECTOR3(-1,-1,-1), &invViewProjection);
	D3DXVec3TransformCoord(&frustum[1], &D3DXVECTOR3(+1,-1,-1), &invViewProjection);
	D3DXVec3TransformCoord(&frustum[2], &D3DXVECTOR3(-1,+1,-1), &invViewProjection);
	D3DXVec3TransformCoord(&frustum[3], &D3DXVECTOR3(+1,+1,-1), &invViewProjection);
	D3DXVec3TransformCoord(&frustum[4], &D3DXVECTOR3(-1,-1,+1), &invViewProjection);
	D3DXVec3TransformCoord(&frustum[5], &D3DXVECTOR3(+1,-1,+1), &invViewProjection);
	D3DXVec3TransformCoord(&frustum[6], &D3DXVECTOR3(-1,+1,+1), &invViewProjection);
	D3DXVec3TransformCoord(&frustum[7], &D3DXVECTOR3(+1,+1,+1), &invViewProjection);	


	// check intersections with upper_bound and lower_bound	
	for(int i=0; i<12; i++){
		int src=cube[i*2], dst=cube[i*2+1];
		if ((upper_bound.a*frustum[src].x + upper_bound.b*frustum[src].y + upper_bound.c*frustum[src].z + upper_bound.d*1)/(upper_bound.a*frustum[dst].x + upper_bound.b*frustum[dst].y + upper_bound.c*frustum[dst].z + upper_bound.d*1)<0){			
			D3DXPlaneIntersectLine( &proj_points[n_points++], &upper_bound, &frustum[src], &frustum[dst]);			
		}
		if ((lower_bound.a*frustum[src].x + lower_bound.b*frustum[src].y + lower_bound.c*frustum[src].z + lower_bound.d*1)/(lower_bound.a*frustum[dst].x + lower_bound.b*frustum[dst].y + lower_bound.c*frustum[dst].z + lower_bound.d*1)<0){			
			D3DXPlaneIntersectLine( &proj_points[n_points++], &lower_bound, &frustum[src], &frustum[dst]);			
		}
	}
	// check if any of the frustums vertices lie between the upper_bound and lower_bound planes
	{
		for(int i=0; i<8; i++){		
			if ((upper_bound.a*frustum[i].x + upper_bound.b*frustum[i].y + upper_bound.c*frustum[i].z + upper_bound.d*1)/(lower_bound.a*frustum[i].x + lower_bound.b*frustum[i].y + lower_bound.c*frustum[i].z + lower_bound.d*1)<0){			
				proj_points[n_points++] = frustum[i];
			}		
		}	
	}

	//
	// create the camera the grid will be projected from
	//
	delete m_ProjectingCamera;
	m_ProjectingCamera = new Camera();
	*m_ProjectingCamera = m_ObservingCamera;
	// make sure the camera isn't too close to the plane
	float height_in_plane = (lower_bound.a*m_ProjectingCamera->Location.x +
		lower_bound.b*m_ProjectingCamera->Location.y +
		lower_bound.c*m_ProjectingCamera->Location.z);

	bool keep_it_simple = false;
	bool underwater=false;

	if (height_in_plane < 0.0f) underwater = true;

	D3DXMATRIX invview = *(D3DXMATRIX*)&m_ObservingCamera.view.Inverse();
	Vector forward;
	D3DXVec3TransformNormal((D3DXVECTOR3*)&forward,&D3DXVECTOR3(0,0,1),&invview);
	//D3DXVec3TransformNormal(&up,&D3DXVECTOR3(0,1,0),&invview);
	//D3DXVec3TransformNormal(&right,&D3DXVECTOR3(1,0,0),&invview);

	if(keep_it_simple)
	{
		update_lookat(m_ProjectingCamera,forward);
	}
	else
	{
		D3DXVECTOR3 aimpoint, aimpoint2;		

		if (height_in_plane < (m_NoiseMaker->fStrength+fElevation))
		{					
			if(underwater)
				m_ProjectingCamera->Location += Vector(lower_bound.a,lower_bound.b,lower_bound.c)*(m_NoiseMaker->fStrength + fElevation - 2*height_in_plane);															
			else
				m_ProjectingCamera->Location += Vector(lower_bound.a,lower_bound.b,lower_bound.c)*(m_NoiseMaker->fStrength + fElevation - height_in_plane);
		} 
		
		// aim the projector at the point where the camera view-vector intersects the plane
		// if the camera is aimed away from the plane, mirror it's view-vector against the plane
		if( (D3DXPlaneDotNormal(&plane, (D3DXVECTOR3*)&(forward)) < 0.0f) log_xor (D3DXPlaneDotCoord(&plane, (D3DXVECTOR3*)&(m_ObservingCamera.Location)) < 0.0f ) )
		{				
			D3DXPlaneIntersectLine( &aimpoint, &plane, (D3DXVECTOR3*)&(m_ObservingCamera.Location), (D3DXVECTOR3*)&(m_ObservingCamera.Location + forward) );			
		}
		else
		{
			D3DXVECTOR3 flipped;
			flipped = *(D3DXVECTOR3*)&forward - 2*normal*D3DXVec3Dot((D3DXVECTOR3*)&(forward),&normal);
			D3DXPlaneIntersectLine( &aimpoint, &plane, (D3DXVECTOR3*)&(m_ObservingCamera.Location), &(*(D3DXVECTOR3*)&m_ObservingCamera.Location + flipped) );			
		}

			// force the point the camera is looking at in a plane, and have the projector look at it
			// works well against horizon, even when camera is looking upwards
			// doesn't work straight down/up
			float af = fabs(D3DXPlaneDotNormal(&plane, (D3DXVECTOR3*)&(forward)));
			//af = 1 - (1-af)*(1-af)*(1-af)*(1-af)*(1-af);
			//aimpoint2 = (m_ObservingCamera.Location + m_ObservingCamera.zfar * forward);
			aimpoint2 = *(D3DXVECTOR3*)&(m_ObservingCamera.Location + 10.0f * forward);
			aimpoint2 = aimpoint2 - normal*D3DXVec3Dot(&aimpoint2,&normal);
		
			// fade between aimpoint & aimpoint2 depending on view angle
			
			aimpoint = aimpoint*af + aimpoint2*(1.0f-af);
			//aimpoint = aimpoint2;
			
			Vector fwd = *(Vector*)&aimpoint - m_ProjectingCamera->Location;
			update_lookat(m_ProjectingCamera,fwd);
	}



	//sprintf( debugdata, "n_points %i\n",n_points);
	{
		for(int i=0; i<n_points; i++){
			// project the point onto the surface plane
			proj_points[i] = proj_points[i] - normal*D3DXVec3Dot(&proj_points[i],&normal);	
		}
	}

	{
		for(int i=0; i<n_points; i++){
			D3DXVec3TransformCoord( &proj_points[i], &proj_points[i], (D3DXMATRIX*)&(m_ProjectingCamera->view));	 
			//sprintf( debugdata, "%s%f  %f  %f\n",debugdata,proj_points[i].x,proj_points[i].y,proj_points[i].z);
			D3DXVec3TransformCoord( &proj_points[i], &proj_points[i], (D3DXMATRIX*)&(m_ProjectingCamera->projection));
		}
	}

	// debughonk

	/*	for(int i=0; i<n_points; i++){
	//sprintf( debugdata, "%s%f  %f  %f\n",debugdata,proj_points[i].x,proj_points[i].y,proj_points[i].z);
	}*/

	// get max/min x & y-values to determine how big the "projection window" must be
	if (n_points > 0){
		x_min = proj_points[0].x;
		x_max = proj_points[0].x;
		y_min = proj_points[0].y;
		y_max = proj_points[0].y;
		for(int i=1; i<n_points; i++){
			if (proj_points[i].x > x_max) x_max = proj_points[i].x;
			if (proj_points[i].x < x_min) x_min = proj_points[i].x;
			if (proj_points[i].y > y_max) y_max = proj_points[i].y;
			if (proj_points[i].y < y_min) y_min = proj_points[i].y;
		}		
		

		//sprintf( debugdata, "%sx = [%f..%f] y = [%f..%f]\n",debugdata,x_min,x_max,y_min,y_max);

		//sprintf( debugdata, "%sheight_in_plane: %f\n",debugdata,height_in_plane);
		
		////sprintf( debugdata,	"%slimit_y_upper = %f\n",debugdata,limit_y_upper);
		//		//sprintf( debugdata, "%sy1 = [%f] y2 = [%f]\n",debugdata,y1,y2);

		// build the packing matrix that spreads the grid across the "projection window"
		D3DXMATRIXA16 pack(	x_max-x_min,	0,				0,		x_min,
							0,				y_max-y_min,	0,		y_min,
							0,				0,				1,		0,	
							0,				0,				0,		1);
		D3DXMatrixTranspose(&pack,&pack);
		*range = pack* *(D3DXMATRIX*)&m_ProjectingCamera->viewProjection.Inverse();

		return true;
	}
	return false;
}


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
Surface::~Surface(){
}

//--------------------------------------------------------------------------------------
// sets the matrices according to the camera
//--------------------------------------------------------------------------------------
void Surface::SetupMatrices(const Camera *camera_view)
{
	D3DXMATRIXA16 matWorld,matProj;
	D3DXMatrixIdentity(&matWorld);
	RenderWrap::dev->SetTransform( D3DTS_WORLD, &matWorld );
	RenderWrap::dev->SetTransform( D3DTS_VIEW, (D3DXMATRIX*)&(camera_view->view) );
	RenderWrap::dev->SetTransform( D3DTS_PROJECTION, (D3DXMATRIX*)&(camera_view->projection) );
}

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void Surface::SetGridSize(int size_x, int size_y)
{
	this->m_GridX = size_x+1;
	this->m_GridY = size_y+1;
	this->m_SurfIndices->Release();
	this->m_SurfVertices->Release();
	this->InitializeData(true);
	
	m_NoiseMaker->resize(m_GridX, m_GridY);
}

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void Surface::SetTextureSize(int size)
{
	this->m_TargetSize = size;

	// Rendertargets
	SAFE_RELEASE(m_ReflectionTex);
	SAFE_RELEASE(m_RefractionTex);
	SAFE_RELEASE(depthstencil);

	this->InitializeData(false,true);
}

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void Surface::SetRenderMode(int rendermode)
{
	this->rendermode = rendermode;
}

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void Surface::SetDisplacementAmplitude(float amplitude){
	D3DXPlaneFromPointNormal( &(this->upper_bound), &(this->pos + amplitude * this->normal), &(this->normal));
	D3DXPlaneFromPointNormal( &(this->lower_bound), &(this->pos - amplitude * this->normal), &(this->normal));	
}

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
float Surface::GetHeightAt( float x, float z )
{
	if (m_NoiseMaker)
		return m_NoiseMaker->GetHeightAt(x,z);
	return 0.0f;
}

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void Surface::CalcEfficiency()
{
	efficiency = 0;
	for(int i=0; i<(m_GridX*m_GridY); i++)
	{
		D3DXVECTOR3 pos;
		pos.x = m_NoiseMaker->vertices[i].x;
		pos.y = m_NoiseMaker->vertices[i].y;
		pos.z = m_NoiseMaker->vertices[i].z;
		if (this->WithinFrustum(&pos))
			efficiency += 1.0f;
	}
	efficiency /= m_GridX*m_GridY;
}