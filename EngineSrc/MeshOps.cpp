//=========== Copyright (c) 2003, Tim Johnson. All rights reserved. ===========
// 
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
void MaxToD3D(Vector* in){
	float y = in->y;
	in->y = in->z;
	in->z = y;
}


//--------------------------------------------------------------------------------------
//
// D3D Mesh helper
//
//--------------------------------------------------------------------------------------
bool MeshOps::Convert(Mesh* mesh, Op op, Matrix& param)
{
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

    if(op == CenterPivot)
    {
        // Get new pivot center
        float radius;
        Vector center = (mesh->m_LocalBox.max+mesh->m_LocalBox.min) / 2;

        // Remove this center from all verts
        for(int i=0;i<numVerts;i++)
		{
            Vertex v = mesh->GetVertex(&Verts[i*stride]);
			v.position -= center;
			mesh->SetVertex(&Verts[i*stride],v);
        }
        // Update tm with new pivot center
        param.m3 = center;

        // Recalc bbox
		DXASSERT(mesh->UpdateBoundingInfo((D3DXVECTOR3*)&Verts[0],pMesh->GetNumVertices(),stride,IndexesBuffer,bDWORDIndices));
  
    }
    if(op == BakeTM)
    {
        Matrix tm = param;
		Matrix rot = tm.GetRotationMatrix();
		for(int i=0;i<numVerts;i++)
		{
            Vertex v = mesh->GetVertex(&Verts[i*stride]);
            v.position = tm * v.position;
            v.normal = rot * v.normal;
            v.tan  = rot * v.tan;
            mesh->SetVertex(&Verts[i*stride],v);
		}

		// Recalc bbox
		DXASSERT(mesh->UpdateBoundingInfo((D3DXVECTOR3*)&Verts[0],pMesh->GetNumVertices(),stride,IndexesBuffer,bDWORDIndices));
        
    }
	// Invert mesh or normals
	if(op == Invert || op == InvertNormals)
	{
		// Flip Normals/Tangent
		for(int j=0;j<pMesh->GetNumVertices();j++)
		{
            Vertex v = mesh->GetVertex(&Verts[j*stride]);
            v.normal = -v.normal;
            v.tan  = -v.tan;
            mesh->SetVertex(&Verts[j*stride],v);
		}

		// Invert actual mesh too??
		if(op == Invert)
		{
			// Flip winding order for faces
			for(int j=0;j<numIndices;j+=3)
			{
				if(bDWORDIndices)
				{
					DWORD temp = dwIndices[j+0];
					dwIndices[j+0] = dwIndices[j+2];
					dwIndices[j+2] = temp;
				}
				else
				{
					WORD temp = wIndices[j+0];
					wIndices[j+0] = wIndices[j+2];
					wIndices[j+2] = temp;
				}
			}
		}
	}
	if(op == FromMax)
	{
		// Flip Normals/Tangent
		for(int j=0;j<pMesh->GetNumVertices();j++)
		{
            Vertex v = mesh->GetVertex(&Verts[j*stride]);
            MaxToD3D(&v.position);
            MaxToD3D(&v.normal);
            MaxToD3D(&v.tan);
            mesh->SetVertex(&Verts[j*stride],v);
		}

		// Flip winding order for faces
		for(int j=0;j<numIndices;j+=3)
		{
			if(bDWORDIndices)
			{
				DWORD temp = dwIndices[j+0];
				dwIndices[j+0] = dwIndices[j+2];
				dwIndices[j+2] = temp;
			}
			else
			{
				WORD temp = wIndices[j+0];
				wIndices[j+0] = wIndices[j+2];
				wIndices[j+2] = temp;
			}
		}
	}

	pMesh->UnlockIndexBuffer();
	pMesh->UnlockVertexBuffer();

	return true;
}


