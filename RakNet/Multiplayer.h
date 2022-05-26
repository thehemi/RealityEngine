// This file is part of RakNet Copyright 2003, 2004 Rakkarsoft LLC and Kevin Jenkins.

// Usage of Raknet is subject to the appropriate licence agreement.
// "Shareware" Licensees with Rakkarsoft LLC are subject to the shareware license found at http://www.rakkarsoft.com/shareWareLicense.html which you agreed to upon purchase of a "Shareware license"
// "Commercial" Licensees with Rakkarsoft LLC are subject to the commercial license found at http://www.rakkarsoft.com/sourceCodeLicense.html which you agreed to upon purchase of a "Commercial license"
// All other users are subject to the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

// Refer to the appropriate license agreement for distribution, modification, and warranty rights.

#ifndef __MULTIPLAYER_H
#define __MULTIPLAYER_H

struct Packet;

template <class InterfaceType>
class Multiplayer
{
public:
	Multiplayer();
	virtual ~Multiplayer();

	// -----------------------------------------------------------------
	// CALL THIS EVERY FRAME
	// Reads any packets from the network, handles the native messages, and sends user defined messages to ProcessUnhandledPacket
	// -----------------------------------------------------------------
	virtual void ProcessPackets(InterfaceType *interfaceType);

protected:
	// -----------------------------------------------------------------
	//  *** YOU PROBABLY WANT TO OVERRIDE THESE FOR YOUR OWN APP ***
	// -----------------------------------------------------------------
	virtual void ReceiveRemoteDisconnectionNotification(Packet *packet,InterfaceType *interfaceType);
	virtual void ReceiveRemoteConnectionLost(Packet *packet,InterfaceType *interfaceType);
	virtual void ReceiveRemoteNewIncomingConnection(Packet *packet,InterfaceType *interfaceType);
	virtual void ReceiveRemoteExistingConnection(Packet *packet,InterfaceType *interfaceType);
	virtual void ReceiveConnectionBanned(Packet *packet,InterfaceType *interfaceType);
	virtual void ReceiveConnectionRequestAccepted(Packet *packet,InterfaceType *interfaceType);
	virtual void ReceiveNewIncomingConnection(Packet *packet,InterfaceType *interfaceType);
	virtual void ReceiveConnectionResumption(Packet *packet,InterfaceType *interfaceType);
	virtual void ReceiveNoFreeIncomingConnections(Packet *packet,InterfaceType *interfaceType);
	virtual void ReceiveDisconnectionNotification(Packet *packet,InterfaceType *interfaceType);
	virtual void ReceiveConnectionLost(Packet *packet,InterfaceType *interfaceType);
	virtual void ReceivedStaticData(Packet *packet,InterfaceType *interfaceType);
	virtual void ReceiveInvalidPassword(Packet *packet,InterfaceType *interfaceType);
	virtual void ReceiveModifiedPacket(Packet *packet,InterfaceType *interfaceType);
	virtual void ReceiveRemotePortRefused(Packet *packet,InterfaceType *interfaceType);
	virtual void ReceiveVoicePacket(Packet *packet,InterfaceType *interfaceType);
	virtual void ReceivePong(Packet *packet,InterfaceType *interfaceType);
	// -----------------------------------------------------------------

	// -----------------------------------------------------------------
	// *** YOU MUST OVERRIDE THIS FOR YOUR OWN APP ***
	// -----------------------------------------------------------------
	// All user defined packets are sent to this function.
	virtual void ProcessUnhandledPacket(Packet *packet, unsigned char packetIdentifier, InterfaceType *interfaceType);
	// -----------------------------------------------------------------

	// Distributed objects.  No need to override these
	virtual void ReceiveDistributedNetworkObject(Packet *packet,InterfaceType *interfaceType);
	virtual void ReceiveDistributedNetworkObjectCreationAccepted(Packet *packet,InterfaceType *interfaceType);
	virtual void ReceiveDistributedNetworkObjectCreationRejected(Packet *packet,InterfaceType *interfaceType);

	// Autopatcher commands.  Send these to the autopatcher class' method
	virtual void ReceiveAutopatcherRequestFileList(Packet *packet,InterfaceType *interfaceType);
	virtual void ReceiveAutopatcherFileList(Packet *packet,InterfaceType *interfaceType);
	virtual void ReceiveAutopatcherRequestFiles(Packet *packet,InterfaceType *interfaceType);
	virtual void ReceiveAutopatcherSetDownloadList(Packet *packet,InterfaceType *interfaceType);
	virtual void ReceiveAutopatcherWriteFile(Packet *packet,InterfaceType *interfaceType);

