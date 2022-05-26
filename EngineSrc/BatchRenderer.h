//=========== Copyright (c) 2003, Tim Johnson. All rights reserved. ===========
/// Core rendering classes, descriptions below
/// 
//=============================================================================


//-----------------------------------------------------------------------------
/// Contains info for Reality to render a material subset of a mesh
//-----------------------------------------------------------------------------
struct BatchItem 
{
	/// Array of lights. Depending on shader model, may only be one light per batch
	vector<Light*>		lights;
	/// Ref counting if batch on multiple trees
	int					refCount;
	/// Source batch, if multiple light batches
	BatchItem*			pFirstItem;
	/// name of the mesh
	string				name;
	/// Material that this mesh subset uses; externally set
	Material*			matRef;
	/// Subset of the mesh to render; externally set
	AttributeRange		subset;
	/// Bone transformations, if any; externally set
	vector<Matrix*>		bone_matrices;
	/// Mesh owning the subset to render; externally set
	Mesh*				mesh;
	/// Transformation of the mesh within the World scene
	Matrix				transform; 
	/// Whether it uses the transformation, immobile meshes could have World transformation built into mesh for optimization
	bool				usesTransform;
	/// Whether this mesh is Static (Prefabs) or Dynamic (moving Actors). Determines whether it can use static outdoor PRT data or not.
	bool StaticObject;
	/// World bounding box of the mesh for culling
	BBox				box;
	/// World that the mesh is contained in
	World*				world;
	/// Whether the mesh uses per-pixel occlusion testing
	bool				usesOcclusion;
	/// Pointer to occlusion state that may be modified during rendering
	DWORD*				occlusionState; 
	/// D3D object to store per-pixel occlusion test results
	LPDIRECT3DQUERY9*	occlusionQuery;
	/// Source model
	Model*				model;

	/// Internal - BatchRenderer sets these
	/// Base passes
	int			passes;
	/// Alpha tree passes
	int			alphaPasses;
	/// Set to true when rendered, so we don't get dual rendering while sky and normal worlds share same queue
	bool		rendered;
	/// Special skip flag for water, etc
	bool		skipMe;
	/// Distance of batchitem's mesh from camera (least bbox vertex), used for z-sorting
	float		Z_Value;
    /// Technique
    Technique*  tech;

	BatchItem(){
		Z_Value			= 0;
		alphaPasses		= 0;
		refCount		= 0;
		skipMe			= false;
		matRef			= NULL;
		occlusionQuery	= NULL;
		world			= NULL;
		mesh			= NULL;
		usesOcclusion	= false;
		StaticObject	= true;
		passes			= 0;
        tech            = 0;
	}
};

//-----------------------------------------------------------------------------
/// Collection of BatchItems grouped by shader technique for efficient rendering
//-----------------------------------------------------------------------------
struct ShaderBatch {
	/// non-SH technique used by these batchitems
	D3DXHANDLE	technique;

	/// SH-techique used by these batchitems, if any (only if they are SH meshes)
	D3DXHANDLE	SHtechnique;

	/// Shader used by these BatchItems
	Shader* shader;

	/// The collection of BatchItems to render within this ShaderBatch
	vector<BatchItem*> items;
	/// Used to speed up HDR pass calculations, just once calculates highest number of passes in the batch
	int maxPasses; 

	ShaderBatch(){SHtechnique = NULL;}
	ShaderBatch(Shader* aShader, D3DXHANDLE aTechnique){ maxPasses = 1; shader = aShader; technique = aTechnique; SHtechnique = NULL;}
	~ShaderBatch(){
		SAFE_DELETE_VECTOR(items);
	}
};

/// \brief This class is at the heart of the scene graph. Pools are pumped through here (by World and Model classes), then sorted by shader, material, lighting.
//-----------------------------------------------------------------------------
/// All passes are performed, all textured and shaders applied, and everything
/// rendered.
/// Render system to efficiently group the rendering of meshes in the scene, eliminating redundant usage of shaders and render passes
//-----------------------------------------------------------------------------
class BatchRenderer 
{
private:
	friend class Model;

	BatchRenderer();

	bool CheckOcclusionTest(BatchItem* p);

	void RenderFinalShaders();
	bool m_bZPass; /// Do a z pass first?

	/// Pushes a BatchItem onto a specific array of Batches, checks for grouping optimizations
	void AddToBatch(vector<ShaderBatch*>& batches, BatchItem* item);

    /// Adds an item to a specific tree
    void QueueBatchItem(BatchItem* item, class SortTree* tree, SortTree* alphaTree);

public:
	//null/black material & technique for unlit batches
	Material*				m_SystemMaterial;

	/// Singleton
	static  BatchRenderer* Instance();

	/// Called during Engine Initialization
	void Initialize();

	
	typedef bool (*RENDER_CALLBACK)(BatchItem* item);
	/// Per-item rendering callback for custom things such as water reflections
	RENDER_CALLBACK	m_Callback;

	/// Main rendering function, Worlds call this
	void RenderQueuedBatches(World* world);

	/// Call this first though!
	void PrepareQueuedBatches(Camera* cam, World* world);

	/// Do LDR stuff like occlusion tests, because it's cheaper on LDR surface, and renders alpha-blended Batches on pre-ps2.0 devices
	void RenderLDR();

	/// Called if Device is reset so outdated device-dependent data is not used for rendering
	void InvalidateDeviceObjects();

	/// Add a BatchItem (mesh) to be rendered; automatically calculates most efficient grouping with other Batches
	void QueueBatchItem(BatchItem* item);

    /// Draw immediately
    void DrawBatches(vector<BatchItem*>& items);

	/// Clears all Batches queued to be rendered
	void ClearAll();
};
