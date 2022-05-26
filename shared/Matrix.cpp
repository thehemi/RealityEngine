//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
///
/// Author: Tim Johnson
//====================================================================================
#include "stdafx.h"
#include "Matrix.h"
#include <math.h>
#include <float.h>

#ifndef _NOD3DX
#include <d3dx9.h> /// Optimized maths
#endif



enum QuatPart {X, Y, Z, W};

/*** Order type constants, constructors, extractors ***/
    /* There are 24 possible conventions, designated by:    */
    /*	  o EulAxI = axis used initially		    */
    /*	  o EulPar = parity of axis permutation		    */
    /*	  o EulRep = repetition of initial axis as last	    */
    /*	  o EulFrm = frame from which axes are taken	    */
    /* Axes I,J,K will be a permutation of X,Y,Z.	    */
    /* Axis H will be either I or K, depending on EulRep.   */
    /* Frame S takes axes from initial static frame.	    */
    /* If ord = (AxI=X, Par=Even, Rep=No, Frm=S), then	    */
    /* {a,b,c,ord} means Rz(c)Ry(b)Rx(a), where Rz(c)v	    */
    /* rotates v around Z by c radians.			    */
#define EulFrmS	     0
#define EulFrmR	     1
#define EulFrm(ord)  ((unsigned)(ord)&1)
#define EulRepNo     0
#define EulRepYes    1
#define EulRep(ord)  (((unsigned)(ord)>>1)&1)
#define EulParEven   0
#define EulParOdd    1
#define EulPar(ord)  (((unsigned)(ord)>>2)&1)
#define EulSafe	     "\000\001\002\000"
#define EulNext	     "\001\002\000\001"
#define EulAxI(ord)  ((int)(EulSafe[(((unsigned)(ord)>>3)&3)]))
#define EulAxJ(ord)  ((int)(EulNext[EulAxI(ord)+(EulPar(ord)==EulParOdd)]))
#define EulAxK(ord)  ((int)(EulNext[EulAxI(ord)+(EulPar(ord)!=EulParOdd)]))
#define EulAxH(ord)  ((EulRep(ord)==EulRepNo)?EulAxK(ord):EulAxI(ord))
    /* EulGetOrd unpacks all useful information about order simultaneously. */
#define EulGetOrd(ord,i,j,k,h,n,s,f) {unsigned o=ord;f=o&1;o>>=1;s=o&1;o>>=1;\
    n=o&1;o>>=1;i=EulSafe[o&3];j=EulNext[i+n];k=EulNext[i+1-n];h=s?k:i;}
    /* EulOrd creates an order value between 0 and 23 from 4-tuple choices. */
#define EulOrd(i,p,r,f)	   (((((((i)<<1)+(p))<<1)+(r))<<1)+(f))
    /* Static axes */
#define EulOrdXYZs    EulOrd(X,EulParEven,EulRepNo,EulFrmS)
#define EulOrdXYXs    EulOrd(X,EulParEven,EulRepYes,EulFrmS)
#define EulOrdXZYs    EulOrd(X,EulParOdd,EulRepNo,EulFrmS)
#define EulOrdXZXs    EulOrd(X,EulParOdd,EulRepYes,EulFrmS)
#define EulOrdYZXs    EulOrd(Y,EulParEven,EulRepNo,EulFrmS)
#define EulOrdYZYs    EulOrd(Y,EulParEven,EulRepYes,EulFrmS)
#define EulOrdYXZs    EulOrd(Y,EulParOdd,EulRepNo,EulFrmS)
#define EulOrdYXYs    EulOrd(Y,EulParOdd,EulRepYes,EulFrmS)
#define EulOrdZXYs    EulOrd(Z,EulParEven,EulRepNo,EulFrmS)
#define EulOrdZXZs    EulOrd(Z,EulParEven,EulRepYes,EulFrmS)
#define EulOrdZYXs    EulOrd(Z,EulParOdd,EulRepNo,EulFrmS)
#define EulOrdZYZs    EulOrd(Z,EulParOdd,EulRepYes,EulFrmS)
    /* Rotating axes */
