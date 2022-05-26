//=========== (C) Copyright 2004, Artficial Studios. All rights reserved. =======
// C# Scripting System
// Author: Mostafa Mohamed
//
//
//===============================================================================
#pragma once
#include "simulator.h"
#include "sensor.h"
#include "geometry.h"
#include "..\EngineInc\TokamakPhysics.h"

using namespace stdcli::language;
using namespace System::Collections;
using namespace System;

namespace ScriptingSystem
{
    public delegate	void CollisionEvent(TCollisionInfo^ info);

    ref class RigidBodyController;
    ref class AnimatedBody;
    ref class Simulator;
    public ref class RigidBody : IDisposable
    {
    public:
        event CollisionEvent^	OnCollision;
    private:
        RigidBodyIdentifier * m_id;
      //  RigidBodyControllerInterface * m_Controller;

        private public:
            neSimulator * m_Simulator;
            neRigidBody * m_RigidBody;
            int           m_index;

            static int       s_currentIndex;
            static Hashtable^ s_rigidBodies;

            void Collide(neCollisionInfo * info)
            {
                if( OnCollision != nullptr)
                {
                    TCollisionInfo^ cInfo=ConvertToCollisionInfo(info);
                    OnCollision(cInfo);
                }
            }
    public:
        

        static RigidBody()
        {
            s_currentIndex=-1;
            s_rigidBodies = gcnew Hashtable();
        }

        RigidBody()
        {
            OnCollision=nullptr;
//            m_Controller = 0;
            s_rigidBodies->Add(s_currentIndex,this);
            m_id = new RigidBodyIdentifier();
            m_id->managedIndex=s_currentIndex;
            m_RigidBody->SetUserData((u32)(int)(void*)m_id);
            m_index=s_currentIndex;
            s_currentIndex--;
        }
        RigidBody(Simulator ^ simulator);

        //PROPERTIES
        property float Mass{float get() {return m_RigidBody->GetMass(); }
        void set(float value) { m_RigidBody->SetMass( value); }}

		property bool IgnoreTerrain{bool get() {return m_id->IgnoreTerrain; }
        void set(bool value) { m_id->IgnoreTerrain = value; }}


        property unsigned int UserData{unsigned int get() { return m_RigidBody->GetUserData(); }
        void set(unsigned int value) { m_RigidBody->SetUserData( value); }}

        property int CollisionID{int get() {return m_RigidBody->GetCollisionID(); }
        void set(int value) { m_RigidBody->SetCollisionID( value); }}

        property MVector^ Force{MVector^ get();

        void set(MVector^ value);}

        property MVector^ Torque{MVector^ get();

         void set(MVector^ value);}

        //Functions
        void SetInertiaTensor(MVector^ tensor);
        void SetInertiaTensor(MMatrix^ tensor);


        void SetForce(MVector^ force,MVector^ pos);

        MVector^ GetPos();
        void SetPos(MVector^ p);

        MMatrix^ GetRotationM3();
        MQuaterion^ GetRotationQ();
        void SetRotation(MMatrix^ m);
        void SetRotation(MQuaterion^ q);

        T3^ GetTransform();

        MVector^ GetVelocity();
        MVector^ GetVelocityAtPoint(MVector^ point);
        void SetVelocity(MVector^ v);

        MVector^ GetAngularVelocity();
        MVector^ GetAngularMomentum();
        void SetAngularMomentum(MVector^ am);

        void SetLinearDamping(float value);
        void SetAngularDamping(float value);
        void SetSleepingParameter(float value);

        void UpdateBoundingInfo();

        void ApplyImpulse(MVector^ impulse);
        void ApplyImpulse(MVector^ impulse,MVector^ pos);
        void ApplyTwist(MVector^ twist);

        void GravityEnable(bool yes);
        bool GravityEnable();

        void CollideConnected(bool yes);
        bool CollideConnected();

        bool Activate();
        void Activate(bool yes,RigidBody ^hint);
        void Activate(bool yes,AnimatedBody ^hint);
        void Activate(bool yes);

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

      /*  RigidBodyController ^ AddController(int period);
        bool RemoveController(RigidBodyController ^ rbController);
        void BeginIterateController();
        RigidBodyController ^ GetNextController();*/

        static MVector^ GetCylinderInertiaTensor(float cylinderDiameter,float cylinderHeight,float massOfCylinder);
        static MVector^ GetBoxInertiaTensor(MVector^ sizeOfCube,float massOfCube);
        static MVector^ GetSphereInertiaTensor(float sphereDiameter,float massOfSphere);

        void Finalize();
		void Dispose(); 

		void SetParent(MActor^ parent);
		MActor^ Parent;

        protected:
            bool  disposed;
            virtual void Dispose(bool disposing);
};
	
    public ref class RigidBodyRayQuery : IDisposable
    {
	private:
		TokRayQuery* m_query;
	public:
		RigidBodyRayQuery()	
		{
			disposed = false;
			m_query = new TokRayQuery();
			m_query->Initialize();
		}
		RigidBody^ GetCollidedBody()
		{
			if(m_query)
			{
			TokRayInfo* info = m_query->GetRayInfo();
			if(info->collidedOnBody)
			{
				if(info->collidedOnBody->GetUserData())
					return (RigidBody^)RigidBody::s_rigidBodies[((RigidBodyIdentifier*)info->collidedOnBody->GetUserData())->managedIndex];
			}
			}

			return nullptr;
		}
		void SetRaySegment(MVector^ start, MVector^ end)
		{
			if(m_query)
				m_query->SetRaySegment(*start->m_vector,*end->m_vector);
		}
		
		void Finalize()
		{
			Dispose(false);
			disposed = true;
		}
		void Dispose()
		{
    Dispose(true);
    disposed = true;
    GC::SuppressFinalize(this);
		}

	protected:
       bool  disposed;
       virtual void Dispose(bool disposing)
	   {
			if (!disposed) 
			{
				m_query->Free();
				delete m_query;
				m_query = NULL;
			}
	   }
	};
}