	// -----------------------------------------------------------------
	// Given a packet, returns the packet identifier.
	// No need to override this
	// -----------------------------------------------------------------
	static unsigned char GetPacketIdentifier(Packet *packet);

private:
};

// If you don't want to use distributed network objects, delete this include
#include "DistributedNetworkObjectManager.h"
#include "PacketEnumerations.h"
#include "NetworkTypes.h"
#include <assert.h>
#include <stdio.h>
#include "GetTime.h"

#ifdef _DEBUG
#include <memory.h>
#endif

template <class InterfaceType>
Multiplayer<InterfaceType>::Multiplayer()
{
}

template <class InterfaceType>
Multiplayer<InterfaceType>::~Multiplayer()
{
}

template <class InterfaceType>
void Multiplayer<InterfaceType>::ProcessPackets(InterfaceType *interfaceType)
{
	Packet* p;
	unsigned char packetIdentifier;

	p = interfaceType->Receive();

	while (p)
	{
		if ((unsigned char)p->data[0] == ID_TIMESTAMP)
		{
			if (p->length > sizeof(unsigned char) + sizeof(unsigned long))
				packetIdentifier = (unsigned char) p->data[sizeof(unsigned char) + sizeof(unsigned long)];
			else
				break;
		}
		else
			packetIdentifier = (unsigned char) p->data[0];

		// Check if this is a native packet
		switch (packetIdentifier)
		{
		case ID_REMOTE_DISCONNECTION_NOTIFICATION:
			ReceiveRemoteDisconnectionNotification(p,interfaceType);
		break;
		case ID_REMOTE_CONNECTION_LOST:
			ReceiveRemoteConnectionLost(p,interfaceType);
		break;
		case ID_REMOTE_NEW_INCOMING_CONNECTION:
			ReceiveRemoteNewIncomingConnection(p,interfaceType);
		break;
		case ID_REMOTE_EXISTING_CONNECTION:
			ReceiveRemoteExistingConnection(p,interfaceType);
		break;
		case ID_CONNECTION_BANNED:
			ReceiveConnectionBanned(p, interfaceType);
		break;
		case ID_CONNECTION_REQUEST_ACCEPTED:
			ReceiveConnectionRequestAccepted(p,interfaceType);
			break;
		case ID_NEW_INCOMING_CONNECTION:
			ReceiveNewIncomingConnection(p,interfaceType);
			break;
		case ID_NO_FREE_INCOMING_CONNECTIONS:
			ReceiveNoFreeIncomingConnections(p,interfaceType);
			break;
		case ID_DISCONNECTION_NOTIFICATION:
			ReceiveDisconnectionNotification(p,interfaceType);
			break;
		case ID_CONNECTION_LOST:
			ReceiveConnectionLost(p,interfaceType);
			break;
		case ID_RECEIVED_STATIC_DATA:
			ReceivedStaticData(p,interfaceType);
			break;
		case ID_INVALID_PASSWORD:
			ReceiveInvalidPassword(p,interfaceType);
			break;
		case ID_MODIFIED_PACKET:
			ReceiveModifiedPacket(p,interfaceType);
			break;
		case ID_REMOTE_PORT_REFUSED:
			ReceiveRemotePortRefused(p,interfaceType);
			break;
		case ID_VOICE_PACKET:
			ReceiveVoicePacket(p,interfaceType);
			break;
		case ID_PONG:
			ReceivePong(p,interfaceType);
			break;
		case ID_UPDATE_DISTRIBUTED_NETWORK_OBJECT:
			ReceiveDistributedNetworkObject(p,interfaceType);
			break;
		case ID_DISTRIBUTED_NETWORK_OBJECT_CREATION_ACCEPTED:
			ReceiveDistributedNetworkObjectCreationAccepted(p,interfaceType);
			break;
		case ID_DISTRIBUTED_NETWORK_OBJECT_CREATION_REJECTED:
			ReceiveDistributedNetworkObjectCreationRejected(p,interfaceType);
			break;
		case ID_AUTOPATCHER_REQUEST_FILE_LIST:
			ReceiveAutopatcherRequestFileList(p,interfaceType);
			break;
		case ID_AUTOPATCHER_FILE_LIST:
			ReceiveAutopatcherFileList(p,interfaceType);
			break;
		case ID_AUTOPATCHER_REQUEST_FILES:
			ReceiveAutopatcherRequestFiles(p,interfaceType);
			break;
		case ID_AUTOPATCHER_SET_DOWNLOAD_LIST:
			ReceiveAutopatcherSetDownloadList(p,interfaceType);
			break;
		case ID_AUTOPATCHER_WRITE_FILE:
			ReceiveAutopatcherWriteFile(p,interfaceType);
			break;
		default:
			// If not a native packet send it to ProcessUnhandledPacket which should have been written by the user
			ProcessUnhandledPacket(p, packetIdentifier,interfaceType);
			break;
		}

		interfaceType->DeallocatePacket(p);

		p = interfaceType->Receive();
	}
}

