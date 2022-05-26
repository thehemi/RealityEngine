//=========== (C) Copyright 2004, Artficial Studios. All rights reserved. =======
// C# Scripting System
// Author: Mostafa Mohamed & Jeremy Stieglitz
//
// This file contains the Managed C++ System that interfaces between C++ and C#
// Engine entrypoints to C# calls are here, as is the C# Compiler and
// Actor management engine
//
// The support files such as Wrappers.h contain the definitions that C#
// have access to. If there is a C++ function C# code needs, that is where to add it.
//
//
//===============================================================================
#pragma once
#include "..\Shared\Matrix.h"
#ifdef SCRIPTINGSYSTEM_EXPORTS
#define SCRIPTINGSYSTEM_API __declspec(dllexport) _cdecl 
#else
#define SCRIPTINGSYSTEM_API __declspec(dllimport) _cdecl 
#endif


extern "C" {

//ACTOR FUNCTIONS
/// Called after the process of rendering all Actors, so the game logic can choose to custom handle this event (generally by drawing any post FX)
void SCRIPTINGSYSTEM_API SSystemActorsPostRender(void* world, void* camera);
/// Called right before all Actors are rendered (but after Tick), so the game logic can choose to custom handle this event (generally by calling a function on each MActor in a World)
void SCRIPTINGSYSTEM_API SSystemActorsPreRender(void* world, void* camera);
/// Called during the process of rendering all Actors, so the game logic can choose to custom handle this event (generally by calling a function on each MActor in a World)
void SCRIPTINGSYSTEM_API SSystemActorsOnRender(void* world, void* camera);
/// Called when an MActor touches another Actor or the World, or is touched by another Actor, for custom response to this event.
/// Fills out pertinent information to a MCollisionInfo object the MActor can use.
void SCRIPTINGSYSTEM_API SSystemActorTouched(void* actor,void* other, void* info);
/// Called when the specified World is Ticked, allowing the game logic to Tick all MActors in the World as it chooses, or otherwise update the scripted game
void SCRIPTINGSYSTEM_API SSystemScriptsTickAll(void* world);
/// Wraps various instantiated C++ Actor Objects to their managed representations
void SCRIPTINGSYSTEM_API SSystemActorWrap(void* actor);
/// Disposes the managed representation of an Actor (if any), generally when that Actor's destructor is being called
void SCRIPTINGSYSTEM_API SSystemActorDelete(void * actor);
/// Serializes a Managed-Actor, namely the MActor will write its browsable Properties to the XML file
void SCRIPTINGSYSTEM_API SSystemActorSerialize(int index,void* XMLsystem,void* node);
/// Deserializes one property, specified by paramName, of a Managed-Actor from the data of a Property XML element contained in its node
void SCRIPTINGSYSTEM_API SSystemActorDeserialize(int index,char* data,const char* paramName,const char* type);

//NETWORKACTOR FUNCTIONS
/// Client-side, processes a single Tick update message associated with a NetworkActor, passing it onto the appropriate MNetworkActor
void SCRIPTINGSYSTEM_API SSystemActorClient_HandleTickMessage(int index, unsigned char message,void * packetBuffer);
/// Client-side, processes a single Spawn update message associated with a NetworkActor, passing it onto the appropriate MNetworkActor
void SCRIPTINGSYSTEM_API SSystemActorClient_HandleSpawnMessage(int index, unsigned char  message,void * packetBuffer);
/// Server-side, creates all Tick (update) messages for sending ti a particular NetworkClient's packets when the NetworkActor is due to send to that NetworkClient.
void SCRIPTINGSYSTEM_API SSystemActorServer_MakeTickMessages(int index, void * packet);
/// Server-side, creates all Spawn messages for sending to everyone when the NetworkActor is first created
void SCRIPTINGSYSTEM_API SSystemActorServer_MakeSpawnMessages(int index);
/// Server-side, creates all On-Join state synchronization messages for sending to a particular NetworkClient who has joined a game in progress
void SCRIPTINGSYSTEM_API SSystemActorServer_MakeOnJoinSynchMessages(int index, void * client, void ** result);
/// Server-side updating of input and mouse due to received network input for a NetworkClient to whom the indexed Actor is an avatar
void SCRIPTINGSYSTEM_API SSystemActorServer_HandleNetworkKeyInput(int index, bool isDown, int NetworkKeyHandle);
/// Server-side updating of input and mouse due to received network input for a NetworkClient to whom the indexed Actor is an avatar
void SCRIPTINGSYSTEM_API SSystemActorServer_HandleNetworkMouseUpdate(int index, float mouseYaw, float mousePitch);

//SCRIPTINGENGINE FUNCTIONS
/// Initializes the ScriptingSystem, its assembly application domain, sets Reality Engine's ScriptingSystem callbacks, and compiles all uncompiled or updated scripts.
/// Called upon app init
void SCRIPTINGSYSTEM_API SSystemCreateDomain();

/// Destroy's the ScriptingSystem's application domain, which causes the assembly to be unloaded and any remaining managed objects to be freed.
/// Called upon app shutdown
void SCRIPTINGSYSTEM_API SSystemDestroyDomain();

/// Creates an Managed-Actor from its classname, adds it to the specified World (wrapping the world if not already done)
/// and sets a passed Actor pointer to the Managed-Actor's Actor representation
void SCRIPTINGSYSTEM_API SSystemCreateActor(const char* fileName,void* world, void** actor);

//PHYSICS FUNCTIONS
/// Called when a collision is determined to involve managed Rigid Bodies, 
/// allows the collision event to be passed through to a C# rigid body
/// Index being the RigidBodyIdentifier hash index, collisionInfo being an neCollisionInfo containing useful information about the collision
void SCRIPTINGSYSTEM_API SSystemRigidBodyCollide(int index,void * collisionInfo);
}
