// This file is part of RakNet Copyright 2003, 2004 Rakkarsoft LLC and Kevin Jenkins.

// Usage of Raknet is subject to the appropriate licence agreement.
// "Shareware" Licensees with Rakkarsoft LLC are subject to the shareware license found at http://www.rakkarsoft.com/shareWareLicense.html which you agreed to upon purchase of a "Shareware license"
// "Commercial" Licensees with Rakkarsoft LLC are subject to the commercial license found at http://www.rakkarsoft.com/sourceCodeLicense.html which you agreed to upon purchase of a "Commercial license"
// All other users are subject to the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

// Refer to the appropriate license agreement for distribution, modification, and warranty rights.

#ifdef __USE_IO_COMPLETION_PORTS

#ifndef __ASYNCHRONOUS_FILE_IO_H
#define __ASYNCHRONOUS_FILE_IO_H

#ifdef _WIN32
#ifdef __USE_IO_COMPLETION_PORTS
#include <WinSock2.h>
#else
#include <WinSock.h>
#endif
#include <windows.h>
#endif
#include "SimpleMutex.h"
struct ExtendedOverlappedStruct;

class AsynchronousFileIO
{
	public:
	AsynchronousFileIO();
	~AsynchronousFileIO();
	bool AssociateSocketWithCompletionPort(SOCKET socket, DWORD dwCompletionKey);
	#endif
	static inline AsynchronousFileIO* Instance() {return &I;}
	void IncreaseUserCount(void);
	void DecreaseUserCount(void);
	void Shutdown(void);
	int GetUserCount(void);

	unsigned threadCount;
	bool killThreads;
	private:
	HANDLE completionPort;
	SimpleMutex userCountMutex;
	SYSTEM_INFO systemInfo;
	int userCount;

	static AsynchronousFileIO I;
};

unsigned __stdcall ThreadPoolFunc(LPVOID arguments);
void WriteAsynch(HANDLE handle, ExtendedOverlappedStruct *extended);
BOOL ReadAsynch(HANDLE handle, ExtendedOverlappedStruct *extended);

#endif
