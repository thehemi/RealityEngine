//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
/// Core Game Engine, powers all game systems and ticks all engine systems
///
/// Author: Artificial Studios
/// Last Update: 20.11.2004
//====================================================================================
#include "stdafx.h"
#include <io.h>
#include "Game.h"
#include "GameEngine.h"
#include "GUISystem.h"
#include "StreamingOgg.h"
#include "Editor.h"
#include "CameraHandler.h"
#include "FXmanager.h"
#include "SkyController.h"
#include "..\EngineInc\GUISystem.h"
#include "Profiler.h"

//--------------------------------------------------------------------------------------
// Globals
//--------------------------------------------------------------------------------------
// Global game engine class
Game g_Game;

//--------------------------------------------------------------------------------------
// Python script module initialization routines
//--------------------------------------------------------------------------------------
extern "C" void initgame(void);
extern "C" void initvectorc(void);
void RegisterConsoleFunctions(GUIConsole& console);

//-----------------------------------------------------------------------------
// Loading callback
//-----------------------------------------------------------------------------
// TODO: Ick, remove globals
float  g_Percent = 0;
string g_Status;
void WorldLoadProgress(float percent, string status)
{
    g_Status = status;
    g_Percent = percent;
    Game::Instance()->m_bIsLoading = true;

    // Redraw loading screen for every percent. If we did it for every fraction
    // it would hurt loading performance
    static float lastPercent = 0;
    if(fabsf(percent-lastPercent) > 1)
    {
	    RenderDevice::Instance()->DoRendering();
        lastPercent = percent;
    }
}

//-----------------------------------------------------------------------------
// CTOR sets some initial values
//-----------------------------------------------------------------------------
Game::Game()
{
	m_Engine = NULL;
	m_ActualDeltaTime = 0;
	m_LastMapLoadedAtTime = -BIG_NUMBER;
	m_TickSpeedScale = 1;
	m_GUISystem = GUISystem::Instance();
}

//--------------------------------------------------------------------------------------
// Pump the windows message loop
//--------------------------------------------------------------------------------------
void Sys_SendKeyEvents(void)
{
	MSG        msg;
	while (PeekMessage (&msg, NULL, 0, 0, PM_NOREMOVE))
	{
		if (!GetMessage (&msg, NULL, 0, 0))
			exit(0);
		TranslateMessage (&msg);
		DispatchMessage (&msg);
	}
}

//-----------------------------------------------------------------------------
// Sets cursor visibility
// When invisible, mouse position is centered in window for mouse-look
//-----------------------------------------------------------------------------
void Game::SetCursorVisible(bool visible) 
{ 
	if(m_Engine->RenderSys)
		m_Engine->RenderSys->ShowCursor(visible,!visible);
	m_CursorVisible = visible; 
}

/// Set the scale of game time for slowmo/fastmo effects
void Game::SetTickSpeedScale(float scaleFactor)
{
    if(m_TickSpeedScale != scaleFactor)
	{
		float AudioSpeed = scaleFactor;
		if(AudioSpeed > 1)
			AudioSpeed = 1;
        AudioDevice::Instance()->SetPlaybackSpeed(AudioSpeed);
	}
    m_TickSpeedScale=scaleFactor;
}

//-----------------------------------------------------------------------------
// Shuts down Game, only call before exiting application
//-----------------------------------------------------------------------------
void Game::Shutdown()
{
	if(!m_Engine)
		return;

	//set out far so that we don't get any 3d sounds blaring upon exiting
	if(!Editor::Instance()->GetEditorMode())
	{
		CameraHandler::Instance()->GetCamera()->Location.Set(1000,1000,1000);
		m_Engine->Update(CameraHandler::Instance()->GetCamera());
	}

	m_Engine = 0;

	SetTickSpeedScale(1);

	if(!IsDedicated() && oggPlayer.IsPlaying())
		oggPlayer.Stop();

	// Shut down network systems
	Server::Instance()->Shutdown();
	Client::Instance()->Shutdown();

	m_World.UnLoad();
	m_Console.Clear();

	GUISystem::Instance()->Shutdown();
	Precacher::PurgeCache();

	Engine::Instance()->Shutdown();
}

