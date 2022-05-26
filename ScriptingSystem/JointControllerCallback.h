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
	ref class Joint;
	ref class JointController;

	public ref class JointControllerCallbackEx
	{
        private public:
        neJointControllerCallback *joint_control;
	public:
		
		virtual void JointControllerCallbackfn(JointController ^  controller) = 0;

		JointControllerCallbackEx(void);
		~JointControllerCallbackEx(void);
	};

	class JointControllerInterface : public neJointControllerCallback
	{
	public:
		gcroot<JointController^> controll;
		JointControllerInterface(Joint ^joint,JointControllerCallbackEx ^callback, int period);
		virtual void JointControllerCallback(neJointController * controller);
	};
}