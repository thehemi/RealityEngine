//======= Copyright (c) 2004, Artificial Studios. All rights reserved. ========
/// Resource tracking & management
//=============================================================================

//--------------------------------------------------------------------------------------
/// Holds all game resources. Used by CVS and media management
/// Resources are pushed on when attempted to load (even if fails, because dedicated server
/// may never really load such resources)
///
/// Resources are removed when destroyed
//--------------------------------------------------------------------------------------
class ENGINE_API ResourceManager
{
protected:
	ResourceManager()
	{
		m_WorldBeingLoaded = 0;
	}
public:

	static ResourceManager* ResourceManager::Instance ()
	{
		static ResourceManager inst;
		return &inst;
	}

	

	enum ResType
	{
		TextureRes,
		SoundRes,
		ScriptRes,
		MaterialRes,
		ModelRes,
		WorldRes,
		PRTRes
	};

	/// A resource tracked by the ResourceManager, these are also distributed via the NetworkEditor
	struct Resource
	{
		/// Object may be associated with world, if loaded within World.Load() call
		World*  world;
		void*   object;
		ResType type;
		Resource(ResType type, void* object, World* curWorld){ this->type = type; this->object = object; this->world = curWorld; }
	};
	vector<Resource> m_LoadedResources;
	World*			 m_WorldBeingLoaded; /// Set during World.Load() call

	/// Resources are added by their core classes. Do not manually add!
	void Add(Material* res){ if(FindResource(res)!=-1) return; m_LoadedResources.push_back(Resource(MaterialRes,res,m_WorldBeingLoaded)); }
	void Add(Texture* res){ if(FindResource(res)!=-1) return; m_LoadedResources.push_back(Resource(TextureRes,res,m_WorldBeingLoaded)); }
	void Add(Sound* res){ if(FindResource(res)!=-1) return; m_LoadedResources.push_back(Resource(SoundRes,res,m_WorldBeingLoaded)); }
	void Add(Script* res){ if(FindResource(res)!=-1) return; m_LoadedResources.push_back(Resource(ScriptRes,res,m_WorldBeingLoaded)); }
	void Add(Model* res){ if(FindResource(res)!=-1) return; m_LoadedResources.push_back(Resource(ModelRes,res,m_WorldBeingLoaded)); }
	void Add(World* res){ if(FindResource(res)!=-1) return; m_LoadedResources.push_back(Resource(WorldRes,res,m_WorldBeingLoaded)); }
	void Add(Mesh* res){ if(FindResource(res)!=-1) return; m_LoadedResources.push_back(Resource(PRTRes,res,m_WorldBeingLoaded)); }
	/// Resources are removed by their core classes. Do not manually remove!
	bool Remove(void* res){ int i = FindResource(res); if(i==-1)return false; m_LoadedResources.erase(m_LoadedResources.begin()+i); return true; }
	//
	int FindResource(void* ptr){ for(int i=0;i<m_LoadedResources.size();i++){if(m_LoadedResources[i].object == ptr){ return i; } } return -1; }
};