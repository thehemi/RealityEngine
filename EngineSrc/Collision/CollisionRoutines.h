//=========== Copyright (c) 2003, Tim Johnson. All rights reserved. ===========
/// CollisionRoutines: Various intersection testing methods
//=============================================================================


//-----------------------------------------
/// Main AABB/Triangle intersection test
//-----------------------------------------
int TriBoxOverlap(float boxcenter[3],float boxhalfsize[3],float *triverts[3]);

//----------------------------------------------------------------
/// Compares two bounding boxes, returns
/// the state of intersection
//----------------------------------------------------------------
bool CheckBoundingBoxes(Vector& bMin1, Vector& bMax1, Vector& bMin2, Vector& bMax2);


/// t u v are return parameters, for finding point on triangle and distance
int RayIntersectsTriangle(float orig[3], float dir[3],
			float vert0[3], float vert1[3], float vert2[3],
			float *t, float *u, float *v);

//----------------------------------------------------------------
/// Checks a box against a sphere, returns
/// the state of intersection
/// This fast box-radius check is from the Real-Time rendering book
//----------------------------------------------------------------
bool CheckBoundingSphereAgainstBox( const BBox& B, const float r, const Vector& C ) ;

/// Slightly faster, slightly better
bool CheckBoundingSphereAgainstBox2( float r, float* C,float* Bmin,float* Bmax);

/// We now have three checks. Roll them up, grr
bool IsBoxIntersectingSphere( const Vector& boxMin, const Vector& boxMax, const Vector& center, float radius );

