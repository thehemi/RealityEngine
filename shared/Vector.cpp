//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
///
/// Author: Tim Johnson
/// 4/22/98/Derek Nedelman & Timothy Johnson: Created file from MATH.H
///	5/22/2000/Timothy Johnson: Modified and cleaned up file for new engine
///	5/30/2000/TJ: Added friend operator(s)
///	6/1/2000/TJ: > < Length checks are now free of sqrt
///	6/2/2000/TJ: Fixed quite a few misc things
///	6/20/2000/DN: Removed a few access functions, added a few consts, etc
//====================================================================================

#include "stdafx.h"
#include "Vector.h"

#ifndef _NOD3DX
#include "d3dx9.h"
#endif

#ifndef RAD_TO_DEG
#define RAD_TO_DEG (180.0f / 3.141592654f)
#endif

static Vector X_Axis(1.0, 0.0, 0.0);
static Vector Y_Axis(0.0, 1.0, 0.0);
static Vector Z_Axis(0.0, 0.0, 1.0);

float  Vector::CalcAngle(Vector& rotation, Vector& target)
{
	Vector xzDir = rotation;
	xzDir.y = 0;
	xzDir.Normalize();
	Vector xzLocation = target - *this;
	xzLocation.y = 0;
	xzLocation.Normalize();
	float angle = atan2(xzDir.z,xzDir.x) - atan2(xzLocation.z,xzLocation.x);
	if(angle < 0)angle += 2*PI;
	return RAD2DEG(angle);
}

Vector4 Vector4::Lerp(const Vector4& v1, float t)
{
	//Linearly interpolate between vectors by an amount t and return the resulting vector. 			
	//v = v0 + (v1 - v0)t = v0 + t(v1) - t(v0) = v0 - t(v0) + t(v1) = (1 - t)v0 + t(v1)
	float omt = 1.f - t;	
	Vector4 v = *this;
	v.x = omt * x + t * v1.x;
	v.y = omt * y + t * v1.y;
	v.z = omt * z + t * v1.z;
	v.w = omt * w + t * v1.w;
	return v;
}

Vector Vector::GetNormal()
{
	float fx = fabsf(x); 
	float fy = fabsf(y); 
	float fz = fabsf(z); 
	if(fx > fy && fx > fz) return Vector(1,0,0); 
	if(fy > fx && fy > fz) return Vector(0,1,0); 
	if(fz > fx && fz > fy) return Vector(0,0,1); 

	return Vector(0,0,0);
}

//Functions--------------------------------------------------------------------
Vector Vector::Lerp(const Vector& v1, float t)
{
	//Linearly interpolate between vectors by an amount t and return the resulting vector. 			
	//v = v0 + (v1 - v0)t = v0 + t(v1) - t(v0) = v0 - t(v0) + t(v1) = (1 - t)v0 + t(v1)
	float omt = 1.f - t;	
	Vector v = *this;
	v.x = omt * x + t * v1.x;
	v.y = omt * y + t * v1.y;
	v.z = omt * z + t * v1.z;
	return v;
}

Vector Vector::NormalFromTriangle(const Vector& v0, const Vector& v1, const Vector& v2)
{
	Vector n1, n2;
	n1 = v1-v0;n2 = v2-v0;
	return Cross(n1,n2);
}
	


Vector Cross(const Vector& v, const Vector& v2)
{
	Vector n;
	n.x = (v2.y * v.z) - (v2.z * v.y);
	n.y = (v2.z * v.x) - (v2.x * v.z);
	n.z = (v2.x * v.y) - (v2.y * v.x);
	return n.Normalized();
}

Vector CrossNoNorm(const Vector& v, const Vector& v2)
{
	Vector n;
	n.x = (v2.y * v.z) - (v2.z * v.y);
	n.y = (v2.z * v.x) - (v2.x * v.z);
	n.z = (v2.x * v.y) - (v2.y * v.x);
	return n;
}



/// FIXME: Hacky and slow
Vector Vector::MakeDirection(float yaw, float pitch, float roll)
{
	//Convert from degrees to radians
	roll  *=0.0174532f;
	pitch *=0.0174532f;
	yaw   *=0.0174532f;
#ifdef _NOD3DX
	double X = (sin(pitch)*sin(yaw)*sin(roll))+ (cos(pitch)*cos(roll));
	double Y = cos(yaw)*sin(roll);
	double Z = (sin(pitch)*cos(roll))- (cos(roll)*sin(pitch)*sin(yaw));
#endif

#ifndef _NOD3DX
	D3DXMATRIX matTrans, matRotateX, matRotateY, matRotateZ, matTemp1, matTemp2;
	D3DXMatrixRotationX( &matRotateX, pitch );
	D3DXMatrixRotationY( &matRotateY, yaw );
	D3DXMatrixRotationZ( &matRotateZ, roll );
	D3DXMatrixMultiply( &matTemp1, &matRotateX, &matRotateY );
	D3DXMatrixMultiply( &matTemp2, &matRotateZ, &matTemp1 );


//	Matrix m;
//	m.SetRotations(pitch,yaw,roll);

		/// Now extract the boid1s direction out of the matrix
	float X = matTemp2._31;
	float Y = matTemp2._32;
	float Z = matTemp2._33;
#endif


	return Vector((float)X,(float)Y,(float)Z).Normalized();
}

float Vector::DistanceToLine(const Vector& p0, const Vector& p1) const
{
	Vector p0_to_p = *this - p0;
	Vector p0_to_p1 = p1 - p0;

	Vector p_on_line = p0_to_p.ProjectionOntoVector(p0_to_p1);
	Vector perp	  = p0_to_p - p_on_line;

	return perp.Length();
}


/**
* Calc the roll (about the velocity vector) of the vector. This is done
* by calculating the "accelerational up" vector. Roll is then the angle
* between the accelerational up vector and the world up vector.
*
* @param acceleration The acceleration of the vec
*/
double Vector::Roll ()
{
	/// roll is an angle from -90 to 90 porportional to the magnitude
	/// of the xy component of the acceleration vector.
	double xy = sqrt (x * x + y * y);
	
	return atan (xy) * RAD_TO_DEG;
	
}


/**
  * Calc the angle the specified vector makes with the x axis
  * in the x-z plane. This is equal to the pitch angle.
  *
  * @param vector The vector whose pitch value we want
*/
double Vector::Pitch ()
{
	/// this formula is derived from circle geometry:
	/// first project vector onto the xz plane. This is simple:
	/// vx = vector.x, vz = vector.z. Next, observe that the pitch
	/// is the angle (vx, vz) makes with the x axis. Now, also observe
	/// that the point (vx, vz) on the unit circle can also be expressed
	/// as (cos theta, sin theta) where theta is the angle with the +x axis.
	/// therefore, pitch = theta = asin (vz) = 90 - acos (vx)
	
	return -asin (z) * RAD_TO_DEG;
	
}


/**
  * Calc the angle the specified vector makes with the x axis
  * in the x-y plane. This is equal to the boid's yaw angle.
  *
  * @param vector The vector whose yaw value we want
*/
double Vector::Yaw ()
{
	double dot = Dot(X_Axis);
	
	double angle = 0.0;
	if (y > 0)
	{
		angle = acos (dot) * RAD_TO_DEG;
	}
	else
	{
		angle = -acos (dot) * RAD_TO_DEG;
	}
	
	return angle;
} 
