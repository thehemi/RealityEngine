//=========== Copyright (c) 2003, Tim Johnson. All rights reserved. ===========
// CollisionRoutines: Various intersection testing methods
//=============================================================================
#include "stdafx.h"
#include "CollisionRoutines.h"
#include "Engine.h"
#include "Collision.h"


// Checks a bbox against a bbox
// Updated touchList with box triangles that need to be checked for collision
// Basically just builds a bbox model from a world bbox min/max
// FIXME: Speed problems? Happened when lights were bboxes and checked. Got 12 fps
void CheckBBox(BBox& sweepBox, Actor* actor, vector<CollisionFace>& touchList){

	BBox worldBox = actor->CollisionBox;

	// This should never happen (because the bbox is derived from the model by default)
	// But it does happen on the first frame before the bbox is set
	// so return and pretend this actor doesn't exist yet.
	if(worldBox == BBox(Vector(0,0,0),Vector(0,0,0)) && actor->MyModel)
		return;
	//	Error("'%s' has CF_BBOX physics turned on,\n\but hasn't set a CollisionBox.\n\All actors that use the physics system must do this.",actor->ClassName());

	// This can happen if no explicit bbox was set, and the model is unloaded
	// Because it happens when you're missing models, let's not make it an error
	if(worldBox == BBox(Vector(0,0,0),Vector(0,0,0)))
		return;


	worldBox.min += actor->Location;
	worldBox.max += actor->Location;
	//worldBox = worldBox.Transformed(actor->Rotation);


	// Test sweep against actor bbox
	if(sweepBox.Touches(worldBox)){
		// Foreach face
		for(int i=0;i<6;i++){

			// Get face verts of box
			Vector vert[4];
			for(int p=0;p<4;p++)
				vert[(-p)+3] = worldBox.GetVert(BBox::facevert[i][p]);

			// Basic culling
			BBox cullBox;
			cullBox+=vert[0];
			cullBox+=vert[1];
			cullBox+=vert[2];
			cullBox+=vert[3];
			if(!sweepBox.Touches(cullBox))
				continue;


			Model* model = actor->MyModel;
			CollisionFace f;
			f.owner = actor;
	
			// TODO: Get a material and put it here
			//if(actor->MyModel && actor->MyModel->materialRefs.size())
			//	f.mat = actor->MyModel->materialRefs[0];
			//else
				f.mat = 0;

			// Face normal (for both tris)
			// FIXME: Why -cross? edge normals might be wrong too
			f.normal = -Cross((vert[1] - vert[0]),(vert[2] - vert[1]));
			f.normal = Cross((vert[1] - vert[0]),(vert[2] - vert[0]));


			// First triangle
			// v
			f.vert[0] = vert[0];
			f.vert[1] = vert[1];
			f.vert[2] = vert[2];

			// Plane/Edge normals (pointing outwards) from first triangle
			f.edgeNorm[0] = Cross((vert[1] - vert[0]),f.normal);
			f.edgeNorm[1] = Cross((vert[2] - vert[1]),f.normal);
			f.edgeNorm[2] = Cross((vert[0] - vert[2]),f.normal);

			touchList.push_back(f);

			// Second triangle
			// v
			f.vert[0] = vert[2];
			f.vert[1] = vert[3];
			f.vert[2] = vert[0];

			// Make sure tri2 normal matches tri1 normal
			assert(f.normal.Dot(Cross((vert[1] - vert[0]),(vert[2] - vert[0]))) > 0.99);
			// e
			// Plane/Edge normals (pointing outwards) from second triangle
			f.edgeNorm[0] = Cross((f.vert[1] - f.vert[0]),f.normal);
			f.edgeNorm[1] = Cross((f.vert[2] - f.vert[1]),f.normal);
			f.edgeNorm[2] = Cross((f.vert[0] - f.vert[2]),f.normal);

			touchList.push_back(f);
		}
	}
}


// A few defines to make things a little faster
#define FPBITS(fp) (*(int *)&(fp))
#define FPSIGNBIT(fp)	(FPBITS(fp)&0x80000000)
#define FPABSBITS(fp)	(FPBITS(fp)&0x7FFFFFFF)
#define FPABS(fp)		(*((int *)&fp)=FPABSBITS(fp))
#define VECDOT(v1,v2) ((v1).x*(v2).x+(v1).y*(v2).y+(v1).z*(v2).z)


