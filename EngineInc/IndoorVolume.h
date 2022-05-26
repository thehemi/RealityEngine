//=========== (C) Copyright 2003, Artificial Studios. All rights reserved. ==================
/// Name: IndoorVolume
/// \brief Box volumes that determine interior areas of the level for lighting state on dynamic Actors
//===========================================================================================

#pragma once

enum
{
INDOORVOLUME_LIGHTING = 0,
INDOORVOLUME_PARTICLES = 1,
};

class ENGINE_API IndoorVolume : public Actor
{
	CLASS_NAME(IndoorVolume);
	static bool IsIndoors(World* world, Vector& location);
	static bool IsInVolumeType(World* world, Vector& location, int VolumeType);
	static vector<IndoorVolume*> IndoorVolumes;

	IndoorVolume(World* world);
	virtual void Tick();
	BBox volumeBox;
	bool hasTicked;
	int IndoorVolumeType;
	bool IsInsideThisVolume(Vector &location);
	virtual ~IndoorVolume();
	virtual void DeserializationComplete();
};
