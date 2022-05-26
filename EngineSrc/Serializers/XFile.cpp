//======= (C) Copyright 2004, Artificial Studios. All rights reserved. ======
// Type: DirectX X-File Loading Module
// Info: Loads Maya and Max X-Files
//
//=============================================================================
#include "stdafx.h"
#include "Serializers\XFile.h"
#include "Collision.h"

float XFileLoad::s_Scale = 0.0254; // (Default) Inches to Meters
bool  XFileLoad::s_UseSkinning = true; // Generate skinned mesh
bool  XFileLoad::s_StripHierarchy = false;


static bool bScale = true; // Always scale by some factor

//--------------------------------------------------------------------------------------
/// Name: class CAllocateHierarchy
/// Desc: Custom version of ID3DXAllocateHierarchy with custom methods to create
///       frames and meshcontainers.
//--------------------------------------------------------------------------------------
class CAllocateHierarchy: public ID3DXAllocateHierarchy
{
public:
    STDMETHOD(CreateFrame)(THIS_ LPCSTR Name, LPD3DXFRAME *ppNewFrame);
    STDMETHOD(CreateMeshContainer)(THIS_ 
        LPCSTR Name, 
        CONST D3DXMESHDATA *pMeshData,
        CONST D3DXMATERIAL *pMaterials, 
        CONST D3DXEFFECTINSTANCE *pEffectInstances, 
        DWORD NumMaterials, 
        CONST DWORD *pAdjacency, 
        LPD3DXSKININFO pSkinInfo, 
        LPD3DXMESHCONTAINER *ppNewMeshContainer);
    STDMETHOD(DestroyFrame)(THIS_ LPD3DXFRAME pFrameToFree);
    STDMETHOD(DestroyMeshContainer)(THIS_ LPD3DXMESHCONTAINER pMeshContainerBase);

	Model*	m_Model; // Source model to fill
    CAllocateHierarchy() {}
};

//--------------------------------------------------------------------------------------
// Name: AllocateName()
// Desc: Allocates memory for a string to hold the name of a frame or mesh
//--------------------------------------------------------------------------------------
HRESULT AllocateName( LPCSTR Name, LPSTR *pNewName )
{
    UINT cbLength;

    if( Name != NULL )
    {
        cbLength = (UINT)strlen(Name) + 1;
        *pNewName = new CHAR[cbLength];
        if (*pNewName == NULL)
            return E_OUTOFMEMORY;
        memcpy( *pNewName, Name, cbLength*sizeof(CHAR) );
    }
    else
    {
        *pNewName = NULL;
    }

    return S_OK;
}

//-----------------------------------------------------------------------------
// Converts D3DXFRAME to ModelFrame
//-----------------------------------------------------------------------------
void XFileLoad::ConvertFrame(LPD3DXFRAME in, ModelFrame* out)
{
	out->Name					= in->Name;
	out->TransformationMatrix	= *(Matrix*)&in->TransformationMatrix;

	// Convert TM
	if(bScale)
		out->TransformationMatrix[3] *= s_Scale;

	// Mesh
	if(in->pMeshContainer)
	{
		Mesh* newMesh = new Mesh;
		
		// Expand mesh to standard vertex decl, so it can hold normals and tangents
		LPD3DXMESH pTemp;
		DXASSERT( in->pMeshContainer->MeshData.pMesh->CloneMesh( in->pMeshContainer->MeshData.pMesh->GetOptions(), VertexFormats::Instance()->FindFormat(sizeof(Vertex))->element, RenderWrap::dev, &pTemp ) );
		SAFE_RELEASE(in->pMeshContainer->MeshData.pMesh);
		in->pMeshContainer->MeshData.pMesh = pTemp;
		D3DXComputeTangent(in->pMeshContainer->MeshData.pMesh,0,0,0,TRUE,0);

		//
		// If Maya, convert centimeters to meters (divide all units by 100)
		//
		// TIM: Seems the max exporter is also centimeters!! 
		if(bScale)
		{
			// Convert mesh
			BYTE* Verts;
			in->pMeshContainer->MeshData.pMesh->LockVertexBuffer(0, (LPVOID*)&Verts);
			DWORD stride = in->pMeshContainer->MeshData.pMesh->GetNumBytesPerVertex();
			for(int i=0;i<in->pMeshContainer->MeshData.pMesh->GetNumVertices();i++)
			{
				Vector* pos = (Vector*)&Verts[i*stride];
				*pos *= s_Scale;
			}

			in->pMeshContainer->MeshData.pMesh->UnlockVertexBuffer();
		}


		// Copy mesh
		newMesh->SetMesh(in->pMeshContainer->MeshData.pMesh);

		newMesh->m_pDeclaration = VertexFormats::Instance()->FindFormat(sizeof(SimpleVertex))->decl;

		// Setup skinning
		if(in->pMeshContainer->pSkinInfo)
		{
			LPD3DXSKININFO skin = in->pMeshContainer->pSkinInfo;

			if(bScale)
			{
				// Copy over reference bone data
				for(int i=0;i<skin->GetNumBones();i++)
				{
					D3DXMATRIX* m = skin->GetBoneOffsetMatrix(i);
					(*m)._41 *= s_Scale;
					(*m)._42 *= s_Scale;
					(*m)._43 *= s_Scale;
				}
			}

           
			newMesh->m_pSkinInfo = skin;
			newMesh->m_pSkinInfo->AddRef();
            if(s_UseSkinning)
            {
			    newMesh->GenerateSkinInfo();
            }
		}

		out->SetMesh(newMesh);

		// Collision mesh
		out->collisionMesh = new CollisionMesh;
		out->collisionMesh->Initialize(newMesh);

	}

	// Recurse children and siblings
	if(in->pFrameSibling)
	{
		out->pFrameSibling = new ModelFrame;
		ConvertFrame(in->pFrameSibling,out->pFrameSibling);
	}
	if(in->pFrameFirstChild)
	{
		out->pFrameFirstChild = new ModelFrame;
		ConvertFrame(in->pFrameFirstChild,out->pFrameFirstChild);
	}
}

