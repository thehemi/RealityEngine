//=========== Copyright (c) 2003, Tim Johnson. All rights reserved. ===========
/// Low-level classes and structures used for rendering
//=============================================================================
#pragma once

/// Forwards
class Material;
class Shader;
typedef struct ID3DXMesh *LPD3DXMESH;
typedef struct ID3DXSkinInfo *LPD3DXSKININFO;
typedef struct IDirect3DVertexDeclaration9 *LPDIRECT3DVERTEXDECLARATION9;

/// Change compiler pack alignment to be BYTE aligned, and pop the current value
#pragma pack( push, 1 )

#define FVF_LVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE |D3DFVF_TEX1)
#define FVF_TLVERTEX (D3DFVF_XYZRHW|D3DFVF_DIFFUSE |D3DFVF_TEX1)
#define FVF_VERTEX (D3DFVF_XYZ|D3DFVF_NORMAL | D3DFVF_TEX2 | D3DFVF_TEXCOORDSIZE3(1)|D3DFVF_TEXCOORDSIZE2(0))
#define FVF_PVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE | D3DFVF_TEX2 | D3DFVF_SPECULAR|D3DFVF_TEXCOORDSIZE2(0))

// Declaration structures
struct SHORT2 { short x,y; };
struct SHORT4 { short x,y,z,w; };
struct USHORT4 { unsigned short x,y,z,w; };
struct UBYTE4 { BYTE x,y,z,w; };

/// Vertex structure
struct Vertex 
{
	 Vector		position; 
	 Vector     normal;
	 Vector		tan;
	 Vector2	tex;
	 
	 Vertex(Vector p,Vector n,Vector2 texCoord){
		position=p;normal=n;tex = texCoord;
	 } 
	 Vertex(){}

	Vertex operator*( const float& fBlend )
	{
		Vertex v;
		v.position = position*fBlend;
		v.normal = normal*fBlend;
		v.tex.x = tex.x*fBlend;
		v.tex.y = tex.y*fBlend;
		v.tan = tan*fBlend;
		return v;
	}
};

/// Vertex with compressed data
struct PackedVertex 
{
	 Vector  pos; 
	 SHORT4  norm;
     SHORT4  tan; 
	 SHORT2  tex;

	 PackedVertex(){}
};

/// ColVertex structure
struct ColVertex 
{
	 Vector		position; 
	 Vector     normal;
	 Vector		tan;
	 Vector2	tex;
     DWORD      col;
	 
	 ColVertex(Vector p,Vector n,Vector2 texCoord){
		position=p;normal=n;tex = texCoord;
	 } 
	 ColVertex(){}

	ColVertex operator*( const float& fBlend )
	{
		ColVertex v;
		v.position = position*fBlend;
		v.normal = normal*fBlend;
		v.tex.x = tex.x*fBlend;
		v.tex.y = tex.y*fBlend;
		v.tan = tan*fBlend;
		return v;
	}
};


/// Vertex structure with 2 texcoords
struct VertexT2
{
	 Vector		position; 
	 Vector     normal;
	 Vector		tan;
	 Vector2	tex[2];
	 
	 VertexT2(Vector p,Vector n,Vector2 t1, Vector2 t2){
		position=p;normal=n;tex[0]=t1;tex[1]=t2;
	 } 
	 VertexT2(){}
};


#define MAX_NUM_PCA 24

struct SHVertex 
{
	 Vector			position; 
	 Vector			normal;
	 Vector			tan;
	 float			tu, tv;
	 int			iClusterOffset;
	 Vector4		vPCAWeights[MAX_NUM_PCA/4];

	 SHVertex(Vector p,Vector n,float u,float v){
		position=p;normal=n;tu=u;tv=v;
	 } 
	 SHVertex(){}
};


/// Vertex used on skinned meshes
struct SkinnedVertex 
{
	Vector position;
	float blend1,blend2,blend3;
	DWORD indices;
	Vector normal;
	float tu,tv;
	Vector tan;

	 SkinnedVertex(Vector p,Vector n,float u,float v){
		position=p;normal=n;tu=u;tv=v;
	 } 
	 SkinnedVertex(){}
};

/// Lit Vertex structure
struct LVertex 
{
	 Vector position;
	 unsigned long diffuse;
	 float       tu, tv;
	 LVertex(Vector p,unsigned long df,float u,float v){
		position=p;tu=u;tv=v;diffuse=df;
	 }
	 LVertex(Vector p, unsigned long df){
		 position = p; diffuse = df;
	 }
	 LVertex(){}
};

/// Transformed and Lit Vertex structure
struct TLVertex 
{
	 Vector4 position;
	 unsigned long diffuse;
	 float       tu, tv;
	 TLVertex(Vector4 p,unsigned long df,float u,float v){
		position=p;tu=u;tv=v;diffuse=df;
	}
	 TLVertex(){}
};

