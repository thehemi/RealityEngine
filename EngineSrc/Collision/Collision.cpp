//=========== Copyright (c) 2003, Tim Johnson. All rights reserved. ===========
// Collision.h -- Core Collision Classes
//=============================================================================
#include "stdafx.h"
#include "Collision.h"
using namespace IceMaths;

CCollider collider;

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CollisionMesh::Initialize(BYTE* pVerts, BYTE* pIndices, int stride, 
                               int indexSize, int numVertices, int numIndices, vector<Material*>& materials)
{
    WORD* indices = (WORD*)pIndices;
    faces.resize(numIndices/3);
	vertices.resize(numVertices);

    int offset = 0; // For when using subsets

    // Add collision vertices
    for(int j=0;j<vertices.size();j++)
        vertices[j] = *(((Vector*)&pVerts[j*stride]));

    // Calculate collision faces
    for(int j=0;j<faces.size();j++){
        CollisionMesh::Face& f = faces[j];

        if(indexSize==4){
            DWORD* dwIndices = (DWORD*)indices;
            f.indices[0] = dwIndices[j*3 + 0] + offset;
            f.indices[1] = dwIndices[j*3 + 1] + offset;
            f.indices[2] = dwIndices[j*3 + 2] + offset;
        }
        else{
            f.indices[0] = indices[j*3 + 0] + offset;
            f.indices[1] = indices[j*3 + 1] + offset;
            f.indices[2] = indices[j*3 + 2] + offset;
        }

        Vector E0 = vertices[f.indices[1]] - vertices[f.indices[0]];
        Vector E1 = vertices[f.indices[2]] - vertices[f.indices[1]];
        Vector E2 = vertices[f.indices[0]] - vertices[f.indices[2]];

        // Face normal
        f.normal = Cross(E0,E1);
        f.mat = materials.size()?materials[0]:0;

        // Edge normals pointing outwards
		f.edgeNorm[0] = Cross((vertices[f.indices[1]] - vertices[f.indices[2]]),-f.normal);
		f.edgeNorm[1] = Cross((vertices[f.indices[0]] - vertices[f.indices[1]]),-f.normal);
		f.edgeNorm[2] = Cross((vertices[f.indices[2]] - vertices[f.indices[0]]),-f.normal);
    }
    localSpace = true;
    Initialize();
}