template <class InterfaceType>
void Multiplayer<InterfaceType>::ProcessUnhandledPacket(Packet *p, unsigned char packetIdentifier, InterfaceType *interfaceType)
{
	// Remove the annoying warnings
	packetIdentifier;
	interfaceType;

	// This class should have been overrided to handle user defined packets
#ifdef _DEBUG
	// Uncomment this to show output as integers
	/*
	int i;
	static unsigned packetNumber=0;
	// Raw output (nonstring)
	printf("Multiplayer::ProcessUnhandledPacket (%i) (%i): ", packetNumber++, p->length);
	for (i=0; i < p->length; i++)
	printf("%i ",p->data[i]);
	printf("\n");
	*/

	// Uncomment this to show output as a string
	
	// Raw output (string)
	printf("%s\n", p->data);
	
#endif
}

template <class InterfaceType>
void Multiplayer<InterfaceType>::ReceiveRemoteDisconnectionNotification(Packet *packet,InterfaceType *interfaceType)
{
	// Another system has disconnected.  Client only.
#ifdef _DEBUG
	printf("ID_REMOTE_DISCONNECTION_NOTIFICATION from PlayerID:%u:%u on %p.\n",packet->playerId.binaryAddress, packet->playerId.port, interfaceType);
#endif
//	ProcessUnhandledPacket(packet, ID_REMOTE_DISCONNECTION_NOTIFICATION,interfaceType);
}

template <class InterfaceType>
void Multiplayer<InterfaceType>::ReceiveRemoteConnectionLost(Packet *packet,InterfaceType *interfaceType)
{
	// Another system has been dropped by the server.  Client only.
#ifdef _DEBUG
	printf("ID_REMOTE_CONNECTION_LOST from PlayerID:%u:%u on %p.\n",packet->playerId.binaryAddress, packet->playerId.port, interfaceType);
#endif
//	ProcessUnhandledPacket(packet, ID_REMOTE_CONNECTION_LOST,interfaceType);
}

template <class InterfaceType>
void Multiplayer<InterfaceType>::ReceiveRemoteNewIncomingConnection(Packet *packet,InterfaceType *interfaceType)
{
	// Another system has connected.  Client only.
#ifdef _DEBUG
	printf("ID_REMOTE_NEW_INCOMING_CONNECTION from PlayerID:%u:%u on %p.\n",packet->playerId.binaryAddress, packet->playerId.port, interfaceType);
#endif
//	ProcessUnhandledPacket(packet, ID_REMOTE_NEW_INCOMING_CONNECTION,interfaceType);
}

template <class InterfaceType>
void Multiplayer<InterfaceType>::ReceiveRemoteExistingConnection(Packet *packet,InterfaceType *interfaceType)
{
	// We just connected to the server and are getting a list of players already connected
	// Note due to thread timing you might get both this and ID_REMOTE_NEW_INCOMING_CONNECTION when first connecting.
	//  Client only.
#ifdef _DEBUG
	printf("ID_REMOTE_EXISTING_CONNECTION from PlayerID:%u:%u on %p.\n",packet->playerId.binaryAddress, packet->playerId.port, interfaceType);
#endif
	//	ProcessUnhandledPacket(packet, ID_REMOTE_EXISTING_CONNECTION,interfaceType);
}

template <class InterfaceType>
void Multiplayer<InterfaceType>::ReceiveConnectionBanned(Packet *packet,InterfaceType *interfaceType)
{
	// We are banned from connecting to the system specified in packet->playerId
	// Peer or client
#ifdef _DEBUG
	printf("ID_CONNECTION_BANNED from PlayerID:%u:%u on %p.\n",packet->playerId.binaryAddress, packet->playerId.port, interfaceType);
#endif
	//	ProcessUnhandledPacket(packet, ID_REMOTE_EXISTING_CONNECTION,interfaceType);
}