//-----------------------------------------------------------------------------
// Loads .X files
//-----------------------------------------------------------------------------
bool XFileLoad::LoadModel(string name, StaticModel* m)
{
	if(Engine::Instance()->IsDedicated())
		return false;

    StartMiniTimer();

	Model* model = (Model*)m;

	if(model->GetLoadingScale() != 1)
		s_Scale = model->GetLoadingScale();

	if(model->m_pFrameRoot)
		SAFE_DELETE(model->m_pFrameRoot);

	LPD3DXFRAME g_pFrameRoot;
	CAllocateHierarchy Alloc;
	Alloc.m_Model = (Model*)model;

	ID3DXAnimationController* pController;

	DXASSERT( D3DXLoadMeshHierarchyFromX( name.c_str(), D3DXMESH_MANAGED, RenderWrap::dev,
		&Alloc, NULL, &g_pFrameRoot, &pController) );

	// Now, convert D3DXFRAME to ModelFrame
	model->m_pFrameRoot = new ModelFrame;

	bool bMaya = g_pFrameRoot->Name && string(g_pFrameRoot->Name).find("DXCC_ROOT") != -1;

	// Rename root frame to first child, we don't want all our meshes to be called Scene_Root
	if(!g_pFrameRoot->pFrameSibling && g_pFrameRoot->pFrameFirstChild)
	{
		delete[] g_pFrameRoot->Name;
		AllocateName((g_pFrameRoot->pFrameFirstChild->Name + string("_")).c_str(),&g_pFrameRoot->Name);

		// Stupid Maya  sometimes includes GroundPlane, if found, skip to next frame
		if(bMaya && g_pFrameRoot->pFrameFirstChild->pFrameSibling)
		{
			delete[] g_pFrameRoot->Name;
			AllocateName((g_pFrameRoot->pFrameFirstChild->pFrameSibling->Name + string("_")).c_str(),&g_pFrameRoot->Name);

			// Must kill root TM on Maya, because it has an inversed scale matrix (WTF?)
			//D3DXMatrixIdentity(&g_pFrameRoot->TransformationMatrix);
		}
	}

	// Convert all frames to engine format
	ConvertFrame(g_pFrameRoot,model->m_pFrameRoot);

	// If animated, setup controller
	if(pController)
	{
		// Convert animations to meters
		if(bScale)
		{
			for(int i=0;i<pController->GetNumAnimationSets();i++)
			{
				LPD3DXKEYFRAMEDANIMATIONSET  set;
				pController->GetAnimationSet(i,(LPD3DXANIMATIONSET*)&set);
				for(int j=0;j<set->GetNumAnimations();j++)
				{
					for(int x=0;x<set->GetNumTranslationKeys(j);x++)
					{
						D3DXKEY_VECTOR3 key;
						set->GetTranslationKey (j,x,&key);
						key.Value *= s_Scale;
						set->SetTranslationKey(j,x,&key);
					}
				}
			}
		}


		model->InitializeAnimationSystem(pController);

		// Need to call this before working on tracks, or the next call will hang forever
		model->m_AnimationController->AdvanceTime(GDeltaTime,0);

		LPD3DXANIMATIONSET set;
		model->m_AnimationController->GetTrackAnimationSet(0,&set);
		if(set)
		{
			// Insert a track struct into model to represent the track loaded from the file
			model->tracks.resize(model->tracks.size()+1);
			model->tracks[model->tracks.size()-1].bLooping = false;
			model->tracks[model->tracks.size()-1].duration = (float)set->GetPeriod() / 4800.0f;
		}
	}

	// FIXME: This should be called, but it's corrupting some memory or deleting something it shouldn't
	// Investigate!!!!
	//D3DXFrameDestroy(g_pFrameRoot,&Alloc);

     LogPrintf("Model '%s' load took %f seconds",name.c_str(),StopMiniTimer()/1000.0f);
	//delete g_pFrameRoot;
	return true;
}


