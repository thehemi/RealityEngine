// This file is part of RakNet Copyright 2003, 2004 Rakkarsoft LLC and Kevin Jenkins.

// Usage of Raknet is subject to the appropriate licence agreement.
// "Shareware" Licensees with Rakkarsoft LLC are subject to the shareware license found at http://www.rakkarsoft.com/shareWareLicense.html which you agreed to upon purchase of a "Shareware license"
// "Commercial" Licensees with Rakkarsoft LLC are subject to the commercial license found at http://www.rakkarsoft.com/sourceCodeLicense.html which you agreed to upon purchase of a "Commercial license"
// All other users are subject to the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

// Refer to the appropriate license agreement for distribution, modification, and warranty rights.

#include "ReliabilityLayer.h"
#include <assert.h>
#include "GetTime.h"
#include "SocketLayer.h"

// alloca
#ifdef _WIN32
#include <malloc.h>
#else
#include <stdlib.h>
#endif

//#include "MemoryManager.h"

// Defined in rand.cpp
extern void seedMT(unsigned long seed);
extern inline unsigned long randomMT(void);
extern inline float frandomMT(void);

static const int ACK_BIT_LENGTH=sizeof(PacketNumberType)*8+1;
static const int MAXIMUM_WINDOW_SIZE=(DEFAULT_MTU_SIZE-UDP_HEADER_SIZE)*8/ACK_BIT_LENGTH; // Sanity check - the most ack packets that could ever (usually) fit into a frame.
static const int MINIMUM_WINDOW_SIZE=5; // how many packets can be sent unacknowledged before waiting for an ack

#ifdef _INTERNET_SIMULATOR
// Lag
struct DataAndTime
{
	char data[2000];
	int length;
	unsigned long sendTime;
};
static BasicDataStructures::List<DataAndTime*> delayList;
#endif

//-------------------------------------------------------------------------------------------------------
// Constructor
//-------------------------------------------------------------------------------------------------------
ReliabilityLayer::ReliabilityLayer() : updateBitStream(MAXIMUM_MTU_SIZE) // preallocate the update bitstream so we can avoid a lot of reallocs at runtime
{
	InitializeVariables();
	#ifdef __USE_IO_COMPLETION_PORTS
	readWriteSocket=INVALID_SOCKET;
	#endif
	freeThreadedMemoryOnNextUpdate=false;
}

//-------------------------------------------------------------------------------------------------------
// Destructor
//-------------------------------------------------------------------------------------------------------
ReliabilityLayer::~ReliabilityLayer()
{
	FreeMemory(true); // Free all memory immediately
	#ifdef __USE_IO_COMPLETION_PORTS
	if (readWriteSocket!=INVALID_SOCKET)
		closesocket(readWriteSocket);
	#endif
}

//-------------------------------------------------------------------------------------------------------
// Resets the layer for reuse
//-------------------------------------------------------------------------------------------------------
void ReliabilityLayer::Reset(void)
{
	FreeMemory(false); // False because Reset can be called by any thread
	InitializeVariables();
}

//-------------------------------------------------------------------------------------------------------
// Sets up encryption
//-------------------------------------------------------------------------------------------------------
void ReliabilityLayer::SetEncryptionKey(const unsigned char* key)
{
	if (key)
		encryptor.SetKey(key);
	else
		encryptor.UnsetKey();
}

//-------------------------------------------------------------------------------------------------------
// Assign a socket for the reliability layer to use for writing
//-------------------------------------------------------------------------------------------------------
void ReliabilityLayer::SetSocket(SOCKET s)
{
	#ifdef __USE_IO_COMPLETION_PORTS
	// If this hits I am probably using sequential ports while doing IO completion ports
	assert(s!=INVALID_SOCKET);
	readWriteSocket=s;
	#endif
}

//-------------------------------------------------------------------------------------------------------
// Get the socket held by the reliability layer
//-------------------------------------------------------------------------------------------------------
SOCKET ReliabilityLayer::GetSocket(void)
{
	#ifdef __USE_IO_COMPLETION_PORTS
	return readWriteSocket;
	#else
	return INVALID_SOCKET;
	#endif
}

//-------------------------------------------------------------------------------------------------------
// Initialize the variables
//-------------------------------------------------------------------------------------------------------
void ReliabilityLayer::InitializeVariables(void)
{
	memset(waitingForOrderedPacketReadIndex, 0, NUMBER_OF_ORDERED_STREAMS);
	memset(waitingForSequencedPacketReadIndex, 0, NUMBER_OF_ORDERED_STREAMS);
	memset(waitingForOrderedPacketWriteIndex, 0, NUMBER_OF_ORDERED_STREAMS);
	memset(waitingForSequencedPacketWriteIndex, 0, NUMBER_OF_ORDERED_STREAMS);
	memset(receivedPackets, 0, RECEIVED_PACKET_LOG_LENGTH * sizeof(unsigned long));
	memset(&statistics, 0, sizeof(statistics));
	statistics.connectionStartTime=RakNetGetTime();
	splitPacketId=0L;
	packetNumber=0;
//	lastPacketSendTime=retransmittedFrames=sentPackets=sentFrames=receivedPacketsCount=bytesSent=bytesReceived=0;
	SetLostPacketResendDelay(1000L);
	deadConnection=cheater=false;
	lastAckTime=0;
	blockWindowIncreaseUntilTime=0;
	// Windowing
	windowSize=MINIMUM_WINDOW_SIZE;
	lossyWindowSize=MAXIMUM_WINDOW_SIZE+1; // Infinite
	lastWindowIncreaseSizeTime=0;
//	lastPacketReceivedTime=0;
}

//-------------------------------------------------------------------------------------------------------
// Frees all allocated memory
//-------------------------------------------------------------------------------------------------------
void ReliabilityLayer::FreeMemory(bool freeAllImmediately)
{
	if (freeAllImmediately)
	{
		FreeThreadedMemory();
		FreeThreadSafeMemory();		
	}
	else
	{
		FreeThreadSafeMemory();
		freeThreadedMemoryOnNextUpdate=true;
	}
	
}
void ReliabilityLayer::FreeThreadedMemory(void)
{
	unsigned i;
	InternalPacket *internalPacket;

	//	if (bytesSent > 0 || bytesReceived > 0)
	//	{
	//reliabilityLayerMutexes[splitPacketList_MUTEX].Lock();
	for (i=0; i < splitPacketList.size(); i++)
	{
		delete [] splitPacketList[i]->data;
		InternalPacketPool::Instance()->ReleasePointer(splitPacketList[i]);
	}
	splitPacketList.clear();
	//reliabilityLayerMutexes[splitPacketList_MUTEX].Unlock();
	//	}


	//	if (bytesSent > 0 || bytesReceived > 0)
	//	{
	//reliabilityLayerMutexes[outputQueue_MUTEX].Lock();
	while (outputQueue.size()>0)
	{
		internalPacket= outputQueue.pop();
		delete [] internalPacket->data;
		InternalPacketPool::Instance()->ReleasePointer(internalPacket);
	}
	outputQueue.clearAndForceAllocation(512);
	//reliabilityLayerMutexes[outputQueue_MUTEX].Unlock();
	//	}

	//	if (bytesSent > 0 || bytesReceived > 0)
	//	{
	//reliabilityLayerMutexes[orderingList_MUTEX].Lock();
	for (i=0; i < orderingList.size(); i++)
	{
		if (orderingList[i])
		{
			BasicDataStructures::LinkedList<InternalPacket*>* theList = orderingList[i];

			if (theList)
			{
				while (theList->size())
				{
					internalPacket = orderingList[i]->pop();
					delete [] internalPacket->data;
					InternalPacketPool::Instance()->ReleasePointer(internalPacket);
				}

				delete theList;
			}
		}
	}
	orderingList.clear();
	//reliabilityLayerMutexes[orderingList_MUTEX].Unlock();
	//	}

	//	if (bytesSent > 0 || bytesReceived > 0)
	//	{
	//reliabilityLayerMutexes[acknowledgementQueue_MUTEX].Lock();
	while (acknowledgementQueue.size()>0)
		InternalPacketPool::Instance()->ReleasePointer(acknowledgementQueue.pop());
	acknowledgementQueue.clearAndForceAllocation(64);
	//reliabilityLayerMutexes[acknowledgementQueue_MUTEX].Unlock();
	//	}


	//	if (bytesSent > 0 || bytesReceived > 0)
	//	{
	//reliabilityLayerMutexes[resendQueue_MUTEX].Lock();
	while (resendQueue.size())
	{
		// The resend Queue can have NULL pointer holes.  This is so we can deallocate blocks without having to compress the array
		internalPacket= resendQueue.pop();
		if (internalPacket)
		{
			delete [] internalPacket->data;
			InternalPacketPool::Instance()->ReleasePointer(internalPacket);
		}
	}
	resendQueue.clearAndForceAllocation(512);
	//reliabilityLayerMutexes[resendQueue_MUTEX].Unlock();
	//	}

	

}
void ReliabilityLayer::FreeThreadSafeMemory(void)
{
	unsigned i,j;
//	InternalPacket *internalPacket;

//	if (bytesSent > 0 || bytesReceived > 0)
//	{
		
		for (i=0; i < NUMBER_OF_PRIORITIES; i++)
		{
			j=0;
			reliabilityLayerMutexes[sendQueueSystemPriority_MUTEX+i].Lock();
			for (; j < sendQueue[i].size(); j++)
			{
				delete [] (sendQueue[i])[j]->data;
				InternalPacketPool::Instance()->ReleasePointer((sendQueue[i])[j]);
			}
			sendQueue[i].clearAndForceAllocation(512); // Preallocate the send lists so we don't do a bunch of reallocations unnecessarily
			reliabilityLayerMutexes[sendQueueSystemPriority_MUTEX+i].Unlock();
		}
		
//	}


#ifdef _INTERNET_SIMULATOR
	for (i=0; i < delayList.size(); i++)
		delete delayList[i];
	delayList.clear();
#endif

}

