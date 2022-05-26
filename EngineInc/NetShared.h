//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
/// NetShared
/// Network message definitions & data streams that both the Engine and Game should know about
/// Author: Jeremy Stieglitz
//====================================================================================

#ifndef NETSHARED_INCLUDED
#define NETSHARED_INCLUDED
#include "..\RakNet\NetworkTypes.h"

/// Network streams. Used for Ordering and Sequencing
#define SERVER_STREAM_WORLDSTATE_RELIABLE 1
#define SERVER_STREAM_WORLDSTATE_UNRELIABLE 2
#define CLIENT_STREAM_MOUSE 3
#define CLIENT_STREAM_KEYS 4
#define SERVER_STREAM_KEEPALIVE 5

#define MAX_CHATSTRING_LENGTH	508
#define MAX_PLAYERNAME_LENGTH	64
#define MAX_WORLDSTATE_FULL	20000
#define MAX_WORLDSTATE_UPDATE 9000

#define ALL_CLIENTS 0
//-----------------------------------------------------------------------------
/// App specific network messages and structures 
//-----------------------------------------------------------------------------
enum 
{
	/// Sent by server
	GAME_MSGID_LOAD_WORLD	= 30, /// Start at 25, since below this is reserved for internal RakNet messages
	/// Sent by client
	GAME_MSGID_CLIENT_SETTINGS_INFO,
	GAME_MSGID_CLIENT_FINISHED_LOADING,

	/// Chat
	GAME_MSGID_CHAT,		/// This is only sent by the server

	GAME_MSGID_OBSERVER_NAMES,		/// This is only sent by the server
	GAME_MSGID_WORLDSTATE_FULL,		/// This is only sent by the server
	GAME_MSGID_WORLDSTATE_UPDATE_UNRELIABLE,		/// This is only sent by the server
	GAME_MSGID_WORLDSTATE_UPDATE_RELIABLE,		/// This is only sent by the server
	GAME_MSGID_ENDGAME,		/// This is only sent by the server
	GAME_MSGID_KEEPALIVE,		/// This is only sent by the server

	GAME_MSGID_CLIENT_MOUSEUPDATE,		/// This is only sent by the client
	GAME_MSGID_CLIENT_MOUSEUPDATE_POS,		/// This is only sent by the client
	GAME_MSGID_CLIENT_KEY_DOWN,		/// This is only sent by the client
	GAME_MSGID_CLIENT_KEY_UP,		/// This is only sent by the client

	GAME_MSGID_BUILD_VERSION
};
// Change compiler pack alignment to be BYTE aligned so that size is consistent
#pragma pack( push, 1 )

/// Base network message structure, just containing header identifier byte
struct GAMEMSG_GENERIC
{
	unsigned char dwType;
};
#ifndef DOXYGEN_IGNORE
struct GAMEMSG_CHAT : public GAMEMSG_GENERIC
{
	char strChatString[MAX_CHATSTRING_LENGTH];
};
struct GAMEMSG_LOAD_WORLD : public GAMEMSG_GENERIC
{
	char mapName[MAX_PATH];
};
struct GAMEMSG_CLIENT_SETTINGS_INFO : public GAMEMSG_GENERIC
{
	DWORD RecievePacketsPerSeconds;
	float buildVersion;
	char playerName[MAX_PLAYERNAME_LENGTH];
};
struct GAMEMSG_BUILD_VERSION : public GAMEMSG_GENERIC
{
	float buildVersion;	
};
struct GAMEMSG_OBSERVER_NAMES : public GAMEMSG_GENERIC
{
	char observerNames[MAX_CHATSTRING_LENGTH];
};
struct GAMEMSG_CLIENT_MOUSEUPDATE_POS : public GAMEMSG_GENERIC
{
	float mouseYaw;
	float mousePitch;
	float playerPosX;
	float playerPosY;
	float playerPosZ;
};
#endif
/// Client sends mouse updates to Server
struct GAMEMSG_CLIENT_MOUSEUPDATE : public GAMEMSG_GENERIC
{
	float mouseYaw;
	float mousePitch;
};
/// Client sends key states to Server
struct GAMEMSG_CLIENT_KEY_DOWN : public GAMEMSG_GENERIC
{
	unsigned char keyID;
};
/// Client sends key states to Server
struct GAMEMSG_CLIENT_KEY_UP : public GAMEMSG_GENERIC
{
	unsigned char keyID;
};
/// Server sends World state updates to Client
struct GAMEMSG_WORLDSTATE_UPDATE
{
	unsigned char useTimeStamp; /// Assign this to ID_TIMESTAMP
	unsigned long timeStamp; /// Put the system time in here returned by timeGetTime()
	unsigned char dwType;
	unsigned short WorldStateDataSize;
	char WorldStateData[MAX_WORLDSTATE_UPDATE];
};

/// Pop the old pack alignment
#pragma pack( pop )

#endif