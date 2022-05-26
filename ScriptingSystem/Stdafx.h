// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifdef ENGINE_EXPORTS
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif

#ifdef GAME_EXPORTS
#define GAME_API __declspec(dllexport)
#else
#define GAME_API __declspec(dllimport)
#endif

#include "..\shared\shared.h"
#include "..\EngineInc\Engine.h"
#include "..\EngineInc\GuiSystem.h"
#include "..\EngineInc\Actor.h"
#include "..\EngineInc\World.h"
#include "..\EngineInc\Model.h"
#include "..\EngineInc\Editor.h"
#include "..\EngineInc\NetworkClient.h"
#include "..\EngineInc\Client.h"
#include "..\EngineInc\Server.h"
#include "..\EngineInc\classmap.h"
#include "..\EngineInc\Precache.h"
#include "..\EngineInc\PostProcess.h"
#include "..\EngineInc\NetworkActor.h"
#include "..\GameDLL\GameEngine.h"
#include "..\GameDLL\CameraHandler.h"
#include "..\GameDLL\Networking\GameClientModule.h"
#include "Helpers.h"

#pragma comment(lib,"..\\physics\\lib\\tokamakdll.lib")

#define WIN32_LEAN_AND_MEAN				///< necessary for any advanced windows programming

using namespace System::Runtime::InteropServices;
#include "common.h"
#include "rigidbodycontrollercallback.h"
#include "jointcontrollercallback.h"


