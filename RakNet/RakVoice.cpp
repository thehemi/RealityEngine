// This file is part of RakNet Copyright 2003, 2004 Rakkarsoft LLC and Kevin Jenkins.

// Usage of Raknet is subject to the appropriate licence agreement.
// "Shareware" Licensees with Rakkarsoft LLC are subject to the shareware license found at http://www.rakkarsoft.com/shareWareLicense.html which you agreed to upon purchase of a "Shareware license"
// "Commercial" Licensees with Rakkarsoft LLC are subject to the commercial license found at http://www.rakkarsoft.com/sourceCodeLicense.html which you agreed to upon purchase of a "Commercial license"
// All other users are subject to the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

// Refer to the appropriate license agreement for distribution, modification, and warranty rights.

#include "RakVoice.h"
#include "speex.h"
#include <assert.h>
#include "PacketEnumerations.h"
#include "BitStream.h"
#include "GetTime.h"
#include <memory.h>
#include <stdio.h>
#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
#include <pthread.h>
#endif

using namespace RakNet;

#ifdef _WIN32
unsigned __stdcall rakVoiceThread(LPVOID arguments);
#else
void*  rakVoiceThread( void*  arguments );
#endif

RakVoice::RakVoice()
{
	init=false;
}

RakVoice::~RakVoice()
{
}

void RakVoice::Init(int samplingRate, int bitsPerSample,RakPeerInterface *_peer)
{
	peer=_peer;
	Init(samplingRate, bitsPerSample);
}

void RakVoice::Init(int samplingRate, int bitsPerSample)
{
	assert(init==false);
	if (init==false)
	{
		if (bitsPerSample > 16)
			bitsPerSample=16;

		writeCursor=readCursor=0;

		speex_bits_init(&bits);

		// Create an encoder so we can get the frame_size
		void *enc_state;
		if (samplingRate>16000)
			enc_state = speex_encoder_init(&speex_uwb_mode);
		if (samplingRate>8000)
			enc_state = speex_encoder_init(&speex_wb_mode);
		else
			enc_state = speex_encoder_init(&speex_nb_mode);

		speex_encoder_ctl(enc_state, SPEEX_SET_SAMPLING_RATE, &samplingRate);
		speex_encoder_ctl(enc_state, SPEEX_GET_FRAME_SIZE,&frame_size);
		speex_encoder_destroy(enc_state);


		bps=bitsPerSample;
		sampleRate=samplingRate;
		blockSize=frame_size;

        // Start the encoding thread
#ifdef _WIN32
		unsigned threadID=0;
		HANDLE threadHandle;
		threadHandle=(HANDLE)_beginthreadex(NULL, 0, rakVoiceThread, this, 0, &threadID);
		CloseHandle(threadHandle);
#else	
		pthread_attr_t attr;
		pthread_attr_init( &attr );
		pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_DETACHED );
		pthread_t threadHandle;
		int error = pthread_create( &threadHandle, &attr, &rakVoiceThread, this );
		assert(error==0);
#endif

		init=true;
	}
}

// Valid to call after a call to Init.
// Returns the frame sized used by the encoder in bytes
// It is best to send input to EncodeSoundData that matches this frame size
int RakVoice::GetFrameSize(void) const
{
	return frame_size * bps / 8;
}

// Set the block size that EncodeSoundPacket will read and GetSoundPacket will write.
// If you don't call this, it will default to GetFrameSize()
void RakVoice::SetBlockSize(int _blockSize)
{
	if (_blockSize > 0 && (_blockSize % frame_size) ==0)
		blockSize=_blockSize;
}

// Whenever a player disconnects we need to know about it.  Otherwise we will be using
// old values for our encoding
void RakVoice::Disconnect(PlayerID id)
{
	unsigned i;
	coderStateListMutex.Lock();
	for (i=0; i < coderStateList.size(); i++)
	{
		if (coderStateList[i]->playerId==id)
		{
			if (coderStateList[i]->decoderState)
				speex_decoder_destroy(coderStateList[i]->decoderState);
			if (coderStateList[i]->encoderState)
				speex_encoder_destroy(coderStateList[i]->encoderState);

			delete coderStateList[i];

			coderStateListMutex.Unlock();
			return;
		}
	}
	coderStateListMutex.Unlock();
}

