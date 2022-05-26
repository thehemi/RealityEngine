//=========== (C) Copyright 2004, Artificial Studios.All rights reserved. ===========
// Tokamak classes for different bodies and vehicles
// Author: Jeremy Stieglitz & David Sleeper
//===============================================================================

#include "stdafx.h"
#undef TOKAMAK_USE_DLL // No DLL
#include "Collision.h"
#include "TokamakPhysics.h"

// Use the static library
// TIM: Debug is very slow, so we won't switch to it unless we need to debug tokamak
//#ifdef _DEBUG
//#pragma comment(lib,"..\\physics\\lib\\tokamak_md.lib") // tokamak_d.lib")
//#else
#pragma comment(lib,"..\\physics\\lib\\tokamakdll.lib") // tokamak.lib")
//#endif


neSimulator* TokamakObject::TokSimulator = NULL;

// Globals
static World* g_World = NULL;
static neCollisionInfo *RBCollisionInfo;


// Hard-coded limits
#define RIGIDBC			950	//256 rigibBodies seems enough
#define RIGIDPC			0	//no rigid particles necessary
#define ANIMATEDBC		0		//or animated bodies

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void RigidBodyControllerCallbackInfo(neCollisionInfo & collisionInfo)
{
	if(collisionInfo.typeA != NE_RIGID_BODY)return;
	u32 userData=((neRigidBody *)collisionInfo.bodyA)->GetUserData();
	if(userData)
	{
		RigidBodyIdentifier * id =(RigidBodyIdentifier*)userData;
		if (id->managedIndex < 0)
			SSystem_RigidBodyCollide(id->managedIndex,&collisionInfo);
		else
			((TokamakObject*)id->rigidBody)->Collided(collisionInfo);  
	}
}

//-----------------------------------------------------------------------------
// Tok's Main Callbacks which will retrieve the data from Opcode
//	Information provided directly from Tokamak guys
//
//	const neV3 & minBound, const neV3 & maxBound, 
//	This is the bound of the rigid body, you should use this to 
//	filter out your triangles. 
//	s32 ** candidateTriangles, 
//	You should assign (*candidateTriangles) = your array of s32 of 
//	indices into (*triangles) those triangles you want Tokamak to test. 
//	neTriangle ** triangles, 
//	(*triangles) = array of your triangles 
//	neV3 ** vertices, 
//	(*vertices) = array of your vertices use by your triangles 
//	s32 * candidateCount, 
//	(*candidateCount) = the size of candidateTriangleArray 
//	s32 * triangleCount 
//	(*triangleCount) = the size of triangle array. 
//-----------------------------------------------------------------------------
void TerrainTriangleQueryCallback (const neV3 & minBound, const neV3 & maxBound, 
								   s32 ** candidateTriangles,
								   neTriangle ** triangles,
								   neV3 ** vertices,
								   s32 * candidateCount,
								   s32 * triangleCount,
								   neRigidBody * body)
{
	// Keep the arrays as static, so the callback has time to use them
	static neV3 *verts		= NULL;
	static neTriangle *tri	= NULL;
	static s32 *triindex	= NULL;

	u32 userData = body->GetUserData();
	bool IgnoreTerrain = false;
	if(userData)
		IgnoreTerrain = ((RigidBodyIdentifier*)userData)->IgnoreTerrain;

	if(IgnoreTerrain || body == PhysicsEngine::Instance()->RayTestBody)
	{
		//this special rigid body is only for testing ray collisions onto other rigid bodies
		*vertices			= 0;
		*triangles			= 0;
		*candidateTriangles = 0;
		*candidateCount		= 0; 
		*triangleCount		= 0;
		return;
	}

	// Delete arrays we're done with from the last callbakc
	SAFE_DELETE_ARRAY(verts);
	SAFE_DELETE_ARRAY(tri);
	SAFE_DELETE_ARRAY(triindex);

	vector<CollisionFace> possibleFaces;
	BBox sweepBox;
	sweepBox.min = *(Vector*)&minBound;
	sweepBox.max = *(Vector*)&maxBound;

	Actor* Owner = NULL;
	if(userData)
		Owner = ((RigidBodyIdentifier*)userData)->Parent;

	g_World->GatherPossibleColliders(Owner,possibleFaces,Vector(0,0,0),sweepBox);

	// Create new arrays
	verts		= new neV3[possibleFaces.size()*3];
	tri			= new neTriangle[possibleFaces.size()];
	triindex	= new s32[possibleFaces.size()];

	for( int j=0; j<possibleFaces.size(); j++ )
	{
		verts[j*3] = *(neV3*)&possibleFaces[j].vert[0];
		verts[j*3+1] = *(neV3*)&possibleFaces[j].vert[1];
		verts[j*3+2] = *(neV3*)&possibleFaces[j].vert[2];
		tri[j].indices[0] = j*3;
		tri[j].indices[1] = j*3+1;
		tri[j].indices[2] = j*3+2;

		//*******************
		//SETTING MATERIALS
		//*******************

		if(possibleFaces[j].owner && !possibleFaces[j].owner->IsPrefab)
			tri[j].materialID =  (s32)(void*)possibleFaces[j].owner;
		else
			tri[j].materialID = MATERIALFLOOR;

		tri[j].flag = neTriangle::NE_TRI_TRIANGLE;

		//set canidates to all the possible
		triindex[j] = j;
	}

	//link up the vertices, triangles, and indices
	*vertices			= verts;
	*triangles			= tri;
	*candidateTriangles = triindex;
	*candidateCount		= possibleFaces.size(); 
	*triangleCount		= possibleFaces.size();
}

