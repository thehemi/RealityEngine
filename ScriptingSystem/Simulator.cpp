//=========== (C) Copyright 2004, Artficial Studios. All rights reserved. =======
// C# Scripting System
// Author: Mostafa Mohamed
//
//
//===============================================================================
#include "stdafx.h"
#include "Simulator.h"
#include "RigidBody.h"
#include "AnimatedBody.h"

using namespace ScriptingSystem;

gcroot<Simulator^> GlobalCollisionCallback::simulator = 0;
gcroot<Simulator^> GlobalBreakageCallback::simulator = 0;

void GlobalCollisionCallback::SetSimulator(Simulator ^s)
{
	simulator = s;
}

void GlobalCollisionCallback::Collision(neCollisionInfo & collisionInfo)
{
	simulator->m_CollisionCallback->Collision(ConvertToCollisionInfo(&collisionInfo));
}

void GlobalBreakageCallback::SetSimulator(Simulator ^s)
{
	simulator = s;
}

void GlobalBreakageCallback::Breakage(neByte * originalBody, neBodyType bodyType, neGeometry * brokenGeometry, neRigidBody * newBody)
{
	RigidBody ^r=gcnew RigidBody();
	r->m_Simulator = GlobalBreakageCallback::simulator->m_Simulator;
	r->m_RigidBody = newBody;
	TokamakGeometry ^g=gcnew TokamakGeometry();
	g->geometry = brokenGeometry;
	simulator->m_BreakageCallback->Breakage(originalBody,(BodyType) bodyType,g,r);
}


RigidBody ^ Simulator::CreateRigidBody()
{
	RigidBody ^rigid=gcnew RigidBody(this);
	return rigid;
}

RigidBody ^ Simulator::CreateRigidParticle()
{
	neRigidBody *body = m_Simulator->CreateRigidParticle();
	RigidBody ^rigid=gcnew RigidBody();
	rigid->m_RigidBody = body;
	rigid->m_Simulator = m_Simulator;
	return rigid;
}

AnimatedBody ^ Simulator::CreateAnimatedBody()
{
	AnimatedBody ^ani=gcnew AnimatedBody(this);
	return ani;
}

void Simulator::FreeRigidBody(RigidBody ^ body)
{
	m_Simulator->FreeRigidBody(body->m_RigidBody);
}

void Simulator::FreeAnimatedBody(AnimatedBody ^ body)
{
	m_Simulator->FreeAnimatedBody(body->m_AnimatedBody);
}


CollisionTable ^ Simulator::GetCollisionTable()
{
	CollisionTable ^ table=gcnew CollisionTable();
	table->m_CollisionTable = m_Simulator->GetCollisionTable();
	return table;
}

bool Simulator::SetMaterial(int index, float friction, float restitution)
{
	return m_Simulator->SetMaterial(index,friction,restitution);
}

bool Simulator::GetMaterial(int index, float & friction, float & restitution)
{
	return m_Simulator->GetMaterial(index,friction,restitution);
}

void Simulator::Advance(float sec)
{
	m_Simulator->Advance(sec);
}

void Simulator::Advance(float sec, int nSteps)
{
	m_Simulator->Advance(sec,nSteps);
}
Simulator::Simulator(neSimulator * simulator)
{
    m_Simulator = simulator;
    needToDelete = false;
}

Simulator::Simulator(SimulatorSizeInfo sizeInfo,MVector^ gravity)
{
	neV3 neGravity;
	neGravity.Set(gravity->x,gravity->y,gravity->z);
	neSimulatorSizeInfo si;
	si.animatedBodiesCount =sizeInfo.animatedBodiesCount;
	si.constraintBufferSize =sizeInfo. constraintBufferSize;
	si.constraintsCount =sizeInfo. constraintsCount;
	si.constraintSetsCount =sizeInfo. constraintSetsCount;
	si.controllersCount =sizeInfo. controllersCount;
	si.geometriesCount =sizeInfo. geometriesCount;
	si.overlappedPairsCount =sizeInfo. overlappedPairsCount;
	si.rigidBodiesCount =sizeInfo. rigidBodiesCount;
	si.rigidParticleCount =sizeInfo. rigidParticleCount;
	si.sensorsCount =sizeInfo. sensorsCount;
	si.terrainNodesStartCount =sizeInfo. terrainNodesStartCount;
	si.terrainNodesGrowByCount =sizeInfo. terrainNodesGrowByCount;
	m_Simulator=neSimulator::CreateSimulator(si,0,&neGravity);

	m_CollisionCallback = nullptr;
	m_BreakageCallback = nullptr;
    needToDelete = true;
}

neTriangle ConvertToneTriangle1(Triangle^ t)
{
	neTriangle triangle;
	triangle.flag = (unsigned int)t->flag;
	triangle.materialID = t->materialID;
	triangle.userData = t->userData;
	triangle.indices[0] = (int) t->indices->x;
	triangle.indices[1] = (int) t->indices->y;
	triangle.indices[2] = (int) t->indices->z;
	return triangle;
}

neTriangleMesh tm;
void Simulator::SetTerrainMesh(TriangleMesh^ tris)
{
	
	tm.triangleCount = tris->triangleCount;
	tm.vertexCount = tris->vertexCount;
	tm.vertices=new neV3[tris->vertexCount];
	for(int i=0;i<tris->vertexCount;i++)
	{
		tm.vertices[i].v[0] = tris->vertices[i]->x;
		tm.vertices[i].v[1] = tris->vertices[i]->y;
		tm.vertices[i].v[2] = tris->vertices[i]->z;
	}

	tm.triangles=new neTriangle[tris->triangleCount];
	for(i=0;i<tris->triangleCount;i++)
	{
		tm.triangles[i] = ConvertToneTriangle1(tris->triangles[i]);
	}

	m_Simulator->SetTerrainMesh(&tm);
}

void Simulator::SetCollisionCallback(CollisionCallback ^ cb)
{
	m_CollisionCallback = cb;
	GlobalCollisionCallback::SetSimulator(this);

	m_Simulator->SetCollisionCallback((neCollisionCallback*) GlobalCollisionCallback::Collision);
}

CollisionCallback ^ Simulator::GetCollisionCallback()
{
	return m_CollisionCallback;
}

void Simulator::SetBreakageCallback(BreakageCallback ^ cb)
{
	m_BreakageCallback = cb;
	GlobalBreakageCallback::SetSimulator(this);

	m_Simulator->SetBreakageCallback((neBreakageCallback*) GlobalBreakageCallback::Breakage);
}

BreakageCallback ^ Simulator::GetBreakageCallback()
{
	return m_BreakageCallback;
}
