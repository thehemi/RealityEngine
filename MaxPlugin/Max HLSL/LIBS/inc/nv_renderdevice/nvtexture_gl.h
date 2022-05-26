#ifndef __NVTEXTURE_GL_H
#define __NVTEXTURE_GL_H

namespace nv_renderdevice
{

typedef std::vector<DWORD> tvecDWORDS;
typedef std::vector<BYTE> tvecBYTES;
typedef std::vector<WORD> tvecWORDS;

class NVTexture_GL : public INVTexture_GL
{
public:
    static bool INTCALLTYPE CreateNVObject(const nv_sys::INVCreator* pCreator, const nv_sys::NVGUID& InterfaceID, void** ppObj);

	// INVObject
	virtual unsigned long INTCALLTYPE AddRef();
	virtual unsigned long INTCALLTYPE Release();
	virtual bool INTCALLTYPE GetInterface(const nv_sys::NVGUID& guid, void** ppObj);

	 // INVTexture
    virtual bool INTCALLTYPE LockRect(UINT Level, NVLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags);
	virtual bool INTCALLTYPE UnlockRect(UINT Level);
	virtual NVTEX_HANDLE INTCALLTYPE GetTextureHandle();
	virtual bool INTCALLTYPE SetTextureHandle(NVTEX_HANDLE Handle);

	// INVTexture_GL
	virtual bool INTCALLTYPE GetTexture(int* iTexture);
	virtual bool INTCALLTYPE SetTexture(int iHandle);

private:
	//UINT Width, UINT Height, UINT MipMaps, DWORD Flags, NVTEXTURETARGETTYPE TargetType, NVTEXTUREFORMATTYPE FormatType
    NVTexture_GL();
	virtual ~NVTexture_GL();

	unsigned int m_Texture;
	DWORD m_dwRefCount;
	DWORD m_dwSize;

	UINT m_Width;
	UINT m_Height;
	UINT m_MipMaps;
	DWORD m_Flags;
	NVTEXTURETARGETTYPE m_TargetType;
	NVTEXTUREFORMATTYPE m_FormatType;

	int m_Format;

	tvecBYTES m_vecBYTES;
	tvecDWORDS m_vecDWORDS;
	tvecWORDS m_vecWORDS;
};

}; // namespace nv_renderdevice

#endif // NVTEXTURE_D3D_H