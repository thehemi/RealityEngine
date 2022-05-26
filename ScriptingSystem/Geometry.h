//=========== (C) Copyright 2004, Artficial Studios. All rights reserved. =======
// C# Scripting System
// Author: Mostafa Mohamed
//
//
//===============================================================================
#pragma once
//#include "rigidbody.h"
//#include "animatedbody.h"



namespace ScriptingSystem
{
	ref class RigidBody;
	ref class AnimatedBody;

	public enum class BreakFlag
	{
		NE_BREAK_DISABLE,
		NE_BREAK_NORMAL,
		NE_BREAK_ALL,
		NE_BREAK_NEIGHBOUR,

		/*	the following are the same as above, 
		except it create a rigid particle instead of a rigid body 
		*/

		NE_BREAK_NORMAL_PARTICLE, 
		NE_BREAK_ALL_PARTICLE,
		NE_BREAK_NEIGHBOUR_PARTICLE,
	};

	public ref class TokamakGeometry
	{
        private public:
            neGeometry * geometry;
	public:
		

		TokamakGeometry(){}
		TokamakGeometry(RigidBody ^body);
		TokamakGeometry(AnimatedBody ^ body);
	
        property T3^ Transform { void set(T3^ t);
        T3^ get();}
        property int MaterialIndex { void set(int index);
        int get();}
        property unsigned int  UserData { void set(unsigned int UserData);
        unsigned int get();}
        property BreakFlag BreakageFlag{ void set(BreakFlag flag);
        BreakFlag get();}
        property float  BreakageMass{ void set(float mass);
        float get();}
        property MVector^ BreakageInertiaTensor{ void set(MVector^ tensor);
        MVector^ get();}
        property float BreakageMagnitude{void set(float mag);
        float get();}
        property float BreakageAbsorption{void set(float absorb);
        float get();}
        property float BreakageNeighbourRadius{ void set(float radius);
        float get();}

        property MVector^ BreakagePlane{ void set(MVector^ planeNormal);
        MVector^ get();}

		void SetSphereDiameter(float diameter);
		bool GetSphereDiameter(float & diameter);
		void SetCylinder(float diameter,float height);
		bool GetCylinder(float  & diameter, float & height);

		void SetBoxSize(float width, float height, float depth);
		void SetBoxSize(MVector^  boxSize);
		MVector^ GetBoxSize(); // return false if geometry is not a box
	};
}
