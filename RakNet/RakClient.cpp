// This file is part of RakNet Copyright 2003, 2004 Rakkarsoft LLC and Kevin Jenkins.

// Usage of Raknet is subject to the appropriate licence agreement.
// "Shareware" Licensees with Rakkarsoft LLC are subject to the shareware license found at http://www.rakkarsoft.com/shareWareLicense.html which you agreed to upon purchase of a "Shareware license"
// "Commercial" Licensees with Rakkarsoft LLC are subject to the commercial license found at http://www.rakkarsoft.com/sourceCodeLicense.html which you agreed to upon purchase of a "Commercial license"
// All other users are subject to the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

// Refer to the appropriate license agreement for distribution, modification, and warranty rights.

#include "RakClient.h"
#include "PacketEnumerations.h"
#include "GetTime.h"

// Constructor
RakClient::RakClient()
{
	password[0]=0;

	unsigned i;
	for (i=0; i < 32; i++)
		otherClients[i].isActive=false;

	nextSeedUpdate=0;
}

// Destructor
RakClient::~RakClient()
{
}

bool RakClient::Connect(char* host, unsigned short serverPort, unsigned short clientPort, unsigned long connectionValidationInteger, int threadSleepTimer)
{
	RakPeer::Disconnect(0L);

	RakPeer::Initialize(1, clientPort,threadSleepTimer);

	if (host[0] < '0' || host[0] > '2')
	{
		host = (char*) SocketLayer::Instance()->DomainNameToIP(host);
	}

	unsigned i;
	for (i=0; i < 32; i++)
	{
		otherClients[i].isActive=false;
		otherClients[i].playerId=UNASSIGNED_PLAYER_ID;
		otherClients[i].staticData.Reset();
	}

	// ignore connectionValidationInteger.  A pointless variable
	return RakPeer::Connect(host, serverPort, password, (int)strlen(password));
}

void RakClient::Disconnect(unsigned long blockDuration)
{
	RakPeer::Disconnect(blockDuration);
}

void RakClient::InitializeSecurity(char *RSAe, char *RSAn)
{
	RakPeer::InitializeSecurity(RSAe, RSAn,0,0);
}

void RakClient::SetPassword(char *_password)
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

bool RakClient::HasPassword(void) const
{
	return password[0]!=0;
}

bool RakClient::Send(char *data, const long length, PacketPriority priority, PacketReliability reliability, char orderingStream)
{
	if (remoteSystemList==0)
		return false;
	return RakPeer::Send(data, length, priority, reliability, orderingStream, remoteSystemList[0].playerId, false);
}

bool RakClient::Send(RakNet::BitStream * bitStream, PacketPriority priority, PacketReliability reliability, char orderingStream)
{
	if (remoteSystemList==0)
		return false;
	return RakPeer::Send(bitStream, priority, reliability, orderingStream, remoteSystemList[0].playerId, false);
}

Packet* RakClient::Receive(void)
{
	Packet *packet = RakPeer::Receive();

	// Intercept specific client / server feature packets
	if (packet)
	{
		RakNet::BitStream bitStream((char*)packet->data, packet->length, false);
		bitStream.IgnoreBits(8); // Ignore identifier
		int i;
		if (packet->data[0]==ID_CONNECTION_REQUEST_ACCEPTED)
		{
			ConnectionAcceptStruct* cas = (ConnectionAcceptStruct*) packet->data;
			localPlayerIndex = cas->playerIndex;
		}
		else if (
			packet->data[0]==ID_REMOTE_NEW_INCOMING_CONNECTION ||
			packet->data[0]==ID_REMOTE_EXISTING_CONNECTION ||
			packet->data[0]==ID_REMOTE_DISCONNECTION_NOTIFICATION ||
			packet->data[0]==ID_REMOTE_CONNECTION_LOST)
		{
			bitStream.Read(packet->playerId.binaryAddress);
			bitStream.Read(packet->playerId.port);

			i = GetOtherClientIndexByPlayerID(packet->playerId);
			if (i>=0)
				otherClients[i].isActive=false;
		}
		else if (packet->data[0]==ID_RECEIVED_RELAYED_STATIC_DATA)
		{
			PlayerID staticDataOriginator;
			bitStream.Read(staticDataOriginator.binaryAddress);
			bitStream.Read(staticDataOriginator.port);

			i = GetOtherClientIndexByPlayerID(staticDataOriginator);
			if (i < 0)
				i=GetFreeOtherClientIndex();
			if (i>=0)
			{
				otherClients[i].playerId=staticDataOriginator;
				otherClients[i].isActive=true;
				otherClients[i].staticData.Reset();
				// The static data is what is left over in the stream
				otherClients[i].staticData.Write((char*)bitStream.GetData()+BITS_TO_BYTES(bitStream.GetReadOffset()), bitStream.GetNumberOfBytesUsed()-BITS_TO_BYTES(bitStream.GetReadOffset()));
				DeallocatePacket(packet);
				return 0;
			}
		}
		else if (packet->data[0]==ID_BROADCAST_PINGS)
		{
			PlayerID playerId;
			int index;
			for (i=0; i < 32; i++)
			{
				if (bitStream.Read(playerId.binaryAddress)==false)
					break; // No remaining data!
				bitStream.Read(playerId.port);
				index = GetOtherClientIndexByPlayerID(playerId);
				if (index >= 0)
					bitStream.Read(otherClients[index].ping);
				else
				{
					index = GetFreeOtherClientIndex();
					if (index >= 0)
					{
						otherClients[index].isActive=true;
						bitStream.Read(otherClients[index].ping);
						otherClients[index].playerId=playerId;
						otherClients[index].staticData.Reset();
					}
					else
						bitStream.IgnoreBits(sizeof(short)*8);
				}
			}
			DeallocatePacket(packet);
			return 0;
		}
		else if (packet->data[0]==ID_TIMESTAMP &&
			packet->length==sizeof(SetRandomNumberSeedStruct) &&
			packet->data[sizeof(unsigned char) + sizeof(unsigned long)]==ID_SET_RANDOM_NUMBER_SEED)
		{
			SetRandomNumberSeedStruct *s = (SetRandomNumberSeedStruct*)packet->data;

			seed = s->seed;
			nextSeed=s->nextSeed;
			nextSeedUpdate=s->timeStamp+9000; // Seeds are updated every 9 seconds

			DeallocatePacket(packet);
			return 0;
		}
	}

	return packet;
}

