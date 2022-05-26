//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
/// Embodies water, with scene refraction and reflection
///
/// Author: Tim Johnson
///
/// Based on http://graphics.cs.lth.se/theses/projects/projgrid/projgrid-hq.pdf
//====================================================================================

/// Custom flag tagged to reflected models, allows us to quickly check if they should be rendered
/// without looping through a list
const static int FLAG_REFLECTED = (1<<5);

//--------------------------------------------------------------------------------------
/// Embodies water, with scene refraction and reflection
///
//--------------------------------------------------------------------------------------
class ENGINE_API WaterSurface  : public Actor
{
protected:
	/// Water grid vertices along x and y
	int m_GridVertsX, m_GridVertsY;

    /// Max tide height
    float MaxTideHeight;
	/// Reflect geometry?
	bool m_bReflect;
	/// Refract geometry?
	bool m_bRefract;
	/// List of models for reflection/refraction
	vector<Model*> m_ExcludeListHandles;
    /// Current active instance
    static WaterSurface* instance;
public:
	CLASS_NAME(WaterSurface);

    static WaterSurface* Instance(){ return instance; }

	virtual void DrawRenderTarget(Camera* cam){Render(cam);}
	
	/// Predefined preset index, can change in editor
	int  m_CurPreset, m_LastPreset;
	/// Previous grid size, so we know when to update if modified in editor
	int  m_LastGridX;
	/// Initialized?
	bool bInitialized;
	/// New texture size, for editor modifications
	int m_NewTextureSize;

	virtual ~WaterSurface(){ instance = 0; }
	WaterSurface(World* world);
	int GetGridX(){ return m_GridVertsX; }
	int GetGridY(){ return m_GridVertsY; }

	/// Grid surface
	class Surface* surf;
	/// Refraction
	void RenderRefractedScene(Camera* cam);
	/// Reflection
	void RenderReflectedScene(Camera* cam);
	/// Render everything
	void Render(Camera* cam);
	/// Initialize the water
	bool Initialize(int gridSize, int texSize);
	/// Load a preset water configuration
	void LoadPresent(int n);
    ///
    virtual void Tick();

};
