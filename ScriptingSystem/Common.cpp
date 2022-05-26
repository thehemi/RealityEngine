//=========== (C) Copyright 2004, Artficial Studios. All rights reserved. =======
// C# Scripting System
// Author: Mostafa Mohamed
//
//
//===============================================================================

#include "stdafx.h"
#include "Common.h"
#include "geometry.h"
#include "rigidBody.h"
using namespace ScriptingSystem;




neM3 ConvertMatrixToM3(MMatrix^ value)
{
    neM3 m3;

    m3.M[0].v[0] = value->m0->x;
    m3.M[0].v[1] = value->m0->y;
    m3.M[0].v[2] = value->m0->z;

    m3.M[1].v[0] = value->m1->x;
    m3.M[1].v[1] = value->m1->y;
    m3.M[1].v[2] = value->m1->z;

    m3.M[2].v[0] = value->m2->x;
    m3.M[2].v[1] = value->m2->y;
    m3.M[2].v[2] = value->m2->z;

    return m3;
}

void _assert(char const * ,char const *,unsigned int)
{
}


MMatrix^ ConvertM3ToMatrix(neM3 value)
{
    MMatrix^ m=gcnew MMatrix();

    m->m0->x = value.M[0].v[0];
    m->m0->y = value.M[0].v[1];
    m->m0->z = value.M[0].v[2];

    m->m1->x  = value.M[1].v[0];
    m->m1->y  = value.M[1].v[1];
    m->m1->z  = value.M[1].v[2];

    m->m2->x  = value.M[2].v[0];
    m->m2->y  = value.M[2].v[1];
    m->m2->z  = value.M[2].v[2];

    return m;
}

neV3 ConvertVectorToV3(MVector^ v)
{
    neV3 v3;
    v3.v[0] = v->x;
    v3.v[1] = v->y;
    v3.v[2] = v->z;

    return v3;
}


MVector^ ConvertV3ToVector(neV3 v3)
{
    MVector^ v = gcnew MVector(v3.v[0],v3.v[1],v3.v[2]);
    return v;
}

neQ ConvertVectorToQ(MQuaterion^ v)
{
    neQ q;
    q.X = v->x;
    q.Y = v->y;
    q.Z = v->z;
    q.W = v->w;

    return q;
}


MQuaterion^ ConvertQToVector(neQ q)
{
    MQuaterion^ v = gcnew MQuaterion();
    v->x = q.X;
    v->y = q.Y;
    v->z = q.Z;
    v->w = q.W;

    return v;
}

T3^ ConvertToT3(neT3 t3)
{
    T3^ t = gcnew T3();
    t->Rot = ConvertM3ToMatrix(t3.rot);
    t->Pos = ConvertV3ToVector(t3.pos);
    return t;
}

neT3 ConvertToneT3(T3^ t)
{
    neT3 t3;
    t3.rot = ConvertMatrixToM3(t->Rot);
    t3.pos = ConvertVectorToV3(t->Pos);

    return t3;
}


void CollisionTable::Set(int collisionID1, int collisionID2)
{
    m_CollisionTable->Set(collisionID1,collisionID2);
}

void CollisionTable::Set(int collisionID1, int collisionID2, ReponseBitFlag response)
{
    m_CollisionTable->Set(collisionID1,collisionID2,(neCollisionTable::neReponseBitFlag)response);
}

ReponseBitFlag CollisionTable::Get(int collisionID1, int collisionID2)
{
    return (ReponseBitFlag) m_CollisionTable->Get(collisionID1,collisionID2);
}

int CollisionTable::GetMaxCollisionID()
{
    return m_CollisionTable->GetMaxCollisionID();
}

neTriangle ConvertToneTriangle(Triangle^ t)
{
    neTriangle triangle;
    triangle.flag =(unsigned int) t->flag;
    triangle.materialID = t->materialID;
    triangle.userData = t->userData;
    triangle.indices[0] = (int) t->indices->x;
    triangle.indices[1] = (int) t->indices->y;
    triangle.indices[2] = (int) t->indices->z;
    return triangle;
}

neTriangleMesh ConvertToneTriangleMesh(TriangleMesh ^ mesh)
{
    neTriangleMesh tm;
    tm.triangleCount = mesh->triangleCount;
    tm.vertexCount = mesh->vertexCount;
    tm.vertices=new neV3[mesh->vertexCount];
    for(int i=0;i<mesh->vertexCount;i++)
    {
        tm.vertices[i].v[0] = mesh->vertices[i]->x;
        tm.vertices[i].v[1] = mesh->vertices[i]->y;
        tm.vertices[i].v[2] = mesh->vertices[i]->z;
    }

    tm.triangles=new neTriangle[mesh->triangleCount];
    for(i=0;i<mesh->triangleCount;i++)
    {
        tm.triangles[i] = ConvertToneTriangle(mesh->triangles[i]);
    }

    return tm;
}

