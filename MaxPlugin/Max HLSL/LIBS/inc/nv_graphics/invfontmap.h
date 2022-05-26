/*********************************************************************NVMH4****
Path:  NVSDK\Common\include\nv_renderdevice
File:  invfontmap.h

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

INVFontMap interfae decleration



******************************************************************************/

#ifndef __INVFONTMAP_H
#define __INVFONTMAP_H

namespace nv_graphics
{

// Font creation flags
static const DWORD NVFONT_BOLD = 0x0001;
static const DWORD NVFONT_ITALIC = 0x0002;

class INVFontMap : public nv_sys::INVObject
{
public:
	// INVObject
	virtual unsigned long INTCALLTYPE AddRef() = 0;
	virtual unsigned long INTCALLTYPE Release() = 0;
	virtual bool INTCALLTYPE GetInterface(const nv_sys::NVGUID& guid, void** ppObj) = 0;

	// INVFontMap
	virtual bool INTCALLTYPE CreateFontMap(nv_renderdevice::INVRenderDevice* pDevice, const char* pszFontName, DWORD dwHeight, DWORD dwFontFlags) = 0;
	virtual bool INTCALLTYPE DestroyFontMap() = 0;
	virtual const vec4& INTCALLTYPE GetCharCoords(const char& szChar) const = 0;
	virtual const float INTCALLTYPE GetTextScale() const = 0;
	virtual void INTCALLTYPE GetTextureDimensions(DWORD& Width, DWORD& Height) const = 0;
	virtual bool INTCALLTYPE GetTexture(nv_renderdevice::INVTexture** pTex) const = 0;

};

}; // nv_graphics

// {B232BCDD-1223-4f2e-AFE2-9EC599E35035}
static const nv_sys::NVGUID IID_INVFontMap = 
{ 0xb232bcdd, 0x1223, 0x4f2e, { 0xaf, 0xe2, 0x9e, 0xc5, 0x99, 0xe3, 0x50, 0x35 } };

#endif // __INVTEXTURE_H