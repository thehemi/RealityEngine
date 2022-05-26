//=========== (C) Copyright 2003, Tim Johnson. All rights reserved. ===========
//
//=============================================================================
#include "stdafx.h"
#include "Collision.h"
#include "Frame.h"
#include "ispatialpartition.h"
#include "Editor.h"

//-----------------------------------------------------------------------------
// Wrapper function for collision call with model hierarchy
//-----------------------------------------------------------------------------
void CollideBoxModel(ModelFrame* frame, Actor* source, BBox& box,  vector<CollisionFace>& faces, const Vector& moveDir){
	if(frame->collisionMesh)
		collider.CollideAABB(source,box,frame->collisionMesh,faces,moveDir,frame->CombinedTransformationMatrix);

	if(frame->pFrameFirstChild)
		CollideBoxModel(frame->pFrameFirstChild,source,box,faces,moveDir);
	if(frame->pFrameSibling)
		CollideBoxModel(frame->pFrameSibling,source,box,faces,moveDir);
}

//-----------------------------------------------------------------------------
// Wrapper function for collision call with model hierarchy
//-----------------------------------------------------------------------------
void CollideRayModel(ModelFrame* frame, const Vector& start, const Vector& end, CollisionInfo& result){
	CollisionInfo newResult;
	if(frame->collisionMesh){
		if(collider.CollideRay(start,end,frame->collisionMesh,newResult,frame->CombinedTransformationMatrix)){
			// Use new result only if nearer than old result
			if(newResult.actualDistance < result.actualDistance)
				result = newResult;
		}
	}

	if(frame->pFrameFirstChild)
		CollideRayModel(frame->pFrameFirstChild,start,end,result);
	if(frame->pFrameSibling)
		CollideRayModel(frame->pFrameSibling,start,end,result);
}


/// All dlights that affect a static prop must mark that static prop every frame.
class EnumColliders : public IPartitionEnumerator
{
public:
	vector<Actor*> actors;

	virtual IterationRetval_t EnumElement( IHandleEntity *pHandleEntity )
	{
        if(((Actor*)pHandleEntity)->MyWorld)
		    actors.push_back((Actor*)pHandleEntity);

		return ITERATION_CONTINUE;
	}
};



//-----------------------------------------------------------------------------
// Name: GatherPossibleColliders()
// Desc: Fills 'touchList' with all Faces that intersect the sweep box for the move
//-----------------------------------------------------------------------------
void World::GatherPossibleColliders( Actor* source, vector<CollisionFace>& touchList, const Vector& inMove, const BBox& worldBox){
	// 1. Construct box to cover move in all possible directions (all the ways the move energy could be dissipated)
	Vector move = inMove * 1.01f; // Expand the move to cover epsilon-bordering faces
	BBox sweepBox = worldBox;
	sweepBox+=(sweepBox.min - move );
	sweepBox+=(sweepBox.max - move );
	sweepBox+=(sweepBox.max + move );
	sweepBox+=(sweepBox.min + move );

	// 2. Find possible colliders

	// 2.1 Check the static world tree
	collider.CollideAABB(NULL,sweepBox,m_CollisionMesh,touchList,move);

	// Gather touching actors and prefabs
	EnumColliders enumerator;
	SpatialPartition()->EnumerateElementsInBox( PARTITION_ENGINE_ACTORS | PARTITION_ENGINE_PREFABS,
		sweepBox.min, sweepBox.max, false, &enumerator );

	// 2.2 Check all actors
	for(int i=0;i<enumerator.actors.size();i++){
		Actor* actor = enumerator.actors[i];
		if(actor->MyWorld == NULL)
			continue; // Actor may be in tree but no longer in world (i.e. deleted)

		// Tokamak objects mustn't touch other tokamak objects
		// We're assuming source = NULL makes it a tokamak object
		if((!source || source->PhysicsFlags & Actor::PHYS_RIGIDBODYDYANMICS) && actor->PhysicsFlags & Actor::PHYS_RIGIDBODYDYANMICS)
			continue; 
		
		// Each actor may contain optional user flags to filter out collisions
		// Check these now
		if(source){
			if(source->IgnoreUserCollisionFlags != 0 && source->IgnoreUserCollisionFlags & actor->UserCollisionFlag)  
				continue;
			// It's bi-directional, so we make sure neither object rejects the other one
			if(actor->IgnoreUserCollisionFlags != 0 && actor->IgnoreUserCollisionFlags & source->UserCollisionFlag)  
				continue;

			if(actor == source || (source != null && actor->Parent == source) || actor->GhostObject || actor->ignoreActor == source || source->ignoreActor == actor )
				continue;
		}

		// Don't collide with passable boxes if tokamak.
		// if non-tokamak, it will gather them for triggers, and reject them later
		if((!source || source->PhysicsFlags & Actor::PHYS_RIGIDBODYDYANMICS) && (actor->CollisionFlags & Actor::CF_PASSABLE_BBOX || actor->CollisionFlags & Actor::CF_CORPSE || actor->GhostObject))
			continue;

		Model* model = actor->GetCollisionModel();

		if(actor->CollisionFlags & Actor::CF_MESH && model && model->m_pFrameRoot){
			// This actor wants accurate collision, so do a bbox/model check
			CollideBoxModel(model->m_pFrameRoot,actor,sweepBox,touchList,move);
		}
		else if((!(actor->CollisionFlags & Actor::CF_MESH) && !(source && source->CollisionFlags & Actor::CF_MESH))){
			// Do a coarse bbox/bbox check against these two actors
			CheckBBox(sweepBox,actor,touchList);
		}
	}
}

