#include "pch.h"
#include <shared\nv_common.h>
#include "invtexture_d3d9.h"
#include "nvtexture_d3d9.h"
 
using namespace nv_sys;

 

namespace nv_renderdevice
{

INVTexture_D3D9* CreateNVTexture_D3D9(){ return new NVTexture_D3D9; }

NVTexture_D3D9::NVTexture_D3D9()
: m_dwRefCount(1),
m_pTexture(NULL)
{
	SAFE_ADDREF(m_pTexture);
}

NVTexture_D3D9::~NVTexture_D3D9()
{
	SAFE_RELEASE(m_pTexture);
}

unsigned long NVTexture_D3D9::AddRef()
{
	m_dwRefCount++;
	return m_dwRefCount;
}

unsigned long NVTexture_D3D9::Release()
{
	unsigned long RefNew = --m_dwRefCount;
	if (RefNew == 0)
		delete this;
	return RefNew;
}

bool INTCALLTYPE NVTexture_D3D9::CreateNVObject(const nv_sys::INVCreator* pCreator, const NVGUID& InterfaceID, void** ppObj)
{
	NVTexture_D3D9* pObj = new NVTexture_D3D9;
	
	if (pObj)
	{
		if (pObj->GetInterface(InterfaceID, ppObj))
		{
			pObj->Release();
			return true;
		}
	
		delete pObj;
	}

	return false;
}

bool NVTexture_D3D9::GetInterface(const NVGUID& guid, void** ppObj)
{
	if (EqualNVGUID(guid, IID_INVObject))
	{
		*ppObj = static_cast<INVTexture*>(this);
	}
	else if (EqualNVGUID(guid, IID_INVTexture))
	{		
		*ppObj = static_cast<INVTexture*>(this);
	}
	else if (EqualNVGUID(guid, IID_INVTexture_D3D9))
	{		
		*ppObj = static_cast<INVTexture_D3D9*>(this);
	}
	else
	{
		return false;
	}
		
	static_cast<INVTexture_D3D9*>(this)->AddRef();
	return true;
}

bool NVTexture_D3D9::LockRect(UINT Level, NVLOCKED_RECT* pLockedRect, CONST RECT* pRect,DWORD Flags)
{
	if (!m_pTexture)
		return false;

	DWORD D3DFlags = D3DLOCK_NOSYSLOCK;
	if (Flags & NVTEXTUREFLAG_DISCARD)
		D3DFlags |= D3DLOCK_DISCARD;
	if (Flags & NVTEXTUREFLAG_NOOVERWRITE)
		D3DFlags |= D3DLOCK_NOOVERWRITE;

	D3DLOCKED_RECT D3DLockRect;
	if (FAILED(m_pTexture->LockRect(Level, &D3DLockRect, pRect, D3DFlags)))
		return false;

	pLockedRect->Pitch = D3DLockRect.Pitch;
	pLockedRect->pBits = D3DLockRect.pBits;

	return true;
}

bool NVTexture_D3D9::UnlockRect(UINT Level)
{
	if (!m_pTexture)
		return false;

	if (FAILED(m_pTexture->UnlockRect(Level)))
		return false;
	return true;
}

NVTEX_HANDLE NVTexture_D3D9::GetTextureHandle()
{
	return reinterpret_cast<NVTEX_HANDLE>(m_pTexture);
}

bool NVTexture_D3D9::SetTextureHandle(NVTEX_HANDLE Handle)
{
	m_pTexture = reinterpret_cast<IDirect3DTexture9*>(Handle);
	return true;
}

bool NVTexture_D3D9::GetTexture(IDirect3DTexture9** ppTexture)
{
	if (!m_pTexture || !ppTexture)
		return false;

	*ppTexture = m_pTexture;
	m_pTexture->AddRef();

	return true;
}

bool NVTexture_D3D9::SetTexture(IDirect3DTexture9* pTexture)
{
	SAFE_RELEASE(m_pTexture);
	m_pTexture = pTexture;
	SAFE_ADDREF(m_pTexture);

	return true;
}

}; // namespace nv_renderdevice

