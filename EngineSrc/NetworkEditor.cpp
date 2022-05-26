//======= Copyright (c) 2004, Artificial Studios. All rights reserved. ========
//
// Network Editing and CVS
// 1. Clients connects
// 2. Requests a map to edit
// 3. Server loads it
// 4. Server sends list of required resources
// 5. Client checks list against its resources
// 6. Client requests resources it needs
// 7. Server sends resources
// 8. All done! Regular networking can continue
//
// 
//=============================================================================
#include "stdafx.h"
#include "NetworkEditor.h"


//-----------------------------------------------------------------------------
// Client has connected...
// Load any world it wants, then send it all our resources
//
//-----------------------------------------------------------------------------
void NetworkEditor::ClientConnected(void* clientInfoHere)
{
	// TODO: Does client want us to load its world for editing? If so, do it here!
	// if(clientInfo.msg == MSGID_CLIENT_LOADWORLD)
	//		Game_NewMap()

	// Send client all our resources
	// TODO: Replace LogPrintf with Send
	for(int i=0;i<ResourceManager::Instance()->m_LoadedResources.size();i++){
		ResourceManager::Resource r = ResourceManager::Instance()->m_LoadedResources[i];
		if(r.type == ResourceManager::TextureRes)
			LogPrintf("Tex, File=%s",((Texture*)r.object)->filename.c_str());
		if(r.type == ResourceManager::ModelRes)
			LogPrintf("Model, File=%s",((Model*)r.object)->m_FileName.c_str());
		if(r.type == ResourceManager::WorldRes)
			LogPrintf("World, File=%s",((World*)r.object)->m_FileName.c_str());
	}
}

//-----------------------------------------------------------------------------
// Client has connected...
//-----------------------------------------------------------------------------
void NetworkEditor::ClientMessage(void* messageInfoHere)
{
	int msg = 0;
	switch(msg)
	{
	case MSGID_CLIENT_LOADWORLD:
		{
			// Client wants us to load this world...
			// -If server has world loaded with clients in, it can reject the request (unless we want to support multiple worlds on one server, eek!)
			// -Server MUST load world before sending resource list, or resources won't include world resources.
			break;
		}
	case MSGID_CLIENT_REQUESTFILES:
		{
			// Client has requested these files, send them to him via network system
			break;
		}
	default:
		Warning("Unknown message: %d",msg);
		break;
	}
}

//-----------------------------------------------------------------------------
// Connects client to server, synchronizes any missing files, then loads
// the level
//-----------------------------------------------------------------------------
bool NetworkEditor::Connect(string server)
{
	// TODO: Regular network connect to server
	// GameClient::Instance()->ConnectToHost(server)

	// TODO: Client asks server to load world
	// GameClient::Instance()->SendMessage(MSGID_CLIENT_LOADWORLD,Editor::Instance()->m_World->m_FileName);

	// Resource indices we need back from server. 
	// TODO: Make strings if we can't ensure indice stability. However indices are so much faster
	vector<short> requiredResources; 

	int numResources = 0;
	// TODO: Fill serverResources from server network message
	// numResources = GetNetworkMessage(blah);

	for(int i=0;i<numResources;i++)
	{
		ResourceManager::ResType type; // = GetNetworkMessage(blah);
		string	filename; // = GetNetworkMessage(blah);

		bool exists = false;
		if(type == ResourceManager::TextureRes)
			exists = FindMedia(filename,"Textures");
		else if(type == ResourceManager::ModelRes)
			exists = FindMedia(filename,"Models");
		else if(type == ResourceManager::WorldRes)
			exists = FindMedia(filename,"Maps");

		// TODO: Also check timestamps and checksum

		if(!exists)
			requiredResources.push_back(i);
	}

	// TODO: Ask server for resources
	// GameClient::Instance()->SendMessage(MSGID_CLIENT_REQUESTRESOURCES,requiredResources);

	// Now we're connected, and we have synchronized all files!
	// CVS has done its job!
	return true;
}