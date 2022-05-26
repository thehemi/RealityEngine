/*********************************************************************NVMH4****
Path:  NVSDK\Common\include\nveffects_cgfx
File:  nveffects_cgfx.h

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

cgfx effects implementation



******************************************************************************/

#ifndef __NVEFFECT_CGFX_H
#define __NVEFFECT_CGFX_H

namespace nv_fx
{


class NVEffect_CgFX : public INVEffect_CgFX
{
public:
	static bool INTCALLTYPE CreateNVObject(const nv_sys::INVCreator* pCreator, const nv_sys::NVGUID& InterfaceID, void** ppObj);

	virtual unsigned long INTCALLTYPE AddRef();
	virtual unsigned long INTCALLTYPE Release();
	virtual bool INTCALLTYPE GetInterface(const nv_sys::NVGUID& guid, void** ppObj);

	// Descriptions
	/* Return description of effect */
	virtual HRESULT INTCALLTYPE GetDesc(D3DXEFFECT_DESC* pDesc);
	/* Return description of the named/indexed parameter */
	virtual HRESULT INTCALLTYPE GetParameterDesc(NVFXHANDLE Parameter, NVFXPARAMETER_DESC* pDesc);
	/* Return description of the named/indexed annotation */
	virtual HRESULT INTCALLTYPE GetAnnotationDesc(NVFXHANDLE Parameter, UINT pAnnotation, NVFXANNOTATION_DESC* pDesc);
	/* Return description of the named/indexed technique */
	virtual HRESULT INTCALLTYPE GetTechniqueDesc(NVFXHANDLE Technique, NVFXTECHNIQUE_DESC* pDesc);
	/* Return description of pass for given technique */
	virtual HRESULT INTCALLTYPE GetPassDesc(NVFXHANDLE Technique, UINT Pass, NVFXPASS_DESC* pDesc);

	// Get/Set Parameter
	virtual HRESULT INTCALLTYPE SetValue(NVFXHANDLE hName, LPCVOID pData, UINT Bytes);
	virtual HRESULT INTCALLTYPE GetValue(NVFXHANDLE hName, LPVOID pData, UINT Bytes);
	virtual HRESULT INTCALLTYPE SetFloat(NVFXHANDLE hName, FLOAT f);
	virtual HRESULT INTCALLTYPE GetFloat(NVFXHANDLE hName, FLOAT* f);
	virtual HRESULT INTCALLTYPE SetVector(NVFXHANDLE hName, const float* pVector, UINT vecSize);
	virtual HRESULT INTCALLTYPE GetVector(NVFXHANDLE hName, float* pVector, UINT* vecSize);
	virtual HRESULT INTCALLTYPE SetMatrix(NVFXHANDLE hName, const float* pMatrix, UINT nRows, UINT nCols);
	virtual HRESULT INTCALLTYPE GetMatrix(NVFXHANDLE hName, float* pMatrix, UINT* nRows, UINT* nCols);
	virtual HRESULT INTCALLTYPE SetDword(NVFXHANDLE hName, DWORD dw);
	virtual HRESULT INTCALLTYPE GetDword(NVFXHANDLE hName, DWORD* dw);
	virtual HRESULT INTCALLTYPE SetBoolValue(NVFXHANDLE hName, bool bvalue);
	virtual HRESULT INTCALLTYPE GetBoolValue(NVFXHANDLE hName, bool* bvalue);
	virtual HRESULT INTCALLTYPE SetString(NVFXHANDLE hName, LPCSTR pString);
	virtual HRESULT INTCALLTYPE GetString(NVFXHANDLE hName, LPCSTR* ppString);
	virtual HRESULT INTCALLTYPE SetTexture(NVFXHANDLE hName, DWORD textureHandle);
	virtual HRESULT INTCALLTYPE GetTexture(NVFXHANDLE hName, DWORD* textureHandle);
	virtual HRESULT INTCALLTYPE SetVertexShader(NVFXHANDLE hName, DWORD vsHandle);
	virtual HRESULT INTCALLTYPE GetVertexShader(NVFXHANDLE hName, DWORD* vsHandle);
	virtual HRESULT INTCALLTYPE SetPixelShader(NVFXHANDLE hName, DWORD psHandle);
	virtual HRESULT INTCALLTYPE GetPixelShader(NVFXHANDLE hName, DWORD* psHandle);

	// Handles
    virtual NVFXHANDLE INTCALLTYPE GetParameter(NVFXHANDLE hParameter, UINT Index);
    virtual NVFXHANDLE INTCALLTYPE GetParameterByName(NVFXHANDLE hParameter, LPCSTR pName);
    virtual NVFXHANDLE INTCALLTYPE GetParameterBySemantic(NVFXHANDLE hParameter, LPCSTR pSemantic);
    virtual NVFXHANDLE INTCALLTYPE GetParameterElement(NVFXHANDLE hParameter, UINT Index);
    virtual NVFXHANDLE INTCALLTYPE GetTechnique(UINT Index);
    virtual NVFXHANDLE INTCALLTYPE GetTechniqueByName(LPCSTR pName);
    virtual NVFXHANDLE INTCALLTYPE GetPass(NVFXHANDLE hTechnique, UINT Index);
    virtual NVFXHANDLE INTCALLTYPE GetPassByName(NVFXHANDLE hTechnique, LPCSTR pName);
    virtual NVFXHANDLE INTCALLTYPE GetAnnotation(NVFXHANDLE hObject, UINT Index);
    virtual NVFXHANDLE INTCALLTYPE GetAnnotationByName(NVFXHANDLE hObject, LPCSTR pName);

	// Set/get current technique:
	virtual HRESULT INTCALLTYPE SetTechnique(LPCSTR pTechnique);
	virtual HRESULT INTCALLTYPE GetTechnique(LPCSTR* ppTechnique);
	
	// Validate current technique:
	virtual HRESULT INTCALLTYPE Validate();
	
	// Returns the number of passes in pPasses:
	virtual HRESULT INTCALLTYPE Begin(UINT* pPasses, DWORD Flags);
	virtual HRESULT INTCALLTYPE Pass(UINT passNum) ;
	virtual HRESULT INTCALLTYPE End();
	virtual HRESULT INTCALLTYPE OnLostDevice();
	virtual HRESULT INTCALLTYPE OnResetDevice();

	virtual bool INTCALLTYPE SetParentDevice(INVEffectDevice* pDevice);
	virtual bool INTCALLTYPE GetParentDevice(INVEffectDevice** ppParent);

	// INVEffect_CgFX
	virtual bool INTCALLTYPE SetEffectCgFX(ICgFXEffect* pCgFXEffect);

private:
    // Internal functions 
	DWORD m_dwRefCount;

	ICgFXEffect* m_pEffect;
	INVEffectDevice* m_pParentDevice;

    // Internal constructor
    NVEffect_CgFX();
	~NVEffect_CgFX();
};

}; //namespace nv_fx

#endif // NVEFFECT_CGFX_H