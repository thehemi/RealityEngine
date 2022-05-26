/*********************************************************************NVMH4****
Path:  NVSDK\Common\include\nv_renderdevice
File:  nvrenderdevice_gl.h

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

OpenGL renderdevice implementation



******************************************************************************/

#ifndef __NVRENDERDEVICE_GL_H
#define __NVRENDERDEVICE_GL_H

#include "GL/gl.h"
#include <GL/glext.h>
#include <GL/glu.h>
// use glh for easy extension
//#define GLH_EXT_SINGLE_FILE
//#include "glh_extensions.h"
//#include <glh_genext.h>
#include "nv_dds.h"

#include "nvrenderindices_gl.h"
#include "invrenderdevice_gl.h" 

#pragma comment (lib, "opengl32.lib")

namespace nv_renderdevice
{

class NVRenderDevice_GL : public INVRenderDevice_GL
{
public:
	static bool INTCALLTYPE CreateNVObject(const nv_sys::INVCreator* pCreator, const nv_sys::NVGUID& InterfaceID, void** ppObj);

	// INVObject
	virtual unsigned long INTCALLTYPE AddRef();
	virtual unsigned long INTCALLTYPE Release();
	virtual bool INTCALLTYPE GetInterface(const nv_sys::NVGUID& guid, void** ppObj);

    // Functions to create, run, pause, and clean up the application
	// INVRenderDevice
	virtual bool INTCALLTYPE Initialize(HWND hWnd);
	virtual bool INTCALLTYPE InitializeByDeviceHandle(DWORD DeviceHandle);
	virtual bool INTCALLTYPE UnInitialize();
	virtual bool INTCALLTYPE Resize();
	virtual bool INTCALLTYPE SetFocus(bool bHasFocus);
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
	virtual bool INTCALLTYPE LoadTexture(const char* pszFilePath, const NVTEXTURETARGETTYPE& TargetType, INVTexture** ppTex);
	virtual bool INTCALLTYPE GetDeviceHandle(UINT_PTR* pdwHandle);
	virtual bool INTCALLTYPE ReleaseDeviceHandle(UINT_PTR pdwHandle);
	virtual bool INTCALLTYPE CreateTexture(UINT Width, UINT Height, UINT MipMaps, DWORD Flags, NVTEXTURETARGETTYPE TargetType, NVTEXTUREFORMATTYPE FormatType, INVTexture** ppTex);
	virtual bool INTCALLTYPE CreateTextureFromHandle(NVTEX_HANDLE Handle, INVTexture** ppTexture);
	virtual bool INTCALLTYPE CreateRenderTexture(UINT Width, UINT Height, UINT Levels, 
		NVTEXTUREFORMATTYPE Format, void** ppRenderTargetColor, void** ppRenderTargetDepth, INVTexture** ppRenderTexture);
	virtual bool INTCALLTYPE SetRenderTarget(void* pRenderTargetColor, void* pRenderTargetDepth);
	virtual bool INTCALLTYPE RestoreRenderTarget();
	virtual bool INTCALLTYPE ToggleFullscreen(HWND hWnd) { return false;}

	// INVRenderDevice_GL
	virtual bool INTCALLTYPE GetPixelFormat(PIXELFORMATDESCRIPTOR* ppFD) { *ppFD = m_sPFD; return true;}
	virtual bool INTCALLTYPE GetDeviceContext(HGLRC* phglrc) { *phglrc = m_hglrc; return true;}

	static void (APIENTRY *nvOglMultiTexCoord3fv)(GLenum, const GLfloat*);
	static void (APIENTRY *nvOglMultiTexCoord4fv)(GLenum, const GLfloat*);
	static void (APIENTRY *nvOglClientActiveTexture)(GLenum);
	void* GetOpenGLFunctionPtr(const char* functionName);

private:
    // Internal functions to manage and render the 3D scene
	bool Initialize3DEnvironment();
	bool SetupPixelFormat(HDC dc);
	bool RecoverSwapFailure();

	DWORD m_dwRefCount;

    // Internal variables for the state of the app
    BOOL              m_bActive;
    BOOL              m_bReady;
    BOOL              m_bHasFocus;

    // Main objects used for creating and rendering the 3D scene
    HWND              m_hWnd;              // The main app window
	RECT			  m_rcWindowBounds;
	RECT			  m_rcWindowClient;

	mat4			 m_ViewMatrix;
	mat4			 m_WorldMatrix;
	mat4			 m_ProjectionMatrix;
	
	HDC							m_hDC;
	HGLRC                       m_hRC;
	int                         m_iPixelFormat;
	PIXELFORMATDESCRIPTOR		m_sPFD;
	HGLRC						m_hglrc;

	void* m_pVertexData;
	NVRenderIndicesDesc_GL		m_IndexDesc;
	unsigned int				m_BaseVertexIndex;

    // Internal constructor
    NVRenderDevice_GL();
};

}; // namespace nv_renderdevice
#endif // NVRENDERDEVICE_GL_H