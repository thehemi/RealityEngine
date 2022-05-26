//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
// NetworkActor
// Base class for networked Actors, 
// automatically replicating core Physics state to NetworkClients
// Author: Jeremy Stieglitz
//====================================================================================

#include "stdafx.h"
#include "NetworkActor.h"
#include "NetworkClient.h"
#include "Client.h"
#include "classmap.h"

Callback_AdaptiveDegradation NetworkActor::callback_AdaptiveDegradation = NULL;

vector<NetworkActor*> NetworkActor::NetworkActors;

NetworkActorID NetworkActor::staticItemID=0;
//BasicDataStructures::AVLBalancedBinarySearchTree<ObjectIDNode> NetworkActor::IDTree;
/*
int operator==(const ObjectIDNode& left, const ObjectIDNode& right)
{
	if (left.objectID == right.objectID) return !0;
	return 0;
}

int operator > (const ObjectIDNode& left, const ObjectIDNode& right)
{
	if (left.objectID > right.objectID) return !0;
	return 0;
}

int operator < (const ObjectIDNode& left, const ObjectIDNode& right)
{
	if (left.objectID < right.objectID) return !0;
	return 0;
}


ObjectIDNode::ObjectIDNode() {object=0;}
ObjectIDNode::ObjectIDNode(NetworkActorID ObjectID, NetworkActor *Object) {objectID = ObjectID; object = Object;}

NetworkActor* NetworkActor::GET_OBJECT_FROM_ID(NetworkActorID x) {ObjectIDNode *n = NetworkActor::IDTree.get_pointer_to_node(ObjectIDNode((x), 0)); if (n) return n->object; return 0;}
*/

NetworkActor* NetworkActor::GET_OBJECT_FROM_ID(NetworkActorID x) 
{
	for(int i = 0; i < NetworkActor::NetworkActors.size();i++)
		if(NetworkActor::NetworkActors[i]->objectID == x)
			return NetworkActor::NetworkActors[i];

	return NULL;
}

NetworkActorPackets::NetworkActorPackets(NetworkActor* forNetworkActor)
{
	m_NetworkActor = forNetworkActor;
	objectID = forNetworkActor->objectID;
	m_ClassName = forNetworkActor->ClassName();
	m_SpawnByName = forNetworkActor->m_SpawnByName;
	m_ActorNameHash = forNetworkActor->m_GUID; 
	m_Packets[0].writePos = 0;
	m_Packets[1].writePos = 0;
	m_Packets[2].writePos = 0;
	m_LastSendTimeNetworkTick = -BIG_NUMBER;
	m_LastSentVelocityTime = -BIG_NUMBER;
	m_NetworkSynchOrder = forNetworkActor->m_NetworkSynchOrder;
	m_DeleteMe = false;
	m_SentSpawnPacket = false;
}

void NetworkActor::SetStaticItemID(NetworkActorID i){staticItemID = i;}


