// This file is part of RarkNet Copyright 2003, 2004 Rakkarsoft LLC and Kevin Jenkins.

// Usage of Raknet is subject to the appropriate licence agreement.
// "Shareware" Licensees with Rakkarsoft LLC are subject to the shareware license found at http://www.rakkarsoft.com/shareWareLicense.html which you agreed to upon purchase of a "Shareware license"
// "Commercial" Licensees with Rakkarsoft LLC are subject to the commercial license found at http://www.rakkarsoft.com/sourceCodeLicense.html which you agreed to upon purchase of a "Commercial license"
// All other users are subject to the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

// Refer to the appropriate license agreement for distribution, modification, and warranty rights.

#ifndef __RAK_CLIENT_H
#define __RAK_CLIENT_H

#include "RakPeer.h"
#include "RakClientInterface.h"

class RakClient : public RakPeer, public RakClientInterface
{
public:
	// Constructor
	RakClient();

	// Destructor
	~RakClient();

	// Call this to connect the client to the specified host (ip or domain name) and server port.
	// This is a non-blocking connection.  You know the connection is successful when IsConnected() returns true
	// or receive gets a packet with the type identifier ID_CONNECTION_REQUEST_ACCEPTED.
	// serverPort is which port to connect to on the remote machine. clientPort is the port you want the client to use.
	// Both ports must be open for UDP
	// validationInteger is legacy and unused
	// _threadSleepTimer: <0 for don't sleep, >=0 for how many ms to Sleep each internal update cycle (recommended you start with 15, try different values)
	// Returns true on successful initiation, false otherwise
	bool Connect(char* host, unsigned short serverPort, unsigned short clientPort, unsigned long connectionValidationInteger, int threadSleepTimer);

	// Stops the client, stops synchronized data, and resets all internal data. 
	// Does nothing if the client is not connected to begin with
	// blockDuration is how long you should wait for all remaining packets to go out
	// If you set it to 0 then the disconnection notification probably won't arrive
	void Disconnect(unsigned long blockDuration);

	// Description:
	// Can be called to use specific public RSA keys. (e and n)
	// In order to prevent altered keys.  Will return ID_RSA_PUBLIC_KEY_MISMATCH in a packet
	// If a key has been altered.
	//
	// Parameters:
	// RSAe, RSAn - Public keys generated from the RSACrypt class.  See the Encryption sample.  Can be 0
	void InitializeSecurity(char *RSAe, char *RSAn);

	// Set the password to use when connecting to a server.  The password persists between connections.
	// Pass 0 for no password.
	void SetPassword(char *_password);

	// Returns true if a password was set, false otherwise
	bool HasPassword(void) const;

	// This function only works while the client is connected (Use the Connect function).  Returns false on failure, true on success
	// Sends the data stream of length length
	// If you aren't sure what to specify for priority and reliability, use HIGH_PRIORITY and RELIABLE, 0 for ordering stream
	bool Send(char *data, const long length, PacketPriority priority, PacketReliability reliability, char orderingStream);

	// This function only works while the client is connected (Use the Connect function).  Returns false on failure, true on success
	// Sends the BitStream
	// If you aren't sure what to specify for priority and reliability, use HIGH_PRIORITY and RELIABLE, 0 for ordering stream
	bool Send(RakNet::BitStream * bitStream, PacketPriority priority, PacketReliability reliability, char orderingStream);

	// Call this to get a packet from the incoming packet queue. Use DeallocatePacket to deallocate the packet after you are done with it.
	// Check the Packet struct at the top of CoreNetworkStructures.h for the format of the struct
	// Returns 0 if no packets are waiting to be handled
	// If the client is not active this will also return 0, as all waiting packets are flushed when the client is Disconnected
	// This also updates all memory blocks associated with synchronized memory
	Packet* Receive(void);

	// Call this to deallocate a packet returned by Receive when you are done handling it.
	void DeallocatePacket(Packet *packet);

	// Send a ping request to the server.  Occasional pings are on by default (see StartOccasionalPing and StopOccasionalPing)
	// so unless you turn them off it is not necessary to call this function.  It is here for completeness if you want it
	// Does nothing if the client is not connected to begin with
	void PingServer(void);

