//=========== (C) Copyright 2004, Artficial Studios. All rights reserved. =======
// Occlusion Culling
// Author: Mostafa Mohamed
//
// Description: this occlusion culling system uses  Coherent Hierarchical Culling
// algorithm, it uses a mix between AABB/KD Trees 
//
// Hints: 
// 1 - The system creates a tree using a collection of prefabs and occluders
// which may be any prefab, it uses this prefabs to create main spliting planes
// then continue the tree generation using prefabs bboxes medians
// 2- while rendering: 
//      a- The system renders visible occluders
//      b- Issue occlusion queries for dynamic visible actors
//      c- traverse the tree using the cohrenet hierarchical culling algorithm
//      d- mark the visible prefabs from results of queries while traversing the tree
//      e- mark the visible dynamic actors from result of actor queries
//      f- draw both actors and prefabs marked as visible
//
// (Tim) Optimization TODOs:
//  -Render visible nodes while waiting for results of queries. May hurt batching more than stalling?
//  -Frame coherence? Can we get away with delaying queries an entire frame under certain conditions?
//  -In open scenes, Query Cost > Saving Cost. Issue queries for leaf nodes only every X seconds.
//		-Could base on degree of fluctuation between frames
//      -Can be brutal, querying every few seconds or more
//
//  -Remove frustum check in Model() for items that have been handled by occlusion tree
//  -No queries if item is out of viewing range (ChooseLOD==NULL)
//  -Cube() function should use VB/IB
//
// a paper about Coherent Hierarchical culling can be found at:
// http://www.cg.tuwien.ac.at/research/vr/chcull/bittner-eg04-chcull.pdf
//===============================================================================
#include "stdafx.h"
#include "OcclusionCulling.h"
#include "BatchRenderer.h"
#include "Profiler.h"
#include "HDR.h"

bool m_bFastOcclusion = false;

#define FRAMES_BETWEEN_ISSUES 15
#define NUM_FRAMES 10

// Float Comparer for sorting
int CompareFloats (const void * a, const void * b)
{
    return ( *(float*)a - *(float*)b );
}

// Calculate the median for prefabs bounding boxes median
float CalculateMedian(vector<Actor*> & prefabs,int dimension)
{
    float * values = new float[prefabs.size()];
    int size=0;

    for (int i=0; i < prefabs.size() ;i++)  {
        if (prefabs[i]->MyModel)
        {
            values[size] = (((float*)&(prefabs[i]->MyModel->GetWorldBBox().min))[dimension] + 
                ((float*)&(prefabs[i]->MyModel->GetWorldBBox().max))[dimension])/2 ;
            size++;
        }
    }

    qsort(values, size , sizeof(float) , CompareFloats);
    float median = values[(size + 1) /2];
    delete[] values;
    return median;
}

// Calculate a Bounding Box from a vector of actors
BBox CalculateBBox(vector<Actor*> actors)
{
    BBox newBox;

    for (int i=0;i < actors.size(); i++)
    {
        // Calculate the root node bounding box
        if ( actors[i]->MyModel)
        {
            BBox bbox = actors[i]->MyModel->GetWorldBBox();
            newBox += bbox;
        }
    }
    return newBox;

}

// Test a box with a plane
BoxNode ClassifyBox(CPlane & plane, BBox & box)
{
    float dist = plane.GetDistance((box.max + box.min) /2);

    if (dist >= 0 )
        return NODE_FRONT;
    else
        return NODE_BACK;
}

