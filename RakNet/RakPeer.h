// This file is part of RakNet Copyright 2003, 2004 Rakkarsoft LLC and Kevin Jenkins.

// Usage of Raknet is subject to the appropriate licence agreement.
// "Shareware" Licensees with Rakkarsoft LLC are subject to the shareware license found at http://www.rakkarsoft.com/shareWareLicense.html which you agreed to upon purchase of a "Shareware license"
// "Commercial" Licensees with Rakkarsoft LLC are subject to the commercial license found at http://www.rakkarsoft.com/sourceCodeLicense.html which you agreed to upon purchase of a "Commercial license"
// All other users are subject to the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

// Refer to the appropriate license agreement for distribution, modification, and warranty rights.

#ifndef __RAK_PEER_H
#define __RAK_PEER_H

#include "ReliabilityLayer.h"
#include "RakPeerInterface.h"
#include "BinarySearchTree.h"
#include "RPCNode.h"
#include "RSACrypt.h"
#include "BitStream.h"

class HuffmanEncodingTree;

class RakPeer : public RakPeerInterface
{
public:
	// Constructor
	RakPeer();

	// Destructor
	~RakPeer();

	// --------------------------------------------------------------------------------------------
	// Major Low Level Functions - Functions needed by most users
	// --------------------------------------------------------------------------------------------

	// Description:
	// Starts the network threads, opens the listen port
	// You must call this before calling SetMaximumIncomingConnections or Connect
	// Multiple calls while already active are ignored.  To call this function again with different settings, you must first call Disconnect()
	// To accept incoming connections, use SetMaximumIncomingConnections
	//
	// Parameters:
	// MaximumNumberOfPeers:  Required so the network can preallocate and for thread safety.
	// - A pure client would set this to 1.  A pure server would set it to the number of allowed clients.
	// - A hybrid would set it to the sum of both types of connections
	// localPort: The port to listen for connections on.
	// _threadSleepTimer: <0 for don't sleep, >=0 for how many ms to Sleep each internal update cycle (recommended you start with 15, try different values)
	//
	// Returns:
	// False on failure (can't create socket or thread), true on success.
	bool Initialize(unsigned short MaximumNumberOfPeers, unsigned short localPort,int _threadSleepTimer);

	// Description:
	// Must be called while offline
	// Secures connections though a combination of SHA1, AES128, SYN Cookies, and RSA to prevent
	// connection spoofing, replay attacks, data eavesdropping, packet tampering, and MitM attacks.
	// There is a significant amount of processing and a slight amount of bandwidth
	// overhead for this feature.
	//
	// If you accept connections, you must call this or else secure connections will not be enabled
	// for incoming connections.
	// If you are connecting to another system, you can call this with values for the
	// (e and p,q) public keys before connecting to prevent MitM
	//
	// Parameters:
	// RSAe, RSAn - Public keys generated from the RSACrypt class.  See the Encryption sample
	// RSAp, RSAq - A pointer to the private keys from the RSACrypt class.  See the Encryption sample
	// If the private keys are 0, then a new key will be generated when this function is called
	void InitializeSecurity(char *RSAe, char *RSAn, char *RSAp, char *RSAq);

	// Description
	// Must be called while offline
	// Disables all security.
	void DisableSecurity(void);

	// Description:
	// Sets how many incoming connections are allowed.  If this is less than the number of players currently connected, no
	// more players will be allowed to connect.  If this is greater than the maximum number of peers allowed, it will be reduced
	// to the maximum number of peers allowed.
	//
	// Parameters:
	// numberAllowed - Maximum number of incoming connections allowed.
	void SetMaximumIncomingConnections(unsigned short numberAllowed);

	// Description:
	// Returns the maximum number of incoming connections, which is always <= MaximumNumberOfPeers
	unsigned short GetMaximumIncomingConnections(void) const;

	// Description:
	// Sets the password incoming connections must match in the call to Connect (defaults to none)
	// Pass 0 to passwordData to specify no password
	//
	// Parameters:
	// passwordData: A data block that incoming connections must match.  This can be just a password, or can be a stream of data.
	// - Specify 0 for no password data
	// passwordDataLength: The length in bytes of passwordData
	void SetIncomingPassword(char* passwordData, int passwordDataLength);

