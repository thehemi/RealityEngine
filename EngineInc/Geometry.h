//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
/// 
///
/// Author: Tim Johnson
//====================================================================================
#include "Actor.h"

/// Core geometry classs. Depreciated
class Geometry : public Actor{
public:
	Geometry(World* WorldToBeIn) : Actor(WorldToBeIn){
		IsPrefab = true;
		StaticObject = true;
		CollisionFlags = CF_MESH;
	}

	/// We need tick so actors interop with world tree and all
	virtual void Tick(){Actor::Tick();}

};