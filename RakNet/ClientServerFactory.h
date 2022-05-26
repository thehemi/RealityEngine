// ----------------------------------------------------------------------
// This API and the code herein created by and wholly and privately owned by Kevin Jenkins except where specifically indicated otherwise.
// Licensed under the "RakNet" brand by Rakkarsoft LLC and subject to the terms of the relevant licensing agreement available at http://www.rakkarsoft.com
// ClientServerFactory.h
// File created January 24, 2003
// Returns instances of the network server or network client
// ----------------------------------------------------------------------


#ifndef __CLIENT_SERVER_FACTORY_H
#define __CLIENT_SERVER_FACTORY_H

class RakClientInterface;
class RakServerInterface;

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
// Unix needs no export, but for name mangling, keep the function name
// clean. If you omit the 'extern "C"', the .so names will be
// compiler dependent.
#define EXPORT extern "C"
#endif

#if defined(DLL_EXPORTS) || defined(_USRDLL)
class EXPORT ClientServerFactory
#else
//class __declspec( dllimport ) ClientServerFactory
class ClientServerFactory
#endif
{
public:
	// Returns a new instance of the network client.  Pass which port you want the socket to use
	static RakClientInterface* GetRakClientInterface(void);

	// Returns a new instance of the network server.  Pass which port you want the socket to use
	static RakServerInterface* GetRakServerInterface(void);

	// Destroys an instance of the network client;
	static void DestroyRakClientInterface(RakClientInterface* i);

	// Destroys an instance of the network server;
	static void DestroyRakServerInterface(RakServerInterface* i);
};

#endif
