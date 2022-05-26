/*********************************************************************NVMH4****
Path:  NVSDK\Common\include\nv_renderdevice
File:  invrenderindices.h

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


Index buffer interface


******************************************************************************/

#ifndef __INVRENDERINDICES_H
#define __INVRENDERINDICES_H

namespace nv_renderdevice
{

static const DWORD NVINDEXFLAG_DISCARD = (1 << 4);
static const DWORD NVINDEXFLAG_NOOVERWRITE = (1 << 5);

class INVRenderIndices : public nv_sys::INVObject
{
public:
	// INVObject
	virtual unsigned long INTCALLTYPE AddRef() = 0;
	virtual unsigned long INTCALLTYPE Release() = 0;
	virtual bool INTCALLTYPE GetInterface(const nv_sys::NVGUID& guid, void** ppObj) = 0;

	// INVRenderIndices
    virtual bool INTCALLTYPE Lock(UINT OffsetToLock, UINT SizeToLock, BYTE** ppbData, DWORD Flags) = 0;
	virtual bool INTCALLTYPE Unlock() = 0;
};

}; // nv_graphics

// {8A65D657-0C03-4c3f-8D52-AB0417D2CA31}
static const nv_sys::NVGUID IID_INVRenderIndices = 
{ 0x8a65d657, 0xc03, 0x4c3f, { 0x8d, 0x52, 0xab, 0x4, 0x17, 0xd2, 0xca, 0x31 } };

#endif // __INVRENDERINDICES_H