//write data types to all clients
void NetworkActor::WriteMessageType(MessageType val, PacketType packetType)
{
	if(MyWorld && !MyWorld->m_IsServer)
		return;

	for(int i = 0; i < NetworkClient::NetworkClients.size();i++)
	{
		NetworkActorPackets* packets = NetworkClient::NetworkClients[i]->GetOrCreateNetworkActorPackets(this);
		WriteMessageType(&packets->m_Packets[packetType],val);
		if(val == MSGID_NETWORKACTOR_DESTRUCTION)
			packets->DeleteMe();
	}
}
void NetworkActor::WriteBool(bool val, PacketType packetType)
{
	if(!MyWorld || !MyWorld->m_IsServer)
		return;

	for(int i = 0; i < NetworkClient::NetworkClients.size();i++)
	{
		NetworkActorPackets* packets = NetworkClient::NetworkClients[i]->GetOrCreateNetworkActorPackets(this);
		WriteBool(&packets->m_Packets[packetType],val);
	}
}
void NetworkActor::WriteInt(int val, PacketType packetType)
{
	if(!MyWorld || !MyWorld->m_IsServer)
		return;

	for(int i = 0; i < NetworkClient::NetworkClients.size();i++)
	{
		NetworkActorPackets* packets = NetworkClient::NetworkClients[i]->GetOrCreateNetworkActorPackets(this);
		WriteInt(&packets->m_Packets[packetType],val);
	}
}
void NetworkActor::WriteChar(unsigned char val, PacketType packetType)
{
	if(!MyWorld || !MyWorld->m_IsServer)
		return;

	for(int i = 0; i < NetworkClient::NetworkClients.size();i++)
	{
		NetworkActorPackets* packets = NetworkClient::NetworkClients[i]->GetOrCreateNetworkActorPackets(this);
		WriteChar(&packets->m_Packets[packetType],val);
	}
}
void NetworkActor::WriteFloat(float val, PacketType packetType)
{
	if(!MyWorld || !MyWorld->m_IsServer)
		return;

	for(int i = 0; i < NetworkClient::NetworkClients.size();i++)
	{
		NetworkActorPackets* packets = NetworkClient::NetworkClients[i]->GetOrCreateNetworkActorPackets(this);
		WriteFloat(&packets->m_Packets[packetType],val);
	}
}
void NetworkActor::WriteDWORD(DWORD val, PacketType packetType)
{
	if(!MyWorld || !MyWorld->m_IsServer)
		return;

	for(int i = 0; i < NetworkClient::NetworkClients.size();i++)
	{
		NetworkActorPackets* packets = NetworkClient::NetworkClients[i]->GetOrCreateNetworkActorPackets(this);
		WriteDWORD(&packets->m_Packets[packetType],val);
	}
}
void NetworkActor::WriteNetworkActorID(NetworkActorID val, PacketType packetType)
{
	if(!MyWorld || !MyWorld->m_IsServer)
		return;

	for(int i = 0; i < NetworkClient::NetworkClients.size();i++)
	{
		NetworkActorPackets* packets = NetworkClient::NetworkClients[i]->GetOrCreateNetworkActorPackets(this);
		WriteNetworkActorID(&packets->m_Packets[packetType],val);
	}
}
void NetworkActor::WriteVector(Vector& val, PacketType packetType)
{
	if(!MyWorld || !MyWorld->m_IsServer)
		return;

	for(int i = 0; i < NetworkClient::NetworkClients.size();i++)
	{
		NetworkActorPackets* packets = NetworkClient::NetworkClients[i]->GetOrCreateNetworkActorPackets(this);
		WriteVector(&packets->m_Packets[packetType],val);
	}
}
void NetworkActor::WriteString(string& val, PacketType packetType)
{
	if(!MyWorld || !MyWorld->m_IsServer)
		return;

	for(int i = 0; i < NetworkClient::NetworkClients.size();i++)
	{
		NetworkActorPackets* packets = NetworkClient::NetworkClients[i]->GetOrCreateNetworkActorPackets(this);
		WriteString(&packets->m_Packets[packetType],val);
	}
}


//read data types from ReadPacketBuffer structure
MessageType NetworkActor::ReadMessageType(ReadPacketBuffer* readPackets)
{
	MessageType read_Val;

	memcpy(&read_Val,&readPackets->readBuffer[readPackets->readPos],sizeof(read_Val)); 
	readPackets->readPos += sizeof(read_Val);

	return read_Val;
}
bool NetworkActor::ReadBool(ReadPacketBuffer* readPackets)
{
	bool read_Val;

	memcpy(&read_Val,&readPackets->readBuffer[readPackets->readPos],sizeof(read_Val)); 
	readPackets->readPos += sizeof(read_Val);

	return read_Val;
}
int NetworkActor::ReadInt(ReadPacketBuffer* readPackets)
{
	int read_Val;

	memcpy(&read_Val,&readPackets->readBuffer[readPackets->readPos],sizeof(read_Val)); 
	readPackets->readPos += sizeof(read_Val);

	return read_Val;
}
unsigned char NetworkActor::ReadChar(ReadPacketBuffer* readPackets)
{
	unsigned char read_Val;

	memcpy(&read_Val,&readPackets->readBuffer[readPackets->readPos],sizeof(read_Val)); 
	readPackets->readPos += sizeof(read_Val);

	return read_Val;
}
float NetworkActor::ReadFloat(ReadPacketBuffer* readPackets)
{
	float read_Val;

	memcpy(&read_Val,&readPackets->readBuffer[readPackets->readPos],sizeof(read_Val)); 
	readPackets->readPos += sizeof(read_Val);

	return read_Val;
}
DWORD NetworkActor::ReadDWORD(ReadPacketBuffer* readPackets)
{
	DWORD read_Val;

	memcpy(&read_Val,&readPackets->readBuffer[readPackets->readPos],sizeof(read_Val)); 
	readPackets->readPos += sizeof(read_Val);

	return read_Val;
}
NetworkActorID NetworkActor::ReadNetworkActorID(ReadPacketBuffer* readPackets)
{
	NetworkActorID read_Val;

	memcpy(&read_Val,&readPackets->readBuffer[readPackets->readPos],sizeof(read_Val)); 
	readPackets->readPos += sizeof(read_Val);

	return read_Val;
}
Vector NetworkActor::ReadVector(ReadPacketBuffer* readPackets)
{
	Vector read_Val;

	read_Val.x = ReadFloat(readPackets);
	read_Val.y = ReadFloat(readPackets);
	read_Val.z = ReadFloat(readPackets);

	return read_Val;
}
string NetworkActor::ReadString(ReadPacketBuffer* readPackets)
{
	int stringLength = ReadInt(readPackets);

	char read_Val[MAX_STRING_SIZE];

	memcpy(read_Val,&readPackets->readBuffer[readPackets->readPos],stringLength); 
	readPackets->readPos += stringLength;

	return read_Val;
}

