//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
// Various Game-side Tokamak Physics Objects
//====================================================================================

#include "stdafx.h"
#include "GameEngine.h"
#include "PhysicsObjects.h"
#include "Editor.h"

//REGISTER DATA-DRIVEN (INI) VARIABLES HERE!
//RegBegin(BuggyRigidBody);
//RegFloat(BuggyRigidBody,springLength);
//RegFloat(BuggyRigidBody,ForceToFlip);

//-----------------------------------------------------------------------------
// Chained Light Physics System
//-----------------------------------------------------------------------------
void ChainedLight::Create(Actor* parent, Vector& BoxSize, float Mass,Vector& Location, float MassChandelier,Vector& BoxChandelier, int _numLinks)
{
	numLinks = _numLinks;
	Vector theLocation = Location;
	Parent = parent;

	float linkLength = .258f;

	rBody[0] = TokSimulator->CreateRigidBody();
	rBody[0]->CollideConnected(false);
	rBody[0]->SetSleepingParameter(0.01f);
	neGeometry * geom = rBody[0]->AddGeometry();
	neV3 boxSize;
	boxSize.Set(BoxSize.x,BoxSize.y,BoxSize.z);
	geom->SetBoxSize(boxSize);
	neT3 t; 		
	t.SetIdentity();
	t.pos.Set(0.0f, 0.0f, 0.0f);
	geom->SetTransform(t);
	geom->SetMaterialIndex(MATERIAL_LOW_BOUNCY);
	rBody[0]->UpdateBoundingInfo();	
	rBody[0]->SetInertiaTensor(neBoxInertiaTensor(boxSize, Mass));
	rBody[0]->SetMass(Mass);
	m_id = RigidBodyIdentifier::CreateFromRigidBody(this);
	m_id.IgnoreTerrain = true;
	rBody[0]->SetUserData((u32)&m_id);
	rBody[0]->SetAngularDamping(.0091f);
	rBody[0]->SetLinearDamping(.0031f);
	rBody[0]->GravityEnable(true);
	rBody[0]->SetPos(*(neV3*)&theLocation);

	neT3 jointFrame;
	jointFrame.SetIdentity();
	joints[0] = TokSimulator->CreateJoint(rBody[0]);
	jointFrame.pos.Set(theLocation.x,theLocation.y + .15, theLocation.z);
	joints[0]->SetJointFrameWorld(jointFrame);
	joints[0]->SetType(neJoint::NE_JOINT_BALLSOCKET);
	joints[0]->SetJointLength(9.6);
	joints[0]->SetDampingFactor(.1);
	joints[0]->Enable(true);

	neRigidBody * lastBody = rBody[0];
	neJoint* lastJoint = NULL;

	for (int i = 1; i < numLinks; i++)
	{
		theLocation.y -= linkLength;
		rBody[i] = TokSimulator->CreateRigidBody();
		rBody[i]->CollideConnected(false);
		rBody[i]->SetSleepingParameter(0.01f);
		neGeometry * geom = rBody[i]->AddGeometry();
		neV3 boxSize;
		boxSize.Set(BoxSize.x,BoxSize.y,BoxSize.z);
		geom->SetBoxSize(boxSize);
		geom->SetMaterialIndex(MATERIAL_LOW_BOUNCY);
		neT3 t; 		
		t.SetIdentity();
		t.pos.Set(0.0f, 0.0f, 0.0f);
		geom->SetTransform(t);
		rBody[i]->UpdateBoundingInfo();	
		rBody[i]->SetInertiaTensor(neBoxInertiaTensor(boxSize, Mass));
		rBody[i]->SetMass(Mass);
		rBody[i]->SetUserData((u32)&m_id);
		rBody[i]->SetAngularDamping(.0091f);
		rBody[i]->SetLinearDamping(.0031f);
		rBody[i]->GravityEnable(true);
		rBody[i]->SetPos(*(neV3*)&theLocation);

		neT3 jointFrame;
		jointFrame.SetIdentity();
		joints[i] = TokSimulator->CreateJoint(rBody[i], lastBody);
		jointFrame.pos.Set(theLocation.x,theLocation.y + .15, theLocation.z);
		joints[i]->SetJointFrameWorld(jointFrame);
		joints[i]->SetType(neJoint::NE_JOINT_BALLSOCKET);
		joints[i]->SetJointLength(9.6);
		joints[i]->SetDampingFactor(.1);
		joints[i]->Enable(true);

		lastBody = rBody[i];
	}

	if(true)
	{
		theLocation.y -= linkLength + BoxChandelier.y/2 - .182;
		rBody[numLinks] = TokSimulator->CreateRigidBody();
		rBody[numLinks]->CollideConnected(false);
		rBody[numLinks]->SetSleepingParameter(0.01f);
		neGeometry * geom = rBody[numLinks]->AddGeometry();
		neV3 boxSize;
		boxSize.Set(BoxChandelier.x,BoxChandelier.y,BoxChandelier.z);
		geom->SetBoxSize(boxSize);
		geom->SetMaterialIndex(MATERIAL_LOW_BOUNCY);
		neT3 t; 		
		t.SetIdentity();
		t.pos.Set(0.0f, 0.0f, 0.0f);
		geom->SetTransform(t);
		rBody[numLinks]->UpdateBoundingInfo();	
		rBody[numLinks]->SetInertiaTensor(neBoxInertiaTensor(boxSize, MassChandelier));
		rBody[numLinks]->SetMass(MassChandelier);
		rBody[numLinks]->SetUserData((u32)&m_id);
		rBody[numLinks]->SetAngularDamping(.0091f);
		rBody[numLinks]->SetLinearDamping(.003f);
		rBody[numLinks]->GravityEnable(true);
		rBody[numLinks]->SetPos(*(neV3*)&theLocation);


		joints[numLinks] = NULL;
		neT3 jointFrame;
		jointFrame.SetIdentity();
		joints[numLinks] = TokSimulator->CreateJoint(rBody[numLinks], lastBody);
		jointFrame.pos.Set(theLocation.x,theLocation.y + BoxChandelier.y/2, theLocation.z);
		joints[numLinks]->SetJointFrameWorld(jointFrame);

		joints[numLinks]->SetType(neJoint::NE_JOINT_BALLSOCKET);
		joints[numLinks]->SetJointLength(9.6);
		joints[numLinks]->SetDampingFactor(.1);
		joints[numLinks]->Enable(true);

		joints[numLinks]->SetEpsilon(TOKEPSILON*2);
		joints[numLinks]->SetIteration(1);
	}
	numSecondsInLowVelocity = 100;
}
//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void ChainedLight::Destroy()
{
	TokamakBody::Destroy();
	for (int i = 0; i < numLinks+1; i++)
	{
	TokSimulator->FreeJoint(joints[i]);
	}
	for (int i = 0; i < numLinks+1; i++)
	{
		TokSimulator->FreeRigidBody(rBody[i]);
	}
}

