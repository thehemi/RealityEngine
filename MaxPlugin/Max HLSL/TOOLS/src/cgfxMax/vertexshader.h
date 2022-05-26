/*********************************************************************NVMH4****
NVSDK not found!
Path:  
File:  vertexshader.h

Copyright NVIDIA Corporation 2002
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED
*AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS
BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES
WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS,
BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS)
ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS
BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.



Comments:

See vertexshader.cpp

******************************************************************************/


#ifndef __VERTEXSHADER_H
#define __VERTEXSHADER_H
 
#include "Lighting.h"
#include "shaderinfo.h"

#define	MAX_MESH_CACHE	500

class IDX9VertexShaderCache : public VertexShaderCache
{
public:
		
};

typedef std::map<std::string, LPDIRECT3DTEXTURE9> tmapFileTexture;

// Per shader vertex shader data
typedef struct tagVS_ShaderData
{
	Lighting*				m_pLighting;
} VS_ShaderData;

typedef std::map<ShaderInfoData*, VS_ShaderData> tmapShaderInfoDataVS_ShaderData;

class MaxVertexShader : public IDX9VertexShader, public IStdDualVSCallback
{

public:


	MaxVertexShader(ReferenceTarget *rtarg);
	~MaxVertexShader();

	// Loading effect...	
	bool                    SetupConnectionParameters();

	HRESULT					Initialize(Mesh *mesh, INode *node);
	HRESULT					Initialize(MNMesh *mnmesh, INode *node);
	// From FPInterface
	LifetimeType			LifetimeControl() { return noRelease; }

	// From IVertexShaders
	HRESULT					ConfirmDevice(ID3D9GraphicsWindow *d3dgw);
	HRESULT					ConfirmPixelShader(IDX9PixelShader *pps);
	bool					CanTryStrips();
	int						GetNumMultiPass();
	LPDIRECT3DVERTEXSHADER9	GetVertexShaderHandle(int numPass) { return 0; }
	HRESULT					SetVertexShader(ID3D9GraphicsWindow *d3dgw, int numPass);
	
	// Draw 3D mesh as TriStrips
	bool					DrawMeshStrips(ID3D9GraphicsWindow *d3dgw, MeshData *data);

	// Draw 3D mesh as wireframe
	bool					DrawWireMesh(ID3D9GraphicsWindow *d3dgw, WireMeshData *data);

	// Draw 3D lines
	void					StartLines(ID3D9GraphicsWindow *d3dgw, WireMeshData *data);
	void					AddLine(ID3D9GraphicsWindow *d3dgw, DWORD *vert, int vis);
	bool					DrawLines(ID3D9GraphicsWindow *d3dgw);
	void					EndLines(ID3D9GraphicsWindow *d3dgw, GFX_ESCAPE_FN fn);

	// Draw 3D triangles
	void					StartTriangles(ID3D9GraphicsWindow *d3dgw, MeshFaceData *data);
	void					AddTriangle(ID3D9GraphicsWindow *d3dgw, DWORD index, int *edgeVis);
	bool					DrawTriangles(ID3D9GraphicsWindow *d3dgw);
	void					EndTriangles(ID3D9GraphicsWindow *d3dgw, GFX_ESCAPE_FN fn);

	// from IStdDualVSCallback
	virtual					ReferenceTarget *GetRefTarg();
	virtual					VertexShaderCache *CreateVertexShaderCache();
	virtual HRESULT			InitValid(Mesh* mesh, INode *node);
	virtual HRESULT			InitValid(MNMesh* mnmesh, INode *node);

	virtual LPD3DXEFFECT	LoadEffect();

	virtual void			Draw();
	HRESULT			        SetupMatrices();
	HRESULT			        SetupMaterials();

	bool					IsEditableParam(nv_sys::INVConnectionParameter* pParam);

	nv_renderdevice::INVTexture* LoadTexture(const char* pszFilePath, const nv_renderdevice::NVTEXTURETARGETTYPE& TargetType);
	nv_renderdevice::INVTexture* LoadTexture(const char* pszFilePath, nv_sys::INVConnectionParameter* pTexture);
	nv_renderdevice::INVTexture* LoadBumpTexture(const char* pszFilePath, nv_sys::INVConnectionParameter* pTexture);
	
	LPDIRECT3DTEXTURE9 MaxVertexShader::LoadDDSTexture2D(const char* pszFilePath);
	LPDIRECT3DCUBETEXTURE9 MaxVertexShader::LoadDDSTextureCube(const char* pszFilePath);
	LPDIRECT3DVOLUMETEXTURE9 MaxVertexShader::LoadDDSTexture3D(const char* pszFilePath);

	int						FreeTexture(nv_renderdevice::INVTexture* pTexture);
	bool					FreeTextures();
	bool					AddRefEffectTextures();

	void					SetDefaultMaxState();
	
	int				        GetMatIndex();
	void			        SetMeshCache(Mesh *mesh, INode *node, TimeValue T);
	HRESULT			        InternalCheck();
	Lighting*		        GetLighting();
	LPDIRECT3DDEVICE9       GetDevice();
	void			        SetDevice(LPDIRECT3DDEVICE9 pDevice);
	TimeValue		        GetTime() const;
	void			        SetTime(TimeValue T);

	int				        m_MaxCache;
	int				        m_CurCache;
	INode*			        m_Node[MAX_MESH_CACHE];
	Mesh*			        m_Mesh[MAX_MESH_CACHE];
	RenderMesh		        m_RMesh[MAX_MESH_CACHE];

	VS_ShaderData&			GetVSShaderData();
	
	// Effects
	bool					RemoveEffect();
	bool			        SetupEffect();
    bool                    SetTechnique(unsigned int TechniqueNum);
	bool                    SetTechnique(const char* TechniqueName);

    CgFXMaterial*           GetMaterial();
    const mat4&             GetView() const         { return m_matView; }
    const mat4&             GetWorld() const        { return m_matWorld; }
    const mat4&             GetProjection() const   { return m_matProjection; }
    DWORD					GetThreadID() const { return m_ThreadID; }

    bool                    m_Reload;
    void                    SetReload(bool bReload);

    HANDLE                  m_hEvent;

	bool					IsTimeDependant() { return m_bTimeDependant; }

    void                    StartWatching ();
    void                    StopWatching ();

	void					DirtyLights(bool bDirty) { m_bLightsDirty = bDirty; }
	bool					IsDirtyLights() const { return m_bLightsDirty; }

	CgFXMaterial*			m_pMaterial;
private:
	
	IStdDualVS*				m_StdDualVS;
	ReferenceTarget*		m_RTarget;
	LPDIRECT3DDEVICE9		m_pDevice;
	bool					m_Draw;
	Mtl*					m_Mtl;
	ID3D9GraphicsWindow*	m_GWindow;
	bool					m_Ready;
	TimeValue				m_T;
	bool					badDevice;
	nv_fx::NVFXHANDLE		m_hTechnique;
	mat4					m_matWorld;
	mat4					m_matView;
	mat4					m_matProjection;
	mat4					m_matWorldView;
	mat4					m_matWorldViewProjection;
	tmapFileTexture			m_mapFileTexture;
	bool					m_bTimeDependant; // Is the current setup changeable with time?
	bool					m_bLightsDirty;	
    DWORD                   m_ThreadID;
    HANDLE                  m_hWatchThread;
};

#endif __VERTEXSHADER_H
