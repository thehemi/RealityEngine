//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
// Server
// Handles Server-side Networking, including management of clients, 
// and distribution of accumulated World-State data
// Author: Jeremy Stieglitz
//====================================================================================

#include "stdafx.h"
#include "Server.h"
#include "NetShared.h"
#include <tchar.h>
#include "..\RakNet\GetTime.h"
#include "..\RakNet\RakServerInterface.h"
#include "..\RakNet\PacketEnumerations.h"
#include "..\RakNet\RakNetworkFactory.h"
#include "GameRecord.h"
#include "GUISystem.h"
#include "classmap.h"
#include "Client.h"

// For debugging network/packet problems
void _DEBUGLOG(const char *fmt, ...);

static bool FirstTickSent = false;

unsigned char GetPacketIdentifier(Packet *p)
{
	if (!p)
		return 255;

	/*if ((unsigned char)p->data[0] == ID_TIMESTAMP)
	{
	assert(p->length > sizeof(unsigned char) + sizeof(unsigned long));
	return (unsigned char) p->data[sizeof(unsigned char) + sizeof(unsigned long)];
	}
	else*/
	return (unsigned char) p->data[0];
}
void Server::ReceiveDisconnectionNotification(Packet *packet)
{
	NetworkClient* client = NetworkClient::GetNetworkClient(packet->playerId);
	if(client)
	{
		SendChat(ALL_CLIENTS,"%s left.",client->m_Name.c_str());
		delete client;
		SendObserverNames(ALL_CLIENTS);
//		NetworkActor::PrintAllNetworkActors();
	}
}
void Server::ReceiveNewNetworkClient(Packet *packet)
{
}
void Server::ReceiveResponsiveNetworkClient(Packet *packet)
{
}
void Server::ReceiveModifiedPacket(Packet *packet)
{
}

void Server::ReceiveConnectionLost(Packet *packet)
{	
	NetworkClient* client = NetworkClient::GetNetworkClient(packet->playerId);
	if(client)
	{
		SendChat(ALL_CLIENTS,"%s was dropped.",client->m_Name.c_str());
		delete client;
		SendObserverNames(ALL_CLIENTS);
		NetworkActor::PrintAllNetworkActors();
	}
}
//===========================================================================
// Handles incoming game messages from clients
//===========================================================================
void Server::ProcessGamePacket(Packet* p, unsigned char packetId)
{
	if(packetId == GAME_MSGID_CLIENT_SETTINGS_INFO)
	{
		if(!NetworkClient::GetNetworkClient(p->playerId))
		{
			GAMEMSG_CLIENT_SETTINGS_INFO* clientSettings = (GAMEMSG_CLIENT_SETTINGS_INFO*)p->data;
			if(clientSettings->buildVersion == (float)BUILD_VERSION)
				JoinNewNetworkClient(new NetworkClient(p->playerId,clientSettings->playerName,clientSettings->RecievePacketsPerSeconds));
			else
				SendBuildVersion(p->playerId);
		}
		return;
	}

	NetworkClient* client = NetworkClient::GetNetworkClient(p->playerId);
	if(!client)
		return;

	unsigned char* pMsgBuffer = p->data;

	switch(packetId)
	{
	case GAME_MSGID_CHAT:
		{
			SendGameChatToAll(client,((GAMEMSG_CHAT*)pMsgBuffer)->strChatString);
			break;
		}
	case GAME_MSGID_CLIENT_FINISHED_LOADING:
		{
			EnterGame(client);
			break;
		}
	default:
		{
			if(m_ServerCallBack_ProcessGamePacket)
				m_ServerCallBack_ProcessGamePacket(client,p,packetId);
			break;
		}
	}
}

//====================================================
// We handle incoming packets before the next game tick
//====================================================
void Server::PreWorldTick()
{
	if(m_ServerStarted)
	{
		if(m_IsMultiplayer)
		{
			Sleep(0);
			HandleIncomingPackets();
			Sleep(0);
		}
	}
}

