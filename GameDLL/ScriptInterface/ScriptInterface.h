//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
/// ScriptInterface: Game functions & data that Python scripts have access to
/// There is a custom build step associated with this file
/// The build step calls Update.bat which generates scriptinterface_wrap.cxx
/// Update: Build step disabled because it was causing project to remain out
/// of date in vs.net
/// You MUST manually run update.bat after this file is modified and saved.
//
/// The _wrap files that are generated must have the following:
/// Precompiled headers: DISABLED
/// Preprocessor defenitions: SWIG_COBJECT_TYPES
//====================================================================================
#ifdef SWIG
%module game
%{
	/* Put header files here (optional) */

#include "..\..\shared\Shared.h"
#include "Engine.h"
#include "ScriptInterface.h"

	%}

#endif

// fader index definitions for the Sky Controller
#define SKY_MOON_INTENSITY 4
#define SKY_MOON_SIZE 5
#define SKY_SUN_INTENSITY 6
#define SKY_SUN_SIZE 7
#define CLOUD_SCROLL_SPEED 8
#define CLOUD_BRIGHTNESS_FACTOR 9
#define CLOUD_SOLIDITY 10
#define CLOUD_MAX_INTENSITY 11
#define CLOUD_MIN_INTENSITY 12
#define CLOUD_ADD_RED_VALUE 13
#define CLOUD_ADD_GREEN_VALUE 14
#define CLOUD_ADD_BLUE_VALUE 15
#define CLOUD_ALPHA_FACTOR 16
#define DAYLIGHT_RED 17
#define DAYLIGHT_GREEN 18
#define DAYLIGHT_BLUE 19
#define NIGHTLIGHT_RED 20
#define NIGHTLIGHT_GREEN 21
#define NIGHTLIGHT_BLUE 22
#define RAIN_INTENSITY 23
#define SKY_ADD_RED_VALUE 24
#define SKY_ADD_GREEN_VALUE 25
#define SKY_ADD_BLUE_VALUE 26
#define SKY_OVERBRIGHT 27
#define SKY_BGMIX1 28 //night
#define SKY_BGMIX2 29 //morning
#define SKY_BGMIX3 30 //noon
#define SKY_BGMIX4 31 //dusk
#define SKY_BGMIX5 32 
#define SKY_BGMIX6 33
#define SKY_BGMIX7 34
#define SKY_BGMIX8 35
#define WEATHERSTATE_CLEAR 0
#define WEATHERSTATE_THUNDERSTORM 1

// Spawns a Game Actor by class name at the specified transformation, and returns a pointer to the created Actor
Actor* SpawnActor(char* ActorType, World* world, Matrix* transform);

// Gets the current daytime, in minutes of the day (60 min * 24 hr = 1440 min per day)
float GetDayTimeMinutes();
// Gets the Sky daytime speed, in Sky Minutes per Game Second
float GetDayTimeSpeed();
// Fades a Sky Mixer (used to control intra-sky blending and coloration) to a specified float value (sky minutes, 24 hour day, actual game scaling determined by sky daytime speed)
void FadeSky(int SkyIndex, float FadeDestination, float MinutesDuration);
// Gets the float value of the specified Sky Mixer
float GetSkyValue(int SkyIndex);
// Fades the fog to a specified density and color over a period of time (sky minutes, 24 hour day, actual game scaling determined by sky daytime speed)
void FadeFog(float newDensity,float newRed,float newGreen, float newBlue, float MinutesDuration);
// Stores the sky state so the sky controller script can proceed through different sections of the day, with different state #'s
void SetSkyState(int stateValue);
// Gets the sky state so the sky controller script can proceed through different sections of the day, with different state #'s
int GetSkyState();
// Stores weather state for control of weather fx systems
void SetWeatherState(int stateValue);
// Gets weather state for control of weather fx systems
int GetWeatherState();
// Sets the time of day if using a sky controller, used particularly by sky scripts to set initial day time
void SetSkyDayTime(float DayTimeMinutes);

// printing methods to aid in debugging
void DoMessageBox(char* Text);
void Print(char* Text);

// Adds a rain FX system, inside the box volume of a mesh in the scene
//void AddRainBox(World* world,Actor* actor, int numParticles);

// gets GDeltaTime for tick framerate scaling in script
float getDeltaTime();
// helper math function
int getRandomWholeNumber(int maxRange);
// adds interior volume to the scene, which is an area where the sky can't be scene and where skylights have no effect
void addInteriorVolumeBox(Actor* actor);
// plays as an ogg music file
void PlayOgg(char* filename);