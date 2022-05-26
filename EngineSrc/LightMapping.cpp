//=========== Copyright (c) 2004, Tim Johnson. All rights reserved. ===========
// Precomputed Radiance Transfer is the preferred method for dynamic lightmapping
// However, for backwards compatibility and memory preservation we support
// converting PRT data into baked lightmaps (per vertex or per pixel)
//
// This file implements this functionality
//
// Author: Tim Johnson
//=============================================================================
#include "stdafx.h"
#include "Engine.h"
#include <dxerr9.h>
#include <d3dx9.h>
#include <crtdbg.h>
#include "PRTMesh.h"
#include "NVMeshMender.h"

//-----------------------------------------------------------------------------
SHORT4 ToShort4(Vector4& v);
SHORT2 ToShort2(Vector2& v);
Vector4 FromShort4(SHORT4& v);
Vector2 FromShort2(SHORT2& v);

//-----------------------------------------------------------------------------
// Name: CalcPRTVertex
// Type: Vertex shader fragment                                  
// Desc: Generates using SH PRT with PCA compression
//-----------------------------------------------------------------------------
Vector CalcPRTVertex(SHVertex& vIN, Vector4* aPRTConstants)
{
	// With compressed PRT, a single diffuse channel is caluated by:
	//       R[p] = (M[k] dot L') + sum( w[p][j] * (B[k][j] dot L');
	// where the sum runs j between 0 and # of PCA vectors
	//       R[p] = exit radiance at point p
	//       M[k] = mean of cluster k 
	//       L' = source radiance coefficients
	//       w[p][j] = the j'th PCA weight for point p
	//       B[k][j] = the j'th PCA basis vector for cluster k
	//
	// Note: since both (M[k] dot L') and (B[k][j] dot L') can be computed on the CPU, 
	// these values are passed in as the array aPRTConstants.   

	Vector4 vExitR = Vector4(0,0,0,0);
	Vector4 vExitG = Vector4(0,0,0,0);
	Vector4 vExitB = Vector4(0,0,0,0);

	// For each channel, multiply and sum all the vPCAWeights[j] by aPRTConstants[x] 
	// where: vPCAWeights[j] is w[p][j]
	//		  aPRTConstants[x] is the value of (B[k][j] dot L') that was
	//		  calculated on the CPU and passed in as a shader constant
	// Note this code is multipled and added 4 floats at a time since each 
	// register is a 4-D vector, and is the reason for using (NUM_PCA/4)
	for (int j=0; j < (MAX_NUM_PCA/4); j++) 
	{
		vExitR += vIN.vPCAWeights[j] * aPRTConstants[vIN.iClusterOffset+j+1+(MAX_NUM_PCA/4)*0];
		vExitG += vIN.vPCAWeights[j] * aPRTConstants[vIN.iClusterOffset+j+1+(MAX_NUM_PCA/4)*1];
		vExitB += vIN.vPCAWeights[j] * aPRTConstants[vIN.iClusterOffset+j+1+(MAX_NUM_PCA/4)*2];
	}    

	// Now for each channel, sum the 4D vector and add aPRTConstants[x] 
	// where: aPRTConstants[x] which is the value of (M[k] dot L') and
	//		  was calculated on the CPU and passed in as a shader constant.
	Vector4 vExitRadiance = aPRTConstants[vIN.iClusterOffset];
	vExitRadiance.x += vExitR.x + vExitR.y + vExitR.z + vExitR.w;
	vExitRadiance.y += vExitG.x + vExitG.y + vExitG.z + vExitG.w;
	vExitRadiance.z += vExitB.x + vExitB.y + vExitB.z + vExitB.w;

	// For spectral simulations the material properity is baked into the transfer coefficients.
	// If using nonspectral, then you can modulate by the diffuse material properity here.
	return Vector(vExitRadiance.x,vExitRadiance.y,vExitRadiance.z);   
}    

//-----------------------------------------------------------------------------
// Name: CalcPRTPixel
// Type: LPD3DXFILL3D
// Desc: Fills texture with per-pixel PRT values
//-----------------------------------------------------------------------------
D3DLOCKED_RECT* srcTextures;
DWORD           dwNumTextures;
Vector4*        aPRTConstants;
struct BYTE4 { char x,y,z,w; };