	// Sends a ping request to a server we are not connected to.  This will also initialize the
	// networking system if it is not already initialized.  You can stop the networking system
	// by calling Disconnect()
	// The final ping time will be encoded in the following 4 bytes (2-5) as an unsigned long
	void PingServer(char* host, unsigned short serverPort, unsigned short clientPort);

	// Returns the average of all ping times read
	int GetAveragePing(void);

	// Returns the last ping time read for the specific player or -1 if none read yet
	int GetLastPing(void) const;

	// Returns the lowest ping time read or -1 if none read yet
	int GetLowestPing(void) const;

	// Returns the last ping for the specified player.  This information is broadcast by the server automatically
	// In order to save bandwidth this information is updated only infrequently and only for the first 32 players
	// Note: You can read your own ping with this method by passing your own playerId, however for more up-to-date
	// readings you should use one of the three functions above
	int GetPlayerPing(PlayerID playerId);

	// Ping the server every so often.  This is on by default.  In games where you don't care about ping you can call
	// StopOccasionalPing to save the bandwidth
	// This will work anytime
	void StartOccasionalPing(void);

	// Stop pinging the server every so often.  The server is pinged by default.  In games where you don't care about ping
	// you can call this to save the bandwidth
	// This will work anytime
	void StopOccasionalPing(void);

	// Returns true if the client is connected to a responsive server
	bool IsConnected(void) const;

	// Returns a number automatically synchronized between the server and client which randomly changes every
	// 9 seconds. The time it changes is accurate to within a few ms and is best used to seed
	// random number generators that you want to usually return the same output on all systems.  Keep in mind this
	// isn't perfectly accurate as there is always a very small chance the numbers will by out of synch during
	// changes so you should confine its use to visual effects or functionality that has a backup method to
	// maintain synchronization.  If you don't need this functionality and want to save the bandwidth call
	// StopSynchronizedRandomInteger after starting the server
	unsigned long GetSynchronizedRandomInteger(void) const;

	/*
	// Call this to automatically synchronize a block of memory.
	// Unique identifier should be an integer corresponding to the same variable between clients and the server.  This integer
	// should start at 0 and not surpass the range of UniqueIDType.  It is recommended you set this from an enum
	// memoryBlock should point to the data you want to read from or write to with size of size in bytes
	// isAuthority should be true if all other computers should match their data in memory block to yours.  This is triggered by
	// when the variable changes.  So setting it to true on both the server and one client would make it so if the synchronized
	// variable on that client changed, the server would then relay it to all clients.
	// In the current implementation, setting isAuthority to true on the server will cause changes to that variable to be broadcast to
	// all connected clients.
	// synchronizationRules is an optional function pointer defined by you.  It should
	// return true if the two passed memory blocks are sufficiently different to synchronize them.  This is an optimization so
	// data that changes rapidly, such as per-frame, can be made to not update every frame
	// The first parameter to synchronizationRules is the new data, the second is the internal copy of the old data
	// secondaryUniqueIdentifier is optional and used when you have the same unique identifier and is intended for multiple instances of a class
	// that derives from NetworkObject.
	// You can call this anytime - however if you call it before the connection is complete initial data will not by synchronized
	void SynchronizeMemory(UniqueIDType uniqueIdentifier, char *memoryBlock, unsigned short size, bool isAuthority, bool (*synchronizationRules) (char*,char*)=0,ObjectID secondaryUniqueIdentifier=UNASSIGNED_OBJECT_ID);

	// Call this to stop synchronization of a block of memory previously defined by uniqueIdentifier and secondaryUniqueIdentifier
	// by the call to SynchronizeMemory
	// CALL THIS BEFORE SYNCHRONIZED MEMORY IS DEALLOCATED!
	// It is not necessary to call this before disconnecting, as all synchronized states will be released then.
	void DesynchronizeMemory(UniqueIDType uniqueIdentifier, ObjectID secondaryUniqueIdentifier=UNASSIGNED_OBJECT_ID);

	// Call this to Desynchronize all synchronized memory
	void DesynchronizeAllMemory(void);
	*/