/**
 *	Computes a ray-triangle intersection test.
 *	Original code from Tomas Möller's "Fast Minimum Storage Ray-Triangle Intersection".
 *	It's been optimized a bit with integer code, and modified to return a non-intersection if distance from
 *	ray origin to triangle is negative.
 *
 *	\param		vert0	[in] triangle vertex
 *	\param		vert1	[in] triangle vertex
 *	\param		vert2	[in] triangle vertex
 *	\return		true on overlap. mStabbedFace is filled with relevant info.
 */
/*
#define LOCAL_EPSILON 0.000001f
#define IS_NEGATIVE_FLOAT(x)	(IR(x)&0x80000000)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ray_intersectFace1(CollisionFace& f, Vector& mOrigin,Vector& mDir,float& dist)
{
	Vector vert0 = f.vert[0], vert1 = f.vert[1], vert2 = f.vert[2];


	// Find vectors for two edges sharing vert0
	Vector edge1 = vert1 - vert0;
	Vector edge2 = vert2 - vert0;


	// Begin calculating determinant - also used to calculate U parameter
	Vector pvec = mDir^edge2;

	// If determinant is near zero, ray lies in plane of triangle
	float det = edge1|pvec;

		// the non-culling branch
		if(det>-LOCAL_EPSILON && det<LOCAL_EPSILON)									return FALSE;
		float OneOverDet = 1.0f / det;

		// Calculate distance from vert0 to ray origin
		Vector tvec = mOrigin - vert0;

		// Calculate U parameter and test bounds
		float mU = (tvec|pvec) * OneOverDet;
		if(IS_NEGATIVE_FLOAT(mU) || IR(mU)>IEEE_1_0)		return FALSE;

		// prepare to test V parameter
		Vector qvec = tvec^edge1;

		// Calculate V parameter and test bounds
		float mV = (mDir|qvec) * OneOverDet;
		if(IS_NEGATIVE_FLOAT(mV) || mU+mV>1.0f)	return FALSE;

		// Calculate t, ray intersects triangle
		dist = (edge2|qvec) * OneOverDet;
		// Intersection point is valid if distance is positive (else it can just be a face behind the orig point)
		if(IS_NEGATIVE_FLOAT(dist))								return FALSE;

	return TRUE;
}
*/


bool SameSide(Vector p1,Vector p2, Vector a,Vector b){
    Vector cp1 = Cross(b-a, p1-a);
   Vector cp2 = Cross(b-a, p2-a);
    if (cp1.Dot(cp2) >= 0) return true;
    else return false;
}

bool PointInTriangle(Vector p, Vector a,Vector b,Vector c){
    if (SameSide(p,a, b,c) && SameSide(p,b, a,c)
        && SameSide(p,c, a,b)) return true;
    else return false;
}



// computes intersection from a ray defined by its origin (ro) and 
// direction vector (rd). Returns true on a collision and the 
// intersection point (ip) and collision distance (dist)
int ray_intersectFace2(CollisionFace& f, Vector& ro,const Vector& rd,Vector& ip,float& dist)
{
	// back face culling
	float x=f.normal.Dot(rd);
	bool b = (FPSIGNBIT(x)==0);
	//if (FPSIGNBIT(x)==0)
	//	return 0; // no intersection

	// compute intersection distance from ray and face plane
	float d0 = f.vert[0].Dot(f.normal);
	dist=(d0-f.normal.Dot(ro))/x;
	if (FPSIGNBIT(dist))
		return 0; // no intersection

	// compute intersection point in face plane

	ip.x=ro.x+rd.x*dist;
	ip.y=ro.y+rd.y*dist;
	ip.z=ro.z+rd.z*dist;

	// check if intersection point is inside the polygon
	// testing the dot products with the polygon edge normals
	for( int i=0;i<3;i++ ){
		if((ip - f.vert[i]).Dot(f.edgeNorm[i]) > 0)
			return 0; // no intersection
	}

	return 1; // intersection found
}


