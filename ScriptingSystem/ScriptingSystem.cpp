//=========== (C) Copyright 2004, Artficial Studios. All rights reserved. =======
// C# Scripting System
// Author: Mostafa Mohamed & Jeremy Stieglitz
//
// This file contains the Managed C++ System that interfaces between C++ and C#
// Reality Engine entrypoints to C# calls are here, as is the C# Compiler and
// Actor management engine
//
// The support files such as Wrappers.h contain the definitions that C#
// have access to. If there is a C++ function C# code needs, that is where to add it.
//
//===============================================================================

#include "stdafx.h"
#include "GuiSystem.h"
#include "Wrappers.h"
#include "NetworkActor.h"
#include "ScriptingEngine.h"
#include "ScriptingSystem.h"
#include "RigidBody.h"
#include "PhysicsEngine.h"
#include "Profiler.h"

using namespace System::IO;
/*using namespace System::Security;
using namespace System::Security::Permissions;
using namespace System::Security::Policy;*/
using namespace ScriptingSystem;
using namespace stdcli::language;

#undef CreateDirectory

#ifdef _DEBUG
#define CACHE_LOG  "\\cacheD.log"
#define SCRIPTS_DLL "\\ScriptsD.dll"
#else
#define SCRIPTS_DLL "\\Scripts.dll"
#define CACHE_LOG  "\\cache.log"
#endif

/// Forward declarations of Actor & Network callback functions to the managed interface
void SSystemNetworkClientJoined(NetworkClient* client);
void SSystemNetworkClientLeft(NetworkClient* client);
void SSystemConsoleCommand(NetworkClient* client, string command);
void SSystemActorLanded(Actor* actor, Vector* hitVelocity);
void SSystemActorDeserializationComplete(int index);

//--------------------------------------------------------------------------------------
/// Clone custom state upon an Editor clone operation
//--------------------------------------------------------------------------------------
void SSystemActorClone(int indexSource, int indexDest)
{
    MActor^ actorSource=(MActor^)MActor::s_actors[indexSource];
    MActor^ actorDest=(MActor^)MActor::s_actors[indexDest];

    if (actorSource != nullptr && actorDest != nullptr)
	{
        actorSource->Clone(actorDest);
		actorSource->DeserializationComplete();
	}
}

//--------------------------------------------------------------------------------------
/// Called when a collision is determined to involve managed Rigid Bodies, 
/// allows the collision event to be passed through to a C# rigid body
/// Index being the RigidBodyIdentifier hash index, collisionInfo being an neCollisionInfo containing useful information about the collision
//--------------------------------------------------------------------------------------
void SSystemRigidBodyCollide(int index,void * collisionInfo)
{
    RigidBody^ rigidBody=(RigidBody^)RigidBody::s_rigidBodies[index];
    if (rigidBody != nullptr)
        rigidBody->Collide((neCollisionInfo*)collisionInfo);
}

//--------------------------------------------------------------------------------------
//Load the cache file of previously compiled scripts, determining if any script source files have been updated and need to be recompiled
//--------------------------------------------------------------------------------------
void LoadCache()
{
    try
    {
        String^ cacheFile = String::Concat( ScriptingEngine::s_tempPath,CACHE_LOG);

        //Check if the file exists
        if (!File::Exists(cacheFile))
        {
            System::Diagnostics::Debug::WriteLine("SSystem:: Cache file is not found.");
            return;
        }

        //Clear the current cache items
        ScriptingEngine::s_cache->Clear();

        //Open and read the file
        System::IO::TextReader^ reader=File::OpenText(cacheFile);
        String^ source=reader->ReadToEnd();
        reader->Close();

        //Loop through the file items and add non modifed files to the cache table
        array<String^>^ lines=source->Split('\n');
        for (int i=0;i<lines->Length;i++)
        {
            array<String^>^ words=lines[i]->Split('|');
            if (words->Length>=2)
            {
				if(File::Exists(words[0]))
				{
                String^ ModificationTime = File::GetLastWriteTime(words[0]).ToString();
                if (words[1] == ModificationTime)
                {
                    ScriptingEngine::s_cache->Add(words[0],words[1]);
                    System::Diagnostics::Debug::WriteLine(String::Concat("SSystem:: CachedItem: ",words[0]," , ",words[1]));
                }
				}
            }
        }

    }
    catch (Exception^ ex)
    {
        System::Diagnostics::Debug::WriteLine("SSystem:: Exception while loading  the cache file.");
    }
}