//====================================================
// We send outgoing packets just after the game tick
//====================================================
void Server::PostWorldTick()
{
	if(m_ServerStarted)
	{	
		if(!FirstTickSent)
		{
			FirstTickSent = true;
			GameRecorder::Instance()->ResetStartTime();
		}

		//if(GSeconds - m_LastKeepAliveSendTime > 5)
		//	SendKeepAlive();
			
		SendWorldStateUpdate();

		if(m_IsMultiplayer)
		{
		Sleep(0);
		if(m_ServerSettings.m_PrintNetworkStats && GSeconds - m_LastPrintStatsTime > 2.0f)
			PrintNetworkStats();
		}
	}
}

void Server::Initialize()
{
	m_RakServer = 0;
	m_ServerStarted = false;
	m_LastPrintStatsTime = 0;
	m_LastSentWorldUpdateTime = 0;
	ServerNetworkClient = 0;

	LoadPreferences();
}

void Server::Shutdown()
{
	Stop();
}

string Server::GetMOTD()
{
	char msg[1024];
	sprintf(msg,"Welcome to '%s' v%.2f",m_ServerSettings.m_SessionName.c_str(),BUILD_VERSION);
	return msg;
}

void Server::BeginHosting(string SessionName, string map, bool isMultiplayer)
{
	Client::Instance()->Disconnect();

	if(!m_ServerStarted || (isMultiplayer != m_IsMultiplayer))
	{
		m_ServerSettings.m_SessionName = SessionName;

		if(isMultiplayer)
		{
			if(!m_RakServer)
				m_RakServer = RakNetworkFactory::GetRakServerInterface();

			if( !Start(SessionName) )
			{
				Warning("Couldn't host game, possibly because there is a server already running on this system with the same port.");
				return;
			}
		}
		else
			Stop();

		m_IsMultiplayer = isMultiplayer;
		m_ServerStarted = true;
	}

	string parseMapFile = string("\\") + map;
	m_CurrentMap = parseMapFile.substr(parseMapFile.find_last_of("\\") + 1);
	m_LastKeepAliveSendTime = GSeconds;
	if(m_ServerCallBack_NewMap)
		m_ServerCallBack_NewMap(map);

	//NetworkActor::PrintAllNetworkActors();
}

void Server::Stop()
{
	GameRecorder::Instance()->ShutDown();
	if(m_ServerStarted && m_IsMultiplayer)
	{
		m_RakServer->Disconnect(2500);
		Sleep(0); //give RakNet thread a chance to shut down
	}
	m_IsMultiplayer = false;
	NetworkClient::DeleteAllNetworkClients();
	m_ServerStarted = false;
	ServerNetworkClient = 0;

	if(m_RakServer)
	{
		RakNetworkFactory::DestroyRakServerInterface(m_RakServer);
		m_RakServer = 0;
	}
}

void Server::HandleIncomingPackets()
{
	// Direct native packets
	Packet* p;
	unsigned char packetIdentifier;

	// Get a packet from either the server or the client
	p = m_RakServer->Receive();
	while (p)
	{
		packetIdentifier = GetPacketIdentifier(p);
		// Check if this is a native packet
		switch(packetIdentifier)
		{
			// ------------------------------
			// Server only
			// ------------------------------
		case ID_DISCONNECTION_NOTIFICATION:
			ReceiveDisconnectionNotification(p);
			break;

		case ID_NEW_INCOMING_CONNECTION:
			ReceiveNewNetworkClient(p);
			break;

		case ID_RECEIVED_STATIC_DATA:
			ReceiveResponsiveNetworkClient(p);
			break;

			// ------------------------------
			// Both server and client
			// ------------------------------
		case ID_MODIFIED_PACKET:
			ReceiveModifiedPacket(p);
			break;

		case ID_CONNECTION_LOST:
			ReceiveConnectionLost(p);
			break;
		default:
			// If not a native packet send it to ProcessUnhandledPacket which should have been written by the user
			ProcessGamePacket(p, packetIdentifier);
			break;
		}
		m_RakServer->DeallocatePacket(p);
		p = m_RakServer->Receive();
	}
}


