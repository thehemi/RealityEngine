// This file is part of RakNet Copyright 2003, 2004 Rakkarsoft LLC and Kevin Jenkins.

// Usage of Raknet is subject to the appropriate licence agreement.
// "Shareware" Licensees with Rakkarsoft LLC are subject to the shareware license found at http://www.rakkarsoft.com/shareWareLicense.html which you agreed to upon purchase of a "Shareware license"
// "Commercial" Licensees with Rakkarsoft LLC are subject to the commercial license found at http://www.rakkarsoft.com/sourceCodeLicense.html which you agreed to upon purchase of a "Commercial license"
// All other users are subject to the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

// Refer to the appropriate license agreement for distribution, modification, and warranty rights.

#include "RakServer.h"
#include "PacketEnumerations.h"
#include "GetTime.h"
#include "Rand.h"

// Defined in rand.cpp
extern void seedMT(unsigned long seed);
extern unsigned long randomMT(void);

RakServer::RakServer()
{
	nextSeedUpdate=0;
	synchronizedRandomInteger=false;
	relayStaticClientData=false;
	broadcastPingsTime=0L;
}

RakServer::~RakServer()
{
}

void RakServer::InitializeSecurity(char *RSAp, char *RSAq)
{
	RakPeer::InitializeSecurity(0,0, RSAp, RSAq);
}

void RakServer::DisableSecurity(void)
{
	RakPeer::DisableSecurity();
}

bool RakServer::Start(unsigned short AllowedPlayers, unsigned long connectionValidationInteger, int threadSleepTimer, unsigned short port)
{
	bool init;

	RakPeer::Disconnect(0L);

	init=RakPeer::Initialize(AllowedPlayers, port,threadSleepTimer);
	RakPeer::SetMaximumIncomingConnections(AllowedPlayers);

	// Random number seed
	long time = RakNetGetTime();
	seedMT(time);
	seed=randomMT();
	if (seed % 2 == 0) // Even
		seed--; // make odd
	nextSeed=randomMT();
	if (nextSeed % 2 == 0) // Even
		nextSeed--; // make odd

	return init;
}

void RakServer::SetPassword(char *_password)
{
	if (_password)
	{
		_password[19]=0; // Limit the password length
		RakPeer::SetIncomingPassword(_password, (int)strlen(_password)+1);
	}
	else
	{
		RakPeer::SetIncomingPassword(0, 0);
	}
}

bool RakServer::HasPassword(void)
{
	return GetIncomingPassword()->GetNumberOfBytesUsed() > 0;
}

void RakServer::Disconnect(unsigned long blockDuration)
{
	RakPeer::Disconnect(blockDuration);
}

bool RakServer::Send(char *data, const long length, PacketPriority priority, PacketReliability reliability, char orderingStream, PlayerID playerId, bool broadcast)
{
	return RakPeer::Send(data, length, priority, reliability, orderingStream, playerId, broadcast);
}

bool RakServer::Send(RakNet::BitStream *bitStream, PacketPriority priority, PacketReliability reliability, char orderingStream, PlayerID playerId, bool broadcast)
{
	return RakPeer::Send(bitStream, priority, reliability, orderingStream, playerId, broadcast);
}

