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
#include "GameRecord.h"

// Returns a singleton instance
GameClientModule* GameClientModule::Instance () 
{
	static GameClientModule inst;
	return &inst;
}


//--------------------------------------------------------------------------------------
// Process a custom Game packet received by the Engine Client
//--------------------------------------------------------------------------------------
void Client_ProcessGamePacket(Packet* p, unsigned char packetId)
{
	unsigned char* pMsgBuffer = p->data;

	switch(packetId)
	{
		case GAME_MSGID_CHAT:
		{
			if(!GamePlayback::Instance()->m_IsPlayingBack)
				g_Game.m_Console.PrintfColor(COLOR_RGBA(255,255,255,225),((GAMEMSG_CHAT*) p->data)->strChatString);
			break;
		}
		case GAME_MSGID_WORLDSTATE_FULL:
		{
			Game::Instance()->m_bIsLoading = false;
			break;
		}
	}
}

//--------------------------------------------------------------------------------------
// Loads a new map string received by the Engine Client
//--------------------------------------------------------------------------------------
World* Client_NewMap(string mapfile)
{
	g_Game.NewMap(mapfile,false);
	g_Game.m_World.Tick();

	if(GamePlayback::Instance()->m_IsPlayingBack)
		Game::Instance()->m_bIsLoading = false;

	return &g_Game.m_World;
}

//--------------------------------------------------------------------------------------
// Allows the Engine Client to print to the Game console
//--------------------------------------------------------------------------------------
void Client_PrintConsole(string msg, COLOR color)
{
	g_Game.m_Console.PrintfColor(color,msg.c_str());
}

//--------------------------------------------------------------------------------------
// Engine Client Event for custom response
//--------------------------------------------------------------------------------------
void Client_ConnectedToServer()
{
}

//--------------------------------------------------------------------------------------
// Engine Client Event for custom response
//--------------------------------------------------------------------------------------
void Client_LostConnection()
{
}


//--------------------------------------------------------------------------------------
// Sends a mouse input update at regular intervals to Server
//--------------------------------------------------------------------------------------
void GameClientModule::SendMouseUpdate(Actor* avatar)
{
	//only send mouse update if past minimum interval
	if(GSeconds - m_LastMouseUpdateTime > 1.f/Client::Instance()->m_ClientSettings.m_SendPacketsPerSecond)
	{
		m_LastMouseUpdateTime = GSeconds;

		if(avatar)
		{
			GAMEMSG_CLIENT_MOUSEUPDATE_POS msgUpdate;
			msgUpdate.dwType = GAME_MSGID_CLIENT_MOUSEUPDATE_POS;

			msgUpdate.playerPosX = avatar->Location.x;
			msgUpdate.playerPosY = avatar->Location.y;
			msgUpdate.playerPosZ = avatar->Location.z;

			msgUpdate.mouseYaw = Input::Instance()->mouseYaw;
			msgUpdate.mousePitch = Input::Instance()->mousePitch;
			Client::Instance()->Send((char*)&msgUpdate,sizeof(msgUpdate),UNRELIABLE_SEQUENCED,MEDIUM_PRIORITY,CLIENT_STREAM_MOUSE);
		}
		else
		{
			GAMEMSG_CLIENT_MOUSEUPDATE msgUpdate;
			msgUpdate.dwType = GAME_MSGID_CLIENT_MOUSEUPDATE;
			msgUpdate.mouseYaw = Input::Instance()->mouseYaw;
			msgUpdate.mousePitch = Input::Instance()->mousePitch;
			Client::Instance()->Send((char*)&msgUpdate,sizeof(msgUpdate),UNRELIABLE_SEQUENCED,MEDIUM_PRIORITY,CLIENT_STREAM_MOUSE);
		}
	}
}

//--------------------------------------------------------------------------------------
// Directly sends a key state, used by managed code that contains its own keys, and does its own state filtering
//--------------------------------------------------------------------------------------
void GameClientModule::SendKey(unsigned char NetworkKeyHandleID,bool isDown)
{
	GAMEMSG_CLIENT_KEY_DOWN msgKeyInput;

	if(isDown)
		msgKeyInput.dwType = GAME_MSGID_CLIENT_KEY_DOWN;
	else
		msgKeyInput.dwType = GAME_MSGID_CLIENT_KEY_UP;

	msgKeyInput.keyID = NetworkKeyHandleID;

	Client::Instance()->Send((char*)&msgKeyInput,sizeof(msgKeyInput),RELIABLE_ORDERED,HIGH_PRIORITY,CLIENT_STREAM_KEYS);
}

//--------------------------------------------------------------------------------------
// Registers custom Game Client message processing & event callbacks with the Engine's Client module
//--------------------------------------------------------------------------------------
void GameClientModule::RegisterGameClientCallBacks()
{
	Client::Instance()->m_ClientCallBack_ProcessGamePacket = Client_ProcessGamePacket;
	Client::Instance()->m_ClientCallBack_NewMap = Client_NewMap;
	Client::Instance()->m_ClientCallBack_PrintConsole = Client_PrintConsole;
	Client::Instance()->m_ClientCallBack_ConnectedToServer = Client_ConnectedToServer;
	Client::Instance()->m_ClientCallBack_LostServer = Client_LostConnection;
}
void GameClientModule::SendEnterGame()
{
	if(GamePlayback::Instance()->m_IsPlayingBack)
		return;

	if(Server::Instance()->m_ServerStarted)
		Server::Instance()->EnterGame(Server::Instance()->ServerNetworkClient);
	else
	{
		GAMEMSG_GENERIC msgFinishedLoad;
		msgFinishedLoad.dwType = GAME_MSGID_CLIENT_FINISHED_LOADING;
		Client::Instance()->Send((char*)&msgFinishedLoad,sizeof(msgFinishedLoad),RELIABLE_ORDERED,HIGH_PRIORITY);
	}
}