template <class InterfaceType>
void Multiplayer<InterfaceType>::ReceiveConnectionRequestAccepted(Packet *packet, InterfaceType *interfaceType)
{
	// Our request to connect to another system has been accepted.  Client only.
#ifdef _DEBUG
	printf("ID_CONNECTION_REQUEST_ACCEPTED from PlayerID:%u:%u on %p.\n",packet->playerId.binaryAddress, packet->playerId.port, interfaceType);
#endif
//	ProcessUnhandledPacket(packet, ID_CONNECTION_REQUEST_ACCEPTED,interfaceType);
}

template <class InterfaceType>
void Multiplayer<InterfaceType>::ReceiveNewIncomingConnection(Packet *packet, InterfaceType *interfaceType)
{
	// Another system has requested to connect to us, which we have accepted.  Server or peer only.
#ifdef _DEBUG
	printf("ID_NEW_INCOMING_CONNECTION from PlayerID:%u:%u on %p.\n",packet->playerId.binaryAddress, packet->playerId.port, interfaceType);
#endif
//	ProcessUnhandledPacket(packet, ID_NEW_INCOMING_CONNECTION,interfaceType);

	// This will send all existing distributed objects to the new player
#ifdef __DISTRIBUTED_NETWORK_OBJECT_MANAGER_H
	DistributedNetworkObjectManager::Instance()->SendAllDistributedObjects(packet->playerId);
#endif
}

template <class InterfaceType>
void Multiplayer<InterfaceType>::ReceiveConnectionResumption(Packet *packet, InterfaceType *interfaceType)
{
	// Someone who was already connected to us connected again.  Server or peer only.
#ifdef _DEBUG
	printf("ID_CONNECTION_RESUMPTION from PlayerID:%u:%u on %p.\n",packet->playerId.binaryAddress, packet->playerId.port, interfaceType);
#endif
//	ProcessUnhandledPacket(packet, ID_CONNECTION_RESUMPTION,interfaceType);
}

template <class InterfaceType>
void Multiplayer<InterfaceType>::ReceiveNoFreeIncomingConnections(Packet *packet, InterfaceType *interfaceType)
{
	// The system we connected to has no free slots to connect to
	// Set free slots by calling SetMaximumIncomingConnections
	// Client or peer only.
#ifdef _DEBUG
	printf("ID_NO_FREE_INCOMING_CONNECTIONS from PlayerID:%u:%u on %p.\n",packet->playerId.binaryAddress, packet->playerId.port, interfaceType);
#endif
//	ProcessUnhandledPacket(packet, ID_NO_FREE_INCOMING_CONNECTIONS,interfaceType);
}

template <class InterfaceType>
void Multiplayer<InterfaceType>::ReceiveDisconnectionNotification(Packet *packet, InterfaceType *interfaceType)
{
	// A remote system terminated the connection.  packet->playerId specifies which remote system
#ifdef _DEBUG
	printf("ID_DISCONNECTION_NOTIFICATION from PlayerID:%u:%u on %p.\n",packet->playerId.binaryAddress, packet->playerId.port, interfaceType);
#endif
//	ProcessUnhandledPacket(packet, ID_DISCONNECTION_NOTIFICATION,interfaceType);
}

template <class InterfaceType>
void Multiplayer<InterfaceType>::ReceiveConnectionLost(Packet *packet, InterfaceType *interfaceType)
{
	// The network cannot send reliable packets so has terminated the connection. packet->playerId specifies which remote system
#ifdef _DEBUG
	printf("ID_CONNECTION_LOST from PlayerID:%u:%u on %p.\n",packet->playerId.binaryAddress, packet->playerId.port, interfaceType);
#endif
//	ProcessUnhandledPacket(packet, ID_CONNECTION_LOST,interfaceType);
}

template <class InterfaceType>
void Multiplayer<InterfaceType>::ReceivedStaticData(Packet *packet, InterfaceType *interfaceType)
{
	// Another system has just sent their static data to us (which we recorded automatically)
#ifdef _DEBUG
	printf("ID_RECEIVED_STATIC_DATA from PlayerID:%u:%u on %p.\n",packet->playerId.binaryAddress, packet->playerId.port, interfaceType);
#endif
//	ProcessUnhandledPacket(packet, ID_RECEIVED_STATIC_DATA,interfaceType);
}