// <summary>
/// Writes the list of compiled scripts with compilation timestamps for subsequent checking in LoadCache
// </summary>
void WriteToCache()
{
    try
    {
        System::IO::TextWriter^ writer=File::CreateText(String::Concat( ScriptingEngine::s_tempPath,CACHE_LOG));
        IEnumerator^ e= ScriptingEngine::s_cache->GetEnumerator();
        while (e->MoveNext())
        {
            DictionaryEntry^ de=((DictionaryEntry^)e->Current);
            writer->Write(String::Concat(de->Key->ToString(),"|",de->Value->ToString(),"\n"));
        }
        writer->Close();
    }
    catch (Exception^ ex)
    {
        System::Diagnostics::Debug::WriteLine("SSystem:: Exception while writing cache");
    }
}

// <summary>
/// Register new world with the C# system, creating a stored MWorld object for it
// </summary>
void SSystemRegisterWorld(World* world)
{
    MWorld^ mworld;

    bool found = false;
    IEnumerator^ e = MWorld::s_worlds->Values->GetEnumerator();

    while (e->MoveNext())
    {
        if( ((MWorld^)e->Current)->m_world == world )
        {
            mworld = (MWorld^)e->Current;
            found = true;
        }
    }

    if(!found)
        mworld = gcnew MWorld(world);

    if(ScriptingEngine::s_logicCore)
        ScriptingEngine::s_logicCore->LoadMap(mworld);
}

//--------------------------------------------------------------------------------------
// Passes off a network console command to the script game's logic object for processing, including the networkclient (user) who sent the command and the command string
//--------------------------------------------------------------------------------------
void SSystemConsoleCommand(NetworkClient* client, string command)
{
    if(ScriptingEngine::s_logicCore)
        ScriptingEngine::s_logicCore->ProcessConsoleCommand(gcnew MNetworkClient(client),Helpers::ToCLIString(command));
}

// FUNCTIONS FOR NETWORKACTOR WRAPPING
//--------------------------------------------------------------------------------------
/// Client-side, processes a single Tick update message associated with a NetworkActor, passing it onto the appropriate MNetworkActor
//--------------------------------------------------------------------------------------
void SSystemActorClient_HandleTickMessage(int index, unsigned char message,void * packetBuffer)
{
    MNetworkActor^ actor=(MNetworkActor^)MActor::s_actors[index];
    if (actor != nullptr)
        actor->Client_HandleTickMessage(message,gcnew MReadPacketBuffer((ReadPacketBuffer*)packetBuffer));
}
//--------------------------------------------------------------------------------------
/// Client-side, processes a single Spawn update message associated with a NetworkActor, passing it onto the appropriate MNetworkActor
//--------------------------------------------------------------------------------------
void SSystemActorClient_HandleSpawnMessage(int index, unsigned char  message,void * packetBuffer)
{
    MNetworkActor^ actor=(MNetworkActor^)MActor::s_actors[index];
    if (actor != nullptr)
        actor->Client_HandleSpawnMessage(message,gcnew MReadPacketBuffer((ReadPacketBuffer*)packetBuffer));
}
//--------------------------------------------------------------------------------------
/// Server-side, creates all Tick (update) messages for sending ti a particular NetworkClient's packets when the NetworkActor is due to send to that NetworkClient.
//--------------------------------------------------------------------------------------
void SSystemActorServer_MakeTickMessages(int index,void * packet)
{
    MNetworkActor^ actor=(MNetworkActor^)MActor::s_actors[index];
    if (actor != nullptr)
        actor->Server_MakeTickMessages(gcnew MNetworkActorPackets((NetworkActorPackets*)packet));
}
//--------------------------------------------------------------------------------------
/// Server-side, creates all Spawn messages for sending to everyone when the NetworkActor is first created
//--------------------------------------------------------------------------------------
void SSystemActorServer_MakeSpawnMessages(int index)
{
    MNetworkActor^ actor=(MNetworkActor^)MActor::s_actors[index];
    if (actor != nullptr)
        actor->Server_MakeSpawnMessages();
}
//--------------------------------------------------------------------------------------
/// Server-side, creates all On-Join state synchronization messages for sending to a particular NetworkClient who has joined a game in progress
//--------------------------------------------------------------------------------------
void SSystemActorServer_MakeOnJoinSynchMessages(int index,void * client, void ** result)
{
    *result = NULL;
    MNetworkActor^ actor=(MNetworkActor^)MActor::s_actors[index];
    if (actor != nullptr)
    {
        MNetworkActorPackets^ packet=actor->Server_MakeOnJoinSynchMessages(gcnew MNetworkClient((NetworkClient*)client));
        *result = (void*)packet->m_networkActorPackets;
    }
}