#define EulOrdZYXr    EulOrd(X,EulParEven,EulRepNo,EulFrmR)
#define EulOrdXYXr    EulOrd(X,EulParEven,EulRepYes,EulFrmR)
#define EulOrdYZXr    EulOrd(X,EulParOdd,EulRepNo,EulFrmR)
#define EulOrdXZXr    EulOrd(X,EulParOdd,EulRepYes,EulFrmR)
#define EulOrdXZYr    EulOrd(Y,EulParEven,EulRepNo,EulFrmR)
#define EulOrdYZYr    EulOrd(Y,EulParEven,EulRepYes,EulFrmR)
#define EulOrdZXYr    EulOrd(Y,EulParOdd,EulRepNo,EulFrmR)
#define EulOrdYXYr    EulOrd(Y,EulParOdd,EulRepYes,EulFrmR)
#define EulOrdYXZr    EulOrd(Z,EulParEven,EulRepNo,EulFrmR)
#define EulOrdZXZr    EulOrd(Z,EulParEven,EulRepYes,EulFrmR)
#define EulOrdXYZr    EulOrd(Z,EulParOdd,EulRepNo,EulFrmR)
#define EulOrdZYZr    EulOrd(Z,EulParOdd,EulRepYes,EulFrmR)

//EulerAngles Eul_(float ai, float aj, float ah, int order);
//void Eul_ToHMatrix(EulerAngles ea, HMatrix M);
//EulerAngles Eul_FromHMatrix(HMatrix M, int order);


template <class T>
T TDMin(T a, T b)
{
	return (a < b) ? a : b;
}

template <class T>
T TDMax(T a, T b)
{
	return (a > b) ? a : b;
}


Vector4 Matrix::GetAngles()
{
	Matrix& M = *this;
    Vector4 ea;

	int order = EulOrdXYZs;
    int i,j,k,h,n,s,f;
    EulGetOrd(order,i,j,k,h,n,s,f);
    if (s==EulRepYes) {
	double sy = sqrt(M[i][j]*M[i][j] + M[i][k]*M[i][k]);
	if (sy > 16*FLT_EPSILON) {
	    ea.x = atan2(M[i][j], M[i][k]);
	    ea.y = atan2((float)sy, M[i][i]);
	    ea.z = atan2(M[j][i], -M[k][i]);
	} else {
	    ea.x = atan2(-M[j][k], M[j][j]);
	    ea.y = atan2((float)sy, M[i][i]);
	    ea.z = 0;
	}
    } else {
	double cy = sqrt(M[i][i]*M[i][i] + M[j][i]*M[j][i]);
	if (cy > 16*FLT_EPSILON) {
	    ea.x = atan2(M[k][j], M[k][k]);
	    ea.y = atan2(-M[k][i], (float)cy);
	    ea.z = atan2(M[j][i], M[i][i]);
	} else {
	    ea.x = atan2(-M[j][k], M[j][j]);
	    ea.y = atan2(-M[k][i], (float)cy);
	    ea.z = 0;
	}
    }
    if (n==EulParOdd) {ea.x = -ea.x; ea.y = - ea.y; ea.z = -ea.z;}
    if (f==EulFrmR) {float t = ea.x; ea.x = ea.z; ea.z = t;}
    ea.w = order;

    return (ea);
}


Vector Matrix::GetDir(){
	return m2.Normalized();
}

Vector Matrix::GetRight(){
	return m0.Normalized();
}

Vector Matrix::GetUp(){
	return m1.Normalized();
}


Matrix Matrix::WithoutScaling() const
{
	Matrix m = this->GetRotationMatrix();
	m[3] = (*this)[3];
	return m;
}

