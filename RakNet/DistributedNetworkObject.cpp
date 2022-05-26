// This file is part of RakNet Copyright 2003, 2004 Rakkarsoft LLC and Kevin Jenkins.

// Usage of Raknet is subject to the appropriate licence agreement.
// "Shareware" Licensees with Rakkarsoft LLC are subject to the shareware license found at http://www.rakkarsoft.com/shareWareLicense.html which you agreed to upon purchase of a "Shareware license"
// "Commercial" Licensees with Rakkarsoft LLC are subject to the commercial license found at http://www.rakkarsoft.com/sourceCodeLicense.html which you agreed to upon purchase of a "Commercial license"
// All other users are subject to the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

// Refer to the appropriate license agreement for distribution, modification, and warranty rights.

#include "DistributedNetworkObject.h"
#include "PacketEnumerations.h"
#include "BitStream.h"
#include "RakClientInterface.h"
#include "RakServerInterface.h"
#include "GetTime.h"
#include <assert.h>
#include <string.h>
#include "DistributedNetworkObjectManager.h"


DistributedNetworkObject::DistributedNetworkObject()
{
	maximumUpdateFrequency=50; // 50 ms by default
	lastBroadcastTime=0;
	localObject=true;
	localObjectIdentifier=255;
	distributedMemoryInitialized=false;
	clientOwnerID=UNASSIGNED_PLAYER_ID;
	memset(heapNodeList, 0, sizeof(heapNodeList));
}

DistributedNetworkObject::~DistributedNetworkObject()
{
	int heapNodeIndex;

	if (DistributedNetworkObjectManager::Instance()->GetRakServerInterface())
	{
		// These 3 lines of code will broadcast to destroy the object
		RakNet::BitStream bitStream;
		SerializeClassHeader(&bitStream, 0, 0, false, 0);
		BroadcastSerializedClass(&bitStream);
	}

	DistributedNetworkObjectManager::Instance()->UnregisterNetworkObject(this);

	for (heapNodeIndex=0; heapNodeIndex < 256; heapNodeIndex++)
	{
		if (heapNodeList[heapNodeIndex].lastWriteValue)
		{
			delete [] heapNodeList[heapNodeIndex].lastWriteValue;
			heapNodeList[heapNodeIndex].lastWriteValue=0;
		}
	}
}

void DistributedNetworkObject::DestroyObjectOnNetwork(void)
{
	// These 3 lines of code will broadcast to destroy the object
	RakNet::BitStream bitStream;
	SerializeClassHeader(&bitStream, 0, 0, false, 0);
	BroadcastSerializedClass(&bitStream);

	DistributedNetworkObjectManager::Instance()->UnregisterNetworkObject(this);

	// OnDistributedObjectDestruction gets called on the clients from the broadcast.  On the server we have to call it directly.
	if (DistributedNetworkObjectManager::Instance()->GetRakServerInterface())
	{
		OnDistributedObjectDestruction(UNASSIGNED_PLAYER_ID);
	}
}