//-----------------------------------------------------------------------------
// A handy functionto return mouse position in pixels relative to window on screen
//-----------------------------------------------------------------------------
POINT GetMousePos()
{
	POINT p;
	RECT clientR;
	GetCursorPos(&p);
	HWND hWnd = Engine::Instance()->hWnd;
	GetClientRect(hWnd, &clientR);
	ClientToScreen(hWnd, (POINT *)&clientR.left);
	ClientToScreen(hWnd, (POINT *)&clientR.right);
	p.x-=clientR.left;
	p.y-=clientR.top;
	return p;
}

//--------------------------------------------------------------------------------------
// This callback function will be called at the end of every frame to perform all the 
// rendering calls for the scene, and it will also be called if the window needs to be 
// repainted. After this function has returned, the engine framework will call 
// IDirect3DDevice9::Present to display the contents of the next buffer in the swap chain
// - try to keep CPU intensive code outside of this section
//--------------------------------------------------------------------------------------
void CALLBACK GFrameRender(){ g_Game.FrameRender(); } // Global callback to call the member function

//-----------------------------------------------------------------------------
// Render path for Game
//-----------------------------------------------------------------------------
void Game::FrameRender()
{
	// If the RB Editor Mode is active, pass the rendering to it
	if(Editor::Instance()->GetEditorMode())
	{
		Editor::Instance()->Render(&m_World);
		return;
	}

	// Get the canvas and do some drawing
	Canvas* canvas = m_Engine->RenderSys->GetCanvas();
	canvas->SetScaling(canvas->Width/1024.f,canvas->Height/768.f);

	if(m_bIsLoading)
	{
		DrawLoadingScreen(g_Percent);
		return;
	}

	// Render the world!
	m_World.Render(CameraHandler::Instance()->GetCamera());
	Engine::Instance()->RenderSys->EndHDR();

	// Set default states
	RenderDevice::Instance()->ResetAllStates();
	RenderWrap::SetView(CameraHandler::Instance()->GetCamera()->view);
	RenderWrap::SetProjection(CameraHandler::Instance()->GetCamera()->projection);
	RenderWrap::SetWorld(Matrix());
	RenderWrap::SetRS( D3DRS_ZWRITEENABLE, FALSE );
	RenderWrap::SetRS( D3DRS_ALPHABLENDENABLE,TRUE );
	RenderWrap::SetRS( D3DRS_LIGHTING,FALSE );
	RenderWrap::SetRS( D3DRS_FOGENABLE,FALSE );
	RenderWrap::SetSS( 0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
	RenderWrap::SetSS( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	RenderWrap::SetSS( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	RenderWrap::SetTSS(0,D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	RenderWrap::SetTSS(0,D3DTSS_ALPHAARG1,D3DTA_TEXTURE );
	RenderWrap::SetTSS(0,D3DTSS_ALPHAARG2,D3DTA_DIFFUSE );
	RenderWrap::SetTSS( 0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	RenderWrap::SetTSS( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	RenderWrap::SetTSS( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

	if(IsInGame())
	{
		// Render post effects, disabling zwrites
		m_World.PostRender(CameraHandler::Instance()->GetCamera());
		// Add it all to our timer
		RenderWrap::SetRS(D3DRS_ZWRITEENABLE,TRUE);

		// Statistic printing to the screen
		if(m_bShowStats)
			PrintStats(canvas);

		// Render in-game console on top of everything
		m_Console.Render(canvas);
	}
	else
	{
		// We're in the menu system, do standard rendering/clearing
		RenderWrap::SetRS(D3DRS_FOGENABLE,FALSE);
		RenderWrap::dev->Clear( 0L, NULL, D3DCLEAR_ZBUFFER/*|D3DCLEAR_STENCIL*/,0, 1.0f, 0L );
		RenderDevice::Instance()->GetCanvas()->Box(COLOR_RGBA(0,0,0,255),-2,-2,1030,780,NULL,BLEND_NONE,BLEND_NONE);
		RenderWrap::SetRS(D3DRS_COLORWRITEENABLE,0xFFFFFFFF);
	}
	if(g_IsGameApp)
		m_MainMenu.Render();
	else
		m_GUISystem->ShowDesktop(false);

	// Draw our custom cursor graphic at the mouse position
	// Disabled, we're using the windows cursor for now
	//if(GetCursorVisible())
	//{
	//POINT pos = GetMousePos();
	//canvas->SetScaling(1,1);
	//canvas->Box(0xFFFFFFFF,pos.x,pos.y,32,32,&m_CursorTex,BLEND_SRCALPHA,BLEND_INVSRCALPHA);
	//canvas->SetScaling(canvas->Width/1024.f,canvas->Height/768.f);
	//}
}

void Game::UpdateInput()
{
	if(g_IsGameApp)
	{
		// toggle GUI if in game mode and pressed button
		if(Input::Instance()->ControlJustPressed(CTRL_APP_TOGGLE_GUI))
		{
			if(m_Console.IsChatLineOn())
				HideChat();
			else
				ToggleMainMenu();
		}
	}

	// don't update player input when a GUI is open
	if(HasAnyGUIopen())
		return;

	if(Input::Instance()->ControlJustPressed(CTRL_APP_CHAT))
	{
		ShowChat();
		return;
	}

	// take a screenshot if pressed
	if(Input::Instance()->ControlJustPressed(CTRL_APP_TAKE_SCREENSHOT))
		Engine::Instance()->RenderSys->TakeScreenshot();
}

//--------------------------------------------------------------------------------------
// This function will be called once at the beginning of every frame. This is the
// best location to handle updates to the scene, but is not 
// intended to contain actual rendering calls, which should instead be placed in the 
// FrameRender callback.  
//--------------------------------------------------------------------------------------
float LastDedicatedPrintStatsTime = -BIG_NUMBER;
void Game::Tick()
{
	if(!m_Engine) // Tick can get called after shutdown or before init, so bail out if Reality is not initialized
		return;

	// if not in editor mode, update the core engine directly
	if (!Editor::Instance()->GetEditorMode())
		m_Engine->Update(CameraHandler::Instance()->GetCamera());
	else //otherwise leave it to the Editor to do that
		Editor::Instance()->Update(&m_World);

	// Update our delta times, accounting for a custom speed (so we can do cool slow-motion effects!)
	m_ActualDeltaTime = GDeltaTime;
	GDeltaTime *= GetTickSpeedScale();

    StartMiniTimer();
	// Update network systems first
	// process received network messages
	Server::Instance()->PreWorldTick();
	// process received network messages and send input out
	Client::Instance()->Tick();

    Profiler::Get()->GameMS += StopMiniTimer();

	// Now physics
	StartMiniTimer();
	// Only tick if not paused/zero-tick
	// This allows us to render a frozen world, matrix style
	if(GDeltaTime > 0)
	{
		int MaxSteps = 5;
		if(GSeconds - m_LastMapLoadedAtTime < 35 || Engine::Instance()->IsDedicated())
			MaxSteps = 0;
		PhysicsEngine::Tick(&m_World,GetTickSpeedScale(),MaxSteps);
	}
	Profiler::Get()->PhysicsMS += StopMiniTimer();

	// And the entire world..
	m_World.Tick();

	// send out all network messages generated during the tick
	Server::Instance()->PostWorldTick();
        
	// Update visual fx systems
    StartMiniTimer();
	FXManager::Instance()->Tick(&m_World);
	ShaderManager::Instance()->Tick();
    Profiler::Get()->FXMS += StopMiniTimer();

	// Handle input, etc if we're not a dedicated server
	if(!IsDedicated())
	{
        StartMiniTimer();
		// update inputs
		UpdateInput();
		// update camera animations
		CameraHandler::Instance()->Tick();
        Profiler::Get()->GameMS += StopMiniTimer();

		// Begin a scene render
		m_Engine->RenderSys->DoRendering();
		m_Engine->InputSys->FlushKeyboardBuffer(); 
	}
	else 
	{
		// If in dedicated server mode with stats or having performance problems
        // print the statistics every 2 seconds
		if((m_bShowStats || (Profiler::Get()->GetTotalMS()/Profiler::Get()->NumFrames > 20)) 
            && GSeconds - LastDedicatedPrintStatsTime > 2)
		{
			LastDedicatedPrintStatsTime = GSeconds;
			PrintStats(NULL);
		}
        else
            Profiler::Get()->Update();
        Sleep(15);
	}

    //StartMiniTimer();
    //Sleep(15);
    //LogPrintf("Sleep took %d MS. FPS=%f Time=%f",(int)StopMiniTimer(),1.0f/GDeltaTime,GSeconds);
}

//--------------------------------------------------------------------------------------
// Handles windows messages...
//--------------------------------------------------------------------------------------
LRESULT Game::HandleMessages( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	// Let the GUI have a look
	m_GUISystem->MsgProc(hWnd, uMsg, wParam, lParam);

	// This can get called when the engine is not yet initialized
	if(!m_Engine)
		return DefWindowProc(hWnd,uMsg,wParam,lParam);

	switch( uMsg)
	{
	case WM_CHAR:
		m_Console.MsgChar(wParam);
		break;

	case WM_KEYDOWN:
		m_Console.MsgKeyDown(wParam);
		break;
	}
	return m_Engine->HandleMessages(hWnd,uMsg,wParam,lParam);
}

//--------------------------------------------------------------------------------------
// Initializes the Game, call when you want the Game to actually start, ticking & rendering
//--------------------------------------------------------------------------------------
void Game::Initialize(HWND topHwnd, HWND childHwnd, HINSTANCE hInst,string cmdLine,bool isDedicated)
{
	StartMiniTimer(); // Let's see how long initialization takes

    m_World.ProgressCallback = WorldLoadProgress;
	// Create config file to load settings from
	ConfigFile cfg;
	if(access("..\\"PROJECT_NAME".ini",0) == 0) // Check root folder
		cfg.Load("..\\"PROJECT_NAME".ini");
	else
		cfg.Load(PROJECT_NAME".ini"); // Otherwise load from system folder

	// Initialize engine and all its subsystems
	m_Engine = Engine::Instance();
	m_Engine->Game_Tick = (LPGAMETICK)Game_Tick;
	m_Engine->Initialize(cfg,topHwnd,childHwnd,hInst,PROJECT_NAME,isDedicated,cmdLine);

	// Determines some global behavior that we may not want if running from the Editor
	g_Game.g_IsGameApp = !Editor::Instance()->GetEditorMode();

	// Initialize networking
	GameClientModule::Instance()->Initialize();
	GameServerModule::Instance()->Initialize();

	m_IsDedicated = isDedicated;
	// Starting with no map loaded
	m_IsInGame	  = false;

	// Register a callback for rendering
	RenderDevice::Instance()->SetRenderCallback(GFrameRender);

	// Initialise this game's Python script modules
	initgame();
	initvectorc();

	// Console functions
	m_Console.Initialize("confront.dds","conback.dds");
	m_LoadingBGTex.usesLOD = false;
	m_LoadingBGTex.Load("loadbg.dds");
	RegisterConsoleFunctions(m_Console);

	// Load some basic game preferences from the config
	m_bShowStats = cfg.GetBool("showstats");

	// Non-dedicated game stuff (e.g. key control mappings & GUIs)
	if(!isDedicated)
	{
		CTRL_APP_TAKE_SCREENSHOT = Engine::Instance()->InputSys->GetControlHandle("TakeScreenshot");
		CTRL_APP_TOGGLE_GUI = Engine::Instance()->InputSys->GetControlHandle("toggle_gui");
		CTRL_APP_CHAT = Engine::Instance()->InputSys->GetControlHandle("Chat");

		m_GUISystem->Initialize();
		m_MainMenu.Initialize();
		m_GUISystem->ShowDesktop(!Editor::Instance()->GetEditorMode());
		SetCursorVisible(true);
		Editor::Instance()->Intialize();
	}

	float loadTime = StopMiniTimer()/1000.0f;
	m_Console.Printf(PROJECT_NAME" Build %.2f started. Initialization took %f seconds",BUILD_VERSION,loadTime);
	float a = Profiler::Get()->TextureLoadSecs,
		b = Profiler::Get()->MeshLoadSecs,
		c = Profiler::Get()->ShaderLoadSecs,
        d = Profiler::Get()->MeshSetupSecs;
    LogPrintf("(Seconds) Textures: %f. Meshes (Load: %f Setup: %f). Shaders: %f. Unaccounted time: %f",a,b,d,c,loadTime-(a+b+c));
	Profiler::Get()->Update();

	// init null World to begin with
	m_World.NewWorld();

	// If we've been given a start map, then skip past the GUIs and load it
	string startMap;
	if(cmdLine.find("map:") != -1)
		startMap = cmdLine.substr(cmdLine.find("map:")+4); 

	// If we've been given a start map, then skip past the GUIs and load it
	if(m_Engine->MainConfig->GetString("StartMap") != "NONE" || startMap.length() > 0)
	{
		string mapName;
		if(startMap.length() > 0)mapName = startMap;
		else mapName = m_Engine->MainConfig->GetString("StartMap");

        //Server::Instance()->BeginHosting(Server::Instance()->m_ServerSettings.m_SessionName,mapName,false);
		Server::Instance()->BeginHosting(Server::Instance()->m_ServerSettings.m_SessionName,mapName,(m_Engine->MainConfig->GetBool("StartMapInMP") || IsDedicated()) && !Editor::Instance()->GetEditorMode());
	}
	else if(!g_Game.g_IsGameApp) // Reality Builder will run singleplayer mode
		Server::Instance()->BeginHosting("Reality Builder","",false);
}

//--------------------------------------------------------------------------------------
// Unloads the current map (if any) and loads a new one, placing you immediately into the action as dictated by scripts. 
// Call anytime!
//----------------------------------------------------------------------------------------------------
string LoadingMapFileName;
void Game::NewMap(string mapfile,bool IsServer)
{
	if(!mapfile.size())
	{
		if(IsServer)Server::Instance()->LoadedNewMap();
		return;
	}

	//draw loading screen BG
	if(!IsDedicated() && !Editor::Instance()->GetEditorMode())
	{
		m_GUISystem->m_Desktop->SetVisible(false);
		SetCursorVisible(false);
		LoadingMapFileName = mapfile;
		Game::Instance()->m_bIsLoading = true;
		//RenderDevice::Instance()->DoRendering();
	}

	//disconnect from the multiplayer game if the client wants to load its own map
	if(!m_World.m_IsServer && IsServer)
		Client::Instance()->Disconnect();

	//set out far so that we don't get any 3d sounds blaring upon loading
	if(!Editor::Instance()->GetEditorMode())
	{
		CameraHandler::Instance()->GetCamera()->Location.Set(10000,10000,10000);
		m_Engine->Update(CameraHandler::Instance()->GetCamera());
	}

	m_World.UnLoad();
	m_World.m_IsServer = IsServer;

	m_ActualDeltaTime = 0;
	m_LastMapLoadedAtTime = -BIG_NUMBER;
	m_TickSpeedScale = 1;
	m_IsInGame = true;

	m_World.Load(mapfile);

	m_Console.Hide();
	m_Console.Clear();

	// This will cause the server to notify clients of the map load
	if(IsServer)
		Server::Instance()->LoadedNewMap();

	m_LastMapLoadedAtTime = GetGSeconds();
}

//--------------------------------------------------------------------------------------
// Returns whether any of the menu Graphical User Interfaces are visible
//--------------------------------------------------------------------------------------
bool Game::HasAnyGUIopen()
{
	return (m_GUISystem->m_Desktop->GetVisible() || m_Console.IsOn() || Editor::Instance()->GetEditorMode());
}

//--------------------------------------------------------------------------------------
// Updates cursor visibility based on GUI menu visibility state. 
//--------------------------------------------------------------------------------------
void Game::UpdateCursorVisible()
{
	//hidden if no open GUIs, meaning you're actually controlling gameplay and the cursor should not be visible.
	SetCursorVisible(HasAnyGUIopen());
}

//--------------------------------------------------------------------------------------
// Toggles visibility of main menu
//--------------------------------------------------------------------------------------
void Game::ToggleMainMenu()
{
	//only able turn main menu off if you're actually playing a game
	if(!IsInGame())
		m_GUISystem->m_Desktop->SetVisible(true);
	else 
		m_GUISystem->m_Desktop->SetVisible(!m_GUISystem->m_Desktop->GetVisible());

	UpdateCursorVisible();
}

//----------------------------------------------------------------------------------------------------------------
// Returns whether the app has loaded a level to play (in which case some additional logic is run in Game::Tick), 
// or is in Editor in which case null World updating is desired
//----------------------------------------------------------------------------------------------------------------
bool Game::IsInGame()
{
	return m_IsInGame || !g_Game.g_IsGameApp;
}

//----------------------------------------------------------------------------------------------------------------
// Prints common performance statistics onto the canvas
//----------------------------------------------------------------------------------------------------------------
void Game::PrintStats(Canvas *canvas)
{
    Profiler* pro = Profiler::Get();
	if(!Engine::Instance()->IsDedicated())
	{
	    canvas->PrintStat("Drawn",m_Engine->RenderSys->MeshesPerFrame(),500,false); 
	    canvas->PrintStat("Tris",m_Engine->RenderSys->TrisPerFrame(),550000,false); 
	    canvas->PrintStat("Verts",m_Engine->RenderSys->VertsPerFrame(),550000,false); 
	    canvas->PrintStat("Dynamic",m_Engine->RenderSys->DynamicLightsDrawn(),5,false);
        canvas->PrintStat("RenderMS",pro->RenderMS/pro->NumFrames,25,false);
        canvas->PrintStat("PreRenderMS",pro->RenderPreMS/pro->NumFrames,15,false);
        canvas->PrintStat("PostRenderMS",pro->RenderPostMS/pro->NumFrames,15,false);
	    canvas->PrintStat("FXMS",pro->FXMS/pro->NumFrames,5,false); 
        canvas->PrintStat("C# MS",pro->ScriptMS/pro->NumFrames,5,false); 
	    canvas->PrintStat("PresMS",pro->PresentMS/pro->NumFrames,25,false);
	    canvas->PrintStat("TickMS",pro->TickMS/pro->NumFrames,5,false);
	    canvas->PrintStat("GameMS",pro->GameMS/pro->NumFrames,2,false);
        canvas->PrintStat("PhysicsMS",pro->PhysicsMS/pro->NumFrames,5,false);
	    canvas->PrintStat("SubMS",pro->SubsystemMS/pro->NumFrames,2,false);
        canvas->PrintStat("WaterMS",(pro->WaterCPUMS+pro->WaterRenderMS)/pro->NumFrames,2,false);
        canvas->PrintStat("BatchMS",pro->BatchMS/pro->NumFrames,5,false);
        canvas->PrintStat("AudioMS",pro->AudioMS/pro->NumFrames,5,false);
        canvas->PrintStat("Actors",pro->Actors/pro->NumFrames,3,false);
		//canvas->PrintStat("LightMS",pro->LightMS/pro->NumFrames,3,false);

        // Calculate the FPS derived from adding all systems together
        // This'll make it clear if there are any flaws in our timing
        float TotalMS = pro->GetTotalMS()/pro->NumFrames;
        canvas->PrintStat("Expected FPS",1000.0f/TotalMS,-20,false);

        canvas->PrintStat("C# Actors",pro->ScriptActors/pro->NumFrames,500,false);
        canvas->PrintStat("FPS",(int)1000.0f/(pro->DeltaMS/pro->NumFrames),-20,false);
        canvas->Textf(SmallFont,0xFF00FF00,20,245,"Loc: %.2f,%.2f,%.2f",CameraHandler::Instance()->GetCamera()->Location.x,CameraHandler::Instance()->GetCamera()->Location.y,CameraHandler::Instance()->GetCamera()->Location.z);
        canvas->Textf(SmallFont,0xFF00FF00,20,265,"Dir: %.2f,%.2f,%.2f",CameraHandler::Instance()->GetCamera()->Direction.x,CameraHandler::Instance()->GetCamera()->Direction.y,CameraHandler::Instance()->GetCamera()->Direction.z);
    }
    else
    {
        LogPrintf("ScriptMS %f",pro->ScriptMS/pro->NumFrames); 
        LogPrintf("PresMS %f",pro->PresentMS/pro->NumFrames);
        LogPrintf("TickMS %f",pro->TickMS/pro->NumFrames);
        LogPrintf("GameMS %f",pro->GameMS/pro->NumFrames);
        LogPrintf("PhysicsMS %f",pro->PhysicsMS/pro->NumFrames);
        LogPrintf("LightMS %f",pro->LightMS/pro->NumFrames);
        LogPrintf("Actors %d",(int)(pro->Actors/pro->NumFrames));
        LogPrintf("Lights %d",(int)(pro->Lights/pro->NumFrames));
        LogPrintf("FPS %f. Expected FPS: %f",1.0f/GDeltaTime,pro->GetTotalMS()/pro->NumFrames);
    }
    pro->Update();
}

Game* Game::Instance()
{
	return &g_Game;
}
void Game::ShowChat()
{
	m_Console.Show(true);
}
void Game::HideChat()
{
	m_Console.Hide();
	m_Console.ClearEditingLine();
}


void Game::DrawLoadingScreen(float percent)
{
    if(percent > 100)
        percent = 100;

	m_Engine->RenderSys->EndHDR();
	RenderWrap::SetRS(D3DRS_FOGENABLE,FALSE);

	//set the viewport to front of Z buffer for canvas draws
	int minZ = RenderDevice::Instance()->MinViewportZ;
	int maxZ = RenderDevice::Instance()->MaxViewportZ;
	D3DVIEWPORT9 viewport;
	viewport.X = 0;
	viewport.Y = 0;
	viewport.MinZ = 0;
	viewport.MaxZ = .05;
	viewport.Height = Canvas::Instance()->Height;
	viewport.Width = Canvas::Instance()->Width;
	RenderWrap::dev->SetViewport(&viewport);

	RenderWrap::SetSS( 0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
	RenderWrap::SetSS( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	RenderWrap::SetSS( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);

	Canvas::Instance()->Box(COLOR_RGBA(255,255,255,255),-2,-2,1028,772,&m_LoadingBGTex,BLEND_NONE,BLEND_NONE);
	Canvas::Instance()->TextCenteredf(LargeFont,COLOR_RGBA(255,255,255,160),512,700,512,700,"Loading %s",LoadingMapFileName.c_str());

    Canvas::Instance()->TextCenteredf(SmallFont,COLOR_RGBA(255,255,255,160),512,768-10,512,768-10,"%d%%",(int)percent);
    Canvas::Instance()->Box(COLOR_RGBA(17,14,17,255),0,768-20,percent*10.24f,25,NULL,BLEND_ONE,BLEND_ONE);

	RenderWrap::SetSS( 0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

	viewport.MinZ = minZ;
	viewport.MaxZ = maxZ;
	RenderWrap::dev->SetViewport(&viewport);
}









