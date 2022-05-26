//#define VERTEX_COLORS
extern LPDIRECT3DDEVICE9 g_pd3dDevice;

// Change compiler pack alignment to be BYTE aligned, and pop the current value
#pragma pack( push, 1 )

#define weld 0.001f
#define sameD(a,b) (a > b-weld && a < b+weld)
#define sameV(a,b) (sameD(a.x,b.x) && sameD(a.y,b.y) && sameD(a.z,b.z))
#define sameV2(a,b) (sameD(a.x,b.x) && sameD(a.y,b.y))

#define DXASSERT(x) {HRESULT hr;if(FAILED(hr=(x))){ Error(" Error: %s, in: "###x,DXGetErrorString9(hr));}}



enum FrameType{
TYPE_UNKNOWN = 0,
TYPE_STATIC_GEOMETRY,
TYPE_ENTITYORPREFAB,
TYPE_LIGHT
};

// Forward decl
struct ImportFrame;

//----------------------------------------------------------------------------------
// Desc: Exportable Vertex class
//----------------------------------------------------------------------------------
struct Vertex{
	Vector p;
	Vector n;
	Vector2 t;
	Vector tan;
#ifdef VERTEX_COLORS
	COLOR c;
#endif

	bool operator== ( const Vertex &c ) const
    {
		// Compare vertices (with floating point tolerance)
#ifdef VERTEX_COLORS
		return sameV(p,c.p) && sameV(n,c.n) /*&& sameV(tan,c.tan)*/ && sameV2(t,c.t) && this->c == c.c;
#else
		return sameV(p,c.p) && sameV(n,c.n) /*&& sameV(tan,c.tan)*/ && sameV2(t,c.t);
#endif
    }
};

// Spherical Harmonics Vertex
#define MAX_NUM_PCA 24
struct SHVertex{
	Vector p;
	Vector n;
	Vector2 t;
	Vector tan;
#ifdef VERTEX_COLORS
	COLOR c;
#endif
	int		iClusterOffset;
	Vector4 vPCAWeights[MAX_NUM_PCA/4];

	bool operator== ( const Vertex &c ) const
    {
		// Compare vertices (with floating point tolerance)
#ifdef VERTEX_COLORS
		return sameV(p,c.p) && sameV(n,c.n) /*&& sameV(tan,c.tan)*/ && sameV2(t,c.t) && this->c == c.c;
#else
		return sameV(p,c.p) && sameV(n,c.n) /*&& sameV(tan,c.tan)*/ && sameV2(t,c.t);
#endif
    }
};

#pragma pack( pop )


//----------------------------------------------------------------------------------
// Desc: Plane class
//----------------------------------------------------------------------------------
struct Plane {
   double a, b, c, d;
   Vector norm;
   Vector center;

   Plane(Vector& n, Vector& p){
	   norm = n;
	   center=p;
   		a = n.x;
		b = n.y;
		c = n.z;
		d =  -(p.x*a + p.y*b + p.z*c);
   }
}; 

//----------------------------------------------------------------------------------
// Desc: Face class used during compiler stage
//----------------------------------------------------------------------------------
struct Face{
	Vertex v[3];
	class Mesh* srcMesh; // Mesh this face came from. Used for misc things like light include/exclude lists

	// The singular material id we use after 'flattening' submaterials
	int flatmatid;

	// For debugging
	int user1;
	int user2;

	Face(){ user1=0; user2=0; }
};

//----------------------------------------------------------------------------------
// Slight modification of D3DX attribute range, addition of VertexOffset
//----------------------------------------------------------------------------------
struct AttributeRange
{
	DWORD AttribId;
	DWORD FaceStart;
	DWORD FaceCount;
	DWORD VertexStart;
	DWORD VertexCount;
	DWORD VertexOffset;
};

//----------------------------------------------------------------------------------
// Desc: A pool is a temp structure used to hold rendering info for the scene tree
// It is NOT used for models
//----------------------------------------------------------------------------------
struct Pool{
	bool Stripified; // always false right now
	int  matID;
	vector<struct Light*>	staticLights; // Lights this tree node includes

	// These are temporary, until the pool data gets merged into a global mesh
	vector<WORD>	indices;
	vector<Vertex>	vertices;

	// Data after merging of buffers
	BBox			   box;
	int				   meshID;
	AttributeRange	   subset;

	Pool(){ matID = -1; Stripified = false; }
};