//register it with the class map type factory
REGISTER_FACTORY(ChainedLightEntity);
//-----------------------------------------------------------------------------
/// ctor Swinging Chained Light
//-----------------------------------------------------------------------------
ChainedLightEntity::ChainedLightEntity(World* world) : Actor(world)
{
	if(Engine::Instance()->IsDedicated())
		return;

	m_NumLinks = 10;
	EnablePhysicsInEditor = false;
	m_AllowIncludeExclude = true;
	swingMagnitude = .014f;
	swingPeriod = 2.65f;
	swingPeriodCounter = 0;

	//EditorVars.push_back(EditorVar("Chain Links",&m_NumLinks,"Physics","Number of chain links upwards from the base model. Each is .2m in height."));
	EditorVars.push_back(EditorVar("Swing Magnitude",&swingMagnitude,"Physics","The magnitude of the impulse that the chain swings with."));
	EditorVars.push_back(EditorVar("Swing Period",&swingPeriod,"Physics","The period the chain's swinging."));
	EditorVars.push_back(EditorVar("Light Name",&MyLightName,"Visuals","Light that this system binds to."));
	EditorVars.push_back(EditorVar("Shadow Name",&MyShadowName,"Visuals","Shadow Projector that this system binds to."));
	EditorVars.push_back(EditorVar("Editor Physics",&EnablePhysicsInEditor,"Physics","Enables rigid body physics for this object within Reality Builder. To refresh physics state with new values when physics are already enabled, set this to False then True again."));

	myLight = NULL;
	myShadow = NULL;

	if(g_Game.g_IsGameApp || EnablePhysicsInEditor)
	{
	PhysicsFlags = PHYS_RIGIDBODYDYANMICS;
	CollisionFlags = CF_BBOX;
	GhostObject = false;
	}
	else 
		GhostObject = true;

	linkParticles = new ChainLightLinkParticles(world,m_NumLinks,.12,.16);
}
void ChainedLightEntity::OnRender(Camera* cam)
{
	if(m_TokamakHandle)
	{
		for(int i = 0; i < m_NumLinks; i++)
		{
			Vector loc = ((ChainedLight*)m_TokamakHandle)->GetLocation(i);
			linkParticles->SetParticlePos(i,loc);
		}
		Location = ((ChainedLight*)m_TokamakHandle)->GetLocation(m_NumLinks);
		Rotation = ((ChainedLight*)m_TokamakHandle)->GetRotation(m_NumLinks);
		MyModel->SetTransform(Rotation,Location);
		Matrix LightRot = Matrix::LookTowards(-Rotation.GetUp());
		if(myLight)
		{
			myLight->Location = Location;
			myLight->GetCurrentState().Position = Location;
			myLight->Rotation = LightRot;
			myLight->GetCurrentState().Intensity = initLightIntensity*.6 + .4f*fabsf(sin(GSeconds*1.7f));
		}
		if(myShadow)
		{
			myShadow->Location = Location;
			myShadow->GetCurrentState().Position = Location + LightRot.GetDir()*.75;
			myShadow->Rotation = LightRot;
		}
	}
}
void ChainedLightEntity::PostRender(Camera* cam)
{
		if(OriginalTransform.m3.Length())
		{
		Location = OriginalTransform.m3;
		Rotation = OriginalTransform.GetRotationMatrix();
		}
}
void ChainedLightEntity::InitializeRigidBody()
{
	if(Engine::Instance()->IsDedicated())
		return;

	m_TokamakHandle = new ChainedLight();
	Vector BoxSize;
	BoxSize.Set(.07,.31,.07);
	Vector BoxSizeChandelier = Vector(.4,.4,.4);
	((ChainedLight*)m_TokamakHandle)->Create(this,BoxSize,.4 * (30.f/m_NumLinks),Location+Vector(0,.258f*m_NumLinks,0),3.4f,BoxSizeChandelier,m_NumLinks);
}
void ChainedLightEntity::Tick()
{
	if(Engine::Instance()->IsDedicated())
		return;

	if(!m_TokamakHandle && (g_Game.g_IsGameApp || (EnablePhysicsInEditor && !IsSelected)))
	{
		PhysicsFlags = PHYS_RIGIDBODYDYANMICS;
		CollisionFlags = CF_BBOX;
		GhostObject = false;

		myLight = MyWorld->FindLight(MyLightName);
		if(myLight)
		{
			initLightIntensity = myLight->GetCurrentState().Intensity;
			OriginalLightTransform = myLight->Rotation;	
			OriginalLightTransform.m3 = myLight->Location;
		}
		myShadow = MyWorld->FindLight(MyShadowName);
		if(myShadow)
		{
			OriginalShadowTransform = myShadow->Rotation;
			OriginalShadowTransform.m3 = myShadow->Location;
		}
	    OriginalTransform = Rotation;
		OriginalTransform.m3 = Location;
		InitializeRigidBody();
	}
	else if(!g_Game.g_IsGameApp && !EnablePhysicsInEditor && IsSelected)
	{
		if(m_TokamakHandle)
			PhysicsEngine::Instance()->DestroyHandle(m_TokamakHandle);
		m_TokamakHandle = NULL;
		GhostObject = true;
		PhysicsFlags = PHYS_NOT_AFFECTED_BY_GRAVITY;

		if(myLight)
		{
			initLightIntensity = myLight->GetCurrentState().Intensity;
			OriginalLightTransform = myLight->Rotation;	
			OriginalLightTransform.m3 = myLight->Location;
		}
		if(myShadow)
		{
			OriginalShadowTransform = myShadow->Rotation;
			OriginalShadowTransform.m3 = myShadow->Location;
		}
	    OriginalTransform = Rotation;
		OriginalTransform.m3 = Location;
	}
	else if(!m_TokamakHandle)
	{
		if(OriginalTransform.m3.Length())
		{
		Location = OriginalTransform.m3;
		Rotation = OriginalTransform.GetRotationMatrix();
		if(myLight)
		{
			myLight->Rotation = OriginalLightTransform.GetRotationMatrix();	
			myLight->Location = OriginalLightTransform.m3;	
		}
		if(myShadow)
		{
			myShadow->Rotation = OriginalShadowTransform;
			myShadow->Location = OriginalShadowTransform.m3;
		}
		}
	}

	if(m_TokamakHandle)
	{
		if(g_Game.GetTickSpeedScale() < 1.01f)
		{
		float deltaTime = GDeltaTime;
		if(deltaTime > .075)
			deltaTime = .075;
		swingPeriodCounter += deltaTime;
		((ChainedLight*)m_TokamakHandle)->AddImpulse(Vector(swingMagnitude*cos(swingPeriod*swingPeriodCounter),0,swingMagnitude*sin(swingPeriod*swingPeriodCounter)));
		}
	}

	Actor::Tick();
}