/*
bool ray_intersectFace3( CollisionFace& f,
                                   Vector LinePoint,
                                   Vector LineVector,
                                   Vector& ip, float& dist)//3 vertices
{
	
	Vector ix;
	Vector* IntersecPoint = &ix;
	Vector Polygon[3];
	Polygon[0] = f.vert[0];
	Polygon[1] = f.vert[1];
	Polygon[2] = f.vert[2];

   //first we get the plain constant and normal
   Vector pnormal,pq, pr,pn,px;  // Difference vectors
   pq= Polygon[1]-Polygon[0] ;
   pr= Polygon[2]-Polygon[0] ;
   pnormal = Cross( pq, pr);

    float pconstent=pnormal.Dot(Polygon[0]);

   //now we get the intersection point
   float lambda = (pconstent - pnormal.Dot(LinePoint))
                  /(pnormal.Dot(LineVector) + 1e-7);

   IntersecPoint->x = LinePoint.x + lambda * LineVector.x;
   IntersecPoint->y = LinePoint.y + lambda * LineVector.y;
   IntersecPoint->z = LinePoint.z + lambda * LineVector.z;
   //first we check if lamda is positive
   if(lambda<0)return false;
     float u0,u1,u2,v0,v1,v2,beta,alpha=0,xy,xz,yz;
     //calculate the dominate Axis of Alignment
     float x1=Polygon[1].x-Polygon[0].x;
     float x2=Polygon[2].x-Polygon[0].x;

     float y1=Polygon[1].y-Polygon[0].y;
     float y2=Polygon[2].y-Polygon[0].y;

     float z1=Polygon[1].z-Polygon[0].z;
     float z2=Polygon[2].z-Polygon[0].z;

     if(pnormal.x<0)pnormal.x*= -1;
     if(pnormal.y<0)pnormal.y*= -1;
     if(pnormal.z<0)pnormal.z*= -1;

     if((pnormal.x<=pnormal.z)&&(pnormal.y<=pnormal.z))
       {
       u0=IntersecPoint->x-Polygon[0].x;
       v0=IntersecPoint->y-Polygon[0].y;
       u1=x1; u2=x2;
       v1=y1; v2=y2;
       }
       else
       if((pnormal.x<=pnormal.y)&&(pnormal.z<=pnormal.y))
         {
         u0=IntersecPoint->x-Polygon[0].x;
         v0=IntersecPoint->z-Polygon[0].z;
         u1=x1; u2=x2;
         v1=z1; v2=z2;
         }
         else
         {
         u0=IntersecPoint->y-Polygon[0].y;
         v0=IntersecPoint->z-Polygon[0].z;
         u1=y1; u2=y2;
         v1=z1; v2=z2;
         }

    if(u1==0)
     {beta=u0/u2;
      if((0<=beta)&&(beta<=1)) alpha =(v0-(beta *v2))/v1;
     }
     else
     {beta=(((v0*u1)-(u0*v1))/((v2*u1)-(u2*v1)));
      if((0<=beta)&&(beta<=1)) alpha =(u0-(beta *u2))/u1;
     }
  //if point is inside the poly then all condition will be met.
	 bool d=(((alpha>=0)&&(beta>=0))&&((alpha+beta)<=1)) ;
	 dist = lambda;
  return d;
}

int ray_intersectFace4( CollisionFace& f,
                                   Vector L0,
                                   Vector u, Vector& ip, float& dist )
{
	
	Vector V0 = f.vert[0];
	Vector Pn = f.normal;

   // Vector    u = L1 - L0;
    Vector    w = L0 - V0;

    float     D = Pn.Dot(u);
    float     N = -Pn.Dot(w);

    if (fabs(D) < 0.0001f) {          // segment is parallel to plane
        if (N == 0)                     // segment lies in plane
            Error("WHEE");//return 2;
        else
            return 0;                   // no intersection
    }
    // they are not parallel
    // compute intersect param
    float sI = N / D;
    if (sI < 0)// || sI > 1)
        return 0;                       // no intersection

  ip = L0 + sI * u;                 // compute segment intersect point

	dist = sI;
    return 1;
}

*/
int ray_intersectFace(CollisionFace& f, Vector& ro,const Vector& rd,Vector& ip,float& dist){
	return ray_intersectFace2(f,ro,rd,ip,dist);
/*	bool b1 = ray_intersectFace1(f,ro,rd,dist);
	//return b1;
	bool b2 = ray_intersectFace2(f,ro,rd,ip,dist);
	bool b3 = ray_intersectFace3(f,ro,rd,ip,dist);
	bool b4 = ray_intersectFace4(f,ro,rd,ip,dist);
	if(b1!=b2 || b1 !=b3){
 		b4 = ray_intersectFace4(f,ro,rd,ip,dist);
		LogPrintf("%d %d %d %d",b1,b2,b3,b4);
	}
	return b2;*/
}


