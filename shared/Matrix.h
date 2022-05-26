//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
///
/// Author: Tim Johnson
//====================================================================================
/*
FILE:		Matrix.h
CONTAINS:	Matrix class
LOG:
	6/20/2000/Derek Nedelman: Created file
	7/1/2000/Timothy Johnson: Re-wrote file

*/


#ifndef TD_TRANSFORM_INCLUDED
#define TD_TRANSFORM_INCLUDED

//#include "TDTypes.h"
#include "Vector.h"
#include <cmath>
#include <memory.h>

//---------------------------------------------------------------------
/// 4x3 (internally 4x4) Matrix class. Has binary compatibility with D3DXMATRIX
//---------------------------------------------------------------------
class Matrix
{	
public:
	Vector m0;	float p0;
	Vector m1;	float p1;
	Vector m2;	float p2;
	Vector m3;	float p3;
	
	Matrix()	{Identity();}
	Matrix(float m00, float m01, float m02, float m03,
			 float m10, float m11, float m12, float m13,
			 float m20, float m21, float m22, float m23,
			 float m30, float m31, float m32, float m33)
	{
		m0 = Vector(m00, m01, m02);	p0 = m03;
		m1 = Vector(m10, m11, m12);	p1 = m13;
		m2 = Vector(m20, m21, m22);	p2 = m23;
		m3 = Vector(m30, m31, m32);	p3 = m33;
	}
	Matrix(const float* r)				{memcpy(&m0, r, sizeof(Matrix));}
	Matrix(const Matrix& r)			{memcpy(&m0, &r, sizeof(Matrix));}
	Matrix(float r)						{Zero(); SetDiagonal(r); p3 = 1;}
	Matrix& operator = (const Matrix& n){memcpy(&m0, &n, sizeof(Matrix)); return *this;}
//	Matrix& operator = (const TDQuat& q);
	Vector operator [] (int i) const		{return *reinterpret_cast<const Vector*>(reinterpret_cast<const float*>(&m0) + (i << 2));}
	Vector& operator [] (int i)				{return *reinterpret_cast<Vector*>(reinterpret_cast<float*>(&m0) + (i << 2));}	
	void Identity()					
	{
		Zero();
		SetDiagonal(1);
		p3 = 1;
	}
	bool IsIdentity(float e=0.001f){
		if(fabsf((m0.x + m1.y + m2.z)-3) > e)
			return false;

		/// Check all identity=0 components
		if(fabsf(m0.y + m0.z + m1.x + m1.z + m2.x + m2.y + m3.x + m3.y + m3.z) > e)
			return false;
		return true;
	}
	void Zero()									{memset(&m0,0, sizeof(Matrix));}
	
	Matrix operator * (const Matrix& n) const
	{
		Matrix result;
		Multiply43(*this, n, result);
		return result;
	}
	Matrix& operator *= (const Matrix& n)
	{
		Matrix result;
		Multiply43(*this, n, result);
		*this = result;
		return *this;
	}
	Matrix operator * (float s) const
	{
		Matrix n(*this);
		n *= s;
		return n;
	}
	Matrix operator - (const Matrix& mat) const
	{
		/// 4x3 matrix so last bit commented out
		return Matrix(m0.x - mat.m0.x, m0.y - mat.m0.y, m0.z - mat.m0.z, p0,//p0 - mat.p0, 
                      m1.x - mat.m1.x, m1.y - mat.m1.y, m1.z - mat.m1.z, p1,//p1 - mat.p1, 
                      m2.x - mat.m2.x, m2.y - mat.m2.y, m2.z - mat.m2.z, p2,//p2 - mat.p2, 
                      m3.x - mat.m3.x, m3.y - mat.m3.y, m3.z - mat.m3.z, p3);//p3 - mat.p3);
	}
	Matrix operator + (const Matrix& mat) const
	{
		/// 4x3 matrix so last bit commented out
		return Matrix(m0.x + mat.m0.x, m0.y + mat.m0.y, m0.z + mat.m0.z, p0,//p0 + mat.p0, 
                      m1.x + mat.m1.x, m1.y + mat.m1.y, m1.z + mat.m1.z, p1,//p1 + mat.p1, 
                      m2.x + mat.m2.x, m2.y + mat.m2.y, m2.z + mat.m2.z, p2,//p2 + mat.p2, 
                      m3.x + mat.m3.x, m3.y + mat.m3.y, m3.z + mat.m3.z, p3);//p3 + mat.p3);
	}
	Matrix& operator *= (float s)
	{
		m0 *= s;
		m1 *= s;
		m2 *= s;
		m3 *= s;//TJ
		return *this;
	}
	Matrix operator / (float s) const
	{
		Matrix n(*this);
		n /= s;
		return n;
	}
	Matrix& operator /= (float s)
	{
		s = 1.f/s;
		m0 *= s;
		m1 *= s;
		m2 *= s;
		m3 *= s; //TJ
		return *this;
	}
	/*Vector2 operator * (const Vector2& v) const
	{
		Vector result;
		TransformVector2Matrix32(v, result);
		return result;
	}*/
	Vector operator * (const Vector& v) const
	{
		Vector result;
		TransformVector3Matrix43(v, result);
		return result;
	}	

