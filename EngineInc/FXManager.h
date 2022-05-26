//======== (C) Copyright 2004, Jeremy Stieglitz. All rights reserved. =========
/// FXmanager: Static system that manages, updates, groups, and efficiently renders FXsystems
//=============================================================================

#ifndef FX_SYSTEMS
#define FX_SYSTEMS

typedef vector<class FXSystem*> FXSYSTEMVECTOR;

#define MAX_BATCHES 30
#define QUADS_PER_BATCH 1500

/// Simple batched quad structure
struct BatchedQuad
{
Texture* texture;
BlendMode destBlend;
DWORD myVertexIndicesBatch;
DWORD numQuads;
};

#ifndef DOXYGEN_IGNORE
struct VertexIndicesBatch
{
	LVertex theVertices[QUADS_PER_BATCH*4 + 4];
	WORD theIndices[QUADS_PER_BATCH*6 + 6];
	bool used;
};
#endif

/// Manages all PostRender visual fx classes
class ENGINE_API FXManager
{
	friend class FXSystem;
private:
	/// the FX systems
	FXSYSTEMVECTOR	systems;
	vector<FXSystem*> SystemHash;
	FXManager(const FXManager&);
	FXManager& operator= (const FXManager&);

	bool				DoesExist(FXSystem *sys) const	{ return (find(systems.begin(), systems.end(), sys) != systems.end()); }
	FXSystem*			GetSystem(int nr) const						{ assert(nr<systems.size()); return systems[nr]; }
	FXSystem*			AddSystem(FXSystem *newSystem)	{ assert(newSystem); systems.push_back(newSystem); return systems[systems.size()-1]; }
	void				RemoveSystem(FXSystem* sys);

	void AddFXSystemToHash(FXSystem* system);

public:
	static VertexIndicesBatch vertexIndicesBatches[MAX_BATCHES];
	static vector<BatchedQuad> batchedQuads;
	static int addNewBatch(Texture* texture,BlendMode destBlend);
	static int getFreeBatch();

	/// ctor
	FXManager();
	/// dtor
	~FXManager(){}

	/// Singleton
	static FXManager* Instance ();

	/// Called after the World is rendered, where most FXSystems will do their rendering
	void				PostRender(World* world,Camera& cam);
	
	/// Updates the FX Systems
	void Tick(World* world);

	/// Frees all FXSystems in the specified (unloading) World, called between World loads
	void Reset(World* world = NULL);

	/// Draws all currently queued quads
	void DrawBatchedQuads();
	/// Removes all currently queued quads
	void FlushBatchedQuads(){batchedQuads.erase(batchedQuads.begin(),batchedQuads.end());}

	void ChangeSystemsWorld(World* oldWorld, World* newWorld);

	/// Gets a batch of quads using a particular Material to group with. If there's no batch currently with said Material, this functional will create a new Batch with said Material.
	static int getBatchForWriting(Texture* texture,BlendMode destBlend);

	/// Returns an FXSystem based on the specified hash index, if a
	FXSystem* GetFXSystemFromHash(DWORD index, FXSystem* system);
};


/// Base interface for all visual FX classes
class ENGINE_API FXSystem
{
public:
	/// Used to ignore an Actor for fx classes that may want to run collision tests
	Actor* ignoreActor;
	/// If true, FXmanager will sort rendering of all sorted FXSystems based on their distance from the Camera. Useful for non-commutative blending modes for FX like smoke, but CPU intensive when there are many FXSystems.
	bool isSorted;
	/// Calculated by FXmanager if a sorted system, used for ordering back-to-front
	float distanceFromCam;
	/// Forces rendering even beyond the far distance clip
	bool forceDistanceRender;
	/// Basic transformation
	Vector Location;
	/// Basic transformation
	Vector Velocity;
	/// Basic transformation
	Matrix Rotation;

	/// Decreases until 0 and then the FXSystem is destroyed. -1 (default) makes the FXSystem immortal until destruction of the World or manual deletion
	float LifeTime;
	/// If IsHidden, FXSystems are not rendered
	bool IsHidden;
	/// Owner World, used by static FXManager to determine which FXSystems to delete upon World load.
	class World* MyWorld;
	/// Index of (optional) fx system hash registration for fast external fxsystem lookup
	int HashIndex;
	/// ctor
	FXSystem(class World* world)
	{
		MyWorld = world;
		FXManager::Instance()->AddSystem(this); // FX Systems register with the manager so that they may recieve Pre/PostRender() calls
		isSorted = false;
		forceDistanceRender = false;
		LifeTime = -1;
		IsHidden = false;
		ignoreActor = 0;
		HashIndex = -1;
	}
	/// registers hash for this fxystem, for fast external lookup. FXSystem::HashIndex will be set upon culling this function.
	void RegisterWithHash()
	{
		FXManager::Instance()->AddFXSystemToHash(this);
	}
	/// dtor
	virtual ~FXSystem(){
		FXManager::Instance()->RemoveSystem(this);
	}
	// Called by the FX manager, overridden in subclasses
	virtual void PostRender(Camera& cam){}
	/// Ticks the FXSystem, FXSystems will generally override this to allow for custom Ticking behavior
	virtual void Tick()
	{
		Location += Velocity*GDeltaTime;
	}
	/// Renders during the HDR stage, after the World is rendered, for HDR effects (if supported by the Device)
	virtual void HDRrender(Camera& cam){}
};
#endif