//--------------------------------------------------------------------------------------
//
// D3D Mesh helper
//
//--------------------------------------------------------------------------------------
bool MeshOps::Mend(Mesh* mesh, bool genNormals, float creaseAngle, float weightNormals, bool fixCylindrical, bool respectSplits)
{
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

    std::vector< MeshMender::Vertex > theVerts;
    std::vector< unsigned int > theIndices;
    std::vector< unsigned int > mappingNewToOld;

    //fill up the vectors with your mesh's data
    for(DWORD i = 0; i < numVerts; ++i)
    {
        MeshMender::Vertex v;

        Vertex vert = mesh->GetVertex(&Verts[i*stride]);

        // We don't know the vertex format, but we know the first few elements and the stride, so copy these
        v.pos = *(D3DXVECTOR3*)&vert.position;
        v.normal  = *(D3DXVECTOR3*)&vert.normal;
        v.tangent = *(D3DXVECTOR3*)&vert.tan;
        v.s = vert.tex.x;
        v.t = vert.tex.y;

        //meshmender will computer normals, tangents, and binormals, no need to fill those in.
        //however, if you do not have meshmender compute the normals, you _must_ pass in valid
        //normals to meshmender
        theVerts.push_back(v);
    }

    for(DWORD ind= 0 ; ind< numIndices; ++ind)
    {
        if(bDWORDIndices)
            theIndices.push_back(dwIndices[ind]);
        else
            theIndices.push_back(wIndices[ind]);
    }

    creaseAngle = cos(DEG2RAD(creaseAngle-90));

    // Pass it in to the mender to do it's stuff
    MeshMender mender;
    mender.Mend( theVerts,  theIndices, mappingNewToOld,
        creaseAngle,
        creaseAngle,
        creaseAngle,
        weightNormals,
        genNormals?MeshMender::CALCULATE_NORMALS:MeshMender::DONT_CALCULATE_NORMALS,
        respectSplits?MeshMender::RESPECT_SPLITS:MeshMender::DONT_RESPECT_SPLITS,
        fixCylindrical?MeshMender::FIX_CYLINDRICAL:MeshMender::DONT_FIX_CYLINDRICAL);

    assert(_CrtCheckMemory());

    // Now we have new data, create a new VB to hold it
    BYTE* newVertices = new BYTE[theVerts.size()*stride];
    for(int i=0;i<theVerts.size();i++)
    {
        // Copy the old vertex data
        memcpy(&newVertices[i*stride],&Verts[mappingNewToOld[i]*stride],stride);
        // Add the new vertex data
        Vertex vert;
        vert.position = *(Vector*)&theVerts[i].pos;
        vert.normal = *(Vector*)&theVerts[i].normal;
        vert.tan = *(Vector*)&theVerts[i].tangent;
        vert.tex.x = theVerts[i].s;
        vert.tex.y = theVerts[i].t;

        mesh->SetVertex(&newVertices[i*stride],vert);
    }

    // New IB, checking for 16/32 bit indices
    int newIndSize = (theIndices.size() >= 65535?sizeof(DWORD):sizeof(WORD));
    BYTE* newIndices = new BYTE[theIndices.size()*newIndSize];
    for(int i=0;i<theIndices.size();i++)
    {
        if(newIndSize == sizeof(DWORD))
            *(DWORD*)&newIndices[i*newIndSize] = (DWORD)theIndices[i];
        else
            *(WORD*)&newIndices[i*newIndSize] = (WORD)theIndices[i];
    }

    // Unlock old buffer
    pMesh->UnlockIndexBuffer();
    pMesh->UnlockVertexBuffer();

    // Create, but backup attribs first
    vector<AttributeRange> attribTable = mesh->m_AttribTable;
    mesh->m_AttribTable.clear();
    mesh->Create(newVertices,newIndices,stride,newIndSize,theVerts.size(),theIndices.size(),mesh->m_Materials);

    // Remap attrib buffer
    mesh->m_AttribTable.clear();
    for(int i=0;i<attribTable.size();i++)
    {
        AttributeRange r;
        r.AttribId = attribTable[i].AttribId;
        r.FaceCount = attribTable[i].FaceCount;
        r.FaceStart = attribTable[i].FaceStart;
        r.VertexCount = mappingNewToOld[attribTable[i].VertexStart+(attribTable[i].VertexCount-1)]-attribTable[i].VertexStart;
        r.VertexStart = mappingNewToOld[attribTable[i].VertexStart];
        //r.VertexOffset = 0; // Unused
        mesh->m_AttribTable.push_back(r);
    }
    mesh->CalcData();

    mesh->Optimize(true);

    delete[] newIndices;
    delete[] newVertices;


    return true;
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

            if(groups[i].indices[j] >= groups[i].table.VertexStart+groups[i].table.VertexCount)
                groups[i].table.VertexCount = groups[i].indices[j]-groups[i].table.VertexStart + 1;
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


//--------------------------------------------------------------------------------------
//
// Breaks a mesh into a bunch of new meshes - one per sub attribute
//
//--------------------------------------------------------------------------------------
bool MeshOps::Break(Mesh* mesh, Actor* actor, vector<Actor*>& newMeshes)
{
    if(mesh->m_AttribTable.size() < 2)
    {
        SeriousWarning("MeshOps::Break: Mesh only has one subattribute/submaterial, nothing to break!\n"
            "This function breaks subelements into seperate meshes.");
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

    for(int i=0;i<mesh->m_AttribTable.size();i++)
    {
        AttributeRange r = mesh->m_AttribTable[i];
        Mesh* newMesh = new Mesh();

        BYTE* indices = new BYTE[(bDWORDIndices?4:2)*r.FaceCount*3];

        // Copy indices, offsetting by face start so they begin at 0
        for(int j=0;j<r.FaceCount*3;j++)
        {
            if(bDWORDIndices)
                ((DWORD*)indices)[j] = dwIndices[r.FaceStart*3 + j] - r.VertexStart;
            else
                ((WORD*)indices)[j] = wIndices[r.FaceStart*3 + j] - r.VertexStart;
        }


        BYTE* vertices = (BYTE*)&Verts[stride*r.VertexStart];

        vector<Material*> materials;
        materials.push_back(mesh->GetMaterial(r.AttribId));

        if(!newMesh->Create(vertices,indices,pMesh->GetNumBytesPerVertex(),
            bDWORDIndices?4:2,r.VertexCount,r.FaceCount*3,materials))
        {
            SeriousWarning("Mesh split failed with %d verts and %d inds",r.VertexCount,r.FaceCount*3);
        }

        delete[] indices;

        Prefab* prefab = new Prefab(actor->MyWorld);
        actor->Clone(prefab,false);
        
        vector<ModelFrame*> frames;
        prefab->MyModel->m_pFrameRoot->EnumerateMeshes(frames);
        if(frames.size() > 1)
            SeriousWarning("MeshOps::Break: >1 frames. This should not have happened");

        frames[0]->SetMesh(newMesh);
        // Ensure unique mesh names
        prefab->MyModel->m_pFrameRoot->FixNames(actor->MyWorld);

        newMeshes.push_back(prefab);
    }

    pMesh->UnlockIndexBuffer();
	pMesh->UnlockVertexBuffer();

    // Delete source actor, been replaced with split actors
    delete actor;
    return true;
}