float Matrix::Determinant () const
{
    float fCofactor00 = (*this)[1][1]*(*this)[2][2] -
        (*this)[1][2]*(*this)[2][1];
    float fCofactor10 = (*this)[1][2]*(*this)[2][0] -
        (*this)[1][0]*(*this)[2][2];
    float fCofactor20 = (*this)[1][0]*(*this)[2][1] -
        (*this)[1][1]*(*this)[2][0];

    float fDet =
        (*this)[0][0]*fCofactor00 +
        (*this)[0][1]*fCofactor10 +
        (*this)[0][2]*fCofactor20;

    return fDet;
}


//Implementation---------------------------------------------------------------
/*Matrix& Matrix::operator = (const TDQuat& q)
{
	//Converts a quaternion rotation to a matrix rotation. 

	float wx, wy, wz, 
			xx, yy, yz, 
			xy, xz, zz, 
			x2, y2, z2;

	/// calculate coefficients
	x2 = q.x + q.x; y2 = q.y + q.y; 
	z2 = q.z + q.z;
	xx = q.x * x2;   xy = q.x * y2;   xz = q.x * z2;
	yy = q.y * y2;   yz = q.y * z2;   zz = q.z * z2;
	wx = q.w * x2;   wy = q.w * y2;   wz = q.w * z2;

	m0[0] = 1.0 - (yy + zz); 	m0[1] = xy - wz;			m0[2] = xz + wy; 
	m1[0] = xy + wz;			m1[1] = 1.0 - (xx + zz);	m1[2] = yz - wx;
	m2[0] = xz - wy;			m2[1] = yz + wx;			m2[2] = 1.0 - (xx + yy);
	m3.Zero();
	
	return *this;
}*/

void Matrix::FromColumnVectors(const Vector& x, const Vector& y, const Vector& z)
{
	m0.x = x.x;	m0.y = y.x;	m0.z = z.x;
	m1.x = x.y;	m1.y = y.y;	m1.z = z.y;
	m2.x = x.z;	m2.y = y.z;	m2.z = z.z;
}

void Matrix::FromRowVectors(const Vector& x, const Vector& y, const Vector& z)
{
	m0 = x;
	m1 = y;
	m2 = z;	
}

void Matrix::SetXTranslation(float t)
{
	m3.x = t;
}

void Matrix::SetYTranslation(float t)
{
	m3.y = t;
}

void Matrix::SetZTranslation(float t)
{
	m3.z = t;
}

void Matrix::SetTranslations(float x, float y, float z)
{
	m3 = Vector(x, y, z);
}

void Matrix::GetTranslations(float& x, float& y, float& z) const
{
	x = m3.x;
	y = m3.y;
	z = m3.z;
}

Matrix Matrix::GetTranslationMatrix() const
{
	Matrix result;
	result[3] = m3;
	return result;
}

void Matrix::SetDiagonal(float d)
{
	m0.x = m1.y = m2.z = d;
}

void Matrix::SetXScale(float s)
{
	m0.x = s;
}

void Matrix::SetYScale(float s)
{
	m1.y = s;
}

void Matrix::SetZScale(float s)
{
	m2.z = s;
}

void Matrix::SetScales(float x, float y, float z)	
{
	m0.x = x;
	m1.y = y;
	m2.z = z;
}

void Matrix::GetScales(float& x, float& y, float& z) const
{
	x = m0.Length();
	y = m1.Length();
	z = m2.Length();
}

Matrix Matrix::GetScaleMatrix() const
{
	Matrix result;
	result[0][0] = m0.Length();
	result[1][1] = m1.Length();
	result[2][2] = m2.Length();
	return result;
};

void Matrix::SetXRotation(float angle)
{
	float sinAngle, cosAngle;
	
	sinAngle = sinf(angle);
	cosAngle = cosf(angle);

	m1.y = cosAngle; m2.y = -sinAngle;
	m1.z = sinAngle; m2.z =  cosAngle;
}

void Matrix::SetYRotation(float angle)
{
	float sinAngle, cosAngle;
	
	sinAngle = sinf(angle);
	cosAngle = cosf(angle);
		
	m0.x = cosAngle;  m2.x = sinAngle;
	m0.z = -sinAngle; m2.z = cosAngle;			
}
	
