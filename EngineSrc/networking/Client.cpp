//=============================================================================
// CLIENT NETWORK
//=============================================================================
#include "stdafx.h"
#include "Client.h"
#include "GameRecord.h"
#include "GUISystem.h"
#include "..\RakNet\rakClientInterface.h"
#include "..\RakNet\RakClientInterface.h"
#include "..\RakNet\PacketEnumerations.h"
#include "..\RakNet\RakNetworkFactory.h"
#include "..\RakNet\gettime.h"
#include "classmap.h"
#include "Server.h"
#include "Editor.h"

string s_HostName;

// ------------------------------
// Client only
// ------------------------------
void Client::ReceiveServerFull(Packet *packet)
{
	// The server you tried to connect to is full.  The network code automatically disconnects you.  Any additional functionality
	// can be added here, such as telling the user the server was full
	Disconnect();

}
void Client::ReceiveKickedByServer(Packet *packet)
{
	GUISystem::Instance()->DoMessageBox("Network Event","Connection to Server lost!");
	Disconnect();
}
void Client::ReceiveEnumerationReply(Packet *packet)
{
	GUISystem::Instance()->CloseMessageBoxBlocking();
	GUISystem::Instance()->ShowDesktop(false);
	if(m_ClientCallBack_ConnectedToServer)
		m_ClientCallBack_ConnectedToServer();
	SetConnected();
}
void Client::SetConnected()
{
	m_IsConnected = true;
	m_ReceivedInitialWorldState = false;
	SendClientSettings();
}
void Client::ReceiveConnectionLost(Packet *packet)
{
	GUISystem::Instance()->DoMessageBox("Network",string("Connection to ") + s_HostName + string(" lost."));
	GUISystem::Instance()->ShowDesktop(true);
	if(m_ClientCallBack_LostServer)
		m_ClientCallBack_LostServer();
	Disconnect();
}

//================================================================================
// Update everything
//================================================================================
void Client::Tick()
{
	if(GameRecorder::Instance()->m_RecordGame)
	{
		GameRecorder::Instance()->Tick();

		for(int i = 0; i < NetworkActor::NetworkActors.size(); i++)
		{
			if(NetworkActor::NetworkActors[i]->Client_PlaybackRecordingPacket.writePos > 0)
				WritePlaybackRecordingPacket(NetworkActor::NetworkActors[i]);
		}
	}
	if(GamePlayback::Instance()->m_IsPlayingBack)
		GamePlayback::Instance()->Tick();

	if(m_RakClient)
	{
		Sleep(0);
		HandleIncomingPackets();
		Sleep(0);
		if(m_RakClient && !m_IsConnected && GSeconds - m_LastConnectionTime > 4.f)
		{
			Disconnect();
			GUISystem::Instance()->DoMessageBox("Network",string("Could not find server ") + s_HostName);
		}
	}

	if(m_IsConnected)
	{
		if(m_ClientSettings.m_PrintNetworkStats  && GSeconds - m_LastNetworkStatsTime > 2.0f)
			PrintNetworkStats();
	}
}