//-----------------------------------------------------------------------------
// Wrapper function for collision call with model hierarchy
//-----------------------------------------------------------------------------
void Gather_Model(ModelFrame* frame, BBox& box, vector<Vertex>& vertices, const Vector& moveDir)
{
	if(frame->collisionMesh)
        collider.GatherRenderingPolys(box,frame->collisionMesh,vertices,moveDir,frame->CombinedTransformationMatrix);

	if(frame->pFrameFirstChild)
		Gather_Model(frame->pFrameFirstChild,box,vertices,moveDir);
	if(frame->pFrameSibling)
		Gather_Model(frame->pFrameSibling,box,vertices,moveDir);
}

//-----------------------------------------------------------------------------
// Name: GatherRenderingPolys()
// Desc: Fills 'touchList' with all Faces that intersect the sweep box for the move
//-----------------------------------------------------------------------------
void World::GatherRenderingPolys( Actor* sourceToIgnore, vector<Vertex>& vertices, const Vector& inMove, const BBox& worldBox){
	// 2.1 Check the static world tree
	//collider.CollideAABB(NULL,worldBox,m_CollisionMesh,vertices,move);
    BBox box = worldBox;

	// Gather touching actors and prefabs
	EnumColliders enumerator;
	SpatialPartition()->EnumerateElementsInBox( PARTITION_ENGINE_ACTORS | PARTITION_ENGINE_PREFABS,
		box.min, box.max, false, &enumerator );

	// 2.2 Check all actors
	for(int i=0;i<enumerator.actors.size();i++)
    {
		Actor* actor = enumerator.actors[i];
        if(!actor->IsPrefab || actor == sourceToIgnore)
            continue;

		if(actor->MyWorld == NULL || actor->CollisionFlags & Actor::CF_PASSABLE_BBOX || actor->CollisionFlags & Actor::CF_CORPSE || actor->GhostObject)
			continue;

		Model* model = actor->GetCollisionModel();

		if(model && model->m_pFrameRoot)
        {
			Gather_Model(model->m_pFrameRoot,box,vertices,inMove);
		}
	}
}