/// Runs the ray tests on rigid bodies' oriented bounding boxes within the PhysicsEngine
class TokRayQueryCallBack : public neRigidBodyControllerCallback
{
public:	
	void RigidBodyControllerCallback(neRigidBodyController * controller)
	{
		Vector nullVec;

		controller->GetRigidBody()->BeginIterateSensor();
		neSensor * sn;
		while (sn = controller->GetRigidBody()->GetNextSensor())
		{
			TokRayQuery* query = (TokRayQuery*)sn->GetUserData();
			if(!query)
				continue;
			query->rayInfo.detectionDepth = sn->GetDetectDepth();
			if(query->rayInfo.detectionDepth > 0)
			{
				query->rayInfo.contactPoint = *(Vector*)&sn->GetDetectContactPoint();
				query->rayInfo.collidedOnBody = sn->GetDetectRigidBody();
			}
			else
			{
				query->rayInfo.contactPoint = nullVec;
				query->rayInfo.collidedOnBody = NULL;
			}
		}
	};
};
TokRayQueryCallBack g_RayQueryCallBack; //rigib body controller callback

//-----------------------------------------------------------------------------
// PhysicsEngine
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Singleton 
//-----------------------------------------------------------------------------
PhysicsEngine* PhysicsEngine::Instance () 
{
	static PhysicsEngine inst;
	return &inst;
}

void TokLogOutput(char* logString)
{
	LogPrintf(logString);
}

