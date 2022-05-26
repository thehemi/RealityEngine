#ifndef __NVTEXTURE_D3D9_H
#define __NVTEXTURE_D3D9_H

namespace nv_renderdevice
{

class NVTexture_D3D9 : public INVTexture_D3D9
{
public:
	static bool INTCALLTYPE CreateNVObject(const nv_sys::INVCreator* pCreator, const nv_sys::NVGUID& InterfaceID, void** ppObj);

    // Functions to create, run, pause, and clean up the application
	// INVObject
	virtual unsigned long INTCALLTYPE AddRef();
	virtual unsigned long INTCALLTYPE Release();
	virtual bool INTCALLTYPE GetInterface(const nv_sys::NVGUID& guid, void** ppObj);

    // INVTexture
    virtual bool INTCALLTYPE LockRect(UINT Level, NVLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags);
	virtual bool INTCALLTYPE UnlockRect(UINT Level);
	virtual NVTEX_HANDLE INTCALLTYPE GetTextureHandle();
	virtual bool INTCALLTYPE SetTextureHandle(NVTEX_HANDLE Handle);


	// INVTexture_D3D
	virtual bool INTCALLTYPE GetTexture(IDirect3DTexture9** pTexture);
	virtual bool INTCALLTYPE SetTexture(IDirect3DTexture9* pVB);

    NVTexture_D3D9();
	~NVTexture_D3D9();

private:

	IDirect3DTexture9* m_pTexture;
	DWORD m_dwRefCount;
};

}; // namespace nv_renderdevice

#endif // NVTEXTURE_D3D9_H