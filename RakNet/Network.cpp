// This file is part of RakNet Copyright 2003, 2004 Rakkarsoft LLC and Kevin Jenkins.

// Usage of Raknet is subject to the appropriate licence agreement.
// "Shareware" Licensees with Rakkarsoft LLC are subject to the shareware license found at http://www.rakkarsoft.com/shareWareLicense.html which you agreed to upon purchase of a "Shareware license"
// "Commercial" Licensees with Rakkarsoft LLC are subject to the commercial license found at http://www.rakkarsoft.com/sourceCodeLicense.html which you agreed to upon purchase of a "Commercial license"
// All other users are subject to the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

// Refer to the appropriate license agreement for distribution, modification, and warranty rights.

#include "Network.h"
#include "BitStream.h"
#include "HuffmanEncodingTree.h"
#include <assert.h>
//#include "MemoryManager.h"
#include <ctype.h>
#include "MTUSize.h"
#include "PacketEnumerations.h"
#include <ctype.h>

RPCNode::RPCNode()
{
	uniqueIdentifier=0;
	functionName=0;
}

RPCNode::RPCNode(char* uniqueID, void (*_functionName)(char *input, int numberOfBitsOfData, PlayerID sender))
{
	#ifdef _DEBUG
	assert(uniqueID);
	assert(uniqueID[0]);
	#endif
	uniqueIdentifier = new char [strlen(uniqueID)+1];
	strcpy(uniqueIdentifier, uniqueID);
	functionName=_functionName;
}

RPCNode::RPCNode(const RPCNode& input)
{
	if (input.uniqueIdentifier!=0)
	{
		uniqueIdentifier = new char [strlen(input.uniqueIdentifier)+1];
		strcpy(uniqueIdentifier, input.uniqueIdentifier);
	}
	else
	{
		uniqueIdentifier=0;
	}

	functionName=input.functionName;
}

RPCNode& RPCNode::operator = (const RPCNode& input)
{
	if (&input == this)
		return *this;

	if (input.uniqueIdentifier!=0)
	{
		if (uniqueIdentifier!=0)
			delete [] uniqueIdentifier;
		uniqueIdentifier = new char [strlen(input.uniqueIdentifier)+1];
		strcpy(uniqueIdentifier, input.uniqueIdentifier);
	}
	else
	{
		delete uniqueIdentifier;
		uniqueIdentifier=0;
	}

	functionName=input.functionName;

	return *this;
}


RPCNode::~RPCNode()
{
	delete [] uniqueIdentifier;
}

int operator==(const RPCNode& left, const RPCNode& right)
{
	if (strcmp(left.uniqueIdentifier, right.uniqueIdentifier)==0) return !0;
	return 0;
}

int operator>(const RPCNode& left, const RPCNode& right)
{
	if (strcmp(left.uniqueIdentifier, right.uniqueIdentifier)>0) return !0;
	return 0;
}
int operator<(const RPCNode& left, const RPCNode& right)
{
	if (strcmp(left.uniqueIdentifier, right.uniqueIdentifier)<0) return !0;
	return 0;
}

Network::Network()
{
	memset(frequencyTable, 0, sizeof(unsigned long) * 256);
	rawBytesSent=rawBytesRecieved=compressedBytesSent=compressedBytesRecieved=0;
	outputTree=inputTree=0;
	connectionSocket=INVALID_SOCKET;
	MTUSize=DEFAULT_MTU_SIZE;
	trackFrequencyTable=false;
}

