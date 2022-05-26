//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
/// NetworkActor
/// Base class for networked Actors, 
/// automatically replicating core Physics state to NetworkClients
/// Author: Jeremy Stieglitz
//====================================================================================

#ifndef NETWORKACTOR_INCLUDED
#define NETWORKACTOR_INCLUDED

#define MAX_BUF_SIZE 1024
#define MAX_STRING_SIZE 512
#include "Actor.h"
#include "..\RakNet\BinarySearchTree.h"

#define MAX_LOCATION_INTERPOLATION_DISTANCE 25
#define SERVER_REDUNDANT_PHYSICS_SEND_ROT_THRESHOLD .075
#define SERVER_REDUNDANT_PHYSICS_SEND_VEL_THRESHOLD .2
#define SERVER_REDUNDANT_PHYSICS_SEND_LOC_THRESHOLD .065

class NetworkActor;
typedef unsigned char MessageType;
typedef unsigned short NetworkActorID;
typedef float (*Callback_AdaptiveDegradation)(int index,void* client);

/*
/// Used internally to contain NetworkActors in the tree.
struct ObjectIDNode
{
	NetworkActorID objectID;
	NetworkActor *object;

	ObjectIDNode();
	ObjectIDNode(NetworkActorID ObjectID, NetworkActor *Object);

	friend int operator==(const ObjectIDNode& left, const ObjectIDNode& right);
	friend int operator > (const ObjectIDNode& left, const ObjectIDNode& right);
	friend int operator < (const ObjectIDNode& left, const ObjectIDNode& right);
};*/

enum PacketType
{
	PACKET_RELIABLE,
	PACKET_UNRELIABLE,
	PACKET_SPAWN,
};

#ifndef DOXYGEN_IGNORE
struct MessagePacket
{
	char sendBuffer[MAX_BUF_SIZE];
	long writePos;
};
#endif

/// \brief Stores packets for this Actor for a particular NetworkClient
//
/// so that Actors can customize data on a per-NetworkClient basis for adaptive-degradation of network data
/// Data written into the spawning, reliable and unreliable buffers get sent to the particular NetworkClient by the Server upon transmission of the World State
class ENGINE_API NetworkActorPackets
{
public:
	class NetworkActor* m_NetworkActor;
	float m_LastSendTimeNetworkTick;
	NetworkActorID objectID;
	string m_ClassName;
	MessagePacket m_Packets[3];
	bool m_DeleteMe;
	bool m_SpawnByName;
	bool m_SentSpawnPacket;
	GUID m_ActorNameHash;

	Vector m_LastSentLocation;
	Vector m_LastSentRotation;
	Vector m_LastSentVelocity;
	float m_LastSentVelocityTime;

	int m_NetworkSynchOrder;

	NetworkActorPackets(class NetworkActor* forNetworkActor);
	void ResetPackets()
	{
		m_Packets[PACKET_UNRELIABLE].writePos = 0;

		if(m_SentSpawnPacket)
		{
		m_Packets[PACKET_RELIABLE].writePos = 0;
		m_Packets[PACKET_SPAWN].writePos = 0;
		}
	}
	void DeleteMe()
	{
		m_DeleteMe = true;
	}
};
#ifndef DOXYGEN_IGNORE
struct ReadPacketBuffer
{
	char* readBuffer;
	long readPos;
};
#endif

/// Base class for networked Actors, automatically replicating core Physics state to NetworkClients
class ENGINE_API NetworkActor : public Actor
{
	friend class Client;
	friend class Server;
public:
	/// Returns classname, with C# override
	inline virtual const char* ClassName()
	{
		if(!IsScriptActor())
			return "NetworkActor"; 
		else  
			return m_managedClassName.c_str();
	}

	/// jump forward by 10 ID's in between classes to provide some room to add more message types
	/// Basic Physics Messages. Stop message denotes end of message sequence and is pushed on during transmission of World State by Server
	const static MessageType MSGID_NETWORKACTOR_STOP = 0;
	const static MessageType MSGID_NETWORKACTOR_LOCATION	= 1;
	const static MessageType MSGID_NETWORKACTOR_ROTATION	= 2;
	const static MessageType MSGID_NETWORKACTOR_VELOCITY	= 3;
	const static MessageType MSGID_NETWORKACTOR_DESTRUCTION	= 4;

