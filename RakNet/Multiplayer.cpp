// ----------------------------------------------------------------------
// This API and the code herein created by and wholly and privately owned by Kevin Jenkins except where specifically indicated otherwise.
// Licensed under the "RakNet" brand by Rakkarsoft LLC and subject to the terms of the relevant licensing agreement available at http://www.rakkarsoft.com
// Filename
// Written by Kevin Jenkins (rakkar@rakkar.org) January 24, 2003
// Description
// ----------------------------------------------------------------------

#include "Multiplayer.h"
#include "RakServerInterface.h"
#include "RakClientInterface.h"
#include "PacketEnumerations.h"
#include "ClientServerFactory.h"
#include <assert.h>


// Instances of the server and the client which are used in this file and NetworkObject.  Use them if you'd like, or delete them if you don't
RakServerInterface* rakServer;
RakClientInterface* rakClient;

Multiplayer::Multiplayer()
{
	// Instances of the server and the client which are used in this file and NetworkObject.  Use them if you'd like, or delete them if you don't
	rakServer = ClientServerFactory::GetRakServerInterface();
	rakClient = ClientServerFactory::GetRakClientInterface();
}

Multiplayer::~Multiplayer()
{
	// Instances of the server and the client which are used in this file and NetworkObject.  Use them if you'd like, or delete them if you don't
	ClientServerFactory::DestroyRakServerInterface(rakServer);
	rakServer=0;
	ClientServerFactory::DestroyRakClientInterface(rakClient);
	rakClient=0;
}

void Multiplayer::ProcessPackets(void)
{
	// Direct native packets
	Packet* p;
	unsigned char packetIdentifier;
	bool fromServer;

	// Get a packet from either the server or the client
	p = GetPacketFromServerOrClient(fromServer);
	packetIdentifier = GetPacketIdentifier(p);

	while (p)
	{
		// Check if this is a native packet
		switch (packetIdentifier)
		{
			// ------------------------------
			// Server only
			// ------------------------------
			case ID_DISCONNECTION_NOTIFICATION:
				ReceiveDisconnectionNotification(p);
				break;

			case ID_NEW_CLIENT:
				ReceiveNewClient(p);
				break;

			case ID_RESPONSIVE_CLIENT:
				ReceiveResponsiveClient(p);
				break;

			// ------------------------------
			// Client only
			// ------------------------------

			case ID_SERVER_FULL:
				ReceiveServerFull(p);
				break;

			case ID_INVALID_PASSWORD:
				ReceiveInvalidPassword(p);
				break;

			case ID_KICKED_BY_SERVER:
				ReceiveKickedByServer(p);
				break;

			case ID_ENUMERATION_REPLY:
				ReceiveEnumerationReply(p);
				break;

			case ID_UNABLE_TO_CONNECT_TO_REMOTE_HOST:
				ReceiveUnableToConnect(p);
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

			case ID_RPC:
				// Remote procedure call.  This is sent through the regular packet layer in case you want to
				// do preprocessing, verification, store, or delay the call
				// RPC BitStream format is
				// ID_RPC - unsigned char
				// Unique identifier string length
				// The unique ID  - string with each letter in upper case, subtracted by 'A' and written in 5 bits.
				// Number of bits of the data (int)
				// The data
				ReceiveRPC(p, fromServer);
				break;

			default:
				// If not a native packet send it to ProcessUnhandledPacket which should have been written by the user
				ProcessUnhandledPacket(p, packetIdentifier);
				break;
		}

		if (rakServer)
			rakServer->DeallocatePacket(p);
		else
			rakClient->DeallocatePacket(p);

		p = GetPacketFromServerOrClient(fromServer);
		packetIdentifier = GetPacketIdentifier(p);
	}
}

void Multiplayer::ProcessUnhandledPacket(Packet *p, unsigned char packetIdentifier)
{
	// This class should have been overrided to handle user defined packets
	//assert(0);
}

Packet* Multiplayer::GetPacketFromServerOrClient(bool &fromServer)
{
	Packet* p=0;

	p = rakServer->Receive();

	if (p==0)
	{
		p = rakClient->Receive();
		fromServer=false;
	}
	else
		fromServer=true;

	return p;
}

unsigned char Multiplayer::GetPacketIdentifier(Packet *p)
{
	if (p==0)
		return 255;

	if ((unsigned char)p->data[0] == ID_TIMESTAMP)
	{
		#ifdef _DEBUG
		assert(p->length > sizeof(unsigned char) + sizeof(unsigned long));
		#endif
		return (unsigned char) p->data[sizeof(unsigned char) + sizeof(unsigned long)];
	}
	else
		return (unsigned char) p->data[0];
}