bool Server::Send(NetworkClient* client, char *data, const long length, 
				  PacketReliability reliability, PacketPriority priority,  char orderingStream, bool secured)
{
	if(!m_ServerStarted)
		return false;

	if(GameRecorder::Instance()->m_RecordGame && (client == ALL_CLIENTS || client->m_IsServer))
		GameRecorder::Instance()->RecordNetworkMessage((unsigned char*) data,length);

	if(!m_IsMultiplayer)
		return false;

	if(reliability == UNRELIABLE_SEQUENCED && orderingStream == 0)
		Error("Debug warning -- you're trying to filter out multiple messages on a single ordering stream");

	if(client && client->m_IsServer)
		return false;

	if(client)
		m_RakServer->Send(data, length, priority, reliability, orderingStream,client->m_RakNetworkClientID,false);
	else 
		m_RakServer->Send(data, length, priority, reliability, orderingStream,UNASSIGNED_PLAYER_ID,true);

	return true;
}
void Server::SendBuildVersion(PlayerID playerID)
{
	if(!m_IsMultiplayer)
		return;

	GAMEMSG_BUILD_VERSION msgBuildVersion;
	msgBuildVersion.dwType = GAME_MSGID_BUILD_VERSION;
	msgBuildVersion.buildVersion = BUILD_VERSION;

	m_RakServer->Send((char*)&msgBuildVersion, sizeof(msgBuildVersion), LOW_PRIORITY, RELIABLE, 0,playerID,false);
}

bool Server::Start(string sessionName, int port)
{
	m_RakServer->SetRelayStaticClientData(false);

	if(port == 0)
		port = Engine::Instance()->MainConfig->GetInt("DefaultServerPort");

	/*unsigned long compressionFrequencyTable[256];
	g_Game.resourceTables.getServerFrequencyTable((unsigned long*)&compressionFrequencyTable);
	bool generated = rakServer->GenerateCompressionLayer(compressionFrequencyTable,true);
	unsigned long decompressionFrequencyTable[256];
	g_Game.resourceTables.getNetworkClientFrequencyTable((unsigned long*)&decompressionFrequencyTable);
	generated = rakServer->GenerateCompressionLayer(decompressionFrequencyTable,false);*/

	bool started = m_RakServer->Start(16, 8234, 15, port);
	if(!m_RakServer->IsActive())
		Error("!m_RakServer->IsActive()");

	m_RakServer->SetRelayStaticClientData(false);
	m_RakServer->StopSynchronizedRandomInteger();

	return started;
}