//--------------------------------------------------------------------------------------
/// Server-side updating of input and mouse due to received network input for a NetworkClient to whom the indexed Actor is an avatar
//--------------------------------------------------------------------------------------
void SSystemActorServer_HandleNetworkKeyInput(int index, bool isDown, int NetworkKeyHandle)
{
	MNetworkActor^ actor=(MNetworkActor^)MActor::s_actors[index];
	if (actor != nullptr)
		actor->Server_HandleNetworkKeyInput(isDown, NetworkKeyHandle);
}

//--------------------------------------------------------------------------------------
/// Server-side updating of input and mouse due to received network input for a NetworkClient to whom the indexed Actor is an avatar
//--------------------------------------------------------------------------------------
void SSystemActorServer_HandleNetworkMouseUpdate(int index, float mouseYaw, float mousePitch)
{
	MNetworkActor^ actor=(MNetworkActor^)MActor::s_actors[index];
	if (actor != nullptr)
		actor->Server_HandleNetworkMouseUpdate(mouseYaw, mousePitch);
}

//--------------------------------------------------------------------------------------
/// Server-side returns factor of network updating on a per-NetworkClient basis, usually calculated on the distance of that NetworkClient's avatar from the object in question
//--------------------------------------------------------------------------------------
float SSystemActorServer_AdaptiveDegration(int index,void* client)
{
    MNetworkActor^ actor=(MNetworkActor^)MActor::s_actors[index];
    if (actor != nullptr)
        return actor->Server_AdaptiveDegradation(gcnew MNetworkClient((NetworkClient*)client));

    return 1;
}
//--------------------------------------------------------------------------------------
/// Adds a collision impulse to an MActor, typically for custom passing on to that MActor's rigid body (if it has one)
//--------------------------------------------------------------------------------------
void SSystemActor_AddImpulse(int index,void* impulse)
{
    MActor^ actor=(MActor^)MActor::s_actors[index];
    if (actor != nullptr)
        return actor->AddImpulse(gcnew MVector(*(Vector*)impulse));
}
//--------------------------------------------------------------------------------------
/// Sets the managed MyWorld pointer of the MActor when its Actor's World changes, such as during a Reality Builder Merge Scene operation
//--------------------------------------------------------------------------------------
void SSystemActorSetWorld(int index, World* world)
{
	MWorld^ mworld = nullptr;
	
	if(world)
	{
    bool found = false;
    IEnumerator^ e = MWorld::s_worlds->Values->GetEnumerator();

    while (e->MoveNext())
    {
        if( ((MWorld^)e->Current)->m_world == world )
        {
            mworld = (MWorld^)e->Current;
            found = true;
        }
    }   
	
	if(!found)
        mworld = gcnew MWorld(world);
	}

	MActor^ mActor=(MActor^)MActor::s_actors[index];
        
	if (mActor != nullptr)
		mActor->SetWorld(mworld);
}

//--------------------------------------------------------------------------------------
/// Reloads all game preferences from the current INI values
//--------------------------------------------------------------------------------------
void SSystemReloadPreferences()
{
	ScriptingEngine::s_logicCore->ReloadPreferences();
}
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
/// Creates an arbritrary object from the ScriptingSystem's loaded assembly by its class name
//--------------------------------------------------------------------------------------
Object^ SSystemCreateObject(const char* className)
{
	try
    {
        Type^ type = ScriptingEngine::s_assembly->GetType(Helpers::ToCLIString(className));
        return Activator::CreateInstance(type,nullptr);
	}
    catch (Exception^ ex) 
    {
        Helpers::PrintError(ex,"Exception While loading a script:");
		return nullptr;
    }
}

