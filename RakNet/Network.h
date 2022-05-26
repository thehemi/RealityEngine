// This file is part of RakNet Copyright 2003, 2004 Rakkarsoft LLC and Kevin Jenkins.

// Usage of Raknet is subject to the appropriate licence agreement.
// "Shareware" Licensees with Rakkarsoft LLC are subject to the shareware license found at http://www.rakkarsoft.com/shareWareLicense.html which you agreed to upon purchase of a "Shareware license"
// "Commercial" Licensees with Rakkarsoft LLC are subject to the commercial license found at http://www.rakkarsoft.com/sourceCodeLicense.html which you agreed to upon purchase of a "Commercial license"
// All other users are subject to the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

// Refer to the appropriate license agreement for distribution, modification, and warranty rights.

#ifndef __NETWORK_H
#define __NETWORK_H

#include "SocketLayer.h"
#include "Queue.h"
#include "PacketPriority.h"
#include "List.h"
#include "BitStream.h"
#include "ReliabilityLayer.h"
#include "BinarySearchTree.h"
#include "SimpleMutex.h"
#include "PacketPool.h"

class HuffmanEncodingTree;

// RPC stuff.  Ignore this
struct RPCNode
{
	char *uniqueIdentifier;
	void (*functionName)(char *input, int numberOfBitsOfData, PlayerID sender);
	RPCNode(char* uniqueID, void (*_functionName)(char *input, int numberOfBitsOfData, PlayerID sender));
	RPCNode& operator = (const RPCNode& input);
	RPCNode(const RPCNode& input);

	RPCNode();
	~RPCNode();

	friend int operator==(const RPCNode& left, const RPCNode& right);
	friend int operator > (const RPCNode& left, const RPCNode& right);
	friend int operator < (const RPCNode& left, const RPCNode& right);
};

// Common stuff between the client and the server
class Network
{
public:
	Network();
	void DesynchronizeMemory(UniqueIDType uniqueIdentifier, ObjectID secondaryUniqueIdentifier);
	void SynchronizeMemory(UniqueIDType uniqueIdentifier, char *memoryBlock, unsigned short size, bool isAuthority, bool (*synchronizationRules) (char*,char*),ObjectID secondaryUniqueIdentifier);
	void GenerateCompressionLayer(unsigned long inputFrequencyTable[256], bool inputLayer);
	void DeleteCompressionLayer(bool inputLayer);
	void GetSendFrequencyTable(unsigned long outputFrequencyTable[256]);
	float GetCompressionRatio(void) const;
	float GetDecompressionRatio(void) const;
	void RegisterAsRemoteProcedureCall(char* uniqueID, void (*functionName)(char *input, int numberOfBitsOfData, PlayerID sender));
	void UnregisterAsRemoteProcedureCall(char* uniqueID);
	bool HandleRPCPacket(char *data, int length, PlayerID playerId);
protected:
	BitStream * UpdateSynchronizedMemory(void);
	void HandleUpdateSynchronizeMemory(char *data, int length);

	// Data that both the client and the server needs
	unsigned long bytesSentPerSecond, bytesReceivedPerSecond;
	bool continualPing,isRecvfromThreadActive,isMainLoopThreadActive, endThreads, isSocketLayerBlocking;
	unsigned long validationInteger;
#ifdef _WIN32
	HANDLE
#else
    pthread_t
#endif 
		processPacketsThreadHandle, recvfromThreadHandle;
	SimpleMutex incomingQueueMutex,synchronizedMemoryQueueMutex, automaticVariableSynchronizationMutex;
	BasicDataStructures::Queue<Packet *> incomingPacketQueue, synchronizedMemoryPacketQueue;
//	BitStream enumerationData;

	// automatic variable synchronization takes a primary and secondary identifier
	// The unique primary identifier is the index into the automaticVariableSynchronizationList
	// The unique secondary identifier (UNASSIGNED_OBJECT_ID for none) is in an unsorted list of memory blocks
	struct MemoryBlock
	{
		char *original, *copy;
		unsigned short size;
		ObjectID secondaryID;
		bool isAuthority;
		bool (*synchronizationRules) (char*,char*);
	};
	BasicDataStructures::List<BasicDataStructures::List<MemoryBlock>* > automaticVariableSynchronizationList;

	// Compression stuff
	unsigned long frequencyTable[256];
	HuffmanEncodingTree *inputTree, *outputTree;
	unsigned long rawBytesSent, rawBytesRecieved, compressedBytesSent, compressedBytesRecieved;
	void DecompressInput(BitStream *bitStream);
	void UpdateOutgoingFrequencyTable(BitStream* bitStream);

	BasicDataStructures::AVLBalancedBinarySearchTree<RPCNode> rpcTree;
	int MTUSize;
	bool trackFrequencyTable;
	int threadPriority;

	SOCKET connectionSocket;
};


#endif