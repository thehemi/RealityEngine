/*********************************************************************NVMH4****
Path:  NVSDK\Common\include\nv_fx
File:  inveffects.h

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


******************************************************************************/

#ifndef __INVEFFECTS_H
#define __INVEFFECTS_H

namespace nv_fx
{

#define NVFX_MAX_DIMENSIONS 4

// Types	
typedef enum _NVFXPARAMETERTYPE
{
	NVFXPT_UNKNOWN			= 0,
	NVFXPT_BOOL,
	NVFXPT_INT,
    NVFXPT_FLOAT,
    NVFXPT_DWORD,
    NVFXPT_STRING,

    // Ref counted objects; Can be accessed as objects or as strings
    NVFXPT_TEXTURE,
    NVFXPT_CUBETEXTURE,
    NVFXPT_VOLUMETEXTURE,
    NVFXPT_VERTEXSHADER,
    NVFXPT_PIXELSHADER,

	NVFXPT_SAMPLER,
	NVFXPT_SAMPLER1D,
	NVFXPT_SAMPLER2D,
	NVFXPT_SAMPLER3D,
	NVFXPT_SAMPLERCUBE,
	NVFXPT_PIXELFRAGMENT,
	NVFXPT_VERTEXFRAGMENT,
	
    // Complex type
    NVFXPT_STRUCT,
    
    // force 32-bit size enum
    NVFXPT_FORCE_DWORD      = 0x7fffffff

} NVFXPARAMETERTYPE;

typedef struct _NVFXEFFECT_DESC
{
	UINT Parameters;	// Number of parameters
	UINT Techniques;	// Number of techniques
	UINT Functions;		// Number of functions
} NVFXEFFECT_DESC;

typedef struct _NVFXPARAMETER_DESC
{
	LPCSTR Name;		// Parameter name.
	LPCSTR Index;		// Parameter index (cast to LPCSTR)

	// Usage
	LPCSTR Semantic;	// Semantic meaning

	// Type
	NVFXPARAMETERTYPE Type;	// Parameter type
	UINT Dimension[NVFX_MAX_DIMENSIONS];		// Elements in array
	UINT Bytes;				// Total size in bytes

	// Annotations
	UINT Annotations;		// Number of annotations.
} NVFXPARAMETER_DESC;

typedef struct _NVFXANNOTATION_DESC
{
    LPCSTR Name;                        // Annotation name
    LPCSTR Index;                       // Annotation index (cast to LPCSTR)
	LPCVOID Value;						// Annotation value (cast to LPCVOID)

    // Type
    NVFXPARAMETERTYPE Type;             // Annotation type
    UINT Dimension[NVFX_MAX_DIMENSIONS];                     // Elements in array
    UINT Bytes;                         // Total size in bytes

} NVFXANNOTATION_DESC;

typedef struct _NVFXTECHNIQUE_DESC
{
    LPCSTR Name;                        // Technique name
    LPCSTR Index;                       // Technique index (cast to LPCSTR)

    UINT Properties;                    // Number of properties
    UINT Passes;                        // Number of passes

} NVFXTECHNIQUE_DESC;

typedef struct _CgFXPASS_DESC
{
    LPCSTR Name;                        // Pass name
    LPCSTR Index;                       // Pass index (cast to LPCSTR)

} NVFXPASS_DESC;

typedef enum _NVFXMode
{
	NVFX_Unknown,
	NVFX_OpenGL,
	NVFX_Direct3D8,
	NVFX_Direct3D9
} NVFXMode;

typedef LPCSTR NVFXHANDLE;

class INVEffectDevice;
class INVEffect : public nv_sys::INVObject
{
public:
	// Descriptions
	/* Return description of effect */
	virtual HRESULT INTCALLTYPE GetDesc(D3DXEFFECT_DESC* pDesc) = 0;
	/* Return description of the named/indexed parameter */
	virtual HRESULT INTCALLTYPE GetParameterDesc(NVFXHANDLE Parameter, NVFXPARAMETER_DESC* pDesc) = 0;
	/* Return description of the named/indexed annotation */
	virtual HRESULT INTCALLTYPE GetAnnotationDesc(NVFXHANDLE Parameter, UINT Annotation, NVFXANNOTATION_DESC* pDesc) = 0;
	/* Return description of the named/indexed technique */
	virtual HRESULT INTCALLTYPE GetTechniqueDesc(NVFXHANDLE Technique, NVFXTECHNIQUE_DESC* pDesc) = 0;
	/* Return description of pass for given technique */
	virtual HRESULT INTCALLTYPE GetPassDesc(NVFXHANDLE Technique, UINT Pass, NVFXPASS_DESC* pDesc) = 0;

