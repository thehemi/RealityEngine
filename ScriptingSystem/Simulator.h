//=========== (C) Copyright 2004, Artficial Studios. All rights reserved. =======
// C# Scripting System
// Author: Mostafa Mohamed
//
//
//===============================================================================
#pragma once
#include "Common.h"

namespace ScriptingSystem
{
	 ref class Simulator;
	class GlobalCollisionCallback
	{
	public:
		static gcroot<Simulator^> simulator;

		static void SetSimulator(Simulator ^s);
		static void Collision(neCollisionInfo & collisionInfo);
	};

	public ref class CollisionCallback
	{
	public:
		virtual void Collision(TCollisionInfo^ table) = 0;
	};

	class GlobalBreakageCallback
	{
	public:
		static gcroot<Simulator^> simulator;

		static void SetSimulator(Simulator ^s);
		static void Breakage(neByte * originalBody, neBodyType bodyType, neGeometry * brokenGeometry, neRigidBody * newBody);
	};

	public ref class BreakageCallback
	{
	public:
		virtual void Breakage(unsigned char * originalBody, BodyType bodyType, TokamakGeometry ^ brokenGeometry, RigidBody ^ newBody) = 0;
	};


	 ref class RigidBody;
	 ref class AnimatedBody;
	public ref class Simulator
	{
        private public:
        neSimulator * m_Simulator;
        Simulator(neSimulator * simulator);
        bool needToDelete;
	public:
		
		CollisionCallback ^m_CollisionCallback;
		BreakageCallback ^m_BreakageCallback;

		Simulator(SimulatorSizeInfo sizeInfo,MVector^ gravity);

		RigidBody ^ CreateRigidBody();
		RigidBody ^ CreateRigidParticle();
		AnimatedBody ^ CreateAnimatedBody();
		void FreeRigidBody(RigidBody ^ body);
		void FreeAnimatedBody(AnimatedBody ^ body);

		CollisionTable ^ GetCollisionTable();

		bool SetMaterial(int index, float friction, float restitution);
		bool GetMaterial(int index, float& friction, float& restitution);

		void Advance(float sec);
		void Advance(float sec, int nSteps);

		void SetTerrainMesh(TriangleMesh ^tris);

		void SetCollisionCallback(CollisionCallback^ cb);
		CollisionCallback ^ GetCollisionCallback();

		void SetBreakageCallback(BreakageCallback ^ cb);
		BreakageCallback ^ GetBreakageCallback();

		void Finalize()
		{
            if (needToDelete)
			    neSimulator::DestroySimulator(m_Simulator);
		}
	};
}