// collide ray defined by ray origin (ro) and ray direction (rd)
// with the bounding box. Returns -1 on no collision and the face index 
// for first intersection if a collision is found together with 
// the distances to the collision points (tnear and tfar)
int ray_intersectBBox(const Vector& min, const Vector& max,  const Vector& ro,const Vector& rd,float& tnear,float& tfar)
{
	float t1,t2,t;
	int ret=-1;

	tnear=-BIG_NUMBER;
	tfar=BIG_NUMBER;

	int a,b;
	for( a=0;a<3;a++ )
	{
		if (rd[a]>-KINDA_SMALL_NUMBER && rd[a]<KINDA_SMALL_NUMBER)
			if (ro[a]<min[a] || ro[a]>max[a])
				return -1;
			else ;
		else 
		{
			t1=(min[a]-ro[a])/rd[a];
			t2=(max[a]-ro[a])/rd[a];
			if (t1>t2)
			{ 
				t=t1; t1=t2; t2=t; 
				b=3+a;
			}
			else
				b=a;
			if (t1>tnear)
			{
				tnear=t1;
				ret=b;
			}
			if (t2<tfar)
				tfar=t2;
			if (tnear>tfar || tfar<KINDA_SMALL_NUMBER)
				return -1;
		}
	}
	
	if (tnear>tfar || tfar<KINDA_SMALL_NUMBER)
		return -1;

	return ret;
}



// collide edge (p1,p2) moving in direction (dir) colliding
// with edge (p3,p4). Return true on a collision with 
// collision distance (dist) and intersection point (ip)
int edge_collision(Vector& p1,Vector& p2,const Vector& dir,Vector& p3,Vector& p4,float& dist,Vector& ip)
{
	Vector v1=p2-p1;
	Vector v2=p4-p3;

	// build plane based on edge (p1,p2) and move direction (dir)
	Vector plane;
	plane = Cross(v1,dir);
	float planeW=VECDOT(plane,p1);

	// if colliding edge (p3,p4) does not cross plane return no collision
	// same as if p3 and p4 on same side of plane return 0
	float temp=(VECDOT(plane,p3)-planeW)*(VECDOT(plane,p4)-planeW);
	if (FPSIGNBIT(temp)==0)
		return 0;

	// if colliding edge (p3,p4) and plane are parallel return no collision
	v2.Normalize();
	temp=VECDOT(plane,v2);
	if(FPBITS(temp)==0)
		return 0;
	
	// compute intersection point of plane and colliding edge (p3,p4)
	ip=p3+v2*((planeW-VECDOT(plane,p3))/temp);

	// find largest 2D plane projection
	FPABS(plane.x);
	FPABS(plane.y);
	FPABS(plane.z);
	int i,j;
	if (plane.x>plane.y) i=0; else i=1;
	if (plane[i]<plane.z) i=2;
	if (i==0) { i=1; j=2; } else if (i==1) { i=0; j=2; } else { i=0; j=1; }

	// compute distance of intersection from line (ip,-dir) to line (p1,p2)
	dist=(v1[i]*(ip[j]-p1[j])-v1[j]*(ip[i]-p1[i]))/
		(v1[i]*dir[j]-v1[j]*dir[i]);
	if (FPSIGNBIT(dist)) 
		return 0;

	// compute intersection point on edge (p1,p2) line
	ip-=dist*dir;

	// check if intersection point (ip) is between egde (p1,p2) vertices
	temp=(p1.x-ip.x)*(p2.x-ip.x)+(p1.y-ip.y)*(p2.y-ip.y)+(p1.z-ip.z)*(p2.z-ip.z);
	if (FPSIGNBIT(temp))
		return 1;	// collision found!
	
	return 0; // no collision
}