	// Description:
	// Returns the password set by SetIncomingPassword in a BitStream
	RakNet::BitStream *GetIncomingPassword(void);

	// Description:
	// Call this to connect to the specified host (ip or domain name) and server port.
	// Calling Connect and not calling SetMaximumIncomingConnections acts as a dedicated client.  Calling both acts as a true peer.
	// This is a non-blocking connection.  You know the connection is successful when IsConnected() returns true
	// or receive gets a packet with the type identifier ID_CONNECTION_ACCEPTED.  If the connection is not
	// successful, such as rejected connection or no response then neither of these things will happen.
	// Requires that you first call Initialize
	//
	// Parameters:
	// host: Either a dotted IP address or a domain name
	// remotePort: Which port to connect to on the remote machine.
	// passwordData: A data block that must match the data block on the server.  This can be just a password, or can be a stream of data
	// passwordDataLength: The length in bytes of passwordData
	//
	// Returns:
	// True on successful initiation. False on incorrect parameters, internal error, or too many existing peers
	bool Connect(char* host, unsigned short remotePort, char* passwordData, int passwordDataLength);

	// Description:
	// Stops the network threads and close all connections.  Multiple calls are ok.
	//
	// Parameters:
	// blockDuration: How long you should wait for all remaining packets to go out, per connected system
	// If you set it to 0 then the disconnection notification probably won't arrive
	void Disconnect(unsigned long blockDuration);

	// Description:
	// Returns true if the network threads are running
	bool IsActive(void) const;

	// Description:
	// Fills the array remoteSystems with the playerID of all the systems we are connected to
	//
	// Parameters:
	// remoteSystems (out): An array of PlayerID structures to be filled with the PlayerIDs of the systems we are connected to
	// - pass 0 to remoteSystems to only get the number of systems we are connected to
	// numberOfSystems (int, out): As input, the size of remoteSystems array.  As output, the number of elements put into the array 
	bool GetConnectionList(PlayerID *remoteSystems, unsigned short *numberOfSystems) const;

	// Description:
	// Sends a block of data to the specified system that you are connected to.
	// This function only works while the client is connected (Use the Connect function).
	//
	// Parameters:
	// data: The block of data to send
	// length: The size in bytes of the data to send
	// bitStream: The bitstream to send
	// priority: What priority level to send on.
	// reliability: How reliability to send this data
	// orderingStream: When using ordered or sequenced packets, what stream to order these on.
	// - Packets are only ordered relative to other packets on the same stream
	// playerId: Who to send this packet to, or in the case of broadcasting who not to send it to.  Use UNASSIGNED_PLAYER_ID to specify none
	// broadcast: True to send this packet to all connected systems.  If true, then playerId specifies who not to send the packet to.
	// Returns:
	// False if we are not connected to the specified recipient.  True otherwise
	bool Send(char *data, const long length, PacketPriority priority, PacketReliability reliability, char orderingStream, PlayerID playerId, bool broadcast);
	bool Send(RakNet::BitStream * bitStream, PacketPriority priority, PacketReliability reliability, char orderingStream, PlayerID playerId, bool broadcast);

	// Description:
	// Gets a packet from the incoming packet queue. Use DeallocatePacket to deallocate the packet after you are done with it.
	// Check the Packet struct at the top of CoreNetworkStructures.h for the format of the struct
	//
	// Returns:
	// 0 if no packets are waiting to be handled, otherwise an allocated packet
	// If the client is not active this will also return 0, as all waiting packets are flushed when the client is Disconnected
	// This also updates all memory blocks associated with synchronized memory and distributed objects
	Packet* Receive(void);

	// Description:
	// Call this to deallocate a packet returned by Receive when you are done handling it.
	void DeallocatePacket(Packet *packet);

	// Description:
	// Return the total number of connections we are allowed
	unsigned short GetMaximumNumberOfPeers(void) const;

