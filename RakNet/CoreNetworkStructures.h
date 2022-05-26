// ----------------------------------------------------------------------
// This API and the code herein created by and wholly and privately owned by Kevin Jenkins except where specifically indicated otherwise.
// Licensed under the "RakNet" brand by Rakkarsoft LLC and subject to the terms of the relevant licensing agreement available at http://www.rakkarsoft.com
// CoreNetworkStructures.h
// File created January 24, 2003
// The built-in network structures used internally by the network engine
// ----------------------------------------------------------------------

#include "CoreNetworking.h"
#include "PacketEnumerations.h"
#include "NetworkStructures.h"

#ifndef __CORE_NETWORK_STRUCTURES_H
#define __CORE_NETWORK_STRUCTURES_H


// ************************
// PACKET STRUCTURES
// ************************
//
// For each structure
// Put #pragma pack(1) before the structure so data is not aligned
// Use either
// unsigned char useTimestamp;  // ID_TIMESTAMP
// unsigned long timeStamp;
// unsigned char typeId;
// OR
// just start with unsigned char typeId
//--------------------------------------------------------------------------

#pragma pack(push)

// Timestamp automatically used for this type of packet
#pragma pack(1)
struct PingStruct
{
	unsigned char typeId; // ID_PING or ID_PONG
	long timeStamp;
};

// Timestamp automatically used for this type of packet
#pragma pack(1)
struct ConnectionVerificationStruct
{
	unsigned char typeId; // ID_CONNECTION_VERIFICATION
	long timeStamp;
	unsigned long validationInteger;
	char password[CONNECTION_VERIFICATION_PASSWORD_LENGTH];
};

// Timestamp automatically used for this type of packet
#pragma pack(1)
struct SetRandomNumberSeedStruct
{
	unsigned char typeId; // ID_SET_RANDOM_NUMBER_SEED
	long timeStamp;
	unsigned long seed;
	unsigned long nextSeed;
};

// The most player pings we'll transmit
#define MAX_PLAYER_PINGS 32

#pragma pack(1)
struct PlayerPingsStruct
{
	PlayerPingsStruct() : typeId(ID_PLAYER_PINGS)  {}
	const unsigned char typeId; //ID_PLAYER_PINGS
	short playerPings[MAX_PLAYER_PINGS];
};

#pragma pack(1)
struct NewClientAttachedToServer
{
	unsigned char typeId; // ID_NEW_CLIENT
	PlayerID id;  // The player ID of the player that has connected.  You should setup your own system to request and send initilization data for the new client
};

#pragma pack(1)
struct ConnectionAcceptStruct
{
	unsigned char typeId; // ID_CONNECTION_ACCEPT
};

#pragma pack(1)
struct DesignateRemotePortStruct
{
	unsigned char typeId; // ID_DESIGNATE_REMOTE_PORT
	unsigned short port;
};

#pragma pack(pop)
#endif
