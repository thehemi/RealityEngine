// This file is part of RakNet Copyright 2003, 2004 Rakkarsoft LLC and Kevin Jenkins.

// Usage of Raknet is subject to the appropriate licence agreement.
// "Shareware" Licensees with Rakkarsoft LLC are subject to the shareware license found at http://www.rakkarsoft.com/shareWareLicense.html which you agreed to upon purchase of a "Shareware license"
// "Commercial" Licensees with Rakkarsoft LLC are subject to the commercial license found at http://www.rakkarsoft.com/sourceCodeLicense.html which you agreed to upon purchase of a "Commercial license"
// All other users are subject to the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

// Refer to the appropriate license agreement for distribution, modification, and warranty rights.
#ifndef __INTERNAL_PACKET_POOL
#define __INTERNAL_PACKET_POOL
#include "SimpleMutex.h"
#include "Queue.h"
#include "InternalPacket.h"

class InternalPacketPool
{
public:
	InternalPacketPool();
	~InternalPacketPool();
	InternalPacket* GetPointer(void);
	void ReleasePointer(InternalPacket *p);
	void ClearPool(void);
	// static function because only static functions can access static members
	static inline InternalPacketPool* Instance() {return &I;}
private:
	static InternalPacketPool I;
	BasicDataStructures::Queue<InternalPacket*> pool;
	SimpleMutex poolMutex;
#ifdef _DEBUG
	int packetsReleased;
#endif
};

#endif

