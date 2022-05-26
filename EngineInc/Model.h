//=========== Copyright (c) 2003, Tim Johnson. All rights reserved. ===========
/// Model: Various Model classes
//
//=============================================================================
#pragma once

#include "Frame.h"

typedef int		ANIMATIONHANDLE;
typedef void*	NODEHANDLE;
struct			ModelFrame;

typedef int DrawFlags;
#define DRAW_NONE (1<<0)
#define DRAW_IMMEDIATE (1<<1)
#define DRAW_UPDATELIGHTING (1<<2)


//-----------------------------------------------------------------------------
/// \brief Instance of a static mesh hierarchy
//
/// Does not contain animation functionality, is thus slightly faster
/// than a normal animated Model
//-----------------------------------------------------------------------------
class ENGINE_API StaticModel {
protected:
	friend class SceneLoader;
	/// Scene loader can use this to make sure we're prepped after data load
	virtual void	InitAfterLoad(); 
	/// Internal optimization flag. Dirtied as soon as mesh moves, or if mesh is not from world
	bool		    m_IsStaticWorldMesh;		
	/// Instances of this model in existence, for reference keeping. 
	/// Kept as pointer so any model that is instanced will share this array and all updates to this array
	vector<StaticModel*>* m_pInstances;	

	void Draw(Camera* cam, ModelFrame* pFrame, World* world, bool StaticObject, DrawFlags flags, Actor* Owner = NULL);
    void DrawSimple(Camera* cam, ModelFrame* pFrame);
    void DrawHierarchy(ModelFrame* pFrame, ModelFrame* parent);

	/// BBox
	BBox m_BBox;
	/// Returns best LOD for current camera, based on distance tests, etc
	Mesh* ChooseLOD(Camera* cam, ModelFrame* pFrame);
	/// Special scaling, to be used when animations and other stuff are loaded with model
	float m_LoadScale;
public:
    /// Scale entire model, including frames, skinning data, mesh, etc
    void Scale(Vector scale);

	/// Special scaling, to be used when animations and other stuff are loaded with model
	void  SetLoadingScale(float scale){ m_LoadScale = scale;}
	/// Get special loading scale
	float GetLoadingScale(){ return m_LoadScale; }

	/// Return the most intense Light that's currently touching a Model, useful for optimizations when you want to do some calculation with the strongest light on an Actor or Model
	Light* FindStrongestLight();

	/// Custom flags. Other classes can use this as an efficient way to 
	/// store meta-data with this model
	DWORD	CustomFlags;

	StaticModel();
	virtual ~StaticModel();

	/// LOD Ratio
	float	m_LODRatio;
	/// Visible distance ratio
	float	m_VisibleDistanceRatio;

	/// Used by renderer, ignore
	DWORD				m_OcclusionState; 
	/// Used by renderer, ignore
	LPDIRECT3DQUERY9	m_OcclusionQuery; 
	/// Base TM
	Matrix				m_RootTransform;
	/// Lights touching this model at any given time
	vector<Light*>	m_TouchingLights;
	/// Number of these that are static and should not be cleared, starting from element 0
	DWORD			m_StaticLights;  

	/// Frame hierarchy. Includes root transform, bones and other model parts, root nodes, Bip01, etc
	ModelFrame*		m_pFrameRoot;

	/// If you don't want the model to have its scene offset included, use this after loading
	void RemoveSceneOffset();
	/// Exportable flag, used by editor
	bool bExportable;

	string	m_FileName;

	//---------------------
	/// Management
	//---------------------
	void Load(const char* name, bool DeepCopy = false);
	bool IsLoaded(){ return m_pFrameRoot != NULL; }
	virtual void Destroy();
	virtual void Update();
	/// Clears the Model's touching light list
	void ClearLights();

	/// We don't completely copy Models, or we'd waste video memory
	/// We just reference the vertex data, and give it new object data
	/// This means reference Models must not modify any pointers that are shared
	virtual void CreateNewInstance(StaticModel* newModel); /// Replaces MakeShallowCopy

