//-------------------------------------------------------------------------------
// Desc: Polygon splitting routines, can be used by octree, also used to clip 
// rendering polygons for shadows and lights
//
//
//-------------------------------------------------------------------------------
#include <stdafx.h>

//----------------------------------------------------------------------------------
// Desc: Plane class
//----------------------------------------------------------------------------------
struct Plane {
   double a, b, c, d;
   Vector norm;
   Vector center;

   Plane(Vector& n, Vector& p){
	   norm = n;
	   center=p;
   		a = n.x;
		b = n.y;
		c = n.z;
		d =  -(p.x*a + p.y*b + p.z*c);
   }
}; 

//----------------------------------------------------------------------------------
// Desc: Face class used during compiler stage
//----------------------------------------------------------------------------------
struct Face
{
	Vertex v[3];
	Face(){  }
};


#define f_epsilon 0.001f
int triBoxOverlap(BBox& box, Face& face);

//-------------------------------------------------------------------------------
// Checks a triangle against a plane, with FP tolerance
//-------------------------------------------------------------------------------
bool FaceCrossesPlane(Plane& plane, Face& face, bool includeBorderFaces = false)
{
	// compute signed distances from vertices to plane
	int positive = 0, negative = 0, border = 0;

	for (int i = 0; i < 3; i++)
	{
		float dist = plane.norm.Dot(face.v[i].p) + plane.d;

		if ( dist >= f_epsilon )
			positive++;
		else if ( dist <= -f_epsilon )
			negative++;
		else
			border++;
	}

	if(includeBorderFaces && border)
		return true;

	if ( negative == 0 || positive == 0)
	{
		// all vertices on one side
		return false;
	}

	return true;
}


//-------------------------------------------------------------------------------
// Clips arbitrary polygon by plane
//-------------------------------------------------------------------------------
#define KEEP_INSIDE 1
#define KEEP_OUTSIDE 2
int ClipPolygon(Vertex* poly, Plane& plane, int FLAG)
{
	Vertex out[8];
	int n = 0;

	// For each edge
	for(int i=0;i<3;i++)
    {
		// Get the two points for this edge
		Vertex v1 = poly[i];
		Vertex v2 = poly[(i+1)%3];

		// Distances from points to plane
		float d1 = plane.norm.Dot(v1.p) + plane.d;
		float d2 = plane.norm.Dot(v2.p) + plane.d;

		// Edge is behind
		if(d1 < f_epsilon && d2 < f_epsilon) 
			continue;

		// Edge is in front
		if(d1 > -f_epsilon && d2 > -f_epsilon){ 
			out[n++] = v1;    // save first vertex
			if(d2 < f_epsilon) // Save point since it wasn't saved on 
				out[n++] = v2; //its own turn since it was considered on the backside then
							   
			continue;
		} 

		// Edge crosses polygon, so get new vertex
		Vertex newVert;
		float s = d1/(d1-d2);
		newVert.p = v1.p + s*(v2.p - v1.p);
		newVert.t = v1.t + s*(v2.t - v1.t);
		newVert.n = (v1.n + s*(v2.n - v1.n)).Normalized(); // NOTE: Can you do this interpolation for normals?


		// ASCII illustration of what's going on:
		// we're only concerned with d1 here, and any new points it connects to
		//    front <---> back
		// A) d1-----|-----d2 = store d1 and the new created point
		// B) d2 ----|-----d1 = keep newVert, as it replaces d1

		if(d1 > 0){
			out[n++] = v1;
			out[n++] = newVert;
		}
		else if(d2 > 0){
			out[n++] = newVert;
		}

	}

	for(int i=0;i<n;i++){
		poly[i] = out[i];
	}
	return n;
}


//-------------------------------------------------------------------------------
// Splits triangle by plane, handles correct winding, deconstruction to tri, etc
//
// TODO: Must generate new normals for the new triangles
//-------------------------------------------------------------------------------
vector<Face> SplitTriangle(Plane& plane, Face& triangle, BBox& box, int FLAG)
{
	vector<Face> faces; // output faces

	// Split triangle
	Vertex poly[8];

	// Fill poly with triangle data
	poly[0] = triangle.v[0];
	poly[1] = triangle.v[1];
	poly[2] = triangle.v[2];

	int verts = ClipPolygon(poly,plane,FLAG);

	if(verts == 0)
		return faces; // return nothing


	// resultant face
	if(verts == 3){
		Face face1 = triangle;

		face1.v[0] = poly[0];
		face1.v[1] = poly[1];
		face1.v[2] = poly[2];

		faces.push_back(face1);
	}



	// Might have generated a quad rather than a triangle, so construct another face
	if(verts != 3)
    {
		if(verts != 4)
        {
			SeriousWarning("Error: Clipped triangle gave a polygon with %d points",verts);
		}

		Face face1 = triangle;
		face1.v[0] = poly[1];
		face1.v[1] = poly[2];
		face1.v[2] = poly[0];

		faces.push_back(face1);

		// Special attention needed here, as winding order can get screwed up
		// Seems ok at the moment
		Face face2 = triangle;
		face2.v[0] = poly[2];
		face2.v[1] = poly[3];
		face2.v[2] = poly[0];

		faces.push_back(face2);
	}

	return faces;
}


