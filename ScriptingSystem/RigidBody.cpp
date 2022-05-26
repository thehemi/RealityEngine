//=========== (C) Copyright 2004, Artficial Studios. All rights reserved. =======
// C# Scripting System
// Author: Mostafa Mohamed
//
//
//===============================================================================

#include "stdafx.h"
#include "Common.h"
#include "RigidBody.h"
#include "animatedbody.h"
#include "rigidbodycontroller.h"
#include "Simulator.h"

using namespace ScriptingSystem;


//Functions
void RigidBody::SetInertiaTensor(MVector^ tensor)
{
    neV3 nev;
    nev.Set(tensor->x,tensor->y,tensor->z);
    m_RigidBody->SetInertiaTensor(nev);
}

void RigidBody::SetInertiaTensor(MMatrix^ tensor)
{
    m_RigidBody->SetInertiaTensor(ConvertMatrixToM3(tensor));
}

MVector^ RigidBody::GetCylinderInertiaTensor(float cylinderDiameter,float cylinderHeight,float massOfCylinder)
{
    return ConvertV3ToVector(neCylinderInertiaTensor(cylinderDiameter,cylinderHeight,massOfCylinder));
}

MVector^ RigidBody::GetBoxInertiaTensor(MVector^ sizeOfCube,float massOfCube)
{
    return ConvertV3ToVector(neBoxInertiaTensor(ConvertVectorToV3(sizeOfCube),massOfCube));
}

MVector^ RigidBody::GetSphereInertiaTensor(float sphereDiameter,float massOfSphere)
{
    return ConvertV3ToVector(neSphereInertiaTensor(sphereDiameter,massOfSphere));
}

void RigidBody::SetForce(MVector^ force,MVector^ pos)
{
    neV3 nevf;
    nevf.Set(force->x,force->y,force->z);

    neV3 nevp;
    nevp.Set(pos->x,pos->y,pos->z);

    m_RigidBody->SetForce(nevf,nevp);
}

MVector^ RigidBody::GetPos()
{
    return ConvertV3ToVector(m_RigidBody->GetPos());
}

void RigidBody::SetPos(MVector^ p)
{
    m_RigidBody->SetPos(ConvertVectorToV3(p));
}

MMatrix^ RigidBody::GetRotationM3()
{
    return ConvertM3ToMatrix(m_RigidBody->GetRotationM3());
}

MQuaterion^ RigidBody::GetRotationQ()
{
    return ConvertQToVector(m_RigidBody->GetRotationQ());
}

void RigidBody::SetRotation(MMatrix^ m)
{
    m_RigidBody->SetRotation(ConvertMatrixToM3(m));
}

void RigidBody::SetRotation(MQuaterion^ q)
{
    m_RigidBody->SetRotation(ConvertVectorToQ(q));
}

T3^ RigidBody::GetTransform()
{
    return ConvertToT3(m_RigidBody->GetTransform());
}

MVector^ RigidBody::GetVelocityAtPoint(MVector^ point)
{
    return ConvertV3ToVector(m_RigidBody->GetVelocityAtPoint(ConvertVectorToV3(point)));
}

MVector^ RigidBody::GetVelocity()
{
    return ConvertV3ToVector(m_RigidBody->GetVelocity());
}

void RigidBody::SetVelocity(MVector^ v)
{
    m_RigidBody->SetVelocity(ConvertVectorToV3(v));
}

MVector^ RigidBody::GetAngularVelocity()
{
    return ConvertV3ToVector(m_RigidBody->GetAngularVelocity());
}

MVector^ RigidBody::GetAngularMomentum()
{
    return ConvertV3ToVector(m_RigidBody->GetAngularMomentum());
}

void RigidBody::SetAngularMomentum(MVector^ am)
{
    m_RigidBody->SetAngularMomentum(ConvertVectorToV3(am));
}

void RigidBody::UpdateBoundingInfo()
{
    m_RigidBody->UpdateBoundingInfo();
}

void RigidBody::ApplyImpulse(MVector^ impulse)
{
    m_RigidBody->ApplyImpulse(ConvertVectorToV3(impulse));
}

void RigidBody::ApplyImpulse(MVector^ impulse,MVector^ pos)
{
    m_RigidBody->ApplyImpulse(ConvertVectorToV3(impulse),ConvertVectorToV3(pos));
}

void RigidBody::ApplyTwist(MVector^ twist)
{
    m_RigidBody->ApplyTwist(ConvertVectorToV3(twist));
}

void RigidBody::GravityEnable(bool yes)
{
    m_RigidBody->GravityEnable((neBool) yes);
}

bool RigidBody::GravityEnable()
{
    return (m_RigidBody->GravityEnable() == 1);
}

void RigidBody::CollideConnected(bool yes)
{
    m_RigidBody->CollideConnected((neBool) yes);
}

bool RigidBody::CollideConnected()
{
    return (m_RigidBody->CollideConnected() == 1);
}

bool RigidBody::Activate()
{
    return (m_RigidBody->Active() == 1);
}

void RigidBody::Activate(bool yes,RigidBody ^hint)
{
    return m_RigidBody->Active((neBool) yes,hint->m_RigidBody);
}

void RigidBody::Activate(bool yes,AnimatedBody ^hint)
{
    return m_RigidBody->Active((neBool) yes,hint->m_AnimatedBody);
}

void RigidBody::Activate(bool yes)
{
    return m_RigidBody->Active((neBool)yes,(neRigidBody*) 0);
}