unsigned int previousTotalBitsSent = 0;
unsigned int previousTotalBitsReceived = 0;
void Server::PrintNetworkStats()
{
	// Network statistics.

	if(NetworkClient::NetworkClients.size() > 1)
	{
	RakNetStatisticsStruct* averaged = new RakNetStatisticsStruct();
	
	for(int i = 1; i < NetworkClient::NetworkClients.size(); i++)
	{
		RakNetStatisticsStruct* stats = m_RakServer->GetStatistics(NetworkClient::NetworkClients[i]->GetRakNetworkClientID());

		averaged->acknowlegementBitsSent += stats->acknowlegementBitsSent;
		averaged->acknowlegementsPending += stats->acknowlegementsPending;
		averaged->acknowlegementsReceived += stats->acknowlegementsReceived;
		averaged->acknowlegementsSent += stats->acknowlegementsSent;
		averaged->bitsReceived += stats->bitsReceived;
		averaged->bitsWithBadCRCReceived += stats->bitsWithBadCRCReceived;
		averaged->duplicateAcknowlegementsReceived += stats->duplicateAcknowlegementsReceived;
		averaged->duplicateMessagesReceived += stats->duplicateMessagesReceived;
		averaged->encryptionBitsSent += stats->encryptionBitsSent;
		averaged->internalOutputQueueSize += stats->internalOutputQueueSize;
		averaged->invalidMessagesReceived += stats->invalidMessagesReceived;
		averaged->lossySize += stats->lossySize;
		averaged->messageDataBitsResent += stats->messageDataBitsResent;
		averaged->messageDataBitsSent[0] += stats->messageDataBitsSent[0];
		averaged->messageDataBitsSent[1] += stats->messageDataBitsSent[1];
		averaged->messageDataBitsSent[2] += stats->messageDataBitsSent[2];
		averaged->messageDataBitsSent[3] += stats->messageDataBitsSent[3];
		averaged->messageResends += stats->messageResends;
		averaged->messageSendBuffer[0] += stats->messageSendBuffer[0];
		averaged->messageSendBuffer[1] += stats->messageSendBuffer[1];
		averaged->messageSendBuffer[2] += stats->messageSendBuffer[2];
		averaged->messageSendBuffer[3] += stats->messageSendBuffer[3];
		averaged->messagesOnResendQueue += stats->messagesOnResendQueue;
		averaged->messagesReceived += stats->messagesReceived;
		averaged->messagesSent[0] += stats->messagesSent[0];
		averaged->messagesSent[1] += stats->messagesSent[1];
		averaged->messagesSent[2] += stats->messagesSent[2];
		averaged->messagesSent[3] += stats->messagesSent[3];
		averaged->messagesTotalBitsResent += stats->messagesTotalBitsResent;
		averaged->messagesWaitingForReassembly += stats->messagesWaitingForReassembly;
		averaged->messageTotalBitsSent[0] += stats->messageTotalBitsSent[0];
		averaged->messageTotalBitsSent[1] += stats->messageTotalBitsSent[1];
		averaged->messageTotalBitsSent[2] += stats->messageTotalBitsSent[2];
		averaged->messageTotalBitsSent[3] += stats->messageTotalBitsSent[3];
		averaged->numberOfSplitMessages += stats->numberOfSplitMessages;
		averaged->numberOfUnsplitMessages += stats->numberOfUnsplitMessages;
		averaged->orderedMessagesInOrder += stats->orderedMessagesInOrder;
		averaged->orderedMessagesOutOfOrder += stats->orderedMessagesOutOfOrder;
		averaged->packetsContainingOnlyAcknowlegements += stats->packetsContainingOnlyAcknowlegements;
		averaged->packetsContainingOnlyAcknowlegementsAndResends += stats->packetsContainingOnlyAcknowlegementsAndResends;
		averaged->packetsReceived += stats->packetsReceived;
		averaged->packetsSent += stats->packetsSent;
		averaged->packetsWithBadCRCRecieved += stats->packetsWithBadCRCRecieved;
		averaged->sequencedMessagesInOrder += stats->sequencedMessagesInOrder;
		averaged->sequencedMessagesOutOfOrder += stats->sequencedMessagesOutOfOrder;
		averaged->totalBitsSent += stats->totalBitsSent;
		averaged->totalSplits += stats->totalSplits;
	}

	unsigned int bytesPerSecondSent = 0;
	unsigned int bytesPerSecondReceived = 0;

	if(previousTotalBitsSent != 0)
		bytesPerSecondSent = ((averaged->totalBitsSent - previousTotalBitsSent)/8)/(GSeconds - m_LastPrintStatsTime);

	if(previousTotalBitsReceived != 0)
		bytesPerSecondReceived = ((averaged->bitsReceived - previousTotalBitsReceived)/8)/(GSeconds - m_LastPrintStatsTime);

	previousTotalBitsSent = averaged->totalBitsSent;
	previousTotalBitsReceived = averaged->bitsReceived;

	LogPrintf(" ");
	LogPrintf(" ");
	LogPrintf("----- SERVER NETWORK STATS TIME: %f -----",GSeconds);
	LogPrintf("- Bytes Per Second Sent (since last stat): %d",bytesPerSecondSent);
	LogPrintf("- Bytes Per Received (since last stat): %d",bytesPerSecondReceived);
	LogPrintf("- Packets Sent: %d",averaged->messagesSent[0]+averaged->messagesSent[1]+averaged->messagesSent[2]+averaged->messagesSent[3]);
	LogPrintf("- Packets received: %d",averaged->messagesReceived);
	LogPrintf("- Bytes sent: %d",averaged->totalBitsSent/8);
	LogPrintf("- Bytes received: %d",averaged->bitsReceived/8);
	LogPrintf("- # of sequenced messaged in order: %d",averaged->sequencedMessagesInOrder);
	LogPrintf("- # of sequenced messages out of order: %d",averaged->sequencedMessagesOutOfOrder);
	LogPrintf("- # of ordered messaged in order: %d",averaged->orderedMessagesInOrder);
	LogPrintf("- # of ordered messages out of order: %d",averaged->orderedMessagesOutOfOrder);	
	LogPrintf("- # of invalid messages received: %d",averaged->invalidMessagesReceived);
	LogPrintf("- Reliable output queue size: %d",averaged->internalOutputQueueSize);
	LogPrintf("- # of reliable ack pending: %d",averaged->acknowlegementsPending);
	LogPrintf(" ");
	LogPrintf(" ");
	//LogPrintf("- Packets Lost: %d",m_RakServer->GetLostPacketCount());
	//LogPrintf("- %% Packet Loss: %d",m_RakServer->GetPacketlossPercentile());

	delete averaged;
	}
	else
	{
		previousTotalBitsSent = 0;
		previousTotalBitsReceived = 0;
	}

	m_LastPrintStatsTime = GSeconds;
}
string Server::GetNetworkClientName(NetworkClient* client)
{
	if(client)
		return client->m_Name;
	else
		return "[NoName]";
}