template <class InterfaceType>
void Multiplayer<InterfaceType>::ReceiveInvalidPassword(Packet *packet, InterfaceType *interfaceType)
{
	// Our connection to another system was refused because the passwords do not match
#ifdef _DEBUG
	printf("ID_INVALID_PASSWORD from PlayerID:%u:%u on %p.\n",packet->playerId.binaryAddress, packet->playerId.port, interfaceType);
#endif
//	ProcessUnhandledPacket(packet, ID_INVALID_PASSWORD,interfaceType);
}

template <class InterfaceType>
void Multiplayer<InterfaceType>::ReceiveModifiedPacket(Packet *packet, InterfaceType *interfaceType)
{
	// The network layer has detected packet tampering
	// This does NOT automatically close the connection
#ifdef _DEBUG
	printf("ID_MODIFIED_PACKET from PlayerID:%u:%u on %p.\n",packet->playerId.binaryAddress, packet->playerId.port, interfaceType);
#endif
//	ProcessUnhandledPacket(packet, ID_MODIFIED_PACKET,interfaceType);
}

template <class InterfaceType>
void Multiplayer<InterfaceType>::ReceiveRemotePortRefused(Packet *packet, InterfaceType *interfaceType)
{
	// The remote system has responded with ICMP_PORT_UNREACHABLE
#ifdef _DEBUG
	printf("ID_REMOTE_PORT_REFUSED from PlayerID:%u:%u on %p.\n",packet->playerId.binaryAddress, packet->playerId.port, interfaceType);
#endif
//	ProcessUnhandledPacket(packet, ID_REMOTE_PORT_REFUSED,interfaceType);
}

template <class InterfaceType>
void Multiplayer<InterfaceType>::ReceiveVoicePacket(Packet *packet, InterfaceType *interfaceType)
{
	// We got a voice packet
#ifdef _DEBUG
	printf("ID_VOICE_PACKET from PlayerID:%u:%u on %p.\n",packet->playerId.binaryAddress, packet->playerId.port, interfaceType);
#endif
//	ProcessUnhandledPacket(packet, ID_VOICE_PACKET,interfaceType);
}

template <class InterfaceType>
void Multiplayer<InterfaceType>::ReceivePong(Packet *packet, InterfaceType *interfaceType)
{
	// Peer or client.  Response from a ping for an unconnected system.
#ifdef _DEBUG
	unsigned long time;
	memcpy((char*)&time, packet->data+sizeof(unsigned char), sizeof(unsigned long));
	printf("ID_PONG from PlayerID:%u:%u on %p. Value is %i\n",packet->playerId.binaryAddress, packet->playerId.port, interfaceType, time);
#endif
	//	ProcessUnhandledPacket(packet, ID_PONG,interfaceType);
}

template <class InterfaceType>
void Multiplayer<InterfaceType>::ReceiveDistributedNetworkObject(Packet *packet,InterfaceType *interfaceType)
{
	// Packet to create a distributed network object
#ifdef _DEBUG
	printf("ID_UPDATE_DISTRIBUTED_NETWORK_OBJECT from PlayerID:%u:%u on %p.\n",packet->playerId.binaryAddress, packet->playerId.port, interfaceType);
#endif

	#ifdef __DISTRIBUTED_NETWORK_OBJECT_MANAGER_H

	// This packet is only valid if the server or client was registered with the DistributedNetworkObjectManager
	// If you this this assert, you need to call
	// DistributedNetworkObjectManager::Instance()->RegisterRakServerInterface(myInstanceOfRakServer);
	// and / or
	// DistributedNetworkObjectManager::Instance()->RegisterRakClientInterface(myInstanceOfRakClient);
	assert(DistributedNetworkObjectManager::Instance()->GetRakClientInterface() || DistributedNetworkObjectManager::Instance()->GetRakServerInterface());

	DistributedNetworkObjectManager::Instance()->HandleDistributedNetworkObjectPacket(packet);
	#endif
}

template <class InterfaceType>
void Multiplayer<InterfaceType>::ReceiveDistributedNetworkObjectCreationAccepted(Packet *packet,InterfaceType *interfaceType)
{
	// Client only - creation of a distributed network object was accepted
#ifdef _DEBUG
	printf("ID_DISTRIBUTED_NETWORK_OBJECT_CREATION_ACCEPTED from PlayerID:%u:%u on %p.\n",packet->playerId.binaryAddress, packet->playerId.port, interfaceType);
#endif

#ifdef __DISTRIBUTED_NETWORK_OBJECT_MANAGER_H
	// This packet is only valid if the client was registered with the DistributedNetworkObjectManager
	// DistributedNetworkObjectManager::Instance()->RegisterRakClientInterface(myInstanceOfRakClient);
	assert(DistributedNetworkObjectManager::Instance()->GetRakClientInterface());

	DistributedNetworkObjectManager::Instance()->HandleDistributedNetworkObjectPacketCreationAccepted(packet);
#endif
}