	//---------------------
	/// Node manipulation
	//---------------------
	NODEHANDLE	GetNodeHandle(string name, ModelFrame* ignore=0);
	ModelFrame* GetFrame(NODEHANDLE node){ return (ModelFrame*)node; }
	Matrix		GetNodeMatrix(NODEHANDLE node);
	/// Used to influence a bone with a custom matrix, such as a Y-rotation matrix applied to the hip bone
	void		SetNodeInfluence(NODEHANDLE node, Matrix& inflMat, float weighting = 1);

	//---------------------
	/// Transformations
	//---------------------
	virtual void SetTransform(Matrix& transform);
	virtual void SetTransform(Matrix& rotOffset, Vector& locOffset); /// Replaces OffsetBaseTransform
	BBox GetWorldBBox(); /// World bounding box

	//---------------------
	/// Rendering
	//---------------------
	/// This can be slow, get it once and save it! NOTE: Case sensitive!
	Material* FindMaterial(string name); 
	/// Batches model up for drawing
	virtual void Draw(Camera* camera, World* world, bool StaticObject, DrawFlags flags = DRAW_NONE, Actor* Owner = NULL);
    void DrawSimple(Camera* camera); 
    /// Draw box hierarchy showing all nodes. Used for skeletal visualization
    void DrawHierarchy(); 
};

//-----------------------------------------------------------------------------
/// \brief Holds a unique animation controller and all frames
//
/// Does not contain skeletal data or similar mesh-dependent attributes
//-----------------------------------------------------------------------------
class ENGINE_API Model : public StaticModel {
protected:
	friend class XFileLoad;
	LPD3DXANIMATIONCONTROLLER   m_AnimationController; 
#ifndef DOXYGEN_IGNORE
	struct TrackInfo{
		bool  bLooping;
		float duration;
		float endTime;
        bool  bEverStarted;
        TrackInfo(){ bEverStarted = false; }
	};
#endif
	vector<TrackInfo>	tracks;
	int					curAnimTrack;
	bool				paused;

public:
	/// Get duration of animation, in seconds
	float GetDuration(ANIMATIONHANDLE anim);
	/// Get remaining time, in seconds
	float GetRemaining(ANIMATIONHANDLE anim);
	/// Get looping state
	bool  IsLooping(ANIMATIONHANDLE anim);
	/// Weight for animation, if blending multiple animations
	void SetAnimationWeight(ANIMATIONHANDLE animation, float weight, float transitionTime=1.1f);

	/// Init anim controller
	void InitializeAnimationSystem(LPD3DXANIMATIONCONTROLLER cloneController = NULL);
	virtual void InitAfterLoad();

	Model();
	virtual ~Model(){}

	/// Pause animation
	void Pause(){ paused = true; }
	/// UnPause animation
	void UnPause(){ paused = false; }
	
	int GetNumAnimations(){ return tracks.size(); }
    /// Load an .anm file, can also specify bones that are influenced by animation
	ANIMATIONHANDLE LoadAnimation(string filename, bool bLooping = true, vector<string>* AffectedBoneNames=NULL);
	/// returns -1 if no animation is playing
	ANIMATIONHANDLE GetCurrentAnimation();
	void  UnloadAllAnimations();
	/// Smoothly changes to an animation. Returns the play duration in seconds
	float TransitionToAnimation(ANIMATIONHANDLE animation, float transitionTime = 1.1f, float weight = 1, float startAtTime = 0, float playSpeed = 1);
	/// Returns true if an animation is playing. Looped animations always return true
	bool  IsPlaying();
	/// Returns true if the specified animation track is playing. Looped animation tracks always return true
	bool  IsPlaying(ANIMATIONHANDLE animation);

	/// Overridden functions
	virtual void CreateNewInstance(StaticModel* newModel, bool bCloneMeshData = false);
	virtual void Destroy();
	virtual void Update();	
	virtual void SetTransform(Matrix& rootTransform);
	virtual void SetTransform(Matrix& rotOffset, Vector& locOffset);
};



//=============================================================================
/// Higher-level implementation
//=============================================================================
/*class IRenderable
{
private:
CalculateLOD();
vector<Light*> lights;

public:
AddDecal();
GetLOD();
SetTouchingLights();	
Render();
};*/