//--------------------------------------------------------------------------------------
/// Initializes the ScriptingSystem, its assembly application domain, sets Reality Engine's ScriptingSystem callbacks, and compiles all uncompiled or updated scripts.
/// Called upon app init
//--------------------------------------------------------------------------------------
void SSystemCreateDomain()
{
    try
    {
		/*// Create a new, empty permission set so we don't mistakenly grant some permission we don't want
		PermissionSet^ pPermissions = gcnew PermissionSet(PermissionState::None);

		// Set the permissions that you will allow, in this case we only want to allow execution of code
		pPermissions->AddPermission(gcnew SecurityPermission(SecurityPermissionFlag::Execution | SecurityPermissionFlag::UnmanagedCode));
                                                                
		// Make sure we have the permissions currently
		pPermissions->Demand();

		// Create the security policy level for this application domain
		PolicyLevel^ pSecurityLevel = PolicyLevel::CreateAppDomainLevel();

		// Give the policy level's root code group a new policy statement based
		// on the new permission set.
		pSecurityLevel->RootCodeGroup->PolicyStatement = gcnew PolicyStatement(pPermissions);

		// Update the application domain's policy now
		AppDomain::CurrentDomain->SetAppDomainPolicy(pSecurityLevel);*/

        System::Diagnostics::Debug::WriteLine("SSystem:: Starting Up...");

        ScriptingEngine::s_initialized = false;
        ScriptingEngine::s_assembly = nullptr;

        //References Lookup directory
        AppDomain::CurrentDomain->ClearPrivatePath();
        AppDomain::CurrentDomain->AppendPrivatePath("System\\");

        String^ base = AppDomain::CurrentDomain::get()->BaseDirectory;
        ScriptingEngine::s_tempPath=String::Concat(base,"\\Temp");

        System::IO::Directory::CreateDirectory(ScriptingEngine::s_tempPath);

        //Load the cached files
        LoadCache();

        //Enumerate all the files in the scripts Game and Spawnable directory
        vector<string> files;
        enumerateFiles("..\\Scripts\\Spawnable",files,1,".cs");
        for(int i = 0; i < files.size(); i++)
        {
            files[i] = string("..\\Scripts\\Spawnable\\") + files[i];
        }
        int oldSize = files.size();
        enumerateFiles("..\\Scripts\\Game",files,1,".cs");
        for(int i = oldSize; i < files.size(); i++)
        {
            files[i] = string("..\\Scripts\\Game\\") + files[i];
        }

        //Check if the scripts.dll is already there
        bool needToRebuild = false;
        String^ tempDLL = String::Concat( ScriptingEngine::s_tempPath,SCRIPTS_DLL);
        array<String^>^ filesToCompile = gcnew array<String^>(files.size());

        for (int i=0;i < files.size();i++)
        {
            String^ fileName = Helpers::ToCLIString(files[i]);
            filesToCompile[i] = fileName;
            if (!ScriptingEngine::s_cache->Contains(filesToCompile[i]))
                needToRebuild=true;
        }

        //Rebuild the dll
        if (needToRebuild)
        {
            CSharpCodeProvider^ codeProvider = gcnew  CSharpCodeProvider();
            ICodeCompiler^ compiler = codeProvider->CreateCompiler();
            CompilerParameters^ parameters = gcnew CompilerParameters();

            //Setup compiler parameters
			parameters->CompilerOptions = "/d:TRACE";
#ifdef _DEBUG
            parameters->IncludeDebugInformation = true;
            parameters->ReferencedAssemblies->Add("ScriptingSystemD.dll");
#else
            parameters->IncludeDebugInformation = true;
            parameters->ReferencedAssemblies->Add("ScriptingSystem.dll");
			// use optimized compilation flag for fastest Release mode
			// parameters->CompilerOptions = String::Concat(parameters->CompilerOptions," /optimize+ /debug-");
#endif
            parameters->ReferencedAssemblies->Add("System.dll");
            parameters->ReferencedAssemblies->Add("System.Windows.Forms.dll");
            parameters->ReferencedAssemblies->Add("System.Drawing.dll");

            parameters->GenerateInMemory = false;
            parameters->OutputAssembly =  String::Concat( ScriptingEngine::s_tempPath,SCRIPTS_DLL);

            CompilerResults^ compilerResult;
            while(true)
            {
                //Compile Files
                compilerResult = compiler->CompileAssemblyFromFileBatch(parameters, filesToCompile);

                //Compile Errors
                if (compilerResult->Errors->HasErrors) 
                {
                    String^ errString = String::Concat("Compiler Errors:");
                    for (int i=0;i<compilerResult->Errors->Count;i++) 
                    {
                        // File.cs
                        errString = String::Concat(errString,"\n",compilerResult->Errors[i]->FileName);
                        // (120) : warning
                        errString = String::Concat(errString,"(",compilerResult->Errors[i]->Line,") : ",compilerResult->Errors[i]->IsWarning?"warning":"");
                        // C1440 : This is some error text
                        errString = String::Concat(errString,compilerResult->Errors[i]->ErrorNumber,": ",compilerResult->Errors[i]->ErrorText);
                    }

                    string error = Helpers::ToCppString(errString);
                    error += "\nClick 'Retry' to recompile again, or 'Cancel' to abort";

                    if(IDCANCEL == MessageBox(Engine::Instance()->hWnd,error.c_str(),"Error Compiling Scripts",MB_RETRYCANCEL))
                    {
                        return;
                    }
                    else // User wants to try again
                        continue;
                }
                else
                    break; // All went ok
            }

            ScriptingEngine::s_assembly = compilerResult->CompiledAssembly;

            //Regenerate cache
            ScriptingEngine::s_cache->Clear();
            for (int i=0 ; i < filesToCompile->Length ; i++)
            {
                ScriptingEngine::s_loadedFiles->Add(Path::GetFileNameWithoutExtension(filesToCompile[i]));
                String^ modificationTime = File::GetLastWriteTime(filesToCompile[i]).ToString();
                ScriptingEngine::s_cache->Add(filesToCompile[i],modificationTime);
            }

            //Write the cache file
            WriteToCache();
        }

        Server::Instance()->m_ServerCallBack_CreateNetworkClientPlayer = SSystemNetworkClientJoined;
		Server::Instance()->m_ServerCallBack_DeleteNetworkClientPlayer = SSystemNetworkClientLeft;
        Server::Instance()->m_ServerCallBack_NetworkConsoleCommand = SSystemConsoleCommand;

        World::m_RegisterWorld = SSystemRegisterWorld;
        World::m_AddTouchedImpulse = SSystemActor_AddImpulse;
        Actor::callback_Landed = SSystemActorLanded;
        Actor::callback_Clone = SSystemActorClone;
		Actor::m_Callback_SetWorld = SSystemActorSetWorld;
		Actor::m_Callback_DeserializationComplete = SSystemActorDeserializationComplete;
        NetworkActor::callback_AdaptiveDegradation = SSystemActorServer_AdaptiveDegration;
		//Game::Instance()->m_MainMenu.m_ReloadPreferences = SSystemReloadPreferences;

        if (ScriptingEngine::s_assembly == nullptr)
            ScriptingEngine::s_assembly = Assembly::LoadFrom(String::Concat(ScriptingEngine::s_tempPath,SCRIPTS_DLL));

        if (ScriptingEngine::s_assembly == nullptr)
        {
			Error("ScriptingSystem: Assembly can't be loadded");
            return;
        }

        //register all the MACtors with the classmap's Actor classname hash for hash-based spawning over the network
        IEnumerator^ e = ScriptingEngine::s_assembly->GetTypes()->GetEnumerator();
        while (e->MoveNext())
        {
            if(((Type^)e->Current)->IsSubclassOf(typeid<MActor>))
                Factory::RegisterHash(Helpers::ToCppString(((Type^)e->Current)->FullName));
        }

        ScriptingEngine::s_logicCore = (LogicCore^)SSystemCreateObject("GameCore");
        ScriptingEngine::s_initialized = true;
    }
    catch (Exception^ ex)
    {
        System::Diagnostics::Debug::WriteLine(ex->Message);
        System::Diagnostics::Debug::WriteLine("SSystem:: Exception while intializing the system.");
    }
}