// Returns a singleton instance
Server* Server::Instance () 
{
	static Server inst;
	return &inst;
}

//====================================================
// sendWorldState()
//
// Order:
// (Guaranteed messages)
// 1. Pawn Spawns
// 2. Other Spawns
// 3. Reliable updates
// 
// (Unguaranteed messages)
// 1. All packets, no order
//
//====================================================
void Server::SendWorldStateUpdate()
{
	for(int i = 0; i < NetworkClient::NetworkClients.size(); i++)
	{
		NetworkClient* client = NetworkClient::NetworkClients[i];

		client->PrepareWorldUpdateBuffers();

	//send lower m_NetworkSynchOrder NetworkActors first, such as Players
	//copy our current networkactors to a temp list
	vector<NetworkActorPackets*> packets = client->m_NetworkActorSends;
	while(packets.size())
	{
		int LowestSynchOrder = BIG_NUMBER;
		//find current lowest order
		for(int i = 0; i < packets.size(); i++)
		{
			if(packets[i]->m_NetworkSynchOrder < LowestSynchOrder)
				LowestSynchOrder = packets[i]->m_NetworkSynchOrder;
		}
		// send all NetworkActors that are <= current lowest order and erase them from the list
		for(int i = 0; i < packets.size(); i++)
		{
			if(packets[i]->m_NetworkSynchOrder <= LowestSynchOrder)
			{
			WriteNetworkActorPackets(client,packets[i],PACKET_SPAWN,client->m_WorldReliableUpdatePacket.WorldStateData,client->m_WorldReliableUpdatePacket.WorldStateDataSize);
			WriteNetworkActorPackets(client,packets[i],PACKET_RELIABLE,client->m_WorldReliableUpdatePacket.WorldStateData,client->m_WorldReliableUpdatePacket.WorldStateDataSize);
			WriteNetworkActorPackets(client,packets[i],PACKET_UNRELIABLE,client->m_WorldUnreliableUpdatePacket.WorldStateData,client->m_WorldUnreliableUpdatePacket.WorldStateDataSize);
			packets.erase(packets.begin() + i);
			i--;
			}
		}
	}

		for(int n = 0; n < client->m_NetworkActorSends.size();n++)
		{
			if(client->m_NetworkActorSends[n]->m_DeleteMe)
			{
				delete client->m_NetworkActorSends[n];
				client->m_NetworkActorSends.erase(client->m_NetworkActorSends.begin() + n);
				n--;
			}
		}

		if(client->m_WorldReliableUpdatePacket.WorldStateDataSize > 0)
			Send(client,(char*)&client->m_WorldReliableUpdatePacket,sizeof(client->m_WorldReliableUpdatePacket) - sizeof(client->m_WorldReliableUpdatePacket.WorldStateData) + client->m_WorldReliableUpdatePacket.WorldStateDataSize,RELIABLE_ORDERED,HIGH_PRIORITY,SERVER_STREAM_WORLDSTATE_RELIABLE);

		if(client->m_WorldUnreliableUpdatePacket.WorldStateDataSize > 0)
			Send(client,(char*)&client->m_WorldUnreliableUpdatePacket,sizeof(client->m_WorldUnreliableUpdatePacket) - sizeof(client->m_WorldUnreliableUpdatePacket.WorldStateData) + client->m_WorldUnreliableUpdatePacket.WorldStateDataSize,UNRELIABLE_SEQUENCED,MEDIUM_PRIORITY,SERVER_STREAM_WORLDSTATE_UNRELIABLE);
	}

	NetworkClient::ClearAllSendBuffers();
}
//===========================================================================
// Updates new client, then sends out a joined message
//===========================================================================
void Server::JoinNewNetworkClient(NetworkClient* client)
{
	if(!IsMultiplayer() || !client)
		return;
	/*
	//Tell client the version of the server build
	GAMEMSG_SERVER_TO_CLIENT_BUILD_VERSION msgBuildVersion;
	msgBuildVersion.dwType  = GAME_MSGID_SERVER_TO_CLIENT_BUILD_VERSION;
	msgBuildVersion.version = BUILD_VERSION;
	Send(clientID,(char*)&msgBuildVersion,sizeof(GAMEMSG_SERVER_TO_CLIENT_BUILD_VERSION),RELIABLE_ORDERED);
	*/

	SendGameStateToNewNetworkClient(client);

	SendChat(client,GetMOTD().c_str());
	SendChat(ALL_CLIENTS,"%s has connected.",client->m_Name.c_str());
}