VOID WINAPI CalcPRTPixel (D3DXVECTOR4* pOut, const D3DXVECTOR2* pTexCoord, const D3DXVECTOR2* pTexelSize, LPVOID pData)
{
    Vector4 vPCAWeights[MAX_NUM_PCA/4];

	for(int j=0; j<MAX_NUM_PCA/4; j++ )
	{
        BYTE* bits = (BYTE*)srcTextures[j].pBits;
        bits += srcTextures[j].Pitch * (int)(pTexCoord->y/pTexelSize->y);
        // Assume 4 bytes per pixel
        bits += (int)(pTexCoord->x/pTexelSize->x)*4;

        BYTE4 b = *(BYTE4*)bits;
        vPCAWeights[j].x = (float)b.x/128.0f;
        vPCAWeights[j].y = (float)b.y/128.0f;
        vPCAWeights[j].z = (float)b.z/128.0f;
        vPCAWeights[j].w = (float)b.w/128.0f;
	}

   	// With compressed PRT, a single diffuse channel is caluated by:
	//       R[p] = (M[k] dot L') + sum( w[p][j] * (B[k][j] dot L');
	// where the sum runs j between 0 and # of PCA vectors
	//       R[p] = exit radiance at point p
	//       M[k] = mean of cluster k 
	//       L' = source radiance coefficients
	//       w[p][j] = the j'th PCA weight for point p
	//       B[k][j] = the j'th PCA basis vector for cluster k
	//
	// Note: since both (M[k] dot L') and (B[k][j] dot L') can be computed on the CPU, 
	// these values are passed in as the array aPRTConstants.   

	Vector4 vExitR = Vector4(0,0,0,0);
	Vector4 vExitG = Vector4(0,0,0,0);
	Vector4 vExitB = Vector4(0,0,0,0);

	// For each channel, multiply and sum all the vPCAWeights[j] by aPRTConstants[x] 
	// where: vPCAWeights[j] is w[p][j]
	//		  aPRTConstants[x] is the value of (B[k][j] dot L') that was
	//		  calculated on the CPU and passed in as a shader constant
	// Note this code is multipled and added 4 floats at a time since each 
	// register is a 4-D vector, and is the reason for using (NUM_PCA/4)
	for (int j=0; j < (MAX_NUM_PCA/4); j++) 
	{
		vExitR += vPCAWeights[j] * aPRTConstants[j+1+(MAX_NUM_PCA/4)*0];
		vExitG += vPCAWeights[j] * aPRTConstants[j+1+(MAX_NUM_PCA/4)*1];
		vExitB += vPCAWeights[j] * aPRTConstants[j+1+(MAX_NUM_PCA/4)*2];
	}    

	// Now for each channel, sum the 4D vector and add aPRTConstants[x] 
	// where: aPRTConstants[x] which is the value of (M[k] dot L') and
	//		  was calculated on the CPU and passed in as a shader constant.
	Vector4 vExitRadiance = aPRTConstants[0];
	vExitRadiance.x += vExitR.x + vExitR.y + vExitR.z + vExitR.w;
	vExitRadiance.y += vExitG.x + vExitG.y + vExitG.z + vExitG.w;
	vExitRadiance.z += vExitB.x + vExitB.y + vExitB.z + vExitB.w;

	// For spectral simulations the material properity is baked into the transfer coefficients.
	// If using nonspectral, then you can modulate by the diffuse material properity here.
	*pOut = D3DXVECTOR4(vExitRadiance.x,vExitRadiance.x,vExitRadiance.x,1);   
}



