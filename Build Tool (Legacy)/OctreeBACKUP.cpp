#include <stdafx.h>
#include "ASEImport.h"
#include "Octree.h"

int NODE_SIZE = 650;



void Sort(Vector& bmin, Vector& bmax){
	Vector pmin, pmax;
	pmin.x = min(bmin.x,bmax.x);
	pmin.y = min(bmin.y,bmax.y);
	pmin.z = min(bmin.z,bmax.z);

	pmax.x = max(bmin.x,bmax.x);
	pmax.y = max(bmin.y,bmax.y);
	pmax.z = max(bmin.z,bmax.z);
	bmin = pmin;
	bmax = pmax;
}

/*
void FillBoxes_ADD(){
	// Build info used for the triboxoverlap tests below
	Point3 boxCenter[4];
	Point3 boxHalf[4];
	for(j=0;j<4;j++){
		boxCenter[j] = (quadBox[j].min + quadBox[j].max)*.5f;
		boxHalf[j] = (quadBox[j].max - quadBox[j].min)*.5f + Point3(0.1f,0.1f,0.1f);
	}

	// Vertex/index lists for each box
	vector<vertex> vertListQ[4];
	vector<WORD> indicesQ[4];


	// Number of vertices/indices for each of the four boxes
	int numVertsQ[]={0,0,0,0};
	int numIndsQ[]={0,0,0,0};

	

	// Take a guess at the size.
	// This is a gross overestimate, just to cover all possibilities
	for(j=0;j<4;j++){
		vertListQ[j].resize(data->numVerts);
		indicesQ[j].resize(data->numInds);
	}


	Point3 testCenter = (box.min + box.max)*.5f;
	Point3 testHalf = (box.max - box.min)*.5f + Point3(0.1f,0.1f,0.1f);

	for(int i=0;i<data->numInds;i+=3){

		vertex v[]={data->vertices[data->indices[(i)+0]],data->vertices[data->indices[(i)+1]],data->vertices[data->indices[(i)+2]]};

		float *tri[]={v[0].v,v[1].v,v[2].v};

		BBox triBox;
		triBox.Init();
		triBox += v[0].v;
		triBox += v[1].v;
		triBox += v[2].v;

		// Foreach box, does it contain the face?
		for(j=0;j<4;j++){

			if(Touches(quadBox[j],triBox)){

				// Add each vertex
				for(int n=0;n<3;n++){
					// Add this vertex to our pool only if it hasn't already been added
					vector< vertex >::iterator result = find( vertListQ[j].begin(), vertListQ[j].end(),v[n]);
					WORD foundAt = result - vertListQ[j].begin();
					if (result == vertListQ[j].end())
					{
						foundAt = numVertsQ[j];

						if(numVertsQ[j]>vertListQ[j].size())
							MessageBox(0,"Jaffa, Creed!",0,0);

						vertListQ[j][numVertsQ[j]] = v[n];

						numVertsQ[j]++;
					}

					// Add the index regardless
					indicesQ[j][numIndsQ[j]++] = foundAt;
				}
			}
		}
	}
}*/


enum TouchType{
	BOX_NOTOUCH,
	BOX_CONTAINED,
	BOX_INTERSECTS,
};

TouchType Touches(BBox& b1, BBox& b2){
	// b1 contains b2
	if((b2.min.x >= b1.min.x && b2.max.x <= b1.max.x &&
		b2.min.y >= b1.min.y && b2.max.y <= b1.max.y &&
		b2.min.z >= b1.min.z && b2.max.z <= b1.max.z)||
		// b2 contains b1
	   (b1.min.x >= b2.min.x && b1.max.x <=b2.max.x &&
		b1.min.y >= b2.min.y && b1.max.y <= b2.max.y &&
		b1.min.z >= b2.min.z && b1.max.z <= b2.max.z))
		return BOX_CONTAINED;

		// they overlap
	if (b1.max.x>b2.min.x && b1.min.x<b2.max.x &&
		b1.max.y>b2.min.y && b1.min.y<b2.max.y &&
		b1.max.z>b2.min.z && b1.min.z<b2.max.z)
		return BOX_INTERSECTS;

	return BOX_NOTOUCH;
}