	// --------------------------------------------------------------------------------------------
	// Remote Procedure Call Functions - Functions to initialize and perform RPC
	// --------------------------------------------------------------------------------------------

	// Description:
	// Register a C function as available for calling as a remote procedure call
	//
	// Parameters:
	// uniqueID: A null terminated non-case senstive string of only letters to identify this procedure
	// functionName(...): The name of the C function or C++ singleton to be used as a function pointer
	// This can be called whether the client is active or not, and registered functions stay registered unless unregistered with
	// UnregisterAsRemoteProcedureCall
	void RegisterAsRemoteProcedureCall(char* uniqueID, void (*functionName)(char *input, int numberOfBitsOfData, PlayerID sender));

	// Description:
	// Unregisters a C function as available for calling as a remote procedure call that was formerly registered
	// with RegisterAsRemoteProcedureCall
	//
	// Parameters:
	// uniqueID: A null terminated non-case senstive string of only letters to identify this procedure.  Must match the parameter
	// passed to RegisterAsRemoteProcedureCall
	void UnregisterAsRemoteProcedureCall(char* uniqueID);

	// Description:
	// Calls a C function on the server that the server already registered using RegisterAsRemoteProcedureCall
	// If you want that function to return data you should call RPC from that system in the same way
	// Returns true on a successful packet send (this does not indicate the recipient performed the call), false on failure
	//
	// Parameters:
	// uniqueID: A null terminated non-case senstive string of only letters to identify this procedure.  Must match the parameter
	// data: The block of data to send
	// length: The size in BITS of the data to send
	// bitStream: The bitstream to send
	// priority: What priority level to send on.
	// reliability: How reliability to send this data
	// orderingStream: When using ordered or sequenced packets, what stream to order these on.
	// broadcast - Send this packet to everyone.
	// playerId: Who to send this packet to, or in the case of broadcasting who not to send it to.  Use UNASSIGNED_PLAYER_ID to specify none
	// broadcast: True to send this packet to all connected systems.  If true, then playerId specifies who not to send the packet to.
	// shiftTimestamp: True to treat the first 4 bytes as a timestamp and make it system relative on arrival (Same as ID_TIMESTAMP for a packet enumeration type)
	bool RPC(char* uniqueID, char *data, long bitLength, PacketPriority priority, PacketReliability reliability, char orderingStream, PlayerID playerId, bool broadcast, bool shiftTimestamp);
	bool RPC(char* uniqueID, RakNet::BitStream *bitStream, PacketPriority priority, PacketReliability reliability, char orderingStream, PlayerID playerId, bool broadcast, bool shiftTimestamp);

	// --------------------------------------------------------------------------------------------
	// Player Management Functions
	// --------------------------------------------------------------------------------------------
	// Description:
	// Close the connection to another host (if we initiated the connection it will disconnect, if they did it will kick them out).
	//
	// Parameters:
	// target: Which connection to close
	// sendDisconnectionNotification: True to send ID_DISCONNECTION_NOTIFICATION to the recipient.  False to close it silently.
	// blockDuration: How long you should wait for all remaining packets to go out, per connected system
	// If you set it to 0 then the disconnection notification probably won't arrive
	void CloseConnection(PlayerID target, bool sendDisconnectionNotification, unsigned long blockDuration);

	// Description:
	// Given a playerID, returns an index from 0 to the maximum number of players allowed - 1.
	//
	// Parameters
	// playerId - The playerID to search for
	//
	// Returns
	// An integer from 0 to the maximum number of peers -1, or -1 if that player is not found
	int GetIndexFromPlayerID(PlayerID playerId);

	// Description:
	// This function is only useful for looping through all players.
	//
	// Parameters
	// index - an integer between 0 and the maximum number of players allowed - 1.
	//
	// Returns
	// A valid playerID or UNASSIGNED_PLAYER_ID if no such player at that index
	PlayerID GetPlayerIDFromIndex(int index);

	// Description:
	// Bans an IP from connecting.  Banned IPs persist between connections.
	//
	// Parameters
	// IP - Dotted IP address.  Can use * as a wildcard, such as 128.0.0.* will ban
	// All IP addresses starting with 128.0.0
	void AddToBanList(const char *IP);