void NetworkActor::Server_MakeTickMessages(NetworkActorPackets* packet)
{
	if(IsScriptActor())
	{
    if (LibsInitialized())
        SSystem_ActorServer_MakeTickMessages(GetManagedIndex(),packet);
	}
	else
	{
	//filters out redundant sends of rotation, velocity, and location

	bool SendRotation = false;

	Vector rotDir = Rotation.GetDir();
	if(rotDir != packet->m_LastSentRotation && (rotDir - packet->m_LastSentRotation).Length() > SERVER_REDUNDANT_PHYSICS_SEND_ROT_THRESHOLD)
	{
		packet->m_LastSentRotation = rotDir;
		SendRotation = true;
	}

	bool SendVelocity = false;

	if(Velocity != packet->m_LastSentVelocity && (Velocity - packet->m_LastSentVelocity).Length() > SERVER_REDUNDANT_PHYSICS_SEND_VEL_THRESHOLD)
	{
		packet->m_LastSentVelocity = Velocity;
		SendVelocity = true;
	}

	bool SendLocation = false;

	if(Location != packet->m_LastSentLocation && (Location - packet->m_LastSentLocation).Length() > SERVER_REDUNDANT_PHYSICS_SEND_LOC_THRESHOLD)
	{
		packet->m_LastSentLocation = Location;
		SendLocation = true;
	}

if(SendRotation)
	WriteRotation(&packet->m_Packets[PACKET_UNRELIABLE]);

if(SendVelocity)
	WriteVelocity(&packet->m_Packets[PACKET_UNRELIABLE]);

if(SendLocation)
	WriteLocation(&packet->m_Packets[PACKET_UNRELIABLE]);
	}
}