//-----------------------------------------------------------------------------
// Main function for checking rays against the world
// We check each object in the world, and the one which is nearest is returned
// Nearest object is chosen by finding the lowest actualDistance
// If actualDistance > the length of the ray, it's too far
//-----------------------------------------------------------------------------
bool World::CollisionCheckRay(Actor* source, const Vector& start, const Vector& end, CheckType check, CollisionInfo& result, bool forceModelAccuracy, vector<Actor*>* ignoreActors){
	if(source && source->GhostObject) return false;

	result.actualDistance = (end-start).Length();

	bool collision = false;
	CollisionInfo newInfo;
	newInfo.actualDistance = result.actualDistance; // So the functions know the max distance of the ray

	Ray_t myRay;
	myRay.Init(start,end);
	EnumColliders enumerator;

	if(check == CHECK_ACTORS || check == CHECK_VISIBLE_ACTORS)
		SpatialPartition()->EnumerateElementsAlongRay ( PARTITION_ENGINE_ACTORS ,myRay, false, &enumerator );
	else if(check == CHECK_GEOMETRY)
		SpatialPartition()->EnumerateElementsAlongRay (PARTITION_ENGINE_PREFABS ,myRay, false, &enumerator );
	else
        SpatialPartition()->EnumerateElementsAlongRay ( PARTITION_ENGINE_ACTORS | PARTITION_ENGINE_PREFABS,myRay, false, &enumerator );

    for(int i=0;i<enumerator.actors.size();i++)
    {
        Actor* actor = enumerator.actors[i];

		switch(check)
		{
		case CHECK_VISIBLE_ACTORS:
			if(actor->IsPrefab || actor->IsHidden)
				continue;
			break;
		case CHECK_ACTORS:
			if(actor->IsPrefab)
				continue;
			break;
		case CHECK_GEOMETRY:
			if(!actor->IsPrefab)
				continue;
			break;
		case CHECK_VISIBLE_GEOMETRY:
			if(!actor->IsPrefab || actor->IsHidden)
				continue;
			break;
		case CHECK_VISIBLE_EVERYTHING:
		case CHECK_UNHIDDEN_EVERYTHING:
			if(actor->IsHidden)
				continue;
			break;
		case CHECK_VISIBLE_ACTORS_UNFROZEN:
			if(actor->IsFrozen || actor->IsHidden)
				continue;
			break;
		}

		if( actor == source || (actor->GhostObject && !Editor::Instance()->GetEditorMode())) /* || actor->ignoreActor == source || (source && source->ignoreActor == actor)*/
            continue;

		if((actor->CollisionFlags & Actor::CF_PASSABLE_BBOX || !actor->m_CollidesRays) && check != CHECK_VISIBLE_ACTORS && check != CHECK_VISIBLE_EVERYTHING && check != CHECK_VISIBLE_ACTORS_UNFROZEN)
			continue;

        // Each actor may contain optional user flags to filter out collisions
        // Check these now
        if(source && source->IgnoreUserCollisionFlags != 0 && source->IgnoreUserCollisionFlags & actor->UserCollisionFlag)  
            continue;

		if(ignoreActors)
		{
		bool found = false;
		// check against our ignore-Actors list, only collide if not on the list
			for(int i = 0; i < (*ignoreActors).size(); i++)
			{
				if((*ignoreActors)[i] == actor)
				{
					found = true;
					break;
				}
			}
		if(found)
			continue;
		}

        Model& model = *actor->MyModel;     

        if(actor->MyModel && actor->MyModel->m_pFrameRoot &&   ((actor->CollisionFlags & Actor::CF_MESH)||forceModelAccuracy))
		{
            Vector rayStart = start;
            Vector rayEnd = end;

            // This actor wants accurate collision, so do a ray/model check
            CollideRayModel(model.m_pFrameRoot,rayStart,rayEnd,newInfo);
            if(newInfo.actualDistance < result.actualDistance)
			{
					result = newInfo;
					result.touched = actor;
					collision = true;
            }
        }
        else{
            // Coarse ray/bbox check
            if(RayCheckBBox(start,end,actor,newInfo) && newInfo.actualDistance < result.actualDistance)
			{
					result = newInfo;
					result.touched = actor;
					collision = true;
            }
        }

    }

	// We're done, no need to verify if actor is behind anything, since there aren't any
	if(!collision)
		return false; 

	// if we only want to check for collisions with non-Prefab Actors, and it collides with a Prefab, then let's pretend there's been no collision with anything
	if((check == CHECK_ACTORS || check == CHECK_VISIBLE_ACTORS) && CollisionCheckRay(source,start,end,CHECK_GEOMETRY,newInfo) && newInfo.actualDistance < result.actualDistance)
		collision = false;

	// 2 Check the static world tree
	//if(collider.CollideRay(start,end,m_CollisionMesh,newInfo) && newInfo.actualDistance < result.actualDistance){
	//	result = newInfo;
	//	result.touched = NULL;
	//	collision = true;
	//}

	return collision;
}

// Actors and helper classes can use this to check various small things
/*bool World::CollisionCheckActor(Actor* actor, const Vector& testLocation, const Vector& testVelocity, CollisionInfo& result){
	// Check box is valid
	if(actor->CollisionBox == BBox(Vector(0,0,0),Vector(0,0,0))){
		Error("One of the Actors has physics turned on,\n\
			but either doesn't have a model loaded, or hasn't set a CollisionBox.\n\
			All actors that use the physics system must do one or the other. Actor pointer is %x",actor);
	}

	BBox box = actor->CollisionBox;
	box.min += actor->Location;
	box.max += actor->Location;

	vector<CollisionFace> possibleobjects;

	GatherPossibleColliders(this,actor,possibleobjects,testLocation,testVelocity*GDeltaTime,box);
	result = AABBSweep(actor, possibleobjects,testLocation,actor->Velocity.Normalized(),box);
	// Collision if nearest object distance is below move distance
	return ( result.actualDistance < actor->Velocity.Length()*GDeltaTime );
}*/

// FIXME: Testlocation is unused
bool World::CollisionCheckActor(Actor* actor, const Vector& testLocation, const Vector& testVelocity){
	// Check box is valid
	if(actor->CollisionBox == BBox(Vector(0,0,0),Vector(0,0,0))){
		Error(_U("One of the Actors has physics turned on,\n\
			but either doesn't have a model loaded, or hasn't set a CollisionBox.\n\
			All actors that use the physics system must do one or the other. Actor pointer is %x"),actor);
	}

	BBox box = actor->CollisionBox;
	box.min += actor->Location;
	box.max += actor->Location;

	vector<CollisionFace> possibleobjects;

	GatherPossibleColliders(actor,possibleobjects,testVelocity*GDeltaTime,box);

	box = actor->CollisionBox;
	box.min += actor->Location + testVelocity*GDeltaTime;
	box.max += actor->Location + testVelocity*GDeltaTime;
	CollisionInfo trace;
	return IsStuck(actor,possibleobjects,box,trace,true);
}