//--------------------------------------------------------------------------------------
// Name: CAllocateHierarchy::CreateFrame()
// Desc: 
//--------------------------------------------------------------------------------------
HRESULT CAllocateHierarchy::CreateFrame( LPCSTR Name, LPD3DXFRAME *ppNewFrame )
{
    HRESULT hr = S_OK;
    D3DXFRAME *pFrame;

    *ppNewFrame = NULL;

    pFrame = new D3DXFRAME;
    if (pFrame == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }

    hr = AllocateName(Name, &pFrame->Name);
    if (FAILED(hr))
        goto e_Exit;

    // initialize other data members of the frame
    D3DXMatrixIdentity(&pFrame->TransformationMatrix);

    pFrame->pMeshContainer = NULL;
    pFrame->pFrameSibling = NULL;
    pFrame->pFrameFirstChild = NULL;

    *ppNewFrame = pFrame;
    pFrame = NULL;

e_Exit:
    delete pFrame;
    return hr;
}


//--------------------------------------------------------------------------------------
// Name: CAllocateHierarchy::CreateMeshContainer()
// Desc: 
//--------------------------------------------------------------------------------------
HRESULT CAllocateHierarchy::CreateMeshContainer(
    LPCSTR Name, 
    CONST D3DXMESHDATA *pMeshData,
    CONST D3DXMATERIAL *pMaterials, 
    CONST D3DXEFFECTINSTANCE *pEffectInstances, 
    DWORD NumMaterials, 
    CONST DWORD *pAdjacency, 
    LPD3DXSKININFO pSkinInfo, 
    LPD3DXMESHCONTAINER *ppNewMeshContainer) 
{
    HRESULT hr;
    D3DXMESHCONTAINER *pMeshContainer = NULL;
    UINT NumFaces;
    LPDIRECT3DDEVICE9 pd3dDevice = NULL;

    LPD3DXMESH pMesh = NULL;

    *ppNewMeshContainer = NULL;

    // this sample does not handle patch meshes, so fail when one is found
    if (pMeshData->Type != D3DXMESHTYPE_MESH)
    {
        hr = E_FAIL;
        goto e_Exit;
    }

    // get the pMesh interface pointer out of the mesh data structure
    pMesh = pMeshData->pMesh;

    // this sample does not FVF compatible meshes, so fail when one is found
    if (pMesh->GetFVF() == 0)
    {
        hr = E_FAIL;
        goto e_Exit;
    }

    // allocate the overloaded structure to return as a D3DXMESHCONTAINER
    pMeshContainer = new D3DXMESHCONTAINER;
    if (pMeshContainer == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }
    memset(pMeshContainer, 0, sizeof(D3DXMESHCONTAINER));

    // make sure and copy the name.  All memory as input belongs to caller, interfaces can be addref'd though
    hr = AllocateName(Name, &pMeshContainer->Name);
    if (FAILED(hr))
        goto e_Exit;        

    pMesh->GetDevice(&pd3dDevice);
    NumFaces = pMesh->GetNumFaces();

	if(pMesh)
	{
		pMeshContainer->MeshData.pMesh = pMesh;
		pMeshContainer->MeshData.Type = D3DXMESHTYPE_MESH;
		// Otherwise is destroyed the second this function returns
		pMesh->AddRef();
	}
  
    // TODO: Material loading (skinnedmesh.cpp)

	if(pSkinInfo)
	{
		pMeshContainer->pSkinInfo = pSkinInfo;
		// Otherwise is destroyed the second this function returns
		pSkinInfo->AddRef();
	}

    *ppNewMeshContainer = pMeshContainer;
    pMeshContainer = NULL;

e_Exit:
    SAFE_RELEASE(pd3dDevice);

    // call Destroy function to properly clean up the memory allocated 
    if (pMeshContainer != NULL)
    {
        DestroyMeshContainer(pMeshContainer);
    }

    return hr;
}


//--------------------------------------------------------------------------------------
// Name: CAllocateHierarchy::DestroyFrame()
// Desc: 
//--------------------------------------------------------------------------------------
HRESULT CAllocateHierarchy::DestroyFrame(LPD3DXFRAME pFrameToFree) 
{
    SAFE_DELETE_ARRAY( pFrameToFree->Name );
    SAFE_DELETE( pFrameToFree );
    return S_OK; 
}


//--------------------------------------------------------------------------------------
// Name: CAllocateHierarchy::DestroyMeshContainer()
// Desc: 
//--------------------------------------------------------------------------------------
HRESULT CAllocateHierarchy::DestroyMeshContainer(LPD3DXMESHCONTAINER pMeshContainerBase)
{
    D3DXMESHCONTAINER *pMeshContainer = (D3DXMESHCONTAINER*)pMeshContainerBase;

    SAFE_DELETE_ARRAY( pMeshContainer->Name );
    SAFE_DELETE_ARRAY( pMeshContainer->pAdjacency );
    SAFE_DELETE_ARRAY( pMeshContainer->pMaterials );

    SAFE_RELEASE( pMeshContainer->MeshData.pMesh );
    SAFE_RELEASE( pMeshContainer->pSkinInfo );
    SAFE_DELETE( pMeshContainer );
    return S_OK;
}