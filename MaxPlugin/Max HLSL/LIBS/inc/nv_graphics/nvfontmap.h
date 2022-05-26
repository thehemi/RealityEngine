/*********************************************************************NVMH4****
Path:  NVSDK\Common\include\nv_renderdevice
File:  nvfontmap.h

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


see nvfontmap_d3d/gl.cpp


******************************************************************************/

#ifndef __FONTMAP_H
#define __FONTMAP_H

namespace nv_graphics
{

class NVFontMap : public INVFontMap
{
public:
	NVFontMap();
	virtual ~NVFontMap();

	static bool INTCALLTYPE CreateNVObject(const nv_sys::INVCreator* pCreator, const nv_sys::NVGUID& InterfaceID, void** ppObj);

	// INVObject
	virtual unsigned long INTCALLTYPE AddRef();
	virtual unsigned long INTCALLTYPE Release();
	virtual bool INTCALLTYPE GetInterface(const nv_sys::NVGUID& guid, void** ppObj);

	// INVFontMap
	virtual bool INTCALLTYPE CreateFontMap(nv_renderdevice::INVRenderDevice* pDevice, const char* pszFontName, DWORD dwHeight, DWORD dwFontFlags);
	virtual bool INTCALLTYPE DestroyFontMap();
	virtual const vec4& INTCALLTYPE GetCharCoords(const char& szChar) const;
	virtual const float INTCALLTYPE GetTextScale() const;
	virtual void INTCALLTYPE GetTextureDimensions(DWORD& Width, DWORD& Height) const;
	virtual bool INTCALLTYPE GetTexture(nv_renderdevice::INVTexture** pTex) const;
private:
	DWORD	m_dwRefCount;
	nv_renderdevice::INVTexture* m_pTexture;  
    DWORD   m_dwTexWidth; 
    DWORD   m_dwTexHeight;
    FLOAT   m_fTextScale;
	vec4    m_TexCoords[128-32];
};


}; // namespace nv_renderdevice

#endif