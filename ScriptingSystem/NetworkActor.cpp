#include "stdafx.h"
#include "Wrappers.h"
#include "NetworkActor.h"
#include "ScriptingEngine.h"
#include "ScriptingSystem.h"

using namespace System::IO;
using namespace ScriptingSystem;
using namespace stdcli::language;

using namespace ScriptingSystem;

void MNetworkActor::Server_MakeTickMessages(MNetworkActorPackets^ packet)
		{
//filters out redundant sends of rotation, velocity, and location

	bool SendRotation = false;

	MVector^ rotDir = Rotation->GetDir();
	if(rotDir != packet->m_LastSentRotation && (rotDir - packet->m_LastSentRotation)->Length() > SERVER_REDUNDANT_PHYSICS_SEND_ROTATION_THRESHOLD)
	{
		packet->m_LastSentRotation = rotDir;
		SendRotation = true;
	}

	bool SendVelocity = false;
	if(Velocity != packet->m_LastSentVelocity && (Velocity - packet->m_LastSentVelocity)->Length() > SERVER_REDUNDANT_PHYSICS_SEND_VELOCITY_THRESHOLD)
	{
		packet->m_LastSentVelocity = Velocity;
		SendVelocity = true;
	}

	bool SendLocation = false;
	if(Location != packet->m_LastSentLocation && (Location - packet->m_LastSentLocation)->Length() > SERVER_REDUNDANT_PHYSICS_SEND_LOCATION_THRESHOLD)
	{
		packet->m_LastSentLocation = Location;
		SendLocation = true;
	}

	if(SendRotation)
		WriteRotation(packet->GetMessagePacket(PACKET_UNRELIABLE));

  if(SendVelocity)
		WriteVelocity(packet->GetMessagePacket(PACKET_UNRELIABLE));

 if(SendLocation)
		WriteLocation(packet->GetMessagePacket(PACKET_UNRELIABLE));
        }

MNetworkActorPackets^ MNetworkActor::Server_MakeOnJoinSynchMessages(MNetworkClient^ client)
        {
			MNetworkActorPackets^ packet = client->GetOrCreateNetworkActorPackets(this);
			WriteMessageType(packet->GetMessagePacket(PACKET_SPAWN),(unsigned char)MSG::MSGID_NETWORKACTOR_LOCATION);
			WriteVector(packet->GetMessagePacket(PACKET_SPAWN),Location);
            return packet;
        }