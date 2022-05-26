//=========== Copyright (c) 2004, Tim Johnson. All rights reserved. ===========
/// Shared Structures. Used by MAX and Engine
//
//=============================================================================
#pragma once

class	Vector;
struct	FloatColor;
class	Vector4;


/// Texture coordinate rectangle
struct CoordRect
{
	float fLeftU, fTopV;
	float fRightU, fBottomV;
};


//-----------------------------------------------------------------------------
/// \brief Generic type container, used by Editor and Engine to serialize
/// any and every class parameter for editing and saving
//-----------------------------------------------------------------------------
class ENGINE_API EditorVar {
public:
	enum Type {
		UNKNOWN,
		BOOL,
		INT,
		FLOAT,
		FLOAT2,
		FLOAT3,
		FLOAT4,
		MATRIX,
		FILENAME,
		TEXTURE,
		D3DTEXTURE,
		STRING,
		COLOR,
	};
	static string ToString(Type t){ 
		if(t==BOOL) return "Bool";
		if(t==INT) return "Int";
		if(t==FLOAT) return "Float";
		if(t==FLOAT2) return "Float2";
		if(t==FLOAT3) return "Float3";
		if(t==FLOAT4) return "Float4";
		if(t==FILENAME) return "Filename";
		if(t==TEXTURE) return "Texture";
		if(t==STRING) return "String";
		if(t==COLOR) return "Color";
		return "Unknown";
	}
	static Type FromString(string t){ 
		if(t=="Bool") return BOOL;
		if(t=="Int") return INT;
		if(t=="Float") return FLOAT;
		if(t=="Float2") return FLOAT2;
		if(t=="Float3") return FLOAT3;
		if(t=="Float4") return FLOAT4;
		if(t=="Filename") return FILENAME;
		if(t=="Texture") return TEXTURE;
		if(t=="String") return STRING;
		if(t=="Color") return COLOR;
		return UNKNOWN;
	}

	/// Var display type
	Type		type;		
	/// Var display name
	string		name;		
	/// Description shown in pane
	string		desc;		
	/// Category within browser
	string		category;	
	/// Data, of type 'type'
	void*		data;		

	/// Overloaded Set() calls
	EditorVar(){}
	EditorVar(string newName, bool* var, string category="General", string desc=""){ name = newName;	data = var; type = BOOL; this->category = category; this->desc = desc;   }
	EditorVar(string newName, int* var, string category="General", string desc=""){ name = newName;		data = var; type = INT; this->category = category; this->desc = desc;   }
	EditorVar(string newName, float* var, string category="General", string desc=""){ name = newName;	data = var; type = FLOAT; this->category = category; this->desc = desc;   }
	EditorVar(string newName, Vector* var, string category="General", string desc=""){ name = newName;	data = var; type = FLOAT3; this->category = category; this->desc = desc;   }
	EditorVar(string newName, FloatColor* var, string category="General", string desc=""){ name = newName;	data = var; type = COLOR; this->category = category; this->desc = desc;   }
	EditorVar(string newName, Vector2* var, string category="General", string desc=""){ name = newName; data = var; type = FLOAT2; this->category = category; this->desc = desc;  }
	EditorVar(string newName, Vector4* var, string category="General", string desc=""){ name = newName; data = var; type = FLOAT4; this->category = category; this->desc = desc;  }
	EditorVar(string newName, string* var, string category="General", string desc=""){ name = newName; data = var; type = STRING; this->category = category; this->desc = desc;  }
	EditorVar(string newName, class Texture* var, string category="General", string desc=""){ name = newName; data = var; type = TEXTURE; this->category = category; this->desc = desc;  }

};


//--------------------------------------------------------------------------------------
#pragma pack(push, 1)
/// Contains per-Mesh settings to use when running the Precomputed Radiance Transfer simulation
struct SIMULATOR_OPTIONS
{
    /// General settings
    int		  dwNumRays;
    int     dwOrder;
    int     dwNumChannels;
    int     dwNumBounces;
    bool      bSubsurfaceScattering;
    float     fLengthScale;

    /// Material options
    FloatColor Diffuse;
    FloatColor Absoption;
    FloatColor ReducedScattering;
    float     fRelativeIndexOfRefraction;
	int     dwPredefinedMatIndex;

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
    //bool      bBinaryOutputXFile;
    
    /// Compression options
    //bool      bSaveCompressedResults;
    //D3DXSHCOMPRESSQUALITYTYPE Quality;
    int     dwNumClusters;
    int     dwNumPCA;

	/// TIM: Per-pixel
	bool	bPerPixel;
	int	dwTextureSize;
};
#pragma pack(pop)

/// Class/Entity data
#define NODEDATA_VERSION 107