void Network::DesynchronizeMemory(UniqueIDType uniqueIdentifier, ObjectID secondaryUniqueIdentifier)
{
	automaticVariableSynchronizationMutex.Lock();
	#ifdef _DEBUG
	assert(uniqueIdentifier < automaticVariableSynchronizationList.size());
	#endif
	if (uniqueIdentifier >= automaticVariableSynchronizationList.size())
	{
		automaticVariableSynchronizationMutex.Unlock();
		return;
	}
	#ifdef _DEBUG
	assert(automaticVariableSynchronizationList[uniqueIdentifier]!=0);
	#endif
	if (automaticVariableSynchronizationList[uniqueIdentifier]==0)
	{
		automaticVariableSynchronizationMutex.Unlock();
		return;
	}

	// If we don't specify a secondary identifier, then the list must only have one element
	#ifdef _DEBUG
	assert(!(secondaryUniqueIdentifier==UNASSIGNED_OBJECT_ID && automaticVariableSynchronizationList[uniqueIdentifier]->size()!=1));
	#endif
	if (secondaryUniqueIdentifier==UNASSIGNED_OBJECT_ID && automaticVariableSynchronizationList[uniqueIdentifier]->size()!=1)
	{
		automaticVariableSynchronizationMutex.Unlock();
		return;
	}

	for (unsigned i=0; i < automaticVariableSynchronizationList[uniqueIdentifier]->size(); i++)
	{
		if ((*(automaticVariableSynchronizationList[uniqueIdentifier]))[i].secondaryID == secondaryUniqueIdentifier)
		{
			delete [] (*(automaticVariableSynchronizationList[uniqueIdentifier]))[i].copy;
			automaticVariableSynchronizationList[uniqueIdentifier]->del(i);
			if (automaticVariableSynchronizationList[uniqueIdentifier]->size()==0) // The sublist is now empty
			{
				delete automaticVariableSynchronizationList[uniqueIdentifier];
				automaticVariableSynchronizationList[uniqueIdentifier]=0;
				automaticVariableSynchronizationMutex.Unlock();
				return;
			}
		}
	}

	automaticVariableSynchronizationMutex.Unlock();
}

void Network::SynchronizeMemory(UniqueIDType uniqueIdentifier, char *memoryBlock, unsigned short size, bool isAuthority, bool (*synchronizationRules) (char*,char*),ObjectID secondaryUniqueIdentifier)
{
	automaticVariableSynchronizationMutex.Lock();
	if (uniqueIdentifier >= automaticVariableSynchronizationList.size() || automaticVariableSynchronizationList[uniqueIdentifier]==0)
	{
		automaticVariableSynchronizationList.replace(new BasicDataStructures::List<MemoryBlock>, 0, uniqueIdentifier);
	}
	else
	{
		// If we are using a secondary identifier, make sure that is unique
#ifdef _DEBUG
		assert(secondaryUniqueIdentifier!=UNASSIGNED_OBJECT_ID);
#endif
		if (secondaryUniqueIdentifier==UNASSIGNED_OBJECT_ID)
		{
			automaticVariableSynchronizationMutex.Unlock();
			return; // Cannot add to an existing list without a secondary identifier
		}

		for (unsigned i=0; i < automaticVariableSynchronizationList[uniqueIdentifier]->size(); i++)
		{
#ifdef _DEBUG
			assert ((*(automaticVariableSynchronizationList[uniqueIdentifier]))[i].secondaryID != secondaryUniqueIdentifier);
#endif
			if ((*(automaticVariableSynchronizationList[uniqueIdentifier]))[i].secondaryID == secondaryUniqueIdentifier)
			{
				automaticVariableSynchronizationMutex.Unlock();
				return; // Already used
			}
		}
	}
	automaticVariableSynchronizationMutex.Unlock();

	MemoryBlock newBlock;
	newBlock.original=memoryBlock;
	if (isAuthority)
	{
		newBlock.copy = new char[size];
		#ifdef _DEBUG
		assert(sizeof(char)==1);
		#endif
		memset(newBlock.copy, 0, size);
	}
	else
		newBlock.copy = 0; // no need to keep a copy if we are only receiving changes
	newBlock.size=size;
	newBlock.secondaryID=secondaryUniqueIdentifier;
	newBlock.isAuthority=isAuthority;
	newBlock.synchronizationRules=synchronizationRules;

	automaticVariableSynchronizationMutex.Lock();
	automaticVariableSynchronizationList[uniqueIdentifier]->insert(newBlock);
	automaticVariableSynchronizationMutex.Unlock();
}

