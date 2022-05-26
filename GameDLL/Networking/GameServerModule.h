//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
// GameServerModule: Game-specific interfacing with Server network system, 
// for custom Server network message sending & handling
//====================================================================================

#ifndef GAMESERVERMODULE_INCLUDED
#define GAMESERVERMODULE_INCLUDED


//--------------------------------------------------------------------------------------
// GameServerModule: Game-specific interfacing with Server network system, 
// for custom Server network message sending & handling
//--------------------------------------------------------------------------------------
class GameServerModule
{
public:
	/// Singleton class
	static GameServerModule* Instance ();
	void Initialize()
	{
		RegisterGameServerCallBacks();
	}
private:
	/// Registers custom Game Server message processing & event callbacks with the Engine's Server module
	void RegisterGameServerCallBacks();
};
#endif