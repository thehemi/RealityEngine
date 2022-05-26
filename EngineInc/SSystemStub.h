//=========== (C) Copyright 2004, Artficial Studios. All rights reserved. =======
/// C# Scripting System
/// Author: Mostafa Mohamed & Jeremy Stieglitz
//
//
//===============================================================================
#pragma once

typedef void ( *TSSystem_ScriptsTickAll)(void* world);
typedef void ( *TSSystem_ActorWrap)(void* actor);
typedef void ( *TSSystem_ActorDelete)(void * actor);
typedef void ( *TSSystem_ActorSerialize)(int index,void* XMLsystem,void* node);
typedef void ( *TSSystem_ActorDeserialize)(int index,char* data,const char* paramName,const char* type);
typedef void ( *TSSystem_ActorsPostRender)(void* world, void* camera);
typedef void ( *TSSystem_ActorsPreRender)(void* world, void* camera);
typedef void ( *TSSystem_ActorsOnRender)(void* world, void* camera);
typedef void ( *TSSystem_ActorTouched)(void* actor,void* other, void* info);

typedef void (*TSSystem_RigidBodyCollide)(int index,void * collisionInfo);

typedef void  ( *TSSystem_ActorClient_HandleTickMessage)(int index, unsigned char message,void * packetBuffer);
typedef void  ( *TSSystem_ActorClient_HandleSpawnMessage)(int index, unsigned char  message,void * packetBuffer);
typedef void  ( *TSSystem_ActorServer_MakeTickMessages)(int index, void * packet);
typedef void  ( *TSSystem_ActorServer_MakeSpawnMessages)(int index);
typedef void ( *TSSystem_ActorServer_MakeOnJoinSynchMessages)(int index, void * client,void ** result);
typedef void  ( *TSSystem_ActorServer_HandleNetworkKeyInput)(int index, bool isDown, int NetworkKeyHandle);
typedef void  ( *TSSystem_ActorServer_HandleNetworkMouseUpdate)(int index, float mouseYaw, float mousePitch);

//SCRIPTINGENGINE FUNCTIONS
typedef void ( *TSSystem_CreateDomain)();
typedef void ( *TSSystem_DestroyDomain)();
typedef void  ( *TSSystem_CreateActor)(const char * fileName,void* world,void** actor);

extern TSSystem_ScriptsTickAll	SSystem_ScriptsTickAll;
extern TSSystem_ActorWrap	    SSystem_ActorWrap;
extern TSSystem_ActorDelete		SSystem_ActorDelete;
extern TSSystem_ActorSerialize  SSystem_ActorSerialize;
extern TSSystem_ActorDeserialize  SSystem_ActorDeserialize;
extern TSSystem_ActorsPostRender SSystem_ActorsPostRender;
extern TSSystem_ActorsPreRender SSystem_ActorsPreRender;
extern TSSystem_ActorsOnRender SSystem_ActorsOnRender;
extern TSSystem_ActorTouched SSystem_ActorTouched;
extern TSSystem_ActorClient_HandleTickMessage SSystem_ActorClient_HandleTickMessage;
extern TSSystem_ActorClient_HandleSpawnMessage SSystem_ActorClient_HandleSpawnMessage;
extern TSSystem_ActorServer_MakeTickMessages SSystem_ActorServer_MakeTickMessages;
extern TSSystem_ActorServer_MakeSpawnMessages SSystem_ActorServer_MakeSpawnMessages;
extern TSSystem_ActorServer_MakeOnJoinSynchMessages SSystem_ActorServer_MakeOnJoinSynchMessages;
extern TSSystem_ActorServer_HandleNetworkKeyInput SSystem_ActorServer_HandleNetworkKeyInput;
extern TSSystem_ActorServer_HandleNetworkMouseUpdate SSystem_ActorServer_HandleNetworkMouseUpdate;

extern TSSystem_RigidBodyCollide SSystem_RigidBodyCollide;

extern TSSystem_CreateDomain    SSystem_CreateDomain;    
extern TSSystem_DestroyDomain   SSystem_DestroyDomain;   
extern TSSystem_CreateActor      SSystem_CreateActor;

/// Stub call
void FreeLibraries();
bool LoadLibraries();
bool LibsInitialized();
