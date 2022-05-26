//=========== (C) Copyright 2004, Tim Johnson. All rights reserved. ===========
// Type: Serializer
// Info: Handles loading/saving, will check all registered serializers
// 'Serializers' are loading/saving modules
//=============================================================================
#include "stdafx.h"
#include "Collision.h"
#include "Frame.h"
#include "ispatialpartition.h"
#include "ShadowMapping.h"
#include "SharedStructures.h"
#include "ScriptSystem.h"
#include "Serializer.h"
#include "Serializers\XMLSerializer.h"
#include "Serializers\XFile.h"
#include "Serializers\OBJ.h"

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
ILoad*	Serializer::GetLoader(string filename)
{
    filename = AsLower(filename);
	ILoad* load = NULL;
	if(filename.find(".xml") != -1)
		load = new XMLLoad();
	else if(filename.find(".x") != -1)
		load = new XFileLoad();
	else if(filename.find(".obj") != -1)
		load = new OBJLoad();

	return load;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
ISave*	Serializer::GetSaver(string filename)
{
	ISave* save = NULL;
	if(filename.find(".xml") != -1)
		save = new XMLSave();
	else if(filename.find(".obj") != -1)
		save = new OBJSave();

	return save;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
bool Serializer::LoadWorld(string name, World* world, bool IsSky)
{
	ResetCurrentDirectory();

	ILoad* load	= GetLoader(name);
	if(!load){
		SeriousWarning("No compatible Load serializer for '%s' world.",name.c_str());
		return false;
	}

	if(FindMedia(name,"Maps")){
		if(!load->LoadWorld(name,world,IsSky)){
			SeriousWarning("Found file %s, but couldn't open it.",name.c_str());
			return false;
		}
	}
	else{
		Warning("Couldn't find file '%s'", name.c_str());
		return false;
	}
	
	delete load;
	return true;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
bool Serializer::LoadModel(string name, StaticModel* Model)
{
	ResetCurrentDirectory();

	ILoad* load	= GetLoader(name);
	if(!load){
		SeriousWarning("No compatible load serializer for '%s' model.",name.c_str());
		return false;
	}

	if(FindMedia(name,"Models")){
		if(!load->LoadModel(name,Model)){

			if(!Engine::Instance()->IsDedicated())
				SeriousWarning("Found file %s, but couldn't open it.",name.c_str());

			return false;
		}
	}
	else{
		Warning("Couldn't find file '%s'", name.c_str());
		return false;
	}

	delete load;
	return true;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
bool Serializer::SaveWorld(string name, World* world, bool IsSky)
{
	ResetCurrentDirectory();

	LogPrintf("Saving World...");
	ISave* save	= GetSaver(name);
	if(!save){
		SeriousWarning("No compatible Save serializer for '%s' world.",name.c_str());
		return false;
	}
	save->SaveWorld(name,world);
	LogPrintf("..Saved");
	return true;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
bool Serializer::SaveModel(string name, StaticModel* Model)
{
	ResetCurrentDirectory();

	LogPrintf("Saving Model...");
	ISave* save	= GetSaver(name);
	if(!save){
		SeriousWarning("No compatible Save serializer for '%s' model.",name.c_str());
		return false;
	}
	save->SaveModel(name,Model);
	LogPrintf("..Saved");
	return true;
}


//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
bool Serializer::Save(string name, vector<Actor*>& items)
{
	ResetCurrentDirectory();

	LogPrintf("Saving Selected...");
	ISave* save	= GetSaver(name);
	if(!save){
		SeriousWarning("No compatible Save serializer for '%s' items.",name.c_str());
		return false;
	}
	save->Save(name,items);
	LogPrintf("..Saved");
	return true;
}