int RigidBody::GetGeometryCount()
{
    return m_RigidBody->GetGeometryCount();
}

TokamakGeometry ^ RigidBody::AddGeometry()
{
    TokamakGeometry ^g=gcnew TokamakGeometry(this);
    return g;
}

bool RigidBody::RemoveGeometry(TokamakGeometry ^ g)
{
    return (m_RigidBody->RemoveGeometry(g->geometry)==1);
}

void RigidBody::BeginIterateGeometry()
{
    m_RigidBody->BeginIterateGeometry();
}

//try to change this way of return by add a void* in the neGeometry class to hold this pointer
TokamakGeometry ^ RigidBody::GetNextGeometry()
{
    neGeometry * gem = this->m_RigidBody->GetNextGeometry();

    if(gem == 0)
        return nullptr;

    TokamakGeometry ^ gem2 = gcnew TokamakGeometry();
    gem2->geometry = gem;

    return gem2;
}

void RigidBody::SetLinearDamping(float value)
{
    m_RigidBody->SetLinearDamping(value);
}

void RigidBody::SetAngularDamping(float value)
{
    m_RigidBody->SetAngularDamping(value);
}

void RigidBody::SetSleepingParameter(float value)
{
    m_RigidBody->SetSleepingParameter(value);
}
//check if we must delete the old rigid body
//if a crash happened then return the old pointer only
RigidBody ^ RigidBody::BreakGeometry(TokamakGeometry ^ g)
{
    RigidBody ^new_r = gcnew RigidBody();
    new_r->m_RigidBody = m_RigidBody->BreakGeometry(g->geometry);
    return new_r;
}

Sensor ^ RigidBody::AddSensor()
{
    Sensor ^s=gcnew Sensor(this);

    if(s->sensor == 0)
        return nullptr;

    return s;
}

bool RigidBody::RemoveSensor(Sensor ^ s)
{
    return (m_RigidBody->RemoveSensor(s->sensor)==1);
}

void RigidBody::BeginIterateSensor()
{
    m_RigidBody->BeginIterateSensor();
}

Sensor ^ RigidBody::GetNextSensor()
{
    Sensor ^s=gcnew Sensor();
    s->sensor = m_RigidBody->GetNextSensor();

    if(s->sensor == 0)
        return nullptr;

    return s;
}
/*
RigidBodyController ^ RigidBody::AddController(int period)
{
    if(m_Controller)
    {
        m_RigidBody->RemoveController(m_Controller->controll->Controller);
        delete m_Controller;
    }

    m_Controller=new RigidB(this,period);
    return m_Controller->controll;
}


bool RigidBody::RemoveController(RigidBodyController ^ rbController)
{
    bool b=(m_RigidBody->RemoveController(rbController->Controller)==1);
    if(m_Controller)
    {
        delete m_Controller;
        m_Controller = 0;
    }

    return b;
}

void RigidBody::BeginIterateController()
{
    m_RigidBody->BeginIterateController();
}

RigidBodyController ^ RigidBody::GetNextController()
{
    RigidBodyController ^c=gcnew RigidBodyController();
    c->body = this;
    c->Controller = m_RigidBody->GetNextController();

    if(c->Controller == 0)
        return nullptr;

    return c;
}*/
void RigidBody::Finalize()
{
    Dispose(false);
    disposed = true;

}		
void RigidBody::Dispose() 
{
    Dispose(true);
    disposed = true;
    GC::SuppressFinalize(this);
}




void RigidBody::Torque::set(MVector^ value) 
{ 
    neV3 nev;
    nev.Set(value->x,value->y,value->z);
    m_RigidBody->SetTorque(nev); 
}

MVector^ RigidBody::Torque::get() 
{ 
    neV3 nev= m_RigidBody->GetTorque();
    MVector^ v = gcnew MVector(nev.v[0],nev.v[1],nev.v[2]);
    return v;
}

void RigidBody::Force::set(MVector^ value) 
{ 
    neV3 nev;
    nev.Set(value->x,value->y,value->z);
    m_RigidBody->SetForce(nev); 
}

MVector^ RigidBody::Force::get() 
{ 
    neV3 nev= m_RigidBody->GetForce();
    MVector^ v = gcnew MVector(nev.v[0],nev.v[1],nev.v[2]);
    return v;
}
RigidBody::RigidBody(Simulator ^simulator)
{
    OnCollision=nullptr;

//    m_Controller = 0;
    m_Simulator=simulator->m_Simulator;
    m_RigidBody=simulator->m_Simulator->CreateRigidBody();

    s_rigidBodies->Add(s_currentIndex,this);
    
       m_id = new RigidBodyIdentifier();
            m_id->managedIndex=s_currentIndex;
            m_RigidBody->SetUserData((u32)(int)(void*)m_id);

    m_index=s_currentIndex;
    s_currentIndex--;
}

void RigidBody::SetParent(MActor^ parent)
{
 if(m_id)
	 m_id->Parent = parent->m_actor;

 Parent = parent;
}

void RigidBody::Dispose(bool disposing) 
{
    if (!disposed) 
    {
        if (disposing) 
        {
            s_rigidBodies->Remove(m_index);
        }

		Parent = nullptr;

		PhysicsEngine::Instance()->QueueForDeletion(m_RigidBody);

        //m_Simulator->FreeRigidBody(this->m_RigidBody);
        m_Simulator=0;

        if(m_id)
            delete m_id;


    }
}
