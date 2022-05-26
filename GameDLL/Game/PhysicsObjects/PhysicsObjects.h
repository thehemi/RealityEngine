//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
/// PhysicsObjects.h 
//  Various Game-side Physics controller objects
//====================================================================================

#pragma once
#ifndef PHYSICSOBJECTS_H
#define PHYSICSOBJECTS_H

#include "..\physics\include\tokamak.h"
#include "..\FX\Particles.h"

//-----------------------------------------------------------------------------
///Swinging Chained Light Physics
//-----------------------------------------------------------------------------
#define NUM_CHAIN_LINKS 20
class ChainedLight : public TokamakBody
{
public:
	int numLinks;
	float numSecondsInLowVelocity;
	neRigidBody* rBody[NUM_CHAIN_LINKS+1];
	neJoint* joints[NUM_CHAIN_LINKS+1];
	ChainedLight() : TokamakBody(){ }
	void Create(Actor* parent, Vector& BoxSize, float Mass,Vector& Location, float MassChandelier,Vector& BoxChandelier, int numLinks);
	Vector GetLocation(int linkIndex)	{ return *(Vector*)&rBody[linkIndex]->GetPos();}
	Matrix GetRotation(int linkIndex)	{neT3 trans; trans.rot = rBody[linkIndex]->GetRotationM3(); return *(Matrix*)&NET3_TO_D3DXMATRIX(trans);}
	Vector GetVelocity(int linkIndex){return *(Vector*)&(rBody[linkIndex]->GetVelocity());	}
	virtual void SetLocation(Vector pos){ rBody[0]->SetPos(*(neV3*)&pos); }
	virtual void SetVelocity(Vector vel){ rBody[0]->SetVelocity(*(neV3*)&vel); }
	virtual void Destroy();
	virtual void AddImpulse(Vector impulse){rBody[numLinks]->ApplyImpulse(*(neV3*)&impulse, rBody[numLinks]->GetPos());}
};

class ChainLightLinkParticles : public ParticleSystem
{
public:
	Texture ChainLinkTexture;
	ChainLightLinkParticles(World* world, int num, float width, float height) : ParticleSystem(world, num, COLOR_RGBA(255,255,255,255), width, height, &ChainLinkTexture,BLEND_INVSRCALPHA)
	{ChainLinkTexture.Load("chainlink.dds");}
	virtual void Tick(){}
	void SetParticlePos(int index, Vector& position){particles[index].position = position;}
};

//-----------------------------------------------------------------------------
/// Swinging Chained Light
//-----------------------------------------------------------------------------
class ChainedLightEntity : public Actor
{
	CLASS_NAME(ChainedLightEntity);
public:
	bool EnablePhysicsInEditor;

	string MyLightName;
	Light* myLight;
	string MyShadowName;
	Light* myShadow;

	float swingMagnitude;
	float swingPeriod;
	float swingPeriodCounter;

	float initLightIntensity;

	ChainedLightEntity(World* world);
	virtual void OnRender(class Camera* cam);
	virtual void PostRender(class Camera* cam);
	virtual void InitializeRigidBody();
	virtual void UpdateFromRigidBody(){}
	virtual void Tick();
	virtual ~ChainedLightEntity()
	{
		linkParticles->LifeTime = 0;
	}
	int m_NumLinks;

	Matrix OriginalLightTransform;
	Matrix OriginalShadowTransform;
	Matrix OriginalTransform;
	ChainLightLinkParticles* linkParticles;
};

#endif