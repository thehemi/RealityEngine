//-----------------------------------------------------------------------------
/// FnMap8
/// 
/// Dx8 C++ style implementation of a procedural textures
//
///   Chris Brennan - ATI Research, Inc. - 2001
//-----------------------------------------------------------------------------

#ifndef __CFNMAP8_HPP
#define __CFNMAP8_HPP

#ifndef DOXYGEN_IGNORE

#include "d3dcustom.h" 

/// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// 
/// CBaseMap8: Base class so that different implementations can be stored together //
/// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// 
class CBaseMap8
{
public:
   Texture texture;

public:
   CBaseMap8();
   ~CBaseMap8();

   LPDIRECT3DBASETEXTURE9 GetTexture() { return texture.GetTexture(); };
   virtual HRESULT Initialize() = 0;

   UINT m_dwLevels;
   D3DFORMAT m_Format;
};

/// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// 
/// CFnMap8: 2D procedural texture
/// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// 
class CFnMap8 : public CBaseMap8
{
private:
   static VOID WINAPI Fill2DWrapper(D3DXVECTOR4* pOut, D3DXVECTOR2* pTexCoord, D3DXVECTOR2* pTexelSize, LPVOID pData);

protected:
   virtual D3DXCOLOR Function(D3DXVECTOR2* pTexCoord, D3DXVECTOR2* pTexelSize) = 0;

public:
   CFnMap8();
   HRESULT Initialize();

   UINT m_dwWidth, m_dwHeight;
};

/// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// 
/// CVolumeMap8: 3D procedural texture
/// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// 
class CVolumeMap8 : public CBaseMap8
{
private:
   static VOID WINAPI Fill3DWrapper(D3DXVECTOR4* pOut, D3DXVECTOR3* pTexCoord, D3DXVECTOR3* pTexelSize, LPVOID pData);

protected:
   virtual D3DXCOLOR Function(D3DXVECTOR3* pTexCoord, D3DXVECTOR3* pTexelSize) = 0;

public:
   CVolumeMap8();
   HRESULT Initialize();

   UINT m_dwWidth, m_dwHeight, m_dwDepth;
};

/// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// 
/// CCubeMap8
/// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// 
class CCubeMap8 : public CBaseMap8
{
private:
   static VOID WINAPI Fill3DWrapper(D3DXVECTOR4* pOut, D3DXVECTOR3* pTexCoord, D3DXVECTOR3* pTexelSize, LPVOID pData);

protected:
   virtual D3DXCOLOR Function(D3DXVECTOR3* pTexCoord, D3DXVECTOR3* pTexelSize) = 0;

public:
   CCubeMap8();
   HRESULT Initialize();

   UINT m_dwSize;
};


/// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// 
/// CNormalizeMap8: An instance of the CCubeMap8 that creates a normalizer cube map
/// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// 
class CNormalizerMap8 : public CCubeMap8
{
protected:
   virtual D3DXCOLOR Function(D3DXVECTOR3* pTexCoord, D3DXVECTOR3* pTexelSize);

public:
   CNormalizerMap8();
};


class CFalloffMap : public CVolumeMap8
{
public:
    CFalloffMap(UINT size);
    D3DXCOLOR Function(D3DXVECTOR3* p, D3DXVECTOR3* s);
};


class C1DFalloffMap : public CFnMap8
{
public:
    C1DFalloffMap(DWORD size);
    D3DXCOLOR Function(D3DXVECTOR2* p, D3DXVECTOR2* s);
};

class CSpecularMap : public CFnMap8
{
public:
    CSpecularMap(DWORD size);
    D3DXCOLOR Function(D3DXVECTOR2* p, D3DXVECTOR2* s);
};


//-----------------------------------------------------------------------------
/// 2D Texture point light falloff class
//-----------------------------------------------------------------------------
class CPointFalloffMap : public CFnMap8
{
public:
    CPointFalloffMap(DWORD size);
    D3DXCOLOR Function(D3DXVECTOR2* p, D3DXVECTOR2* s);
};

#endif

#endif __CFNMAP8_HPP