/* Here's how a vertex structure looks: */
struct VERTEX
{
    float x, y, z; // its position, xyz in 3D, xy in 2D
    float u, v, c; // u and v are coordinates of the source texture
      // corresponding to the vertex. C is the color of the vertex,
      // for gouraud shading (covered later)
}; 

typedef struct Polygon
{
   VERTEX c[16];   // up to 16 vertices!
   int count;             // w is world points, c is for camera points
} POLYGON; 



typedef struct Plane {
   float a, b, c, d;
} PLANE; 

/*
Okay, let's build a function that takes a polygon and a plane, and clips the polygon to the plane. The polygon's vertex data is assumed to be in the c[] array and the function will output the new vertex data to the c[] array. Brace yourself, this is a bit long: 
*/
int ClipPolygon(POLYGON *poly, PLANE *plane) 
{ 
   /* Plan: cycle through the vertices, considering pairs of them.
      If both vertices are visible, add them to the new array.
      If both vertices are invisible, add neither to the new array.
      If one vertex is visible and the other is not, move the one
      that's not, and then add both vertices to the new array.
    */
   float dist1, dist2; // distances of points to plane 
   float distratio; // fraction of distance between two points 
   static VERTEX tempvtx[10];
     // new vertices might be created. Don't change 
     // polygon's vertices because we might still need 
     // them. Instead, use tempvtx array and update vertices 
     // array at the end. Create tempvtx array once only.
   int i, ii, j=0;    

   /* Check if plane is a valid plane */ 
   if (!(plane->a || plane->b || plane->c)) return -1; 
   // if not valid plane, don't change polygon and return an error; 

   /* The vertices should, as for all functions, be arranged in cyclic 
   order. That is, if a line was drawn from each vertex to the next 
   it would form the correct outline of the polygon in 3D space. 
   This routine might create new vertices because of the clipping, 
   but the cyclic order will be preserved. 
   */ 

   for (i=0; i<poly->count; i++) 
   { 
      ii = (i+1)%poly->count;
      dist1 = plane->a * poly->c[i].x + plane->b * poly->c[i].y + 
      plane->c * poly->c[i].z + plane->d; 
      dist2 = plane->a * poly->c[ii].x + plane->b * poly->c[ii].y + 
      plane->c * poly->c[ii].z + plane->d; 
      if (dist1<0 && dist2<0) // line unclipped and invisible 
         continue;
      if (dist1>0 && dist2>0) // line unclipped and visible
         tempvtx[j++]=poly->c[i];
      else // line partially visible 
      if (dist1>0) // first vertex is visible 
      { 
         distratio = dist1/(dist1-dist2); 
         tempvtx[j] = poly->c[i];
         j++; // Copied 1st vertex
         tempvtx[j].x = poly->c[i].x + 
            (poly->c[ii].x - poly->c[i].x) * distratio; 
         tempvtx[j].y = poly->c[i].y + 
            (poly->c[ii].y - poly->c[i].y) * distratio; 
         tempvtx[j].z = poly->c[i].z + 
            (poly->c[ii].z - poly->c[i].z) * distratio; 
         tempvtx[j].u = poly->c[i].u + 
            (poly->c[ii].u - poly->c[i].u) * distratio; 
         tempvtx[j].v = poly->c[i].v + 
            (poly->c[ii].v - poly->c[i].v) * distratio; 
         tempvtx[j].c = poly->c[ii].c; 
         j++; // Copied second vertex
      } 
      else // second vertex is visible 
      { 
         distratio = dist2/(dist2-dist1); 
         tempvtx[j].c = poly->c[i].c;
         tempvtx[j].x = poly->c[ii].x + 
            (poly->c[i].x - poly->c[ii].x) * distratio; 
         tempvtx[j].y = poly->c[ii].y + 
            (poly->c[i].y - poly->c[ii].y) * distratio; 
         tempvtx[j].z = poly->c[ii].z + 
            (poly->c[i].z - poly->c[ii].z) * distratio; 
         tempvtx[j].u = poly->c[ii].u + 
            (poly->c[i].u - poly->c[ii].u) * distratio; 
         tempvtx[j].v = poly->c[ii].v + 
            (poly->c[i].v - poly->c[ii].v) * distratio; 
         j++; // Copy only first vertex. 2nd vertex will be copied
              // in next iteration of loop
      }
   } 

   for (i=0; i<j; i++) 
      poly->c[i] = tempvtx[i]; // Update the vertices in polygon
   poly->count = j;            // Update the vertex count
   return j; 
} 


