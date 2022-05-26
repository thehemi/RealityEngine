//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
// 
//====================================================================================
#include "stdafx.h"
#include "GameRecord.h"
#include "Client.h"
#include "Server.h"
#include "Server.h"
#include "dxstdafx.h"
#include "GUISystem.h"
//#include "GameEngine.h"
//#include "DemoPlayer.h"
#include "..\RakNet\PacketEnumerations.h"

//--------------------------------------------------------------------------------------
// Creates a new recording file to write to
//--------------------------------------------------------------------------------------
void GameRecorder::NewRecordFile(string filename)
{
	if(m_RecordFile)
	{
		fclose(m_RecordFile);
		m_RecordFile = NULL;
	}
	string extension = ".rec";
	int numAppend = 0;
	string testFileName = filename;
	while(FindMedia(testFileName + extension,"Recordings"))
	{
		numAppend++;
		testFileName = filename + ToStr(numAppend);
	}
	filename = testFileName;
	filename += extension;

	string dirPathFile = "..\\Recordings\\"; 
	dirPathFile += filename;
	m_RecordFile = fopen(dirPathFile.c_str(), "wb");
	RecordTimer = 0;
	//g_Game.m_Console.Printf("---%s GAME RECORDING ACTIVE ---",filename.c_str());
}

void GameRecorder::Tick()
{
	float deltaTime = GDeltaTime;
	if(deltaTime > .5)
			deltaTime = .5;
	RecordTimer += deltaTime;
}

//--------------------------------------------------------------------------------------
// Writes a network packet to the current record file
//--------------------------------------------------------------------------------------
void GameRecorder::RecordNetworkMessage(unsigned char* data, const long length)
{
	if(!m_RecordFile)
		return;
		
	fwrite(&RecordTimer, 1, sizeof(RecordTimer), m_RecordFile);
	fwrite(&length, 1, sizeof(length), m_RecordFile);

	if((unsigned char) data[0] == ID_TIMESTAMP)
	{
		unsigned long latency = Client::Instance()->GetLatency() * 1000.f;
		memcpy(&data[1],&latency, sizeof(unsigned long));
	}

	fwrite(data, 1, length, m_RecordFile);
	fflush(m_RecordFile);
}

//--------------------------------------------------------------------------------------
// Stops recording and releases the File
//--------------------------------------------------------------------------------------
void GameRecorder::ShutDown()
{
	if(m_RecordFile)
	{
		fclose(m_RecordFile);
		m_RecordFile = NULL;
	}
}

//--------------------------------------------------------------------------------------
// Begins playback of session from the specified rec file
//--------------------------------------------------------------------------------------
bool GamePlayback::Playback(string filename)
{
	ShutDown();

	m_PlaybackFilename = filename;

	if(!FindMedia(filename,"Recordings"))
		return false;

	HANDLE hFile= CreateFile(filename.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);  
	if(hFile==INVALID_HANDLE_VALUE)
		return false;

	// get the filesize;   
	DWORD FileSize = GetFileSize(hFile,0); 
	CloseHandle(hFile);

	FILE* PlayBackFile = fopen(filename.c_str(), "rb");
	char* totalPlaybackBuffer = new char[FileSize];
	int bufferLength = 0;
	bufferLength = fread(totalPlaybackBuffer, 1, FileSize, PlayBackFile); 

	int bufferPos = 0;

	while(bufferPos < bufferLength)
	{
		int index = playbackPackets.size();
		playbackPackets.resize(index + 1);

		memcpy(&playbackPackets[index].PlaybackTime,&totalPlaybackBuffer[bufferPos],sizeof(float));
		bufferPos += sizeof(float);

		memcpy(&playbackPackets[index].packet.length,&totalPlaybackBuffer[bufferPos],sizeof(long));
		bufferPos += sizeof(long);

		playbackPackets[index].buffer = new unsigned char[playbackPackets[index].packet.length];

		memcpy(&playbackPackets[index].buffer[0],&totalPlaybackBuffer[bufferPos],playbackPackets[index].packet.length);
		bufferPos += playbackPackets[index].packet.length;

		playbackPackets[index].packet.data = playbackPackets[index].buffer;
		playbackPackets[index].packet.bitSize = playbackPackets[index].packet.length * 8;
		playbackPackets[index].packet.playerId.binaryAddress = 0;
		playbackPackets[index].packet.playerId.port = 0;
	}

	fclose(PlayBackFile);
	PlayBackFile = NULL;
	delete[] totalPlaybackBuffer;

	m_CurrentPlaybackTime = 0;
	m_IsPlayingBack = true;
	m_CurrentPlaybackPacketIndex = 0;
	m_TotalPlaybackPackets = playbackPackets.size();

	m_RanFirstPacket = false;

	return true;
}

//--------------------------------------------------------------------------------------
// Stops playback of a session and resets state
//--------------------------------------------------------------------------------------
void GamePlayback::ShutDown()
{
	if(m_IsPlayingBack)
	{
		for(int index = 0; index < playbackPackets.size(); index++)
			delete[] playbackPackets[index].buffer;

		playbackPackets.erase(playbackPackets.begin(),playbackPackets.end());
	}

	m_IsPlayingBack = false;
}

//--------------------------------------------------------------------------------------
// Updates playback logic every frame, to run through new packets over the frame's interval of time
//--------------------------------------------------------------------------------------
void GamePlayback::Tick()
{
	if(m_IsPlayingBack)
	{
		if(m_CurrentPlaybackPacketIndex >= m_TotalPlaybackPackets - 1)
			return;

		float deltaTime = GDeltaTime;
		if(deltaTime > .5)
			deltaTime = .5;

		m_CurrentPlaybackTime += deltaTime;

		for(int i = m_CurrentPlaybackPacketIndex; i < m_TotalPlaybackPackets;i++)
		{

			if(m_CurrentPlaybackTime >= playbackPackets[i].PlaybackTime)
			{
				playbackPackets[i].packet.data = playbackPackets[i].buffer;
				Client::Instance()->ProcessGamePacket(&playbackPackets[i].packet,((GAMEMSG_GENERIC*)playbackPackets[i].packet.data)->dwType);
				/*if(!m_RanFirstPacket)
				{
					m_RanFirstPacket = true;
//					g_Game.m_Console.Printf("---%s PLAYBACK BEGUN ---",m_PlaybackFilename.c_str());
//					if(!DemoPlayer::Instance())
//						new DemoPlayer(&g_Game.m_World,Matrix());
					m_CurrentPlaybackTime = playbackPackets[i+1].PlaybackTime+1;
				}*/
				m_CurrentPlaybackPacketIndex = i+1;
			}
			else
				break;
		}


		if(m_CurrentPlaybackPacketIndex >= m_TotalPlaybackPackets - 1)
		{
			Server::Instance()->m_ServerCallBack_PrintConsole("Demo Complete",COLOR_RGBA(255,255,255,200));
			GUISystem::Instance()->ShowDesktop(true);
			Engine::Instance()->RenderSys->ShowCursor(true,false);
		}

		//Server::Instance()->BeginHosting("Playback",m_PlaybackFilename,false);

		//g_Game.m_Console.Printf("---%s PLAYBACK FINISHED, TOTAL TIME %f ---",m_PlaybackFilename.c_str(),m_CurrentPlaybackTime);
	}
}

GamePlayback* GamePlayback::Instance () 
{
    static GamePlayback inst;
    return &inst;
}

GameRecorder* GameRecorder::Instance () 
{
    static GameRecorder inst;
    return &inst;
}