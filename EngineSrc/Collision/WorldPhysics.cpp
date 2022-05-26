//=========== (C) Copyright 2003, Tim Johnson. All rights reserved. ===========
// WorldPhysics
// Actor/World physics interactions
//
//=============================================================================
#include "stdafx.h"
#include "Collision.h"
#include "TokamakPhysics.h"


// Main collision function
// Returns list of touched actors, if any
bool CollideAndSlide(World* world, Actor& collider, float gravity, vector<Actor*>& touchList, CollisionInfo& returnInfo);

/*
==================
PM_Friction

Handles both ground friction and water friction
==================
*/
float	pm_stopspeed = 3;
void PM_Friction(Vector& vel, bool onGround, float pm_friction) {
	float	speed, newspeed, control;
	float	drop;

	Vector vec = vel;

	if ( onGround ) {
		vec.y = 0;	// ignore slope movement
	}

	speed = sqrt(vel.x*vel.x +vel.z*vel.z);
	if (!speed) {
		vel.x = vel.z = 0;
		return;
	}

	drop = 0;

	// apply ground friction
	if(onGround){
		control = speed < pm_stopspeed ? pm_stopspeed : speed;
		drop = control*pm_friction*GDeltaTime;
	}

	// scale the velocity
	newspeed = speed - drop;
	if (newspeed < 0) {
		newspeed = 0;
	}
	newspeed /= speed;

	vel *= newspeed;
}

//-----------------------------------------------------------------------------
// Entry-point for actor physics
//-----------------------------------------------------------------------------
void World::RunPhysics(Actor* actor){
	// No moving for geometry, for speed/logic
	if(actor->IsPrefab || actor->PhysicsFlags & Actor::PHYS_RIGIDBODYDYANMICS || actor->StaticObject)
		return;

	// Bail out if actor doesn't have physics
	if(actor->GhostObject){
		// Just increment location without physics
		actor->Location += actor->Velocity * GDeltaTime;

		// Will only happen if an object changes to ghost/static while touching an actor
		if(actor->touchingActors.size()){
			for(int i=0;i<actor->touchingActors.size();i++)
				actor->touchingActors[i]->UnTouched(actor);
		}
		return;
	}

	// Should get from zone

	// WHY this funky gravity??
	float gravity = 24;//9.8f;
	if(actor->PhysicsFlags & Actor::PHYS_NOT_AFFECTED_BY_GRAVITY)
		gravity = 0;

	// TODO: Enable this once spaceships are fixed
	// We can't collide mesh objects
	//if(actor->CollisionFlags & Actor::CF_MESH){
		//Error("Physics is enabled for a CF_MESH object, %s. Please us CF_BBOX for any objects in the physics system",actor->ClassName());
	//	return;
	//}

	// Add friction
	// Friction is friction of actor + ground plane
	float friction = actor->Friction;

	friction *= 6; // Standard ground friction. TODO: Should be different for each material

	Vector startVelocity = actor->Velocity;

	CollisionInfo returnInfo;
	vector<Actor*> touchList;
	bool hitSomething = CollideAndSlide(*actor,gravity,touchList,returnInfo);

	// If we hit something and it wasn't an actor, and this actor wants to be known
	// about world collisions, call actor->Touched(NULL)
	if(hitSomething && actor->CollisionFlags & Actor::CF_WANT_WORLD_TOUCHED  && touchList.size() == 0)
			actor->Touched(NULL, &returnInfo);

	// NOTE: Two actors touch, won't they each trigger touch once, resulting in two touches?
	// Perhaps that's correct?

	// New touch list
	for(int j=0;j<touchList.size();j++){
		// Stuff to do with touched actors every frame
		// TODO: To make it so you can smoothly push objects, objects need to attempt
		// to move before they are collided against
		//if(touchList[j]->moveable)
		//	touchList[j]->velocity = startVelocity;


		// Trigger Touch() calls for actors that we just bumped into
		vector<Actor*>::iterator location = find(actor->touchingActors.begin(), actor->touchingActors.end(), touchList[j]);
		// Make sure that we've just touched this object
		if(location == actor->touchingActors.end()){
			// This object is not on old touch list, so call Touch()
			// for the first time
			// FIXME: Should check other actor's touched list too?
			// I think only if the other actor doesn't auto-update each frame

			// Let this actor (touchList[j]) know it's referenced by our actor->touchList
			// That way, when it dies, it can delete itself from our list and avoid a
			// dangling pointer!

			touchList[j]->AddRef(actor);

			// Give the other actor a Touched event unless we are not a passable object
			if(! (actor->CollisionFlags & Actor::CF_PASSABLE_BBOX))
				touchList[j]->Touched(actor); // No CollisionInfo for other actor

			// don't receive Touched from StaticObjects unless receiver is CF_WANT_WORLD_TOUCHED
			// if(!touchList[j]->StaticObject || actor->CollisionFlags & Actor::CF_WANT_WORLD_TOUCHED)
				actor->Touched(touchList[j], &returnInfo);
		}
	}

	// Old touch list, untouching..
	for(int j=0;j<actor->touchingActors.size();j++){
		// Look for actors that were touching us (actor) last time, but are no longer touching
		// If we find any, call UnTouched() for both of us
		vector<Actor*>::iterator location = find(touchList.begin(), touchList.end(), actor->touchingActors[j]);
		if(location == touchList.end()){
			// Tell the actor that we don't reference it any more
			actor->touchingActors[j]->DelRef(actor);
			// Untouch event
			actor->UnTouched(actor->touchingActors[j]);

			// Give the other actor an UnTouched event unless we are a passable object
			if(! (actor->CollisionFlags & Actor::CF_PASSABLE_BBOX))
				actor->touchingActors[j]->UnTouched(actor);
		}
	}

	actor->touchingActors = touchList;

	if(actor->GroundNormal != Vector(0,0,0)){
		if(!(actor->CurrentState & Actor::STATE_ONGROUND)){
			actor->CurrentState = Actor::STATE_ONGROUND; // Must set it here, in case Landed uses it, must never set before, or the above if statement will be invalid
			actor->Landed(startVelocity);
		}
		actor->CurrentState = Actor::STATE_ONGROUND;
	}
	// Clear onground state if we've built up any real falling velocity
	else if(abs(actor->Velocity.y) > 1.5) 
		actor->CurrentState = 0;


	PM_Friction (actor->Velocity,(actor->CurrentState == Actor::STATE_ONGROUND),friction);

	return;
}