// This will take a packet, decode it, and put the raw output into a queue.
// You can read immediately from this queue if you want.  However, it is better if you
// lag things during silence for a short time so ping latency won't be noticeable.
void RakVoice::DecodeAndQueueSoundPacket(char* data, int length)
{
	unsigned char bitsPerSample;
	PlayerID sender, target;
	unsigned short packetNumber;
	char *input;
	int inputLength,i,j,k,l;
	int speexReturnValue;
	int outputIndex;
	speex_encoding_type speexData[640];
	PCMDataStruct *s;
	unsigned short sr; // sample rate
	BitStream b(data, length, false);

	assert(frame_size <= 640);
	if (frame_size>640)
		return;

	b.IgnoreBits(8); // Ignore ID_VOICE_PACKET
	b.Read((char*)&target, sizeof(target));
	b.Read(bitsPerSample);
	b.Read(sr);// sample rate
	b.Read((char*)&sender,sizeof(sender)); // Read who sent this packet.
	b.Read(packetNumber); // Read this packet number

	// Get a convenience pointer to the raw data
	input = (char*)(b.GetData() + b.GetReadOffset()/8);
	inputLength = b.GetNumberOfBytesUsed() - b.GetReadOffset()/8;

	// Get the decoder based on who sent this packet.
	CoderStateWithPlayerIDMapStruct *cswpims = GetCoderFromPlayerID(sr,sender, true);

	// If this packet number is from the last guy who sent us a packet, and this packet number is out of order,
	// then skip packets
	if ((unsigned short)((unsigned short)cswpims->lastReceivedPacketNumber+(unsigned short)1)!=packetNumber)
	{
		// We lost one or more frames.
		unsigned short numberOfLostPackets;
		// Even with wrapping over 65535 this works!
		numberOfLostPackets=packetNumber - cswpims->lastReceivedPacketNumber;
		if (numberOfLostPackets<=10)
		{
#ifdef _DEBUG
			printf("%i voice packets lost or out of sequence in RakVoice.cpp.\nIf this is over 5 we should be worried.\n", numberOfLostPackets);
#endif
			for (j=0; j<numberOfLostPackets; j++)
			{
				if (PCMQueuePool.size()>0)
				{
					s=PCMQueuePool.pop();
				}
				else
				{
					s = new PCMDataStruct;
					s->data=new char[blockSize];
				}

				outputIndex=0;

				// Call speex_decode for as many times as we have frames per packet
				for (k=0; k < blockSize / frame_size; k++)
				{
					speex_decode(cswpims->decoderState, 0, speexData);

					if (bitsPerSample==8)
					{
						for (l=0; l < frame_size; l++)
							((char*)(s->data))[outputIndex++]=(char)speexData[l];
					}
					else
					{
						for (l=0; l < frame_size; l++)
							((short*)(s->data))[outputIndex++]=(short)speexData[l];
					}
				}

				s->sender=sender;
				PCMQueueMutex.Lock();
				PCMQueue.push(s);
				PCMQueueMutex.Unlock();
			}
		}
		else
		{
			// Something screwed up since it's so highly unlikely we'd lose that many frames exactly in a row
			assert(0);
		}
	}

	// Encode the input data into the speex bitstream class
	speex_bits_read_from((SpeexBits*)(&bits), input, inputLength);

	if (PCMQueuePool.size()>0)
	{
		s=PCMQueuePool.pop();
	}
	else
	{
		s = new PCMDataStruct;
		s->data=new char[blockSize];
	}


	outputIndex=0;

	// Call speex_decode for as many times as we have frames
	for (j=0; j < blockSize / frame_size; j++)
	{
		speexReturnValue = speex_decode(cswpims->decoderState, &bits, speexData);
		assert(speexReturnValue==0);

		if (bitsPerSample==8)
		{
			for (i=0; i < frame_size; i++)
				((char*)(s->data))[outputIndex++]=(char)speexData[i];			
		}
		else
		{
			for (i=0; i < frame_size; i++)
				((short*)(s->data))[outputIndex++]=(short)speexData[i];
		}
	}

	cswpims->lastReceivedPacketNumber=packetNumber;

	s->sender=sender;
	PCMQueueMutex.Lock();
	PCMQueue.push(s);
	PCMQueueMutex.Unlock();
}

// Give input of size blockSize to encode this sound data and send it as a packet
void RakVoice::EncodeSoundPacket(char *input, PlayerID recipient)
{
	// If this assert hits, increase the size of the 2nd dimension of the inputList array
	assert(frame_size <= 160);

	// Write this data at the write cursor
	int frameIndex=0;
	int numberOfSpeexFrames=(blockSize / frame_size);
	int i;
	int index;

	for (frameIndex=0; frameIndex < numberOfSpeexFrames; frameIndex++)
	{
		if (((writeCursor+1) % RAK_VOICE_INPUT_LIST_BUFFER_SIZE)==readCursor)
		{
			// Write buffer is full!!  Increase RAK_VOICE_INPUT_LIST_BUFFER_SIZE
			assert(0);
			return;
		}

		index = frameIndex*frame_size;
		// If we are using 16 bits per sample we have to convert it to a short* so we read 2 bytes at a time
		if (bps==16)
		{
			for (i=0; i < frame_size; i++)
				inputList[writeCursor][i]=((short*)input)[i+index];
		}
		else
		{
			// 8 bits per sample
			for (i=0; i < frame_size; i++)
				inputList[writeCursor][i]=((char*)input)[i+index];
		}

		targetedSendRecipient[writeCursor]=recipient;
		writeCursor = (writeCursor + 1) % RAK_VOICE_INPUT_LIST_BUFFER_SIZE;
	}
}

