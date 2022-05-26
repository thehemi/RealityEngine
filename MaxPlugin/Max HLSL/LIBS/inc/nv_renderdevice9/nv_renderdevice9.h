#ifndef __NV_RENDERDEVICE9_H
#define __NV_RENDERDEVICE9_H

#include "nv_renderdevice\nv_renderdevice.h"
#include "nv_renderdevice9\invrenderdevice_d3d9.h"
#include "nv_renderdevice9\invrendervertices_d3d9.h"
#include "nv_renderdevice9\invrenderindices_d3d9.h"
#include "nv_renderdevice9\invtexture_d3d9.h"

// {1D11DE00-FCBF-4e57-9B51-7DFDAF5631ED}
static const nv_sys::NVGUID CLSID_NVRenderDevice_D3D9 = 
{ 0x1d11de00, 0xfcbf, 0x4e57, { 0x9b, 0x51, 0x7d, 0xfd, 0xaf, 0x56, 0x31, 0xed } };

// {0D296F4C-FCF9-45e4-BBC2-8CC7636663B9}
static const nv_sys::NVGUID CLSID_NVRenderDevice_D3D9Ref = 
{ 0xd296f4c, 0xfcf9, 0x45e4, { 0xbb, 0xc2, 0x8c, 0xc7, 0x63, 0x66, 0x63, 0xb9 } };

// {E84E0482-41F0-4af7-B6D7-A32968142470}
static const nv_sys::NVGUID CLSID_NVRenderVertices_D3D9 = 
{ 0xe84e0482, 0x41f0, 0x4af7, { 0xb6, 0xd7, 0xa3, 0x29, 0x68, 0x14, 0x24, 0x70 } };

// {C6C510D9-67F6-446b-97BF-6ED98782118E}
static const nv_sys::NVGUID CLSID_NVRenderIndices_D3D9 = 
{ 0xc6c510d9, 0x67f6, 0x446b, { 0x97, 0xbf, 0x6e, 0xd9, 0x87, 0x82, 0x11, 0x8e } };

// {789B693B-3BBD-4ee2-A6C5-C6EB3468171F}
static const nv_sys::NVGUID CLSID_NVTexture_D3D9 = 
{ 0x789b693b, 0x3bbd, 0x4ee2, { 0xa6, 0xc5, 0xc6, 0xeb, 0x34, 0x68, 0x17, 0x1f } };


#endif __NV_RENDERDEVICE9_H