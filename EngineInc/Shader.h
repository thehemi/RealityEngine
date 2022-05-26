//=========== (C) Copyright 2003, Tim Johnson. All rights reserved. ===========
/// Shader Object
//
/// A wrapper around the D3DX effects files and a cached texture manager.
/// Also sets many of the common constants that are used in vertex and 
/// pixel shaders.
//
//=============================================================================

#pragma once

/// These were used in the pipeline that bypasses effects for speed
/// These days we recommend sticking to effect files
#define MAX_PASSES 2		/// When using constant tables
#define MAX_TECHNIQUES 20   /// Hacky

typedef LPCSTR D3DXHANDLE;
typedef interface ID3DXEffect *LPD3DXEFFECT;

//-----------------------------------------------------------------------------
/// \brief Variable object. Use these to set shader variables
//
/// Info:
/// To create a shader var simply call one of the overloaded routines
///
/// Manual management:
/// The DeleteType type allows you to let the shader var auto-delete some data on
/// destruction. This is useful for management.
///
/// NOTE: Keep a copy of your ShaderVar after creating. 
/// Creating them every frame is _SLOW_
///
//-----------------------------------------------------------------------------
struct ENGINE_API ShaderVar {
	friend class Shader;
public:
	EditorVar::Type type;
	string			name;
	string			desc;
	void*			data; /// Derived from 'type'
    int             dataLen;

	enum DeleteType {
		VAR_DONTDELETE,
		VAR_DELETE,
		VAR_DELETEARRAY,
		VAR_DELETETEXTURE
	};

	/// Overloaded Set() calls
	void Set(string newName, bool var){ name = newName; Alloc(&var,sizeof(var)); type = EditorVar::BOOL;   }
	void Set(string newName, int var){ name = newName; Alloc(&var,sizeof(var)); type = EditorVar::INT;   }
	void Set(string newName, float var){ name = newName; Alloc(&var,sizeof(var)); type = EditorVar::FLOAT;   }
	void Set(string newName, Matrix var){ name = newName; Alloc(&var,sizeof(var)); type = EditorVar::MATRIX;   }
	void Set(string newName, Vector var){ name = newName; Alloc(&var,sizeof(var)); type = EditorVar::FLOAT3;   }
	void Set(string newName, Vector4 var){ name = newName; Alloc(&var,sizeof(var)); type = EditorVar::FLOAT4;  }
	void Set(string newName, Texture* var){ name = newName; Destroy(); type = EditorVar::TEXTURE; del = VAR_DELETETEXTURE; data = var;  }
	/// Non-overloaded, avoid when possible
	void Set(string newName, EditorVar::Type newType, DeleteType newDel = VAR_DONTDELETE, void* newData = NULL);

	ShaderVar(string newName=(""), EditorVar::Type newType=EditorVar::FLOAT, DeleteType newDel = VAR_DONTDELETE, void* newData = NULL);
	~ShaderVar(){ Destroy(); }

	void Reset(){ handle = NULL; }

	void Alloc(void* var, int size) { Destroy(); dataLen=size; data = new BYTE[size]; memcpy(data,var,size); del = VAR_DELETEARRAY; }

    D3DXHANDLE GetHandle(){ return handle; }

private:

	//
	/// Private members are filled in automatically
	//
	friend class Shader;
	D3DXHANDLE handle;
	/// These are used if we bypass the D3DEffects Set system
	D3DXHANDLE	Reg[MAX_TECHNIQUES][MAX_PASSES];
	bool	PixelConst[MAX_TECHNIQUES][MAX_PASSES];

	bool	allocated;
	bool	allocatedArray;
	DeleteType del;

	void Destroy();
};


//-----------------------------------------------------------------------------
/// Encapsulates loading & state manipulation of shaders
//-----------------------------------------------------------------------------
class ENGINE_API Shader {
	friend class ShaderManager;
	friend class Material;
public:

    // Custom shader flags set by effects via the annotations system
    /// Shader wants tColorBuffer0 prepared with scene image
    const static int ReadsColorBuffer = (1<<0);
	/// Shader wants tColorBuffer0 prepared with a per-item copy of the frame's LDR buffer (slower)
    const static int ReadsLDRBuffer = (1<<1);
	/// Shader wants tColorBuffer0 prepared with a unified copy of the frame's LDR buffer once for all alpha items (very fast)
	const static int ReadsGlobalColorBuffer = (1<<2);

    /// Special shader flags
    DWORD m_Flags;


	/// Ref system
	int		m_RefCount;
	void	AddRef();
	DWORD	Release();

	Shader();
	~Shader();
	/// Load may return a pointer to an existing Shader, so always use the return value
	bool Load(const CHAR* filename);

	void CommitChanges();

	/// Set any type of effects file variable
	VOID SetVar(ShaderVar& c);
	VOID SetVar(ShaderVar& c, void* data);

	/// Useful sets
	void SetWorld(const Matrix& world);
	void SetLightShaderConstants(vector<Light*>& lights);
	void SetCommonShaderConstants();
	void SetFog(float Density, FloatColor color);
	void SetHDR(bool bEnable, bool bBlend,bool alphaTestPass=false);
	void SetColorBuffer(PDIRECT3DTEXTURE9 colorBufferTexture);
	void UnbindHDRTarget();
	void SetSkinning(int iInfluences, int iMatrices, D3DXMATRIX* matrices);
	void SetUnlit();
    void SetMeshParams(bool hasPRT, bool perPixel, bool colorVertex);
	string GetFilename(){ return filename; }

    /// Number of techniques in shader
    int GetNumTechniques();
    /// Get string of annotation etc
    string GetString(D3DXHANDLE handle);
	
	
	void	   SetTechnique(const D3DXHANDLE technique);
	void	   SetTechnique(string technique){SetTechnique(GetTechnique(technique));}