template <class InterfaceType>
void Multiplayer<InterfaceType>::ReceiveDistributedNetworkObjectCreationRejected(Packet *packet,InterfaceType *interfaceType)
{
	// Client only - creation of a distributed network object was accepted
#ifdef _DEBUG
	printf("ID_DISTRIBUTED_NETWORK_OBJECT_CREATION_REJECTED from PlayerID:%u:%u on %p.\n",packet->playerId.binaryAddress, packet->playerId.port, interfaceType);
#endif

#ifdef __DISTRIBUTED_NETWORK_OBJECT_MANAGER_H
	// This packet is only valid if the client was registered with the DistributedNetworkObjectManager
	// DistributedNetworkObjectManager::Instance()->RegisterRakClientInterface(myInstanceOfRakClient);
	assert(DistributedNetworkObjectManager::Instance()->GetRakClientInterface());

	DistributedNetworkObjectManager::Instance()->HandleDistributedNetworkObjectPacketCreationRejected(packet);
#endif
}

template <class InterfaceType>
void Multiplayer<InterfaceType>::ReceiveAutopatcherRequestFileList(Packet *packet, InterfaceType *interfaceType)
{
	// Request for a list of files served by the autopatcher.
	// Send to Autopatcher::SendDownloadableFileList(packet->playerId)
#ifdef _DEBUG
	printf("ID_AUTOPATCHER_REQUEST_FILE_LIST from PlayerID:%u:%u on %p.\n",packet->playerId.binaryAddress, packet->playerId.port, interfaceType);
#endif
	// ProcessUnhandledPacket(packet, ID_AUTOPATCHER_REQUEST_FILE_LIST,interfaceType);
}

template <class InterfaceType>
void Multiplayer<InterfaceType>::ReceiveAutopatcherFileList(Packet *packet, InterfaceType *interfaceType)
{
	// Got a list of files served by a remote autopatcher
	// Send to Autopatcher::OnAutopatcherFileList(packet, bool);
#ifdef _DEBUG
	printf("ID_AUTOPATCHER_FILE_LIST from PlayerID:%u:%u on %p.\n",packet->playerId.binaryAddress, packet->playerId.port, interfaceType);
#endif
	// ProcessUnhandledPacket(packet, ID_AUTOPATCHER_FILE_LIST,interfaceType);
}

template <class InterfaceType>
void Multiplayer<InterfaceType>::ReceiveAutopatcherRequestFiles(Packet *packet, InterfaceType *interfaceType)
{
	// Got a request for a set of files from our autopatcher
	// Send to Autopatcher::OnAutopatcherRequestFiles(packet);
#ifdef _DEBUG
	printf("ID_AUTOPATCHER_REQUEST_FILES from PlayerID:%u:%u on %p.\n",packet->playerId.binaryAddress, packet->playerId.port, interfaceType);
#endif
	// ProcessUnhandledPacket(packet, ID_AUTOPATCHER_REQUEST_FILES,interfaceType);
}

template <class InterfaceType>
void Multiplayer<InterfaceType>::ReceiveAutopatcherSetDownloadList(Packet *packet, InterfaceType *interfaceType)
{
	// Got confirmation of what files we are about to get from the remote autopatcher
	// Send to Autopatcher::OnAutopatcherSetDownloadList(packet);
#ifdef _DEBUG
	printf("ID_AUTOPATCHER_SET_DOWNLOAD_LIST from PlayerID:%u:%u on %p.\n",packet->playerId.binaryAddress, packet->playerId.port, interfaceType);
#endif
	// ProcessUnhandledPacket(packet, ID_AUTOPATCHER_SET_DOWNLOAD_LIST,interfaceType);
}

template <class InterfaceType>
void Multiplayer<InterfaceType>::ReceiveAutopatcherWriteFile(Packet *packet, InterfaceType *interfaceType)
{
	// Got a file from a remote autopatcher
	// Send to Autopatcher::OnAutopatcherWriteFile(packet);
#ifdef _DEBUG
	printf("ID_AUTOPATCHER_WRITE_FILE from PlayerID:%u:%u on %p.\n",packet->playerId.binaryAddress, packet->playerId.port, interfaceType);
#endif
	// ProcessUnhandledPacket(packet, ID_AUTOPATCHER_WRITE_FILE,interfaceType);
}

#endif