//--------------------------------------------------------------------------------------
//
// D3D Mesh helper
//
//--------------------------------------------------------------------------------------
bool MeshOps::PRTToStatic(ModelFrame *frame, World *world)
{
    Mesh* mesh = frame->GetMesh(0);
    if(!mesh->GetPRTMesh() || !mesh->GetPRTMesh()->GetCompBuffer())
        return false;

    if(!mesh->GetPRTMesh()->GetCompBuffer()->IsTexture())
    {
        LPD3DXMESH pMesh = mesh->GetHardwareMesh();
        DWORD stride = pMesh->GetNumBytesPerVertex();
        if(stride != sizeof(SHVertex))
            return false;

	    SHVertex* Verts;
	    pMesh->LockVertexBuffer(0, (LPVOID*)&Verts);
	    int   numVerts = pMesh->GetNumVertices();

        // Create new colored vertex array
        PackedVertex* newVerts = new PackedVertex[numVerts];

        float tu=1, tv=1;
        //
        // Find max U/V, so we can normalize down to [-1,+1]
        //
        for(int i=0;i<numVerts;i++)
        {
            if(fabsf(Verts[i].tu) > tu)
                tu = fabsf(Verts[i].tu);
            if(fabsf(Verts[i].tv) > tv)
                tv = fabsf(Verts[i].tv);
        }

        //
        //
        //
        for(int i=0;i<numVerts;i++)
        {
            FloatColor col = FloatColor(CalcPRTVertex(Verts[i],(Vector4*)mesh->GetPRTMesh()->m_aPRTConstants));
            col.Clamp();
            //newVerts[i].col = col.DWORDColor();
            newVerts[i].pos = Verts[i].position;
            newVerts[i].tan = ToShort4(Vector4(Verts[i].tan.x,Verts[i].tan.y,Verts[i].tan.z,1));
            newVerts[i].norm = ToShort4(Vector4(Verts[i].normal.x,Verts[i].normal.y,Verts[i].normal.z,col.r));
            newVerts[i].tex = ToShort2(Vector2(Verts[i].tu/tu,Verts[i].tv/tv));
        }

    	pMesh->UnlockVertexBuffer();

        ///
        // Backup old indices
        ///
        BYTE* IndexesBuffer;
	    pMesh->LockIndexBuffer(0, (LPVOID*)&IndexesBuffer);

        int numIndices = pMesh->GetNumFaces()*3;
        int bytes = (pMesh->GetOptions() & D3DXMESH_32BIT)?4:2;
        BYTE* indices = new BYTE[bytes*numIndices];
        memcpy(indices,IndexesBuffer,bytes*numIndices);
        pMesh->UnlockIndexBuffer();

        //
        // Create new mesh
        //
        mesh->Create((BYTE*)newVerts,indices,sizeof(PackedVertex),bytes,numVerts,numIndices,mesh->m_Materials);

        SAFE_DELETE_ARRAY(newVerts);
        SAFE_DELETE_ARRAY(indices);
    }
    //
    // Per-pixel PRT, so bake into lightmap
    //
    else
    {
        if(!mesh->m_LightMap)
            mesh->m_LightMap = new Texture();

        CPRTMesh* pMesh = mesh->GetPRTMesh();
        aPRTConstants = (Vector4*)mesh->GetPRTMesh()->m_aPRTConstants;
        DWORD dwTextureSize = mesh->m_SHOptions.dwTextureSize;
        // Create lightmap
        mesh->m_LightMap->CreateBlank(TT_DIFFUSE,D3DFMT_L8,dwTextureSize,dwTextureSize);

        // Lock PRT textures so we can read from them
        dwNumTextures      = pMesh->m_pPRTCompBuffer->GetNumPCA()/4;
        srcTextures = new D3DLOCKED_RECT[dwNumTextures];
        for( DWORD i=0; i<dwNumTextures; i++ )
            pMesh->m_pPCAWeightTexture[i]->LockRect(0,&srcTextures[i],0,D3DLOCK_READONLY);

        DXASSERT(D3DXFillTexture(mesh->m_LightMap->GetTexture(),CalcPRTPixel,0));
        
        // Unlock, we're done
        for( DWORD i=0; i<dwNumTextures; i++ )
            pMesh->m_pPCAWeightTexture[i]->UnlockRect(0);
        SAFE_DELETE_ARRAY(srcTextures);

        // Filter!
        D3DXFilterTexture( mesh->m_LightMap->GetTexture(), NULL, D3DX_DEFAULT, D3DX_DEFAULT );

        // Save in map folder

        // Read filename via MapName_Data\MeshName.dds
        string parentFile = world->GetFileName();
        string file = parentFile.substr(0,parentFile.find_last_of("."));
        if(file.find_last_of("\\") != -1)
            file = file.substr(0,file.find_last_of("\\"));
        file += "\\" + frame->Name + ".dds";

        D3DXSaveTextureToFile(file.c_str(),D3DXIFF_DDS ,mesh->m_LightMap->GetTexture(),NULL);
    }

    // Die PRT, Die!
    SAFE_DELETE(mesh->m_pPRTMesh);
    mesh->m_SHOptions.Enabled = false;

    return true;
}