//-------------------------------------------------------------------------------
// Splits faces by box, then puts all faces that lie inside the box in 'boxFaces'
// Remaining faces are left in 'faces'
//-------------------------------------------------------------------------------
void SplitByBox(BBox& box, vector<Face>& faces, vector<Face>& boxFaces)
{
	int iterations =0;
	Vector min = box.min;
	Vector max = box.max;
	// Box planes
	Plane p[6]=
	{ Plane(Vector(0,1,0),min), Plane(Vector(1,0,0),min),Plane(Vector(0,0,1),min), // Min sides
	  Plane(Vector(0,-1,0),max), Plane(Vector(-1,0,0),max),Plane(Vector(0,0,-1),max), // Max sides
	};

	// Inverted box planes, for finding outside triangles
	Plane pR[6]=
	{ Plane(Vector(0,-1,0),min), Plane(Vector(-1,0,0),min),Plane(Vector(0,0,-1),min), // Min sides
	  Plane(Vector(0,1,0),max), Plane(Vector(1,0,0),max),Plane(Vector(0,0,1),max), // Max sides
	};
	

	// We expand the checking box slightly to cover epsilon bordering cases
	BBox checkBox = box;
	checkBox.min -= Vector(0.01f,0.01f,0.01f);
	checkBox.max += Vector(0.01f,0.01f,0.01f);

	for(int i=0;i<faces.size();i++)
    {
		// This is a very overestimating rough test
		// To stop us splitting more triangles than 100% needed
		if(!triBoxOverlap(checkBox,faces[i]))
			continue;


		// See if this face needs slicing up before going in the box
		for(int j=0;j<6;j++){
			if(FaceCrossesPlane(p[j],faces[i])){
				// Slice triangle by this plane
				vector<Face> frontfaces = SplitTriangle(p[j],faces[i],box,KEEP_OUTSIDE);

				// And the other side of this plane
				vector<Face> backfaces = SplitTriangle(pR[j],faces[i],box,KEEP_INSIDE);

				if(!backfaces.size() || !frontfaces.size()){
					cout << "Not a real split" << endl;
				}

				// Add new faces

					
				// !!!!!!!!!!! ERROR CHECKING CODE !!!!!!!!!!!!!!!!!!!!!
			/*	bool skip=false; // don't use this new split triangle
				for(int k=0;k<frontfaces.size();k++){
					if(FaceCrossesPlane(p[j],frontfaces[k].V))
						cout << "ERROR -- NEW FACES NOT SPLIT PROPERLY!!!!!!" << endl;

					if(IsDegenerate(frontfaces[k])){
						cout << "degen";
						skip = true;
					}
				}

				// Faces outside the box
				for(int k=0;k<backfaces.size();k++){
					if(FaceCrossesPlane(p[j],backfaces[k].V))
						cout << "ERROR -- NEW FACES NOT SPLIT PROPERLY!!!!!!" << endl;

					if(IsDegenerate(backfaces[k])){
						cout << "degen";
						skip = true;
					}
				}
				//if(skip)
				//	continue;
			*/

				// !!!!!!!!!!! END ERROR CHECKING CODE !!!!!!!!!!!!!!!!!!!!!


				// Faces inside the box
				for(int k=0;k<frontfaces.size();k++){
					frontfaces[k].user1 = 1;
					frontfaces[k].user2 = faces[i].user2; 

					faces.push_back(frontfaces[k]);
				}

				// Faces outside the box
				for(int k=0;k<backfaces.size();k++){
					backfaces[k].user1 = 2;
					backfaces[k].user2 = faces[i].user2; 

					faces.push_back(backfaces[k]);
				}

				break;
			}
		}
		iterations++;
		
		if(j == 6){
			// It's been sliced. So figure out where it is
			// Logic:
			// Inside if: ANY points inside
			// Outside if: ANY points outside
			// If all points bordering: do triBoxCheck, if in, INSIDE, if not, ERROR

			int b1 = Inside(box,faces[i].v[0].p);
			int b2 = Inside(box,faces[i].v[1].p);
			int b3 = Inside(box,faces[i].v[2].p);

			if((b1 == 1 || b2 == 1 || b3 == 1) && (b1 == 0 || b2 == 0 || b3 == 0)){
				SeriousWarning("ERRROR - SHOULD HAVE BEEN SPLIT PROPERLY - PART IS STILL IN AND OUT THOUGH");
				
				// Add to both for testing
				boxFaces.push_back(faces[i]);
				continue;
			}

			// ANY points inside
			if(b1 == 1 || b2 == 1 || b3 == 1){
				boxFaces.push_back(faces[i]);
				faces.erase(faces.begin() + i);
				i--; // put us on the right face
				continue;
			}

			// ANY points outside
			if(b1 == 0 || b2 == 0 || b3 == 0)
				continue; // Do nothing

			if(!(b1 == -1 && b2==-1 && b3 == -1))
				SeriousWarning("IMPOSSIBLE - NOT INSIDE, OUTSIDE, OR FULLY BORDERING!!!");

			// All points bordering..
			BBox abox = box;
			abox.min -= Vector(0.001f,0.001f,0.001f);
			abox.max += Vector(0.001f,0.001f,0.001f);

			if(triBoxOverlap(box,faces[i])){
				boxFaces.push_back(faces[i]);
				faces.erase(faces.begin() + i);
				i--; // put us on the right face
			}
			else
				SeriousWarning("ERROROR all points bordering and tri is outside - tri must be degenerate!");


			// else stays on the outside face list
		}
		else{
			// If we've split the face then we want to disregard it now
			// The new faces are further down the array and will be automatically processed

			faces.erase(faces.begin() + i);
			i--; // put us on the right face
			continue;
		}
	}
}




