//=========== (C) Copyright 2004, Artficial Studios. All rights reserved. =======
// C# Scripting System Reality API Wrappers
// Author: Mostafa Mohamed & Jeremy Stieglitz
//
//===============================================================================
#pragma	once
#pragma warning( disable : 4240 )

using namespace	System;
using namespace	System::Collections;
using namespace System::Reflection;
using namespace System::ComponentModel;
using namespace System::Drawing;
using namespace stdcli::language;
#include "..\EngineInc\ScriptEngine.h"
#include "HDR.h"
#include "GameRecord.h"
#include "..\GameDLL\Networking\GameClientModule.h"
#include "..\GameDLL\Game\FX\SurfaceDecal.h"
#include "..\GameDLL\Game\FX\Particles.h"
#include "..\GameDLL\Game\FX\LightBeam.h"
#include "StreamingOgg.h"
#include "LODManager.h"
#include "SkyController.h"
#include "WaterSurface.h"
#include "Helpers.h"

namespace ScriptingSystem
{
	public delegate	void SSystem_Delegate();

	// <summary>
	/// enumeration of the Light Types
	// </summary>
	public enum class  MLightType{
		// <summary>
		/// omni light
		// </summary>
		MLIGHT_OMNI = 1,
		// <summary>
		/// spot light
		// </summary>
		MLIGHT_SPOT = 2,
		// <summary>
		/// directional light
		// </summary>
		MLIGHT_DIR = 4,
		// <summary>
		/// omni projector light
		// </summary>
		MLIGHT_OMNI_PROJ = 8,
	};

	// <summary>
	/// enumeration of drawing blend modes
	// </summary>
	public enum class MBlendMode
	{
		// <summary>
		/// no blending
		// </summary>
		MBLEND_NONE		  = 0,
		// <summary>
		/// subtractive
		// </summary>
		MBLEND_ZERO              = 1,
		// <summary>
		/// additive
		// </summary>
		MBLEND_ONE               = 2,
		MBLEND_SRCCOLOR          = 3,
		MBLEND_INVSRCCOLOR       = 4,
		MBLEND_SRCALPHA          = 5,
		MBLEND_INVSRCALPHA       = 6,
		MBLEND_DESTALPHA         = 7,
		MBLEND_INVDESTALPHA      = 8,
		MBLEND_DESTCOLOR         = 9,
		MBLEND_INVDESTCOLOR      = 10,
		MBLEND_SRCALPHASAT       = 11,
		MBLEND_BOTHSRCALPHA      = 12,
		MBLEND_BOTHINVSRCALPHA   = 13,
	};

	// <summary>
	/// different ray-collision check types, for optimization of the calculation speed
	// </summary>
	public enum class MCheckType{
		// <summary>
		/// only test against non-Prefab Actors
		// </summary>
		CHECK_ACTORS,
		// <summary>
		/// only test against Prefabs
		// </summary>
		CHECK_GEOMETRY,
		// <summary>
		/// test with all Actors, Prefabs
		// </summary>
		CHECK_EVERYTHING,
		// <summary>
		/// test with only visible Prefabs
		// </summary>
		CHECK_VISIBLE_GEOMETRY,
		// <summary>
		/// test with only visible non-Prefab Actors
		// </summary>
		CHECK_VISIBLE_ACTORS,
		// <summary>
		/// test with only visible Actors that are unfrozen
		// </summary>
		CHECK_VISIBLE_ACTORS_UNFROZEN,
		// <summary>
		/// test with only visible Prefabs AND non-Prefab Actors, including CF_PASSABLE Actors
		// </summary>
		CHECK_VISIBLE_EVERYTHING,
		// <summary>
		/// test with only visible Prefabs AND non-Prefab Actors, not including CF_PASSABLE Actors
		// </summary>
		CHECK_UNHIDDEN_EVERYTHING
	};

	// <summary>
	/// floating point color, equivalent to a vector4
	// </summary>
	public ref class MFloatColor 
	{
		private	public:
			FloatColor *	 m_color;
			bool		needToDelete;

			MFloatColor(FloatColor * floatColor)
			{ 
				m_color=floatColor;
				needToDelete=false;
			}
			//PROPERTIES
	public:
		// <summary>
		/// ctor copies value of input
		// </summary>
		MFloatColor(FloatColor floatColor)
		{ 
			m_color=new FloatColor(floatColor.r,floatColor.g,floatColor.b,floatColor.a);
			needToDelete=true;
		}

		// <summary>
		/// ctor
		// </summary>
		MFloatColor()
		{
			m_color=new FloatColor();
			needToDelete=true;
		}

		// <summary>
		/// ctor
		// </summary>
		MFloatColor(float r,float g,float b,float a)
		{
			m_color=new FloatColor(r,g,b,a);
			needToDelete=true;
		}

		// <summary>
		/// ctor generates floatcolor from long color
		// </summary>
		MFloatColor(unsigned long color)
		{
			m_color=new FloatColor(COLOR_GETRED(color)/255.0f,COLOR_GETGREEN(color)/255.0f,COLOR_GETBLUE(color)/255.0f,COLOR_GETALPHA(color)/255.0f);
			needToDelete=true;
		}

		virtual void Finalize()
		{
			if (needToDelete &&	m_color)
				delete m_color;
		}

		// <summary>
		/// sets all the floatcolor's component values
		// </summary>
		void Set(float red, float green, float blue, float alpha)
		{
			*m_color = FloatColor(red,green,blue,alpha);
		}
		
		void Clamp()
		{
			m_color->Clamp();
		}

		/// OPERATOR OVERLOADS
		static MFloatColor^ operator + ( MFloatColor^ first, MFloatColor^ second)
		{
			return gcnew MFloatColor(*first->m_color + *second->m_color);
		}
		/// OPERATOR OVERLOADS
		static MFloatColor^ operator - ( MFloatColor^ first, MFloatColor^ second)
		{
			return gcnew MFloatColor(*first->m_color - *second->m_color);
		}
		/// OPERATOR OVERLOADS
		static MFloatColor^ operator * ( MFloatColor^ first, MFloatColor^ second)
		{
			return gcnew MFloatColor((*first->m_color) * (*second->m_color));
		}
		/// OPERATOR OVERLOADS
		static MFloatColor^ operator * ( float first, MFloatColor^ second)
		{
			return gcnew MFloatColor((*second->m_color)*first);
		}

		// <summary>
		/// Red component
		// </summary>
		property float r 
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get()	
			{
				return m_color->r;
			}
			void set(float value) 
			{
				m_color->r=value;
			}
		}