ScriptingSystem::TCollisionInfo^ ConvertToCollisionInfo(neCollisionInfo *collision)
{
    ScriptingSystem::TCollisionInfo^ info= gcnew ScriptingSystem::TCollisionInfo();
    info->bodyA = collision->bodyA;
    info->bodyB = collision->bodyB;

	info->typeB = (ScriptingSystem::BodyType)(int)collision->typeB;
	info->typeA = (ScriptingSystem::BodyType)(int)collision->typeA;

    info->geometryA = gcnew TokamakGeometry();
    info->geometryB = gcnew TokamakGeometry();
    info->geometryA->geometry = collision->geometryA;
    info->geometryB->geometry = collision->geometryB;

    info->materialIdA = collision->materialIdA;
    info->materialIdB = collision->materialIdB;

    info->bodyContactPointA = gcnew MVector(collision->bodyContactPointA.v[0],
        collision->bodyContactPointA.v[1],
        collision->bodyContactPointA.v[2]);

    info->bodyContactPointB = gcnew MVector(collision->bodyContactPointB.v[0],
        collision->bodyContactPointB.v[1],
        collision->bodyContactPointB.v[2]);

    info->worldContactPointA = gcnew MVector(collision->worldContactPointA.v[0],
        collision->worldContactPointA.v[1],
        collision->worldContactPointA.v[2]);

    info->worldContactPointB = gcnew MVector(collision->worldContactPointB.v[0],
        collision->worldContactPointB.v[1],
        collision->worldContactPointB.v[2]);

    info->relativeVelocity = gcnew MVector(collision->relativeVelocity.v[0],
        collision->relativeVelocity.v[1],
        collision->relativeVelocity.v[2]);

    info->collisionNormal = gcnew MVector(collision->collisionNormal.v[0],
        collision->collisionNormal.v[1],
        collision->collisionNormal.v[2]);

	if(collision->typeB == NE_TERRAIN)
    {
        if(collision->materialIdB && collision->materialIdB != MATERIALFLOOR)
        {
            //we've collided with a non-Tok Actor
			info->CollidedOnto = MActor::GetFromActor((Actor*)collision->materialIdB);
		}
	}
	/*else
	{
	RigidBody^ bodyB = info->GetBodyA();
	if(bodyB != nullptr)
		info->CollidedOnto = bodyB->Parent;
	}*/

    return info;
}



MMatrix^ T3::GetD3DCompatibleMatrix()
{
    MMatrix^ m=gcnew MMatrix();

    Rot->m0->Copy(m->m0);
    Rot->m1->Copy(m->m1);
    Rot->m2->Copy(m->m2);

    m->p0  = 0;
    m->p1 = 0;
    m->p2= 0;
    m->p3=1;

    m->m3->x = Pos->x;
    m->m3->y  = Pos->y;
    m->m3->z  = Pos->z;

    return m;
}

void T3::SetIdentity()
{
    Rot->m_matrix->Identity();
    Pos->x =0;
    Pos->y = 0;
    Pos->z = 0;
}

void T3::TrimRotation()
{
    Rot->p0 = 0;
    Rot->p1 = 0;
    Rot->p2 = 0;
    Rot->p3 = 0;
    Rot->m3->x = 0;
    Rot->m3->y = 0;
    Rot->m3->z = 0;
}

T3^ T3::Multiply(T3^ t1,T3^ t2)
{
    neT3 net1,net2,net3;

    net1 = ConvertToneT3(t1);
    net2 = ConvertToneT3(t2);

    net3 = net1 * net2;

    return ConvertToT3(net3);
}

RigidBody^ TCollisionInfo::GetBodyB()
{
    if(typeB != BodyType::NE_RIGID_BODY)return nullptr;
    u32 userData=((neRigidBody *)bodyB)->GetUserData();
    if(userData)
    {
        RigidBodyIdentifier * id =(RigidBodyIdentifier*)userData;
        if (id->managedIndex < 0)
            return (RigidBody^)RigidBody::s_rigidBodies[id->managedIndex];
    }
    return nullptr;
}

RigidBody^ TCollisionInfo::GetBodyA()
{
    if(typeA != BodyType::NE_RIGID_BODY)return nullptr;
    u32 userData=((neRigidBody *)bodyA)->GetUserData();
    if(userData)
    {
        RigidBodyIdentifier * id =(RigidBodyIdentifier*)userData;
        if (id->managedIndex < 0)
            return (RigidBody^)RigidBody::s_rigidBodies[id->managedIndex];
    }
    return nullptr;
}