// Does what it says on the box
bool RayCheckBBox(const Vector& start, const Vector& end, Actor* actor, CollisionInfo& info){
	float dirLength = (end - start).Length();
	Vector dir = (end - start).Normalized();

	BBox worldBox = actor->CollisionBox;
	worldBox.min += actor->Location;
	worldBox.max += actor->Location;

	// Cast ray to model bbox
	float fNear,fFar;
	int fi = ray_intersectBBox(worldBox.min,worldBox.max,start,dir,fNear,fFar);

	// fNear is the distance to the object
	// fi is the face index
	if(fi != -1 && fNear > 0 && fNear < dirLength){
		// TODO: Get a material and put it here
		//if(actor->MyModel && actor->MyModel->materialRefs.size())
		//	info.mat = actor->MyModel->materialRefs[0];
		//else
			info.mat = 0;

		info.point = start + (fNear * dir);
		info.normal = *(Vector*)BBox::facenorm[fi];
		info.actualDistance = fNear;
		return true;
	}
	return false;
}

#define X 0
#define Y 1
#define Z 2
#define EPSILON 0.000001

#define CROSS(dest,v1,v2) \
          dest[0]=v1[1]*v2[2]-v1[2]*v2[1]; \
          dest[1]=v1[2]*v2[0]-v1[0]*v2[2]; \
          dest[2]=v1[0]*v2[1]-v1[1]*v2[0]; 

#define DOT(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])

#define SUB(dest,v1,v2) \
          dest[0]=v1[0]-v2[0]; \
          dest[1]=v1[1]-v2[1]; \
          dest[2]=v1[2]-v2[2]; 

#define FINDMINMAX(x0,x1,x2,min,max) \
  min = max = x0;   \
  if(x1<min) min=x1;\
  if(x1>max) max=x1;\
  if(x2<min) min=x2;\
  if(x2>max) max=x2;

int PlaneBoxOverlap(float normal[3],float d, float maxbox[3])
{
  int q;
  float vmin[3],vmax[3];
  for(q=X;q<=Z;q++)
  {
    if(normal[q]>0.0f)
    {
      vmin[q]=-maxbox[q];
      vmax[q]=maxbox[q];
    }
    else
    {
      vmin[q]=maxbox[q];
      vmax[q]=-maxbox[q];
    }
  }
  if(DOT(normal,vmin)+d>0.0f) return 0;
  if(DOT(normal,vmax)+d>=0.0f) return 1;
  
  return 0;
}


/*======================== X-tests ========================*/
#define AXISTEST_X01(a, b, fa, fb)			   \
	p0 = a*v0[Y] - b*v0[Z];			       	   \
	p2 = a*v2[Y] - b*v2[Z];			       	   \
        if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} \
	rad = fa * boxhalfsize[Y] + fb * boxhalfsize[Z];   \
	if(min>rad || max<-rad) return 0;

#define AXISTEST_X2(a, b, fa, fb)			   \
	p0 = a*v0[Y] - b*v0[Z];			           \
	p1 = a*v1[Y] - b*v1[Z];			       	   \
        if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
	rad = fa * boxhalfsize[Y] + fb * boxhalfsize[Z];   \
	if(min>rad || max<-rad) return 0;

/*======================== Y-tests ========================*/
#define AXISTEST_Y02(a, b, fa, fb)			   \
	p0 = -a*v0[X] + b*v0[Z];		      	   \
	p2 = -a*v2[X] + b*v2[Z];	       	       	   \
        if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} \
	rad = fa * boxhalfsize[X] + fb * boxhalfsize[Z];   \
	if(min>rad || max<-rad) return 0;

#define AXISTEST_Y1(a, b, fa, fb)			   \
	p0 = -a*v0[X] + b*v0[Z];		      	   \
	p1 = -a*v1[X] + b*v1[Z];	     	       	   \
        if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
	rad = fa * boxhalfsize[X] + fb * boxhalfsize[Z];   \
	if(min>rad || max<-rad) return 0;

/*======================== Z-tests ========================*/

