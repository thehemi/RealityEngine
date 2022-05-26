//=========== (C) Copyright 2003, Tim Johnson. All rights reserved. ===========
//
// TODO: Support frame ops for lods other than 0, particularly skinning matrices
//=============================================================================
#include "stdafx.h"
#include "d3dcustom.h" 
#include "Engine.h"
#include "Frame.h"
#include "Collision.h"
#include "Editor.h"
#include "LODManager.h"

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
Mesh* ModelFrame::GetMesh(int lod)
{ 
	if(m_MeshLODs.size() <= lod) 
		return 0; 

	return m_MeshLODs[lod]; 
}

//-----------------------------------------------------------------------------
// Sets a mesh and builds LODs
//-----------------------------------------------------------------------------
void ModelFrame::SetMesh(Mesh* mesh)
{
	m_MeshLODs.clear();
	m_MeshLODs.push_back(mesh);
	LODManager::Instance()->GenerateLODs(m_MeshLODs);
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
ModelFrame* ModelFrame::Clone(bool bCloneMesh)
{
	ModelFrame *newFrame = new ModelFrame;

	// Copy frame data
	*newFrame = *this;

	if(bCloneMesh && GetMesh()){
		for(int i=0;i<this->GetNumLODs();i++)
		{
			Mesh* m = new Mesh();
			this->GetMesh(i)->Clone(m);
			newFrame->SetLOD(m,i);
		}
	}

    if (pFrameSibling != NULL)
    {
		newFrame->pFrameSibling = pFrameSibling->Clone(bCloneMesh);
    }

    if (pFrameFirstChild != NULL)
    {
		newFrame->pFrameFirstChild = pFrameFirstChild->Clone(bCloneMesh);
    }
	return newFrame;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void ModelFrame::FixNames(World* world)
{
    // FIXME/TODO/NOTE: This is very slow when cloning in huge scenes
    string newName = Name;
    do{ newName = GenerateNewName(newName); }
    while(world->FindMeshFrame(newName,this));
    Name = newName;
}

//-----------------------------------------------------------------------------
// Finds a frame material 
//-----------------------------------------------------------------------------
Material* ModelFrame::FindMat(string& name){
	if(GetMesh()){
		for(int i=0;i<GetMesh()->m_Materials.size();i++){
			if(GetMesh()->m_Materials[i]->m_Name == name)
				return GetMesh()->m_Materials[i];
		}
	}

	if (pFrameSibling != NULL){
		Material* ptr = pFrameSibling->FindMat(name);
		if(ptr)
			return ptr;
	}

	if (pFrameFirstChild != NULL){
		Material* ptr = pFrameFirstChild->FindMat(name);
		if(ptr)
			return ptr;
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Finds frame meshes
//-----------------------------------------------------------------------------
void ModelFrame::EnumerateMeshes(vector<ModelFrame*>& meshes){
	if(GetMesh()){
		meshes.push_back(this);
	}

	if (pFrameSibling != NULL){
		pFrameSibling->EnumerateMeshes(meshes);
	}

	if (pFrameFirstChild != NULL){
		pFrameFirstChild->EnumerateMeshes(meshes);
	}
}
//-----------------------------------------------------------------------------
// Finds frame siblings and children
//-----------------------------------------------------------------------------
void ModelFrame::EnumerateFrames(vector<ModelFrame*>& meshes){
	meshes.push_back(this);

	if (pFrameSibling != NULL){
		pFrameSibling->EnumerateFrames(meshes);
	}

	if (pFrameFirstChild != NULL){
		pFrameFirstChild->EnumerateFrames(meshes);
	}
}

//-----------------------------------------------------------------------------
// FIXME: This doesn't work with skinning. Mesh probably needs reinit
//-----------------------------------------------------------------------------
void ModelFrame::Scale(Vector& scale)
{
    TransformationMatrix.m3         *= scale;
    CombinedTransformationMatrix.m3 *= scale;
	if(GetMesh())
    {
        Matrix tm;
        tm.SetScales(scale.x,scale.y,scale.z);
        MeshOps::Convert(GetMesh(),MeshOps::BakeTM,tm);

        if(collisionMesh)
		{
			collisionMesh->Destroy();
			collisionMesh->Initialize(GetMesh());
		}

        // Scale bone offsets
        for(int i=0;i<GetMesh()->GetNumBones();i++)
        {
            D3DXMATRIX* m = GetMesh()->m_pSkinInfo->GetBoneOffsetMatrix(i);
            (*m)._41 *= scale.x;
            (*m)._42 *= scale.y;
            (*m)._43 *= scale.z;
        }
       // if(GetMesh()->GetNumBones())
       //     GetMesh()->GenerateSkinInfo();
        
    }

	if (pFrameSibling != NULL){
		pFrameSibling->Scale(scale);
	}

	if (pFrameFirstChild != NULL){
		pFrameFirstChild->Scale(scale);
	}
}

//-----------------------------------------------------------------------------
// Finds frame materials
//-----------------------------------------------------------------------------
void ModelFrame::FindMaterials(vector<Material*>& materials){
	if(GetMesh()){
		for(int i=0;i<GetMesh()->m_Materials.size();i++){

			// Make sure this material isn't already on the list
			bool found = false;
			for(int j=0;j<materials.size();j++){
				if(materials[j]->m_Name == GetMesh()->m_Materials[i]->m_Name){
					found = true;
					break;
				}
			}

			// FIXME: Default will fuck things up like submat indexing if user lacks materials
            // TIM: I'm disabling the default check, because it prevents default mat submats from being resaved
			if(!found)// && GetMesh()->m_Materials[i] != MaterialManager::Instance()->GetDefaultMaterial())
				materials.push_back(GetMesh()->m_Materials[i]);
		}
	}

	if (pFrameSibling != NULL){
		pFrameSibling->FindMaterials(materials);
	}

	if (pFrameFirstChild != NULL){
		pFrameFirstChild->FindMaterials(materials);
	}
}

//-----------------------------------------------------------------------------
// Finds a frame 
//-----------------------------------------------------------------------------
ModelFrame* ModelFrame::Find(string& name, ModelFrame* ignore){
    if(this == ignore)
        return NULL;

	if(Name == name)
		return this;

	if (pFrameSibling != NULL){
		ModelFrame* ptr = pFrameSibling->Find(name);
		if(ptr)
			return ptr;
	}

	if (pFrameFirstChild != NULL){
		ModelFrame* ptr = pFrameFirstChild->Find(name);
		if(ptr)
			return ptr;
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Total bounding box encompassing all meshes in hierarchy
//-----------------------------------------------------------------------------
BBox ModelFrame::GetWorldBBox(){
	BBox box;
	if(GetMesh())
	{
		box = GetMesh()->m_LocalBox.Transformed(CombinedTransformationMatrix);

        // Use this as an opportunity to update world attrib boxes
        m_WorldAttribBoxes.clear();
        for(int i=0;i<GetMesh()->m_AttribBoxes.size();i++)
            m_WorldAttribBoxes.push_back(GetMesh()->m_AttribBoxes[i].Transformed(CombinedTransformationMatrix));
	}

	if(pFrameSibling){
		BBox sub = pFrameSibling->GetWorldBBox();
		if(!(sub == BBox())){
			box += sub.min;
			box += sub.max;
		}
	}
	if(pFrameFirstChild){
		BBox sub = pFrameFirstChild->GetWorldBBox();
		if(!(sub == BBox())){
			box += sub.min;
			box += sub.max;
		}
	}

	return box;
}

//-----------------------------------------------------------------------------
// Removes scene embedded offsets from root nodes
// To do this, it strips from the first model frames
// TODO: This must strip light once we implement lights
//
// FIXME/WTF/TODO: Why were we only stripping from mesh frame?
// Stripping from all frames has corrected bugs we were seeing in the mansion prefabs
// and makes sense because the xref tm completely overrides the model file tm.
// Keep an eye on this!!!
//-----------------------------------------------------------------------------
void ModelFrame::RemoveSceneOffset(){
	//if(mesh){
		CombinedTransformationMatrix = Matrix();
		TransformationMatrix		 = Matrix();
	//}

	if(pFrameSibling)
		pFrameSibling->RemoveSceneOffset();
	if(pFrameFirstChild && !GetMesh()) // Only strip from children if we haven't been stripped
		pFrameFirstChild->RemoveSceneOffset();
}


//-----------------------------------------------------------------------------
// Name: UpdateFrameMatrices()
// Desc: update the frame matrices
//-----------------------------------------------------------------------------
void ModelFrame::UpdateMatrices(Matrix ParentMatrix)
{
	D3DXMatrixMultiply((D3DXMATRIX*)&CombinedTransformationMatrix,(D3DXMATRIX*)&CustomTransformationMatrix,(D3DXMATRIX*)&TransformationMatrix);
	D3DXMatrixMultiply((D3DXMATRIX*)&CombinedTransformationMatrix,(D3DXMATRIX*)&CombinedTransformationMatrix,(D3DXMATRIX*)&ParentMatrix);

	// Add in custom influence
	//CombinedTransformationMatrix = CustomTransformationMatrix * TransformationMatrix;
   // CombinedTransformationMatrix = CombinedTransformationMatrix * ParentMatrix;

    if (pFrameSibling != NULL)
    {
		pFrameSibling->UpdateMatrices(ParentMatrix);
    }

    if (pFrameFirstChild != NULL)
    {
        pFrameFirstChild->UpdateMatrices(CombinedTransformationMatrix);
    }
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
ModelFrame::ModelFrame()
{
	pFrameSibling		= NULL;
	pFrameFirstChild	= NULL;
	collisionMesh		= NULL;
	m_CurrentLOD		= 0;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
ModelFrame::~ModelFrame()
{
	SAFE_DELETE(pFrameSibling);
	SAFE_DELETE(pFrameFirstChild);
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void ModelFrame::FreeContents()
{
	SAFE_DELETE_VECTOR(m_MeshLODs);
	SAFE_DELETE(collisionMesh);
	if(pFrameSibling)
		pFrameSibling->FreeContents();
	if(pFrameFirstChild)
		pFrameFirstChild->FreeContents();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ModelFrame::AppendChild(ModelFrame* child){
	child->pFrameSibling = pFrameFirstChild;
	pFrameFirstChild = child;
}


//-----------------------------------------------------------------------------
// Registers the bone pointers for a skinned mesh in the frame, by collecting 
// the pointers from all other bone frames
//-----------------------------------------------------------------------------
void ModelFrame::RegisterSkinMatrices(ModelFrame* root){
	int total = 1;

	if(GetMesh() && GetMesh()->GetNumBones()){
		bone_matrices.clear();
		for(int i=0;i<GetMesh()->GetNumBones();i++){
			ModelFrame* frame = root->Find(GetMesh()->GetBoneName(i));
			assert(frame);
			bone_matrices.push_back(&frame->CombinedTransformationMatrix);
		}
	}

	if (pFrameSibling != NULL)
		pFrameSibling->RegisterSkinMatrices(root);

	if (pFrameFirstChild != NULL)
		pFrameFirstChild->RegisterSkinMatrices(root);
}

//-----------------------------------------------------------------------------
// Total number of frames
//-----------------------------------------------------------------------------
int ModelFrame::CountFrames(){
	int total = 1;

	if (pFrameSibling != NULL)
		total += pFrameSibling->CountFrames();

	if (pFrameFirstChild != NULL)
		total += pFrameFirstChild->CountFrames();

	return total;
}

//-----------------------------------------------------------------------------
// Registers frames as output matrices
// Animation data will be output to the frame transforms
//-----------------------------------------------------------------------------
void ModelFrame::Register(LPD3DXANIMATIONCONTROLLER ac){
	HRESULT hr;
	if(FAILED(hr=ac->RegisterAnimationOutput(Name.c_str(),(D3DXMATRIX*)&TransformationMatrix,0,0,0))){
		LogPrintf("RegisterAnimationOutput failed for %s",Name.c_str());
	}

	if (pFrameSibling != NULL)
		pFrameSibling->Register(ac);

    if (pFrameFirstChild != NULL)
		pFrameFirstChild->Register(ac);
}

void ModelFrame::Draw()
{
	ModelFrame* pFrame = this;

	if(pFrame->pFrameFirstChild)
		pFrame->pFrameFirstChild->Draw();

	if(pFrame->pFrameSibling)
		pFrame->pFrameSibling->Draw();

	if(!pFrame->GetMesh(0))
		return;

	for (int i=0;i<pFrame->GetMesh()->m_AttribTable.size();i++)
		pFrame->GetMesh(0)->DrawSubset(pFrame->GetMesh(0)->m_AttribTable[i]);
}

//-----------------------------------------------------------------------------
// Name: UpdateFrameMatrices()
// Desc: update only the children matrices
//-----------------------------------------------------------------------------
void ModelFrame::UpdateChildrenMatrices()
{
	if (pFrameFirstChild != NULL)
	{
		pFrameFirstChild->UpdateMatrices(CombinedTransformationMatrix);
	}
}