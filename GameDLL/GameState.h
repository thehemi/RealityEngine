//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
/// GameState functions
///
/// GameState: Important variables and state information pertaining to the Game
//====================================================================================
#pragma once
#ifndef GAMESTATE_H
#define GAMESTATE_H

//--------------------------------------------------------------------------------------
/// Global game-wide user preferences
//--------------------------------------------------------------------------------------
class GamePreferences
{
public:
	/// Print scene statistics to screen?
	bool m_ShowStats;
	/// Can player fly around without hitting geometry
	bool m_NoClip;
	/// Name used in client for networking
	string m_PlayerName;
	/// Recording a game?
	bool m_RecordGame;
	/// First or third person when in vehicles
	bool m_VehicleFirstPerson;
	/// Mouse up/down inverted
	bool m_InvertFootPitch;
	// Whether to draw player HUD elements
	bool m_DisplayHud;

	/// Defaults
	GamePreferences()
	{
		m_RecordGame	= false;
		m_ShowStats		= false;
		m_NoClip		= false;
		m_VehicleFirstPerson = false;
		m_InvertFootPitch	 = false;
		m_DisplayHud = true;
	}
};

extern GamePreferences g_GamePreferences;

#endif

