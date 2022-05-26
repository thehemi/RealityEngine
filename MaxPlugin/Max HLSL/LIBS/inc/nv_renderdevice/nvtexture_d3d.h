/*********************************************************************NVMH4****
Path:  NVSDK\Common\include\nv_renderdevice
File:  nvtexture_d3d.h

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


d3d texture implementation for nv_renderdevice.


******************************************************************************/

#ifndef __NVTEXTURE_D3D8_H
#define __NVTEXTURE_D3D8_H

namespace nv_renderdevice
{

class NVTexture_D3D8 : public INVTexture_D3D8
{
public:
    static bool INTCALLTYPE CreateNVObject(const nv_sys::INVCreator* pCreator, const nv_sys::NVGUID& InterfaceID, void** ppObj);

	// INVObject
	virtual unsigned long INTCALLTYPE AddRef();
	virtual unsigned long INTCALLTYPE Release();
	virtual bool INTCALLTYPE GetInterface(const nv_sys::NVGUID& guid, void** ppObj);

	// INVTexture
    virtual bool INTCALLTYPE LockRect(UINT Level, NVLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags);
	virtual bool INTCALLTYPE UnlockRect(UINT Level);
	virtual NVTEX_HANDLE INTCALLTYPE GetTextureHandle();
	virtual bool INTCALLTYPE SetTextureHandle(NVTEX_HANDLE Handle);

	// INVTexture_D3D8
	virtual bool INTCALLTYPE GetTexture(IDirect3DTexture8** pTexture);
	virtual bool INTCALLTYPE SetTexture(IDirect3DTexture8* pTexture);

private:
    NVTexture_D3D8();
	~NVTexture_D3D8();

	IDirect3DTexture8* m_pTexture;
	DWORD m_dwRefCount;
};

}; // namespace nv_renderdevice

#endif // NVTEXTURE_D3D8_H