	// This is an optional function to generate the compression layer from the input frequency table.
	// You should call this twice - once with inputLayer as true and once as false.
	// The frequency table passed here with inputLayer=true should match the frequency table on the recipient with inputLayer=false.
	// Likewise, the frequency table passed here with inputLayer=false should match the frequency table on the recipient with inputLayer=true
	// Calling this function when there is an existing layer will overwrite the old layer
	// You should only call this when disconnected
	// Return value: false (failure) if connected.  Otherwise true (success)
	bool GenerateCompressionLayer(unsigned long inputFrequencyTable[256], bool inputLayer);

	// Delete the output or input layer as specified.  This is not necessary to call and is only valuable for freeing memory
	// You should only call this when disconnected
	// Return value: false (failure) if connected.  Otherwise true (success)
	bool DeleteCompressionLayer(bool inputLayer);

	// Register a C function as available for calling as a remote procedure call
	// uniqueID should be a null terminated non-case senstive string of only letters to identify this procedure
	// Parameter 2 should be the name of the C function or C++ singleton to be used as a function pointer
	// This can be called whether the client is active or not, and registered functions stay registered unless unregistered with
	// UnregisterAsRemoteProcedureCall
	void RegisterAsRemoteProcedureCall(char* uniqueID, void (*functionName)(char *input, int numberOfBitsOfData, PlayerID sender));

	// Unregisters a C function as available for calling as a remote procedure call that was formerly registered
	// with RegisterAsRemoteProcedureCall
	void UnregisterAsRemoteProcedureCall(char* uniqueID);

	// Calls a C function on the server that the server already registered using RegisterAsRemoteProcedureCall
	// Pass the data you want to pass to that function in parameters, or 0 for no data to pass
	// You can also pass a regular data stream which will be converted to a bitstream internally by passing data and bit length
	// If you want that function to return data you should call RPC from that system in the same way
	// Returns true on a successful packet send (this does not indicate the recipient performed the call), false on failure
	// The uniqueID must be composed of a string with only characters from a-z and is not case sensitive
	bool RPC(char* uniqueID, char *data, long bitLength, PacketPriority priority, PacketReliability reliability, char orderingStream, bool shiftTimestamp);
	bool RPC(char* uniqueID, RakNet::BitStream *parameters, PacketPriority priority, PacketReliability reliability, char orderingStream, bool shiftTimestamp);

	// Handles an RPC packet.  If you get a packet with the ID ID_RPC you should pass it to this function
	// This is already done in Multiplayer.cpp, so if you use the Multiplayer class it is handled for you.
	// Returns true on success, false on a bad packet or an unregistered function
	// bool HandleRPCPacket(Packet* packet);

	// Enables or disables frequency table tracking.  This is required to get a frequency table, which is used to generate
	// A new compression layer.
	// You can call this at any time - however you SHOULD only call it when disconnected.  Otherwise you will only track
	// part of the values sent over the network.
	// This value persists between connect calls and defaults to false (no frequency tracking)
	void SetTrackFrequencyTable(bool b);

	// Returns the frequency of outgoing bytes into outputFrequencyTable
	// The purpose is to save to file as either a master frequency table from a sample game session for passing to
	// GenerateCompressionLayer.
	// You should only call this when disconnected.
	// Requires that you first enable data frequency tracking by calling SetTrackFrequencyTable(true)
	// Return value: false (failure) if connected or if frequency table tracking is not enabled.  Otherwise true (success)
	bool GetSendFrequencyTable(unsigned long outputFrequencyTable[256]);

	// Returns the compression ratio.  A low compression ratio is good.  Compression is for outgoing data
	float GetCompressionRatio(void) const;

	// Returns the decompression ratio.  A high decompression ratio is good.  Decompression is for incoming data
	float GetDecompressionRatio(void) const;

	// The server internally maintains a data struct that is automatically sent to clients when the connect.
	// This is useful to contain data such as the server name or message of the day.  Access that struct with this
	// function.
	// The data is entered as an array and stored and returned as a BitStream.  
	// Everytime you call GetStaticServerData it resets the read pointer to the start of the bitstream.  To do multiple reads without reseting the pointer
	// Maintain a pointer copy to the bitstream as in
	// RakNet::BitStream *copy = ...->GetStaticServerData(...);
	// To store a bitstream, use the GetData() and GetNumberOfBytesUsed() methods
	// of the bitstream for the 2nd and 3rd parameters
	// Note that the server may change at any time the
	// data contents and/or its length!
	RakNet::BitStream * GetStaticServerData(void);
	void SetStaticServerData(char *data, const long length);