//-----------------------------------------------------------------------------
// Desc: Simple collision data that's converted to a tree in the engine
//-----------------------------------------------------------------------------
struct CollisionData {
	vector<DWORD>	indices;
	vector<Vector>	vertices;
	vector<int>		faceIDs;
	bool			localspace;
};

//-----------------------------------------------------------------------------
// Desc: Exportable Map
//-----------------------------------------------------------------------------
struct TexMap{
	DWORD  type;
	string filename;
	float  amount;
	float  uOff, vOff, uTile, vTile;
	string texVar;
	string transformVar;
};

//-----------------------------------------------------------------------------
// Desc: Exportable Material
//-----------------------------------------------------------------------------
struct Material{
	bool	bReference; // Material was already exported, so just export reference info so game can find it
	int		id; // Unique ID so we can do fast referencing. Mainly for collision faces
	bool	twosided;

	string		name;
	string		shader;
	string		technique;
	vector<ShaderParam> params;

	vector<TexMap> maps;

	Material(){ twosided = false; bReference = false; }
};

//----------------------------------------------------------------------------------
// Desc: A single Mesh. Can hold skinning data too.
//----------------------------------------------------------------------------------
class Mesh {
protected:
	friend class Exporter;

	void GenerateTangentsAndNormals(vector<Vertex>& new_verts, vector<WORD>& new_indices, vector<DWORD>& remap);

	LPD3DXMESH				mesh;
	LPD3DXBUFFER            pAdjacency;
	LPD3DXSKININFO          pSkinInfo;
public:
	string				m_Name;
	bool				m_HasSHFile;
	vector<Material>	materials;

	Mesh(){
		m_HasSHFile = false;
		mesh = NULL;
		pAdjacency = NULL;
		pSkinInfo = NULL;
	}
	~Mesh();

	struct SkinWeights{
		string name;
		vector<DWORD>  indices;
		vector<float> weights;
		Matrix offset;
	};

	int GetNumSubsets(){
		if(!mesh)
			return 0;
		DWORD attribTableSize;
		mesh->GetAttributeTable(NULL,&attribTableSize); 
		assert(attribTableSize != 0);
		return attribTableSize;
	}
	//void GetSHData(vector<WORD>& indices, vector<SHVertex>& vertices, BBox& box, int subset=-1);
	void GetData(vector<WORD>& indices, vector<Vertex>& vertices, BBox& box, int subset=-1);
	void GetBones(vector<SkinWeights>& bones);
	void Optimize(bool bWeld);

	void SetMesh(LPD3DXMESH m);
	LPD3DXMESH GetMesh(){ return mesh; }
	void Create(vector<Vertex>& vertices, vector<WORD>& indices, vector<Material> materials = vector<Material>(0));
	void Create(string name, LPD3DXMESH mesh, LPD3DXSKININFO inSkinInfo, LPD3DXBUFFER inAdjacency, vector<Material>& materials, Matrix scaling, bool optimize = true);
	void GetAllFaces(ImportFrame* srcFrame, vector<Face>& faces, Matrix tm, vector<int> materialLookup);
};

//----------------------------------------------------------------------------------
// Desc: Exportable Light Frame
//----------------------------------------------------------------------------------
struct LightFrame {
	DWORD		time;
	D3DXCOLOR	color;
	float		intensity;
	float		hotsize;
	float		falloff;
	float		attenStart;
	float		attenEnd;
	Matrix		tm;
};

//-----------------------------------------------------------------------------
// Desc: Exportable animated Light
//-----------------------------------------------------------------------------
struct Light{
	string  name;
	BBox	box;
	DWORD	type;
	string projectionMap;
	string shadowMap;

	bool IsExcludeList;
	Light(){IsExcludeList = true; }
	vector<string> excludeIncludeList;
	vector<class Mesh*> excludeIncludeMeshPointers;

	vector<LightFrame> keyframes;
	float keyframeTime;
};

//-----------------------------------------------------------------------------
// Desc: Uncompiled Importer Hierarchy
//-----------------------------------------------------------------------------
struct ImportFrame
{
public:
	// Scene root properties (only ever set for first node)
	SceneProperties* sceneProperties;

	// DX Imported Mesh data
	LPD3DXMESH				pMesh;
	LPD3DXBUFFER			pMaterials;
	DWORD                   NumMaterials;
	LPD3DXBUFFER            pAdjacency;
	LPD3DXSKININFO          pSkinInfo;

