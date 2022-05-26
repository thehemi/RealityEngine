//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
//
// Author: Tim Johnson
//====================================================================================
#include "stdafx.h"
#include "ispatialpartition.h"
#include "SSystemStub.h"
#include "Editor.h"
#include "IndoorVolume.h"
#include "ShadowMapping.h"


Actor::Callback_Landed Actor::callback_Landed = NULL;
Actor::Callback_Clone Actor::callback_Clone = NULL;
Actor::Callback_SetWorld Actor::m_Callback_SetWorld = NULL;
Actor::Callback_DeserializationComplete Actor::m_Callback_DeserializationComplete = NULL;

float IndoorOutdoorRefreshTime = .5f;

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
Prefab::Prefab(World* WorldToBeIn) : Actor(WorldToBeIn){
    IsPrefab = true;
    StaticObject = true;
    m_AllowIncludeExclude	= true;
    CollisionFlags = CF_MESH;
}

//-----------------------------------------------------------------------------
//
// Simplified Tick for Prefabs
//
//-----------------------------------------------------------------------------
void Prefab::Tick()
{
    if(/*Editor::Instance()->GetEditorMode() &&*/ MyModel)
    {
        MyModel->m_VisibleDistanceRatio = VisibleDistanceRatio*MyWorld->m_fLODCullBias;
        MyModel->m_LODRatio				= LODRatio*MyWorld->m_fLODCullBias;
    }

    // Create handle on first tick
    if(m_Handle == -1 && MyModel)
    {
        BBox box = MyModel->GetWorldBBox();
        m_Handle = SpatialPartition()->CreateHandle((IHandleEntity*)this,IsPrefab?PARTITION_ENGINE_PREFABS:PARTITION_ENGINE_ACTORS,box.min,box.max);
    }

    // Can only move prefabs if selected in editor mode
    if(IsSelected && m_Handle != -1)
    {
        BBox box = MyModel->GetWorldBBox();
        SpatialPartition()->ElementMoved(m_Handle,box.min,box.max);
    }
}

//-----------------------------------------------------------------------------
// ctor
//-----------------------------------------------------------------------------
Actor::Actor(World* world){
    Initialize(world,false);
}

//-----------------------------------------------------------------------------
// ctor for Script Actors that doesn't add the base class to managed table, to avoid double-managing them
//-----------------------------------------------------------------------------
Actor::Actor(World *world, bool IsScriptActor)
{
    Initialize(world,IsScriptActor);
}