//-----------------------------------------------------------------------------
// Called at Engine Init
//-----------------------------------------------------------------------------
void PhysicsEngine::Initialize()
{
	if(TokamakBody::TokSimulator!=NULL)return; //already initialized

	//set the gravity
	neV3 gravity;
	gravity.Set(0.0f, -12.5f, 0.0f);
	//gravity.Set(0.0f, -9.814f, 0.0f);

	neSimulatorSizeInfo sizeInfo;

	sizeInfo.rigidBodiesCount = RIGIDBC;
	sizeInfo.rigidParticleCount = RIGIDPC;
	sizeInfo.animatedBodiesCount = ANIMATEDBC;

	s32 totalBody = RIGIDBC + ANIMATEDBC + RIGIDPC;

	sizeInfo.geometriesCount = totalBody;

	sizeInfo.overlappedPairsCount = totalBody * (totalBody - 1) / 2; 
	sizeInfo.constraintsCount = 500;
	sizeInfo.constraintSetsCount = 500;
	sizeInfo.terrainNodesStartCount = 0;
	sizeInfo.constraintBufferSize=10000;

	//sizeInfo.terrainNodesGrowByCount = 200;

	TokamakBody::TokSimulator = neSimulator::CreateSimulator(sizeInfo, NULL, &gravity);

	//Setup some basic Materials
	TokamakBody::TokSimulator->SetMaterial(MATERIALFLOOR, 1, 1.31); 
	TokamakBody::TokSimulator->SetMaterial(MATERIAL_LOW_BOUNCY, .475f, 0.72f);
	TokamakBody::TokSimulator->SetMaterial(MATERIAL_MEDIUM_BOUNCY, .65f, 1.f);
	TokamakBody::TokSimulator->SetMaterial(MATERIAL_HIGH_BOUNCY, .9f, 1.45f);
	TokamakBody::TokSimulator->SetMaterial(MATERIAL_EXTREME_BOUNCY, .9f, 1.8f);
	TokamakBody::TokSimulator->SetMaterial(MATERIAL_LOW_BOUNCE_HIGHSLIDE, 1.1f, 0.15f);
	TokamakBody::TokSimulator->SetMaterial(MATERIAL_LOWBOUNCE_LOWSLIDE, .65f, 0.5f);

	//Set the Main CallBack
	TokamakBody::TokSimulator->SetTerrainTriangleQueryCallback(TerrainTriangleQueryCallback);

	//Normal objects
	TokamakBody::TokSimulator->GetCollisionTable()->Set(ALLOBJECTCOLLIDE,TERRAINCOLLIDEID,neCollisionTable::RESPONSE_IMPULSE_CALLBACK);
	TokamakBody::TokSimulator->GetCollisionTable()->Set(ALLOBJECTCOLLIDE,ALLOBJECTCOLLIDE,neCollisionTable::RESPONSE_IMPULSE_CALLBACK); 
	TokamakBody::TokSimulator->GetCollisionTable()->Set(ALLOBJECTCOLLIDE,IGNORERAGDOLLS,neCollisionTable::RESPONSE_IMPULSE_CALLBACK);
	TokamakBody::TokSimulator->GetCollisionTable()->Set(ALLOBJECTCOLLIDE,ONLYTERRAINCOLLIDE,neCollisionTable::RESPONSE_IGNORE);
	TokamakBody::TokSimulator->GetCollisionTable()->Set(ALLOBJECTCOLLIDE,RIGIDBODYRAYQUERY,neCollisionTable::RESPONSE_IGNORE);

	//Only terrain colliding objects
	TokamakBody::TokSimulator->GetCollisionTable()->Set(ONLYTERRAINCOLLIDE,ONLYTERRAINCOLLIDE,neCollisionTable::RESPONSE_IMPULSE_CALLBACK);
	TokamakBody::TokSimulator->GetCollisionTable()->Set(ONLYTERRAINCOLLIDE,TERRAINCOLLIDEID,neCollisionTable::RESPONSE_IMPULSE_CALLBACK);
	TokamakBody::TokSimulator->GetCollisionTable()->Set(ONLYTERRAINCOLLIDE,ALLOBJECTCOLLIDE,neCollisionTable::RESPONSE_IGNORE);
	TokamakBody::TokSimulator->GetCollisionTable()->Set(ONLYTERRAINCOLLIDE,IGNORERAGDOLLS,neCollisionTable::RESPONSE_IGNORE);
	TokamakBody::TokSimulator->GetCollisionTable()->Set(ONLYTERRAINCOLLIDE,RIGIDBODYRAYQUERY,neCollisionTable::RESPONSE_IGNORE);

	//ragdolls ignore each other before dying
	TokamakBody::TokSimulator->GetCollisionTable()->Set(IGNORERAGDOLLS,TERRAINCOLLIDEID,neCollisionTable::RESPONSE_IGNORE);
	TokamakBody::TokSimulator->GetCollisionTable()->Set(IGNORERAGDOLLS,ALLOBJECTCOLLIDE,neCollisionTable::RESPONSE_IGNORE);
	TokamakBody::TokSimulator->GetCollisionTable()->Set(IGNORERAGDOLLS,ONLYTERRAINCOLLIDE,neCollisionTable::RESPONSE_IGNORE);
	TokamakBody::TokSimulator->GetCollisionTable()->Set(IGNORERAGDOLLS,IGNORERAGDOLLS,neCollisionTable::RESPONSE_IGNORE);
	TokamakBody::TokSimulator->GetCollisionTable()->Set(IGNORERAGDOLLS,COLLIDERAGDOLLS,neCollisionTable::RESPONSE_IGNORE);
	TokamakBody::TokSimulator->GetCollisionTable()->Set(IGNORERAGDOLLS,RIGIDBODYRAYQUERY,neCollisionTable::RESPONSE_IGNORE);

	//ragdolls collide after dying
	TokamakBody::TokSimulator->GetCollisionTable()->Set(COLLIDERAGDOLLS,TERRAINCOLLIDEID,neCollisionTable::RESPONSE_IMPULSE_CALLBACK);
	TokamakBody::TokSimulator->GetCollisionTable()->Set(COLLIDERAGDOLLS,ALLOBJECTCOLLIDE,neCollisionTable::RESPONSE_IMPULSE_CALLBACK); 
	TokamakBody::TokSimulator->GetCollisionTable()->Set(COLLIDERAGDOLLS,IGNORERAGDOLLS,neCollisionTable::RESPONSE_IMPULSE_CALLBACK);
	TokamakBody::TokSimulator->GetCollisionTable()->Set(COLLIDERAGDOLLS,ONLYTERRAINCOLLIDE,neCollisionTable::RESPONSE_IGNORE);
	TokamakBody::TokSimulator->GetCollisionTable()->Set(COLLIDERAGDOLLS,IGNORERAGDOLLS,neCollisionTable::RESPONSE_IGNORE);
	TokamakBody::TokSimulator->GetCollisionTable()->Set(COLLIDERAGDOLLS,RIGIDBODYRAYQUERY,neCollisionTable::RESPONSE_IGNORE);

	//ray query collides on everything but itself
	TokamakBody::TokSimulator->GetCollisionTable()->Set(RIGIDBODYRAYQUERY,TERRAINCOLLIDEID,neCollisionTable::RESPONSE_IMPULSE_CALLBACK);
	TokamakBody::TokSimulator->GetCollisionTable()->Set(RIGIDBODYRAYQUERY,ALLOBJECTCOLLIDE,neCollisionTable::RESPONSE_IMPULSE_CALLBACK); 
	TokamakBody::TokSimulator->GetCollisionTable()->Set(RIGIDBODYRAYQUERY,IGNORERAGDOLLS,neCollisionTable::RESPONSE_IMPULSE_CALLBACK);
	TokamakBody::TokSimulator->GetCollisionTable()->Set(RIGIDBODYRAYQUERY,ONLYTERRAINCOLLIDE,neCollisionTable::RESPONSE_IMPULSE_CALLBACK);
	TokamakBody::TokSimulator->GetCollisionTable()->Set(RIGIDBODYRAYQUERY,IGNORERAGDOLLS,neCollisionTable::RESPONSE_IMPULSE_CALLBACK);
	TokamakBody::TokSimulator->GetCollisionTable()->Set(RIGIDBODYRAYQUERY,RIGIDBODYRAYQUERY,neCollisionTable::RESPONSE_IGNORE);

	TokamakBody::TokSimulator->SetCollisionCallback(RigidBodyControllerCallbackInfo);
	TokamakBody::TokSimulator->SetLogOutputCallback(&TokLogOutput);
	TokamakBody::TokSimulator->SetLogOutputLevel(neSimulator::LOG_OUTPUT_LEVEL_FULL);

	numCustomTokMaterials = MATERIAL_LOWBOUNCE_LOWSLIDE + 1;

	RayTestBody = NULL;
}

