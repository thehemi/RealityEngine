//=========== (C) Copyright 2004, Artficial Studios. All rights reserved. =======
// C# Scripting System
// Author: Mostafa Mohamed
//
//
//===============================================================================
#include "stdafx.h"
#define _WIN32_WINNT 0x0501
#include <windows.h>
#include <string>
using namespace std;
#include "SSystemStub.h"

TSSystem_ScriptsTickAll                         SSystem_ScriptsTickAll;
TSSystem_ActorWrap                              SSystem_ActorWrap;
TSSystem_ActorDelete                            SSystem_ActorDelete;
TSSystem_ActorSerialize                         SSystem_ActorSerialize;
TSSystem_ActorDeserialize                       SSystem_ActorDeserialize;
TSSystem_ActorsPostRender                       SSystem_ActorsPostRender;
TSSystem_ActorsOnRender                         SSystem_ActorsOnRender;
TSSystem_ActorsPreRender                        SSystem_ActorsPreRender;
TSSystem_ActorTouched                           SSystem_ActorTouched;
TSSystem_ActorClient_HandleTickMessage          SSystem_ActorClient_HandleTickMessage;
TSSystem_ActorClient_HandleSpawnMessage         SSystem_ActorClient_HandleSpawnMessage;
TSSystem_ActorServer_MakeTickMessages           SSystem_ActorServer_MakeTickMessages;
TSSystem_ActorServer_MakeSpawnMessages          SSystem_ActorServer_MakeSpawnMessages;
TSSystem_ActorServer_MakeOnJoinSynchMessages    SSystem_ActorServer_MakeOnJoinSynchMessages;
TSSystem_ActorServer_HandleNetworkKeyInput		SSystem_ActorServer_HandleNetworkKeyInput;
TSSystem_ActorServer_HandleNetworkMouseUpdate	SSystem_ActorServer_HandleNetworkMouseUpdate;

TSSystem_RigidBodyCollide SSystem_RigidBodyCollide;

TSSystem_CreateDomain    SSystem_CreateDomain;    
TSSystem_DestroyDomain   SSystem_DestroyDomain;   
TSSystem_CreateActor     SSystem_CreateActor;

HINSTANCE m_hHandle;
bool bInitialized = false;
bool LibsInitialized()
{
	return bInitialized;
}