//-------------------------------------------------------------------------------------------------------
// Packets are read directly from the socket layer and skip the reliability 
//layer  because unconnected players do not use the reliability layer
// This function takes packet data after a player has been confirmed as 
//connected.  The game should not use that data directly
// because some data is used internally, such as packet acknowledgement and 
//split packets
//-------------------------------------------------------------------------------------------------------
bool ReliabilityLayer::HandleSocketReceiveFromConnectedPlayer(char *buffer, int length)
{
	#ifdef _DEBUG
	assert(!(length <=0 || buffer==0));
	#endif
	if (length <=1 || buffer==0) // Length of 1 is a connection request resend that we just ignore
		return true;

	int numberOfAcksInFrame=0;
	unsigned long time;
	bool pushedPacket;
	int count;

//	bytesReceived+=length + UDP_HEADER_SIZE;
	
	

	// decode this whole chunk if the decoder is defined.
	if (encryptor.IsKeySet())
	{
		if (encryptor.Decrypt((unsigned char*)buffer, length, (unsigned char*)buffer, &length)==false)
		{
			statistics.bitsWithBadCRCReceived+=length*8;
			statistics.packetsWithBadCRCRecieved++;
			return false;
		}
	}

	statistics.bitsReceived+=length*8;
	statistics.packetsReceived++;

	RakNet::BitStream socketData(buffer, length, false); // Convert the incoming data to a bitstream for easy parsing

//	time = lastPacketReceivedTime = RakNetGetTime();
	time=RakNetGetTime();

	//printf("In HandleSocketReceiveFromConnectedPlayer %i bytes\n",length);
	//for (int ass=0; ass < length && ass < 10; ass++)
	//	printf("%i ", ((char*)(socketData.GetData()))[ass]);
	//printf("\n\n");

	// Parse the bitstream to create an internal packet
	InternalPacket* internalPacket = CreateInternalPacketFromBitStream(&socketData,time);

	while (internalPacket)
	{
		if (internalPacket->isAcknowledgement)
		{
		//	printf("Got ack at %i\n", RakNetGetTime());

			numberOfAcksInFrame++;
			//reliabilityLayerMutexes[resendQueue_MUTEX].Lock();
			if (resendQueue.size()==0)
			{
				//reliabilityLayerMutexes[resendQueue_MUTEX].Unlock();				
				//reliabilityLayerMutexes[lastAckTime_MUTEX].Lock();
				lastAckTime=0;  // Not resending anything so clear this var so we don't drop the connection on not getting any more acks
				//reliabilityLayerMutexes[lastAckTime_MUTEX].Unlock();
			}
			else
			{
				//reliabilityLayerMutexes[resendQueue_MUTEX].Unlock();

				//reliabilityLayerMutexes[lastAckTime_MUTEX].Lock();
				lastAckTime=time; // Just got an ack.  Record when we got it so we know the connection is alive
				//reliabilityLayerMutexes[lastAckTime_MUTEX].Unlock();
			}

			// SHOW - ack received
			// printf("Got Ack. resendQueue.size()=%i sendQueue[0].size() = %i\n",resendQueue.size(), sendQueue[0].size());

			RemovePacketFromResendQueueAndDeleteOlderReliableSequenced(internalPacket->packetNumber);
			InternalPacketPool::Instance()->ReleasePointer(internalPacket);
		}
		else
		{
//			receivedPacketsCount++;

			if (internalPacket->reliability==RELIABLE_SEQUENCED || internalPacket->reliability==RELIABLE_ORDERED || internalPacket->reliability==RELIABLE)
			{
				SendAcknowledgementPacket(internalPacket->packetNumber, time);
			}

			// If this packet number was recently used then it has already been received.
			// If the timer was very long ago then I am just reusing this packet number
			// NUMBER_OF_TRIES in this case is where I am guessing any packet would 
			// make it through within NUMBER_OF_TRIES tries. If the last time this packet 
			// number was used is older than that
			// then it must be a different packet
			if (internalPacket->packetNumber >= RECEIVED_PACKET_LOG_LENGTH)
			{
				statistics.invalidMessagesReceived++;

				delete [] internalPacket->data;
				InternalPacketPool::Instance()->ReleasePointer(internalPacket);
				#ifdef _DEBUG
				printf("Error!! internalPacket->packetNumber >= RECEIVED_PACKET_LOG_LENGTH\n");
				#endif
				return true;
			}

			// testing
	//		if (internalPacket->reliability==UNRELIABLE)
	//			printf("Got unreliable packet number %i\n", internalPacket->packetNumber);
	//		else if (internalPacket->reliability==RELIABLE_SEQUENCED || internalPacket->reliability==RELIABLE_ORDERED || internalPacket->reliability==RELIABLE)
	//			printf("Got reliable packet number %i\n", internalPacket->packetNumber);

			if (receivedPackets[internalPacket->packetNumber] > time - TIMEOUT_TIME) 
				// I can receive RECEIVED_PACKET_LOG_LENGTH packets per TIMEOUT_TIME seconds before overrun on timestamps
			{
				// SHOW - duplicate packets
#ifdef _DEBUG
				// printf("Warning - got duplicate packet (%i).  Did RECEIVED_PACKET_LOG_LENGTH overrun?\n",internalPacket->packetNumber);
				// testing
				//printf("Got duplicate packet\n");
#endif

				statistics.duplicateMessagesReceived++;

				// Duplicate packet
				delete [] internalPacket->data;
				InternalPacketPool::Instance()->ReleasePointer(internalPacket);
				goto CONTINUE_SOCKET_DATA_PARSE_LOOP;
			}


			statistics.messagesReceived++;

			// Record that this particular packet was received at this time
			receivedPackets[internalPacket->packetNumber]=time;

			if (internalPacket->reliability==RELIABLE_SEQUENCED || internalPacket->reliability==UNRELIABLE_SEQUENCED)
			{
				#ifdef _DEBUG
				assert(internalPacket->orderingStream < NUMBER_OF_ORDERED_STREAMS);
				#endif
				if (internalPacket->orderingStream >= NUMBER_OF_ORDERED_STREAMS)
				{
					// Invalid packet
#ifdef _DEBUG
					printf("Got invalid packet\n");
#endif
					delete [] internalPacket->data;
					InternalPacketPool::Instance()->ReleasePointer(internalPacket);
					goto CONTINUE_SOCKET_DATA_PARSE_LOOP;
				}

				if (IsOlderOrderedPacket(internalPacket->orderingIndex, waitingForSequencedPacketReadIndex[internalPacket->orderingStream])==false)
				{
					statistics.sequencedMessagesInOrder++;

					// Check for older packets in the output list.  Delete any found
					// UPDATE:
					// Disabled.  We don't have enough info to consistently do this.  Sometimes newer data does supercede
					// older data such as with constantly declining health, but not in all cases.
					// For example, with sequenced unreliable sound packets just because you send a newer one doesn't mean you
					// don't need the older ones because the odds are they will still arrive in order
					/*
					reliabilityLayerMutexes[outputQueue_MUTEX].Lock();
					DeleteSequencedPacketsInList(internalPacket->orderingStream, outputQueue);
					reliabilityLayerMutexes[outputQueue_MUTEX].Unlock();

					// Check for older packets in the split packet list.  Delete any found
					reliabilityLayerMutexes[splitPacketList_MUTEX].Lock();
					DeleteSequencedPacketsInList(internalPacket->orderingStream, splitPacketList, internalPacket->splitPacketId);
					reliabilityLayerMutexes[splitPacketList_MUTEX].Unlock();
*/
					// Is this a split packet?
					if (internalPacket->splitPacketCount>0)
					{
						// Generate the split
						// Verify some parameters to make sure we don't get junk data
						#ifdef _DEBUG
						assert(internalPacket->splitPacketIndex < internalPacket->splitPacketCount);
						assert(internalPacket->dataBitLength < MAXIMUM_MTU_SIZE*8);

						//reliabilityLayerMutexes[splitPacketList_MUTEX].Lock();
						// Make sure this is not a duplicate insertion.  If this assert hits then splitPacketId overflowed into existing waiting split packets (i.e. more than rangeof(splitPacketId) waiting) 
						for (unsigned cnt=0; cnt < splitPacketList.size(); cnt++)
							assert (!(splitPacketList[cnt]->splitPacketIndex==internalPacket->splitPacketIndex && splitPacketList[cnt]->splitPacketId==splitPacketId));
						int splitPacketListSize = splitPacketList.size()+1;
						//reliabilityLayerMutexes[splitPacketList_MUTEX].Unlock();
						#endif

						// Check for a rebuilt packet
						InsertIntoSplitPacketList(internalPacket);

						// Sequenced
						internalPacket = BuildPacketFromSplitPacketList(internalPacket->splitPacketId,time);

						if (internalPacket)
						{
#ifdef _DEBUG
							//reliabilityLayerMutexes[splitPacketList_MUTEX].Lock();
							assert(splitPacketList.size() == splitPacketListSize-internalPacket->splitPacketCount );
							//reliabilityLayerMutexes[splitPacketList_MUTEX].Unlock();
#endif
							// Update our index to the newest packet
							waitingForSequencedPacketReadIndex[internalPacket->orderingStream]=internalPacket->orderingIndex+1;

							// If there is a rebuilt packet, add it to the output queue
//							reliabilityLayerMutexes[outputQueue_MUTEX].Lock();
							outputQueue.push(internalPacket);
//							reliabilityLayerMutexes[outputQueue_MUTEX].Unlock();
							internalPacket=0;
						}
#ifdef _DEBUG
						else
						{
							//reliabilityLayerMutexes[splitPacketList_MUTEX].Lock();
							assert(splitPacketList.size() == splitPacketListSize );
							//reliabilityLayerMutexes[splitPacketList_MUTEX].Unlock();
						}
#endif
						// else don't have all the parts yet
					}
					else
					{
						// Update our index to the newest packet
						waitingForSequencedPacketReadIndex[internalPacket->orderingStream]=internalPacket->orderingIndex+1;

						// Not a split packet. Add the packet to the output queue
//						reliabilityLayerMutexes[outputQueue_MUTEX].Lock();
						outputQueue.push(internalPacket);
//						reliabilityLayerMutexes[outputQueue_MUTEX].Unlock();
						internalPacket=0;
					}
				}
				else
				{
					statistics.sequencedMessagesOutOfOrder++;
					
					// Older sequenced packet.  Discard it
					delete [] internalPacket->data;
					InternalPacketPool::Instance()->ReleasePointer(internalPacket);
				}

				goto CONTINUE_SOCKET_DATA_PARSE_LOOP;
			}

			// Is this an unsequenced split packet?
			if (internalPacket->splitPacketCount>0)
			{
				// An unsequenced split packet.  May be ordered though.
				
				// Verify some parameters to make sure we don't get junk data
				#ifdef _DEBUG
				assert(internalPacket->splitPacketIndex < internalPacket->splitPacketCount);
				assert(internalPacket->dataBitLength < MAXIMUM_MTU_SIZE*8);
				#endif

				// Check for a rebuilt packet
				if (internalPacket->reliability!=RELIABLE_ORDERED)
					internalPacket->orderingStream=255; // Use 255 to designate not sequenced and not ordered

#ifdef _DEBUG
				//reliabilityLayerMutexes[splitPacketList_MUTEX].Lock();
				// Make sure this is not a duplicate insertion.  If this assert hits then splitPacketId overflowed into existing waiting split packets (i.e. more than rangeof(splitPacketId) waiting) 
				for (unsigned cnt=0; cnt < splitPacketList.size(); cnt++)
					assert (!(splitPacketList[cnt]->splitPacketIndex==internalPacket->splitPacketIndex && splitPacketList[cnt]->splitPacketId==internalPacket->splitPacketId));
				int splitPacketListSize = splitPacketList.size()+1;
				//reliabilityLayerMutexes[splitPacketList_MUTEX].Unlock();
#endif
				InsertIntoSplitPacketList(internalPacket);

				internalPacket = BuildPacketFromSplitPacketList(internalPacket->splitPacketId,time);

				if (internalPacket==0)
				{
					#ifdef _DEBUG
					//reliabilityLayerMutexes[splitPacketList_MUTEX].Lock();
					assert(splitPacketList.size() == splitPacketListSize );
					//reliabilityLayerMutexes[splitPacketList_MUTEX].Unlock();
					#endif

					// Don't have all the parts yet
					goto CONTINUE_SOCKET_DATA_PARSE_LOOP;
				}
				#ifdef _DEBUG
				else
				{
					//reliabilityLayerMutexes[splitPacketList_MUTEX].Lock();
					assert(splitPacketList.size() == splitPacketListSize-internalPacket->splitPacketCount );
					//reliabilityLayerMutexes[splitPacketList_MUTEX].Unlock();
				}
				#endif
				// else continue down to handle RELIABLE_ORDERED
			}

			if (internalPacket->reliability==RELIABLE_ORDERED)
			{
				#ifdef _DEBUG
				assert(internalPacket->orderingStream < NUMBER_OF_ORDERED_STREAMS);
				#endif
				if (internalPacket->orderingStream >= NUMBER_OF_ORDERED_STREAMS)
				{
					// Invalid packet
					delete [] internalPacket->data;
					InternalPacketPool::Instance()->ReleasePointer(internalPacket);
					goto CONTINUE_SOCKET_DATA_PARSE_LOOP;
				}

				if (waitingForOrderedPacketReadIndex[internalPacket->orderingStream]==internalPacket->orderingIndex)
				{
					// Get the list to hold ordered packets for this stream
					BasicDataStructures::LinkedList<InternalPacket*> *orderingListAtOrderingStream;
					unsigned char orderingStreamCopy=internalPacket->orderingStream;

					statistics.orderedMessagesInOrder++;

					// Push the packet for the user to read
//					reliabilityLayerMutexes[outputQueue_MUTEX].Lock();
					outputQueue.push(internalPacket);
//					reliabilityLayerMutexes[outputQueue_MUTEX].Unlock();
					internalPacket=0; // Don't reference this any longer since other threads access it

					// Wait for the next ordered packet in sequence
					waitingForOrderedPacketReadIndex[orderingStreamCopy]++; // This wraps at 255

					//reliabilityLayerMutexes[orderingList_MUTEX].Lock();
					orderingListAtOrderingStream = GetOrderingListAtOrderingStream(orderingStreamCopy);

					if (orderingListAtOrderingStream!=0)
					{
						// There is a list for this ordering stream that may contain waiting packets.  Pop them in order

						// Scan the list and pop packets that were delayed due to ordering in order
						while (orderingListAtOrderingStream->size()>0)
						{
							orderingListAtOrderingStream->beginning();
							count=orderingListAtOrderingStream->size();
							pushedPacket=false;
							while (count!=0)
							{
								if (orderingListAtOrderingStream->peek()->orderingIndex == waitingForOrderedPacketReadIndex[orderingStreamCopy])
								{
									// A packet was waiting, so output it in order.
									// This is already mutexed above

									// Push the packet for the user to read
//									reliabilityLayerMutexes[outputQueue_MUTEX].Lock();
									outputQueue.push(orderingListAtOrderingStream->pop());
//									reliabilityLayerMutexes[outputQueue_MUTEX].Unlock();

									// Wait for the next ordered packet in sequence
									waitingForOrderedPacketReadIndex[orderingStreamCopy]++; // This wraps at 255

									pushedPacket=true;

									break;
								}

								(*orderingListAtOrderingStream)++;
								count--;
							}

							if (pushedPacket==false)
							{
								break; // No more packets to pop
							}
						}

					}
					//reliabilityLayerMutexes[orderingList_MUTEX].Unlock();

					internalPacket=0;
				}
				else
				{
					statistics.orderedMessagesOutOfOrder++;

					// This is a newer ordered packet than we are waiting for.  Store it for future use
					AddToOrderingList(internalPacket);
				}

				goto CONTINUE_SOCKET_DATA_PARSE_LOOP;
			}

			// Nothing special about this packet.  Add it to the output queue
//			reliabilityLayerMutexes[outputQueue_MUTEX].Lock();
			outputQueue.push(internalPacket); 
//			reliabilityLayerMutexes[outputQueue_MUTEX].Unlock();

			// Output queue fill rate test
//			if (outputQueue.size()%50==0)
//				printf("outputQueue.size()=%i Time=%i\n", outputQueue.size(), RakNetGetTime());

			internalPacket=0;
		}

		// Used for a goto to jump to the next packet immediately
		CONTINUE_SOCKET_DATA_PARSE_LOOP:
		// Parse the bitstream to create an internal packet
		internalPacket = CreateInternalPacketFromBitStream(&socketData,time);
	}


	// numberOfAcksInFrame>=windowSize is almost never true
	if (numberOfAcksInFrame>=windowSize && (sendQueue[SYSTEM_PRIORITY].size()>0 || sendQueue[HIGH_PRIORITY].size()>0 || sendQueue[MEDIUM_PRIORITY].size()>0))
	{

	//	reliabilityLayerMutexes[windowSize_MUTEX].Lock();
		if (windowSize < lossyWindowSize || time - lastWindowIncreaseSizeTime > lostPacketResendDelay*2) // Increases the window size slowly, testing for packetloss
		{
			// If we get a frame which clears out the resend queue after handling one or more acks, and we have packets waiting to go out,
			// and we didn't recently lose a packet then increase the window size by 1
			windowSize++;

			if (time - lastWindowIncreaseSizeTime > lostPacketResendDelay*2) // The increase is to test for packetloss
				lastWindowIncreaseSizeTime=time;

			// If the window is so large that we couldn't possibly fit any more packets into the frame, then just leave it alone
			if (windowSize > MAXIMUM_WINDOW_SIZE)
				windowSize=MAXIMUM_WINDOW_SIZE;
			// SHOW - WINDOWING
			//else
			//	printf("Increasing windowSize to %i.  Lossy window size = %i\n", windowSize, lossyWindowSize);

			// If we are more than 5 over the lossy window size, increase the lossy window size by 1
			if (windowSize==MAXIMUM_WINDOW_SIZE || windowSize - lossyWindowSize > 5)
				lossyWindowSize++;
		}
	//	reliabilityLayerMutexes[windowSize_MUTEX].Unlock();
	}

	return true;
}

