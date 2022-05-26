/*********************************************************************NVMH4****
Path:  NVSDK\Common\include\nv_renderdevice
File:  nvrendervertices_d3d.h

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

d3d render vertices implementation.



******************************************************************************/

#ifndef __NVRENDERVERTICES_D3D_H
#define __NVRENDERVERTICES_D3D_H

namespace nv_renderdevice
{

class NVRenderVertices_D3D8 : public INVRenderVertices_D3D8
{
public:
	static bool INTCALLTYPE CreateNVObject(const nv_sys::INVCreator* pCreator, const nv_sys::NVGUID& InterfaceID, void** ppObj);

    // INVObject
	virtual unsigned long INTCALLTYPE AddRef();
	virtual unsigned long INTCALLTYPE Release();
	virtual bool INTCALLTYPE GetInterface(const nv_sys::NVGUID& guid, void** ppObj);

	// INVRenderVertices
    virtual bool INTCALLTYPE Lock(UINT OffsetToLock, UINT SizeToLock, BYTE** ppbData, DWORD Flags);
	virtual bool INTCALLTYPE Unlock();

	// INVRenderVertices_D3D
	virtual bool INTCALLTYPE GetVB(IDirect3DVertexBuffer8** pVB);
	virtual bool INTCALLTYPE SetVB(IDirect3DVertexBuffer8* pVB);

private:
    NVRenderVertices_D3D8();
	~NVRenderVertices_D3D8();

	IDirect3DVertexBuffer8* m_pVB;

	DWORD m_dwRefCount;

};

}; // namespace nv_renderdevice

#endif // NVRENDERDEVICE_D3D_H