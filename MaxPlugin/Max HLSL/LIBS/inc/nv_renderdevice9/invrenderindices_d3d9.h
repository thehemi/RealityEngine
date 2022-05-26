#ifndef __INVRENDERINDICES_D3D9_H
#define __INVRENDERINDICES_D3D9_H

struct IDirect3DIndexBuffer9;
namespace nv_renderdevice
{

// Private interface to access the VB
class INVRenderIndices_D3D9 : public INVRenderIndices
{
public:
	// INVRenderIndices_D3D9
	virtual bool INTCALLTYPE GetIB(IDirect3DIndexBuffer9** pIB) = 0;
	virtual bool INTCALLTYPE SetIB(IDirect3DIndexBuffer9* pIB) = 0;
};

}; // namespace nv_renderdevice

// {07B61D9F-B578-4b11-8751-28A56530C998}
static const nv_sys::NVGUID IID_INVRenderIndices_D3D9 = 
{ 0x7b61d9f, 0xb578, 0x4b11, { 0x87, 0x51, 0x28, 0xa5, 0x65, 0x30, 0xc9, 0x98 } };

#endif // INVRENDERINDICES_D3D_H