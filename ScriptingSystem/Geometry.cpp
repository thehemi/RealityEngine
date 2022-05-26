//=========== (C) Copyright 2004, Artficial Studios. All rights reserved. =======
// C# Scripting System
// Author: Mostafa Mohamed
//
//
//===============================================================================
#include "StdAfx.h"
#include ".\geometry.h"

using namespace ScriptingSystem;
#include "rigidbody.h"
#include "animatedbody.h"



TokamakGeometry::TokamakGeometry(RigidBody ^ body)
{
	this->geometry = body->m_RigidBody->AddGeometry();
}

TokamakGeometry::TokamakGeometry(AnimatedBody ^ body)
{
	this->geometry = body->m_AnimatedBody->AddGeometry();
}



void TokamakGeometry::Transform::set(T3^ t)
{
	this->geometry->SetTransform(ConvertToneT3(t));
}

T3^ TokamakGeometry::Transform::get()
{
	return ConvertToT3(this->geometry->GetTransform());
}

void TokamakGeometry::MaterialIndex::set(int index)
{
	this->geometry->SetMaterialIndex(index);
}

int TokamakGeometry::MaterialIndex::get()
{
	return this->geometry->GetMaterialIndex();
}

void TokamakGeometry::UserData::set(unsigned int UserData)
{
	this->geometry->SetUserData(UserData);
}

unsigned int TokamakGeometry::UserData::get()
{
	return this->geometry->GetUserData();
}

void TokamakGeometry::SetSphereDiameter(float diameter)
{
	this->geometry->SetSphereDiameter(diameter);
}

bool TokamakGeometry::GetSphereDiameter(float & diameter)
{
	return (this->geometry->GetSphereDiameter(diameter) == 1);
}


void TokamakGeometry::SetCylinder(float diameter,float height)
{
	this->geometry->SetCylinder(diameter,height);
}

bool TokamakGeometry::GetCylinder(float & diameter, float & height)
{
	return (this->geometry->GetCylinder(diameter,height)==1);
}

void TokamakGeometry::BreakageFlag::set(BreakFlag flag)
{
	this->geometry->SetBreakageFlag((neGeometry::neBreakFlag)flag);
}

BreakFlag TokamakGeometry::BreakageFlag::get()
{
	return (BreakFlag)this->geometry->GetBreakageFlag();
}

void TokamakGeometry::BreakageMass::set(float mass)
{
	this->geometry->SetBreakageMass(mass);
}

float TokamakGeometry::BreakageMass::get()
{
	return this->geometry->GetBreakageMass();
}


void TokamakGeometry::BreakageInertiaTensor::set(MVector^ tensor)
{
	this->geometry->SetBreakageInertiaTensor(ConvertVectorToV3(tensor));
}

MVector^ TokamakGeometry::BreakageInertiaTensor::get()
{
	return ConvertV3ToVector(this->geometry->GetBreakageInertiaTensor());
}

void TokamakGeometry::BreakageMagnitude::set(float mag)
{
	this->geometry->SetBreakageMagnitude(mag);
}

float TokamakGeometry::BreakageMagnitude::get()
{
	return this->geometry->GetBreakageMagnitude();
}

void TokamakGeometry::BreakageAbsorption::set(float absorb)
{
	this->geometry->SetBreakageAbsorption(absorb);
}

float TokamakGeometry::BreakageAbsorption::get()
{
	return this->geometry->GetBreakageAbsorption();
}

void TokamakGeometry::BreakageNeighbourRadius::set(float radius)
{
	this->geometry->SetBreakageNeighbourRadius(radius);
}

float TokamakGeometry::BreakageNeighbourRadius::get()
{
	return this->geometry->GetBreakageNeighbourRadius();
}

void TokamakGeometry::BreakagePlane::set(MVector^ planeNormal)
{
	this->geometry->SetBreakagePlane(ConvertVectorToV3(planeNormal));
}

MVector^ TokamakGeometry::BreakagePlane::get()
{
	return ConvertV3ToVector(this->geometry->GetBreakagePlane());
}

void TokamakGeometry::SetBoxSize(float width,float height,float depth)
{
	this->geometry->SetBoxSize(width,height,depth);
}

void TokamakGeometry::SetBoxSize(MVector^  BoxSize)
{
	this->geometry->SetBoxSize(ConvertVectorToV3(BoxSize));
}

MVector^ TokamakGeometry::GetBoxSize()
{
    neV3 vec;
	if (this->geometry->GetBoxSize(vec))
        return ConvertV3ToVector(vec);
    return nullptr;
}