//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
///
/// Author: Tim Johnson
/// Author: Tim Johnson
/// 4/22/98/Derek Nedelman & Timothy Johnson: Created file from MATH.H
///	5/22/2000/Timothy Johnson: Modified and cleaned up file for new engine
///	5/30/2000/TJ: Added friend operator(s)
///	6/1/2000/TJ: > < Length checks are now free of sqrt
///	6/2/2000/TJ: Fixed quite a few misc things
///	6/20/2000/DN: Removed a few access functions, added a few consts, etc
/// 12/12/2002/TJ: Removed pass-throughs for += etc.
//====================================================================================
#ifndef TD_VECTOR_INCLUDED
#define TD_VECTOR_INCLUDED


//Includes---------------------------------------------------------------------
#include <cmath>

/// Constants.
#undef  PI
#define	PI					3.1415926535897932384626433832795028841971693993751f	//!< PI
#define SMALL_NUMBER		(1.e-8f)
#define KINDA_SMALL_NUMBER	(1.e-4f)
#define BIG_NUMBER		    (1.0e+10f)

/// Simple Vector2 class
class Vector2
{
public:
	float x, y;

	Vector2() :	x(0), y(0)
	{}

	Vector2( float InX, float InY )
	:	x(InX), y(InY)
	{}
	float& operator [] (int i)				{return (reinterpret_cast<float*>(this))[i];}
	float operator [] (int i) const			{return (reinterpret_cast<const float*>(this))[i];}
	float Average() const						{return (x + y) * 0.5f;}	
	void operator() (float nx, float ny)	{x = nx; y = ny;}	
	bool operator == (const Vector2& w) const{return x == w.x && y == w.y;}
	Vector2& operator *= (float s)			{x *= s; y *= s; return *this;}	
	
	Vector2 operator-( const Vector2& V ) const {return Vector2( x - V.x, y - V.y);}

	friend Vector2 operator*( float Scale, const Vector2& V )
	{
		return Vector2( V.x * Scale, V.y * Scale);
	}

	Vector2 operator+( const Vector2& V ) const
	{
		return Vector2( x + V.x, y + V.y);
	}

	float  Dot(const Vector2& v)	const				{return x*v.x + y*v.y;}
	float Length() const
	{
		return sqrtf( x*x + y*y );
	}
	bool Normalize()
	{
		float squareSum = x*x+y*y;
		if( squareSum >= SMALL_NUMBER )
		{
			float scale = 1.f/sqrtf(squareSum);
			x *= scale; y *= scale;
			return true;
		}
		else return false;
	}
};

/// Vector class with x,y,z components, used as core class by entire engine
class Vector
{
public:
	float x,y,z;
	
	float& operator [] (int i)					{return ((float*)this)[i];}
	float operator [] (int i) const				{return ((float*)this)[i];}
	/// ---------------------------------
    /// Constructors
    /// ---------------------------------
	/// Constructors.
	Vector( float InX, float InY, float InZ )
	:	x(InX), y(InY), z(InZ)
	{}

	Vector() :	x(0), y(0), z(0)
	{}


	/// Binary math operators.
	Vector operator^( const Vector& V ) const
	{
		return Vector
		(
			y * V.z - z * V.y,
			z * V.x - x * V.z,
			x * V.y - y * V.x
		);
	}
	float operator|( const Vector& V ) const
	{
		return x*V.x + y*V.y + z*V.z;
	}
	friend Vector operator*( float Scale, const Vector& V )
	{
		return Vector( V.x * Scale, V.y * Scale, V.z * Scale );
	}
	Vector operator+( const Vector& V ) const
	{
		return Vector( x + V.x, y + V.y, z + V.z );
	}
	Vector operator-( const Vector& V ) const
	{
		return Vector( x - V.x, y - V.y, z - V.z );
	}
	Vector operator*( float Scale ) const
	{
		return Vector( x * Scale, y * Scale, z * Scale );
	}
	Vector operator/( float Scale ) const
	{
		float RScale = 1.f/Scale;
		return Vector( x * RScale, y * RScale, z * RScale );
	}
	Vector operator*( const Vector& V ) const
	{
		return Vector( x * V.x, y * V.y, z * V.z );
	}

	/// Binary comparison operators.
	bool operator==( const Vector& V ) const
	{
		return x==V.x && y==V.y && z==V.z;
	}
	bool operator!=( const Vector& V ) const
	{
		return x!=V.x || y!=V.y || z!=V.z;
	}

	/// Unary operators.
	Vector operator-() const
	{
		return Vector( -x, -y, -z );
	}

	/// Assignment operators.
	void operator+=( const Vector& V )
	{
		x += V.x; y += V.y; z += V.z;
	}
	void operator=( const Vector2& V )
	{
		x = V.x; y = V.y;
	}
	void operator-=( const Vector& V )
	{
		x -= V.x; y -= V.y; z -= V.z;
	}
	void operator*=( float Scale )
	{
		x *= Scale; y *= Scale; z *= Scale;
	}
	void operator/=( float V )
	{
		float RV = 1.f/V;
		x *= RV; y *= RV; z *= RV;
	}
	void operator*=( const Vector& V )
	{
		x *= V.x; y *= V.y; z *= V.z;
	}
	void operator/=( const Vector& V )
	{
		x /= V.x; y /= V.y; z /= V.z;
	}

