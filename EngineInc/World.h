//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
/// Game world. Encapsulates a full game world.
///
/// Contains the world's actor list and geometry information
///
/// Author: Tim Johnson
//====================================================================================
#ifndef WORLD_INC
#define WORLD_INC
#include "Mesh.h"
#include "SSystemStub.h"

class Actor;
class Material;
class Light;
class Script;
class OcclusionCulling;

//-----------------------------------------------------------------
/// \brief Info returned for collision against world. Not all fields are
/// used. Always check for NULL
//-----------------------------------------------------------------
struct ENGINE_API CollisionInfo {
	/// Collision check started in solid
	bool stuck;		
	/// Actual distance from nearest plane, even if we never touched it
	double actualDistance; 
	/// Plane normal
	Vector normal;	
	/// Intersection point (rarely used)
	Vector point;	
	/// Touched actor, if any
	Actor* touched;	
	/// Another touched actor that didn't cause a collision
	Actor* otherTouched; 
	/// Material touched
	Material* mat; 

	CollisionInfo(){
		mat		= NULL;
		touched = NULL;
		/// Othertouched is a non-blocking object, like a trigger
		/// The reason for it being 'other' is that if we were inside it, it would void collisions
		/// for all other solid objects because it would be the nearest.
		/// So it's just passively recorded.
		otherTouched = NULL;
		stuck = false;
	}
};

//-----------------------------------------------------------------
/// A face used in collision data
//-----------------------------------------------------------------
struct CollisionFace{
	Vector normal;
	Vector vert[3];
	/// Edge normals
	Vector edgeNorm[3]; 
	/// Face's material (usually just the actor's mesh material)
	Material* mat;		
	/// If not static geometry
	Actor* owner;		
};

//-----------------------------------------------------------------
/// Used by the render tree
//-----------------------------------------------------------------
struct RenderableNode {
	int					meshID; /// Source mesh
	BBox				box;
	vector<Light*>		staticLights;
	vector<Light*>		dynamicLights;
	AttributeRange		subset;
};

//-----------------------------------------------------------------
/// RenderTrees are used for the static scene
//-----------------------------------------------------------------
struct RenderTree {
	RenderTree(){ 
		for(int i=0;i<4;i++)
			children[i] = 0;
	}

	~RenderTree(){
		for(int i=0;i<numChildren;i++){
			SAFE_DELETE(children[i]);
		}
	}

	vector<RenderableNode*> data;
	BBox		box;
	int			numChildren;
	RenderTree*	children[4];

	void GetNodesInSphere(Vector& center, float radius, vector<RenderableNode*>& nodes, bool bCheck = true);
	void GetNodesInView(Camera* camera, vector<RenderableNode*>& nodes, bool bCheck = true);
	void FlushDynamicLights();
	void RemoveStaticLight(Light* light);
};

/// For ray checks
enum CheckType{
	CHECK_ACTORS,
	CHECK_GEOMETRY,
	CHECK_EVERYTHING,
	CHECK_VISIBLE_GEOMETRY,
	CHECK_VISIBLE_ACTORS,
	CHECK_VISIBLE_ACTORS_UNFROZEN,
	CHECK_VISIBLE_EVERYTHING,
	CHECK_UNHIDDEN_EVERYTHING
};

//-----------------------------------------------------------------------------
/// \brief World - Everything happens here.
/// An instance of a game can have multiple worlds, though this is usually
/// rare except on servers
//-----------------------------------------------------------------------------
class ENGINE_API World : public RenderBase {
protected:
	friend class BatchRenderer;
    friend class SortTree;
    friend class XMLLoad;
    friend class XMLSave;

    /// Occlusion Culling
    OcclusionCulling *    m_OcclusionCulling;
    /// (Serialized). Force occlusion off on this map?
    bool    m_DisableOcclusionCulling;
	/// Scene properties
	FloatColor	m_FogColor;
	/// Scene properties
	float		m_FogDistance;
	/// Scene properties
	COLOR		m_Ambient;
	/// Scene properties
	bool		m_HasSky;
	/// Scene properties
	int			m_ClipPlane;
	bool		m_IsSky;
	World*		m_SkyWorld;
	
