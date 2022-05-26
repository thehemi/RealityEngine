// This file is part of RakNet Copyright 2003, 2004 Rakkarsoft LLC and Kevin Jenkins.

// Usage of Raknet is subject to the appropriate licence agreement.
// "Shareware" Licensees with Rakkarsoft LLC are subject to the shareware license found at http://www.rakkarsoft.com/shareWareLicense.html which you agreed to upon purchase of a "Shareware license"
// "Commercial" Licensees with Rakkarsoft LLC are subject to the commercial license found at http://www.rakkarsoft.com/sourceCodeLicense.html which you agreed to upon purchase of a "Commercial license"
// All other users are subject to the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

// Refer to the appropriate license agreement for distribution, modification, and warranty rights.

#ifndef __SOCKET_LAYER_H
#define __SOCKET_LAYER_H

#ifdef _WIN32
#ifdef __USE_IO_COMPLETION_PORTS
#include <WinSock2.h> // DON'T FORGET TO INLCLUDE Ws2_32.lib to your project!
#else
#include <WinSock.h>
#endif
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
typedef int SOCKET;
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#endif
#include "ClientContextStruct.h"

class RakPeer;

class SocketLayer
{
	public:
	SocketLayer();
	~SocketLayer();
	static inline SocketLayer* Instance() {return &I;}
	SOCKET Connect(SOCKET writeSocket, unsigned long binaryAddress, unsigned short port);
	SOCKET CreateBoundSocket(unsigned short port, bool blockingSocket); // Creates a socket to listen for incoming connections on the specified port
	const char* DomainNameToIP(const char *domainName);
	#ifdef __USE_IO_COMPLETION_PORTS
	void AssociateSocketWithCompletionPort(SOCKET socket, ClientContextStruct* completionKey);
#endif
	// Start an asynchronous read using the specified socket.  The callback will use the specified PlayerID (associated with this socket) and call either the client or the server callback (one or the other should be 0)
	bool AssociateSocketWithCompletionPortAndRead(SOCKET readSocket, unsigned long binaryAddress, unsigned short port, RakPeer* rakPeer);
	void Write(SOCKET writeSocket, const char* data, int length);
	// Returns true if you successfully read data
	bool RecvFrom(SOCKET s, RakPeer *rakPeer, int *errorCode);
	void GetMyIP(char ipList[10][16]);
	int SendTo(SOCKET s, char *data, int length, char ip[16], unsigned short port);
	int SendTo(SOCKET s, char *data, int length, unsigned long binaryAddress, unsigned short port);
	
	private:
	static bool socketLayerStarted;
	#ifdef _WIN32
	static WSADATA winsockInfo;
	#endif
	static SocketLayer I;
};

#endif