#define AXISTEST_Z12(a, b, fa, fb)			   \
	p1 = a*v1[X] - b*v1[Y];			           \
	p2 = a*v2[X] - b*v2[Y];			       	   \
        if(p2<p1) {min=p2; max=p1;} else {min=p1; max=p2;} \
	rad = fa * boxhalfsize[X] + fb * boxhalfsize[Y];   \
	if(min>rad || max<-rad) return 0;

#define AXISTEST_Z0(a, b, fa, fb)			   \
	p0 = a*v0[X] - b*v0[Y];				   \
	p1 = a*v1[X] - b*v1[Y];			           \
        if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
	rad = fa * boxhalfsize[X] + fb * boxhalfsize[Y];   \
	if(min>rad || max<-rad) return 0;

int TriBoxOverlap(float boxcenter[3],float boxhalfsize[3],float *triverts[3])
{

  /*    use separating axis theorem to test overlap between triangle and box */
  /*    need to test for overlap in these directions: */
  /*    1) the {x,y,z}-directions (actually, since we use the AABB of the triangle */
  /*       we do not even need to test these) */
  /*    2) normal of the triangle */
  /*    3) crossproduct(edge from tri, {x,y,z}-directin) */
  /*       this gives 3x3=9 more tests */
   float v0[3],v1[3],v2[3];
   //float axis[3];
   float min,max,d,p0,p1,p2,rad,fex,fey,fez;  
   float normal[3],e0[3],e1[3],e2[3];

   /* This is the fastest branch on Sun */
   /* move everything so that the boxcenter is in (0,0,0) */
   SUB(v0,triverts[0],boxcenter);
   SUB(v1,triverts[1],boxcenter);
   SUB(v2,triverts[2],boxcenter);

   /* compute triangle edges */
   SUB(e0,v1,v0);      /* tri edge 0 */
   SUB(e1,v2,v1);      /* tri edge 1 */
   SUB(e2,v0,v2);      /* tri edge 2 */

   /* Bullet 3:  */
   /*  test the 9 tests first (this was faster) */
   fex = fabs(e0[X]);
   fey = fabs(e0[Y]);
   fez = fabs(e0[Z]);
   AXISTEST_X01(e0[Z], e0[Y], fez, fey);
   AXISTEST_Y02(e0[Z], e0[X], fez, fex);
   AXISTEST_Z12(e0[Y], e0[X], fey, fex);

   fex = fabs(e1[X]);
   fey = fabs(e1[Y]);
   fez = fabs(e1[Z]);
   AXISTEST_X01(e1[Z], e1[Y], fez, fey);
   AXISTEST_Y02(e1[Z], e1[X], fez, fex);
   AXISTEST_Z0(e1[Y], e1[X], fey, fex);

   fex = fabs(e2[X]);
   fey = fabs(e2[Y]);
   fez = fabs(e2[Z]);
   AXISTEST_X2(e2[Z], e2[Y], fez, fey);
   AXISTEST_Y1(e2[Z], e2[X], fez, fex);
   AXISTEST_Z12(e2[Y], e2[X], fey, fex);

   /* Bullet 1: */
   /*  first test overlap in the {x,y,z}-directions */
   /*  find min, max of the triangle each direction, and test for overlap in */
   /*  that direction -- this is equivalent to testing a minimal AABB around */
   /*  the triangle against the AABB */

   /* test in X-direction */
   FINDMINMAX(v0[X],v1[X],v2[X],min,max);
   if(min>boxhalfsize[X] || max<-boxhalfsize[X]) return 0;

   /* test in Y-direction */
   FINDMINMAX(v0[Y],v1[Y],v2[Y],min,max);
   if(min>boxhalfsize[Y] || max<-boxhalfsize[Y]) return 0;

   /* test in Z-direction */
   FINDMINMAX(v0[Z],v1[Z],v2[Z],min,max);
   if(min>boxhalfsize[Z] || max<-boxhalfsize[Z]) return 0;

   /* Bullet 2: */
   /*  test if the box intersects the plane of the triangle */
   /*  compute plane equation of triangle: normal*x+d=0 */
   CROSS(normal,e0,e1);
   d=-DOT(normal,v0);  /* plane eq: normal.x+d=0 */
   if(!PlaneBoxOverlap(normal,d,boxhalfsize)) return 0;

   return 1;   /* box and triangle overlaps */
}



