//=========== Copyright (c) 2003, Tim Johnson. All rights reserved. ===========
/// The Light class
///
/// You can subclass from this to make more advanced lights (like lights with flares)
/// This class provides basic lighting and light animation capabilities
//=============================================================================
#pragma once

#define NUM_LIGHT_TYPES 4

/// Type enumeration
enum LightType{
	LIGHT_OMNI = (1<<0),
	LIGHT_SPOT = (1<<1),
	LIGHT_DIR = (1<<2),
	LIGHT_OMNI_PROJ = (1<<3),
	FORCE_DWORD    = 0x7fffffff,
};

/// Method enumeration
enum LightMethod
{
	LIGHT_METHOD_PERPIXEL = 0,
	LIGHT_METHOD_SH = 1
};

//------------------------------------------------------------
/// Simple structure that holds the state of a light
//-------------------------------------------------------------
class ENGINE_API LightState {
public:
	/// NOTE: Call UpdateProperties() after changing these!!
	FloatColor		Diffuse;          /*!< Diffuse color of light */
	float			Intensity;
	FloatColor		Specular;         /*!< Specular color of light */
	Vector			Position;         /*!< Position in world space */
	Vector			Direction;        /*!< Direction in world space */
	float           Range;            /*!< Cutoff range */
	float           Spot_Falloff;     /*!< The hotspot falloff size in degrees. */
	float           Spot_Size;        /*!< The hotspot size in degrees. */

	LightState();
	LightState(const Vector& pos, const FloatColor& diffuse, float Range);
	/// Use this CTor if you don't want to specify a position for a keyframe you are creating
	LightState(const FloatColor& diffuse, float Range);

public:
	friend class SceneLoader;

	/// These operators allow us to interpolate lights etc
	LightState operator * (float n) const
	{
		LightState result = *this;
		result.Range = n * Range;
		result.Position = n * Position;
		result.Diffuse = Diffuse * n;
		result.Intensity = Intensity * n;
		return result;
	}

	LightState operator + (const LightState& n) const
	{
		LightState result = *this;
		result.Range = n.Range + Range;
		result.Position = n.Position + Position;
		result.Diffuse = n.Diffuse + Diffuse;
		result.Intensity = Intensity + n.Intensity;
		return result;
	}

	LightState operator - (const LightState& n) const
	{
		LightState result = *this;
		result.Range = Range - n.Range;
		result.Position = Position - n.Position;
		result.Diffuse = Diffuse - n.Diffuse;
		result.Intensity = Intensity - n.Intensity;
		return result;
	}
};

//-----------------------------------------------------------------------------
/// The Light class
//
/// You can subclass from this to make more advanced lights (like lights with flares)
/// This class provides basic lighting and light animation capabilities
//-----------------------------------------------------------------------------
class ENGINE_API Light : public Actor {
	inline virtual const char* ClassName()
	{
		if(!IsScriptActor()) 
			return "Light"; 
		else  
			return m_managedClassName.c_str();
	}
protected:
	friend class BatchRenderer;
	friend class Shader;
	/// Use Light actor Location instead
	bool				IgnoreKeyframePositions; 

	bool				m_bDynamic;
	/// So we can detect a static light becoming dynamic
	Vector				m_OldLocation;
	/// So we can detect a static light becoming dynamic
	float				m_OldRange;    
	/// Internal Light object - don't modify.
	LightState			curState; 
	/// Animation frames for light
	vector<LightState>	keyframes; 
    /// Max length of keyframe animations
	float				keyframeTime;
    /// Current time in animation
	float				curTime;
	/// loops keyframes
	bool				m_KeyFrameLooping;
	/// play the keyframes
	bool				m_PlaysKeyFrames; 
	BBox				box;
	Vector              m_UpVec;
    /// Inits default state
	void				Initialize();

public:
    /// Shadow casting geometry
    vector<Model*>   m_ShadowMapItems;


	/// State vars - Only alter these if you want to build a special light dynamically

