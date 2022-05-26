#ifndef __INVTEXTURE_GL_H
#define __INVTEXTURE_GL_H

namespace nv_renderdevice
{

class INVTexture_GL : public INVTexture
{
public:
	// INVTexture_GL
	virtual bool INTCALLTYPE GetTexture(int* iHandle) = 0;
	virtual bool INTCALLTYPE SetTexture(int iHandle) = 0;
};

}; // namespace nv_renderdevice

// {79861073-7945-4870-9E8C-88B34C1A63AC}
static const nv_sys::NVGUID IID_INVTexture_GL = 
{ 0x79861073, 0x7945, 0x4870, { 0x9e, 0x8c, 0x88, 0xb3, 0x4c, 0x1a, 0x63, 0xac } };

#endif // INVTEXTURE_D3D_H