	/// Simple functions.
	float Length() const
	{
		return sqrtf( x*x + y*y + z*z );
	}
	int IsNearlyZero() const
	{
		return
				fabs(x)<KINDA_SMALL_NUMBER
			&&	fabs(y)<KINDA_SMALL_NUMBER
			&&	fabs(z)<KINDA_SMALL_NUMBER;
	}
	bool IsZero() const
	{
		return x==0.f && y==0.f && z==0.f;
	}
	bool Normalize()
	{
		float squareSum = x*x+y*y+z*z;
		if( squareSum >= SMALL_NUMBER )
		{
			float scale = 1.f/sqrtf(squareSum);
			x *= scale; y *= scale; z *= scale;
			return true;
		}
		else return false;
	}

	//---------------------------------
	/// Operator replacement functions
	//---------------------------------
	void Set(float x, float y, float z) { this->x = x; this->y = y; this->z = z; }
	void Add(Vector& v){ *this += v; }
	void Sub(Vector& v){ *this -= v; }
	void Mul(Vector& v){ *this *= v; }
	bool Cmp(Vector& v){ return *this == v; }

	//---------------------------------
	/// Static functions
	//---------------------------------
	static Vector MakeDirection(float yaw, float pitch, float roll);
	static Vector NormalFromTriangle(const Vector& v0, const Vector& v1, const Vector& v2);


	//---------------------------------
	/// The rest
	//---------------------------------
	double Yaw ();
	double Pitch ();
	double Roll ();
	void Zero()										{x = y = z = 0;}
	Vector GetNormal();
	Vector Normalized() const						{ Vector v = *this; v.Normalize(); return v; }//float s = 1.f/Length(); Vector n = *this * s; return n;}
	float  DotSelf() const							{return x*x + y*y + z*z;}
	float  Dot(const Vector& v)	const				{return x*v.x + y*v.y + z*v.z;}
	float  ScalarProjectionOntoVector(Vector& v1)	{return Dot(v1)/v1.Length();}
	Vector ProjectionOntoVector(const Vector& v1)	{return v1*(Dot(v1)/v1.DotSelf());}
	Vector Lerp(const Vector& v1, float t);
	float  RadAngle(const Vector& v1) const		    {return acosf(Dot(v1));}
	float  CosAngle(const Vector& v1) const		    {return Dot(v1);}
	Vector Reflected(const Vector& n)				{Vector r = n*(2.f * Dot(n));r-=*this;return r;}
	float  DistanceToLine(const Vector& p0, const Vector& p1) const;
	float  Average() const							{return (x + y + z) * 0.333333f;}
	Vector HalfWay(const Vector& dest) const		{return Vector(0.5f * (x + dest.x),0.5f * (y + dest.y),0.5f * (z + dest.z));}
	float CalcAngle(Vector& dir, Vector& target);

};


/// Cross normalizes. All functions have made this assumption
Vector Cross(const Vector& v, const Vector& v2);
/// Cross Without Normalize
Vector CrossNoNorm(const Vector& v, const Vector& v2);



/// Extension of vector class with 4th (w) component
class Vector4: public Vector
{
public:
	float w;

	Vector4() : w(0)						{}
	Vector4(const float v[4])	{x=v[0];y=v[1];z=v[2];w=v[3];}
	Vector4(float nx, float ny, float nz, float nw)	
	{
		x = nx; y = ny; z = nz; w = nw;
	}	
	Vector4 Lerp(const Vector4& v1, float t);
	Vector4(const Vector4& w)					{*this = w;}	
	Vector4 operator * (float s) const			{return Vector4(x*s, y*s, z*s, w*s);}
	Vector4& operator *= (float s)				{x *= s; y *= s; z *= s; w *= s; return *this;}
	Vector4 operator * (const Vector4& a) const	{return Vector4(x*a.x, y*a.y, z*a.z, w*a.w);}
	Vector4& operator *= (const Vector4& a)		{x *= a.x; y *= a.y; z *= a.z; w *= a.w; return *this;}	
	Vector4 operator + (const Vector4& a) const	{return Vector4(x+a.x, y+a.y, z+a.z, w+a.w);}
	Vector4& operator += (const Vector4& a)		{x += a.x; y += a.y; z += a.z; w += a.w; return *this;}
	Vector4 operator - (const Vector4& a) const	{return Vector4(x-a.x, y-a.y, z-a.z, w-a.w);}
	Vector4& operator -= (const Vector4& a)		{x -= a.x; y -= a.y; z -= a.z; w -= a.w; return *this;}
	Vector4 operator / (float s) const			{s = 1.f/s; return Vector4(x*s, y*s, z*s, w*s);}	
	Vector4& operator /= (float s)				{s = 1.f/s; x *= s; y *= s; z *= s; w *= s; return *this;}	
	float Length() const							{return sqrtf(x*x+y*y+z*z+w*w);}
	float Average() const							{return (x + y + z + w) * 0.25f;}	
	Vector4 Normalized() const					{float s = 1.f/Length(); Vector4 n = *this * s; return n;}
};


#endif