//-------------------------------------------------------------------------------------------------------
// This gets an end-user packet already parsed out. Returns number of BITS put into the buffer
//-------------------------------------------------------------------------------------------------------
int ReliabilityLayer::Receive(char **data)
{
	InternalPacket* internalPacket;

//	reliabilityLayerMutexes[outputQueue_MUTEX].Lock();
	if (outputQueue.size()>0)
	{
		//		#ifdef _DEBUG
		//		assert(bitStream->GetNumberOfBitsUsed()==0);
		//		#endif
		internalPacket = outputQueue.pop();
//		reliabilityLayerMutexes[outputQueue_MUTEX].Unlock();

		//printf("In Receive %i bytes\n",(internalPacket->dataBitLength-1)/8+1);
		//for (int ass=0; ass < (internalPacket->dataBitLength-1)/8+1 && ass < 10; ass++)
		//	printf("%i ", ((char*)(internalPacket->data))[ass]);
		//printf("\n\n");

		//bitStream->SetData((const unsigned char*)internalPacket->data,internalPacket->dataBitLength);
		//delete [] internalPacket->data;
		int bitLength;
		*data = internalPacket->data;
		bitLength = internalPacket->dataBitLength;
		InternalPacketPool::Instance()->ReleasePointer(internalPacket);
		return bitLength;
	}
	else
	{
	//	reliabilityLayerMutexes[outputQueue_MUTEX].Unlock();
		return 0;
	}
	
}

//-------------------------------------------------------------------------------------------------------
// Puts data on the send queue
// bitStream contains the data to send
// priority is what priority to send the data at
// reliability is what reliability to use
// ordering stream is from 0 to 255 and specifies what stream to use
//-------------------------------------------------------------------------------------------------------
bool ReliabilityLayer::Send(RakNet::BitStream *bitStream, PacketPriority priority, PacketReliability reliability, unsigned char orderingStream, bool makeDataCopy, int MTUSize)
{
	#ifdef _DEBUG
	assert(!(reliability > RELIABLE_SEQUENCED || reliability < 0));
	assert(!(priority > NUMBER_OF_PRIORITIES || priority < 0));
	assert(!(orderingStream < 0 || orderingStream >= NUMBER_OF_ORDERED_STREAMS));
	assert(bitStream->GetNumberOfBytesUsed()>0);
	#endif

	#ifdef __USE_IO_COMPLETION_PORTS
	if (readWriteSocket==INVALID_SOCKET)
		return false;
	#endif

	// Fix any bad parameters
	if (reliability > RELIABLE_SEQUENCED || reliability < 0)
		reliability = RELIABLE;

	if (priority > NUMBER_OF_PRIORITIES || priority < 0)
		priority = HIGH_PRIORITY;

	if (orderingStream >= NUMBER_OF_ORDERED_STREAMS)
		orderingStream=0;

	if (bitStream->GetNumberOfBytesUsed()==0)
	{
#ifdef _DEBUG
		printf("Error!! ReliabilityLayer::Send bitStream->GetNumberOfBytesUsed()==0\n");
#endif
		return false;
	}

	InternalPacket * internalPacket = InternalPacketPool::Instance()->GetPointer();
#ifdef _DEBUG
	// Remove boundschecker accessing undefined memory error
	memset(internalPacket, 255, sizeof(InternalPacket));
#endif

	internalPacket->creationTime=RakNetGetTime();
	if (makeDataCopy)
	{
		internalPacket->data = new char [bitStream->GetNumberOfBytesUsed()];
		memcpy(internalPacket->data, bitStream->GetData(), bitStream->GetNumberOfBytesUsed());
	}
	else
		// Allocate the data elsewhere, delete it in here
		internalPacket->data=(char*)bitStream->GetData();
	internalPacket->dataBitLength=bitStream->GetNumberOfBitsUsed();
	internalPacket->isAcknowledgement=false;
	internalPacket->nextActionTime=0;
	reliabilityLayerMutexes[packetNumber_MUTEX].Lock();
	internalPacket->packetNumber=packetNumber;
	reliabilityLayerMutexes[packetNumber_MUTEX].Unlock();
	internalPacket->priority=priority;
	internalPacket->reliability=reliability;
	internalPacket->splitPacketCount=0;

	// Calculate if I need to split the packet
	int headerLength = BITS_TO_BYTES(GetBitStreamHeaderLength(internalPacket));
	int maxDataSize = MTUSize - UDP_HEADER_SIZE - headerLength;
	if (encryptor.IsKeySet())
		maxDataSize-=16; // Extra data for the encryptor
	bool splitPacket = bitStream->GetNumberOfBytesUsed() > maxDataSize;

	// If a split packet, we might have to upgrade the reliability
	if (splitPacket)
	{
		statistics.numberOfSplitMessages++;
		// Split packets must be sent reliably.  Otherwise not all the parts might arrive
		if (internalPacket->reliability==UNRELIABLE)
			internalPacket->reliability=RELIABLE;
		else if (internalPacket->reliability==UNRELIABLE_SEQUENCED)
			internalPacket->reliability=RELIABLE_SEQUENCED;
	}
	else
		statistics.numberOfUnsplitMessages++;

	// Increment the cyclical receivedPacketsIndex for use by the next packet.
	// This variable is used as the identifier of the packet on the remote machine.
	// When it cycles it will reuse older numbers but that is ok because by the time it
	// cycles those older packets will be pretty much guaranteed to arrive by then
	reliabilityLayerMutexes[packetNumber_MUTEX].Lock();
	if (++packetNumber==RECEIVED_PACKET_LOG_LENGTH)
		packetNumber=0;
	reliabilityLayerMutexes[packetNumber_MUTEX].Unlock();

	if (internalPacket->reliability==RELIABLE_SEQUENCED || internalPacket->reliability==UNRELIABLE_SEQUENCED)
	{
		// Assign the sequence stream and index
		internalPacket->orderingStream=orderingStream;
		reliabilityLayerMutexes[waitingForSequencedPacketWriteIndex_MUTEX].Lock();
		internalPacket->orderingIndex=waitingForSequencedPacketWriteIndex[orderingStream]++;
		reliabilityLayerMutexes[waitingForSequencedPacketWriteIndex_MUTEX].Unlock();

		// This packet supercedes all other sequenced packets on the same ordering stream
		// Delete all packets in all send lists that are sequenced and on the same ordering stream
		// UPDATE:
		// Disabled.  We don't have enough info to consistently do this.  Sometimes newer data does supercede
		// older data such as with constantly declining health, but not in all cases.
		// For example, with sequenced unreliable sound packets just because you send a newer one doesn't mean you
		// don't need the older ones because the odds are they will still arrive in order
		/*
		for (int i=0; i < NUMBER_OF_PRIORITIES; i++)
		{
			reliabilityLayerMutexes[sendQueue_MUTEX].Lock();
			DeleteSequencedPacketsInList(orderingStream, sendQueue[i]);
			reliabilityLayerMutexes[sendQueue_MUTEX].Unlock();
		}
		*/
	}
	else if (internalPacket->reliability==RELIABLE_ORDERED)
	{
		// Assign the ordering stream and index
		internalPacket->orderingStream=orderingStream;
		reliabilityLayerMutexes[waitingForOrderedPacketWriteIndex_MUTEX].Lock();
		internalPacket->orderingIndex=waitingForOrderedPacketWriteIndex[orderingStream]++;
		reliabilityLayerMutexes[waitingForOrderedPacketWriteIndex_MUTEX].Unlock();
	}

	if (splitPacket) // If it uses a secure header it will be generated here
	{
		// Must split the packet.  This will also generate the SHA1 if it is required.  It also adds it to the send list.
		SplitPacketAndDeleteOriginal(internalPacket, MTUSize);
		return true;
	}
	
	reliabilityLayerMutexes[sendQueueSystemPriority_MUTEX+internalPacket->priority].Lock();
    sendQueue[internalPacket->priority].push(internalPacket);
	reliabilityLayerMutexes[sendQueueSystemPriority_MUTEX+internalPacket->priority].Unlock();

	return true;
}