void Matrix::SetZRotation(float angle)
{
	float sinAngle, cosAngle;
	
	sinAngle = sinf(angle);
	cosAngle = cosf(angle);
		
	m0.x = cosAngle; m1.x = -sinAngle;
	m0.y = sinAngle; m1.y = cosAngle;
}	

void Matrix::SetRotations(float xAngle, float yAngle, float zAngle)
{
	Matrix x, y, z, rot;

	x.SetXRotation(xAngle);
	y.SetYRotation(yAngle);
	z.SetZRotation(zAngle);
	rot = x * y * z;
	
	m0 = rot[0];
	m1 = rot[1];
	m2 = rot[2];
}

void Matrix::GetRotations(float& x, float& y, float& z) const
{
	const float EULER_TOLERANCE = .05f;
	const int i = 2,	//These are here because I needed to play 
				j = 1,	//with the values until it worked
				k = 0;
	const Matrix m = (*this).Normalized();

    float cy = sqrtf(m[i][i]*m[i][i] + m[j][i]*m[j][i]);	
		
	if (cy > EULER_TOLERANCE) 
	{
		x = atan2f(m[j][i], m[i][i]);
		y = atan2f(-m[k][i], cy);
		z = atan2f(m[k][j], m[k][k]);
	} 
	else 
	{
		x = 0;
		y = atan2f(-m[k][i], cy);
		z = atan2f(-m[j][k], m[j][j]);
	}    

		y = asin(-m[0][2]);
        if (fabs(cos(y)) > EULER_TOLERANCE)
        {
            x = atan2 (m[1][2], m[2][2]);
            z = atan2 (m[0][1], m[0][0]);
        }
        else
        {
            x = atan2 (m[1][0], m[1][1]);
            z = 0.0;
        }

}

Matrix Matrix::GetRotationMatrix() const
{
	Matrix result(*this);
	result[3].Zero();
	return result.Normalized();
}

Matrix& Matrix::Normalize()
{
	m0 *= (1.f/m0.Length());
	m1 *= (1.f/m1.Length());
	m2 *= (1.f/m2.Length());
	return *this;
}

Matrix& Matrix::TransposeRotation()
{
	//Transposes the rotation part of m and stores it in mt
	Matrix result;
	result.FromColumnVectors(m0, m1, m2);	
	*this = result;
	return *this;
}


void Matrix::Invert()
{
#ifndef _NOD3DX
	/// Using this because it's heavily optimized
	D3DXMatrixInverse((D3DXMATRIX*)this,0,(D3DXMATRIX*)this);
	return;
#else
	//Calculates the inverse of m, assuming that m is invertible
	//m = [S][R][T], m_inv = [-T][Rtranspose][1/S]

	Matrix r(*this);				//m after normalization
	float oo_xs, oo_ys, oo_zs;		//Inverse scales for each colur
	Matrix m_inv;
	
	oo_xs = 1.f/m0.Length();
	oo_ys = 1.f/m1.Length();
	oo_zs = 1.f/m2.Length();

	r.m0 *= oo_xs;
	r.m1 *= oo_ys;
	r.m2 *= oo_zs;
	
	m_inv = r.TransposedRotation();				//m_inv = [Rtranspose]

	m_inv.m3.x = -m3.Dot(r.m0);
	m_inv.m3.y = -m3.Dot(r.m1);
	m_inv.m3.z = -m3.Dot(r.m2);		//m_inv = [-T][Rtranspose]
		
	//m_inv = [-T][Rtranspose][1/S]	
	m_inv.m0.x *= oo_xs;	m_inv.m0.y *= oo_ys;	m_inv.m0.z *= oo_zs;	
	m_inv.m1.x *= oo_xs; m_inv.m1.y *= oo_ys;	m_inv.m1.z *= oo_zs;	
	m_inv.m2.x *= oo_xs; m_inv.m2.y *= oo_ys;	m_inv.m2.z *= oo_zs;	
	m_inv.m3.x *= oo_xs; m_inv.m3.y *= oo_ys;	m_inv.m3.z *= oo_zs;		

	*this = m_inv;
#endif
}