Packet* RakServer::Receive(void)
{
	Packet *packet = RakPeer::Receive();

	// This is just a regular time based update.  Nowhere else good to put it
	if (RakPeer::IsActive() && occasionalPing)
	{
		unsigned long time = RakNetGetTime();
		if (time > broadcastPingsTime || (packet && packet->data[0]==ID_RECEIVED_STATIC_DATA))
		{
			if (time > broadcastPingsTime)
				broadcastPingsTime = time + 30000; // Broadcast pings every 30 seconds

			unsigned i, count;
			RemoteSystemStruct *remoteSystem;
			RakNet::BitStream bitStream((sizeof(PlayerID) + sizeof(short)) * 32 + sizeof(unsigned char));
			unsigned char typeId=ID_BROADCAST_PINGS;
			bitStream.Write(typeId);
			for (i=0, count=0; count < 32 && i < maximumNumberOfPeers; i++)
			{
				remoteSystem=remoteSystemList+i;
				if (remoteSystem->playerId!=UNASSIGNED_PLAYER_ID)
				{
					bitStream.Write(remoteSystem->playerId.binaryAddress);
					bitStream.Write(remoteSystem->playerId.port);
					bitStream.Write(remoteSystem->pingAndClockDifferential[remoteSystem->pingAndClockDifferentialWriteIndex].pingTime);
					count++;
				}
			}

			if (count>0) // If we wrote anything
			{
				if (packet && packet->data[0]==ID_NEW_INCOMING_CONNECTION) // If this was a new connection
					Send(&bitStream, SYSTEM_PRIORITY, RELIABLE, 0, packet->playerId, false); // Send to the new connection
				else
					Send(&bitStream, SYSTEM_PRIORITY, RELIABLE, 0, UNASSIGNED_PLAYER_ID, true); // Send to everyone
			}
		}
	}

	// This is just a regular time based update.  Nowhere else good to put it
	if (RakPeer::IsActive() && synchronizedRandomInteger)
	{
		unsigned long time = RakNetGetTime();
		if (time > nextSeedUpdate || (packet && packet->data[0]==ID_NEW_INCOMING_CONNECTION))
		{
			if (time > nextSeedUpdate)
				nextSeedUpdate = time + 9000; // Seeds are updated every 9 seconds

			seed=nextSeed;
			nextSeed=randomMT();
			if (nextSeed % 2 == 0) // Even
				nextSeed--; // make odd

			SetRandomNumberSeedStruct s;
			s.ts=ID_TIMESTAMP;
			s.timeStamp=RakNetGetTime();
			s.typeId=ID_SET_RANDOM_NUMBER_SEED;
			s.seed=seed;
			s.nextSeed=nextSeed;

			if (packet && packet->data[0]==ID_NEW_INCOMING_CONNECTION)
				Send((char*)&s, sizeof(SetRandomNumberSeedStruct),  SYSTEM_PRIORITY, RELIABLE, 0, packet->playerId, false);
			else
				Send((char*)&s, sizeof(SetRandomNumberSeedStruct),  SYSTEM_PRIORITY, RELIABLE, 0, UNASSIGNED_PLAYER_ID, true);
		}
	}

	if (packet)
	{
		packet->playerIndex=(PlayerIndex)RakPeer::GetIndexFromPlayerID(packet->playerId);
		if (packet->playerIndex==65535)
		{
			DeallocatePacket(packet);
			return 0;
		}
			
		// Intercept specific client / server feature packets.  This will do an extra send and still pass on the data to the user
		if (packet->data[0]==ID_RECEIVED_STATIC_DATA)
		{
			if (relayStaticClientData)
			{
				// Relay static data to the other systems but the sender
				RakNet::BitStream bitStream(packet->length + sizeof(PlayerID));
				unsigned char typeId=ID_RECEIVED_RELAYED_STATIC_DATA;
				bitStream.Write(typeId);
				bitStream.Write(packet->playerId.binaryAddress);
				bitStream.Write(packet->playerId.port);
				bitStream.Write((char*)packet->data+sizeof(unsigned char), packet->length-sizeof(unsigned char));
				Send(&bitStream, SYSTEM_PRIORITY, RELIABLE, 0, UNASSIGNED_PLAYER_ID, true);
			}			
		}
		else if (packet->data[0]==ID_DISCONNECTION_NOTIFICATION || packet->data[0]==ID_CONNECTION_LOST || packet->data[0]==ID_NEW_INCOMING_CONNECTION)
		{
			// Relay the disconnection
			RakNet::BitStream bitStream(packet->length + sizeof(PlayerID));
			unsigned char typeId;
			if (packet->data[0]==ID_DISCONNECTION_NOTIFICATION)
				typeId=ID_REMOTE_DISCONNECTION_NOTIFICATION;
			else if (packet->data[0]==ID_CONNECTION_LOST)
				typeId=ID_REMOTE_CONNECTION_LOST;
			else
				typeId=ID_REMOTE_NEW_INCOMING_CONNECTION;
			bitStream.Write(typeId);
			bitStream.Write(packet->playerId.binaryAddress);
			bitStream.Write(packet->playerId.port);
			Send(&bitStream, SYSTEM_PRIORITY, RELIABLE, 0, packet->playerId, true);

			if (packet->data[0]==ID_NEW_INCOMING_CONNECTION)
			{
				unsigned i;
				for (i=0; i < maximumNumberOfPeers; i++)
				{
					if (remoteSystemList[i].playerId!=UNASSIGNED_PLAYER_ID && packet->playerId!=remoteSystemList[i].playerId)
					{
						bitStream.Reset();
						typeId=ID_REMOTE_EXISTING_CONNECTION;
						bitStream.Write(typeId);
						bitStream.Write(remoteSystemList[i].playerId.binaryAddress);
						bitStream.Write(remoteSystemList[i].playerId.port);
						// One send to tell them of the connection
						Send(&bitStream, SYSTEM_PRIORITY, RELIABLE, 0, packet->playerId, false);

						if (relayStaticClientData)
						{
							bitStream.Reset();
							typeId=ID_RECEIVED_RELAYED_STATIC_DATA;
							bitStream.Write(typeId);
							bitStream.Write(remoteSystemList[i].playerId.binaryAddress);
							bitStream.Write(remoteSystemList[i].playerId.port);
							bitStream.Write((char*)remoteSystemList[i].staticData.GetData(), remoteSystemList[i].staticData.GetNumberOfBytesUsed());
							// Another send to tell them of the static data
							Send(&bitStream, SYSTEM_PRIORITY, RELIABLE, 0, packet->playerId, false);
						}
					}
				}
			}
		}
	}

	return packet;
}

