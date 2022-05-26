//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
// 
//====================================================================================
#include "stdafx.h"
#include "NetShared.h"
#include <tchar.h>
#include "..\RakNet\GetTime.h"
#include "..\RakNet\RakServerInterface.h"
#include "..\RakNet\PacketEnumerations.h"
#include "..\RakNet\RakNetworkFactory.h"
#include "GameEngine.h"

// Returns a singleton instance
GameServerModule* GameServerModule::Instance () 
{
	static GameServerModule inst;
	return &inst;
}

//--------------------------------------------------------------------------------------
// Process a custom Game packet received by the Engine Server, including Client input states
//--------------------------------------------------------------------------------------
void Server_ProcessGamePacket(NetworkClient* client, Packet* p, unsigned char packetId)
{
	unsigned char* pMsgBuffer = p->data;

	switch(packetId)
	{
		// HANDLE CLIENTS INPUT MESSAGES

	case GAME_MSGID_CLIENT_MOUSEUPDATE:
		{
			if(client->GetActorAvatar())
			{
				GAMEMSG_CLIENT_MOUSEUPDATE* msgUpdate = (GAMEMSG_CLIENT_MOUSEUPDATE*)pMsgBuffer;
				((Actor*)(client->GetActorAvatar()))->Server_HandleNetworkMouseUpdate(msgUpdate->mouseYaw,msgUpdate->mousePitch);
			}

			break;
		}
	case GAME_MSGID_CLIENT_KEY_DOWN:
		{
			if(client->GetActorAvatar())
			{
				GAMEMSG_CLIENT_KEY_DOWN* msgKey = (GAMEMSG_CLIENT_KEY_DOWN*)pMsgBuffer;
				((Actor*)(client->GetActorAvatar()))->Server_HandleNetworkKeyInput(true,msgKey->keyID);
			}
			break;
		}
	case GAME_MSGID_CLIENT_KEY_UP:
		{
			if(client->GetActorAvatar())
			{
				GAMEMSG_CLIENT_KEY_UP* msgKey = (GAMEMSG_CLIENT_KEY_UP*)pMsgBuffer;
				((Actor*)(client->GetActorAvatar()))->Server_HandleNetworkKeyInput(false,msgKey->keyID);
			}
			break;
		}
	case GAME_MSGID_CLIENT_MOUSEUPDATE_POS:
		{
			if(client->GetActorAvatar())
			{
				GAMEMSG_CLIENT_MOUSEUPDATE_POS* msgUpdate = (GAMEMSG_CLIENT_MOUSEUPDATE_POS*)pMsgBuffer;
				((Actor*)(client->GetActorAvatar()))->Location.Set(msgUpdate->playerPosX,msgUpdate->playerPosY,msgUpdate->playerPosZ);
				((Actor*)(client->GetActorAvatar()))->Server_HandleNetworkMouseUpdate(msgUpdate->mouseYaw,msgUpdate->mousePitch);
			}
			break;
		}
	}
}

//--------------------------------------------------------------------------------------
// Loads a new map string passed by the Engine Server after its network is initialized
//--------------------------------------------------------------------------------------
World* Server_NewMap(string mapfile)
{
	g_Game.NewMap(mapfile,true);
	Game::Instance()->m_bIsLoading = false;
	return &g_Game.m_World;
}

//--------------------------------------------------------------------------------------
// Allows the Engine Server to print to the Game console
//--------------------------------------------------------------------------------------
void Server_PrintConsole(string msg, COLOR color)
{
	g_Game.m_Console.PrintfColor(color,msg.c_str());
}

//--------------------------------------------------------------------------------------
// Registers custom Game Server message processing & event callbacks with the Server's Client module
//--------------------------------------------------------------------------------------
void GameServerModule::RegisterGameServerCallBacks()
{
	Server::Instance()->m_ServerCallBack_ProcessGamePacket = Server_ProcessGamePacket;
	Server::Instance()->m_ServerCallBack_NewMap = Server_NewMap;
	Server::Instance()->m_ServerCallBack_PrintConsole = Server_PrintConsole;
}