	Matrix operator -() const
	{
		return Matrix(m0.x,m0.y,m0.z,p0,
			m1.x,m1.y,m1.z,p1,
			m2.x,m2.y,m2.z,p2,
			m3.x,m3.y,m3.z,p3);
	}

	bool operator ==(const Matrix& rhs) const
	{
		return rhs.m0 == m0 && rhs.m1 == m1 && rhs.m2 == m2 && rhs.m3 == m3;
	}

	Vector GetDir();
	Vector GetRight();
	Vector GetUp();

	Vector4 GetAngles(); /// Euler angles in radians

	float Determinant () const;


	static Matrix LookAt(Vector& dir);

	/// FIXME:
	/// When multiplied with matrix
	/// generates useable matrix, but the scales will be invalid
	/// if accessed again
	/// Need to handle up = 1 case
	static Matrix LookTowards(Vector& Direction) {
		  Matrix m;
		  Vector forward, right;
		  Vector up = Vector(0,1,0);
 
		  forward = Direction.Normalized();
		  right = Cross(forward,up);
		  up = Cross(right,forward);
  
		  //m = matrixfromvectors(right,forward, up);
		  m[0] = right;
		  m[1] = up;
		  m[2] = forward;
		  return m;
	}
		
	float ColumnMag(unsigned int i) const			{return sqrtf(m0[i]*m0[i] + m1[i]*m1[i] + m2[i]*m2[i]);}
	float RowMag(unsigned int i) const				{return (*this)[i].Length();}
	
	void GetColumn(unsigned int i, Vector& v) const	{v.x = m0[i];	v.y = m1[i];	v.z = m2[i];}
	void SetColumn(unsigned int i, const Vector& v)	{m0[i] = v.x;	m1[i] = v.y;	m2[i] = v.z;}	
		
	void FromColumnVectors(const Vector& x, const Vector& y, const Vector& z);
	void FromRowVectors(const Vector& x, const Vector& y, const Vector& z);
	
	void SetXTranslation(float t);
	void SetYTranslation(float t);
	void SetZTranslation(float t);
	void SetTranslations(float x, float y, float z);
	void GetTranslations(float& x, float& y, float& z) const;
	Matrix GetTranslationMatrix() const;
	
	Matrix WithoutScaling() const;

	void SetDiagonal(float d);	
	void SetXScale(float s);
	void SetYScale(float s);
	void SetZScale(float s);
	void SetScales(float x, float y, float z);
	void GetScales(float& x, float& y, float& z) const;	
	Matrix GetScaleMatrix() const;
	
	void SetXRotation(float angle);
	void SetYRotation(float angle);
	void SetZRotation(float angle);
	void SetRotations(float xAngle, float yAngle, float zAngle);
	void GetRotations(float& x, float& y, float& z) const;	
	Matrix GetRotationMatrix() const;	

	Matrix& Normalize();	
	Matrix Normalized() const			{Matrix m(*this); m.Normalize(); return m;}
	
	Matrix& TransposeRotation();
	Matrix TransposedRotation() const	{Matrix m(*this); m.TransposeRotation(); return m;}

	void Invert();	
	Matrix Inverse() const				{Matrix m(*this); m.Invert(); return m;}
	Matrix& InvertOrthonormal();	
	Matrix InverseOrthonormal() const	{Matrix m(*this); m.InvertOrthonormal(); return m;}

	Matrix& Orthonormalize();	
	Matrix Orthonormalized() const		{Matrix m(*this); m.Orthonormalize(); return m;}
		
	

	void FromNormal(const Vector& normal);
	void FromTriangle(const Vector& p0, const Vector& p1, const Vector& p2);
	void FromBiggestOrthoProjection(const Vector& p0, const Vector& p1, const Vector& p2);	
		
	void TransformVector2Matrix2(const Vector2& v, Vector2& mv) const;
	void TransformVector2Matrix32(const Vector2& v, Vector2& mv) const;
	void TransformVector3Matrix3(const Vector& v, Vector& mv) const;
	void TransformVector3Matrix43(const Vector& v, Vector& mv) const;		
	void TransformNormal(const Vector& v, Vector& mv) const;
	float TransformNormalZ(const Vector& v) const;
	float TransformVectorZ(const Vector& v) const;
	
	void ColumnsToVectors(Vector& c0, Vector& c1, Vector& c2) const;
	void RowsToVectors(Vector& c0, Vector& c1, Vector& c2) const;

private:
	static void Multiply33(const Matrix& a, const Matrix& b, Matrix& c);
	static void Multiply43(const Matrix& a, const Matrix& b, Matrix& c);
};


//Functions--------------------------------------------------------------------
//TDArchive& operator << (TDArchive& ar, const Matrix& v);
//TDArchive& operator >> (TDArchive& ar, Matrix& v);

void MatrixGeometryVectorHermiteMatrix(const Vector4& v, Vector4& mv);
void MatrixGeometryVectorBezierMatrix(const Vector4& v, Vector4& mv);
void MatrixGeometryVectorCatmullRomMatrix(const Vector4& v, Vector4& mv);

	

#endif