/// Simple vertex
struct SimpleVertex
{
    Vector pos;
};



//--------------------------------------------------------------------------------------
/// Holds settings used to generate PRT data for a mesh
//--------------------------------------------------------------------------------------
struct PRTSettings
{
	vector<EditorVar>	EditorVars;

	/// High-Level Settings
	vector<string>		InBlockers;
	vector<string>		OutBlockers;
	bool				Enabled;
    /// Has light mapping instead of PRT
    //bool                bLightMapping; 

	/// Simulator settings
	WCHAR     strInitialDir[MAX_PATH];
	WCHAR     strInputMesh[MAX_PATH];
	WCHAR     strResultsFile[MAX_PATH];
	int     dwNumRays;
	int     dwOrder;
	int     dwNumChannels;
	int     dwNumBounces;
	bool      bSubsurfaceScattering;
	float     fLengthScale;
	bool      bShowTooltips;

	/// Material options
	int     dwPredefinedMatIndex;
	FloatColor Diffuse;
	FloatColor Absoption;
	FloatColor ReducedScattering;
	float     fRelativeIndexOfRefraction;
	float		fSubSurfaceMultiplier;

	/// Adaptive options
	bool      bAdaptive;
	bool      bRobustMeshRefine;
	float     fRobustMeshRefineMinEdgeLength;
	int     dwRobustMeshRefineMaxSubdiv;
	bool      bAdaptiveDL;
	float     fAdaptiveDLMinEdgeLength;
	float     fAdaptiveDLThreshold;
	int     dwAdaptiveDLMaxSubdiv;
	bool      bAdaptiveBounce;
	float     fAdaptiveBounceMinEdgeLength;
	float     fAdaptiveBounceThreshold;
	int     dwAdaptiveBounceMaxSubdiv;
	WCHAR     strOutputMesh[MAX_PATH];
	bool      bBinaryOutputXFile;

	/// Compression options
	bool      bSaveCompressedResults;
	D3DXSHCOMPRESSQUALITYTYPE Quality;
	int     dwNumClusters;
	int     dwNumPCA;
	bool	  bPerPixel;	
	int     dwTextureSize;

	
	bool Compare(PRTSettings* rhs)
	{
		/// Compare data, has anything changed that should trigger a recompile?
		bool shChanged = Enabled != rhs->Enabled || dwNumRays != rhs->dwNumRays || dwNumBounces != rhs->dwNumBounces || bSubsurfaceScattering != rhs->bSubsurfaceScattering
			|| fLengthScale != rhs->fLengthScale || Diffuse != rhs->Diffuse || Absoption != rhs->Absoption || ReducedScattering != rhs->ReducedScattering
			|| fRelativeIndexOfRefraction != rhs->fRelativeIndexOfRefraction || bAdaptive != rhs->bAdaptive || bRobustMeshRefine != rhs->bRobustMeshRefine
			|| fRobustMeshRefineMinEdgeLength != rhs->fRobustMeshRefineMinEdgeLength  || dwTextureSize != rhs->dwTextureSize  || Quality != rhs->Quality  
			|| bPerPixel != rhs->bPerPixel  || dwTextureSize != rhs->dwTextureSize || fSubSurfaceMultiplier != rhs->fSubSurfaceMultiplier;

		shChanged = shChanged || (InBlockers.size() != rhs->InBlockers.size());
		shChanged = shChanged || (OutBlockers.size() != rhs->OutBlockers.size());

		/// Compare blocker lists
		if(!shChanged){
			for(int i=0;i<InBlockers.size();i++){
				if(InBlockers[i] != rhs->InBlockers[i])
					shChanged = true;
			}
			for(int i=0;i<OutBlockers.size();i++){
				if(OutBlockers[i] != rhs->OutBlockers[i])
					shChanged = true;
			}
		}

		return shChanged;
	}

	void AddInBlockers(vector<string>& blockers)
	{
		for(int i=0;i<blockers.size();i++)
		{
			bool found = false;
			for(int j=0;j<InBlockers.size();j++)
			{
				if(InBlockers[j] == blockers[i]){
					found = true;
					break;
				}
			}
			if(!found)
				InBlockers.push_back(blockers[i]);
		}
	}

	void AddOutBlockers(vector<string>& blockers)
	{
		for(int i=0;i<blockers.size();i++)
		{
			bool found = false;
			for(int j=0;j<OutBlockers.size();j++)
			{
				if(OutBlockers[j] == blockers[i]){
					found = true;
					break;
				}
			}
			if(!found)
				OutBlockers.push_back(blockers[i]);
		}
	}

