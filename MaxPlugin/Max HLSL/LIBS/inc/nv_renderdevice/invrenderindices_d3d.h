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


d3d render indices interface


******************************************************************************/

#ifndef __INVRENDERINDICES_D3D_H
#define __INVRENDERINDICES_D3D_H

#include <D3D8.h>

namespace nv_renderdevice
{

// Private interface to access the VB
class INVRenderIndices_D3D8 : public INVRenderIndices
{
public:
	// INVRenderIndices_D3D8
	virtual bool INTCALLTYPE GetIB(IDirect3DIndexBuffer8** pIB) = 0;
	virtual bool INTCALLTYPE SetIB(IDirect3DIndexBuffer8* pIB) = 0;
};

}; // namespace nv_renderdevice

// {60C4822B-FAB2-4545-ACE7-F993CF4E91AA}
static const nv_sys::NVGUID IID_INVRenderIndices_D3D8 = 
{ 0x60c4822b, 0xfab2, 0x4545, { 0xac, 0xe7, 0xf9, 0x93, 0xcf, 0x4e, 0x91, 0xaa } };

#endif // INVRENDERINDICES_D3D_H