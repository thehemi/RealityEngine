/*********************************************************************NVMH4****
Path:  NVSDK\Common\include\nv_renderdevice
File:  nvrenderindices_d3d.h

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


d3d render indices implementation.


******************************************************************************/

#ifndef __NVRENDERINDICES_D3D_H
#define __NVRENDERINDICES_D3D_H

#include "invrenderindices_d3d.h"

namespace nv_renderdevice
{

class NVRenderIndices_D3D8 : public INVRenderIndices_D3D8
{
public:
	static bool INTCALLTYPE CreateNVObject(const nv_sys::INVCreator* pCreator, const nv_sys::NVGUID& InterfaceID, void** ppObj);

	// INVObject
	virtual unsigned long INTCALLTYPE AddRef();
	virtual unsigned long INTCALLTYPE Release();
	virtual bool INTCALLTYPE GetInterface(const nv_sys::NVGUID& guid, void** ppObj);

	// INVRenderIndices
	virtual bool INTCALLTYPE Lock(UINT OffsetToLock, UINT SizeToLock, BYTE** ppbData, DWORD Flags);
	virtual bool INTCALLTYPE Unlock();

	// INVRenderIndices_D3D
	virtual bool INTCALLTYPE GetIB(IDirect3DIndexBuffer8** pIB);
	virtual bool INTCALLTYPE SetIB(IDirect3DIndexBuffer8* pIB);

private:
    NVRenderIndices_D3D8();
	~NVRenderIndices_D3D8();

	IDirect3DIndexBuffer8* m_pIB;

	DWORD m_dwRefCount;

};

}; // namespace nv_renderdevice

#endif // NVRENDERINDICES_D3D_H