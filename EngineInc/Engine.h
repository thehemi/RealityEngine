//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
/// Name: Core Engine
///
/// Author: Tim Johnson
//====================================================================================
/*! \mainpage RealityEngine Documentation v0.51
 *
 * \section intro_sec Introduction
 * RealityEngine Documentation Index. This documentation is for the <b>core engine only</b>!<br><br>
 * The following internal modules are excluded: <br><i>GameDLL, RealityBuilder, MaxTools, MayaTools, ScriptingEngine, Cry Havoc, Cry Havoc Dedicated, Cry Havoc Dedicated (Linux)</i><br><br>
 * The following external modules are excluded: <br><i>Tokamak, Opcode, OggVorbis, Python, Xerces, PortAudio, RakNet, D3DX</i>
 * <br><br>
 * Refer to their documentation seperately.
 *
 *
 * \section samp_sec Tutorials & Examples
 * The best source of examples are the game and technology code provided as part of the licensing package.
 * For more support, refer back to http://reality.artificialstudios.com/
 *
 * <br><br><br> <center><i>The documentation listed here is still experimental and may have some omissions or errors.</i></center>
 */
#pragma once

/*-----------------------------------------------------------------------------
Forward declarations
-----------------------------------------------------------------------------*/
class Mesh;
class World;
class Actor;
class Light;
class Material;
class Shader;
class Texture;
class Model;
struct VertexBuffer;
struct IndexBuffer;
struct CollisionMesh;
struct ShaderVar;

enum LogLevel{
	LOG_OFF = 0,/// Nothing
	LOG_LOW,	/// Serious problems
	LOG_MEDIUM, /// Resource fails
	LOG_HIGH	/// Resource loads
};

#ifdef ENGINE_EXPORTS
#    define DECLSPECIFIER __declspec(dllexport)
#    define EXPIMP_TEMPLATE
#else
#    define DECLSPECIFIER __declspec(dllimport)
#    define EXPIMP_TEMPLATE extern
#endif

// Disable warnings on extern before template instantiation
#pragma warning (disable : 4231)

// Instantiate template classes
// This does not create an object. It only forces the generation of all
// of the members of classes. It exports
// them from the DLL and imports them into the .exe file.
EXPIMP_TEMPLATE template class DECLSPECIFIER std::vector<string>;
EXPIMP_TEMPLATE template class DECLSPECIFIER std::vector<class Actor*>;
EXPIMP_TEMPLATE template class DECLSPECIFIER std::vector<struct ModelFrame*>;
EXPIMP_TEMPLATE template class DECLSPECIFIER std::vector<class Model*>;
EXPIMP_TEMPLATE template class DECLSPECIFIER std::vector<class EditorVar*>;
EXPIMP_TEMPLATE template class DECLSPECIFIER std::vector<class Material*>;

//EXPIMP_TEMPLATE template class DECLSPECIFIER std::string<char>;

/*-----------------------------------------------------------------------------
Engine public includes.
-----------------------------------------------------------------------------*/
#include "Base.h"
#include "Audio.h"				/// Audio system.
#include "Input.h"				/// Input system.
#include "RenderDevice.h"		/// Rendering system
#include "Canvas.h"				/// High-Level Drawing
#include "RenderWrap.h"			/// Rendering call wrapper
#include "SharedStructures.h"	/// Shared between exporters and engine
#include "Model.h"				/// Model objects.
#include "World.h"				/// Level object.
#include "Camera.h"				/// Viewport camera.
#include "Script.h"				/// Python scripts
#include "Actor.h"				/// Actor inlines.
#include "Geometry.h"			/// Geom object
#include "Shader.h"				/// Shader systems
#include "Texture.h"			/// Texture class
#include "Light.h"				/// Base Light Actor
#include "Material.h"			/// Surfaces properties, textures.
#include "ScriptEngine.h"
#include "ResourceManager.h"	 /// Resource tracking/management
#include "ConfigDatabase.h"      /// Device configuration manager
#include "ConfigManager.h"      /// Device configuration manager

typedef void    (CALLBACK *LPGAMETICK)();

//-----------------------------------------------------------------------------
/// Main Engine, Manages all core subsystems
//-----------------------------------------------------------------------------
class ENGINE_API Engine {
private:
	Engine();
	Engine(const Engine&);
	Engine& operator= (const Engine&);
	~Engine();
	/// No subsystems
	bool dedicatedMode; 
	/// Level of logging to file. 0 = errors/warnings 1 = map/errors/warnings 2 = also resource fails 3 = also resource loads
	LogLevel logLevel;  

public:
    /// Engine running in safe mode?
    bool                SafeMode;

	LogLevel LogLevel(){ return logLevel; }
	bool IsDedicated(){ return dedicatedMode; }

	static Engine* Instance();

    /// Device configuration manager
    CConfigManager*     ConfigManager;

	// You shouldn't access these directly. Each class has a singleton Instance() function
	// which is the preferred method of access
	AudioDevice*		AudioSys;
	Input*				InputSys;
	RenderDevice*		RenderSys;
	ConfigFile*			MainConfig;
	HWND				hWnd, TopHwnd;
	LPGAMETICK			Game_Tick;

	LRESULT HandleMessages( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	void	Initialize(ConfigFile& cfg,  HWND topHwnd, HWND childHwnd,  HINSTANCE AppHInst, const char* AppName, bool dedicated, string cmdLine);
	void	Update(class Camera* ActiveCamera);
	void	Shutdown();
};


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/



