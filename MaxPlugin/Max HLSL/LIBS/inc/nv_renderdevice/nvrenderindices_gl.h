/*********************************************************************NVMH4****
Path:  NVSDK\Common\include\nv_renderdevice
File:  nvrenderindices_gl.h

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


GL renderindices implementation.


******************************************************************************/

#ifndef __NVRENDERINDICES_GL_H
#define __NVRENDERINDICES_GL_H

namespace nv_renderdevice
{

class NVRenderIndices_GL : public INVRenderIndices_GL
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

	// INVRenderIndices_GL
	virtual bool INTCALLTYPE GetDesc(NVRenderIndicesDesc_GL* pDesc);
	virtual bool INTCALLTYPE SetDesc(NVRenderIndicesDesc_GL* pDesc);

private:
    NVRenderIndices_GL();
	~NVRenderIndices_GL();

	NVRenderIndicesDesc_GL m_Desc;
	DWORD m_dwRefCount;

};

}; // namespace nv_renderdevice

#endif // NVRENDERINDICES_GL_H