//=========== (C) Copyright 2004, Artficial Studios. All rights reserved. =======
// C# Scripting System
// Author: Mostafa Mohamed
//
//
//===============================================================================
#include "StdAfx.h"
#include ".\rigidbodycontrollercallback.h"

using namespace ScriptingSystem;
#include "JointController.h"
#include "joint.h"

JointControllerCallbackEx::JointControllerCallbackEx(void)
{
}

JointControllerCallbackEx::~JointControllerCallbackEx(void)
{
}

JointControllerInterface::JointControllerInterface(Joint ^joint,JointControllerCallbackEx ^ callback,int period)
{
	callback->joint_control =(neJointControllerCallback*) this;
	JointController ^controller=gcnew JointController(joint,callback,period);
	controll =  controller;
}

void JointControllerInterface::JointControllerCallback(neJointController * controller)
{
	controll->SetController(controller);
	controll->callback->JointControllerCallbackfn(controll);
}