BitStream * Network::UpdateSynchronizedMemory(void)
{
	// Prepare to write out a bitstream containing all the synchronization data
	BitStream *bitStream=0;

	automaticVariableSynchronizationMutex.Lock();

	for (UniqueIDType i=0; i < automaticVariableSynchronizationList.size(); i++)
	{
		if (automaticVariableSynchronizationList[i])
		{
			#ifdef _DEBUG
			assert(automaticVariableSynchronizationList[i]->size()>0);
			#endif
			for (unsigned j=0; j < automaticVariableSynchronizationList[i]->size(); j++)
			{
				// Just copy the data to memoryBlock so it's easier to access
				MemoryBlock memoryBlock = (*(automaticVariableSynchronizationList[i]))[j];
				automaticVariableSynchronizationMutex.Unlock();

				if (memoryBlock.isAuthority) // If this is not the authoritative block then ignore it
				{
					bool doSynchronization;
					if (memoryBlock.synchronizationRules) // If the user defined synchronization rules then use them
						doSynchronization=memoryBlock.synchronizationRules(memoryBlock.original, memoryBlock.copy);
					else
						// If the user did not define synchronization rules then just synchronize them whenever the memory is different
						doSynchronization = (memcmp(memoryBlock.original, memoryBlock.copy, memoryBlock.size)!=0);

					if (doSynchronization)
					{
						if (bitStream==0)
						{
							bitStream=new BitStream(memoryBlock.size + 1 + 2 + 2);
							// Stream header, use WriteBits instead of Write so the BitStream class does not use the TYPE_CHECKING
							// define and add an extra identifier byte at the front of the stream.  This way
							// the first byte of the stream will correctly be ID_SYNCHRONIZE_MEMORY
							unsigned char ch=ID_SYNCHRONIZE_MEMORY;
							bitStream->WriteBits((unsigned char*)&ch, sizeof(unsigned char)*8, false);
						}
						bitStream->Write(i); // First write the unique ID
						// If there is a secondary ID, write 1 and then it.  Otherwise write 0
						if (memoryBlock.secondaryID!=UNASSIGNED_OBJECT_ID)
						{
							bitStream->Write(true);
							bitStream->WriteCompressed(memoryBlock.secondaryID);
						}
						else
						{
							bitStream->Write(false);
						}
						// Write the length of the memory block
						bitStream->WriteCompressed(memoryBlock.size);
						// Write the new memory block
						bitStream->Write(memoryBlock.original, memoryBlock.size);
						// Save the updated memory
						memcpy(memoryBlock.copy, memoryBlock.original, memoryBlock.size);
					}
				}

				automaticVariableSynchronizationMutex.Lock();
			}
		}
	}

	automaticVariableSynchronizationMutex.Unlock();

	return bitStream;
}

void Network::HandleUpdateSynchronizeMemory(char *data, int length)
{
	#ifdef _DEBUG
	assert(data[0]==ID_SYNCHRONIZE_MEMORY);
	assert(length > 2);
	#endif

	// Push the data into a bitstream for easy parsing
	BitStream bitStream(data+1, length-1, false);
	UniqueIDType uniqueID;
	bool hasSecondaryID;
	ObjectID secondaryID;
	unsigned short memoryBlockSize;
	char *externalMemoryBlock;

	while (bitStream.GetNumberOfUnreadBits()>0) // Just read until we can't read anymore
	{
		if (bitStream.Read(uniqueID)==false)
			break;
		if (bitStream.Read(hasSecondaryID)==false)
			break;
		if (hasSecondaryID)
		{
			if (bitStream.ReadCompressed(secondaryID)==false)
				break;
		}
		if (bitStream.ReadCompressed(memoryBlockSize)==false)
			break;

		automaticVariableSynchronizationMutex.Lock();
		if (uniqueID >= automaticVariableSynchronizationList.size() ||
			automaticVariableSynchronizationList[uniqueID]==0 ||
			(hasSecondaryID==false && automaticVariableSynchronizationList[uniqueID]->size()>1))
		{
			automaticVariableSynchronizationMutex.Unlock();
			return; // Unknown identifier
		}

		if (hasSecondaryID)
		{
			externalMemoryBlock=0;
			// One or more elements in this list uniquely identified.  Find it to get the outside data pointer
			for (unsigned i=0; i < automaticVariableSynchronizationList[uniqueID]->size(); i++)
			{
				if (	(*(automaticVariableSynchronizationList[uniqueID]))[i].secondaryID==secondaryID)
				{
					externalMemoryBlock=(*(automaticVariableSynchronizationList[uniqueID]))[i].original;
					break;
				}
			}
		}
		else
			// If no secondary identifier then the list only contains one element so the data we are looking for is at index 0
			externalMemoryBlock=(*(automaticVariableSynchronizationList[uniqueID]))[0].original;

		automaticVariableSynchronizationMutex.Unlock();

		if (externalMemoryBlock==0)
		{
			// Couldn't find the specified data
			bitStream.IgnoreBits(memoryBlockSize*8);
		}
		else
		{
			// Found the specified data, read the new data into it
			if (bitStream.Read(externalMemoryBlock, memoryBlockSize)==false)
				break;
		}
	}
}