	// Description:
	// Allows a previously banned IP to connect.
	//
	// Parameters
	// IP - Dotted IP address.  Can use * as a wildcard, such as 128.0.0.* will ban
	// All IP addresses starting with 128.0.0
	void RemoveFromBanList(const char *IP);

	// Description:
	// Allows all previously banned IPs to connect.
	void ClearBanList(void);

	// Description:
	// Determines if a particular IP is banned.
	//
	// Parameters
	// IP - Complete dotted IP address
	//
	// Returns
	// True if IP matches any IPs in the ban list, accounting for any wildcards.
	// False otherwise.
	bool IsBanned(const char *IP);

	// --------------------------------------------------------------------------------------------
	// Pinging Functions - Functions dealing with the automatic ping mechanism
	// --------------------------------------------------------------------------------------------

	// Description:
	// Send a ping to the specified connected system.
	//
	// Parameters:
	// target - who to ping
	void Ping(PlayerID target);

	// Description:
	// Send a ping to the specified unconnected system.
	// The remote system, if it is Initialized, will respond with ID_PONG.
	// The final ping time will be encoded in the following 4 bytes (2-5) as an unsigned long
	//
	// Requires:
	// The sender and recipient must already be started via a successful call to Initialize
	//
	// Parameters:
	// host: Either a dotted IP address or a domain name
	// remotePort: Which port to connect to on the remote machine.
	void Ping(char* host, unsigned short remotePort);

	// Description:
	// Returns the average of all ping times read for a specified target
	//
	// Parameters:
	// target - whose time to read
	int GetAveragePing(PlayerID target);

	// Description:
	// Returns the last ping time read for the specific player or -1 if none read yet
	//
	// Parameters:
	// target - whose time to read
	int GetLastPing(PlayerID target) const;

	// Description:
	// Returns the lowest ping time read or -1 if none read yet
	//
	// Parameters:
	// target - whose time to read
	int GetLowestPing(PlayerID target) const;

	// Description:
	// Ping the remote systems every so often.  This is off by default
	// This will work anytime
	//
	// Parameters:
	// doPing - True to start occasional pings.  False to stop them.
	void SetOccasionalPing(bool doPing);

	// --------------------------------------------------------------------------------------------
	// Synchronized Memory Functions - Functions dealing with synchronized memory
	// --------------------------------------------------------------------------------------------

	/*
	// Description:
	// Automatically synchronizes a block of memory between systems.
	// Can be called anytime.  Calling it before a connection is initiated will cause the data to be synchronized on connection
	//
	// Parameters:
	// uniqueIdentifier: an integer (enum) corresponding to the same variable between clients and the server.  Start the indexing at 0
    // memoryBlock: Pointer to the data you want to read from or write to
	// size: Size of memoryBlock in bytes
	// isAuthority: True to tell all connected systems to match their data to yours.  Data changes are relayed to the authoritative
	// - client which broadcasts the change
	// synchronizationRules: Optional function pointer that decides whether or not to update changed memory.  It should
	// - return true if the two passed memory blocks are sufficiently different to synchronize them.  This is an optimization so
	// - data that changes rapidly, such as per-frame, can be made to not update every frame
	// - The first parameter to synchronizationRules is the new data, the second is the internal copy of the old data
	// secondaryUniqueIdentifier:  Optional and used when you have the same unique identifier and is intended for multiple instances of a class
	// - that derives from NetworkObject.
	void SynchronizeMemory(UniqueIDType uniqueIdentifier, char *memoryBlock, unsigned short size, bool isAuthority, bool (*synchronizationRules) (char*,char*)=0,ObjectID secondaryUniqueIdentifier=UNASSIGNED_OBJECT_ID);

	// Description:
	// Stops synchronization of a block of memory previously defined by uniqueIdentifier and secondaryUniqueIdentifier
	// by the call to SynchronizeMemory
	// CALL THIS BEFORE SYNCHRONIZED MEMORY IS DEALLOCATED!
	// It is not necessary to call this before disconnecting, as all synchronized states will be released then.
	// Parameters:
	// uniqueIdentifier: an integer (enum) corresponding to the same variable between clients and the server.  Start the indexing at 0
	// secondaryUniqueIdentifier:  Optional and used when you have the same unique identifier and is intended for multiple instances of a class
	// - that derives from NetworkObject.
	void DesynchronizeMemory(UniqueIDType uniqueIdentifier, ObjectID secondaryUniqueIdentifier=UNASSIGNED_OBJECT_ID);

	// Description:
	// Desynchronizes all synchronized memory
	void DesynchronizeAllMemory(void);
	*/