void DistributedNetworkObject::SynchronizeMemory(unsigned char memoryBlockIndex, char* memoryBlock, unsigned short memoryBlockSize, bool serverAuthority)
{
	// Was memory previously allocated?
	if (heapNodeList[memoryBlockIndex].lastWriteValue)
	{
		// If new blocksize is greater than allocated blocksize, then delete and reallocate the block
		if (memoryBlockSize > heapNodeList[memoryBlockIndex].allocatedBlockSize)
		{
			delete [] heapNodeList[memoryBlockIndex].lastWriteValue;
			heapNodeList[memoryBlockIndex].lastWriteValue = new char [memoryBlockSize];
			heapNodeList[memoryBlockIndex].allocatedBlockSize = memoryBlockSize;
		}
		// else we just change the amount we use down below
	}
	else
	{
		// Else the existing block is empty
		// Allocate the block.
		heapNodeList[memoryBlockIndex].lastWriteValue = new char [memoryBlockSize];
		heapNodeList[memoryBlockIndex].allocatedBlockSize = memoryBlockSize;
	}

	heapNodeList[memoryBlockIndex].usedBlockSize=memoryBlockSize;

	// Change what we are watching
	heapNodeList[memoryBlockIndex].watchedData=memoryBlock;

	// Initialize nextUpdateTime to the current time.
	heapNodeList[memoryBlockIndex].nextUpdateTime=RakNetGetTime();

	// Set the authority
	heapNodeList[memoryBlockIndex].serverAuthority=serverAuthority;
}
void DistributedNetworkObject::DesynchronizeMemory(unsigned char memoryBlockIndex)
{
	if (IsMemoryBlockIndexUsed(memoryBlockIndex)==false)
		return;

	heapNodeList[memoryBlockIndex].watchedData=0;

	// If a lot of data was allocated, free it.
	if (heapNodeList[memoryBlockIndex].allocatedBlockSize > 4)
	{
		heapNodeList[memoryBlockIndex].allocatedBlockSize=0;
		delete [] heapNodeList[memoryBlockIndex].lastWriteValue;
		heapNodeList[memoryBlockIndex].lastWriteValue=0;
	}
}
void DistributedNetworkObject::SetAuthority(unsigned char memoryBlockIndex, bool serverAuthority)
{
	heapNodeList[memoryBlockIndex].serverAuthority=serverAuthority;
}
bool DistributedNetworkObject::IsMemoryBlockIndexUsed(unsigned char memoryBlockIndex)
{
	return heapNodeList[memoryBlockIndex].watchedData!=0;
}
void DistributedNetworkObject::SetMaxMemoryBlockUpdateFrequency(unsigned char memoryBlockIndex, int max)
{
	heapNodeList[memoryBlockIndex].maximumUpdateInterval=max;
}
// Same as ProcessDistributedMemoryStack, but for the heapNodeList.
// Returns true if data was written
bool DistributedNetworkObject::WriteToBitstreamFromHeap(RakNet::BitStream *bitStream, bool forceWrite)
{
    int heapNodeIndex;
	unsigned long time;
	RakNet::BitStream heapData(256);
	time=RakNetGetTime();
	unsigned char numberOfBlocksWritten;

	numberOfBlocksWritten=0;
	
	for (heapNodeIndex=0; heapNodeIndex < 256; heapNodeIndex++)
	{
		// Is this a block we are responsible for updating, and it is time to update?
		if (IsMemoryBlockIndexUsed((unsigned char)heapNodeIndex) &&
			forceWrite ||
			(time >= heapNodeList[heapNodeIndex].nextUpdateTime &&
			((DistributedNetworkObjectManager::Instance()->GetRakServerInterface() && heapNodeList[heapNodeIndex].serverAuthority) ||
			(DistributedNetworkObjectManager::Instance()->GetRakClientInterface() && heapNodeList[heapNodeIndex].serverAuthority==false))))
		{
			// Is the block different between the current value and the last write?
			if (forceWrite || memcmp(heapNodeList[heapNodeIndex].lastWriteValue, heapNodeList[heapNodeIndex].watchedData, heapNodeList[heapNodeIndex].usedBlockSize)!=0)
			{
				// Block is different.
				// Write the new data to the given bitstream and write it to our copy of the data
				heapData.Write((unsigned char) heapNodeIndex);
#ifdef _DEBUG
				// In debug also send the block size so we can check that it matches.
				heapData.Write(heapNodeList[heapNodeIndex].usedBlockSize);
#endif
				heapData.Write((char*)heapNodeList[heapNodeIndex].watchedData, heapNodeList[heapNodeIndex].usedBlockSize);
				memcpy(heapNodeList[heapNodeIndex].lastWriteValue, heapNodeList[heapNodeIndex].watchedData, heapNodeList[heapNodeIndex].usedBlockSize);
				
				// Update the next write time.
				heapNodeList[heapNodeIndex].nextUpdateTime = time + heapNodeList[heapNodeIndex].maximumUpdateInterval;  

				numberOfBlocksWritten++;
			}
		}
	}

	if (numberOfBlocksWritten>0)
	{
		bitStream->WriteCompressed(numberOfBlocksWritten);
		bitStream->Write((char*)heapData.GetData(), heapData.GetNumberOfBytesUsed());
		return true;
	}

	if (forceWrite)
		WriteCreationData(bitStream);

	return false;
}

void DistributedNetworkObject::WriteToHeapFromBitstream(RakNet::BitStream *bitStream)
{
	int index;
	unsigned char heapNodeIndex;
	unsigned char numberOfBlocksWritten;

#ifdef _DEBUG
	unsigned short usedBlockSize;
#endif

	bitStream->ReadCompressed(numberOfBlocksWritten);

	for (index=0; index < numberOfBlocksWritten; index++)
	{
		if (bitStream->Read(heapNodeIndex)==false)
			return;
		
		if (IsMemoryBlockIndexUsed(heapNodeIndex)==false)
		{
#ifdef _DEBUG
			// If this assert hits, then the sender wrote to block heapNodeIndex which was
			// never specified as being in use by the recipient.  This is probably
			// caused by memoryBlockIndex not matching the same value on another system for the
			// same variable when calling SynchronizeMemory.
			// However, if you are calling
			// SynchronizeMemory at runtime on both systems then this could also be caused by lag
			// and you can ignore it.
			//
			assert(0);
#endif
			return;
		}

#ifdef _DEBUG
		if (bitStream->Read(usedBlockSize)==false)
			return;
		// If this assert hits, the block size specified by sending system for this heapNodeIndex
		// does not match the block size specified by the receiving system.  This is always a bug.
		assert(heapNodeList[heapNodeIndex].usedBlockSize==usedBlockSize);
#endif

		if (bitStream->Read((char*)heapNodeList[heapNodeIndex].watchedData, heapNodeList[heapNodeIndex].usedBlockSize)==false)
			return;

		// Read the data into the watched data.  Now copy that into the last write value so we don't write it again.
		memcpy(heapNodeList[heapNodeIndex].lastWriteValue,heapNodeList[heapNodeIndex].watchedData,heapNodeList[heapNodeIndex].usedBlockSize);
	}
}