//--------------------------------------------------------------------------------------
/// Destroy's the ScriptingSystem's application domain, which causes the assembly to be unloaded and any remaining managed objects to be freed.
/// Called upon app shutdown
//--------------------------------------------------------------------------------------
void SSystemDestroyDomain()
{	
    System::Diagnostics::Debug::WriteLine("SSystem:: Destroying the system");
    try
    {
        if(ScriptingEngine::s_logicCore != nullptr)
            ScriptingEngine::s_logicCore->ShutDown();

        IEnumerator^ e= MActor::s_actors->Values->GetEnumerator();
        while (e->MoveNext())
            if (((MActor^)e->Current)->m_actor)
                ((MActor^)e->Current)->m_actor=NULL;

        MActor::s_actors->Clear();

        e = MWorld::s_worlds->Values->GetEnumerator();
        while (e->MoveNext())
            if (((MWorld^)e->Current)->m_world)
                ((MWorld^)e->Current)->m_world=NULL;

        MWorld::s_worlds->Clear();

        ScriptingEngine::s_logicCore = nullptr;

		ScriptingEngine::s_assembly = nullptr;

        System::GC::Collect();
    }
    catch (Exception^ ex) 
    {
        Helpers::PrintError(ex,"Exception While Destroying the Scripting System");
    }
}