//-----------------------------------------------------------------------------
/// \brief Actor Node Data
/// This is used as meta-data when re-exporting the scene.
//
/// This also holds parameters used during the compile or load process
/// Script Properties, Spherical Harmonics, etc
//-----------------------------------------------------------------------------
struct NodeData 
{
	/// Timestamps for incremental compiling
	SYSTEMTIME	timeMoved;
	SYSTEMTIME  timeModified;

	/// Information about a Script (class name and parameters), if the Actor has a Script bound to it.
	struct ScriptData{
		string	 filename;
		string	 classname;
		string	 parentclass;
		string	 comments;
		vector<string> parameters;
		vector<string> paramvalues;
		bool				bIncludeModel;
	};
	ScriptData			script;

	bool				bSHEnabled;
	bool				bUseCustom;

	/// Mesh SH data
	string				receiverGroup;
	vector<string>		inBlockers;
	vector<string>		outBlockers;
	SIMULATOR_OPTIONS	shOptions;

	NodeData(){
		GetSystemTime(&timeMoved);
		GetSystemTime(&timeModified);
		script.bIncludeModel = false;
		bSHEnabled			 = false;
		bUseCustom			 = false;
	}

	~NodeData(){
	}

	bool CompareSH(NodeData& rhs)
	{
		/// Compare data, has anything changed that should trigger a recompile?
		bool shChanged = (memcmp(&shOptions,&rhs.shOptions,sizeof(SIMULATOR_OPTIONS)) != 0);
		shChanged = shChanged || (receiverGroup != rhs.receiverGroup);
		shChanged = shChanged || (inBlockers.size() != rhs.inBlockers.size());
		shChanged = shChanged || (outBlockers.size() != rhs.outBlockers.size());

		/// Compare blocker lists
		if(!shChanged){
			for(int i=0;i<inBlockers.size();i++){
				if(inBlockers[i] != rhs.inBlockers[i])
					shChanged = true;
			}
			for(int i=0;i<outBlockers.size();i++){
				if(outBlockers[i] != rhs.outBlockers[i])
					shChanged = true;
			}
		}

		return shChanged;
	}


	operator == (NodeData& rhs){
		return script.filename == rhs.script.filename && script.paramvalues.size() == rhs.script.paramvalues.size(); 
			//&& receiverGroup == rhs.receiverGroup;
	}
};


/// Data held in .max files that overrides config data
struct SceneProperties {
	TCHAR		skyworld[MAX_PATH];
	TCHAR		miniMap[MAX_PATH];
	float		miniMapScale;
	COLORREF	fogColor;
	DWORD		fogStart;
	DWORD		clipPlane;

	TCHAR		modelfile[MAX_PATH];
	TCHAR		mapfile[MAX_PATH];
	TCHAR		cubeMap[MAX_PATH];

	/// Non-Exported scene properties
	DWORD		bInitialized;
	DWORD		dwRays;
	DWORD		dwBounces;
	DWORD		dwTextureSize;
	WORD		bPRTOverride; /// Override values come from options.xml

	SceneProperties(){
		bInitialized = false;
		bPRTOverride = false;
		fogColor = 0xFFFFFF;
		fogStart = 1000;
		clipPlane = 2000;
	}
};
/*
struct CustomTextureMap 
{
	DWORD  type;
	string texVar;
	string transformVar;
	TCHAR* name;
	float amount;
	float uTile;
	float vTile;
	float uOff, vOff;
};

/// Used by material. Simply holds a generic shader parameter
struct ShaderParam {
	string		name;
	DWORD		type;
	D3DXVECTOR4	vVal;
	float		fVal;
	string		sVal;
	DWORD		iVal;
	bool		bVal;

	/// Overloaded ctors...
	ShaderParam(string sName, D3DXVECTOR4 v){
		type = EditorVar::FLOAT4;
		vVal = v;
		name = sName;
	}
	ShaderParam(string sName, DWORD c){
		type = EditorVar::INT;
		iVal = c;
		name = sName;
	}
	ShaderParam(string sName, bool b){
		type = EditorVar::BOOL;
		bVal = b;
		name = sName;
	}
	ShaderParam(string sName, float c){
		type = EditorVar::FLOAT;
		fVal = c;
		name = sName;
	}
	ShaderParam(string sName, string c){
		type = EditorVar::TEXTURE;
		sVal = c;
		name = sName;
	}
	ShaderParam(){}
};


struct CUSTOMD3DXMATERIAL {
	/// Shared
	string		shader;
	string		technique;
	TCHAR*		matname;

	vector<ShaderParam> params;

	/// This is for the shader plugin
	bool			dxMaterial;
	/// These are for the standard 3dsmax material
	vector<CustomTextureMap> maps;

	/// These are for the X-File tool
	D3DMATERIAL9  MatD3D;
	LPSTR         pTextureFilename;

	CUSTOMD3DXMATERIAL(){
		dxMaterial = false;
	}
};
*/