	D3DXHANDLE GetTechnique(string techniqueName);
	D3DXHANDLE GetCurrentTechnique(){ return curTechniqueHandle; }

	/// Does this shader fully control its own blending states?
	bool OverridesEngineMultipass(){ return m_bOverridesEngineMultipass;} 
	/// Reapply the states this pass uses
	void RestorePassStates();
	/// Returns num passes it wants
	UINT Begin(bool saveState = false); 
	/// Prepares for pass
	void BeginPass(UINT pass); 
	/// Ends a pass
	void EndPass(); 
	/// Ends technique
	void End(); 

	LPD3DXEFFECT GetEffect(){ return effect; }

    /// Assigns shader-specific handle to parameter
    void CreateHandle(ShaderVar& var);

protected:

    //
    // Scripting support
    //

    /// Process script globals
    void ProcessGlobals();

	int					m_CurTechniqueIndex;
	ID3DXConstantTable*	m_CurPixelTable[MAX_TECHNIQUES][MAX_PASSES];
	ID3DXConstantTable*	m_CurVertexTable[MAX_TECHNIQUES][MAX_PASSES];

	LPD3DXEFFECT	effect;
	D3DXHANDLE		curTechniqueHandle;
	string			curTechniqueName;
	Matrix			curWorldMat, curMatWorldInv;
	bool			begun;
	int				curPass;
	string			filename;

	/// Info we collect from shaders
	bool		m_bOverridesEngineMultipass;

	/// Standard constants
	ShaderVar CC_WorldIMatrix;
	ShaderVar CC_pOV;
	ShaderVar CC_mWV;
	ShaderVar CC_mVI;
	ShaderVar CC_mWVP;
	ShaderVar CC_mW;
	ShaderVar CC_mVP;
	ShaderVar CC_mP;
	ShaderVar CC_fSeconds;
	ShaderVar CC_fogDensity;
	ShaderVar CC_fogColor;

	/// Light constants
	ShaderVar CC_LightPos;
	ShaderVar CC_LightRange;
	ShaderVar CC_LightColor;
	ShaderVar CC_LightDir;
	ShaderVar CC_LightFalloff;
	ShaderVar CC_LightHotspot;
	ShaderVar CC_LightProjection;
	ShaderVar CC_LightProjectionMap;
	ShaderVar CC_LightOmniProjectionMap;
	ShaderVar CC_LightOmniProjectionMapBlur;

    /// Light constants (Arrays)
    ShaderVar CC_NumLights;
	ShaderVar CC_LightPosArray;
	ShaderVar CC_LightRangeArray;
	ShaderVar CC_LightColorArray;
	ShaderVar CC_LightDirArray;
	ShaderVar CC_LightFalloffArray;
	ShaderVar CC_LightHotspotArray;
	ShaderVar CC_LightProjectionArray;
	ShaderVar CC_LightProjectionMapArray;
	ShaderVar CC_LightOmniProjectionMapArray;
	ShaderVar CC_LightOmniProjectionMapBlurArray;

	/// Skinning
	ShaderVar CC_CurNumBones;

	/// HDR
	ShaderVar CC_bHDRBlend;
	ShaderVar CC_bHDRalphaTestPass;
	ShaderVar CC_tColorBuffer;
	ShaderVar CC_fScaleBias;
    ShaderVar CC_bDOF;

    /// PRT/Lighting
    ShaderVar CC_bHasPRT;
    ShaderVar CC_bPerPixelPRT;
    ShaderVar CC_bBakedLighting;

	void MapTables();
};


//-----------------------------------------------------------------------
/// \brief Shader management, avoiding redundant Shader & Texture loading & setting global Shader states
/// Also allows reinit of all Shaders if the Device is reset.
//-----------------------------------------------------------------------
class ENGINE_API ShaderManager {
protected:
	friend class Shader;
    ConfigFile  m_Config;
	VOID    SetUsedTextures(DWORD effect);

	/// Texture Management for textures shared by all effects
	struct ShaderTex{
		Texture* texture;
		string name; /// ie tBase in FX file
	};
    vector<ShaderTex> m_sharedTextures;
#ifndef DOXYGEN_IGNORE
	struct ShaderInfo{
		string	filename;
		Shader* shader;
		ShaderInfo(Shader* s, string& f){
			filename = f;
			shader   = s;
		}
	};
#endif
	vector<ShaderInfo> m_Shaders;

	Matrix m_WorldWarp;
	ShaderManager();
public: 

	static ShaderManager* ShaderManager::Instance ();

	/// D3D Resource management

	/// User might have just modified an effects file while game is running
	void ReloadShaders(); 
	void D3DInitialize();
	void D3DRestore();
	void D3DInvalidate();
	void D3DDelete();

	void	Shutdown();
    void    Initialize();
	int		GetNumShaders(){ return m_Shaders.size(); }
	Shader* GetShader(int i){  return m_Shaders[i].shader; }
	/// WARNING: Increments ref count!!
	Shader* GetShader(string& filename); 

	/// Registers textures for shared usage among all effects
    VOID RegisterSharedTexture(CHAR* name, class Texture* pTexture);
	void SetSharedBool(string name, bool b);
	void SetSharedInt(string name, int i);
	/// anisotropy, srgb, etc.
	void UpdateRenderSettings();
	void Tick();
	/// Warps (multiplies) all world matrices by this matrix
	void SetWorldWarp(Matrix& worldWarp){ m_WorldWarp = worldWarp; }
	/// Gets world warp
	Matrix GetWorldWarp(){ return m_WorldWarp; }
};


