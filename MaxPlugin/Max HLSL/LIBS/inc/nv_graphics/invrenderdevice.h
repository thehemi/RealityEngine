/*********************************************************************NVMH4****
Path:  NVSDK\Common\include\nv_renderdevice
File:  invrenderdevice.h

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

INVRenderDevice.  The base interface for communication with a rendering device
in an API independant way.  Allows the CgFX Viewer to draw with GL or D3D, swapping
on the fly between the two.



******************************************************************************/

#ifndef __INVRENDERDEVICE_H
#define __INVRENDERDEVICE_H

namespace nv_renderdevice
{

class INVRenderVertices;
class INVRenderIndices;
class INVTexture;

static const DWORD NVCREATEFLAG_STATIC = (1 << 0);
static const DWORD NVCREATEFLAG_DYNAMIC = (1 << 1);
static const DWORD NVCREATEFLAG_INDEX16 = (1 << 2);
static const DWORD NVCREATEFLAG_INDEX32 = (1 << 3);
static const DWORD NVCREATEFLAG_WRITEONLY = (1 << 4);
static const DWORD NVCREATEFLAG_SOFTWAREPROCESSING = (1 << 5);

static const DWORD NVCLEARFLAG_TARGET = (1 << 0);
static const DWORD NVCLEARFLAG_ZBUFFER = (1 << 1);

typedef UINT_PTR NVTEX_HANDLE;

// Temporary static vertex type
#define D3DFVF_XYZ              0x002
#define D3DFVF_NORMAL           0x010
#define D3DFVF_DIFFUSE          0x040
#define D3DFVF_SPECULAR         0x080
#define D3DFVF_TEX1             0x100
#define D3DFVF_TEX4				0x400
#define D3DFVF_TEX8             0x800

#define D3DFVF_TEXTUREFORMAT2 0         // Two floating point values
#define D3DFVF_TEXTUREFORMAT1 3         // One floating point value
#define D3DFVF_TEXTUREFORMAT3 1         // Three floating point values
#define D3DFVF_TEXTUREFORMAT4 2         // Four floating point values

#define D3DFVF_TEXCOORDSIZE3(CoordIndex) (D3DFVF_TEXTUREFORMAT3 << (CoordIndex*2 + 16))
#define D3DFVF_TEXCOORDSIZE2(CoordIndex) (D3DFVF_TEXTUREFORMAT2)
#define D3DFVF_TEXCOORDSIZE4(CoordIndex) (D3DFVF_TEXTUREFORMAT4 << (CoordIndex*2 + 16))
#define D3DFVF_TEXCOORDSIZE1(CoordIndex) (D3DFVF_TEXTUREFORMAT1 << (CoordIndex*2 + 16))


static const DWORD NVMAX_TEXCOORDS = 8;
typedef struct tagNVVertex
{
	vec3 Position;
	vec3 Normal;
	DWORD Diffuse;
	vec3 TexCoord[NVMAX_TEXCOORDS];
} NVVertex;
static const DWORD NVVERTEX_FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_NORMAL | D3DFVF_TEX8 | 
	D3DFVF_TEXCOORDSIZE3(0) | D3DFVF_TEXCOORDSIZE3(1) | D3DFVF_TEXCOORDSIZE3(2) | D3DFVF_TEXCOORDSIZE3(3) | 
	D3DFVF_TEXCOORDSIZE3(4) | D3DFVF_TEXCOORDSIZE3(5) | D3DFVF_TEXCOORDSIZE3(6) | D3DFVF_TEXCOORDSIZE3(7);
	
typedef struct tagNVLOCKED_RECT
{
	DWORD Pitch;
	void* pBits;
} NVLOCKED_RECT;

typedef enum NVDEVICETYPE
{
    NVDEVICETYPE_HAL         = 1,
    NVDEVICETYPE_REF         = 2,
    NVDEVICETYPE_SW          = 3,

    NVDEVICETYPE_FORCE_DWORD  = 0x7fffffff
} NVDEVICETYPE;

// Matches DX8 structure at the beginning.
typedef struct tagNVDEVICECAPS
{
    /* Device Info */
    NVDEVICETYPE  DeviceType;
    UINT    AdapterOrdinal;

    /* Caps from DX7 Draw */
    DWORD   Caps;
    DWORD   Caps2;
    DWORD   Caps3;
    DWORD   PresentationIntervals;

    /* Cursor Caps */
    DWORD   CursorCaps;

    /* 3D Device Caps */
    DWORD   DevCaps;

    DWORD   PrimitiveMiscCaps;
    DWORD   RasterCaps;
    DWORD   ZCmpCaps;
    DWORD   SrcBlendCaps;
    DWORD   DestBlendCaps;
    DWORD   AlphaCmpCaps;
    DWORD   ShadeCaps;
    DWORD   TextureCaps;
    DWORD   TextureFilterCaps;          // D3DPTFILTERCAPS for IDirect3DTexture8's
    DWORD   CubeTextureFilterCaps;      // D3DPTFILTERCAPS for IDirect3DCubeTexture8's
    DWORD   VolumeTextureFilterCaps;    // D3DPTFILTERCAPS for IDirect3DVolumeTexture8's
    DWORD   TextureAddressCaps;         // D3DPTADDRESSCAPS for IDirect3DTexture8's
    DWORD   VolumeTextureAddressCaps;   // D3DPTADDRESSCAPS for IDirect3DVolumeTexture8's

    DWORD   LineCaps;                   // D3DLINECAPS

    DWORD   MaxTextureWidth, MaxTextureHeight;
    DWORD   MaxVolumeExtent;

    DWORD   MaxTextureRepeat;
    DWORD   MaxTextureAspectRatio;
    DWORD   MaxAnisotropy;
    float   MaxVertexW;

    float   GuardBandLeft;
    float   GuardBandTop;
    float   GuardBandRight;
    float   GuardBandBottom;

    float   ExtentsAdjust;
    DWORD   StencilCaps;

    DWORD   FVFCaps;
    DWORD   TextureOpCaps;
    DWORD   MaxTextureBlendStages;
    DWORD   MaxSimultaneousTextures;

    DWORD   VertexProcessingCaps;
    DWORD   MaxActiveLights;
    DWORD   MaxUserClipPlanes;
    DWORD   MaxVertexBlendMatrices;
    DWORD   MaxVertexBlendMatrixIndex;

    float   MaxPointSize;

    DWORD   MaxPrimitiveCount;          // max number of primitives per DrawPrimitive call
    DWORD   MaxVertexIndex;
    DWORD   MaxStreams;
    DWORD   MaxStreamStride;            // max stride for SetStreamSource

    DWORD   VertexShaderVersion;
    DWORD   MaxVertexShaderConst;       // number of vertex shader constant registers

    DWORD   PixelShaderVersion;
    float   MaxPixelShaderValue;        // max value of pixel shader arithmetic component
} NVDEVICECAPS;

typedef enum tagNVPRIMITIVETYPE 
{
    NVPRIMITIVE_POINTLIST       = 1,
    NVPRIMITIVE_LINELIST        = 2,
    NVPRIMITIVE_LINESTRIP       = 3,
    NVPRIMITIVE_TRIANGLELIST    = 4,
    NVPRIMITIVE_TRIANGLESTRIP   = 5,
    NVPRIMITIVE_TRIANGLEFAN     = 6,
    NVPRIMITIVE_FORCE_DWORD     = 0x7fffffff, /* force 32-bit size enum */
} NVPRIMITIVETYPE;

typedef enum tagNVTRANSFORMTYPE
{
	NVTRANSFORM_WORLD = 1,
	NVTRANSFORM_VIEW = 2,
	NVTRANSFORM_PROJECTION = 3,
	NVTRANSFORM_FORCE_DWORD = 0x7fffffff
} NVTRANSFORMTYPE;

typedef enum tagNVFILLMODE
{
	NVFILL_SOLID = 1,
	NVFILL_WIREFRAME = 2,
	NVFILL_POINT = 3,
	NVFILL_FORCE_DWORD = 0x7fffffff
} NVFILLMODE;

typedef enum tagNVCULLMODE
{
	NVCULL_CW = 1,
	NVCULL_CCW = 2,
	NVCULL_NONE = 3,
	NVCULL_FORCE_DWORD = 0x7fffffff
} NVCULLMODE;

typedef enum tagNVRENDERSTATETYPE
{ 
	NVRS_FILLMODE = 1,
	NVRS_CULLMODE = 2,
	NVRS_FORCE_DWORD = 0x7fffffff
} NVRENDERSTATETYPE;

typedef enum tagNVTEXTURETARGETTYPE
{
	NVTEXTURE_UNKNOWN = 0,
	NVTEXTURE_1D = 1,
	NVTEXTURE_2D = 2,
	NVTEXTURE_CUBE = 3,
	NVTEXTURE_RECT = 4,
	NVTEXTURE_3D = 5
} NVTEXTURETARGETTYPE;

typedef enum tagNVTEXTUREFORMATTYPE
{
	NVR8G8B8 = 1,
	NVA8R8G8B8 = 2,
	NVX8R8G8B8 = 3,
	NVA4R4G4B4 = 4
} NVTEXTUREFORMATTYPE;


class INVRenderDevice : public nv_sys::INVObject
{
public:
	// INVObject
	virtual unsigned long INTCALLTYPE AddRef() = 0;
	virtual unsigned long INTCALLTYPE Release() = 0;
	virtual bool INTCALLTYPE GetInterface(const nv_sys::NVGUID& guid, void** ppObj) = 0;

