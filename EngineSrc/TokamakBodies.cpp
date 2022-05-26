//=========== Copyright (c) 2004, Artificial Studios.All rights reserved. ===========
// Classes for various tokamak objects
// Author: Dave Sleeper
//=============================================================================
#include "stdafx.h"
#include "TokamakPhysics.h"

//-----------------------------------------------------------------------------
// MultiPrimitive Rigid Body geometry element (box)
//-----------------------------------------------------------------------------
void CustomRigidBody::AddBoxGeometry(Vector& BoxSize,Matrix& transform, int material)
{
    neV3 m0, m1, m2;
    m0.Set(transform.m0.x,transform.m0.y,transform.m0.z);
    m1.Set(transform.m1.x,transform.m1.y,transform.m1.z);
    m2.Set(transform.m2.x,transform.m2.y,transform.m2.z);
    neM3 rot;
    rot.SetColumns(m0, m1, m2);

    neGeometry * geom = rBody->AddGeometry();

    neV3 boxSize;
    boxSize.Set(BoxSize.x,BoxSize.y,BoxSize.z);
    geom->SetBoxSize(boxSize);

    neT3 t; 		
    t.SetIdentity();
    t.pos.Set(transform.m3.x,transform.m3.y,transform.m3.z);
    t.rot = rot;
    geom->SetTransform(t);

    rBody->UpdateBoundingInfo();
    geom->SetMaterialIndex(material);
}

//-----------------------------------------------------------------------------
// User-Configurable Rigid Body (Multi-Primitive Container Body)
//-----------------------------------------------------------------------------
void CustomRigidBody::Create(Actor* parent, Vector& BoxSize, float Mass, int material,bool hasMultiPrimitives)
{
    Parent = parent;
    rBody = TokSimulator->CreateRigidBody();
    rBody->SetSleepingParameter(0.0075f);

    neV3 boxSize;
    boxSize.Set(BoxSize.x,BoxSize.y,BoxSize.z);

    rBody->SetInertiaTensor(neBoxInertiaTensor(boxSize, Mass));
    rBody->SetMass(Mass);
    m_id = RigidBodyIdentifier::CreateFromRigidBody(this);
    rBody->SetUserData((u32)&m_id);
    rBody->SetAngularDamping(0.01f);
    rBody->SetLinearDamping(0.01f);
    rBody->GravityEnable(true);	
}

//-----------------------------------------------------------------------------
// User-Configurable Rigid Body (BOX)
//-----------------------------------------------------------------------------
void CustomRigidBody::Create(Actor* parent, Vector& BoxSize, float Mass, int material)
{
    Parent = parent;
    rBody = TokSimulator->CreateRigidBody();
    rBody->SetSleepingParameter(0.0075f);

    neGeometry * geom = rBody->AddGeometry();

    neV3 boxSize;
    boxSize.Set(BoxSize.x,BoxSize.y,BoxSize.z);
    geom->SetBoxSize(boxSize);
    neT3 t; 		
    t.SetIdentity();
    t.pos.Set(0.0f, 0.0f, 0.0f);
    geom->SetTransform(t);

    geom->SetMaterialIndex(material);

    rBody->UpdateBoundingInfo();	
    rBody->SetInertiaTensor(neBoxInertiaTensor(boxSize, Mass));
    rBody->SetMass(Mass);
    m_id = RigidBodyIdentifier::CreateFromRigidBody(this);
    rBody->SetUserData((u32)&m_id);
    rBody->SetAngularDamping(0.01f);
    rBody->SetLinearDamping(0.01f);
    rBody->GravityEnable(true);	
}

