//=========== Copyright (c) 2003, Tim Johnson. All rights reserved. ===========
//	RenderDevice encapsulates the system rendering device, all it's properties,
//	modes, settings, and capabitilies.
//=============================================================================
#pragma once


/// A screen resolution suppoted by a Device
struct DeviceMode {
	int SizeX, SizeY;
};

/// Glare form library
enum EGLARELIBTYPE
{
	GLT_DISABLE					= 0,

	GLT_CAMERA,
	GLT_NATURAL,
	GLT_CHEAPLENS,
	//GLT_AFTERIMAGE,
	GLT_FILTER_CROSSSCREEN,
	GLT_FILTER_CROSSSCREEN_SPECTRAL,
	GLT_FILTER_SNOWCROSS,
	GLT_FILTER_SNOWCROSS_SPECTRAL,
	GLT_FILTER_SUNNYCROSS,
	GLT_FILTER_SUNNYCROSS_SPECTRAL,
	GLT_CINECAM_VERTICALSLITS,
	GLT_CINECAM_HORIZONTALSLITS,

	NUM_GLARELIBTYPES,
	GLT_USERDEF					= -1,
	GLT_DEFAULT					= GLT_FILTER_CROSSSCREEN,
} ;

typedef void    (CALLBACK *LPFRAMERENDER)();
typedef void    (CALLBACK *LPSETTINGSUI)( class CDXUTDialog* pSettingsDialog );

//--------------------------------------------------------------------------------------
/// Abstract class. Derive from this to get callbacks from RenderDevice!
//--------------------------------------------------------------------------------------
class ENGINE_API RenderBase {
protected:
	friend class RenderDevice;
	RenderBase();
	~RenderBase();
	//virtual HRESULT OnCreateDevice(){};
	virtual HRESULT OnResetDevice(){ return S_OK; };
	virtual void	OnLostDevice(){};
	virtual void	OnDestroyDevice(){};

};

//--------------------------------------------------------------------------------------
/// Occlusion query item. Managed by RenderDevice
//--------------------------------------------------------------------------------------
struct ENGINE_API OcclusionQuery 
{
	struct IDirect3DQuery9* query;
    /// Force an occlusion query to flush and return pixels visible
	DWORD GetPixels();
    /// Begin a query
	void Begin();
    /// End a query
	void End();
    /// Initialize this query
    void Create();
    /// Destroy it
    void Free();

    OcclusionQuery(){ query = 0; pending = false; }
    ~OcclusionQuery(){ Free(); }

    bool pending;
};


//--------------------------------------------------------------------------------------
/// \brief RenderDevice encapsulates the system rendering device, all its properties,
///	modes, settings, and capabitilies.
//--------------------------------------------------------------------------------------
class ENGINE_API RenderDevice {
protected:

	RenderDevice();
	RenderDevice(const RenderDevice&);
	RenderDevice& operator= (const RenderDevice&);
	~RenderDevice();
	
	friend class RenderWrap;
	friend class Canvas;

	/// Engine calls
	friend class Engine;
	/// MsgProc: Resize, Repaint, Mouse Movement (for FS cursor)
	LRESULT MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	void	Update(class Camera* ActiveCamera);
	bool	Initialize(HINSTANCE hInst, HWND hWnd, string cmdLine);
	void	Shutdown();

	/// Callbacks
	LPFRAMERENDER m_FrameRenderCallback;
	LPSETTINGSUI  m_SettingsUI;

	/// Render properties
	/// User's screen gamma. Needed if we alter in windowed mode
	WORD originalGamma[3][256]; 
	float	Gamma;
	float	Brightness;
	float	Contrast;
	bool	m_bCursorVisible;
	bool	m_bCenterCursor;
	bool	m_bDeviceLost;
    bool    m_b3DC;
	bool    UseSRGB;
	DWORD	ClearColor;
	bool	ClearScreen;
    bool    m_bCompressPRT;
	bool CompressScreenshots;
	bool	m_OcclusionTesting;
	/// 1 = Bi/Trilinear
	int	  AnisotropyLevel;
	DWORD D3DTextureFilter;

	bool dynamicLights;
	bool FFP;

    bool m_bShadows;



	/// Per-frame data
	friend class Mesh;
	int m_DrawCalls;
	int m_Triangles;
	int m_Vertices;

	void LoadConfig();
	void SaveConfig();

	vector<RenderBase*> m_Callbacks;

public:

    /// Unbinds all textures (sets all stages to 0)
    void    UnbindTextures();
    void    SetCompressPRTMaps(bool val){ m_bCompressPRT = val; }
    bool    CompressPRTMaps(){ return m_bCompressPRT; }
	/// Can device do target blending?
	bool	TargetSupportsBlending();
	void	RegisterCallback(RenderBase* base){ m_Callbacks.push_back(base); }
	void	RemoveCallback(RenderBase* base){ vector_erase(m_Callbacks,base); }

	/// Internal callbacks, ignore!
	HRESULT OnResetDevice( struct IDirect3DDevice9* pd3dDevice, const struct _D3DSURFACE_DESC* pBackBufferSurfaceDesc );
	void	RenderCallback();
	HRESULT FinalCleanup();
	void	OnLostDevice();
	void	OnDestroyDevice();


