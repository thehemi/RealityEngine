#ifndef __INVRENDERVERTICES_D3D9_H
#define __INVRENDERVERTICES_D3D9_H

struct IDirect3DVertexBuffer9;
struct IDirect3DVertexDeclaration9;

namespace nv_renderdevice
{

// Private interface to access the VB
class INVRenderVertices_D3D9 : public INVRenderVertices
{
public:
	// INVRenderVertices_D3D9
	virtual bool INTCALLTYPE GetVB(IDirect3DVertexBuffer9** pVB) const = 0;
	virtual bool INTCALLTYPE GetDecl(IDirect3DVertexDeclaration9** pVB) const = 0;
	virtual bool INTCALLTYPE SetVB(IDirect3DVertexBuffer9* pVB) = 0;
	virtual bool INTCALLTYPE SetDecl(IDirect3DVertexDeclaration9* pVB) = 0;

};

}; // namespace nv_renderdevice

// {D3D8628E-9F50-40b2-AA44-D4EC57704C56}
static const nv_sys::NVGUID IID_INVRenderVertices_D3D9 = 
{ 0xd3d8628e, 0x9f50, 0x40b2, { 0xaa, 0x44, 0xd4, 0xec, 0x57, 0x70, 0x4c, 0x56 } };

#endif // INVRENDERDEVICE_D3D_H