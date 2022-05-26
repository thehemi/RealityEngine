//=============================================================================
// CollideAndSLide:
// Axis-aligned bounding box collision handling
// 23/12/2001: TimJ Creates
// KNOWN ISSUES:
//
// **The classic 'stair' problem. This includes simple terrain surfaces or anything where
// the next triangle you might bump into is higher, even if connected to the old one.
//
//
// **AT FRAMERATES > 60FPS EPSILON ERRORS CAUSE RESULTS TO BE INCONSISTENT, SO
// YOU MAY GET STUCK RUBBING AGAINST A CONCAVE CORNER
//
// **When meeting the edge of a box or similar, the function cannot properly decide
// which plane you've hit first, so you end up sliding along the wrong one
// To test: Make a thin box slanted / and try to push up against the edge, and you'll
// slide up it!
//
// What I need: A solid AABB/Triangle intersection test that returns the time
// or
// Another swept bounding volume. Perhaps two spheres?
// People talk about inflating world to volume and doing line test. I guess they mean for sweep area
//
// Tim Schroeder/Schoeder has an article+source in august 2001 gamedev about sphere collision used in red faction
//
// Need to ask what the general consensus is these days for swept volumes and triangle soup
//
//=============================================================================
#include "stdafx.h"
#include "Collision.h"

//-----------------------------------------------------------------------------
// Name: ClipVelocity()
// Desc: Clips velocity to a normal
//-----------------------------------------------------------------------------
void ClipVelocity (Vector& in, Vector& normal, Vector& out, float overbounce)
{
	float	backoff;
	Vector	change;
	
	backoff = in.Dot(normal);

	if ( backoff < 0 ) {
		backoff *= overbounce;
	} else {
		backoff /= overbounce;
	}

	change = normal*backoff;
	out = in - change;
}


