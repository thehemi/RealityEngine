//
// FIXME: Damn problems on catching edges
//
//
#include <stdafx.h>
#include "Engine.h"
#include "Collision.h"


//
// Tests if a box is overlapping a triangle
// 
int TriBoxOverlap(float boxcenter[3],float boxhalfsize[3],float *triverts[3]);
bool IsStuck(Actor* source, vector<CollisionFace>& touchList, const BBox& worldBox, CollisionInfo& outTrace, bool largerBBox){
	// This is for the TriBoxOverlap
	// Which figures out if our bbox (without velocity/move) STARTED in a solid
	Vector center = (worldBox.max+worldBox.min)*.5f;

	
	Vector half;
	if(!largerBBox) // Minus a little because we only want to know when we're well and truly buggered
		half = (worldBox.max-worldBox.min)*.5f- Vector(0.001f,0.001f,0.001f);
	else // We want to know when we're coming close to a plane, so we can avoid a collision
		half = (worldBox.max-worldBox.min)*.5f+ Vector(0.001f,0.001f,0.001f);
	
	for(int i=0;i<touchList.size();i++){
		// Skip passable faces
		if(touchList[i].owner && (touchList[i].owner->CollisionFlags & Actor::CF_PASSABLE_BBOX || (source && source->CollisionFlags & Actor::CF_PASSABLE_BBOX)))
			continue;

		// First, make sure we didn't start in any solids
		float *triverts[]={(float*)&touchList[i].vert[0],(float*)&touchList[i].vert[1],(float*)&touchList[i].vert[2]};
		
		if(TriBoxOverlap((float*)&center,(float*)&half,triverts)){
			outTrace.normal = touchList[i].normal;
			outTrace.touched = touchList[i].owner;
			outTrace.mat = touchList[i].mat;
			return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
// Name: AABBSweep()
// Desc: Traces Box through world, and finds nearest intersecting Face
// Input: face list, from FindPossibleColliders()
// Out: CollisionInfo structure
//
// Note: Backface culling commented out because it didn't work, nor should it
// The vertex normals for the box point outwards, but you may still rely on them
// while moving away from them, such as when going down a slope
//
// NOTE: It is vital we get the real normal, and not the face normal, as we need to slide parallel to it
// For example, if we hit an edge and the normal is (0,1,0) we won't adjust velocity, and will hit
// it again next bounce
//-----------------------------------------------------------------------------
int ray_intersectFace(CollisionFace& f, Vector& ro,const Vector& rd,Vector& ip,float& dist);
int ray_intersectBBox(const Vector& min, const Vector& max, const Vector& ro, const Vector& rd,float& tnear,float& tfar);
int edge_collision(Vector& p1,Vector& p2,const Vector& dir,Vector& p3,Vector& p4,float& dist,Vector& ip);
// This is useful for debugging
#define AABBVERT_FACE 1
#define FACEVERT_AABB 2
#define EDGE_EDGE	  3

CollisionInfo AABBSweep(Actor* source, vector<CollisionFace>& touchList, const Vector& start, const Vector& dir,const BBox& worldBox){
	CollisionInfo trace;

	// We return any interesections nearer than this. Yes, we could make BIG_NUMBER
	// the move distance, but it can cause epsilon errors, so play it safe
	double nearest = BIG_NUMBER;

	int collision = false;
	BBox aabb = worldBox;

	// Make sure we didn't start stuck!
	if(IsStuck(source,touchList,aabb,trace,false)){
		trace.stuck = true;
		trace.actualDistance = 0;
		return trace;
	}

	for(int i=0;i<touchList.size();i++){
		CollisionFace& face = touchList[i];
		// If our general move is against the plane, ignore it..
		// NOTE: We must do this, or we collide with innoculus planes when there
		// are important ones parallel (messy floor with one plane higher than other)
		if(dir.Dot(face.normal) > 0)
			continue;
		
		Vector ip;
		float dist;
		int j;

		// For each of the 8 AABB vertices:
		// Does the ray(AABB vert, move direction) intersect a face
		for(j=0;j<8;j++){
			//if(dir.Dot(*(Vector*)&BBox::vertnorm[j]) < 0)
			// continue;
			
			if(ray_intersectFace(face,aabb.GetVert(j),dir,ip,dist)){
				if(dist < nearest){
					if(face.owner && face.owner->CollisionFlags & Actor::CF_PASSABLE_BBOX){
						// Non blocking object, just record it for calling touch() later
						// If we bumped into a passable, it's a different type of touched message (one-way, and extra to a normal message)
						trace.otherTouched = face.owner;
					}
					else{
						nearest = dist;
						trace.mat = face.mat;
						trace.touched = face.owner;
						collision = AABBVERT_FACE;
						trace.normal = face.normal;
						trace.point = ip;
					}
				}
			}
		}
		
		// For each of the face vertices
		// Does the ray(Face vert, negative move direction) intersect the bounding box?
		// This is pretty rare. 1 / 20 polygons peharps.
		for(j=0;j<3;j++){
		
			float fNear,fFar;
			int fi = ray_intersectBBox(aabb.min,aabb.max,face.vert[j],-dir,fNear,fFar);
			if(fi != -1){
				if(fNear < nearest){
					if(face.owner && face.owner->CollisionFlags & Actor::CF_PASSABLE_BBOX){
						// Non blocking object, just record it for calling touch() later
						// If we bumped into a passable, it's a different type of touched message (one-way, and extra to a normal message)
						trace.otherTouched = face.owner;
					}
					else{
						nearest = fNear;
						trace.mat = face.mat;
						trace.touched = face.owner;
						collision = FACEVERT_AABB;
						// The reflection normal is the inverse face normal of our AABB
						trace.normal = -*(Vector*)&BBox::facenorm[fi];
					}
				}
			}
		}

		// For each of the 12 AABB edges
		// Do any of the 12 AABB edges touch any of the 3 face edges?
		// NOTE: This is usually the problematic one, as it triggers when bumping into polygon edges
		for(j=0;j<12;j++){
			//if(dir.Dot(*(Vector*)&BBox::edgenorm[j]) < 0)
			//	continue;

			// Edge list
			Vector edges[3][2];
			edges[0][0] = face.vert[1];
			edges[0][1] = face.vert[0];
			//edges[0][2] = face.vert[2]; // Non-edge

			edges[1][0] = face.vert[2];
			edges[1][1] = face.vert[1];
			//edges[1][2] = face.vert[0]; // Non-edge

			edges[2][0] = face.vert[0];
			edges[2][1] = face.vert[2];
			//edges[2][2] = face.vert[1]; // Non-edge

				for(int m=0;m<3;m++){
					 //if((face.edge[m]).Dot(dir) > 0)
					 //     continue;

					// For each of the 3 face edges
					if(edge_collision(aabb.GetVert(BBox::edgevert[j][1]),aabb.GetVert(BBox::edgevert[j][0]),dir,edges[m][1],edges[m][0],dist,ip)){
						if(dist < nearest){
							if(face.owner && face.owner->CollisionFlags & Actor::CF_PASSABLE_BBOX){
								// Non blocking object, just record it for calling touch() later
								// If we bumped into a passable, it's a different type of touched message (one-way, and extra to a normal message)
								trace.otherTouched = face.owner;
							}
							else{
								nearest = dist;
								collision = EDGE_EDGE + m;
								trace.point = ip;
								trace.touched = face.owner;
								trace.mat = face.mat;
								trace.normal = face.normal;

								// Calculate
								Vector p1 = aabb.GetVert(BBox::edgevert[j][0]) - aabb.GetVert(BBox::edgevert[j][1]);
								Vector p2 = edges[m][1] - edges[m][0];
								p1.Normalize();
								p2.Normalize();

								trace.normal = Cross(p1,p2);
								// TIM: This causes issues
								// Cross is non-commutative, so make sure we've got the result the right way around 
								if(trace.normal.Dot(dir) > 0){
									trace.normal = -trace.normal;
								}
							}
						}
					}
				}
		}
	}

	// Log for checking how far away we are, to see if we should consider ourselves falling etc
	trace.actualDistance = nearest;

	return trace;
}