//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ==================
/// Name: LODManager
/// \brief Creates Mesh LODs using progressive meshes, imposters, alpha fading, etc
//===========================================================================================
#pragma once

class ENGINE_API LODManager 
{
public:
	/// Visible distance
	float	VisibleRange;
	/// LOD distance
	float   LODRange;
	/// Instance function
	static	LODManager* Instance () ;
	/// Fills lods with desired lod levels. Assumes lods[0] contains source mesh
	bool	GenerateLODs(vector<Mesh*>& lods);
	/// Initialize lod manager settings
	void Initialize();

	LODManager()
	{
		VisibleRange = 1;
		LODRange	 = 1;
	}
};