/* code rewritten to do tests on the sign of the determinant */
/* the division is before the test of the sign of the det    */
/* and one CROSS has been moved out from the if-else if-else */
int RayIntersectsTriangle(float orig[3], float dir[3],
			float vert0[3], float vert1[3], float vert2[3],
			float *t, float *u, float *v)
{
   float edge1[3], edge2[3], tvec[3], pvec[3], qvec[3];
   float det,inv_det;

   /* find vectors for two edges sharing vert0 */
   SUB(edge1, vert1, vert0);
   SUB(edge2, vert2, vert0);

   /* begin calculating determinant - also used to calculate U parameter */
   CROSS(pvec, dir, edge2);

   /* if determinant is near zero, ray lies in plane of triangle */
   det = DOT(edge1, pvec);

   /* calculate distance from vert0 to ray origin */
   SUB(tvec, orig, vert0);
   inv_det = 1.0 / det;
   
   CROSS(qvec, tvec, edge1);
      
   if (det > EPSILON)
   {
      *u = DOT(tvec, pvec);
      if (*u < 0.0 || *u > det)
	 return 0;
            
      /* calculate V parameter and test bounds */
      *v = DOT(dir, qvec);
      if (*v < 0.0 || *u + *v > det)
	 return 0;
      
   }
   else if(det < -EPSILON)
   {
      /* calculate U parameter and test bounds */
      *u = DOT(tvec, pvec);
      if (*u > 0.0 || *u < det)
	 return 0;
      
      /* calculate V parameter and test bounds */
      *v = DOT(dir, qvec) ;
      if (*v > 0.0 || *u + *v < det)
	 return 0;
   }
   else return 0;  /* ray is parallell to the plane of the triangle */

   *t = DOT(edge2, qvec) * inv_det;
   (*u) *= inv_det;
   (*v) *= inv_det;

   return 1;
}


//----------------------------------------------------------------
// Compares two bounding boxes, returns
// the state of intersection
//----------------------------------------------------------------
bool CheckBoundingBoxes(Vector& bMin1, Vector& bMax1, Vector& bMin2, Vector& bMax2){
	if( bMax1.x<bMin1.x||
		bMax1.y<bMin2.y||
		bMax1.z<bMin2.z||
		bMin1.x>bMax2.x||
		bMin1.y>bMax2.y||
		bMin1.z>bMax2.z)
	{
		return false;
	}

	return true;
}



//-----------------------------------------------------------------------------
// Returns true if a box intersects with a sphere
//-----------------------------------------------------------------------------
bool IsBoxIntersectingSphere( const Vector& boxMin, const Vector& boxMax, const Vector& center, float radius )
{
	// See Graphics Gems, box-sphere intersection
	float dmin = 0.0f;
	float flDelta;

	// Unrolled the loop.. this is a big cycle stealer...
	if (center[0] < boxMin[0])
	{
		flDelta = center[0] - boxMin[0];
		dmin += flDelta * flDelta;
	}
	else if (center[0] > boxMax[0])
	{
		flDelta = boxMax[0] - center[0];
		dmin += flDelta * flDelta;
	}

	if (center[1] < boxMin[1])
	{
		flDelta = center[1] - boxMin[1];
		dmin += flDelta * flDelta;
	}
	else if (center[1] > boxMax[1])
	{
		flDelta = boxMax[1] - center[1];
		dmin += flDelta * flDelta;
	}

	if (center[2] < boxMin[2])
	{
		flDelta = center[2] - boxMin[2];
		dmin += flDelta * flDelta;
	}
	else if (center[2] > boxMax[2])
	{
		flDelta = boxMax[2] - center[2];
		dmin += flDelta * flDelta;
	}

	return dmin < radius * radius;
}