void RakVoice::Deinit(void)
{
	unsigned i;

	if (init)
	{
		speex_bits_destroy(&bits);
		init=false;

		coderStateListMutex.Lock();
		for (i=0; i < coderStateList.size(); i++)
		{
			if (coderStateList[i]->decoderState)
				speex_decoder_destroy(coderStateList[i]->decoderState);
			if (coderStateList[i]->encoderState)
				speex_encoder_destroy(coderStateList[i]->encoderState);

			delete coderStateList[i];
		}
		coderStateList.clear();
		coderStateListMutex.Unlock();

		PCMDataStruct* s;

		PCMQueueMutex.Lock();
		while (PCMQueue.size())
		{
			s=PCMQueue.pop();
			delete [] s->data;
			delete s;
		}
		PCMQueueMutex.Unlock();

		while (PCMQueuePool.size())
		{
			s=PCMQueuePool.pop();
			delete [] s->data;
			delete s;
		}
	}
}

int RakVoice::GetNextSoundPacketSize(void)
{
	int size;

	PCMQueueMutex.Lock();
	if (PCMQueue.size()==0)
		size=0;
	else
		size=blockSize;
	PCMQueueMutex.Unlock();

	return size;
}

int RakVoice::GetOutputBufferSize(void)
{
	int size=0;

	PCMQueueMutex.Lock();
	size=PCMQueue.size() * blockSize;
	PCMQueueMutex.Unlock();

	return size;
}

bool RakVoice::GetSoundPacket(char *output, PlayerID *sender)
{
	PCMDataStruct* s;
	bool b;

	PCMQueueMutex.Lock();
	if (PCMQueue.size()>0)
	{
		s=PCMQueue.pop();
		memcpy(output, s->data, blockSize);
		b=true;
		*sender=s->sender;
		PCMQueuePool.push(s);
	}
	else
	{
		b=false;
	}
	PCMQueueMutex.Unlock();

	return b;
}

CoderStateWithPlayerIDMapStruct* RakVoice::CreateCoderStateWithPlayerIDMapStruct(int samplingRate, PlayerID playerId, bool decoder)
{
	// Track that this decoder was created by adding it to the decoderStateList
	CoderStateWithPlayerIDMapStruct *s;
	s = new CoderStateWithPlayerIDMapStruct;
	s->lastUsageTime=RakNetGetTime();
	s->playerId=playerId;
	s->lastReceivedPacketNumber=65535;
	s->nextPacketNumber=0;

	if (decoder)
	{
		s->decoderState = CreateCoderState(samplingRate, decoder);
		s->encoderState=0;
	}
	else
	{
		s->encoderState = CreateCoderState(samplingRate, decoder);
		s->decoderState=0;
	}

	coderStateListMutex.Lock();
	coderStateList.insert(s);
	coderStateListMutex.Unlock();
	
	return s;
}

void *RakVoice::CreateCoderState(int samplingRate, bool decoder)
{
	void *state;
	if (samplingRate>16000)
	{
		if (decoder)
			state = speex_decoder_init(&speex_uwb_mode);
		else
			state = speex_encoder_init(&speex_uwb_mode);
	}
	else if (samplingRate>8000)
	{
		if (decoder)
			state = speex_decoder_init(&speex_wb_mode);
		else
			state = speex_encoder_init(&speex_wb_mode);
	}
	else
	{
		if (decoder)
			state = speex_decoder_init(&speex_nb_mode);
		else
			state = speex_encoder_init(&speex_nb_mode);
	}

	// Turn on the perceptual post-filter
	if (decoder)
	{
		int ehc=1;
		speex_decoder_ctl(state, SPEEX_SET_ENH, &ehc);
	}
	// Set the sampling rate
	else
	{
		speex_encoder_ctl(state, SPEEX_SET_SAMPLING_RATE, &samplingRate);
	}

	return state;
}

CoderStateWithPlayerIDMapStruct *RakVoice::GetCoderFromPlayerID(unsigned short sr, PlayerID id, bool decoder)
{
	unsigned i;
	CoderStateWithPlayerIDMapStruct *output;
	// Check our list to see if it already has the decoder we are looking for.
	i=0;
	coderStateListMutex.Lock();
	while (i < coderStateList.size())
	{
		if (coderStateList[i]->playerId==id)
		{
			if (decoder && coderStateList[i]->decoderState==0)
				coderStateList[i]->decoderState = CreateCoderState(sr, decoder);
			else if (decoder==false && coderStateList[i]->encoderState==0)
				coderStateList[i]->encoderState = CreateCoderState(sr, decoder);

			coderStateList[i]->lastUsageTime=RakNetGetTime();
			output=coderStateList[i];
			coderStateListMutex.Unlock();
			return output;
		}
		else
			i++;
	}
	coderStateListMutex.Unlock();

	// No existing coder found.
	return CreateCoderStateWithPlayerIDMapStruct(sr,id, decoder);
}

