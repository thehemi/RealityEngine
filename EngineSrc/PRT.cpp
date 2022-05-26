//=========== Copyright (c) 2004, Tim Johnson. All rights reserved. ===========
//
// Spherical Harmonics Environment. Subset of World.
//
//
//=============================================================================
#include "stdafx.h"
#include "World.h"
#include "PRTMesh.h"
#include "SkyController.h"
#include "dxstdafx.h"

//-----------------------------------------------------------------------------
// Returns index of probe in world
//-----------------------------------------------------------------------------
int World::LoadSHProbe(string filename)
{
	if(Engine::Instance()->IsDedicated())
		return -1;

	SHProbe* probe = new SHProbe;
	probe->strTexture = filename;
	probe->fBlendFactor = 0.3f;

	Texture t;
	if(!t.Load(filename))
	{
		Warning("SH Probe %s not found!",filename.c_str());
		return -1;
	}

	ZeroMemory( probe->fData, 3*D3DXSH_MAXORDER*D3DXSH_MAXORDER*sizeof(float) );
	HRESULT hr;
	V( D3DXSHProjectCubeMap( 6, (LPDIRECT3DCUBETEXTURE9)t.GetTexture(), probe->fData[0], probe->fData[1], probe->fData[2] ) );
	t.Destroy();
	m_SHProbes.push_back(probe);
	return m_SHProbes.size()-1;
}

