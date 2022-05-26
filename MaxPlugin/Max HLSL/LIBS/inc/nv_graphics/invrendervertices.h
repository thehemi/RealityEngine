/*********************************************************************NVMH4****
Path:  NVSDK\Common\include\nv_renderdevice
File:  invrendervertices.h

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


vertex buffer interface


******************************************************************************/

#ifndef __INVRENDERVERTICES_H
#define __INVRENDERVERTICES_H

namespace nv_renderdevice
{

static const DWORD NVVERTEXFLAG_DISCARD = (1 << 4);
static const DWORD NVVERTEXFLAG_NOOVERWRITE = (1 << 5);

class INVRenderVertices : public nv_sys::INVObject
{
public:
	// INVObject
	virtual unsigned long INTCALLTYPE AddRef() = 0;
	virtual unsigned long INTCALLTYPE Release() = 0;
	virtual bool INTCALLTYPE GetInterface(const nv_sys::NVGUID& guid, void** ppObj) = 0;

	// INVRenderVertices
    virtual bool INTCALLTYPE Lock(UINT OffsetToLock, UINT SizeToLock, BYTE** ppbData, DWORD Flags) = 0;
	virtual bool INTCALLTYPE Unlock() = 0;
};

}; //nv_renderdevice

// {1299D055-F383-433c-B70F-0C43C0C6A906}
static const nv_sys::NVGUID IID_INVRenderVertices = 
{ 0x1299d055, 0xf383, 0x433c, { 0xb7, 0xf, 0xc, 0x43, 0xc0, 0xc6, 0xa9, 0x6 } };

#endif // __INVRENDERDEVICE_H