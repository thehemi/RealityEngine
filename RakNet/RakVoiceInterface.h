// This file is part of RakNet Copyright 2003, 2004 Rakkarsoft LLC and Kevin Jenkins.

// Usage of Raknet is subject to the appropriate licence agreement.
// "Shareware" Licensees with Rakkarsoft LLC are subject to the shareware license found at http://www.rakkarsoft.com/shareWareLicense.html which you agreed to upon purchase of a "Shareware license"
// "Commercial" Licensees with Rakkarsoft LLC are subject to the commercial license found at http://www.rakkarsoft.com/sourceCodeLicense.html which you agreed to upon purchase of a "Commercial license"
// All other users are subject to the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

// Refer to the appropriate license agreement for distribution, modification, and warranty rights.

#ifndef __RAK_VOICE_INTERFACE_H
#define __RAK_VOICE_INTERFACE_H

class RakPeerInterface;
#include "NetworkTypes.h"

class RakVoiceInterface
{
public:
	// Destructor
	virtual ~RakVoiceInterface() {}
	
	// Call this before using voice packets.
	// Use the server version to send packets through the server, client version to send packets through the client
	// _blockSize is the size of each block that you want to process at a time.  Each network packet will have this size before compression.
	// It must be a multiple of frame_size.  The best _blockSize is what would be compressed to slightly under your MTU.
	// You can safely assume the compression rate is 50% (actually it's closer to 75%).
	virtual void Init(int samplingRate, int bitsPerSample,RakPeerInterface *_peer)=0;

	// Call this before shutting down
	virtual void Deinit(void)=0;
	
	// Valid to call after a call to Init.
	// Returns the frame size used by the encoder in bytes
	// It is best to send input to EncodeSoundData that matches this frame size
	virtual int GetFrameSize(void) const=0;

	// Whenever a player disconnects we need to know about it.  Otherwise we will be using
	// old values for our encoding.  Passing an id that has never been used is ok, it will be ignored.
	virtual void Disconnect(PlayerID id)=0;

	// Set the block size that EncodeSoundPacket will read and GetSoundPacket will write.
	// If you don't call this, it will default to GetFrameSize()
	// You should only call this after calling Init.  It is reset every call to Init.
	// This must be a multiple of GetFrameSize().
	virtual void SetBlockSize(int _blockSize)=0;

	// When you have raw sound data, pass it to this function.
	// Input must be of size blockSize that you specified in Init
	// This will encode and send in another thread the data as a packet
	// Because of the way encoding works, you cannot broadcast voice data.  You must specify a recipient
	// If you want to send to everyone, you have to call this once for each recipient
	// Use UNASSIGNED_PLAYER_ID to send to the server (if you are a client).
	virtual void EncodeSoundPacket(char *input, PlayerID recipient)=0;

	// When you get a packet with the type ID_VOICE_PACKET,
	// Pass the data and length to this function.
	// This will decode the data and put it in the internal queue, or simply relay the data if
	// This is the server and the target is not the server
	virtual void DecodeAndQueueSoundPacket(char* data, int length)=0;

	// This will get the next sound data packet and write it to output
	// Returns false if no packets are waiting.
    	// The originator of the packet is written to sender
	virtual bool GetSoundPacket(char *output, PlayerID *sender)=0;

	// Gives you the size, in bytes, of the next sound packet, or 0 for nothing left
	virtual int GetNextSoundPacketSize(void)=0;

	// This will tell you the total number of bytes in the output buffer
	virtual int GetOutputBufferSize(void)=0;
};

#endif