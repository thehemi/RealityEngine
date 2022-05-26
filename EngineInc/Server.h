//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
/// Server
/// Handles Server-side Networking, including management of clients, 
/// and distribution of accumulated World-State data
/// Author: Jeremy Stieglitz
//====================================================================================

#ifndef SERVER_INCLUDED
#define SERVER_INCLUDED
#include "NetShared.h"
#include "..\RakNet\PacketPriority.h"
#include "NetworkClient.h"

/// Stores Server Network Settings loaded from config INI
class ServerSettings
{
public:
	ServerSettings()
	{
		m_PrintNetworkStats = false;
		m_ServerPacketTimeOut = 60;
		m_MaxPacketsPerSecond = 20;
	}
	string	m_SessionName;
	DWORD	m_ServerPort;
	bool m_PrintNetworkStats;
	DWORD m_ServerPacketTimeOut;
	DWORD m_MaxPacketsPerSecond;
};

/// Game Callbacks for common Server Events, so that Game can have custom functionality on these Events
typedef void (*ServerCallBack_CreateNetworkClientPlayer)(class NetworkClient* client);
typedef void (*ServerCallBack_DeleteNetworkClientPlayer)(class NetworkClient* client);
typedef void (*ServerCallBack_ProcessGamePacket)(class NetworkClient* client, Packet* p, unsigned char packetId);
typedef World* (*ServerCallBack_NewMap)(string mapfile);
typedef void (*ServerCallBack_PrintConsole)(string text, COLOR color);
typedef void (*ServerCallBack_NetworkConsoleCommand)(class NetworkClient* client, string command);

unsigned char ENGINE_API GetPacketIdentifier(Packet *p);

/// Handles Server-side Networking, including management of clients, and distribution of accumulated World-State data
class ENGINE_API Server 
{
	friend class NetworkClient;
public:
	/// Singleton
	static Server* Instance ();

	/// Game Callbacks for common Server Events, so that Game can have custom functionality on these Events
	/// Game-Callback on validation of NetworkClient for Player avatar entry into World
	ServerCallBack_CreateNetworkClientPlayer m_ServerCallBack_CreateNetworkClientPlayer;

	/// Game-Callback on deletion of NetworkClient for Player avatar destruction from World
	ServerCallBack_DeleteNetworkClientPlayer m_ServerCallBack_DeleteNetworkClientPlayer;

	/// Game-Callback for processing of custom network messages
	ServerCallBack_ProcessGamePacket m_ServerCallBack_ProcessGamePacket;

	/// Game-Callback for server-instigated loading of new map
	ServerCallBack_NewMap m_ServerCallBack_NewMap;

	/// Game-Callback for server printing to console
	ServerCallBack_PrintConsole m_ServerCallBack_PrintConsole;

	/// Game-Callback for running network console commands
	ServerCallBack_NetworkConsoleCommand m_ServerCallBack_NetworkConsoleCommand;

	Server()
	{
		m_ServerCallBack_CreateNetworkClientPlayer = NULL;
		m_ServerCallBack_DeleteNetworkClientPlayer = NULL;
		m_ServerCallBack_ProcessGamePacket = NULL;
		m_ServerCallBack_NewMap = NULL;
		m_ServerCallBack_PrintConsole = NULL;
	}

	/// Stores INI-driven server settings
	ServerSettings m_ServerSettings;

	/// Whether Server is currently running, true for both SP and MP modes
	bool m_ServerStarted;

	/// Server can run in either SP (no actual network initialization) or MP mode
	bool m_IsMultiplayer;

	/// Names of all NetworkClients currently connected to Server as observers
	string m_ObserverNames;

	/// Returns the name of the NetworkClient with protection for NULL NetworkClient, to be used inline
	string GetNetworkClientName(NetworkClient* client);

	/// Whether the Server is started in multiplayer mode
	bool IsMultiplayer(){return m_ServerStarted && m_IsMultiplayer;}

	/// Dummy NetworkClient if non-dedicated server
	NetworkClient* ServerNetworkClient;

	/// Relays chat message from specified NetworkClient to all clients
	void SendGameChatToAll(NetworkClient* client,string chatMessage);

	/// Broadcasts server message to specified client (or all clients if NULL)
	void SendChat(NetworkClient* client,const char* txt, ...);

	/// Returns the set 'Message Of The Day'
	string GetMOTD();

	/// Initializes the starting Server state, called during Engine init
	void Initialize();

	/// Shuts down the Server, also Ends hosting if currently hosting a game.
	void Shutdown();

	/// Call after map is loaded to transmit the post-load state to currently connected NetworkClients
	void LoadedNewMap();

	/// Ends hosting of the Server, if the Server is currenltly hosting a game
	void Stop();

	/// Sends arbritrary data to the specified NetworkClient, with options for reliability priority, ordering streams, and encryption
	bool Send(NetworkClient* client, char *data, const long length, PacketReliability reliability = UNRELIABLE_SEQUENCED, PacketPriority priority = LOW_PRIORITY, char orderingStream = 0, bool secured = false);

	/// Hosts a game on the specified map, in either SP (no actual network init) or MP modes
	void BeginHosting(string SessionName, string map, bool isMultiplayer=true);

	/// Call before World Tick, for handling incoming packets before World state is updated
	void PreWorldTick();

	/// Call after World Tick, for sending outgoing packets after World state is updated
	void PostWorldTick();

	/// Prints various statistics about the network state to the log
	void PrintNetworkStats();

	/// loads server's user preferences from the ini
	void LoadPreferences();

	/// server processes request from client to enter the game
	void EnterGame(NetworkClient* client);

	~Server(){}

	/// current loaded  map pathless filename on the server, sent to clients when they join
	string m_CurrentMap;
private:
	float m_LastSentWorldUpdateTime;
	float m_LastPrintStatsTime;
	float m_LastKeepAliveSendTime;
	bool Start(string sessionName, int port = 0);
	void HandleIncomingPackets();
	void ProcessGamePacket(Packet *p, unsigned char packetIdentifier);
	void ReceiveDisconnectionNotification(Packet *p);
	void ReceiveNewNetworkClient(Packet *p);
	void ReceiveResponsiveNetworkClient(Packet *p);
	void ReceiveModifiedPacket(Packet *p);
	void ReceiveConnectionLost(Packet *p);

	/// Join a client. Uses the following functions in nested order:
	void JoinNewNetworkClient(NetworkClient* client);
	void SendGameStateToNewNetworkClient(NetworkClient* client);
	void SendWorldStateFull(NetworkClient* client);

	void WriteNetworkActorPackets(NetworkClient* client,NetworkActorPackets* packets, PacketType packetType, char* buffer, unsigned short& bufferPos, bool IsFullWorldState = false);
	void SendWorldStateUpdate();
	void SendLoadMap(NetworkClient* client);
	void SendEndGame(NetworkClient* client);
	void SendObserverNames(NetworkClient* client);
	void SendKeepAlive();

	/// Base network transmission layer
	class RakServerInterface* m_RakServer;

	//sends the server's build version # to the specified client id
	void SendBuildVersion(PlayerID playerID);
};
#endif