	void RemoveInBlockers(vector<string>& blockers)
	{
		for(int i=0;i<blockers.size();i++)
		{
			bool found = false;
			int j;
			for(j=0;j<InBlockers.size();j++)
			{
				if(InBlockers[j] == blockers[i]){
					found = true;
					break;
				}
			}
			if(found)
				InBlockers.erase(InBlockers.begin()+j);
		}
	}

	void RemoveOutBlockers(vector<string>& blockers)
	{
		for(int i=0;i<blockers.size();i++)
		{
			bool found = false;
			int j;
			for(j=0;j<OutBlockers.size();j++)
			{
				if(OutBlockers[j] == blockers[i]){
					found = true;
					break;
				}
			}
			if(found)
				OutBlockers.erase(OutBlockers.begin()+j);
		}
	}

	PRTSettings();
};

//=============================================================================
/// Holds rendering element
//=============================================================================
struct AttributeRange
{
	DWORD AttribId;
	DWORD FaceStart;
	DWORD FaceCount;
	DWORD VertexStart;
	DWORD VertexCount;
	//DWORD VertexOffset;
};

#pragma pack( pop )
//=============================================================================
/// \brief A single Mesh. It's a collection of data that fully
/// specifies everything needed for rendering. 
//
/// This struct is used by the rendering tree, and models.
/// You should never need to directly create this
/// Can hold skinning data too.
//=============================================================================
class Mesh {
protected:
    friend class MeshOps;
	/// Used to encapsulate mesh spherical harmonics processing
	class CPRTMesh*					m_pPRTMesh;	
	
	/// Skinning
	DWORD					m_NumPaletteEntries;
	DWORD					m_NumInfl;
	DWORD					m_NumAttributeGroups;
	/// Temp output matrices, ignore
	D3DXMATRIXA16*			m_pBoneMatrices;	
	/// Offsets, directly from skin info
	D3DXMATRIX*				m_pBoneOffsetMatrices; 

	LPD3DXMESH				m_pMesh;

	/// Slow to get VB and IB every frame from LPD3DXMESH, so store here
	LPDIRECT3DVERTEXBUFFER9 m_pVertexBuffer;
	LPDIRECT3DINDEXBUFFER9  m_pIndexBuffer;

     /// Update internal buffers after mesh change
    void UpdateBuffers();
    
    /// Updates all internal bounding boxes
    HRESULT UpdateBoundingInfo(const D3DXVECTOR3 *pFirstPosition, DWORD NumVertices, DWORD dwStride, BYTE* Indices, bool b32bit);

public:
    /// Optional lightmap texture
    Texture*        m_LightMap;
    ///
    bool            HasLightMapping();
    /// Remaps an attribute ID to a specific material
    vector<DWORD>   m_AttribToMaterial;
    /// Store boxes for sub-attributes
    vector<BBox>    m_AttribBoxes;
    
	LPD3DXSKININFO			m_pSkinInfo;
	LPDIRECT3DVERTEXDECLARATION9	m_pDeclaration;
	
	/// Editable variables displayed in reality builder
	vector<EditorVar>	m_EditorVars;
	/// Draw calls on this mesh since last frame, for debugging/profiling
	int m_DrawCalls;
	/// Timestamps for incremental compiling
	SYSTEMTIME	m_TimeMoved;
	SYSTEMTIME  m_TimeModified;

	/// Mesh SH data
	PRTSettings					m_SHOptions;
	int							m_OriginalVertexSize;
	void						SetMesh(LPD3DXMESH mesh, bool setAttribs=true);
	LPD3DXMESH					GetHardwareMesh(){ return m_pMesh; }
	CPRTMesh*					GetPRTMesh(){ return m_pPRTMesh; }
	bool						UsingPRT();
    bool                        UsingColorVertex();
	bool						m_bPRTMultiPass;
	bool						m_bDontRender;
	LPD3DXBUFFER				m_pBoneComb;
	BBox						m_LocalBox;
	vector<Material*>			m_Materials;
	vector<AttributeRange>		m_AttribTable;
	/// Counter for updating PRT only periodically
	float						m_UpdateTime;
    /// Last PRT color, so we can force an update if there is a significant lighting change
    FloatColor                  m_LastColor;
    /// Last PRT sun dir, if sky lit
    Vector                      m_LastSunDir;

	Mesh();
	~Mesh();