void Network::GenerateCompressionLayer(unsigned long inputFrequencyTable[256], bool inputLayer)
{
	DeleteCompressionLayer(inputLayer);
	if (inputLayer)
	{
		inputTree = new HuffmanEncodingTree;
		inputTree->GenerateFromFrequencyTable(inputFrequencyTable);
	}
	else
	{
		outputTree = new HuffmanEncodingTree;
		outputTree->GenerateFromFrequencyTable(inputFrequencyTable);
	}
}

void Network::DeleteCompressionLayer(bool inputLayer)
{
	if (inputLayer)
	{
		if (inputTree)
		{
			delete inputTree;
			inputTree=0;
		}
	}
	else
	{
		if (outputTree)
		{
			delete outputTree;
			outputTree=0;
		}
	}	
}

void Network::GetSendFrequencyTable(unsigned long outputFrequencyTable[256])
{
	memcpy(outputFrequencyTable, frequencyTable, sizeof(unsigned long) * 256);
}


// Returns the compression and decompression ratios.  A low compression ratio is good while a high decompression ratio is good.
// Compression is for outgoing data while decompression is for incoming data
float Network::GetCompressionRatio(void) const
{
	if (rawBytesSent>0L)
	{
		return (float)compressedBytesSent / (float)rawBytesSent;
	}
	else return 0.0f;
}

float Network::GetDecompressionRatio(void) const
{
	if (rawBytesRecieved>0L)
	{
		return (float)compressedBytesRecieved / (float)rawBytesRecieved;
	}
	else return 0.0f;
}

void Network::RegisterAsRemoteProcedureCall(char* uniqueID, void (*functionName)(char *input, int numberOfBitsOfData, PlayerID sender))
{
	if (uniqueID==0 || uniqueID[0]==0 || functionName==0)
		return;

	#ifdef _DEBUG
	assert(strlen(uniqueID)<256);
	#endif

	char uppercaseUniqueID[256];
	int counter=0;
	while (uniqueID[counter])
	{
		uppercaseUniqueID[counter]=(char)toupper(uniqueID[counter]);
		counter++;
	}
	uppercaseUniqueID[counter]=0;

	// Each id must be unique
	#ifdef _DEBUG
	assert(rpcTree.is_in(RPCNode(uppercaseUniqueID, functionName))==false);
	#endif
	rpcTree.add(RPCNode(uppercaseUniqueID, functionName));
}

void Network::UnregisterAsRemoteProcedureCall(char* uniqueID)
{
	if (uniqueID==0 || uniqueID[0]==0)
		return;

	#ifdef _DEBUG
	assert(strlen(uniqueID)<256);
	#endif

	char uppercaseUniqueID[256];
	strcpy(uppercaseUniqueID,uniqueID);
	int counter=0;
	while (uniqueID[counter])
	{
		uppercaseUniqueID[counter]=(char)toupper(uniqueID[counter]);
		counter++;
	}
	uppercaseUniqueID[counter]=0;

	// Unique ID must exist
	#ifdef _DEBUG
	assert(rpcTree.is_in(RPCNode(uppercaseUniqueID, 0))==true);
	#endif
	rpcTree.del(RPCNode(uppercaseUniqueID, 0));
}