// Create a spliting plane for node, first use occluders and if there is 
// no more occluders use the median of the prefabs bounding boxes
void CreatePlaneForNode(TreeNode * node)
{
    // Find the largest occluder and create a splitting plane using its largest
    // side
    if (node->occluders.size())
    {
        int occluder = 0;
        float currentSize=SMALL_NUMBER;
        int currentAxis = 0;
        BBox currentBox ; 

        for (int i=0; i< node->occluders.size(); i++)
        {
            if (node->occluders[i]->MyModel)
            {
                BBox bbox = node->occluders[i]->MyModel->GetWorldBBox();

                float width= bbox.max.x - bbox.min.x;
                float height= bbox.max.y - bbox.min.y;
                float depth= bbox.max.z - bbox.min.z;

                float size = max(max(width,height),depth);

                int axis=2;
                if (width <= height && width <= depth)
                    axis=0;
                else if (height <= width && height <= depth)
                    axis=1;

                // If this occluder size is larger than the current one
                if (size > currentSize)
                {
                    // Set it as the current spliter
                    currentBox = bbox;
                    currentAxis = axis;
                    currentSize = size;
                    occluder = i;
                }
            }
        }

        // Remove the selected occluder
        node->occluders.erase(node->occluders.begin() + occluder);
        // Create plane from the occluder
        ((float*)&(node->partitionPlane.normal))[currentAxis] = 1;
        node->partitionPlane.distance= (((float*)&(currentBox.max))[currentAxis] + ((float*)&(currentBox.min))[currentAxis])/2;
        return;
    }

    // If no occluders left use bounding boxes median
    float width= node->boundingBox.max.x - node->boundingBox.min.x;
    float height= node->boundingBox.max.y - node->boundingBox.min.y;
    float depth= node->boundingBox.max.z - node->boundingBox.min.z;

    int axis = 2;
    if (width >= height && width >= depth)
        axis=0;
    else if (height >= width && height >= depth)
        axis=1;

    // Calculate Median
    float median = 0;
    median = CalculateMedian(node->prefabs, axis);

    // Create Plane
    ((float*)&(node->partitionPlane.normal))[axis] = 1;
    node->partitionPlane.distance = median;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void OcclusionCulling::Reset()
{ 
    SAFE_DELETE(treeRoot.front);
    SAFE_DELETE(treeRoot.back);
    treeRoot.prefabs.clear();
    treeRoot.SetDefaults();
    while(!m_queriesQueue.empty()) { m_queriesQueue.pop(); }
    while(!m_actorQueries.empty()) { m_actorQueries.pop(); }
}

void OcclusionCulling::BuildFromWorld()
{
    int nodesCount = 0;
    stack<TreeNode*> nodesStack;

    // Push the root to the stack
    nodesStack.push(&treeRoot);

    while (!nodesStack.empty())
    {
        TreeNode* node = nodesStack.top();
        nodesStack.pop();
        nodesCount++;

        // Calculate Node BoundingBox
        node->boundingBox = CalculateBBox(node->prefabs);

        // Create the spliting plane
        CreatePlaneForNode(node);

        // Create front and back node
        node->front = new TreeNode();
        node->back = new TreeNode();
        node->front->parent = node->back->parent = node;

        // Add Prefabs to thier right nodes
        for (int i=0; i < node->prefabs.size(); i++)
        {
            if (node->prefabs[i]->MyModel)
            {
                if (ClassifyBox(node->partitionPlane, node->prefabs[i]->MyModel->GetWorldBBox()) == NODE_FRONT)
                    node->front->prefabs.push_back(node->prefabs[i]);
                else
                    node->back->prefabs.push_back(node->prefabs[i]);
            }
        }

        // Add occluders to thier right nodes
        for (int i=0; i < node->occluders.size(); i++)
        {
            if (node->occluders[i]->MyModel)
            {
                if (ClassifyBox(node->partitionPlane, node->occluders[i]->MyModel->GetWorldBBox()) == NODE_FRONT)
                    node->front->occluders.push_back(node->occluders[i]);
                else
                    node->back->occluders.push_back(node->occluders[i]);
            }
        } 

        // Check if the node is not splitabl
        if ((!node->back->prefabs.size() || !node->front->prefabs.size()) &&  !node->occluders.size())
        {
            node->query.Create();
            node->isLeaf = true;
            node->occluders.clear();

            SAFE_DELETE(node->back);
            SAFE_DELETE(node->front);
            continue;
        }

        // Clear node occluders
        node->occluders.clear();

        if (node->front->prefabs.size() > 1)
            nodesStack.push(node->front);
        else 
        {
            node->front->boundingBox = CalculateBBox(node->prefabs);
            node->front->isLeaf = true;
            node->front->query.Create();
        }

        if (node->back->prefabs.size() > 1)
            nodesStack.push(node->back);
        else 
        {
            node->back->boundingBox = CalculateBBox(node->prefabs);
            node->back->isLeaf = true;
            node->back->query.Create();
        }


        // Clear the prefabs for not leaf nodes
        node->prefabs.clear();

        // Create an occlusion query for the node
        node->query.Create();
    }
}

//-----------------------------------------------------------------------------
// Build tree for occlusion from the world prefabs
//-----------------------------------------------------------------------------
void OcclusionCulling::BuildFromWorld(World & world)
{
    m_World = &world;
    this->Clear();

    for (int i=0;i < world.m_Actors.size(); i++)
    {
        if (world.m_Actors[i]->MyModel)
        {
            if(m_bFastOcclusion)
            {
                if(world.m_Actors[i]->IsPrefab)
                    this->treeRoot.prefabs.push_back((Actor*)world.m_Actors[i]);
            }
            else{
                if (world.m_Actors[i]->IsPrefab && !world.m_Actors[i]->IsOccluder)
                    this->treeRoot.prefabs.push_back((Actor*)world.m_Actors[i]);
                else if (world.m_Actors[i]->IsPrefab && world.m_Actors[i]->IsOccluder)
                    this->treeRoot.occluders.push_back((Actor*)world.m_Actors[i]);
            }
        }
    }

    this->BuildFromWorld();
}


//-----------------------------------------------------------------------------
// Occlusion culling 
//-----------------------------------------------------------------------------
void OcclusionCulling::InitBuffers()
{
    if(m_pCubeIB)
        return;

    // create the vertexbuffer
    DXASSERT( RenderWrap::dev->CreateVertexBuffer( 8*sizeof(LVertex),
        D3DUSAGE_WRITEONLY, FVF_LVERTEX,
        D3DPOOL_DEFAULT, &m_pCubeVB, NULL ) );	

    LVertex* pVertices;
    DXASSERT(m_pCubeVB->Lock(0,0,(void**)&pVertices,0));
    BBox cube(Vector(-0.5f,-0.5f,-0.5f),Vector(0.5f,0.5f,0.5f));
    pVertices[0].position = cube.min;
    pVertices[1].position = Vector( cube.min.x, cube.max.y, cube.min.z );
    pVertices[2].position = Vector( cube.max.x, cube.max.y, cube.min.z );
    pVertices[3].position = Vector( cube.max.x, cube.min.y, cube.min.z );
    pVertices[4].position = Vector( cube.min.x, cube.min.y, cube.max.z );
    pVertices[5].position = Vector( cube.min.x, cube.max.y, cube.max.z );
    pVertices[6].position = cube.max;
    pVertices[7].position = Vector( cube.max.x, cube.min.y, cube.max.z );
    DXASSERT(m_pCubeVB->Unlock());


    // create/fill the indexbuffer
    DXASSERT( RenderWrap::dev->CreateIndexBuffer(	36*sizeof(WORD),
        D3DUSAGE_WRITEONLY,			
        D3DFMT_INDEX16,	D3DPOOL_DEFAULT,&m_pCubeIB,NULL));

    WORD *indices;
    DXASSERT(m_pCubeIB->Lock(0,0,(void**)&indices,0 ) );
    // front side
    indices[0]  = 0; indices[1]  = 1; indices[2]  = 2;
    indices[3]  = 0; indices[4]  = 2; indices[5]  = 3;

    // back side
    indices[6]  = 4; indices[7]  = 6; indices[8]  = 5;
    indices[9]  = 4; indices[10] = 7; indices[11] = 6;

    // left side
    indices[12] = 4; indices[13] = 5; indices[14] = 1;
    indices[15] = 4; indices[16] = 1; indices[17] = 0;

    // right side
    indices[18] = 3; indices[19] = 2; indices[20] = 6;
    indices[21] = 3; indices[22] = 6; indices[23] = 7;

    // top
    indices[24] = 1; indices[25] = 5; indices[26] = 6;
    indices[27] = 1; indices[28] = 6; indices[29] = 2;

    // bottom
    indices[30] = 4; indices[31] = 0; indices[32] = 3;
    indices[33] = 4; indices[34] = 3; indices[35] = 7;
    m_pCubeIB->Unlock();
}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void OcclusionCulling::DrawCube(BBox& box)
{
    Matrix mat;
    mat.SetScales(box.max.x-box.min.x,box.max.y-box.min.y,box.max.z-box.min.z);
    mat.m3 = (box.min+box.max)/2;
    RenderWrap::dev->SetTransform(D3DTS_WORLD,(D3DXMATRIX*)&mat);
    DXASSERT(RenderWrap::dev->DrawIndexedPrimitive( D3DPT_TRIANGLELIST,0,0, 8,0, 36/3 ));
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void OcclusionCulling::IssueActorQuery (Actor * actor)
{
    // Create query if it is not already created
    if (!actor->m_query.query)
        actor->m_query.Create();

    if(actor->m_query.pending)
    {
        actor->m_query.GetPixels();
        return; // Why does this happen??
    }

    Profiler::Get()->OcclusionQueries++;
    BBox box = actor->MyModel->GetWorldBBox();
    // Bias box so it doesn't fight its own prefab
    box.max += Vector(0.1f,0.1f,0.1f);
    box.min -= Vector(0.1f,0.1f,0.1f);
    // Issue occlusion
    actor->m_query.Begin();
    DrawCube(box);
    actor->m_query.End();

    // push the actor to the issued queries list
    m_actorQueries.push(actor);
}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void OcclusionCulling::PullUpVisibilty(TreeNode * node)
{
    // Pull up visibilty
    TreeNode* pullVis = node;
    while (!pullVis->visible)            
    {
        pullVis->visible = true; 
        pullVis = pullVis->parent; 
        if (!pullVis)
            break;
    }
}

//-----------------------------------------------------------------------------
// Mark leaf prefabs as visible
//-----------------------------------------------------------------------------
void OcclusionCulling::RenderLeaf(Camera* camera, World * world,TreeNode * leaf)
{
    for (int i =0;i<leaf->prefabs.size();i++)
    {
        Actor* actor = leaf->prefabs[i];

        //Distrbute newly visible actors over NUM_FRAMES frames
        m_actorID++;
        if (m_actorID > NUM_FRAMES) m_actorID = 0;


        if (actor->MyModel && camera->BoxInFrustum(actor->MyModel->GetWorldBBox()))
        {
            //if the actor was visible the last frame
            if (actor->m_QueryFrame <= world->m_FrameID )
            {
                IssueActorQuery(actor);
                actor->m_QueryFrame = world->m_FrameID + FRAMES_BETWEEN_ISSUES * m_actorID;
            }
            else if (actor->m_FrameID < world->m_FrameID  - 1)
            {
                IssueActorQuery(actor);
                actor->m_QueryFrame = 0;
            }
            else
                actor->m_FrameID = world->m_FrameID+1;
        }
    }
}

//-----------------------------------------------------------------------------
// Check available query results
//
//-----------------------------------------------------------------------------
void OcclusionCulling::CheckQueries(Camera* camera,World* world)
{
    DWORD result = 5;
    while (  !m_queriesQueue.empty() &&
        ( m_queriesQueue.front()->query.query->GetData(&result,sizeof(DWORD),D3DGETDATA_FLUSH) != S_FALSE || m_finishedTraversal)) 
    {
        TreeNode * N = m_queriesQueue.front();
        m_queriesQueue.pop();

        result = N->query.GetPixels();

        if (result > 5)
        {
            // Pull up visibilty
            PullUpVisibilty(N);

            // Render or traverse visible nodes
            TraverseNode(N,camera,world);
        }
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void OcclusionCulling::TraverseNode(TreeNode * node,Camera * camera, World * world)
{
    // Render or traverse visible nodes
    if (node->isLeaf )
        RenderLeaf(camera,world,node);
    else
    {
        if (node->front) Render(node->front,camera,world);
        if (node->back)  Render(node->back,camera,world);
    }
}


//-----------------------------------------------------------------------------
// Traverse a tree node
//
//
//-----------------------------------------------------------------------------
void OcclusionCulling::Render(TreeNode* node,Camera* camera,World * world)
{
    // if(m_bFastOcclusion)
    CheckQueries(camera,world);

    // If inside node, it is visible
    if (node->boundingBox.IsPointInBox(camera->Location))
    {
        // Pull up visibilty
        PullUpVisibilty(node);

        // Render or traverse visible nodes
        TraverseNode(node,camera,world);
    }
    // Otherwise, if in view frustum, test visibility
    else if (camera->BoxInFrustum(node->boundingBox))
    {
        // Identify previously visible nodes
        bool wasVisible = node->visible && (node->lastVisited == world->m_FrameID - 1);
        // Identify previously opened nodes
        bool opened = wasVisible && !node->isLeaf;
        // Reset node's visibility classification
        node->visible = false;
        // Update node's visited flag
        node->lastVisited = world->m_FrameID;
        // Skip testing for all previously opened nodes
        if (!opened)
        {
            node->query.Begin();
            Profiler::Get()->OcclusionQueries++;
            DrawCube(node->boundingBox);
            node->query.End();

            m_queriesQueue.push(node);
        }

        // Traverse node until invisible
        if (wasVisible)
            TraverseNode(node,camera,world);
    }
}

void OcclusionCulling::RenderFast(TreeNode* node,Camera* camera,World * world)
{

    // If inside node, it is visible
    if (node->boundingBox.IsPointInBox(camera->Location))
    {
        // Render or traverse visible nodes
        TraverseNode(node,camera,world);
    }
    // Otherwise, if in view frustum, test visibility
    else if (camera->BoxInFrustum(node->boundingBox))
    {


        node->query.Begin();
        Profiler::Get()->OcclusionQueries++;
        DrawCube(node->boundingBox);
        node->query.End();

        m_queriesQueue.push(node);

        TraverseNode(node,camera,world);
    }
}

//-----------------------------------------------------------------------------
// Entrypoint for occlusion culling
//-----------------------------------------------------------------------------
void OcclusionCulling::Render(Camera* camera,World * world)
{
    // Can't issue occlusion queries in HDR with COLORWRITE flags, so switch to LDR
    if(!m_bFastOcclusion)
        HDRSystem::Instance()->SetLDR(true);

    if(!m_bFastOcclusion)
        RenderWrap::dev->Clear(0L, NULL, D3DCLEAR_ZBUFFER,0, 1.0f, 0L );

    m_finishedTraversal = false;
    m_actorID = 0;
    if (this->treeRoot.front || this->treeRoot.back || this->treeRoot.isLeaf)
    {
        // Set Default States
        RenderDevice::Instance()->SetDefaultStates();
        RenderWrap::dev->SetPixelShader(0);
        RenderWrap::dev->SetVertexShader(0);
        RenderWrap::SetRS(D3DRS_COLORWRITEENABLE,0);
        RenderWrap::SetView(camera->view);
        RenderWrap::SetProjection(camera->projection);
        RenderWrap::dev->SetFVF(D3DFVF_XYZ);

        if(!m_bFastOcclusion)
        {
            // Render Visible Occluders
            for (int i=0;i < world->m_Actors.size(); i++)
            {
                if (world->m_Actors[i]->MyModel && world->m_Actors[i]->IsOccluder)
                {
                    if (camera->BoxInFrustum(world->m_Actors[i]->MyModel->GetWorldBBox()))
                    {
                        world->m_Actors[i]->m_FrameID = world->m_FrameID;
                        world->m_Actors[i]->MyModel->DrawSimple(camera);
                    }
                }
            }
        }

        // Set Identity World
        RenderWrap::SetWorld(Matrix());

        // Turn off zwrite and culling
        RenderWrap::SetRS( D3DRS_ZWRITEENABLE , FALSE);
        RenderWrap::SetRS( D3DRS_CULLMODE , D3DCULL_NONE );
        RenderWrap::SetRS( D3DRS_FILLMODE , D3DFILL_SOLID);

        // Set VB/IB just once here, which will be used for all the cubes we draw
        RenderWrap::dev->SetFVF(FVF_LVERTEX);
        RenderWrap::dev->SetStreamSource(0, m_pCubeVB, 0, sizeof(LVertex));
        RenderWrap::dev->SetIndices(m_pCubeIB);
        RenderWrap::dev->SetPixelShader(0);
        RenderWrap::dev->SetVertexShader(0);

        // Issue Occlusion queries for dynamic visible actors
        for (int i=0;i < world->m_Actors.size(); i++)
        {
            Actor * actor = world->m_Actors[i];
            if (actor->MyModel && !actor->IsPrefab)
            {
                if(!m_bFastOcclusion && actor->IsOccluder)
                    continue;

                // If actor visible
                if (camera->BoxInFrustum(actor->MyModel->GetWorldBBox()))
                {
                    // Create query if it is not already created
                    if (!actor->m_query.query)
                        actor->m_query.Create();

                    if(actor->m_query.pending) // Why does this happen??
                    {
                        actor->m_query.GetPixels();
                    }

                    // Issue occlusion
                    Profiler::Get()->OcclusionQueries++;
                    actor->m_query.Begin();
                    DrawCube(actor->MyModel->GetWorldBBox());
                    actor->m_query.End();

                    // push the actor to the issued queries list
                    m_actorQueries.push(actor);
                }
            }
        }

        // Traverse the prefabs tree
        Render(&this->treeRoot, camera,world);
        m_finishedTraversal = true;

        CheckQueries(camera,world);
        // Check remaining queries
        // Mark visible actors from occlusion
        while (!m_actorQueries.empty())
        {
            DWORD result=m_actorQueries.front()->m_query.GetPixels();
            if (result>5)
                m_actorQueries.front()->m_FrameID = world->m_FrameID + 1;
            m_actorQueries.pop();
        }

        // Restore render states
        RenderWrap::SetRS(D3DRS_COLORWRITEENABLE,0xFFFFFFFF);
        RenderWrap::SetRS( D3DRS_CULLMODE , D3DCULL_CCW );
        RenderWrap::SetRS( D3DRS_ZWRITEENABLE , TRUE);

        if(!m_bFastOcclusion)
        {
            // Clear the zbuffer for avoiding the fighting
            RenderWrap::dev->Clear(0L, NULL, D3DCLEAR_ZBUFFER,0, 1.0f, 0L );
        }
    }

    RenderDevice::Instance()->SetDefaultStates();

    if(!m_bFastOcclusion)
        HDRSystem::Instance()->SetLDR(false);
}

//-----------------------------------------------------------------------------
// Render Nodes bounding Boxes. For visualization/debugging.
//-----------------------------------------------------------------------------
void OcclusionCulling::RenderBoxes(TreeNode* node, Camera* camera,World * world)
{
    RenderDevice::Instance()->GetCanvas()->Cube(node->boundingBox);
    if (node->front) RenderBoxes(node->front,camera,world);
    if (node->back)  RenderBoxes(node->back,camera,world);
}

//-----------------------------------------------------------------------------
// Render Nodes bounding Boxes. For visualization/debugging.
//-----------------------------------------------------------------------------
void OcclusionCulling::RenderBoxes( Camera* camera,World * world)
{
    // Set states for rendering boxes
    RenderDevice::Instance()->SetDefaultStates();
    RenderWrap::dev->SetPixelShader(0);
    RenderWrap::dev->SetVertexShader(0);
    RenderWrap::SetRS( D3DRS_CULLMODE , D3DCULL_NONE);
    RenderWrap::SetRS( D3DRS_FILLMODE , D3DFILL_WIREFRAME);
    RenderWrap::SetWorld(Matrix());

    // Render boxes
    RenderBoxes(&treeRoot,camera,world);

    // Restore states
    RenderWrap::SetRS( D3DRS_FILLMODE , D3DFILL_SOLID);
    RenderWrap::SetRS( D3DRS_CULLMODE , D3DCULL_CCW);
}

//-----------------------------------------------------------------------------
// For fast queries, we issue the render AFTER the z buffer has been written to
//-----------------------------------------------------------------------------
void OcclusionCulling::PostRender(Camera *camera, World *world)
{
    if(m_bFastOcclusion)
        Render(camera,world);
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void OcclusionCulling::PreRender(Camera *camera, World *world)
{
    // For standard queries we issue them all now, before rendering
    if(!m_bFastOcclusion)
    {
        Render(camera,world);
        return;
    }
    /*
    // For frame-latency occlusion queries only - get the results prior to rendering
    // from the last frame
    CheckQueries(camera,world);

    // Mark visible actors from occlusion
    while (!m_actorQueries.empty())
    {
        DWORD result=m_actorQueries.front()->m_query.GetPixels();
        if (result>5) // Leaf visible, so render this frame
            m_actorQueries.front()->m_FrameID = world->m_FrameID + 1;
        m_actorQueries.pop();
    }*/
}