// This file is part of RakNet Copyright 2003, 2004 Rakkarsoft LLC and Kevin Jenkins.

// Usage of Raknet is subject to the appropriate licence agreement.
// "Shareware" Licensees with Rakkarsoft LLC are subject to the shareware license found at http://www.rakkarsoft.com/shareWareLicense.html which you agreed to upon purchase of a "Shareware license"
// "Commercial" Licensees with Rakkarsoft LLC are subject to the commercial license found at http://www.rakkarsoft.com/sourceCodeLicense.html which you agreed to upon purchase of a "Commercial license"
// All other users are subject to the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

// Refer to the appropriate license agreement for distribution, modification, and warranty rights.

#include "RakPeer.h"

#ifdef __USE_IO_COMPLETION_PORTS
#include "AsynchronousFileIO.h"
#endif

#ifdef _WIN32
//#include <Shlwapi.h>
#include <process.h>
#else
#define closesocket close
#include <unistd.h>
#include <pthread.h>
#endif
#include <ctype.h> // toupper

#include "GetTime.h"
#include "PacketEnumerations.h"
#include "HuffmanEncodingTree.h"
#include "PacketPool.h"
#include "Rand.h"

// alloca
#ifdef _WIN32
#include <malloc.h>
#else
#include <stdlib.h>
#endif

static const unsigned long SYN_COOKIE_OLD_RANDOM_NUMBER_DURATION=5000;


// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Constructor
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
RakPeer::RakPeer()
{
	usingSecurity=false;
	memset(frequencyTable, 0, sizeof(unsigned long) * 256);
	rawBytesSent=rawBytesReceived=compressedBytesSent=compressedBytesReceived=0;
	outputTree=inputTree=0;
	connectionSocket=INVALID_SOCKET;
	MTUSize=DEFAULT_MTU_SIZE;
	trackFrequencyTable=false;
	maximumIncomingConnections=0;
	maximumNumberOfPeers=0;
	remoteSystemList=0;
	bytesSentPerSecond=bytesReceivedPerSecond=0;
	endThreads=true;
	isMainLoopThreadActive=false;
//	isRecvfromThreadActive=false;
	occasionalPing=false;
	connectionSocket=INVALID_SOCKET;
	myPlayerId=UNASSIGNED_PLAYER_ID;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Destructor
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
RakPeer::~RakPeer()
{
	unsigned i;

	Disconnect(0L);

	// Clear out the lists:
	for (i=0; i < requestedConnectionsList.size(); i++)
		delete requestedConnectionsList[i];
	requestedConnectionsList.clear();

	// Free the ban list.
	ClearBanList();
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Starts the network threads, opens the listen port
// You must call this before calling SetMaximumIncomingConnections or Connect
// Multiple calls while already active are ignored.  To call this function again with different settings, you must first call Disconnect()
//
// Parameters:
// MaximumNumberOfPeers:  Required so the network can preallocate and for thread safety.
// - A pure client would set this to 1.  A pure server would set it to the number of allowed clients.
// - A hybrid would set it to the sum of both types of connections
// localPort: The port to listen for connections on.
// _threadSleepTimer: <0 for don't sleep, >=0 for how many ms to Sleep each internal update cycle (recommended you start with 15, try different values)
//
// Returns:
// False on failure (can't create socket or thread), true on success.
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool RakPeer::Initialize(unsigned short MaximumNumberOfPeers, unsigned short localPort,int _threadSleepTimer)
{
	unsigned i;

	assert(MaximumNumberOfPeers>0);
	if (MaximumNumberOfPeers<=0)
		return false;

	if (connectionSocket==INVALID_SOCKET)
	{
		connectionSocket = SocketLayer::Instance()->CreateBoundSocket(localPort, true);
		if (connectionSocket==INVALID_SOCKET)
			return false;
	}

	if (maximumNumberOfPeers==0)
	{
		rakPeerMutexes[RakPeer::remoteSystemList_Mutex].Lock();
		remoteSystemList = new RemoteSystemStruct[MaximumNumberOfPeers];
		for (i=0; i < MaximumNumberOfPeers; i++)
		{
			remoteSystemList[i].playerId=UNASSIGNED_PLAYER_ID;
		}
		rakPeerMutexes[RakPeer::remoteSystemList_Mutex].Unlock();

		// Don't allow more incoming connections than we have peers.
		if (maximumIncomingConnections>MaximumNumberOfPeers)
			maximumIncomingConnections=MaximumNumberOfPeers;

		maximumNumberOfPeers=MaximumNumberOfPeers;
	}

	// For histogram statistics
//	nextReadBytesTime=0;
//	lastSentBytes=lastReceivedBytes=0;

	if (endThreads)
	{
		// Reset the frequency table that we use to save outgoing data
		memset(frequencyTable, 0, sizeof(unsigned long) * 256);

		// Reset the statistical data

		rawBytesSent=rawBytesReceived=compressedBytesSent=compressedBytesReceived=0;

		endThreads=false;
		// Create the threads
		threadSleepTimer=_threadSleepTimer;

		char ipList[10][16];
		SocketLayer::Instance()->GetMyIP(ipList);
		myPlayerId.port=localPort;
		myPlayerId.binaryAddress=inet_addr(ipList[0]);

#ifdef _WIN32
		if (isMainLoopThreadActive==false)
		{
			unsigned ProcessPacketsThreadID=0;
			processPacketsThreadHandle=(HANDLE)_beginthreadex(NULL, 0, UpdateNetworkLoop, this, 0, &ProcessPacketsThreadID);
//			if (threadSleepTimer==2 && processPacketsThreadHandle)
//				SetThreadPriority(processPacketsThreadHandle, THREAD_PRIORITY_HIGHEST);
//			else
				if (processPacketsThreadHandle==0)
			{
				Disconnect(0L);
				return false;
			}

			CloseHandle(processPacketsThreadHandle);
			processPacketsThreadHandle=0;

		}

		/*
		if (isRecvfromThreadActive==false)
		{
			unsigned recvfromThreadID=0;
			recvfromThreadHandle=(HANDLE)_beginthreadex(NULL, 0, RecvFromNetworkLoop, this, 0, &recvfromThreadID);

#ifndef __USE_IO_COMPLETION_PORTS
			if (threadSleepTimer==2 && recvfromThreadHandle)
				SetThreadPriority(recvfromThreadHandle, THREAD_PRIORITY_HIGHEST);
#endif

			if (recvfromThreadHandle==0)
			{
				Disconnect();
				return false;
			}

			CloseHandle(recvfromThreadHandle);
			recvfromThreadHandle=0;
		}
		*/

#else
		pthread_attr_t attr;

		pthread_attr_init( &attr );
		pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_DETACHED );
	//	if (threadSleepTimer==2)
	//	{
	//		sched_param sp;
	//		sp.sched_priority = sched_get_priority_max(SCHED_OTHER);
	//		pthread_attr_setschedparam(&attr, &sp);
	//	}

		int error;

		if (isMainLoopThreadActive==false)
		{
			error = pthread_create( &processPacketsThreadHandle, &attr, &UpdateNetworkLoop, this );
			if (error)
			{
				Disconnect(0L);
				return false;
			}
		}
/*
		if (	isRecvfromThreadActive==false)
		{
			error = pthread_create( &recvfromThreadHandle, &attr, &RecvFromNetworkLoop, this );
			if (error)
			{
				Disconnect();
				return false;
			}
		}
		pthread_attr_destroy( &attr );
*/
		processPacketsThreadHandle=0;
#endif


		// Wait for the threads to activate.  When they are active they will set these variables to true
		while (/*isRecvfromThreadActive==false || */isMainLoopThreadActive==false)
#ifdef _WIN32
			Sleep(10);
#else
			usleep(10 * 1000);
#endif

	}

	return true;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Must be called while offline
// Secures connections though a combination of SHA1, AES128, SYN Cookies, and RSA to prevent
// connection spoofing, replay attacks, data eavesdropping, packet tampering, and MitM attacks.
// There is a significant amount of processing and a slight amount of bandwidth
// overhead for this feature.
//
// If you accept connections, you must call this or else secure connections will not be enabled
// for incoming connections.
// If you are connecting to another system, you can call this with values for the
// (e and p,q) public keys before connecting to prevent MitM
//
// Parameters:
// RSAe, RSAn - Public keys generated from the RSACrypt class.  See the Encryption sample
// RSAp, RSAq - A pointer to the private keys from the RSACrypt class.  See the Encryption sample
// If the private keys are 0, then a new key will be generated when this function is called
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::InitializeSecurity(char *RSAe, char *RSAn, char *RSAp, char *RSAq)
{
	if (endThreads==false)
		return;

	// Setting the client key is e,n,
	// Setting the server key is p,q
	// These are mutually exclusive
	if ((RSAe && RSAn && (RSAp || RSAq)) ||
		(RSAp && RSAq && (RSAe || RSAn)) ||
		(RSAe && RSAn==0) ||
		(RSAn && RSAe==0) ||
		(RSAp && RSAq==0) ||
		(RSAq && RSAp==0))
	{
		// Invalid parameters
		assert(0);
	}

	seedMT(RakNetGetTime());

	GenerateSYNCookieRandomNumber();

	usingSecurity=true;

	if (RSAe==0 && RSAn==0 &&RSAp==0 && RSAq==0)
	{
		keysLocallyGenerated=true;
		rsacrypt.generateKeys();
	}
	else
	{
		if (RSAe && RSAn)
		{
			// Save public keys
			memcpy((char*)&publicKeyE, RSAe, sizeof(publicKeyE));
			memcpy(publicKeyN, RSAn, sizeof(publicKeyN));
		}
		else if (RSAp && RSAq)
		{
			BIGHALFSIZE(RSA_BIT_SIZE, p);
			BIGHALFSIZE(RSA_BIT_SIZE, q);
			memcpy(p, RSAp, sizeof(p));
			memcpy(q, RSAq, sizeof(q));
			// Save private keys
			rsacrypt.setPrivateKey(p, q);
		}
		keysLocallyGenerated=false;
	}
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description
// Must be called while offline
// Disables all security.
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::DisableSecurity(void)
{
	if (endThreads==false)
		return;

	usingSecurity=false;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Sets how many incoming connections are allowed.  If this is less than the number of players currently connected, no
// more players will be allowed to connect.  If this is greater than the maximum number of peers allowed, it will be reduced
// to the maximum number of peers allowed.
//
// Parameters:
// numberAllowed - Maximum number of incoming connections allowed.
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::SetMaximumIncomingConnections(unsigned short numberAllowed)
{
	maximumIncomingConnections=numberAllowed;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Returns the maximum number of incoming connections, which is always <= MaximumNumberOfPeers
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
unsigned short RakPeer::GetMaximumIncomingConnections(void) const
{
	return maximumIncomingConnections;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Sets the password incoming connections must match in the call to Connect (defaults to none)
// Pass 0 to passwordData to specify no password
//
// Parameters:
// passwordData: A data block that incoming connections must match.  This can be just a password, or can be a stream of data.
// - Specify 0 for no password data
// passwordDataLength: The length in bytes of passwordData
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::SetIncomingPassword(char* passwordData, int passwordDataLength)
{
	// Set the incoming password data
	rakPeerMutexes[incomingPasswordBitStream_Mutex].Lock();
	incomingPasswordBitStream.Reset();
	if (passwordData && passwordDataLength>0)
		incomingPasswordBitStream.Write(passwordData, passwordDataLength);
	rakPeerMutexes[incomingPasswordBitStream_Mutex].Unlock();
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Returns the password set by SetIncomingPassword in a BitStream
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
RakNet::BitStream *RakPeer::GetIncomingPassword(void)
{
	return &incomingPasswordBitStream;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Call this to connect to the specified host (ip or domain name) and server port.
// Calling Connect and not calling SetMaximumIncomingConnections acts as a dedicated client.  Calling both acts as a true peer.
// This is a non-blocking connection.  You know the connection is successful when IsConnected() returns true
// or receive gets a packet with the type identifier ID_CONNECTION_ACCEPTED.  If the connection is not
// successful, such as rejected connection or no response then neither of these things will happen.
// Requires that you first call Initialize
//
// Parameters:
// host: Either a dotted IP address or a domain name
// remotePort: Which port to connect to on the remote machine.
// passwordData: A data block that must match the data block on the server.  This can be just a password, or can be a stream of data
// passwordDataLength: The length in bytes of passwordData
//
// Returns:
// True on successful initiation. False on incorrect parameters, internal error, or too many existing peers
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool RakPeer::Connect(char* host, unsigned short remotePort, char* passwordData, int passwordDataLength)
{
	// If endThreads is true here you didn't call Initialize() first.
	if (host==0 || connectionSocket==INVALID_SOCKET || endThreads)
		return false;

	unsigned i,numberOfFreeSlots;
	numberOfFreeSlots=0;
	for (i=0; i < maximumNumberOfPeers; i++)
	{
		if (remoteSystemList[i].playerId==UNASSIGNED_PLAYER_ID)
			numberOfFreeSlots++;
	}

	if (numberOfFreeSlots==0)
		return false;

	// Set the incoming password data
	rakPeerMutexes[outgoingPasswordBitStream_Mutex].Lock();
	outgoingPasswordBitStream.Reset();
	if (passwordData && passwordDataLength>0)
		outgoingPasswordBitStream.Write(passwordData, passwordDataLength);
	rakPeerMutexes[outgoingPasswordBitStream_Mutex].Unlock();

	// If the host starts with something other than 0, 1, or 2 it's (probably) a domain name.
	if (host[0] < '0' || host[0] > '2')
	{
		host = (char*) SocketLayer::Instance()->DomainNameToIP(host);
	}

	// Connecting to ourselves in the same instance of the program?
	if ((strcmp(host, "127.0.0.1")==0 || strcmp(host, "0.0.0.0")==0)&& remotePort==myPlayerId.port)
	{
		// Feedback loop.
		 if (GetNumberOfIncomingConnections()+1 > maximumIncomingConnections)
		 {
			 // Tell the game that this person has connected
			 Packet *p;
			 p = PacketPool::Instance()->GetPointer();

			 p->data = new unsigned char [1];
			 p->data[0]=(unsigned char)ID_NO_FREE_INCOMING_CONNECTIONS;
			 p->playerId=myPlayerId;
			 p->length = 1;

			#ifdef _DEBUG
			 assert(p->data);
			#endif
			 incomingQueueMutex.Lock();
			 incomingPacketQueue.push(p);
			 incomingQueueMutex.Unlock();
		 }
		 else
		 {
			 // Just assume we are connected.  This is really just for testing.
			 RemoteSystemStruct* remoteSystem=AssignPlayerIDToRemoteSystemList(myPlayerId,0, false);

			 if (remoteSystem!=0)
			 {
				 ResetRemoteSystemData(remoteSystem, true);

				 /*
				 // Send the connection request complete to the game
				 Packet *packet = PacketPool::Instance()->GetPointer();
				 packet->data = new char[1];
				 packet->data[0]=ID_NEW_INCOMING_CONNECTION;
				 packet->length=sizeof(char);
				 packet->bitSize=sizeof(char)*8;
				 packet->playerId=myPlayerId;
				 incomingQueueMutex.Lock();
				 incomingPacketQueue.push(packet);
				 incomingQueueMutex.Unlock();
				 */

				 // Tell the remote system via the reliability layer that we connected
				 NewIncomingConnectionStruct newIncomingConnectionStruct;
				 newIncomingConnectionStruct.typeId=ID_NEW_INCOMING_CONNECTION;
				 newIncomingConnectionStruct.externalID=myPlayerId;
				 Send((char*)&newIncomingConnectionStruct, sizeof(newIncomingConnectionStruct), SYSTEM_PRIORITY, RELIABLE, 0, myPlayerId, false);

				 return true;
			 }
			 else
				 return false;
		 }
	}

	RecordConnectionAttempt(host, remotePort);
    
	return SendConnectionRequest(host, remotePort);
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Stops the network threads and close all connections.  Multiple calls are ok.
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::Disconnect(unsigned long blockDuration)
{
	unsigned i;
	unsigned short maxPeers=maximumNumberOfPeers; // This is done for threading reasons

	// Call close connection in a loop for all open connections.
	for (i=0; i < maxPeers; i++)
	{
		// CloseConnection uses maximumNumberOfPeers
		CloseConnection(remoteSystemList[i].playerId, true, blockDuration);
	}

	// Setting this to 0 allows remoteSystemList to be reallocated in Initialize and prevents threads from accessing the reliability layer
	maximumNumberOfPeers=0;

	// Stop the threads
	endThreads=true;

	while(isMainLoopThreadActive)
#ifdef _WIN32
		Sleep(10);
#else
		usleep(10 * 1000);
#endif

	if (connectionSocket != INVALID_SOCKET)
	{
		closesocket(connectionSocket);
		connectionSocket = INVALID_SOCKET;
	}

	// Write to ourselves to unblock if necessary
//	if (isSocketLayerBlocking==true)
//	{
//		char c=255;
//		SocketLayer::Instance()->SendTo(connectionSocket, &c, 1, "127.0.0.1", myPlayerId.port);
//	}

//	while(isRecvfromThreadActive)
//#ifdef _WIN32
//		Sleep(10);
//#else
//		usleep(10 * 1000);
//#endif

	isSocketLayerBlocking=false;

//	if (connectionSocket != INVALID_SOCKET)
//	{
//		closesocket(connectionSocket);
//		connectionSocket = INVALID_SOCKET;
//	}

	// Clear out the queues
	while (incomingPacketQueue.size())
		PacketPool::Instance()->ReleasePointer(incomingPacketQueue.pop());

	/*
	synchronizedMemoryQueueMutex.Lock();
	while (synchronizedMemoryPacketQueue.size())
		PacketPool::Instance()->ReleasePointer(synchronizedMemoryPacketQueue.pop());
	synchronizedMemoryQueueMutex.Unlock();
	*/

	bytesSentPerSecond=bytesReceivedPerSecond=0;


	// Clear out the reliabilty layer list in case we want to reallocate it in a successive call to Init.
	RemoteSystemStruct * temp = remoteSystemList;
	remoteSystemList=0;
	delete [] temp;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Returns true if the network threads are running
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool RakPeer::IsActive(void) const
{
	return endThreads==false;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Fills the array remoteSystems with the playerID of all the systems we are connected to
//
// Parameters:
// remoteSystems (out): An array of PlayerID structures to be filled with the PlayerIDs of the systems we are connected to
// - pass 0 to remoteSystems to only get the number of systems we are connected to
// numberOfSystems (int, out): As input, the size of remoteSystems array.  As output, the number of elements put into the array 
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool RakPeer::GetConnectionList(PlayerID *remoteSystems, unsigned short *numberOfSystems) const
{
	int count, index;

	if (remoteSystemList==0 || endThreads==true)
	{
		*numberOfSystems=0;
		return false;
	}

	for (count=0, index=0; (remoteSystems==0 || count < *numberOfSystems) && index < maximumNumberOfPeers; ++index)
		if (remoteSystemList[index].playerId!=UNASSIGNED_PLAYER_ID)
		{
			if (remoteSystems)
				remoteSystems[count]=remoteSystemList[index].playerId;
			++count;
		}

	*numberOfSystems=(unsigned short)count;

	return 0;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Sends a block of data to the specified system that you are connected to.
// This function only works while the client is connected (Use the Connect function).
//
// Parameters:
// data: The block of data to send
// length: The size in bytes of the data to send
// bitStream: The bitstream to send
// priority: What priority level to send on.
// reliability: How reliability to send this data
// orderingStream: When using ordered or sequenced packets, what stream to order these on.
// - Packets are only ordered relative to other packets on the same stream
// playerId: Who to send this packet to, or in the case of broadcasting who not to send it to.  Use UNASSIGNED_PLAYER_ID to specify none
// broadcast: True to send this packet to all connected systems.  If true, then playerId specifies who not to send the packet to.
// Returns:
// False if we are not connected to the specified recipient.  True otherwise
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool RakPeer::Send(char *data, const long length, PacketPriority priority, PacketReliability reliability, char orderingStream, PlayerID playerId, bool broadcast)
{
#ifdef _DEBUG
	assert(data && length>0);
#endif
	if (data==0 || length < 0)
		return false;

	RakNet::BitStream temp(data, length, false);
	return Send(&temp, priority, reliability, orderingStream, playerId, broadcast);

}

bool RakPeer::Send(RakNet::BitStream * bitStream, PacketPriority priority, PacketReliability reliability, char orderingStream, PlayerID playerId, bool broadcast)
{
#ifdef _DEBUG
	assert(bitStream->GetNumberOfBytesUsed()>0);
#endif
	if (bitStream->GetNumberOfBytesUsed()==0)
		return false;
	if (remoteSystemList==0 || endThreads==true)
		return false;
	if (broadcast==false && playerId==UNASSIGNED_PLAYER_ID)
		return false;

	unsigned remoteSystemIndex;

	for (remoteSystemIndex=0; remoteSystemIndex < maximumNumberOfPeers; remoteSystemIndex++)
		if (remoteSystemList[remoteSystemIndex].playerId != UNASSIGNED_PLAYER_ID &&
			((broadcast==false && remoteSystemList[remoteSystemIndex].playerId==playerId) ||
			(broadcast==true && remoteSystemList[remoteSystemIndex].playerId!=playerId))
			)
		{

			if (trackFrequencyTable)
			{
				unsigned numberOfBytesUsed = bitStream->GetNumberOfBytesUsed();

				// Store output frequency
				for (unsigned i=0; i < numberOfBytesUsed; i++)
				{
					frequencyTable[bitStream->GetData()[i]]++;
				}

				rawBytesSent+=numberOfBytesUsed;
			}

			if (outputTree)
			{
				RakNet::BitStream bitStreamCopy(bitStream->GetNumberOfBytesUsed());
				outputTree->EncodeArray(bitStream->GetData(),bitStream->GetNumberOfBytesUsed(), &bitStreamCopy);
				compressedBytesSent+=bitStreamCopy.GetNumberOfBytesUsed();
				remoteSystemList[remoteSystemIndex].reliabilityLayer.Send(&bitStreamCopy, priority,reliability,orderingStream, true, MTUSize);
			}
			else
				remoteSystemList[remoteSystemIndex].reliabilityLayer.Send(bitStream, priority,reliability,orderingStream, true, MTUSize);

			if (broadcast==false)
				return true;
		}

	return true;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Gets a packet from the incoming packet queue. Use DeallocatePacket to deallocate the packet after you are done with it.
// Check the Packet struct at the top of CoreNetworkStructures.h for the format of the struct
//
// Returns:
// 0 if no packets are waiting to be handled, otherwise an allocated packet
// If the client is not active this will also return 0, as all waiting packets are flushed when the client is Disconnected
// This also updates all memory blocks associated with synchronized memory and distributed objects
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Packet* RakPeer::Receive(void)
{
	if (!(IsActive()))	return 0;

	// Prepare to write out a bitstream containing all the synchronization data
//	RakNet::BitStream *bitStream=0;
/*
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

	if (bitStream)
	{
		// Send out this data
		Send(bitStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_PLAYER_ID, true, false);
		delete bitStream;
	}

	synchronizedMemoryQueueMutex.Lock();
	while (synchronizedMemoryPacketQueue.size())
	{
		Packet *pack = synchronizedMemoryPacketQueue.pop();
#ifdef _DEBUG
		assert(data[0]==ID_SYNCHRONIZE_MEMORY);
		assert(length > 2);
#endif

		// Push the data into a bitstream for easy parsing
		RakNet::BitStream bitStream(data+1, length-1, false);
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
		PacketPool::Instance()->ReleasePointer(pack);
	}
	synchronizedMemoryQueueMutex.Unlock();
*/

	Packet *val;
	int offset;

	incomingQueueMutex.Lock();
	if (incomingPacketQueue.size() > 0)
	{
		val = incomingPacketQueue.pop();
	}
	else
	{
		incomingQueueMutex.Unlock();
		return 0;
	}

	incomingQueueMutex.Unlock();

#ifdef _DEBUG
	assert(val->data);
#endif

	if ( (val->length >= sizeof(unsigned char) + sizeof(long)) && 
		((unsigned char)val->data[0] == ID_TIMESTAMP))
	{
		offset = sizeof(unsigned char);
		ShiftIncomingTimestamp((char*)val->data + offset, val->playerId);
	}

	return val;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Call this to deallocate a packet returned by Receive when you are done handling it.
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::DeallocatePacket(Packet *packet)
{
	if (packet==0)
		return;

	PacketPool::Instance()->ReleasePointer(packet);
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Return the total number of connections we are allowed
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
unsigned short RakPeer::GetMaximumNumberOfPeers(void) const
{
	return maximumNumberOfPeers;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Register a C function as available for calling as a remote procedure call
//
// Parameters:
// uniqueID: A null terminated non-case senstive string of only letters to identify this procedure
// functionName(...): The name of the C function or C++ singleton to be used as a function pointer
// This can be called whether the client is active or not, and registered functions stay registered unless unregistered with
// UnregisterAsRemoteProcedureCall
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::RegisterAsRemoteProcedureCall(char* uniqueID, void (*functionName)(char *input, int numberOfBitsOfData, PlayerID sender))
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

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Unregisters a C function as available for calling as a remote procedure call that was formerly registered
// with RegisterAsRemoteProcedureCall
//
// Parameters:
// uniqueID: A null terminated non-case senstive string of only letters to identify this procedure.  Must match the parameter
// passed to RegisterAsRemoteProcedureCall
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::UnregisterAsRemoteProcedureCall(char* uniqueID)
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

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Calls a C function on the server that the server already registered using RegisterAsRemoteProcedureCall
// If you want that function to return data you should call RPC from that system in the same way
// Returns true on a successful packet send (this does not indicate the recipient performed the call), false on failure
//
// Parameters:
// uniqueID: A null terminated non-case senstive string of only letters to identify this procedure.  Must match the parameter
// data: The block of data to send
// length: The size in BITS of the data to send
// bitStream: The bitstream to send
// priority: What priority level to send on.
// reliability: How reliability to send this data
// orderingStream: When using ordered or sequenced packets, what stream to order these on.
// broadcast - Send this packet to everyone.
// playerId: Who to send this packet to, or in the case of broadcasting who not to send it to.  Use UNASSIGNED_PLAYER_ID to specify none
// broadcast: True to send this packet to all connected systems.  If true, then playerId specifies who not to send the packet to.
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool RakPeer::RPC(char* uniqueID, char *data, long bitLength, PacketPriority priority, PacketReliability reliability, char orderingStream, PlayerID playerId, bool broadcast, bool shiftTimestamp)
{
#ifdef _DEBUG
	assert(uniqueID && uniqueID[0]);
#endif
	if (strlen(uniqueID) > 256)
	{
#ifdef _DEBUG
		assert(0);
#endif
		return false; // Unique ID is too long
	}

	if (shiftTimestamp && (data==0 || bitLength < 8 * sizeof(unsigned long)))
	{
		assert(0); // Not enough bits to shift!
		return false;
	}

	RakNet::BitStream outgoingBitStream;
	unsigned char uniqueIDLength, ch;
	uniqueIDLength = (unsigned char)strlen(uniqueID);

	// First write the ID, then write the size of the unique ID in characters, then the unique ID, then write the length of the data in bits, then write the data
	if (shiftTimestamp)
		outgoingBitStream.Write((unsigned char)ID_RPC_WITH_TIMESTAMP);
	else
		outgoingBitStream.Write((unsigned char)ID_RPC);
	outgoingBitStream.WriteCompressed(uniqueIDLength);
	for (int counter=0; uniqueID[counter]; counter++)
	{
		ch = (unsigned char)toupper(uniqueID[counter]);
// Dev-C++ doesn't support toupper.  How lame.
    //  if (uniqueID[counter] > 'Z')
  //      uniqueID[counter]-='a'-'A';

		if (ch < 'A' || ch > 'Z')
		{
#ifdef _DEBUG
			assert(0);
#endif
			return false; // Only letters allowed
		}

		// Make the range of the char from 0 to 32
		ch-='A';
		outgoingBitStream.WriteBits((unsigned char*)&ch, 5, true); // Write the char with 5 bits
	}

	if (data==0)
		bitLength=0;

	outgoingBitStream.WriteCompressed(bitLength);

	// False to write the raw data from another bitstream, rather than shifting from user data
	if (bitLength > 0)
		outgoingBitStream.WriteBits((unsigned char*)data, bitLength);

	return Send(&outgoingBitStream, priority, reliability, orderingStream,playerId,broadcast);
}

bool RakPeer::RPC(char* uniqueID, RakNet::BitStream *bitStream, PacketPriority priority, PacketReliability reliability, char orderingStream, PlayerID playerId, bool broadcast, bool shiftTimestamp)
{
	if (bitStream && bitStream->GetNumberOfBitsUsed()>0)
		return RPC(uniqueID, (char*)bitStream->GetData(), bitStream->GetNumberOfBitsUsed(), priority, reliability, orderingStream, playerId, broadcast,shiftTimestamp);
	else
		return RPC(uniqueID, 0,0, priority, reliability, orderingStream, playerId, broadcast,shiftTimestamp);
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Close the connection to another host (if we initiated the connection it will disconnect, if they did it will kick them out).
//
// Parameters:
// target: Which connection to close
// sendDisconnectionNotification: True to send ID_DISCONNECTION_NOTIFICATION to the recipient.  False to close it silently.
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::CloseConnection(PlayerID target, bool sendDisconnectionNotification, unsigned long blockDuration)
{
	unsigned i, stopWaitingTime;
	if (remoteSystemList==0 || endThreads==true)
		return;

	if (sendDisconnectionNotification)
	{
		unsigned char c=ID_DISCONNECTION_NOTIFICATION;
		Send((char*)&c, sizeof(c), SYSTEM_PRIORITY, RELIABLE, 0, target, false);
	}

	i=0;
	rakPeerMutexes[RakPeer::remoteSystemList_Mutex].Lock();
	for (; i < maximumNumberOfPeers; i++)
		if (remoteSystemList[i].playerId==target)
		{
			// Send out any last packets
			// Update isn't thread safe to call outside of the internal thread
			// remoteSystemList[i].reliabilityLayer.Update(connectionSocket, remoteSystemList[i].playerId, MTUSize);

			if (blockDuration>=0)
			{
				stopWaitingTime=RakNetGetTime() + blockDuration;

				while (RakNetGetTime() < stopWaitingTime)
				{
					// If this system is out of packets to send, then stop waiting
					if (remoteSystemList[i].reliabilityLayer.GetStatistics()->messageSendBuffer[SYSTEM_PRIORITY]==0)
						break;

					// This will probably cause the update thread to run which will probably
					// send the disconnection notification
#ifdef _WIN32
					Sleep(30);
#else
					usleep(30 * 1000);
#endif
				}
			}

			// Reserve this reliability layer for ourselves
			remoteSystemList[i].playerId=UNASSIGNED_PLAYER_ID; // This one line causes future incoming packets to go through the reliability layer

			// Remove any remaining packets
			remoteSystemList[i].reliabilityLayer.Reset();
			break;
		}

		rakPeerMutexes[remoteSystemList_Mutex].Unlock();
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Given a playerID, returns an index from 0 to the maximum number of players allowed - 1.
//
// Parameters
// playerId - The playerID to search for
//
// Returns
// An integer from 0 to the maximum number of peers -1, or -1 if that player is not found
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
int RakPeer::GetIndexFromPlayerID(PlayerID playerId)
{
	unsigned i;

	if (playerId==UNASSIGNED_PLAYER_ID)
		return -1;

	for (i=0; i < maximumNumberOfPeers; i++)
		if (remoteSystemList[i].playerId==playerId)
			return i;

	return -1;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// This function is only useful for looping through all players.
//
// Parameters
// index - an integer between 0 and the maximum number of players allowed - 1.
//
// Returns
// A valid playerID or UNASSIGNED_PLAYER_ID if no such player at that index
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
PlayerID RakPeer::GetPlayerIDFromIndex(int index)
{
	if (index >=0 && index < maximumNumberOfPeers)
		return remoteSystemList[index].playerId;

	return UNASSIGNED_PLAYER_ID;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Bans an IP from connecting.  Banned IPs persist between connections.
//
// Parameters
// IP - Dotted IP address.  Can use * as a wildcard, such as 128.0.0.* will ban
// All IP addresses starting with 128.0.0
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::AddToBanList(const char *IP)
{
	unsigned index;
	char *IPCopy;

	if (IP==0 || IP[0]==0 || strlen(IP)>15)
		return;

	// If this guy is already in the ban list, do nothing
	index=0;

	banListMutex.Lock();
	for (; index < banList.size(); index++)
	{
		if (strcmp(IP, banList[index])==0)
		{
			banListMutex.Unlock();
			return;
		}
	}
	banListMutex.Unlock();

	IPCopy = new char [16];
	strcpy(IPCopy, IP);
	banListMutex.Lock();
	banList.insert(IPCopy);
	banListMutex.Unlock();
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Allows a previously banned IP to connect.
//
// Parameters
// IP - Dotted IP address.  Can use * as a wildcard, such as 128.0.0.* will ban
// All IP addresses starting with 128.0.0
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::RemoveFromBanList(const char *IP)
{
	unsigned index;
	char *temp;

	if (IP==0 || IP[0]==0 || strlen(IP)>15)
		return;

	index=0;
	temp=0;
	banListMutex.Lock();
	for (; index < banList.size(); index++)
	{
		if (strcmp(IP, banList[index])==0)
		{
			temp = banList[index];
			banList[index]=banList[banList.size()-1];
			banList.del(banList.size()-1);
			break;
		}
	}
	banListMutex.Unlock();
	if (temp)
		delete [] temp;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Allows all previously banned IPs to connect.
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::ClearBanList(void)
{
	unsigned index;
	index=0;
	banListMutex.Lock();
	for (; index < banList.size(); index++)
		delete [] banList[index];
	banList.clear();
	banListMutex.Unlock();
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Determines if a particular IP is banned.
//
// Parameters
// IP - Complete dotted IP address
//
// Returns
// True if IP matches any IPs in the ban list, accounting for any wildcards.
// False otherwise.
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool RakPeer::IsBanned(const char *IP)
{
	unsigned banListIndex, characterIndex;

	if (IP==0 || IP[0]==0 || strlen(IP)>15)
		return false;

	banListIndex=0;
	if (banList.size()==0)
		return false; // Skip the mutex if possible
	banListMutex.Lock();
	for (; banListIndex < banList.size(); banListIndex++)
	{
		characterIndex=0;
		while (true)
		{
			if (banList[banListIndex][characterIndex]==IP[characterIndex])
			{
				// Equal characters
				if (IP[characterIndex]==0)
				{
					banListMutex.Unlock();

					// End of the string and the strings match
					return true;
				}
				characterIndex++;
			}
			else
			{
				if (banList[banListIndex][characterIndex]==0 || IP[characterIndex]==0)
				{
					// End of one of the strings
					break;
				}
				// Characters do not match
				if (banList[banListIndex][characterIndex]=='*')
				{
					banListMutex.Unlock();

					// Domain is banned.
					return true;
				}
				// Characters do not match and it is not a *
				break;
			}
		}
	}
	banListMutex.Unlock();

	// No match found.
	return false;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Send a ping to the specified connected system.
//
// Parameters:
// target - who to ping
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::Ping(PlayerID target)
{
	if (IsActive()==false)
		return;

	PingStruct ping;
	ping.typeId=ID_PING;
	ping.sendPingTime=RakNetGetTime();

	Send((char*)&ping, sizeof(PingStruct), SYSTEM_PRIORITY, UNRELIABLE, 0, target, false);
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Send a ping to the specified unconnected system.
// The remote system, if it is Initialized, will respond with ID_PONG.
// The final ping time will be encoded in the following 4 bytes (2-5) as an unsigned long
//
// Requires:
// The sender and recipient must already be started via a successful call to Initialize
//
// Parameters:
// host: Either a dotted IP address or a domain name
// remotePort: Which port to connect to on the remote machine.
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::Ping(char* host, unsigned short remotePort)
{
	if (host==0)
		return;

	// If the host starts with something other than 0, 1, or 2 it's (probably) a domain name.
	if (host[0] < '0' || host[0] > '2')
	{
		host = (char*) SocketLayer::Instance()->DomainNameToIP(host);
	}

	UnconnectedPingStruct s;
	s.typeId=ID_PING;
	s.sendPingTime=RakNetGetTime();

	SocketLayer::Instance()->SendTo(connectionSocket, (char*)&s, sizeof(UnconnectedPingStruct), (char*)host, remotePort);
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Returns the average of all ping times read for a specified target
//
// Parameters:
// target - whose time to read
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
int RakPeer::GetAveragePing(PlayerID target)
{
	int sum, quantity;
	RemoteSystemStruct *remoteSystem = GetRemoteSystemFromPlayerID(target);
	if (remoteSystem==0)
		return -1;
	for (sum=0, quantity=0; quantity < PING_TIMES_ARRAY_SIZE; quantity++)
	{
		if (remoteSystem->pingAndClockDifferential[quantity].pingTime==-1)
			break;
		else
			sum+=remoteSystem->pingAndClockDifferential[quantity].pingTime;
	}

	if (quantity>0)
		return sum / quantity;
	else
		return -1;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Returns the last ping time read for the specific player or -1 if none read yet
//
// Parameters:
// target - whose time to read
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
int RakPeer::GetLastPing(PlayerID target) const
{
	RemoteSystemStruct *remoteSystem = GetRemoteSystemFromPlayerID(target);
	if (remoteSystem==0)
		return -1;
	if (remoteSystem->pingAndClockDifferentialWriteIndex==0)
		return remoteSystem->pingAndClockDifferential[PING_TIMES_ARRAY_SIZE-1].pingTime;
	else
		return remoteSystem->pingAndClockDifferential[remoteSystem->pingAndClockDifferentialWriteIndex-1].pingTime;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Returns the lowest ping time read or -1 if none read yet
//
// Parameters:
// target - whose time to read
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
int RakPeer::GetLowestPing(PlayerID target) const
{
	RemoteSystemStruct *remoteSystem = GetRemoteSystemFromPlayerID(target);
	if (remoteSystem==0)
		return -1;
	return remoteSystem->lowestPing;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Ping the remote systems every so often.  This is off by default
// This will work anytime
//
// Parameters:
// doPing - True to start occasional pings.  False to stop them.
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::SetOccasionalPing(bool doPing)
{
	occasionalPing=doPing;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Automatically synchronizes a block of memory between systems.
// Can be called anytime.  Calling it before a connection is initiated will cause the data to be synchronized on connection
//
// Parameters:
// uniqueIdentifier: an integer (enum) corresponding to the same variable between clients and the server.  Start the indexing at 0
// memoryBlock: Pointer to the data you want to read from or write to
// size: Size of memoryBlock in bytes
// isAuthority: True to tell all connected systems to match their data to yours.  Data changes are relayed to the authoritative
// - client which broadcasts the change
// synchronizationRules: Optional function pointer that decides whether or not to update changed memory.  It should
// - return true if the two passed memory blocks are sufficiently different to synchronize them.  This is an optimization so
// - data that changes rapidly, such as per-frame, can be made to not update every frame
// - The first parameter to synchronizationRules is the new data, the second is the internal copy of the old data
// secondaryUniqueIdentifier:  Optional and used when you have the same unique identifier and is intended for multiple instances of a class
// - that derives from NetworkObject.
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/*
void RakPeer::SynchronizeMemory(UniqueIDType uniqueIdentifier, char *memoryBlock, unsigned short size, bool isAuthority, bool (*synchronizationRules) (char*,char*),ObjectID secondaryUniqueIdentifier)
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

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Stops synchronization of a block of memory previously defined by uniqueIdentifier and secondaryUniqueIdentifier
// by the call to SynchronizeMemory
// CALL THIS BEFORE SYNCHRONIZED MEMORY IS DEALLOCATED!
// It is not necessary to call this before disconnecting, as all synchronized states will be released then.
// Parameters:
// uniqueIdentifier: an integer (enum) corresponding to the same variable between clients and the server.  Start the indexing at 0
// secondaryUniqueIdentifier:  Optional and used when you have the same unique identifier and is intended for multiple instances of a class
// - that derives from NetworkObject.
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::DesynchronizeMemory(UniqueIDType uniqueIdentifier, ObjectID secondaryUniqueIdentifier)
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

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Desynchronizes all synchronized memory
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::DesynchronizeAllMemory(void)
{
	int i;

	automaticVariableSynchronizationMutex.Lock();
	for (i=0; i < (int)automaticVariableSynchronizationList.size(); i++)
	{
		if (automaticVariableSynchronizationList[i])
		{
			for (unsigned j=0; j < automaticVariableSynchronizationList[i]->size(); j++)
				delete [] (*(automaticVariableSynchronizationList[i]))[j].copy;
			delete automaticVariableSynchronizationList[i];
		}
	}
	automaticVariableSynchronizationList.clear();
	automaticVariableSynchronizationMutex.Unlock();

	synchronizedMemoryQueueMutex.Lock();
	while (synchronizedMemoryPacketQueue.size())
	{
		PacketPool::Instance()->ReleasePointer(synchronizedMemoryPacketQueue.pop());
	}
	synchronizedMemoryQueueMutex.Unlock();
}
*/
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// All systems have a block of data associated with them, for user use.  This block of data can be used to easily
// specify typical system data that you want to know on connection, such as the player's name.
//
// Parameters:
// playerId: Which system you are referring to.  Pass the value returned by GetInternalID to refer to yourself
//
// Returns:
// The data passed to SetRemoteStaticData stored as a bitstream
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
RakNet::BitStream * RakPeer::GetRemoteStaticData(PlayerID playerId)
{
	if (playerId==myPlayerId)
		return &localStaticData;

	RemoteSystemStruct *remoteSystem = GetRemoteSystemFromPlayerID(playerId);
	if (remoteSystem)
		return &(remoteSystem->staticData);
	else
		return 0;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// All systems have a block of data associated with them, for user use.  This block of data can be used to easily
// specify typical system data that you want to know on connection, such as the player's name.
//
// Parameters:
// playerId: Whose static data to change.  Use your own playerId to change your own static data
// data: a block of data to store
// length: The length of data in bytes	
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::SetRemoteStaticData(PlayerID playerId, char *data, const long length)
{
	if (playerId==myPlayerId)
	{
		localStaticData.Reset();
		if (data && length > 0)
			localStaticData.Write(data, length);
	}
	else
	{
		RemoteSystemStruct *remoteSystem = GetRemoteSystemFromPlayerID(playerId);
		if (remoteSystem==0)
			return;
		remoteSystem->staticData.Reset();
		if (data && length > 0)
			remoteSystem->staticData.Write(data, length);
	}
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Sends your static data to the specified system.  This is automatically done on connection.
// You should call this when you change your static data.
// To send the static data of another system (such as relaying their data) you should do this normally with Send
//
// Parameters:
// target: Who to send your static data to.  Specify UNASSIGNED_PLAYER_ID to broadcast to all
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::SendStaticData(PlayerID target)
{
	RakNet::BitStream reply(sizeof(unsigned char) + localStaticData.GetNumberOfBytesUsed());
	reply.Write((unsigned char) ID_RECEIVED_STATIC_DATA);
	reply.Write((char*)localStaticData.GetData(), localStaticData.GetNumberOfBytesUsed());
	if (target==UNASSIGNED_PLAYER_ID)
		Send(&reply, SYSTEM_PRIORITY, RELIABLE, 0, target, true);
	else
		Send(&reply, SYSTEM_PRIORITY, RELIABLE, 0, target, false);
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Return the unique PlayerID that represents you on the the network
// Note that unlike in previous versions, this is a struct and is not sequential
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
PlayerID RakPeer::GetInternalID(void) const
{
	return myPlayerId;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Return the unique address identifier that represents you on the the network and is based on your external
// IP / port (the IP / port the specified player uses to communicate with you)
// Note that unlike in previous versions, this is a struct and is not sequential
//
// Parameters:
// target: Which remote system you are referring to for your external ID
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
PlayerID RakPeer::GetExternalID(PlayerID target) const
{
	RemoteSystemStruct *remoteSystem = GetRemoteSystemFromPlayerID(target);
	if (remoteSystem==0)
		return UNASSIGNED_PLAYER_ID;
	else
		return remoteSystem->myExternalPlayerId;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Change the MTU size in order to improve performance when sending large packets
// This can only be called when not connected.
// A too high of value will cause packets not to arrive at worst and be fragmented at best.
// A too low of value will split packets unnecessarily.
//
// Parameters:
// size: Set according to the following table:
// 1500. The largest Ethernet packet size
// This is the typical setting for non-PPPoE, non-VPN connections. The default value for NETGEAR routers, adapters and switches. 
// 1492. The size PPPoE prefers. 
// 1472. Maximum size to use for pinging. (Bigger packets are fragmented.) 
// 1468. The size DHCP prefers. 
// 1460. Usable by AOL if you don't have large email attachments, etc. 
// 1430. The size VPN and PPTP prefer. 
// 1400. Maximum size for AOL DSL. 
// 576. Typical value to connect to dial-up ISPs. (Default)
//
// Returns:
// False on failure (we are connected).  True on success.  Maximum allowed size is MAXIMUM_MTU_SIZE
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool RakPeer::SetMTUSize(int size)
{
	if (IsActive())
		return false;

	if (size < 512)
		size=512;
	else if (size > MAXIMUM_MTU_SIZE)
		size=MAXIMUM_MTU_SIZE;

	MTUSize=size;
	return true;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Returns the current MTU size
//
// Returns:
// The MTU sized specified in SetMTUSize
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
int RakPeer::GetMTUSize(void) const
{
	return MTUSize;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Returns the number of IP addresses we have
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
unsigned int RakPeer::GetNumberOfAddresses(void)
{
	char ipList[10][16];
	memset(ipList, 0, sizeof(char) * 16 * 10);
	SocketLayer::Instance()->GetMyIP(ipList);

	int i=0;
	while (ipList[i][0])
		i++;

	return i;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Given a PlayerID struct, returns the dotted IP address string this binaryAddress field represents
//
// Returns:
// Null terminated dotted IP address string.
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
const char* RakPeer::PlayerIDToDottedIP(PlayerID playerId) const
{
	in_addr in;
	in.s_addr=playerId.binaryAddress;
	return inet_ntoa(in);
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Returns an IP address at index 0 to GetNumberOfAddresses-1
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
const char* RakPeer::GetLocalIP(unsigned int index)
{
	char ipList[10][16];
	if (index >=10)
		index=9;
	memset(ipList, 0, sizeof(char) * 16 * 10);
	SocketLayer::Instance()->GetMyIP(ipList);
	return ipList[index];
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Enables or disables our tracking of bytes input to and output from the network.
// This is required to get a frequency table, which is used to generate a new compression layer.
// You can call this at any time - however you SHOULD only call it when disconnected.  Otherwise you will only track
// part of the values sent over the network.
// This value persists between connect calls and defaults to false (no frequency tracking)
// 
// Parameters:
// doCompile - true to track bytes.  Defaults to false
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::SetCompileFrequencyTable(bool doCompile)
{
	trackFrequencyTable=doCompile;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Returns the frequency of outgoing bytes into outputFrequencyTable
// The purpose is to save to file as either a master frequency table from a sample game session for passing to
// GenerateCompressionLayer(false)
// You should only call this when disconnected.
// Requires that you first enable data frequency tracking by calling SetCompileFrequencyTable(true)
//
// Parameters:
// outputFrequencyTable (out): The frequency of each corresponding byte
//
// Returns:
// Ffalse (failure) if connected or if frequency table tracking is not enabled.  Otherwise true (success)
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool RakPeer::GetOutgoingFrequencyTable(unsigned long outputFrequencyTable[256])
{
	if (IsActive())
		return false;

	if (trackFrequencyTable==false)
		return false;

	memcpy(outputFrequencyTable, frequencyTable, sizeof(unsigned long) * 256);

	return true;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Generates the compression layer from the input frequency table.
// You should call this twice - once with inputLayer as true and once as false.
// The frequency table passed here with inputLayer=true should match the frequency table on the recipient with inputLayer=false.
// Likewise, the frequency table passed here with inputLayer=false should match the frequency table on the recipient with inputLayer=true
// Calling this function when there is an existing layer will overwrite the old layer
// You should only call this when disconnected
//
// Parameters:
// inputFrequencyTable: The frequency table returned from GetSendFrequencyTable(...)
// inputLayer - Whether inputFrequencyTable represents incoming data from other systems (true) or outgoing data from this system (false)
//
// Returns:
// False on failure (we are connected).  True otherwise
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool RakPeer::GenerateCompressionLayer(unsigned long inputFrequencyTable[256], bool inputLayer)
{
	if (IsActive())
		return false;

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

	return true;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Deletes the output or input layer as specified.  This is not necessary to call and is only valuable for freeing memory
// You should only call this when disconnected
//
// Parameters:
// inputLayer - Specifies the corresponding compression layer generated by GenerateCompressionLayer.
//
// Returns:
// False on failure (we are connected).  True otherwise
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool RakPeer::DeleteCompressionLayer(bool inputLayer)
{
	if (IsActive())
		return false;

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

	return true;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Returns:
// The compression ratio.  A low compression ratio is good.  Compression is for outgoing data
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
float RakPeer::GetCompressionRatio(void) const
{
	if (rawBytesSent>0L)
	{
		return (float)compressedBytesSent / (float)rawBytesSent;
	}
	else return 0.0f;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Returns:
// The decompression ratio.  A high decompression ratio is good.  Decompression is for incoming data
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
float RakPeer::GetDecompressionRatio(void) const
{
	if (rawBytesReceived>0L)
	{
		return (float)compressedBytesReceived / (float)rawBytesReceived;
	}
	else return 0.0f;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Returns the data you passed to the passwordData parameter in Connect
//
// Parameters
// passwordData (out): Should point to a block large enough to hold the password data you passed to Connect
// passwordDataLength (in, out): Maximum size of the array passwordData.  Modified to hold the number of bytes actually written
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::GetPasswordData(char *passwordData, int *passwordDataLength)
{
	int length;
	if (incomingPasswordBitStream.GetNumberOfBytesUsed() < *passwordDataLength)
		length = incomingPasswordBitStream.GetNumberOfBytesUsed();
	else
		length = *passwordDataLength;

	memcpy(passwordData, incomingPasswordBitStream.GetData(), length);
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Put a packet back at the end of the receive queue in case you don't want to deal with it immediately
//
// Parameters
// packet: The packet you want to push back.
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::PushBackPacket(Packet *packet)
{
	if (packet)
	{
#ifdef _DEBUG
		assert(packet->data);
#endif
		incomingPacketQueue.push(packet);
	}
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
RakNetStatisticsStruct *  const RakPeer::GetStatistics(PlayerID playerId)
{
	RemoteSystemStruct *rss;
	rss=GetRemoteSystemFromPlayerID(playerId);
	if (rss)
		return rss->reliabilityLayer.GetStatistics();
	return 0;
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::RecordConnectionAttempt(const char* host, unsigned short remotePort)
{
	PlayerID playerId;
	RequestedConnectionStruct *s;
	s = new RequestedConnectionStruct;
	IPToPlayerID(host, remotePort, &playerId);
	s->playerId=playerId;
	s->time=RakNetGetTime();
	s->setAESKey=false;
	s->nextRequestTime=s->time + 2000;

	// Record that we tried to connect to this host
	rakPeerMutexes[requestedConnections_MUTEX].Lock();
	requestedConnectionsList.push(s);
	rakPeerMutexes[requestedConnections_MUTEX].Unlock();
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::RemoveFromRequestedConnectionsList(PlayerID playerId)
{
	int i;
	rakPeerMutexes[RakPeer::requestedConnections_MUTEX].Lock();
	for (i=0; i < (int)requestedConnectionsList.size();)
	{
		if (requestedConnectionsList[i]->playerId==playerId)
		{
			delete requestedConnectionsList[i];
			requestedConnectionsList.del(i);			
			break;
		}
	}
	rakPeerMutexes[RakPeer::requestedConnections_MUTEX].Unlock();
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool RakPeer::SendConnectionRequest(const char* host, unsigned short remotePort)
{
	int j;

	const unsigned char c=ID_CONNECTION_REQUEST;
	RakNet::BitStream temp(sizeof(unsigned char) + outgoingPasswordBitStream.GetNumberOfBytesUsed());
	temp.Write(c);
	if (outgoingPasswordBitStream.GetNumberOfBytesUsed()>0)
		temp.Write((char*)outgoingPasswordBitStream.GetData(), outgoingPasswordBitStream.GetNumberOfBytesUsed());

	j=SocketLayer::Instance()->SendTo(connectionSocket, (char*)temp.GetData(), temp.GetNumberOfBytesUsed(), (char*)host, remotePort);

#ifdef _WIN32
	if (j==WSAECONNRESET)
	{
		PlayerID temp;
		temp.binaryAddress=inet_addr(host);
		temp.port=remotePort;
		PushPortRefused(temp);
		closesocket(connectionSocket);
		rakPeerMutexes[requestedConnections_MUTEX].Lock();
		delete requestedConnectionsList.pop();
		rakPeerMutexes[requestedConnections_MUTEX].Unlock();
	}
#endif

	return j==0; // j==0 is success
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::IPToPlayerID(const char* host, unsigned short remotePort, PlayerID *playerId)
{
	if (host==0)
		return;
	playerId->binaryAddress=inet_addr(host);
	playerId->port=remotePort;
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
RakPeer::RemoteSystemStruct *RakPeer::GetRemoteSystemFromPlayerID(PlayerID playerID) const
{
	unsigned i;

	if (playerID==UNASSIGNED_PLAYER_ID)
		return 0;

	for (i=0; i < maximumNumberOfPeers; i++)
		if (remoteSystemList[i].playerId==playerID)
			return remoteSystemList+i;

	return 0;
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::HandleConnectionRequest(PlayerID playerId,unsigned char *AESKey, bool setAESKey)
{
	RemoteSystemStruct *remoteSystem;

	if (GetNumberOfIncomingConnections() < maximumIncomingConnections)
	{
		remoteSystem=AssignPlayerIDToRemoteSystemList(playerId, AESKey, setAESKey);

		if (remoteSystem==0)
		{
			// No reliability layer available
			unsigned char c = ID_NO_FREE_INCOMING_CONNECTIONS;
			SocketLayer::Instance()->SendTo(connectionSocket, (char*)&c, sizeof(char), playerId.binaryAddress, playerId.port);
			return;
		}

		#ifdef __USE_IO_COMPLETION_PORTS
		unsigned index;
		for (index=0; index < maximumNumberOfPeers; index++)
			if (remoteSystemList+index==remoteSystem)
				break;

		if (SetupIOCompletionPortSocket(index)==false)
		{
			// Socket error
			assert(0);
			return;
		}
		#endif

		ResetRemoteSystemData(remoteSystem, false);

		ConnectionAcceptStruct ds;
		ds.typeId=ID_CONNECTION_REQUEST_ACCEPTED;
		#ifdef __USE_IO_COMPLETION_PORTS
		ds.remotePort = myPlayerId.port+(unsigned short)index+(unsigned short)1;
		#else
		ds.remotePort = myPlayerId.port;
		#endif
		ds.externalID=playerId;
		ds.playerIndex=(PlayerIndex)GetIndexFromPlayerID(playerId);
		// Write using the new socket so the client knows what port to use
		int result=SocketLayer::Instance()->SendTo(connectionSocket, (char*)&ds, sizeof(ds), playerId.binaryAddress, playerId.port);

		if (result!=0)
		{
			CloseConnection(playerId, false,0L);
			return;
		}
	}
	else
	{
		unsigned char c = ID_NO_FREE_INCOMING_CONNECTIONS;
		SocketLayer::Instance()->SendTo(connectionSocket, (char*)&c, sizeof(char), playerId.binaryAddress, playerId.port);
	}
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
unsigned RakPeer::GetNumberOfIncomingConnections(void) const
{
	unsigned i,numberOfIncomingConnections;

	if (remoteSystemList==0 || endThreads==true)
		return 0;

	numberOfIncomingConnections=0;
	for (i=0; i < maximumNumberOfPeers; i++)
	{
		if (remoteSystemList[i].playerId!=UNASSIGNED_PLAYER_ID && remoteSystemList[i].weInitiatedTheConnection==false)
			numberOfIncomingConnections++;
	}

	return numberOfIncomingConnections;
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
RakPeer::RemoteSystemStruct * RakPeer::AssignPlayerIDToRemoteSystemList(PlayerID playerId,unsigned char *AESKey, bool setAESKey)
{
	RemoteSystemStruct *remoteSystem=0;
	unsigned i;

	rakPeerMutexes[RakPeer::remoteSystemList_Mutex].Lock();
	for (i=0; i < maximumNumberOfPeers; i++)
	{
		if (remoteSystemList[i].playerId==UNASSIGNED_PLAYER_ID)
		{
			if (setAESKey)
				(remoteSystemList[i]).reliabilityLayer.SetEncryptionKey(AESKey);
			else
				(remoteSystemList[i]).reliabilityLayer.SetEncryptionKey(0);
			// Reserve this reliability layer for ourselves
			(remoteSystemList[i]).playerId=playerId; // This one line causes future incoming packets to go through the reliability layer
			remoteSystem=remoteSystemList+i;
			break;
		}
	}

	rakPeerMutexes[remoteSystemList_Mutex].Unlock();

	return remoteSystem;
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::ResetRemoteSystemData(RemoteSystemStruct *remoteSystem, bool weInitiatedTheConnection)
{
	unsigned i;

	// Reset all the ping times for this guy
	for (i=0; i < PING_TIMES_ARRAY_SIZE; i++)
	{
		remoteSystem->pingAndClockDifferential[i].pingTime=-1;
		remoteSystem->pingAndClockDifferential[i].clockDifferential=0;
	}
	remoteSystem->pingAndClockDifferentialWriteIndex=0;
	remoteSystem->lowestPing=-1;
	remoteSystem->nextPingTime=0; // Ping immediately
	remoteSystem->weInitiatedTheConnection=weInitiatedTheConnection;
	remoteSystem->staticData.Reset();
	remoteSystem->connectionTime=RakNetGetTime();
	remoteSystem->myExternalPlayerId=UNASSIGNED_PLAYER_ID;
	remoteSystem->reliabilityLayer.Reset();
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Adjust the first four bytes (treated as unsigned long) of the pointer
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::ShiftIncomingTimestamp(char *data, PlayerID playerId) const
{
#ifdef _DEBUG
	assert(IsActive());
	assert(data);
#endif

	unsigned long encodedTimestamp;
	memcpy(&encodedTimestamp, data, sizeof(unsigned long));
	encodedTimestamp = encodedTimestamp - GetBestClockDifferential(playerId);
	memcpy(data, &encodedTimestamp, sizeof(unsigned long));
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Thanks to Chris Taylor (cat02e@fsu.edu) for the improved timestamping algorithm
unsigned long RakPeer::GetBestClockDifferential(PlayerID playerId) const
{
	int counter, clockDifferential, lowestPingSoFar;
	RemoteSystemStruct *remoteSystem = GetRemoteSystemFromPlayerID(playerId);
	if (remoteSystem==0)
		return 0;

	lowestPingSoFar=65535;
	clockDifferential=0;

	for (counter=0; counter < PING_TIMES_ARRAY_SIZE; counter++)
	{
		if (remoteSystem->pingAndClockDifferential[0].pingTime==-1)
			break;

		if (remoteSystem->pingAndClockDifferential[counter].pingTime < lowestPingSoFar)
		{
			clockDifferential=remoteSystem->pingAndClockDifferential[counter].clockDifferential;
			lowestPingSoFar=remoteSystem->pingAndClockDifferential[counter].pingTime;
		}
	}

	return clockDifferential;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Handles an RPC packet.  If you get a packet with the ID ID_RPC you should pass it to this function
// This is already done in Multiplayer.cpp, so if you use the Multiplayer class it is handled for you.
//
// Parameters:
// packet - A packet returned from Receive with the ID ID_RPC
//
// Returns:
// true on success, false on a bad packet or an unregistered function
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool RakPeer::HandleRPCPacket(char *data, int length, PlayerID playerId)
{
	// RPC BitStream format is
	// ID_RPC - unsigned char
	// Unique identifier string length - unsigned char
	// The unique ID  - string with each letter in upper case, subtracted by 'A' and written in 5 bits.
	// Number of bits of the data (long)
	// The data

	RakNet::BitStream incomingBitStream(data, length, false);
	unsigned char uniqueIDLength, ch, packetID;
	char uniqueIdentifier[256];
	int counter;
	long bitLength;
	char *userData;

	if (incomingBitStream.Read(packetID)==false)
	{
#ifdef _DEBUG
		assert(0); // bitstream was not long enough.  Some kind of internal error
#endif
		return false; 
	}

	if (packetID!=ID_RPC && packetID!=ID_RPC_WITH_TIMESTAMP)
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


	if (uniqueIDLength==255)
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
		//char *userData = new char[BITS_TO_BYTES(incomingBitStream.GetNumberOfUnreadBits())];
		userData=(char*)alloca(BITS_TO_BYTES(incomingBitStream.GetNumberOfUnreadBits()));

		// The false means read out the internal representation of the bitstream data rather than
		// aligning it as we normally would with user data.  This is so the end user can cast the data received
		// into a bitstream for reading
		if (incomingBitStream.ReadBits((unsigned char*)userData, bitLength, false)==false)
		{
#ifdef _DEBUG
			assert(0);
#endif
		//	delete [] userData;
			return false; // Not enough data to read
		}

		if (packetID==ID_RPC_WITH_TIMESTAMP)
			ShiftIncomingTimestamp(userData, playerId);
		// Call the function callback
		node->functionName(userData, bitLength, playerId);
		// Free the memory
//		delete [] userData;
	}

	return true;
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#ifdef __USE_IO_COMPLETION_PORTS
bool RakPeer::SetupIOCompletionPortSocket(int index)
{
	SOCKET newSocket;

	if (remoteSystemList[index].reliabilityLayer.GetSocket()!=INVALID_SOCKET)
		closesocket(remoteSystemList[index].reliabilityLayer.GetSocket());

	newSocket = SocketLayer::Instance()->CreateBoundSocket(myPlayerId.port+index+1, false);
	SocketLayer::Instance()->Connect(newSocket, remoteSystemList[index].playerId.binaryAddress,remoteSystemList[index].playerId.port); // port is the port of the client
	remoteSystemList[index].reliabilityLayer.SetSocket(newSocket);

	// Associate our new socket with a completion port and do the first read
	return SocketLayer::Instance()->AssociateSocketWithCompletionPortAndRead(newSocket, remoteSystemList[index].playerId.binaryAddress,remoteSystemList[index].playerId.port, this);
}
#endif

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::GenerateSYNCookieRandomNumber(void)
{
	unsigned long number;
	int i;
	memcpy(oldRandomNumber, newRandomNumber, sizeof(newRandomNumber));

	for (i=0; i < sizeof(newRandomNumber); i+=sizeof(number))
	{
		number=randomMT();
		memcpy(newRandomNumber+i, (char*)&number, sizeof(number));
	}

	randomNumberExpirationTime = RakNetGetTime() + SYN_COOKIE_OLD_RANDOM_NUMBER_DURATION;
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::SecuredConnectionResponse(PlayerID playerId)
{
	CSHA1 sha1;
	RSA_BIT_SIZE n;
	big::u32 e;
	unsigned char connectionRequestResponse[1+sizeof(big::u32) + sizeof(RSA_BIT_SIZE) + 20];
	connectionRequestResponse[0]=ID_SECURED_CONNECTION_RESPONSE;

	// Hash the SYN-Cookie
	// s2c syn-cookie = SHA1_HASH(source ip address + source port + random number)
	sha1.Reset();
	sha1.Update((unsigned char*)&playerId.binaryAddress, sizeof(playerId.binaryAddress));
	sha1.Update((unsigned char*)&playerId.port, sizeof(playerId.port));
	sha1.Update((unsigned char*)&(newRandomNumber), 20);
	sha1.Final();

	// Write the cookie
	memcpy(connectionRequestResponse+1, sha1.GetHash(), 20);

	// Write the public keys
	rsacrypt.getPublicKey(e,n);
	memcpy(connectionRequestResponse+1+20, (char*)&e, sizeof(big::u32));
	memcpy(connectionRequestResponse+1+20+sizeof(big::u32), n, sizeof(RSA_BIT_SIZE));

	// s2c public key, syn-cookie
	SocketLayer::Instance()->SendTo(connectionSocket, (char*)connectionRequestResponse, 1+sizeof(big::u32) + sizeof(RSA_BIT_SIZE) + 20, playerId.binaryAddress, playerId.port);
}
void RakPeer::SecuredConnectionConfirmation(PlayerID playerId, char* data)
{
	int i,j;
	unsigned char randomNumber[20];
	unsigned long number;
	bool doSend;
	Packet *packet;
	big::u32 e;
	RSA_BIT_SIZE n, message,encryptedMessage;
	big::RSACrypt<RSA_BIT_SIZE> rsaEncrypt;

	// Make sure that we still want to connect
	bool requestedConnection=false;
	rakPeerMutexes[RakPeer::requestedConnections_MUTEX].Lock();
	for (i=0; i < (int)requestedConnectionsList.size();i++)
	{
		if (requestedConnectionsList[i]->playerId==playerId)
		{
			// We did request this connection
			requestedConnection=true;
			break;
		}
	}
	rakPeerMutexes[RakPeer::requestedConnections_MUTEX].Unlock();

	if (requestedConnection==false)
		return; // Don't want to connect

	doSend=false;

	// Copy out e and n
	memcpy((char*)&e,data+1+20, sizeof(big::u32));
	memcpy(n, data+1+20+sizeof(big::u32), sizeof(RSA_BIT_SIZE));

	// If we preset a size and it doesn't match, or the keys do not match, then tell the user
	if (usingSecurity==true && keysLocallyGenerated==false)
	{
		if (memcmp((char*)&e, (char*)&publicKeyE, sizeof(big::u32))!=0 ||
			memcmp(n, publicKeyN, sizeof(RSA_BIT_SIZE))!=0)
		{
			packet = PacketPool::Instance()->GetPointer();
			packet->data = new unsigned char[1];
			packet->data[0]=ID_RSA_PUBLIC_KEY_MISMATCH;
			packet->length=sizeof(char);
			packet->bitSize=sizeof(char)*8;
			packet->playerId=playerId;
			incomingQueueMutex.Lock();
			incomingPacketQueue.push(packet);
			incomingQueueMutex.Unlock();
			RemoveFromRequestedConnectionsList(playerId);
			return;
		}
	}

	// Create a random number
	for (i=0; i < sizeof(randomNumber); i+=sizeof(number))
	{
		number=randomMT();
		memcpy(randomNumber+i, (char*)&number, sizeof(number));
	}

	memset(message, 0, sizeof(message));
	assert(sizeof(message) >= sizeof(randomNumber));
	memcpy(message, randomNumber, sizeof(randomNumber));
	rsaEncrypt.setPublicKey(e,n);
	rsaEncrypt.encrypt(message,encryptedMessage);

	rakPeerMutexes[RakPeer::requestedConnections_MUTEX].Lock();
	for (i=0; i < (int)requestedConnectionsList.size();)
	{
		if (requestedConnectionsList[i]->playerId==playerId)
		{
			doSend=true;
			// Generate the AES key
			for (j=0; j < 16; j++)
				requestedConnectionsList[i]->AESKey[j]=data[1+j] ^ randomNumber[j];
			requestedConnectionsList[i]->setAESKey=true;
			break;
		}
	}
	rakPeerMutexes[RakPeer::requestedConnections_MUTEX].Unlock();

	if (doSend)
	{
		char reply[1+20+sizeof(RSA_BIT_SIZE)];
		// c2s RSA(random number), same syn-cookie
		reply[0]=ID_SECURED_CONNECTION_CONFIRMATION;
		memcpy(reply+1, data+1, 20);  // Copy the syn-cookie
		memcpy(reply+1+20, encryptedMessage, sizeof(RSA_BIT_SIZE)); // Copy the encoded random number
		SocketLayer::Instance()->SendTo(connectionSocket, reply, 1+20+sizeof(RSA_BIT_SIZE), playerId.binaryAddress, playerId.port);
	}
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::PushPortRefused(PlayerID target)
{
	// Tell the game we can't connect to this host
	Packet *p;
	p = PacketPool::Instance()->GetPointer();
	p->data=new unsigned char[1];
	p->data[0]=ID_REMOTE_PORT_REFUSED;
	p->length=sizeof(char);
	p->playerId=target; // We don't know this!

	#ifdef _DEBUG
	assert(p->data);
	#endif
	// Relay this message to the game
	incomingQueueMutex.Lock();
	incomingPacketQueue.push(p);
	incomingQueueMutex.Unlock();
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/*
#ifdef _WIN32
unsigned __stdcall RecvFromNetworkLoop(LPVOID arguments)
#else
void*  RecvFromNetworkLoop( void*  arguments )
#endif
{
	RakPeer *peer = (RakPeer *)arguments;
	unsigned long errorCode;

#ifdef __USE_IO_COMPLETION_PORTS
	AsynchronousFileIO::Instance()->IncreaseUserCount();
#endif

	peer->isRecvfromThreadActive=true;

	while(peer->endThreads==false)
	{
		peer->isSocketLayerBlocking=true;
		errorCode=SocketLayer::Instance()->RecvFrom(peer->connectionSocket, peer);
		peer->isSocketLayerBlocking=false;

#ifdef _WIN32
		if (errorCode==WSAECONNRESET)
		{
			peer->PushPortRefused(UNASSIGNED_PLAYER_ID);
			//closesocket(peer->connectionSocket);
			//peer->connectionSocket = SocketLayer::Instance()->CreateBoundSocket(peer->myPlayerId.port, true);
		}
		else if (errorCode!=0 && peer->endThreads==false)
		{
#ifdef _DEBUG
			printf("Server RecvFrom critical failure!\n");
#endif
			// Some kind of critical error
			peer->isRecvfromThreadActive=false;
			peer->endThreads=true;
			peer->Disconnect();
			break;
		}
#else
		if (errorCode==-1)
		{
			peer->isRecvfromThreadActive=false;
			peer->endThreads=true;
			peer->Disconnect();
			break;
		}
#endif
	}

#ifdef __USE_IO_COMPLETION_PORTS
	AsynchronousFileIO::Instance()->DecreaseUserCount();
#endif

	peer->isRecvfromThreadActive=false;

#ifdef _WIN32
	//_endthreadex( 0 );
#endif
	return 0;
}
*/
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#ifdef _WIN32
void __stdcall ProcessNetworkPacket(unsigned long binaryAddress, unsigned short port, char *data, int length, RakPeer *rakPeer)
#else
void ProcessNetworkPacket(unsigned long binaryAddress, unsigned short port, char *data, int length, RakPeer *rakPeer)
#endif
{
	PlayerID playerId;
	unsigned i;
	playerId.binaryAddress=binaryAddress;
	playerId.port=port;
	RakPeer::RemoteSystemStruct *remoteSystem = rakPeer->GetRemoteSystemFromPlayerID(playerId);

	if (remoteSystem)
	{
		// Handle regular incoming data
		// HandleSocketReceiveFromConnectedPlayer is only safe to be called from the same thread as Update,
		// which is this thread
		if (remoteSystem->reliabilityLayer.HandleSocketReceiveFromConnectedPlayer(data, length)==false)
		{
			// Cheater
			Packet *packet = PacketPool::Instance()->GetPointer();
			packet->data = new unsigned char[1];
			packet->data[0]=ID_MODIFIED_PACKET;
			packet->length=sizeof(char);
			packet->bitSize=sizeof(char)*8;
			packet->playerId=playerId;
			rakPeer->incomingQueueMutex.Lock();
			rakPeer->incomingPacketQueue.push(packet);
			rakPeer->incomingQueueMutex.Unlock();
		}
	}
	else
	{
		if ((unsigned char)(data)[0]==ID_CONNECTION_REQUEST)
		{
			// If we are full, tell the sender
			if (rakPeer->GetNumberOfIncomingConnections() >= rakPeer->GetMaximumIncomingConnections())
			{
				unsigned char c = ID_NO_FREE_INCOMING_CONNECTIONS;
				SocketLayer::Instance()->SendTo(rakPeer->connectionSocket, (char*)&c, sizeof(char), playerId.binaryAddress, playerId.port);
			}
			char *password = data + sizeof(unsigned char);
			int passwordLength = length - sizeof(unsigned char);
			if (rakPeer->IsBanned(rakPeer->PlayerIDToDottedIP(playerId)))
			{
				// This one we only send once since we don't care if it arrives.
				unsigned char c = ID_CONNECTION_BANNED;
				SocketLayer::Instance()->SendTo(rakPeer->connectionSocket, (char*)&c, sizeof(char), playerId.binaryAddress, playerId.port);
			}
			else if (rakPeer->incomingPasswordBitStream.GetNumberOfBytesUsed()==passwordLength &&
				memcmp(password, rakPeer->incomingPasswordBitStream.GetData(), passwordLength)==0)
			{
				if (rakPeer->usingSecurity==false)
				{
					// Connect this player assuming we have open slots
					rakPeer->HandleConnectionRequest(playerId, 0, false);
				}
				else
					rakPeer->SecuredConnectionResponse(playerId);
				
			}
			else
			{
				// This one we only send once since we don't care if it arrives.
				unsigned char c = ID_INVALID_PASSWORD;
				SocketLayer::Instance()->SendTo(rakPeer->connectionSocket, (char*)&c, sizeof(char), playerId.binaryAddress, playerId.port);
			}
		}
		else if ((unsigned char)(data)[0]== ID_SECURED_CONNECTION_RESPONSE &&
			length==1+sizeof(big::u32) + sizeof(RSA_BIT_SIZE) + 20)
		{
			rakPeer->SecuredConnectionConfirmation(playerId, data);
		}
		else if ((unsigned char)(data)[0]== ID_SECURED_CONNECTION_CONFIRMATION &&
			length==1+20+sizeof(RSA_BIT_SIZE))
		{
			CSHA1 sha1;
			bool confirmedHash, newRandomNumber;

			confirmedHash=false;

			// Hash the SYN-Cookie
			// s2c syn-cookie = SHA1_HASH(source ip address + source port + random number)
			sha1.Reset();
			sha1.Update((unsigned char*)&playerId.binaryAddress, sizeof(playerId.binaryAddress));
			sha1.Update((unsigned char*)&playerId.port, sizeof(playerId.port));
			sha1.Update((unsigned char*)&(rakPeer->newRandomNumber), 20);
			sha1.Final();

			newRandomNumber=false;

			// Confirm if
			//syn-cookie ?= HASH(source ip address + source port + last random number)
			//syn-cookie ?= HASH(source ip address + source port + current random number)
			if (memcmp(sha1.GetHash(), data+1, 20)==0)
			{
				confirmedHash=true;
				newRandomNumber=true;
			}
			else if (rakPeer->randomNumberExpirationTime < RakNetGetTime())
			{
				sha1.Reset();
				sha1.Update((unsigned char*)&playerId.binaryAddress, sizeof(playerId.binaryAddress));
				sha1.Update((unsigned char*)&playerId.port, sizeof(playerId.port));
				sha1.Update((unsigned char*)&(rakPeer->oldRandomNumber), 20);
				sha1.Final();

				if (memcmp(sha1.GetHash(), data+1, 20)==0)
					confirmedHash=true;
			}

			if (confirmedHash)
			{
				int i;
				unsigned char AESKey[16];
				RSA_BIT_SIZE message,encryptedMessage;

				// On connection accept, AES key is c2s RSA_Decrypt(random number) XOR s2c syn-cookie
				// Get the random number first
				memcpy(encryptedMessage, data+1+20, sizeof(RSA_BIT_SIZE));
				rakPeer->rsacrypt.decrypt(encryptedMessage, message);

				// Save the AES key
				for (i=0; i < 16; i++)
					AESKey[i]=data[1+i] ^ ((unsigned char*)(message))[i];

				// Connect this player assuming we have open slots
				rakPeer->HandleConnectionRequest(playerId,AESKey, true);

				// Invalidate the new random number
				if (newRandomNumber)
					rakPeer->GenerateSYNCookieRandomNumber();
			}
		}
		else if ((unsigned char)(data)[0]== ID_CONNECTION_REQUEST_ACCEPTED && length==sizeof(ConnectionAcceptStruct))
		{
			 // Make sure this connection accept is from someone we wanted to connect to
			bool requestedConnection;
//			unsigned long time = RakNetGetTime();
			unsigned char AESKey[16];
			bool setAESKey;
			setAESKey=false;
			requestedConnection=false;
			rakPeer->rakPeerMutexes[RakPeer::requestedConnections_MUTEX].Lock();
			for (i=0; i < rakPeer->requestedConnectionsList.size();i++)
			{
				if (rakPeer->requestedConnectionsList[i]->playerId==playerId)
				{
					// We did request this connection
					requestedConnection=true;
					setAESKey=rakPeer->requestedConnectionsList[i]->setAESKey;
					if (setAESKey)
						memcpy(AESKey, rakPeer->requestedConnectionsList[i]->AESKey, 16);
					delete rakPeer->requestedConnectionsList[i];
					rakPeer->requestedConnectionsList.del(i);
					break;
				}
			}
			rakPeer->rakPeerMutexes[RakPeer::requestedConnections_MUTEX].Unlock();

			 if (requestedConnection)
			 {
				// Find a free remote system struct to use
				ConnectionAcceptStruct* cas = (ConnectionAcceptStruct*) data;
				playerId.port=cas->remotePort;
				remoteSystem=rakPeer->AssignPlayerIDToRemoteSystemList(playerId, AESKey,setAESKey);

				if (remoteSystem!=0)
				{
					rakPeer->ResetRemoteSystemData(remoteSystem, true);

					// The remote system told us our external IP, so save it
					remoteSystem->myExternalPlayerId=cas->externalID;

#ifdef __USE_IO_COMPLETION_PORTS
					bool b;
					// Create a new nonblocking socket
					remoteSystem->reliabilityLayer.SetSocket(SocketLayer::Instance()->CreateBoundSocket(rakPeer->myPlayerId.port, false));

					SocketLayer::Instance()->Connect(remoteSystem->reliabilityLayer.GetSocket(), playerId.binaryAddress, playerId.port);
					// Associate our new socket with a completion port and do the first read
					b=SocketLayer::Instance()->AssociateSocketWithCompletionPortAndRead(remoteSystem->reliabilityLayer.GetSocket(), playerId.binaryAddress, playerId.port, rakPeer);
					//client->//reliabilityLayerMutex.Unlock();

					if (b==false) // Some damn completion port error... windows is so unreliable
					{
#ifdef _DEBUG
						printf("RakClient - AssociateSocketWithCompletionPortAndRead failed");
#endif
						return;
					}
#endif

					// Send the connection request complete to the game
					Packet *packet = PacketPool::Instance()->GetPointer();
					packet->data = new unsigned char[1];
					packet->data[0]=ID_CONNECTION_REQUEST_ACCEPTED;
					packet->length=sizeof(char);
					packet->bitSize=sizeof(char)*8;
					packet->playerId=playerId;

					#ifdef _DEBUG
					assert(packet->data);
					#endif
					rakPeer->incomingQueueMutex.Lock();
					rakPeer->incomingPacketQueue.push(packet);
					rakPeer->incomingQueueMutex.Unlock();

					NewIncomingConnectionStruct newIncomingConnectionStruct;
					newIncomingConnectionStruct.typeId=ID_NEW_INCOMING_CONNECTION;
					newIncomingConnectionStruct.externalID=playerId;

					rakPeer->Send((char*)&newIncomingConnectionStruct, sizeof(newIncomingConnectionStruct), SYSTEM_PRIORITY, RELIABLE, 0, playerId, false);
					rakPeer->Ping(playerId);
					rakPeer->SendStaticData(playerId);
				}
				else
				{
					// Cancel the connection attempt
					char c = ID_DISCONNECTION_NOTIFICATION;
					SocketLayer::Instance()->SendTo( rakPeer->connectionSocket, (char*)&c, sizeof(char), playerId.binaryAddress, playerId.port);
				}
			}
			 else
			 {
#ifdef _DEBUG
				 printf("Error: Got a connection accept when we didn't request the connection.\n");
#endif
			 }
		 }
		 else if ((unsigned char)(data)[0]== ID_PING && length==sizeof(UnconnectedPingStruct))
		 {
			 data[0]=ID_PONG;
			 SocketLayer::Instance()->SendTo( rakPeer->connectionSocket, data, sizeof(UnconnectedPingStruct), playerId.binaryAddress, playerId.port);
		 }
		 else if ((unsigned char)(data)[0]== ID_PONG && length==sizeof(UnconnectedPingStruct))
		 {
			 Packet *packet = PacketPool::Instance()->GetPointer();

			 packet->data = new unsigned char [sizeof(UnconnectedPingStruct)];
			 unsigned long time;
			 memcpy((char*)&time, data+sizeof(unsigned char), sizeof(unsigned long));
			 time = RakNetGetTime() - time;
			 packet->data[0]=ID_PONG;
			 memcpy(packet->data+sizeof(unsigned char), (char*)&time, sizeof(unsigned long));
			 
			 packet->length=sizeof(UnconnectedPingStruct);
			 packet->bitSize=sizeof(UnconnectedPingStruct) * 8;

			 packet->playerId=playerId;

			 rakPeer->incomingQueueMutex.Lock();
			 (rakPeer->incomingPacketQueue).push(packet);
			 rakPeer->incomingQueueMutex.Unlock();
		 }
		 else if ((unsigned char)(data)[0]==ID_NO_FREE_INCOMING_CONNECTIONS && length==sizeof(unsigned char))
		 {
			 Packet *packet = PacketPool::Instance()->GetPointer();
			 packet->data = new unsigned char [1 * sizeof(unsigned char)];
			 packet->data[0]=ID_NO_FREE_INCOMING_CONNECTIONS;
			 packet->length=1 * sizeof(unsigned char);
			 packet->bitSize=8 * sizeof(unsigned char);
			 rakPeer->incomingQueueMutex.Lock();
			 (rakPeer->incomingPacketQueue).push(packet);
			 rakPeer->incomingQueueMutex.Unlock();
		 }
		 else if ((unsigned char)(data)[0]==ID_CONNECTION_BANNED && length==sizeof(unsigned char))
		 {
			 Packet *packet = PacketPool::Instance()->GetPointer();
			 packet->data = new unsigned char [1 * sizeof(unsigned char)];
			 packet->data[0]=ID_CONNECTION_BANNED;
			 packet->length=1 * sizeof(unsigned char);
			 packet->bitSize=8 * sizeof(unsigned char);
			 rakPeer->incomingQueueMutex.Lock();
			 (rakPeer->incomingPacketQueue).push(packet);
			 rakPeer->incomingQueueMutex.Unlock();
		 }
	}
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool RakPeer::RunUpdateCycle(void)
{
	RakPeer::RemoteSystemStruct* remoteSystem;
	unsigned remoteSystemIndex;
	Packet *packet;
	long ping, lastPing;
//	int currentSentBytes,currentReceivedBytes;
	unsigned long time;
	unsigned numberOfBytesUsed;
	unsigned numberOfBitsUsed;
	//PlayerID authoritativeClientPlayerId;
	RakNet::BitStream dataBitStream(MAXIMUM_MTU_SIZE);
	int bitSize, byteSize;
	char *data;
	int errorCode;
	bool gotData;

	// We calculate this from the lowest numerical player ID
	//authoritativeClientPlayerId=UNASSIGNED_PLAYER_ID;

	do
	{
		// Read a packet
		gotData=SocketLayer::Instance()->RecvFrom(connectionSocket, this, &errorCode);

		if (gotData==false)
		{

#ifdef _WIN32
			if (errorCode==WSAECONNRESET)
			{
				PushPortRefused(UNASSIGNED_PLAYER_ID);
				//closesocket(peer->connectionSocket);

				//peer->connectionSocket = SocketLayer::Instance()->CreateBoundSocket(peer->myPlayerId.port, true);
			}
			else if (errorCode!=0 && endThreads==false)
			{
#ifdef _DEBUG
				printf("Server RecvFrom critical failure!\n");
#endif
				// Some kind of critical error
				//	peer->isRecvfromThreadActive=false;
				endThreads=true;
				Disconnect(0L);
				return false;
			}
#else
			if (errorCode==-1)
			{
				//	isRecvfromThreadActive=false;
				endThreads=true;
				Disconnect(0L);
				return false;
			}
#endif
		}
		if (endThreads)
			return false;
	} while (gotData); // Read until there is nothing left


	// Get the current system time
	time = RakNetGetTime();

	// Update the requested connection list.
	if (requestedConnectionsList.size()>0)
	{
		remoteSystemIndex=0;
		rakPeerMutexes[RakPeer::requestedConnections_MUTEX].Lock();
		while (remoteSystemIndex < requestedConnectionsList.size())
		{
			// After X seconds give up
			if (time - requestedConnectionsList[remoteSystemIndex]->time > SYN_COOKIE_OLD_RANDOM_NUMBER_DURATION*2)
			{
				delete requestedConnectionsList[remoteSystemIndex];
				requestedConnectionsList.del(remoteSystemIndex);
			}
			else if (time > requestedConnectionsList[remoteSystemIndex]->nextRequestTime)
			{
				SendConnectionRequest(
					PlayerIDToDottedIP(requestedConnectionsList[remoteSystemIndex]->playerId),
					requestedConnectionsList[remoteSystemIndex]->playerId.port);

				// Send again 2 seconds later
				requestedConnectionsList[remoteSystemIndex]->nextRequestTime=time + 2000;
				remoteSystemIndex++;
			}
			else
				remoteSystemIndex++;
		}
		rakPeerMutexes[RakPeer::requestedConnections_MUTEX].Unlock();
	}

	for (remoteSystemIndex=0; remoteSystemIndex < maximumNumberOfPeers; ++remoteSystemIndex)
	{
		if (remoteSystemList[remoteSystemIndex].playerId!=UNASSIGNED_PLAYER_ID)
		{
			// Found an active remote system
			remoteSystem = remoteSystemList+remoteSystemIndex;
			// Update is only safe to call from the same thread that calls HandleSocketReceiveFromConnectedPlayer,
			// which is this thread
			remoteSystem->reliabilityLayer.Update(connectionSocket, remoteSystem->playerId, MTUSize, time);

			// Was the reliability layer unable to deliver a reliable packet?
			if (remoteSystem->reliabilityLayer.IsDeadConnection())
			{
				packet = PacketPool::Instance()->GetPointer();

				packet->data = new unsigned char [sizeof(char) + remoteSystem->staticData.GetNumberOfBytesUsed()];
				packet->data[0]=ID_CONNECTION_LOST;
				memcpy(packet->data+sizeof(char), remoteSystem->staticData.GetData(), remoteSystem->staticData.GetNumberOfBytesUsed());

				packet->length=sizeof(char) + remoteSystem->staticData.GetNumberOfBytesUsed();
				packet->bitSize=(sizeof(char) + remoteSystem->staticData.GetNumberOfBytesUsed()) * 8;

				packet->playerId=remoteSystem->playerId;

				incomingQueueMutex.Lock();
				(incomingPacketQueue).push(packet);
				incomingQueueMutex.Unlock();

				CloseConnection(remoteSystem->playerId, false, 0L);
				continue;
			}

			// Did the reliability layer detect a modified packet?
			if (remoteSystem->reliabilityLayer.IsCheater())
			{
				packet = PacketPool::Instance()->GetPointer();
				packet->length=1;
				packet->data = new unsigned char [1];
				packet->data[0] = (unsigned char)ID_MODIFIED_PACKET;
				packet->playerId=remoteSystem->playerId;

				incomingQueueMutex.Lock();
				(incomingPacketQueue).push(packet);
				incomingQueueMutex.Unlock();

				continue;
			}

			// Ping this guy if it is time to do so
			if (time > remoteSystem->nextPingTime && (occasionalPing || remoteSystem->lowestPing==-1))
			{
				remoteSystem->nextPingTime = time + 5000;
				Ping(remoteSystem->playerId);
			}

			// Find whoever has the lowest player ID
			//if (remoteSystem->playerId < authoritativeClientPlayerId)
			//	authoritativeClientPlayerId=remoteSystem->playerId;

			// Does the reliability layer have any packets waiting for us?
			// To be thread safe, this has to be called in the same thread as HandleSocketReceiveFromConnectedPlayer
			bitSize = remoteSystem->reliabilityLayer.Receive(&data);

			while (bitSize > 0)
			{
				// Put the input through compression if necessary
				if (inputTree)
				{
					// Since we are decompressing input, we need to copy to a bitstream, decompress, then copy back to a probably
					// larger data block.  It's slow, but the user should have known that anyway
					dataBitStream.Reset();
					dataBitStream.WriteAlignedBytes((unsigned char*)data, BITS_TO_BYTES(bitSize));
					numberOfBytesUsed = dataBitStream.GetNumberOfBytesUsed();
					numberOfBitsUsed = dataBitStream.GetNumberOfBitsUsed();
					rawBytesReceived+=numberOfBytesUsed;
					// Decompress the input data.
					if (inputTree)
					{
#ifdef _DEBUG
						assert(numberOfBitsUsed>0);
#endif
						unsigned char *dataCopy = new unsigned char[numberOfBytesUsed];
						memcpy(dataCopy, dataBitStream.GetData(), numberOfBytesUsed);
						dataBitStream.Reset();
						inputTree->DecodeArray(dataCopy,numberOfBitsUsed, &dataBitStream);
						compressedBytesReceived+=dataBitStream.GetNumberOfBytesUsed();
						delete [] dataCopy;
					}
					byteSize = dataBitStream.GetNumberOfBytesUsed();
					if (byteSize > BITS_TO_BYTES(bitSize)) // Probably the case - otherwise why decompress?
					{
						delete [] data;
						data = new char [byteSize];
					}
					memcpy(data, dataBitStream.GetData(), byteSize);
				}
				else
					// Fast and easy - just use the data that was returned
					byteSize = BITS_TO_BYTES(bitSize);

				// Read any system packets
				if ((unsigned char)data[0]==ID_PONG && byteSize==sizeof(PingStruct))
				{
					// Copy into the ping times array the current time - the value returned
					// First extract the sent ping
					PingStruct *ps = (PingStruct *)data;

					ping=time - ps->sendPingTime;
					lastPing = remoteSystem->pingAndClockDifferential[remoteSystem->pingAndClockDifferentialWriteIndex].pingTime;

					// Ignore super high spikes in the average
					if (lastPing <= 0 || (((int)ping < (lastPing * 3)) && ping < 1200))
					{
						remoteSystem->pingAndClockDifferential[remoteSystem->pingAndClockDifferentialWriteIndex].pingTime=(short)ping;
						// Thanks to Chris Taylor (cat02e@fsu.edu) for the improved timestamping algorithm
						remoteSystem->pingAndClockDifferential[remoteSystem->pingAndClockDifferentialWriteIndex].clockDifferential=ps->sendPongTime - (time + ps->sendPingTime) / 2;
						if (remoteSystem->lowestPing==-1 ||  remoteSystem->lowestPing > ping)
							remoteSystem->lowestPing = ping;
						// Most packets should arrive by the ping time.
						remoteSystem->reliabilityLayer.SetLostPacketResendDelay(ping*2);

						if (++(remoteSystem->pingAndClockDifferentialWriteIndex) == PING_TIMES_ARRAY_SIZE)
							remoteSystem->pingAndClockDifferentialWriteIndex=0;
					}

					delete [] data;
				}
				else if ((unsigned char)data[0]==ID_PING && byteSize==sizeof(PingStruct))
				{
					PingStruct *ps = (PingStruct*)data;
					ps->typeId=ID_PONG;
					ps->sendPongTime=RakNetGetTime();

					Send(data,byteSize, SYSTEM_PRIORITY, UNRELIABLE, 0, remoteSystem->playerId, false);
					delete [] data;
				}
				else if ((unsigned char)data[0]==ID_NEW_INCOMING_CONNECTION && byteSize==sizeof(NewIncomingConnectionStruct))
				{
					Ping(remoteSystem->playerId);
					SendStaticData(remoteSystem->playerId);

					NewIncomingConnectionStruct *newIncomingConnectionStruct = (NewIncomingConnectionStruct *) data;
					remoteSystem->myExternalPlayerId=newIncomingConnectionStruct->externalID;

					// Send this info down to the game
					packet = PacketPool::Instance()->GetPointer();
					packet->data = (unsigned char*)data;
					packet->length=byteSize;
					packet->bitSize=bitSize;
					packet->playerId=remoteSystem->playerId;

#ifdef _DEBUG
					assert(packet->data);
#endif
					incomingQueueMutex.Lock();
					incomingPacketQueue.push(packet);
					incomingQueueMutex.Unlock();
				}
				/*
				else if ((unsigned char)data[0]==ID_SYNCHRONIZE_MEMORY)
				{
				if (byteSize>2)
				{
				packet = PacketPool::Instance()->GetPointer();
				packet->data = data;
				packet->length=byteSize;
				packet->bitSize=bitSize;
				packet->playerId=remoteSystem->playerId;

				synchronizedMemoryQueueMutex.Lock();
				synchronizedMemoryPacketQueue.push(packet);
				synchronizedMemoryQueueMutex.Unlock();
				}
				else
				delete [] data;
				}
				*/
				else if ((unsigned char)data[0]==ID_DISCONNECTION_NOTIFICATION)
				{
					packet = PacketPool::Instance()->GetPointer();
					if (remoteSystem->staticData.GetNumberOfBytesUsed()>0)
					{
						packet->data = new unsigned char [sizeof(char) + remoteSystem->staticData.GetNumberOfBytesUsed()];
						packet->data[0]=ID_DISCONNECTION_NOTIFICATION;
						memcpy(packet->data+sizeof(char), remoteSystem->staticData.GetData(), remoteSystem->staticData.GetNumberOfBytesUsed());

						packet->length=sizeof(char) + remoteSystem->staticData.GetNumberOfBytesUsed();
						packet->bitSize=sizeof(char)*8 + remoteSystem->staticData.GetNumberOfBitsUsed();

						delete [] data;
					}
					else
					{
						packet->data=(unsigned char*)data;
						packet->bitSize=bitSize;
						packet->length=1;
					}

					packet->playerId=remoteSystem->playerId;

					CloseConnection(remoteSystem->playerId, false, 0L);

#ifdef _DEBUG
					assert(packet->data);
#endif
					// Relay this message to the game
					incomingQueueMutex.Lock();
					incomingPacketQueue.push(packet);
					incomingQueueMutex.Unlock();

				}
				else if ((unsigned char)data[0]==ID_REQUEST_STATIC_DATA)
				{
					SendStaticData(remoteSystem->playerId);
					delete [] data;
				}
				else if ((unsigned char)data[0] == ID_RECEIVED_STATIC_DATA)
				{
					remoteSystem->staticData.Reset();
					remoteSystem->staticData.Write((char*)data+sizeof(unsigned char), byteSize-1);

					// Inform game server code that we got static data
					packet = PacketPool::Instance()->GetPointer();
					packet->data = (unsigned char*)data;
					packet->length = byteSize;
					packet->bitSize=bitSize;
					packet->playerId = remoteSystem->playerId;

#ifdef _DEBUG
					assert(packet->data);
#endif
					incomingQueueMutex.Lock();
					incomingPacketQueue.push(packet);
					incomingQueueMutex.Unlock();
				}
				else if ((unsigned char)data[0] == ID_RPC || (unsigned char)data[0] == ID_RPC_WITH_TIMESTAMP)
				{
					HandleRPCPacket(data, byteSize, remoteSystem->playerId);
					delete [] data;
				}
				else
				{
					packet = PacketPool::Instance()->GetPointer();
					packet->data = (unsigned char*)data;
					packet->length=byteSize;
					packet->bitSize=bitSize;
					packet->playerId=remoteSystem->playerId;

#ifdef _DEBUG
					assert(packet->data);
#endif
					incomingQueueMutex.Lock();
					incomingPacketQueue.push(packet);
					incomingQueueMutex.Unlock();
				}

				// Does the reliability layer have any more packets waiting for us?
				// To be thread safe, this has to be called in the same thread as HandleSocketReceiveFromConnectedPlayer
				bitSize = remoteSystem->reliabilityLayer.Receive(&data);
			}
		}
	}


	/*
	// Statistics histogram
	if (time > nextReadBytesTime)
	{
		nextReadBytesTime = time + 1000L; // 1 second
		for (remoteSystemIndex=0; remoteSystemIndex < maximumNumberOfPeers; ++remoteSystemIndex)
		{
		currentSentBytes = GetBytesSent();
		currentReceivedBytes = GetBytesReceived();
		bytesSentPerSecond = currentSentBytes - lastSentBytes;
		bytesReceivedPerSecond = currentReceivedBytes - lastReceivedBytes;
		lastSentBytes=currentSentBytes;
		lastReceivedBytes=currentReceivedBytes;
	}
	*/

	return true;
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#ifdef _WIN32
unsigned __stdcall UpdateNetworkLoop(LPVOID arguments)
#else
void*  UpdateNetworkLoop( void*  arguments )
#endif
{
	RakPeer *rakPeer = (RakPeer *)arguments;

#ifdef __USE_IO_COMPLETION_PORTS
	AsynchronousFileIO::Instance()->IncreaseUserCount();
#endif

	rakPeer->isMainLoopThreadActive=true;

	while(rakPeer->endThreads==false)
	{

		if (rakPeer->RunUpdateCycle()==false)
			break;

		// Context switch so other threads can run
		if (rakPeer->threadSleepTimer>=0)
		{
	#ifdef _WIN32
			Sleep(rakPeer->threadSleepTimer);
	#else
			usleep(rakPeer->threadSleepTimer * 1000);
	#endif
		}
//		else if (rakPeer->threadSleepTimer==1)
//		{
//	#ifdef _WIN32
//			Sleep(0);
//	#else
//			usleep(0 * 1000);
//	#endif
//		}

	}
	rakPeer->isMainLoopThreadActive=false;

#ifdef __USE_IO_COMPLETION_PORTS
	AsynchronousFileIO::Instance()->DecreaseUserCount();
#endif


	return 0;
}
