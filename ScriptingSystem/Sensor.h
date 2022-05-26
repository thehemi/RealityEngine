//=========== (C) Copyright 2004, Artficial Studios. All rights reserved. =======
// C# Scripting System
// Author: Mostafa Mohamed
//
//
//===============================================================================
#pragma once

namespace ScriptingSystem
{
	ref class RigidBody;
	ref class AnimatedBody;
	public ref class Sensor
	{
        private public:
            neSensor * sensor;
	public:
		
		Sensor(RigidBody ^ body);
		Sensor(AnimatedBody ^ body);
		Sensor();

		void SetLineSensor( MVector^  pos,  MVector^ lineVector);
        property unsigned int UserData
        {
            void set(unsigned int userData);
         unsigned int get();
        };
        property  MVector^ LineVector{MVector^ get();}
        property  MVector^ LineUnitVector{MVector^ get();}
        property  MVector^ LinePos{MVector^ get();}
        property  float DetectDepth{float get();}
        property  MVector^ DetectNormal{MVector^ get();}
        property  MVector^ DetectContactPoint{MVector^ get();}
        property  RigidBody ^ DetectRigidBody{RigidBody ^ get();}
        property  AnimatedBody ^ DetectAnimatedBody{AnimatedBody ^ get();}
        property  int DetectMaterial{int get();}
	};
}