vector<Face> SplitTriangle(PLANE& plane, Face& triangle){
	vector<Face> faces; // output faces

	// Split triangle
	POLYGON p;
	p.count = 3;
	
	// Fill poly with triangle data
	p.c[0].x = triangle.A.x;
	p.c[0].y = triangle.A.y;
	p.c[0].z = triangle.A.z;
	p.c[0].u = triangle.TA.x;
	p.c[0].v = triangle.TA.y;

	p.c[1].x = triangle.B.x;
	p.c[1].y = triangle.B.y;
	p.c[1].z = triangle.B.z;
	p.c[1].u = triangle.TB.x;
	p.c[1].v = triangle.TB.y;

	p.c[2].x = triangle.C.x;
	p.c[2].y = triangle.C.y;
	p.c[2].z = triangle.C.z;
	p.c[2].u = triangle.TC.x;
	p.c[2].v = triangle.TC.y;

	ClipPolygon(&p,&plane);

	if(p.count == 0)
		return faces;

	// resultant face
	if(p.count == 3){
		Face face1;
		memcpy(&face1.A,&p.c[0].x,sizeof(float)*3);
		memcpy(&face1.B,&p.c[1].x,sizeof(float)*3);
		memcpy(&face1.C,&p.c[2].x,sizeof(float)*3);

		memcpy(&face1.TA,&p.c[0].u,sizeof(float)*2);
		memcpy(&face1.TB,&p.c[1].u,sizeof(float)*2);
		memcpy(&face1.TC,&p.c[2].u,sizeof(float)*2);
		face1.matID = triangle.matID;
		face1.submatID = triangle.submatID;

		faces.push_back(face1);
	}


	// Might have generated a quad rather than a triangle, so construct another face
	if(p.count != 3){
		if(p.count != 4)
			cout << "Error: Clipped triangle gave a polygon with "<<p.count<<" points!"<<endl;

		Face face1;
		memcpy(&face1.A,&p.c[1].x,sizeof(float)*3);
		memcpy(&face1.B,&p.c[2].x,sizeof(float)*3);
		memcpy(&face1.C,&p.c[0].x,sizeof(float)*3);

		memcpy(&face1.TA,&p.c[1].u,sizeof(float)*2);
		memcpy(&face1.TB,&p.c[2].u,sizeof(float)*2);
		memcpy(&face1.TC,&p.c[0].u,sizeof(float)*2);
		face1.matID = triangle.matID;
		face1.submatID = triangle.submatID;

		faces.push_back(face1);

		// FIXME: Winding order may be wrong
		Face face2;
		memcpy(&face2.A,&p.c[2].x,sizeof(float)*3);
		memcpy(&face2.B,&p.c[3].x,sizeof(float)*3);
		memcpy(&face2.C,&p.c[0].x,sizeof(float)*3);

		memcpy(&face2.TA,&p.c[2].u,sizeof(float)*2);
		memcpy(&face2.TB,&p.c[3].u,sizeof(float)*2);
		memcpy(&face2.TC,&p.c[0].u,sizeof(float)*2);

		// Copy normals
		// FIXME: Generate new normals
		face2.NA = face1.NA;
		face2.NB = face1.NB;
		face2.NC = face1.NC;

		face2.matID = triangle.matID;
		face2.submatID = triangle.submatID;

		faces.push_back(face2);

	}

	return faces;
}

