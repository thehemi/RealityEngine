// This file is part of RakNet Copyright 2003, 2004 Rakkarsoft LLC and Kevin Jenkins.

// Usage of Raknet is subject to the appropriate licence agreement.
// "Shareware" Licensees with Rakkarsoft LLC are subject to the shareware license found at http://www.rakkarsoft.com/shareWareLicense.html which you agreed to upon purchase of a "Shareware license"
// "Commercial" Licensees with Rakkarsoft LLC are subject to the commercial license found at http://www.rakkarsoft.com/sourceCodeLicense.html which you agreed to upon purchase of a "Commercial license"
// All other users are subject to the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

// Refer to the appropriate license agreement for distribution, modification, and warranty rights.

#ifndef __RAK_VOICE_FACTORY_H
#define __RAK_VOICE_FACTORY_H

class RakVoiceInterface;

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
// Unix needs no export, but for name mangling, keep the function name
// clean. If you omit the 'extern "C"', the .so names will be
// compiler dependent.
#define EXPORT extern "C"
#endif

#if defined(_USRDLL)
class EXPORT RakVoiceFactory
#else
class RakVoiceFactory
#endif
{
public:
	// Returns a new instance of the network server.  Pass which port you want the socket to use
	static RakVoiceInterface* GetRakVoiceInterface(void);

	// Destroys an instance of the network server;
	static void DestroyRakVoiceInterface(RakVoiceInterface* i);
};

#endif
