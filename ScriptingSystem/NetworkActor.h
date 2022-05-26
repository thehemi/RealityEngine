//=========== (C) Copyright 2004, Artficial Studios. All rights reserved. =======
// C# Scripting System
// Author: Mostafa Mohamed
//
//
//===============================================================================
#pragma	once
using namespace	System;
using namespace	System::Collections;
using namespace System::Reflection;
using namespace System::ComponentModel;
using namespace stdcli::language;

#include "Wrappers.h"
#include "Helpers.h"

namespace ScriptingSystem
{
// <summary>
/// wraps lowest-level packet structure to pass to buffer writing functions
// </summary>
	public ref class MMessagePacket
	{
		private public:
			MessagePacket* m_messagePacket;

			MMessagePacket(MessagePacket* messagePacket)
			{
				m_messagePacket = messagePacket;
			}

	};

// <summary>
/// wraps client incoming read buffer structure to pass to buffer reading functions
// </summary>
	public ref class MReadPacketBuffer
	{
		private public:
			ReadPacketBuffer* m_readPacket;

			MReadPacketBuffer(ReadPacketBuffer* readPacket)
			{
				m_readPacket = readPacket;
			}

	};

// <summary>
/// core NetworkActor network messages
// </summary>
	public enum class MSG
	{
		MSGID_NETWORKACTOR_STOP = 0, /// stops reading of this networkactor's incoming buffer, automatically placed at end
		MSGID_NETWORKACTOR_LOCATION = 1, /// location update
		MSGID_NETWORKACTOR_ROTATION = 2, /// rotation update
		MSGID_NETWORKACTOR_VELOCITY = 3, /// velocity update
		MSGID_NETWORKACTOR_DESTRUCTION = 4, /// destroys this networkactor, automatically sent upon server-side destruction
		MSGID_NETWORKACTOR_TEAM = 5 /// sets the team of this networkactor, can be used by game logic
	};

// <summary>
/// Base class for networked Actors, automatically replicating core Physics state to NetworkClients
// </summary>
	public ref class MNetworkActor : MActor
	{
	private	public:
			property virtual MActorType ActorType
#ifdef DOXYGEN_IGNORE 
; _()
#endif
			{
				MActorType	get()
				{
					return MActorType::NetworkActor;
				}
			}
			//Properties
	public:
		static const int PACKET_RELIABLE = 0; /// packet types to pass to message writing functions
		static const int PACKET_UNRELIABLE = 1; /// packet types to pass to message writing functions
		static const int PACKET_SPAWN = 2; /// packet types to pass to message writing functions
		static const float MAX_LOCATION_INTERP_DISTANCE = 25; /// locations won't interpolate beyond this distance
		static const float SERVER_REDUNDANT_PHYSICS_SEND_ROTATION_THRESHOLD = .077; /// delta thresholds for server sending physics updates, coupled with redundant filtering
		static const float SERVER_REDUNDANT_PHYSICS_SEND_VELOCITY_THRESHOLD = .21; /// delta thresholds for server sending physics updates, coupled with redundant filtering
		static const float SERVER_REDUNDANT_PHYSICS_SEND_LOCATION_THRESHOLD = .067; /// delta thresholds for server sending physics updates, coupled with redundant filtering

		// FUNCTIONS
		[BrowsableAttribute(false)]
// <summary>
/// 
// </summary>
		property unsigned short NetworkID
#ifdef DOXYGEN_IGNORE 
; _()
#endif
		{
			unsigned short get()
			{
				return ((NetworkActor*)m_actor)->GetID();
			}
		}
		[BrowsableAttribute(false)]
// <summary>
/// 
// </summary>
		property bool HasTicked
#ifdef DOXYGEN_IGNORE 
; _()
#endif
		{
			bool get()
			{
				return ((NetworkActor*)m_actor)->m_HasTicked;
			}
			void set(bool value)
			{
				((NetworkActor*)m_actor)->m_HasTicked=value;
			}
		}

		[BrowsableAttribute(false)]
// <summary>
/// 
// </summary>
		property bool Client_DoesNetworkPrediction
#ifdef DOXYGEN_IGNORE 
; _()
#endif
		{
			bool get()
			{
				return ((NetworkActor*)m_actor)->Client_DoesNetworkPrediction;
			}
			void set(bool value)
			{
				((NetworkActor*)m_actor)->Client_DoesNetworkPrediction=value;
			}
		}

		[BrowsableAttribute(false)]
// <summary>
/// 
// </summary>
		property bool Server_UseAdaptiveDegradationCallback
#ifdef DOXYGEN_IGNORE 
; _()
#endif
		{
			bool get()
			{
				return ((NetworkActor*)m_actor)->Server_UseAdaptiveDegradationCallback;
			}
			void set(bool value)
			{
				((NetworkActor*)m_actor)->Server_UseAdaptiveDegradationCallback=value;
			}
		}


		[BrowsableAttribute(false)]
// <summary>
/// 
// </summary>
		property bool SpawnByName
#ifdef DOXYGEN_IGNORE 
; _()
#endif
		{
			bool get()
			{
				return ((NetworkActor*)m_actor)->m_SpawnByName;
			}
			void set(bool value)
			{
				((NetworkActor*)m_actor)->m_SpawnByName=value;
			}
		}

		[BrowsableAttribute(false)]
// <summary>
/// 
// </summary>
		property float Client_NetworkPredictionFactor
#ifdef DOXYGEN_IGNORE 
; _()
#endif
		{
			float get()
			{
				return ((NetworkActor*)m_actor)->Client_NetworkPredictionFactor;
			}
			void set(float value)
			{
				((NetworkActor*)m_actor)->Client_NetworkPredictionFactor=value;
			}
		}

		[BrowsableAttribute(false)]
// <summary>
/// 
// </summary>
		property int m_NetworkSynchOrder
#ifdef DOXYGEN_IGNORE 
; _()
#endif
		{
			int get()
			{
				return ((NetworkActor*)m_actor)->m_NetworkSynchOrder;
			}
			void set(int value)
			{
				((NetworkActor*)m_actor)->m_NetworkSynchOrder=value;
			}
		}

		[BrowsableAttribute(false)]
// <summary>
/// 
// </summary>
		property unsigned short objectID
#ifdef DOXYGEN_IGNORE 
; _()
#endif
		{
			unsigned short get()
			{
				return ((NetworkActor*)m_actor)->objectID;
			}
		}

	public:

// <summary>
/// 
// </summary>
		static MNetworkActor^ FindFromID(unsigned short objectID)
		{
			NetworkActor* act = NetworkActor::GET_OBJECT_FROM_ID(objectID);

			if(!act)
				return nullptr;

			return (MNetworkActor^)MActor::s_actors[act->GetManagedIndex()];
		}

// <summary>
/// 
// </summary>
		MNetworkActor(Actor * actor) : MActor(actor)
		{ 

		}


// <summary>
/// 
// </summary>
		MNetworkActor(MWorld^ world) : MActor(world)
		{
		}

// <summary>
/// 
// </summary>
		virtual	void Tick()
		{
			MActor::Tick();
		}

		//Writing Functions
		//MessageType
		void WriteMessageType(unsigned char val, int packetType)
		{
			((NetworkActor*)m_actor)->WriteMessageType(val,(PacketType)packetType);
		}
		void WriteMessageType(unsigned char val){ WriteMessageType(val, PACKET_RELIABLE);}

		//Bool
		void WriteBool(bool val, int packetType)
		{
			((NetworkActor*)m_actor)->WriteBool(val,(PacketType)packetType);
		}
		void WriteBool(bool val){ WriteBool(val, PACKET_RELIABLE);}

		//Int
		void WriteInt(int val, int packetType)
		{
			((NetworkActor*)m_actor)->WriteInt(val,(PacketType)packetType);
		}
		void WriteInt(int val){ WriteInt(val, PACKET_RELIABLE);}

		//Char
		void WriteChar(unsigned char val, int packetType)
		{
			((NetworkActor*)m_actor)->WriteChar(val,(PacketType)packetType);
		}
		void WriteChar(unsigned char val){ WriteChar(val, PACKET_RELIABLE);}

		//Float
		void WriteFloat(float val, int packetType)
		{
			((NetworkActor*)m_actor)->WriteFloat(val,(PacketType)packetType);
		}
		void WriteFloat(float val){ WriteFloat(val, PACKET_RELIABLE);}

		//Unsigned Long
		void WriteDWORD(unsigned long val, int packetType)
		{
			((NetworkActor*)m_actor)->WriteDWORD(val,(PacketType)packetType);
		}
		void WriteDWORD(unsigned long val){ WriteDWORD(val, PACKET_RELIABLE);}

		//Unsigned Short
		void WriteNetworkActorID(unsigned short val, int packetType)
		{
			((NetworkActor*)m_actor)->WriteNetworkActorID(val,(PacketType)packetType);
		}
		void WriteNetworkActorID(unsigned short val){ WriteNetworkActorID(val, PACKET_RELIABLE);}

		//Vector
		void WriteVector(MVector^ val, int packetType)
		{
			((NetworkActor*)m_actor)->WriteVector(*val->m_vector,(PacketType)packetType);
		}
		void WriteVector(MVector^ val){ WriteVector(val, PACKET_RELIABLE);}

		//String
		void WriteString(String^ val, int packetType)
		{
			((NetworkActor*)m_actor)->WriteString(Helpers::ToCppString(val), (PacketType)packetType);
		}
		void WriteString(String^ val){ WriteString(val, PACKET_RELIABLE);}



		//MessageType
		void WriteMessageType(MMessagePacket^ packet, unsigned char val)
		{
			((NetworkActor*)m_actor)->WriteMessageType(packet->m_messagePacket ,val);
		}

		//Bool
		void WriteBool(MMessagePacket^ packet, bool val)
		{
			((NetworkActor*)m_actor)->WriteBool(packet->m_messagePacket ,val);
		}

		//Int
		void WriteInt(MMessagePacket^ packet, int val)
		{
			((NetworkActor*)m_actor)->WriteInt(packet->m_messagePacket ,val);
		}

		//Char
		void WriteChar(MMessagePacket^ packet, unsigned char val)
		{
			((NetworkActor*)m_actor)->WriteChar(packet->m_messagePacket ,val);
		}

		//Float
		void WriteFloat(MMessagePacket^ packet, float val)
		{
			((NetworkActor*)m_actor)->WriteFloat(packet->m_messagePacket ,val);
		}

		//DWORD
		void WriteDWORD(MMessagePacket^ packet, unsigned long val)
		{
			((NetworkActor*)m_actor)->WriteDWORD(packet->m_messagePacket ,val);
		}

		//ActorID
		void WriteNetworkActorID(MMessagePacket^ packet, unsigned short val)
		{
			((NetworkActor*)m_actor)->WriteNetworkActorID(packet->m_messagePacket ,val);
		}

		//Vector
		void WriteVector(MMessagePacket^ packet,MVector^ val)
		{
			((NetworkActor*)m_actor)->WriteVector(packet->m_messagePacket ,*val->m_vector);
		}

		//String
		void WriteString(MMessagePacket^ packet,String^ val)
		{
			((NetworkActor*)m_actor)->WriteString(packet->m_messagePacket ,Helpers::ToCppString(val));
		}

		//MESSAGES READING
		//bool
		bool ReadBool(MReadPacketBuffer^ readPackets)
		{
			return ((NetworkActor*)m_actor)->ReadBool(readPackets->m_readPacket);
		}

		//int
		int ReadInt(MReadPacketBuffer^ readPackets)
		{
			return ((NetworkActor*)m_actor)->ReadInt(readPackets->m_readPacket);
		}

		//float
		float ReadFloat(MReadPacketBuffer^ readPackets)
		{
			return ((NetworkActor*)m_actor)->ReadFloat(readPackets->m_readPacket);
		}

		//char
		unsigned char ReadChar(MReadPacketBuffer^ readPackets)
		{
			return ((NetworkActor*)m_actor)->ReadChar(readPackets->m_readPacket);
		}

		//NetworkActorId
		unsigned short ReadNetworkActorID(MReadPacketBuffer^ readPackets)
		{
			return ((NetworkActor*)m_actor)->ReadNetworkActorID(readPackets->m_readPacket);
		}

		//DWORD
		unsigned long ReadDWORD(MReadPacketBuffer^ readPackets)
		{
			return ((NetworkActor*)m_actor)->ReadDWORD(readPackets->m_readPacket);
		}

		//String
		String^ ReadString(MReadPacketBuffer^ readPackets)
		{
			return Helpers::ToCLIString(((NetworkActor*)m_actor)->ReadString(readPackets->m_readPacket));
		}

		//Vector
		MVector^ ReadVector(MReadPacketBuffer^ readPackets)
		{
			return gcnew MVector(((NetworkActor*)m_actor)->ReadVector(readPackets->m_readPacket));
		}

		//Virtual functions


// <summary>
/// 
// </summary>
		MVector^ Client_GetPredictedLocation(MVector^ location)
		{
			return gcnew MVector(((NetworkActor*)m_actor)->Client_GetPredictedLocation(*location->m_vector));
		}
// <summary>
/// 
// </summary>
		void Client_InterpolateLocation(MVector^ location,float interpolationTime)
		{
			((NetworkActor*)m_actor)->Client_InterpolateLocation(*location->m_vector,interpolationTime);
		}
// <summary>
/// 
// </summary>
		float GetInterpolationFactor()
		{
			return Client::Instance()->m_ClientSettings.m_NetworkInterpolationFactor;
		}
// <summary>
/// 
// </summary>
		virtual void Client_ReadLocation(MReadPacketBuffer^ packetBuffer, bool DoInterpolation, float InterpolationTime)
		{
			if(DoInterpolation)
				Client_InterpolateLocation(Client_GetPredictedLocation(ReadVector(packetBuffer)),InterpolationTime*GetInterpolationFactor());
			else
				Location = ReadVector(packetBuffer);
		}
// <summary>
/// 
// </summary>
		virtual void Client_ReadVelocity(MReadPacketBuffer^ packetBuffer)
		{
			Velocity = ReadVector(packetBuffer);
		}
// <summary>
/// 
// </summary>
		void Client_InterpolateRotation(MMatrix^ rotation,float interpolationTime)
		{
			((NetworkActor*)m_actor)->Client_InterpolateRotation(*rotation->m_matrix,interpolationTime);
		}
// <summary>
/// 
// </summary>
		virtual void Client_ReadRotation(MReadPacketBuffer^ packetBuffer, bool DoInterpolation, float InterpolationTime)
		{
			MVector^ dir = ReadVector(packetBuffer);
			if(DoInterpolation)
				Client_InterpolateRotation(MMatrix::LookTowards(dir->Normalized()),InterpolationTime*GetInterpolationFactor());
			else
				Rotation = MMatrix::LookTowards(dir->Normalized());
		}

// <summary>
/// 
// </summary>
		virtual void WriteLocation(MMessagePacket^ packet)
		{
			WriteMessageType(packet,(unsigned char)MSG::MSGID_NETWORKACTOR_LOCATION);
			WriteVector(packet,Location);
		}

// <summary>
/// 
// </summary>
		virtual void WriteRotation(MMessagePacket^ packet)
		{
			WriteMessageType(packet,(unsigned char)MSG::MSGID_NETWORKACTOR_ROTATION);
			WriteVector(packet,Rotation->GetDir());
		}

// <summary>
/// 
// </summary>
		virtual void WriteVelocity(MMessagePacket^ packet)
		{
			WriteMessageType(packet,(unsigned char)MSG::MSGID_NETWORKACTOR_VELOCITY);
			WriteVector(packet,Velocity);
		}


// <summary>
/// 
// </summary>
		virtual void Client_HandleTickMessage(unsigned char message, MReadPacketBuffer^ packetBuffer)
		{
			switch(message)
			{
			case (unsigned char)MSG::MSGID_NETWORKACTOR_LOCATION:
				Client_ReadLocation(packetBuffer,true,.175f);
				break;
			case (unsigned char)MSG::MSGID_NETWORKACTOR_ROTATION:
				Client_ReadRotation(packetBuffer,true,.175f);
				break;
			case (unsigned char)MSG::MSGID_NETWORKACTOR_VELOCITY:
				Client_ReadVelocity(packetBuffer);
				break;
			case (unsigned char)MSG::MSGID_NETWORKACTOR_DESTRUCTION:
				LifeTime = 0;
				break;
			case (unsigned char)MSG::MSGID_NETWORKACTOR_TEAM:
				Team = ReadInt(packetBuffer);
				break;
			default:
				Error("Unhandled Tick Message '%d' by actor '%s'",message,"MNetworkActor");
			}
		}

// <summary>
/// 
// </summary>
		virtual void Client_HandleSpawnMessage(unsigned char  message, MReadPacketBuffer^ packetBuffer)
		{
			switch(message)
			{
			case (unsigned char)MSG::MSGID_NETWORKACTOR_LOCATION:
				Client_ReadLocation(packetBuffer,false,.175f);
				break;
			case (unsigned char)MSG::MSGID_NETWORKACTOR_ROTATION:
				Client_ReadRotation(packetBuffer,false,.175f);
				break;
			case (unsigned char)MSG::MSGID_NETWORKACTOR_VELOCITY:
				Client_ReadVelocity(packetBuffer);
				break;
			case (unsigned char)MSG::MSGID_NETWORKACTOR_TEAM:
				Team = ReadInt(packetBuffer);
				break;
			default:
				Error("Unhandled Spawn Message '%d' by actor '%s'",message,"MNetworkActor");
			}
		}

// <summary>
/// 
// </summary>
		void SetNetworkTickRate(float NetworkTickRate)
		{
			((NetworkActor*)m_actor)->SetNetworkTickRate(NetworkTickRate);
		}

// <summary>
/// 
// </summary>
		virtual void Server_MakeTickMessages(ref class MNetworkActorPackets^ packet);

// <summary>
/// 
// </summary>
		virtual void Server_MakeSpawnMessages()
		{
			WriteMessageType((unsigned char)MSG::MSGID_NETWORKACTOR_LOCATION,PACKET_SPAWN);
			WriteVector(Location,PACKET_SPAWN);
		}

// <summary>
/// 
// </summary>
		virtual MNetworkActorPackets^ Server_MakeOnJoinSynchMessages(ref class MNetworkClient^ client);
// <summary>
/// 

// </summary>
		virtual float Server_AdaptiveDegradation(ref class MNetworkClient^ client){return 1;}

// <summary>
/// 
// </summary>
		MMessagePacket^ GetClientRecordingPacket()
		{
			return gcnew MMessagePacket(&((NetworkActor*)m_actor)->Client_PlaybackRecordingPacket);
		}
	};

	
// <summary>
/// Wraps the NetworkActorPackets of a NetworkActor, which contain the packet streams of one NetworkActor for a particular NetworkClient
// </summary>
	public ref class MNetworkActorPackets
	{
		private public:
			bool needToDelete;
			NetworkActorPackets * m_networkActorPackets;

// <summary>
/// 
// </summary>
			MNetworkActorPackets(NetworkActorPackets* networkActorPackets)
			{
				needToDelete = false;
				m_networkActorPackets=networkActorPackets;
			}
	public:
// <summary>
/// 
// </summary>
		property float m_LastSendTimeNetworkTick
#ifdef DOXYGEN_IGNORE 
; _()
#endif
		{
			float get()
			{
				return m_networkActorPackets->m_LastSendTimeNetworkTick;
			}
			void set(float value)
			{
				m_networkActorPackets->m_LastSendTimeNetworkTick=value;
			}
		}
// <summary>
/// 
// </summary>
		property unsigned short objectID
#ifdef DOXYGEN_IGNORE 
; _()
#endif
		{
			unsigned short get()
			{
				return m_networkActorPackets->objectID;
			}
		}

// <summary>
/// 
// </summary>
		property bool m_DeleteMe
#ifdef DOXYGEN_IGNORE 
; _()
#endif
		{
			bool get()
			{
				return m_networkActorPackets->m_DeleteMe;
			}
			void set(bool value)
			{
				m_networkActorPackets->m_DeleteMe=value;
			}
		}

// <summary>
/// 
// </summary>
		property float m_LastSentVelocityTime
#ifdef DOXYGEN_IGNORE 
; _()
#endif
		{
			float get()
			{
				return m_networkActorPackets->m_LastSentVelocityTime;
			}
			void set(float value)
			{
				m_networkActorPackets->m_LastSentVelocityTime=value;
			}
		}

// <summary>
/// 
// </summary>
		property String^ m_ClassName
#ifdef DOXYGEN_IGNORE 
; _()
#endif
		{
			String^ get()
			{
				return Helpers::ToCLIString(m_networkActorPackets->m_ClassName);
			}
			void set(String^ value)
			{
				m_networkActorPackets->m_ClassName=Helpers::ToCppString(value);
			}
		}

// <summary>
/// 
// </summary>
		property MVector^ m_LastSentLocation
#ifdef DOXYGEN_IGNORE 
; _()
#endif
		{
			MVector^ get()
			{
				return gcnew MVector(&m_networkActorPackets->m_LastSentLocation);
			}

			void set(MVector^ value)
			{
				if (value!=nullptr)
					m_networkActorPackets->m_LastSentLocation=*value->m_vector;
			}
		}

// <summary>
/// 
// </summary>
		property MVector^ m_LastSentRotation
#ifdef DOXYGEN_IGNORE 
; _()
#endif
		{
			MVector^ get()
			{
				return gcnew MVector(&m_networkActorPackets->m_LastSentRotation);
			}

			void set(MVector^ value)
			{
				if (value!=nullptr)
					m_networkActorPackets->m_LastSentRotation=*value->m_vector;
			}
		}

// <summary>
/// 
// </summary>
		property MVector^ m_LastSentVelocity
#ifdef DOXYGEN_IGNORE 
; _()
#endif
		{
			MVector^ get()
			{
				return gcnew MVector(&m_networkActorPackets->m_LastSentVelocity);
			}

			void set(MVector^ value)
			{
				if (value!=nullptr)
					m_networkActorPackets->m_LastSentVelocity=*value->m_vector;
			}
		}

// <summary>
/// 
// </summary>
		MMessagePacket^ GetMessagePacket(int index)
		{
			if (index>2 || index <0) return nullptr;
			return gcnew MMessagePacket(&m_networkActorPackets->m_Packets[index]);
		}

// <summary>
/// 
// </summary>
		void SetMessagePacket(int index,MMessagePacket^ packet)
		{
			if (index>2 || index <0) return;
			m_networkActorPackets->m_Packets[index] = *packet->m_messagePacket;
		}

// <summary>
/// 
// </summary>
		MNetworkActorPackets(MNetworkActor^ forNetworkActor)
		{
			m_networkActorPackets = new NetworkActorPackets((NetworkActor*)forNetworkActor->m_actor);
			needToDelete = true;
		}

// <summary>
/// 
// </summary>
		virtual void Finalize()
		{
			if (needToDelete)
				delete m_networkActorPackets;
		}
	};

// <summary>
/// Wraps a NetworkClient, which is the Server-side representation of a user connected to the session (that is, even before they actually spawn in the game World)
// </summary>
	public ref class MNetworkClient
	{
		private public:
			NetworkClient * m_networkClient;

// <summary>
/// 
// </summary>
			MNetworkClient(NetworkClient* networkClient)
			{
				m_networkClient=networkClient;
			}
	public:

// <summary>
/// 
// </summary>
		static property MNetworkClient^ ServerNetworkClient
		{
			MNetworkClient^ get()
			{
				if(Server::Instance()->ServerNetworkClient)
					return gcnew MNetworkClient(Server::Instance()->ServerNetworkClient);
				else
					return nullptr;
			}
		}

// <summary>
/// 
// </summary>
		Object^ m_GameUserData;
		
// <summary>
/// 
// </summary>
		property String^ m_Name
#ifdef DOXYGEN_IGNORE 
; _()
#endif
		{
			String^ get()
			{
				return Helpers::ToCLIString(m_networkClient->m_Name);
			}
			void set(String^ value)
			{
				m_networkClient->m_Name=Helpers::ToCppString(value);
			}
		}

// <summary>
/// 
// </summary>
		property MActor^ ActorAvatar
#ifdef DOXYGEN_IGNORE 
; _()
#endif
		{
		MActor^ get()
		{
			return MActor::GetFromActor(m_networkClient->GetActorAvatar());
		}
		void set(MActor^ value)
		{
			if(value != nullptr)
				m_networkClient->SetActorAvatar(value->m_actor);
			else
				m_networkClient->SetActorAvatar(NULL);
		}
		}

// <summary>
/// 
// </summary>
		property bool IsAdministrator
#ifdef DOXYGEN_IGNORE 
; _()
#endif
		{
			bool get()
			{
				return m_networkClient->m_IsAdministrator;
			}
			void set(bool value)
			{
				m_networkClient->m_IsAdministrator=value;
			}
		}

// <summary>
/// 
// </summary>
		property bool IsServer
		{
			bool get()
			{
				return m_networkClient->m_IsServer;
			}
			void set(bool value)
			{
				m_networkClient->m_IsServer=value;
			}
		}

// <summary>
/// 
// </summary>
		MNetworkActorPackets^ GetNetworkActorPackets(MNetworkActor^ forNetworkActor)
		{
			return gcnew MNetworkActorPackets(m_networkClient->GetNetworkActorPackets((NetworkActor*)forNetworkActor->m_actor));
		}

// <summary>
/// 
// </summary>
		MNetworkActorPackets^ GetOrCreateNetworkActorPackets(MNetworkActor^ forNetworkActor)
		{
			return gcnew MNetworkActorPackets(m_networkClient->GetOrCreateNetworkActorPackets((NetworkActor*)forNetworkActor->m_actor));
		}

	};


// <summary>
/// Provides functions interfacing with the static Reality Server object
// </summary>
	public ref class MServer
	{
	public:
		
// <summary>
/// sends a print message to the specified networkclient, or if null sends to everyone
// </summary>
		static void SendGameMessage(MNetworkClient^ client, String^ text)
		{
			if(client != nullptr)
				Server::Instance()->SendChat(client->m_networkClient,Helpers::ToCppString(text).c_str());
			else
				Server::Instance()->SendChat(NULL,Helpers::ToCppString(text).c_str());
		}

// <summary>
/// forces the Server to reload its user preference settings based on the current INI values
// </summary>
		static void LoadPreferences()
		{
			Server::Instance()->LoadPreferences();
		}

// <summary>
/// loads a new map in server mode, defaulting multiplayer to current value
// </summary>
		static void NewMap(String^ MapFileName)
		{
			Server::Instance()->BeginHosting(Server::Instance()->m_ServerSettings.m_SessionName,Helpers::ToCppString(MapFileName),Server::Instance()->m_IsMultiplayer);
		}

// <summary>
/// loads a new map in server mode, specifying new multiplayer value
// </summary>
		static void NewMap(String^ MapFileName, bool isMultiplayer)
		{
			Server::Instance()->BeginHosting(Server::Instance()->m_ServerSettings.m_SessionName,Helpers::ToCppString(MapFileName),isMultiplayer);
		}
		
		// <summary>
		/// Delimited names all clients connetected to the game session, including those who have not spawned yet. Accessible on Client systems too!
		// </summary>
		static property String^ ClientNames
#ifdef DOXYGEN_IGNORE 
; _()
#endif
		{
			String^ get()	
			{return Helpers::ToCLIString(Server::Instance()->m_ObserverNames);}
		}

		// <summary>
		/// The name of the current server session, if server's started
		// </summary>
		static property String^ SessionName
#ifdef DOXYGEN_IGNORE 
; _()
#endif
		{
			String^ get()	
			{return Helpers::ToCLIString(Server::Instance()->m_ServerSettings.m_SessionName);}
		}


		// <summary>
		/// current loaded  map pathless filename on the server, sent to clients when they join
		// </summary>
		static property String^ CurrentMap
#ifdef DOXYGEN_IGNORE 
; _()
#endif
		{
			String^ get()	
			{return Helpers::ToCLIString(Server::Instance()->m_CurrentMap);}
		}


		// <summary>
		/// Whether the Server is currently running in networked multiplayer mode
		// </summary>
		static property bool IsMultiplayer
#ifdef DOXYGEN_IGNORE 
; _()
#endif
		{
			bool get()	
			{return Server::Instance()->m_IsMultiplayer;}
		}

	};

// <summary>
/// Provides functions interfacing with the static Reality Client object
// </summary>
	public ref class MClient
	{
	public:
		
// <summary>
/// sends request to Server to enter game, ideal to send sometime after completed joining & load
// </summary>
		static void SendEnterGame()
		{
			GameClientModule::Instance()->SendEnterGame();
		}

// <summary>
/// sends request for Client to connect to a game session running at the specified network address
// </summary>
		static void Connect(String^ serverAddress)
		{
			Client::Instance()->ConnectToHost(Helpers::ToCppString(serverAddress));
		}

// <summary>
/// forces the Client to reload its user preference settings based on the current INI values
// </summary>
		static void LoadPreferences()
		{
			Client::Instance()->LoadPreferences();
		}

// <summary>
/// Gets the average TTL ping to the Server. -1 if not connected.
// </summary>
		static int GetAveragePing()
		{
			return Client::Instance()->GetAveragePing();
		}

		// <summary>
		/// Whether the Client is currently connected to a Server
		// </summary>
		static property bool IsConnected
#ifdef DOXYGEN_IGNORE 
; _()
#endif
		{
			bool get()	
			{return Client::Instance()->IsConnected();}
		}
	};


}






