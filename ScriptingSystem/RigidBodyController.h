//=========== (C) Copyright 2004, Artficial Studios. All rights reserved. =======
// C# Scripting System
// Author: Mostafa Mohamed
//
//
//===============================================================================
#pragma once
#include "rigidbody.h"

namespace ScriptingSystem
{
    public delegate	void ControllerEvent(RigidBodyController^ Controller);
	public ref class RigidBodyController
	{
        private public:
            neRigidBodyController * Controller;
            void SetController(neRigidBodyController *control);     
             RigidBodyControllerInterface * callback;

             void ControllerCallback()
             {
                 if (OnControllerCallback!=nullptr)
							OnControllerCallback(this);
             }
	public:
        event ControllerEvent^	OnControllerCallback;

       
		//RigidBodyControllerCallbackEx ^ callback;
		RigidBody^ body;

	
		RigidBodyController(RigidBody^ body,int period);

        property RigidBody ^ CurrentRigidBody{RigidBody^ get();}
        property MVector^ ControllerForce{MVector^ get();}
        property MVector^ ControllerTorque{MVector^ get();}
		void SetControllerForce( MVector^  force);
		void SetControllerTorque( MVector^  torque);
		void SetControllerForceWithTorque( MVector^  force, MVector^  pos);
		void RemoveController();

		virtual void Finalize()
        {
            delete callback;
        }
	};

}