//--------------------------------------------------------------------------------------
/// Creates an Managed-Actor from its classname, adds it to the specified World (wrapping the world if not already done)
/// and sets a passed Actor pointer to the Managed-Actor's Actor representation
//--------------------------------------------------------------------------------------
void SSystemCreateActor(const char* fileName,void* world, void** actor)
{
    *actor = NULL;

#ifndef _DEBUG
    try
    {
#endif

        std::string file(fileName);
        //if (!FindMedia(file,"Scripts")) return;
        String^ fileOnly= Helpers::ToCLIString(file); //Path::GetFileNameWithoutExtension(Helpers::ToCLIString(file));

        array<MWorld^>^ args=nullptr;

        if (world)
        {
            bool found = false;
            IEnumerator^ e = MWorld::s_worlds->Values->GetEnumerator();

            while (e->MoveNext())
            {
                if( ((MWorld^)e->Current)->m_world == world )
                {
                    args=gcnew array<MWorld^> {(MWorld^)e->Current};
                    found = true;
                }
            }

            if(!found)
                args=gcnew array<MWorld^> {gcnew MWorld((World*)world)};
        }

        Object^ object = nullptr;
        Type^ type = ScriptingEngine::s_assembly->GetType(fileOnly);

        try
        {
            if (type!=nullptr)
                object = Activator::CreateInstance(type,args);
        }    

        catch (Exception^ ex)   { }

        if (object==nullptr)
        {
            object = Activator::CreateInstanceFrom(String::Concat(ScriptingEngine::s_tempPath,SCRIPTS_DLL),
                fileOnly, true, 
                BindingFlags::Instance | BindingFlags::Public | BindingFlags::CreateInstance
                , nullptr, args,
                nullptr, nullptr, nullptr)->Unwrap();
        }

        if(object != nullptr)
            *actor = ((MActor^)object)->m_actor;

#ifndef _DEBUG    
	}
    catch (Exception^ ex) 
    {
        Helpers::PrintError(ex,"Exception While loading a script:");
    }
#endif
}

//--------------------------------------------------------------------------------------
/// Serializes a Managed-Actor, namely the MActor will write its browsable Properties to the XML file
//--------------------------------------------------------------------------------------
void SSystemActorSerialize(int index,void* XMLsystem,void* node)
{
    MActor^ actor=(MActor^)MActor::s_actors[index];
    if (actor != nullptr)
        actor->Serialize(XMLsystem,node);
}