	// --------------------------------------------------------------------------------------------
	// Static Data Functions - Functions dealing with API defined synchronized memory
	// --------------------------------------------------------------------------------------------

	// Description:
	// All systems have a block of data associated with them, for user use.  This block of data can be used to easily
	// specify typical system data that you want to know on connection, such as the player's name.
	//
	// Parameters:
	// playerId: Which system you are referring to.  Pass the value returned by GetInternalID to refer to yourself
	//
	// Returns:
	// The data passed to SetRemoteStaticData stored as a bitstream
	RakNet::BitStream * GetRemoteStaticData(PlayerID playerId);

	// Description:
	// All systems have a block of data associated with them, for user use.  This block of data can be used to easily
	// specify typical system data that you want to know on connection, such as the player's name.
	//
	// Parameters:
	// playerId: Whose static data to change.  Use your own playerId to change your own static data
	// data: a block of data to store
	// length: The length of data in bytes	
	void SetRemoteStaticData(PlayerID playerId, char *data, const long length);

	// Description:
	// Sends your static data to the specified system.  This is automatically done on connection.
	// You should call this when you change your static data.
	// To send the static data of another system (such as relaying their data) you should do this normally with Send
	//
	// Parameters:
	// target: Who to send your static data to.  Specify UNASSIGNED_PLAYER_ID to broadcast to all
	void SendStaticData(PlayerID target);

	// --------------------------------------------------------------------------------------------
	// Network Functions - Functions dealing with the network in general
	// --------------------------------------------------------------------------------------------

	// Description:
	// Return the unique address identifier that represents you on the the network and is based on your local IP / port
	// Note that unlike in previous versions, this is a struct and is not sequential
	PlayerID GetInternalID(void) const;

	// Description:
	// Return the unique address identifier that represents you on the the network and is based on your external
	// IP / port (the IP / port the specified player uses to communicate with you)
	// Note that unlike in previous versions, this is a struct and is not sequential
	//
	// Parameters:
	// target: Which remote system you are referring to for your external ID
	PlayerID GetExternalID(PlayerID target) const;

	// Description:
	// Change the MTU size in order to improve performance when sending large packets
	// This can only be called when not connected.
	// A too high of value will cause packets not to arrive at worst and be fragmented at best.
	// A too low of value will split packets unnecessarily.
	//
	// Parameters:
	// size: Set according to the following table:
	// 1500. The largest Ethernet packet size; it is also the default value.
	// This is the typical setting for non-PPPoE, non-VPN connections. The default value for NETGEAR routers, adapters and switches. 
	// 1492. The size PPPoE prefers. 
	// 1472. Maximum size to use for pinging. (Bigger packets are fragmented.) 
	// 1468. The size DHCP prefers. 
	// 1460. Usable by AOL if you don't have large email attachments, etc. 
	// 1430. The size VPN and PPTP prefer. 
	// 1400. Maximum size for AOL DSL. 
	// 576. Typical value to connect to dial-up ISPs. (Default)
	//
	// Returns:
	// False on failure (we are connected).  True on success.  Maximum allowed size is MAXIMUM_MTU_SIZE
	bool SetMTUSize(int size);

	// Description:
	// Returns the current MTU size
	//
	// Returns:
	// The MTU sized specified in SetMTUSize
	int GetMTUSize(void) const;

	// Description:
	// Returns the number of IP addresses we have
	unsigned GetNumberOfAddresses(void);