		// <summary>
		/// Green component
		// </summary>
		property float g
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get()	
			{
				return m_color->g;
			}
			void set(float value) 
			{
				m_color->g=value;
			}
		}

		// <summary>
		/// Blue component
		// </summary>
		property float b
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get()	
			{
				return m_color->b;
			}
			void set(float value) 
			{
				m_color->b=value;
			}
		}

		// <summary>
		/// Alpha component
		// </summary>
		property float a
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get()	
			{
				return m_color->a;
			}
			void set(float value) 
			{
				m_color->a=value;
			}
		}

		// <summary>
		/// copies values to another mfloatcolor
		// </summary>
		void Copy(MFloatColor^ destination)
		{
			*destination->m_color=*m_color;
		}
	};

	// <summary>
	/// 3-float Vector
	// </summary>
	public ref class MVector 
	{
	public:
		Vector *	 m_vector;
		private	public:
			bool		needToDelete;

			MVector(Vector * vector)
			{ 
				m_vector=vector;
				needToDelete=false;
			}
			//PROPERTIES
	public:
		// <summary>
		/// ctor
		// </summary>
		MVector(Vector vector)
		{ 
			m_vector=new Vector(vector.x,vector.y,vector.z);
			needToDelete=true;
		}

		// <summary>
		/// ctor
		// </summary>
		MVector()
		{
			m_vector=new Vector(0,0,0);
			needToDelete=true;
		}

		// <summary>
		/// ctor -- set from components
		// </summary>
		MVector(float X,float Y,float Z)
		{
			m_vector=new Vector(X,Y,Z);
			needToDelete=true;
		}

		// <summary>
		/// ctor -- copies another vectors _values_
		// </summary>
		MVector(MVector^ vector)
		{
			m_vector=new Vector();
			*m_vector = *vector->m_vector;
			needToDelete=true;
		}

		virtual void Finalize()
		{
			if (needToDelete &&	m_vector)
				delete m_vector;
		}

		// <summary>
		/// x component
		// </summary>
		property float x
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get()	
			{
				return m_vector->x;
			}
			void set(float value) 
			{
				m_vector->x=value;
			}
		}

		// <summary>
		/// y component
		// </summary>
		property float y
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get()	
			{
				return m_vector->y;
			}
			void set(float value) 
			{
				m_vector->y=value;
			}
		}

		// <summary>
		/// z component
		// </summary>
		property float z
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get()	
			{
				return m_vector->z;
			}
			void set(float value) 
			{
				m_vector->z=value;
			}
		}

		// OPERATOR OVERLOADS
	public: 
		static MVector^ operator + ( MVector^ first, MVector^ second)
		{

			return gcnew MVector(first->m_vector->x + second->m_vector->x,
				first->m_vector->y + second->m_vector->y,
				first->m_vector->z + second->m_vector->z);
		}

		static MVector^ operator - ( MVector^ first, MVector^ second)
		{
			return gcnew MVector(first->m_vector->x - second->m_vector->x,
				first->m_vector->y - second->m_vector->y,
				first->m_vector->z - second->m_vector->z);
		}

		static MVector^ operator * ( MVector^ first, float second)
		{
			return gcnew MVector(first->m_vector->x * second,
				first->m_vector->y * second,
				first->m_vector->z * second);
		}
		static MVector^ operator * (float second, MVector^ first)
		{
			return gcnew MVector(first->m_vector->x * second,
				first->m_vector->y * second,
				first->m_vector->z * second);
		}

		static MVector^ operator * ( MVector^ first, MVector^ second)
		{
			return gcnew MVector(first->m_vector->x * second->m_vector->x,
				first->m_vector->y * second->m_vector->y,
				first->m_vector->z * second->m_vector->z);
		}

		static MVector^ operator / ( MVector^ first, float second)
		{
			return gcnew MVector(first->m_vector->x / second,
				first->m_vector->y / second,
				first->m_vector->z / second);
		}

		static bool operator == ( MVector^ first, MVector^ second)
		{
			if (Object::ReferenceEquals(first,nullptr) &&
				Object::ReferenceEquals(second,nullptr))
				return true;
			if (!Object::ReferenceEquals(first,nullptr) &&
				Object::ReferenceEquals(second,nullptr))
				return false;
			if (Object::ReferenceEquals(first,nullptr) &&
				!Object::ReferenceEquals(second,nullptr))
				return false;
			return *first->m_vector == *second->m_vector;
		}

		static bool operator != ( MVector^ first, MVector^ second)
		{
			if (Object::ReferenceEquals(first,nullptr) &&
				Object::ReferenceEquals(second,nullptr))
				return false;
			if (!Object::ReferenceEquals(first,nullptr) &&
				Object::ReferenceEquals(second,nullptr))
				return true;
			if (Object::ReferenceEquals(first,nullptr) &&
				!Object::ReferenceEquals(second,nullptr))
				return true;
			return *first->m_vector != *second->m_vector;
		}

		static MVector^ operator - ( MVector^ first)
		{
			return gcnew MVector(-first->m_vector->x,-first->m_vector->y,-first->m_vector->z);
		}

		//FUNCTIONS.

		// <summary>
		/// gets the yaw and pitch directions of the vector, in degrees
		// </summary>
		void GetAngles(float% Yaw, float% Pitch)
		{
			Yaw = RAD2DEG(atan2(m_vector->z,-m_vector->x)) - 90.f;
			Pitch = -(RAD2DEG(m_vector->RadAngle(Vector(0,-1,0))) - 90.f);
		}

		// <summary>
		/// 
		// </summary>
		float Length()
		{
			return m_vector->Length();
		}

		// <summary>
		/// returns whether the vector length is very close to zero within floating point inaccuracy
		// </summary>
		bool IsNearlyZero()
		{
			return (bool)m_vector->IsNearlyZero();
		}

		// <summary>
		/// returns whether the vector length is exactly zero
		// </summary>
		bool IsZero()
		{
			return m_vector->IsZero();
		}

		// <summary>
		/// normalizes the vector, returns false if zero length
		// </summary>
		bool Normalize()
		{
			return m_vector->Normalize();
		}

		void Set(float x, float y, float z) 
		{
			m_vector->Set(x,y,z);
		}

		// <summary>
		/// subtracts the projection of input vector onto this vector
		// </summary>
		void RemoveComponent(MVector^ vec)
		{
			float dot = Dot(vec);
			*m_vector -= dot*(*vec->m_vector);
		}

		// <summary>
		/// adds another vector from this vector
		// </summary>
		void Add(MVector^ v)
		{ 
			*m_vector += *v->m_vector;
		}

		// <summary>
		/// subtracts another vector from this vector
		// </summary>
		void Sub(MVector^ v)
		{ 
			*m_vector -= *v->m_vector;
		}

		// <summary>
		/// multiplies another vector by this vector
		// </summary>
		void Mul(MVector^ v)
		{ 
			*m_vector *= *v->m_vector;
		}

		// <summary>
		/// compares to the two vectors to determine if they are equal
		// </summary>
		bool Cmp(MVector^ v)
		{ 
			return *m_vector == *v->m_vector;
		}

		// <summary>
		/// returns the vector cross product
		// </summary>
		MVector^ cross(MVector^ v2)
		{
			return gcnew MVector(CrossNoNorm(*v2->m_vector, *m_vector));
		}

		// <summary>
		/// returns the closest point to this point on the line segment a/b
		// </summary>
		MVector^ ClosestPointOnLine(MVector^ segmentA,MVector^ segmentB)
		{  
			float d;  
			float t;

			Vector a = *segmentA->m_vector;
			Vector b = *segmentB->m_vector;
			Vector p = *m_vector;

			Vector v= (b - a).Normalized();

			d=v.Length();

			t = v.Dot(p - a) ;

			if (t<=0) 
				return gcnew MVector(a);

			if (t>=d) 
				return gcnew MVector(b);

			return gcnew MVector((a + (t*v)));
		}

		// <summary>
		/// returns the normalized vector cross product
		// </summary>
		MVector^ crossNormalized(MVector^ v2)
		{
			return gcnew MVector(Cross(*m_vector,*v2->m_vector));
		}

		//---------------------------------
		// Static functions
		//---------------------------------

		// <summary>
		/// makes vector from yaw-pitch-roll direction
		// </summary>	
		static MVector^ MakeDirection(float yaw, float pitch, float roll)
		{
			return gcnew MVector(Vector::MakeDirection(yaw,pitch,roll));
		}

		// <summary>
		/// calculates normal vector from triangle vertices
		// </summary>
		static MVector^ NormalFromTriangle(MVector^  v0,MVector^  v1,MVector^  v2)
		{
			return gcnew MVector(Vector::NormalFromTriangle(*v0->m_vector,*v1->m_vector,*v2->m_vector));
		}

		//---------------------------------
		// OTHER FUNCTIONS
		//---------------------------------

		// <summary>
		/// gets the yaw in degrees of the vector direction
		// </summary>
		double Yaw ()
		{
			return m_vector->Yaw();
		}

		// <summary>
		/// gets the pitch in degrees of the vector direction
		// </summary>
		double Pitch ()
		{
			return m_vector->Pitch();
		}

		// <summary>
		/// gets the roll in degrees
		// </summary>
		double Roll ()
		{
			return m_vector->Roll();
		}

		// <summary>
		/// zeroes the vector
		// </summary>
		void Zero()
		{
			m_vector->Zero();
		}

		// <summary>
		/// gets the Normal of this vector
		// </summary>
		MVector^ GetNormal()
		{
			return gcnew MVector(m_vector->GetNormal());
		}

		// <summary>
		/// returns the Normalized vector of this vector
		// </summary>
		MVector^  Normalized() 
		{
			return gcnew MVector(m_vector->Normalized());
		}

		// <summary>
		/// returns the dot product with itself
		// </summary>
		float  DotSelf() 
		{
			return m_vector->DotSelf();
		}

		// <summary>
		/// returns the dot product with another vector
		// </summary>
		float  Dot(MVector^ v)
		{
			return m_vector->Dot(*v->m_vector);
		}

		// <summary>
		/// returns the magnitude of the projection onto another vector
		// </summary>
		float  ScalarProjectionOntoVector(MVector^ v)
		{			
			return m_vector->ScalarProjectionOntoVector(*v->m_vector);
		}

		// <summary>
		/// returns the vector projection onto another vector
		// </summary>
		MVector^ ProjectionOntoVector(MVector^ v1)	
		{
			return gcnew MVector(m_vector->ProjectionOntoVector(*v1->m_vector));
		}

		// <summary>
		/// an alternative projection method, same results just done in managed code
		// </summary>
		MVector^ Project(MVector^ v1)	
		{

			//f32 dot = (*this).Dot(v);
			//return (v * dot);

			float dot = m_vector->Dot(*v1->m_vector);
			return gcnew MVector(*v1->m_vector * dot);
			//return gcnew

		}

		// <summary>
		/// returns the lerped value from this vector to the input vector based on T's value from 0 to 1
		// </summary>
		MVector^ Lerp(MVector^ v1, float t)
		{
			return gcnew MVector(m_vector->Lerp(*v1->m_vector,t));
		}

		// <summary>
		/// returns the angle in randians between this vector and the input vector
		// </summary>
		float  RadAngle(MVector^ v1) 
		{
			return  m_vector->RadAngle(*v1->m_vector);
		}

		// <summary>
		/// returns the angle in degrees betwee this vector and the input vector
		// </summary>
		float  CosAngle(MVector^ v1) 
		{
			return  m_vector->CosAngle(*v1->m_vector);
		}

		// <summary>
		/// returns the reflection of this vector onto a surface-normal vector
		// </summary>
		MVector^ Reflected(MVector^ n)
		{
			return gcnew MVector(m_vector->Reflected(*n->m_vector));
		}

		// <summary>
		/// 
		// </summary>
		float  DistanceToLine(MVector^ p0, MVector^ p1) 
		{
			return m_vector->DistanceToLine(*p0->m_vector,*p1->m_vector);
		}

		// <summary>
		/// returns the average float value of the vector's components
		// </summary>
		float  Average() 
		{
			return m_vector->Average();
		}

		// <summary>
		/// returns the midpoint between this vector and another vector
		// </summary>
		MVector^ HalfWay(MVector^ dest) 
		{
			return gcnew MVector(m_vector->HalfWay(*dest->m_vector));
		}

		// <summary>
		/// 
		// </summary>
		float CalcAngle(MVector^dir, MVector^ target)
		{
			return m_vector->CalcAngle(*dir->m_vector,*target->m_vector);
		}


		// <summary>
		/// Copies this vector's _values_ into another vector object
		// </summary>
		void Copy(MVector^ destination)
		{
			*destination->m_vector=*m_vector;
		}

		// <summary>
		/// Sets this vector's _values_ from another vector object
		// </summary>
		void Set(MVector^ source)
		{
			*m_vector = *source->m_vector;
		}
	};

	// <summary>
	/// 4x3 Matrix
	// </summary>
	public ref class MMatrix : MarshalByRefObject
	{
		private	public:
			Matrix*		m_matrix;
			bool		needToDelete;
			MVector^ m_m0;
			MVector^ m_m1;
			MVector^ m_m2;
			MVector^ m_m3;

	public:

		// <summary>
		/// ctor from unmanaged pointer, won't delete the unmanaged object
		// </summary>
		MMatrix(Matrix * matrix)
		{ 
			m_matrix=matrix;
			if (matrix)
			{
				m_m0=gcnew MVector(&matrix->m0);
				m_m1=gcnew MVector(&matrix->m1);
				m_m2=gcnew MVector(&matrix->m2);
				m_m3=gcnew MVector(&matrix->m3);
			}
			needToDelete=false;
		}

		// <summary>
		/// ctor for copying another unmanaged vector's values
		// </summary>
		MMatrix(Matrix matrix)
		{ 
			m_matrix=new Matrix(matrix);
			m_m0=gcnew MVector(&m_matrix->m0);
			m_m1=gcnew MVector(&m_matrix->m1);
			m_m2=gcnew MVector(&m_matrix->m2);
			m_m3=gcnew MVector(&m_matrix->m3);
			needToDelete=true;
		}

		// <summary>
		/// ctor for creating a new managed matrix with identity values
		// </summary>
		MMatrix()
		{ 
			m_matrix=new Matrix();
			m_m0=gcnew MVector(&m_matrix->m0);
			m_m1=gcnew MVector(&m_matrix->m1);
			m_m2=gcnew MVector(&m_matrix->m2);
			m_m3=gcnew MVector(&m_matrix->m3);
			needToDelete=true;
		}

		// <summary>
		/// ctor for copying another managed vector's values
		// </summary>
		MMatrix(MMatrix^ matrix)
		{
			m_matrix=new Matrix(*matrix->m_matrix);
			m_m0=gcnew MVector(&m_matrix->m0);
			m_m1=gcnew MVector(&m_matrix->m1);
			m_m2=gcnew MVector(&m_matrix->m2);
			m_m3=gcnew MVector(&m_matrix->m3);
			needToDelete=true;
		}

		// <summary>
		/// dtor
		// </summary>
		virtual void Finalize()
		{	  
			if (needToDelete && m_matrix)
				delete m_matrix;
		}

		//PROPERTIES
	public:

		// <summary>
		/// m0 row
		// </summary>
		property MVector^ m0
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			MVector^ get()
			{
				return m_m0;
			}

			void set(MVector^ value)
			{
				*m_m0->m_vector=*value->m_vector;
			}
		}

		// <summary>
		/// m1 row
		// </summary>
		property MVector^ m1
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			MVector^ get()
			{
				return m_m1;
			}

			void set(MVector^ value)
			{
				*m_m1->m_vector=*value->m_vector;
			}
		}


		// <summary>
		/// m2 row
		// </summary>
		property MVector^ m2
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			MVector^ get()
			{
				return m_m2;
			}

			void set(MVector^ value)
			{
				*m_m2->m_vector=*value->m_vector;
			}
		}


		// <summary>
		/// m3 row
		// </summary>
		property MVector^ m3
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			MVector^ get()
			{
				return m_m3;
			}

			void set(MVector^ value)
			{
				*m_m3->m_vector=*value->m_vector;
			}
		}

		// <summary>
		/// p0 column
		// </summary>
		property float p0
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get()	
			{
				return m_matrix->p0;
			}
			void set(float value) 
			{
				m_matrix->p0=value;
			}
		}

		// <summary>
		/// p1 column
		// </summary>
		property float p1
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get()	
			{
				return m_matrix->p1;
			}
			void set(float value) 
			{
				m_matrix->p1=value;
			}
		}

		// <summary>
		/// p2 column
		// </summary>
		property float p2
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get()	
			{
				return m_matrix->p2;
			}
			void set(float value) 
			{
				m_matrix->p2=value;
			}
		}

		// <summary>
		/// p3 column
		// </summary>
		property float p3
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get()	
			{
				return m_matrix->p3;
			}
			void set(float value) 
			{
				m_matrix->p3=value;
			}
		}

		//---------------------------------
		// helper functions
		//---------------------------------


		// <summary>
		/// gets the forward-direction unit vector of the transformation matrix, no scaling
		// </summary>
		MVector^ GetDir()
		{
			return gcnew MVector(m_matrix->GetDir());
		}


		// <summary>
		/// gets the right-direction unit vector of the transformation matrix, no scaling
		// </summary>
		MVector^ GetRight()
		{
			return gcnew MVector(m_matrix->GetRight());
		}


		// <summary>
		/// gets the up-direction unit vector of the transformation matrix, no scaling
		// </summary>
		MVector^ GetUp()
		{
			return gcnew MVector(m_matrix->GetUp());
		}


		// <summary>
		/// sets the rotation Matrix based on yaw, pitch and roll rotations. This will erase any previous values that the Matrix has.
		// </summary>
		void SetRotations(float x, float y, float z)
		{
			m_matrix->SetRotations(x,y,z);
		}


		// <summary>
		/// gets the rotation matrix -- no scaling or translation
		// </summary>
		MMatrix^ GetRotationMatrix()
		{
			return gcnew MMatrix(m_matrix->GetRotationMatrix());
		}

		// <summary>
		/// rotates the Matrix by relative pitch (X), yaw (Y), and roll (Z) degree values contained in a vector
		// </summary>
		void Rotate(MVector^ vectorDegrees);

		// <summary>
		/// rotates the Matrix by relative pitch, yaw, and roll degree values
		// </summary>
		void Rotate(float PitchDeg, float YawDeg, float RollDeg);

		// <summary>
		/// orthonormalized this matrix, useful to do after lerping/interpolating Matrix values to prevent scaling distortion
		// </summary>
		void Orthonormalize()
		{
			m_matrix->Orthonormalize();
		}


		// <summary>
		/// returns the inverse of this matrix
		// </summary>
		MMatrix^ Inverse()
		{
			return gcnew MMatrix(m_matrix->Inverse());
		}

		// <summary>
		/// transforms the vector by the Matrix
		// </summary>
		MVector^ TransformVector3Matrix3(MVector^ mvec)
		{
			Vector mv;
			m_matrix->TransformVector3Matrix3(*mvec->m_vector,mv);
			return gcnew MVector(mv);
		}


		// <summary>
		/// copies this Matrix's _values_ to the input Matrix object
		// </summary>
		void Copy(MMatrix^ destination)
		{
			*destination->m_matrix=*m_matrix;
		}

		//---------------------------------
		// Static functions
		//---------------------------------


		// <summary>
		/// Returns a new Matrix oriented towards the specified direction vector
		// </summary>
		static MMatrix^ LookTowards(MVector^ mvector)
		{
			return gcnew MMatrix(Matrix::LookTowards(*mvector->m_vector));
		}


		// <summary>
		/// Returns a new Matrix oriented towards the specified point in World space from the specified vantage point
		// </summary>
		static MMatrix^ LookAt(MVector^ FromPoint, MVector^ ToPoint, MVector^ UpVec)
		{
			MMatrix^ transform = gcnew MMatrix();
			D3DXMatrixLookAtLH((D3DXMATRIX*)transform->m_matrix,(D3DXVECTOR3*)FromPoint->m_vector,(D3DXVECTOR3*)ToPoint->m_vector,(D3DXVECTOR3*)UpVec->m_vector);
			return transform;
		}

		//---------------------------------
		// OVERLOADS
		//---------------------------------

		static MMatrix^ operator * ( MMatrix^ first, MMatrix^ second)
		{
			return gcnew MMatrix((*first->m_matrix)*(*second->m_matrix));
		}
		static MMatrix^ operator - ( MMatrix^ first, MMatrix^ second)
		{
			return gcnew MMatrix((*first->m_matrix)-(*second->m_matrix));
		}
		static MMatrix^ operator + ( MMatrix^ first, MMatrix^ second)
		{
			return gcnew MMatrix((*first->m_matrix)+(*second->m_matrix));
		}

		static MMatrix^ operator * ( MMatrix^ first, float second)
		{
			return gcnew MMatrix((*first->m_matrix)*second);
		}
		static MMatrix^ operator / ( MMatrix^ first, float second)
		{
			return gcnew MMatrix((*first->m_matrix)/second);
		}

		static MVector^ operator * ( MMatrix^ first, MVector^ second)
		{
			return gcnew MVector((*first->m_matrix)*(*second->m_vector));
		}
		//---------------------------------
		//---------------------------------
	};

	// <summary>
	/// Quaternion
	// </summary>
	public ref class MQuaterion
	{
	public:
		Vector4 *	 m_vector;
		private	public:
			bool		needToDelete;


			//PROPERTIES
	public:


		// <summary>
		/// ctor takes values from an inputted Vector4
		// </summary>
		MQuaterion(Vector4 vector)
		{ 
			m_vector=new Vector4(vector.x,vector.y,vector.z,vector.w);
			needToDelete=true;
		}

		// <summary>
		/// ctor identity values
		// </summary>
		MQuaterion()
		{
			m_vector=new Vector4(0,0,0,0);
			needToDelete=true;
		}

		// <summary>
		/// ctor values from floats
		// </summary>
		MQuaterion(float X,float Y,float Z,float W)
		{
			m_vector=new Vector4(X,Y,Z,W);
			needToDelete=true;
		}

		// <summary>
		/// dtor
		// </summary>
		virtual void Finalize()
		{
			if (needToDelete &&	m_vector)
				delete m_vector;
		}

	public: 
		// <summary>
		/// sets axis of the Quat
		// </summary>
		void SetAngleAxis(float angle, float x , float y, float z) {
			float halfAngle = 0.5f * angle;
			float sin = (float)Math::Sin(halfAngle);

			m_vector->w = (float)Math::Cos(halfAngle);
			m_vector->x = sin * x; 
			m_vector->y = sin * y; 
			m_vector->z = sin * z; 
		}

		// <summary>
		/// converts quat to a transformation matrix
		// </summary>
		MMatrix^ GetRotation()
		{
			float xx      = m_vector->x * m_vector->x;
			float xy      = m_vector->x * m_vector->y;
			float xz      = m_vector->x * m_vector->z;
			float xw      = m_vector->x * m_vector->w;

			float yy      = m_vector->y * m_vector->y;
			float yz      = m_vector->y * m_vector->z;
			float yw      = m_vector->y * m_vector->w;

			float zz      = m_vector->z * m_vector->z;
			float zw      = m_vector->z * m_vector->w;

			Matrix m;
			m.m0.x  = 1 - 2 * ( yy + zz );
			m.m0.y =     2 * ( xy - zw );
			m.m0.z  =     2 * ( xz + yw );

			m.m1.x  =     2 * ( xy + zw );
			m.m1.y  = 1 - 2 * ( xx + zz );
			m.m1.z  =     2 * ( yz - xw );

			m.m2.x  =     2 * ( xz - yw );
			m.m2.y  =     2 * ( yz + xw );
			m.m2.z = 1 - 2 * ( xx + yy );
			return gcnew MMatrix(m);
		}

		// <summary>
		/// x component
		// </summary>
		property float x
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get()	
			{
				return m_vector->x;
			}
			void set(float value) 
			{
				m_vector->x=value;
			}
		}

		// <summary>
		/// y component
		// </summary>
		property float y
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get()	
			{
				return m_vector->y;
			}
			void set(float value) 
			{
				m_vector->y=value;
			}
		}

		// <summary>
		/// z component
		// </summary>
		property float z
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get()	
			{
				return m_vector->z;
			}
			void set(float value) 
			{
				m_vector->z=value;
			}
		}


		// <summary>
		/// w component
		// </summary>
		property float w
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get()	
			{
				return m_vector->w;
			}
			void set(float value) 
			{
				m_vector->w=value;
			}
		}

		// <summary>
		/// 
		// </summary>
		static MQuaterion^ operator * ( MQuaterion^ left, MQuaterion^ right)
		{
			MQuaterion^ q = gcnew MQuaterion();
			q->w = left->w * right->w - left->x * right->x - left->y * right->y - left->z * right->z;
			q->x = left->w * right->x + left->x * right->w + left->y * right->z - left->z * right->y;
			q->y = left->w * right->y + left->y * right->w + left->z * right->x - left->x * right->z;
			q->z = left->w * right->z + left->z * right->w + left->x * right->y - left->y * right->x;

			return q;
		}

	};




	// <summary>
	/// 4-float Vector
	// </summary>
	public ref class MVector4 
	{
	public:
		Vector4 *	 m_vector;
		private	public:
			bool		needToDelete;

			MVector4(Vector4 * vector)
			{ 
				m_vector=vector;
				needToDelete=false;
			}
			//PROPERTIES
	public:


		// <summary>
		/// ctor copies values froma vector4
		// </summary>
		MVector4(Vector4 vector)
		{ 
			m_vector=new Vector4(vector.x,vector.y,vector.z,vector.w);
			needToDelete=true;
		}


		// <summary>
		/// ctor identity values
		// </summary>
		MVector4()
		{
			m_vector=new Vector4(0,0,0,0);
			needToDelete=true;
		}

		// <summary>
		/// ctor takes values from floats
		// </summary>
		MVector4(float X,float Y,float Z,float W)
		{
			m_vector=new Vector4(X,Y,Z,W);
			needToDelete=true;
		}

		// <summary>
		/// dtor
		// </summary>
		virtual void Finalize()
		{
			if (needToDelete &&	m_vector)
				delete m_vector;
		}

	public: 
		// <summary>
		/// converts to a rotation matrix
		// </summary>
		MMatrix^ GetRotation()
		{
			float xx      = m_vector->x * m_vector->x;
			float xy      = m_vector->x * m_vector->y;
			float xz      = m_vector->x * m_vector->z;
			float xw      = m_vector->x * m_vector->w;

			float yy      = m_vector->y * m_vector->y;
			float yz      = m_vector->y * m_vector->z;
			float yw      = m_vector->y * m_vector->w;

			float zz      = m_vector->z * m_vector->z;
			float zw      = m_vector->z * m_vector->w;

			Matrix m;
			m.m0.x  = 1 - 2 * ( yy + zz );
			m.m0.y =     2 * ( xy - zw );
			m.m0.z  =     2 * ( xz + yw );

			m.m1.x  =     2 * ( xy + zw );
			m.m1.y  = 1 - 2 * ( xx + zz );
			m.m1.z  =     2 * ( yz - xw );

			m.m2.x  =     2 * ( xz - yw );
			m.m2.y  =     2 * ( yz + xw );
			m.m2.z = 1 - 2 * ( xx + yy );
			return gcnew MMatrix(m);
		}

		// <summary>
		/// x component
		// </summary>
		property float x
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get()	
			{
				return m_vector->x;
			}
			void set(float value) 
			{
				m_vector->x=value;
			}
		}

		// <summary>
		/// y component
		// </summary>
		property float y
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get()	
			{
				return m_vector->y;
			}
			void set(float value) 
			{
				m_vector->y=value;
			}
		}

		// <summary>
		/// z component
		// </summary>
		property float z
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get()	
			{
				return m_vector->z;
			}
			void set(float value) 
			{
				m_vector->z=value;
			}
		}


		// <summary>
		/// w component
		// </summary>
		property float w
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get()	
			{
				return m_vector->w;
			}
			void set(float value) 
			{
				m_vector->w=value;
			}
		}

		static MVector4^ operator * ( MVector4^ first, MVector4^ second)
		{
			return gcnew MVector4(first->m_vector->x * second->m_vector->x,
				first->m_vector->y * second->m_vector->y,
				first->m_vector->z * second->m_vector->z,first->m_vector->w * second->m_vector->w);
		}

	};

	// <summary>
	/// Returns information about the material type names, used for functional differentiation of interaction with surfaces based on their material 'type', e.g. "metal, "wood", "grass", "snow"
	// </summary>
	public ref class MMaterialManager
	{
	public:

		// <summary>
		/// Gets the number of material type names currently loaded in Reality
		// </summary>
		static property int NumMaterialTypes
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			int get()
			{
				return MaterialManager::Instance()->m_MaterialTypes.size();
			}
		}

		// <summary>
		/// Gets the material-type name string based on set index
		// </summary>
		static String^ GetMaterialType(int index)
		{
			if(index < 0 || index > MaterialManager::Instance()->m_MaterialTypes.size())
				return "";

			return Helpers::ToCLIString(MaterialManager::Instance()->m_MaterialTypes[index]);
		}
	};


	// <summary>
	/// Wraps and provides access to the capabilities of Reality's Material. A material wholly encapsulates the surface properties of any object
	// </summary>
	public ref class MMaterial
	{
		private public:
			Material* m_material;
			bool needToDelete;
	public:

		// <summary>
		/// ctor wraps an existing unmanaged Reality material
		// </summary>
		MMaterial(Material* material)
		{
			m_material = material;
			needToDelete = false;
		}

		// <summary>
		/// ctor creates a new Reality Material to associate with this object
		// </summary>
		MMaterial(String^ name)
		{
			m_material = new Material(Helpers::ToCppString(name));
			needToDelete = true;
		}

		// <summary>
		/// dtor
		// </summary>
		virtual void Finalize()
		{
			if (needToDelete &&	m_material)
				m_material->Release();
		}

		// <summary>
		/// Loads the material parameters from an external Material XML file, expected to reside in "Materials" directory unless path otherwise specified.
		// </summary>
		void Load(String^ MaterialFile)
		{
			m_material->Load(Helpers::ToCppString(MaterialFile).c_str());
		}

		// <summary>
		/// Adds one reference count to the Material -- A Material will only be auto-deleted when all its references are Released.
		// </summary>
		void AddRef()
		{
			m_material->AddRef();
		}

		// <summary>
		/// Releases one reference count to the Material -- A Material will only be auto-deleted when all its references are Released.
		// </summary>
		void Release()
		{
			m_material->Release();
			needToDelete = false;
		}

		// <summary>
		/// The type-set name of the Material, used for functional differentiation of interaction with surfaces based on their material type
		// </summary>		
		property String^ type
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			String^ get()
			{
				return Helpers::ToCLIString(m_material->m_Type);
			}
		}
	};

	// <summary>
	/// Wraps and provides
	// </summary>
	public ref class MModelFrame
	{
		private public:
			ModelFrame* m_modelframe;
			MMatrix^ m_TransformationMatrix;
			MMatrix^ m_CombinedTransformationMatrix;
			String^ m_Name;
	public:

		// <summary>
		/// Wraps and provides access to a ModelFrame, which is a hierarchially transformed element in a Model, can contain a Mesh or represent an animation bone
		// </summary>
		MModelFrame(ModelFrame* modelFrame)
		{
			m_modelframe = modelFrame;
			m_Name = Helpers::ToCLIString(modelFrame->Name);
			m_TransformationMatrix = gcnew MMatrix(&m_modelframe->TransformationMatrix);
			m_CombinedTransformationMatrix = gcnew MMatrix(&m_modelframe->CombinedTransformationMatrix);
		}

		// <summary>
		/// The name of the modelframe
		// </summary>
		property String^ Name
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			String^ get()
			{
				return m_Name;
			}
		}

		// <summary>
		/// the current local transformation of the modelframe with respect to its parent
		// </summary>
		property MMatrix^ TransformationMatrix
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			MMatrix^ get()
			{
				return m_TransformationMatrix;
			}
			void set(MMatrix^ value)
			{
				*m_TransformationMatrix->m_matrix=*value->m_matrix;
			}
		}

		// <summary>
		/// the current absolute world transformation of the modelframe
		// </summary>
		property MMatrix^ CombinedTransformationMatrix
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			MMatrix^ get()
			{
				return m_CombinedTransformationMatrix;
			}
			void set(MMatrix^ value)
			{
				*m_CombinedTransformationMatrix->m_matrix=*value->m_matrix;
			}
		}

		// <summary>
		/// returns all the children and sibling nodes of this modelframe which contain meshes (3d geometry)
		// </summary>
		ArrayList^ EnumerateMeshes();

		// <summary>
		/// returns all the children and sibling nodes of this modelframe, even if they don't have meshes
		// </summary>
		ArrayList^ EnumerateFrames();


		// <summary>
		/// Sets the current Material of this ModelFrame, will remove one reference to the previously applied Material on the ModelFrame, if any.
		// </summary>
		void SetMaterial(int MatIndex,MMaterial^ material);

		// <summary>
		/// Updates the transformation of all this frame's children frames
		// </summary>
		void UpdateChildrenMatrices()
		{
			m_modelframe->UpdateChildrenMatrices();
		}

		// <summary>
		/// Gets the current Material of this ModelFrame
		// </summary>
		MMaterial^ GetMaterial(int MatIndex);
	};

	// <summary>
	/// Wraps and provides access to a Model, which is an instance of a static mesh hierarchy
	// </summary>
	public ref class MModel	: MarshalByRefObject
	{
		private	public:
			bool		needToDelete;
			Model *		m_model;
			MMatrix^	m_rootTransform;

			MModel(Model * model)
			{ 
				m_model=model;
				m_rootTransform = gcnew MMatrix(&m_model->m_RootTransform);
				needToDelete=false;
			}
			//PROPERTIES
	public:

		// <summary>
		/// ctor creating a new managed model, will not automatically delete the low-level unmanaged model upon disposal, since most Models are bound to Actors which delete their MyModels
		// </summary>
		MModel()
		{
			m_model=new	Model();
			m_rootTransform = gcnew MMatrix(&m_model->m_RootTransform);
			needToDelete=false;
		}

		// <summary>
		/// ctor creating a new model object, that will automatically delete the low-level unmanaged model upon disposal. Only use when Model is not to be set as MyModel for an Actor.
		// </summary>
		MModel(bool forceDeletion)
		{
			m_model=new	Model();
			m_rootTransform = gcnew MMatrix(&m_model->m_RootTransform);
			needToDelete=forceDeletion;
		}

		// <summary>
		/// Loads model data from an XML or an X file, expected to reside in "Models" directory unless other path specified
		// </summary>
		void Load(String^ fileName,bool	deepCopy)
		{
			try
			{
				m_model->Load(Helpers::ToCppString(fileName).c_str(),deepCopy);
			}
			catch (Exception^ ex) 
			{
				System::Diagnostics::Debug::WriteLine("---Exception	While Loading a	Model:");
				System::Diagnostics::Debug::WriteLine(ex->Source);
				System::Diagnostics::Debug::WriteLine(ex->StackTrace);
				System::Diagnostics::Debug::WriteLine(ex->Message);
			}
		}


		// <summary>
		/// Returns whether the Model has loaded data from a file.
		// </summary>
		bool IsLoaded()
		{
			return m_model->IsLoaded();
		}

		// <summary>
		/// Loads an animation from an X file with specified looping behavior, returns the animation track index of the loaded animation.
		// </summary>
		int LoadAnimation(String^ animationFile, bool looping)
		{
			return (int)m_model->LoadAnimation(Helpers::ToCppString(animationFile),looping);
		}

		// <summary>
		/// Loads an animation from an X file with specified looping behavior, and a list of which ModelFrame Name strings to affect. Useful for hierarchial animations that only affect part of the character's body, such as differentiating between upper body and lower body animations.
		// </summary>
		int LoadAnimation(String^ animationFile, bool looping, ArrayList^ AffectedFrameNames);


		// <summary>
		/// Creates a new shallow copy of the model, very fast. Shallow copies can be transformed and culled uniquely, and even play unique animations using static animation track indices!
		// </summary>
		void CreateNewInstance(MModel^ model)
		{
			m_model->CreateNewInstance(model->m_model);
		}

		// <summary>
		/// Sets the transformation of the Model that will be used for rendering
		// </summary>
		void SetTransform(MMatrix^ Rotation,MVector^ Location)
		{
			m_model->SetTransform(*Rotation->m_matrix,*Location->m_vector);
		}

		// <summary>
		/// Sets the transformation of the Model that will be used for rendering
		// </summary>
		void SetTransform(MMatrix^ Transform)
		{
			m_model->SetTransform(*Transform->m_matrix);
		}

		// <summary>
		/// Pauses all the Model's animation, if any
		// </summary>
		void Pause()
		{
			m_model->Pause();
		}

		// <summary>
		/// Unpauses all the Model's animation, if any
		// </summary>
		void Unpause()
		{
			m_model->UnPause();
		}

		// <summary>
		/// Updates the Model's hierarchial transformations based on the scene transformation (which itself can be set with SetTransform)
		// </summary>
		void Update()
		{
			m_model->Update();
		}

		// <summary>
		/// Gets a ModelFrame in the Model's hierarchy by name, if any
		// </summary>
		MModelFrame^ GetModelFrame(String^ frameName)
		{
			ModelFrame* frame = (ModelFrame*)m_model->GetNodeHandle(Helpers::ToCppString(frameName));
			if(!frame)
			{
				//SeriousWarning("Model Frame '%s' not found in %s",Helpers::ToCppString(frameName).c_str(),m_model->m_FileName.c_str());
				return nullptr;
			}

			return gcnew MModelFrame(frame);
		}

		// <summary>
		/// Gets the root frame, the parent of all modelframes, which contains the base scene transformation
		// </summary>
		MModelFrame^ GetRootFrame()
		{
			return gcnew MModelFrame(m_model->m_pFrameRoot);
		}

		// <summary>
		/// Gets the number of currently loaded animations in the Model
		// </summary>
		int GetNumAnimations()
		{
			return m_model->GetNumAnimations();
		}

		// <summary>
		/// Returns whether the most recently played animation is currently still playing
		// </summary>
		bool IsPlaying()
		{
			return m_model->IsPlaying();
		}

		// <summary>
		/// Returns whether a specified track index is currently still playing
		// </summary>
		bool IsPlaying(int AnimationIndex)
		{
			return m_model->IsPlaying(AnimationIndex);
		}

		// <summary>
		/// Starts play of a specified animation track at full weighting, will stop all other animation tracks
		// </summary>
		void TransitionToAnimation(int AnimationIndex, float TransitionTime)
		{
			m_model->TransitionToAnimation((ANIMATIONHANDLE)AnimationIndex,TransitionTime);
		}

		// <summary>
		/// Starts play of a specified animation track at variable weighting, will continue to play other animation tracks at their current weighting
		// </summary>
		void TransitionToAnimation(int AnimationIndex, float TransitionTime, float Weighting, float PlaySpeed)
		{
			m_model->TransitionToAnimation((ANIMATIONHANDLE)AnimationIndex,TransitionTime,Weighting,0,PlaySpeed);
		}

		// <summary>
		/// Start play of a specified animation track from a specified time in the track, at variable weighting.
		// </summary>
		void TransitionToAnimation(int AnimationIndex, float TransitionTime, float Weighting, float StartAtTime, float PlaySpeed)
		{
			m_model->TransitionToAnimation((ANIMATIONHANDLE)AnimationIndex,TransitionTime,Weighting,StartAtTime,PlaySpeed);
		}

		// <summary>
		/// Adjusts the weighting value, over time, of the specified animation track.
		// </summary>
		void SetAnimationWeight(int AnimationIndex, float Weighting, float TransitionTime)
		{
			m_model->SetAnimationWeight((ANIMATIONHANDLE)AnimationIndex,Weighting,TransitionTime);
		}

		// <summary>
		/// Gets the most recently played (transitioned to) animation track
		// </summary>
		int GetCurrentAnimation()
		{
			return (int)m_model->GetCurrentAnimation();
		}

		// <summary>
		/// Sets the uniform scaling value for X animation loads, should be equal to the Reality Builder X import scaling value for animated models
		// </summary>
		void SetLoadingScale(float scale)
		{
			m_model->SetLoadingScale(scale);
		}

		// <summary>
		/// Returns the duration in seconds of the specified animation track
		// </summary>
		float GetDuration(int AnimationIndex)
		{
			return m_model->GetDuration((ANIMATIONHANDLE)AnimationIndex);
		}

		// <summary>
		/// Returns the remaining time, if any, of the specified non-looping animation track's current play
		// </summary>
		float GetRemaining(int AnimationIndex)
		{
			return m_model->GetRemaining((ANIMATIONHANDLE)AnimationIndex);
		}

		// <summary>
		/// Returns whether the specified animation track is a looping track
		// </summary>
		bool IsLooping(int AnimationIndex)
		{
			return m_model->IsLooping((ANIMATIONHANDLE)AnimationIndex);
		}

		// <summary>
		/// Queues the current Model for drawing in the world at its currently set transformation. Only necessary to call for Models which aren't MyModel on Actor, in order to draw them manually.
		// </summary>
		void Draw(ref class MWorld^ world);

		// <summary>
		/// Immediately draws the current Model in the World. Useful for z-buffer staged drawing, but slower than regular Draw() so don't call superfluously.
		// </summary>
		void DrawImmediate(ref class MWorld^ world);

		// <summary>
		/// Queues the current Model for drawing in the world at its currently set transformation. Passes an owner Actor to Draw() for usage of its owner's per-Light inclusion/exclusion results and indoor/outdoor state.
		// </summary>
		void Draw(ref class MWorld^ world, ref class MActor^ owner);

		// <summary>
		/// Immediately draws the current Model in the World. Passes an owner Actor to Draw() for usage of its owner's per-Light inclusion/exclusion results and indoor/outdoor state.
		// </summary>
		void DrawImmediate(ref class MWorld^ world, ref class MActor^ owner);

		// <summary>
		/// Draw box hierarchy showing all nodes. Used for skeletal visualization
		// </summary>
		void DrawHierarchy(){ m_model->DrawHierarchy(); }

		// <summary>
		/// Sets a relative transformation that influences the frame's own transformation
		// </summary>
		void SetFrameInfluence(MModelFrame^ frame, MMatrix^ influence)
		{
			if(frame != nullptr)
				m_model->SetNodeInfluence((NODEHANDLE)frame->m_modelframe,*influence->m_matrix);
		}

		// <summary>
		/// Gets the maxs (half) vector size of the Model's bbox currently transformed in the World. (if you want the bbox at the origin, you can transform the model to identity matrix first, or get the bbox of its local frame)
		// </summary>
		MVector^ GetWorldBBoxMax()
		{
			return gcnew MVector(m_model->GetWorldBBox().max);
		}

		// <summary>
		/// Returns the current RootTransform of the Model, which is its top-hierarchy transformation in the World. SetTransform sets this.
		// </summary>
		property MMatrix^ RootTransform
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			MMatrix^ get()
			{
				return m_rootTransform;
			}
			void set(MMatrix^ value)
			{
				*m_rootTransform->m_matrix = *value->m_matrix;
			}
		}

		// <summary>
		/// dtor
		// </summary>
		virtual void Finalize()
		{
			Dispose(false);
			disposed = true;

		}		

		// <summary>
		/// unmanaged resource disposal
		// </summary>
		void Dispose() 
		{
			Dispose(true);
			disposed = true;
			GC::SuppressFinalize(this);
		}
	protected:
		bool disposed;

		// <summary>
		/// unmanaged resource disposal
		// </summary>
		virtual void Dispose(bool disposing) 
		{
			if (!disposed) 
			{
				if (disposing) 
				{
					// Dispose managed resources.
				}
				// Dispose unmanaged resources
				if (needToDelete &&	m_model)
					delete m_model;
			}
		}
	};


	// <summary>
	/// Contains collision information returned by a ray check or a rigid body collision
	// </summary>
	public ref struct MCollisionInfo
	{

		// <summary>
		/// Actual distance from nearest plane, even if we never touched it
		// </summary>
		double actualDistance; 

		// <summary>
		/// Plane normal
		// </summary>
		ref class  MVector^ normal;	


		// <summary>
		/// Intersection point (rarely used)
		// </summary>
		ref class  MVector^ point;	


		// <summary>
		/// Touched actor, if any
		// </summary>
		ref class  MActor^ touched;	

		// <summary>
		/// Material touched
		// </summary>
		ref class MMaterial^ mat; 

		// <summary>
		/// ctor
		// </summary>
		MCollisionInfo(){
			touched = nullptr;
			mat = nullptr;
		}

	};

	// <summary>
	/// Different types of volumes that can be set up in the World and checked against, particularly for environmental region-specific functionality or indoor/outdoor differentiation
	// </summary>
	public enum class MVolumeType
	{
		VOLUME_INDOORLIGHTING = 0,
		VOLUME_INDOORPARTICLES = 1
	};

	// <summary>
	/// Wraps and provides access to a Reality World, which contains Actors that make up the gameplay, every Actor generally has a World.
	// </summary>
	public ref class MWorld	: MarshalByRefObject
	{
		private	public:
			static Hashtable^	s_worlds;
			static int			s_worldsKey;

			World*				m_world;
			//FUNCTIONS


			// <summary>
			/// 
			// </summary>
			static MWorld()
			{
				s_worlds = gcnew Hashtable();
				s_worldsKey=0;
			}

			// <summary>
			/// ctor wraps unmanaged World
			// </summary>
			MWorld(World * world)
			{ 
				m_world=world;
				s_worlds->Add(s_worldsKey,this);
				m_world=world;
				m_world->m_managedIndex	= s_worldsKey;
				s_worldsKey++;
				MActors = gcnew ArrayList();
			}
	public:

		// <summary>
		/// a list of all C# MActors in this World, can be used for script-side iteration or enumeration of all MActors in the world
		// </summary>
		ArrayList^ MActors;

		// <summary>
		/// 
		// </summary>
		World* GetUnManagedPointer()
		{
			return m_world;
		}

		// <summary>
		/// Whether the World is being run in Server-mode or not. This value is important for programming aspects of different behavior in Actors between the Client and Server
		// </summary>
		property bool IsServer
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			bool get()
			{
				return m_world->m_IsServer;
			}
		}

		// <summary>
		/// The filename of this World, if loaded
		// </summary>
		property String^ FileName
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			String^ get()
			{
				return Helpers::ToCLIString(m_world->m_FileName);
			}
		}

		// <summary>
		/// Runs a ray check between the specified points, ignoring a caster Actor, if any. Takes an already-initialized CollisionInfo reference that it will fill out with the resulting data if the ray collides with anything.
		// </summary>
		bool CollisionCheckRay(MActor^ sourceToIngore, MVector^ start, MVector^ end, MCheckType checkType, MCollisionInfo^ mResult);

		// <summary>
		/// Returns whether a vector point in the World is "indoors" or not, testing it against any IndoorVolumes of type VOLUME_INDOORLIGHTING that are set up in the World
		// </summary>
		bool IsIndoors(MVector^ location);

		// <summary>
		/// Returns whether a vector point in the World is inside an IndoorVolume of a specific VolumeType (collision of weather system particles for instance), testing it against any IndoorVolumes of that particle type that are set up in the World
		// </summary>
		bool IsInVolumeType(MVector^ location, MVolumeType volumeType);

		// <summary>
		/// Gets the color of the averaged lighting at a point in the World space, predicated on whether that point is specified as Indoors or Outdoors.
		// </summary>
		unsigned long GetAverageLighting(MVector^ location, bool Indoors, bool Outdoors);

		// <summary>
		/// Runs Reality Engine collide-&-slide physics on an Actor over the current DeltaTime. Note this is automatically done for Actors in their base.Tick(), this function just provides the ability to manually call it in case you do not wish to use the base.Tick() physics behavior.
		// </summary>
		void RunPhysics(MActor^ actor);
	};


	// <summary>
	/// Data pertaining to the state of a particular Light, used directly by the Shaders
	// </summary>
	public ref class MLightState 
	{
		private	public:
			LightState *	 m_lightState;
			bool		needToDelete;

			// <summary>
			/// ctor
			// </summary>
			MLightState(LightState * lightState)
			{ 
				m_lightState=lightState;
				needToDelete=false;
			}
			//PROPERTIES
	public:

		// <summary>
		/// Diffuse color of the Light
		// </summary>
		property MFloatColor^ Diffuse
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			MFloatColor^ get()
			{
				return gcnew MFloatColor(&m_lightState->Diffuse);
			}

			void set(MFloatColor^ value)
			{
				m_lightState->Diffuse=*value->m_color;
			}
		}

		// <summary>
		/// Specular color of the Light (depreciated, currently handled in shaders by RGB spec map)
		// </summary>
		property MFloatColor^ Specular
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			MFloatColor^ get()
			{
				return gcnew MFloatColor(&m_lightState->Specular);
			}

			void set(MFloatColor^ value)
			{
				m_lightState->Specular=*value->m_color;
			}
		}

		// <summary>
		/// Position of the Light
		// </summary>
		property MVector^ Position
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			MVector^ get()
			{
				return gcnew MVector(&m_lightState->Position);
			}

			void set(MVector^ value)
			{
				m_lightState->Position=*value->m_vector;
			}
		}

		// <summary>
		/// Direction of the Light
		// </summary>
		property MVector^ Direction
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			MVector^ get()
			{
				return gcnew MVector(&m_lightState->Direction);
			}

			void set(MVector^ value)
			{
				m_lightState->Direction=*value->m_vector;
			}
		}

		// <summary>
		/// Range of the Light
		// </summary>
		property float Range
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get() 
			{
				return m_lightState->Range;
			}
			void set(float value) 
			{
				m_lightState->Range=value;
			}
		}

		// <summary>
		/// Intensity of the Light, multiplied by the Diffuse Color for the final color result used in the shaders
		// </summary>
		property float Intensity
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get() 
			{
				return m_lightState->Intensity;
			}
			void set(float value) 
			{
				m_lightState->Intensity=value;
			}
		}

		// <summary>
		/// Outer cone area of the spotlight (in degrees)
		// </summary>
		property float Spot_Falloff
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get() 
			{
				return m_lightState->Spot_Falloff;
			}
			void set(float value) 
			{
				m_lightState->Spot_Falloff=value;
			}
		}

		// <summary>
		/// Inner cone area of the spotlight (in degrees). Should be less than the falloff degrees.
		// </summary>
		property float Spot_Size
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get() 
			{
				return m_lightState->Spot_Size;
			}
			void set(float value) 
			{
				m_lightState->Spot_Size=value;
			}
		}

	public:

		// <summary>
		/// ctor copies values
		// </summary>
		MLightState(LightState lightState)
		{ 
			m_lightState=new LightState();
			*m_lightState=lightState;
			needToDelete=true;
		}

		// <summary>
		/// ctor
		// </summary>
		MLightState()
		{
			m_lightState=new LightState();
			needToDelete=true;
		}

		// <summary>
		/// dtor
		// </summary>
		virtual void Finalize()
		{
			if (needToDelete &&	m_lightState)
				delete m_lightState;
		}
	};

	private enum class MActorType
	{
		Actor,
		Light,
		NetworkActor
	};

	[Flags]
	// <summary>
	/// Physics flags used by the Actor::PhysicsFlags that determine physics behavior in Reality
	// </summary>
	public enum class PHYSICS_FLAGS
	{
		PHYS_PUSHABLE	= 1,
		PHYS_NOT_AFFECTED_BY_GRAVITY = 2,
		/// What to do when colliding (part of physics flags)
		PHYS_ONBLOCK_STOP          =  32,  /// stop moving
		PHYS_ONBLOCK_SLIDE         =  64,  /// slide along
		PHYS_ONBLOCK_CLIMBORSLIDE  =  128,  /// clim up a stair or slide along
		PHYS_ONBLOCK_BOUNCE        =  256,  /// bounce off
		PHYS_ONBLOCK_PUSH          =  512,  /// push the obstacle
		PHYS_RIGIDBODYDYANMICS	   =  1024 /// Tokmak rigid body dynamics
	};

	[Flags]
	// <summary>
	/// Collision flags used by the Actor::PhysicsFlags that determine collision behavior in Reality
	// </summary>
	public enum class COLLISION_FLAGS
	{
		/// Collision volume type
		CF_MESH = 1, /// A Mesh. Perfectly accurate, but slower
		CF_BBOX = 2, /// A bounding box. Most common for player and entities
		CF_PASSABLE_BBOX = 4, /// A bbox which gets collision messages (touched() untouched()) but allows actors to pass through. Used for triggers, zones, projectiles
		CF_CORPSE = 8, /// Can be shot but not collided with
		CF_ORIENTED_TOKAMAK_BBOX = 16,
		CF_WANT_WORLD_TOUCHED	= 32, /// Wants Touched() to trigger on world (actor will be NULL)
		CF_IGNORE_TOUCHED = 64, //Ignore all Touched() checking to save a bit of speed on simple Actors
		/// Crappy hack flag to avoid slow actor-to-mesh-actor collisions
		CF_IGNORE_MESH_USE_BBOX	= 128,
		CF_ALLOW_STUCK = 256,
		/// mansion hack flag to improve tok collision sounds
		CF_RECEIVE_INCOMING_ACTOR_TOUCH = 512
	};

    
    // <summary>
    /// Shadow type for this actor
    // </summary>
    public enum class Shadows
    {
        None = 0, /// No shadows
        Blob = 1, /// Texture blobs
        Drop = 2, /// Render-to-texture projection
        ShadowMap = 3 /// True shadow map
    };

	// <summary>
	/// Wraps and provides the ability to effectively extend Actor through MC++ or C#. Actor is the base class of all gameplay objects.
	// </summary>
	public ref class MActor	: MarshalByRefObject, IDisposable 
	{
		private public:

			// <summary>
			/// 
			// </summary>
			property virtual MActorType ActorType
#ifdef DOXYGEN_IGNORE 
				; _()
#endif
			{
				MActorType	get()
				{
					return MActorType::Actor;
				}
			}

			// <summary>
			/// the unmanaged Actor wrapped by this particular Managed Actor
			// </summary>
			Actor *	m_actor;

	private:

		// <summary>
		/// Track whether Dispose has been called.
		// </summary>
		bool  disposed;

		private	public:

			// <summary>
			/// Binds the Managed-Actor representation to its C++ sibling
			// </summary>
			void InitFromActor(Actor* actor)
			{
				m_actor=actor;
				m_location = gcnew MVector(&m_actor->Location);
				m_velocity = gcnew MVector(&m_actor->Velocity);
				m_rotation = gcnew MMatrix(&m_actor->Rotation);
				array<String^>^ words=this->GetType()->ToString()->Split('.');
				String^ className=words[words->Length-1];
				m_actor->SetManagedClassName(Helpers::ToCppString(className));
				m_actor->SetManagedIndex(s_actorsKey);
				s_actors->Add(s_actorsKey,this);
				s_actorsKey++;
			}

			// <summary>
			/// Initialize a Managed-Actor from the core C++ one
			// </summary>
			MActor(Actor * actor)
			{ 
				Team = 0;
				InitFromActor(actor);
			}

			// Dispose(bool disposing) executes in two distinct scenarios.
			// If disposing equals true, the method has been called directly
			// or indirectly by a user's code. Managed and unmanaged resources
			// can be disposed.
			// If disposing equals false, the method has been called by the
			// runtime from inside the finalizer and you should not reference
			// other objects. Only unmanaged resources can be disposed.
			// <summary>
			/// once resource disposal upon finalization
			// </summary>
			void Dispose(bool disposing) 
			{
				// Check to see if Dispose has already been called.
				if (!disposed) 
				{
					// If disposing equals true, dispose all managed
					// and unmanaged resources.
					if (disposing) 
					{
						// Dispose managed resources.
						DisposeResources();
					}

					if(MyWorld != nullptr)
						MyWorld->MActors->Remove(this);

					// Dispose unmanaged resources
					s_actors->Remove(m_actor->GetManagedIndex());

					m_actor = NULL;
				}
			}

	public:
		static int			s_actorsKey;
		static Hashtable^	s_actors;

		// <summary>
		/// Gets a Managed-Actor from its Actor sibling, if any, via the hashtable. Used particularly by the callbacks to find appropriate MActors from passed Actors.
		// </summary>
		static MActor^ GetFromActor(Actor* actor)
		{
			if(!actor)
				return nullptr;

			return (MActor^)s_actors[actor->GetManagedIndex()];
		}

		// <summary>
		/// Actor state flag, is touching the ground
		// </summary>
		static const int STATE_ONGROUND = 1;

		// <summary>
		/// 
		// </summary>
		static MActor()
		{
			s_actors = gcnew Hashtable();
			s_actorsKey=0;
		}

		// <summary>
		/// World that the MActor is in, if any
		// </summary>
		MWorld^ MyWorld;

		// <summary>
		/// ctor used for direct construction of a Managed-Actor from managed code
		// </summary>
		MActor(MWorld^ world)
		{
			Team = 0;
			MyWorld = world;
			MyWorld->MActors->Add(this);

			Actor *	actor;

			switch (this->ActorType)
			{
			case MActorType::Light:
				actor =	new Light( world->GetUnManagedPointer(),true);// Factory::create("Buggy",world->GetUnManagedPointer());
				break;
			case MActorType::Actor:
				actor =	new Actor(world->GetUnManagedPointer(),true);// Factory::create("Buggy",world->GetUnManagedPointer());
				break;
			case MActorType::NetworkActor:
				actor =	new NetworkActor(world->GetUnManagedPointer(),true);// Factory::create("Buggy",world->GetUnManagedPointer());
			}
			InitFromActor(actor);
		}

		// <summary>
		/// dtor
		// </summary>
		virtual void Finalize()
		{
			// Do not re-create Dispose clean-up code here.
			// Calling Dispose(false) is optimal in terms of
			// readability and maintainability.
			Dispose(false);
			disposed = true;
		}

		// <summary>
		/// manual disposal, for MActor use Destroy() instead if you want instant deletion in C# script
		// </summary>
		void Dispose() 
		{
			Dispose(true);
			disposed = true;
			// This object will be cleaned up by the Dispose method.
			// Therefore, you should call GC::SupressFinalize to
			// take this object off the finalization queue
			// and prevent finalization code for this object
			// from executing a second time.
			GC::SuppressFinalize(this);
		}

		// <summary>
		/// deletes the MActor and its unmanaged Actor immediately, use this if you want instant deletion
		// </summary>
		void Destroy(){delete m_actor;}

	protected:

		// <summary>
		/// disposal of resources that can be overriden in the derived classes
		// </summary>
		virtual void DisposeResources(){}

	public:

		// <summary>
		/// Can be set & used by game code for logic purposes
		// </summary>
		int Team;

		// <summary>
		/// saves the Managed-Actors Properties in Reality Builder
		// </summary>
		void Serialize(void* XMLSystem,void* node)
		{
			array<PropertyInfo^>^ members=this->GetType()->GetProperties();
			for (int i=0;i<members->Length;i++)
			{

				BrowsableAttribute^  myAttribute =(BrowsableAttribute^)Attribute::GetCustomAttribute(members[i],__typeof(BrowsableAttribute)); 

				bool browsable=true; 

				if (myAttribute!=nullptr) 
					browsable = myAttribute->Browsable;  

				if (members[i]->CanRead && browsable )
				{
					if (typeid<System::Boolean> == members[i]->PropertyType)
						::Serialize(XMLSystem,(char*)Helpers::ToCppString(members[i]->Name).c_str(),Convert::ToBoolean(members[i]->GetValue(this,nullptr)),node);
					else if (typeid<System::Int32> == members[i]->PropertyType)
						::Serialize(XMLSystem,(char*)Helpers::ToCppString(members[i]->Name).c_str(),Convert::ToInt32(members[i]->GetValue(this,nullptr)),node);
					else if (typeid<System::Single> == members[i]->PropertyType)
						::Serialize(XMLSystem,(char*)Helpers::ToCppString(members[i]->Name).c_str(),Convert::ToSingle(members[i]->GetValue(this,nullptr)),node);
					else if (typeid<System::String> == members[i]->PropertyType)
						::Serialize(XMLSystem,(char*)Helpers::ToCppString(members[i]->Name).c_str(),Helpers::ToCppString((String^)members[i]->GetValue(this,nullptr)).c_str(),node);
					else if (typeid<MVector> == members[i]->PropertyType)
						::Serialize(XMLSystem,(char*)Helpers::ToCppString(members[i]->Name).c_str(),*((MVector^)members[i]->GetValue(this,nullptr))->m_vector,node);
                    // Not sure what it is, so serialize as Int32 in case it's an enum
                    else 
                    {
                        try{
                        ::Serialize(XMLSystem,(char*)Helpers::ToCppString(members[i]->Name).c_str(),Convert::ToInt32(members[i]->GetValue(this,nullptr)),node);
                        }
                        catch(...){} // Invalid cast
                    }
					
                    //For Debugging
					//System::Diagnostics::Debug::WriteLine(members[i]->Name);
					//System::Diagnostics::Debug::WriteLine(members[i]->GetValue(this,nullptr)->ToString());
					//System::Diagnostics::Debug::WriteLine(members[i]->PropertyType->ToString());
				}
			}
		}

		// <summary>
		/// Adds a force upon being hit in collide&slide, for rigid body Managed-Actors, overriden in their derived classes
		// </summary>
		virtual void AddImpulse(MVector^ impulse){}

		// <summary>
		/// Adds another Actor to this Actor's DropShadow partner list, for sharing of DropShadow Render Targets to eliminate the appearance of overlapping shadows (such as a Player holding a Weapon)
		// </summary>
		void AddDropShadowPartner(MActor^ partner)
		{
			m_actor->AddDropShadowPartner(partner->m_actor);
		}
		
		// <summary>
		/// Removes another Actor from this Actor's DropShadow partner list, it it's on the list
		// </summary>
		void RemoveDropShadowPartner(MActor^ partner)
		{
			m_actor->RemoveDropShadowPartner(partner->m_actor);
		}

		// <summary>
		/// Sets the MActor's owner World, taking care to remove itself from its previous Worlds' MActor array and adding it to the new one, if any
		// </summary>
		virtual void SetWorld(MWorld^ world)
		{
			if(MyWorld != nullptr)
				MyWorld->MActors->Remove(this);

			MyWorld = world;

			if(MyWorld != nullptr)
				MyWorld->MActors->Add(this);
		}

		// <summary>
		/// Updates the Managed-Actor's game logic in derived classes, and in the MActor updates the Actor::Tick(). If this MActor.Tick() -- and in turn Actor::Tick() -- is not called, physics and other core Actor state will not be updated!
		// </summary>
		virtual	void Tick()
		{
			//only tick Actors that aren't about to be destroyed
			if(m_actor->LifeTime != 0)
				m_actor->Tick();
		}

		// <summary>
		/// overridable function for copying of custom state (properties & otherwise) during a Reality Builder Clone operation. 
		// </summary>
		virtual void Clone(MActor^ dest){}

		// <summary>
		/// overridable function to render to the screen after the World is drawn, for vfx and HUD
		// </summary>
		virtual	void PostRender(ref class MCamera^ camera){}

		// <summary>
		/// overridable function to render to the screen before the World is drawn, for pre-World vfx or updating of transformations after all Ticks
		// </summary>
		virtual	void PreRender(ref class MCamera^ camera){}

		// <summary>
		/// overridable function to render to the screen immediately before this Actor is drawn, for additional World Model draws or updating of transformations immediately before drawing
		// </summary>
		virtual	void OnRender(ref class MCamera^ camera){}

		// <summary>
		/// called by the ScriptingSystem when an MActor's deserialization (loading of saved properties from the XML) is completed, useful to override for custom initialization routine based on those properties
		// </summary>
		virtual void DeserializationComplete(){}

		// <summary>
		/// Called when this Managed-Actor hits the ground
		// </summary>
		virtual void Landed(MVector^ hitVelocity){}

		// <summary>
		/// Called to have this MActor draw informationa bout itself to the screen (such as its name), used by game logic, overridablew
		// </summary>
		virtual void DrawMouseOverInfo(MActor^ activator)
		{
			Canvas::Instance()->TextCenteredf(SmallFont, COLOR_RGBA(255, 255, 255, 180), 512, 420, 512, 420, Helpers::ToCppString(ToString()).c_str());
		}

		// <summary>
		/// Called when this MActor is "activated", used by the game logic, overridable
		// </summary>
		virtual void Server_Activate(MActor^ activator, MVector^ activationPoint){}

		// <summary>
		/// Called when the Game is currently controlling this actor, when this actor is the avatar
		// </summary>
		virtual void ProcessInput(){}

		// <summary>
		/// Called when the Game is currently controlling this actor, when this actor is the avatar
		// </summary>
		virtual void UpdateCamera(){}

		// <summary>
		/// Called when a network message is received for a keypress on this actor, ownership validated in Engine prior to being called
		// </summary>
		virtual void Server_HandleNetworkKeyInput(bool isDown, int NetworkKeyHandle){}

		// <summary>
		/// Called when a mouse update is received for a keypress on this actor, ownership validated in Engine prior to being called
		// </summary>
		virtual void Server_HandleNetworkMouseUpdate(float mouseYaw, float mousePitch){}

		// <summary>
		/// Whether this MActor exists in a World
		// </summary>
		bool HasWorld(){return m_actor->MyWorld != NULL;}

		// <summary>
		/// Called when this Managed-Actor gets Touched by another Managed-Actor or hits the World itself (if it has the CF_WANT_WORLD_TOUCHED collisionflag set)
		// </summary>
		virtual void Touched(MActor^ other, MCollisionInfo^ info)
		{

		}

		// <summary>
		/// Sets the size of this MActor's custom collision box, which will override the default usage of his MyModel's box if any.
		// </summary>
		void SetCollisionBox(MVector^ min, MVector^ max)
		{
			m_actor->CollisionBox.min = *min->m_vector;
			m_actor->CollisionBox.max = *max->m_vector;
		}

		// <summary>
		/// Creates a Managed MyModel from an existing Actor::MyModel if the Managed-Actor expects to have a loaded Model set on it by the Engine, rather than creating and loading the MyModel itself. (this is true particularly for Managed-Actors that expect to be applied by the "Change" operation to existing models in Reality Builder)
		// </summary>
		MModel^ GetMyModel()
		{
			if(m_actor->MyModel)
				return gcnew MModel(m_actor->MyModel);
			else
				return gcnew MModel();
		}

		// <summary>
		/// Sets the collision Model used for other Actors colliding onto this Actor when CF_MESH is enabled. Defaults to MyModel, but for instance could manually be set to a simplified collision hull or an oriented box.
		// </summary>
		void SetCollisionModel(MModel^ collisionModel)
		{
			m_actor->SetCollisionModel(collisionModel->m_model);
		}

		//MEMBER VARIABLES
	private:

		// <summary>
		/// internal loc wrap
		// </summary>
		MVector^ m_location;

		// <summary>
		/// interal rot wrap
		// </summary>
		MMatrix^ m_rotation;

		// <summary>
		/// internal vel wrap
		// </summary>
		MVector^ m_velocity;

		// <summary>
		/// internal model wrap
		// </summary>
		MModel^ m_mymodel;

		//PROPERTIES
	public:

		[BrowsableAttribute(false)]

		// <summary>
		/// Whether this Actor is currently selected in Reality Builder.
		// </summary>
		property bool IsSelected
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			bool	get()
			{
				return m_actor->IsSelected;
			}
		}

		[BrowsableAttribute(false)]

		// <summary>
		/// Whether this Actor allows the World Tick to automatically update its MyModel transformation. If not, the Actor itself is responsible for manually Updating the transformation and incrementing animation via Update(), if it wants to
		// </summary>
		property bool AutoUpdateModelTransformation
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			bool get()
			{
				return m_actor->m_AutoUpdateModelTransformation;
			}
			void set(bool value)
			{
				m_actor->m_AutoUpdateModelTransformation = value;
			}
		}

		[BrowsableAttribute(false)]

		// <summary>
		/// The Parent Managed-Actor of this Managed-Actor, if any. Parents will automatically nullify upon destruction of the Parent.
		// </summary>
		property MActor^ Parent
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			MActor^	get()
			{
				return MActor::GetFromActor(m_actor->GetParent());
			}
			void set(MActor^ value)
			{
				if(value != nullptr)
					m_actor->SetParent(value->m_actor);
				else
					m_actor->SetParent(NULL);
			}
		}

		[BrowsableAttribute(false)]

		// <summary>
		/// For Actors that don't want general inclusions/exclusion list, they can specify a particular light to allow usage of its list on this
		// </summary>
		property MActor^ AllowIncludeExcludeLight
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			MActor^	get()
			{
				return MActor::GetFromActor(m_actor->m_AllowIncludeExcludeLight);
			}
			void set(MActor^ value)
			{
				if(value != nullptr)
					m_actor->m_AllowIncludeExcludeLight = (Light*)value->m_actor;
				else
					m_actor->m_AllowIncludeExcludeLight = NULL;
			}
		}

		[BrowsableAttribute(false)]

		//<summary>
		/// Whether or not the Actor obeys Light Inclusion/Exclusion lists, or is just lit by all Lights that reach it
		// </summary>
		property bool AllowLightIncludeExclude
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			bool get()
			{
				return m_actor->m_AllowIncludeExclude;
			}
			void set(bool value)
			{
				m_actor->m_AllowIncludeExclude = value;
			}
		}


		[BrowsableAttribute(false)]

		// <summary>
		/// The collision-ignore Managed-Actor of this Managed-Actor, if any. IgnoreActors will automatically nullify upon destruction of the Parent.
		// </summary>
		property MActor^ IgnoreActor
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			MActor^	get()
			{
				return MActor::GetFromActor(m_actor->ignoreActor);
			}
			void set(MActor^ value)
			{
				if(value != nullptr)
					m_actor->ignoreActor = value->m_actor;
				else
					m_actor->ignoreActor = NULL;
			}
		}

		[BrowsableAttribute(false)]

		// <summary>
		/// The Model used for automatic drawing at this Actor's transformation every frame, also applies a dynamically sized bbox for this Actor if the Actor doesn't set its own CollisionBox.
		// </summary>
		property MModel^ MyModel
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			MModel^	get()
			{
				return m_mymodel;
			}
			void set(MModel^ value)
			{
				if(value != nullptr)
				{
					m_mymodel = value;
					m_actor->MyModel = m_mymodel->m_model;
				}
				else
				{
					m_mymodel = nullptr;
					m_actor->MyModel = NULL;
				}
			}
		}

		[BrowsableAttribute(false)]

		// <summary>
		/// The Mass of this Actor, used for determining mangitude of impulse collisions onto rigid bodies in collide&slide
		// </summary>
		property float mass
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float	get(){return m_actor->Mass;}
			void set(float value){m_actor->Mass = value;}
		}

		[BrowsableAttribute(false)]

		// <summary>
		/// Whether this Actor is actually a Light
		// </summary>
		property virtual bool IsLight
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			bool get() 
			{
				return false;
			}
		}

		// <summary>
		/// Shadow type for this actor
		// </summary>
		property Shadows ShadowType
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			Shadows get() 
			{
				return (Shadows)(int)m_actor->m_ShadowType;
			}
			void set(Shadows value) 
			{
                DWORD val = (DWORD)value;
                m_actor->m_ShadowType=(Actor::ShadowType)value;
			}
		}

        [BrowsableAttribute(false)]
        // <summary>
		/// Whether this Actor can be exported, saved in Reality Builder. This is by default set to true for an instance that's Inserted or Changed to by the Reality Builder user interfance, but its behavior can be altered at will in the MActor classes too.
		// </summary>
		property bool Exportable
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			bool get() 
			{
				return m_actor->bExportable;
			}
			void set(bool value) 
			{
				m_actor->bExportable=value;
			}
		}

		[BrowsableAttribute(false)]

		// <summary>
		/// The current Material that the Actor is standing on, if any
		// </summary>
		property MMaterial^ GroundMat
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			MMaterial^ get() 
			{
				if(m_actor->GroundMat)
					return gcnew MMaterial(m_actor->GroundMat);
				else
					return nullptr;
			}
		}

		[BrowsableAttribute(false)]

		// <summary>
		/// Whether this Actor is a Prefab. Prefabs, static in the scene, don't update their transformations or spatial partioning outside of Reality Builder, they're immovable and generally allow mesh-accurate collisions onto themselves.
		/// They also bake their rotations and scaling into their vertex transformations, so that they can use a global static PRT calculation, since Prefabs are assumed not to move, a major optimization for immobile meshes.
		// </summary>
		property bool Prefab
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			bool get()
			{
				return m_actor->IsPrefab;
			}

			void set(bool value)
			{

				m_actor->IsPrefab=value;
			}
		}

		[BrowsableAttribute(false)]

		// <summary>
		/// Whether this Actor is a StaticObject. Doesn't have physics, a big optimization! Velocity still works, but be careful, or it'll get stuck!
		// </summary>
		property bool StaticObject
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			bool get()
			{
				return m_actor->StaticObject;
			}

			void set(bool value)
			{

				m_actor->StaticObject=value;
			}
		}

		// <summary>
		/// Whether this Actor is a GhostObject. Doesn't interact with world and world doesn't interact with it. Velocity still works.
		// </summary>
		property bool GhostObject
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			bool get()
			{
				return m_actor->GhostObject;
			}

			void set(bool value)
			{

				m_actor->GhostObject=value;
			}
		}

		// <summary>
		/// Whether this Actor is Hidden. Hidden Actor MyModels aren't automatically drawn, though their Pre/Post/On-Render functions are still called for custom draws as desired.
		// </summary>
		property bool IsHidden
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			bool get()
			{
				return m_actor->IsHidden;
			}

			void set(bool value)
			{

				m_actor->IsHidden=value;
			}
		}

		[BrowsableAttribute(false)]

		// <summary>
		/// In seconds, before actor is destroyed. -1 = never
		// </summary>
		property float LifeTime
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get()
			{
				return m_actor->LifeTime;
			}
			void set(float value)
			{

				m_actor->LifeTime=value;
			}
		}

		[BrowsableAttribute(false)]

		// <summary>
		/// Current physics state. Reality physics will set the flags such as CURRENTSTATE_ONGROUND
		// </summary>
		property int CurrentState
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			int get()
			{
				return (int)m_actor->CurrentState;
			}
			void set(int value)
			{
				m_actor->CurrentState = (DWORD)value;
			}
		}

		[BrowsableAttribute(false)]

		// <summary>
		/// PHYSICS_FLAGS that determine physics behavior in Reality
		// </summary>
		property PHYSICS_FLAGS PhysicsFlags
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			PHYSICS_FLAGS get()
			{
				return (PHYSICS_FLAGS)m_actor->PhysicsFlags;
			}
			void set(PHYSICS_FLAGS value)
			{

				m_actor->PhysicsFlags = (DWORD)value;
			}
		}

		[BrowsableAttribute(false)]

		// <summary>
		/// COLLISION_FLAGS that determine collision behavior in Reality
		// </summary>
		property COLLISION_FLAGS CollisionFlags
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			COLLISION_FLAGS get()
			{
				return (COLLISION_FLAGS)m_actor->CollisionFlags;
			}
			void set(COLLISION_FLAGS value)
			{

				m_actor->CollisionFlags = (DWORD)value;
			}
		}

		// <summary>
		/// If false, ray tests will ignore this Actor (except Reality Builder checks)
		// </summary>
		property bool CollidesRays
	#ifdef DOXYGEN_IGNORE 
			; _()
	#endif
		{
			bool get()
			{
				return m_actor->m_CollidesRays;
			}
			void set(bool value)
			{
				m_actor->m_CollidesRays=value;
			}
		}


		[BrowsableAttribute(false)]

		// <summary>
		/// If true, the Actor will update its Inside and Outside variables every frame/rate based on its position in the World. If inside and IndoorVolume, it will be flagged as Inside=true,Outside=false for lighting and logic purposes.
		// </summary>
		property bool UpdateIndoorOutdoorState
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			bool get()
			{
				return m_actor->m_UpdateIndoorOutdoorState;
			}

			void set(bool value)
			{

				m_actor->m_UpdateIndoorOutdoorState=value;
			}
		}

		[BrowsableAttribute(false)]

		// <summary>
		/// Determines the rate at which the Inside and Outside states are updated, if update is true. 1.0 is every frame.
		// </summary>
		property float IndoorOutdoorStateUpdateRate
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get()
			{
				return m_actor->m_IndoorOutdoorStateUpdateRate;
			}

			void set(float value)
			{

				m_actor->m_IndoorOutdoorStateUpdateRate=value;
			}
		}

		// <summary>
		/// Whether the Actor is classified as inside some building/interior structure for lighting and logic purposes
		// </summary>
		property bool Inside
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			bool get()
			{
				return m_actor->Inside;
			}

			void set(bool value)
			{

				m_actor->Inside=value;
			}
		}

		// <summary>
		/// Whether the Actor is classified as out in the great outdoors for lighting and logic purposes (e.g. then lit by the sky lights)
		// </summary>
		property bool Outside
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			bool get()
			{
				return m_actor->Outside;
			}

			void set(bool value)
			{
				m_actor->Outside=value;
			}
		}

		// <summary>
		/// Name of the Actor, primarily used for display in Reality Builder where RB auto-enforces unique Names on a per-instance basis
		// </summary>
		property String^ Name
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			String^	get()
			{
				return Helpers::ToCLIString(m_actor->m_Name);
			}

			void set(String^ value)
			{
				m_actor->m_Name=Helpers::ToCppString(value);
			}
		}

		[BrowsableAttribute(false)]

		// <summary>
		/// Location of the Actor, direct wrap
		// </summary>
		property MVector^ Location
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			MVector^ get()
			{
				return m_location;
			}

			void set(MVector^ value)
			{
				*m_location->m_vector=*value->m_vector;
			}
		}

		[BrowsableAttribute(false)]

		// <summary>
		/// Velocity of the Actor, direct wrap
		// </summary>
		property MVector^ Velocity
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			MVector^ get()
			{
				return m_velocity;
			}

			void set(MVector^ value)
			{
				*m_velocity->m_vector=*value->m_vector;
			}
		}

		[BrowsableAttribute(false)]

		// <summary>
		/// Rotation of the Actor, direct wrap
		// </summary>
		property MMatrix^ Rotation
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			MMatrix^ get()
			{
				return m_rotation;
			}
			void set(MMatrix^ value)
			{
				*m_rotation->m_matrix=*value->m_matrix;
			}
		}
	};


	// <summary>
	/// Categories of textures loaded or created by Texture
	// </summary>
	public enum class MTextureType {
		MTT_AUTO,    /// Will figure out if diffuse/normal from filename
		MTT_DIFFUSE, /// standard diffuse map, non-DDS' are compressed to DXT3 if alpha, DXT1 if no alpha.
		MTT_NORMALMAP, /// compressed via DXT5 or 3DC normal map compression if enabled
		/// Stick this in the alpha of its corresponding shader texture
		/// This is a special flag, and does not mean the texture has alpha
		MTT_PUTINALPHA,
		/// Cube & Volume are only used for creation of new surfaces
		MTT_CUBE,	
		MTT_VOLUME,
	};

	// <summary>
	/// Wrapper for Texture --  Game-level texture objects. Device-level management is automatic.
	// </summary>
	public ref class MTexture : IDisposable
	{
		private public:
			Texture * m_texture;
			bool needToDelete;
			// <summary>
			/// ctor to wrap existing unmanaged texture
			// </summary>
			MTexture(Texture* texture)
			{
				m_texture = texture;
				needToDelete = false;
			}

	public:

		// <summary>
		/// ctor for new texture
		// </summary>
		MTexture()
		{
			m_texture = new Texture();
			needToDelete = true;
		}

		// <summary>
		/// Loads a Texture from a file, with optional specifications. Animation frames are loaded if they exist and texture name/path contains animated
		// </summary>
		bool Load(String^ name, MTextureType type , float uOff, float vOff, float uTile, float vTile, float vAng)
		{
			TextureType ttype=(TextureType)(int)type;
			return m_texture->Load(Helpers::ToCppString(name),ttype,uOff,vOff,uTile,vTile,vAng);
		}

		// <summary>
		/// Loads a Texture from a file. Animation frames are loaded if they exist and texture name/path contains animated
		// </summary>
		bool Load(String^ name)
		{
			return m_texture->Load(Helpers::ToCppString(name));
		}

		// <summary>
		/// Whether a texture file has been loaded into this object
		// </summary>
		bool IsLoaded()
		{
			return m_texture->IsLoaded();
		}

		// <summary>
		/// U-tiling
		// </summary>
		property float uTile
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get() 
			{
				return m_texture->uTile;
			}
			void set(float value) 
			{
				m_texture->uTile = value;
			}
		}

		// <summary>
		/// V-tiling
		// </summary>
		property float vTile
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get() 
			{
				return m_texture->vTile;
			}
			void set(float value) 
			{
				m_texture->vTile = value;
			}
		}

		// <summary>
		/// U-offset
		// </summary>
		property float uOff
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get() 
			{
				return m_texture->uOff;
			}
			void set(float value) 
			{
				m_texture->uOff = value;
			}
		}

		// <summary>
		/// V-offset
		// </summary>
		property float vOff
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get() 
			{
				return m_texture->vOff;
			}
			void set(float value) 
			{
				m_texture->vOff = value;
			}
		}

		// <summary>
		/// Whether the Texture uses LODs (downscaling of size according to Reality's TextureSizePercent). Some textures, such as HUD textures, may opt not to uses LODs.
		// </summary>
		property float usesLOD
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get() 
			{
				return m_texture->usesLOD;
			}
			void set(float value) 
			{
				m_texture->usesLOD = value;
			}
		}

		// <summary>
		/// Destroys the object. MTexture should not be used after this, should be nullified.
		// </summary>
		void Destroy()
		{
			m_texture->Destroy();
		}

		// <summary>
		/// dtor
		// </summary>
		virtual void Finalize()
		{
			Dispose(false);
			disposed = true;

		}		

		// <summary>
		/// disposal
		// </summary>
		void Dispose() 
		{
			Dispose(true);
			disposed = true;
			GC::SuppressFinalize(this);
		}

	protected:
		bool disposed;

		// <summary>
		/// unmanaged resource disposal
		// </summary>
		virtual void Dispose(bool disposing) 
		{
			if (!disposed) 
			{
				if (disposing) 
				{
					// Dispose managed resources.
				}

				// Dispose unmanaged resources
				if (needToDelete && m_texture)
					delete m_texture;
			}
		}

	};



	// <summary>
	/// Wraps Light class and allows extension of Light functionality and effects in C#.
	// </summary>
	public ref class MLight : MActor
	{
		private public:
			ref class MTexture^ projectionMap;

			// <summary>
			/// 
			// </summary>
			property virtual MActorType ActorType
#ifdef DOXYGEN_IGNORE 
				; _()
#endif
			{
				MActorType	get()
				{
					return MActorType::Light;
				}
			}
	public:
		static ArrayList^ Lights;
		static MLight()
		{
			Lights = gcnew ArrayList();
		}

		// <summary>
		/// The texture map used for spotlight projections
		// </summary>
		property MTexture^ ProjectionMap
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			MTexture^ get() 
			{
				return projectionMap;
			}
		}

		// <summary>
		/// used for serialization of the LightType
		// </summary>
		property int LightTypeInt
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			int get() 
			{
				return (int)LightType; 
			}
			void set(int value)
			{
				LightType = (MLightType)value;
			}
		}

		// <summary>
		/// Returns the CurrentState (LightState) of the Light, namely its immediate color, range, and transformation values that are used for lighting. Lights update from their Location and Rotation every frame, but CurrentState allows direct access to the values used by the shaders.
		// </summary>
		property MLightState^ CurrentState
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			MLightState^ get() 
			{
				return gcnew MLightState(&((Light*)m_actor)->GetCurrentState());
			}
		}

		// <summary>
		/// Light is subtractive shadow projector
		// </summary>
		property bool IsShadowProjector
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			bool get() 
			{
				return ((Light*)m_actor)->m_bShadowProjector;
			}
			void set(bool value) 
			{
				((Light*)m_actor)->m_bShadowProjector = value;
			}
		}

		// <summary>
		/// Shadow map texture size
		// </summary>
		property int ShadowMapSize
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			int get() 
			{
				return ((Light*)m_actor)->m_ShadowMapSize;
			}
			void set(int value) 
			{
				((Light*)m_actor)->m_ShadowMapSize = value;
			}
		}

		// <summary>
		/// Cutoff & attenuation falloff range of the light, from its CurrentState
		// </summary>
		property float Range
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get() 
			{
				return ((Light*)m_actor)->GetCurrentState().Range;
			}
			void set(float value) 
			{
				((Light*)m_actor)->SetRange(value);
			}
		}

		// <summary>
		/// Intensity multiplier of the color, from its CurrentState
		// </summary>
		property float Intensity
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get() 
			{
				return ((Light*)m_actor)->GetCurrentState().Intensity;
			}
			void set(float value) 
			{
				((Light*)m_actor)->GetCurrentState().Intensity=value;
			}
		}

		// <summary>
		/// The spotlight falloff size in degrees, from its CurrentState
		// </summary>
		property float Spot_Falloff
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get() 
			{
				return ((Light*)m_actor)->GetCurrentState().Spot_Falloff;
			}
			void set(float value) 
			{
				((Light*)m_actor)->GetCurrentState().Spot_Falloff=value;
			}
		}

		// <summary>
		/// The spotlight hotspot size in degrees, from its CurrentState. Should be smaller than the falloff degrees.
		// </summary>
		property float Spot_Size
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get() 
			{
				return ((Light*)m_actor)->GetCurrentState().Spot_Size;
			}
			void set(float value) 
			{
				((Light*)m_actor)->GetCurrentState().Spot_Size=value;
			}
		}


		// <summary>
		/// enum Type of light source -- omni, spot, dir, omniproj, etc
		// </summary>
		property MLightType LightType
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			MLightType get() 
			{
				return (MLightType)(int)((Light*)m_actor)->m_Type;
			}

			void set(MLightType value) 
			{
				DWORD TypeValue = (DWORD)value;
				((Light*)m_actor)->m_Type=(::LightType)TypeValue;
			}
		}

		// <summary>
		/// Diffuse color of light, from its CurrentState. In FloatColor format.
		// </summary>
		property MFloatColor^ Diffuse
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			MFloatColor^ get() 
			{
				return gcnew MFloatColor(&((Light*)m_actor)->GetCurrentState().Diffuse);
			}
			void set(MFloatColor^ value) 
			{
				((Light*)m_actor)->SetColor(*value->m_color);
			}
		}

		// <summary>
		/// Diffuse color of light, from its CurrentState. In 0-255 uint format.
		// </summary>
		property Color LightColor
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			Color get() 
			{
				return Color::FromArgb((int)(((Light*)m_actor)->GetCurrentState().Diffuse.a * 255.0f), (int)(((Light*)m_actor)->GetCurrentState().Diffuse.r * 255.0f), (int)(((Light*)m_actor)->GetCurrentState().Diffuse.g * 255.0f), (int)(((Light*)m_actor)->GetCurrentState().Diffuse.b * 255.0f));
			}
			void set(Color value) 
			{
				((Light*)m_actor)->GetCurrentState().Diffuse = FloatColor(((float)value.R) / 255.0f, ((float)value.G) / 255.0f, ((float)value.B) / 255.0f,1); 	
			}
		}

		// <summary>
		/// Red diffuse color component of light, from its CurrentState. Used for serialization
		// </summary>
		property float ColorRed
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get() {return ((Light*)m_actor)->GetCurrentState().Diffuse.r;}
			void set(float value) {((Light*)m_actor)->GetCurrentState().Diffuse.r = value;}
		}

		// <summary>
		/// Green diffuse color component of light, from its CurrentState. Used for serialization
		// </summary>
		property float ColorGreen
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get() { return ((Light*)m_actor)->GetCurrentState().Diffuse.g; }
			void set(float value) {	((Light*)m_actor)->GetCurrentState().Diffuse.g = value; }
		}

		// <summary>
		/// Blue diffuse color component of light, from its CurrentState. Used for serialization
		// </summary>
		property float ColorBlue
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get() { return ((Light*)m_actor)->GetCurrentState().Diffuse.b; }
			void set(float value) {((Light*)m_actor)->GetCurrentState().Diffuse.b = value; }
		}

		// <summary>
		/// Whether this Managed-Actor is a Light, True for MLight.
		// </summary>
		property virtual bool IsLight
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			bool get() 
			{
				return true;
			}
		}

		// <summary>
		/// Whether this Light is a dynamic moving Light. Dynamic Lights update their lighting lists every frame, and so come at a slightly greater CPU expense. By default Lights are not dynamic until they are moved or their ranges are adjusted over time.
		// </summary>
		property bool IsDynamic
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			bool get() 
			{
				return ((Light*)m_actor)->IsDynamic();
			}
			void set(bool value) 
			{
				return ((Light*)m_actor)->SetDynamic(value);
			}
		}

		// <summary>
		/// Whether the Actor lighting list is an inclusion list or an exclusion list
		// </summary>
		property bool IsExclusionList
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			bool get() 
			{
				return m_actor->m_IsExcludeList;
			}
			void set(bool value) 
			{
				m_actor->m_IsExcludeList=value;
			}
		}

	protected:

		virtual void DisposeResources()
		{
			Lights->Remove(this);
		}

		private	public:
			MLight(Actor * actor) : MActor(actor)
			{ 
				projectionMap = gcnew MTexture(&((Light*)m_actor)->m_tProjectionMap);
				Lights->Add(this);
			}
	public:

		// <summary>
		/// ctor for direct construction through C#
		// </summary>
		MLight(MWorld^ world) : MActor(world)
		{
			projectionMap = gcnew MTexture(&((Light*)m_actor)->m_tProjectionMap);
			((Light*)m_actor)->m_Method = LIGHT_METHOD_PERPIXEL;
			Lights->Add(this);
		}

		// <summary>
		/// adds a keyframe that will adjust light values over time
		// </summary>
		void AddKeyFrame(float Range, MFloatColor^ color, float Intensity, float TimeSeconds);

		// <summary>
		/// adds an Actor to this Light's inclusion/exclusion list
		// </summary>
		void AddExclusionListHandle(MActor^ handle)
		{
			((Light*)m_actor)->AddExclusionListHandle(handle->m_actor);	
		}

		// <summary>
		/// removes an Actor from this Light's inclusion/exclusion list (if it exists on it, it's safe if not)
		// </summary>
		void RemoveExclusionListHandle(MActor^ handle)
		{
			((Light*)m_actor)->RemoveExclusionListHandle(handle->m_actor);	
		}

		// <summary>
		/// removes all entries in the Light's inclusion/exclusion list. If the Light is !IsExclusion list, it will not light anything if its list is cleared.
		// </summary>
		void ClearExclusionListHandles()
		{
			((Light*)m_actor)->ClearExclusionListHandles();
		}
	};

	// <summary>
	/// Sound flags
	// </summary>
	public enum class MSoundFlags {
		NONE,
		SOUND_LOOP  /// looping sound
	};

	// <summary>
	/// 3D positional sound source to play sounds through
	// </summary>
	public ref class MSoundEmitter
	{
		private public:
			SoundEmitter* m_soundemitter;
			bool needToDelete;

	public:

		// <summary>
		/// ctor
		// </summary>
		MSoundEmitter()
		{
			m_soundemitter = new SoundEmitter();
			needToDelete = true;
		}

		// <summary>
		/// Updates the 3d state of the emitter
		// </summary>
		void Update3DParameters(MVector^ location, MVector^ velocity, float range)
		{
			m_soundemitter->Update3DParameters(*location->m_vector,*velocity->m_vector,range);
		}

		// <summary>
		/// dtor
		// </summary>
		virtual void Finalize()
		{
			if (needToDelete &&	m_soundemitter)
				delete m_soundemitter;
		}
	};

	// <summary>
	/// A sound object
	// </summary>
	public ref class MSound : IDisposable
	{
		private public:
			Sound* m_Sound;
			bool needToDelete;
			bool disposed;
	public:

		// <summary>
		/// ctor
		// </summary>
		MSound()
		{
			m_Sound = new Sound();
			needToDelete = true;
		}

		// <summary>
		/// loads 3d wave from file, must be mono
		// </summary>
		void Load(String^ name)
		{
			m_Sound->Load(Helpers::ToCppString(name));
		}

		// <summary>
		/// loads 2d wave from file, must be stereo
		// </summary>
		void Load2D(String^ name)
		{
			m_Sound->Load2D(Helpers::ToCppString(name));
		}

		// <summary>
		/// plays 3d sound at world point with appropriate params, only use if 3d sound is loaded
		// </summary>
		void Play(MVector^ position, MVector^ velocity, float Range, MSoundFlags flags, float volume)
		{
			m_Sound->Play(*position->m_vector,*velocity->m_vector,Range,(SoundFlags)flags,volume);
		}

		// <summary>
		/// plays 3d sound at emitter with appropriate params, only use if 3d sound is loaded
		// </summary>
		void Play(MSoundEmitter^ emitter, MSoundFlags flags, float volume)
		{
			m_Sound->Play(*emitter->m_soundemitter,(SoundFlags)flags,volume);
		}

		// <summary>
		/// plays 2d sound
		// </summary>
		void Play2D(MSoundFlags flags, float volume)
		{
			m_Sound->Play2D((SoundFlags)flags,volume);
		}

		// <summary>
		/// stops currently playing instances of this particular Sound Object
		// </summary>
		void Stop()
		{
			m_Sound->Stop();
		}

		// <summary>
		/// sets the volume of this particular Sound Object
		// </summary>
		void SetVolume(float volume)
		{
			m_Sound->SetVolume(volume);
		}

		// <summary>
		/// Whether a file has been loaded into this Sound Object
		// </summary>
		bool IsLoaded()
		{
			return m_Sound->IsLoaded();
		}

		// <summary>
		/// Whether this sound object has any currently playing buffers
		// </summary>
		bool IsPlaying()
		{
			return m_Sound->IsPlaying();
		}

		// <summary>
		/// Frees the Sound object's buffers, done automatically upon dtor anyways
		// </summary>
		void Free()
		{
			m_Sound->Free();
		}

		// <summary>
		/// dtor
		// </summary>
		virtual void Finalize()
		{
			Dispose(false);
			disposed = true;
		}		

		// <summary>
		/// disposal
		// </summary>
		void Dispose() 
		{
			Dispose(true);
			disposed = true;
			GC::SuppressFinalize(this);
		}

		// <summary>
		/// updates the 3d paramters of the Sound Object and any currently playing buffers of this particular Sound Object
		// </summary>
		void Update3DParameters(MVector^ position, MVector^ velocity, float MinSoundDist)
		{
			m_Sound->Update3DParameters(*position->m_vector,*velocity->m_vector,MinSoundDist);
		}

	protected:

		// deletes the unmanaged resources upon disposal
		virtual void Dispose(bool disposing) 
		{
			if (!disposed) 
			{
				if (disposing) 
				{
					// Dispose managed resources.
				}

				// Dispose unmanaged resources
				if (needToDelete && m_Sound)
					delete m_Sound;
			}
		}

	};


	// <summary>
	/// Useful helper functions and variables in C#
	// </summary>
	public ref class MHelpers
	{
	public:
		/// Useful color
		static const unsigned long FullBrightColor = COLOR_RGBA(255,255,255,255);

		/// Useful matrix
		static const MMatrix^ IdentityMatrix = gcnew MMatrix();

		/// Useful vectors
		static const MVector^ NullVector = gcnew MVector();
		static const MVector^ VectorRight = gcnew MVector(1,0,0);
		static const MVector^ VectorLeft = gcnew MVector(-1,0,0);
		static const MVector^ VectorUp = gcnew MVector(0,1,0);
		static const MVector^ VectorDown = gcnew MVector(0,-1,0);
		static const MVector^ VectorForward = gcnew MVector(0,0,1);
		static const MVector^ VectorBack = gcnew MVector(0,0,-1);

		// <summary>
		/// The build version of Reality being used by the ScriptingSystem
		// </summary>
		static property float RealityBuildVersion
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get()
			{
				return BUILD_VERSION;
			}
		}


		// <summary>
		/// The time-change value from the previous frame in Seconds, used for scaling anything time-based by the framerate or desired speed of the game (can be altered for slow-time effects)
		// </summary>
		static property float DeltaTime
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get()
			{
				return GDeltaTime;
			}
			void set(float value)
			{
				GDeltaTime=value;
			}
		}

		// <summary>
		/// The time-change value from the previous frame in Seconds, this is the value that is not ever scaled during gameplay slow-time effects.
		// </summary>
		static property float UnscaledDeltaTime
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get()
			{
				return Game::Instance()->m_ActualDeltaTime;
			}
		}

		// <summary>
		/// The current slow-time multiplier, default 1
		// </summary>
		static property float SpeedScale
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get()
			{
				return Game::Instance()->GetTickSpeedScale();
			}
			void set(float value)
			{
				Game::Instance()->SetTickSpeedScale(value);
			}
		}

		// <summary>
		/// The absolute time in Seconds since the application was started. This is never altered, even during slow-mo time.
		// </summary>
		static property float Seconds
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get()
			{
				return GSeconds;
			}
		}

		// <summary>
		/// Timestamp of the current position in a game recording that's playing back, if any
		// </summary>
		static property float PlaybackTime
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get()
			{
				return GamePlayback::Instance()->m_CurrentPlaybackTime;
			}
		}

		// <summary>
		/// Gets the absolute seconds since the application was started, accurate calculated any time it is called.
		// </summary>
		static float GetAbsoluteSeconds()
		{
			return GetGSeconds();
		}

		// <summary>
		/// Sets the global volume level, 0 to 1
		// </summary>
		static void SetGlobalVolume(float volume)
		{
		 Engine::Instance()->AudioSys->SetMasterParameters(volume,22);
		}

		// <summary>
		/// Displays a messagebox with the specified text for debugging purposes
		// </summary>
		static void DebugMessageBox(String^ text)
		{
			MessageBox(0,Helpers::ToCppString(text).c_str(),"ScriptingSystem Debug",0);
		}

		// <summary>
		/// Exits the application
		// </summary>
		static void ShutdownApp()
		{
			// This is the 'proper' way to force quit without causing any exceptions
			SendMessage(Engine::Instance()->hWnd,WM_SYSCOMMAND,SC_CLOSE,0);
			exit(0);
		}

		// <summary>
		/// Returns a string array with the full paths to all the subdirectories within a specified path, down to a given subdirectory depth
		// </summary>
		static ArrayList^ EnumerateDirectories(String^ path, int depth);

		// <summary>
		/// Returns  a string array with all the filenames (without path) that are in a specified directory, of a particular extension (e.g. ".xml"), down to a given subdir depth
		// </summary>
		static ArrayList^ EnumerateFiles(String^ path, String^ fileExtension, int depth);

		// <summary>
		/// Whether the application is in Reality Builder Editor Mode, useful for disabling some gameplay features (physics, hidden objects) in game code to allow for easier editing.
		// </summary>
		static property bool EditorMode
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			bool get()
			{
				return Editor::Instance()->GetEditorMode();
			}
		}

		// <summary>
		/// Whether the Game prints performance statistics to the screen, for debugging & analysis purposes
		// </summary>
		static property bool ShowStats
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			bool get()
			{
				return Game::Instance()->m_bShowStats;
			}
			void set(bool value)
			{
				Game::Instance()->m_bShowStats = value;
			}
		}

		// <summary>
		/// Returns a uint color from the component values 0-255
		// </summary>
		static unsigned long  ColorFromRGBA(int r,int g,int b,int a)
		{
			return COLOR_RGBA(r,g,b,a);
		}

		// <summary>
		/// Returns a uint color with the alpha channel altered 0-255
		// </summary>
		static unsigned long  ColorSetAlpha(unsigned long color,int a)
		{
			return COLOR_RGBA(COLOR_GETRED(color),COLOR_GETGREEN(color),COLOR_GETBLUE(color),a);
		}

		// <summary>
		/// Returns a uint color (0-255) from a clamped floatcolor (0-1.0)
		// </summary>
		static unsigned long  ColorFromFloatColor(MFloatColor^ col)
		{
			FloatColor color = *col->m_color;
			color.Clamp();
			return (unsigned long)color.DWORDColor();
		}

		// <summary>
		/// Prints text to the console
		// </summary>
		static void Printf(String^ text)
		{
			Game::Instance()->m_Console.Printf(Helpers::ToCppString(text).c_str());
		}

		// <summary>
		/// Returns whether the application is launched through the dedicated Game executable, and not the Editor. Useful for critical game logic differentiation in behavior when not in Reality Builder, such as multiplayer features.
		// </summary>
		static bool IsGameApp()
		{
			return Game::Instance()->g_IsGameApp;
		}

		// <summary>
		/// Sets the visiblity state of the mouse cursor
		// </summary>
		static void SetCursorVisible(bool visible)
		{
			Game::Instance()->SetCursorVisible(visible);
		}

		// <summary>
		/// Whether there are any GUI's or the console open on the screen, if so then player input is usually disabled in the game code.
		// </summary>
		static bool HasAnyGUIopen()
		{
			return Game::Instance()->HasAnyGUIopen();
		}

		// <summary>
		/// Whether the application is in game playback mode, meaning it is playing back a previously recorded game and thus should not accept direct input over players.
		// </summary>
		static bool IsPlayback()
		{
			return GamePlayback::Instance()->m_IsPlayingBack;
		}

		// <summary>
		/// Whether the application is currently recording, meaning it may wish to pump custom network messages into the recording buffer to allow for unique behavior upon subsequent playback.
		// </summary>
		static bool IsRecording()
		{
			return GameRecorder::Instance()->m_RecordGame;
		}

		// <summary>
		/// converts degrees to radians
		// </summary>
		static float Deg2Rad(float degrees)
		{
			return DEG2RAD(degrees);
		}

		// <summary>
		/// converts radians to degrees
		// </summary>
		static float Rad2Deg(float radians)
		{
			return RAD2DEG(radians);
		}

		// <summary>
		/// useful for getting various types of random values
		// </summary>
		static Random^ random = gcnew Random();


		// <summary>
		/// gets the yaw value from a vector
		// </summary>
		static float GetYawFromDir(MVector^ dir)
		{
			return RAD2DEG(atan2(dir->z,-dir->x)) - 90.f;
		}

		// <summary>
		/// gets the pitch value from a vector
		// </summary>
		static float GetPitchFromDir(MVector^ dir)
		{
			return -(RAD2DEG(dir->m_vector->RadAngle(Vector(0,-1,0))) - 90.f);
		}

		// <summary>
		/// Finds any file in a specified game subdirectory. will return true if found, with the fileStringOut containing the relative path to pass to a file loading function. if false, fileStringOut will not be altered.
		// </summary>
		static bool findMedia(String^% filestringOut, String^ subdirectory)
		{
			string file = Helpers::ToCppString(filestringOut);
			bool val = FindMedia(file,Helpers::ToCppString(subdirectory).c_str());
			filestringOut = Helpers::ToCLIString(file);
			return val;
		}

		// <summary>
		/// Whether the Engine is running in dedicated server mode, if so then various graphics-related code is not run in the game scripts.
		// </summary>
		static bool IsDedicated()
		{
			return Engine::Instance()->IsDedicated();
		}

		// <summary>
		/// Creates a Managed-Actor from its string class name, and returns the MActor reference to it.
		// </summary>
		static MActor^ CreateActor(MWorld^ world, String^ ActorClassName)
		{
			Actor* actor = Factory::create(Helpers::ToCppString(ActorClassName),world->m_world);
			return MActor::GetFromActor(actor);
		}

		// <summary>
		/// Sets the viewport z-buffer ranges, useful for alteration of the z-buffer range during OnRender process (together with DrawImmediate) for model overlays such as a FPV gun model that is always on top of the screen, or Models on the HUD.
		// </summary>
		static void SetViewportZ(float minZ, float maxZ);
	};

	// <summary>
	/// Provides access to various input-related functionality
	// </summary>
	public ref class MInput
	{
	public:
		static MInput^ Instance = gcnew MInput();

		// <summary>
		/// whether an input handle is currently being pressed
		// </summary>
		static bool ControlDown(int ControlHandle);

		// <summary>
		/// returns the handle for the specified control name contained in the config; the handle can be used with the ControlDown and ControlJustPressed functions
		// </summary>
		static int GetControlHandle(String^ ControlName);

		// <summary>
		/// whether an input handle was first polled pressed this frame
		// </summary>
		static bool ControlJustPressed(int ControlHandle);

		// <summary>
		/// whether the mouse pitch is inverted
		// </summary>
		static property bool MouseInvertPitch
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			bool get()	
			{
				return Input::Instance()->m_bInvertPitch;
			}
			void set(bool value) 
			{
				Input::Instance()->m_bInvertPitch = value;
			}
		}

		// <summary>
		/// the sensitivity setting for the mouse from 0 to 1, good avg value is .2
		// </summary>
		static property float MouseSensitivity
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get()	
			{
				return Engine::Instance()->InputSys->GetSensitivity();
			}
			void set(float value) 
			{
				Engine::Instance()->InputSys->SetSensitivity(value);
			}
		}

		// <summary>
		/// the current yaw value of the mouse
		// </summary>
		static property float mouseYaw
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get()	
			{
				return Engine::Instance()->InputSys->mouseYaw;
			}
			void set(float value) 
			{
				Engine::Instance()->InputSys->mouseYaw = value;
			}
		}

		// <summary>
		/// the current pitch value of the mouse
		// </summary>
		static property float mousePitch
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get()	
			{
				return Engine::Instance()->InputSys->mousePitch;
			}
			void set(float value) 
			{
				Engine::Instance()->InputSys->mousePitch = value;
			}
		}

		// <summary>
		/// sends input key press state to the server
		// </summary>
		static void Client_SendKey(unsigned char NetworkHandleID, bool isDown)
		{
			GameClientModule::Instance()->SendKey(NetworkHandleID,isDown);
		}

		// <summary>
		/// sends current mouse yaw/pitch to the server
		// </summary>
		static void Client_SendMouseUpdate(MActor^ avatar)
		{
			if(avatar != nullptr)
				GameClientModule::Instance()->SendMouseUpdate(avatar->m_actor);
			else
				GameClientModule::Instance()->SendMouseUpdate(NULL);
		}
	};

	// <summary>
	/// Provides a layer through which to animate and auto-update the camera
	// </summary>
	public ref class MCameraHandler
	{
	public:
		// <summary>
		/// sets the camera to a new view point
		// </summary>
		static void setToView(MVector^ location, MMatrix^ rotation)
		{
			CameraHandler::Instance()->setToView(*location->m_vector,*rotation->m_matrix);
		}

		// <summary>
		/// sets the camera to a new view point
		// </summary>
		static void setToView(MVector^ location, MMatrix^ rotation,float roll, float FOV)
		{
			CameraHandler::Instance()->setToView(*location->m_vector,*rotation->m_matrix,roll,FOV);
		}

		// <summary>
		/// Moves the camera to a new view point with optional interpolation
		// </summary>
		static void moveToView(float interpTime,MVector^ location, MMatrix^ rotation)
		{
			CameraHandler::Instance()->moveToView(interpTime,*location->m_vector,*rotation->m_matrix);
		}

		// <summary>
		/// Moves the camera to a new FOV with interpolation
		// </summary>
		static void moveToFOV(float interpTime, float FOV)
		{
			CameraHandler::Instance()->moveToFOV(interpTime,FOV);
		}

		// <summary>
		/// Moves the camera to a new view point with optional interpolation
		// </summary>
		static void moveToView(float interpTime,MVector^ location, MMatrix^ rotation,float Roll, float FOV)
		{
			CameraHandler::Instance()->moveToView(interpTime,*location->m_vector,*rotation->m_matrix,Roll,FOV);
		}

		// <summary>
		/// Whether the camera is currently doing an interpolated movement between views
		// </summary>
		static bool IsMoving()
		{
			return CameraHandler::Instance()->isInCameraMove();
		}

		// <summary>
		/// Updates the camera movement destination (or sets if not moving)
		// </summary>
		static void UpdateDestView(MVector^ location, MMatrix^ rotation,float Roll, float FOV)
		{
			CameraHandler::Instance()->updateDestView(*location->m_vector,*rotation->m_matrix,Roll,FOV);
		}

		// <summary>
		/// Applies the current camera values to the base render object, use if modifying MCameraHandler values outside of UpdateCamera()
		// </summary>
		static void ApplyCameraValues()
		{
			CameraHandler::Instance()->ApplyCameraValues();
		}


		// <summary>
		/// gets the current camera location
		// </summary>
		static MVector^ GetCameraLocation()
		{
			return gcnew MVector(CameraHandler::Instance()->GetCamera()->Location);
		}

		// <summary>
		/// gets the current camera direction
		// </summary>
		static MVector^ GetCameraDirection()
		{
			return gcnew MVector(CameraHandler::Instance()->GetCamera()->Direction);
		}
		static ref class MCamera^ GetCamera();
	};

	// <summary>
	/// Provides access to screen drawing functions
	// </summary>
	public ref class MCanvas
	{
	public:

		// <summary>
		/// various font types/sizes defined in Fonts.ini
		// </summary>
		static const int SystemFont = 0;
		static const int SmallFont = 1;
		static const int MediumFont = 2;
		static const int LargeFont = 3;
		static const int HUDFont = 4;


		// <summary>
		/// Absolute width of the unscaled canvas
		// </summary>
		static property int Width
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			int get()	
			{
				return Canvas::Instance()->Width;
			}
		}

		// <summary>
		/// Absolute height of the unscaled canvas
		// </summary>
		static property int Height
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			int get()	
			{
				return Canvas::Instance()->Height;
			}
		}

		// <summary>
		/// prints text to the screen left-aligned at a point scaled on 1024x768
		// </summary>
		static void Textf(int WhichFont,unsigned long Color,int x, int y, String^ text)
		{
			Canvas::Instance()->Textf((FontType)WhichFont,Color,x,y,Helpers::ToCppString(text).c_str());
		}

		// <summary>
		/// Gets the X size in 1024x768 virtual pixels of the specified text using the specified font
		// </summary>
		static int GetTextSizeX(int WhichFont, String^ text)
		{
			return Canvas::Instance()->GetTextSize((FontType)WhichFont,Helpers::ToCppString(text).c_str()).cx;
		}

		// <summary>
		/// Gets the Y size in 1024x768 virtual pixels of the specified text using the specified font
		// </summary>
		static int GetTextSizeY(int WhichFont, String^ text)
		{
			return Canvas::Instance()->GetTextSize((FontType)WhichFont,Helpers::ToCppString(text).c_str()).cy;
		}

		// <summary>
		/// Gets the X and Y sizes, via float refs, in 1024x768 virtual pixels of the specified text using the specified font
		// </summary>
		static void GetTextSize(int WhichFont, String^ text, float% X, float% Y)
		{
			SIZE size = Canvas::Instance()->GetTextSize((FontType)WhichFont,Helpers::ToCppString(text).c_str());
			X = size.cx;
			Y = size.cy;
		}

		// <summary>
		/// print text to the screen centered at a point scaled on 1024x768
		// </summary>
		static void TextCenteredf(int WhichFont,unsigned long Color,int x, int y, String^ text)
		{
			Canvas::Instance()->TextCenteredf((FontType)WhichFont,Color,x,y,x,y,Helpers::ToCppString(text).c_str());
		}

		// <summary>
		/// draws a billboard in world space with specified blend modes
		// </summary>
		static void BillBoard(MVector^ pos, float size, unsigned long color, MTexture^ texture, MBlendMode src,MBlendMode dest)
		{
			if(texture != nullptr)
				Canvas::Instance()->BillBoard(*pos->m_vector,size,color,texture->m_texture,(BlendMode)(int)src,(BlendMode)(int)dest);
			else
				Canvas::Instance()->BillBoard(*pos->m_vector,size,color,NULL,(BlendMode)(int)src,(BlendMode)(int)dest);
		}

		// <summary>
		/// draws a billboard in world space
		// </summary>
		static void BillBoard(MVector^ pos, float size, unsigned long color, MTexture^ texture)
		{
			if(texture != nullptr)
				Canvas::Instance()->BillBoard(*pos->m_vector,size,color,texture->m_texture,BLEND_SRCCOLOR,BLEND_INVSRCCOLOR);
			else
				Canvas::Instance()->BillBoard(*pos->m_vector,size,color,NULL,BLEND_SRCCOLOR,BLEND_INVSRCCOLOR);
		}

		// <summary>
		/// draws a textured box to the screen with top-left at point scaled on 1024x768, with blend modes
		// </summary>
		static void Box(unsigned long color, int x, int y, int width, int height, MTexture^ texture,MBlendMode src,MBlendMode dest)
		{
			Canvas::Instance()->Box(color,x,y,width,height,texture->m_texture,(BlendMode)(int)src,(BlendMode)(int)dest);
		}

		// <summary>
		/// draws an untextured box to the screen with top-left at point scaled on 1024x768, with blend modes
		// </summary>
		static void Box(unsigned long color, int x, int y, int width, int height, MBlendMode src,MBlendMode dest)
		{
			Canvas::Instance()->Box(color,x,y,width,height,NULL,(BlendMode)(int)src,(BlendMode)(int)dest);
		}

		// <summary>
		/// draws a textured box to the screen with top-left at point scaled on 1024x768
		// </summary>
		static void Box(unsigned long color, int x, int y, int width, int height, MTexture^ texture)
		{
			Canvas::Instance()->Box(color,x,y,width,height,texture->m_texture,BLEND_SRCCOLOR,BLEND_INVSRCCOLOR);
		}

		// <summary>
		/// draws a ritated textured box to the screen with top-left at point scaled on 1024x768, rotated about its the center point
		// </summary>
		static void RotatedBox(unsigned long color, int x, int y, int width, int height,float angleDeg, MTexture^ texture, MBlendMode src,MBlendMode dest)
		{
			Canvas::Instance()->RotatedBox(color,x,y,width,height,angleDeg,Vector(x + width/2,y + height/2,0),texture->m_texture,(BlendMode)(int)src,(BlendMode)(int)dest);
		}

		// <summary>
		/// draws a ritated textured box to the screen with top-left at point scaled on 1024x768, rotated about an arbritrary origin screen point (X, Y, 0)
		// </summary>
		static void RotatedBox(unsigned long color, int x, int y, int width, int height,float angleDeg,MVector^ origin, MTexture^ texture, MBlendMode src,MBlendMode dest)
		{
			Vector originVec = *origin->m_vector;
			Canvas::Instance()->RotatedBox(color,x,y,width,height,angleDeg,originVec,texture->m_texture,(BlendMode)(int)src,(BlendMode)(int)dest);
		}

	};

	// <summary>
	/// Provides precaching functionality to pre-load, manage, and purge class' static media
	// </summary>
	public ref class MPrecacher
	{
	public:

		// <summary>
		/// Loads all the registered resources for a given class name (if not already loaded). Will exit out immediately if hasCached = true, to provide inline-style bail-out.
		// </summary>
		static bool Precache(String^ ClassName, bool hasCached);

		// <summary>
		/// Destroys all the registered resources for a given class name
		// </summary>
		static void PurgeCache(String^ ClassName);

		// <summary>
		/// Destroys all the registered resources for all registered classes (called automatically upon app destruction, but could be called in between World loads on a memory-constricted platform)
		// </summary>
		static void PurgeTotalCache();

		// <summary>
		/// Registered a class' Model resource for precaching and returns a reference to the Model
		// </summary>
		static MModel^ PrecacheModel(String^ ClassName,String^ FileName);

		// <summary>
		/// Registered a class' Material resource for precaching and returns a reference to the Material
		// </summary>
		static MMaterial^ PrecacheMaterial(String^ ClassName,String^ FileName);

		// <summary>
		/// Registered a class' Texture resource for precaching and returns a reference to the Texture
		// </summary>
		static MTexture^ PrecacheTexture(String^ ClassName,String^ FileName);

		// <summary>
		/// Registered a class' Sound resource (either 2d or 3d) for precaching and returns a reference to the Sound 
		// </summary>
		static MSound^ PrecacheSound(String^ ClassName,String^ FileName, bool b2D);
	};


	// <summary>
	/// Wrapper for ConfigFile class, which loads and parses config files. Contains reference to the static app configfile too.
	// </summary>
	public ref class MConfigFile
	{
		private public:
			ConfigFile* m_configfile;
			bool needToDelete;
	public:

		// <summary>
		/// the static app configfile, such as RealityEngine.ini
		// </summary>
		static MConfigFile^ MainConfig = gcnew MConfigFile(Engine::Instance()->MainConfig);

		// <summary>
		/// gets a boolean key from the file
		// </summary>
		bool GetBool(String^ key)
		{
			return m_configfile->GetBool(Helpers::ToCppString(key));
		}

		// <summary>
		/// sets a boolean key in the file
		// </summary>
		void SetBool(String^ key, bool val)
		{
			string s = Helpers::ToCppString(key);
			bool v = val;
			m_configfile->SetBool(s,v);
		}

		// <summary>
		/// gets a float key from the file
		// </summary>
		float GetFloat(String^ key)
		{
			return m_configfile->GetFloat(Helpers::ToCppString(key));
		}

		// <summary>
		/// sets a float key in the file
		// </summary>
		void SetFloat(String^ key, float val)
		{
			string s = Helpers::ToCppString(key);
			float v = val;
			m_configfile->SetFloat(s,v);
		}

		// <summary>
		/// gets an integer key from the file
		// </summary>
		int GetInt(String^ key)
		{
			return m_configfile->GetInt(Helpers::ToCppString(key));
		}

		// <summary>
		/// sets an integer key in the file
		// </summary>
		void SetInt(String^ key, int val)
		{
			string s = Helpers::ToCppString(key);
			int v = val;
			m_configfile->SetInt(s,v);
		}


		// <summary>
		/// gets a string key from the file
		// </summary>
		String^ GetString(String^ key)
		{
			return Helpers::ToCLIString(m_configfile->GetString(Helpers::ToCppString(key)));
		}

		// <summary>
		/// sets a string key in the file
		// </summary>
		void SetString(String^ key, String^ val)
		{
			string s = Helpers::ToCppString(key);
			string v = Helpers::ToCppString(val);
			m_configfile->SetString(s,v);
		}

		// <summary>
		/// ctor for new configfile
		// </summary>
		MConfigFile()
		{
			m_configfile = new ConfigFile();
			needToDelete = true;
		}

		// <summary>
		/// ctor to wrap existing configfile
		// </summary>
		MConfigFile(ConfigFile* configFile)
		{
			m_configfile = configFile;
			needToDelete = false;
		}

		// <summary>
		/// dtor
		// </summary>
		virtual void Finalize()
		{
			if (needToDelete &&	m_configfile)
				delete m_configfile;
		}
	};

	// <summary>
	/// wrapper for a shader effect drawn over the screen
	// </summary>
	public ref class MPostProcessEffect
	{
		private public:
			String^ ShaderFile;
			String^ TechniqueName;
			bool disposed;
	public:

		// <summary>
		/// Renders all PostProcess Effects that are currently Active on the stack
		// </summary>
		static void RenderEffects(){PostProcess::Instance()->PostRender();}

		// <summary>
		/// ctor for a postprocess effect, default active==false
		// </summary>
		MPostProcessEffect(String^ shaderFile, String^ techniqueName)
		{
			ShaderFile = shaderFile;
			TechniqueName = techniqueName;
			new PostProcessEffect_Permanent(Helpers::ToCppString(ShaderFile),Helpers::ToCppString(TechniqueName),false);
			SetActive(false);
		}

		// <summary>
		/// ctor for a postprocess effect, specifies initial active state and a preferred index for manual ordering of stacked effects
		// </summary>
		MPostProcessEffect(String^ shaderFile, String^ techniqueName, bool StartActive, int orderingPreference)
		{
			ShaderFile = shaderFile;
			TechniqueName = techniqueName;
			new PostProcessEffect_Permanent(Helpers::ToCppString(ShaderFile),Helpers::ToCppString(TechniqueName),StartActive,orderingPreference);
			SetActive(false);
		}

		// <summary>
		/// sets active state of the post-process effect. inactive postprocess effects won't be drawn and come at no expense beyond a list entry.
		// </summary>
		void SetActive(bool active)
		{
			PostProcessEffect* effect = PostProcess::Instance()->FindPostProcessEffect(Helpers::ToCppString(ShaderFile),Helpers::ToCppString(TechniqueName));
			if(effect)
				effect->Active = active;
		}

		// <summary>
		/// activates the post-process effect for just this frame
		// </summary>
		void ActivateOneFrame()
		{
			PostProcessEffect* effect = PostProcess::Instance()->FindPostProcessEffect(Helpers::ToCppString(ShaderFile),Helpers::ToCppString(TechniqueName));
			if(effect)
				effect->ActiveOneFrame = true;
		}

		// <summary>
		/// dtor
		// </summary>
		virtual void Finalize()
		{
			Dispose(false);
			disposed = true;

		}		

		// <summary>
		/// disposal
		// </summary>
		void Dispose() 
		{
			Dispose(true);
			disposed = true;
			GC::SuppressFinalize(this);
		}

		// <summary>
		/// sets a float value in the post-process effect, such as the blurring amount in a blur effect
		// </summary>
		void SetFloatVar(String^ VarName, float value)
		{
			PostProcessEffect* effect = PostProcess::Instance()->FindPostProcessEffect(Helpers::ToCppString(ShaderFile),Helpers::ToCppString(TechniqueName));
			if(effect)
				effect->Mat->m_Shader->GetEffect()->SetFloat( Helpers::ToCppString(VarName).c_str(), value );
		}

	protected:

		// <summary>
		/// disposes unmanaged resources
		// </summary>
		virtual void Dispose(bool disposing) 
		{
			if (!disposed) 
			{
				if (disposing) 
				{
					// Dispose managed resources.
				}
					// Dispose managed resources.
					PostProcessEffect_Permanent* effect = (PostProcessEffect_Permanent*)PostProcess::Instance()->FindPostProcessEffect(Helpers::ToCppString(ShaderFile),Helpers::ToCppString(TechniqueName));
					if(effect)
					{
						effect->Release();
						PostProcess::Instance()->RemovePostProcessEffect(effect);
						delete effect;
					}
			}
		}
	};


	// <summary>
	/// Overridable Game Logic class to allow custom global static behavior of a game through script -- such as initializing, ticking, and rendering
	// </summary>
	public ref class LogicCore
	{
	public:

		// <summary>
		/// ctor & initializer
		// </summary>
		LogicCore() { }

		// <summary>
		/// tick per world
		// </summary>
		virtual void Tick(MWorld^ world) { }

		// <summary>
		/// postrender per world
		// </summary>
		virtual void PostRender(MWorld^ world, ref class MCamera^ camera) { }

		// <summary>
		/// prerender per world
		// </summary>
		virtual void PreRender(MWorld^ world, ref class MCamera^ camera) { }

		// <summary>
		/// on per world
		// </summary>
		virtual void OnRender(MWorld^ world, ref class MCamera^ camera) { }

		// <summary>
		/// shutdown per app
		// </summary>
		virtual void ShutDown() { }

		// <summary>
		/// load a new world
		// </summary>
		virtual void LoadMap(MWorld^ loadWorld) { }

		// <summary>
		/// processes a console command preceded with "/", supports networked console commands too (RCON)
		// </summary>
		virtual void ProcessConsoleCommand(ref class MNetworkClient^ client, String^ command){}

		// <summary>
		/// Handles the event of a new client requesting to join the game after connecting, such as spawning an avatar for it or sending it some custom data
		// </summary>
		virtual void NetworkClientJoined(ref class MNetworkClient^ client){}

		// <summary>
		/// Handles the event of a new client leaving the game, any custom logic that the game wants to run
		// </summary>
		virtual void NetworkClientLeft(ref class MNetworkClient^ client){}

		// <summary>
		/// Called upon changing the game preferences via the GUI, allows updating of custom game preference values
		// </summary>
		virtual void ReloadPreferences(){}
	};


	// <summary>
	/// representative object that can is passed to MFXManager functions for quick look-up of a returned effect added to the c++ optimized-batched fx manager
	// </summary>
	public ref struct FXSystemHash
	{
		private public:
			unsigned int HashIndex;
			FXSystem* systemPointer;
	};

	// <summary>
	/// provides functions to interact with the optimized c++ batched fx manager
	// </summary>
	public ref class MFXManager
	{
	public:

		// <summary>
		/// adds a surface decal to the c++ fx manager
		// </summary>
		static void AddSurfaceDecal(MWorld^ world,MTexture^ tex,unsigned long color, MBlendMode destblend,MVector^ pos,MVector^ normal, float size,float lifeTime,float fadeTime)
		{
			new SurfaceDecal(world->m_world,tex->m_texture, *pos->m_vector, *normal->m_vector, color, size, lifeTime, fadeTime,(BlendMode)(int)destblend);
		}

		// <summary>
		/// adds a particle system to the c++ fx manager, returning a hash object that can be used to modify the system afterwards
		// </summary>
		static FXSystemHash^ AddParticleSystemHash(MWorld^ world,MTexture^ tex,unsigned long color, MBlendMode destblend,float spawnInterval,int numParticles, int maxParticles, MVector^ centerpos, MVector^ spraydirection, MVector^ acceleration, float particleSpeed, float particleDirVariance, float particleSpeedVariance, float particleLifeTimeMS, float particleLifeTimeVarianceMS, float particleTimeFadeOutMS, float particleSizeMeters, float particleSizeVarianceMeters)
		{
			FXSystemHash^ hash = gcnew FXSystemHash();
			ParticleSystem* ps = new ParticleSystem(world->m_world,color,spawnInterval,numParticles,maxParticles,*centerpos->m_vector,*spraydirection->m_vector,*acceleration->m_vector,particleSpeed,particleDirVariance,particleSpeedVariance,particleLifeTimeMS,particleLifeTimeVarianceMS,particleTimeFadeOutMS,particleSizeMeters,particleSizeVarianceMeters,false,tex->m_texture,(BlendMode)(int)destblend);			
			ps->RegisterWithHash();
			hash->HashIndex = ps->HashIndex;
			hash->systemPointer = ps;
			return hash;
		}

		// <summary>
		/// adds a particle system to the c++ fx manager
		// </summary>
		static void AddParticleSystem(MWorld^ world,MTexture^ tex,unsigned long color, MBlendMode destblend,float spawnInterval,int numParticles, int maxParticles, MVector^ centerpos, MVector^ spraydirection, MVector^ acceleration, float particleSpeed, float particleDirVariance, float particleSpeedVariance, float particleLifeTimeMS, float particleLifeTimeVarianceMS, float particleTimeFadeOutMS, float particleSizeMeters, float particleSizeVarianceMeters, float emitterLifeTimeMS, bool StopEmittingImmediately)
		{
			ParticleSystem* ps = new ParticleSystem(world->m_world,color,spawnInterval,numParticles,maxParticles,*centerpos->m_vector,*spraydirection->m_vector,*acceleration->m_vector,particleSpeed,particleDirVariance,particleSpeedVariance,particleLifeTimeMS,particleLifeTimeVarianceMS,particleTimeFadeOutMS,particleSizeMeters,particleSizeVarianceMeters,false,tex->m_texture,(BlendMode)(int)destblend);			
			ps->LifeTime = emitterLifeTimeMS/1000.0f;
			if(StopEmittingImmediately)
				ps->StopSpawningNow();
		}

		// <summary>
		/// set properties of a particle system referenced by its hash
		// </summary>
		static void SetParticleSystemProperties(FXSystemHash^ systemHash, unsigned long color, MBlendMode destblend,float spawnInterval, int maxParticles, MVector^ centerpos, MVector^ spraydirection, MVector^ acceleration, float particleSpeed, float particleDirVariance, float particleSpeedVariance, float particleLifeTimeMS, float particleLifeTimeVarianceMS, float particleTimeFadeOutMS, float particleSizeMeters, float particleSizeVarianceMeters, MVector^ posVariance, bool preventIndoors, float YaxisConstraint);
		static void SetParticleSystemColor(FXSystemHash^ systemHash, unsigned long color);
		static void SetParticleSystemTransformation(FXSystemHash^ systemHash, MVector^ centerpos, MVector^ spraydirection);
		static void SetParticleSystemAlphaMultiplier(FXSystemHash^ systemHash, float alphaMultiplier);

		// <summary>
		/// adds a bouncy particle system to the c++ fx manager -- bouncy particles will efficiently 'collide' with the floor and bounce back
		// </summary>
		static void AddBouncyParticleSystem(MWorld^ world,MTexture^ tex,unsigned long color, MBlendMode destblend,float spawnInterval,int numParticles, int maxParticles, MVector^ centerpos, MVector^ spraydirection, MVector^ acceleration, float particleSpeed, float particleDirVariance, float particleSpeedVariance, float particleLifeTimeMS, float particleLifeTimeVarianceMS, float particleTimeFadeOutMS, float particleSizeMeters, float particleSizeVarianceMeters, float emitterLifeTimeMS, bool StopEmittingImmediately)
		{
			BouncyParticles* ps = new BouncyParticles(world->m_world,color,spawnInterval,numParticles,maxParticles,*centerpos->m_vector,*spraydirection->m_vector,*acceleration->m_vector,particleSpeed,particleDirVariance,particleSpeedVariance,particleLifeTimeMS,particleLifeTimeVarianceMS,particleTimeFadeOutMS,particleSizeMeters,particleSizeVarianceMeters,false,tex->m_texture,(BlendMode)(int)destblend);			
			ps->LifeTime = emitterLifeTimeMS/1000.0f;
			if(StopEmittingImmediately)
				ps->StopSpawningNow();
		}

		// <summary>
		/// adds a laser particle system to the c++ fx manager -- laser particles are drawn with only their y-axis oriented towards the camera so that they appear to have both direction and width
		// </summary>
		static void AddLaserParticleSystem(MWorld^ world,MTexture^ tex,MBlendMode destblend,bool bouncyParticles,float laserLength,float spawnInterval,int numParticles, int maxParticles, MVector^ centerpos, MVector^ spraydirection, MVector^ acceleration, float particleSpeed, float particleDirVariance, float particleSpeedVariance, float particleLifeTimeMS, float particleLifeTimeVarianceMS, float particleTimeFadeOutMS, float particleSizeMeters, float particleSizeVarianceMeters, float emitterLifeTimeMS, bool StopEmittingImmediately);

		// <summary>
		/// adds a laser to the c++ fx manager and returns a hash object which can be used for quick-look-up later, to alter the laser's parameters over time.  laser are drawn with only their y-axis oriented towards the camera so that they appear to have both direction and width
		// </summary>
		static FXSystemHash^ AddLaser(MWorld^ world,MTexture^ tex,unsigned long color,float width, MVector^ startPos, MVector^ endPos, MBlendMode destblend);

		// <summary>
		/// adds a light beam to the c++ fx manager and returns a hash object which can be used for quick-look-up later, to alter the laser's parameters over time.  light beams are volumetric cones used to portray the phenomena of light reflecting on airborne particles
		// </summary>
		static FXSystemHash^ AddLightBeam(MWorld^ world,MTexture^ tex,unsigned long color,int NumUnits, float DistanceMultiplier, float StartSize, float SizeMultiplier,float TotalScalingFactor);

		// <summary>
		/// Sets the position and direction of a light beam referenced by its hash
		// </summary>
		static void SetLightBeamTransformation(FXSystemHash^ systemHash, MVector^ Pos, MVector^ Dir);

		// <summary>
		/// Sets all the properties of an existing light beam, looked up by its hash ref
		// </summary>
		static void SetLightBeamProperties(FXSystemHash^ systemHash, unsigned long color,int NumUnits, float DistanceMultiplier, float StartSize, float SizeMultiplier,float TotalScalingFactor);

		// <summary>
		/// Sets the color of an existing light beam, looked up by its hash ref
		// </summary>
		static void SetLightBeamColor(FXSystemHash^ systemHash, unsigned long color);

		// <summary>
		/// adds a surface decal to the c++ fx manager, returning a hash that can be used to alter it later
		// </summary>
		static FXSystemHash^ AddSurfaceDecalHash(MWorld^ world,MTexture^ tex,unsigned long color, MBlendMode destblend,MVector^ pos,MVector^ normal, float size,float lifeTime,float fadeTime);

		// <summary>
		/// Sets the position and direction of a surface decal referenced by its hash
		// </summary>
		static void SetSurfaceDecalTransformation(FXSystemHash^ systemHash, MVector^ Pos, MVector^ Dir);

		// <summary>
		/// sets the endpoints of a laser found with the hash object, if any (if it expires, it's still safe)
		// </summary>
		static void SetLaserEndPoints(FXSystemHash^ systemHash,MVector^ startPos, MVector^ endPos);

		// <summary>
		/// sets the color of a laser found with the hash object, if any (if it expires, it's still safe)
		// </summary>
		static void SetLaserColor(FXSystemHash^ systemHash, unsigned long color);

		// <summary>
		/// sets the hidden state of an effect found with the hash object, if any (if it expires, it's still safe)
		// </summary>
		static void SetEffectHidden(FXSystemHash^ systemHash, bool IsHidden);

		// <summary>
		/// destroys a particular effect in fx manager found with the hash value, if any (safe if expired)
		// </summary>
		static void DestroyEffect(FXSystemHash^ systemHash);

		// <summary>
		/// sets the World that an Effect is rendered in. Set to NULL for the effect not to be rendered at all (will still be destroyed when the C++ FXManager is Reset).
		// </summary>
		static void SetEffectWorld(FXSystemHash^ systemHash,MWorld^ world);
	};


	// <summary>
	/// Wraps a Camera. Cameras are used to define the player's world-space orientation, positioning, speed, and field of view.
	// </summary>
	public ref class MCamera
	{
		private public:
			Camera*	 m_camera;
			MVector^ m_location;
			MVector^ m_direction;
			// <summary>
			/// wrap an existing Camera
			// </summary>
			MCamera(Camera* camera)
			{
				m_camera = camera;
				m_location = gcnew MVector(&m_camera->Location);
				m_direction = gcnew MVector(&m_camera->Direction);
			}
	public:

		// <summary>
		/// Location of the Camera
		// </summary>
		property MVector^ Location
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			MVector^ get()
			{
				return m_location;
			}

			void set(MVector^ value)
			{
				*m_location->m_vector=*value->m_vector;
			}
		}

		// <summary>
		/// Direction of the Camera
		// </summary>
		property MVector^ Direction
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			MVector^ get()
			{
				return m_direction;
			}

			void set(MVector^ value)
			{
				*m_direction->m_vector=*value->m_vector;
			}
		}

		// <summary>
		/// Near clipping distance in meters
		// </summary>
		property float NearClip
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get()	
			{
				return m_camera->NearClip;
			}
			void set(float value) 
			{
				m_camera->NearClip=value;
			}
		}

		// <summary>
		/// Far clipping distance in meters
		// </summary>
		property float FarClip
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get()	
			{
				return m_camera->FarClip;
			}
			void set(float value) 
			{
				m_camera->FarClip=value;
			}
		}

		// <summary>
		/// Field-of-view in degrees, default 90
		// </summary>
		property float Fov
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get()	
			{
				return m_camera->Fov;
			}
			void set(float value) 
			{
				m_camera->Fov=value;
			}
		}

		// <summary>
		/// View matrix of the camera for manual view tranformation (useful if in a gimbal lock situation, such as for a flight sim)
		// </summary>
		property MMatrix^ View
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			MMatrix^ get()
			{
				return gcnew MMatrix(m_camera->view);
			}
			void set(MMatrix^ value)
			{
				m_camera->view = *value->m_matrix;
			}
		}

		// <summary>
		/// View Projection matrix of the camera
		// </summary>
		property MMatrix^ ViewProjection
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			MMatrix^ get()
			{
				return gcnew MMatrix(m_camera->viewProjection);
			}
			void set(MMatrix^ value)
			{
				m_camera->viewProjection = *value->m_matrix;
			}
		}

		// <summary>
		/// Projection matrix of the camera
		// </summary>
		property MMatrix^ Projection
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			MMatrix^ get()
			{
				return gcnew MMatrix(m_camera->projection);
			}
			void set(MMatrix^ value)
			{
				m_camera->projection = *value->m_matrix;
			}
		}

		// <summary>
		/// Creates the clip planes from the current projection
		// </summary>
		void CreateClipPlanes()
		{
			m_camera->CreateClipPlanes();
		}

		// <summary>
		/// Automatically calculates all the camera's Matrix values
		// </summary>
		void Update()
		{
			Matrix view = m_camera->view;
			m_camera->Update();
			m_camera->view = view;
			m_camera->viewProjection = m_camera->view * m_camera->projection;
		}

		// <summary>
		/// Sets the camera as the transformation for any subsequent rendering
		// </summary>
		void SetForRendering()
		{
			RenderWrap::SetView(m_camera->view);
			RenderWrap::SetProjection(m_camera->projection);
		}

		// <summary>
		/// Gets screen coordinates (scaled 1024x768) from a world point with respect to this camera's view
		// </summary>
		MVector^ GetCoords_ScreenFromWorld(MVector^ posInWorld, bool doNotCull);

		// <summary>
		/// Gets world coordinates with respect to this camera's view (scaled 1024x768) from a screen point, using the current viewport Z values
		// </summary>
		MVector^ GetCoords_WorldFromScreen(MVector^ posOnScreen);
	};


	// <summary>
	/// Provides access to information about the SkyController, which manages & renders Animated Day/Night Sky system and is networked
	// </summary>
	public ref class MSkyController
	{
	public:
		// <summary>
		/// Gets the Day Time in minutes of the sky, if no sky returns noon.
		// </summary>
		static float GetDayTime();

		// <summary>
		/// Sets the Day Time in minutes of the sky
		// </summary>
		static void SetDayTime(float daytimeminutes);

		// <summary>
		/// Sets the speed of the sky day/night cycle, in Sky Minutes per Game Second.
		// </summary>
		static void SetSpeed(float minutesPerGameSecond);

		// <summary>
		/// Gets the current color of the Sky light
		// </summary>
		static unsigned long GetAmbientColor(MVector^ atLocation);

		// <summary>
		/// Gets the Sky Light itself
		// </summary>
		static MLight^ GetAmbientLight();

		// <summary>
		/// Whether there is a Sky in the specified World
		// </summary>
		static bool HasSky(MWorld^ world);

		// <summary>
		/// Gets the current sun position, non-normalized so it's many miles out (useful for world-to-screen calculations)).
		// </summary>
		static MVector^ GetSunPosition();

		// <summary>
		/// Gets the current lightning intensity of the Sky, with 0 being no lightning currently flashing
		// </summary>
		static float GetLightningIntensity();

		// <summary>
		/// Whether Reality renders the Sky, only can be set once a SkyController effect is active
		// </summary>
		static property bool RenderSky
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			bool get()
			{
				if(SkyController::Instance)
					return SkyController::Instance->bRender;
		
				return false;
			}
			void set(bool value)
			{
				if(SkyController::Instance)
					SkyController::Instance->bRender = value;
			}
		}

		// <summary>
		/// Whether Reality renders Clouds, only can be set once a SkyController effect is active
		// </summary>
		static property bool RenderClouds
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			bool get()
			{
				if(SkyController::Instance)
					return SkyController::Instance->HasClouds;
		
				return false;
			}
			void set(bool value)
			{
				if(SkyController::Instance)
					SkyController::Instance->HasClouds = value;
			}
		}
	};

	// <summary>
	/// Provides interface to control the static OggPlayer, which streams Ogg files.
	// </summary>
	public ref class MOggPlayer
	{
	public:

		// <summary>
		/// whether the OggPlayer is currently playing a stream
		// </summary>
		static bool IsPlaying(){return oggPlayer.IsPlaying();}

		// <summary>
		/// Stops the OggPlayer's currently loaded stream
		// </summary>
		static void Stop(){oggPlayer.Stop();}

		// <summary>
		/// Sets the volume of the OggPlayer's currently loaded stream
		// </summary>
		static void SetVolume(float volume){oggPlayer.SetVolume(volume);}

		// <summary>
		/// Plays the OggPlayer's currently loaded stream
		// </summary>
		static void Play(bool loop){oggPlayer.Play(loop);}

		// <summary>
		/// Opens a stream for the OggPlayer from the specified OggFilePath, takes app-relative path (use with findMedia).
		// </summary>
		static void Open(String^ OggFilePath){oggPlayer.OpenOgg(Helpers::ToCppString(OggFilePath).c_str());}

		// <summary>
		/// Opens a stream for the OggPlayer from the specified OggFilePath, takes app-relative path (use with findMedia). Also specifies a stream buffer size to use, in bytes.
		// </summary>
		static void Open(String^ OggFilePath, int BufferSize){oggPlayer.OpenOgg(Helpers::ToCppString(OggFilePath).c_str(), BufferSize);}
	};

	// <summary>
	/// Wraps functionality associated with the Reality Builder editor classes
	// </summary>
	public ref class MEditor
	{
	public:
		// <summary>
		/// returns the Editor Camera
		// </summary>		
		static MCamera^ GetEditorCamera(){return gcnew MCamera(&Editor::Instance()->m_Camera);}
	};

	// <summary>
	/// Sampler state type
	// </summary>
	public enum class  SamplerStateType
	{
		ADDRESSU       = 1,  /* TEXTUREADDRESS for U coordinate */
		ADDRESSV       = 2,  /* TEXTUREADDRESS for V coordinate */
		ADDRESSW       = 3,  /* TEXTUREADDRESS for W coordinate */
		BORDERCOLOR    = 4,  /* COLOR */
		MAGFILTER      = 5,  /* TEXTUREFILTER filter to use for magnification */
		MINFILTER      = 6,  /* TEXTUREFILTER filter to use for minification */
		MIPFILTER      = 7,  /* TEXTUREFILTER filter to use between mipmaps during minification */
		MIPMAPLODBIAS  = 8,  /* float Mipmap LOD bias */
		MAXMIPLEVEL    = 9,  /* DWORD 0..(n-1) LOD index of largest map to use (0 == largest) */
		MAXANISOTROPY  = 10, /* DWORD maximum anisotropy */
		SRGBTEXTURE    = 11, /* Default = 0 (which means Gamma 1.0,
							 no correction required.) else correct for
							 Gamma = 2.2 */
							 ELEMENTINDEX   = 12, /* When multi-element texture is assigned to sampler, this
												  indicates which element index to use.  Default = 0.  */
												  DMAPOFFSET     = 13, /* Offset in vertices in the pre-sampled displacement map.
																	   Only valid for DMAPSAMPLER sampler  */
	};

	// <summary>
	/// Texture filter type
	// </summary>
	public enum class  SSFilterType
	{
		NONE            = 0,    // filtering disabled (valid for mip filter only)
		POINT           = 1,    // nearest
		LINEAR          = 2,    // linear interpolation
		ANISOTROPIC     = 3,    // anisotropic
		PYRAMIDALQUAD   = 6,    // 4-sample tent
		GAUSSIANQUAD    = 7,    // 4-sample gaussian
	};

	// <summary>
	/// Render State type
	// </summary>
	public enum class  RenderStateType
	{
		ZENABLE                   = 7,    /* ZBUFFERTYPE (or TRUE/FALSE for legacy) */
		FILLMODE                  = 8,    /* FILLMODE */
		SHADEMODE                 = 9,    /* SHADEMODE */
		ZWRITEENABLE              = 14,   /* TRUE to enable z writes */
		ALPHATESTENABLE           = 15,   /* TRUE to enable alpha tests */
		LASTPIXEL                 = 16,   /* TRUE for last-pixel on lines */
		SRCBLEND                  = 19,   /* BLEND */
		DESTBLEND                 = 20,   /* BLEND */
		CULLMODE                  = 22,   /* CULL */
		ZFUNC                     = 23,   /* CMPFUNC */
		ALPHAREF                  = 24,   /* FIXED */
		ALPHAFUNC                 = 25,   /* CMPFUNC */
		DITHERENABLE              = 26,   /* TRUE to enable dithering */
		ALPHABLENDENABLE          = 27,   /* TRUE to enable alpha blending */
		FOGENABLE                 = 28,   /* TRUE to enable fog blending */
		SPECULARENABLE            = 29,   /* TRUE to enable specular */
		FOGCOLOR                  = 34,   /* COLOR */
		FOGTABLEMODE              = 35,   /* FOGMODE */
		FOGSTART                  = 36,   /* Fog start (for both vertex and pixel fog) */
		FOGEND                    = 37,   /* Fog end      */
		FOGDENSITY                = 38,   /* Fog density  */
		RANGEFOGENABLE            = 48,   /* Enables range-based fog */
		STENCILENABLE             = 52,   /* BOOL enable/disable stenciling */
		STENCILFAIL               = 53,   /* STENCILOP to do if stencil test fails */
		STENCILZFAIL              = 54,   /* STENCILOP to do if stencil test passes and Z test fails */
		STENCILPASS               = 55,   /* STENCILOP to do if both stencil and Z tests pass */
		STENCILFUNC               = 56,   /* CMPFUNC fn.  Stencil Test passes if ((ref & mask) stencilfn (stencil & mask)) is true */
		STENCILREF                = 57,   /* Reference value used in stencil test */
		STENCILMASK               = 58,   /* Mask value used in stencil test */
		STENCILWRITEMASK          = 59,   /* Write mask applied to values written to stencil buffer */
		TEXTUREFACTOR             = 60,   /* COLOR used for multi-texture blend */
		WRAP0                     = 128,  /* wrap for 1st texture coord. set */
		WRAP1                     = 129,  /* wrap for 2nd texture coord. set */
		WRAP2                     = 130,  /* wrap for 3rd texture coord. set */
		WRAP3                     = 131,  /* wrap for 4th texture coord. set */
		WRAP4                     = 132,  /* wrap for 5th texture coord. set */
		WRAP5                     = 133,  /* wrap for 6th texture coord. set */
		WRAP6                     = 134,  /* wrap for 7th texture coord. set */
		WRAP7                     = 135,  /* wrap for 8th texture coord. set */
		CLIPPING                  = 136,
		LIGHTING                  = 137,
		AMBIENT                   = 139,
		FOGVERTEXMODE             = 140,
		COLORVERTEX               = 141,
		LOCALVIEWER               = 142,
		NORMALIZENORMALS          = 143,
		DIFFUSEMATERIALSOURCE     = 145,
		SPECULARMATERIALSOURCE    = 146,
		AMBIENTMATERIALSOURCE     = 147,
		EMISSIVEMATERIALSOURCE    = 148,
		VERTEXBLEND               = 151,
		CLIPPLANEENABLE           = 152,
		POINTSIZE                 = 154,   /* float point size */
		POINTSIZE_MIN             = 155,   /* float point size min threshold */
		POINTSPRITEENABLE         = 156,   /* BOOL point texture coord control */
		POINTSCALEENABLE          = 157,   /* BOOL point size scale enable */
		POINTSCALE_A              = 158,   /* float point attenuation A value */
		POINTSCALE_B              = 159,   /* float point attenuation B value */
		POINTSCALE_C              = 160,   /* float point attenuation C value */
		MULTISAMPLEANTIALIAS      = 161,  // BOOL - set to do FSAA with multisample buffer
		MULTISAMPLEMASK           = 162,  // DWORD - per-sample enable/disable
		PATCHEDGESTYLE            = 163,  // Sets whether patch edges will use float style tessellation
		DEBUGMONITORTOKEN         = 165,  // DEBUG ONLY - token to debug monitor
		POINTSIZE_MAX             = 166,   /* float point size max threshold */
		INDEXEDVERTEXBLENDENABLE  = 167,
		COLORWRITEENABLE          = 168,  // per-channel write enable
		TWEENFACTOR               = 170,   // float tween factor
		BLENDOP                   = 171,   // BLENDOP setting
		POSITIONDEGREE            = 172,   // NPatch position interpolation degree. DEGREE_LINEAR or DEGREE_CUBIC (default)
		NORMALDEGREE              = 173,   // NPatch normal interpolation degree. DEGREE_LINEAR (default) or DEGREE_QUADRATIC
		SCISSORTESTENABLE         = 174,
		SLOPESCALEDEPTHBIAS       = 175,
		ANTIALIASEDLINEENABLE     = 176,
		MINTESSELLATIONLEVEL      = 178,
		MAXTESSELLATIONLEVEL      = 179,
		ADAPTIVETESS_X            = 180,
		ADAPTIVETESS_Y            = 181,
		ADAPTIVETESS_Z            = 182,
		ADAPTIVETESS_W            = 183,
		ENABLEADAPTIVETESSELLATION = 184,
		TWOSIDEDSTENCILMODE       = 185,   /* BOOL enable/disable 2 sided stenciling */
		CCW_STENCILFAIL           = 186,   /* STENCILOP to do if ccw stencil test fails */
		CCW_STENCILZFAIL          = 187,   /* STENCILOP to do if ccw stencil test passes and Z test fails */
		CCW_STENCILPASS           = 188,   /* STENCILOP to do if both ccw stencil and Z tests pass */
		CCW_STENCILFUNC           = 189,   /* CMPFUNC fn.  ccw Stencil Test passes if ((ref & mask) stencilfn (stencil & mask)) is true */
		COLORWRITEENABLE1         = 190,   /* Additional ColorWriteEnables for the devices that support PMISCCAPS_INDEPENDENTWRITEMASKS */
		COLORWRITEENABLE2         = 191,   /* Additional ColorWriteEnables for the devices that support PMISCCAPS_INDEPENDENTWRITEMASKS */
		COLORWRITEENABLE3         = 192,   /* Additional ColorWriteEnables for the devices that support PMISCCAPS_INDEPENDENTWRITEMASKS */
		BLENDFACTOR               = 193,   /* COLOR used for a constant blend factor during alpha blending for devices that support PBLENDCAPS_BLENDFACTOR */
		SRGBWRITEENABLE           = 194,   /* Enable rendertarget writes to be DE-linearized to SRGB (for formats that expose USAGE_QUERY_SRGBWRITE) */
		DEPTHBIAS                 = 195,
		WRAP8                     = 198,   /* Additional wrap states for vs_3_0+ attributes with DECLUSAGE_TEXCOORD */
		WRAP9                     = 199,
		WRAP10                    = 200,
		WRAP11                    = 201,
		WRAP12                    = 202,
		WRAP13                    = 203,
		WRAP14                    = 204,
		WRAP15                    = 205,
		SEPARATEALPHABLENDENABLE  = 206,  /* TRUE to enable a separate blending function for the alpha channel */
		SRCBLENDALPHA             = 207,  /* SRC blend factor for the alpha channel when SEPARATEDESTALPHAENABLE is TRUE */
		DESTBLENDALPHA            = 208,  /* DST blend factor for the alpha channel when SEPARATEDESTALPHAENABLE is TRUE */
		BLENDOPALPHA              = 209,  /* Blending operation for the alpha channel when D3DSEPARATEDESTALPHAENABLE is TRUE */
	};

	// <summary>
	/// Wraps functionality associated with the device's rendering state
	// </summary>
	public ref class MRenderWrap
	{
	public:

		// <summary>
		/// Sets the specified stage Sampler State to a value
		// </summary>
		static void SetSS(int stage, SamplerStateType samplerState, SSFilterType value)
		{
			RenderWrap::SetSS(stage,(D3DSAMPLERSTATETYPE)(DWORD)samplerState, (DWORD)value);
		}

		// <summary>
		/// Gets the value of the specified stage Sampler State
		// </summary>
		static SSFilterType GetSS(int stage, SamplerStateType samplerState)
		{
			return (SSFilterType)RenderWrap::GetSS(stage,(D3DSAMPLERSTATETYPE)(DWORD)samplerState);
		}

		// <summary>
		/// Sets the specified Render State to a value
		// </summary>
		static unsigned int SetRS(RenderStateType renderState, unsigned int value)
		{
			return RenderWrap::SetRS((D3DRENDERSTATETYPE)(DWORD)renderState, (DWORD)value);
		}

		// <summary>
		/// Sets the gamma, brightness, and contrast of the RenderDevice
		// </summary>
		static void SetGammaLevel(bool linear, float gamma, float brightness, float contrast)
		{
			RenderDevice::Instance()->SetGammaLevel(linear,gamma,brightness,contrast);
		}

		// <summary>
		/// Whether the renderdevice has HDR enabled (note that enabling via setting requires app restart to take effect)
		// </summary>
		static property bool HDRenabled
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			bool get()
			{
				return RenderDevice::Instance()->GetHDR();
			}
			void set(bool value)
			{
				RenderDevice::Instance()->SetHDR(value);
			}
		}
		// <summary>
		/// The light exposure setting of the High Dynamic Range image processing. Higher value will yield more over-exposed, glow-heavy image
		// </summary>
		static property float HDRexposure
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get() 
			{
				return RenderDevice::Instance()->GetHDRExposure();
			}
			void set(float value) 
			{
				RenderDevice::Instance()->SetHDRExposure(value);
			}
		}

		// <summary>
		/// Whether the renderdevice has HDR Blue Shift enabled
		// </summary>
		static property bool BlueShift
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			bool get()
			{
				return RenderDevice::Instance()->GetBlueShift();
			}
			void set(bool value)
			{
				RenderDevice::Instance()->SetBlueShift(value);
			}
		}

		// <summary>
		/// The luminance bias towards Blue Shift (lower value = blue shift only in darker environments)
		// </summary>
		static property float BlueShiftLuminanceBias
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get() 
			{
				return RenderDevice::Instance()->GetBlueShiftCoefficient();
			}
			void set(float value) 
			{
				RenderDevice::Instance()->SetBlueShiftCoefficient(value);
			}
		}

		// <summary>
		/// Whether the renderdevice has HDR Tone Mappping enabled
		// </summary>
		static property bool ToneMapping
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			bool get()
			{
				return RenderDevice::Instance()->GetToneMapping();
			}
			void set(bool value)
			{
				RenderDevice::Instance()->SetToneMapping(value);
			}
		}

		// <summary>
		/// The minimum eye-adaption luminance when HDR and Tone Mapping are enabled
		// </summary>
		static property float ToneMappingMinLuminance
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get() 
			{
				return RenderDevice::Instance()->GetMinLuminance();
			}
			void set(float value) 
			{
				RenderDevice::Instance()->SetMinLuminance(value);
			}
		}

		// <summary>
		/// The maximum eye-adaption luminance when HDR and Tone Mapping are enabled
		// </summary>
		static property float ToneMappingMaxLuminance
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get() 
			{
				return RenderDevice::Instance()->GetMaxLuminance();
			}
			void set(float value) 
			{
				RenderDevice::Instance()->SetMaxLuminance(value);
			}
		}

		// <summary>
		/// Scaling % value for World textures, to conserve memory on older cards use lower %
		// </summary>
		static property float TextureSizePercent
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get() 
			{
				return RenderDevice::Instance()->GetTextureSizePercent();
			}
			void set(float value) 
			{
				RenderDevice::Instance()->SetTextureSizePercent(value);
			}
		}

		// <summary>
		/// Scaling % value for shadow maps, to conserve memory on older cards use lower %
		// </summary>
		static property float ShadowmapSizePercent
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get() 
			{
				return RenderDevice::Instance()->m_ShadowMapScale;
			}
			void set(float value) 
			{
				RenderDevice::Instance()->m_ShadowMapScale = value;
			}
		}

		// <summary>
		/// % value of Prefabs' Visible Range
		// </summary>
		static property float VisibleRange
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get() 
			{
				return LODManager::Instance()->VisibleRange;
			}
			void set(float value) 
			{
				LODManager::Instance()->VisibleRange = value;
			}
		}


		// <summary>
		/// Whether the RenderDevice compresses normal maps as it loads them, to save memory
		// </summary>
		static property bool CompressNormalMaps
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			bool get()
			{
				return RenderDevice::Instance()->GetCompressNormalMaps();
			}
			void set(bool value)
			{
				RenderDevice::Instance()->SetCompressNormalMaps(value);
			}
		}

		// <summary>
		/// Whether the RenderDevice compresses PRT maps as it loads them, to save memory
		// </summary>
		static property bool CompressPRTMaps
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			bool get()
			{
				return RenderDevice::Instance()->CompressPRTMaps();
			}
			void set(bool value)
			{
				RenderDevice::Instance()->SetCompressPRTMaps(value);
			}
		}

		// <summary>
		/// Whether Reality runs per-pixel occlusion tests on the World
		// </summary>
		static property bool OcclusionTesting
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			bool get()
			{
				return RenderDevice::Instance()->GetOcclusionTesting();
			}
			void set(bool value)
			{
				RenderDevice::Instance()->SetOcclusionTesting(value);
			}
		}

		// <summary>
		/// Whether Reality clears the canvas (with the Canvas Color) in between frames
		// </summary>
		static property bool ClearScreen
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			bool get()
			{
				return RenderDevice::Instance()->GetClearScreen();
			}
			void set(bool value)
			{
				RenderDevice::Instance()->SetClearScreen(value);
			}
		}

		// <summary>
		/// Whether Reality has SRGB color enabled
		// </summary>
		static property bool EnabledSRGB
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			bool get()
			{
				return RenderDevice::Instance()->GetSRGB();
			}
			void set(bool value)
			{
				RenderDevice::Instance()->SetSRGB(value);
			}
		}


		// <summary>
		/// Whether Reality draws projected shadow maps from shadow projectors (different from DropShadows)
		// </summary>
		static property bool RenderShadowMaps
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			bool get()
			{
				return RenderDevice::Instance()->GetShadows();
			}
			void set(bool value)
			{
				RenderDevice::Instance()->SetShadows(value);
			}
		}

		// <summary>
		/// Whether Reality renders Water effect, only can be set once a Water effect is active
		// </summary>
		static property bool RenderWater
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			bool get()
			{
				if(WaterSurface::Instance())
					return !WaterSurface::Instance()->IsHidden;
		
				return false;
			}
			void set(bool value)
			{
				if(WaterSurface::Instance())
					WaterSurface::Instance()->IsHidden = !value;
			}
		}

		// <summary>
		/// Whether the RenderDevice has the Dropshadows effect enabled
		// </summary>
		static property bool DropShadows
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			bool get()
			{
				return RenderDevice::Instance()->GetDropShadows();
			}
			void set(bool value)
			{
				RenderDevice::Instance()->SetDropShadows(value);
			}
		}

		// <summary>
		/// Anisotropic filtering amount on the current RenderDevice
		// </summary>
		static property int AnisotropyLevel
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			int get()
			{
				return RenderDevice::Instance()->GetAnisotropyLevel();
			}
			void set(int value)
			{
				RenderDevice::Instance()->SetAnisotropyLevel(value);
			}
		}

		

		// <summary>
		/// The numerical pixel shader version of the currently active Device, such as 2.0 for ps2.0
		// </summary>
		static property float PixelShaderVersion
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get()
			{
				return RenderDevice::Instance()->PixelShaderVersion;
			}
		}

		// <summary>
		/// The numerical vertex shader version of the currently active Device, such as 1.1 for vs1.1
		// </summary>
		static property float VertexShaderVersion