//================================================================================
// Process a message from the server
//================================================================================
void Client::ProcessGamePacket(Packet* p, unsigned char packetId)
{
	unsigned char* data = p->data;
	unsigned char dwMessageId = packetId;

	if(dwMessageId == ID_TIMESTAMP)
	{
		if(!m_ClientSettings.m_PlaybackGameRecording)
		{
			unsigned long sentTime = 0;
			memcpy((char*)&sentTime, p->data+1, sizeof(unsigned long));
			m_LastRecievedLatency = (RakNetGetTime() - sentTime)/1250.f;

			if(m_LastRecievedLatency > .2)
				m_LastRecievedLatency = .2;

			//assert(p->length > sizeof(unsigned char) + sizeof(unsigned long));
			dwMessageId = (unsigned char) p->data[sizeof(unsigned char) + sizeof(unsigned long)];
		}
		else
		{
			//get stored timestamp
			unsigned long latency;
			memcpy((char*)&latency, p->data+1, sizeof(unsigned long));
			m_LastRecievedLatency = latency/1000.f;
			dwMessageId = (unsigned char) p->data[sizeof(unsigned char) + sizeof(unsigned long)];
		}
	}

	switch( dwMessageId )
	{
	case GAME_MSGID_LOAD_WORLD:
		{
			GAMEMSG_LOAD_WORLD* loadWorld = (GAMEMSG_LOAD_WORLD*) data;

			m_ReceivedInitialWorldState = false;

			if(m_ClientCallBack_NewMap)
				m_CurrentWorld = m_ClientCallBack_NewMap(loadWorld->mapName);

			if(GameRecorder::Instance()->m_RecordGame)
			{
				string MapName = loadWorld->mapName;
				GameRecorder::Instance()->NewRecordFile(MapName.substr(0,MapName.size() - 4));
			}

			break;
		}
	case GAME_MSGID_BUILD_VERSION:
		{
			GAMEMSG_BUILD_VERSION* msgBuildVersion = (GAMEMSG_BUILD_VERSION*) data;	
			GUISystem::Instance()->DoMessageBox("Network","Incompatible build!\n Server " + s_HostName + " is running v" + ToStr(msgBuildVersion->buildVersion) + ",\n you have v" + ToStr(BUILD_VERSION));
			GUISystem::Instance()->ShowDesktop(true);
			Disconnect();
			break;
		}
	case GAME_MSGID_ENDGAME:
		{
			SendClientSettings();
			break;
		}
	case GAME_MSGID_OBSERVER_NAMES:
		{
			Server::Instance()->m_ObserverNames = ((GAMEMSG_OBSERVER_NAMES*)data)->observerNames;
			break;
		}
	case GAME_MSGID_WORLDSTATE_FULL:
		{
			m_LastRecievedLatency = 0;
			ProcessWorldStatePacket(true,((GAMEMSG_WORLDSTATE_UPDATE*)data)->WorldStateData,((GAMEMSG_WORLDSTATE_UPDATE*)data)->WorldStateDataSize,true);
			//pass on to game for finished loading handling
			if(m_ClientCallBack_ProcessGamePacket)
				m_ClientCallBack_ProcessGamePacket(p,dwMessageId);
			break;
		}
	case GAME_MSGID_WORLDSTATE_UPDATE_RELIABLE:
		{
			ProcessWorldStatePacket(false,((GAMEMSG_WORLDSTATE_UPDATE*)data)->WorldStateData,((GAMEMSG_WORLDSTATE_UPDATE*)data)->WorldStateDataSize,true);
			break;
		}
	case GAME_MSGID_WORLDSTATE_UPDATE_UNRELIABLE:
		{
			ProcessWorldStatePacket(false,((GAMEMSG_WORLDSTATE_UPDATE*)data)->WorldStateData,((GAMEMSG_WORLDSTATE_UPDATE*)data)->WorldStateDataSize,false);
			break;
		}
	default:
		{
			if(m_ClientCallBack_ProcessGamePacket)
				m_ClientCallBack_ProcessGamePacket(p,dwMessageId);
			break;
		}
	}

	if(GameRecorder::Instance()->m_RecordGame)
		GameRecorder::Instance()->RecordNetworkMessage(data,p->length);
}
void Client::ProcessWorldStatePacket(bool FullWorldState, char* buffer, int bufferSize, bool IsReliable)
{
	//only check for initial world state if not playing back recording
	if(!m_ClientSettings.m_PlaybackGameRecording)
	{
		//don't process world updates until client has received initial full world state
		if(FullWorldState)
			m_ReceivedInitialWorldState = true;
		else if(!m_ReceivedInitialWorldState)
			return;
	}

	int bufferPos = 0;

	while(bufferPos < bufferSize)
	{
		bool isSpawnPacket = false;

		//parse the world state data according to the reliable/unreliable toggle
		if(IsReliable)
		{
			//only reliable packets can be spawn packets, and contain this boolean value
			memcpy(&isSpawnPacket,&buffer[bufferPos],sizeof(isSpawnPacket));
			bufferPos += sizeof(isSpawnPacket);
		}

		NetworkActor* networkActor = NULL;

		if(isSpawnPacket)
		{
			bool spawnByName = false;
			memcpy(&spawnByName,&buffer[bufferPos],sizeof(spawnByName));
			bufferPos += sizeof(spawnByName);

			if(spawnByName)
			{
				//is spawn packet with instance name hash, link to an existing Actor saved in the level by looking it up based on its unique Name Hash Key
				GUID ActorGUID;
				memcpy(&ActorGUID,&buffer[bufferPos],sizeof(GUID));
				bufferPos += sizeof(GUID);

				networkActor = NULL;

				Actor* actor = m_CurrentWorld->FindActor(ActorGUID);

				if(actor && actor->IsNetworkActor())
					networkActor = (NetworkActor*)actor;

				if(!networkActor)
				{
					BYTE* buf;
					UuidToStringA(&ActorGUID,&buf);
					Error("Client: Couldn't find Actor with GUID '%s'",(char*)buf);
				}
			}
			else
			{
				//is spawn packet static class-type hash, create a new NetworkActor according to the Class Hash Key (corresponding to Class Name)
				unsigned long ActorClassHash = 0;
				memcpy(&ActorClassHash,&buffer[bufferPos],sizeof(ActorClassHash));
				bufferPos += sizeof(ActorClassHash);

				string ClassName = Factory::GetClassName(ActorClassHash);

				if(!ClassName.length())
					Error("Client: Couldn't find Actor ClassName for Hash Key #: %i",ActorClassHash);

				networkActor = (NetworkActor*)Factory::create(ClassName,m_CurrentWorld);

				if(!networkActor)
					Error("Client: Couldn't spawn Actor of type '%s'",ClassName.c_str());
			}

			NetworkActorID objectID = 0;
			memcpy(&objectID,&buffer[bufferPos],sizeof(objectID));
			bufferPos += sizeof(objectID);

			networkActor->SetID(objectID);
		}
		else
		{
			//is synch packet, find an existing NetworkActor
			NetworkActorID objectID = 0;
			memcpy(&objectID,&buffer[bufferPos],sizeof(objectID));
			bufferPos += sizeof(objectID);

			networkActor = NetworkActor::GET_OBJECT_FROM_ID(objectID);

			if(!networkActor && IsReliable)
			{
				NetworkActor::PrintAllNetworkActors();
				Error("Couldn't find Reliable-Packet NetworkActor for ID %i",objectID);
			}
		}

		//copy synch data size
		unsigned short dataSize = 0;
		memcpy(&dataSize,&buffer[bufferPos],sizeof(dataSize));
		bufferPos += sizeof(dataSize);

		//pass the offsetted buffer to the networkActor where it will process it 
		//until it has reached a stop point or (should never happen) exceeded the data size
		if(networkActor)
			networkActor->Client_ProcessNetworkBuffer(&buffer[bufferPos],dataSize,isSpawnPacket);

		bufferPos += dataSize;
	}
}
void Client::Initialize()
{
	m_RakClient = 0;
	m_IsConnected = false;
	m_LastNetworkStatsTime = 0;
	m_LastConnectionTime = 0;
	m_LastRecievedLatency = 0;
	LoadPreferences();
}
void Client::LoadPreferences()
{
	m_ClientSettings.m_RecievePacketsPerSecond = Engine::Instance()->MainConfig->GetInt("clientRecievePacketsPerSecond");
	m_ClientSettings.m_SendPacketsPerSecond = Engine::Instance()->MainConfig->GetInt("clientSendPacketsPerSecond");
	m_ClientSettings.m_NetworkInterpolationFactor = Engine::Instance()->MainConfig->GetFloat("ClientInterpolationFactor");
	m_ClientSettings.m_PlayerName = Engine::Instance()->MainConfig->GetString("PlayerName");
	m_ClientSettings.m_PrintNetworkStats = Engine::Instance()->MainConfig->GetBool("PrintNetworkStats");
	GameRecorder::Instance()->m_RecordGame = Engine::Instance()->MainConfig->GetBool("RecordGame");
	if(Editor::Instance()->GetEditorMode())
		GameRecorder::Instance()->m_RecordGame = false;
}
void Client::Disconnect()
{
	GamePlayback::Instance()->ShutDown();
	GameRecorder::Instance()->ShutDown();
	if(m_IsConnected)
	{
		if(m_RakClient)
		{
			m_RakClient->Disconnect(2500);
			Sleep(0); //give RakNet thread a chance to shut down
		}

		m_IsConnected = false;
	}
	if(m_RakClient)
	{
		RakNetworkFactory::DestroyRakClientInterface(m_RakClient);
		m_RakClient = 0;
	}
	m_LastRecievedLatency = 0;
}