	// The client internally maintains a data struct that is automatically sent to the server on connection
	// This is useful to contain data such as the player name.  Access that struct with this
	// function. Pass UNASSIGNED_PLAYER_ID for playerId to reference your internal data.  A playerId value to access the data of another player.
	// *** NOTE ***
	// If you change any data in the struct the server won't reflect this change unless you manually update it
	// Do so by calling SendStaticClientDataToServer
	// The data is entered as an array and stored and returned as a BitStream.
	// Everytime you call GetStaticServerData it resets the read pointer to the start of the bitstream.  To do multiple reads without reseting the pointer
	// Maintain a pointer copy to the bitstream as in
	// RakNet::BitStream *copy = ...->GetStaticServerData(...);
	// To store a bitstream, use the GetData() and GetNumberOfBytesUsed() methods
	// of the bitstream for the 2nd and 3rd parameters
	RakNet::BitStream * GetStaticClientData(PlayerID playerId);
	void SetStaticClientData(PlayerID playerId, char *data, const long length);

	// Send the static server data to the server
	// The only time you need to call this function is to update clients that are already connected when you change the static
	// server data by calling GetStaticServerData and directly modifying the object pointed to.  Obviously if the
	// connected clients don't need to know the new data you don't need to update them, so it's up to you
	// The server must be active for this to have meaning
	void SendStaticClientDataToServer(void);

	// Return the player number of the server.
	PlayerID GetServerID(void) const;

	// Return the player number the server has assigned to you.
	// Note that unlike in previous versions, this is a struct and is not sequential
	PlayerID GetPlayerID(void) const;

	// Description:
	// Returns the dotted IP address for the specified playerId
	//
	// Parameters:
	// playerId - Any player ID other than UNASSIGNED_PLAYER_ID, even if that player is not currently connected
	const char* PlayerIDToDottedIP(PlayerID playerId) const;

	// Put a packet back at the end of the receive queue in case you don't want to deal with it immediately
	void PushBackPacket(Packet *packet);

	// Change the MTU size in order to improve performance when sending large packets
	// This can only be called when not connected.
	// Returns false on failure (we are connected).  True on success.  Maximum allowed size is MAXIMUM_MTU_SIZE
	// A too high of value will cause packets not to arrive at worst and be fragmented at best.
	// A too low of value will split packets unnecessarily.
	// Set according to the following table:
	// 1500. The largest Ethernet packet size; it is also the default value.
	// This is the typical setting for non-PPPoE, non-VPN connections. The default value for NETGEAR routers, adapters and switches. 
	// 1492. The size PPPoE prefers. 
	// 1472. Maximum size to use for pinging. (Bigger packets are fragmented.) 
	// 1468. The size DHCP prefers. 
	// 1460. Usable by AOL if you don't have large email attachments, etc. 
	// 1430. The size VPN and PPTP prefer. 
	// 1400. Maximum size for AOL DSL. 
	// 576. Typical value to connect to dial-up ISPs. (Default)
	bool SetMTUSize(int size);

	// Returns the current MTU size
	int GetMTUSize(void) const;

	// Description:
	// Returns a structure containing a large set of network statistics for the server/client connection
	// You can map this data to a string using the C style StatisticsToString function
	//
	// Returns:
	// 0 on can't find the specified system.  A pointer to a set of data otherwise.
	RakNetStatisticsStruct * const GetStatistics(void);

	// For internal use
	PlayerIndex GetPlayerIndex(void);

	private:

	int GetOtherClientIndexByPlayerID(PlayerID playerId);
	int GetFreeOtherClientIndex(void);

	char password[20];
	struct OtherClientsStruct
	{
		PlayerID playerId;
		short ping;
		RakNet::BitStream staticData;
		bool isActive;
	} otherClients[32];
	// Synchronized random integer
	unsigned long seed, nextSeed, nextSeedUpdate;
	PlayerIndex localPlayerIndex;
	PlayerID externalPlayerID; // This is your external ID (and also IP) (returned from the server)
};

#endif
