//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
// GameClientModule: Game-specific interfacing with Client network system, 
// for custom Client network message sending & handling
//====================================================================================

#ifndef GAMECLIENTMODULE_INCLUDED
#define GAMECLIENTMODULE_INCLUDED


//-----------------------------------------------------------------------------
// GameClientModule: Game-specific interfacing with Client network system, 
// for custom Client network message sending & handling
//-----------------------------------------------------------------------------
class GAME_API GameClientModule
{
public:
	/// Singleton class
	static GameClientModule* Instance ();
	/// Sends a mouse yaw/pitch update to the Server (with redundancy and time interval limiting)
	void SendMouseUpdate(Actor* avatar);

	/// Directly sends a key state, used by managed code that contains its own keys, and does its own state filtering
	void SendKey(unsigned char NetworkKeyHandleID,bool isDown);

	void Initialize()
	{
		m_LastMouseUpdateTime = -BIG_NUMBER;
		RegisterGameClientCallBacks();
	}

	void SendEnterGame();

private:
	/// Last time sent mouse update, for time interval limiting as specified by the ClientSendPacketsPerSecond in the INI
	float m_LastMouseUpdateTime;
	/// Registers custom Game message processing & event callbacks with the Engine's Client module
	void RegisterGameClientCallBacks();
};

#endif