Matrix& Matrix::InvertOrthonormal()
{
	//Calculates the inverse of m, assuming that the columns of the rotation part of m are orthonormal
	//m = [R][T], m_inv = [-T][Rtranspose]
	Matrix m_inv;
	
	//Transpose rotations
	m_inv = TransposedRotation();			//m_inv = [Rtranspose]

	//Invert translations
	m_inv[3].x = -m3.Dot(m0);
	m_inv[3].y = -m3.Dot(m1);
	m_inv[3].z = -m3.Dot(m2);			//m_inv = [-T][Rtranspose]

	*this = m_inv;
	return *this;
}

Matrix& Matrix::Orthonormalize()
{
	//Orthogonalizes a matrix using the Gram-Schmidt process
	Vector c0, c1, c2,	
			v0, v1, v2,			//perpendicular vectors generated from orthogonalization process
		    c1_on_v0,			//c1 projected onto c0			
			c2_on_v0, c2_on_v1;	//c2 projected onto v0 and v1
			
	
	c0 = m0;
	c1 = m1;
	c2 = m2;

	//First basis vector
	//v0 = c0
	v0 = c0;

	//Second basis vector	
	//v1 = c1 - c1_on_v0
	c1_on_v0 = c1.ProjectionOntoVector(v0);
	v1 = c1 - c1_on_v0;	
		
	//Third basis vector
	//v2 = c2 - c2_on_v0 - c2_on_v1 = c2 - (c2_on_v0 + c2_on_v1)
	c2_on_v0 = c2.ProjectionOntoVector(v0);
	c2_on_v1 = c2.ProjectionOntoVector(v1);	
	v2 = c2 - (c2_on_v0 + c2_on_v1);

	//Set the matrix
	//m0 = v0;
	m1 = v1;
	m2 = v2;

	//Normalize the matrix
	Normalize();

	return *this;
}

Matrix Matrix::LookAt(Vector& v)
{
	//This function creates a matrix for a camera that "looks" at p = (a, b, c)	
	Vector f, r, u;
		    
	//Create the z axis by normalizing the vector that goes from the origin of m to the follow position
	/// Tim: Flipped for LH CS
	/// Forwards vector (correct)
	f = v;
	f.Normalize();
				
	//Find the x axis by normalizing the cross product of the local and world z axes
	/// Tim: Changed up to Y. Right vector (correct)
	u = Vector(0.0f,1.0f,0.0f); /// Initial up guess
	r = Cross(u,f);
	
	/// Handle f =~ up case
	if(r.Length() < 0.1f){
		u = Vector(0.1f,0.9f,0.1f);
		r = Cross(u,f);
	}
	/// commented out 'cause cross auto-normalizes
	//x.Normalize();

	//Find the y axis by normalizing the cross product of the z and x axes	
	u = Cross(r,f);	
	//y.Normalize();

	//Done
	Matrix m;
	m.m0 = r;
	m.m1 = u;
	m.m2 = f;
	return m;

//	m0.x=r.x; m0.y=u.x; m0.z=f.x;
//	m1.x=r.y; m1.y=u.y; m1.z=f.y;
//	m2.x=r.z; m2.y=u.z; m2.z=f.z;

}

void Matrix::FromNormal(const Vector& normal)
{
	//TDBool lookingUp = TDFalse;

	Vector vx;

	vx.z = 0.0f;
	vx.x = -normal.y;
	vx.y = normal.x;	
	if (vx.x == 0.0f && vx.y == 0.0f) 
	{
		//lookingUp = TDTrue;
		vx.x = 1.f;
	}
	
	m0 = vx;
	m1 = Cross(normal,vx);
	m2 = normal;
	m3.Zero();
	Normalize();

	/*
	if (lookingUp)
	{
		m0.y *= -1.f;
		m1.y *= -1.f;
		m2.y *= -1.f;	
	}
	else
	{
		m0.x *= -1.f;
		m1.x *= -1.f;
		m2.x *= -1.f;	
	}
	*/
}

