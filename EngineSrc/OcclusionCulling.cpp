//=========== (C) Copyright 2004, Artficial Studios. All rights reserved. =======
// Occlusion Culling
// Author: Tim Johnson
//
// Simple occlusion engine using frame coherence
// -Issues queries for visibility against current scene z-buffer
//     -Always issues queries for invisible objects to see if they have become visible
//     -For visible objects, only queries every few seconds
// -If an object goes out of frustum, visibility state (IsOccluded) remains same, so objects
//  don't flicker when they come back into view
//
// 
//
// (Tim) Optimization TODOs:
//  -Remove frustum check in Model() for items that have been handled by occlusion tree
//  -No queries if item is out of viewing range (ChooseLOD==NULL)
//
//===============================================================================
#include "stdafx.h"
#include "OcclusionCulling.h"
#include "BatchRenderer.h"
#include "Profiler.h"
#include "HDR.h"


void OcclusionCulling::BuildFromWorld(World & world)
{

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

Camera* cam;

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void OcclusionCulling::IssueActorQuery (Actor * actor)
{
    // Create query if it is not already created
    if (!actor->m_query.query)
        actor->m_query.Create();

    assert(!actor->m_query.pending);
    
    BBox box;
    if(actor->MyModel)
        box = actor->MyModel->GetWorldBBox();
    else
        box = actor->CollisionBox;
    // If we had a perfect fit, object may occlude itself, so grow box slightly
    box.max += Vector(0.01f,0.01f,0.01f);
    box.min -= Vector(0.01f,0.01f,0.01f);

    // Issue occlusion
    actor->m_query.Begin();
    DrawCube(box);
    actor->m_query.End();

    // push the actor to the issued queries list
    m_QueriedActors.push_back(actor);
    Profiler::Get()->OcclusionQueries++;
}


//-----------------------------------------------------------------------------
// Entrypoint for occlusion culling
//-----------------------------------------------------------------------------
void OcclusionCulling::Render(Camera* camera,World * world)
{
    // Clear _ALL_ states - this is vital
    RenderDevice::Instance()->ResetAllStates();
    RenderDevice::Instance()->UpdateViewport();
    RenderWrap::SetView(camera->view);
    RenderWrap::SetProjection(camera->projection);
    RenderWrap::SetWorld(Matrix());

    // Turn off zwrite and culling
    RenderWrap::SetRS(D3DRS_COLORWRITEENABLE,0);
    RenderWrap::SetRS( D3DRS_ZWRITEENABLE , FALSE);
    RenderWrap::SetRS( D3DRS_CULLMODE , D3DCULL_NONE );
    RenderWrap::SetRS( D3DRS_FILLMODE , D3DFILL_SOLID);
    RenderWrap::SetRS( D3DRS_ALPHABLENDENABLE, FALSE);
    RenderWrap::SetRS( D3DRS_SHADEMODE, D3DSHADE_FLAT);

    RenderWrap::SetRS( D3DRS_LIGHTING, FALSE);
    RenderWrap::dev->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_DISABLE );
    RenderWrap::dev->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_DISABLE );

    RenderWrap::dev->SetPixelShader(0);
    RenderWrap::dev->SetVertexShader(0);
    RenderWrap::dev->SetFVF(D3DFVF_XYZ);
    RenderWrap::dev->SetStreamSource(0, m_pCubeVB, 0, sizeof(LVertex));
    RenderWrap::dev->SetIndices(m_pCubeIB);
    

    cam = camera;
    // Issue Occlusion queries for dynamic visible actors
    for (int i=0;i < world->m_Actors.size(); i++)
    {
        Actor * actor = world->m_Actors[i];

        if ((actor->m_ForceOcclusionTest || actor->MyModel) && !actor->IsHidden && !actor->m_query.pending)
        {
            BBox box;
            if(actor->MyModel)
                box = actor->MyModel->GetWorldBBox();
            else
                box = actor->CollisionBox;

            // If inside object, always assume it's visible
            if(box.IsPointInBox(cam->Location))
                actor->IsOccluded = false;
            // Was visible, so continue assuming it's visible for two seconds
            // +Random amount so frame queries are nicely spread out
            else if(!actor->IsOccluded && float(actor->m_QueryFrame)*GDeltaTime < 1.5f + RANDF())
                actor->m_QueryFrame++;
            // If invisible, or hit update rate, force a query
            // But only if on screen, otherwise will of course return 0
            else
            {
                Vector delta = Vector(0.5f,0.5f,0.5f);
                // If occluded, oversize slightly so we update the occlusion state before it comes
                // into view
                if(actor->IsOccluded)
                {
                    box.max += delta;
                    box.min -= delta;
                }
                // If visible, undersize, in case it's almost out of view
                // Because we don't want this falsely flagged as occluded because it's
                // just out of view
                else
                {
                    box.max -= delta;
                    box.min += delta;
                }
                int ret = camera->BoxInFrustum(box);

                // We won't issue a query if it was visible and now intersecting frustum
                // Because we don't want this falsely flagged as occluded because it's
                // just out of view
                // -Except if already occluded, then check intersection because it may have just
                // come into view
                //if(ret == CULL_INCLUSION || (ret == CULL_INTERSECT && actor->IsOccluded))
                if(ret != CULL_EXCLUSION)
                {
                    IssueActorQuery(actor);
                }
                actor->m_QueryFrame = 0;
            }
        }
    }

    RenderDevice::Instance()->SetDefaultStates();
}

//-----------------------------------------------------------------------------
// Render Nodes bounding Boxes. For visualization/debugging.
//-----------------------------------------------------------------------------
void OcclusionCulling::RenderBoxes( Camera* camera,World * world)
{
}

//-----------------------------------------------------------------------------
// For fast queries, we issue the render AFTER the z buffer has been written to
//-----------------------------------------------------------------------------
void OcclusionCulling::PostRender(Camera *camera, World *world)
{
    Render(camera,world);  
}


//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void OcclusionCulling::RemoveActor(Actor* actor)
{
    actor->m_query.pending = false;

    // Start from back, as actor is more likely to be on back
    for(int i=m_QueriedActors.size()-1;i>=0;i--)
    {
        if(m_QueriedActors[i] == actor)
        {
            m_QueriedActors.erase(m_QueriedActors.begin()+i);
            return;
        }
    }
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void OcclusionCulling::Reset()
{ 
    m_QueriedActors.clear();
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void OcclusionCulling::PreRender(Camera *camera, World *world)
{
    //
    // Check for huge jump in position, if so flush all occlusion states
    //
    static Vector lastLocation;
    if((camera->Location-lastLocation).Length() > 25)
    {
        
        for(int i=0;i<world->m_Actors.size();i++)
        {
            world->m_Actors[i]->IsOccluded = false;
            world->m_Actors[i]->m_query.pending = false;
            world->m_Actors[i]->m_QueryFrame = 99999; // Force fresh query this frame
        }

        m_QueriedActors.clear();
    }
    lastLocation = camera->Location;

     // Get query results
    for(int i=0;i<m_QueriedActors.size();i++)
    {
        Actor* actor = m_QueriedActors[i];
        DWORD pixels;

        // Only remove queries that have succeeded
        if(actor->m_query.query->GetData(&pixels,sizeof(DWORD),0 ) == S_OK)
        {
            pixels = actor->m_query.GetPixels();

            // Invisible
            if (!pixels) 
                actor->IsOccluded = true;
            else
                actor->IsOccluded = false;

            actor->m_query.pending = false;

            // TODO: Move me to a list or some sort to avoid reshuffle
            m_QueriedActors.erase(m_QueriedActors.begin()+i);
            i--;
        }
    }
}