bool Network::HandleRPCPacket(char *data, int length, PlayerID playerId)
{
	// RPC BitStream format is
	// ID_RPC - unsigned char
	// Unique identifier string length - unsigned char
	// The unique ID  - string with each letter in upper case, subtracted by 'A' and written in 5 bits.
	// Number of bits of the data (long)
	// The data

	BitStream incomingBitStream(data, length, false);
	unsigned char uniqueIDLength, ch, packetID;
	char uniqueIdentifier[256];
	int counter;
	long bitLength;

	if (incomingBitStream.Read(packetID)==false)
	{
		#ifdef _DEBUG
		assert(0); // bitstream was not long enough.  Some kind of internal error
		#endif
		return false; 
	}

	if (packetID!=ID_RPC)
	{
		#ifdef _DEBUG
		assert(0); // This function shouldn't be called if the packet is not a remote procedure call
		#endif
		return false;
	}


	if (incomingBitStream.ReadCompressed(uniqueIDLength)==false)
	{
		#ifdef _DEBUG
		assert(0); // bitstream was not long enough.  Some kind of internal error
		#endif
		return false;
	}

	
	if (uniqueIDLength==256)
	{
		#ifdef _DEBUG
		assert(0);
		#endif
		return false;  // Some hacker tried an overflow
	}

	// Get the unique identifier out of the bits
	for (counter=0; counter < uniqueIDLength; counter++)
	{
		if (incomingBitStream.ReadBits((unsigned char*)&ch, 5)==false)
		{
			#ifdef _DEBUG
			assert(0); // bitstream was not long enough.  Some kind of internal error
			#endif
			return false;
		}

		ch+='A';
		uniqueIdentifier[counter]=ch;
	}

	uniqueIdentifier[counter]=0;

	if (incomingBitStream.ReadCompressed(bitLength)==false)
	{
		#ifdef _DEBUG
		assert(0); // bitstream was not long enough.  Some kind of internal error
		#endif
		return false;
	}

	RPCNode *node = rpcTree.get_pointer_to_node(RPCNode(uniqueIdentifier, 0));
	if (node==0)
	{
		#ifdef _DEBUG
		assert(0);
		#endif
		return false; // This function was not registered!
	}

	// Call the function
	if (bitLength==0)
		node->functionName(0, bitLength, playerId);
	else
	{
		if (incomingBitStream.GetNumberOfUnreadBits() == 0)
		{
			#ifdef _DEBUG
			assert(0);
			#endif
			return false; // No data was appended!
		}

		// We have to copy into a new data chunk because the user data might not be byte aligned.
		char *userData = new char[BITS_TO_BYTES(incomingBitStream.GetNumberOfUnreadBits())];

		// The false means read out the internal representation of the bitstream data rather than
		// Aligning it as we normally would with user data.  This is so the end user can cast the data received
		// into a bitstream for reading
		if (incomingBitStream.ReadBits((unsigned char*)userData, bitLength, false)==false)
		{
				#ifdef _DEBUG
				assert(0);
				#endif
				delete [] userData;
				return false; // Not enough data to read
		}

		// Call the function callback
		node->functionName(userData, bitLength, playerId);
		// Free the memory
		delete [] userData;
	}

	return true;
}

void Network::DecompressInput(BitStream *bitStream)
{
	unsigned numberOfBytesUsed = bitStream->GetNumberOfBytesUsed();
	unsigned numberOfBitsUsed = bitStream->GetNumberOfBitsUsed();
	rawBytesRecieved+=numberOfBytesUsed;
	// Decompress the input data.
	if (inputTree)
	{
		#ifdef _DEBUG
		assert(numberOfBitsUsed>0);
		#endif
		unsigned char *dataCopy = new unsigned char[numberOfBytesUsed];
		memcpy(dataCopy, bitStream->GetData(), numberOfBytesUsed);
		bitStream->Reset();
		inputTree->DecodeArray(dataCopy,numberOfBitsUsed, bitStream);
		compressedBytesRecieved+=bitStream->GetNumberOfBytesUsed();
		delete [] dataCopy;
	}
}

void Network::UpdateOutgoingFrequencyTable(BitStream* bitStream)
{
	if (trackFrequencyTable==false)
		return;

	unsigned numberOfBytesUsed;
	numberOfBytesUsed = bitStream->GetNumberOfBytesUsed();
	// Store output frequency
	for (unsigned i=0; i <numberOfBytesUsed; i++)
	{
		frequencyTable[bitStream->GetData()[i]]++;
	}

	rawBytesSent+=numberOfBytesUsed;
}