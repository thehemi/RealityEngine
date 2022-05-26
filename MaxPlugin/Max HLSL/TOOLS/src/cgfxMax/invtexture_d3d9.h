#ifndef __INVTEXTURE_D3D9_H
#define __INVTEXTURE_D3D9_H

struct IDirect3DTexture9;

namespace nv_renderdevice
{

// Private interface to access the texture
class INVTexture_D3D9 : public INVTexture
{
public:
	// INVTexture_D3D9
	virtual bool INTCALLTYPE GetTexture(IDirect3DTexture9** pVB) = 0;
	virtual bool INTCALLTYPE SetTexture(IDirect3DTexture9* pVB) = 0;
};

INVTexture_D3D9* CreateNVTexture_D3D9();

}; // namespace nv_renderdevice



// {812DF045-4AF8-44db-8DAF-C514F2590924}
static const nv_sys::NVGUID IID_INVTexture_D3D9 = 
{ 0x812df045, 0x4af8, 0x44db, { 0x8d, 0xaf, 0xc5, 0x14, 0xf2, 0x59, 0x9, 0x24 } };

// {789B693B-3BBD-4ee2-A6C5-C6EB3468171F}
static const nv_sys::NVGUID CLSID_NVTexture_D3D9 = 
{ 0x789b693b, 0x3bbd, 0x4ee2, { 0xa6, 0xc5, 0xc6, 0xeb, 0x34, 0x68, 0x17, 0x1f } };


#endif // INVTEXTURE_D3D9_H