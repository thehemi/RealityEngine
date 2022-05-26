//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
/// Collision.h -- Core Collision Classes
///
/// Author: Tim Johnson
//====================================================================================

#include "..\opcode\opcode.h"

//-----------------------------------------------------------------------------
/// Contains the geometric data used for calculation collisions into a mesh
//-----------------------------------------------------------------------------
struct CollisionMesh 
{
protected:
	void Initialize();

public:
	/// One face in a CollisionMesh
	struct Face
	{
		DWORD indices[3];
		Material* mat;
		Vector normal;
        Vector edgeNorm[3];
	};

	/// Frame-to-frame Coherency cache for faster checks
	Opcode::AABBCache		Cache;
	/// OPCODE collision model/tree
	Opcode::Model*			ocModel; 
	/// OPCODE structure
	Opcode::MeshInterface*	mInterface;
	vector<Vector>	vertices;
	vector<Face>	faces;
	/// Data in localspace?
	bool			localSpace;

	~CollisionMesh();
	CollisionMesh(){}
	/// Destroys all data
	void Destroy();
	/// Creates collision mesh from source mesh
	void Initialize(Mesh* mesh);
    /// Creates collision mesh from source data
    void Initialize(BYTE* vertices, BYTE* indices, int vertexSize, int indexSize, int numVertices, int numIndices,vector<Material*>& materials); 
};

//-----------------------------------------------------------------------------
/// 
//-----------------------------------------------------------------------------
/// Computes basic AABB and raytest collisions against a node in the World
class CCollider {
	Opcode::RayCollider RC;
	Opcode::AABBCollider AC;

public:

	void Initialize();
	void CollideAABB(Actor* source, BBox box, CollisionMesh* node,  vector<CollisionFace>& faces, const Vector& moveDir, Matrix modelTM = Matrix());
	bool CollideRay(const Vector& start, const Vector& end, CollisionMesh* node,  CollisionInfo& result, Matrix modelTM = Matrix());
    void GatherRenderingPolys(BBox box, CollisionMesh* node,  
                                     vector<Vertex>& verts, const Vector& moveDir, Matrix& modelTM);
};

extern CCollider collider;

/// Functions used by other classes ..
CollisionInfo AABBSweep(Actor* source, vector<CollisionFace>& touchList, const Vector& start, const Vector& dir,const BBox& worldBox);
//void GatherPossibleColliders(World* world, Actor* source, vector<CollisionFace>& touchList, const Vector& start, const Vector& move,const BBox& worldBox);
bool IsStuck(Actor* source, vector<CollisionFace>& touchList, const BBox& worldBox, CollisionInfo& outTrace, bool largerBBox);
bool RayCheckBBox(const Vector& start, const Vector& end, Actor* actor, CollisionInfo& info);
void CheckBBox(BBox& sweepBox, Actor* actor, vector<CollisionFace>& touchList);
