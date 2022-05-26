//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
/// Name: Frame.h
///
/// Author: Tim Johnson
//====================================================================================
#pragma once

//-----------------------------------------------------------------------------
/// Name: struct ModelFrame
/// \brief A hierarchially transformed element in a Model, can contain a Mesh or represent an animation bone
//-----------------------------------------------------------------------------
struct ENGINE_API ModelFrame 
{
protected:
	/// LOD levels. Use public accessors
	vector<Mesh*>	m_MeshLODs; 

public:
    /// Store world-space attrib boxes if has mesh with attribute culling
    vector<BBox>    m_WorldAttribBoxes;

    /// Frame name
	string		  Name;
    /// Relative TM
	Matrix        TransformationMatrix;
    /// CustomTransformationMatrix*TransformationMatrix*ParentTM
    Matrix        CombinedTransformationMatrix;
    // User influence
	Matrix		  CustomTransformationMatrix;

	ModelFrame    *pFrameSibling;
	ModelFrame    *pFrameFirstChild;

	/// Current LOD this frame, set when ChooseBestLOD is called
	int			m_CurrentLOD;	
	/// Sets a mesh and builds LODs
	void		SetMesh(Mesh* mesh);
	/// Gets a mesh at a specified LOD
	Mesh*		GetMesh(int lod=0);
	/// Number of lods
	int			GetNumLODs(){ return m_MeshLODs.size(); }
	/// Sets a LOD
	void		SetLOD(Mesh* mesh, int lod){ 
		if(lod>= m_MeshLODs.size()) m_MeshLODs.resize(lod+1);
		m_MeshLODs[lod] = mesh;
	}

	/// Sorthand, holds frame matrix pointers that mesh can use
	vector<Matrix*> bone_matrices; 
	/// With collision data
	CollisionMesh*	collisionMesh; 

	ModelFrame();
	~ModelFrame();
	/// Actually frees the contained data
	void		FreeContents(); 
	void		AppendChild(ModelFrame* child);
	int			CountFrames();
	void		Register(LPD3DXANIMATIONCONTROLLER ac);
	ModelFrame* Find(string& name, ModelFrame* ignore=0);
	Material*	FindMat(string& matName);
	void		FindMaterials(vector<Material*>& materials);
	void		EnumerateMeshes(vector<ModelFrame*>& meshes);
	void		EnumerateFrames(vector<ModelFrame*>& meshes);
	void		UpdateMatrices(Matrix pParentMatrix);
	void		RemoveSceneOffset();
	BBox		GetWorldBBox();
	ModelFrame* Clone(bool bCloneMesh);
	void		RegisterSkinMatrices(ModelFrame* root);
	void Draw();
	void UpdateChildrenMatrices();
    /// Scales hierarchy, meshes, skinning data, etc
    void        Scale(Vector& scale);
    /// Assigns unique names to all frames
    void FixNames(World* world);
};


