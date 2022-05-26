/*********************************************************************NVMH4****
Path:  NVSDK\Common\include\nv_renderdevice
File:  nvrenderdevice_d3d.h

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

d3d renderdevice implementation



******************************************************************************/

#ifndef __NVRENDERDEVICE_D3D_H
#define __NVRENDERDEVICE_D3D_H

#include <D3D8.h>
#include "d3dx8.h"
#include "Dxerr8.h"

#pragma comment(lib,"d3dx8.lib")
#pragma comment(lib,"dxerr8.lib")

namespace nv_renderdevice
{

//-----------------------------------------------------------------------------
// Error codes
//-----------------------------------------------------------------------------
enum APPMSGTYPE { MSG_NONE, MSGERR_APPMUSTEXIT, MSGWARN_SWITCHEDTOREF };

#define D3DAPPERR_NODIRECT3D          0x82000001
#define D3DAPPERR_NOWINDOW            0x82000002
#define D3DAPPERR_NOCOMPATIBLEDEVICES 0x82000003
#define D3DAPPERR_NOWINDOWABLEDEVICES 0x82000004
#define D3DAPPERR_NOHARDWAREDEVICE    0x82000005
#define D3DAPPERR_HALNOTCOMPATIBLE    0x82000006
#define D3DAPPERR_NOWINDOWEDHAL       0x82000007
#define D3DAPPERR_NODESKTOPHAL        0x82000008
#define D3DAPPERR_NOHALTHISMODE       0x82000009
#define D3DAPPERR_NONZEROREFCOUNT     0x8200000a
#define D3DAPPERR_MEDIANOTFOUND       0x8200000b
#define D3DAPPERR_RESIZEFAILED        0x8200000c
#define D3DAPPERR_NULLREFDEVICE       0x8200000d



//-----------------------------------------------------------------------------
// Name: struct D3DModeInfo
// Desc: Structure for holding information about a display mode
//-----------------------------------------------------------------------------
struct D3DModeInfo
{
    DWORD      Width;      // Screen width in this mode
    DWORD      Height;     // Screen height in this mode
    D3DFORMAT  Format;     // Pixel format in this mode
    DWORD      dwBehavior; // Hardware / Software / Mixed vertex processing
    D3DFORMAT  DepthStencilFormat; // Which depth/stencil format to use with this mode
};




//-----------------------------------------------------------------------------
// Name: struct D3DDeviceInfo
// Desc: Structure for holding information about a Direct3D device, including
//       a list of modes compatible with this device
//-----------------------------------------------------------------------------
struct D3DDeviceInfo
{
    // Device data
    D3DDEVTYPE   DeviceType;      // Reference, HAL, etc.
    D3DCAPS8     d3dCaps;         // Capabilities of this device
    const TCHAR* strDesc;         // Name of this device
    BOOL         bCanDoWindowed;  // Whether this device can work in windowed mode

    // Modes for this device
    DWORD        dwNumModes;
    D3DModeInfo  modes[150];

    // Current state
    DWORD        dwCurrentMode;
    BOOL         bWindowed;
    D3DMULTISAMPLE_TYPE MultiSampleTypeWindowed;
    D3DMULTISAMPLE_TYPE MultiSampleTypeFullscreen;
};




//-----------------------------------------------------------------------------
// Name: struct D3DAdapterInfo
// Desc: Structure for holding information about an adapter, including a list
//       of devices available on this adapter
//-----------------------------------------------------------------------------
struct D3DAdapterInfo
{
    // Adapter data
    D3DADAPTER_IDENTIFIER8 d3dAdapterIdentifier;
    D3DDISPLAYMODE d3ddmDesktop;      // Desktop display mode for this adapter

    // Devices for this adapter
    DWORD          dwNumDevices;
    D3DDeviceInfo  devices[5];

    // Current state
    DWORD          dwCurrentDevice;
};



//-----------------------------------------------------------------------------
// Name: class CD3DApplication
// Desc: A base class for creating sample D3D8 applications. To create a simple
//       Direct3D application, simply derive this class into a class (such as
//       class CMyD3DApplication) and override the following functions, as 
//       needed:
//          OneTimeSceneInit()    - To initialize app data (alloc mem, etc.)
//          InitDeviceObjects()   - To initialize the 3D scene objects
//          FrameMove()           - To animate the scene
//          Render()              - To render the scene
//          DeleteDeviceObjects() - To cleanup the 3D scene objects
//          FinalCleanup()        - To cleanup app data (for exitting the app)
//          MsgProc()             - To handle Windows messages
//-----------------------------------------------------------------------------
class NVRenderDevice_D3D8 : public INVRenderDevice_D3D8
{
public:
	static bool INTCALLTYPE CreateNVObject(const nv_sys::INVCreator* pCreator, const nv_sys::NVGUID& InterfaceID, void** ppObj);

    // Functions to create, run, pause, and clean up the application
	// INVObject
	virtual unsigned long INTCALLTYPE AddRef();
	virtual unsigned long INTCALLTYPE Release();
	virtual bool INTCALLTYPE GetInterface(const nv_sys::NVGUID& guid, void** ppObj);

