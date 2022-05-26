//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
/// Actor: The base class of all actors (Abstract)
///
/// Author: Tim Johnson
//====================================================================================
#pragma once

#include "Audio.h"
#include "Model.h"
#include "World.h"
#include "TokamakPhysics.h"



static unsigned int ACTOR_ID_COUNT=0;
/// Use to set class name, as below
#define CLASS_NAME(T) public: inline virtual const char* ClassName(){ return #T; }

/// \brief Actor is the base class of all gameplay objects.
//-----------------------------------------------------------------------------
/// A large number of properties, behaviors and interfaces are implemented in Actor, including:
/// -	Display 
/// -	Animation
/// -	Physics and world interaction
/// -	Making sounds
/// -	Networking properties X
/// -	Actor creation and destruction
/// -	Triggering and timers
//-----------------------------------------------------------------------------
class ENGINE_API Actor {
	friend class WorldPhysics;
	friend class World;
	friend class TokamakBody;
    friend class BatchRenderer;
    friend class ShadowEngine;
private:
	/// Custom flags. Other classes can use this as an efficient way to 
	/// store meta-data without the actor knowing
	DWORD	CustomFlags;

	void Initialize(World* world, bool scriptActor);
    Matrix PrevRotation;
	Vector PrevLocation;
	/// Pretend none of this private stuff exists
	vector<Actor***> safePointers;

	Actor* Parent;

	/// Internally used to check whether an actor has overriden the default model collision box
	BBox OldCollisionBox; 

	/// References for pointer-safety, any actors that reference us
	/// must have their pointers set to NULL if we die
	vector<Actor*> refs;
	void AddRef(Actor* ref);
	void DelRef(Actor* ref);

	/// Currently touching actors
	vector<Actor*> touchingActors;

	/// Drop-shadow partners sharing same RT
	vector<Actor*> dropShadowPartners;

	float m_IndoorOutdoorStateUpdateTime;
	Vector m_IndoorOutdoorStateUpdateLocation;

    struct CachedShadow* m_CachedShadow;

private:
	/// if true, then this is a C# script Actor, used to determine whether to call stub functions on this Actor
    bool m_isScriptActor;
	int  m_managedIndex;

protected:
	string m_managedClassName;

public:

    /// Shadow type for this actor
    enum ShadowType
    {
        None = 0, /// No shadows
        Blob = 1, /// Texture blobs
        Drop = 2, /// Render-to-texture projection
        ShadowMap = 3 /// True shadow map
    };

    /// Shadow type for this actor
    ShadowType    m_ShadowType;

    /// If actor doesn't have model, it can force an occlusion query using CollisionBox with this
    bool    m_ForceOcclusionTest;

    /// Globally unique identifier for actor
    GUID		m_GUID;
    /// Occlusion query
    OcclusionQuery m_query;
    /// Object currently occluded?
    bool IsOccluded;
    /// Frames since last query
    int m_QueryFrame;
	/// Visible distance ratio (how far before not drawn, as a ratio of user prefs)
	float VisibleDistanceRatio;
	/// How far before LODing, as a ratio of user prefs
	float LODRatio;

	/// C# script callback for Landed() Actor event
	typedef void (*Callback_Landed)(Actor* actor,Vector* hitVelocity);
	/// C# script callback for Landed() Actor event
	static Callback_Landed callback_Landed;
	/// C# script callback for Clone() Actor event
	typedef void (*Callback_Clone)(int indexSource,int indexDest);
	/// C# script callback for Clone() Actor event
	static Callback_Clone callback_Clone;

    /// C# script callback to set world
	typedef void (*Callback_SetWorld)(int managedIndex, World* world);
	/// sets the current World of the Actor in the C# system
    static Callback_SetWorld   m_Callback_SetWorld;

	typedef void (*Callback_DeserializationComplete)(int managedIndex);
	/// C# script callback after loading all this Actor's serialized variables during map load
    static Callback_DeserializationComplete   m_Callback_DeserializationComplete;

	/// Called after loading all this Actor's serialized variables during map load
	virtual void DeserializationComplete()
	{
		if(IsScriptActor() && m_Callback_DeserializationComplete)
			m_Callback_DeserializationComplete(m_managedIndex);
	}

	//// sets the current World of the Actor, passes through to C# system as well.
	void SetWorld(World* world);


	/// Actor Unique ID
	unsigned int		m_ActorID;

	/// exclude/` lists for actors like light,water etc...
	bool				m_IsExcludeList;	

	vector<string>		m_ExcludeList;	

    bool                m_HasIncludeExcludeList;

	/// Optimization flag. Can this actor be included and excluded by lights? By default set to true
	bool				m_AllowIncludeExclude; 

	/// Actor is inside actor
	bool				Inside;		
	/// Actor is outside actor
	bool				Outside;	
	/// Is actor a light of some sort?
	virtual bool			IsLight(){ return false; }