	// Description:
	// Returns the dotted IP address for the specified playerId
	//
	// Parameters:
	// playerId - Any player ID other than UNASSIGNED_PLAYER_ID, even if that player is not currently connected
	const char* PlayerIDToDottedIP(PlayerID playerId) const;

	// Description:
	// Returns an IP address at index 0 to GetNumberOfAddresses-1
	const char* GetLocalIP(unsigned int index);

	// --------------------------------------------------------------------------------------------
	// Compression Functions - Functions related to the compression layer
	// --------------------------------------------------------------------------------------------

	// Description:
	// Enables or disables our tracking of bytes input to and output from the network.
	// This is required to get a frequency table, which is used to generate a new compression layer.
	// You can call this at any time - however you SHOULD only call it when disconnected.  Otherwise you will only track
	// part of the values sent over the network.
	// This value persists between connect calls and defaults to false (no frequency tracking)
	// 
	// Parameters:
	// doCompile - true to track bytes.  Defaults to false
	void SetCompileFrequencyTable(bool doCompile);

	// Description:
	// Returns the frequency of outgoing bytes into outputFrequencyTable
	// The purpose is to save to file as either a master frequency table from a sample game session for passing to
	// GenerateCompressionLayer(false);
	// You should only call this when disconnected.
	// Requires that you first enable data frequency tracking by calling SetCompileFrequencyTable(true)
	//
	// Parameters:
	// outputFrequencyTable (out): The frequency of each corresponding byte
	//
	// Returns:
	// Ffalse (failure) if connected or if frequency table tracking is not enabled.  Otherwise true (success)
	bool GetOutgoingFrequencyTable(unsigned long outputFrequencyTable[256]);

	// Description:
	// Generates the compression layer from the input frequency table.
	// You should call this twice - once with inputLayer as true and once as false.
	// The frequency table passed here with inputLayer=true should match the frequency table on the recipient with inputLayer=false.
	// Likewise, the frequency table passed here with inputLayer=false should match the frequency table on the recipient with inputLayer=true
	// Calling this function when there is an existing layer will overwrite the old layer
	// You should only call this when disconnected
	//
	// Parameters:
	// inputFrequencyTable: The frequency table returned from GetSendFrequencyTable(...);
	// inputLayer - Whether inputFrequencyTable represents incoming data from other systems (true) or outgoing data from this system (false)
	//
	// Returns:
	// False on failure (we are connected).  True otherwise
	bool GenerateCompressionLayer(unsigned long inputFrequencyTable[256], bool inputLayer);

	// Description:
	// Deletes the output or input layer as specified.  This is not necessary to call and is only valuable for freeing memory
	// You should only call this when disconnected
	//
	// Parameters:
	// inputLayer - Specifies the corresponding compression layer generated by GenerateCompressionLayer.
	//
	// Returns:
	// False on failure (we are connected).  True otherwise
	bool DeleteCompressionLayer(bool inputLayer);

	// Returns:
	// The compression ratio.  A low compression ratio is good.  Compression is for outgoing data
	float GetCompressionRatio(void) const;

	// Returns:
	// The decompression ratio.  A high decompression ratio is good.  Decompression is for incoming data
	float GetDecompressionRatio(void) const;

	// --------------------------------------------------------------------------------------------
	// Micellaneous Functions
	// --------------------------------------------------------------------------------------------

	// Description:
	// Returns the data you passed to the passwordData parameter in Connect
	//
	// Parameters
	// passwordData (out): Should point to a block large enough to hold the password data you passed to Connect
	// passwordDataLength (in, out): Maximum size of the array passwordData.  Modified to hold the number of bytes actually written
	void GetPasswordData(char *passwordData, int *passwordDataLength);

	// Description:
	// Put a packet back at the end of the receive queue in case you don't want to deal with it immediately
	//
	// Parameters
	// packet: The packet you want to push back.
	void PushBackPacket(Packet *packet);


	// --------------------------------------------------------------------------------------------
	// Statistical Functions - Functions dealing with API performance
	// --------------------------------------------------------------------------------------------