void NetworkActor::Server_MakeSpawnMessages()
{
	if(IsScriptActor())
	{
    if (LibsInitialized())
        SSystem_ActorServer_MakeSpawnMessages(GetManagedIndex());
	}
	else
	{
	WriteMessageType(MSGID_NETWORKACTOR_LOCATION,PACKET_SPAWN);
	WriteVector(Location,PACKET_SPAWN);
	}
}
NetworkActorPackets* NetworkActor::Server_MakeOnJoinSynchMessages(NetworkClient* client)
{
	NetworkActorPackets* packets = NULL;
	if(!IsScriptActor())
	{
	packets = client->GetOrCreateNetworkActorPackets(this);
	WriteMessageType(&packets->m_Packets[PACKET_SPAWN],MSGID_NETWORKACTOR_LOCATION);
	WriteVector(&packets->m_Packets[PACKET_SPAWN],Location);
	}
	else if(LibsInitialized())
		SSystem_ActorServer_MakeOnJoinSynchMessages(GetManagedIndex(), (void*) client , (void**) &packets);

	return packets;
}
bool NetworkActor::Server_ReadyToSendNetworkTick(NetworkClient* client,NetworkActorPackets* packets)
{
	if(!m_NetworkTickRate)
		return false;

	float AdaptiveDegradationFactor = 1;

	if(!client->m_IsServer)
	{
	if(IsScriptActor() && Server_UseAdaptiveDegradationCallback && callback_AdaptiveDegradation)
		AdaptiveDegradationFactor = callback_AdaptiveDegradation(GetManagedIndex(),(void*)client);
	else
	{
	Actor* avatar = client->GetActorAvatar();
	if(avatar)
	{
		float distance = (Location - avatar->Location).Length();
		if(distance < 20)
			AdaptiveDegradationFactor = 1;
		else if(distance < 50)
			AdaptiveDegradationFactor = .6;
		else if(distance < 100)
			AdaptiveDegradationFactor = .4;
		else if(distance < 160)
			AdaptiveDegradationFactor = .25;
		else
			AdaptiveDegradationFactor = .14;
	}
	}
	}

	return (GSeconds - packets->m_LastSendTimeNetworkTick > (1.f/client->m_RecievePacketsPerSecond)/(m_NetworkTickRate*AdaptiveDegradationFactor));
}
void NetworkActor::Client_UpdateInterpolations()
{
	//interpolate Location
	if(m_InterpLocation)
	{
		float DeltaTime = GDeltaTime;
		m_InterpLocationCurrentTime += DeltaTime;
		if(m_InterpLocationCurrentTime >= m_InterpLocationTotalTime)
		{
			m_InterpLocation = false;
			//subtract overrun
			DeltaTime -= (m_InterpLocationCurrentTime - m_InterpLocationTotalTime);
		}
		Location += m_InterpLocationTotalStep * (DeltaTime/m_InterpLocationTotalTime);
	}

	//interpolate Rotation
	if(m_InterpRotation)
	{
		float DeltaTime = GDeltaTime;
		m_InterpRotationCurrentTime += DeltaTime;
		if(m_InterpRotationCurrentTime >= m_InterpRotationTotalTime)
		{
			m_InterpRotation = false;
			//subtract overrun
			DeltaTime -= (m_InterpRotationCurrentTime - m_InterpRotationTotalTime);
		}
		Rotation = Rotation + m_InterpRotationTotalStep*(DeltaTime/m_InterpRotationTotalTime);
		Rotation.Orthonormalize();
	}
}
void NetworkActor::Tick()
{
	Actor::Tick();

	if(m_HasTicked)
	{
		if(MyWorld->m_IsServer)
			Server_NetworkTick();
		else
			Client_UpdateInterpolations();
	}
	else
	{
		m_HasTicked = true;
		if(MyWorld->m_IsServer)
			Server_MakeSpawnMessages();
	}
}
void NetworkActor::Server_NetworkTick()
{
	for(int i = 0; i < NetworkClient::NetworkClients.size();i++)
	{
		NetworkClient* client = NetworkClient::NetworkClients[i];
		NetworkActorPackets* packets = client->GetOrCreateNetworkActorPackets(this);
		if(Server_ReadyToSendNetworkTick(client,packets))
		{
			packets->m_LastSendTimeNetworkTick = GSeconds;
			Server_MakeTickMessages(packets);
		}	
	}
}
void NetworkActor::Client_HandleTickMessage(MessageType message, ReadPacketBuffer* packetBuffer)
{
	if(IsScriptActor())
	{
    if (LibsInitialized())
            SSystem_ActorClient_HandleTickMessage(GetManagedIndex(),message,packetBuffer);
	}
	else
	{
	switch(message)
	{
	case MSGID_NETWORKACTOR_LOCATION:
		Client_ReadLocation(packetBuffer);
		break;
	case MSGID_NETWORKACTOR_ROTATION:
		Client_ReadRotation(packetBuffer);
		break;
	case MSGID_NETWORKACTOR_VELOCITY:
		Client_ReadVelocity(packetBuffer);
		break;
	case MSGID_NETWORKACTOR_DESTRUCTION:
		LifeTime = 0;
		break;
	default:
		Error("Unhandled Tick Message '%d' by actor '%s'",message,ClassName());
	}
	}
}
void NetworkActor::Client_HandleSpawnMessage(MessageType message, ReadPacketBuffer* packetBuffer)
{
	if(IsScriptActor())
	{
    if (LibsInitialized())
            SSystem_ActorClient_HandleSpawnMessage(GetManagedIndex(),message,packetBuffer);
	}
	else
	{
	switch(message)
	{
	case MSGID_NETWORKACTOR_LOCATION:
		Client_ReadLocation(packetBuffer,false);
		break;
	case MSGID_NETWORKACTOR_ROTATION:
		Client_ReadRotation(packetBuffer,false);
		break;
	case MSGID_NETWORKACTOR_VELOCITY:
		Client_ReadVelocity(packetBuffer);
		break;
	default:
		Error("Unhandled Spawn Message '%d' by actor '%s'",message,ClassName());
	}
	}
}
void NetworkActor::Client_InterpolateLocation(Vector& destination, float InterpolationTime)
{
	if(InterpolationTime <= 0 || (destination - Location).Length() > MAX_LOCATION_INTERPOLATION_DISTANCE)
	{
		m_InterpLocation = false;
		Location = destination;
		return;
	}

	m_InterpLocation = true;
	m_InterpLocationCurrentTime = 0;
	m_InterpLocationTotalTime = InterpolationTime;
	m_InterpLocationTotalStep = destination - Location;
}
void NetworkActor::Client_InterpolateRotation(Matrix& destination, float InterpolationTime)
{
	if(InterpolationTime <= 0)
	{
		m_InterpRotation = false;
		Rotation = destination;
		return;
	}

	m_InterpRotation = true;
	m_InterpRotationCurrentTime = 0;
	m_InterpRotationTotalTime = InterpolationTime;
	m_InterpRotationTotalStep = destination - Rotation;
}
void NetworkActor::Client_ReadLocation(ReadPacketBuffer* packetBuffer, bool DoInterpolation, float InterpolationTime)
{
	if(DoInterpolation)
		Client_InterpolateLocation(Client_GetPredictedLocation(ReadVector(packetBuffer)),InterpolationTime*Client::Instance()->m_ClientSettings.m_NetworkInterpolationFactor);
	else
		Location = Client_GetPredictedLocation(ReadVector(packetBuffer));
}

