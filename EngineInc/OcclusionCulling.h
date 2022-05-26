//=========== (C) Copyright 2004, Artficial Studios. All rights reserved. =======
// Occlusion Culling
// Author: Mostafa Mohamed
// See .cpp for comments and usage
//
//===============================================================================
#ifndef OcclusionCulling_INCLUDED
#define OcclusionCulling_INCLUDED

#include <hash_map>
#include <queue> 
#include <stack>
#include "..\shared\Plane.h"

using namespace stdext;

// KD-TREE Node Structure
struct TreeNode
{
    OcclusionQuery  query;

    TreeNode * parent;

    TreeNode * front;
    TreeNode * back;

    CPlane partitionPlane;

    bool isLeaf;

    bool visible;

    int lastVisited;

    vector<Actor*> prefabs;

    vector<Actor*> occluders;

    BBox boundingBox;

    void SetDefaults()
    {
        lastVisited = 0;
        visible = false;
        parent = front = back = NULL;
        isLeaf = false;
    }

    ~TreeNode()
    {
        prefabs.clear();
        occluders.clear();
        SAFE_DELETE(front);
        SAFE_DELETE(back);
    }
    TreeNode()
    {
        SetDefaults();
    }
};


enum BoxNode { NODE_FRONT, NODE_BACK, NODE_BOTH};

//-----------------------------------------------------------------------------
/// Hierarchical occlusion culling tree
//-----------------------------------------------------------------------------
class OcclusionCulling : public RenderBase
{
private:
    queue<TreeNode*>    m_queriesQueue;
    queue<Actor*>       m_actorQueries;
    stack<TreeNode*>    m_renderStack;
    bool    m_finishedTraversal;
    int     m_actorID;
    World*  m_World;
    
    vector<Actor*>      m_QueriedActors;

public:
    void RemoveActor(Actor* actor);


    TreeNode treeRoot;
    OcclusionCulling(){ m_World = 0; m_pCubeVB = 0; m_pCubeIB = 0; }

private:
    void TraverseNode(TreeNode * node,Camera * camera, World * world);
    void PullUpVisibilty(TreeNode * node);
    void IssueActorQuery (Actor * actor);
    void RenderBoxes(TreeNode* node, Camera* camera,World * world);
    void RenderLeaf(Camera* camera, World * world,TreeNode * leaf);
    void CheckQueries(Camera* camera,World* world);
    void BuildFromWorld();
    void Render(TreeNode* node,Camera* camera,World * world);
    void RenderFast(TreeNode* node,Camera* camera,World * world);
    
    void DrawCube(BBox& box);
    LPDIRECT3DVERTEXBUFFER9 m_pCubeVB;
    LPDIRECT3DINDEXBUFFER9 m_pCubeIB;
    void    InitBuffers();
    virtual HRESULT OnResetDevice(){ InitBuffers(); return S_OK; }
    /// Kill occlusion queries
    virtual void	OnLostDevice(){ SAFE_RELEASE(m_pCubeVB); SAFE_RELEASE(m_pCubeIB); } 
	//virtual void	OnDestroyDevice(){};

public:
    // Reset the system
    void Reset();

    void RenderBoxes( Camera* camera,World * world);

    // Build the tree for world prefabs
    void BuildFromWorld(World & world);

    void Render(Camera* camera,World * world);

    void PreRender(Camera* camera, World* world);
    void PostRender(Camera* camera, World* world);
};

#endif