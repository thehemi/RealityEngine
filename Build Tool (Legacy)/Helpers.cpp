//-----------------------------------------------------------------------------------
// Useful helper functions
//-----------------------------------------------------------------------------------
#include <stdafx.h>


// Sort bmin and bmax values
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

// Damn precision
#define above(a,b) ((a-b) > -0.001f)
#define below(a,b) ((a-b) < 0.001f)
#define equal(a,b) (fabsf(a-b) < 0.001f)

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

int CountFaces(Node& node){
	int total = 0;
	for(int i=0;i<node.numChildren;i++)
		total += CountFaces(*node.children[i]);
	total+= node.faces.size();
	return total;
}


// From:
//
// xx xy xz
// yx yy yz
// zx zy zz
//
// To:
//
// xx xz xy
// zx zz zy
// yx yz yy
//
// that is, go from the MAX right-handed Z-up to our left-handed Y-up system
void ConvertMaxMatrix(Matrix& MaxMatrix)
{
	float temp;

	temp=MaxMatrix[0][1];
	MaxMatrix[0][1]=MaxMatrix[0][2];
	MaxMatrix[0][2]=temp;

	temp=MaxMatrix[1][0];
	MaxMatrix[1][0]=MaxMatrix[2][0];
	MaxMatrix[2][0]=temp;

	temp=MaxMatrix[1][1];
	MaxMatrix[1][1]=MaxMatrix[2][2];
	MaxMatrix[2][2]=temp;

	temp=MaxMatrix[1][2];
	MaxMatrix[1][2]=MaxMatrix[2][1];
	MaxMatrix[2][1]=temp;

	temp=MaxMatrix[3][1];
	MaxMatrix[3][1]=MaxMatrix[3][2];
	MaxMatrix[3][2]=temp;
}


// Converts a 3dsmax point to d3d space
Vector AsD3DPoint(Vector& p){
	Vector r;
	r.y = p.z;
	r.z = p.y;
	r.x = p.x;
	return r;
}