//-----------------------------------------------------------------------------
// PRT from main sky lighting, we can do this just once a frame
// for objects with no rotation (all prefabs)
//-----------------------------------------------------------------------------
void World::UpdateGlobalPRT()
{
	m_GlobaldwOrder = 6;

	if(SkyController::Instance)
	{
		vector<Light*> lights = SkyController::Instance->GetLights();
		CalculatePRTSum(m_GlobalfSum,m_GlobaldwOrder,lights,Matrix());
	}
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void World::UpdateStaticPRT(Mesh* mesh)
{
	CPRTMesh* prtMesh = mesh->GetPRTMesh();
	if(!prtMesh)
		return;
	prtMesh->ComputeShaderConstants( m_GlobalfSum[0], m_GlobalfSum[1], m_GlobalfSum[2], m_GlobaldwOrder*m_GlobaldwOrder );
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void World::UpdateDynamicPRT(Mesh* mesh, Matrix& Transform, vector<Light*>& Lights)
{
	CPRTMesh* prtMesh = mesh->GetPRTMesh();
	if(!prtMesh)
		return;
	DWORD dwOrder = prtMesh->GetOrder();
	float fSum[3][D3DXSH_MAXORDER*D3DXSH_MAXORDER];
	CalculatePRTSum(fSum,dwOrder,Lights,Transform);
	prtMesh->ComputeShaderConstants( fSum[0], fSum[1], fSum[2], dwOrder*dwOrder );
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void World::CalculatePRTSum(float fSum[3][D3DXSH_MAXORDER*D3DXSH_MAXORDER], DWORD dwOrder, vector<Light*>& Lights, Matrix& tm)
{
	HRESULT hr;
	ZeroMemory( fSum, 3*D3DXSH_MAXORDER*D3DXSH_MAXORDER*sizeof(float) );

	float fLight[MAX_LIGHTS][3][D3DXSH_MAXORDER*D3DXSH_MAXORDER];  

	// Pass in the light direction, the intensity of each channel, and it returns
	// the source radiance as an array of order^2 SH coefficients for each color channel.  
	D3DXVECTOR4 lightObjectSpace;
	Matrix mWorldInv = tm.Inverse(); //Transform.Inverse();

	int count = 0;
	for( int i=0; i<Lights.size(); i++ )
	{
		if(count >= MAX_LIGHTS)
			break;

		if(!Lights[i] || !Lights[i]->m_Method == LIGHT_METHOD_SH)
			continue;

		if(Lights.size() > 1 && Lights[i]->m_ForceSHMultiPass)
			continue;

		// Transform the world space light dir into object space
		// Note that if there's multiple objects using PRT in the scene, then
		// for each object you need to either evaulate the lights in object space
		// evaulate the lights in world and rotate the light coefficients 
		// into object space.
		Vector vLight = Lights[i]->Location;
		FloatColor lightColor = Lights[i]->GetCurrentState().Diffuse*Lights[i]->GetCurrentState().Intensity;

		float radius = Lights[i]->GetCurrentState().Range;
		if(Lights[i]->m_Type == LIGHT_DIR || Lights[i]->m_Type == LIGHT_SPOT)
			vLight = -Lights[i]->GetCurrentState().Direction;

		// Push light away from origin, it's not local after all
		// TODO: Move light around bounding box of object, instead of this hack
		if(Lights[i]->m_Type == LIGHT_OMNI){
			//FIXME: Figure out why multipass omni have less intensity 
			//Hack increase multipass omni intensity
			//if(Lights.size() == 1)
			//	lightColor = lightColor*2.5;

			D3DXVec3Transform( &lightObjectSpace, (D3DXVECTOR3*)&vLight, (D3DXMATRIX*)&mWorldInv );

			Vector v = *(Vector*)&lightObjectSpace;
			v += v.Normalized()*radius;
			lightObjectSpace = *(D3DXVECTOR4*)&v;

			// This uses D3DXSHEvalSphericalLight(), but there's other 
			// types of lights provided by D3DXSHEval*.  Pass in the 
			// order of SH, color of the light, and the direction of the light 
			// in object space.
			// The output is the source radiance coefficients for the SH basis functions.  
			// There are 3 outputs, one for each channel (R,G,B). 
			// Each output is an array of m_dwOrder^2 floats.  
			V( D3DXSHEvalSphericalLight( dwOrder, (D3DXVECTOR3*)&lightObjectSpace, (radius),
				lightColor.r, lightColor.g, lightColor.b,
				fLight[count][0], fLight[count][1], fLight[count][2] ) );
		}
		else if(Lights[i]->m_Type == LIGHT_DIR)
        {
			D3DXVec3TransformNormal( (D3DXVECTOR3*)&lightObjectSpace, (D3DXVECTOR3*)&vLight, (D3DXMATRIX*)&mWorldInv );
/*
			V( D3DXSHEvalDirectionalLight( dwOrder, (D3DXVECTOR3*)&lightObjectSpace,
				lightColor.r, lightColor.g, lightColor.b,
				fLight[count][0], fLight[count][1], fLight[count][2] ) );
*/
            V( D3DXSHEvalConeLight( dwOrder, (D3DXVECTOR3*)&lightObjectSpace, DEG2RAD(m_fConeRadius),
                                lightColor.r, lightColor.g, lightColor.b,
                                fLight[i][0], fLight[i][1], fLight[i][2] ) );
		}

		count++;
	}



	// Now sum up the lights using D3DXSHAdd().
	for( int i=0; i<count; i++ )
	{
		// D3DXSHAdd will add Order^2 floats.  There are 3 color channels, 
		// so call it 3 times.
		D3DXSHAdd( fSum[0], dwOrder, fSum[0], fLight[i][0] );
		D3DXSHAdd( fSum[1], dwOrder, fSum[1], fLight[i][1] );
		D3DXSHAdd( fSum[2], dwOrder, fSum[2], fLight[i][2] );
	}
	// Add in environment maps
	for(int i=0;i<m_SHProbes.size();i++)
	{
		float fSkybox1[3][D3DXSH_MAXORDER*D3DXSH_MAXORDER];  
		float fSkybox1Rot[3][D3DXSH_MAXORDER*D3DXSH_MAXORDER];  

		D3DXSHScale( fSkybox1[0], dwOrder, m_SHProbes[i]->fData[0], m_SHProbes[i]->fBlendFactor );
		D3DXSHScale( fSkybox1[1], dwOrder, m_SHProbes[i]->fData[1], m_SHProbes[i]->fBlendFactor );
		D3DXSHScale( fSkybox1[2], dwOrder, m_SHProbes[i]->fData[2], m_SHProbes[i]->fBlendFactor );
		D3DXSHRotate( fSkybox1Rot[0], dwOrder, (D3DXMATRIX*)&mWorldInv, fSkybox1[0] );
		D3DXSHRotate( fSkybox1Rot[1], dwOrder, (D3DXMATRIX*)&mWorldInv, fSkybox1[1] );
		D3DXSHRotate( fSkybox1Rot[2], dwOrder, (D3DXMATRIX*)&mWorldInv, fSkybox1[2] );
		D3DXSHAdd( fSum[0], dwOrder, fSum[0], fSkybox1Rot[0] );
		D3DXSHAdd( fSum[1], dwOrder, fSum[1], fSkybox1Rot[1] );
		D3DXSHAdd( fSum[2], dwOrder, fSum[2], fSkybox1Rot[2] );
	}
}