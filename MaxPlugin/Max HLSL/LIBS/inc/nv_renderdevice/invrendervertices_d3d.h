/*********************************************************************NVMH4****
Path:  NVSDK\Common\include\nv_renderdevice
File:  invrendervertices_d3d.h

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

d3d render vertices interface



******************************************************************************/

#ifndef __INVRENDERVERTICES_D3D_H
#define __INVRENDERVERTICES_D3D_H

interface IDirect3DVertexBuffer8;
namespace nv_renderdevice
{

class INVRenderVertices_D3D8 : public INVRenderVertices
{
public:
	// INVRenderVertices_D3D
	virtual bool INTCALLTYPE GetVB(IDirect3DVertexBuffer8** pVB) = 0;
	virtual bool INTCALLTYPE SetVB(IDirect3DVertexBuffer8* pVB) = 0;
};

}; // namespace nv_renderdevice

// {7BDC9D99-D864-4a32-880C-8389AA04818F}
static const nv_sys::NVGUID IID_INVRenderVertices_D3D8 = 
{ 0x7bdc9d99, 0xd864, 0x4a32, { 0x88, 0xc, 0x83, 0x89, 0xaa, 0x4, 0x81, 0x8f } };

#endif //__INVRENDERVERTICES_D3D_H