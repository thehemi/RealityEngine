//=========== (C) Copyright 2004, Artficial Studios. All rights reserved. =======
// C# Scripting System
// Author: Mostafa Mohamed
//
//
//===============================================================================
#pragma once
#include <vcclr.h>

namespace ScriptingSystem
{
	ref class RigidBody;
	ref class RigidBodyController;

	/*public ref class RigidBodyControllerCallbackEx
    {
        private public:
    neRigidBodyControllerCallback *rigid_control;
	public:
		
		virtual void RigidBodyControllerCallbackfn(RigidBodyController ^controller) = 0;

		RigidBodyControllerCallbackEx(void);
	};*/

	class RigidBodyControllerInterface : public neRigidBodyControllerCallback
	{
	public:
		gcroot<RigidBodyController^> controll;
		RigidBodyControllerInterface(RigidBodyController^ controller);
		virtual void RigidBodyControllerCallback(neRigidBodyController * controller);
	};
}