void Matrix::FromTriangle(const Vector& p0, const Vector& p1, const Vector& p2)
{	
	Vector normal,
		     mid;
	
	normal = Cross((p1 - p0),(p2 - p0)).Normalized();
	mid = (p0 + p1 + p2) * .333333f;
	
	FromNormal(normal);
	m3 = mid;
}

void Matrix::FromBiggestOrthoProjection(const Vector& p0, const Vector& p1, const Vector& p2)
{
	//Creates a transformation matrix that, when applied to the input triangle vertices results in the largest
	//possible orthographic projection into 2D (x/y) coordinates.  This is done by dropping the dimension of the
	//triangle for which its absolute normal component is greatest
	//m should be zeroed out before this function is called

	Vector n;
	float max;

	n = Cross((p1 - p0),p2 - p0);
	
	n.x = fabsf(n.x);
	n.y = fabsf(n.y);
	n.z = fabsf(n.z);

	max = TDMax(TDMax(n.x, n.y), n.z);
	
	if (max == n.x)
	{
		m0.y = 1.f;
		m1.z = 1.f;
	}
	else if (max == n.y)
	{
		m0.x = 1.f;
		m1.z = 1.f;		
	}
	else //(max == n.z)
	{
		m0.x = 1.f;
		m1.y = 1.f;		
	}
}

void Matrix::TransformVector2Matrix2(const Vector2& v, Vector2& mv) const
{	
	//Transforms a vector as if it were given in R2
	mv.x = (m0.x * v.x) + (m1.x * v.y);
	mv.y = (m0.y * v.x) + (m1.y * v.y);
}

void Matrix::TransformVector2Matrix32(const Vector2& v, Vector2& mv) const
{	
	//Transforms a vector as if it were in R2 with translation
	//The translation is where it would be for the 4x4 case
	mv.x = (m0.x * v.x) + (m1.x * v.y) + m3.x;
	mv.y = (m0.y * v.x) + (m1.y * v.y) + m3.y;
}

void Matrix::TransformVector3Matrix3(const Vector& v, Vector& mv) const
{
	//Transforms a vector as if were in R3
	mv.x = (m0.x * v.x) + (m1.x * v.y) + (m2.x * v.z);
	mv.y = (m0.y * v.x) + (m1.y * v.y) + (m2.y * v.z);
	mv.z = (m0.z * v.x) + (m1.z * v.y) + (m2.z * v.z);
}

void Matrix::TransformVector3Matrix43(const Vector& v, Vector& mv) const
{
	//Transforms a vector as if it were in R3 with translation
	mv.x = (m0.x * v.x) + (m1.x * v.y) + (m2.x * v.z) + m3.x;
	mv.y = (m0.y * v.x) + (m1.y * v.y) + (m2.y * v.z) + m3.y;
	mv.z = (m0.z * v.x) + (m1.z * v.y) + (m2.z * v.z) + m3.z;
}

void Matrix::TransformNormal(const Vector& v, Vector& mv) const
{
	//Multiplies and matrix and a vector, ignoring the translation part of the matrix
	mv.x = (m0.x * v.x) + (m1.x * v.y) + (m2.x * v.z);
	mv.y = (m0.y * v.x) + (m1.y * v.y) + (m2.y * v.z);
	mv.z = (m0.z * v.x) + (m1.z * v.y) + (m2.z * v.z);
}

float Matrix::TransformNormalZ(const Vector& v) const
{
	//Transforms and returns only the z component of v
	return (m0.z * v.x) + (m1.z * v.y) + (m2.z * v.z);
}

float Matrix::TransformVectorZ(const Vector& v) const
{
	//Transforms and returns only the z component of v
	return (m0.z * v.x) + (m1.z * v.y) + (m2.z * v.z) + m3.z;
}