//-----------------------------------------------------------------------------
// Called at app shutdown
//-----------------------------------------------------------------------------
void PhysicsEngine::Destroy()
{
	if(TokamakBody::TokSimulator!=NULL){//lets destroy this machine 		
		if(RayTestBody)
			TokamakBody::TokSimulator->FreeRigidBody(RayTestBody);
		//	RayTestBody=NULL;

		for(int i = 0; i < RBDeletionQueue.size(); i++)
		{
			TokamakBody::TokSimulator->FreeRigidBody(RBDeletionQueue[i]);
		}
		RBDeletionQueue.clear();

		neSimulator::DestroySimulator(TokamakBody::TokSimulator);
		TokamakBody::TokSimulator=NULL;
	}

}
//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void PhysicsEngine::DestroyHandle(void* tokamakHandle)
{
	if(tokamakHandle != NULL)
	{
		((TokamakBody*)tokamakHandle)->Destroy();
		delete tokamakHandle;
	}
}

//-----------------------------------------------------------------------------
// Called once per frame
//-----------------------------------------------------------------------------
void PhysicsEngine::Tick(World* world, float TimeScale, int MaxSteps)
{
	// Time left over from previous frame
	static float leftOverTime = 0.0f;

	g_World = world;

	// Get total time 
	float  time = GDeltaTime/TimeScale + leftOverTime; 

	float currentElapsed = GDeltaTime/TimeScale;
	// Determine the number of steps to perform 
	if(currentElapsed > 0.0001f && currentElapsed < .28f)
	{
		float secondsPerStep = 1 / float(60.f); //mStepsPerSecond
		unsigned int steps = (unsigned int)(time / secondsPerStep);

		unsigned int StepsToUse = steps;
		float secondsPerStepToUse = secondsPerStep;

		if(MaxSteps > 0 && StepsToUse > MaxSteps)
		{
			StepsToUse = MaxSteps;
			secondsPerStepToUse = (steps * secondsPerStep)/StepsToUse;			
		}

		// Advance the physics Simulation
		SetExceptionHandling(false);

		// Physics won't be reliable at > 1 Tick rates (which isn't practical for gameplay anyways, just for recording playback), so let's not Advance the physics sim any actual amount
		if(TimeScale > 1.01f)
			StepsToUse = 0;

		TokamakBody::TokSimulator->Advance((secondsPerStepToUse * StepsToUse)*TimeScale, StepsToUse);
		for(int i = 0; i < Instance()->RBDeletionQueue.size(); i++)
			TokamakBody::TokSimulator->FreeRigidBody(Instance()->RBDeletionQueue[i]);
		Instance()->RBDeletionQueue.clear();

		SetExceptionHandling(true);

		// Set leftover time
		leftOverTime = time - (steps * secondsPerStep);
	}
}
//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void World::AddTouchedImpulse(Vector& impulse, Actor* touched)
{
	if(!(touched->PhysicsFlags & Actor::PHYS_RIGIDBODYDYANMICS) || !(touched->PhysicsFlags & Actor::PHYS_PUSHABLE))return;
	if(!impulse.Length())return;

	if(touched->IsScriptActor())
	{
		if(!m_AddTouchedImpulse)return;
		m_AddTouchedImpulse(touched->GetManagedIndex(),(void*)&impulse);
	}
	else
	{
		if(!touched->m_TokamakHandle)return;
		((TokamakBody*)touched->m_TokamakHandle)->AddImpulse(impulse);
	}
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void World::AddTouchedImpulse(Vector& contactPoint, Vector& impulse ,Actor* touched)
{
	if(!(touched->PhysicsFlags & Actor::PHYS_RIGIDBODYDYANMICS) || !(touched->PhysicsFlags & Actor::PHYS_PUSHABLE))return;
	if(!touched->m_TokamakHandle)return;
	if(!impulse.Length())return;
	((TokamakBody*)touched->m_TokamakHandle)->AddImpulseContactPoint(impulse,contactPoint);
}
int PhysicsEngine::CreateTokMaterial(float Friction, float Restitution)
{
	numCustomTokMaterials++;
	TokamakBody::TokSimulator->SetMaterial(numCustomTokMaterials,Friction,Restitution);
	return numCustomTokMaterials;
}


Vector PhysicsEngine::GetLocation(neRigidBody* rBody)
{
	Vector loc = *(Vector*)&rBody->GetPos();
	return loc;
}
void PhysicsEngine::ApplyImpulse(neRigidBody* rBody,Vector impulse)
{
	rBody->ApplyImpulse(*(neV3*)&impulse,rBody->GetPos());
}


void TokRayQuery::Initialize()
{
	if(!PhysicsEngine::Instance()->RayTestBody)
		PhysicsEngine::Instance()->CreateRayTestBody();

	mySensor = NULL;

	neSensor * sn;
	PhysicsEngine::Instance()->RayTestBody->BeginIterateSensor();
	// try to find an existing unused sensor first
	while (sn = PhysicsEngine::Instance()->RayTestBody->GetNextSensor())
	{
		if(!sn->GetUserData())
			mySensor = sn;
	}

	// otherwise create a new one
	if(!mySensor)
		mySensor = PhysicsEngine::Instance()->RayTestBody->AddSensor();

	mySensor->SetUserData((u32)this);
}
void TokRayQuery::SetRaySegment(Vector& StartPos,Vector& DirSegment)
{
	mySensor->SetLineSensor(*(neV3*)&(StartPos+Vector(0,1000,0)),*(neV3*)&DirSegment);
	PhysicsEngine::Instance()->RayTestBody->UpdateBoundingInfo();
}
TokRayInfo* TokRayQuery::GetRayInfo()
{
	return &rayInfo;
}
TokRayQuery::~TokRayQuery()
{
	if(mySensor)
		Free();
}
void TokRayQuery::Free()
{
	SetRaySegment(Vector(0,0,0),Vector(0,0,0));
	PhysicsEngine::Instance()->RayTestBody->UpdateBoundingInfo();
	mySensor->SetUserData(NULL);
	//PhysicsEngine::Instance()->RayTestBody->RemoveSensor(mySensor);
	//PhysicsEngine::Instance()->RayTestBody->UpdateBoundingInfo();
	mySensor = NULL;
}
void PhysicsEngine::CreateRayTestBody()
{
	if(!RayTestBody)
	{
		//create special ray-test rigid body
		RayTestBody = TokamakBody::TokSimulator->CreateRigidBody();
		RayTestBody->SetSleepingParameter(1);
		neGeometry * geom = RayTestBody->AddGeometry();
		neV3 boxSize;
		boxSize.Set(.1,.1,.1);
		geom->SetBoxSize(boxSize);
		neT3 t; 		
		t.SetIdentity();
		t.pos.Set(0.0f, 0.0f, 0.0f);
		geom->SetTransform(t);
		RayTestBody->UpdateBoundingInfo();	
		RayTestBody->SetInertiaTensor(neBoxInertiaTensor(boxSize, 1));
		RayTestBody->SetMass(1);
		RayTestBody->SetUserData(NULL);
		RayTestBody->SetAngularDamping(0.01f);
		RayTestBody->SetLinearDamping(0.01f);
		RayTestBody->GravityEnable(false);
		RayTestBody->SetPos(*(neV3*)&Vector(0,-1000,0));
		RayTestBody->SetCollisionID(RIGIDBODYRAYQUERY);
		RayTestBody->AddController(&g_RayQueryCallBack,0);
	}
}

void PhysicsEngine::QueueForDeletion(neRigidBody* rBody)
{
	neV3 nullVec;
	nullVec.Set(0,0,0);
	rBody->SetVelocity(nullVec);
	rBody->SetAngularMomentum(nullVec);
	neV3 position;
	position.Set(10000,10000,10000);
	rBody->SetPos(position);
	rBody->SetCollisionID(IGNORERAGDOLLS);
	rBody->SetUserData(NULL);
	RBDeletionQueue.push_back(rBody);
}