void Client::Say(string chat)
{
	if(!chat.size()) return;
	if(Server::Instance()->m_ServerStarted)
	{
		Server::Instance()->SendGameChatToAll(Server::Instance()->ServerNetworkClient,chat);
		return;
	}
	if(!IsConnected())
		return;

	GAMEMSG_CHAT msgChat;
	msgChat.dwType = GAME_MSGID_CHAT;
	strcpy(msgChat.strChatString,chat.c_str());
	Send((char*)&msgChat,sizeof(msgChat.dwType) + strlen(msgChat.strChatString) + 1,RELIABLE_ORDERED);
}
void Client::SendClientSettings()
{
	if(!IsConnected())
		return;

	GAMEMSG_CLIENT_SETTINGS_INFO settingsInfo;
	settingsInfo.dwType = GAME_MSGID_CLIENT_SETTINGS_INFO;
	settingsInfo.RecievePacketsPerSeconds = m_ClientSettings.m_RecievePacketsPerSecond;
	settingsInfo.buildVersion = BUILD_VERSION;
	strcpy(settingsInfo.playerName,m_ClientSettings.m_PlayerName.c_str());
	Send((char*)&settingsInfo,sizeof(settingsInfo) - sizeof(settingsInfo.playerName) + strlen(settingsInfo.playerName) + 1 ,RELIABLE_ORDERED);
}