	/// NetworkActors self-register themselves on this array and remove themselves with destroyed
	static vector<NetworkActor*> NetworkActors;

	/// returns NetworkActor with specified object ID, if any
	static NetworkActor* GetNetworkActor(NetworkActorID objectID);

	/// returns NetworkActor with specified name-hash, if any
	static NetworkActor* GetNetworkActor(unsigned long NameHash);

	NetworkActor(World* world);

    /// alternate ctor used by Script Actors that doesn't add the base class to managed table, to avoid double-managing them
	NetworkActor(World* world, bool IsScriptActor);

	//static BasicDataStructures::AVLBalancedBinarySearchTree<ObjectIDNode> IDTree;

	/// The network ID of this object
	NetworkActorID objectID;

	/// Stores the last Network ID allocated, so that a new NetworkActor can increment one further
	static NetworkActorID staticItemID;

	/// Prints all existing NetworkActors' Actor Names, their classnames, and their network ID's.
	static void PrintAllNetworkActors();

	virtual ~NetworkActor();

	/// Used to reset the static (last) Network ID allocated upon map reload, to 0
	static void SetStaticItemID(NetworkActorID i);

	/// Manually sets the NetworkActor ID, called by the Client after constructing a new NetworkActor from a network message
	virtual void SetID(NetworkActorID id)
	{
		NetworkActor* collision = GET_OBJECT_FROM_ID(id);

		if(Client_ReceivedIDFromServer)
			Error("NetworkClient received a double spawn message for NetworkActor of type '%s', objectID %d",ClassName(),id);
		
		objectID = id;
		Client_ReceivedIDFromServer = true;

		if(collision)
		{
			if(collision != this)
			{
				if(collision->Client_ReceivedIDFromServer)
					Error("NetworkClient received new NetworkActor objectID %i that already exists in tree. Tried to set on type '%s' but collided with type '%s'",id,ClassName(),collision->ClassName());
				else
					collision->objectID = GetUniqueID();
			}
		}
	}
	static NetworkActorID GetUniqueID()
	{
	NetworkActor* collision;
	NetworkActorID objectID;
	do
	{
		objectID = rand()%65000;
		collision = GET_OBJECT_FROM_ID(objectID);
	} while (collision);
	return objectID;
	}

	/// read common data types from Client's received packet buffer
	MessageType ReadMessageType(ReadPacketBuffer* readPackets);
	/// read common data types from Client's received packet buffer
	bool ReadBool(ReadPacketBuffer* readPackets);
	/// read common data types from Client's received packet buffer
	int ReadInt(ReadPacketBuffer* readPackets);
	/// read common data types from Client's received packet buffer
	unsigned char ReadChar(ReadPacketBuffer* readPackets);
	/// read common data types from Client's received packet buffer
	float ReadFloat(ReadPacketBuffer* readPackets);
	/// read common data types from Client's received packet buffer
	DWORD ReadDWORD(ReadPacketBuffer* readPackets);
	/// read common data types from Client's received packet buffer
	NetworkActorID ReadNetworkActorID(ReadPacketBuffer* readPackets);
	/// read common data types from Client's received packet buffer
	Vector ReadVector(ReadPacketBuffer* readPackets);
	/// read common data types from Client's received packet buffer
	string ReadString(ReadPacketBuffer* readPackets);

	/// write common data types to all NetworkClients
	void WriteMessageType(MessageType val, PacketType packetType = PACKET_RELIABLE);
	/// write common data types to all NetworkClients
	void WriteBool(bool val, PacketType packetType = PACKET_RELIABLE);
	/// write common data types to all NetworkClients
	void WriteInt(int val, PacketType packetType = PACKET_RELIABLE);
	/// write common data types to all NetworkClients
	void WriteChar(unsigned char val, PacketType packetType = PACKET_RELIABLE);
	/// write common data types to all NetworkClients
	void WriteFloat(float val, PacketType packetType = PACKET_RELIABLE);
	/// write common data types to all NetworkClients
	void WriteDWORD(DWORD val, PacketType packetType = PACKET_RELIABLE);
	/// write common data types to all NetworkClients
	void WriteNetworkActorID(NetworkActorID val, PacketType packetType = PACKET_RELIABLE);
	/// write common data types to all NetworkClients
	void WriteVector(Vector& val, PacketType packetType = PACKET_RELIABLE);
	/// write common data types to all NetworkClients
	void WriteString(string& val, PacketType packetType = PACKET_RELIABLE);