    /// Params BatchRenderer can use (hacky, but allows interactive profiling)
    bool        m_bDrawMeshes;
    bool        m_bDoFlips;
    bool        m_bSetSHStates;
    bool        m_bPostRender;

	/// Only one or two of these for a scene
	vector<Mesh*>	m_BatchMeshes;   
	/// Scene octree root
	RenderTree*		m_Root;			 
	/// Single collision mesh for entire world
	CollisionMesh*	m_CollisionMesh;
	/// Static/Dynamic instanced prefabs. These are inserted into the tree
	vector<Model*>	m_Prefabs;		 
	
	/// Hard-code max SH lights (evil)
	#define MAX_LIGHTS 20
	/// Global ambient coefficients - calculated once per frame
	float m_GlobalfSum[3][D3DXSH_MAXORDER*D3DXSH_MAXORDER];
	/// Updates global coefficients
	void UpdateGlobalPRT();
	/// Calculates PRT coefficients that can be combined with mesh coeffs
	void CalculatePRTSum(float fSum[3][D3DXSH_MAXORDER*D3DXSH_MAXORDER], DWORD dwOrder, vector<Light*>& Lights, Matrix& tm);
	/// Global order of SH order*order
	DWORD m_GlobaldwOrder;
    /// Cone radius for SH dir lights. Serialized
    float m_fConeRadius;

    virtual HRESULT OnResetDevice(){ return S_OK; };
	virtual void	OnLostDevice();
	virtual void	OnDestroyDevice(){};
public:

    /// Overall scene LOD/culling bias. Default 1. Serialized
    float m_fLODCullBias;

    typedef void (*RegisterWorld)(World* world);
    /// C# Managed callback for world registration
    static RegisterWorld   m_RegisterWorld;

	typedef void (*Calllback_AddImpulse)(int managedIndex, void* impulse);
	/// C# Managed callback for touched impulse onto rigid body physics Actors
	static Calllback_AddImpulse m_AddTouchedImpulse;

    /// Render Occlusion tree boxes
    bool m_RenderOcclusionBoxes;

    /// Use occlusion culling system
    bool m_UseOcclusionCulling;

	/// Config .ini for this map
	ConfigFile	m_Config;
	/// Editable parameters displayed inside Reality Builder
	vector<EditorVar>	m_EditorVars;

	/// Editor-only Properties, held in scene file
	PRTSettings	m_DefaultSH;

	/// Exposed to game after loading from file
	string	m_FileName;

    //Mostafa:Scripting
    int m_managedIndex;

	~World();
	World();

	// Methods only to be used by actors
	/// Add actor to world. Returns actor for function nesting
	Actor*	AddActor(Actor* actor);
	/// Remove actor from world (doesn't destroy it). false if not found
	bool	RemoveActor(Actor* actor); 
	
	/// Loads a sky (background) World that is rendered behind this World
	void	LoadSkyWorld(string worldFile); 
	FloatColor	GetFogColor(){ return m_FogColor; }
	float		GetFogDistance(){ return m_FogDistance; }
	void	SetFog(FloatColor Color, float Density);
	/// Sets far clip plane distance
	void	SetClipPlane(int clip){ m_ClipPlane = clip; }
	float	GetClipPlane(){ return m_ClipPlane; }
	/// Wher this World has a Sky World
	bool	HasSky(){ return m_HasSky; }
	const CHAR* GetFileName(){ return m_FileName.c_str(); }
	RenderTree* GetTree(){ return m_Root; }
	/// Returns frame (if frame contains mesh)
	ModelFrame*	FindMeshFrame(string name, ModelFrame* ignore=0);		
	void	EnumerateMeshes(vector<ModelFrame*>& meshFrames);
	
	/// You almost never want to manually edit these lists.
	/// Actors add themselves, 
	/// Lights add themselves
	/// Scripts and materials are added at world load
	/// Actors include Lights

	/// Actors currently in the World, Actors are self-registering/unregistering on this list
	vector<Actor*>		m_Actors;	
	/// Lights currently in the World, Actors are self-registering/unregistering on this list
	vector<Light*>		m_Lights;

	/// Scripts aren't actors
	vector<Script*>		m_Scripts;	

