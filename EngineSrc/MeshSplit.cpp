//=========== Copyright (c) 2003, Tim Johnson. All rights reserved. ===========
// Low-level classes and structures used for rendering
//
// Author: Tim Johnson
//=============================================================================
#include "stdafx.h"
#include <dxerr9.h>
#include <d3dx9.h>
#include <crtdbg.h>
#include "PRTMesh.h"
#include "NVMeshMender.h"


//--------------------------------------------------------------------------------------
// Subset grouping info for split
//--------------------------------------------------------------------------------------
struct Group
{
    vector<DWORD>  indices;
    AttributeRange table;
    int            matID;
    BBox box;
};

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
int ClassifyFace(vector<Group>& groups, Vector& v1, Vector& v2, Vector& v3, int matID)
{
    for(int i=0;i<groups.size();i++)
    {
        if(groups[i].matID == matID && (groups[i].box.IsPointInBox(v1) || groups[i].box.IsPointInBox(v2) || groups[i].box.IsPointInBox(v3)))
        {
            return i;
        }
    }
    return -1;
}

//--------------------------------------------------------------------------------------
//
// D3D Mesh helper
//
//--------------------------------------------------------------------------------------
bool MeshOps::Split(Mesh* mesh, int xSegs, int ySegs, int zSegs)
{
    if(!mesh->m_Materials.size())
    {
        SeriousWarning("MeshOps::Split: Mesh has no materials, cannot continue.");
        return false;
    }

	LPD3DXMESH pMesh = mesh->GetHardwareMesh();

	BYTE* Verts;
	pMesh->LockVertexBuffer(0, (LPVOID*)&Verts);
	DWORD stride = pMesh->GetNumBytesPerVertex();
	int   numVerts = pMesh->GetNumVertices();

	bool bDWORDIndices = pMesh->GetOptions() & D3DXMESH_32BIT;
	int numIndices = pMesh->GetNumFaces()*3;
	BYTE* IndexesBuffer;
	pMesh->LockIndexBuffer(0, (LPVOID*)&IndexesBuffer);
	DWORD* dwIndices = (DWORD*)IndexesBuffer;
	WORD* wIndices = (WORD*)IndexesBuffer;

    vector<Group> groups;
    //
    // Create groups
    //

    // Get the box size for a single segment
    Vector size = (mesh->m_LocalBox.max-mesh->m_LocalBox.min);
    size.x /= xSegs;
    size.y /= ySegs;
    size.z /= zSegs;

    // Attrib id we will make unique for each segment
    int id = 0;

    // 1. By mat
    for(int i=0;i<mesh->m_Materials.size();i++)
    {
        // 2. By segment classification
        for(int a=0;a<xSegs;a++)
        {
            for(int b=0;b<ySegs;b++)
            {
                for(int c=0;c<zSegs;c++)
                {
                    BBox box(mesh->m_LocalBox.min,mesh->m_LocalBox.min);
                    // Increment min
                    box.min.x += size.x * a - 0.01f;
                    box.min.y += size.y * b - 0.01f;
                    box.min.z += size.z * c - 0.01f;
                    // Increment max
                    box.max.x += size.x * (a+1) + 0.01f;
                    box.max.y += size.y * (b+1) + 0.01f;
                    box.max.z += size.z * (c+1) + 0.01f;

                    Group g;
                    g.box = box;
                    g.matID = i;
                    ZeroMemory(&g.table,sizeof(AttributeRange));
                    g.table.VertexStart = 9999999;
                    g.table.AttribId = id++;
                    groups.push_back(g);
                }
            }
        }
    }


    DWORD *pBuffer;
	pMesh->LockAttributeBuffer(0,&pBuffer);

    //
    // Create new groups, based on face classification
    //
    for(int i=0;i<pMesh->GetNumFaces();i++)
    {
        int f1,f2,f3;
        if(bDWORDIndices)
        {
            f1 = dwIndices[i*3+0];
            f2 = dwIndices[i*3+1];
            f3 = dwIndices[i*3+2];
        }
        else
        {
            f1 = wIndices[i*3+0];
            f2 = wIndices[i*3+1];
            f3 = wIndices[i*3+2];
        }

        // Get material for face, so we can add to correct group
        int matID = pBuffer[i];
        if(matID < mesh->m_AttribToMaterial.size())
        {
            matID = mesh->m_AttribToMaterial[matID];
        }
        
        int id = ClassifyFace(groups,*(Vector*)&Verts[f1*stride],*(Vector*)&Verts[f2*stride],*(Vector*)&Verts[f3*stride],matID);
        assert(id != -1);


        if(f1 < groups[id].table.VertexStart)
            groups[id].table.VertexStart = f1;
        if(f2 < groups[id].table.VertexStart)
            groups[id].table.VertexStart = f2;
        if(f3 < groups[id].table.VertexStart)
            groups[id].table.VertexStart = f3;

        groups[id].indices.push_back(f1);
        groups[id].indices.push_back(f2);
        groups[id].indices.push_back(f3);
    }

    pMesh->UnlockAttributeBuffer();

    //
    // Create the new index buffer
    //
    int count = 0;
    for(int i=0;i<groups.size();i++)
    {
        groups[i].table.FaceStart = count/3;
        groups[i].table.FaceCount = groups[i].indices.size()/3;

        for(int j=0;j<groups[i].indices.size();j++)
        {
            if(bDWORDIndices)
                dwIndices[count++] = groups[i].indices[j];
            else
                wIndices[count++] = groups[i].indices[j];

            if(groups[i].indices[j] > groups[i].table.VertexStart+groups[i].table.VertexCount)
                groups[i].table.VertexCount = groups[i].indices[j]-groups[i].table.VertexStart;
        }
    }
    
    pMesh->UnlockIndexBuffer();
	pMesh->UnlockVertexBuffer();

    //
    // Create new tables
    //
    mesh->m_AttribTable.clear();
    mesh->m_AttribToMaterial.clear();
    for(int i=0;i<groups.size();i++)
    {
        if(groups[i].table.FaceCount)
        {
            groups[i].table.AttribId = mesh->m_AttribTable.size();
            mesh->m_AttribTable.push_back(groups[i].table);
            mesh->m_AttribToMaterial.push_back(groups[i].matID);
        }
    }
    // Fill mesh with new attrib tables
    mesh->CalcData(true);

	return true;
}