	/// Editor vars
	vector<EditorVar>	EditorVars;
	/// Selected in editor?
	bool				IsSelected;
	/// Saved into scene file on export?
	bool				bExportable;	
	/// Identifying tag only
	string				m_Name;		
	/// Actor script data
	ScriptData			script;
	/// Tokamak Handle
	TokamakBody*		m_TokamakHandle; 
	/// Spatial partition handle
	int					m_Handle;

    /// If a mesh tm was baked for optimization/rendering reasons, all the baking is held
    /// in this matrix. This allows us to reapply such transformations if a prefab is reloaded from file
    Matrix  BakedTM;

	/// Is this a Prefab Actor? NOTE: Don't ever set this
	bool IsPrefab; 

	/// Actors that want to be notified of events for this actor
	/// Base Events are: Killed, Activated, Touched, UnTouched
	vector<Actor*> dependents;

	/// Use this to keep your references safe
	void SetNULLOnDeath(Actor* caller, Actor** ptr);
	/// Use this to keep your references safe
	void DontSetNULLOnDeath(Actor* caller, Actor** ptr);
	/// Higher-level pointer set function, using both the above.
	/// It unties any old reference and sets the new one, and only if it's not already set
	void SafeSetPointer(Actor** dest, Actor** source);

	/// Ignore this specific actor in physics checks.
	/// The actor in this list will also ignore us (dual way)
	Actor* ignoreActor;

	/// Pointer-safe way to set parent. Parent will become NULL if parent dies
	void SetParent(Actor* parent);
	Actor* GetParent(){ return Parent; }

	/// User-definable collision flag
	DWORD UserCollisionFlag; 
	/// Actor will ignore other actors that have these flags set
	DWORD IgnoreUserCollisionFlags; 

	/// World actor belongs to
	World* MyWorld; 
	/// Actor's rendering/collision model
	Model* MyModel;
	/// The model pointer returned for mesh-based collision tests -- defaults to MyModel, but for instance could manually be set to a simplified collision hull or an oriented box.
	Model** m_CollisionModel;

	/// Doesn't interact with world and world doesn't interact with it. Velocity still works.
	bool GhostObject; 
	/// Doesn't have physics, a big optimization! Velocity still works, but be careful, or it'll get stuck!
	bool StaticObject;

	/// Collision volume type
	const static int CF_MESH = (1<<0); /// A Mesh. Perfectly accurate, but slower
	const static int CF_BBOX = (1<<1); /// A bounding box. Most common for player and entities
	const static int CF_PASSABLE_BBOX = (1<<2); /// A bbox which gets collision messages (touched() untouched()) but allows actors to pass through. Used for triggers, zones, projectiles
	const static int CF_CORPSE = (1<<3); /// Can be shot but not collided with

	const static int CF_WANT_WORLD_TOUCHED	= (1<<5); /// Wants Touched() to trigger on world (actor will be NULL)
	const static int CF_IGNORE_TOUCHED = (1<<6); //Ignore all Touched() checking to save a bit of speed on simple Actors
	/// Crappy hack flag to avoid slow actor-to-mesh-actor collisions
	const static int CF_IGNORE_MESH_USE_BBOX	= (1<<7);
	const static int CF_ALLOW_STUCK = (1<<8);
	/// mansion hack flag to improve tok collision sounds
	const static int CF_RECEIVE_INCOMING_ACTOR_TOUCH = (1<<9);


	DWORD CollisionFlags;

	/// Physics flags
	const static int PHYS_PUSHABLE	= (1<<0);
	const static int PHYS_NOT_AFFECTED_BY_GRAVITY = (1<<1);
	/// What to do when colliding (part of physics flags)
	const static int PHYS_ONBLOCK_STOP          =  (1<<5);  /// stop moving
	const static int PHYS_ONBLOCK_SLIDE         =  (1<<6);  /// slide along
	const static int PHYS_ONBLOCK_CLIMBORSLIDE  =  (1<<7);  /// clim up a stair or slide along
	const static int PHYS_ONBLOCK_BOUNCE        =  (1<<8);  /// bounce off
	const static int PHYS_ONBLOCK_PUSH          =  (1<<9);  /// push the obstacle
	const static int PHYS_RIGIDBODYDYANMICS	    =  (1<<10); /// Tokmak rigid body dynamics
	DWORD PhysicsFlags;
	float Mass;

	/// Current state. Physics system will set these flags:
	const static int STATE_ONGROUND	= (1<<0);
	DWORD CurrentState;
	/// Normal of floor actor is standing on (only used by PHYS_Walking)
	Vector GroundNormal; 
	/// Material actor is on. Used for footstep sounds etc
	Material* GroundMat; 

	/// Collision bounding box. Used if CF_BBOX. Defaults to model bbox.
	BBox CollisionBox; 

	// Move data
	/// 0..1 factor of bounce if( ONBLOCK_BOUNCE ) 1 = perpetual 0 = stops dead
	float BounceFactor; 
	/// This object's surface friction, 3 = normal 0 = perpetual sliding
	float Friction; 
	/// Model can't be seen
	bool  IsHidden; 
	/// Model UnSelectable in Editor
	bool  IsFrozen;
	/// In seconds, before actor is destroyed. -1 = never
	float LifeTime; 