//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void Actor::Initialize(World* world, bool scriptActor)
{	
    if(Editor::Instance()->GetEditorMode())
    {
        // Editor-spawned actors get GUIDs
        CoCreateGuid(&m_GUID);
    }
    else
    {
        ZeroMemory(&m_GUID,sizeof(GUID));
    }

    // set it first to false here so that SetWorld called by AddActor doesn't try to trigger the scriptingsystem callback too early
    m_isScriptActor = false;

    MyWorld = world;

    if(MyWorld)
        MyWorld->AddActor(this);

    CustomFlags = 0;
    IsSelected = false;

    //Mostafa: Scripting 
    m_managedIndex = -1;
    m_isScriptActor=scriptActor;

    m_HasIncludeExcludeList = false;
    m_IsExcludeList = true;
    bExportable = false;
    m_SpawnByName = true;
    m_Handle		= -1;

    m_ActorID		= ACTOR_ID_COUNT++;

    StaticObject	= false;
    GhostObject		= false;
    Velocity		= Vector(0,0,0);
    Location		= Vector(0,0,0);
    GroundNormal	= Vector(0,0,0);
    GroundMat		= 0;
    CurrentState	= 0;

    m_ForceOcclusionTest = false;

    IgnoreUserCollisionFlags = 0;
    UserCollisionFlag = 0;

    PhysicsFlags	= PHYS_PUSHABLE | PHYS_ONBLOCK_CLIMBORSLIDE;
    CollisionFlags	= CF_BBOX;
    Mass = 1.f;

    LifeTime		= -1;
    BounceFactor	= 0.5f;
    IsPrefab		= false;
    IsHidden		= false;
    IsFrozen		= false;
    Friction		= 1;
    IsHidden		= false;
    MyModel			= 0;
    m_CollisionModel = &MyModel;
    Parent			= 0;
    m_TokamakHandle	= 0;
    Inside = true;
    Outside = true;
    m_AllowIncludeExclude = false;
    m_CollidesRays = true;
    m_AutoUpdateModelTransformation = true;
    m_UpdateIndoorOutdoorState = false;
    m_IndoorOutdoorStateUpdateRate = 1;
    m_IndoorOutdoorStateUpdateTime = -BIG_NUMBER;
    m_AllowIncludeExcludeLight = NULL;

    // The model's bbox (if actor has a model) will be used 
    // unless these default values are overriden
    CollisionBox.min = Vector(0,0,0);
    CollisionBox.max = Vector(0,0,0);
    OldCollisionBox	 = CollisionBox;

    VisibleDistanceRatio = 500;
    LODRatio = 500;
    m_CachedShadow = 0;
    m_ShadowType = None;
    IsOccluded = false;
    m_QueryFrame = 0;

    EditorVars.push_back(EditorVar("ShadowType",(int*)&m_ShadowType,"General","Shadow casting type. 0 = None, 1 = Blob, 2 = Drop, 3 = ShadowMap"));
    EditorVars.push_back(EditorVar("VisibleDistanceRatio",&VisibleDistanceRatio,"Prefab","Distance ratio to no longer render prefab. Multiplied by 'VisibleRange' to get distance in meters"));
    EditorVars.push_back(EditorVar("LODRatio",&LODRatio,"Prefab","For first LOD switching. Multiplied by config 'LODRange' to get distance in meters"));
    EditorVars.push_back(EditorVar(" Name",&m_Name,"General"));
    EditorVars.push_back(EditorVar("IsHidden",&IsHidden,"General","Is object visible"));
    EditorVars.push_back(EditorVar("GhostObject",&GhostObject,"General","Is object colliable"));
    EditorVars.push_back(EditorVar("Inside",&Inside,"General","Has inside flag for lighting, etc. Outside lights cast on outside meshes, and vice versa. An actor can be both inside/outside."));
    EditorVars.push_back(EditorVar("Outside",&Outside,"General","Has outside flag for lighting, etc. Outside lights cast on outside meshes, and vice versa.  An actor can be both inside/outside."));
	EditorVars.push_back(EditorVar("CollidesRays",&m_CollidesRays,"General","Whether this Actor collides standard raytests for things like gunshots passing through fences and water volumes."));
}


//-----------------------------------------------------------------------------
// Virtual clone, mainly used by editor. Doesn't copy per-actor data like handles
//-----------------------------------------------------------------------------
void Actor::Clone(Actor* dest, bool bDeep)
{
    if(MyModel)
    {
        dest->MyModel = new Model;
        MyModel->CreateNewInstance(dest->MyModel,bDeep);
    }

    // Generate new actor names for exportable items
    if(bExportable)
    {
        string newName = m_Name;
        do{
            newName = GenerateNewName(newName);
        }
        while(MyWorld->FindActor(newName));
        dest->m_Name = newName;
    }

    dest->Location		= Location;
    dest->Rotation		= Rotation;
    dest->bExportable	 = bExportable;
    dest->CollisionBox	 = CollisionBox;
    dest->CollisionFlags = CollisionFlags;
    dest->PhysicsFlags   = PhysicsFlags;
    dest->script		 = script;
    dest->GhostObject	 = GhostObject;
    dest->IsHidden		 = IsHidden;
    dest->Inside		 = Inside;
    dest->Outside        = Outside;
    dest->VisibleDistanceRatio = VisibleDistanceRatio;
    dest->LODRatio = LODRatio;
    dest->IsPrefab      = IsPrefab;

    if(IsScriptActor() && callback_Clone)
        callback_Clone(GetManagedIndex(),dest->GetManagedIndex());
}