//-----------------------------------------------------------------------------
// Name: CollideAndSlide()
// Desc: Main collision routine
// TODO: Calc ground plane
//-----------------------------------------------------------------------------
bool World::CollideAndSlide(Actor& actor, float gravity,vector<Actor*>& touchList, CollisionInfo& returnInfo){
	const int maxBumps = 4;		// Bump around no more than four times
	const int maxClipPlanes = 5;// Clip against no more than 5 planes (3 hit + vel + ground)

	const float OVERCLIP = 1.001f;

	Vector planes[maxClipPlanes];// Planes to clip velocity by
	int numPlanes = 0;
	int bumpCount = 0;

	int			i, j;
	
	Vector startPos = actor.Location;
	Vector startVel = actor.Velocity;

	// Gravity must be dealt with specially, because we're dealing with discreet intervals of time
	// gravity is P = (V+0.5*G*T), then V = (V+G*T)
	if ( gravity ) {
		float t = GDeltaTime;
		actor.Velocity.y -=  gravity * t;
	}

	// The input velocity is in constant game time, we need to work in update/frame time
	// for movement to avoid framerate dependent behaviour
	float timeLeft = GDeltaTime;

	// Pre-calculate possible objects. If you've got an efficient culling algorithm
	// this can be directly intergrated into the TraceThroughWorld function
	vector<CollisionFace> possibleobjects;

	BBox box = actor.CollisionBox;
	box.min += actor.Location;
	box.max += actor.Location;

	GatherPossibleColliders(&actor,possibleobjects,actor.Velocity*timeLeft,box);

	static CollisionInfo prevTrace;
	static Vector prevLoc;
	static Vector prevDir;
	static BBox  prevBox;
	static int prevBump;
	static vector<CollisionFace> prevObjects;

	const double MIN_DIST = 0.001;

	bool triedCorrecting = false;
	// Do the response the Quake way
	for(bumpCount=0;bumpCount<maxBumps;bumpCount++){
		// Direction vector
		Vector dir = actor.Velocity.Normalized();
		// Move distance requested
		double move = actor.Velocity.Length()*timeLeft;

		// Update bounding box for new check
		box = actor.CollisionBox;
		box.min += actor.Location;
		box.max += actor.Location;

		Vector loc = actor.Location; // tmep

		// Attempt the move
		CollisionInfo trace = AABBSweep(&actor,possibleobjects,actor.Location,dir,box);

		bool collided = (trace.actualDistance < move);

		if(collided || bumpCount == 0)
			returnInfo = trace; // Set return info (even if there's not a collision)

		// Are we on the ground??
		if(actor.Velocity.y > 3){
			// Never on ground when going up fast
			actor.GroundNormal = Vector(0,0,0);
			actor.GroundMat = 0;
		}
		else if(collided && trace.normal.y > 0.7){ 
			// Touching ground plane
			actor.GroundNormal = trace.normal;
			actor.GroundMat = trace.mat;
		}
		// Otherwise, see if the ground is slightly below us
		else{
			BBox test = box;
			test.min.y -= 0.2f; // Check 20cm below
			CollisionInfo groundCheck;
			if(!IsStuck(&actor,possibleobjects,test,groundCheck,true)){
				// Ground is not slightly below, so reset ground plane
				// Otherwise we leave it to previous value. That allows us to retain the normal & mat without having to
				// do another full sweep test to find the plane slightly below us (IsStuck does not do a sweep)
				actor.GroundNormal = Vector(0,0,0);
				actor.GroundMat = 0;
			}
		}

		// #########################
		// PLUG & PLAY: TRIGGERS
		// otherTouched is a non-blocking object such as a zone
		if(trace.otherTouched){
			trace.otherTouched->Touched(&actor,&trace);
		}

		// If we touched an entity, add to our touched list
		if(trace.touched && !trace.touched->StaticObject) //IsPrefab?
		{
			// Only add if not in list already
			vector<Actor*>::iterator location = find(touchList.begin(), touchList.end(), trace.touched);
			if(location == touchList.end()){
				touchList.push_back(trace.touched);
				AddTouchedImpulse(actor.Velocity*actor.Mass,trace.touched);
			}
		}

		//#####################################
		// --------- PLUG & PLAY: STEP-UP --------------------------
		// TODO: Momentum must be based on height we need to climb.
		// So we must get the distance, then figure out how much velocity to apply to get there
		//
		// 1. Test position above where we want to be
		// 2. If clear, step up
		//
		// BEHAVIOUR: Usually only triggers once for small steps
		// The smooth ascent therefore comes from clearing the step enough
		if((trace.actualDistance < move) && trace.normal.y < 0.2  && !(actor.PhysicsFlags & Actor::PHYS_ONBLOCK_BOUNCE) && !(actor.CollisionFlags & Actor::CF_PASSABLE_BBOX) && !trace.stuck){
			Vector wantPos = dir*move;
			//wantPos.y += 1.5f; // 0.5 Meter climb max
			float height = 0.3f; // 0.3 Meter climb max

			// How low can we go (without hitting a step)?
			int p;
			for(p=0;p<5;p++){
				BBox test = box;
				test.min += wantPos + Vector(0,height,0);
				test.max += wantPos + Vector(0,height,0);
				CollisionInfo trace2;
				if(IsStuck(&actor,possibleobjects,test,trace2,true))
					break; // Lowest we can go
				height -= 0.06f;
			}

			if(height < 0.06f)
				height = 0.06f;

			if(p){ // If we found a place [0,1] that isn't stuck
				if(dir.y < 0)
					dir.y = 0;
				
				// Give us some upward momentum
				actor.Location += height * Vector(0,1,0);

				// We can now complete the whole move (could just return here, but playing it safe)
				trace.actualDistance = move;
				collided = false;

				// Note: For some sick reason, going up (or down) makes it catch more!
				// Ideas:
				// Too much air time where velocity can't be gained
				// Stuck doesn't trigger helping us along
				// Stair doesn't trigger for 2nd/3rd time
				//if(actor.Velocity.y < (actor.Velocity.Length() * height * GDeltaTime * 100))
				//	actor.Velocity.y -= actor.Velocity.Length() * height * 5;;

				//LogPrintf("STEPPING %f %f",height,GDeltaTime);
			}
			else{
				//LogPrintf("Failed to step up (Step too high?)");
			}

		}


		// Stuck
		if(trace.stuck && !(actor.PhysicsFlags & Actor::PHYS_ONBLOCK_BOUNCE)){
			if(!(actor.CollisionFlags & Actor::CF_ALLOW_STUCK))
			{
			//static int count = 0;
			//LogPrintf("%d: stuck ,%f %f %f",count,trace.normal.x,trace.normal.y,trace.normal.z);
			//count++;
			actor.Location += trace.normal*0.5f * GDeltaTime;
			ClipVelocity (actor.Velocity, trace.normal, actor.Velocity, 1.05f);
			}
		}
		// Not stuck
		else{
			/*if(bumpCount == 0){
				prevLoc = loc;
				prevDir = dir;
				prevBox = box;
				prevObjects = possibleobjects;
			}
			prevBump =bumpCount;*/

			if (trace.actualDistance > MIN_DIST)
			{	// actually covered some distance

				// Move the distance
				double newMove = trace.actualDistance - MIN_DIST*1.5f;

				// Cap move to [0,move]
				if(newMove > move)
					newMove = move;
				else if(newMove < 0)
					newMove = 0;

				actor.Location += dir * newMove;

				numPlanes = 0;
			}

			// Somehow we started nearer than the min distance, so back off
			// !!! FIXME !!!!
			// Appears to be a bug with edges, causing this code to trigger
			// For some reason several bounces occur on a single triangle edge
			// The normal is fine, so why does this happen??
			// !!! FIXME !!!!
			if(trace.actualDistance < MIN_DIST){
				// We have no choice but to step back, even though this may interfere with another plane
				actor.Location -= dir * MIN_DIST;

				//CollisionInfo trace2 = AABBSweep(&actor,prevObjects,prevLoc,prevDir,prevBox);
				//LogPrintf("Eeek(%d) d=%f v=%f",prevBump,trace2.actualDistance, actor.Velocity.Length());
			}
		}

		prevTrace = trace;

		//#####################################
		// --------- PLUG & PLAY: PUSHABLE OBJECTS --------------------------
		// What the hell's plug and play?
		// It's things which should really be in a different function alltogether
		// but were put here for ease-of-use
		// If the actor was pushable, then try pushing it and retry the collision


		// TODO: Rewrite this ASAP with Tokamak!!
		/*	if(!(actor.PhysicsFlags & Actor::PHYS_ONBLOCK_BOUNCE) && actor.PhysicsFlags & Actor::PHYS_ONBLOCK_PUSH && !triedCorrecting && trace.touched && trace.touched->PhysicsFlags & Actor::PHYS_PUSHABLE && 
			(!trace.touched->MyModel ||(trace.touched->MyModel&& actor.GroundMat != trace.touched->MyModel->materialRefs[0]))) // This line says if it has a model and ground mat matches, don't push
		{
			triedCorrecting = true;
			// It probably can't happen, but make sure the actor doesn't collide with us
			bool oldState = actor.StaticObject;

			actor.StaticObject = true;

			// Push the actor by our current velocity
			Vector newVelocity = actor.Velocity;
			newVelocity.y = 0;
			trace.touched->Velocity = newVelocity;
			RunPhysics(trace.touched);
			bumpCount--;

			actor.StaticObject = oldState;

			// Reset the possible colliders, which now contains an outdated actor position
			possibleobjects.erase(possibleobjects.begin(),possibleobjects.end());
			GatherPossibleColliders(&actor,possibleobjects,actor.Location,actor.Velocity*timeLeft,box);
			
			continue;
		}
*/
		if (!collided)
			 break;		// moved the entire distance

		// Deduct from our time the fraction of our velocity expended
		// Unless something goes bad, ALL velocity will be used, once we adjust
		// velocity not to collide with any clip planes
		timeLeft -= timeLeft * (trace.actualDistance / move);

		// slide along this plane
		if (numPlanes >= maxClipPlanes)
		{	
			Error("this shouldn't really happen");
			break;
		}

		planes[numPlanes++] = trace.normal;

		// Bounce!
		if(actor.PhysicsFlags & Actor::PHYS_ONBLOCK_BOUNCE){
			Vector dir = actor.Velocity.Normalized();
			Vector reflect_dir = dir + trace.normal * (-2.0 * trace.normal.Dot(dir));
			Vector bounce = reflect_dir*actor.Velocity.Length();
			actor.Velocity = bounce * actor.BounceFactor;

			//FIXME: Not the correct way to damp bounce
			//Problem is gravity causes perpetual rebounce
			if(actor.Velocity.Length() < 0.6f){
				actor.Velocity = Vector(0,0,0);
			}

			// Never do anything else on a bounce, or we might
			// (A) Have a second collision which multiplies out more bounce
			// (B) Slide, if we ran the code below
			return 1; 
		}

		// Stop!
		if(actor.PhysicsFlags & Actor::PHYS_ONBLOCK_STOP){
			actor.Velocity = Vector(0,0,0);
			continue;
		}

		//
		// modify original_velocity so it parallels all of the clip planes
		//
		for (i=0 ; i<numPlanes ; i++)
		{
			ClipVelocity (actor.Velocity, planes[i], actor.Velocity, 1.05f);

			for (j=0 ; j<numPlanes ; j++)
				if (j != i)
				{
					if (actor.Velocity.Dot(planes[j]) < 0)
						break;	// not ok
				}
			if (j == numPlanes)
				break;
		}
		
		if (i != numPlanes || actor.PhysicsFlags & Actor::PHYS_ONBLOCK_BOUNCE)
		{	// go along this plane
		}
		else
		{	// go along the crease
			if (numPlanes != 2)
			{
			//	LogPrintf("stopping dead. numplanes == %i",numPlanes);
				actor.Velocity = Vector(0,0,0);
				break;
			}
			Vector dir = Cross(planes[0],planes[1]);
			float d = dir.Dot(actor.Velocity);
			actor.Velocity = dir * d;
		}
		//
		// if velocity is against the original velocity, stop dead
		// to avoid tiny occilations in sloping corners
		//
		if (actor.Velocity.Dot(startVel) <= 0)
		{
			actor.Velocity = Vector(0,0,0);
			break;
		}
	}


	// If bumpCount > 0, we hit something along the way
	return (bumpCount != 0);
}

