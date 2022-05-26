//=========== (C) Copyright 2004, Artficial Studios. All rights reserved. =======
// C# Scripting System
// Author: Mostafa Mohamed
//
//
//===============================================================================
#include "stdafx.h"
#include "Common.h"
#include "Joint.h"
#include "jointcontroller.h"
#include "jointcontrollercallback.h"
using namespace ScriptingSystem;

void Joint::SetType(JointType t)
{
	m_Joint->SetType((neJoint::ConstraintType) t);
}

JointType Joint::GetType()
{
	return (JointType) m_Joint->GetType();
}

RigidBody ^ Joint::GetRigidBodyA()
{
	return ra;
}

RigidBody ^ Joint::GetRigidBodyB()
{
	return rb;
}

AnimatedBody ^ Joint::GetAnimatedBodyB()
{
	return ab;
}

void Joint::Enable(bool yes)
{
	m_Joint->Enable((neBool)yes);
}

bool Joint::Enable()
{
	return (m_Joint->Enable() ==1);
}

void Joint::SetJointFrameA(T3^ frameA)
{
	m_Joint->SetJointFrameA(ConvertToneT3(frameA));
}

void Joint::SetJointFrameB(T3^ frameB)
{
	m_Joint->SetJointFrameB(ConvertToneT3(frameB));
}

T3^ Joint::GetJointFrameA()
{
	return ConvertToT3(m_Joint->GetJointFrameA());
}

T3^ Joint::GetJointFrameB()
{
	return ConvertToT3(m_Joint->GetJointFrameB());
}

void Joint::SetJointFrameWorld(T3^ frame)
{
	m_Joint->SetJointFrameWorld(ConvertToneT3(frame));
}

void Joint::SetJointLength(float length)
{
	m_Joint->SetJointLength(length);
}

float Joint::GetJointLength()
{
	return m_Joint->GetJointLength();
}

void Joint::EnableLimit(bool yes)
{
	m_Joint->EnableLimit((neBool) yes);
}

bool Joint::EnableLimit()
{
	return (m_Joint->EnableLimit() == 1);
}

void Joint::EnableLimit2(bool yes)
{
	m_Joint->EnableLimit2((neBool) yes);
}

bool Joint::EnableLimit2()
{
	return (m_Joint->EnableLimit2()==1);
}

float Joint::GetUpperLimit()
{
	return m_Joint->GetUpperLimit();
}

void Joint::SetUpperLimit(float upperLimit)
{
	m_Joint->SetUpperLimit(upperLimit);
}

float Joint::GetLowerLimit()
{
	return m_Joint->GetLowerLimit();
}

void Joint::SetLowerLimit(float lowerLimit)
{
	m_Joint->SetLowerLimit(lowerLimit);
}

float Joint::GetUpperLimit2()
{
	return m_Joint->GetUpperLimit2();
}

void Joint::SetUpperLimit2(float upperLimit)
{
	return m_Joint->SetUpperLimit2(upperLimit);
}

float Joint::GetLowerLimit2()
{
	return m_Joint->GetLowerLimit2();
}

void Joint::SetLowerLimit2(float lowerLimit)
{
	m_Joint->SetLowerLimit2(lowerLimit);
}

void Joint::SetEpsilon(float e)
{
	m_Joint->SetEpsilon(e);
}

float Joint::GetEpsilon()
{
	return m_Joint->GetEpsilon();
}

void Joint::SetIteration(int i)
{
	m_Joint->SetIteration(i);
}

int Joint::GetIteration()
{
	return m_Joint->GetIteration();
}


JointController ^ Joint::AddController(JointControllerCallbackEx ^ controller, int period)
{
	if(m_Controller)
	{
		m_Joint->RemoveController(m_Controller->controll->controller);
		delete m_Controller;
	}

	m_Controller=new JointControllerInterface(this,controller,period);
	return m_Controller->controll;
}


bool Joint::RemoveController(JointController ^ rbController)
{
	bool b=(m_Joint->RemoveController(rbController->controller)==1);
	if(m_Controller)
	{
		delete m_Controller;
		m_Controller = 0;
	}

	return b;
}

void Joint::BeginIterateController()
{
	m_Joint->BeginIterateController();
}


void Joint::Finalize()
{
    Dispose(false);
    disposed = true;

}		
void Joint::Dispose() 
{
    Dispose(true);
    disposed = true;
    GC::SuppressFinalize(this);
}

void Joint::Dispose(bool disposing) 
{
    if (!disposed) 
    {
		m_Simulator->FreeJoint(this->m_Joint);
		m_Simulator=0;

        if(m_Controller)
            delete m_Controller;
    }
}

JointController ^ Joint::GetNextController()
{
	JointController ^c=gcnew JointController();
	c->joint = this;
	c->controller = m_Joint->GetNextController();

	return c;
}