// Writes to or reads from a bitstream for all distributed memory.  This function does not need to be modified by the end-user
bool DistributedNetworkObject::ProcessDistributedMemoryStack(RakNet::BitStream *bitStream, bool isWrite, bool forceWrite, bool isServerAuthoritative)
{
	return false;
}

// Interpolates between all variables on the network that are set to interpolate.  This function does not need to be modified by the end-user
void DistributedNetworkObject::InterpolateDistributedMemory(bool isServerAuthoritative)
{
}

// This function cannot go in the constructor because the vtable won't be setup to call GetClassName
void DistributedNetworkObject::UpdateDistributedObject(char *classID, bool isClassIDEncoded)
{
	bool serverProcess;
	bool dataWritten1, dataWritten2;

	if (DistributedNetworkObjectManager::Instance()->GetRakServerInterface() &&
		DistributedNetworkObjectManager::Instance()->GetRakServerInterface()->IsActive())
		serverProcess=true;
	else
		serverProcess=false;

	if ((DistributedNetworkObjectManager::Instance()->GetRakServerInterface() && DistributedNetworkObjectManager::Instance()->GetRakServerInterface()->IsActive()) ||
	 (DistributedNetworkObjectManager::Instance()->GetRakClientInterface() && DistributedNetworkObjectManager::Instance()->GetRakClientInterface()->IsConnected()))
	{
		char classIdentifier[MAXIMUM_CLASS_IDENTIFIER_LENGTH];
		RakNet::BitStream bitStream;

		if (isClassIDEncoded)
			memcpy(classIdentifier, classID, MAXIMUM_CLASS_IDENTIFIER_LENGTH);
		else
			EncodeClassName(classID, classIdentifier);

		bool firstTimeAddedToRegistry;
		// The only purpose of registering and unregistering network objects is so we can call SendAllDistributedObjects from
		// DistributedNetworkObjectManager to new players
		firstTimeAddedToRegistry = DistributedNetworkObjectManager::Instance()->RegisterNetworkObject(this,classIdentifier, localObjectIdentifier);

		// This logic is correct - ProcessDistributedMemoryStack has a side effect of updating network memory to the last send so you don't
		// want to call it unnecessarily
		if (firstTimeAddedToRegistry && (localObject || DistributedNetworkObjectManager::Instance()->GetRakServerInterface()))
		{
			SerializeClassHeader(&bitStream, classIdentifier, 2, firstTimeAddedToRegistry && DistributedNetworkObjectManager::Instance()->GetRakClientInterface(), localObjectIdentifier);
			ProcessDistributedMemoryStack(&bitStream, true, true,serverProcess);
			WriteToBitstreamFromHeap(&bitStream, true);	
			BroadcastSerializedClass(&bitStream);
		}
		else if ((localObject == false || DistributedNetworkObjectManager::Instance()->GetRakServerInterface()) &&
			RakNetGetTime() > lastBroadcastTime + maximumUpdateFrequency)
		{
			SerializeClassHeader(&bitStream, classIdentifier, 1, firstTimeAddedToRegistry && DistributedNetworkObjectManager::Instance()->GetRakClientInterface(), localObjectIdentifier);
			dataWritten1=ProcessDistributedMemoryStack(&bitStream, true, false,serverProcess);
			dataWritten2=WriteToBitstreamFromHeap(&bitStream, false);
			if (dataWritten1 || dataWritten2)
			{					
				BroadcastSerializedClass(&bitStream);
			}
		}
	}

	InterpolateDistributedMemory(serverProcess);
}

void DistributedNetworkObject::WriteCreationData(RakNet::BitStream *initialData)
{
}

void DistributedNetworkObject::ReadCreationData(RakNet::BitStream *initialData)
{
}