	// Frame data
	FrameType				type;
	char					Name[512];
	Matrix					TransformationMatrix;
	Matrix					CombinedTransformationMatrix;

	// Entity attributes
	NodeData				nodeData;

	// Items that can be in a frame
	Mesh*				mesh;		// Check .GetNumSubsets()
	vector<Material>	materials;  // Check .size()
	Light				light;		// Check .keyframes.size()

	ImportFrame* m_pNext;
	ImportFrame* m_pChild;

public:
	bool CompareName(string name){
		if(name == string(Name))
			return true;
		if((name+"_Child") == string(Name))
			return true;
		return false;
	}

	ImportFrame* FindFrame(string& name){
		if(CompareName(name))
			return this;

		if(m_pChild){
			ImportFrame* ret = m_pChild->FindFrame(name);
			if(ret)
				return ret;
		}

		if(m_pNext){
			ImportFrame* ret = m_pNext->FindFrame(name);
			if(ret)
				return ret;
		}

		return NULL;
	}

	void PrintFrames(){
		LogPrintf("%s\n",Name);
		if(string(Name)=="frontdoorframe")
			m_pChild=m_pChild;

		if(m_pChild){
			m_pChild->PrintFrames();
		}

		if(m_pNext){
			m_pNext->PrintFrames();
		}
	}

	ImportFrame* FindMeshFrame(string& name){
		// Return mesh, or if it's a prefab return too, because
		// it won't have a mesh, that'll be external instead
		if(CompareName(name) && (mesh || type == TYPE_ENTITYORPREFAB))
			return this;

		if(m_pChild){
			ImportFrame* ret = m_pChild->FindMeshFrame(name);
			if(ret)
				return ret;
		}

		if(m_pNext){
			ImportFrame* ret = m_pNext->FindMeshFrame(name);
			if(ret)
				return ret;
		}

		return NULL;
	}

	ImportFrame(const char* name="Root Frame"){
		type		= TYPE_STATIC_GEOMETRY;
		sceneProperties = NULL;
		m_pNext		= NULL;
		m_pChild	= NULL;
		pSkinInfo	= NULL;
		pMaterials	= NULL;
		pMesh		= NULL;
		mesh		= NULL;
		NumMaterials= NULL;
		pAdjacency	= NULL;
		strcpy(Name,name);
	}

	virtual ~ImportFrame(){
		SAFE_RELEASE(pMesh);
		SAFE_RELEASE(pSkinInfo);
		SAFE_RELEASE(pAdjacency);
		SAFE_RELEASE(pMaterials);

		SAFE_DELETE(m_pNext);
		SAFE_DELETE(m_pChild);
		SAFE_DELETE(sceneProperties);
	}
};


#include "Octree.h"

//-----------------------------------------------------------------------------
// Desc: Model File exportable frame
//-----------------------------------------------------------------------------
struct ExportFrame
{
public:
	string					name;
	Matrix					tm;
	Mesh*					mesh;
	CollisionData			collisionData;
	Light					light;
	ExportFrame*			m_pNext;
	ExportFrame*			m_pChild;

public:
	ExportFrame(){
		m_pNext = NULL;
		m_pChild = NULL;
	}
	virtual ~ExportFrame(){
		SAFE_DELETE(m_pNext);
		SAFE_DELETE(m_pChild);
	}
};

//-----------------------------------------------------------------------------
// Desc: Entity or Prefab
// TODO: Add support for level-defined animation sets here
//-----------------------------------------------------------------------------
struct Entity {
	ExportFrame*	modelFrame; // Entities can have models tagged directly from max
	NodeData		data;
	Matrix			worldTM;

	Entity(){ modelFrame = NULL; }

	// TODO:
	// AnimationSet* prefabFrames;
};

//-----------------------------------------------------------------------------
// Desc: Level File exportable definition
//-----------------------------------------------------------------------------
struct LevelFile{
	SceneProperties sceneProperties;

	// TODO: Add keyframe hierarchy for moving parts. Share code with Model
	// Could just use a model and pass it our world AnimationSet? That'll work!
	vector<Light>				lights;
	vector< Entity >			entities;
	vector< Mesh* >				meshes;
	Node						root;
	CollisionData				collisionData;
};



//-----------------------------------------------------------------------------
// Desc: Model File exportable definition
//-----------------------------------------------------------------------------
struct ModelFile{
	ExportFrame*			root;
	ModelFile(){
		root = NULL;
	}
	~ModelFile(){
		SAFE_DELETE(root);
	}
};

