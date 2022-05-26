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

#ifndef __INVRENDERINDICES_GL_H
#define __INVRENDERINDICES_GL_H

namespace nv_renderdevice
{

// Private interface to access the IB
typedef struct tagNVRenderIndicesDesc_GL
{
	void* m_pData;
	unsigned long m_Length;
	bool m_b32Bits;
} NVRenderIndicesDesc_GL;

class INVRenderIndices_GL : public INVRenderIndices
{
public:
	// INVRenderIndices_GL
	virtual bool INTCALLTYPE GetDesc(NVRenderIndicesDesc_GL* pData) = 0;
	virtual bool INTCALLTYPE SetDesc(NVRenderIndicesDesc_GL* pData) = 0;
};

}; // namespace nv_renderdevice

// {5D29E6CE-CA2B-4f76-93FE-32CEA365EECF}
static const nv_sys::NVGUID IID_INVRenderIndices_GL = 
{ 0x5d29e6ce, 0xca2b, 0x4f76, { 0x93, 0xfe, 0x32, 0xce, 0xa3, 0x65, 0xee, 0xcf } };

#endif // INVRENDERINDICES_GL_H