//=========== (C) Copyright 2004, Artficial Studios. All rights reserved. =======
// C# Scripting System
// Author: Mostafa Mohamed
//
//
//===============================================================================
#pragma once

namespace ScriptingSystem
{
	ref class Joint;
	public ref class JointController
	{
        private public:
            neJointController * controller;
            void SetController(neJointController *control);
	public:
		Joint ^ joint;
		JointControllerCallbackEx ^ callback;
		
		JointController(){}
		JointController(Joint ^ joint,JointControllerCallbackEx ^ callback,int period);


        property Joint ^ GetJoint{Joint ^ get();}
        property MVector^ ControllerForceBodyA{MVector^ get();void set(MVector^ force);}
        property MVector^ ControllerForceBodyB{MVector^ get();void set(MVector^ force);}
        property MVector^ ControllerTorqueBodyA{MVector^ get();void set(MVector^ torque);}
        property MVector^ ControllerTorqueBodyB{MVector^ get();void set(MVector^ torque);}

		void SetControllerForceWithTorqueBodyA( MVector^  force,  MVector^  pos);
		void SetControllerForceWithTorqueBodyB( MVector^  force,  MVector^  pos);
	};
}