// This file is part of RakNet Copyright 2003, 2004 Rakkarsoft LLC and Kevin Jenkins.

// Usage of Raknet is subject to the appropriate licence agreement.
// "Shareware" Licensees with Rakkarsoft LLC are subject to the shareware license found at http://www.rakkarsoft.com/shareWareLicense.html which you agreed to upon purchase of a "Shareware license"
// "Commercial" Licensees with Rakkarsoft LLC are subject to the commercial license found at http://www.rakkarsoft.com/sourceCodeLicense.html which you agreed to upon purchase of a "Commercial license"
// All other users are subject to the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

// Refer to the appropriate license agreement for distribution, modification, and warranty rights.

#include "RakVoiceFactory.h"
#include "RakVoiceInterface.h"
#include "RakVoice.h"

////#include "MemoryManager.h"

// Returns a new instance of the network client.  Pass which port you want the socket to use
RakVoiceInterface* RakVoiceFactory::GetRakVoiceInterface(void)
{
	return new RakVoice;
}


// Destroys an instance of the network client;
void RakVoiceFactory::DestroyRakVoiceInterface(RakVoiceInterface* i)
{
	delete (RakVoice*)i;
}
