//=============================================================================
// ModelElement: Various ModelElement classes.
//
//=============================================================================
#include "stdafx.h"
#include "Graphics\MeshManager.h" // VertexDecl
#include "Frame.h"

#define MAX(a,b)            (((a) > (b)) ? (a) : (b))
#define MIN(a,b)            (((a) < (b)) ? (a) : (b))

void GenerateSkinnedMeshAndConvertToPools(vector<Pool>& renderingPools, vector<BoneInfo>& bones, LPD3DXSKININFO pSkinInfo, LPD3DXMESH pOrigMesh, Pool* pool, DWORD* pAdjacency );
void GenerateNewPools(vector<Pool>& renderingPools, vector<BoneInfo>& bones, LPD3DXSKININFO pSkinInfo, LPD3DXMESH mesh, Pool* pool, LPD3DXBUFFER pBoneCombinationBuf, DWORD NumAttributeGroups);



void AnimatedModel::LinkBonePointersToFrame(ModelFrame* pFrame){
	// Update the pool matrix pointers
	for(int i=0;i<renderingPools.size();i++){
		Pool& p = renderingPools[i];

		if(!p.bSkinned)
			continue;

		// Link skinning matrices to animation frames
		for(int x=0;x<p.bones.size();x++){
			ModelFrame* frame = (ModelFrame*)D3DXFrameFind( pFrameRoot, p.bones[x].name.c_str() );
			if(frame)
				p.pAnimatedBoneMats[x] =  frame->CombinedTransformationMatrix;
			else
				Error("Could not find bone for pool in main bone array. bone is called '%s'",p.bones[x].name.c_str());
		}
	}
}


