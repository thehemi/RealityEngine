// ----------------------------------------------------------------------
// This API and the code herein created by and wholly and privately owned by Kevin Jenkins except where specifically indicated otherwise.
// Licensed under the "RakNet" brand by Rakkarsoft LLC and subject to the terms of the relevant licensing agreement available at http://www.rakkarsoft.com
// UserNetworkStructures.h
// File created January 24, 2003
// Used to add user specific structures to send over the network
// ----------------------------------------------------------------------

#ifndef __USER_NETWORK_STRUCTURES_H
#define __USER_NETWORK_STRUCTURES_H

#pragma pack(push)

#include "NetworkTypes.h"
#include "PacketEnumerations.h"
#include <memory.h>

// Precede each struct with a #pragma pack(1)
// --------------------------
// YOUR STRUCTURES BELOW HERE!
// --------------------------

// REMOVE ME!
// EXAMPLE STRUCT
#pragma pack(1)
struct ChatMessageStruct
{
	unsigned char typeId; // ID_CHAT_MESSAGE
	// You could append data manually.  You could also put an array such as
	// char myMessage[50]; // Very inefficient and unsafe!  Hackers could overflow the array
	// Or you could use a BitStream (the best solution) as in
	// BitStream bitStream;
	// bitStream.Write((unsigned char)ID_CHAT_MESSAGE); bitStream.Write("Hello World", strlen("HelloWorld")+1);
	// With the case of the BitStream, you wouldn't need a struct at all.  See the documentation for more details.
};



// --------------------------
// YOUR STRUCTURES HERE!
// --------------------------







// --------------------------
// YOUR STRUCTURES ABOVE HERE!
// --------------------------

#pragma pack(pop)

#endif