//===========================================================================
// SendWorldStateToNewPlayer
// Send the world state to the new player
//===========================================================================
void Server::SendGameStateToNewNetworkClient(NetworkClient* client)
{
	SendLoadMap(client);
	SendWorldStateFull(client);
}

void Server::SendWorldStateFull(NetworkClient* client)
{
	client->PrepareWorldUpdateBuffers();
	client->m_WorldReliableUpdatePacket.dwType = GAME_MSGID_WORLDSTATE_FULL;

	//send lower m_NetworkSynchOrder NetworkActors first, such as Players
	//to do this, copy our current networkactors list to a temp list for sorting
	vector<NetworkActor*> ActorsToSynch = NetworkActor::NetworkActors;
	while(ActorsToSynch.size())
	{
		int LowestSynchOrder = BIG_NUMBER;
		//find current lowest order
		for(int i = 0; i < ActorsToSynch.size(); i++)
		{
			if(ActorsToSynch[i]->m_NetworkSynchOrder < LowestSynchOrder)
				LowestSynchOrder = ActorsToSynch[i]->m_NetworkSynchOrder;
		}
		// send all NetworkActors that are <= current lowest order and erase them from the list
		for(int i = 0; i < ActorsToSynch.size(); i++)
		{
			if(ActorsToSynch[i]->m_NetworkSynchOrder <= LowestSynchOrder)
			{
			if(ActorsToSynch[i]->m_HasTicked)
			{
				NetworkActorPackets* packets =ActorsToSynch[i]->Server_MakeOnJoinSynchMessages(client);
				WriteNetworkActorPackets(client,packets,PACKET_SPAWN,client->m_WorldReliableUpdatePacket.WorldStateData,client->m_WorldReliableUpdatePacket.WorldStateDataSize,true);			
			}
			ActorsToSynch.erase(ActorsToSynch.begin() + i);
			i--;
			}
		}
	}

	Send(client,(char*)&client->m_WorldReliableUpdatePacket,sizeof(client->m_WorldReliableUpdatePacket) - sizeof(client->m_WorldReliableUpdatePacket.WorldStateData) + client->m_WorldReliableUpdatePacket.WorldStateDataSize,RELIABLE_ORDERED,HIGH_PRIORITY,SERVER_STREAM_WORLDSTATE_RELIABLE);
	client->ClearNetworkSendBuffers();
}
void Server::SendLoadMap(NetworkClient* client)
{
	GAMEMSG_LOAD_WORLD msgLoad;
	msgLoad.dwType = GAME_MSGID_LOAD_WORLD;
	strcpy(msgLoad.mapName, m_CurrentMap.c_str());
	Send(client,(char*)&msgLoad,sizeof(msgLoad.dwType) + strlen(msgLoad.mapName)+1,RELIABLE_ORDERED,HIGH_PRIORITY,SERVER_STREAM_WORLDSTATE_RELIABLE);
}
void Server::SendEndGame(NetworkClient* client)
{
	GAMEMSG_GENERIC msgEndGame;
	msgEndGame.dwType = GAME_MSGID_ENDGAME;
	Send(client,(char*)&msgEndGame,sizeof(msgEndGame),RELIABLE_ORDERED,HIGH_PRIORITY,SERVER_STREAM_WORLDSTATE_RELIABLE);
}
void Server::SendObserverNames(NetworkClient* client)
{
	string observerNames;
	for(int i = 0; i < NetworkClient::NetworkClients.size();i++)
	{
		observerNames += NetworkClient::NetworkClients[i]->m_Name;
		if(i < NetworkClient::NetworkClients.size() - 1)
			observerNames += ", ";
	}
	m_ObserverNames = observerNames;

	GAMEMSG_OBSERVER_NAMES msgObserverNames;
	msgObserverNames.dwType = GAME_MSGID_OBSERVER_NAMES;
	strcpy(msgObserverNames.observerNames, m_ObserverNames.c_str());
	Send(client,(char*)&msgObserverNames,sizeof(msgObserverNames.dwType) + strlen(msgObserverNames.observerNames)+1,RELIABLE_ORDERED);
}
void Server::LoadedNewMap()
{
	SendEndGame(ALL_CLIENTS);

	if(GameRecorder::Instance()->m_RecordGame)
		GameRecorder::Instance()->NewRecordFile(m_CurrentMap.substr(0,m_CurrentMap.size() - 4));

	SendLoadMap(ServerNetworkClient);
	NetworkClient::DeleteAllNetworkClients();

	PlayerID server;
	server.binaryAddress = 0;
	server.port = 0;
	if(!GameRecorder::Instance()->m_RecordGame)
		ServerNetworkClient = new NetworkClient(server,Client::Instance()->m_ClientSettings.m_PlayerName,1,true);
	else
		ServerNetworkClient = new NetworkClient(server,Client::Instance()->m_ClientSettings.m_PlayerName,25,true);

	SendChat(ServerNetworkClient,GetMOTD().c_str());

	FirstTickSent = false;
}
//===========================================================================
// Helper function used by SendWorldStateUpdate()
// Writes all packets an actor has in its list
//===========================================================================
void Server::WriteNetworkActorPackets(NetworkClient* client,NetworkActorPackets* packets, PacketType packetType, char* buffer, unsigned short& bufferPos, bool IsFullWorldState)
{
	if(!packets)
		return;

	if(packets->m_Packets[packetType].writePos <= 0)
		return;

	if((packetType == PACKET_RELIABLE || packetType == PACKET_UNRELIABLE) && !packets->m_SentSpawnPacket)
		return;

	if(packetType == PACKET_SPAWN && packets->m_SentSpawnPacket)
		return;

	MessageType messageType = NetworkActor::MSGID_NETWORKACTOR_STOP;
	memcpy(&packets->m_Packets[packetType].sendBuffer[packets->m_Packets[packetType].writePos],&messageType,sizeof(messageType)); 
	packets->m_Packets[packetType].writePos += sizeof(messageType);

	if(packetType == PACKET_SPAWN || packetType == PACKET_RELIABLE)
	{
		bool isSpawn = (packetType == PACKET_SPAWN);
		memcpy(&buffer[bufferPos],&isSpawn,sizeof(isSpawn));
		bufferPos += sizeof(isSpawn);
	}

	if(packetType == PACKET_SPAWN)
	{
		packets->m_SentSpawnPacket = true;
 
		bool spawnByName = packets->m_SpawnByName;
		memcpy(&buffer[bufferPos],&spawnByName,sizeof(spawnByName));
		bufferPos += sizeof(spawnByName);

		if(spawnByName)
		{
		//copy Actor Name Hash Key
		GUID ActorNameHash = packets->m_ActorNameHash;

		if(ActorNameHash.Data1 == 0)
			Error("Server: Tried to send Actor Name Hash == 0, meaning that Name Hash was not set for Actor of type '%s'",packets->m_ClassName.c_str());

		memcpy(&buffer[bufferPos],&ActorNameHash,sizeof(ActorNameHash));
		bufferPos += sizeof(ActorNameHash);
		}
		else
		{
		//copy Actor Class Hash Key
		unsigned long ActorClassHash = Factory::GetHashKey(packets->m_ClassName);

		if(!ActorClassHash)
			Error("Server: Couldn't find Actor Class Hash Key for Actor ClassName: %s",packets->m_ClassName.c_str());

		memcpy(&buffer[bufferPos],&ActorClassHash,sizeof(ActorClassHash));
		bufferPos += sizeof(ActorClassHash);
		}
	}

	//copy Actor ID to buffer
	memcpy(&buffer[bufferPos],&packets->objectID,sizeof(packets->objectID));
	bufferPos += sizeof(packets->objectID);

	//copy synch data size to buffer
	unsigned short dataSize = packets->m_Packets[packetType].writePos;
	memcpy(&buffer[bufferPos],&dataSize,sizeof(dataSize));
	bufferPos += sizeof(dataSize);

	//copy synch data to buffer
	memcpy(&buffer[bufferPos],&packets->m_Packets[packetType].sendBuffer,dataSize);
	bufferPos += dataSize;

	if(!IsFullWorldState && bufferPos > MAX_WORLDSTATE_UPDATE)
		Error("Server: max World state-update buffer size %i exceeded, attempted buffer size %i",MAX_WORLDSTATE_UPDATE,bufferPos);
	else if(IsFullWorldState && bufferPos > MAX_WORLDSTATE_FULL)
		Error("Server: max World state-full buffer size %i exceeded, attempted buffer size %i",MAX_WORLDSTATE_FULL,bufferPos);
}

