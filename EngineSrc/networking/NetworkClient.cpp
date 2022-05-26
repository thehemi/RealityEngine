//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
// NetworkClient
// A NetworkClient that is connected to the Server, server-side representation
// Author: Jeremy Stieglitz
//====================================================================================
#include "stdafx.h"
#include "NetworkActor.h"
#include "NetworkClient.h"
#include "..\RakNet\GetTime.h"
#include "..\RakNet\RakServerInterface.h"
#include "..\RakNet\PacketEnumerations.h"
#include "..\RakNet\RakNetworkFactory.h"
#include "Server.h"

vector<NetworkClient*> NetworkClient::NetworkClients;

NetworkClient* NetworkClient::GetNetworkClient(PlayerID playerID)
{
	for(int i = 0; i < NetworkClients.size(); i++)
	{
		if(NetworkClients[i]->m_RakNetworkClientID == playerID)
			return NetworkClients[i];
	}
	return 0;
}
void NetworkClient::DeleteAllNetworkClients()
{
for(int i = 0; i < NetworkClients.size();i++)
{
	delete NetworkClients[i];
	i--;
}
}
void NetworkClient::ClearAllSendBuffers()
{
	for(int i = 0; i < NetworkClients.size();i++)
	{
		NetworkClients[i]->ClearNetworkSendBuffers();
	}
}

void NetworkClient::ClearNetworkSendBuffers()
{
	for(int i = 0; i < m_NetworkActorSends.size();i++)
	{
		m_NetworkActorSends[i]->ResetPackets();
	}
}
void NetworkClient::PrepareWorldUpdateBuffers()
{
	m_WorldReliableUpdatePacket.useTimeStamp = ID_TIMESTAMP;
	m_WorldReliableUpdatePacket.timeStamp = RakNetGetTime();
	m_WorldReliableUpdatePacket.dwType = GAME_MSGID_WORLDSTATE_UPDATE_RELIABLE;
	m_WorldReliableUpdatePacket.WorldStateDataSize = 0;

	m_WorldUnreliableUpdatePacket.useTimeStamp = ID_TIMESTAMP;
	m_WorldUnreliableUpdatePacket.timeStamp = RakNetGetTime();
	m_WorldUnreliableUpdatePacket.dwType = GAME_MSGID_WORLDSTATE_UPDATE_UNRELIABLE;
	m_WorldUnreliableUpdatePacket.WorldStateDataSize = 0;
}
NetworkClient::NetworkClient(PlayerID RakNetworkClientID,string name,float RecievePacketsPerSecond, bool isServer)
{
	m_RakNetworkClientID = RakNetworkClientID;
	m_Name = name;
	m_IsServer = isServer;
	m_IsAdministrator = false;
	m_ActorAvatar = NULL;

	if(!isServer)
	{
		if(RecievePacketsPerSecond > Server::Instance()->m_ServerSettings.m_MaxPacketsPerSecond)
			m_RecievePacketsPerSecond = Server::Instance()->m_ServerSettings.m_MaxPacketsPerSecond;
		else
			m_RecievePacketsPerSecond = RecievePacketsPerSecond;
	}
	else
		m_RecievePacketsPerSecond = RecievePacketsPerSecond;

	NetworkClients.push_back(this);
	Server::Instance()->SendObserverNames(ALL_CLIENTS);
}
void NetworkClient::CreatePlayer()
{
	if(Server::Instance()->m_ServerCallBack_CreateNetworkClientPlayer)
		Server::Instance()->m_ServerCallBack_CreateNetworkClientPlayer(this);
}
NetworkClient::~NetworkClient()
{
	if(Server::Instance()->m_ServerCallBack_DeleteNetworkClientPlayer)
		Server::Instance()->m_ServerCallBack_DeleteNetworkClientPlayer(this);

	for(int i = 0; i < m_NetworkActorSends.size(); i++)
	{
		delete m_NetworkActorSends[i];
	}
	m_NetworkActorSends.clear();
	NetworkClients.erase(find(NetworkClients.begin(),NetworkClients.end(),this));
}
void NetworkClient::SetActorAvatar(Actor* avatar)
{
m_ActorAvatar = avatar;
}
NetworkActorPackets* NetworkClient::GetOrCreateNetworkActorPackets(NetworkActor* forNetworkActor)
{
	NetworkActorPackets* packets = GetNetworkActorPackets(forNetworkActor);

	if(packets)
		return packets;
	
	packets = new NetworkActorPackets(forNetworkActor);
	m_NetworkActorSends.push_back(packets);
	return packets;
}
NetworkActorPackets* NetworkClient::GetNetworkActorPackets(NetworkActor* forNetworkActor)
{
	for(int i = 0; i < m_NetworkActorSends.size(); i++)
	{
		if(m_NetworkActorSends[i]->m_NetworkActor == forNetworkActor)
				return m_NetworkActorSends[i];
	}
	return 0;
}