	// Description:
	// Returns a structure containing a large set of network statistics for the specified system
	// You can map this data to a string using the C style StatisticsToString function
	//
	// Parameters
	// playerId: Which connected system to get statistics for
	//
	// Returns:
	// 0 on can't find the specified system.  A pointer to a set of data otherwise.
	RakNetStatisticsStruct * const GetStatistics(PlayerID playerId);

protected:

#ifdef _WIN32
//	friend unsigned __stdcall RecvFromNetworkLoop(LPVOID arguments);
	friend void __stdcall ProcessNetworkPacket(unsigned long binaryAddress, unsigned short port, char *data, int length, RakPeer *rakPeer);
	friend unsigned __stdcall UpdateNetworkLoop(LPVOID arguments);
#else
//	friend void*  RecvFromNetworkLoop( void*  arguments );
	friend void ProcessNetworkPacket(unsigned long binaryAddress, unsigned short port, char *data, int length, RakPeer *rakPeer);
	friend void*  UpdateNetworkLoop( void*  arguments );
#endif

	struct  PingAndClockDifferential {
		short pingTime;
		unsigned long clockDifferential;
	};

	struct RemoteSystemStruct
	{
		PlayerID playerId; // The remote system associated with this reliability layer
		PlayerID myExternalPlayerId; // Your own IP, as reported by the remote system
		ReliabilityLayer reliabilityLayer; // The reliability layer associated with this player
		bool weInitiatedTheConnection; // True if we started this connection via Connect.  False if someone else connected to us.
		PingAndClockDifferential pingAndClockDifferential[PING_TIMES_ARRAY_SIZE]; // last x ping times and calculated clock differentials with it
		int pingAndClockDifferentialWriteIndex; // The index we are writing into the pingAndClockDifferential circular buffer
		int lowestPing;
		unsigned long nextPingTime; // When to next ping this player
		RakNet::BitStream staticData;
		unsigned long connectionTime;
	};

	void RecordConnectionAttempt(const char* host, unsigned short remotePort);
	void RemoveFromRequestedConnectionsList(PlayerID playerId);
	bool SendConnectionRequest(const char* host, unsigned short remotePort);
	// Converts a dotted IP to a playerId
	void IPToPlayerID(const char* host, unsigned short remotePort, PlayerID *playerId);
	// Get the reliability layer associated with a playerID.  Returns 0 if none
	RemoteSystemStruct *GetRemoteSystemFromPlayerID(PlayerID playerID) const;
	// When we get a connection request from an ip / port, either accept or reject it
	void HandleConnectionRequest(PlayerID playerId,unsigned char *AESKey, bool setAESKey);
	// Returns how many remote systems initiated a connection to us
	unsigned GetNumberOfIncomingConnections(void) const;
	// Get a free remote system from the list and assign our playerID to it
	RemoteSystemStruct * AssignPlayerIDToRemoteSystemList(PlayerID playerId,unsigned char *AESKey, bool setAESKey);
	// Reset the variables for a remote system
	void ResetRemoteSystemData(RemoteSystemStruct *remoteSystem, bool weInitiatedTheConnection);
	// An incoming packet has a timestamp, so adjust it to be relative to this system
	void ShiftIncomingTimestamp(char *data, PlayerID playerId) const;
	// Get the most probably accurate clock differential for a certain player
	unsigned long GetBestClockDifferential(PlayerID playerId) const;
	void PushPortRefused(PlayerID target);
	// Description:
	// Handles an RPC packet.  If you get a packet with the ID ID_RPC you should pass it to this function
	// This is already done in Multiplayer.cpp, so if you use the Multiplayer class it is handled for you.
	//
	// Parameters:
	// packet - A packet returned from Receive with the ID ID_RPC
	//
	// Returns:
	// true on success, false on a bad packet or an unregistered function
	bool HandleRPCPacket(char *data, int length, PlayerID playerId);
	#ifdef __USE_IO_COMPLETION_PORTS
	bool SetupIOCompletionPortSocket(int index);
	#endif