#ifdef _WIN32
unsigned __stdcall rakVoiceThread(LPVOID arguments)
#else
void*  rakVoiceThread( void*  arguments )
#endif
{
	SpeexBits bits;
	char output[2000];
	int outputLength;
	unsigned char typeID;
	PlayerID id;
	BitStream b;
	int availableChunks;
	int MTU;
	int numberOfChunksPerSend;
	int i;
	unsigned long lastSendTime;
	CoderStateWithPlayerIDMapStruct *cswpims;
	lastSendTime=RakNetGetTime();
	PlayerID target;

	RakVoice *rakVoice = (RakVoice*) arguments;

	numberOfChunksPerSend = rakVoice->blockSize/rakVoice->frame_size;

	speex_bits_init(&bits);

	MTU=rakVoice->peer->GetMTUSize();

	while (rakVoice->init)
	{
		if (rakVoice->writeCursor >= rakVoice->readCursor)
            availableChunks=rakVoice->writeCursor-rakVoice->readCursor;
		else
            availableChunks = RAK_VOICE_INPUT_LIST_BUFFER_SIZE - rakVoice->readCursor + rakVoice->writeCursor;

		while (availableChunks >= numberOfChunksPerSend)
		{
			// Get a bit of a buffer before we start sending so we don't "grind" the data and get popping as data continually arrives and runs out
			if (RakNetGetTime() - lastSendTime > 1000 && availableChunks < numberOfChunksPerSend * 3)
				break;

			// Grab data at the read cursor and encode it
			speex_bits_reset(&bits);

			target=rakVoice->targetedSendRecipient[rakVoice->readCursor];
			cswpims=rakVoice->GetCoderFromPlayerID(rakVoice->sampleRate,target, false);

			for (i=0; i < numberOfChunksPerSend; i++)
			{
				// For each frame, call speex_encode
				speex_encode(cswpims->encoderState, rakVoice->inputList[rakVoice->readCursor], &bits);
				rakVoice->readCursor=(rakVoice->readCursor+1) % RAK_VOICE_INPUT_LIST_BUFFER_SIZE;
			}

			availableChunks-=numberOfChunksPerSend;
		
			// Write the encoded bitstream
			outputLength = speex_bits_write((SpeexBits*)(&bits), output, 2000);
#ifdef _DEBUG
			static bool printOnce=true;
			if (printOnce==true && outputLength > MTU)
			{
				printf("Warning - compressed data larger than MTU! This will result in split packets and poor speech.\nYou should use a lower blockSize in the call to Init.\n");
				printOnce=false;
			}
			else if (printOnce==true && outputLength < MTU/4)
			{
				printf("Warning - compressed data smaller than 1/4 the MTU.  This is not an efficient use of bandwidth.\nYou might want to use a larger blockSize in the call to Init.\n");
				printOnce=false;
			}
#endif

			b.Reset();
			typeID=ID_VOICE_PACKET;
			b.Write(typeID);
			b.Write((char*)&target, sizeof(target));
			b.Write(rakVoice->bps); // Write how many bits we are encoding the data with
			b.Write(rakVoice->sampleRate);  // Write the sampling rate
			id=rakVoice->peer->GetInternalID();
			b.Write((char*)&id, sizeof(id)); // Write who is sending this packet
			b.Write(cswpims->nextPacketNumber); // Write what speech packet number this is, so we can compensate for lost packets
			cswpims->nextPacketNumber++;
			b.Write(output, outputLength);


			/*
			// This is for showing memory usage
			printf("PCMQueue=%i PCMQueuePool=%i\ndecoderStateList=%i encoderStateList=%i\n",
				rakVoice->PCMQueue.size(),
				rakVoice->PCMQueuePool.size(),
				rakVoice->decoderStateList.size(),
				rakVoice->encoderStateList.size());
				*/

			// THis was for testing as a feedback loop
			//if (target==rakVoice->peer->GetInternalID())
			//	rakVoice->DecodeAndQueueSoundPacket((char*)b.GetData(), b.GetNumberOfBytesUsed());
			//else
				rakVoice->peer->Send(&b, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, target, false);

			lastSendTime=RakNetGetTime();
		}

		// Send out a packet aggreggate roughly every x ms
#ifdef _WIN32
		Sleep(30);
#else
		usleep(30 * 1000);
#endif
	}

	speex_bits_destroy(&bits);
	return 0;
}