#define equal(a,b) (fabsf(a-b) < 0.001f)

int ClassifyPoint( PLANE& plane, Vector& destPt )
{
	float p = (plane.a*destPt.x + plane.b*destPt.y + plane.c*destPt.z) + plane.d;

	if(equal(p,0)) return -1; // it's on the plane

	if( p > 0.0f ) return 0; // front
	else if( p < 0.0f ) return 1; // back
}

bool TriTouchesPlane(PLANE& plane, Vector& a, Vector& b, Vector& c){
	int val =  ClassifyPoint(plane,a);
	int val1 = ClassifyPoint(plane,b);
	int val2 = ClassifyPoint(plane,c);

	// If any points sit ON the plane then ignore them, and assume they are on whatever side the rest
	// of the triangle is
	{
		if(val == -1){
			if(val1 != -1)
				val = val1;
			else if(val2 != -1)
				val = val2;
			else return false; // all points sit on the plane
		}

		if(val1 == -1){
			if(val != -1)
				val1 = val;
			else if(val2 != -1)
				val1 = val2;
			else return false; // all points sit on the plane
		}

		if(val2 == -1){
			if(val != -1)
				val2 = val;
			else if(val1 != -1)
				val2 = val1;
			else return false; // all points sit on the plane
		}
	}

	if((val2==val1) && (val1==val))
		return false;

	return true;
}


// Damn precision
#define above(a,b) ((a-b) > -0.001f)
#define below(a,b) ((a-b) < 0.001f)