bool LoadLibraries()
{	
#ifdef _DEBUG
	TCHAR* ScriptingSystem = "ScriptingSystemD.dll";
#else
	TCHAR* ScriptingSystem = "ScriptingSystem.dll";
#endif
     
	// Load DLL dynamically
 	m_hHandle = ::LoadLibrary(ScriptingSystem);
	if(!m_hHandle){
		m_hHandle = ::LoadLibrary(ScriptingSystem);
		if(!m_hHandle){
#ifdef _DEBUG
			MessageBoxA(0,"Could not find or load ScriptingSystemD.DLL (Debug).\nPlease ensure the file is present and you have the Microsoft .NET 2.0 FrameWork installed.\nA link for this is available in the technical section of http://reality.artificialstudios.com",0,0);
#else
			MessageBoxA(0,"Could not find ScriptingSystem.DLL\n\nPlease ensure the file is present and you have the Microsoft .NET 2.0 FrameWork installed.\nA link for this is available in the technical section of http://reality.artificialstudios.com",0,0);
#endif
			return false;
		}
	}

	// Load dynamically linked routines
    SSystem_ScriptsTickAll	= (TSSystem_ScriptsTickAll)::GetProcAddress(m_hHandle,("SSystemScriptsTickAll"));
    SSystem_ActorWrap		= (TSSystem_ActorWrap)::GetProcAddress(m_hHandle,("SSystemActorWrap"));
    SSystem_ActorDelete		= (TSSystem_ActorDelete)::GetProcAddress(m_hHandle,("SSystemActorDelete"));
    SSystem_ActorSerialize	= (TSSystem_ActorSerialize)::GetProcAddress(m_hHandle,("SSystemActorSerialize"));
    SSystem_ActorDeserialize= (TSSystem_ActorDeserialize)::GetProcAddress(m_hHandle,("SSystemActorDeserialize"));
    SSystem_ActorsPostRender= (TSSystem_ActorsPostRender)::GetProcAddress(m_hHandle,("SSystemActorsPostRender"));
	SSystem_ActorsPreRender= (TSSystem_ActorsPostRender)::GetProcAddress(m_hHandle,("SSystemActorsPreRender"));
	SSystem_ActorsOnRender= (TSSystem_ActorsPostRender)::GetProcAddress(m_hHandle,("SSystemActorsOnRender"));
	SSystem_ActorTouched = (TSSystem_ActorTouched)::GetProcAddress(m_hHandle,("SSystemActorTouched"));
    SSystem_ActorClient_HandleTickMessage = (TSSystem_ActorClient_HandleTickMessage)::GetProcAddress(m_hHandle,("SSystemActorClient_HandleTickMessage"));
    SSystem_ActorClient_HandleSpawnMessage = (TSSystem_ActorClient_HandleSpawnMessage)::GetProcAddress(m_hHandle,("SSystemActorClient_HandleSpawnMessage"));
    SSystem_ActorServer_MakeTickMessages = (TSSystem_ActorServer_MakeTickMessages)::GetProcAddress(m_hHandle,("SSystemActorServer_MakeTickMessages"));
    SSystem_ActorServer_MakeSpawnMessages = (TSSystem_ActorServer_MakeSpawnMessages)::GetProcAddress(m_hHandle,("SSystemActorServer_MakeSpawnMessages"));
    SSystem_ActorServer_MakeOnJoinSynchMessages = (TSSystem_ActorServer_MakeOnJoinSynchMessages)::GetProcAddress(m_hHandle,("SSystemActorServer_MakeOnJoinSynchMessages"));
	
	SSystem_ActorServer_HandleNetworkKeyInput = (TSSystem_ActorServer_HandleNetworkKeyInput)::GetProcAddress(m_hHandle,("SSystemActorServer_HandleNetworkKeyInput"));
	SSystem_ActorServer_HandleNetworkMouseUpdate = (TSSystem_ActorServer_HandleNetworkMouseUpdate)::GetProcAddress(m_hHandle,("SSystemActorServer_HandleNetworkMouseUpdate"));

	SSystem_RigidBodyCollide = (TSSystem_RigidBodyCollide)::GetProcAddress(m_hHandle,("SSystemRigidBodyCollide"));

    SSystem_CreateDomain	= (TSSystem_CreateDomain)::GetProcAddress(m_hHandle,("SSystemCreateDomain"));
    SSystem_DestroyDomain	= (TSSystem_DestroyDomain)::GetProcAddress(m_hHandle,("SSystemDestroyDomain"));
    SSystem_CreateActor		= (TSSystem_CreateActor)::GetProcAddress(m_hHandle,("SSystemCreateActor"));

	if(!SSystem_ScriptsTickAll || !SSystem_ActorWrap || !SSystem_ActorDelete
       || !SSystem_CreateDomain || !SSystem_DestroyDomain || !SSystem_CreateActor
       || !SSystem_ActorSerialize || !SSystem_ActorDeserialize || !SSystem_ActorsPostRender
       || !SSystem_ActorClient_HandleTickMessage || !SSystem_ActorClient_HandleSpawnMessage 
       || !SSystem_ActorServer_MakeTickMessages  || !SSystem_ActorServer_MakeSpawnMessages
	    || !SSystem_ActorServer_MakeOnJoinSynchMessages 
	    || !SSystem_ActorServer_HandleNetworkKeyInput 
		|| !SSystem_ActorServer_HandleNetworkMouseUpdate || !SSystem_ActorTouched
		|| !SSystem_ActorsPreRender || !SSystem_ActorsOnRender
        || !SSystem_RigidBodyCollide){
		MessageBoxA(0,"Entrypoint not initialized. Your ScriptinSystem.DLL may be outdated or invalid",0,0);
		return false;
	}

	bInitialized = true;
	return true;
}

void FreeLibraries()
{
    if (bInitialized)
      if (FreeLibrary(m_hHandle))
        bInitialized=false;
}





















