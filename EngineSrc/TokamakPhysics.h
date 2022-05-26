//=========== (C) Copyright 2004, Artificial Studios.All rights reserved. ===========
/// Tokamak classes for different bodies and vehicles
/// Author: Jeremy Stieglitz & David Sleeper
//===================================================================================
#include "..\physics\include\tokamak.h"

#ifndef TOKAMAKPHYSICS_H
#define TOKAMAKPHYSICS_H

//-----
/// Rigid Body collision ID sets
//-----
#define TERRAINCOLLIDEID -1
#define ALLOBJECTCOLLIDE 0
#define ONLYTERRAINCOLLIDE 1
#define IGNORERAGDOLLS 2
#define RIGIDPARTICLE_NOCALLBACK_COLLIDEID 3
#define COLLIDERAGDOLLS 4

//-----------------------------------------------------------------------------
/// Hard-coded materials defining object properties, also users have the option of manually specifying materials
//-----------------------------------------------------------------------------
#define MATERIALFLOOR			1
#define MATERIAL_LOW_BOUNCY		2
#define MATERIAL_MEDIUM_BOUNCY	3
#define MATERIAL_HIGH_BOUNCY	4
#define MATERIAL_EXTREME_BOUNCY 5
#define MATERIAL_LOW_BOUNCE_HIGHSLIDE 6
#define MATERIAL_LOWBOUNCE_LOWSLIDE 7
/// ??
#define CUSTOM_RIGIDBODY_TYPE_BOX		0
#define CUSTOM_RIGIDBODY_TYPE_SPHERE	1
#define CUSTOM_RIGIDBODY_TYPE_CYLINDER	2
#define CUSTOM_RIGIDBODY_TYPE_PICTUREFRAME	3
/// ??
#define TOKEPSILON 0.1f

#define MIN_FLIP_INTERVAL 3.0f //3 seconds in between flips


/// Conversion Macros
#ifndef NET3_TO_D3DXMATRIX   
#define NET3_TO_D3DXMATRIX( tmat )                                 \
	D3DXMATRIX( tmat.rot[0][0], tmat.rot[0][1], tmat.rot[0][2], 0.0f,  \
	tmat.rot[1][0], tmat.rot[1][1], tmat.rot[1][2], 0.0f,  \
	tmat.rot[2][0], tmat.rot[2][1], tmat.rot[2][2], 0.0f,  \
	tmat.pos[0],    tmat.pos[1],    tmat.pos[2],    1.0f );
#endif

#define NV3_TO_VECTOR( v3 )        \
	*(Vector*)&v3;

/// Base abstract class for all Engine Tokamak representations
class ENGINE_API TokamakObject
{
public:
/// Global simulator for Tokamak Objects
static neSimulator* TokSimulator;
TokamakObject(){}
/// Universal "Collided" callback to be overriden
virtual void Collided(neCollisionInfo& collisionInfo){}
};

/// \brief Abstract Engine-side encapsulation of common Tokamak Physics interfaces
/// Useful as a base for physics systems that are composed of more than one rigid body
class ENGINE_API TokamakBody : public TokamakObject
{
public:

