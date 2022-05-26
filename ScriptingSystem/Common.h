//=========== (C) Copyright 2004, Artficial Studios. All rights reserved. =======
// C# Scripting System
// Author: Mostafa Mohamed
//
//
//===============================================================================
#pragma once
#include "Wrappers.h"
using namespace stdcli::language;
using namespace ScriptingSystem;

neM3 ConvertMatrixToM3(MMatrix^ value);
MMatrix^ ConvertM3ToMatrix(neM3 value);

neV3 ConvertVectorToV3(MVector^ v);
MVector^ ConvertV3ToVector(neV3 v3);

neQ ConvertVectorToQ(MQuaterion^ v);
MQuaterion^ ConvertQToVector(neQ q);


void _assert(char const * ,char const *,unsigned int);

namespace ScriptingSystem
{
/*
	public value struct T3
	{
	public:
		neT3 m_T3;

		//Temp Variables
		MVector^ v;

		T3()
		{
		}

		property MVector^ get
		{
			neT3 gc & ref = static_cast<neT3 gc &>(m_T3);
			v->x = ref.pos.v[0];
			v->y = ref.pos.v[1];
			v->z = ref.pos.v[2];
			return v;
		}

		property void set(MVector^ value) 
		{
			neT3 gc & ref = static_cast<neT3 gc &>(m_T3);
			ref.pos.v[0]=value->x;
			ref.pos.v[1]=value->y;
			ref.pos.v[2]=value->z;
		}

		property Matrix get
		{
			neT3 gc & ref = static_cast<neT3 gc &>(m_T3);
			Matrix m;

			m.M11 = ref.rot.M[0].v[0];
			m.M12 = ref.rot.M[0].v[1];
			m.M13 = ref.rot.M[0].v[2];

			m.M21 = ref.rot.M[1].v[0];
			m.M22 = ref.rot.M[1].v[1];
			m.M23 = ref.rot.M[1].v[2];

			m.M31 = ref.rot.M[2].v[0];
			m.M32 = ref.rot.M[2].v[1];
			m.M33 = ref.rot.M[2].v[2];

			return m;
		}

		property void set(Matrix value) 
		{
			m_T3.rot.M[0].v[0] = value.M11;
			m_T3.rot.M[0].v[1] = value.M12;
			m_T3.rot.M[0].v[2] = value.M13;

			m_T3.rot.M[1].v[0] = value.M21;
			m_T3.rot.M[1].v[1] = value.M22;
			m_T3.rot.M[1].v[2] = value.M23;

			m_T3.rot.M[2].v[0] = value.M31;
			m_T3.rot.M[2].v[1] = value.M32;
			m_T3.rot.M[2].v[2] = value.M33;
		}

		void SetRot(Matrix value)
		{
			m_T3.rot.M[0].v[0] = value.M11;
			m_T3.rot.M[0].v[1] = value.M12;
			m_T3.rot.M[0].v[2] = value.M13;

			m_T3.rot.M[1].v[0] = value.M21;
			m_T3.rot.M[1].v[1] = value.M22;
			m_T3.rot.M[1].v[2] = value.M23;

			m_T3.rot.M[2].v[0] = value.M31;
			m_T3.rot.M[2].v[1] = value.M32;
			m_T3.rot.M[2].v[2] = value.M33;
		}

		T3 FastInverse()
		{
			T3 t;
			T3 gc & ref = static_cast<T3 gc &>(t);
			neT3 gc & ref2 = static_cast<neT3 gc &>(m_T3);
//			ref.m_T3
			neT3 t3 = ref2.FastInverse();
//			ref.m_T3.pos.n = t3.pos.n;
			ref.m_T3.pos.v[0] = t3.pos.v[0];
			ref.m_T3.pos.v[1] = t3.pos.v[1];
			ref.m_T3.pos.v[2] = t3.pos.v[2];
			ref.m_T3.pos.v[3] = t3.pos.v[3];

			ref.m_T3.rot.M[0].v[0] = t3.rot.M[0].v[0];
			ref.m_T3.rot.M[0].v[1] = t3.rot.M[0].v[1];
			ref.m_T3.rot.M[0].v[2] = t3.rot.M[0].v[2];
			ref.m_T3.rot.M[0].v[3] = t3.rot.M[0].v[3];
//			ref.m_T3.rot.M[0].n = t3.rot.M[0].n;

			ref.m_T3.rot.M[1].v[0] = t3.rot.M[1].v[0];
			ref.m_T3.rot.M[1].v[1] = t3.rot.M[1].v[1];
			ref.m_T3.rot.M[1].v[2] = t3.rot.M[1].v[2];
			ref.m_T3.rot.M[1].v[3] = t3.rot.M[1].v[3];
//			ref.m_T3.rot.M[1].n = t3.rot.M[1].n;

			ref.m_T3.rot.M[2].v[0] = t3.rot.M[2].v[0];
			ref.m_T3.rot.M[2].v[1] = t3.rot.M[2].v[1];
			ref.m_T3.rot.M[2].v[2] = t3.rot.M[2].v[2];
			ref.m_T3.rot.M[2].v[3] = t3.rot.M[2].v[3];
//			ref.m_T3.rot.M[2].n = t3.rot.M[2].n;
			return t;
		}

		/*bool IsFinite()
		{
			neT3 gc & ref2 = static_cast<neT3 gc &>(m_T3);
			return ref2.IsFinite() == 1;
		};*/

/*
		void MakeD3DCompatibleMatrix()
		{
			neT3 gc & ref = static_cast<neT3 gc &>(m_T3);
			ref.rot.M[0].v[3] = 0.0f;
			ref.rot.M[1].v[3] = 0.0f;
			ref.rot.M[2].v[3] = 0.0f;
			ref.pos.v[3] = 1.0f;
		}

		Matrix GetD3DCompatibleMatrix()
		{
			neT3 gc & ref = static_cast<neT3 gc &>(m_T3);
			Matrix m;

			m.M11 = ref.rot.M[0].v[0];
			m.M12 = ref.rot.M[0].v[1];
			m.M13 = ref.rot.M[0].v[2];
			m.M14 = 0.0f;

			m.M21 = ref.rot.M[1].v[0];
			m.M22 = ref.rot.M[1].v[1];
			m.M23 = ref.rot.M[1].v[2];
			m.M24 = 0.0f;

			m.M31 = ref.rot.M[2].v[0];
			m.M32 = ref.rot.M[2].v[1];
			m.M33 = ref.rot.M[2].v[2];
			m.M34 = 0.0f;

			m.M41 = ref.pos.v[0];
			m.M42 = ref.pos.v[1];
			m.M43 = ref.pos.v[2];
			m.M44 = 1.0f;

			return m;
		}

		void SetIdentity()
		{
			neT3 gc & ref = static_cast<neT3 gc &>(m_T3);
			neM3 gc & rot = static_cast<neM3 gc &>(ref.rot);
			neV3 gc & pos = static_cast<neV3 gc &>(ref.pos);
			rot.SetIdentity();
			pos.SetZero();
			MakeD3DCompatibleMatrix();
		}

		T3 Multiply(T3& t1,T3& t2)
		{
			T3 t;
			Matrix Transform1,Transform2,Final;
			
			Transform1 = t1.Rot;
			Transform2 = t2.Rot;

			Transform1.M41 = t1.m_T3.pos.v[0];
			Transform1.M42 = t1.m_T3.pos.v[1];
			Transform1.M43 = t1.m_T3.pos.v[2];

			Transform2.M41 = t2.m_T3.pos.v[0];
			Transform2.M42 = t2.m_T3.pos.v[1];
			Transform2.M43 = t2.m_T3.pos.v[2];

			Final = Transform1 * Transform2;

			t.Rot = Final;
			t.Pos->x = Final.M41;
			t.Pos->y = Final.M42;
			t.Pos->x = Final.M43;

			return t;
		}
	};*/