//-----------------------------------------------------------------------------
// User-Configurable Rigid Body (Sphere)
//-----------------------------------------------------------------------------
void CustomRigidBody::Create(Actor* parent, float SphereDiameter, float Mass, int material)
{
    Parent = parent;
    rBody = TokSimulator->CreateRigidBody();
    rBody->SetSleepingParameter(0.0075f);

    neGeometry * geom = rBody->AddGeometry();
    geom->SetSphereDiameter(SphereDiameter);

    neT3 t; 		
    t.SetIdentity();
    t.pos.Set(0.0f, 0.0f, 0.0f);
    geom->SetTransform(t);

    geom->SetMaterialIndex(material);

    rBody->UpdateBoundingInfo();
    rBody->SetInertiaTensor(neSphereInertiaTensor(SphereDiameter, Mass));
    rBody->SetMass(Mass);
    m_id = RigidBodyIdentifier::CreateFromRigidBody(this);
    rBody->SetUserData((u32)&m_id);
    rBody->SetAngularDamping(0.01f);
    rBody->SetLinearDamping(0.01f);
    rBody->GravityEnable(true);
    isRigidBodySphere = true;
}

//-----------------------------------------------------------------------------
// User-Configurable Rigid Body (Capsule)
//-----------------------------------------------------------------------------
void CustomRigidBody::Create(Actor* parent, float CylinderHeight, float CylinderDiameter, float Mass, int material)
{
    Parent = parent;
    rBody = TokSimulator->CreateRigidBody();
    rBody->SetSleepingParameter(0.0075f);

    neGeometry * geom = rBody->AddGeometry();
    geom->SetCylinder(CylinderDiameter,CylinderHeight);

    neT3 t; 		
    t.SetIdentity();
    t.pos.Set(0.0f, 0.0f, 0.0f);
    geom->SetTransform(t);

    geom->SetMaterialIndex(material);

    rBody->UpdateBoundingInfo();
    rBody->SetInertiaTensor(neCylinderInertiaTensor(CylinderDiameter,CylinderHeight,Mass));
    rBody->SetMass(Mass);
    m_id = RigidBodyIdentifier::CreateFromRigidBody(this);
    rBody->SetUserData((u32)&m_id);
    rBody->SetAngularDamping(0.01f);
    rBody->SetLinearDamping(0.01f);
    rBody->GravityEnable(true);
}

//-----------------------------------------------------------------------------
// ??
//-----------------------------------------------------------------------------
void TokamakRigidBody::Destroy()
{
    TokamakBody::Destroy();
    TokSimulator->FreeRigidBody(rBody);
    if(rBody!=NULL)rBody=NULL;
}

//-----------------------------------------------------------------------------
// ??
//-----------------------------------------------------------------------------
void TokamakBody::Collided(neCollisionInfo& collisionInfo)
{
    if(Parent->CollisionFlags & Actor::CF_IGNORE_TOUCHED)
        return;

    if(collisionInfo.typeB == NE_TERRAIN)
    {
        if(collisionInfo.materialIdB && collisionInfo.materialIdB != MATERIALFLOOR)
        {
            //we've collided with a non-Tok Actor
            lastCollisionInfo = collisionInfo;
            touchList.push_back((Actor*)collisionInfo.materialIdB);
        }
        else
        {
            isCollidedOnWorld = true;
            LastCollidedOnWorld = GSeconds;
            if(GetAngularVelocity().Length() > GetVelocity().Length())
            {
                isAngularCollidedOnWorld = true;
                LastAngularCollidedOnWorld = GSeconds;
            }
            if(Parent->CollisionFlags & Actor::CF_WANT_WORLD_TOUCHED && (!wasCollidedOnWorld || (!isRigidBodySphere && GetAngularVelocity().Length() > GetVelocity().Length() && !wasAngularCollidedOnWorld)))
            {
                CollisionInfo info;
                info.mat = NULL;
                info.otherTouched = NULL;
                info.actualDistance = 0;
                info.touched = NULL;
                info.normal = NV3_TO_VECTOR(collisionInfo.collisionNormal);
                info.point = NV3_TO_VECTOR(collisionInfo.worldContactPointA);

                //we've collided with the World geometry
                Parent->Touched(NULL,&info);
            }
        }
    }
    else if(collisionInfo.typeB == NE_RIGID_BODY)
    {
        //we've collided with another Rigid Body - Tok Actor
        lastCollisionInfo = collisionInfo;
        neRigidBody * rbB = (neRigidBody *)collisionInfo.bodyB;
        u32 userData=rbB->GetUserData();
        if(userData)
        {
            RigidBodyIdentifier * id =(RigidBodyIdentifier*)userData;
            if (id->managedIndex >0)
                touchList.push_back(((TokamakBody*)id->rigidBody)->Parent);
        }
    }
}

