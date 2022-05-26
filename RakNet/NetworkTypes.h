// This file is part of RakNet Copyright 2003, 2004 Rakkarsoft LLC and Kevin Jenkins.

// Usage of Raknet is subject to the appropriate licence agreement.
// "Shareware" Licensees with Rakkarsoft LLC are subject to the shareware license found at http://www.rakkarsoft.com/shareWareLicense.html which you agreed to upon purchase of a "Shareware license"
// "Commercial" Licensees with Rakkarsoft LLC are subject to the commercial license found at http://www.rakkarsoft.com/sourceCodeLicense.html which you agreed to upon purchase of a "Commercial license"
// All other users are subject to the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

// Refer to the appropriate license agreement for distribution, modification, and warranty rights.

#ifndef __NETWORK_TYPES_H
#define __NETWORK_TYPES_H

typedef unsigned short ObjectID;
typedef unsigned char UniqueIDType;
typedef unsigned short PlayerIndex;

struct PlayerID
{
	unsigned long binaryAddress; // From inet_addr
	unsigned short port;
	PlayerID& PlayerID::operator = (const PlayerID& input) {binaryAddress=input.binaryAddress; port=input.port; return *this;}
	friend int operator==(const PlayerID& left, const PlayerID& right);
	friend int operator!=(const PlayerID& left, const PlayerID& right);
	friend int operator > (const PlayerID& left, const PlayerID& right);
	friend int operator < (const PlayerID& left, const PlayerID& right);
};


struct RequestedConnectionStruct
{
	PlayerID playerId; // Who we wanted to connect to
	unsigned long time; // When we requested this connection
	unsigned char AESKey[16];
	bool setAESKey;
	unsigned long nextRequestTime;
};

struct Packet
{
	PlayerIndex playerIndex; // Server only - this is the index into the player array that this playerId maps to
	PlayerID playerId;
	unsigned long length;
	unsigned long bitSize; // Same as length but represents bits.  Length is obsolete and retained for backwards compatibility
	unsigned char* data;
};

#pragma pack(push,1)
#pragma pack(1)
struct ConnectionAcceptStruct
{
	unsigned char typeId;
	unsigned short remotePort;
	PlayerID externalID; // We tell the remote system its own IP / port this way
	PlayerIndex playerIndex; // For internal use
};

#pragma pack(1)
struct PingStruct
{
	unsigned char typeId; // ID_PING or ID_PONG
	unsigned long sendPingTime;
	unsigned long sendPongTime;
};

#pragma pack(1)
struct UnconnectedPingStruct
{
	unsigned char typeId; // ID_PING or ID_PONG
	unsigned long sendPingTime;
};

// Timestamp automatically used for this type of packet
#pragma pack(1)
struct SetRandomNumberSeedStruct
{
	unsigned char ts; // ID_TIMESTAMP
	unsigned long timeStamp;
	unsigned char typeId; // ID_SET_RANDOM_NUMBER_SEED
	unsigned long seed;
	unsigned long nextSeed;
};

#pragma pack(1)
struct NewIncomingConnectionStruct
{
	unsigned char typeId; // ID_NEW_INCOMING_CONNECTION
	PlayerID externalID; // We tell the remote system its own IP / port this way
};

#pragma pack(pop)

const PlayerIndex UNASSIGNED_PLAYER_INDEX=65535;
const PlayerID UNASSIGNED_PLAYER_ID={0xFFFFFFFF, 0xFFFF};
const ObjectID UNASSIGNED_OBJECT_ID=65535;
const int PING_TIMES_ARRAY_SIZE=5;

//-----------------------------------------------------
// RPC FUNCTION IMPLEMENTATION
// --------------------------------------------------------
//
// Use the following C function prototype for your callbacks
// void functionName(char *input, int numberOfBitsOfData, PlayerID sender);
//
// If you pass input data, you can parse the input data in two ways.
//
// 1.
// Cast input to a struct (such as if you sent a struct)
// i.e. MyStruct *s = (MyStruct*) input;
// Make sure that the sizeof(MyStruct) is equal to the number of bytes passed!
//
// 2.
// Create a BitStream instance with input as data and the number of bytes
// i.e. BitStream myBitStream(input, (numberOfBitsOfData-1)/8+1)
//
// (numberOfBitsOfData-1)/8+1 is how convert from bits to bytes
//
// Full example:
//
// void MyFunc(char *input, int numberOfBitsOfData, PlayerID sender) {}
// RakClient *rakClient;
// REGISTER_AS_REMOTE_PROCEDURE_CALL(rakClient, MyFunc);
// This would allow MyFunc to be called from the server using  (for example)
// rakServer->RPC("MyFunc", 0, clientID, false);

#define REGISTER_AS_REMOTE_PROCEDURE_CALL(networkObject, functionName) (networkObject)->RegisterAsRemoteProcedureCall((#functionName),(functionName))
// Unregisters a remote procedure call
#define UNREGISTER_AS_REMOTE_PROCEDURE_CALL(networkObject,functionName) (networkObject)->UnregisterAsRemoteProcedureCall((#functionName))

#endif