//-----------------------------------------------------------------------------
// Name: InitSkeletalData()
// Copy data to D3DX format so that we can use the D3DX animation framework
// THINGS WE MUST FILL:
// pSkinInfo
// pOrigMesh
// pAdjacency
//-----------------------------------------------------------------------------
/*void AnimatedModel::InitializeSkinning(){
	int originalNumberOfPools = renderingPools.size(); // This will change as soon
	// as the pools are expanded with skinning data, so store it now

	// We keep looping over pool 0, because it is deleted each time, in place of new pools
	// So we loop until we have gone through all old pools
	if(Engine::Instance()->RenderSys){
		for(int i=0;i<originalNumberOfPools;i++)
			//ConvertPoolToSkinnedPools(&renderingPools[0]);
	}
}*/
/*
//--------------------
// Entry function -- Does everything to convert a pool into a set of skinning-capable pools
//--------------------
void ModelElement::ConvertPoolToSkinnedPools(Pool* pool){
	// Get vertex/index/face counts
	int numIndices = pool->ib->GetIndexCount();
	int numVertices = pool->numVertices;
	int numFaces = pool->ib->HasStrips() ? numIndices -2 : numIndices/3;

	// Build temporary system memory mesh
	LPD3DXMESH pOrigMesh;
	HRESULT hr = D3DXCreateMesh(numFaces, numVertices, D3DXMESH_SYSTEMMEM , VertexDecl , RenderWrap::dev, &pOrigMesh);
	if(FAILED(hr))
		Error("D3DXCreateMesh failed with %d indices and %d vertices",numIndices,numVertices);

	// Get pointers to vertices and indices
	Vertex* vertices;
	if(pool->worldVBNum == -1){
		vertices = &((Vertex*)pool->vb->GetVertices())[pool->vertexOffset];
	}
	else{
		vertices = &((Vertex*)pool->world->TreeVertexBuffers[pool->worldVBNum].GetVertices())[pool->vertexOffset];
	}
	WORD* indices = pool->ib->GetIndices();

	//	Copy vertices to mesh
	Vertex* VerticesBuffer;
	pOrigMesh->LockVertexBuffer(0, (LPVOID*)&VerticesBuffer);
	memcpy(VerticesBuffer,vertices,sizeof(Vertex)*numVertices);
	pOrigMesh->UnlockVertexBuffer();

	// Copy indices to mesh
	WORD* IndexesBuffer;
	pOrigMesh->LockIndexBuffer(0, (LPVOID*)&IndexesBuffer);
	memcpy(IndexesBuffer,indices,sizeof(WORD)*numIndices);
	pOrigMesh->UnlockIndexBuffer();

	// Generate adjacency (used by skinned mesh)
	DWORD* pAdjacency;
	pAdjacency = new DWORD[3 * numFaces * sizeof(DWORD)];
	if(FAILED(hr=pOrigMesh->GenerateAdjacency(0,pAdjacency)))
		Error("GenerateAdjacency failed with %s",DXGetErrorString9(hr));


	// perform simple cleansing operations on mesh
	LPD3DXMESH pTempMesh;
    DXASSERT(D3DXCleanMesh( pOrigMesh, pAdjacency, &pTempMesh, pAdjacency, NULL ) );
    SAFE_RELEASE(pOrigMesh);
    pOrigMesh = pTempMesh;


	// Create basic skin info to copy bones, weights, and indices to
	LPD3DXSKININFO          pSkinInfo;
	if(FAILED(hr=D3DXCreateSkinInfo(pool->numVertices,SkinnedVertexDecl,pool->bones.size(),&pSkinInfo)))
		Error("D3DXCreateSkinInfo failed with %s",DXGetErrorString9(hr));

	// Set reference bone data
	for(int i=0;i<pool->bones.size();i++){
		if(FAILED(hr=pSkinInfo->SetBoneInfluence(i, pool->bones[i].weights.size(),&pool->bones[i].indices[0], &pool->bones[i].weights[0])))
			Error("SetBoneInfluence failed with %s",DXGetErrorString9(hr));

		if(FAILED(hr=pSkinInfo->SetBoneOffsetMatrix(i,(D3DXMATRIX*)&pool->bones[i].invPoseMatrix)))
			Error("SetBoneOffsetMatrix failed with %s",DXGetErrorString9(hr));
	}


	GenerateSkinnedMeshAndConvertToPools(renderingPools,bones,pSkinInfo,pOrigMesh,pool,pAdjacency);


	// We're done with the original mesh
	SAFE_RELEASE(pOrigMesh);
	SAFE_RELEASE(pSkinInfo);
	delete[] pAdjacency;
}


struct SV2 
{
	Vector position;
	float blend1,blend2;
	DWORD indices;
	Vector normal;
	float tu,tv;
	Vector tan;

	 SV2(Vector p,Vector n,float u,float v){
		position=p;normal=n;tu=u;tv=v;
	 } 
	 SV2(){}
};

struct SV1 
{
	Vector position;
	float blend1;
	DWORD indices;
	Vector normal;
	float tu,tv;
	Vector tan;

	 SV1(Vector p,Vector n,float u,float v){
		position=p;normal=n;tu=u;tv=v;
	 } 
	 SV1(){}
};
//--------------------
// Takes a pool + D3DX Skinning data, and transforms it into the number of pools needed
// to fit the skeletal animation in the vertex shader
// It then deletes the original pool
//--------------------
void GenerateNewPools(vector<Pool>& renderingPools, vector<BoneInfo>& bones, LPD3DXSKININFO pSkinInfo, LPD3DXMESH mesh, Pool* pool, LPD3DXBUFFER pBoneCombinationBuf, DWORD NumAttributeGroups){
	LPD3DXBONECOMBINATION pBoneComb;
	pBoneComb = reinterpret_cast<LPD3DXBONECOMBINATION>(pBoneCombinationBuf->GetBufferPointer());

	int numSegments = NumAttributeGroups;
	
	// Get index to delete this pool when we're done with it
	int index = -1; 
	for(int i=0;i<renderingPools.size();i++) if(pool==&renderingPools[i]) index = i;

	// Attribute table, each attribute holds a different subsection of the mesh
	D3DXATTRIBUTERANGE attribTable[50];
	DWORD attribTableSize;
	mesh->GetAttributeTable(attribTable,&attribTableSize);

	int offset = renderingPools.size();
	renderingPools.resize(renderingPools.size() + numSegments);

	// Need to get new pool pointer, as shuffling the array means it has changed
	pool = &renderingPools[index]; 

	for(int i=0;i<numSegments;i++){
		// Copy old pool data
		renderingPools[i+offset] = *pool;
		Pool& newPool = renderingPools[i+offset];
		newPool.bSkinned = true;

		if(mesh->GetNumBytesPerVertex() == 48){
			newPool.vb = 0;
			newPool.numVertices = 0;
			newPool.bSkinned = false;
			continue;
		}

		// Copy over array
		newPool.staticLights = new Light*[newPool.numStaticLights];
		memcpy(newPool.staticLights,pool->staticLights,newPool.numStaticLights);

		// Resize pointer array in preparation of linkage with animation controller outputs
		newPool.pAnimatedBoneMats.resize(pSkinInfo->GetNumBones());

		// Allocate space for blend matrices
		newPool.pShaderBoneMatrices.resize(newPool.NumPaletteEntries);

		// Convert mesh attribute section to pool buffer...
		// There's one attribute section for each segment of the skeletal
		// mesh that could fix in the vertex shader constant registers
		newPool.ib = new IndexBuffer();
		WORD* IndexesBuffer;
		mesh->LockIndexBuffer(D3DLOCK_READONLY, (LPVOID*)&IndexesBuffer);
		// Set the index buffer to the SUBSET of this mesh
		WORD* newBuffer = new WORD[attribTable[i].FaceCount*3];
		// Remove vertex offset from indices
		for(int p=0;p<attribTable[i].FaceCount*3;p++)
			newBuffer[p] = IndexesBuffer[p + attribTable[i].FaceStart*3] - attribTable[i].VertexStart;
		newPool.ib->SetIndices(attribTable[i].FaceCount*3,newBuffer,false);
		delete[] newBuffer;
		mesh->UnlockIndexBuffer();

		newPool.vb = new VertexBuffer();
		newPool.numVertices = attribTable[i].VertexCount;
		
		// Invalid pool. Probably only has one weight, or something funky like that
		// Really should just correct declaration
		if(mesh->GetNumBytesPerVertex() != sizeof(SkinnedVertex)){
			//int dbg = mesh->GetNumBytesPerVertex();
			//int f = mesh->GetNumFaces();
			//D3DVERTEXELEMENT9 decl[10];
			//mesh->GetDeclaration(decl);
			//newPool.vb = 0;
			//newPool.numVertices = 0;
			//newPool.bSkinned = false;
			//Warning("Serious error, Skinned pool ignored!");
			//continue;
			int meshBytes = mesh->GetNumBytesPerVertex();
			if(meshBytes == sizeof(SV2)){
				SV2* vertices;
				SkinnedVertex* verts = new SkinnedVertex[attribTable[i].VertexCount];

				mesh->LockVertexBuffer(D3DLOCK_READONLY, (LPVOID*)&vertices);
				int count=0;
				for(int p=attribTable[i].VertexStart;p<attribTable[i].VertexCount+attribTable[i].VertexStart;p++){
					verts[count].blend1 = vertices[p].blend1;
					verts[count].blend2 = vertices[p].blend2;
					verts[count].blend3 = 0;

					verts[count].indices = vertices[p].indices;
					verts[count].normal = vertices[p].normal;
					verts[count].position = vertices[p].position;
					verts[count].tan = vertices[p].tan;
					verts[count].tu = vertices[p].tu;
					verts[count].tv = vertices[p].tv;
					count++;
				}

				newPool.vb->SetVertices(attribTable[i].VertexCount,verts,VT_SkinnedVertex,false);
				
				delete[] verts;
			}
			else if(meshBytes == sizeof(SV1)){
				SV1* vertices;
				SkinnedVertex* verts = new SkinnedVertex[attribTable[i].VertexCount];

				mesh->LockVertexBuffer(D3DLOCK_READONLY, (LPVOID*)&vertices);
				int count=0;
				for(int p=attribTable[i].VertexStart;p<attribTable[i].VertexCount+attribTable[i].VertexStart;p++){
					verts[count].blend1 = vertices[p].blend1;
					verts[count].blend2 = 0;
					verts[count].blend3 = 0;

					verts[count].indices = vertices[p].indices;
					verts[count].normal = vertices[p].normal;
					verts[count].position = vertices[p].position;
					verts[count].tan = vertices[p].tan;
					verts[count].tu = vertices[p].tu;
					verts[count].tv = vertices[p].tv;
					count++;
				}

				newPool.vb->SetVertices(attribTable[i].VertexCount,verts,VT_SkinnedVertex,false);
				
				delete[] verts;
			}
			else{
					mesh->UnlockVertexBuffer();
				Error("STOP! %d",meshBytes);

			}

		}
		else{
			SkinnedVertex* vertices;
			mesh->LockVertexBuffer(D3DLOCK_READONLY, (LPVOID*)&vertices);
			// Set the vertex buffer to the SUBSET of this mesh
			
			newPool.vb->SetVertices(attribTable[i].VertexCount,&vertices[attribTable[i].VertexStart],VT_SkinnedVertex,false);
		}

		mesh->UnlockVertexBuffer();

		// Create bone palette lookup table from D3DX bone combination buffer + attrib id
		// The attribID is i, since numSegments = numAttribGroups
		renderingPools[i+offset].BonePaletteIndices = new int[pool->NumPaletteEntries];
        for (int iPaletteEntry = 0; iPaletteEntry < pool->NumPaletteEntries; ++iPaletteEntry)
        {
          renderingPools[i+offset].BonePaletteIndices[iPaletteEntry] = pBoneComb[i].BoneId[iPaletteEntry];
		}
	}

	if(index != -1)
		renderingPools.erase(renderingPools.begin() + index);
}


//-----------------------------------------------------------------------------
// Name: GenerateSkinnedMeshAndConvertToPools()
// Desc: This function uses the pSkinInfo of the mesh 
//       container to generate the desired drawable mesh and bone combination 
//       table.
//-----------------------------------------------------------------------------
void GenerateSkinnedMeshAndConvertToPools(vector<Pool>& renderingPools, vector<BoneInfo>& bones, LPD3DXSKININFO pSkinInfo, LPD3DXMESH pOrigMesh, Pool* pool, DWORD* pAdjacency )
{
	LPD3DXMESH pMesh;
	DWORD NumAttributeGroups;
	LPD3DXBUFFER pBoneCombinationBuf;

	// Get palette size
	// First 9 constants are used for other data.  Each 4x3 matrix takes up 3 constants.
	// (96 - 9) /3 i.e. Maximum constant count - used constants 
	UINT MaxMatrices = 26; 
	pool->NumPaletteEntries = MIN(MaxMatrices, pSkinInfo->GetNumBones());

	// NOTE: Using system mem as it's a temp buffer
	DWORD Flags = D3DXMESHOPT_VERTEXCACHE | D3DXMESH_SYSTEMMEM;

//	HRESULT hr;
	if(FAILED(pSkinInfo->ConvertToIndexedBlendedMesh
		(
		pOrigMesh,
		Flags, 
		pool->NumPaletteEntries, 
		pAdjacency, 
		NULL, NULL, NULL,             
		&pool->NumInfl,
		&NumAttributeGroups, 
		&pBoneCombinationBuf, 
		&pMesh)))
	{
		LPD3DXBUFFER  buf;
		D3DXValidMesh(pOrigMesh,pAdjacency,&buf);
		Warning("ConvertToIndexedBlendedMesh failed. Verts: %d. Faces: %d Mat: %s Mesh Report: %s",pOrigMesh->GetNumFaces(), pOrigMesh->GetNumVertices(),pool->matRef->Name.c_str(),buf);
		
		// This pool has failed, so skip it
		int index = -1;
		for(int i=0;i<renderingPools.size();i++) 
			if(pool==&renderingPools[i]) index = i;
		if(index==-1)
			Error("Impossible");
		renderingPools.erase(renderingPools.begin() + index);

		return;

	}

	GenerateNewPools(renderingPools,bones,pSkinInfo,pMesh,pool,pBoneCombinationBuf,NumAttributeGroups);

	SAFE_RELEASE(pBoneCombinationBuf);
	SAFE_RELEASE(pMesh);
}*/
