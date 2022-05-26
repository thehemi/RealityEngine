//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
/// GameRecorder: Network data recording, for demo playback
//  Takes received(Client) or sent(Server) network packets and writes them to a file
/// GamePlayBack: Plays back game demo files. 
//  Reads stored packet from the file buffer and passes them off to Client as network messages
//====================================================================================
#ifndef GAMERECORDER_INCLUDED
#define GAMERECORDER_INCLUDED

#include <stdio.h>
#include "..\RakNet\NetworkTypes.h"

//--------------------------------------------------------------------------------------
/// GameRecorder: Network data recording, for demo playback
//  Takes received(Client) or sent(Server) network packets and writes them to a file
//--------------------------------------------------------------------------------------
class ENGINE_API GameRecorder
{
public:
	GameRecorder(){m_RecordFile = NULL;m_RecordGame=false;}
	// Creates a new recording file to write to
	void NewRecordFile(string filename);
	// Writes a network packet to the current record file
	void RecordNetworkMessage(unsigned char *data, const long length);
	// Stops recording and releases the File
	void ShutDown();
	bool m_RecordGame;
	static GameRecorder* Instance();
	void ResetStartTime(){RecordTimer = 0;}
	void Tick();
private:
	// Starting time fo the recording, so that packet writing timestamps can be made relative to the start of the recording
	float RecordTimer;
	// File handle to write recorded packets to
	FILE* m_RecordFile;
};

//--------------------------------------------------------------------------------------
/// GamePlayBack: Plays back game demo files. 
//  Reads stored packet from the file buffer and passes them off to Client as network messages
//--------------------------------------------------------------------------------------
class ENGINE_API GamePlayback
{
public:
	GamePlayback(){m_IsPlayingBack = false;}
	// Begins playback of session from the specified rec file
	bool Playback(string filename);
	// Filename of the current playback
	string m_PlaybackFilename;
	// Whether the system is currently playing back a session
	bool m_IsPlayingBack;
	// Stops playback of a session and resets state
	void ShutDown();
	// Updates playback logic every frame, to run through new packets over the frame's interval of time
	void Tick();
	static GamePlayback* Instance();
	// Current playback time in the recorded game
	float m_CurrentPlaybackTime;

private:
	// Stores one network message and the time it was received, for playback
	struct PlaybackPacket
	{
		float PlaybackTime;
		Packet packet;
		unsigned char* buffer;
	};
	// array of loaded packets to run through during playback
	vector<PlaybackPacket> playbackPackets;
	// Whether the Playback has run the first packet or not for immediate initial World loading (which will always be the first packet)
	bool m_RanFirstPacket;
	// Index of the last played packet in the packet array
	int m_CurrentPlaybackPacketIndex;
	// Total number of packets in the playback file
	int m_TotalPlaybackPackets;
};
#endif