Vector NetworkActor::Client_GetPredictedLocation(Vector& startingLocation)
{
float Latency = Client::Instance()->GetLatency()*Client_NetworkPredictionFactor;

if(!Client_DoesNetworkPrediction || Latency <= 0)
	return startingLocation;

Vector currentLocation = Location;
Vector currentVelocity = Velocity;

Location = startingLocation;

float realDeltaTime = GDeltaTime;
GDeltaTime = Latency;
MyWorld->RunPhysics(this);
GDeltaTime = realDeltaTime;
Vector predictedLocation = Location;

Location = currentLocation;
Velocity = currentVelocity;

return predictedLocation;
}

void NetworkActor::Client_ReadRotation(ReadPacketBuffer* packetBuffer, bool DoInterpolation, float InterpolationTime)
{
	Vector dir = ReadVector(packetBuffer);
	if(DoInterpolation)
		Client_InterpolateRotation(Matrix::LookTowards(dir.Normalized()),InterpolationTime*Client::Instance()->m_ClientSettings.m_NetworkInterpolationFactor);
	else
		Rotation = Matrix::LookTowards(dir.Normalized());
}
void NetworkActor::Client_ReadVelocity(ReadPacketBuffer* packetBuffer)
{
	Velocity = ReadVector(packetBuffer);
}

void NetworkActor::Client_ProcessNetworkBuffer(char* buffer, unsigned short dataSize,bool isSpawnBuffer)
{
	ReadPacketBuffer packetBuffer;
	packetBuffer.readBuffer = buffer;
	packetBuffer.readPos = 0;

	while(packetBuffer.readPos < dataSize)
	{
		MessageType message = ReadMessageType(&packetBuffer);

		if(message == MSGID_NETWORKACTOR_STOP)
			return;

		if(!isSpawnBuffer)
			Client_HandleTickMessage(message,&packetBuffer);
		else
			Client_HandleSpawnMessage(message,&packetBuffer);
	}
}
NetworkActor::~NetworkActor()
{
	if(MyWorld && MyWorld->m_IsServer)
		WriteMessageType(MSGID_NETWORKACTOR_DESTRUCTION);

	NetworkActors.erase(find(NetworkActors.begin(),NetworkActors.end(),this));
}

void NetworkActor::Initialize()
{
	objectID = GetUniqueID();
	Client_ReceivedIDFromServer = false;

	if(!MyWorld->m_IsServer)
	{
		CollisionFlags = CF_BBOX | CF_ALLOW_STUCK;
		PhysicsFlags = PHYS_NOT_AFFECTED_BY_GRAVITY | PHYS_ONBLOCK_STOP;
	}

	m_HasTicked = false;
	m_InterpLocation = false;
	m_InterpRotation = false;
	Client_DoesNetworkPrediction = false;
	m_SpawnByName = false;
	Server_UseAdaptiveDegradationCallback = false;
	m_NetworkSynchOrder = 5;
	Client_PlaybackRecordingPacket.writePos = 0;
	Client_NetworkPredictionFactor = 1;
	SetNetworkTickRate(1);

	NetworkActors.push_back(this);
}

NetworkActor::NetworkActor(World* world) : Actor(world)
{
    Initialize();
}

NetworkActor::NetworkActor(World* world, bool IsScriptActor) : Actor(world,IsScriptActor)
{
    Initialize();
}

NetworkActor* NetworkActor::GetNetworkActor(NetworkActorID objectID)
{
return GET_OBJECT_FROM_ID(objectID);
}
void NetworkActor::PrintAllNetworkActors()
{
for(int i = 0; i < NetworkActor::NetworkActors.size();i++)
{
		LogPrintf("Network Actor '%s' of class '%s' ID: %d",NetworkActor::NetworkActors[i]->m_Name.c_str(),NetworkActor::NetworkActors[i]->ClassName(),NetworkActor::NetworkActors[i]->objectID);
}
}