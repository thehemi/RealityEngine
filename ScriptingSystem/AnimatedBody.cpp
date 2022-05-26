//=========== (C) Copyright 2004, Artficial Studios. All rights reserved. =======
// C# Scripting System
// Author: Mostafa Mohamed
//
//
//===============================================================================
#include "stdafx.h"
#include "Common.h"
#include "AnimatedBody.h"
#include "rigidbody.h"
#include "Simulator.h"
using namespace ScriptingSystem;


AnimatedBody::AnimatedBody(Simulator ^simulator)
{
	m_Simulator=simulator->m_Simulator;
	m_AnimatedBody=simulator->m_Simulator->CreateAnimatedBody();
}

void AnimatedBody::Finalize()
{
	m_Simulator->FreeAnimatedBody(this->m_AnimatedBody);
	m_Simulator=0;
}

MVector^ AnimatedBody::GetPos()
{
	return ConvertV3ToVector(m_AnimatedBody->GetPos());
}

void AnimatedBody::SetPos(MVector^ p)
{
	m_AnimatedBody->SetPos(ConvertVectorToV3(p));
}

MMatrix^ AnimatedBody::GetRotationM3()
{
	return ConvertM3ToMatrix(m_AnimatedBody->GetRotationM3());
}

MQuaterion^ AnimatedBody::GetRotationQ()
{
	return ConvertQToVector(m_AnimatedBody->GetRotationQ());
}

void AnimatedBody::SetRotation(MMatrix^ m)
{
	m_AnimatedBody->SetRotation(ConvertMatrixToM3(m));
}

void AnimatedBody::SetRotation(MQuaterion^ q)
{
	m_AnimatedBody->SetRotation(ConvertVectorToQ(q));
}

T3^ AnimatedBody::GetTransform()
{
	return ConvertToT3(m_AnimatedBody->GetTransform());	
}

int AnimatedBody::GetGeometryCount()
{
	return m_AnimatedBody->GetGeometryCount();
}


TokamakGeometry ^ AnimatedBody::AddGeometry()
{
	TokamakGeometry ^g=gcnew TokamakGeometry(this);
	return g;
}

bool AnimatedBody::RemoveGeometry(TokamakGeometry ^ g)
{
	return (m_AnimatedBody->RemoveGeometry(g->geometry)==1);
}

void AnimatedBody::BeginIterateGeometry()
{
	m_AnimatedBody->BeginIterateGeometry();
}

//try to change this way of return by add a void* in the neGeometry class to hold this pointer
TokamakGeometry ^ AnimatedBody::GetNextGeometry()
{
	neGeometry * gem = this->m_AnimatedBody->GetNextGeometry();

	if(gem == 0)
		return nullptr;

	TokamakGeometry ^ gem2 = gcnew TokamakGeometry();
	gem2->geometry = gem;
	return gem2;
}

//check if we must delete the old rigid body
//if a crash happened then return the old pointer only
RigidBody ^ AnimatedBody::BreakGeometry(TokamakGeometry ^ g)
{
	RigidBody ^new_r = gcnew RigidBody();
	new_r->m_RigidBody = m_AnimatedBody->BreakGeometry(g->geometry);
	return new_r;
}

Sensor ^ AnimatedBody::AddSensor()
{
	Sensor ^s=gcnew Sensor(this);

	if(s->sensor == 0)
		return nullptr;

	return s;
}

bool AnimatedBody::RemoveSensor(Sensor ^ s)
{
	return (m_AnimatedBody->RemoveSensor(s->sensor)==1);
}

void AnimatedBody::BeginIterateSensor()
{
	m_AnimatedBody->BeginIterateSensor();
}

Sensor ^ AnimatedBody::GetNextSensor()
{
	Sensor ^s=gcnew Sensor();
	s->sensor = m_AnimatedBody->GetNextSensor();

	if(s->sensor == 0)
		return nullptr;

	return s;
}

void AnimatedBody::UpdateBoundingInfo()
{
	m_AnimatedBody->UpdateBoundingInfo();
}

void AnimatedBody::CollideConnected(bool yes)
{
	m_AnimatedBody->CollideConnected((neBool) yes);
}

bool AnimatedBody::CollideConnected()
{
	return (m_AnimatedBody->CollideConnected() == 1);
}


bool AnimatedBody::Activate()
{
	return (m_AnimatedBody->Active() == 1);
}

void AnimatedBody::Activate(bool yes,RigidBody ^hint)
{
	return m_AnimatedBody->Active((neBool) yes,hint->m_RigidBody);
}

void AnimatedBody::Activate(bool yes,AnimatedBody ^hint)
{
	return m_AnimatedBody->Active((neBool) yes,hint->m_AnimatedBody);
}

void AnimatedBody::Activate(bool yes)
{
	return m_AnimatedBody->Active((neBool)yes,(neRigidBody*) 0);
}