	Actor* Parent;
	float impulseCoeff;
	TokamakBody() : TokamakObject()
	{
		impulseCoeff = 1.f;
		isCollidedOnWorld = true;
		wasCollidedOnWorld = true;
		wasAngularCollidedOnWorld = true;
		isAngularCollidedOnWorld = true;
		isRigidBodySphere = false;
		Parent = NULL;
		LastCollidedOnWorld = -BIG_NUMBER;
		LastAngularCollidedOnWorld = -BIG_NUMBER;
	}
	float LastCollidedOnWorld;
	float LastAngularCollidedOnWorld;
	virtual void Destroy();
	virtual void RigidController(neRigidBodyController * controller)
	{ return; };
	virtual void RigidControllerInfo(neCollisionInfo & collisionInfo)
	{ return; };
	virtual void AddImpulseContactPoint(Vector impulse, Vector point){}
	virtual void AddImpulse(Vector impulse){}
	virtual Vector GetVelocity(){return Vector();}
	virtual Vector GetAngularVelocity(){return Vector();}
	virtual void SetLocation(Vector pos){}
	virtual void SetVelocity(Vector vel){}
	virtual void SetRotation(Matrix mRot){}
	virtual Matrix GetRotation(){return Matrix();}
	virtual Vector GetLocation(){return Vector();}
	virtual void AddForce(Vector force){}
	virtual void UpdateBoundingInfo(){}
	virtual void Gravity(bool enableb){}
	virtual void SetAngularMomentum(Vector momentum){}
	virtual void Collided(neCollisionInfo& collisionInfo);
	virtual void Tick();
	bool isCollidedOnWorld;
	bool wasCollidedOnWorld;
	bool wasAngularCollidedOnWorld;
	bool isAngularCollidedOnWorld;
	bool isRigidBodySphere;
	vector<Actor*> touchList;
	neCollisionInfo lastCollisionInfo;
	Matrix ConvertToMatrix(neM3& rot)
	{
		return *(Matrix*)&Matrix(rot[0][0], rot[0][1], rot[0][2], 0.0f,
			rot[1][0], rot[1][1], rot[1][2], 0.0f,
			rot[2][0], rot[2][1], rot[2][2], 0.0f,0,0,0,1.f);
	}
	neM3 ConvertToNeM3(Matrix& mRot)
	{
		neV3 m0, m1, m2;
		m0.Set(mRot.m0.x,mRot.m0.y,mRot.m0.z);
		m1.Set(mRot.m1.x,mRot.m1.y,mRot.m1.z);
		m2.Set(mRot.m2.x,mRot.m2.y,mRot.m2.z);
		neM3 rot;
		rot.SetColumns(m0, m1, m2);
		return rot;
	}
	virtual Vector GetBBoxSize(){return Vector();}
};

/// Passthrough class for Tokamak to call to TokamakBody's Physics Controller (if any)
class RBControllerCallback: public neRigidBodyControllerCallback
{
public:	
	void RigidBodyControllerCallback(neRigidBodyController * controller)
	{
		((TokamakBody*)controller->GetRigidBody()->GetUserData())->RigidController(controller);
	};	
};

/// Main physics engine subsystem
class ENGINE_API PhysicsEngine {
public:
	int numCustomTokMaterials;
	void DestroyHandle(void* tokamakHandle);
	void Initialize();
	void Destroy();
	static PhysicsEngine* Instance ();
	int CreateTokMaterial(float Friction, float Restitution);
	static void Tick(class World* world, float TimeScale);
	Vector GetLocation(neRigidBody* rBody);
	void ApplyImpulse(neRigidBody* rBody,Vector impulse);
	neRigidBody* RayTestBody;
	bool IsInServerMode;
	void SetServerMode(bool ServerMode);
	RBControllerCallback m_RBControllerCallback; //rigib body controller callback
};

/// Structure for Tokamak Ray Query
struct ENGINE_API TokRayInfo
{
	float detectionDepth;
	Vector contactPoint;
	neRigidBody* collidedOnBody;
};

/// \brief Handles raytesting collisions onto oriented Rigid Bodies
//
/// Very fast but the rays are set to only onto Tokamak meshes, useful for precision determination
/// Of impact point onto oriented bbox, or determination of specific collision onto Tokamak mesh within the interior
/// of a physics system (such as rigid body elements on a ragdoll character)
class ENGINE_API TokRayQuery
{
public:
	TokRayQuery()
	{
	mySensor = NULL;
	rayInfo.collidedOnBody = NULL;
	rayInfo.detectionDepth = 0;
	}
	TokRayInfo* GetRayInfo();
	void Initialize();
	void SetRaySegment(Vector& StartPos,Vector& DirSegment);
	virtual ~TokRayQuery();
	TokRayInfo rayInfo;
private:
	neSensor* mySensor;
};

