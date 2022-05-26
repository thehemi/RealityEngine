#ifndef CLIENT_INCLUDED
#define CLIENT_INCLUDED
#include "NetShared.h"
#include "..\RakNet\PacketPriority.h"
#include "..\RakNet\NetworkTypes.h"

/// Stores Client Network Settings loaded from config INI
class ENGINE_API ClientSettings
{
public:
	ClientSettings()
	{
		m_PrintNetworkStats = false;
		m_ClientPacketTimeOut = 60;
		m_RecievePacketsPerSecond = 15;
		m_SendPacketsPerSecond = 20;
		m_NetworkInterpolationFactor = 1;
		m_PlaybackGameRecording = false;
	}
	string	m_PlayerName;
	DWORD	m_ClientPort;
	bool m_PrintNetworkStats;
	DWORD m_ClientPacketTimeOut;
	DWORD m_RecievePacketsPerSecond;
	DWORD m_SendPacketsPerSecond;
	float m_NetworkInterpolationFactor;
	bool m_PlaybackGameRecording;
};

/// Game Callbacks for common Client Events, so that Game can have custom functionality on these Events
typedef void (*ClientCallBack_ConnectedToServer)();
typedef void (*ClientCallBack_LostServer)();
typedef void (*ClientCallBack_ProcessGamePacket)(Packet* p, unsigned char packetId);
typedef World* (*ClientCallBack_NewMap)(string mapfile);
typedef void (*ClientCallBack_PrintConsole)(string text, COLOR color);

/// Handles Client-side Networking, including connection, processing of received World state information, transmission of player inputs (handled by Game in callbacks), and recording of playback information
class ENGINE_API Client
{
public:
	static Client* Instance ();

	/// returns the latency of the Client, if currently connected, in milliseconds
	float GetLatency();

	/// Ticks the Client, processing all incoming messages
	void Tick();

	/// Sends a text chat message, handy
	void Say(string chat);

	/// Clients' INI-driven settings
	ClientSettings m_ClientSettings;

	/// Game Callbacks for common Client Events, so that Game can have custom functionality on these Events
	/// Game-Callback upon successful connection to Server
	ClientCallBack_ConnectedToServer m_ClientCallBack_ConnectedToServer;

	/// Game-Callback upon loss of Server connection
	ClientCallBack_LostServer m_ClientCallBack_LostServer;

	/// Game-Callback for processing of custom network messages
	ClientCallBack_ProcessGamePacket m_ClientCallBack_ProcessGamePacket;

	/// Game-Callback for server-instigated loading of new map
	ClientCallBack_NewMap m_ClientCallBack_NewMap;

	/// Game-Callback for Client printing to console
	ClientCallBack_PrintConsole m_ClientCallBack_PrintConsole;

	Client()
	{
		m_ClientCallBack_ConnectedToServer = NULL;
		m_ClientCallBack_LostServer = NULL;
		m_ClientCallBack_ProcessGamePacket = NULL;
		m_ClientCallBack_NewMap = NULL;
		m_ClientCallBack_PrintConsole = NULL;
		m_CurrentWorld = NULL;
	}

	/// Initializes the starting Client state, called during Engine init
	void Initialize();

	/// Disconnects the Client if currently connected
	void Disconnect();
	/// Shuts down the Client upon app closure, disconnecting if connected
	void Shutdown();

	/// Connects a Client to a Server, if exists, denoted by the IP or nameserver information in hostname
	bool ConnectToHost(string hostname);

	/// Sends arbritrary data to the Server, with options for reliability priority, and ordering streams
	bool Send(char *data, const long length, PacketReliability reliability = UNRELIABLE_SEQUENCED, PacketPriority priority = MEDIUM_PRIORITY, char orderingStream = 0);

	/// Prints various statistics about the network state to the log
	void PrintNetworkStats();

	/// Sends current Client settings (such as requested number of World state updates per second, player name) to the Server. Automatically done once by Client upon connnecting, call again if settings change.
	void SendClientSettings();

	/// is currently connected
	bool IsConnected(){return m_IsConnected;}

	void ProcessGamePacket(Packet *p, unsigned char packetIdentifier);

	int GetAveragePing();

	/// loads clients' user preferences from the ini
	void LoadPreferences();
	
private:
	class RakClientInterface* m_RakClient;
	float m_LastNetworkStatsTime;
	float m_LastConnectionTime;
	bool m_IsConnected;
	bool m_ReceivedInitialWorldState;
	void HandleIncomingPackets();
	float m_LastRecievedLatency;
	bool m_SentClientSettings;
	void ProcessWorldStatePacket(bool FullWorldState, char* buffer, int bufferSize, bool IsReliable);
	void SetConnected();
	void ReceiveServerFull(Packet *p);
	void ReceiveKickedByServer(Packet *p);
	void ReceiveEnumerationReply(Packet *p);
	void ReceiveConnectionLost(Packet *p);
	World* m_CurrentWorld;
	void WritePlaybackRecordingPacket(class NetworkActor* actor);
};
#endif