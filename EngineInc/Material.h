//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
/// Material:
/// 
/// A material wholly encapsulates the surface properties of any object
/// 
/// Materials can have sounds and effects associated with them. 
/// This is at the game level - see MaterialHelper.h
/// 
/// All models and world objects have materials, which contain all the surface
/// data (like textures, type, illumination) needed for rendering and world interactions
/// 
/// Author: Tim Johnson
///
//====================================================================================
#pragma once

//-----------------------------------------------------------------------------
// Technique info
//
/// Holds everything Reality must know about a shader technique
struct Technique
{
    /// Technique handle
    LPCSTR  Handle;
    /// Maximum lights per pass
    int     MaxLights;
    /// Supports PRT?
    bool    PRT;
    /// Supports Per-Pixel?
    bool    PerPixel;
    /// Supports light-mapping
    bool    LightMapping;
    /// Supported light types
    int     LightTypes;
    /// Required shader version
    float   PixelShaderVersion;
    /// Supports shadow projector?
    bool    ShadowProjector;
    /// Used to specify grouping, if one shader has many different techniques.
    /// Filtering will only occur if technique strings match
    string  Group;
    /// Technique name, for debugging purposes
    string  Name;
    /// Blend state
    bool    m_AlphaBlend;
    /// Test state
    bool    m_AlphaTest;
    /// Source blending
    DWORD   m_SrcBlend;
    /// Dest blending
    DWORD   m_DestBlend;

    Technique();
};


//-----------------------------------------------------------------------------
/// High-Level Material. Contains a shader and textures
//-----------------------------------------------------------------------------
class ENGINE_API Material {
private:
    /// All valid techniques loaded
    vector<Technique>   m_Techniques;
   
	~Material();
    /// Parameter tweakble in engine, FX Composer, RenderMonkey, etc??
    bool Tweakable(LPCSTR hParam);
public:
    ///
    void CloneTo(Material* material);

	GUID		m_GUID;

     /// Token string defining current enumerated techniques
    string      m_Token;
	/// Used for editor
	vector<EditorVar>	EditorVars;
	vector<string>		m_TechniqueNames;
	string				m_Category;

    /// Refresh material settings from shader, if adjusted
    void        Refresh();
    /// Enumerates techniques containing a token
    void        SetTechnique(string token="");
	/// Gets technique capable of supporting specified attributes
	Technique*	GetTechnique(LightType tech, bool m_bPRTEnabled, bool shadowProjector, bool lightMapping); 
    /// Get technique based on group tag
    Technique*  GetTechnique(string groupTag);
	/// Used for collision face lookup at scene load, ignore
	int			m_ID;							
	/// Shader for this material
	Shader*		m_Shader;						
	string		m_ShaderName;
	string		m_Name;
	/// See MaterialHelper for use of this
	string		m_Type;				
	/// All shaders have this. Kept for speedy lookup
	ShaderVar*  m_Emissive;			

	/// Optimized D3DXHANDLE block holding all params
	LPCSTR				m_ParamBlock; 
	/// All effects parameters
	vector<ShaderVar*>	m_Parameters; 

	ShaderVar*  FindVar(string name);
	/// Call this after all custom var->Set() calls, because Materials cache their vars into stateblocks
	/// This updates the state block, which is a bit slow, so don't do it too often
	void		UpdateStates();
	/// Call after setting all parameters
	bool		Initialize(const char* shader, const char* technique, bool bForceRecompile = false); 
	bool		Apply(bool useParamBlock=true);
	void		ExtractParameters();
	void		SetEditorVars();

	Material();
	Material(string name);
	
	/// File system
	string			m_FileName;
	/// Loads material from file
	bool			Load(const char* filename);
	/// Saves material to file
	bool			Save(string filename);

	/// Ref system
	int		m_RefCount;
	void	AddRef();
	DWORD	Release();
};


//-----------------------------------------------------------------------------
/// \brief Material Library structure used by editor
//
/// GUIDs are used to identify materials even if they get renamed
/// (No two GUIDs on earth can ever be the same ;-))
//
/// Only initializes materials as they are requested, for performance
/// 
/// CoCreateGuid
//
//-----------------------------------------------------------------------------
class ENGINE_API MaterialLibrary
{
protected:
	vector<Material*>	Materials;
public:
	MaterialLibrary();
	~MaterialLibrary();
	
	
	string				FileName;

	/// Gets material, loading on-the-fly as necessary
	int		  Count(){return Materials.size();}
	Material* GetMaterial(int index);
	Material* GetMaterial(GUID guid);
	void	  AddMaterial(Material* mat){ Materials.push_back(mat); }
	/// Loads library from file
	bool			Load(string filename);
	/// Saves library to file
	bool			Save(string filename);
};

//-----------------------------------------------------------------------------
/// Material global mangement. Just handles default material right now
//-----------------------------------------------------------------------------
class ENGINE_API MaterialManager{
private:
	friend class MaterialHelper;
	friend class Material;
	friend class MaterialLibrary;
	friend class SceneLoader;
	Material* m_DefaultMaterial;

	/// String array of material names.
	/// Sound filenames are built from these in MaterialHelper.h
	vector<Material*>			m_Materials;
	vector<MaterialLibrary*>	m_Libraries;
	int							m_TotalRefs;

	MaterialManager(){ m_DefaultMaterial = NULL; }
public:
	vector<string>				m_MaterialTypes;

	static		MaterialManager* Instance();
	Material*	GetDefaultMaterial(){ return m_DefaultMaterial; }
	void		Initialize();
	void		Shutdown();
	Material*	FindMaterial(string name);

	/// Material library
	MaterialLibrary*	FindLibrary(string library){ library = AsLower(library); for(int i=0;i<m_Libraries.size();i++) if(AsLower(m_Libraries[i]->FileName).find(library)!=-1){ return m_Libraries[i]; } return 0; }
	

	/// Gets the material type based on texture name lookup
	/// For example if the path or name contains grass, the material type
	/// will be MT_Grass
	/// This should be the default fallback if the designer didn't set a type in the editor
	string GetType(string texName);
};