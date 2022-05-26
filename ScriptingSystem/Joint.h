//=========== (C) Copyright 2004, Artficial Studios. All rights reserved. =======
// C# Scripting System
// Author: Mostafa Mohamed
//
//
//===============================================================================
#pragma once
#include "RigidBody.h"
#include "AnimatedBody.h"

namespace ScriptingSystem
{
	public enum class JointType
	{
		NE_JOINT_BALLSOCKET,
		NE_JOINT_BALLSOCKET2,
		NE_JOINT_HINGE,
		NE_JOINT_SLIDE,
	};

	 ref class JointController;
	class JointControllerInterface;
	 ref class JointControllerCallbackEx;
	public ref class Joint
	{
	private:
		JointControllerInterface * m_Controller;
		neSimulator * m_Simulator;
		RigidBody ^ra,^rb;
		AnimatedBody ^ab;

	public public:
		neJoint * m_Joint;

    public:
		Joint ()
		{
			m_Controller = 0;
		}

		Joint(Simulator ^simulator,RigidBody ^rigid)
		{
			m_Controller = 0;
			m_Simulator=simulator->m_Simulator;
			m_Joint=simulator->m_Simulator->CreateJoint(rigid->m_RigidBody);
			ra = rigid;
		}

		Joint(Simulator ^simulator,RigidBody ^a,RigidBody ^b)
		{
			m_Simulator=simulator->m_Simulator;
			m_Joint=simulator->m_Simulator->CreateJoint(a->m_RigidBody,b->m_RigidBody);
			ra = a;
			rb = b;
		}

		Joint(Simulator ^simulator,RigidBody ^a,AnimatedBody ^b)
		{
			m_Simulator=simulator->m_Simulator;
			m_Joint=simulator->m_Simulator->CreateJoint(a->m_RigidBody,b->m_AnimatedBody);
			ra = a;
			ab = b;
		}

		void SetType(JointType t);
		JointType GetType();

		RigidBody ^ GetRigidBodyA();
		RigidBody ^ GetRigidBodyB();
		AnimatedBody ^ GetAnimatedBodyB();

		void Enable(bool yes);
		bool Enable();

		void SetJointFrameA(T3^ frameA);
		void SetJointFrameB(T3^ frameB);
		T3 ^ GetJointFrameA();
		T3^ GetJointFrameB();

		void SetJointFrameWorld(T3^ frame);

		void SetJointLength(float length);
		float GetJointLength();

		void EnableLimit(bool yes);
		bool EnableLimit();
		void EnableLimit2(bool yes);
		bool EnableLimit2();

		float GetUpperLimit();
		void SetUpperLimit(float upperLimit);
		float GetLowerLimit();
		void SetLowerLimit(float lowerLimit);
		float GetUpperLimit2();
		void SetUpperLimit2(float upperLimit);
		float GetLowerLimit2();
		void SetLowerLimit2(float lowerLimit);

		void SetEpsilon(float e);
		float GetEpsilon();
		void SetIteration(int i);
		int GetIteration();

		JointController ^ Joint::AddController(JointControllerCallbackEx ^ controller, int period);
		bool Joint::RemoveController(JointController ^ rbController);
		void Joint::BeginIterateController();
		JointController ^ Joint::GetNextController();
        void SetDampingFactor(float factor)
        {
            m_Joint->SetDampingFactor(factor);
        }
            

		void Finalize();
		void Dispose(); 

        protected:
            bool  disposed;
            virtual void Dispose(bool disposing);
	};
}