void RakServer::Kick(PlayerID playerId)
{
	RakPeer::CloseConnection(playerId, true, 0L);
}

void RakServer::DeallocatePacket(Packet *packet)
{
	RakPeer::DeallocatePacket(packet);
}

void RakServer::SetAllowedPlayers(unsigned short AllowedPlayers)
{
	RakPeer::SetMaximumIncomingConnections(AllowedPlayers);
}

unsigned short RakServer::GetAllowedPlayers(void) const
{
	return RakPeer::GetMaximumIncomingConnections();
}

unsigned short RakServer::GetConnectedPlayers(void)
{
	unsigned short numberOfSystems;

	RakPeer::GetConnectionList(0, &numberOfSystems);
	return numberOfSystems;
}

void RakServer::GetPlayerIPFromID(PlayerID playerId, char returnValue[22], unsigned short *port)
{
	*port=playerId.port;
	strcpy(returnValue, RakPeer::PlayerIDToDottedIP(playerId));
}

void RakServer::PingPlayer(PlayerID playerId)
{
	RakPeer::Ping(playerId);
}

int RakServer::GetAveragePing(PlayerID playerId)
{
	return RakPeer::GetAveragePing(playerId);
}

int RakServer::GetLastPing(PlayerID playerId)
{
	return RakPeer::GetLastPing(playerId);
}

int RakServer::GetLowestPing(PlayerID playerId)
{
	return RakPeer::GetLowestPing(playerId);
}

void RakServer::StartOccasionalPing(void)
{
	RakPeer::SetOccasionalPing(true);
}

void RakServer::StopOccasionalPing(void)
{
	RakPeer::SetOccasionalPing(false);
}

bool RakServer::IsActive(void) const
{
	return RakPeer::IsActive();
}

unsigned long RakServer::GetSynchronizedRandomInteger(void) const
{
	return seed;
}

void RakServer::StartSynchronizedRandomInteger(void)
{
	synchronizedRandomInteger=true;
}

void RakServer::StopSynchronizedRandomInteger(void)
{
	synchronizedRandomInteger=false;
}

/*

void RakServer::SynchronizeMemory(UniqueIDType uniqueIdentifier, char *memoryBlock, unsigned short size, bool isAuthority, bool (*synchronizationRules) (char*,char*),ObjectID secondaryUniqueIdentifier)
{
	RakPeer::SynchronizeMemory(uniqueIdentifier, memoryBlock, size, isAuthority, synchronizationRules, secondaryUniqueIdentifier);
}

void RakServer::DesynchronizeMemory(UniqueIDType uniqueIdentifier, ObjectID secondaryUniqueIdentifier)
{
	RakPeer::DesynchronizeMemory(uniqueIdentifier, secondaryUniqueIdentifier);
}

void RakServer::DesynchronizeAllMemory(void)
{
	RakPeer::DesynchronizeAllMemory();
}
*/

bool RakServer::GenerateCompressionLayer(unsigned long inputFrequencyTable[256], bool inputLayer)
{
	return RakPeer::GenerateCompressionLayer(inputFrequencyTable, inputLayer);
}

bool RakServer::DeleteCompressionLayer(bool inputLayer)
{
	return RakPeer::DeleteCompressionLayer(inputLayer);
}

void RakServer::RegisterAsRemoteProcedureCall(char* uniqueID, void (*functionName)(char *input, int numberOfBitsOfData, PlayerID sender))
{
	RakPeer::RegisterAsRemoteProcedureCall(uniqueID, functionName);
}

void RakServer::UnregisterAsRemoteProcedureCall(char* uniqueID)
{
	RakPeer::UnregisterAsRemoteProcedureCall(uniqueID);
}

bool RakServer::RPC(char* uniqueID, char *data, long bitLength, PacketPriority priority, PacketReliability reliability, char orderingStream, PlayerID playerId, bool broadcast, bool shiftTimestamp)
{
		return RakPeer::RPC(uniqueID, data, bitLength, priority, reliability, orderingStream, playerId, broadcast, shiftTimestamp);
}