	public ref struct T3
	{
	public:
		MVector^ Pos;
		MMatrix^ Rot;

		T3()
		{
            Pos = gcnew MVector();
            Rot = gcnew MMatrix();
		}

		MMatrix^ GetD3DCompatibleMatrix();

		void SetIdentity();

		void TrimRotation();

		static T3^ Multiply(T3^ t1,T3^ t2);
	};
}


T3^ ConvertToT3(neT3 t3);
neT3 ConvertToneT3(T3^ t);


namespace ScriptingSystem
{

	public value struct SimulatorSizeInfo
	{
		int animatedBodiesCount;
		int constraintBufferSize;
		int constraintsCount;
		int constraintSetsCount;
		int controllersCount;
		int geometriesCount;
		int overlappedPairsCount;
		int rigidBodiesCount;
		int rigidParticleCount;
		int sensorsCount;
		int terrainNodesStartCount;
		int terrainNodesGrowByCount;
	};

	public enum class ReponseBitFlag
	{
		RESPONSE_IGNORE = 0,
		RESPONSE_IMPULSE = 1,
		RESPONSE_CALLBACK = 2,
		RESPONSE_IMPULSE_CALLBACK = 3,
	};

	public enum class Max_Collision
	{
		NE_COLLISION_TABLE_MAX = 64
	};

	public ref class CollisionTable
	{
	public:

		CollisionTable()
		{}

		neCollisionTable * m_CollisionTable;

		void Set(int collisionID1, int collisionID2);
		void Set(int collisionID1, int collisionID2, ReponseBitFlag response);
		ReponseBitFlag Get(int collisionID1, int collisionID2);
		int GetMaxCollisionID();

	};

	public ref struct Triangle
	{
	public:
		Triangle()
		{
            flag = TriangleType::NE_TRI_TRIANGLE;
			materialID = 0;
			userData = 0;
		}

		enum class TriangleType
		{
			NE_TRI_TRIANGLE = 0,
			NE_TRI_HEIGHT_MAP,
		};

		MVector^ indices;
		int materialID;
		TriangleType flag;
		unsigned int userData;
	};
}

neTriangle ConvertToneTriangle(Triangle^ t);

namespace ScriptingSystem
{
	public enum class BodyType
	{
		NE_TERRAIN = 0, 
		NE_RIGID_BODY,
		NE_ANIMATED_BODY,
	};


	public ref class TriangleMesh
	{
	public:
		array<MVector^>^ vertices;
		int vertexCount;
		array<Triangle^>^ triangles;
		int triangleCount;
	};

	ref class TokamakGeometry;
    ref class RigidBody;
	public ref struct TCollisionInfo
	{
		unsigned char * bodyA;
		unsigned char * bodyB;

		BodyType typeA;
		BodyType typeB;

		TokamakGeometry ^ geometryA;
		TokamakGeometry ^ geometryB;
		int materialIdA;
		int materialIdB;
		MVector^ bodyContactPointA;		// contact point A in body space of A
		MVector^ bodyContactPointB;		// contact point B in body space of B
		MVector^ worldContactPointA;	// contact point A in world space
		MVector^ worldContactPointB;	// contact point B in world space
		MVector^ relativeVelocity;
		MVector^ collisionNormal;

        RigidBody^ GetBodyB();
        RigidBody^ GetBodyA();

		MActor^ CollidedOnto;
	};
}

neTriangleMesh ConvertToneTriangleMesh(TriangleMesh ^ mesh);
ScriptingSystem::TCollisionInfo^ ConvertToCollisionInfo(neCollisionInfo *collision);