//-------------------------------------------------------------------------------------------------------
// Run this once per game cycle.  Handles internal lists and actually does the send
//-------------------------------------------------------------------------------------------------------
void ReliabilityLayer::Update(SOCKET s, PlayerID playerId, int MTUSize, unsigned long time)
{
#ifdef __USE_IO_COMPLETION_PORTS
	if (readWriteSocket==INVALID_SOCKET)
		return;
#endif
	//	unsigned resendQueueSize;
	bool reliableDataSent;
	unsigned long lastAck;

	if (freeThreadedMemoryOnNextUpdate)
	{
		freeThreadedMemoryOnNextUpdate=false;
		FreeThreadedMemory();
	}

	// Accuracy isn't important on this value, and since this is called so often the mutex is sometimes causing deadlock problems.
	// So it is presently disabled
//	reliabilityLayerMutexes[resendQueue_MUTEX].Lock();
//	resendQueueSize=resendQueue.size();
//	reliabilityLayerMutexes[resendQueue_MUTEX].Unlock();

//	reliabilityLayerMutexes[lastAckTime_MUTEX].Lock();
	lastAck=lastAckTime;
//	reliabilityLayerMutexes[lastAckTime_MUTEX].Unlock();

	if (resendQueue.size()>0 && lastAck && time - lastAck > TIMEOUT_TIME)
	{
		// SHOW - dead connection
		// printf("The connection has been lost.\n");
		// We've waited a very long time for a reliable packet to get an ack and it never has
		deadConnection=true;
		return;
	}

	//if (outputWindowFullTime && RakNetGetTime() > TIMEOUT_TIME + outputWindowFullTime)
	//{
	//	// We've waited a long time with no data from the other system.  Assume the connection is lost
	//	deadConnection=true;
	//	return;
	//}

	// Not a frame but a packet actually.
	// However, in a sense it is a frame because we are filling multiple logical packets into one datagram
	//reliabilityLayerMutexes[updateBitStream_MUTEX].Lock();

	// Keep sending to available bandwidth
	while (IsFrameReady(time))
	{
		updateBitStream.Reset();
		GenerateFrame(&updateBitStream, MTUSize, &reliableDataSent,time);
		if (updateBitStream.GetNumberOfBitsUsed()>0)
		{
#ifndef _INTERNET_SIMULATOR
			SendBitStream(s, playerId, &updateBitStream);
#else
			// Delay the send to simulate lag
			DataAndTime *dt;
			dt=new DataAndTime;
			memcpy(dt->data, updateBitStream.GetData(), updateBitStream.GetNumberOfBytesUsed());
			dt->length=updateBitStream.GetNumberOfBytesUsed();
			dt->sendTime=time+100+(randomMT()%100);
			delayList.insert(dt);
#endif
		}
		else
			break;
	}
	

#ifdef _INTERNET_SIMULATOR
	// Do any lagged sends
	unsigned i=0;
	while (i < delayList.size())
	{
		if (delayList[i]->sendTime<time)
		{
			updateBitStream.Reset();
			updateBitStream.Write(delayList[i]->data, delayList[i]->length);
			// Send it now
			SendBitStream(s, playerId, &updateBitStream,time);

			delete delayList[i];
			delayList[i]=delayList[delayList.size()-1];
			delayList.del();
		}
		else
			i++;
	}
#endif

	//reliabilityLayerMutexes[updateBitStream_MUTEX].Unlock();
}

//-------------------------------------------------------------------------------------------------------
// Writes a bitstream to the socket
//-------------------------------------------------------------------------------------------------------
void ReliabilityLayer::SendBitStream(SOCKET s, PlayerID playerId, RakNet::BitStream *bitStream)
{
 // SHOW - showing reliable flow
//	if (bitStream->GetNumberOfBytesUsed()>50)
//		printf("Sending %i bytes. sendQueue[0].size()=%i, resendQueue.size()=%i\n", bitStream->GetNumberOfBytesUsed(), sendQueue[0].size(),resendQueue.size());

	int oldLength, length;

//	sentFrames++;

#ifdef _INTERNET_SIMULATOR

	/*
	// packetloss
	if (windowSize>MINIMUM_WINDOW_SIZE && frandomMT() <= (float)(windowSize-MINIMUM_WINDOW_SIZE)/(float)(MAXIMUM_WINDOW_SIZE-MINIMUM_WINDOW_SIZE))
	{
		// printf("Frame %i lost\n", sentFrames);
		lastPacketSendTime=RakNetGetTime();
		return;
	}
	*/
#endif


	// Encode the whole bitstream if the encoder is defined.
	if (encryptor.IsKeySet())
	{
		length = bitStream->GetNumberOfBytesUsed();
		oldLength = length;
		encryptor.Encrypt((unsigned char*)bitStream->GetData(), length, (unsigned char*)bitStream->GetData(), &length);
		statistics.encryptionBitsSent=(length-oldLength)*8;

//		if (encryptor.IsKeySet())
//			bytesSent+=length - oldLength;

		assert((length % 16 )== 0);
	}
	else
	{
		length = bitStream->GetNumberOfBytesUsed();
	}

#ifdef __USE_IO_COMPLETION_PORTS
	if (readWriteSocket==INVALID_SOCKET)
	{
		assert(0);	return;
	}
	statistics.packetsSent++;
	statistics.totalBitsSent+=length*8;
	SocketLayer::Instance()->Write(readWriteSocket, (const char*)bitStream->GetData(), length);
#else
	statistics.packetsSent++;
	statistics.totalBitsSent+=length*8;
	//printf("total bits=%i length=%i\n", BITS_TO_BYTES(statistics.totalBitsSent), length);
    SocketLayer::Instance()->SendTo(s, (char*)bitStream->GetData(), length, playerId.binaryAddress, playerId.port);
#endif // __USE_IO_COMPLETION_PORTS

//	lastPacketSendTime=time;
}

//-------------------------------------------------------------------------------------------------------
// Returns true if we can or should send a frame.  False if we should not
//-------------------------------------------------------------------------------------------------------
bool ReliabilityLayer::IsFrameReady(unsigned long time)
{
	if (IsSendThrottled()==false)
		return true;

	//reliabilityLayerMutexes[acknowledgementQueue_MUTEX].Lock();

	// Any acknowledgement packets waiting?  We will send these even if the send is throttled.
	// Otherwise the throttle may never end
	if (acknowledgementQueue.size() >= MINIMUM_WINDOW_SIZE
		// Try not waiting to send acks - will take more bandwidth but maybe less packetloss
		// || acknowledgementQueue.peek()->nextActionTime < time
		)
	{
		//reliabilityLayerMutexes[acknowledgementQueue_MUTEX].Unlock();
		return true;
	}
	
//	reliabilityLayerMutexes[acknowledgementQueue_MUTEX].Unlock();

	// Does the oldest packet need to be resent?  If so, send it.
	// Otherwise the throttle may never end
//	reliabilityLayerMutexes[resendQueue_MUTEX].Lock();
	if (resendQueue.size()>0 && resendQueue.peek() && resendQueue.peek()->nextActionTime < time)
	{
//		reliabilityLayerMutexes[resendQueue_MUTEX].Unlock();
		return true;
	}
//	reliabilityLayerMutexes[resendQueue_MUTEX].Unlock();

	// Send is throttled.  Don't send.
	return false;
}

//-------------------------------------------------------------------------------------------------------
// Generates a frame (coalesced packets)
//-------------------------------------------------------------------------------------------------------

