//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
// ScriptInterface: Functions that Python scripts have access to
//====================================================================================
#include "stdafx.h"
#include <windows.h>
#include <python.h>
#include "Object.h"
#include "ScriptInterface.h"
#include "GameEngine.h"
#include "SkyController.h"
#include "FXmanager.h"
#include "classmap.h"
#include "StreamingOgg.h"


//--------------------------------------------------------------------------------------
// printing methods to aid in debugging
//--------------------------------------------------------------------------------------
void DoMessageBox(char* Text)
{
	MessageBox(0,Text,"",0);
}
void Print(char* Text)
{
	g_Game.m_Console.Printf(Text);
}

//--------------------------------------------------------------------------------------
// Spawns a Game Actor by class name at the specified transformation, and returns a pointer to the created Actor
//--------------------------------------------------------------------------------------
Actor* SpawnActor(char* ActorType, World* world, Matrix* transform)
{
	if(world->m_IsServer)
	{
		try{
			Actor* actor = Factory::create(ActorType,world);
			if(actor->MyModel)*transform *= actor->MyModel->m_pFrameRoot->TransformationMatrix.Inverse();
			actor->Location = transform->m3;
			actor->Rotation = transform->GetRotationMatrix();
			return actor;
		}
		catch(...)
		{
			g_Game.m_Console.Printf("SpawnActor: '%s' failed. Probably wrong case or spelling.",ActorType);
			return 0;
		}
	}
	else return 0;
}



//--------------------------------------------------------------------------------------
// Gets the current daytime, in minutes of the day (60 min * 24 hr = 1440 min per day)
//--------------------------------------------------------------------------------------
float GetDayTimeMinutes()
{
	if(SkyController::Instance)
		return SkyController::Instance->GetDayTimeMinutes();

	return 0;
}

//--------------------------------------------------------------------------------------
// Fades a Sky Mixer (used to control intra-sky blending and coloration) to a specified float value (sky minutes, 24 hour day, actual game scaling determined by sky daytime speed)
//--------------------------------------------------------------------------------------
void FadeSky(int SkyIndex, float FadeDestination, float MinutesDuration)
{
	if(SkyController::Instance)
		SkyController::Instance->FadeSky(SkyIndex,FadeDestination,MinutesDuration);
}

//--------------------------------------------------------------------------------------
// Fades the fog to a specified density and color over a period of time (sky minutes, 24 hour day, actual game scaling determined by sky daytime speed)
//--------------------------------------------------------------------------------------
void FadeFog(float newDensity,float newRed,float newGreen, float newBlue, float MinutesDuration)
{
	if(SkyController::Instance)
		SkyController::Instance->FadeFog(newDensity,newRed/255.f,newGreen/255.f,newBlue/255.f,MinutesDuration);
}

//--------------------------------------------------------------------------------------
// Gets the sky state so the sky controller script can proceed through different sections of the day, with different state #'s
//--------------------------------------------------------------------------------------
int GetSkyState()
{
	if(SkyController::Instance)
		return SkyController::Instance->m_SkyState;

	return -1;
}

//--------------------------------------------------------------------------------------
// Stores the sky state so the sky controller script can proceed through different sections of the day, with different state #'s
//--------------------------------------------------------------------------------------
void SetSkyState(int stateValue)
{
	if(SkyController::Instance)
		SkyController::Instance->m_SkyState = stateValue;
}

//--------------------------------------------------------------------------------------
// Sets the time of day if using a sky controller, used particularly by sky scripts to set initial day time
//--------------------------------------------------------------------------------------
void SetSkyDayTime(float DayTimeMinutes)
{
	if(SkyController::Instance)
		SkyController::Instance->SetDayTime(DayTimeMinutes);
}

//--------------------------------------------------------------------------------------
// Gets the float value of the specified Sky Mixer
//--------------------------------------------------------------------------------------
float GetSkyValue(int SkyIndex)
{
	if(SkyController::Instance)
		return SkyController::Instance->SkyBGMix[SkyIndex];

	return 0;
}

//--------------------------------------------------------------------------------------
// Stores weather state for control of weather fx systems
//--------------------------------------------------------------------------------------
void SetWeatherState(int stateValue)
{
	if(SkyController::Instance)
		SkyController::Instance->m_WeatherState = stateValue;
}

//--------------------------------------------------------------------------------------
// Gets weather state for control of weather fx systems
//--------------------------------------------------------------------------------------
int GetWeatherState()
{
	if(SkyController::Instance)
		return SkyController::Instance->m_WeatherState;

	return -1;
}

//--------------------------------------------------------------------------------------
// gets GDeltaTime for tick framerate scaling in script
//--------------------------------------------------------------------------------------
float getDeltaTime()
{
	return GDeltaTime;
}

//--------------------------------------------------------------------------------------
// helper math function
//--------------------------------------------------------------------------------------
int getRandomWholeNumber(int maxRange)
{
	return rand()%maxRange;
}

//--------------------------------------------------------------------------------------
// Gets the Sky daytime speed, in Sky Minutes per Game Second
//--------------------------------------------------------------------------------------
float GetDayTimeSpeed()
{
	if(SkyController::Instance)
		return SkyController::Instance->GetMinutesPerGameSecond();

	return 0;
}


//-----------------------------------------------------------------------------------------
// adds interior volume box to control display of sky lighting for meshes that are indoors
//-----------------------------------------------------------------------------------------
void addInteriorVolumeBox(Actor* actor)
{
	// get the bbox size from the script container Actor's model and add it as an indoors-defining volume
	SkyController::Instance->AddSkyVolumeBox(actor->MyModel->GetWorldBBox());

	// dispose of the container Actor after we've used its model
	delete actor;
}

void PlayOgg(char* filename)
{
	if(Engine::Instance()->IsDedicated())
		return;

	if(oggPlayer.IsPlaying())
			oggPlayer.Stop();

	string music = filename;

	if(FindMedia(music,"Music"))
	{
		oggPlayer.OpenOgg(music.c_str());
		oggPlayer.Play(true);
		oggPlayer.SetVolume(Engine::Instance()->MainConfig->GetFloat("MusicVolume"));
	}
}