//--------------------------------------------------------------------------------------
/// Deserializes one property, specified by paramName, of a Managed-Actor from the data of a Property XML element contained in its node
//--------------------------------------------------------------------------------------
void SSystemActorDeserialize(int index,char* data,const char* paramName,const char* type)
{
    try
    {
        MActor^ actor=(MActor^)MActor::s_actors[index];
        if (actor != nullptr)
        {
            String^ pName=Helpers::ToCLIString(paramName);
            string typeS=type;
            PropertyInfo^ pInfo=actor->GetType()->GetProperty(pName);
            //System::Diagnostics::Debug::WriteLine(Helpers::ToCLIString(typeS));
            if (pInfo != nullptr && pInfo->CanWrite)
            {
                if (typeS == "Bool")
                    pInfo->SetValue(actor,*(bool*)data,nullptr);
                else if (typeS == "Int")
                    pInfo->SetValue(actor,*(int*)data,nullptr);
                else if (typeS == "Float")
                    pInfo->SetValue(actor,*(float*)data,nullptr);
                else if (typeS == "Float3")
                {
                    MVector^ vec=( MVector^)pInfo->GetValue(actor,nullptr);
                    vec->x=((Vector*)data)->x;
                    vec->y=((Vector*)data)->y;
                    vec->z=((Vector*)data)->z;
                }
                else if (typeS == "String")
                    pInfo->SetValue(actor,Helpers::ToCLIString(string((char*)data)),nullptr);   
            }
        }
        else
            System::Diagnostics::Debug::WriteLine("No Actor");
    }
    catch (Exception^ ex) 
    {
        Helpers::PrintError(ex,"Scripting System Exception");
    }
}

//--------------------------------------------------------------------------------------
/// Called when the Deserialization process -- the loading of Properties from an XML node when the specified MActor is loaded from a level file -- is complete.
/// Allows the MActor to initialize itself knowing all its Properties are set to the desired initial values.
//--------------------------------------------------------------------------------------
void SSystemActorDeserializationComplete(int index)
{
    try
    {
        MActor^ actor=(MActor^)MActor::s_actors[index];
		if(actor != nullptr)
			actor->DeserializationComplete();
    }
    catch (Exception^ ex) 
    {
        Helpers::PrintError(ex,"Scripting System Exception");
    }
}

//--------------------------------------------------------------------------------------
/// Called when an MActor lands on the ground, for custom response to this event
//--------------------------------------------------------------------------------------
void SSystemActorLanded(Actor* actor, Vector* hitVelocity)
{
    MActor^ mActor=(MActor^)MActor::s_actors[((Actor*)actor)->GetManagedIndex()];
    if (mActor != nullptr)
        mActor->Landed(gcnew MVector(*hitVelocity));
}

//--------------------------------------------------------------------------------------
/// Called when an MActor touches another Actor or the World, or is touched by another Actor, for custom response to this event.
/// Fills out pertinent information to a MCollisionInfo object the MActor can use.
//--------------------------------------------------------------------------------------
void SSystemActorTouched(void* actor,void* other, void* info)
{

    try{
        MActor^ mActor=(MActor^)MActor::s_actors[((Actor*)actor)->GetManagedIndex()];
        if (mActor != nullptr)
        {
            MCollisionInfo^ mInfo = gcnew MCollisionInfo();
            if(info)
            {
                CollisionInfo* colInfo = (CollisionInfo*)info;
                mInfo->actualDistance = colInfo->actualDistance;
                mInfo->normal = gcnew MVector(colInfo->normal);
                mInfo->point = gcnew MVector(colInfo->point);
                if(colInfo->mat)
                    mInfo->mat = gcnew MMaterial(colInfo->mat);
            }
            else
            {
                mInfo->normal = nullptr;
                mInfo->point = nullptr;
            }

            MActor^ mOther = nullptr;
            if(other)
                mOther = MActor::GetFromActor((Actor*)other);

            mActor->Touched(mOther,mInfo);
        }
    }
    catch (Exception^ ex) 
    {
        Helpers::PrintError(ex,"Scripting System Exception");
    }
}

//--------------------------------------------------------------------------------------
/// Called right before all Actors are rendered (but after Tick), so the game logic can choose to custom handle this event (generally by calling a function on each MActor in a World)
//--------------------------------------------------------------------------------------
void SSystemActorsPreRender(void* world, void* camera)
{
#ifndef _DEBUG
    try
    {
#endif
        if(ScriptingEngine::s_logicCore != nullptr)
            ScriptingEngine::s_logicCore->PreRender((MWorld^)MWorld::s_worlds[((World*)world)->m_managedIndex],gcnew MCamera((Camera*)camera));
#ifndef _DEBUG  
	}
    catch (Exception^ ex) 
    {
        Helpers::PrintError(ex,"Scripting System Exception");
    }
#endif
}