void ReliabilityLayer::GenerateFrame(RakNet::BitStream *output, int MTUSize, bool *reliableDataSent,unsigned long time)
{
	InternalPacket *internalPacket;
	int maxDataBitSize;
	int reliableBits=0;
	int nextPacketBitLength;
	unsigned i;
	bool isReliable,onlySendUnreliable;
	bool acknowledgementPacketsSent;
	bool anyPacketsLost=false;

	maxDataBitSize = MTUSize - UDP_HEADER_SIZE;
	if (encryptor.IsKeySet())
		maxDataBitSize-=16; // Extra data for the encryptor
	maxDataBitSize<<=3;

	acknowledgementPacketsSent=false;

	*reliableDataSent=false;
	

	// Packet acknowledgements always go out first if they are overdue or if there are a lot of them
	//reliabilityLayerMutexes[acknowledgementQueue_MUTEX].Lock();
	//	reliabilityLayerMutexes[remoteFramesAwaitingAck_MUTEX].Lock();
	if (acknowledgementQueue.size()>0 &&
		(acknowledgementQueue.size() >=MINIMUM_WINDOW_SIZE ||
		acknowledgementQueue.peek()->nextActionTime < time))
	{
		do
		{
			//	reliabilityLayerMutexes[remoteFramesAwaitingAck_MUTEX].Unlock();
			internalPacket = acknowledgementQueue.pop();
			//reliabilityLayerMutexes[acknowledgementQueue_MUTEX].Unlock();

			// Write the acknowledgement to the output bitstream
			statistics.acknowlegementsSent++;
			statistics.acknowlegementBitsSent+=WriteToBitStreamFromInternalPacket(output, internalPacket);
			acknowledgementPacketsSent=true;

			// Delete the acknowledgement
			InternalPacketPool::Instance()->ReleasePointer(internalPacket);

			if (output->GetNumberOfBitsUsed() + ACK_BIT_LENGTH > maxDataBitSize)
			{
				// SHOW - show ack
				//printf("Sending FULL ack (%i) at time %i. acknowledgementQueue.size()=%i\n", output->GetNumberOfBytesUsed(), RakNetGetTime(),acknowledgementQueue.size());

				statistics.packetsContainingOnlyAcknowlegements++;
				goto END_OF_GENERATE_FRAME;
			}

	//		reliabilityLayerMutexes[acknowledgementQueue_MUTEX].Lock();
		} while(acknowledgementQueue.size() > 0);
	}
	//reliabilityLayerMutexes[acknowledgementQueue_MUTEX].Unlock();

	

	// SHOW - show ack
	//if (output->GetNumberOfBitsUsed()>0)
	//	printf("Sending ack (%i) at time %i. acknowledgementQueue.size()=%i\n", output->GetNumberOfBytesUsed(), RakNetGetTime(),acknowledgementQueue.size());

	//reliabilityLayerMutexes[resendQueue_MUTEX].Lock();

	// The resend Queue can have NULL pointer holes.  This is so we can deallocate blocks without having to compress the array
	while (resendQueue.size() > 0)
	{
		if (resendQueue.peek()==0)
		{
			resendQueue.pop();
			continue; // This was a hole
		}

		if (resendQueue.peek()->nextActionTime < time)
		{
			internalPacket = resendQueue.pop();
			//reliabilityLayerMutexes[resendQueue_MUTEX].Unlock();
			// Testing
			//printf("Resending %i. queue size = %i\n", internalPacket->packetNumber, resendQueue.size());

			nextPacketBitLength = GetBitStreamHeaderLength(internalPacket) + internalPacket->dataBitLength;

			if (output->GetNumberOfBitsUsed() + nextPacketBitLength > maxDataBitSize)
			{
				//reliabilityLayerMutexes[resendQueue_MUTEX].Lock();
				resendQueue.pushAtHead(internalPacket); // Not enough room to use this packet after all!
				//reliabilityLayerMutexes[resendQueue_MUTEX].Unlock();

				if (anyPacketsLost)
				{
					UpdatePacketloss(time);
				}

				goto END_OF_GENERATE_FRAME;
			}

#ifdef _DEBUG
			assert(internalPacket->priority>=0);
			assert(internalPacket->reliability>=0);
#endif

			// SHOW - show resends
			//printf("Resending packet. resendQueue.size()=%i. Data=%s\n",resendQueue.size(), internalPacket->data);

			// Write to the output bitstream
//			sentPackets++;
			statistics.messageResends++;
			statistics.messageDataBitsResent+=internalPacket->dataBitLength;

	// TODO - uncomment this
			statistics.messagesTotalBitsResent+=WriteToBitStreamFromInternalPacket(output, internalPacket);
			*reliableDataSent=true;

			//		if (output->GetNumberOfBitsUsed() + ACK_BIT_LENGTH > maxDataBitSize)
			//			printf("Frame full of just acks and resends at time %i.\n", RakNetGetTime());

			statistics.packetsContainingOnlyAcknowlegementsAndResends++;

			anyPacketsLost=true;
			internalPacket->nextActionTime = time + lostPacketResendDelay;

			// Put the packet back into the resend list at the correct spot
			InsertPacketIntoResendQueue(internalPacket, time);
			//reliabilityLayerMutexes[resendQueue_MUTEX].Lock();

		}
		else
		{
			break;
		}
	}
	//reliabilityLayerMutexes[resendQueue_MUTEX].Unlock();

	

	if (anyPacketsLost)
	{
		// Update packetloss
		UpdatePacketloss(time);
	}

	onlySendUnreliable=false;

	if (IsSendThrottled())
		return; // Don't send regular data if we are supposed to be waiting on the window

	// From highest to lowest priority, fill up the output bitstream from the send lists
	for (i=0; i < NUMBER_OF_PRIORITIES; i++)
	{
	//	if (i==LOW_PRIORITY && sendQueue[LOW_PRIORITY].size() > 0 && (sendQueue[LOW_PRIORITY].size()%100)==0)
	//	{
	//		printf("%i\n", sendQueue[LOW_PRIORITY].size());
	//	}

		// Not mutexed - may give a wrong value if another thread is inserting something but it's ok
		// Because we can avoid a slow mutex call a lot of the time
		if (sendQueue[i].size()==0)
			continue;
		reliabilityLayerMutexes[sendQueueSystemPriority_MUTEX+i].Lock();
		while (sendQueue[i].size())
		{
			internalPacket = sendQueue[i].pop();
			reliabilityLayerMutexes[sendQueueSystemPriority_MUTEX+i].Unlock();

			nextPacketBitLength = GetBitStreamHeaderLength(internalPacket) + internalPacket->dataBitLength;

			if (output->GetNumberOfBitsUsed() + nextPacketBitLength > maxDataBitSize)
			{
				// This output won't fit.
				reliabilityLayerMutexes[sendQueueSystemPriority_MUTEX+i].Lock();
				sendQueue[i].pushAtHead(internalPacket); // Push this back at the head so it is the next thing to go out
				break;
			}

			if (internalPacket->reliability==RELIABLE || internalPacket->reliability==RELIABLE_SEQUENCED || internalPacket->reliability==RELIABLE_ORDERED)
				isReliable=true;
			else
				isReliable=false;

			// Write to the output bitstream
//			sentPackets++;
			statistics.messagesSent[i]++;
			statistics.messageDataBitsSent[i]+=internalPacket->dataBitLength;
			statistics.messageTotalBitsSent[i]+=WriteToBitStreamFromInternalPacket(output, internalPacket);
			if (isReliable)
			{
				// Reliable packets are saved to resend later
				reliableBits+=internalPacket->dataBitLength;
				internalPacket->nextActionTime = time + lostPacketResendDelay;
				//printf("Resending at %i\n", internalPacket->nextActionTime);
				InsertPacketIntoResendQueue(internalPacket, time);

				*reliableDataSent=true;
			}
			else
			{
				// Unreliable packets are deleted
				delete [] internalPacket->data;
				InternalPacketPool::Instance()->ReleasePointer(internalPacket);
			}

			reliabilityLayerMutexes[sendQueueSystemPriority_MUTEX+i].Lock();
		}

		reliabilityLayerMutexes[sendQueueSystemPriority_MUTEX+i].Unlock();

	}

	// Optimization - if we sent data but didn't send an acknowledgement packet previously then send them now
	if (acknowledgementPacketsSent==false && output->GetNumberOfBitsUsed()>0)
	{
		if (acknowledgementQueue.size() > 0)
		{
			//reliabilityLayerMutexes[acknowledgementQueue_MUTEX].Lock();
			while(output->GetNumberOfBitsUsed() + ACK_BIT_LENGTH < maxDataBitSize && acknowledgementQueue.size() > 0)
			{
				internalPacket = acknowledgementQueue.pop();
				//reliabilityLayerMutexes[acknowledgementQueue_MUTEX].Unlock();

				// Write the acknowledgement to the output bitstream
				WriteToBitStreamFromInternalPacket(output, internalPacket);

				// Delete the acknowledgement
				InternalPacketPool::Instance()->ReleasePointer(internalPacket);

				//reliabilityLayerMutexes[acknowledgementQueue_MUTEX].Lock();
			} 

			//reliabilityLayerMutexes[acknowledgementQueue_MUTEX].Unlock();
		}
	}

END_OF_GENERATE_FRAME:
;

//	if (output->GetNumberOfBitsUsed()>0)
//	{
		// Update the throttle with the header
//		bytesSent+=output->GetNumberOfBytesUsed() + UDP_HEADER_SIZE;
	//}
}


//-------------------------------------------------------------------------------------------------------
// This will return true if we should not send at this time
//-------------------------------------------------------------------------------------------------------
bool ReliabilityLayer::IsSendThrottled(void)
{
	int win;

//	reliabilityLayerMutexes[windowSize_MUTEX].Lock();
	win = windowSize;
//	reliabilityLayerMutexes[windowSize_MUTEX].Unlock();

	return (int)resendQueue.size() >=win;
}

//-------------------------------------------------------------------------------------------------------
// We lost a packet
//-------------------------------------------------------------------------------------------------------
void ReliabilityLayer::UpdatePacketloss(unsigned long time)
{
//	unsigned long time = RakNetGetTime();
	/*
	maximumWindowSize = (unsigned int)((double)maximumWindowSize * DECREASE_THROUGHPUT_DELTA);
	if (maximumWindowSize < MINIMUM_THROUGHPUT)
	{
		maximumWindowSize = MINIMUM_THROUGHPUT;
	}
	*/

	//printf("Lost packet. resendQueue.size()=%i sendQueue[0].size() = %i\n",resendQueue.size(), sendQueue[0].size());

//	reliabilityLayerMutexes[windowSize_MUTEX].Lock();


//	reliabilityLayerMutexes[windowSize_MUTEX].Unlock();
//	retransmittedFrames++;

	// The window size will decrease everytime we have to retransmit a frame
	//reliabilityLayerMutexes[windowSize_MUTEX].Lock();
	if (--windowSize < MINIMUM_WINDOW_SIZE)
		windowSize=MINIMUM_WINDOW_SIZE;
	//reliabilityLayerMutexes[windowSize_MUTEX].Unlock();
	lossyWindowSize=windowSize;
	lastWindowIncreaseSizeTime=time; // This will block the window size from increasing immediately
	// SHOW - windowing
//	if (resendQueue.size()>0)
//		printf("Frame lost.  New window size = %i.  Lossy window size = %i. Time=%i. Next send time=%i\n", windowSize, lossyWindowSize, RakNetGetTime(),resendQueue.peek()->nextActionTime);
}

//-------------------------------------------------------------------------------------------------------
// Does what the function name says
//-------------------------------------------------------------------------------------------------------
void ReliabilityLayer::RemovePacketFromResendQueueAndDeleteOlderReliableSequenced(PacketNumberType packetNumber)
{
	InternalPacket *internalPacket;
	PacketReliability reliability; // What type of reliability algorithm to use with this packet
	unsigned char orderingStream; // What ordering stream this packet is on, if the reliability type uses ordering streams
	unsigned char orderingIndex; // The ID used as identification for ordering streams

//	reliabilityLayerMutexes[resendQueue_MUTEX].Lock();

	for (unsigned i=0; i < resendQueue.size(); i ++)
	{
		if (resendQueue[i] && packetNumber==resendQueue[i]->packetNumber)
		{
			// Found what we wanted to ack
			statistics.acknowlegementsReceived++;

			if (i==0)
				internalPacket=resendQueue.pop();
			else
			{
				
				// Generate a hole
				internalPacket = resendQueue[i];
				// testing
			//	printf("Removing packet %i from resend\n", internalPacket->packetNumber);
				resendQueue[i]=0;
			}

			//reliabilityLayerMutexes[resendQueue_MUTEX].Unlock();

			// Save some of the data of the packet
			reliability=internalPacket->reliability;
			orderingStream=internalPacket->orderingStream;
			orderingIndex=internalPacket->orderingIndex;

			// Delete the packet
			delete [] internalPacket->data;
			InternalPacketPool::Instance()->ReleasePointer(internalPacket);

			// If the deleted packet was reliable sequenced, also delete all older reliable sequenced resends on the same ordering stream.
			// This is because we no longer need to send these.
			if (reliability==RELIABLE_SEQUENCED)
			{
				unsigned j=0;

				//reliabilityLayerMutexes[resendQueue_MUTEX].Lock();
				while (j < resendQueue.size())
				{
					internalPacket = resendQueue[j];
					if (internalPacket && internalPacket->reliability==RELIABLE_SEQUENCED && internalPacket->orderingStream==orderingStream && IsOlderOrderedPacket(internalPacket->orderingIndex, orderingIndex))
					{
						// Delete the packet
						delete [] internalPacket->data;
						InternalPacketPool::Instance()->ReleasePointer(internalPacket);
						resendQueue[j]=0; // Generate a hole
					}

					j++;
				}
//				reliabilityLayerMutexes[resendQueue_MUTEX].Unlock();
			}

			//reliabilityLayerMutexes[resendQueue_MUTEX].Lock();
			return;
		}
	}

	//reliabilityLayerMutexes[resendQueue_MUTEX].Unlock();

	// Didn't find what we wanted to ack
	statistics.duplicateAcknowlegementsReceived++;
}

