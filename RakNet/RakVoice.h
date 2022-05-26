// This file is part of RakNet Copyright 2003, 2004 Rakkarsoft LLC and Kevin Jenkins.

// Usage of Raknet is subject to the appropriate licence agreement.
// "Shareware" Licensees with Rakkarsoft LLC are subject to the shareware license found at http://www.rakkarsoft.com/shareWareLicense.html which you agreed to upon purchase of a "Shareware license"
// "Commercial" Licensees with Rakkarsoft LLC are subject to the commercial license found at http://www.rakkarsoft.com/sourceCodeLicense.html which you agreed to upon purchase of a "Commercial license"
// All other users are subject to the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

// Refer to the appropriate license agreement for distribution, modification, and warranty rights.

#ifndef __RAK_VOICE_H
#define __RAK_VOICE_H

#include "speex\speex_bits.h"
#include "RakPeerInterface.h"
#include "NetworkTypes.h"
#include "Queue.h"
#include "SimpleMutex.h"
#include "ArrayList.h"
#include "RakVoiceInterface.h"

// This is a float in speex 1.0.3 and a short in 1.1.x
typedef short speex_encoding_type;
// Size of the internal queue to hold sound data.  This should be written to the sound buffer at a fixed rate.
#define RAK_VOICE_INPUT_LIST_BUFFER_SIZE 200

// Each paired player permutation requires its own encoder and decoder
struct CoderStateWithPlayerIDMapStruct
{
	void *decoderState, *encoderState;
	unsigned long lastUsageTime;
	PlayerID playerId;
	unsigned short lastReceivedPacketNumber, nextPacketNumber;
};

class RakVoice : public RakVoiceInterface
{
public:
	RakVoice();
	~RakVoice();
	
	// Call this before using voice packets.
	// Use the server version to send packets through the server, client version to send packets through the client
	// _blockSize is the size of each block that you want to process at a time.  Each network packet will have this size before compression.
	// It must be a multiple of frame_size.  The best _blockSize is what would be compressed to slightly under your MTU.
	// You can safely assume the compression rate is 50% (actually it's closer to 75%).
	void Init(int samplingRate, int bitsPerSample,RakPeerInterface *_peer);	

	// Valid to call after a call to Init.
	// Returns the frame size used by the encoder in bytes
	// It is best to send input to EncodeSoundData that matches this frame size
	int GetFrameSize(void) const;

	// Whenever a player disconnects we need to know about it.  Otherwise we will be using
	// old values for our encoding.  Passing an id that has never been used is ok, it will be ignored.
	void Disconnect(PlayerID id);

	// Set the block size that EncodeSoundPacket will read and GetSoundPacket will write.
	// If you don't call this, it will default to GetFrameSize()
	// You should only call this after calling Init.  It is reset every call to Init.
	// This must be a multiple of GetFrameSize().
	void SetBlockSize(int _blockSize);

	// Call this before shutting down
	void Deinit(void);

	// When you have raw sound data, pass it to this function.
	// Input must be of size blockSize that you specified in Init
	// This will encode and send in another thread the data as a packet
	// Because of the way encoding works, you cannot broadcast voice data.  You must specify a recipient
	// If you want to send to everyone, you have to call this once for each recipient
	void EncodeSoundPacket(char *input, PlayerID recipient);

	// When you get a packet with the type ID_VOICE_PACKET,
	// Pass the data and length to this function.
	// This will decode the data and put it in the internal queue, or simply relay the data if
	// This is the server and the target is not the server
	void DecodeAndQueueSoundPacket(char* data, int length);

	// This will get the next sound data packet and write it to output
	// Returns false if no packets are waiting.
    // The originator of the packet is written to sender
	bool GetSoundPacket(char *output, PlayerID *sender);

	// Gives you the size, in bytes, of the next sound packet, or 0 for nothing left
	int GetNextSoundPacketSize(void);

	// This will tell you the total number of bytes in the output buffer
	int GetOutputBufferSize(void);

private:

	void Init(int samplingRate, int bitsPerSample);

	// Creates encoders and decoders
	CoderStateWithPlayerIDMapStruct* CreateCoderStateWithPlayerIDMapStruct(int samplingRate, PlayerID playerId, bool decoder);
	void *RakVoice::CreateCoderState(int samplingRate, bool decoder);

	CoderStateWithPlayerIDMapStruct *GetCoderFromPlayerID(unsigned short sr, PlayerID id, bool decoder);

	bool init;

	// Encoding calls are buffered to this input list
	// The thread will then parse this list to actually create the encoded data and send it through the network
	PlayerID targetedSendRecipient[RAK_VOICE_INPUT_LIST_BUFFER_SIZE];
	speex_encoding_type inputList[RAK_VOICE_INPUT_LIST_BUFFER_SIZE][160];
	int writeCursor, readCursor;

	unsigned char bps; // bits per sample
	SpeexBits bits;
	int frame_size;

	struct PCMDataStruct
	{
		char *data;
		PlayerID sender;
	};


#ifdef _WIN32
	friend unsigned __stdcall rakVoiceThread(LPVOID arguments);
#else
	friend void*  rakVoiceThread( void*  arguments );
#endif

	BasicDataStructures::Queue<PCMDataStruct*> PCMQueue;
	BasicDataStructures::Queue<PCMDataStruct*> PCMQueuePool;
	BasicDataStructures::List<CoderStateWithPlayerIDMapStruct*> coderStateList;
	SimpleMutex PCMQueueMutex;
	SimpleMutex coderStateListMutex;
	RakPeerInterface *peer;
	unsigned short sampleRate;
	int blockSize; // Must be a multiple of frame_size
};

#endif