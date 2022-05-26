#include "stdafx.h"
#include "IndoorVolume.h"
#include "Editor.h"
#include "SkyController.h"

vector<IndoorVolume*> IndoorVolume::IndoorVolumes;

IndoorVolume::IndoorVolume(World* world) : Actor(world)
{
	GhostObject = true;
	StaticObject = true;
	hasTicked = false;
	MyModel = NULL;
	IndoorVolumeType = 0;
	EditorVars.push_back(EditorVar("IndoorVolumeType",&IndoorVolumeType,"Volume Parameters","Determines the functionality of this IndoorVolume. \n0: activates Indoor lighting. \n2: activates Indoor lighting and collides outdoor weather effects. \n3: only collides outdoor weather effects."));
	IndoorVolumes.push_back(this);
}

void IndoorVolume::Tick()
{
	if(MyModel)
	{
		if(!hasTicked || IsSelected)
		{
		volumeBox = MyModel->GetWorldBBox();
		if(!Editor::Instance()->GetEditorMode())
		{
			IsHidden = true;
			delete MyModel;
			MyModel = NULL;
		}
		GhostObject = true;
		hasTicked = true;
		}
	}

	Actor::Tick();
}

void IndoorVolume::DeserializationComplete()
{
	if(MyModel)
	{
		volumeBox = MyModel->GetWorldBBox();
		if(!Editor::Instance()->GetEditorMode())
		{
			IsHidden = true;
			delete MyModel;
			MyModel = NULL;
		}
		GhostObject = true;
		hasTicked = true;
	}
}

IndoorVolume::~IndoorVolume()
{
	IndoorVolumes.erase(find(IndoorVolumes.begin(),IndoorVolumes.end(),this));
}

bool IndoorVolume::IsInsideThisVolume(Vector &location)
{
	BBox* bbox = &volumeBox;

	if(location.x < bbox->max.x && location.x > bbox->min.x && location.y < bbox->max.y && location.y > bbox->min.y && location.z < bbox->max.z && location.z > bbox->min.z)
		return true;

	return false;
}

bool IndoorVolume::IsIndoors(World* world, Vector &location)
{
	if(!SkyController::Instance)
		return true;

	for(int i = 0; i < IndoorVolumes.size(); i++)
	{
		if(IndoorVolumes[i]->MyWorld != world || IndoorVolumes[i]->IndoorVolumeType != INDOORVOLUME_LIGHTING)
			continue;

		BBox* bbox = &IndoorVolumes[i]->volumeBox;
		if(location.x < bbox->max.x && location.x > bbox->min.x && location.y < bbox->max.y && location.y > bbox->min.y && location.z < bbox->max.z && location.z > bbox->min.z)
			return true;
	}

	return false;
}

bool IndoorVolume::IsInVolumeType(World* world, Vector &location, int VolumeType)
{
	for(int i = 0; i < IndoorVolumes.size(); i++)
	{
		if(IndoorVolumes[i]->MyWorld != world || IndoorVolumes[i]->IndoorVolumeType != VolumeType)
			continue;

		BBox* bbox = &IndoorVolumes[i]->volumeBox;
		if(location.x < bbox->max.x && location.x > bbox->min.x && location.y < bbox->max.y && location.y > bbox->min.y && location.z < bbox->max.z && location.z > bbox->min.z)
			return true;
	}

	return false;
}