	/// Dynamic Shadow map
	class ShadowMap*	m_ShadowMap;		
	/// Projection map for spotlight
	Texture				m_tProjectionMap;	
	/// Cube map for omni projector
	Texture				m_tOmniProjCubeMap;	
	/// Blurred cube map for omni projector
	Texture				m_tOmniProjCubeMapBlur;	
	/// Light is subtractive shadow projector
	bool				m_bShadowProjector; 
	/// Shadow map texture size
	int m_ShadowMapSize;
	/// Type of light source
	LightType			m_Type;             
	/// Lighting method used, i.e. Regular or SH
	LightMethod			m_Method;			

	/// Handles to speed up include/exclude
	//vector<Model*>		m_ExcludeListHandles;

	vector<Actor*>		m_ExcludeListHandles;

	/// Special case that dynamic lights want to exclude the entire static tree, in addition to excluding other prefabs
	bool				m_ExcludeStaticTree; 
	/// always multipasses, even if an SH light on a single-pass mesh
	bool				m_ForceSHMultiPass; 

	// previous size of the inclusion/exclusion list, to dynamically update inclusions/exclusions if new mesh names added to the list
	int m_ExclusionListPreviousSize;

public:
	virtual void Clone(Actor* dest, bool bDeep = true);

	virtual bool IsLight(){ return true; }
	void	IgnoreKeyframes(){ IgnoreKeyframePositions = true; }
	LightState& GetCurrentState(){ return curState; }
	
	float GetKeyFrameTime(){ return keyframeTime; }
	///
	void AddKeyframe(const LightState& light, float TimeMS,bool loopKeyFrames = true);
	///
	void SetColor(FloatColor& col);
	///
	void SetRange(float Range);
	/// Resets keyframe animation to time = 0
	void ResetAnimation(){ m_PlaysKeyFrames = true; curTime = 0; }
	void SetPlaysKeyFrames(bool plays){ m_PlaysKeyFrames = plays; }
	int  GetNumKeyFrames(){return keyframes.size();}

	/// For spotlights
	Matrix GetProjection(); 
	/// For spotlights
	Matrix GetView();		
	Matrix GetViewProjection(){ return GetView()*GetProjection(); }
	//bool IsExcluded(Model* model);
	bool IsExcluded(Actor* actorIn);
	
	void MarkLitItems();
	void MarkStaticItems();
	virtual void Tick();
	bool IsDynamic(){ return m_bDynamic; }
	void SetDynamic(bool dynamic){m_bDynamic = dynamic;}
	/// Do not call this unless you know what the fuck you are doing
	void _MakeStatic(){ m_bDynamic = false; } 

	/// Doesn't initialize any light state, use AddKeyframe()
	Light(World* world);

	/// alternate ctor used by Script Actors that doesn't add the base class to managed table, to avoid double-managing them
	Light(World* world, bool IsScriptActor);

	/// Initializes a light state, you can add more (non-position) keyframes after
	Light(World* world, const FloatColor& col, float range);
	virtual ~Light();
	void SetUpVector(Vector& up){m_UpVec = up;}

	bool HasExclusionList();
	/// Whether the Light is actually visible for rendering or not
	/// For instance, if it's IsHidden or Intensity == 0 it will not be visible and hence not rendered
	bool IsVisible();

	void AddExclusionListHandle(Actor* handle)
	{
			if(!handle)
				return;

			std::vector<Actor*>::iterator it = std::find(m_ExcludeListHandles.begin(),m_ExcludeListHandles.end(),handle); 
			if(it == m_ExcludeListHandles.end())
				m_ExcludeListHandles.push_back(handle);	
	}
		
	void RemoveExclusionListHandle(Actor* handle)
	{
			if(!handle)
				return;

			std::vector<Actor*>::iterator it = std::find(m_ExcludeListHandles.begin(),m_ExcludeListHandles.end(),handle); 
			if(it != m_ExcludeListHandles.end())
				m_ExcludeListHandles.erase(it);
	}
	void ClearExclusionListHandles()
	{
		m_ExcludeListHandles.clear();
	}
};