void RakClient::DeallocatePacket(Packet *packet)
{
	RakPeer::DeallocatePacket(packet);
}

void RakClient::PingServer(void)
{
	if (remoteSystemList==0)
		return;
	RakPeer::Ping(remoteSystemList[0].playerId);
}

void RakClient::PingServer(char* host, unsigned short serverPort, unsigned short clientPort)
{
	RakPeer::Initialize(1, clientPort,0);
	RakPeer::Ping(host, serverPort);
}

int RakClient::GetAveragePing(void)
{
	if (remoteSystemList==0)
		return -1;
	return RakPeer::GetAveragePing(remoteSystemList[0].playerId);
}

int RakClient::GetLastPing(void) const
{
	if (remoteSystemList==0)
		return -1;
	return RakPeer::GetLastPing(remoteSystemList[0].playerId);
}

int RakClient::GetLowestPing(void) const
{
	if (remoteSystemList==0)
		return -1;
	return RakPeer::GetLowestPing(remoteSystemList[0].playerId);
}

int RakClient::GetPlayerPing(PlayerID playerId)
{
	int i;
	for (i=0; i < 32; i++)
		if (otherClients[i].playerId==playerId)
			return otherClients[i].ping;
	return -1;
}

void RakClient::StartOccasionalPing(void)
{
	RakPeer::SetOccasionalPing(true);
}

void RakClient::StopOccasionalPing(void)
{
	RakPeer::SetOccasionalPing(false);
}

bool RakClient::IsConnected(void) const
{
	unsigned short numberOfSystems;

	RakPeer::GetConnectionList(0, &numberOfSystems);
	return numberOfSystems==1;
}

unsigned long RakClient::GetSynchronizedRandomInteger(void) const
{
	if (RakNetGetTime() > nextSeedUpdate)
		return nextSeed;
	else
		return seed;
}

/*
void RakClient::SynchronizeMemory(UniqueIDType uniqueIdentifier, char *memoryBlock, unsigned short size, bool isAuthority, bool (*synchronizationRules) (char*,char*),ObjectID secondaryUniqueIdentifier)
{
	RakPeer::SynchronizeMemory(uniqueIdentifier, memoryBlock, size, isAuthority, synchronizationRules, secondaryUniqueIdentifier);
}

void RakClient::DesynchronizeMemory(UniqueIDType uniqueIdentifier, ObjectID secondaryUniqueIdentifier)
{
	RakPeer::DesynchronizeMemory(uniqueIdentifier, secondaryUniqueIdentifier);
}

void RakClient::DesynchronizeAllMemory(void)
{
	RakPeer::DesynchronizeAllMemory();
}
*/
bool RakClient::GenerateCompressionLayer(unsigned long inputFrequencyTable[256], bool inputLayer)
{
	return RakPeer::GenerateCompressionLayer(inputFrequencyTable, inputLayer);
}

bool RakClient::DeleteCompressionLayer(bool inputLayer)
{
	return RakPeer::DeleteCompressionLayer(inputLayer);
}

void RakClient::RegisterAsRemoteProcedureCall(char* uniqueID, void (*functionName)(char *input, int numberOfBitsOfData, PlayerID sender))
{
	RakPeer::RegisterAsRemoteProcedureCall(uniqueID, functionName);
}

void RakClient::UnregisterAsRemoteProcedureCall(char* uniqueID)
{
	RakPeer::UnregisterAsRemoteProcedureCall(uniqueID);
}