void DistributedNetworkObject::SetMaximumUpdateFrequency(unsigned long frequency)
{
	maximumUpdateFrequency=frequency;
}

void DistributedNetworkObject::BroadcastSerializedClass(RakNet::BitStream *bitStream)
{
	if (DistributedNetworkObjectManager::Instance()->GetRakServerInterface() && DistributedNetworkObjectManager::Instance()->GetRakServerInterface()->IsActive())
	{
		PlayerID localClient = UNASSIGNED_PLAYER_ID;
		// By default send to everyone except the local client to avoid duplicate creation
		if (DistributedNetworkObjectManager::Instance()->GetRakClientInterface() && DistributedNetworkObjectManager::Instance()->GetRakClientInterface()->IsConnected())
		{
			localClient = DistributedNetworkObjectManager::Instance()->GetRakClientInterface()->GetPlayerID();
		}

		DistributedNetworkObjectManager::Instance()->GetRakServerInterface()->Send(bitStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, localClient, true);
		lastBroadcastTime=RakNetGetTime();
	}
	else if (DistributedNetworkObjectManager::Instance()->GetRakClientInterface() && DistributedNetworkObjectManager::Instance()->GetRakClientInterface()->IsConnected())
	{
		// This sends a request to create or update the object
		DistributedNetworkObjectManager::Instance()->GetRakClientInterface()->Send(bitStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0);
		lastBroadcastTime=RakNetGetTime();
	}
}

void DistributedNetworkObject::SerializeClassHeader(RakNet::BitStream *bitStream, char *classIdentifier, int action, bool localObject, unsigned char localObjectIndex)
{
	static const unsigned char packetId = ID_UPDATE_DISTRIBUTED_NETWORK_OBJECT;

	bitStream->Write(packetId);

	// Construction to true means we are creating or updating this object.  Otherwise it means we are destroying this object.
	bitStream->WriteBits((unsigned char*)&action, 2);

	bitStream->Write(GetID());

	if (action==2)
	{
		bitStream->Write(classIdentifier, classIdentifier[0]+1);
	}

	bitStream->Write(localObject);
	if (localObject)
		bitStream->Write(localObjectIndex);
}

bool DistributedNetworkObject::DeserializeClassHeader(RakNet::BitStream *bitStream, int &action, ObjectID &objectId, char classIdentifier[MAXIMUM_CLASS_IDENTIFIER_LENGTH], bool &localObject, unsigned char &localObjectIndex)
{
	unsigned char packetId;
	if (bitStream->Read(packetId)==false)
		return false;

	assert(packetId==ID_UPDATE_DISTRIBUTED_NETWORK_OBJECT);

	action=0;
	if (bitStream->ReadBits((unsigned char*)&action,2)==false)
		return false;

	if (bitStream->Read(objectId)==false)
		return false;

	if (action==2)
	{
		unsigned char classNameLength;
		if (bitStream->Read(classNameLength)==false)
			return false;
		if (bitStream->Read(classIdentifier+1, classNameLength)==false)
			return false;

		classIdentifier[0]=classNameLength;
	}
	else
		classIdentifier=0;

	bool isLocalObjectInpacket;
	if (bitStream->Read(isLocalObjectInpacket)==false)
		return false;
	if (isLocalObjectInpacket)
	{
		if (bitStream->Read(localObjectIndex)==false)
			return false;
	}

	return true;
}

bool DistributedNetworkObject::AllowSpectatorUpdate(PlayerID sender)
{
	return false;
}

void DistributedNetworkObject::OnNetworkUpdate(PlayerID sender)
{
}

bool DistributedNetworkObject::OnDistributedObjectCreation(PlayerID senderID)
{
#ifdef _DEBUG
	assert(GetID()!=UNASSIGNED_OBJECT_ID);
#endif

	return true;
}

void DistributedNetworkObject::OnDistributedObjectDestruction(PlayerID senderID)
{
	delete this;
}

bool DistributedNetworkObject::IsLocalObject(void) const
{
	return localObject;
}

void DistributedNetworkObject::OnDistributedObjectRejection(void)
{
	delete this;
}

unsigned char DistributedNetworkObject::GetLocalObjectIdentifier(void) const
{
	return localObjectIdentifier;
}

void DistributedNetworkObject::DistributedMemoryInit(bool isServerAuthoritative)
{
	distributedMemoryInitialized=true;
}

PlayerID DistributedNetworkObject::GetClientOwnerID(void) const
{
	return clientOwnerID;
}
void DistributedNetworkObject::SetClientOwnerID(PlayerID id)
{
	clientOwnerID=id;
}

void DistributedNetworkObject::SetLocalObject(bool b)
{
	localObject=b;
}