//-------------------------------------------------------------------------------------------------------
// Acknowledge receipt of the packet with the specified packetNumber
//-------------------------------------------------------------------------------------------------------
void ReliabilityLayer::SendAcknowledgementPacket(PacketNumberType packetNumber, unsigned long time)
{
	InternalPacket *internalPacket;

	// Disabled - never gets called anyway so just wastes CPU cycles
	/*
	// High load optimization - if there are over 100 acks waiting scan the list to make sure what we are adding isn't already scheduled to go out
	reliabilityLayerMutexes[acknowledgementQueue_MUTEX].Lock();
	size = acknowledgementQueue.size();
	if (size>100)
	{
		for (i=0; i < size; i++)
		{
			internalPacket=acknowledgementQueue[i];
			if (internalPacket && internalPacket->packetNumber==packetNumber)
			{
				reliabilityLayerMutexes[acknowledgementQueue_MUTEX].Unlock();
				//printf("Eliminating duplicate ack. acknowledgementQueue.size()=%i\n",acknowledgementQueue.size());
				return; // No need to add it - it is already here
			}
		}
	}
	reliabilityLayerMutexes[acknowledgementQueue_MUTEX].Unlock();
	*/

	internalPacket = InternalPacketPool::Instance()->GetPointer();
#ifdef _DEBUG
	// Remove boundschecker accessing undefined memory error
	memset(internalPacket, 255, sizeof(InternalPacket));
#endif
	internalPacket->packetNumber=packetNumber;
	internalPacket->isAcknowledgement=true;

	internalPacket->creationTime = time;
	// We send this acknowledgement no later than 1/4 the time the remote 
	//machine would send the original packet again
	// DEBUG
	internalPacket->nextActionTime = internalPacket->creationTime + (lostPacketResendDelay>>2);
	//internalPacket->nextActionTime = internalPacket->creationTime;
	//reliabilityLayerMutexes[acknowledgementQueue_MUTEX].Lock();
	acknowledgementQueue.push(internalPacket);
//	printf("<Server>Adding ack at time %i. acknowledgementQueue.size=%i\n",RakNetGetTime(), acknowledgementQueue.size());
	//reliabilityLayerMutexes[acknowledgementQueue_MUTEX].Unlock();
}

//-------------------------------------------------------------------------------------------------------
// Parse an internalPacket and figure out how many header bits would be 
// written.  Returns that number
//-------------------------------------------------------------------------------------------------------
int ReliabilityLayer::GetBitStreamHeaderLength(const InternalPacket *const internalPacket)
{
	#ifdef _DEBUG
	assert(internalPacket);
	#endif
	int bitLength;

	if (internalPacket->isAcknowledgement)
		return ACK_BIT_LENGTH;

	// Write if this packet has a security header (1 bit)
	//bitStream->Write(internalPacket->hasSecurityHeader);
	// -- bitLength+=1;
	bitLength=ACK_BIT_LENGTH;

	// Write the PacketReliability.  This is encoded in 3 bits
	//bitStream->WriteBits((unsigned char*)&(internalPacket->reliability), 3, true);
	bitLength+=3;

	// If the reliability requires an ordering stream and ordering index, we Write those.
	if (internalPacket->reliability==UNRELIABLE_SEQUENCED || internalPacket->reliability==RELIABLE_SEQUENCED || internalPacket->reliability==RELIABLE_ORDERED)
	{
		// ordering stream encoded in 5 bits (from 0 to 31)
		//bitStream->WriteBits((unsigned char*)&(internalPacket->orderingStream), 5, true);
		// -- bitLength+=5;

		// ordering index is one byte
		//bitStream->Write(internalPacket->orderingIndex);
		// -- bitLength+=8;
		bitLength+=13;
	}

	// Write if this is a split packet (1 bit)
	bool isSplitPacket = internalPacket->splitPacketCount>0;
	//bitStream->Write(isSplitPacket);
	bitLength+=1;
	if (isSplitPacket)
	{
		// split packet indices are two bytes (so one packet can be split up to 65535 
		// times - maximum packet size would be about 500 * 65535)
		//bitStream->Write(internalPacket->splitPacketId);
		//bitStream->WriteCompressed(internalPacket->splitPacketIndex);
		//bitStream->WriteCompressed(internalPacket->splitPacketCount);
		bitLength+=3*8*2;
	}

	// Write how many bits the packet data is.  Stored in an unsigned short and 
	// read from 16 bits
	//bitStream->WriteBits((unsigned char*)&(internalPacket->dataBitLength), 16, true);

	// Read how many bits the packet data is.  Stored in 16 bits
	bitLength+=16;

	// Byte alignment
	//bitLength += 8 - ((bitLength -1) %8 + 1);

	return bitLength;
}

//-------------------------------------------------------------------------------------------------------
// Parse an internalPacket and create a bitstream to represent this data
//-------------------------------------------------------------------------------------------------------
int ReliabilityLayer::WriteToBitStreamFromInternalPacket(RakNet::BitStream *bitStream, const InternalPacket *const internalPacket)
{
	#ifdef _DEBUG
	assert(bitStream && internalPacket);
	#endif

	int start=bitStream->GetNumberOfBitsUsed();

// testing
//	if (internalPacket->reliability==UNRELIABLE)
//		printf("Sending unreliable packet %i\n", internalPacket->packetNumber);
//	else if (internalPacket->reliability==RELIABLE_SEQUENCED || internalPacket->reliability==RELIABLE_ORDERED || internalPacket->reliability==RELIABLE)
//		printf("Sending reliable packet number %i\n", internalPacket->packetNumber);

	//bitStream->AlignWriteToByteBoundary();

	// Write the packet number (2 bytes)
	bitStream->Write(internalPacket->packetNumber);

	// Write if this packet is an acknowledgement (1 bit)
	bitStream->Write(internalPacket->isAcknowledgement);
	// Acknowledgement packets have no more data than the packetnumber and whether it is an acknowledgement
	if (internalPacket->isAcknowledgement)
	{
		return bitStream->GetNumberOfBitsUsed()-start;
	}

	#ifdef _DEBUG
	assert(internalPacket->dataBitLength>0);
	#endif

	// Write the PacketReliability.  This is encoded in 3 bits
	unsigned char reliability = (unsigned char)internalPacket->reliability;
	bitStream->WriteBits((unsigned char*)(&(reliability)), 3, true);

	// If the reliability requires an ordering stream and ordering index, we Write those.
	if (internalPacket->reliability==UNRELIABLE_SEQUENCED || internalPacket->reliability==RELIABLE_SEQUENCED || internalPacket->reliability==RELIABLE_ORDERED)
	{
		// ordering stream encoded in 5 bits (from 0 to 31)
		bitStream->WriteBits((unsigned char*)&(internalPacket->orderingStream), 5, true);

		// ordering index is one byte
		bitStream->Write(internalPacket->orderingIndex);
	}

	// Write if this is a split packet (1 bit)
	bool isSplitPacket = internalPacket->splitPacketCount>0;
	bitStream->Write(isSplitPacket);
	if (isSplitPacket)
	{
		// split packet indices are two bytes (so one packet can be split up to 65535 times - maximum packet size would be about 500 * 65535)
		bitStream->Write(internalPacket->splitPacketId);
		bitStream->WriteCompressed(internalPacket->splitPacketIndex);
		bitStream->WriteCompressed(internalPacket->splitPacketCount);
	}

	// Write how many bits the packet data is.  Stored in 13 bits
	#ifdef _DEBUG
	assert(BITS_TO_BYTES(internalPacket->dataBitLength) < MAXIMUM_MTU_SIZE); // I never send more than MTU_SIZE bytes
	#endif
	unsigned short length = (unsigned short) internalPacket->dataBitLength; // Ignore the 2 high bytes for WriteBits

	bitStream->WriteCompressed(length);

	// Write the actual data.
	bitStream->WriteAlignedBytes((unsigned char*)internalPacket->data, BITS_TO_BYTES(internalPacket->dataBitLength));
	//bitStream->WriteBits((unsigned char*)internalPacket->data, internalPacket->dataBitLength);

	return bitStream->GetNumberOfBitsUsed()-start;
}

//-------------------------------------------------------------------------------------------------------
// Parse a bitstream and create an internal packet to represent this data
//-------------------------------------------------------------------------------------------------------
InternalPacket* ReliabilityLayer::CreateInternalPacketFromBitStream(RakNet::BitStream *bitStream, unsigned long time)
{
	bool bitStreamSucceeded;
	InternalPacket* internalPacket;

	if (bitStream->GetNumberOfUnreadBits() < sizeof(internalPacket->packetNumber)*8)
		return 0; // leftover bits
	
	internalPacket = InternalPacketPool::Instance()->GetPointer();
#ifdef _DEBUG
	// Remove boundschecker accessing undefined memory error
	memset(internalPacket, 255, sizeof(InternalPacket));
#endif

	internalPacket->creationTime=time;

	//bitStream->AlignReadToByteBoundary();

	// Read the packet number (2 bytes)
	bitStreamSucceeded = bitStream->Read(internalPacket->packetNumber);
	#ifdef _DEBUG
	assert(bitStreamSucceeded);
	#endif
	if (bitStreamSucceeded==false) {InternalPacketPool::Instance()->ReleasePointer(internalPacket); return 0;}

	// Read if this packet is an acknowledgement (1 bit)
	bitStreamSucceeded = bitStream->Read(internalPacket->isAcknowledgement);
#ifdef _DEBUG
	assert(bitStreamSucceeded);
#endif
	if (bitStreamSucceeded==false) {InternalPacketPool::Instance()->ReleasePointer(internalPacket); return 0;}
	// Acknowledgement packets have no more data than the packetnumber and whether it is an acknowledgement
	if (internalPacket->isAcknowledgement)
		return internalPacket;

	// Read the PacketReliability.  This is encoded in 3 bits
	unsigned char reliability;
	bitStreamSucceeded = bitStream->ReadBits((unsigned char*)(&(reliability)), 3);
	internalPacket->reliability=(PacketReliability)reliability;
#ifdef _DEBUG
	assert(bitStreamSucceeded);
#endif
	if (bitStreamSucceeded==false) {InternalPacketPool::Instance()->ReleasePointer(internalPacket); return 0;}

	// If the reliability requires an ordering stream and ordering index, we read those.
	if (internalPacket->reliability==UNRELIABLE_SEQUENCED || internalPacket->reliability==RELIABLE_SEQUENCED || internalPacket->reliability==RELIABLE_ORDERED)
	{
		// ordering stream encoded in 5 bits (from 0 to 31)
		bitStreamSucceeded = bitStream->ReadBits((unsigned char*)&(internalPacket->orderingStream), 5);
#ifdef _DEBUG
		assert(bitStreamSucceeded);
#endif
		if (bitStreamSucceeded==false) {InternalPacketPool::Instance()->ReleasePointer(internalPacket); return 0;}

		// ordering index is one byte
		bitStreamSucceeded = bitStream->Read(internalPacket->orderingIndex);
#ifdef _DEBUG
		assert(bitStreamSucceeded);
#endif
		if (bitStreamSucceeded==false) {InternalPacketPool::Instance()->ReleasePointer(internalPacket); return 0;}
	}

	// Read if this is a split packet (1 bit)
	bool isSplitPacket;
	bitStreamSucceeded = bitStream->Read(isSplitPacket);
#ifdef _DEBUG
	assert(bitStreamSucceeded);
#endif
	if (bitStreamSucceeded==false) {InternalPacketPool::Instance()->ReleasePointer(internalPacket); return 0;}
	if (isSplitPacket)
	{
		// split packet indices are one byte (so one packet can be split up to 65535 times - maximum packet size would be about 500 * 65535)
		bitStreamSucceeded = bitStream->Read(internalPacket->splitPacketId);
#ifdef _DEBUG
		assert(bitStreamSucceeded);
#endif
		if (bitStreamSucceeded==false) {InternalPacketPool::Instance()->ReleasePointer(internalPacket); return 0;}
		bitStreamSucceeded = bitStream->ReadCompressed(internalPacket->splitPacketIndex);
#ifdef _DEBUG
		assert(bitStreamSucceeded);
#endif
		if (bitStreamSucceeded==false) {InternalPacketPool::Instance()->ReleasePointer(internalPacket); return 0;}
		bitStreamSucceeded = bitStream->ReadCompressed(internalPacket->splitPacketCount);
#ifdef _DEBUG
		assert(bitStreamSucceeded);
#endif
		if (bitStreamSucceeded==false) {InternalPacketPool::Instance()->ReleasePointer(internalPacket); return 0;}
	}
	else
		internalPacket->splitPacketIndex=internalPacket->splitPacketCount=0;

	// Optimization - do byte alignment here
	//unsigned char zero;
	//bitStream->ReadBits(&zero, 8 - (bitStream->GetNumberOfBitsUsed() %8));
	//assert(zero==0);

	
	unsigned short length;
	bitStreamSucceeded = bitStream->ReadCompressed(length);
	// Read into an unsigned short.  Otherwise the data would be offset too high by two bytes
#ifdef _DEBUG
	assert(bitStreamSucceeded);
#endif
	if (bitStreamSucceeded==false) {InternalPacketPool::Instance()->ReleasePointer(internalPacket); return 0;}
	internalPacket->dataBitLength=length;
#ifdef _DEBUG
	assert(internalPacket->dataBitLength > 0 && BITS_TO_BYTES(internalPacket->dataBitLength) < MAXIMUM_MTU_SIZE);
#endif
	if (! (internalPacket->dataBitLength > 0 && BITS_TO_BYTES(internalPacket->dataBitLength) < MAXIMUM_MTU_SIZE))	{delete [] internalPacket->data; InternalPacketPool::Instance()->ReleasePointer(internalPacket); return 0;}

	// Allocate memory to hold our data
	internalPacket->data = new char [BITS_TO_BYTES(internalPacket->dataBitLength)];
	// Set the last byte to 0 so if ReadBits does not read a multiple of 8 the last bits are 0'ed out
	internalPacket->data[BITS_TO_BYTES(internalPacket->dataBitLength)-1]=0;
	// Read the data the packet holds
	bitStreamSucceeded = bitStream->ReadAlignedBytes((unsigned char*)internalPacket->data, BITS_TO_BYTES(internalPacket->dataBitLength));
	//bitStreamSucceeded = bitStream->ReadBits((unsigned char*)internalPacket->data, internalPacket->dataBitLength);
	#ifdef _DEBUG
	assert(bitStreamSucceeded);	if (bitStreamSucceeded==false) {delete [] internalPacket->data; InternalPacketPool::Instance()->ReleasePointer(internalPacket); return 0;}
	#endif

// PRINTING UNRELIABLE STRINGS
//	if (internalPacket->data && internalPacket->dataBitLength>5*8)
//		printf("Received %s\n",internalPacket->data);

	return internalPacket;
}

