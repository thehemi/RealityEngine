// This file is part of RakNet Copyright 2003, 2004 Rakkarsoft LLC and Kevin Jenkins.

// Usage of Raknet is subject to the appropriate licence agreement.
// "Shareware" Licensees with Rakkarsoft LLC are subject to the shareware license found at http://www.rakkarsoft.com/shareWareLicense.html which you agreed to upon purchase of a "Shareware license"
// "Commercial" Licensees with Rakkarsoft LLC are subject to the commercial license found at http://www.rakkarsoft.com/sourceCodeLicense.html which you agreed to upon purchase of a "Commercial license"
// All other users are subject to the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

// Refer to the appropriate license agreement for distribution, modification, and warranty rights.


#ifndef __PACKET_ENUMERATIONS_H
#define __PACKET_ENUMERATIONS_H

enum {
	//
	// RESERVED TYPES - DO NOT CHANGE THESE
	//
	// Ignore these:
	ID_PING, // Ping (internal use only)
	ID_PONG, // Pong.  Returned if we ping a system we are not connected so
	ID_PING_OPEN_CONNECTIONS, // Only reply to the unconnected ping if we have open connections
	ID_REQUEST_STATIC_DATA, // Someone asked for our static data (internal use only)
	ID_CONNECTION_REQUEST, // Asking for a new connection (internal use only)
	ID_SECURED_CONNECTION_RESPONSE, // Connecting to a secured server/peer
	ID_SECURED_CONNECTION_CONFIRMATION, // Connecting to a secured server/peer
	ID_RPC, // Remote procedure call (internal use only)
	ID_RECEIVED_RELAYED_STATIC_DATA, // Server / Client only - Sending the static data for another system (internal use only)
	ID_BROADCAST_PINGS, // Server / Client only - The server is broadcasting the pings of all players in the game (internal use only)
	ID_SET_RANDOM_NUMBER_SEED, // Server / Client only - The server is broadcasting a random number seed (internal use only)
	ID_RPC_WITH_TIMESTAMP, // Same as RPC, but treat the first 4 bytes as a timestamp
	// Handle these below.  Possible recipients in [...]
	ID_RSA_PUBLIC_KEY_MISMATCH, // [CLIENT|PEER] We preset an RSA public key which does not match what the system we connected to is using.
	ID_REMOTE_DISCONNECTION_NOTIFICATION, // [CLIENT] In a client/server enviroment, a client other than ourselves has disconnected gracefully.  Packet::playerID is modified to reflect the playerID of this client.
	ID_REMOTE_CONNECTION_LOST, // [CLIENT] In a client/server enviroment, a client other than ourselves has been forcefully dropped.  Packet::playerID is modified to reflect the playerID of this client.
	ID_REMOTE_NEW_INCOMING_CONNECTION, // [CLIENT] In a client/server enviroment, a client other than ourselves has connected.  Packet::playerID is modified to reflect the playerID of this client.
	ID_REMOTE_EXISTING_CONNECTION, // [CLIENT] On our initial connection to the server, we are told of every other client in the game.  Packet::playerID is modified to reflect the playerID of this client.
	ID_CONNECTION_BANNED, // [PEER|CLIENT] We are banned from the system we attempted to connect to.
	ID_CONNECTION_REQUEST_ACCEPTED, // [CLIENT] In a client/server enviroment, our connection request to the server has been accepted.
	ID_NEW_INCOMING_CONNECTION, // [PEER|SERVER] A remote system has successfully connected.
	ID_NO_FREE_INCOMING_CONNECTIONS, // [PEER|CLIENT] The system we attempted to connect to is not accepting new connections.
	ID_DISCONNECTION_NOTIFICATION, // [PEER|SERVER|CLIENT] The system specified in Packet::playerID has disconnected from us.  For the client, this would mean the server has shutdown.
	ID_CONNECTION_LOST, // [PEER|SERVER|CLIENT] Reliable packets cannot be delivered to the system specifed in Packet::playerID.  The connection to that system has been closed.
	ID_TIMESTAMP, // [PEER|SERVER|CLIENT] The four bytes following this byte represent an unsigned long which is automatically modified by the difference in system times between the sender and the recipient. Requires that you call StartOccasionalPing.
	ID_RECEIVED_STATIC_DATA, // [PEER|SERVER|CLIENT] We got a bitstream containing static data.  You can now read this data.  This packet is transmitted automatically on connections, and can also be manually sent.
	ID_INVALID_PASSWORD, // [PEER|CLIENT] The remote system is using a password and has refused our connection because we did not set the correct password.
	ID_MODIFIED_PACKET, // [PEER|SERVER|CLIENT] A packet has been tampered with in transit.  The sender is contained in Packet::playerID.
	ID_REMOTE_PORT_REFUSED, // [PEER|SERVER|CLIENT] The remote host is not accepting data on this port.  This only comes up when connecting to yourself on the same computer and there is no bound socket on that port.
	ID_VOICE_PACKET, // [PEER] This packet contains voice data.  You should pass it to the RakVoice system.
	ID_UPDATE_DISTRIBUTED_NETWORK_OBJECT, // [CLIENT|SERVER] Indicates creation or update of a distributed network object.  Pass to DistributedNetworkObjectManager::Instance()->HandleDistributedNetworkObjectPacket
	ID_DISTRIBUTED_NETWORK_OBJECT_CREATION_ACCEPTED, // [CLIENT] Client creation of a distributed network object was accepted.  Pass to DistributedNetworkObjectManager::Instance()->HandleDistributedNetworkObjectPacketCreationAccepted
	ID_DISTRIBUTED_NETWORK_OBJECT_CREATION_REJECTED, // [CLIENT] Client creation of a distributed network object was rejected.  Pass to DistributedNetworkObjectManager::Instance()->HandleDistributedNetworkObjectPacketCreationRejected
	ID_AUTOPATCHER_REQUEST_FILE_LIST, // [PEER|SERVER|CLIENT] Request for a list of downloadable files. Pass to Autopatcher::SendDownloadableFileList
	ID_AUTOPATCHER_FILE_LIST, // [PEER|SERVER|CLIENT] Got a list of downloadable files. Pass to Autopatcher::OnAutopatcherFileList
	ID_AUTOPATCHER_REQUEST_FILES, // [PEER|SERVER|CLIENT] Request for a particular set of downloadable files. Pass to Autopatcher::OnAutopatcherRequestFiles
	ID_AUTOPATCHER_SET_DOWNLOAD_LIST, // [PEER|SERVER|CLIENT] Set the list of files that were approved for download and are incoming. Pass to Autopatcher::OnAutopatcherSetDownloadList
	ID_AUTOPATCHER_WRITE_FILE, // [PEER|SERVER|CLIENT] Got a file that we requested for download.  Pass to Autopatcher::OnAutopatcherWriteFile
	ID_QUERY_MASTER_SERVER, // [MASTERSERVER] Request to the master server for the list of servers that contain at least one of the specified keys
	ID_MASTER_SERVER_DELIST_SERVER, // [MASTERSERVER] Remove a game server from the master server.
	ID_MASTER_SERVER_UPDATE_SERVER, // [MASTERSERVER|MASTERCLIENT] Add or update the information for a server.
	ID_MASTER_SERVER_SET_SERVER, // [MASTERSERVER|MASTERCLIENT] Add or set the information for a server.
	ID_RESERVED3, // For future versions
	ID_RESERVED4, // For future versions
	ID_RESERVED5, // For future versions
	ID_RESERVED6, // For future versions
	ID_RESERVED7, // For future versions
	ID_RESERVED8, // For future versions
	ID_RESERVED9, // For future versions
 	//-------------------------------------------------------------------------------------------------------------
 	

	//
	// YOUR TYPES HERE!
	// WARNING - By default it is assumed that the packet identifier is one byte (unsigned char)
	// In the unlikely event that you need more than 256 types, including the built-in types, then you'll need
	// to request a special edition with larger identifiers, or change it yourself
	//



};

#endif