	// INVRenderDevice_D3D
	virtual bool INTCALLTYPE Initialize(HWND hWnd);
	virtual bool INTCALLTYPE InitializeByDeviceHandle(DWORD DeviceHandle);
	virtual bool INTCALLTYPE UnInitialize();
	virtual bool INTCALLTYPE Resize();
	virtual bool INTCALLTYPE SetFocus(bool bHasFocus);
	virtual bool INTCALLTYPE CheckDevice();
	virtual bool INTCALLTYPE Present();
	virtual bool INTCALLTYPE Clear(DWORD Count, const RECT* pRects, DWORD Flags, DWORD Color, float Z, DWORD Stencil);
	virtual bool INTCALLTYPE CreateRenderVertices(UINT Length, DWORD Flags, INVRenderVertices** ppVerts);
	virtual bool INTCALLTYPE CreateRenderIndices(UINT Length, DWORD Flags, INVRenderIndices** ppIndices);
	virtual bool INTCALLTYPE SetRenderVertices(INVRenderVertices* pVertices, UINT Stride);
	virtual bool INTCALLTYPE SetRenderIndices(INVRenderIndices* pIndices,UINT BaseVertexIndex);
	virtual bool INTCALLTYPE DrawIndexedPrimitive(NVPRIMITIVETYPE Primitive, UINT minIndex, UINT NumVertices, UINT startIndex, UINT primCount);
	virtual bool INTCALLTYPE DrawPrimitive(NVPRIMITIVETYPE Primitive,UINT StartVertex,UINT PrimitiveCount);
	virtual bool INTCALLTYPE BeginScene();
	virtual bool INTCALLTYPE EndScene();
	virtual bool INTCALLTYPE GetDeviceCaps(NVDEVICECAPS* pCaps);
	virtual bool INTCALLTYPE SetTransform(NVTRANSFORMTYPE Transform, const mat4* pMatrix);
	virtual bool INTCALLTYPE SetRenderState(NVRENDERSTATETYPE RenderState, DWORD Value);
	virtual bool INTCALLTYPE LoadTexture(const char* pszFilePath, const NVTEXTURETARGETTYPE& TextureTarget, INVTexture** pTexture);
	virtual bool INTCALLTYPE CreateTexture(UINT Width, UINT Height, UINT MipMaps, DWORD Flags, NVTEXTURETARGETTYPE TargetType, NVTEXTUREFORMATTYPE FormatType, INVTexture** ppTex);
	virtual bool INTCALLTYPE CreateTextureFromHandle(NVTEX_HANDLE Handle, INVTexture** ppTexture);
	virtual bool INTCALLTYPE GetDeviceHandle(UINT_PTR* pdwHandle);
	virtual bool INTCALLTYPE ReleaseDeviceHandle(UINT_PTR pdwHandle);
	virtual bool INTCALLTYPE CreateRenderTexture(UINT Width, UINT Height, UINT Levels, 
		NVTEXTUREFORMATTYPE Format, void** ppRenderTargetColor, void** ppRenderTargetDepth, INVTexture** ppRenderTexture);
	virtual bool INTCALLTYPE SetRenderTarget(void* pRenderTargetColor, void* pRenderTargetDepth);
	virtual bool INTCALLTYPE RestoreRenderTarget();
	virtual bool INTCALLTYPE ToggleFullscreen(HWND hWnd) {return false;}

	// INVRenderDevice_D3D
	virtual bool INTCALLTYPE GetDevice(IDirect3DDevice8** pDevice) 
	{
		*pDevice = m_pd3dDevice;
		if (m_pd3dDevice)
			m_pd3dDevice->AddRef();
		return true; 
	}

private:
    // Internal functions to manage and render the 3D scene
    HRESULT BuildDeviceList();
    BOOL    FindDepthStencilFormat( UINT iAdapter, D3DDEVTYPE DeviceType,
                D3DFORMAT TargetFormat, D3DFORMAT* pDepthStencilFormat );
	LPDIRECT3DTEXTURE8 LoadDDSTexture2D(const char* pszFilePath);
	LPDIRECT3DVOLUMETEXTURE8 LoadDDSTexture3D(const char* pszFilePath);
	LPDIRECT3DCUBETEXTURE8 LoadDDSTextureCube(const char* pszFilePath);
	HRESULT Initialize3DEnvironment();

	DWORD m_dwRefCount;

    // Internal variables for the state of the app
    D3DAdapterInfo    m_Adapters[10];
    DWORD             m_dwNumAdapters;
    DWORD             m_dwAdapter;
    BOOL              m_bActive;
    BOOL              m_bReady;
    BOOL              m_bHasFocus;
	bool			  m_bOwnDevice;

    // Main objects used for creating and rendering the 3D scene
    D3DPRESENT_PARAMETERS m_d3dpp;         // Parameters for CreateDevice/Reset
    HWND              m_hWnd;              // The main app window
    LPDIRECT3D8       m_pD3D;              // The main D3D object
    LPDIRECT3DDEVICE8 m_pd3dDevice;        // The D3D rendering device
    D3DCAPS8          m_d3dCaps;           // Caps for the device
    D3DSURFACE_DESC   m_d3dsdBackBuffer;   // Surface desc of the backbuffer
    DWORD             m_dwCreateFlags;     // Indicate sw or hw vertex processing
	RECT			  m_rcWindowBounds;
	RECT			  m_rcWindowClient;

	// Overridable variables for the app
    BOOL              m_bUseDepthBuffer;   // Whether to autocreate depthbuffer
    DWORD             m_dwMinDepthBits;    // Minimum number of bits needed in depth buffer
    DWORD             m_dwMinStencilBits;  // Minimum number of bits needed in stencil buffer
	DWORD			  m_dwVertexShader;		// Vertex shader.

	mat4			 m_ViewMatrix;
	mat4			 m_WorldMatrix;
	mat4			 m_ProjectionMatrix;

	bool			m_bSWVP;
	bool			m_bRefRast;

	IDirect3DSurface8* m_backBufferColor;
	IDirect3DSurface8* m_backBufferDepth;
    // Internal constructor
    NVRenderDevice_D3D8(bool bRef);
};

}; //namespace nv_renderdevice

#endif // NVRENDERDEVICE_D3D_H