/// Encapsulates common usage of a Single Rigid-Body system
class ENGINE_API TokamakRigidBody : public TokamakBody
{
protected:
	neRigidBody* rBody;
	neGeometry* m_MainGeometry;
public:	
	virtual Vector GetAngularVelocity(){return NV3_TO_VECTOR(rBody->GetAngularVelocity());}
	virtual void SetLocation(Vector pos){ rBody->SetPos(*(neV3*)&pos); }
	virtual void SetVelocity(Vector vel){ rBody->SetVelocity(*(neV3*)&vel); }
	virtual void SetAngularMomentum(Vector momentum)
	{ 
		rBody->SetAngularMomentum(*(neV3*)&momentum); 
	}
	virtual void SetRotation(Matrix mRot)
	{			
		neV3 m0, m1, m2;
		m0.Set(mRot.m0.x,mRot.m0.y,mRot.m0.z);
		m1.Set(mRot.m1.x,mRot.m1.y,mRot.m1.z);
		m2.Set(mRot.m2.x,mRot.m2.y,mRot.m2.z);
		neM3 rot;
		rot.SetColumns(m0, m1, m2);
		rBody->SetRotation(rot);
	}

	virtual Vector GetVelocity()	{ return *(Vector*)&(rBody->GetVelocity());	}
	virtual Vector GetLocation()	{ return *(Vector*)&rBody->GetPos();}
	virtual Matrix GetRotation()	{neT3 trans; trans.rot = rBody->GetRotationM3(); return *(Matrix*)&NET3_TO_D3DXMATRIX(trans);}


	/// Add a little force with translate
	virtual void AddForce(Vector force) 
	{
		neT3 body2World = rBody->GetTransform();
		neV3 tempforce=body2World.rot * ((*(neV3*)&force));
		rBody->SetForce(tempforce, rBody->GetPos());
	}

	/// Add a little torque with translate
	virtual void AddTorque(Vector torque) 
	{
		neT3 body2World = rBody->GetTransform();
		neV3 temptorque=body2World.rot * ((*(neV3*)&torque));
		rBody->SetForce(temptorque, rBody->GetPos());
	}

	/// generic center impulse
	virtual void AddImpulse(Vector impulse) 
	{
		impulse *= impulseCoeff;
		rBody->ApplyImpulse(*(neV3*)&impulse, rBody->GetPos()+*(neV3*)&(Vector(0,0.4f,0)));
	}

	/// impulse at point, using world space not local
	virtual void AddImpulseContactPoint(Vector impulse, Vector point) {
		impulse *= impulseCoeff;
		rBody->ApplyImpulse(*(neV3*)&impulse, *(neV3*)&point );	}

	virtual void Gravity(bool enableb)	{ rBody->GravityEnable(enableb);	}

	virtual void UpdateBoundingInfo() { rBody->UpdateBoundingInfo(); };

	neRigidBody *retRigidBody() { return rBody; };	
	TokamakRigidBody() : TokamakBody()
	{ rBody=NULL; };
	virtual void Destroy();
	void setDampings(float linear, float angular)
	{
		rBody->SetLinearDamping(linear);
		rBody->SetAngularDamping(angular);
	}
	void SetSleepingParameter(float sleepParam)
	{
		rBody->SetSleepingParameter(sleepParam);
	}
	virtual Vector GetBBoxSize()
	{
	if(m_MainGeometry)
	{
		neV3 boxSize;
		m_MainGeometry->GetBoxSize(boxSize);
		return *(Vector*)&boxSize;
	}
	return Vector();
	}
};

/// Allows creation of some common rigid body types, including boxes, spheres, cylinders, and multi-primitive rigid bodies
class ENGINE_API CustomRigidBody : public TokamakRigidBody
{
public:
	/// Box rigid body
	void Create(Actor* parent, Vector& BoxSize, float Mass, int material);
	/// Sphere rigid body
	void Create(Actor* parent, float SphereDiameter, float Mass, int material);
	/// Cylinder rigid body
	void Create(Actor* parent, float CylinderHeight, float CylinderDiameter, float Mass, int material);

	/// Multi-primitive core rigid body
	void Create(Actor* parent, Vector& BoxSize, float Mass, int material, bool hasMultiPrimitives);
	/// Multi-primitive box primitive to add onto core
	void AddBoxGeometry(Vector& BoxSize,Matrix& transform, int material);
};

#endif