//-----------------------------------------------------------------------------
// Pointer management for actors referencing other actors.
// C++ Sucks :P
//-----------------------------------------------------------------------------
void Actor::SafeSetPointer(Actor** dest, Actor** source){
    if(dest != source){ 
        // Untie old pointer
        if(*dest)(*dest)->DontSetNULLOnDeath(this,dest); 
        if(source){ // In case user flat-out passes NULL
            *dest = *source;
            // Tell this new actor pointer to set itself to NULL upon death
            if(*dest)(*dest)->SetNULLOnDeath(this,dest); 
        }
    }
}

//-----------------------------------------------------------------------------
// Remove the pointer from our array and the supporting actor's array
//-----------------------------------------------------------------------------
void Actor::DontSetNULLOnDeath(Actor* caller, Actor** ptr){
    for(int i=0;i<safePointers.size();i++){
        if(*safePointers[i] == ptr){
            // The caller is looking after our pointer, not ptr, so make sure it deletes it
            if(caller)
                caller->DontSetNULLOnDeath(NULL,(Actor**)safePointers[i]);

            // We're looking after ptr, so get rid of it
            delete safePointers[i];
            safePointers.erase(safePointers.begin() + i);
            break;
        }
    }
}

//-----------------------------------------------------------------------------
// localPtr = our pointer, should never be set
// *localPtr = Address of actor POINTER (the one we want to set to NULL)
// **localPtr = Actor address, ignore, we never touch this
//-----------------------------------------------------------------------------
void Actor::SetNULLOnDeath(Actor* caller, Actor** ptr){
    // We've gotta build a pointer on the heap, so that the vector doesn't alter it :(
    // Simply points to our Actor** object
    Actor*** localPtr = new Actor**;
    *localPtr = ptr;

    // Circular problem here, the pointer we're keeping safe might be destroyed before we are!
    // So we need the other actor to safe-guard our pointer
    if(caller != NULL)
        caller->SetNULLOnDeath(NULL,(Actor**)localPtr);

    safePointers.push_back(localPtr); 
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Actor::DelRef(Actor* ref){
    // Delete ref from refs
    std::vector<Actor*>::iterator it = std::find(refs.begin(),refs.end(),ref); 
    if(it!=refs.end())
        refs.erase(it);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Actor::AddRef(Actor* ref){ 
    refs.push_back(ref); 
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Actor::SetParent(Actor* parent){ 
    // Divorce current parent, if any
    if(Parent)
        Parent->DelRef(this);

    // Marry new parent
    Parent = parent;
    if(Parent)
        Parent->AddRef(this); 
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Actor::Touched(Actor* other, CollisionInfo* info)
{
    if(IsScriptActor())
        SSystem_ActorTouched(this,other,info);

    // notify dependents of this event
    for(int i=0;i<dependents.size();i++)
        dependents[i]->Touched(other,info);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Actor::UnTouched(Actor* other){
    // notify dependents of this event
    for(int i=0;i<dependents.size();i++)
        dependents[i]->UnTouched(other);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Actor::Killed(Actor* other){
    // notify dependents of this event
    for(int i=0;i<dependents.size();i++)
        dependents[i]->Killed(other);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Actor::Activated(Actor* other){
    // notify dependents of this event
    for(int i=0;i<dependents.size();i++)
        dependents[i]->Activated(other);
}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
Actor::~Actor()
{
    SAFE_DELETE(m_CachedShadow);

    //Mostafa: Scripting 
    if (m_managedIndex != -1 && LibsInitialized()) 
        SSystem_ActorDelete(this);

    // Remove actor from Tokamak
    PhysicsEngine::Instance()->DestroyHandle(m_TokamakHandle);

    // Custom safe pointers that point to us
    for(int i=0;i<safePointers.size();i++){
        if(*safePointers[i]) // Check that the ptr is non-NULL (meaning other actor didn't die and set it to NULL)
            **safePointers[i] = NULL;
        delete safePointers[i];
    }

    if(Parent)
        Parent->DelRef(this);

    // FIXME: REASSES NEED!!!
    // Free our dependents
    // NOTE: Do we really wanna delete them?
    for(int i = 0; i < dependents.size();i++)
        delete dependents[i];

    // Remove any dangling pointers to this actor (pointers from our touching system)
    for(int i=0;i<refs.size();i++){
        // Refs may be from touching actor list, so check that first
        vector_erase(refs[i]->touchingActors,this);

        // Refs may simply be from the dependents/parent pointer..
        if(refs[i]->Parent == this)
            refs[i]->Parent = NULL;
    }

    // Untouch ourselves from all the actors we're touching
    //
    // FIXME: This is bugged when destroying things while touching. Try closing straight after
    // spwaning while getting stuck in that infuriating dusb spawnpad
    for(int x=0;x<touchingActors.size();x++){
        if(touchingActors[x]){
            touchingActors[x]->UnTouched(this);

            // Some actors might have us on their ref list
            // These will all be actors we're touching. So let them know that we are dying
            touchingActors[x]->DelRef(this);
        }
    }


    if(MyWorld)
        MyWorld->RemoveActor(this);
    if(MyModel)
        delete MyModel;

    // Remove actor from tree
    if(m_Handle != -1)
        SpatialPartition()->DestroyHandle(m_Handle); 
}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
// FIXME: Doing SetTransform twice.
void Actor::Tick()
{
    if(!MyWorld)
        return;

    if(/*Editor::Instance()->GetEditorMode() &&*/ MyModel)
    {
        MyModel->m_VisibleDistanceRatio = VisibleDistanceRatio*MyWorld->m_fLODCullBias;
        MyModel->m_LODRatio				= LODRatio*MyWorld->m_fLODCullBias;
    }

    // Update the collision box from the model
    // Only update it if the actor hasn't overridden the model box with it's own custom size
    if(MyModel && m_AutoUpdateModelTransformation && CollisionBox == OldCollisionBox)
    {
        // Need to update model bwbox before calling GetBWBox
        MyModel->SetTransform(Rotation,Location);
        // Set the collision box to the model bbox
        // Must do this each frame to account for any rotation that might be applied
        BBox bw = MyModel->GetWorldBBox();
        CollisionBox.min = bw.min - Location;
        CollisionBox.max = bw.max - Location;
        OldCollisionBox = CollisionBox;
    }

    // Objects that have models OR are not ghosts get on the tree
    if(!GhostObject || MyModel)
    {
        BBox box(CollisionBox.min+Location,CollisionBox.max+Location);

        if(m_Handle == -1)
            m_Handle = SpatialPartition()->CreateHandle((IHandleEntity*)this,IsPrefab?PARTITION_ENGINE_PREFABS:PARTITION_ENGINE_ACTORS,box.min,box.max);

        SpatialPartition()->ElementMoved(m_Handle,box.min,box.max);
    }

    // This is here because Actors should have their physics updated before they run
    // their custom tick code, so that any physics-related activities they do, such as spawning
    // missiles, won't be lagged a tick.
    MyWorld->RunPhysics(this);

    if(m_UpdateIndoorOutdoorState)
    {
        if(m_IndoorOutdoorStateUpdateTime == -BIG_NUMBER)
        {
            static float cycle = 0;
            m_IndoorOutdoorStateUpdateTime = GSeconds + fmod(cycle,IndoorOutdoorRefreshTime);

            // Spread out updates one per frame
            if(m_IndoorOutdoorStateUpdateRate != 0)
                cycle += 2.0f*GDeltaTime;

            m_IndoorOutdoorStateUpdateLocation = Location;
            if(IndoorVolume::IndoorVolumes.size())
            {
                Inside = IndoorVolume::IsIndoors(MyWorld,Location);
                Outside = !Inside;
            }
        }
        else if(m_IndoorOutdoorStateUpdateRate != 0 && GSeconds - m_IndoorOutdoorStateUpdateTime > IndoorOutdoorRefreshTime/m_IndoorOutdoorStateUpdateRate && (Location - m_IndoorOutdoorStateUpdateLocation).Length() > .3f*GDeltaTime)
        {
            m_IndoorOutdoorStateUpdateTime = GSeconds;
            m_IndoorOutdoorStateUpdateLocation = Location;
            if(IndoorVolume::IndoorVolumes.size())
            {
                //Update Indoors/Outdoors states here
                Inside = IndoorVolume::IsIndoors(MyWorld,Location);
                Outside = !Inside;
            }
        }
    }
}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Actor::InitializeRigidBody()
{
    m_TokamakHandle = new CustomRigidBody;
    //default box size
    Vector BoxSize;
    BoxSize.Set(1,1,1);
    if(MyModel)
    {
        //if the actor has a model, get its model bbox size
        MyModel->SetTransform(Matrix());
        BoxSize = MyModel->GetWorldBBox().max*2;
    }
    ((CustomRigidBody*)m_TokamakHandle)->Create(this,BoxSize,Mass,MATERIAL_MEDIUM_BOUNCY);
    ((CustomRigidBody*)m_TokamakHandle)->SetLocation(Location);
    ((CustomRigidBody*)m_TokamakHandle)->SetRotation(Rotation);
    ((CustomRigidBody*)m_TokamakHandle)->UpdateBoundingInfo();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Actor::UpdateFromRigidBody()
{
    Location = (((TokamakBody*)m_TokamakHandle)->GetLocation());
    Rotation = (((TokamakBody*)m_TokamakHandle)->GetRotation());
    Rotation.m3.Set(0,0,0);
    Velocity = ((TokamakBody*)m_TokamakHandle)->GetVelocity();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Actor::SetRigidBodyTransform(Vector location, Matrix rotation, Vector velocity,Vector angMomentum)
{
    Location = location;
    Rotation = rotation;
    Velocity = velocity;
    if(m_TokamakHandle)
    {
        ((TokamakBody*)m_TokamakHandle)->SetLocation(Location);
        ((TokamakBody*)m_TokamakHandle)->SetRotation(Rotation);
        ((TokamakBody*)m_TokamakHandle)->SetVelocity(Velocity);
        ((TokamakBody*)m_TokamakHandle)->SetAngularMomentum(angMomentum);
        ((TokamakBody*)m_TokamakHandle)->UpdateBoundingInfo();
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
Model* Actor::GetCollisionModel()
{
    return *m_CollisionModel;
}

//-----------------------------------------------------------------------------
// Server-side updating of input and mouse due to received network input for a NetworkClient to whom this Actor is an avatar
//-----------------------------------------------------------------------------
void Actor::Server_HandleNetworkKeyInput(bool isDown, int NetworkKeyHandle)
{
    if(IsScriptActor())
        SSystem_ActorServer_HandleNetworkKeyInput(GetManagedIndex(),isDown,NetworkKeyHandle);
}

//-----------------------------------------------------------------------------
// Server-side updating of input and mouse due to received network input for a NetworkClient to whom this Actor is an avatar
//-----------------------------------------------------------------------------
void Actor::Server_HandleNetworkMouseUpdate(float mouseYaw, float mousePitch)
{
    if(IsScriptActor())
        SSystem_ActorServer_HandleNetworkMouseUpdate(GetManagedIndex(),mouseYaw,mousePitch);
}

//-----------------------------------------------------------------------------
/// sets the current World of the Actor, passes through to C# system as well.
//-----------------------------------------------------------------------------
void Actor::SetWorld(World* world)
{
    MyWorld = world;

    if(IsScriptActor() && m_Callback_SetWorld)
        m_Callback_SetWorld(m_managedIndex,world);
}

//-----------------------------------------------------------------------------
/// Allows grouping of DropShadows into the same Render Target for obvious overlapping shadows like a Player holding a Weapon. 
/// Any Actor on the partners list will be included in this Actor's DropShadow rendering.
//-----------------------------------------------------------------------------
void Actor::AddDropShadowPartner(Actor* actor)
{
	if(find(dropShadowPartners.begin(),dropShadowPartners.end(),actor) == dropShadowPartners.end() && actor->MyModel)
		dropShadowPartners.push_back(actor);
}
//-----------------------------------------------------------------------------
/// Removes an Actor from the DropShadow partners list, if it's on the list
//-----------------------------------------------------------------------------
void Actor::RemoveDropShadowPartner(Actor* partner)
{
	std::vector<Actor*>::iterator it = std::find(dropShadowPartners.begin(),dropShadowPartners.end(),partner); 
	if(it!=dropShadowPartners.end())
		dropShadowPartners.erase(it);
}