//-------------------------------------------------------------------------------------------------------
// Get the SHA1 code
//-------------------------------------------------------------------------------------------------------
void ReliabilityLayer::GetSHA1(unsigned char * const buffer, unsigned long 
nbytes, char code[SHA1_LENGTH])
{
	CSHA1 sha1;

	sha1.Reset();
	sha1.Update((unsigned char*)buffer, nbytes);
	sha1.Final();
	memcpy(code, sha1.GetHash(), SHA1_LENGTH);
}

//-------------------------------------------------------------------------------------------------------
// Check the SHA1 code
//-------------------------------------------------------------------------------------------------------
bool ReliabilityLayer::CheckSHA1(char code[SHA1_LENGTH], unsigned char * 
const buffer, unsigned long nbytes)
{
	char code2[SHA1_LENGTH];
	GetSHA1(buffer, nbytes, code2);

	for (int i=0; i < SHA1_LENGTH; i++)
		if (code[i]!=code2[i])
			return false;

	return true;
}

//-------------------------------------------------------------------------------------------------------
// Search the specified list for sequenced packets on the specified ordering 
// stream, optionally skipping those with splitPacketId, and delete them
//-------------------------------------------------------------------------------------------------------
void  ReliabilityLayer::DeleteSequencedPacketsInList(unsigned char orderingStream, BasicDataStructures::List<InternalPacket*>&theList, int splitPacketId)
{
	unsigned i=0;

	while (i < theList.size())
	{
		if ((theList[i]->reliability==RELIABLE_SEQUENCED || theList[i]->reliability==UNRELIABLE_SEQUENCED) &&
			theList[i]->orderingStream==orderingStream && (splitPacketId==-1 || theList[i]->splitPacketId!=splitPacketId))
		{
			InternalPacket *internalPacket = theList[i];
			theList.del(i);
			delete [] internalPacket->data;
			InternalPacketPool::Instance()->ReleasePointer(internalPacket);
		}
		else
			i++;
	}
}

//-------------------------------------------------------------------------------------------------------
// Search the specified list for sequenced packets with a value less than orderingIndex and delete them
// Note - I added functionality so you can use the Queue as a list (in this case for searching) but it is less efficient to do so than a regular list
//-------------------------------------------------------------------------------------------------------
void  ReliabilityLayer::DeleteSequencedPacketsInList(unsigned char orderingStream, BasicDataStructures::Queue<InternalPacket*>&theList)
{
	InternalPacket *internalPacket;
	int listSize=theList.size();
	int i=0;

	while (i < listSize)
	{
		if ((theList[i]->reliability==RELIABLE_SEQUENCED || theList[i]->reliability==UNRELIABLE_SEQUENCED) &&	theList[i]->orderingStream==orderingStream)
		{
			internalPacket = theList[i];
			theList.del(i);
			delete [] internalPacket->data;
			InternalPacketPool::Instance()->ReleasePointer(internalPacket);
			listSize--;
		}
		else
			i++;
	}
}

//-------------------------------------------------------------------------------------------------------
// Returns true if newPacketOrderingIndex is older than the waitingForPacketOrderingIndex
//-------------------------------------------------------------------------------------------------------
bool ReliabilityLayer::IsOlderOrderedPacket(unsigned char newPacketOrderingIndex, unsigned char waitingForPacketOrderingIndex)
{
	// Any number sequence within the last 128, given a max of 256, is bad
	if (waitingForPacketOrderingIndex > 127)
	{
		if (newPacketOrderingIndex >= waitingForPacketOrderingIndex - 128 && newPacketOrderingIndex < waitingForPacketOrderingIndex)
		{
			// Within the last 128
			return true;
		}
	}
	else if (newPacketOrderingIndex >= (unsigned char)(waitingForPacketOrderingIndex - (unsigned char)128) ||
		newPacketOrderingIndex < waitingForPacketOrderingIndex)
	{
		// Within the last 128
		return true;
	}

	// Old packet
	return false;
}

//-------------------------------------------------------------------------------------------------------
// Split the passed packet into chunks under MTU_SIZEbytes (including headers) and save those new chunks
// Optimized version
//-------------------------------------------------------------------------------------------------------
void ReliabilityLayer::SplitPacketAndDeleteOriginal(InternalPacket *internalPacket, int MTUSize)
{
	// Doing all sizes in bytes in this function so I don't write partial bytes with split packets
	internalPacket->splitPacketCount=1; // This causes GetBitStreamHeaderLength to account for the split packet header
	int headerLength = BITS_TO_BYTES(GetBitStreamHeaderLength(internalPacket));
	int dataByteLength = BITS_TO_BYTES(internalPacket->dataBitLength);
	int maxDataSize;
	int maximumSendBlock, byteOffset, bytesToSend;
	unsigned short splitPacketIndex;
	int i;
	InternalPacket **internalPacketArray;

	maxDataSize = MTUSize - UDP_HEADER_SIZE;
	if (encryptor.IsKeySet())
		maxDataSize-=16; // Extra data for the encryptor


	#ifdef _DEBUG
	// Make sure we need to split the packet to begin with
	assert(dataByteLength > maxDataSize - headerLength);

	// If this assert is hit the packet is so tremendous we need to widen the split packet types.  You should never send something that big anyway
	assert((dataByteLength-1) / (maxDataSize-headerLength) + 1 < 65535);
	#endif

	// How much to send in the largest block
	maximumSendBlock = maxDataSize-headerLength;

	// Calculate how many packets we need to create
	internalPacket->splitPacketCount = (unsigned short)((dataByteLength-1) / (maximumSendBlock) + 1);

	statistics.totalSplits+=internalPacket->splitPacketCount;

	// Optimization
	// internalPacketArray = new InternalPacket*[internalPacket->splitPacketCount];
	internalPacketArray = (InternalPacket**)alloca(sizeof(InternalPacket*)*internalPacket->splitPacketCount);
	for (i=0; i < (int)internalPacket->splitPacketCount; i++)
	{
		internalPacketArray[i]=InternalPacketPool::Instance()->GetPointer();
		memcpy(internalPacketArray[i], internalPacket, sizeof(InternalPacket));
	}

	// This identifies which packet this is in the set
	splitPacketIndex=0;

	// Do a loop to send out all the packets
	do
	{
		byteOffset = splitPacketIndex * maximumSendBlock;
		bytesToSend = dataByteLength - byteOffset;
		if (bytesToSend > maximumSendBlock)
			bytesToSend=maximumSendBlock;

		// Copy over our chunk of data
		internalPacketArray[splitPacketIndex]->data = new char[bytesToSend];
		memcpy(internalPacketArray[splitPacketIndex]->data, internalPacket->data + byteOffset, bytesToSend);

		if (bytesToSend!=maximumSendBlock)
			internalPacketArray[splitPacketIndex]->dataBitLength = internalPacket->dataBitLength - splitPacketIndex * (maximumSendBlock << 3);
		else
			internalPacketArray[splitPacketIndex]->dataBitLength = bytesToSend << 3;
		internalPacketArray[splitPacketIndex]->splitPacketIndex=splitPacketIndex;
		internalPacketArray[splitPacketIndex]->splitPacketId=splitPacketId;
		internalPacketArray[splitPacketIndex]->splitPacketCount=internalPacket->splitPacketCount;
		if (splitPacketIndex>0) // For the first split packet index we keep the packetNumber already assigned
		{
			// For every further packet we use a new packetNumber.
			// Note that all split packets are reliable
			reliabilityLayerMutexes[packetNumber_MUTEX].Lock();
			internalPacketArray[splitPacketIndex]->packetNumber=packetNumber;
			if (++packetNumber==RECEIVED_PACKET_LOG_LENGTH)
				packetNumber=0;
			reliabilityLayerMutexes[packetNumber_MUTEX].Unlock();
		}

		// Add the new packet to send list at the correct priority
//		reliabilityLayerMutexes[sendQueue_MUTEX].Lock();
//		sendQueue[internalPacket->priority].insert(newInternalPacket);
//		reliabilityLayerMutexes[sendQueue_MUTEX].Unlock();
		// SHOW SPLIT PACKET GENERATION
	//	if (splitPacketIndex % 100 == 0)
	//		printf("splitPacketIndex=%i\n",splitPacketIndex);
	//} while(++splitPacketIndex < internalPacket->splitPacketCount);
		} while(++splitPacketIndex < internalPacket->splitPacketCount);
		
	splitPacketId++; // It's ok if this wraps to 0

	// Copy all the new packets into the split packet list
	reliabilityLayerMutexes[sendQueueSystemPriority_MUTEX+internalPacket->priority].Lock();
	for (i=0; i < (int)internalPacket->splitPacketCount; i++)
		sendQueue[internalPacket->priority].push(internalPacketArray[i]);
	reliabilityLayerMutexes[sendQueueSystemPriority_MUTEX+internalPacket->priority].Unlock();

	// Delete the original
	delete [] internalPacket->data;
	InternalPacketPool::Instance()->ReleasePointer(internalPacket);

	//delete [] internalPacketArray;
}