	// Get/Set Parameter
	virtual HRESULT INTCALLTYPE SetValue(NVFXHANDLE Handle, LPCVOID pData, UINT Bytes) = 0;
	virtual HRESULT INTCALLTYPE GetValue(NVFXHANDLE Handle, LPVOID pData, UINT Bytes) = 0;
	virtual HRESULT INTCALLTYPE SetFloat(NVFXHANDLE Handle, FLOAT f) = 0;
	virtual HRESULT INTCALLTYPE GetFloat(NVFXHANDLE Handle, FLOAT* f) = 0;
	virtual HRESULT INTCALLTYPE SetVector(NVFXHANDLE Handle, const float* pVector, UINT vecSize) = 0;
	virtual HRESULT INTCALLTYPE GetVector(NVFXHANDLE Handle, float* pVector, UINT* vecSize) = 0;
	virtual HRESULT INTCALLTYPE SetMatrix(NVFXHANDLE Handle, const float* pMatrix, UINT nRows, UINT nCols) = 0;
	virtual HRESULT INTCALLTYPE GetMatrix(NVFXHANDLE Handle, float* pMatrix, UINT* nRows, UINT* nCols) = 0;
	virtual HRESULT INTCALLTYPE SetDword(NVFXHANDLE Handle, DWORD dw) = 0;
	virtual HRESULT INTCALLTYPE GetDword(NVFXHANDLE Handle, DWORD* dw) = 0;
	virtual HRESULT INTCALLTYPE SetBoolValue(NVFXHANDLE Handle, bool bvalue) = 0;
	virtual HRESULT INTCALLTYPE GetBoolValue(NVFXHANDLE Handle, bool* bvalue) = 0;
	virtual HRESULT INTCALLTYPE SetString(NVFXHANDLE Handle, LPCSTR pString) = 0;
	virtual HRESULT INTCALLTYPE GetString(NVFXHANDLE Handle, LPCSTR* ppString) = 0;
	virtual HRESULT INTCALLTYPE SetTexture(NVFXHANDLE Handle, DWORD textureHandle) =0;
	virtual HRESULT INTCALLTYPE GetTexture(NVFXHANDLE Handle, DWORD* textureHandle) = 0;
	virtual HRESULT INTCALLTYPE SetVertexShader(NVFXHANDLE Handle, DWORD vsHandle) = 0;
	virtual HRESULT INTCALLTYPE GetVertexShader(NVFXHANDLE Handle, DWORD* vsHandle) = 0;
	virtual HRESULT INTCALLTYPE SetPixelShader(NVFXHANDLE Handle, DWORD psHandle) = 0;
	virtual HRESULT INTCALLTYPE GetPixelShader(NVFXHANDLE Handle, DWORD* psHandle) = 0;

    // Handle operations
    virtual NVFXHANDLE INTCALLTYPE GetParameter(NVFXHANDLE hParameter, UINT Index) = 0;
    virtual NVFXHANDLE INTCALLTYPE GetParameterByName(NVFXHANDLE hParameter, LPCSTR pName) = 0;
    virtual NVFXHANDLE INTCALLTYPE GetParameterBySemantic(NVFXHANDLE hParameter, LPCSTR pSemantic) = 0;
    virtual NVFXHANDLE INTCALLTYPE GetParameterElement(NVFXHANDLE hParameter, UINT Index) = 0;
    virtual NVFXHANDLE INTCALLTYPE GetTechnique(UINT Index) = 0;
    virtual NVFXHANDLE INTCALLTYPE GetTechniqueByName(LPCSTR pName) = 0;
    virtual NVFXHANDLE INTCALLTYPE GetPass(NVFXHANDLE hTechnique, UINT Index) = 0;
    virtual NVFXHANDLE INTCALLTYPE GetPassByName(NVFXHANDLE hTechnique, LPCSTR pName) = 0;
    virtual NVFXHANDLE INTCALLTYPE GetAnnotation(NVFXHANDLE hObject, UINT Index) = 0;
    virtual NVFXHANDLE INTCALLTYPE GetAnnotationByName(NVFXHANDLE hObject, LPCSTR pName) = 0;

	// Set current technique:
	virtual HRESULT INTCALLTYPE SetTechnique(NVFXHANDLE pTechnique) = 0;
	
	// Validate current technique:
	virtual HRESULT INTCALLTYPE Validate() = 0;
	
	// Returns the number of passes in pPasses:
	virtual HRESULT INTCALLTYPE Begin(UINT* pPasses, DWORD Flags) = 0;
	virtual HRESULT INTCALLTYPE Pass(UINT passNum) = 0;
	virtual HRESULT INTCALLTYPE End() = 0;
	virtual HRESULT INTCALLTYPE OnLostDevice() = 0;
	virtual HRESULT INTCALLTYPE OnResetDevice() = 0;

	virtual bool INTCALLTYPE GetParentDevice(INVEffectDevice** ppParent) = 0;
	virtual bool INTCALLTYPE SetParentDevice(INVEffectDevice* pDevice) = 0;

};

class INVEffectDevice : public nv_sys::INVObject
{
public:
	// INVEffectDevice
	virtual bool INTCALLTYPE Initialize(nv_renderdevice::INVRenderDevice* pDevice) = 0;
	virtual bool INTCALLTYPE UnInitialize() = 0;
	virtual bool INTCALLTYPE CreateEffectFromFile(LPCSTR pSrcFile, DWORD Flags, LPD3DXEFFECT* pEffect, const char** ppCompilationErrors) = 0;
	virtual bool INTCALLTYPE GetErrors(const char** ppErrors) = 0;
	virtual bool INTCALLTYPE GetRenderDevice(nv_renderdevice::INVRenderDevice** ppDevice) = 0;
};


}; // namespace nv_fx

// {C13E339A-2012-4946-B603-CD80D9B3A3D9}
static const nv_sys::NVGUID IID_INVEffect = 
{ 0xc13e339a, 0x2012, 0x4946, { 0xb6, 0x3, 0xcd, 0x80, 0xd9, 0xb3, 0xa3, 0xd9 } };

// {6EE1B48A-913F-41ec-B03C-B99F0C60E28C}
static const nv_sys::NVGUID IID_INVEffectDevice = 
{ 0x6ee1b48a, 0x913f, 0x41ec, { 0xb0, 0x3c, 0xb9, 0x9f, 0xc, 0x60, 0xe2, 0x8c } };

#endif // __INVEFFECTS_H
