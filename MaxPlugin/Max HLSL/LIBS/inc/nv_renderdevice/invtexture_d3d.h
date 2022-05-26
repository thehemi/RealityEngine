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


d3d texture interface


******************************************************************************/

#ifndef __INVTEXTURE_D3D8_H
#define __INVTEXTURE_D3D8_H

namespace nv_renderdevice
{

// Private interface to access the texture
class INVTexture_D3D8 : public INVTexture
{
public:
	// INVTexture_D3D8
	virtual bool INTCALLTYPE GetTexture(IDirect3DTexture8** ppTexture) = 0;
	virtual bool INTCALLTYPE SetTexture(IDirect3DTexture8* pTexture) = 0;
};

}; // namespace nv_renderdevice

// {7683A057-53D7-40d1-8F02-8BE99508B0AD}
static const nv_sys::NVGUID IID_INVTexture_D3D8 = 
{ 0x7683a057, 0x53d7, 0x40d1, { 0x8f, 0x2, 0x8b, 0xe9, 0x95, 0x8, 0xb0, 0xad } };

#endif // NVTEXTURE_D3D8_H