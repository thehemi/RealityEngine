//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ==================
//
/// Game: The game-side input and render path for the entire game. 
/// Delegates responsibility to subclasses as needed in Game::Tick() and Game::FrameRender().
//============================================================================================
#pragma once
#ifndef GAME_H
#define GAME_H

//--------------------------------------------------------------------------------------
#include "GUIConsole.h"

//--------------------------------------------------------------------------------------
// Loose project name strings. Still need to change windows resources
#define PROJECT_NAME "RealityEngine" // "Cry Havoc"
#define PROJECT_NAME_UNICODE L"RealityEngine" // L"Cry Havoc"

//--------------------------------------------------------------------------------------
/// Game: The game-side input and render path for the entire game. 
/// Delegates responsibility to subclasses as needed in Game::Tick() and Game::FrameRender().
//--------------------------------------------------------------------------------------
class GAME_API Game 
{
protected:
	/// Keep a pointer to the engine
	class Engine* m_Engine;
	/// GUI/Game cursor
	bool m_CursorVisible;
	/// Dedicated server mode?
	bool m_IsDedicated;
	/// Playing a game or in the menus? Don't use this, use IsInGame()
	bool m_IsInGame;
	/// GUI Engine
	class GUISystem*	m_GUISystem;

	/// Cursor state
	void UpdateCursorVisible();

	/// In-game state
	bool IsInGame();

public:

	Game();
	~Game(){}

	/// Exported functions used by exes, Reality Builder, etc
	/// See Game.h/.cpp for the exports
	/// We use LoadLibrary() to link dynamically
	void	NewMap(string mapfile,bool IsServer);
	/// Core logic tick, no rendering
	void	Tick();
	/// Core initialize
	void	Initialize(HWND topHwnd, HWND childHwnd, HINSTANCE hInst, string cmdLine, bool isDedicated);
	/// Core Shutdown
	void	Shutdown();
	/// Windows message pump, for Win32 clients only
	LRESULT HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	/// Misc
	bool GetCursorVisible(){return m_CursorVisible;}
	///
	void SetCursorVisible(bool visible);
	/// Dedicated server mode
	bool IsDedicated(){return m_IsDedicated;}
	/// Used by GUI/input classes
	bool HasAnyGUIopen();
	/// Callback function for rendering, needs to be public
	void FrameRender();
	/// Called by InputHandler for toggling menu
	void ToggleMainMenu();
	/// Prints common performance statistics onto the canvas
	void PrintStats(Canvas* canvas);

	/// Current world
	World		m_World;

	/// Console GUI
	GUIConsole	m_Console;
	/// false if run from Reality Builder, allowing extra behavior specific to editor mode
	bool g_IsGameApp;

	static Game* Instance();

	void SetLocalAvatarActor(Actor* avatar);

	/// Currently loaded map
	string m_CurrentMap;
	// timestamp of the last level load, set by game upon level load
	float m_LastMapLoadedAtTime;

	/// When in slow-motion or pause mode, the real delta between frames, necessary for some systems
	float m_ActualDeltaTime;

	/// Set the scale of game time for slowmo/fastmo effects
	void  SetTickSpeedScale(float scaleFactor = 1.f);
	/// Get the scale of game time for slowmo/fastmo effects
	float GetTickSpeedScale(){return m_TickSpeedScale;}

	/// How much faster/slower is the game running than realtime?
	float m_TickSpeedScale;
	void ShowChat();
	void HideChat();
	bool m_bShowStats;
	bool m_bIsLoading;
	void DrawLoadingScreen(float percent = 0);
	private:
	Texture m_LoadingBGTex;
	int CTRL_APP_TAKE_SCREENSHOT;
	int CTRL_APP_TOGGLE_GUI;
	int CTRL_APP_CHAT;
	void UpdateInput();

};

/// Global game
extern Game g_Game;

#endif