// ------------------------------
// Server only
// ------------------------------
void Multiplayer::ReceiveDisconnectionNotification(Packet *packet)
{
	// Somebody disconnected from the server willingly by calling Client::Disconnect
	// This person can be identified by the playerID field of the packet: p->playerID
	// You can use this identifier in a variety of methods in the Server class to get data about this person,
	// such as their client data (by default this is just their name) or their IP.
	ProcessUnhandledPacket(packet, ID_DISCONNECTION_NOTIFICATION);
}

void Multiplayer::ReceiveNewClient(Packet *packet)
{
	// Somebody connected to your server but has not yet finished the connection process.  You do not yet have
	// any data in the StaticClientData packet that is stored on the server
	// This packet is always sent before ReceiveResponsiveClient and is part of the connection scheme
	// While you do do not yet have their static client data, you can use the playerID field of the packet: p->playerID
	// to get their IP.  The most useful thing to do in this function is check ban lists and kick out people who are banned
	ProcessUnhandledPacket(packet, ID_NEW_CLIENT);
}

void Multiplayer::ReceiveResponsiveClient(Packet *packet)
{
	// Somebody is now fully connected to your server, and you have the static client data for them.
	// This this point you can display messages such as "Joe has connected to your server"
	// You can get the static client data (by default the client name) by getting the playerID field of the packet: p->playerID
	// and passing that to appropriate methods of the server class
	ProcessUnhandledPacket(packet, ID_RESPONSIVE_CLIENT);
}

// ------------------------------
// Client only
// ------------------------------
void Multiplayer::ReceiveServerFull(Packet *packet)
{
	// The server you tried to connect to is full.  The network code automatically disconnects you.  Any additional functionality
	// can be added here, such as telling the user the server was full
	ProcessUnhandledPacket(packet, ID_SERVER_FULL);
}

void Multiplayer::ReceiveInvalidPassword(Packet *packet)
{
	// The server requires a password and it doesn't match what you specified
	ProcessUnhandledPacket(packet, ID_INVALID_PASSWORD);
}


void Multiplayer::ReceiveKickedByServer(Packet *packet)
{
	// The server operator kicked you out.  When you get this message you wind up here. Any additional functionality
	// can be added here, such as having the game display a message and exit
	ProcessUnhandledPacket(packet, ID_KICKED_BY_SERVER);
}

void Multiplayer::ReceiveEnumerationReply(Packet *packet)
{
	// When you connect to a server it will send you a bunch of data that describes the server, called enumeration data
	// The enumeration data is defined in the struct EnumerationDataStruct which you can modify by modifying the fields of Server::GetEnumerationDataStruct
	// It includes data that you want automatically sent to every client that gets a new connection, such as as message
	// of the day and server name.  When this data arrives you get to this function, and if you want to handle any of it
	// (such as showing the message of the day) you can.
	ProcessUnhandledPacket(packet, ID_ENUMERATION_REPLY);
}

void Multiplayer::ReceiveUnableToConnect(Packet *packet)
{
	// When you connect to a server it will send you a bunch of data that describes the server, called enumeration data
	// The enumeration data is defined in the struct EnumerationDataStruct which you can modify by modifying the fields of Server::GetEnumerationDataStruct
	// It includes data that you want automatically sent to every client that gets a new connection, such as as message
	// of the day and server name.  When this data arrives you get to this function, and if you want to handle any of it
	// (such as showing the message of the day) you can.
	ProcessUnhandledPacket(packet, ID_UNABLE_TO_CONNECT_TO_REMOTE_HOST);
}

void Multiplayer::ReceiveModifiedPacket(Packet *packet)
{
	// The sender modified a packet over the network (i.e. is cheating)
	// If it was from the server, you probably want to play on another server
	// If it was from the client, you probably want to ban them.
	ProcessUnhandledPacket(packet, ID_MODIFIED_PACKET);
}

void Multiplayer::ReceiveConnectionLost(Packet *packet)
{
	// If you are a server and get this, the playerID field indicates who just timed out unwillingly (this often means crashed).
	// The network code automatically drops that player, but you may wish to tell the server operator or the players that such and such has been dropped
	// If you are a client and get this, it means you aren't getting any data from the server and have been disconnected.
	// You should handle this as appropriate in the game
	ProcessUnhandledPacket(packet, ID_CONNECTION_LOST);
}

void Multiplayer::ReceiveRPC(Packet *packet, bool fromServer)
{
	// Remote procedure call.  This is sent through the regular packet layer in case you want to
	// do preprocessing, verification, store, or delay the call
	// RPC BitStream format is
	// ID_RPC - unsigned char
	// Unique identifier string length - unsigned char
	// The unique ID  - string with each letter in upper case, subtracted by 'A' and written in 5 bits.
	// Number of bits of the data (long)
	// The data

	// In practice you will want to check the return value from HandleRPCPacket during debugging
	// as a false return value would usually indicate calling an unregistered function
	if (fromServer)
		rakServer->HandleRPCPacket(packet);
	else
		rakClient->HandleRPCPacket(packet);
}
