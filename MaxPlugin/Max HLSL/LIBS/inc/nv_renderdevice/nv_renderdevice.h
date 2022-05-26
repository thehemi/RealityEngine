/*********************************************************************NVMH4****
Path:  NVSDK\Common\include\nv_renderdevice
File:  nv_renderdevice.h

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





******************************************************************************/

#ifndef __NV_RENDERDEVICE_H
#define __NV_RENDERDEVICE_H

#include "nv_sys\nv_sys.h"

#include "nv_graphics\invrenderdevice.h"
#include "nv_graphics\invrendervertices.h"
#include "nv_graphics\invrenderindices.h"
#include "nv_graphics\invtexture.h"
#include "nv_graphics\invfontmap.h"
#include "nv_graphics\dynamicvertices.h"


// {B0619034-9872-4515-82BE-BFC36B1023D6}
static const nv_sys::NVGUID CLSID_NVRenderDevice_D3D8 = 
{ 0xb0619034, 0x9872, 0x4515, { 0x82, 0xbe, 0xbf, 0xc3, 0x6b, 0x10, 0x23, 0xd6 } };

// {3F03E35E-C77D-4ba8-924A-BC387A77F5E7}
static const nv_sys::NVGUID CLSID_NVRenderDevice_D3D8Ref = 
{ 0x3f03e35e, 0xc77d, 0x4ba8, { 0x92, 0x4a, 0xbc, 0x38, 0x7a, 0x77, 0xf5, 0xe7 } };

// {E2625F72-0037-4ed3-8B6F-F7BD5E383CC2}
static const nv_sys::NVGUID CLSID_NVRenderDevice_GL = 
{ 0xe2625f72, 0x37, 0x4ed3, { 0x8b, 0x6f, 0xf7, 0xbd, 0x5e, 0x38, 0x3c, 0xc2 } };

// {C3F79999-71A6-4d8a-B9E8-F847EF72972F}
static const nv_sys::NVGUID CLSID_NVRenderVertices_D3D8 = 
{ 0xc3f79999, 0x71a6, 0x4d8a, { 0xb9, 0xe8, 0xf8, 0x47, 0xef, 0x72, 0x97, 0x2f } };

// {7E9AB3BE-70A0-4c2e-B35F-20F80008CB19}
static const nv_sys::NVGUID CLSID_NVRenderIndices_D3D8 = 
{ 0x7e9ab3be, 0x70a0, 0x4c2e, { 0xb3, 0x5f, 0x20, 0xf8, 0x0, 0x8, 0xcb, 0x19 } };

// {6F1AF4C1-ABD7-4d28-A915-0F64CFE663AA}
static const nv_sys::NVGUID CLSID_NVRenderVertices_GL = 
{ 0x6f1af4c1, 0xabd7, 0x4d28, { 0xa9, 0x15, 0xf, 0x64, 0xcf, 0xe6, 0x63, 0xaa } };

// {751DDE7B-AAD2-4cb9-B1E7-6EFFAC842C42}
static const nv_sys::NVGUID CLSID_NVRenderIndices_GL = 
{ 0x751dde7b, 0xaad2, 0x4cb9, { 0xb1, 0xe7, 0x6e, 0xff, 0xac, 0x84, 0x2c, 0x42 } };

// {09CEBACC-A368-453f-8642-6931EF994A08}
static const nv_sys::NVGUID CLSID_NVTexture_D3D8 = 
{ 0x9cebacc, 0xa368, 0x453f, { 0x86, 0x42, 0x69, 0x31, 0xef, 0x99, 0x4a, 0x8 } };

// {0A42D31F-EDA3-4da5-8ACB-63C0F5B35C4D}
static const nv_sys::NVGUID CLSID_NVTexture_GL = 
{ 0xa42d31f, 0xeda3, 0x4da5, { 0x8a, 0xcb, 0x63, 0xc0, 0xf5, 0xb3, 0x5c, 0x4d } };


#endif
