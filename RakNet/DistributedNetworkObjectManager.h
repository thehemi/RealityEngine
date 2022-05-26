// This file is part of RakNet Copyright 2003, 2004 Rakkarsoft LLC and Kevin Jenkins.

// Usage of Raknet is subject to the appropriate licence agreement.
// "Shareware" Licensees with Rakkarsoft LLC are subject to the shareware license found at http://www.rakkarsoft.com/shareWareLicense.html which you agreed to upon purchase of a "Shareware license"
// "Commercial" Licensees with Rakkarsoft LLC are subject to the commercial license found at http://www.rakkarsoft.com/sourceCodeLicense.html which you agreed to upon purchase of a "Commercial license"
// All other users are subject to the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

// Refer to the appropriate license agreement for distribution, modification, and warranty rights.

#ifndef __DISTRIBUTED_NETWORK_OBJECT_MANAGER_H
#define __DISTRIBUTED_NETWORK_OBJECT_MANAGER_H

#include "ArrayList.h"
#include "NetworkTypes.h"
#include "EncodeClassName.h"

class DistributedNetworkObject;
class DistributedNetworkObjectBaseStub;
struct Packet;
class RakServerInterface;
class RakClientInterface;

class DistributedNetworkObjectManager
{
public:
	DistributedNetworkObjectManager();
	~DistributedNetworkObjectManager();

	// You need to register your instance of RakServer and/or RakClient to be usable by the DistributedNetworkObjectManager
	void RegisterRakServerInterface(RakServerInterface *_rakServerInterface);
	void RegisterRakClientInterface(RakClientInterface *_rakClientInterface);
	RakServerInterface *GetRakServerInterface(void) const;
	RakClientInterface *GetRakClientInterface(void) const;

	DistributedNetworkObject* HandleDistributedNetworkObjectPacket(Packet *packet);
	void HandleDistributedNetworkObjectPacketCreationAccepted(Packet *packet);
	void HandleDistributedNetworkObjectPacketCreationRejected(Packet *packet);

	bool ExistsNetworkObject(DistributedNetworkObject *object);
	bool RegisterNetworkObject(DistributedNetworkObject *object,char classIdentifier[MAXIMUM_CLASS_IDENTIFIER_LENGTH], unsigned char &localObjectIndex); // True on successful add, false on already exists
	void UnregisterNetworkObject(DistributedNetworkObject *object);
	// This will send all registered network objects to the specified player (server only)
	void SendAllDistributedObjects(PlayerID playerId);

	static inline DistributedNetworkObjectManager* Instance() {return &I;}
	void AddClassStub(DistributedNetworkObjectBaseStub *stub);
	DistributedNetworkObject *GetClassInstanceByIdentifier(char *classIdentifier);
protected:
	static DistributedNetworkObjectManager I;

	DistributedNetworkObject* GetObjectByLocalObjectIndex(unsigned char localObjectIndex);

	struct DistributedNetworkObjectRegistryNode
	{
		DistributedNetworkObject* object;
		char classIdentifier[MAXIMUM_CLASS_IDENTIFIER_LENGTH];
	};

	BasicDataStructures::List<DistributedNetworkObjectBaseStub*> classList;
	BasicDataStructures::List<DistributedNetworkObjectRegistryNode*> distributedNetworkObjectInstanceRegistry;

	RakServerInterface *rakServerInterface;
	RakClientInterface *rakClientInterface;

	unsigned char localObjectIdentifier; // This cycles from 0 to 255 and around giving identifiers to objects that don't have IDs set by the server yet.  This way we can still access them
};

#endif