	// INVRenderDevice
	virtual bool INTCALLTYPE Initialize(HWND hWnd) = 0;
	virtual bool INTCALLTYPE InitializeByDeviceHandle(DWORD DeviceHandle) = 0;
	virtual bool INTCALLTYPE UnInitialize() = 0;
	virtual bool INTCALLTYPE Resize() = 0;
	virtual bool INTCALLTYPE SetFocus(bool bHasFocus) = 0;
	virtual bool INTCALLTYPE Present() = 0;
	virtual bool INTCALLTYPE Clear(DWORD Count, const RECT* pRects, DWORD Flags, DWORD Color, float Z, DWORD Stencil) = 0;
	virtual bool INTCALLTYPE CreateRenderVertices(UINT Length, DWORD Flags, INVRenderVertices** ppVerts) = 0;
	virtual bool INTCALLTYPE CreateRenderIndices(UINT Length, DWORD Flags, INVRenderIndices** ppIndices) = 0;
    virtual bool INTCALLTYPE SetRenderVertices(INVRenderVertices* pVertices, UINT Stride) = 0;
	virtual bool INTCALLTYPE SetRenderIndices(INVRenderIndices* pIndices, UINT BaseVertexIndex) = 0;
	virtual bool INTCALLTYPE DrawIndexedPrimitive(NVPRIMITIVETYPE Primitive, UINT minIndex, UINT NumVertices, UINT startIndex, UINT primCount) = 0;
	virtual bool INTCALLTYPE DrawPrimitive(NVPRIMITIVETYPE Primitive,UINT StartVertex,UINT PrimitiveCount) = 0;
	virtual bool INTCALLTYPE LoadTexture(const char* pszFilePath, const NVTEXTURETARGETTYPE& TargetType, INVTexture** ppTex) = 0;
	virtual bool INTCALLTYPE GetDeviceHandle(UINT_PTR* pdwHandle) = 0;
	virtual bool INTCALLTYPE ReleaseDeviceHandle(UINT_PTR dwHandle) = 0;
	virtual bool INTCALLTYPE BeginScene() = 0;
	virtual bool INTCALLTYPE EndScene() = 0;
	virtual bool INTCALLTYPE GetDeviceCaps(NVDEVICECAPS* pCaps) = 0;
	virtual bool INTCALLTYPE CreateRenderTexture(UINT Width, UINT Height, UINT Levels, NVTEXTUREFORMATTYPE Format, 
		void** ppRenderTargetColor, void** ppRenderTargetDepth, INVTexture** ppRenderTexture) = 0;
	virtual bool INTCALLTYPE CreateTexture(UINT Width, UINT Height, UINT MipMaps, DWORD Flags, NVTEXTURETARGETTYPE TargetType, NVTEXTUREFORMATTYPE FormatType, INVTexture** ppTex) = 0;
	virtual bool INTCALLTYPE CreateTextureFromHandle(NVTEX_HANDLE Handle, INVTexture** ppTexture) = 0;
	virtual bool INTCALLTYPE SetRenderTarget(void* pRenderTargetColor, void* pRenderTargetDepth) = 0;
	virtual bool INTCALLTYPE RestoreRenderTarget() = 0;
	virtual bool INTCALLTYPE ToggleFullscreen(HWND hWnd) = 0;

	// Temp, until .fx support is in.
	virtual bool INTCALLTYPE SetTransform(NVTRANSFORMTYPE Transform, const mat4* pMatrix) = 0;
	virtual bool INTCALLTYPE SetRenderState(NVRENDERSTATETYPE RenderState, DWORD Value) = 0;
};

}; // namespace nv_renderdevice

// {77F3232A-2AAD-4959-9E3E-77E24E574464}
static const nv_sys::NVGUID IID_INVRenderDevice = 
{ 0x77f3232a, 0x2aad, 0x4959, { 0x9e, 0x3e, 0x77, 0xe2, 0x4e, 0x57, 0x44, 0x64 } };

#endif // __INVRENDERDEVICE_H
