/*********************************************************************NVMH4****
Path:  NVSDK\Common\include\nv_renderdevice
File:  invtexture.h

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


Texture interface


******************************************************************************/

#ifndef __INVTEXTURE_H
#define __INVTEXTURE_H

namespace nv_renderdevice
{

static const DWORD NVTEXTUREFLAG_DISCARD = (1 << 4);
static const DWORD NVTEXTUREFLAG_NOOVERWRITE = (1 << 5);

class INVTexture : public nv_sys::INVObject
{
public:
	// INVObject
	virtual unsigned long INTCALLTYPE AddRef() = 0;
	virtual unsigned long INTCALLTYPE Release() = 0;
	virtual bool INTCALLTYPE GetInterface(const nv_sys::NVGUID& guid, void** ppObj) = 0;

	// INVTexture
	virtual bool INTCALLTYPE LockRect(UINT Level, NVLOCKED_RECT* pLockedRect, CONST RECT* pRect,DWORD Flags) = 0;
	virtual bool INTCALLTYPE UnlockRect(UINT Level) = 0;
	virtual NVTEX_HANDLE INTCALLTYPE GetTextureHandle() = 0;
	virtual bool INTCALLTYPE SetTextureHandle(NVTEX_HANDLE hTex) = 0;
};

}; // nv_renderdevice

// {87F7801A-287B-41d8-91DF-9AC3058DB2C7}
static const nv_sys::NVGUID IID_INVTexture = 
{ 0x87f7801a, 0x287b, 0x41d8, { 0x91, 0xdf, 0x9a, 0xc3, 0x5, 0x8d, 0xb2, 0xc7 } };

#endif // __INVTEXTURE_H