	/// write common data types to a specific packet stream of a particular NetworkClient
	void WriteMessageType(MessagePacket* packet,MessageType val)
	{
		memcpy(&packet->sendBuffer[packet->writePos],&val,sizeof(val)); 
		packet->writePos += sizeof(val);
	}
	/// write common data types to a specific packet stream of a particular NetworkClient
	void WriteBool(MessagePacket* packet,bool val){memcpy(&packet->sendBuffer[packet->writePos],&val,sizeof(val)); packet->writePos += sizeof(val);}
	/// write common data types to a specific packet stream of a particular NetworkClient
	void WriteInt(MessagePacket* packet,int val){memcpy(&packet->sendBuffer[packet->writePos],&val,sizeof(val)); packet->writePos += sizeof(val);}
	/// write common data types to a specific packet stream of a particular NetworkClient
	void WriteChar(MessagePacket* packet,unsigned char val){memcpy(&packet->sendBuffer[packet->writePos],&val,sizeof(val)); packet->writePos += sizeof(val);}
	/// write common data types to a specific packet stream of a particular NetworkClient
	void WriteFloat(MessagePacket* packet,float val){memcpy(&packet->sendBuffer[packet->writePos],&val,sizeof(val)); packet->writePos += sizeof(val);}
	/// write common data types to a specific packet stream of a particular NetworkClient
	void WriteDWORD(MessagePacket* packet,DWORD val){memcpy(&packet->sendBuffer[packet->writePos],&val,sizeof(val)); packet->writePos += sizeof(val);}
	/// write common data types to a specific packet stream of a particular NetworkClient
	void WriteNetworkActorID(MessagePacket* packet,NetworkActorID val){memcpy(&packet->sendBuffer[packet->writePos],&val,sizeof(val)); packet->writePos += sizeof(val);}
	/// write common data types to a specific packet stream of a particular NetworkClient
	void WriteVector(MessagePacket* packet,Vector& val)
	{
		memcpy(&packet->sendBuffer[packet->writePos],&val.x,sizeof(float)); 
		packet->writePos += sizeof(float);
		memcpy(&packet->sendBuffer[packet->writePos],&val.y,sizeof(float)); 
		packet->writePos += sizeof(float);
		memcpy(&packet->sendBuffer[packet->writePos],&val.z,sizeof(float)); 
		packet->writePos += sizeof(float);
	}
	/// write common data types to a specific packet stream of a particular NetworkClient
	void WriteString(MessagePacket* packet,string& val)
	{
		int length = val.size()+1;
		WriteInt(packet,length);
		memcpy(&packet->sendBuffer[packet->writePos],val.c_str(),length); 
		packet->writePos += length;
	}
	/// write common data types to a specific packet stream of a particular NetworkClient
	void WriteLocation(MessagePacket* packet){WriteMessageType(packet,MSGID_NETWORKACTOR_LOCATION);WriteVector(packet,Location);}
	/// write common data types to a specific packet stream of a particular NetworkClient
	void WriteRotation(MessagePacket* packet){WriteMessageType(packet,MSGID_NETWORKACTOR_ROTATION);WriteVector(packet,Rotation.GetDir());}
	/// write common data types to a specific packet stream of a particular NetworkClient
	void WriteVelocity(MessagePacket* packet){WriteMessageType(packet,MSGID_NETWORKACTOR_VELOCITY);WriteVector(packet,Velocity);}

	/// Updates the NetworkActor's state replication calculations
	virtual void Tick();

	/// Set to true after first Tick, for one-off replication of initial spawn state
	bool m_HasTicked;

	/// how often (relative to each particular's client's receive rate setting) 
	/// this network actor sends unreliable network tick messages
	void SetNetworkTickRate(float NetworkTickRate)
	{
		m_NetworkTickRate = NetworkTickRate;
	}