	bool endThreads, isMainLoopThreadActive;
	// bool isRecvfromThreadActive; // Tracks thread states
	bool occasionalPing; // Do we occasionally ping the other systems?
	unsigned short maximumNumberOfPeers,maximumIncomingConnections;
	// localStaticData necessary because we may not have a RemoteSystemStruct representing ourselves in the list
	RakNet::BitStream incomingPasswordBitStream, outgoingPasswordBitStream, localStaticData;
	PlayerID myPlayerId;

	// This is an array of pointers to RemoteSystemStruct
	// This allows us to preallocate the list when starting, so we don't have to allocate or delete at runtime.
	// Another benefit is that is lets us add and remove active players simply by setting playerId
	// and moving elements in the list by copying pointers variables without affecting running threads, even if they are in the
	// reliability layer
	RemoteSystemStruct* remoteSystemList;

	enum
	{
		requestedConnections_MUTEX,
		incomingPasswordBitStream_Mutex,
		outgoingPasswordBitStream_Mutex,
		remoteSystemList_Mutex, // This mutex is a writer lock for reserving player IDs only
		NUMBER_OF_RAKPEER_MUTEXES
	};
	SimpleMutex rakPeerMutexes[NUMBER_OF_RAKPEER_MUTEXES];


	// The list of people we have tried to connect to recently
	BasicDataStructures::Queue<RequestedConnectionStruct*> requestedConnectionsList;

	// Data that both the client and the server needs
	unsigned long bytesSentPerSecond, bytesReceivedPerSecond;
	bool isSocketLayerBlocking;
	//bool continualPing,isRecvfromThreadActive,isMainLoopThreadActive, endThreads, isSocketLayerBlocking;
	unsigned long validationInteger;
#ifdef _WIN32
	HANDLE
#else
	pthread_t
#endif 
		processPacketsThreadHandle, recvfromThreadHandle;
	SimpleMutex incomingQueueMutex, banListMutex;//,synchronizedMemoryQueueMutex, automaticVariableSynchronizationMutex;
	BasicDataStructures::Queue<Packet *> incomingPacketQueue;//, synchronizedMemoryPacketQueue;
	//	BitStream enumerationData;

	// automatic variable synchronization takes a primary and secondary identifier
	// The unique primary identifier is the index into the automaticVariableSynchronizationList
	// The unique secondary identifier (UNASSIGNED_OBJECT_ID for none) is in an unsorted list of memory blocks
	struct MemoryBlock
	{
		char *original, *copy;
		unsigned short size;
		ObjectID secondaryID;
		bool isAuthority;
		bool (*synchronizationRules) (char*,char*);
	};
	//BasicDataStructures::List<BasicDataStructures::List<MemoryBlock>* > automaticVariableSynchronizationList;
	BasicDataStructures::List<char*> banList;

	// Compression stuff
	unsigned long frequencyTable[256];
	HuffmanEncodingTree *inputTree, *outputTree;
	unsigned long rawBytesSent, rawBytesReceived, compressedBytesSent, compressedBytesReceived;
	//void DecompressInput(RakNet::BitStream *bitStream);
	//void UpdateOutgoingFrequencyTable(RakNet::BitStream * bitStream);
	void GenerateSYNCookieRandomNumber(void);
	void SecuredConnectionResponse(PlayerID playerId);
	void SecuredConnectionConfirmation(PlayerID playerId, char* data);
	bool RunUpdateCycle(void);

	BasicDataStructures::AVLBalancedBinarySearchTree<RPCNode> rpcTree;
	int MTUSize;
	bool trackFrequencyTable;
	int threadSleepTimer;

	SOCKET connectionSocket;

	// Histogram statistics
	//unsigned long nextReadBytesTime;
	//int lastSentBytes,lastReceivedBytes;

	// Encryption and security
	big::RSACrypt<RSA_BIT_SIZE> rsacrypt;
	big::u32 publicKeyE;
	RSA_BIT_SIZE publicKeyN;
	bool keysLocallyGenerated, usingSecurity;
	unsigned long randomNumberExpirationTime;
	unsigned char newRandomNumber[20], oldRandomNumber[20];
};

#endif