bool Client::Send(char *data, const long length, PacketReliability reliability, PacketPriority priority , char orderingStream)
{
	if(!IsConnected())
		return false;

	if(reliability == UNRELIABLE_SEQUENCED && orderingStream == 0)
		Error("Debug warning -- you're trying to filter out multiple messages on a single ordering stream");

	bool b = m_RakClient->Send(data, length, priority, reliability, 0);
	//if(!b)
	//	Error("Client send failed");
	return b;
}

void Client::HandleIncomingPackets()
{
	// Direct native packets
	Packet* p;
	unsigned char packetIdentifier;

	// Get a packet from either the server or the client
	p = m_RakClient->Receive();
	while (p)
	{
		packetIdentifier = GetPacketIdentifier(p);
		// Check if this is a native packet
		switch(packetIdentifier)
		{
			// ------------------------------
			// Client only
			// ------------------------------

		case ID_CONNECTION_REQUEST_ACCEPTED:
			break;

		case ID_NO_FREE_INCOMING_CONNECTIONS:
			ReceiveServerFull(p);
			break;

		case ID_DISCONNECTION_NOTIFICATION:
			ReceiveKickedByServer(p);
			break;

		case ID_RECEIVED_STATIC_DATA:
			ReceiveEnumerationReply(p);
			break;

		case ID_CONNECTION_LOST:
			ReceiveConnectionLost(p);
			break;

		default:
			// If not a native packet send it to ProcessGamePacket which should have been written by the user
			ProcessGamePacket(p, packetIdentifier);
			break;
		}
		if(m_RakClient)
		{
			m_RakClient->DeallocatePacket(p);
			p = m_RakClient->Receive();
		}
		else 
		{
			delete p;
			p = NULL;
		}
	}
}

void couldNotConnect(void* waitbox)
{
	GUISystem::Instance()->CloseMessageBoxBlocking();
	GUISystem::Instance()->DoMessageBox("Connection error","Could not find Server at that address.");
	Client::Instance()->Disconnect();
}

bool Client::ConnectToHost( string hostName)
{
	Server::Instance()->Stop();

	Disconnect();

	if(hostName.length() > 4 && hostName.substr(hostName.size() - 4) == ".rec" && GamePlayback::Instance()->Playback(hostName))
	{
		m_ClientSettings.m_PlaybackGameRecording = true;
		return true;
	}

	m_ClientSettings.m_PlaybackGameRecording = false;

	if(!m_RakClient)
		m_RakClient = RakNetworkFactory::GetRakClientInterface();

	// Parse out port if it exists (expected form of "xxx.xxx.xxx.xxx:port")
	DWORD serverPort = Engine::Instance()->MainConfig->GetInt("DefaultServerPort");

	int portPos = hostName.find(":");

	if(portPos != -1)
	{
		serverPort = atoi(hostName.substr(portPos+1).c_str()); // Get port
		hostName = hostName.substr(0,portPos); // Remove :port from string
	}

	int clientPort = Engine::Instance()->MainConfig->GetInt("DefaultClientPort");

	// Get exeSize for validation
	//long exeSize = 82383;

	/*unsigned long compressionFrequencyTable[256];
	g_Game.resourceTables.getClientFrequencyTable((unsigned long*)&compressionFrequencyTable);
	bool generated = rakClient->GenerateCompressionLayer(compressionFrequencyTable,true);
	unsigned long decompressionFrequencyTable[256];
	g_Game.resourceTables.getServerFrequencyTable((unsigned long*)&decompressionFrequencyTable);
	generated = rakClient->GenerateCompressionLayer(decompressionFrequencyTable,false);*/

	m_RakClient->Connect((char*)hostName.c_str(), serverPort, clientPort, 8234, 15);
	m_LastConnectionTime = GSeconds;

	s_HostName = hostName;

	GUISystem::Instance()->DoMessageBoxBlocking("Network",string("Connecting to ") + s_HostName,4.0,couldNotConnect);

	return true;
}