void Matrix::Multiply33(const Matrix& a, const Matrix& b, Matrix& c)
{
	//Does a 3x3 matrix multiplication

	c[0].x = a[0].Dot(Vector(b[0].x, b[1].x, b[2].x));
	c[0].y = a[0].Dot(Vector(b[0].y, b[1].y, b[2].y));
	c[0].z = a[0].Dot(Vector(b[0].z, b[1].z, b[2].z));

	c[1].x = a[1].Dot(Vector(b[0].x, b[1].x, b[2].x));
	c[1].y = a[1].Dot(Vector(b[0].y, b[1].y, b[2].y));
	c[1].z = a[1].Dot(Vector(b[0].z, b[1].z, b[2].z));

	c[2].x = a[2].Dot(Vector(b[0].x, b[1].x, b[2].x));
	c[2].y = a[2].Dot(Vector(b[0].y, b[1].y, b[2].y));
	c[2].z = a[2].Dot(Vector(b[0].z, b[1].z, b[2].z));
}

void Matrix::Multiply43(const Matrix& a, const Matrix& b, Matrix& c)
{
	/// Tests showed this to be a zillion times faster in DEBUG, it's insane
#ifndef _NOD3DX
	D3DXMatrixMultiply((D3DXMATRIX*)&c,(D3DXMATRIX*)&a,(D3DXMATRIX*)&b);
#else

	//Does a 4x4 matrix multiplication, treating the bottom row as though it has all zeroes, except for
	//m3[4], which is assumed to be 1
	c[0].x = a[0].Dot(Vector(b[0].x, b[1].x, b[2].x));
	c[0].y = a[0].Dot(Vector(b[0].y, b[1].y, b[2].y));
	c[0].z = a[0].Dot(Vector(b[0].z, b[1].z, b[2].z));

	c[1].x = a[1].Dot(Vector(b[0].x, b[1].x, b[2].x));
	c[1].y = a[1].Dot(Vector(b[0].y, b[1].y, b[2].y));
	c[1].z = a[1].Dot(Vector(b[0].z, b[1].z, b[2].z));

	c[2].x = a[2].Dot(Vector(b[0].x, b[1].x, b[2].x));
	c[2].y = a[2].Dot(Vector(b[0].y, b[1].y, b[2].y));
	c[2].z = a[2].Dot(Vector(b[0].z, b[1].z, b[2].z));

	c[3].x = a[3].Dot(Vector(b[0].x, b[1].x, b[2].x)) + b[3].x;
	c[3].y = a[3].Dot(Vector(b[0].y, b[1].y, b[2].y)) + b[3].y;
	c[3].z = a[3].Dot(Vector(b[0].z, b[1].z, b[2].z)) + b[3].z;
	#endif
}
 

void MatrixGeometryVectorHermiteMatrix(const Vector4& v, Vector4& mv)
{
	//Transforms a geometry vector using a Hermite basis matrix.  Actually, it doesn't use a matrix
	//but instead makes use of the fact that we know the matrix beforehand to speed up computation.

	mv.x = 2.f * (v.x - v.y) + v.z + v.w;
	mv.y = -(3.f * (v.x - v.y) + (2.f * v.z) + v.w);
	mv.z = v.z;
	mv.w = v.x;
}

void MatrixGeometryVectorBezierMatrix(const Vector4& v, Vector4& mv)
{
	mv.x = -v.x + 3.f * (v.y - v.z) + v.w;
	mv.y = 3.f * (v.x - (2.f * v.y) + v.z);
	mv.z = 3.f * (v.y - v.x);
	mv.w = v.x;
}

void MatrixGeometryVectorCatmullRomMatrix(const Vector4& v, Vector4& mv)
{
	mv.x = .5f * (v.w - v.x) + 1.5f * (v.y - v.z);	
	mv.y = v.x - (2.5f * v.y) + (2.f * v.z) - (.5f * v.w);
	mv.z = .5f * (-v.x + v.z);
	mv.w = v.y;	
}

