//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
/// Axis-aligned bounding box (AABB) class
///
/// Author: Tim Johnson
//====================================================================================
#ifndef BBOX_H
#define BBOX_H

#undef largest
#define largest(a,b)            (((a) > (b)) ? (a) : (b))

#undef smallest
#define smallest(a,b)            (((a) < (b)) ? (a) : (b))


/// Axis-aligned bounding box (AABB) class
class BBox{
private:
public:
	Vector min,max;

	static int facevert[6][4];
	static int edgevert[12][2];
	static int edgefaces[12][2];
	static float vertnorm[8][3];
	static float edgenorm[12][3];
	static float facenorm[6][3];


	BBox(Vector& bMin, Vector& bMax){ min = bMin; max = bMax; }
	BBox() { min = Vector(BIG_NUMBER,BIG_NUMBER,BIG_NUMBER); max = Vector(-BIG_NUMBER,-BIG_NUMBER,-BIG_NUMBER); }

	void Reset(){ min = Vector(BIG_NUMBER,BIG_NUMBER,BIG_NUMBER); max = Vector(-BIG_NUMBER,-BIG_NUMBER,-BIG_NUMBER); }
	
	/// Grows box to fit point
	void operator +=(const Vector& point){
		min.x = smallest(point.x,min.x);
		min.y = smallest(point.y,min.y);
		min.z = smallest(point.z,min.z);

		max.x = largest(point.x,max.x);
		max.y = largest(point.y,max.y);
		max.z = largest(point.z,max.z);
	}

	/// Grows box to fit box
	void operator +=(const BBox& boxIn)
	{
		min.x = smallest(min.x,boxIn.min.x);
		min.y = smallest(min.y,boxIn.min.y);
		min.z = smallest(min.z,boxIn.min.z);

		max.x = largest(max.x,boxIn.max.x);
		max.y = largest(max.y,boxIn.max.y);
		max.z = largest(max.z,boxIn.max.z);
	}

	bool operator==(const BBox& other){
		return other.min == min && other.max == max;
	}

	void operator *=(const int mul){
		min *= mul;
		max *= mul;
	}

	/// Overlap test
	bool Touches(BBox& other){
		if (max.x>=other.min.x && min.x<=other.max.x &&
				max.y>=other.min.y && min.y<=other.max.y &&
				max.z>=other.min.z && min.z<=other.max.z)
				return 1;
		return 0;
	}


	/// Corner vertices
	Vector GetVert(int ind)
	{
		switch(ind)
		{
		case 0: return min;
		case 1: return max;
		case 2: return Vector(max.x,min.y,min.z);
		case 3: return Vector(min.x,max.y,max.z);
		case 4: return Vector(max.x,max.y,min.z);
		case 5: return Vector(min.x,min.y,max.z);
		case 6: return Vector(min.x,max.y,min.z);
		case 7: return Vector(max.x,min.y,max.z);
		default: return Vector(0,0,0);
		}
	};

	/// Box transformed by matrix
	BBox Transformed(const Matrix& mtx)
	{
		BBox newBox;

		for(int i=0;i<8;i++){
			newBox += mtx*GetVert(i);
		}

		return newBox;
	}

	//-----------------------------------------------------------------------------
	/// returns true if the point is in the box
	//-----------------------------------------------------------------------------
	bool IsPointInBox( const Vector& pt)
	{
		//assert( min[0] < max[0] );
		//assert( min[1] < max[1] );
		//assert( min[2] < max[2] );

		if ( (pt[0] > max[0]) || (pt[0] < min[0]) )
			return false;
		if ( (pt[1] > max[1]) || (pt[1] < min[1]) )
			return false;
		if ( (pt[2] > max[2]) || (pt[2] < min[2]) )
			return false;
		return true;
	}


};



#endif //BBOX_H