bool RakClient::RPC(char* uniqueID, char *data, long bitLength, PacketPriority priority, PacketReliability reliability, char orderingStream, bool shiftTimestamp)
{
	if (remoteSystemList==0)
		return false;
	return RakPeer::RPC(uniqueID, data, bitLength, priority, reliability, orderingStream, remoteSystemList[0].playerId, false, shiftTimestamp);
}

bool RakClient::RPC(char* uniqueID, RakNet::BitStream *parameters, PacketPriority priority, PacketReliability reliability, char orderingStream, bool shiftTimestamp)
{
	if (remoteSystemList==0)
		return false;
	return RakPeer::RPC(uniqueID, parameters, priority, reliability, orderingStream, remoteSystemList[0].playerId, false, shiftTimestamp);
}

/*
bool RakClient::HandleRPCPacket(Packet* packet)
{
	if (remoteSystemList==0)
		return false;
	return RakPeer::HandleRPCPacket(packet->data, packet->length, remoteSystemList[0].playerId);
}*/

void RakClient::SetTrackFrequencyTable(bool b)
{
	RakPeer::SetCompileFrequencyTable(b);
}

bool RakClient::GetSendFrequencyTable(unsigned long outputFrequencyTable[256])
{
	return RakPeer::GetOutgoingFrequencyTable(outputFrequencyTable);
}

float RakClient::GetCompressionRatio(void) const
{
	return RakPeer::GetCompressionRatio();
}

float RakClient::GetDecompressionRatio(void) const
{
	return RakPeer::GetDecompressionRatio();
}

RakNet::BitStream * RakClient::GetStaticServerData(void)
{
	if (remoteSystemList==0)
		return 0;
	return RakPeer::GetRemoteStaticData(remoteSystemList[0].playerId);
}

void RakClient::SetStaticServerData(char *data, const long length)
{
	if (remoteSystemList==0)
		return;
	RakPeer::SetRemoteStaticData(remoteSystemList[0].playerId, data, length);
}

RakNet::BitStream * RakClient::GetStaticClientData(PlayerID playerId)
{
	int i;

	if (playerId==UNASSIGNED_PLAYER_ID)
	{
		return &localStaticData;
	}
	else
	{
		i = GetOtherClientIndexByPlayerID(playerId);
		if (i>=0)
		{
			return &(otherClients[i].staticData);
		}
	}

	return 0;
}

void RakClient::SetStaticClientData(PlayerID playerId, char *data, const long length)
{
	int i;

	if (playerId==UNASSIGNED_PLAYER_ID)
	{
		localStaticData.Reset();
		localStaticData.Write(data, length);
	}
	else
	{
		i = GetOtherClientIndexByPlayerID(playerId);
		if (i>=0)
		{
			otherClients[i].staticData.Reset();
			otherClients[i].staticData.Write(data,length);
		}
		else
			RakPeer::SetRemoteStaticData(playerId, data, length);
	}
	
}

void RakClient::SendStaticClientDataToServer(void)
{
	if (remoteSystemList==0)
		return;
	RakPeer::SendStaticData(remoteSystemList[0].playerId);
}

PlayerID RakClient::GetServerID(void) const
{
	if (remoteSystemList==0)
		return UNASSIGNED_PLAYER_ID;
	return remoteSystemList[0].playerId;
}

PlayerID RakClient::GetPlayerID(void) const
{
	if (remoteSystemList==0)
		return UNASSIGNED_PLAYER_ID;
	// GetExternalID is more accurate because it reflects our external IP and port to the server.
	// GetInternalID only matches the parameters we passed
	PlayerID myID = RakPeer::GetExternalID(remoteSystemList[0].playerId);
	if (myID==UNASSIGNED_PLAYER_ID)
		return RakPeer::GetInternalID();
	else
		return myID;
}

const char* RakClient::PlayerIDToDottedIP(PlayerID playerId) const
{
	return RakPeer::PlayerIDToDottedIP(playerId);
}

void RakClient::PushBackPacket(Packet *packet)
{
	RakPeer::PushBackPacket(packet);
}

bool RakClient::SetMTUSize(int size)
{
	return RakPeer::SetMTUSize(size);
}

int RakClient::GetMTUSize(void) const
{
	return RakPeer::GetMTUSize();
}

RakNetStatisticsStruct* const RakClient::GetStatistics(void)
{
	return RakPeer::GetStatistics(remoteSystemList[0].playerId);
}

int RakClient::GetOtherClientIndexByPlayerID(PlayerID playerId)
{
	unsigned i;
	for (i=0; i < 32; i++)
	{
		if (otherClients[i].playerId==playerId)
			return i;
	}
	return -1;
}

int RakClient::GetFreeOtherClientIndex(void)
{
	unsigned i;
	for (i=0; i < 32; i++)
	{
		if (otherClients[i].isActive==false)
			return i;
	}
	return -1;
}

PlayerIndex RakClient::GetPlayerIndex(void)
{
	return localPlayerIndex;
}