	/// Server-side generates state update replication for the particular Client specified in packet, allowing adaptive degradation of network data. Actors inheriting from NetworkActor can override this if they have custom state information to replicate.
	virtual void Server_MakeTickMessages(NetworkActorPackets* packet);
	/// Server-side generates replication of initial spawn state for all Clients. Actors inheriting from NetworkActor can override this if they have custom state information to replicate.
	virtual void Server_MakeSpawnMessages();
	/// Server-side generates replication of full state for newly-joining Clients
	virtual NetworkActorPackets* Server_MakeOnJoinSynchMessages(class NetworkClient* client);
	/// Determines whether the NetworkActor will send a state replication update or not this frame. Can be overriden for custom state replication logic.
	virtual bool Server_ReadyToSendNetworkTick(class NetworkClient* client,NetworkActorPackets* packets);

	/// If true, Client will predict Physics from received network messages based on Clients' latency
	bool Client_DoesNetworkPrediction;
	/// amount of prediction used, best to keep at 1.0 unless object has highly erratic, unpredictable movement
	float Client_NetworkPredictionFactor;

	/// callback for custom adaptive degradation calculation
	static Callback_AdaptiveDegradation callback_AdaptiveDegradation;
	///  uses callback for custom adaptive degradation calculation
	bool Server_UseAdaptiveDegradationCallback;
	
	/// True for NetworkActors, lets Client and Server know to treat them as such
	virtual bool IsNetworkActor(){ return true; }

	/// Called if an Actor attempts to "Use" this NetworkActor
	virtual void Activated(class Actor* activator, Vector& activationPoint)
	{

		Actor::Activated((Actor*)activator);
	}
	virtual string GetDisplayName(){return "Network Actor";}

	/// returns network ID for this NetworkActor
	NetworkActorID GetID(){return objectID;}

	/// gets the NetworkActor, if any, with the specified ID
	static NetworkActor* GET_OBJECT_FROM_ID(NetworkActorID x);

	//protected:

	/// Iterates through Clients and calls Server_MakeTickMessages() to replicate changes in state
	virtual void Server_NetworkTick();
	/// Processes a state-update replication message
	virtual void Client_HandleTickMessage(MessageType message, ReadPacketBuffer* packetBuffer);
	/// Processes a spawn-state message
	virtual void Client_HandleSpawnMessage(MessageType message, ReadPacketBuffer* packetBuffer);
	/// Updates Client-side Network Physics interpolations
	virtual void Client_UpdateInterpolations();
	/// Client-side sets interpolateion to Network Predicted Location
	virtual void Client_InterpolateLocation(Vector& destination, float InterpolationTime);
	/// Client-side sets interpolateion to Network Predicted Rotation
	virtual void Client_InterpolateRotation(Matrix& destination, float InterpolationTime);
	/// Reads a Location data chunk from the current network message, and handily sets up physics interpolation
	virtual void Client_ReadLocation(ReadPacketBuffer* packetBuffer, bool DoInterpolation = true, float InterpolationTime = .175f);
	/// Reads a Rotation data chunk from the current network message, and handily sets up physics interpolation
	virtual void Client_ReadRotation(ReadPacketBuffer* packetBuffer, bool DoInterpolation = true, float InterpolationTime = .175f);
	/// Reads a Velocity data chunk from the current network message, and handily sets up physics interpolation
	virtual void Client_ReadVelocity(ReadPacketBuffer* packetBuffer);
	/// Client-side gets the predicted Location for a moving NetworkActor based on the Clients' latency, by running physics for latency time
	Vector Client_GetPredictedLocation(Vector& startingLocation);

	/// inits default state
    void Initialize();

	/// NetworkActors with lower values are synchronized first, used for synch-on-join info logic (e.g. InventoryItems immediately specifying their Owners when they are join-synched)
	int m_NetworkSynchOrder;

	/// each NetworkActor has this, which is written out during game recording to allow clients to push custom messages into the recording buffer (such as spectator tranformations)
	MessagePacket Client_PlaybackRecordingPacket;

	private:
	float m_NetworkTickRate;
	bool m_InterpLocation;
	float m_InterpLocationTotalTime;
	float m_InterpLocationCurrentTime;
	Vector m_InterpLocationTotalStep;

	bool m_InterpRotation;
	float m_InterpRotationTotalTime;
	float m_InterpRotationCurrentTime;
	Matrix m_InterpRotationStart;
	Matrix m_InterpRotationTotalStep;
	void Client_ProcessNetworkBuffer(char* buffer, unsigned short dataSize,bool isSpawnBuffer);
	bool Client_ReceivedIDFromServer;
};

#endif