// ----------------------------------------------------------------------
// This API and the code herein created by and wholly and privately owned by Kevin Jenkins except where specifically indicated otherwise.
// Licensed under the "RakNet" brand by Rakkarsoft LLC and subject to the terms of the relevant licensing agreement available at http://www.rakkarsoft.com
// CoreNetworking.h
// File created January 24, 2003
// Primary definition file for internal use by the network engine
// ----------------------------------------------------------------------

#ifndef __CORE_NETWORKING_H
#define __CORE_NETWORKING_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "NetworkTypes.h"

// ************************
// CONSTANTS
// ************************

const int PING_TIMES_ARRAY_SIZE=10; // Must be >= 1

//#define _USE_BEGIN_THREAD_EX

const int IP_ADDRESS_LENGTH=22; // xxx.xxx.xxx.xxx:<port (5)>0
const unsigned char MAX_FILENAME_LENGTH=50;
const unsigned char MAX_EXTENSION_LENGTH=20;
const unsigned char MAX_DESTINATION_LENGTH=80;
const unsigned long BROADCAST_PLAYER_PING_INTERVAL=30000;
const unsigned long RESEED_INTERVAL=9000; // Change the random number seed every 90 seconds
const unsigned long KEEP_ALIVE_PING_INTERVAL = 4000;
const int relayStaticClientDataMaxIndex=32; // The most players we will relay static client data to
#define CONNECTION_VERIFICATION_PASSWORD_LENGTH 10

#endif