//-----------------------------------------------------------------------------
// ??
//-----------------------------------------------------------------------------
void TokamakBody::Tick()
{
    wasCollidedOnWorld = isCollidedOnWorld;
    wasAngularCollidedOnWorld = isAngularCollidedOnWorld;

    if(GSeconds - LastCollidedOnWorld > .065)
        isCollidedOnWorld = false;

    if(GSeconds - LastAngularCollidedOnWorld > .065)
        isAngularCollidedOnWorld = false;

    if(touchList.size())
    {
        CollisionInfo info;
        info.mat = NULL;
        info.otherTouched = NULL;
        info.actualDistance = 0;
        info.touched = NULL;
        info.normal = NV3_TO_VECTOR(lastCollisionInfo.collisionNormal);
        info.point = NV3_TO_VECTOR(lastCollisionInfo.worldContactPointA);
        // New touch list
        for(int j=0;j<touchList.size();j++)
        {
            //vector<Actor*>::iterator location = find(Parent->touchingActors.begin(), Parent->touchingActors.end(), touchList[j]);
            // Make sure that we've just touched this object
            bool notFound = true;
            for(int i = 0; i < Parent->touchingActors.size();i++)
            {
                if(Parent->touchingActors[i] == touchList[j])
                {
                    notFound = false;
                    break;
                }
            }
            if(notFound)//location == Parent->touchingActors.end()
            {
                Parent->Touched(touchList[j], &info);
                if(touchList[j]->CollisionFlags & Actor::CF_RECEIVE_INCOMING_ACTOR_TOUCH)touchList[j]->Touched(Parent,&info);
            }
        }
    }

    //FIXME: is this really necessary? 
    //seems to allow for the possibility of massive stacked addRefs 
    //unless there's a perfect 1:1 correlation between Touched & Untouched
    /*// Old touch list, untouching..
    for(int j=0;j<Parent->touchingActors.size();j++){
    // Look for actors that were touching us (actor) last time, but are no longer touching
    // If we find any, call UnTouched() for both of us
    vector<Actor*>::iterator location = find(touchList.begin(), touchList.end(), Parent->touchingActors[j]);
    if(location == touchList.end()){
    // Tell the actor that we don't reference it any more
    Parent->touchingActors[j]->DelRef(Parent);
    // Untouch event
    Parent->UnTouched(Parent->touchingActors[j]);

    // Give the other actor an UnTouched event unless we are a passable object -- Tok Actors can't be passable objects?
    //if(! (actor->CollisionFlags & Actor::CF_PASSABLE_BBOX))
    Parent->touchingActors[j]->UnTouched(Parent);
    }
    }*/

    //FIXME: Touchlist "safe" refs aren't getting cleared...
    //for(int i = 0; i < Parent->touchingActors.size(); i++)
    //{
    //Parent->touchingActors[i]->DelRef(Parent);
    //}
    Parent->touchingActors = touchList;
    //for(int i = 0; i < Parent->touchingActors.size(); i++)
    //{
    //	Parent->touchingActors[i]->AddRef(Parent);
    //}
    touchList.clear();
}

//-----------------------------------------------------------------------------
// ??
//-----------------------------------------------------------------------------
void TokamakBody::Destroy()
{
    //FIXME: HACCKK!!! Touched actor refs aren't getting properly cleared so let's cheat and manually clear them here
    Parent->touchingActors.clear();	
}