/********************************************************/
/* AABB-triangle overlap test code                      */
/* by Tomas Akenine-Möller                              */
/* Function: int triBoxOverlap(float boxcenter[3],      */
/*          float boxhalfsize[3],float triverts[3][3]); */
/* History:                                             */
/*   2001-03-05: released the code in its first version */
/*   2001-06-18: changed the order of the tests, faster */
/*                                                      */
/* Acknowledgement: Many thanks to Pierre Terdiman for  */
/* suggestions and discussions on how to optimize code. */
/* Thanks to David Hunt for finding a ">="-bug!         */
/********************************************************/
#include <math.h>
#include <stdio.h>

#define X 0
#define Y 1
#define Z 2

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

int planeBoxOverlap(float normal[3],float d, float maxbox[3])
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
#define AXISTEST_X01(a, b, fa, fb)             \
    p0 = a*v0[Y] - b*v0[Z];                    \
    p2 = a*v2[Y] - b*v2[Z];                    \
        if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} \
    rad = fa * boxhalfsize[Y] + fb * boxhalfsize[Z];   \
    if(min>rad || max<-rad) return 0;

#define AXISTEST_X2(a, b, fa, fb)              \
    p0 = a*v0[Y] - b*v0[Z];                    \
    p1 = a*v1[Y] - b*v1[Z];                    \
        if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
    rad = fa * boxhalfsize[Y] + fb * boxhalfsize[Z];   \
    if(min>rad || max<-rad) return 0;

/*======================== Y-tests ========================*/
#define AXISTEST_Y02(a, b, fa, fb)             \
    p0 = -a*v0[X] + b*v0[Z];                   \
    p2 = -a*v2[X] + b*v2[Z];                       \
        if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} \
    rad = fa * boxhalfsize[X] + fb * boxhalfsize[Z];   \
    if(min>rad || max<-rad) return 0;

#define AXISTEST_Y1(a, b, fa, fb)              \
    p0 = -a*v0[X] + b*v0[Z];                   \
    p1 = -a*v1[X] + b*v1[Z];                       \
        if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
    rad = fa * boxhalfsize[X] + fb * boxhalfsize[Z];   \
    if(min>rad || max<-rad) return 0;

/*======================== Z-tests ========================*/

#define AXISTEST_Z12(a, b, fa, fb)             \
    p1 = a*v1[X] - b*v1[Y];                    \
    p2 = a*v2[X] - b*v2[Y];                    \
        if(p2<p1) {min=p2; max=p1;} else {min=p1; max=p2;} \
    rad = fa * boxhalfsize[X] + fb * boxhalfsize[Y];   \
    if(min>rad || max<-rad) return 0;

#define AXISTEST_Z0(a, b, fa, fb)              \
    p0 = a*v0[X] - b*v0[Y];                \
    p1 = a*v1[X] - b*v1[Y];                    \
        if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
    rad = fa * boxhalfsize[X] + fb * boxhalfsize[Y];   \
    if(min>rad || max<-rad) return 0;

int triBoxOverlap(BBox& box, Face& face)
{
	Vector center = (box.max+box.min)*.5f;
	Vector half = (box.max-box.min)*.5f;

	float* boxcenter = (float*)&center;
	float* boxhalfsize = (float*)&half;

	float* triverts[3];
	triverts[0] = (float*)&face.v[0].p;
	triverts[1] = (float*)&face.v[1].p;
	triverts[2] = (float*)&face.v[2].p;

  /*    use separating axis theorem to test overlap between triangle and box */
  /*    need to test for overlap in these directions: */
  /*    1) the {x,y,z}-directions (actually, since we use the AABB of the triangle */
  /*       we do not even need to test these) */
  /*    2) normal of the triangle */
  /*    3) crossproduct(edge from tri, {x,y,z}-directin) */
  /*       this gives 3x3=9 more tests */
   float v0[3],v1[3],v2[3];
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
   if(!planeBoxOverlap(normal,d,boxhalfsize)) return 0;

   return 1;   /* box and triangle overlaps */
}