	bool	IsDeviceLost();


	void	SetDeviceStats();

	/// Config Preferences
	bool  m_bUseSWVP;
	bool  m_bUseVSync;
	bool  m_bIgnoreGamma;
	bool  m_bHardwareSync;
	float m_fAspect;
	float m_ShadowMapScale;

    /// Maximum viewport Z value (0,1)
    float MaxViewportZ;
    /// Minimum viewport Z value (0,1)
    float MinViewportZ;
    /// Updates viewport based on these values
    void UpdateViewport();

	vector<OcclusionQuery*> m_Queries;

	float	PixelShaderVersion;
	float	VertexShaderVersion;
	string  PixelShaderString;  /// ps_x_x
	string  VertexShaderString; /// vs_x_x

	bool	GetOcclusionTesting(){ return m_OcclusionTesting; }
	void	SetOcclusionTesting(bool on){ m_OcclusionTesting = on; }

    /// Shadows
    void    SetShadows(bool enable){ m_bShadows = enable; }
    bool    GetShadows(){ return m_bShadows; }
	void	SetDropShadows(bool enable);
	bool	GetDropShadows();

    /// 3DC compression?
    bool Supports3DC(){ return m_b3DC; }

	/// High-Dynamic Range
	void			SetHDRGlare(EGLARELIBTYPE glare);
	EGLARELIBTYPE	GetHDRGlare();
	void	SetHDR(bool enable);
	bool	GetHDR();
	bool	GetPRT(){return true;}
	void	SetHDRExposure(float exposure);
	float	GetHDRExposure();
	void	SetToneMapping(bool toneMap);
	void	SetBlueShift(bool blueShift);
	bool	GetBlueShift();
	void	SetBlueShiftCoefficient(float blueShift);
	float	GetBlueShiftCoefficient();
	bool	GetToneMapping();
	float	GetMinLuminance();
	float	GetMaxLuminance();
	void	SetMinLuminance(float min);
	void	SetMaxLuminance(float max);
	/// Flip our pair of ping-pong targets for hdr blending
	void	FlipHDRTargets();


	void	SetClearColor(COLOR clear){ ClearColor = clear; }
	COLOR	GetClearColor(){ return ClearColor; }
	void	SetClearScreen(bool clear){ ClearScreen = clear; }
	bool	GetClearScreen(){ return ClearScreen; }
	void	SetDynamicLights(bool dynamic){ dynamicLights = dynamic;}
	bool	GetDynamicLights(){ return dynamicLights;}
	/// DX9 SRGB 2.2 Gamma correction
	void	SetSRGB(bool sRGB);
	bool	GetSRGB(){ return UseSRGB; }
	/// Texture creation settings
	void	PreCacheLoadedTextures();
	bool	GetCompressNormalMaps();
	void	SetCompressNormalMaps(bool enable);
	int		GetTextureSizePercent();
	void	SetTextureSizePercent(int percent);
	/// Device settings
	void	RestoreGamma();
	void	SetGammaLevel(bool Linear=true, float contrast=1, float brightness=1, float gamma=1);
	float	GetGamma(){ return Gamma; }
	/// Tex settings
	void	SetAnisotropyLevel(int NewAnisotropyLevel); /// 1 = Trilinear
	int		GetAnisotropyLevel(){ return AnisotropyLevel; }
	DWORD	GetTexFilter(){ return D3DTextureFilter; }
	/// Viewport
	bool	GetVSync();
	int		GetViewportX();
	int		GetViewportY();
	int		GetViewportColorDepth();
	bool	IsFullscreen();
	bool	ReadyToRender();
	/// Stats
	int		TrisPerFrame();
	int		VertsPerFrame();
	int		MeshesPerFrame();
	int		DynamicLightsDrawn();
	int		CountUsedTextureMB();
	/// Misc
	void	ReloadShaders();
	void	TakeScreenshot();
	void	SaveThumbnail(const char* fileNameAndPath);

	void	SetWireframe(bool wireframe);
	bool	GetFFP(){ return FFP; }
    /// Sets all default D3DRS_ states
	void	SetDefaultStates();
    /// Sets ALL device states to defaults
    void    ResetAllStates();
	/// Toggle mouse cursor on/off
	void	ShowCursor(bool Show, bool CenterCursor);
	bool	IsCursorVisible();

	/// Singleton
	static RenderDevice* Instance();

	/// Callbacks
	void SetRenderCallback(LPFRAMERENDER frameRender){ m_FrameRenderCallback = frameRender; }
	void SetSettingsUICallback(LPSETTINGSUI settingsUI){ m_SettingsUI = settingsUI; }

	/// GUI
	class CD3DSettingsDlg* GetSettingsDialog();

	/// Called once per frame, triggers LPFRAMERENDER callback
	void	DoRendering();  
	HRESULT EndHDR();		/// Call when you want to do non-HDR rendering like text

	/// Get the drawing canvas for this device
	class Canvas* GetCanvas();

	string GetDeviceString(); /// e.g.: "HAL (pure hw vp) NVIDIA GeForce Ti 200"
	vector<DeviceMode> GetDeviceModes();
	bool SetDisplayMode(int NewSizeX, int NewSizeY, int NewColorDepth, bool NewFullScreen, bool NewVSync);
};