int Inside(BBox& b, Vector& p){
	if(above(p.x,b.min.x) && below(p.x,b.max.x) &&
		above(p.y,b.min.y) && below(p.y,b.max.y) &&
		above(p.z,b.min.z) && below(p.z,b.max.z)){

		if(equal(p.x,b.min.x) || equal(p.x,b.max.x) ||
		equal(p.y,b.min.y) || equal(p.y,b.max.y) ||
		equal(p.z,b.min.z) ||equal(p.z,b.max.z))
		return -1; // it's on the border

		else return 1;
		}
	return 0;
}
long splits = 0;
void FillBoxes_SPLIT(Vector& center,vector<Face>& faces, vector<BBox>& boxes,vector<vector<Face> >& facesOut){
	int nodes = boxes.size(); // 8 for octree, 4 for quadtree

			
	// Tree planes
	PLANE p[2];
	p[0].a = 1; //x
	p[0].b = 0; //y
	p[0].c = 0; //z
	p[0].d = - (center.x*p[0].a+center.y*p[0].b+center.z*p[0].c);

	p[1].a = 0; //x
	p[1].b = 0; //y
	p[1].c = 1; //z
	p[1].d = - (center.x*p[1].a+center.y*p[1].b+center.z*p[1].c);

	PLANE pR[2];
	pR[0].a = -1; //x
	pR[0].b = 0; //y
	pR[0].c = 0; //z
	pR[0].d = - (center.x*pR[0].a+center.y*pR[0].b+center.z*pR[0].c);

	pR[1].a = 0; //x
	pR[1].b = 0; //y
	pR[1].c = -1; //z
	pR[1].d = - (center.x*pR[1].a+center.y*pR[1].b+center.z*pR[1].c);
	
	// 1. Split all boundary triangles by the tree planes (3 for octree, 2 for quadtree)
	// 2. Classify triangles into tree nodes. _ALL_ cases should be BOX_CONTAINS as none overlap edges any more
	for(int i=0;i<faces.size();i++){

		// 1. Split triangle by tree planes
		bool split = false;
		for(int j=0;j<2;j++){
			if(TriTouchesPlane(p[j],faces[i].A,faces[i].B,faces[i].C)){
				// Slice triangle by this plane
				vector<Face> splitfaces = SplitTriangle(p[j],faces[i]);

				// And the other side of this plane
				vector<Face> splitfaces2 = SplitTriangle(pR[j],faces[i]);


				// Add new faces

				// in front of plane
				for(int k=0;k<splitfaces.size();k++){
					if(j == 1) // |
						splitfaces[k].user1 = 1;
					else // --
						splitfaces[k].user2 = 1;
					faces.push_back(splitfaces[k]);
				}

				// behind plane
				for(int k=0;k<splitfaces2.size();k++){
					if(j == 1) // |
						splitfaces2[k].user1 = 2;
					else // --
						splitfaces2[k].user2 = 2;
					faces.push_back(splitfaces2[k]);
				}

#ifdef _DEBUG
				// Assert that none of the new triangles touch this plane
				for(int k=0;k<splitfaces.size();k++){
					if(TriTouchesPlane(p[j],splitfaces[k].A,splitfaces[k].B,splitfaces[k].C)){
						split=split;
						TriTouchesPlane(p[j],splitfaces2[k].A,splitfaces2[k].B,splitfaces2[k].C);
					}
				}

				// Assert that none of the new triangles touch this plane
				for(int k=0;k<splitfaces2.size();k++){
					if(TriTouchesPlane(p[j],splitfaces2[k].A,splitfaces2[k].B,splitfaces2[k].C)){
						split=split;
						TriTouchesPlane(p[j],splitfaces2[k].A,splitfaces2[k].B,splitfaces2[k].C);
					}
				}
#endif
				j = 1000; // don't want to slice the old face with the next plane

				split = true;
				splits++;
			}
		}

		// If we've split the face then we want to disregard it now
		// The new faces are further down the array and will be automatically processed
		if(split){

			// Erase old face
			faces.erase(faces.begin() + i);
			i--; // put us on the right face

			continue;
		}

		if(i == 178)
			i=i;


		// Which box is this triangle in?
		bool foundbox = false;
		for(int j=0;j<nodes;j++){

			//TouchType type = Touches(boxes[j],triBox);

			// We can do a simple point-in-box test because all triangles would have been clipped
			// by this box, so a triangle spanning the box is impossible
			int p1 = Inside(boxes[j],faces[i].A);
			int p2 = Inside(boxes[j],faces[i].B);
			int p3 = Inside(boxes[j],faces[i].C);


			// eliminate and borderline triangles that we know are outside
			if(p1 == -1 || p2 == -1 || p3 == -1){
				// check for positives first:
				if(faces[i].user1 == 2 && faces[i].user2 == 2 && j == 2 ){ // left/bottom
					foundbox = true;
				facesOut[j].push_back(faces[i]);
				break;
				}

				if(faces[i].user1 == 1 && faces[i].user2 == 2 && j == 3 ){ // right/bottom
					foundbox = true;
				facesOut[j].push_back(faces[i]);
				break;
				}

				if(faces[i].user1 == 1 && faces[i].user2 == 1 && j == 1 ){ // right/top
					foundbox = true;
				facesOut[j].push_back(faces[i]);
				break;
				}

				if(faces[i].user1 == 2 && faces[i].user2 == 1 && j == 0 ){ // left/top
					foundbox = true;
				facesOut[j].push_back(faces[i]);
				break;
				}



				if(faces[i].user1 == 1 && (j != 1 && j != 3) ) // right of |
					continue;

				if(faces[i].user1 == 2 && (j != 0 && j != 2) ) // left of |
					continue;

				if(faces[i].user2 == 1 && (j != 2 && j != 3) ) // right of --
					continue;

				if(faces[i].user2 == 2 && (j != 0 && j != 1) ) // left of --
					continue;

				if(faces[i].user1 == 2 && faces[i].user2 == 2 && j != 2 ) // left/bottom
					continue;

				if(faces[i].user1 == 1 && faces[i].user2 == 2 && j != 3 ) // right/bottom
					continue;

				if(faces[i].user1 == 1 && faces[i].user2 == 1 && j != 1 ) // right/top
					continue;

				if(faces[i].user1 == 2 && faces[i].user2 == 1 && j != 0 ) // left/top
					continue;
			}

			if(p1 != -1)
				p1 = p1;
			else if(p2 != -1)
				p1 = p2;
			else if(p3 != -1)
				p1 = p3;


			bool inside = p1 != 0;

			if(!inside)
				continue;
			
			if(foundbox)
				cout << "Error - clipped triangle added to more than one box!!" << endl;

			// Add triangle to this box
			foundbox = true;
			facesOut[j].push_back(faces[i]);
		}

		if(!foundbox)
			cout << "Error, box not found for triangle" << endl;

	}

}

