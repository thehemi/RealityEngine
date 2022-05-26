//=========== (C) Copyright 2004, Artficial Studios. All rights reserved. =======
// C# Scripting System
// Author: Mostafa Mohamed
//
//
//===============================================================================
#include "StdAfx.h"
#include ".\rigidbodycontroller.h"

using namespace ScriptingSystem;



RigidBodyController::RigidBodyController(RigidBody ^ body,int period)
{
    OnControllerCallback = nullptr;
	this->callback = new RigidBodyControllerInterface(this);
	this->body = body;
	this->Controller = this->body->m_RigidBody->AddController(callback,period);
}

void RigidBodyController::RemoveController()
{
	OnControllerCallback = nullptr;
	//body->m_RigidBody->RemoveController(Controller);
}

void RigidBodyController::SetControllerForce (MVector^  force)
{
	this->Controller->SetControllerForce(ConvertVectorToV3(force));
}

void RigidBodyController::SetControllerForceWithTorque(MVector^  force,
													    MVector^  pos)
{
	this->Controller->SetControllerForceWithTorque(ConvertVectorToV3(force),ConvertVectorToV3(pos));
}

void RigidBodyController::SetControllerTorque(MVector^  torque)
{
	this->Controller->SetControllerTorque(ConvertVectorToV3(torque));
}

MVector^ RigidBodyController::ControllerForce::get()
{
	return ConvertV3ToVector( this->Controller->GetControllerForce());
}

MVector^ RigidBodyController::ControllerTorque::get()
{
	return ConvertV3ToVector( this->Controller->GetControllerTorque());
}

RigidBody ^ RigidBodyController::CurrentRigidBody::get()
{
	RigidBody ^ body = gcnew RigidBody();
	body->m_RigidBody = this->Controller->GetRigidBody();
	return body;
}

void RigidBodyController::SetController(neRigidBodyController *control)
{
	Controller = control;
}