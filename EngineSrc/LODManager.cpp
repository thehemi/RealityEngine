//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ==================
/// Name: LODManager
/// \brief Creates Mesh LODs using progressive meshes, imposters, alpha fading, etc
//
// TODO: What's faster, progressive or LODing previous LODs? Investigate!
//
// TODO:
// 0. Add LOD fading
// 1. Cache LODs
// 2. Add LOD options to Prefab (filter down to model)
// 3. Add lOD functionality to RB (Progressive??)
// 4. Map SH data back to LODs (Can just copy over vertex data on LODs)
//===========================================================================================
#include "stdafx.h"
#include "LODManager.h"

//-----------------------------------------------------------------------------
LODManager* LODManager::Instance()
{
	static LODManager inst;
	return &inst;
}

//-----------------------------------------------------------------------------
// Initialize config settings for global LOD
//-----------------------------------------------------------------------------
void LODManager::Initialize()
{
	VisibleRange = Engine::Instance()->MainConfig->GetFloat("VisibleRange");
	LODRange     = Engine::Instance()->MainConfig->GetFloat("LODRange");
}


//-----------------------------------------------------------------------------
// Creates LOD Levels
//
// LOD levels depend on factors such as topology of mesh and current face count
// If mesh cannot maintain shape or is already small, no further lods will be
// created
//
//
//-----------------------------------------------------------------------------
bool	LODManager::GenerateLODs(vector<Mesh*>& lods)
{
	return true;
	LPD3DXMESH source = lods[0]->GetHardwareMesh();
	lods.resize(1);

	LPD3DXBUFFER pAdjacency;
	D3DXCreateBuffer(source->GetNumFaces() * 3 * sizeof(DWORD),&pAdjacency);
	DXASSERT(source->GenerateAdjacency(0,(DWORD*)pAdjacency->GetBufferPointer()));

	// Perform simple cleansing operations on mesh
	LPD3DXMESH pTempMesh;
	HRESULT hr;
	if( FAILED( hr = D3DXCleanMesh( D3DXCLEAN_SIMPLIFICATION, source, (DWORD*)pAdjacency->GetBufferPointer(), &pTempMesh, 
		(DWORD*)pAdjacency->GetBufferPointer(), NULL ) ) )
	{
		SeriousWarning("Simplifying failed (CleanMesh error)");
		return false;
	}

	// Each LOD is half the previous LOD
	int numPolys = pTempMesh->GetNumFaces();
	for(int i=0;i<4;i++)
	{
		// Worth simplifying further?
		if(numPolys < 80)
			break;

		// Simplify!
		LPD3DXMESH pNewMesh;
		DXASSERT(D3DXSimplifyMesh( pTempMesh, (DWORD*)pAdjacency->GetBufferPointer(),
			NULL, NULL, numPolys/2 , D3DXMESHSIMP_FACE, &pNewMesh ));

		// Won't simplify further?
		if(pNewMesh->GetNumFaces() == numPolys){
			SAFE_RELEASE(pNewMesh);
			break;
		}

		// Add new LOD
		Mesh* newMesh = new Mesh;
		lods[0]->Clone(newMesh,false);
		newMesh->SetMesh(pNewMesh);
		lods.push_back(newMesh);

		// Set next iteration polys
		numPolys = pNewMesh->GetNumFaces();
	}


	SAFE_RELEASE(pTempMesh);
	SAFE_RELEASE(pAdjacency);

	return true;
}