	/// Stores per-bone weighting values for animated Mesh
	struct SkinWeights{
		string name;
		vector<DWORD>  indices;
		vector<float> weights;
		Matrix offset;
	};
	/// Extracts full Bone data
	void GetBones(vector<SkinWeights>& bones);
	/// Creates appropriate skinning info for bones. Must follow with GenerateSkinInfo()
	void SetBones(vector<SkinWeights>& bones);
	/// Number of bones
	int GetNumBones(){ if(!m_pSkinInfo) return 0; return m_pSkinInfo->GetNumBones(); }
	string GetBoneName(int i){ return m_pSkinInfo->GetBoneName(i); }
	void LoadPRT(string prtFile);
	void Optimize(bool bWeld);
    /// Draw a mesh subset
	void DrawSubset(AttributeRange subset);
    /// DrawSubset without vertex decl
    void DrawSubsetFVF(AttributeRange subset);
	bool Create(BYTE* vertices, BYTE* indices, int vertexSize, int indexSize, int numVertices, int numIndices, vector<Material*> materials = vector<Material*>(0));
	void SetSHStates(Shader* shader, AttributeRange subset);
	void SetSkinningRenderStates(Shader* shader, AttributeRange subset, vector<Matrix*> pBoneMatrixPtrs);
	void GenerateSkinInfo(); /// Generates from bones array
	void Clone(Mesh* newMesh, bool cloneData = true);
	void CalcData(bool calcBox = false);
	/// Fills default materials/attribs from D3DX mesh
	void FillAttributesFromMesh(ID3DXMesh* mesh);
    /// Get material from id. This handles skinning subsets etc too
    Material*   GetMaterial(int id);

    // Functions for handling altering of vertex data
    // These functions handle the internal vertex state, including compression
    void    SetVertex(BYTE* vertex, Vertex& v);
    Vertex  GetVertex(BYTE* vertex);

};


//----------------------------------------------------------------------------------
/// Mesh Helper class
//----------------------------------------------------------------------------------
class ENGINE_API VertexFormats : public RenderBase
{ 
public:
    /// Holds decl/element pair
    struct VFormat
    {
        LPDIRECT3DVERTEXDECLARATION9 decl;
        D3DVERTEXELEMENT9* element;
        string name;
        int vertexSize;
    };

    /// Find vertex by size
    VFormat* FindFormat(int vertexSize)
    {
        for(int i=0;i<formats.size();i++)
            if(formats[i].vertexSize == vertexSize)
                return &formats[i];
        return 0;
    }

    /// Find vertex by size
    VFormat* FindFormat(string name)
    {
        for(int i=0;i<formats.size();i++)
            if(formats[i].name == name)
                return &formats[i];
        return 0;
    }

    /// Initializes all default formats
    void Initialize();
    /// Shuts down system
    void Shutdown(){ for(int i=0;i<formats.size();i++) SAFE_DELETE(formats[i].element); }

    /// Add a new vertex format
    bool AddFormat(D3DVERTEXELEMENT9* element, int vertexSize, string name)
    {
        VFormat v;
        int elements = D3DXGetDeclLength(element)+1;
        v.vertexSize = vertexSize;
        v.element = new D3DVERTEXELEMENT9[elements];
        v.name = name;
        memcpy(v.element,element,elements*sizeof(D3DVERTEXELEMENT9));
        if(RenderWrap::dev->CreateVertexDeclaration( v.element, &v.decl ) == S_OK )
        {
            formats.push_back(v);
            return true;
        }
        return false;
    }

    /// Rebuilds decls on reset
  /*  virtual HRESULT OnResetDevice(){ 
        for(int i=0;i<formats.size();i++)
            DXASSERT(RenderWrap::dev->CreateVertexDeclaration( formats[i].element, &formats[i].decl ) );
        return S_OK;
    }
    /// Kills decls on lost device
    virtual void	OnLostDevice(){ for(int i=0;i<formats.size();i++) SAFE_RELEASE(formats[i].decl); } 
*/
    /// Singleton
	static VertexFormats* Instance();

protected:
    /// Holds all vertex formats
    vector<VFormat> formats;
};


//----------------------------------------------------------------------------------
/// Mesh Helper class
//----------------------------------------------------------------------------------
class ENGINE_API MeshOps
{
public:
	/// Operations to perform on mesh
	enum Op
	{
		Invert,
		InvertNormals,
		FromMax,
		FromMaya,
        BakeTM,                 // Param is tm to bake
        CenterPivot             // Param will be new pivot
	};

	/// Perform specified operation on mesh data
	static bool Convert(Mesh* mesh, Op op, Matrix& param=Matrix());
    /// Mesh split operation used for lighting
    static bool Split(Mesh* mesh, int xSegs, int ySegs, int zSegs);
    /// PRT->Static Lighting function. Takes source world supplying current lighting information
    static bool PRTToStatic(ModelFrame* mesh, World* world);
    /// Generate fresh tangents/normals
    static bool Mend(Mesh* mesh, bool genNormals, float creaseAngle, float weightNormals, bool fixCylindrical, bool respectSplits);
    /// Breaks mesh into new meshes(prefabs) based on attribute elements
    static bool Break(Mesh* mesh, Actor* actor, vector<Actor*>& newMeshes);
};