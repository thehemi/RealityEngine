//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
// This file contains console functions you can register
// Console functions can be typed in in-game to trigger functions with parameters
// See GUIConsole.h for more information
//====================================================================================
#include "stdafx.h"
#include "GameEngine.h"
#include "SkyController.h"
#include "Editor.h"
#include "classmap.h"

//---------------------------------------------------------------------------------------
// changes map
//----------------------------------- ---------------------------------------------------
void changemap(char* value)
{
	Server::Instance()->BeginHosting(Server::Instance()->m_ServerSettings.m_SessionName,value,true);
}

//--------------------------------------------------------------------------------------
// Spawns an actor in front of the player, demo player, or editor camera view
// CASE sensitive
//--------------------------------------------------------------------------------------
void spawnactor(const char* actorname)
{
	if(!Server::Instance()->m_ServerStarted)
		return;

	Vector Loc = CameraHandler::Instance()->GetCamera()->Location;
	Vector rotDir = CameraHandler::Instance()->GetCamera()->Direction;

	try
	{
		//attempt to spawn the Actor, could fail if an invalid classname is specified so catch the error in that case.
		Actor* a = Factory::create(actorname,&g_Game.m_World);
		Vector rotVec;
		a->Location = Loc +  rotDir* 5.5;
		rotVec = -rotDir;
		rotVec.y = 0;
		a->Rotation = Matrix::LookTowards(rotVec);
		a->bExportable = true;
	}
	catch(...)
	{
		g_Game.m_Console.Printf("Error trying to spawn Actor: %s. Check case and spelling, or review log.",actorname);
	}
}

//--------------------------------------------------------------------------------------
// sets the SkyController system, if any, to a specified 24-hr time of day
//--------------------------------------------------------------------------------------
void setdaytime(char* time)
{
	if(!Server::Instance()->m_ServerStarted)
		return;

	if(SkyController::Instance)
	{
		//parse and calculate the total minutes from the string
		string TimeString = time;

		string HoursPart = TimeString.substr(0,TimeString.find(":"));
		string MinutesPart = TimeString.substr(TimeString.find(":")+1);

		float DayTime = atoi(HoursPart.c_str())*60.f + atoi(MinutesPart.c_str());

		SkyController::Instance->SetDayTime(DayTime);
	}
}

//--------------------------------------------------------------------------------------
// Sets the sky minutes per game second time scaling of the SkyController system
//--------------------------------------------------------------------------------------
void setdaytimespeed(char* speed)
{
	if(!Server::Instance()->m_ServerStarted)
		return;

	if(SkyController::Instance)
		SkyController::Instance->SetMinutesPerGameSecond(atof(speed));
}

//--------------------------------------------------------------------------------------
// Sets the HDR exposure value
//--------------------------------------------------------------------------------------
void sethdr(char* hdrexposure)
{
	RenderDevice::Instance()->SetHDRExposure(atof(hdrexposure));
}

//-----------------------------------------------------------------------------------------------------------------
// Prints all Actor classes registered with the class map, including C# Actors located in the Scripts\Actors subdir
//-----------------------------------------------------------------------------------------------------------------
void PrintActorClasses()
{
	// get list of spawnable Actor classes from the REGISTER_FACTORY classmap and print them
	vector<string> ActorClasses = Factory::GetClasses();
	g_Game.m_Console.Printf("--- Begin Actors List ---");
	for(int i = 0; i < ActorClasses.size();i++)
		g_Game.m_Console.Printf(ActorClasses[i].c_str());
	g_Game.m_Console.Printf("--- End of Actors List ---");
}
void PrintNetworkActors()
{
	NetworkActor::PrintAllNetworkActors();
}

//--------------------------------------------------------------------------------------
// Registers functions and variables with the console system
//--------------------------------------------------------------------------------------
void RegisterConsoleFunctions(GUIConsole& console)
{
	console.DeclareSymbol(ST_FUNCTION,"changemap",changemap);	
	console.DeclareSymbol(ST_FUNCTION,"spawn",spawnactor);	
	console.DeclareSymbol(ST_FUNCTION,"setdaytime",setdaytime);
	console.DeclareSymbol(ST_FUNCTION,"setdaytimespeed",setdaytimespeed);
	console.DeclareSymbol(ST_FUNCTION,"sethdr",sethdr);
	console.DeclareSymbol(ST_FUNCTION,"actorclasses",PrintActorClasses);
	console.DeclareSymbol(ST_FUNCTION,"printnetworkactors",PrintNetworkActors);
	console.DeclareSymbol(ST_BOOL,"showstats",&g_Game.m_bShowStats);
}


