//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
/// stdafx.h : include file for standard system include files,
/// or project specific include files that are used frequently, but
/// are changed infrequently
//====================================================================================

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

#ifdef _DEBUG
#define CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif
/// TODO: reference additional headers your program requires here
#pragma warning(disable:4305)
#define _WIN32_DCOM
#include "Shared.h"
#include "Engine.h"
#include "classmap.h"
#include "Precache.h"
#include "NetworkActor.h"
#include "Server.h"
#include "Client.h"
#include "Frame.h"
#include "CameraHandler.h"
#include "Networking\GameClientModule.h"
#include "Networking\GameServerModule.h"