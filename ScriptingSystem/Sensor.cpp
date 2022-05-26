//=========== (C) Copyright 2004, Artficial Studios. All rights reserved. =======
// C# Scripting System
// Author: Mostafa Mohamed
//
//
//===============================================================================
#include "StdAfx.h"
#include ".\sensor.h"

#include "rigidbody.h"
#include "animatedbody.h"
using namespace ScriptingSystem;


Sensor::Sensor()
{

}

Sensor::Sensor(RigidBody ^ body)
{
	this->sensor = body->m_RigidBody->AddSensor();
}

Sensor::Sensor(AnimatedBody ^ body)
{
	this->sensor = body->m_AnimatedBody->AddSensor();
}



void Sensor::SetLineSensor(MVector^  pos,MVector^ lineVector)
{
	this->sensor->SetLineSensor(ConvertVectorToV3(pos),ConvertVectorToV3(lineVector));
}

void Sensor::UserData::set (unsigned int UserData)
{
	this->sensor->SetUserData(UserData);
}

MVector^ Sensor::LineVector::get ()
{
	return ConvertV3ToVector( this->sensor->GetLinePos());
}

MVector^ Sensor::LineUnitVector::get ()
{
	return ConvertV3ToVector(this->sensor->GetLineUnitVector());
}

MVector^ Sensor::LinePos::get ()
{
	return ConvertV3ToVector(this->sensor->GetLinePos());
}

float Sensor::DetectDepth::get ()
{
	return this->sensor->GetDetectDepth();
}

MVector^ Sensor::DetectNormal::get ()
{
	return ConvertV3ToVector(this->sensor->GetDetectNormal());
}

MVector^ Sensor::DetectContactPoint::get ()
{
	return ConvertV3ToVector(this->sensor->GetDetectContactPoint());
}

int Sensor::DetectMaterial::get ()
{
	return this->sensor->GetDetectMaterial();
}

AnimatedBody ^ Sensor::DetectAnimatedBody::get ()
{
	AnimatedBody ^ body = gcnew AnimatedBody();
	body->m_AnimatedBody = this->sensor->GetDetectAnimatedBody();
	return body;
}

RigidBody ^ Sensor::DetectRigidBody::get ()
{
	RigidBody ^ body = gcnew RigidBody();
	body->m_RigidBody = this->sensor->GetDetectRigidBody();
	return body;
}

unsigned int Sensor::UserData::get ()
{
	return this->sensor->GetUserData();
}