bool RakServer::RPC(char* uniqueID, RakNet::BitStream *parameters, PacketPriority priority, PacketReliability reliability, char orderingStream, PlayerID playerId, bool broadcast, bool shiftTimestamp)
{
	return RakPeer::RPC(uniqueID, parameters, priority, reliability, orderingStream, playerId, broadcast, shiftTimestamp);
}

/*
bool RakServer::HandleRPCPacket(Packet* packet)
{
	return RakPeer::HandleRPCPacket(packet->data, packet->length, packet->playerId);
}*/

void RakServer::SetTrackFrequencyTable(bool b)
{
	RakPeer::SetCompileFrequencyTable(b);
}

bool RakServer::GetSendFrequencyTable(unsigned long outputFrequencyTable[256])
{
	return RakPeer::GetOutgoingFrequencyTable(outputFrequencyTable);
}

float RakServer::GetCompressionRatio(void) const
{
	return RakPeer::GetCompressionRatio();
}

float RakServer::GetDecompressionRatio(void) const
{
	return RakPeer::GetDecompressionRatio();
}

RakNet::BitStream * RakServer::GetStaticServerData(void)
{
	return RakPeer::GetRemoteStaticData(myPlayerId);
}

void RakServer::SetStaticServerData(char *data, const long length)
{
	RakPeer::SetRemoteStaticData(myPlayerId, data, length);
}

void RakServer::SetRelayStaticClientData(bool b)
{
	relayStaticClientData=b;
}

void RakServer::SendStaticServerDataToClient(PlayerID playerId)
{
	RakPeer::SendStaticData(playerId);
}

RakNet::BitStream * RakServer::GetStaticClientData(PlayerID playerId)
{
	return RakPeer::GetRemoteStaticData(playerId);
}

void RakServer::SetStaticClientData(PlayerID playerId, char *data, const long length)
{
	RakPeer::SetRemoteStaticData(playerId, data, length);
}

// This will read the data from playerChangedId and send it to playerToSendToId
void RakServer::ChangeStaticClientData(PlayerID playerChangedId, PlayerID playerToSendToId)
{
	RemoteSystemStruct *remoteSystem = GetRemoteSystemFromPlayerID(playerChangedId);
	if (remoteSystem==0)
		return; // No such playerChangedId

    // Relay static data to the other systems but the sender
	RakNet::BitStream bitStream(remoteSystem->staticData.GetNumberOfBytesUsed() + sizeof(PlayerID) + sizeof(unsigned char));
	unsigned char typeId=ID_RECEIVED_RELAYED_STATIC_DATA;
	bitStream.Write(typeId);
	bitStream.Write(playerChangedId.binaryAddress);
	bitStream.Write(playerChangedId.port);
	bitStream.Write((char*)remoteSystem->staticData.GetData(), remoteSystem->staticData.GetNumberOfBytesUsed());
	Send(&bitStream, SYSTEM_PRIORITY, RELIABLE, 0, playerToSendToId, true);
}

unsigned int RakServer::GetNumberOfAddresses(void)
{
	return RakPeer::GetNumberOfAddresses();
}

const char* RakServer::GetLocalIP(unsigned int index)
{
	return RakPeer::GetLocalIP(index);
}

void RakServer::PushBackPacket(Packet *packet)
{
	RakPeer::PushBackPacket(packet);
}

int RakServer::GetIndexFromPlayerID(PlayerID playerId)
{
	return RakPeer::GetIndexFromPlayerID(playerId);
}

PlayerID RakServer::GetPlayerIDFromIndex(int index)
{
	return RakPeer::GetPlayerIDFromIndex(index);
}

void RakServer::AddToBanList(const char *IP)
{
	RakPeer::AddToBanList(IP);
}

void RakServer::RemoveFromBanList(const char *IP)
{
	RakPeer::RemoveFromBanList(IP);
}

void RakServer::ClearBanList(void)
{
	RakPeer::ClearBanList();
}

bool RakServer::IsBanned(const char *IP)
{
	return RakPeer::IsBanned(IP);
}

bool RakServer::IsActivePlayerID(PlayerID playerId)
{
	return RakPeer::GetRemoteSystemFromPlayerID(playerId)!=0;
}

bool RakServer::SetMTUSize(int size)
{
	return RakPeer::SetMTUSize(size);
}

int RakServer::GetMTUSize(void) const
{
	return RakPeer::GetMTUSize();
}
RakNetStatisticsStruct * const RakServer::GetStatistics(PlayerID playerId)
{
	return RakPeer::GetStatistics(playerId);
}