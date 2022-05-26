//=========== (C) Copyright 2004, Artficial Studios. All rights reserved. =======
// C# Scripting System
// Author: Mostafa Mohamed
//
//
//===============================================================================
#pragma once
#include "sensor.h"
#include "geometry.h"
namespace ScriptingSystem
{
	 ref class RigidBody;
	 ref class Simulator;
	public ref class AnimatedBody
	{
	private public:
		neSimulator * m_Simulator;
        neAnimatedBody * m_AnimatedBody;

	public:
		
		AnimatedBody(){}
		AnimatedBody(Simulator^ simulator);


		//PROPERTIES

		property unsigned int UserData 
        {
          unsigned int get() { return m_AnimatedBody->GetUserData(); }
		  void set(unsigned int value) { m_AnimatedBody->SetUserData( value); }
        }

        property int CollisionID {
            int get() {return m_AnimatedBody->GetCollisionID(); }
		   void set(int value) { m_AnimatedBody->SetCollisionID( value); }
        }

		MVector^ GetPos();
		void SetPos(MVector^ p);

		MMatrix^ GetRotationM3();
		MQuaterion^ GetRotationQ();
		void SetRotation(MMatrix^ m);
		void SetRotation(MQuaterion^ q);;

		T3^ GetTransform();

		int GetGeometryCount();

		TokamakGeometry ^ AddGeometry();
		bool RemoveGeometry(TokamakGeometry ^ g);
		void BeginIterateGeometry();
		TokamakGeometry ^ GetNextGeometry();
		RigidBody ^ BreakGeometry(TokamakGeometry ^ g);

		Sensor ^ AddSensor();
		bool RemoveSensor(Sensor ^ s);
		void BeginIterateSensor();
		Sensor ^ GetNextSensor();

		void UpdateBoundingInfo();

		void CollideConnected(bool yes);
		bool CollideConnected();

		bool Activate();
		void Activate(bool yes,RigidBody ^hint);
		void Activate(bool yes,AnimatedBody ^hint);
		void Activate(bool yes);

		void Finalize();

	};
}