//=========== (C) Copyright 2004, Artficial Studios. All rights reserved. =======
// C# Scripting System
// Author: Mostafa Mohamed
//
//
//===============================================================================
#include "StdAfx.h"
#include "jointcontroller.h"
#include "joint.h"
#include "JointControllerCallback.h"

using namespace ScriptingSystem;

JointController::JointController(Joint ^ joint,JointControllerCallbackEx ^ callback,int period)
{
	this->joint = joint;
	this->callback = callback;
	this->controller = joint->m_Joint->AddController(callback->joint_control,period);
}



void JointController::ControllerForceBodyA::set(MVector^ force)
{
	this->controller->SetControllerForceBodyA(ConvertVectorToV3(force));
}

void JointController::ControllerForceBodyB::set(MVector^ force)
{
	this->controller->SetControllerForceBodyB(ConvertVectorToV3(force));
}

void JointController::ControllerTorqueBodyA::set(MVector^ force)
{
	this->controller->SetControllerTorqueBodyA(ConvertVectorToV3(force));
}

void JointController::ControllerTorqueBodyB::set(MVector^ force)
{
	this->controller->SetControllerTorqueBodyB(ConvertVectorToV3(force));
}

void JointController::SetControllerForceWithTorqueBodyA(MVector^  force, MVector^  pos)
{
	this->controller->SetControllerForceWithTorqueBodyA(ConvertVectorToV3(force),
		ConvertVectorToV3(pos));
}

void JointController::SetControllerForceWithTorqueBodyB( MVector^  force, MVector^  pos)
{
	this->controller->SetControllerForceWithTorqueBodyB(ConvertVectorToV3(force),
		ConvertVectorToV3(pos));
}

Joint ^ JointController::GetJoint::get()
{
	Joint ^ joint = gcnew Joint();
	joint->m_Joint = this->controller->GetJoint();
	return joint;
}

MVector^ JointController::ControllerForceBodyA::get()
{
	return ConvertV3ToVector( this->controller->GetControllerForceBodyA());
}

MVector^ JointController::ControllerForceBodyB::get()
{
	return ConvertV3ToVector(this->controller->GetControllerForceBodyB());
}

MVector^ JointController::ControllerTorqueBodyA::get()
{
	return ConvertV3ToVector(this->controller->GetControllerTorqueBodyA());
}

MVector^ JointController::ControllerTorqueBodyB::get()
{
	return ConvertV3ToVector(this->controller->GetControllerTorqueBodyB());
}

void JointController::SetController(neJointController *control)
{
	controller = control;
}