#define FACE_THRESHOLD 1

// Simple heuristic for measuring box size
long Size(BBox& b){
	Vector size = b.max - b.min;
	return size.x;
}

// Only end nodes are filled with faces
bool QuadSplit(Node* aNode, vector<Face>& faces, int depth){

	depth++;

	// Terminating critera reached.
	// Create end node and return
	if(Size(aNode->box) < NODE_SIZE){
		aNode->numChildren = 0;
		aNode->faces = faces;
		return false;
	}


	BBox box = aNode->box;

	// Create the 4 new bounding boxes
	Vector midPoint = (box.max + box.min) *0.5f;
	midPoint.y = box.max.y; // Remove Y axis, or it would be an octree

	vector<BBox> boxes(4);

	// Default values
	for(int j=0;j<4;j++){
		boxes[j].min = box.min;
		boxes[j].max = midPoint;
	}

	// quad tree looks like this from top: X is ---->
	// |1|2|
	// |3|4|

	// First quadrant (all default values, so nothing to do)

	// Second quadrant (X max)
	boxes[1].min.x = box.max.x;

	// Third quadrant (Z max)
	boxes[2].min.z = box.max.z;

	// Fourth (Z and X max)
	boxes[3].min = Vector(box.max.x,box.min.y,box.max.z);

	// Sort their max/mins for good measure
	for(j=0;j<4;j++){
		Sort(boxes[j].min,boxes[j].max);
	}


	// We now have four new boxes
	// Loop through the faces and fill each box
	// If two boxes contain a face, add it to both
	//FillBoxes_ADD();
	vector<vector<Face> > boxfaces(4);
	FillBoxes_SPLIT(midPoint,faces,boxes,boxfaces);

	// If our boxes didn't do any good, just output what we have
	/*if(boxfaces[0].size() == faces.size()||boxfaces[1].size() == faces.size()||
		boxfaces[2].size() == faces.size()||boxfaces[3].size() == faces.size()||
		boxfaces[0].size() > faces.size()*0.8f || boxfaces[1].size() > faces.size()*0.8f||
		boxfaces[2].size() > faces.size()*0.8f|| boxfaces[3].size() > faces.size()*0.8f){
		// Terminating critera reached.
		// Create end node

		aNode->numChildren = 0;
		aNode->faces = faces;

		return false;
	}*/


	// We now have our boxes, add them to our children list then recurse
	for(j=0;j<4;j++){
		// Node is empty
		if(boxfaces[j].size() == 0)
			continue;

		aNode->children[aNode->numChildren] = new Node;
		aNode->children[aNode->numChildren]->box = boxes[j];

		// Pass node and data
		// Node will either fill itself with data and terminate
		// or spawn children
		QuadSplit(aNode->children[aNode->numChildren],boxfaces[j],depth);

		aNode->numChildren++;
	}

	return true;
}
