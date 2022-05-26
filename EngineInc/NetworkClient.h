//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
/// NetworkClient
/// A NetworkClient that is connected to the Server, server-side representation
/// Author: Jeremy Stieglitz
//====================================================================================

#ifndef NETWORKCLIENT_INCLUDED
#define NETWORKCLIENT_INCLUDED
#include "NetworkActor.h"
#include "NetShared.h"
#include "..\RakNet\PacketPriority.h"

/// A NetworkClient that is connected to the Server, this is a server-side representation
class ENGINE_API NetworkClient
{
	friend class Server;
	friend class NetworkActor;
public:
	/// Name of this Client
	string m_Name;
	/// Gets packet buffers associated with a particular NetworkActor to send to this NetworkClient
	NetworkActorPackets* GetNetworkActorPackets(NetworkActor* forNetworkActor);
	/// Gets packet buffers associated with a particular NetworkActor to send to this NetworkClient, creates them if they don't exist
	NetworkActorPackets* GetOrCreateNetworkActorPackets(NetworkActor* forNetworkActor);
	/// True if this is dummy NetworkClient for server-side gameplay on non-dedicated server. Allows NetworkActors to treat this NetworkClient specially should they so desire.
	bool m_IsServer;
	/// Actor that Game can use to associate with this Client, auto-deleted when Client leaves session
	void SetActorAvatar(Actor* avatar);
	inline Actor* GetActorAvatar(){return m_ActorAvatar;}
	bool m_IsAdministrator;
	PlayerID GetRakNetworkClientID(){return m_RakNetworkClientID;}

private:
	static vector<NetworkClient*> NetworkClients;

	static NetworkClient* GetNetworkClient(PlayerID playerID);
	static void DeleteAllNetworkClients();

	/// Clears all buffers for all Clients, Server calls in-between network frames
	static void ClearAllSendBuffers();

	NetworkClient(PlayerID RakNetworkClientID,string name,float RecievePacketsPerSecond, bool isServer = false);
	virtual ~NetworkClient();

	void ClearNetworkSendBuffers();

	PlayerID m_RakNetworkClientID;
	vector<NetworkActorPackets*> m_NetworkActorSends;

	float m_RecievePacketsPerSecond;

	GAMEMSG_WORLDSTATE_UPDATE m_WorldReliableUpdatePacket;
	GAMEMSG_WORLDSTATE_UPDATE m_WorldUnreliableUpdatePacket;

	void PrepareWorldUpdateBuffers();
	void CreatePlayer();

	/// Actor that Game can use to associate with this Client, auto-deleted when Client leaves session
	Actor* m_ActorAvatar;
};

#endif