//--------------------------------------------------------------------------------------
/// Called during the process of rendering all Actors, so the game logic can choose to custom handle this event (generally by calling a function on each MActor in a World)
//--------------------------------------------------------------------------------------
void SSystemActorsOnRender(void* world, void* camera)
{
#ifndef _DEBUG
    try
    {
#endif
        if(ScriptingEngine::s_logicCore != nullptr)
            ScriptingEngine::s_logicCore->OnRender((MWorld^)MWorld::s_worlds[((World*)world)->m_managedIndex],gcnew MCamera((Camera*)camera));
#ifndef _DEBUG   
	}
    catch (Exception^ ex) 
    {
        Helpers::PrintError(ex,"Scripting System Exception");
    }
#endif
}

//--------------------------------------------------------------------------------------
/// Called after the process of rendering all Actors, so the game logic can choose to custom handle this event (generally by drawing any post FX)
//--------------------------------------------------------------------------------------
void SSystemActorsPostRender(void* world, void* camera)
{
#ifndef _DEBUG
    try
    {
#endif
        if(ScriptingEngine::s_logicCore != nullptr)
            ScriptingEngine::s_logicCore->PostRender((MWorld^)MWorld::s_worlds[((World*)world)->m_managedIndex],gcnew MCamera((Camera*)camera));
#ifndef _DEBUG
	}
    catch (Exception^ ex) 
    {
        Helpers::PrintError(ex,"Scripting System Exception");
    }
#endif
}

//--------------------------------------------------------------------------------------
/// Called when the specified World is Ticked, allowing the game logic to Tick all MActors in the World as it chooses, or otherwise update the scripted game
//--------------------------------------------------------------------------------------
void SSystemScriptsTickAll(void* world)
{
#ifndef _DEBUG
    try
    {
#endif
        Profiler::Get()->ScriptActors += ((MWorld^)MWorld::s_worlds[((World*)world)->m_managedIndex])->MActors->Count;

        if(ScriptingEngine::s_logicCore != nullptr)
            ScriptingEngine::s_logicCore->Tick((MWorld^)MWorld::s_worlds[((World*)world)->m_managedIndex]);
#ifndef _DEBUG  
	}
    catch (Exception^ ex) 
    {
        Helpers::PrintError(ex,"Scripting System Exception");
    }
#endif
}

//--------------------------------------------------------------------------------------
/// Wraps various instantiated C++ Actor Objects to their managed representations
//--------------------------------------------------------------------------------------
void SSystemActorWrap(void * actor)
{
    //put in checks here for other types of non-C# Actors that you want to wrap
    if(actor && ((Actor*)actor)->IsLight())
    {
        try
        {
            if(((Actor*)actor)->IsLight())
                gcnew MLight((Actor*)actor);
        }
        catch (Exception^ ex) 
        {
            Helpers::PrintError(ex,"---Exception While wrapping actor:");
        }
    }
}
//--------------------------------------------------------------------------------------
// Disposes the managed representation of an Actor (if any), generally when that Actor's destructor is being called
//--------------------------------------------------------------------------------------
void SSystemActorDelete(void * actor)
{
    try
    {
        MActor^ mActor= MActor::GetFromActor((Actor*)actor);
        if (mActor != nullptr)
			mActor->Dispose();
    }
    catch (Exception^ ex) 
    {
        Helpers::PrintError(ex,"---Exception While deleteing actor:");
    }
}

//--------------------------------------------------------------------------------------
/// Called on the Server when a new NetworkClient requests to joins the game, 
/// allows the game logic to determine what to do with that NetworkClient, 
/// for instance give it a Player avatar
//--------------------------------------------------------------------------------------
void SSystemNetworkClientJoined(NetworkClient* client)
{
    try
	{
	 ScriptingEngine::s_logicCore->NetworkClientJoined(gcnew MNetworkClient(client));
    }
    catch (Exception^ ex) 
    {
        Helpers::PrintError(ex,"Scripting System Exception");
    }
}

//--------------------------------------------------------------------------------------
/// Called on the Server when a new NetworkClient leaves the game, 
/// allows the game logic to determine what to do with that NetworkClient, 
/// for instance dispose any Player avatar that it currently has bound to it.
//--------------------------------------------------------------------------------------
void SSystemNetworkClientLeft(NetworkClient* client)
{
    try
	{
	 ScriptingEngine::s_logicCore->NetworkClientLeft(gcnew MNetworkClient(client));
    }
    catch (Exception^ ex) 
    {
        Helpers::PrintError(ex,"Scripting System Exception");
    }
}