//-----------------------------------------------------------------------------
// FIXME: Needs to handle subset ids. Currently uses first material only
//-----------------------------------------------------------------------------
void CollisionMesh::Initialize(Mesh* mesh)
{
	// Extract buffers from mesh
	BYTE* VerticesBuffer;
    if(!mesh->GetHardwareMesh())
    {
        SeriousWarning("No mesh to build collision info from. FIXME: Load directly");
        return;
    }

	mesh->GetHardwareMesh()->LockVertexBuffer(D3DLOCK_READONLY, (LPVOID*)&VerticesBuffer);

	WORD* indices;
	mesh->GetHardwareMesh()->LockIndexBuffer(D3DLOCK_READONLY, (LPVOID*)&indices);
	bool bDWORDIndices = mesh->GetHardwareMesh()->GetOptions() & D3DXMESH_32BIT;

    Initialize(VerticesBuffer,(BYTE*)indices,mesh->GetHardwareMesh()->GetNumBytesPerVertex(),bDWORDIndices?4:2,mesh->GetHardwareMesh()->GetNumVertices(),mesh->GetHardwareMesh()->GetNumFaces()*3,mesh->m_Materials);

	mesh->GetHardwareMesh()->UnlockVertexBuffer();
	mesh->GetHardwareMesh()->UnlockIndexBuffer();
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void CollisionMesh::Initialize(){
	// 1) Initialize the creation structure
	Opcode::OPCODECREATE OPCC;
	OPCC.mIMesh = new Opcode::MeshInterface;
	OPCC.mIMesh->SetNbTriangles(faces.size());
	OPCC.mIMesh->SetNbVertices(vertices.size());
	OPCC.mIMesh->SetStrides(sizeof(CollisionMesh::Face));
	OPCC.mIMesh->SetPointers((IceMaths::IndexedTriangle*)&faces[0],(IceMaths::Point*)&vertices[0]);
	mInterface = OPCC.mIMesh; // Keep pointer so that we can free it properly later
	// Tree building settings
	OPCC.mSettings.mRules = Opcode::SPLIT_SPLATTER_POINTS | Opcode::SPLIT_GEOM_CENTER;  
	// Max 16 tris (this is FORCED on HybridModels anyway)
	// Usually Opcode builds trees with a limit of 1, as this allows certain optimizations
	// But the hybrid model allows us to build smaller trees that are still as optimized
	//OPCC.mSettings.mLimit = 1; 
	OPCC.mNoLeaf     = true; 
	OPCC.mQuantized = true;
	ocModel = new Opcode::Model();   
	ocModel->Build(OPCC);
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
CollisionMesh::~CollisionMesh()
{
	Destroy();
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void CollisionMesh::Destroy()
{
	faces.clear();
	vertices.clear();
	SAFE_DELETE(mInterface);
	SAFE_DELETE(ocModel);
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void CCollider::Initialize(){
	AC.SetFirstContact(false);
	AC.SetTemporalCoherence(false);

	RC.SetFirstContact(false);
	RC.SetTemporalCoherence(true);
	RC.SetClosestHit(true); // Only return nearest hit
	// FIXME: Our wind order is wrong, so using culling culls front faces
	RC.SetCulling(false); // Don't hit backfaces
	//RC.SetMaxDist(...);
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
bool CCollider::CollideRay(const Vector& start, const Vector& end, CollisionMesh* node,  CollisionInfo& result, Matrix modelTM){
	if(!node)
		return false;

	if(_isnan(modelTM.m0.x))
    {
#ifdef _DEBUG
        Error("Warning: Input model to CollideRay has an #IND000 matrix");
#endif
        Warning("Warning: Input model to CollideRay has an #IND000 matrix");
        return false;
    }

	Ray WorldRay(*(Point*)&start,*(Point*)&(end-start).Normalized());

	RC.SetMaxDist((end-start).Length());
	
	Opcode::CollisionFaces CF;
	RC.SetDestination(&CF);
	RC.SetCulling(true);

	static udword Cache;
	bool IsOk = RC.Collide(WorldRay, *node->ocModel, (Matrix4x4*)&modelTM, &Cache);

	int faces = CF.GetNbFaces();
	if(faces){
		if(faces != 1)
			Error("More than one face!");

		Opcode::CollisionFace f = CF.GetFaces()[0];
		CollisionMesh::Face& face = node->faces[f.mFaceID];

		result.mat = face.mat;
		result.actualDistance = f.mDistance;
		result.normal = -face.normal;
		result.point = node->vertices[face.indices[0]]+f.mU*(node->vertices[face.indices[1]]-node->vertices[face.indices[0]])+f.mV*(node->vertices[face.indices[2]]-node->vertices[face.indices[0]]);
		
		if(node->localSpace){
			result.point = modelTM * result.point;
			result.normal = modelTM.GetRotationMatrix() * result.normal;
		}
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void CCollider::CollideAABB(Actor* source, BBox box, CollisionMesh* node,  vector<CollisionFace>& faces, const Vector& moveDir, Matrix modelTM){
	if(!node)
		return;

	if(node->faces.size() == 0)
		return;

	if(node->localSpace)
		box = box.Transformed(modelTM.Inverse());
	
	Opcode::CollisionAABB LocalAABB;

	LocalAABB.SetMinMax(*(IceMaths::Point*)&box.min,*(IceMaths::Point*)&box.max);
	RC.SetCulling(false);
	Opcode::AABBCache test;
	bool IsOk = AC.Collide(test, LocalAABB, *node->ocModel);
	// The contact status is given by :
	BOOL Status = AC.GetContactStatus();

	// List of touched primitives is given by :
	int numFaces = AC.GetNbTouchedPrimitives();
	const udword* faceIndices = AC.GetTouchedPrimitives();

    Matrix tmRot = modelTM.GetRotationMatrix();
	// Build CollisionFaces
	for(int i=0;i<numFaces;i++){
		CollisionMesh::Face& face = node->faces[faceIndices[i]];
		CollisionFace cFace;

		cFace.owner = source;
		cFace.mat = face.mat;
		cFace.normal = -face.normal;

		cFace.vert[2] = node->vertices[face.indices[0]];
		cFace.vert[1] = node->vertices[face.indices[1]];
		cFace.vert[0] = node->vertices[face.indices[2]];

        cFace.edgeNorm[0] = face.edgeNorm[0];
        cFace.edgeNorm[1] = face.edgeNorm[1];
        cFace.edgeNorm[2] = face.edgeNorm[2];

		// If this model's in localspace, convert the output to worldspace
        // TODO: Optimize this! Cache the world positiosn somewhere
		if(node->localSpace)
        {
            D3DXVec3TransformNormal((D3DXVECTOR3*)&cFace.normal,(D3DXVECTOR3*)&cFace.normal,(D3DXMATRIX*)&tmRot);
            //cFace.normal = tmRot * cFace.normal;
			for(int i=0;i<3;i++)
            {
                //cFace.vert[i]       =  *(Vector*)D3DXVec3Transform(0,(D3DXVECTOR3*)&cFace.vert[i],(D3DXMATRIX*)&modelTM);
                D3DXVec3TransformNormal((D3DXVECTOR3*)&cFace.edgeNorm[i],(D3DXVECTOR3*)&cFace.edgeNorm[i],(D3DXMATRIX*)&tmRot);
				cFace.vert[i] = modelTM * cFace.vert[i];
				//cFace.edgeNorm[i] = tmRot * cFace.edgeNorm[i];
			}
		}

		if(moveDir.Dot(cFace.normal) > 0)
			continue;
		faces.push_back(cFace);
	}
}


//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void CCollider::GatherRenderingPolys(BBox box, CollisionMesh* node,  
                                     vector<Vertex>& verts, const Vector& moveDir, Matrix& modelTM)
{
	if(!node)
		return;

	if(node->faces.size() == 0)
		return;

	if(node->localSpace)
		box = box.Transformed(modelTM.Inverse());
	
	Opcode::CollisionAABB LocalAABB;
	LocalAABB.SetMinMax(*(IceMaths::Point*)&box.min,*(IceMaths::Point*)&box.max);
	RC.SetCulling(false);
	Opcode::AABBCache test;
	bool IsOk = AC.Collide(test, LocalAABB, *node->ocModel);
	// The contact status is given by :
	BOOL Status = AC.GetContactStatus();

	// List of touched primitives is given by :
	int numFaces = AC.GetNbTouchedPrimitives();
	const udword* faceIndices = AC.GetTouchedPrimitives();

    int v = verts.size();
    verts.resize(verts.size()+numFaces*3);
    
	// Build CollisionFaces
	for(int i=0;i<numFaces;i++)
    {
		CollisionMesh::Face& face = node->faces[faceIndices[i]];
		//if(moveDir.Dot(cFace.normal) > 0)
		//	continue;

        verts[v++].position = modelTM * node->vertices[face.indices[0]];
        verts[v++].position = modelTM * node->vertices[face.indices[1]];
        verts[v++].position = modelTM * node->vertices[face.indices[2]];
	}
}

