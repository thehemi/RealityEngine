//=========== (C) Copyright 2004, Artficial Studios. All rights reserved. =======
// C# Scripting System
// Author: Mostafa Mohamed
//
//
//===============================================================================
#include "StdAfx.h"
#include ".\rigidbodycontrollercallback.h"

using namespace ScriptingSystem;
#include "RigidBodyController.h"
/*
RigidBodyControllerCallbackEx::RigidBodyControllerCallbackEx(void)
{
}
*/


RigidBodyControllerInterface::RigidBodyControllerInterface(RigidBodyController^ controller)
{
	controll =  controller;
}

void RigidBodyControllerInterface::RigidBodyControllerCallback(neRigidBodyController * controller)
{
	controll->ControllerCallback();
}