void Server::SendChat(NetworkClient* client,const char* txt, ...)
{
	va_list		argptr;
	char		msg[1024];
	va_start (argptr,txt);
	vsprintf (msg,txt,argptr);
	va_end (argptr);

	if(client == ALL_CLIENTS || client == ServerNetworkClient)
	{
		if(m_ServerCallBack_PrintConsole)
			m_ServerCallBack_PrintConsole(msg,COLOR_RGBA(255,255,255,225));
	}

	GAMEMSG_CHAT msgChat;
	msgChat.dwType = GAME_MSGID_CHAT;
	strcpy(msgChat.strChatString, msg);
	Send(client,(char*)&msgChat,sizeof(msgChat.dwType) + strlen(msgChat.strChatString)+1,RELIABLE_ORDERED);
}
void Server::SendGameChatToAll(NetworkClient* client,string chatMessage)
{
	if(!client || !chatMessage.length())
		return;

	if(chatMessage.find("/")==0)
	{
	if(m_ServerCallBack_NetworkConsoleCommand)
		m_ServerCallBack_NetworkConsoleCommand(client, chatMessage.substr(1));

	return;
	}
	
	string msg = client->m_Name + ": ";
	msg += chatMessage;

	if(m_ServerCallBack_PrintConsole)
		m_ServerCallBack_PrintConsole(msg,COLOR_RGBA(255,255,255,225));

	GAMEMSG_CHAT msgChat;
	msgChat.dwType = GAME_MSGID_CHAT;
	strcpy(msgChat.strChatString, msg.c_str());
	Send(ALL_CLIENTS,(char*)&msgChat,sizeof(msgChat.dwType) + strlen(msgChat.strChatString)+1,RELIABLE_ORDERED);
}
void Server::SendKeepAlive()
{
	m_LastKeepAliveSendTime = GSeconds;
	GAMEMSG_GENERIC msgKeepAlive;
	msgKeepAlive.dwType = GAME_MSGID_KEEPALIVE;
	Send(ALL_CLIENTS,(char*)&msgKeepAlive,sizeof(msgKeepAlive),RELIABLE_ORDERED,LOW_PRIORITY,SERVER_STREAM_KEEPALIVE);
}
void Server::EnterGame(NetworkClient* client)
{
		client->CreatePlayer();
}

void Server::LoadPreferences()
{
	m_ServerSettings.m_SessionName = Engine::Instance()->MainConfig->GetString("ServerName");
	m_ServerSettings.m_MaxPacketsPerSecond = Engine::Instance()->MainConfig->GetInt("serverMaxSendPacketsPerSecond");
	m_ServerSettings.m_PrintNetworkStats = Engine::Instance()->MainConfig->GetBool("PrintNetworkStats");
}