//----------------------------------------------------------------
// Checks a box against a sphere, returns
// the state of intersection
// This fast box-radius check is from the Real-Time rendering book
//----------------------------------------------------------------
//Check to see if the sphere overlaps the AABB
bool CheckBoundingSphereAgainstBox ( const BBox& B, const float r, const Vector& C ) 
{ 
	float s, d = 0; 
	//find the square of the distance
	//from the sphere to the box
	for( long i=0 ; i<3 ; i++ ) 
	{ 

		if( C[i] < B.min[i] )
		{
			s = C[i] - B.min[i];
			d += s*s; 
		}

		else if( C[i] > B.max[i] )
		{ 
			s = C[i] - B.max[i];
			d += s*s; 
		}
	}
	return d <= r*r;
}

/*
int    n;       /// The dimension of the space.           
float*  Bmin;  // The minimum of the box for each axis.  
float*  Bmax;  // The maximum of the box for each axis. 
float  C[];     // The sphere center in n-space.        
float  r;       // The radius of the sphere.             
int    mode;    // Selects hollow or solid.              
*/
/*
 *  This routine tests for intersection between an n-dimensional             
 *  axis-aligned box and an n-dimensional sphere.  The mode argument       
 *  indicates whether the objects are to be regarded as surfaces or        
 *  solids.  The values are:                                               
 *                                                                         
 *     mode                                                                
 *                                                                         
 *       0   Hollow Box, Hollow Sphere                                     
 *       1   Hollow Box, Solid  Sphere
 *       2   Solid  Box, Hollow Sphere                                     
 *       3   Solid  Box, Solid  Sphere                                     
 *                                                                         
*/

#ifndef MAX
#define MAX(x, y)       (((x) > (y)) ? (x) : (y))
#endif
#ifndef MIN
#define MIN(x, y)       (((x) < (y)) ? (x) : (y))
#endif
#define SQR(a)((a)*(a))

bool CheckBoundingSphereAgainstBox2( float r, float* C,float* Bmin,float* Bmax)

    {
    float  a, b;
    float  dmin, dmax;
    float  r2 = SQR( r );
    int    i, face;

	int n = 3; // 3D
	int mode = 3;


    switch( mode )
        {
        case 0: /* Hollow Box - Hollow Sphere */
            dmin = 0;
            dmax = 0;
            face = FALSE;
            for( i = 0; i < n; i++ ) {
                a = SQR( C[i] - Bmin[i] );
                b = SQR( C[i] - Bmax[i] );
                dmax += MAX( a, b );
                if( C[i] < Bmin[i] ) {
                    face = TRUE;
                    dmin += a;
                    }
                else if( C[i] > Bmax[i] ) {
                    face = TRUE;
                    dmin += b;
                    }
                else if( MIN( a, b ) <= r2 ) face = TRUE;
                }
            if(face && ( dmin <= r2 ) && ( r2 <= dmax)) return(TRUE);
            break;

        case 1: /* Hollow Box - Solid Sphere */
            dmin = 0;
            face = FALSE;
            for( i = 0; i < n; i++ ) {
                if( C[i] < Bmin[i] ) {
                    face = TRUE;
                    dmin += SQR( C[i] - Bmin[i] );
                    }
                else if( C[i] > Bmax[i] ) {
                    face = TRUE;
                    dmin += SQR( C[i] - Bmax[i] );     
                    }
                else if( C[i] - Bmin[i] <= r ) face = TRUE;
                else if( Bmax[i] - C[i] <= r ) face = TRUE;
                }
            if( face && ( dmin <= r2 ) ) return( TRUE );
            break;


        case 2: /* Solid Box - Hollow Sphere */
            dmax = 0;
            dmin = 0;
            for( i = 0; i < n; i++ ) {
                a = SQR( C[i] - Bmin[i] );
                b = SQR( C[i] - Bmax[i] );
                dmax += MAX( a, b );
                if( C[i] < Bmin[i] ) dmin += a; else
                if( C[i] > Bmax[i] ) dmin += b;
                }
            if( dmin <= r2 && r2 <= dmax ) return( TRUE );
            break;

        case 3: /* Solid Box - Solid Sphere */
            dmin = 0;
            for( i = 0; i < n; i++ ) {
                if( C[i] < Bmin[i] ) dmin += SQR(C[i] - Bmin[i] ); else
                if( C[i] > Bmax[i] ) dmin += SQR( C[i] - Bmax[i] );     
                }
            if( dmin <= r2 ) return( TRUE );
            break;
  
        } /* end switch */

    return( FALSE );
} 




