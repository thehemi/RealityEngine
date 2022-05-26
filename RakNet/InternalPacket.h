// This file is part of RakNet Copyright 2003, 2004 Rakkarsoft LLC and Kevin Jenkins.

// Usage of Raknet is subject to the appropriate licence agreement.
// "Shareware" Licensees with Rakkarsoft LLC are subject to the shareware license found at http://www.rakkarsoft.com/shareWareLicense.html which you agreed to upon purchase of a "Shareware license"
// "Commercial" Licensees with Rakkarsoft LLC are subject to the commercial license found at http://www.rakkarsoft.com/sourceCodeLicense.html which you agreed to upon purchase of a "Commercial license"
// All other users are subject to the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

// Refer to the appropriate license agreement for distribution, modification, and warranty rights.

#ifndef __INTERNAL_PACKET_H
#define __INTERNAL_PACKET_H

#include "SHA1.h"
#include "PacketPriority.h"

// This must be able to hold the highest value of RECEIVED_PACKET_LOG_LENGTH.
typedef unsigned short PacketNumberType;

struct InternalPacket
{
	bool isAcknowledgement; // True if this is an acknowledgement packet
	PacketNumberType packetNumber; // The number of this packet, used as an identifier
	PacketPriority priority; // The priority level of this packet
	PacketReliability reliability; // What type of reliability algorithm to use with this packet
	unsigned char orderingStream; // What ordering stream this packet is on, if the reliability type uses ordering streams
	unsigned char orderingIndex; // The ID used as identification for ordering streams
	unsigned long splitPacketId; // The ID of the split packet, if we have split packets
	unsigned long splitPacketIndex; // If this is a split packet, the index into the array of split packets
	unsigned long splitPacketCount; // The size of the array of split packets
	unsigned long creationTime; // When this packet was created
	unsigned long nextActionTime; // The next time to take action on this packet
	unsigned long dataBitLength; // How many bits the data is
	char *data; // Buffer is a pointer to the actual data, assuming this packet has data at all
};

#endif