	/// Actor's velocity in m/sec
	Vector Velocity;
    /// Actor location (will be applied to actor's model)
    Vector Location;
    /// Actor rotation (will be applied to actor's model)
	Matrix Rotation;
    /// Actor scaling (will be applied to actor's model)
    //Matrix Scaling;
	/// Type identification
	virtual bool IsNetworkActor(){ return false; }

	/// World interaction

	/// Custom events
	virtual void UserEvent(int event){}; 
	virtual void Touched(Actor* other, CollisionInfo* info = NULL);
	virtual void UnTouched(Actor* other);
	virtual void Falling(){};
	virtual void Landed(Vector& hitVelocity)
	{
		if(IsScriptActor() && callback_Landed)
			callback_Landed(this,&hitVelocity);
	}
	virtual void Tick();
	virtual void Killed(Actor* other);
	virtual void Activated(Actor* other);

	// Rendering
	virtual void PreRender(class Camera* canvas){}
	virtual void PostRender(class Camera* canvas){}
	virtual void OnRender(class Camera* cam){}
	/// Called after batches are ready, for drawing scene to rendertargets
	virtual void DrawRenderTarget(Camera* cam){}

	/// ctor
	Actor(World* world);

	/// alternate ctor used by Script Actors that doesn't add the base class to managed table, to avoid double-managing them
	Actor(World* world, bool IsScriptActor);

	virtual ~Actor();
	/// Virtual Clone() function for when cloning actors in editor. bDeep copies raw mesh
	virtual void Clone(Actor* destination, bool bDeep = true);

	// Rigid body physics
	virtual void InitializeRigidBody();
	virtual void UpdateFromRigidBody();
	virtual void SetRigidBodyTransform(Vector location, Matrix rotation, Vector velocity,Vector angMomentum);

	/// Alternate model for collision
	virtual Model* GetCollisionModel();
	/// Sets the collision Model used for other Actors colliding onto this Actor when CF_MESH is enabled. Defaults to MyModel, but for instance could manually be set to a simplified collision hull or an oriented box.
	virtual void SetCollisionModel(Model* collisionModel){m_CollisionModel = &collisionModel;}
	
	/// Server-side updating of input and mouse due to received network input for a NetworkClient to whom this Actor is an avatar
	virtual void Server_HandleNetworkKeyInput(bool isDown, int NetworkKeyHandle);
	virtual void Server_HandleNetworkMouseUpdate(float mouseYaw, float mousePitch);

	/// Allows grouping of DropShadows into the same Render Target for obvious overlapping shadows like a Player holding a Weapon. 
	/// Any Actor on the partners list will be included in this Actor's DropShadow rendering.
	void AddDropShadowPartner(Actor* actor);
	/// Removes an Actor from the DropShadow partners list, if it's on the list
	void RemoveDropShadowPartner(Actor* partner);

	/// C# Interfacing
	/// If true, then this Actor calls appropriate callbacks to spur logic to the managed system
	bool IsScriptActor()
	{
		return m_isScriptActor;
	}
	/// managed hash index for this Actor for fast look-up. -1 for non-managed Actors
	int GetManagedIndex()
	{
		return m_managedIndex;
	}
	/// Returns classname, with C# override
	inline virtual const char* ClassName()
	{
		if(!IsScriptActor()) 
			return "Actor"; 
		else  
			return m_managedClassName.c_str();
	}
	void SetManagedIndex(int managedIndex)
	{
		m_managedIndex = managedIndex;
	}
	void SetManagedClassName(string& managedClassName)
	{
		m_managedClassName = managedClassName;
	}

	/// If false, the Actor::Tick and World::Tick will not update the Model's transformation, effectively requiring that the derived Actor update the Model transformation manually (good for Models with large hierarchies like characters that may want to manually update the transformation)
	bool m_AutoUpdateModelTransformation;

	/// If true, the Actor will update its Inside and Outside variables every frame/rate based on its position in the World. If inside and IndoorVolume, it will be flagged as Inside=true,Outside=false for lighting and logic purposes.
	bool m_UpdateIndoorOutdoorState;

	/// If false, ray tests will ignore this Actor (except Reality Builder checks)
	bool m_CollidesRays;

	/// Determines the rate at which the Inside and Outside states are updated, if update is true. 1.0 is every frame.
	float m_IndoorOutdoorStateUpdateRate;

	/// For Actors that don't want general inclusions/exclusion list, they can specify a particular light to allow usage of its list on this
	class Light* m_AllowIncludeExcludeLight;

	/// if true, server sends unique GUID instead of classname to spawn-synch with Client, used for NetworkActor types that can be saved into the level
	bool m_SpawnByName;
};

//-----------------------------------------------------------------------------
/// Simple container for world prefabs (static meshes)
//-----------------------------------------------------------------------------
class Prefab : public Actor{
public:
	CLASS_NAME(Prefab);

	Prefab(World* WorldToBeIn);
	virtual void Tick();
}; 