#ifdef DOXYGEN_IGNORE 
			; _()
#endif
		{
			float get()
			{
				return RenderDevice::Instance()->VertexShaderVersion;
			}
		}
	};

	// <summary>
	/// Wraps functionality associated with a Reality OcclusionQuery
	// </summary>
	public ref class MOcclusionQuery : IDisposable
		{
			private public:
				OcclusionQuery* m_occlusionquery;
				bool needToDelete;
				// <summary>
				/// ctor to wrap existing unmanaged occlusionquery
				// </summary>
				MOcclusionQuery(OcclusionQuery* occlusionquery)
				{
					m_occlusionquery = occlusionquery;
					needToDelete = false;
				}

		public:

			// <summary>
			/// ctor for new occlusionquery
			// </summary>
			MOcclusionQuery()
			{
				m_occlusionquery = new OcclusionQuery();
				needToDelete = true;
			}

			// <summary>
			/// dtor
			// </summary>
			virtual void Finalize()
			{
				Dispose(false);
				disposed = true;

			}		

			// <summary>
			/// Initialize this query
			// </summary>
			void Create()
			{
				m_occlusionquery->Create();
			}

			// <summary>
			/// Destroy it
			// </summary>
			void Free()
			{
				m_occlusionquery->Free();
			}
			// <summary>
			/// Begin a query
			// </summary>
			void Begin()
			{
				m_occlusionquery->Begin();
			}
			// <summary>
			/// End a query
			// </summary>				
			void End()
			{
				m_occlusionquery->End();
			}

			// <summary>
			/// Force an occlusion query to flush and return pixels visible
			// </summary>
			unsigned int GetPixels()
			{
				return m_occlusionquery->GetPixels();
			}

			/// disposal
			// </summary>
			void Dispose() 
			{
				Dispose(true);
				disposed = true;
				GC::SuppressFinalize(this);
			}

		protected:
			bool disposed;

			// <summary>
			/// unmanaged resource disposal
			// </summary>
			virtual void Dispose(bool disposing) 
			{
				if (!disposed) 
				{
					if (disposing) 
					{
						// Dispose managed resources.
					}

					// Dispose unmanaged resources
					if (needToDelete && m_occlusionquery)
						delete m_occlusionquery;
				}
			}
		};
	}