//-------------------------------------------------------------------------------------------------------
// Insert a packet into the split packet list
//-------------------------------------------------------------------------------------------------------
void ReliabilityLayer::InsertIntoSplitPacketList(InternalPacket * internalPacket)
{
	//reliabilityLayerMutexes[splitPacketList_MUTEX].Lock();
	splitPacketList.insert(internalPacket);
	//reliabilityLayerMutexes[splitPacketList_MUTEX].Unlock();
}

//-------------------------------------------------------------------------------------------------------
// Take all split chunks with the specified splitPacketId and try to 
//reconstruct a packet.  If we can, allocate and return it.  Otherwise return 0
// Optimized version
//-------------------------------------------------------------------------------------------------------
InternalPacket * ReliabilityLayer::BuildPacketFromSplitPacketList(unsigned long splitPacketId,unsigned long time)
{
	int i,j,size;
	// How much data all blocks but the last hold
	int maxDataSize;
	int numParts;
	int bitlength;
	int *indexList;
	int indexListIndex;

	//reliabilityLayerMutexes[splitPacketList_MUTEX].Lock();
	size = splitPacketList.size();

	for (i=0; i < size; i++)
	{
		if (splitPacketList[i]->splitPacketId==splitPacketId)
		{
			// Is there enough elements in the list to have all the parts?
			if (splitPacketList[i]->splitPacketCount > splitPacketList.size() - i)
			{
	//			if (splitPacketList.size() % 100 == 0 || splitPacketList[i]->splitPacketCount-splitPacketList.size()<100)
	//				printf("%i out of %i\n", splitPacketList.size(), splitPacketList[i]->splitPacketCount);
				//reliabilityLayerMutexes[splitPacketList_MUTEX].Unlock();
				return 0;
			}

	//		printf("%i out of %i\n", splitPacketList.size(), splitPacketList[i]->splitPacketCount);
			// Keep track of the indices of the elements through our first scan so we don't have to rescan to find them
			indexListIndex=0;

			numParts=1;
			bitlength=splitPacketList[i]->dataBitLength;

			// indexList = new int[splitPacketList[i]->splitPacketCount];
			indexList = (int*) alloca(sizeof(int)*splitPacketList[i]->splitPacketCount);
			indexList[indexListIndex++]=i;

			maxDataSize=BITS_TO_BYTES(splitPacketList[i]->dataBitLength);

			// Are all the parts there?
			for (j=i+1; j < size; j++)
			{
				if (splitPacketList[j]->splitPacketId==splitPacketId)
				{
					indexList[indexListIndex++]=j;
					numParts++;
					bitlength+=splitPacketList[j]->dataBitLength;
					if ((int)BITS_TO_BYTES(splitPacketList[j]->dataBitLength) > maxDataSize)
						maxDataSize=BITS_TO_BYTES(splitPacketList[j]->dataBitLength);
				}
			}
			if (numParts==splitPacketList[i]->splitPacketCount)
			{
				// All the parts are here
				InternalPacket *internalPacket=CreateInternalPacketCopy(splitPacketList[i],0,0,time);
				internalPacket->data=new char[BITS_TO_BYTES(bitlength)];
#ifdef _DEBUG
				internalPacket->splitPacketCount = splitPacketList[i]->splitPacketCount;
#endif

				// Add each part to internalPacket
				j=0;
				while (j<indexListIndex)
				{
					if (splitPacketList[indexList[j]]->splitPacketCount-1 == splitPacketList[indexList[j]]->splitPacketIndex)
					{
						// Last split packet
						// If this assert fails,
						// then the total bit length calculated by adding the last block to the maximum block size * the number of blocks that are not the last block
						// doesn't match the amount calculated from traversing the list
						#ifdef _DEBUG
						assert(BITS_TO_BYTES(splitPacketList[indexList[j]]->dataBitLength) + splitPacketList[indexList[j]]->splitPacketIndex * maxDataSize == (bitlength-1)/8+1);
						#endif
						memcpy(internalPacket->data + splitPacketList[indexList[j]]->splitPacketIndex * maxDataSize, splitPacketList[indexList[j]]->data, BITS_TO_BYTES(splitPacketList[indexList[j]]->dataBitLength));
					}
					else
					{
						// Not last split packet
						memcpy(internalPacket->data + splitPacketList[indexList[j]]->splitPacketIndex * maxDataSize, splitPacketList[indexList[j]]->data, maxDataSize);
					}

					internalPacket->dataBitLength+=splitPacketList[indexList[j]]->dataBitLength;
					InternalPacket *temp;

					temp = splitPacketList[indexList[j]];
					delete [] temp->data;
					InternalPacketPool::Instance()->ReleasePointer(temp);
					splitPacketList[indexList[j]]=0;

#ifdef _DEBUG
					numParts--;
#endif

					j++;
					size--;
				}

#ifdef _DEBUG
				assert(numParts==0); // Make sure the correct # of elements was removed from the list
#endif

				j=0;
				while ((unsigned)j<splitPacketList.size())
					if (splitPacketList[j]==0)
					{
						// Since order doesn't matter, swap from the tail to the current element.
						splitPacketList[j]=splitPacketList[splitPacketList.size()-1];
						splitPacketList[splitPacketList.size()-1]=0;
						// Then just delete the tail (just changes a counter)
						splitPacketList.del(splitPacketList.size()-1);
					}
					else
						j++;

				//reliabilityLayerMutexes[splitPacketList_MUTEX].Unlock();

			//	delete [] indexList;

				return internalPacket;
			}

		//	delete [] indexList;
			break;
		}
	}

	//reliabilityLayerMutexes[splitPacketList_MUTEX].Unlock();

	return 0;
}

//-------------------------------------------------------------------------------------------------------
// Creates a copy of the specified internal packet with data copied from the original starting at dataByteOffset for dataByteLength bytes.
// Does not copy any split data parameters as that information is always generated does not have any reason to be copied
//-------------------------------------------------------------------------------------------------------
InternalPacket * ReliabilityLayer::CreateInternalPacketCopy(InternalPacket *original, int dataByteOffset, int dataByteLength, unsigned long time)
{
	InternalPacket *copy = InternalPacketPool::Instance()->GetPointer();
#ifdef _DEBUG
	// Remove boundschecker accessing undefined memory error
	memset(copy, 255, sizeof(InternalPacket));
#endif
	// Copy over our chunk of data
	if (dataByteLength>0)
	{
		copy->data = new char[dataByteLength];
		memcpy(copy->data, original->data + dataByteOffset, dataByteLength);
	}
	else
		copy->data=0;

	copy->dataBitLength=dataByteLength<<3;
	copy->creationTime = time;
	copy->isAcknowledgement=original->isAcknowledgement;
	copy->nextActionTime=0;
	copy->orderingIndex=original->orderingIndex;
	copy->orderingStream=original->orderingStream;
	copy->packetNumber=original->packetNumber;
	copy->priority=original->priority;
	copy->reliability=original->reliability;

	return copy;
}

//-------------------------------------------------------------------------------------------------------
// Get the specified ordering list
// LOCK THIS WHOLE BLOCK WITH reliabilityLayerMutexes[orderingList_MUTEX].Unlock();
//-------------------------------------------------------------------------------------------------------
BasicDataStructures::LinkedList<InternalPacket*> *ReliabilityLayer::GetOrderingListAtOrderingStream(unsigned char orderingStream)
{
	if (orderingStream >= orderingList.size())
		return 0;

	return orderingList[orderingStream];
}

//-------------------------------------------------------------------------------------------------------
// Add the internal packet to the ordering list in order based on order index
//-------------------------------------------------------------------------------------------------------
void ReliabilityLayer::AddToOrderingList(InternalPacket * internalPacket)
{
	#ifdef _DEBUG
	assert(internalPacket->orderingStream< NUMBER_OF_ORDERED_STREAMS);
	#endif
	if (internalPacket->orderingStream >= NUMBER_OF_ORDERED_STREAMS)	
		return;

	//reliabilityLayerMutexes[orderingList_MUTEX].Lock();
	if (internalPacket->orderingStream >= orderingList.size() || orderingList[internalPacket->orderingStream]==0)
	{
		// Need a linked list in this index
		orderingList.replace(new BasicDataStructures::LinkedList<InternalPacket*>, 0, internalPacket->orderingStream);
		(orderingList[internalPacket->orderingStream])->add(internalPacket);
		//reliabilityLayerMutexes[orderingList_MUTEX].Unlock();
		return;
	}
	else
	{
		// Have a linked list in this index
		if (orderingList[internalPacket->orderingStream]->size()==0)
		{
			// The linked list is empty
			(orderingList[internalPacket->orderingStream])->add(internalPacket);
		}
		else
		{
			BasicDataStructures::LinkedList<InternalPacket*> *theList;
			theList = GetOrderingListAtOrderingStream(internalPacket->orderingStream);
			// Add this packet in no particular order
			theList->beginning();
			theList->insert(internalPacket);
		}
		//reliabilityLayerMutexes[orderingList_MUTEX].Unlock();
	}
}

//-------------------------------------------------------------------------------------------------------
// Inserts a packet into the resend list in order
// THIS WHOLE FUNCTION SHOULD BE LOCKED WITH
// reliabilityLayerMutexes[resendQueue_MUTEX].Lock();
//-------------------------------------------------------------------------------------------------------
void ReliabilityLayer::InsertPacketIntoResendQueue(InternalPacket *internalPacket, unsigned long time)
{
	//reliabilityLayerMutexes[lastAckTime_MUTEX].Lock();
	if (lastAckTime==0 || resendQueue.size()==0)
		lastAckTime=time; // Start the timer for the ack of this packet if we aren't already waiting for an ack
	//reliabilityLayerMutexes[lastAckTime_MUTEX].Unlock();

	//reliabilityLayerMutexes[resendQueue_MUTEX].Lock();
	resendQueue.push(internalPacket);
	//reliabilityLayerMutexes[resendQueue_MUTEX].Unlock();
}

//-------------------------------------------------------------------------------------------------------
// If Read returns -1 and this returns true then a modified packet was detected
//-------------------------------------------------------------------------------------------------------
bool ReliabilityLayer::IsCheater(void) const {return cheater;}

//-------------------------------------------------------------------------------------------------------
//  Were you ever unable to deliver a packet despite retries?
//-------------------------------------------------------------------------------------------------------
bool ReliabilityLayer::IsDeadConnection(void) const {return deadConnection;}

//-------------------------------------------------------------------------------------------------------
// How long to wait between packet resends
//-------------------------------------------------------------------------------------------------------
void ReliabilityLayer::SetLostPacketResendDelay(unsigned long i)
{
	if (i > 0) lostPacketResendDelay=i;
	if (lostPacketResendDelay<100)
		lostPacketResendDelay=100;
}

//-------------------------------------------------------------------------------------------------------
// Statistics
//-------------------------------------------------------------------------------------------------------
RakNetStatisticsStruct * const ReliabilityLayer::GetStatistics(void)
{
	int i;
	for (i=0; i < NUMBER_OF_PRIORITIES; i++)
	{
		statistics.messageSendBuffer[i]=sendQueue[i].size();
	}

	statistics.acknowlegementsPending=acknowledgementQueue.size();
	statistics.messagesWaitingForReassembly=splitPacketList.size();
	statistics.internalOutputQueueSize=outputQueue.size();
	statistics.windowSize=windowSize;
	statistics.lossySize=lossyWindowSize==MAXIMUM_WINDOW_SIZE+1 ? 0 : lossyWindowSize;
	statistics.messagesOnResendQueue=resendQueue.size();

	return &statistics;
}