	/// Spherical Harmonics Probes (Lighting Environment Maps)
	struct SHProbe {
		/// SH data
		float	fData[3][D3DXSH_MAXORDER*D3DXSH_MAXORDER];	
		/// filename of probe
		string	strTexture;		
		/// 0 to 1
		float	fBlendFactor;	
	};
	/// Spherical Harmonics -- Env Map light probes
	vector<SHProbe*> m_SHProbes;
	/// Spherical Harmonics -- Env Map light probes
	int				 LoadSHProbe(string filename); /// Returns index of probe
	/// Spherical Harmonics -- Env Map light probes
	void			 FreeSHProbe(int index){ delete m_SHProbes[index]; m_SHProbes.erase(m_SHProbes.begin()+index); }
	/// Spherical Harmonics
	void UpdateDynamicPRT(Mesh* mesh,Matrix& Transform, vector<Light*>& Lights);
	/// For static Objects
	void UpdateStaticPRT(Mesh* mesh);
	/// Renders the World onto the default RT from the specified Camera's perspective
	void Render(class Camera* cam);

	/// Must call after Render for actors to do their overlay drawings etc
	void PostRender(Camera* cam);	
	/// Unload world from memory. Remaining actors will be deleted
	void UnLoad();		
	/// Load world from file
	void Load(string fileName); 
    /// Initialize a fresh world
    void NewWorld();
	/// Update world state. Includes actors and scripts
	void Tick();		
	/// Checks if an Actors BBox can fit centered at a point in the World
	bool CollisionCheckActor(Actor* actor, const Vector& testLocation, const Vector& testVelocity);
	/// SPEED TIP: If you only care about actors, select checkActors
	/// It'll still make sure they are behind world items, but it can bail out fast if you aren't looking at one
	bool CollisionCheckRay(Actor* sourceToIngore, const Vector& start, const Vector& end, CheckType check, CollisionInfo& result, bool forceModelAccuracy = false,vector<Actor*>* ignoreActors = NULL);

	void GatherPossibleColliders( Actor* source, vector<CollisionFace>& touchList, const Vector& inMove, const BBox& worldBox);

    /// Gathers rendering polygons within a volume. Can be used for fillrate heavy tasks, such as drawing drop-shadows, by
    /// only gathering the precise triangles needed for rendering
    void GatherRenderingPolys(Actor* sourceToIgnore, vector<Vertex>& vertices, const Vector& inMove, const BBox& worldBox);

	/// Updates physics for a given Actor according over the delta time
	void RunPhysics(Actor* actor);

	/// Updates collision physics for a non-Rigid Body
	bool CollideAndSlide(Actor& actor, float gravity,vector<Actor*>& touchList, CollisionInfo& returnInfo);

	/// Adds an impulse to an Actor at its center Location
	void AddTouchedImpulse(Vector& impulse, Actor* touched);
	/// Adds an impulse to an Actor at a World contact point
	void AddTouchedImpulse(Vector& contactPoint, Vector& impulse ,Actor* touched);

    void RegenerateOcclusionTree();

	/// Finds an Light by name
	Light* FindLight(string name);
	
	/// Finds an actor by name
	Actor* FindActor(string& name);

	/// Finds an actor by GUID
	Actor* FindActor(GUID guid);

    /// Find strongest light at a point
    Light* FindStrongestLight(Vector& pos, bool Indoors = true, bool Outdoors = true);
    /// Get average lighting at a point
    FloatColor GetAverageLighting(Vector& pos,  bool Indoors = true, bool Outdoors = true);

	/// Creates an Actor defined within a C# script (handled through the wrapped C# Actor management), for C#-driven gameplay
	Actor* CreateScriptActor(string ClassName);

	/// Server sets this to true if hosted a game (either MP or SP), Client sets this to false if joined a game. This value can be used to determine authoritative behavior on the part of NetworkActors
	bool m_IsServer;

	/// Reloads the dynamic sky
	virtual void ReloadSkyController();
    /// Progress callback for loading/saving
    typedef void (*PROGRESS_CALLBACK)(float percent, string status);
    PROGRESS_CALLBACK ProgressCallback;

	private:
		void InitializeScripts();
};

#endif