unsigned int previousClientTotalBitsSent = 0;
unsigned int previousClientTotalBitsReceived = 0;
void Client::PrintNetworkStats()
{
	RakNetStatisticsStruct* averaged = m_RakClient->GetStatistics();

	unsigned int bytesPerSecondSent = 0;
	unsigned int bytesPerSecondReceived = 0;

	if(previousClientTotalBitsSent != 0)
		bytesPerSecondSent = ((averaged->totalBitsSent - previousClientTotalBitsSent)/8)/(GSeconds - m_LastNetworkStatsTime);

	if(previousClientTotalBitsReceived != 0)
		bytesPerSecondReceived = ((averaged->bitsReceived - previousClientTotalBitsReceived)/8)/(GSeconds - m_LastNetworkStatsTime);

	previousClientTotalBitsSent = averaged->totalBitsSent;
	previousClientTotalBitsReceived = averaged->bitsReceived;

	// Network statistics.
	LogPrintf(" ");
	LogPrintf(" ");
	LogPrintf("----- CLIENT NETWORK STATS TIME: %f -----",GSeconds);
	LogPrintf("- Bytes Per Second Sent (since last stat): %d",bytesPerSecondSent);
	LogPrintf("- Bytes Per Received (since last stat): %d",bytesPerSecondReceived);
	LogPrintf("- Packets Sent: %d",averaged->messagesSent);
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

	m_LastNetworkStatsTime = GSeconds;
}

void Client::Shutdown()
{
	Disconnect();
}
float Client::GetLatency()
{
	return m_LastRecievedLatency;
}
// Returns a singleton instance
Client* Client::Instance () 
{
	static Client inst;
	return &inst;
}

void Client::WritePlaybackRecordingPacket(NetworkActor* actor)
{
	MessagePacket* packet = &actor->Client_PlaybackRecordingPacket;

	if(packet->writePos <= 0)
		return;

	GAMEMSG_WORLDSTATE_UPDATE msg;
	msg.dwType = GAME_MSGID_WORLDSTATE_UPDATE_UNRELIABLE;
	msg.useTimeStamp = ID_TIMESTAMP;
	msg.timeStamp = 0;
	
	char* buffer = msg.WorldStateData;
	int bufferPos = 0;

	MessageType messageType = NetworkActor::MSGID_NETWORKACTOR_STOP;
	memcpy(&packet->sendBuffer[packet->writePos],&messageType,sizeof(messageType)); 
	packet->writePos += sizeof(messageType);

	//copy Actor ID to buffer
	memcpy(&buffer[bufferPos],&actor->objectID,sizeof(actor->objectID));
	bufferPos += sizeof(actor->objectID);

	//copy synch data size to buffer
	unsigned short dataSize = packet->writePos;
	memcpy(&buffer[bufferPos],&dataSize,sizeof(dataSize));
	bufferPos += sizeof(dataSize);

	//copy synch data to buffer
	memcpy(&buffer[bufferPos],&packet->sendBuffer,dataSize);
	bufferPos += dataSize;

	msg.WorldStateDataSize = bufferPos;

	GameRecorder::Instance()->RecordNetworkMessage((unsigned char*)&msg,sizeof(msg) - sizeof(msg.WorldStateData) + msg.WorldStateDataSize);

	packet->writePos = 0;
}

int Client::GetAveragePing()
{
if(m_RakClient)
	return m_RakClient->GetAveragePing();
else
	return -1;
}