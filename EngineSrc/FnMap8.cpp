//-----------------------------------------------------------------------------
// FnMap8
// Tim Johnson
// Procedural textures
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "RenderDevice.h"
#include "FnMap8.h"
#include <tchar.h>
#include <dxerr9.h>
#define SIZE 256

/*
void MainApp::createAngleMap(int size){
	unsigned char *imgNeg, *imgPos, *destNeg, *destPos;
	float vx, vy, vz;
	int x, y, z;
	float mid = 0.5f * (size - 1);

	Image *images[6];
	for (unsigned int i = 0; i < 6; i++) images[i] = new Image();

	destPos = imgPos = new unsigned char[size * size * 3];
	destNeg = imgNeg = new unsigned char[size * size * 3];
	for (y = 0; y < size; y++){
		for (z = 0; z < size; z++){
			vx = mid;
			vy = mid - y;
			vz = mid - z;

			float ang0 = atan2f(vy, vx) / PI;
			float ang1 = dot(normalize(vec3(vx, vz, vy)), normalize(vec3(vx, 0, vy)));
			ang1 = sqrtf(fabsf(1 - ang1 * ang1));

			*destPos++ = 0;
			*destPos++ = Pack1(ang0);
			*destPos++ = Pack2(ang1 * (vz > 0));
			*destNeg++ = 0;
			*destNeg++ = Pack1(ang0 - sign(vy));
			*destNeg++ = Pack2(ang1 * (vz < 0));
		}
	}
	images[0]->loadFromMemory(imgPos, size, size, FORMAT_RGB8, true, false);
	images[1]->loadFromMemory(imgNeg, size, size, FORMAT_RGB8, true, false);

	destPos = imgPos = new unsigned char[size * size * 3];
	destNeg = imgNeg = new unsigned char[size * size * 3];
	for (z = 0; z < size; z++){
		for (x = 0; x < size; x++){
			vx = x - mid;
			vy = mid;
			vz = z - mid;

			float ang0 = atan2f(vy, vx) / PI;
			float ang1 = dot(normalize(vec3(vx, vz, vy)), normalize(vec3(vx, 0, vy)));
			ang1 = sqrtf(fabsf(1 - ang1 * ang1));

			*destPos++ = 0;
			*destPos++ = Pack1(ang0);
			*destPos++ = Pack2(ang1 * (vz > 0));
			*destNeg++ = 0;
			*destNeg++ = Pack1(-ang0);
			*destNeg++ = Pack2(ang1 * (vz < 0));
		}
	}
	images[2]->loadFromMemory(imgPos, size, size, FORMAT_RGB8, true, false);
	images[3]->loadFromMemory(imgNeg, size, size, FORMAT_RGB8, true, false);

	destPos = imgPos = new unsigned char[size * size * 3];
	destNeg = imgNeg = new unsigned char[size * size * 3];
	for (y = 0; y < size; y++){
		for (x = 0; x < size; x++){
			vx = x - mid;
			vy = mid - y;
			vz = mid;

			float ang0 = atan2f(vy, vx) / PI;
			float ang1 = dot(normalize(vec3(vx, vz, vy)), normalize(vec3(vx, 0, vy)));
			ang1 = sqrtf(fabsf(1 - ang1 * ang1));

			*destPos++ = 0;
			*destPos++ = Pack1(ang0);
			*destPos++ = Pack2(ang1);
			*destNeg++ = 0;
			*destNeg++ = Pack1(sign(vy) - ang0);
			*destNeg++ = 0;
		}
	}
	images[4]->loadFromMemory(imgPos, size, size, FORMAT_RGB8, true, false);
	images[5]->loadFromMemory(imgNeg, size, size, FORMAT_RGB8, true, false);

	angleMap = renderer->addCubemap(images);

	angleMap->saveToFile("angleMap.dds");
}
*/

void ConvertToArray(DWORD array[][SIZE],LPDIRECT3DTEXTURE9 tex){
	// Lock the surface and write the alpha values for the set pixels
	D3DLOCKED_RECT d3dlr;

	tex->LockRect( 0, &d3dlr, 0, 0 );
	BYTE* pBits = (BYTE*)d3dlr.pBits;
	LONG pitch = d3dlr.Pitch;

	long m_dwTexHeight = 0;
	long m_dwTexWidth = 0;

	// Holds information about the destination surface
	D3DSURFACE_DESC desc;

	// Get information about the destination surface
	tex->GetLevelDesc(0, &desc );

	m_dwTexWidth = desc.Width;
	m_dwTexHeight = desc.Height;

	if(m_dwTexHeight != SIZE)
		Error("3D attenuation texture is not %dx%d",SIZE,SIZE);

	int SourceOffset = 0;
	/*for (int y = 0 ; y < m_dwTexHeight ; y++ )
	{
		for ( int x = 0 ; x < m_dwTexWidth ; x++ )
		{

			array[x][y] = pBits[ SourceOffset ];

			SourceOffset++;
		}
		//SourceOffset += pitch/4;
	}*/
	for( DWORD y = 0; y < desc.Height; y++ )
	{
		DWORD* pPixel = (DWORD*)pBits;

		for( DWORD x = 0; x < desc.Width; x++ )
		{
			// ARGB
			array[x][y] = *pPixel++;
		}
		pBits += d3dlr.Pitch;
	}


	//unlock texture
	tex->UnlockRect( 0 );
}


//////////////////////////////////////////////////////////////////////////////
// CBaseMap8 implementation /////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

CBaseMap8::CBaseMap8()
{
   m_Format = D3DFMT_A8R8G8B8;
   m_dwLevels = 0;
}

CBaseMap8::~CBaseMap8()
{
}

//////////////////////////////////////////////////////////////////////////////
// CFnMap8 implementation /////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

CFnMap8::CFnMap8()
{
   m_dwWidth = m_dwHeight = 1;
}

VOID WINAPI CFnMap8::Fill2DWrapper(D3DXVECTOR4* pOut, D3DXVECTOR2* pTexCoord, D3DXVECTOR2* pTexelSize, LPVOID pData)
{
    CFnMap8* map = (CFnMap8*)pData;
    const D3DXCOLOR& c = map->Function( pTexCoord, pTexelSize );
    *pOut = D3DXVECTOR4((const float*)c);
}

HRESULT CFnMap8::Initialize()
{
	texture.Destroy(); // If loaded before
   HRESULT hr;

   texture.CreateBlank(TT_DIFFUSE,m_Format,m_dwWidth,m_dwHeight);

   if (FAILED (hr = D3DXFillTexture(texture.GetTexture(), (LPD3DXFILL2D)Fill2DWrapper, (void*)this) ) )
       return hr;

   return S_OK;
}

//////////////////////////////////////////////////////////////////////////////
// CVolumeMap8 implementation /////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
Texture src;
DWORD array[SIZE][SIZE];


CVolumeMap8::CVolumeMap8()
{
   m_dwWidth = m_dwHeight = m_dwDepth = 1;
}

VOID WINAPI CVolumeMap8::Fill3DWrapper(D3DXVECTOR4* pOut, D3DXVECTOR3* pTexCoord, D3DXVECTOR3* pTexelSize, LPVOID pData)
{
    CVolumeMap8* map = (CVolumeMap8*)pData;
    const D3DXCOLOR& c = map->Function( pTexCoord, pTexelSize );
    *pOut = D3DXVECTOR4((const float*)c);
}

HRESULT CVolumeMap8::Initialize()
{
	texture.Destroy(); // If loaded before

	if(!src.IsValid()){ // If unloaded
		if(!src.Load("Attenuate3D.dds"))
			Error("Couldn't find Attenuate3D.dds, unfortunately this texture is vital.\nEnsure you are running from the correct directory and have the media extracted to the correct subdirectory.");
		ConvertToArray(array,src.GetTexture());
	}

   texture.CreateBlank(TT_VOLUME,D3DFMT_L8,m_dwWidth,m_dwHeight,m_dwDepth);
   HRESULT hr;

   if (FAILED (hr = D3DXFillVolumeTexture((LPDIRECT3DVOLUMETEXTURE9)texture.GetTexture(), (LPD3DXFILL3D)Fill3DWrapper, (void*)this) ) )
      Error("FILL VOLUME MAP FAILED");

   // Not worth caching (32x32x32)
   //D3DXSaveTextureToFile("Attenuation.DDS",D3DXIFF_DDS,texture.GetTexture(),NULL);
   //texture.Load("VOLUMEMAP.DDS",0,0,1,1,0,TT_VOLUME);

   return S_OK;
}

//////////////////////////////////////////////////////////////////////////////
// CCubeMap8 implementation /////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

CCubeMap8::CCubeMap8()
{
   m_dwSize = 1;
}

VOID WINAPI CCubeMap8::Fill3DWrapper(D3DXVECTOR4* pOut, D3DXVECTOR3* pTexCoord, D3DXVECTOR3* pTexelSize, LPVOID pData)
{
    CCubeMap8* map = (CCubeMap8*)pData;
   const D3DXCOLOR& c = map->Function( pTexCoord, pTexelSize );
    *pOut = D3DXVECTOR4((const float*)c);
}


HRESULT CCubeMap8::Initialize()
{
	
	//if(texture.CurrentFrame == -1)

	//texture.Load("CUBEMAP.DDS",0,0,1,1,0,TT_CUBE);

	texture.Destroy(); // If loaded before

	texture.CreateBlank(TT_CUBE,m_Format,m_dwSize,0,0);

	HRESULT hr;
	if (FAILED (hr = D3DXFillCubeTexture((LPDIRECT3DCUBETEXTURE9)texture.GetTexture(), (LPD3DXFILL3D)Fill3DWrapper, (void*)this) ) )
		Error("FILL CUBE MAP FAILED : %s",DXGetErrorString9(hr));

	D3DXFilterTexture(texture.GetTexture(),NULL, D3DX_DEFAULT , D3DX_DEFAULT );

	//D3DXSaveTextureToFile("Normalizer.DDS",D3DXIFF_DDS,texture.GetTexture(),NULL);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////
// CNormalizeMap8 implementation /////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

CNormalizerMap8::CNormalizerMap8()
{
   m_dwSize = 256;
}


D3DXCOLOR CNormalizerMap8::Function(D3DXVECTOR3* p, D3DXVECTOR3* s)
{
    D3DXVec3Normalize(p, p);
    *p /= 2;
    *p += D3DXVECTOR3(.5, .5, .5);
    return D3DXCOLOR(p->x, p->y, p->z, 1);
}




//-----------------------------------------------------------------------------
// Name: CFalloffMap()
// Desc: A Volumetric Texture used for point light falloff.
//-----------------------------------------------------------------------------


CFalloffMap::CFalloffMap(UINT size)
{
    m_Format = D3DFMT_A8;
    m_dwWidth = m_dwHeight = m_dwDepth = size;
}

// To build 3D falloff from 2D texture:
// Need to pick an axis to rotate around X, Y, or Z
// E.G. rotation around Y would mean two planes will be exactly the 2D map
// Planes = X=.5 Z =.5

// If we rotate around Y axis then the Y component is always the same
// We just get X lookup from either X or Z

D3DXCOLOR CFalloffMap::Function(D3DXVECTOR3* p, D3DXVECTOR3* s)
{
   // Adjust to be 0 -> 1
 /*  FLOAT x = (p->x-(1.0f/m_dwWidth)/2)*m_dwWidth/(m_dwWidth-1.0f);
   FLOAT y = (p->y-(1.0f/m_dwHeight)/2)*m_dwHeight/(m_dwHeight-1.0f);
   FLOAT z = (p->z-(1.0f/m_dwDepth)/2)*m_dwDepth/(m_dwDepth-1.0f);
   
   // Signed scaling. So range is -1 to 1
   x -= 0.5f; //x/=2;
   y -= 0.5f; //y/=2;
   z -= 0.5f; //z/=2;

   FLOAT distance = (float)(x*x + y*y + z*z);
   if (distance == 0) return D3DXCOLOR(1, 1, 1, 1);
   //			SIZE				 falloff
   float f = ((1.0f/16.0f)/distance - 1.0f/16.0f)/8.f;
   FLOAT falloff = min(1.0f, max(0.0f, f )); // Clamp to 0..1
   return D3DXCOLOR(falloff,falloff,falloff,falloff);
	// Actual working linear attenuation
	/*float xx = fabsf((x-256)/512.f)*4.f; // 2 = linear atten
	float yy = fabsf((y-256)/512.f)*4.f;
	float zz = fabsf((z-256)/512.f)*4.f;
	float dd =  sqrtf(xx*xx+yy*yy+zz*zz); /// sqrtf(512*512+512*512+512*512);
	// how to convert length dd to 1 to 1 linear
	

	if(dd>1) dd = 1;
	if(dd<0) dd = 0;*/
	//return D3DXCOLOR(1-dd,1-dd,1-dd,1-dd);

    // Convert 2D to 3D. Rotation axis is used for 1 dimension, 
	// and other dimension is length of other two axes from center

	float fx = p->x - 0.5f;
	float fz = p->z - 0.5f;
	float xz = sqrt(fx*fx + fz*fz);

	xz += 0.5f;

	// Clamp texture max. At edges length will be sqrt(size*size + size*size) which is > size
	if(xz > 1)	xz = 1;

	return array[(int)(p->y*SIZE)][(int)(xz*SIZE)];
}



//-----------------------------------------------------------------------------
// Name: CFalloffMap()
// Desc: A Texture used for point light falloff.
//-----------------------------------------------------------------------------
C1DFalloffMap::C1DFalloffMap(DWORD size)
{
    m_dwWidth = size;
    m_Format = D3DFMT_A8R8G8B8;
}

D3DXCOLOR C1DFalloffMap::Function(D3DXVECTOR2* p, D3DXVECTOR2* s)
{
    // input is p->x = (Distance^2/Attenuation^2)
   FLOAT distance = (p->x-(1.0f/m_dwWidth)/2)*m_dwWidth/(m_dwWidth-1.0f);
   if (distance == 0) return D3DXCOLOR(1, 1, 1, 1);
   FLOAT falloff = (1.0f/16.0f)/distance - 1.0f/16.0f;
   FLOAT falloff1 = min(1.0f, max(0.0f, falloff/16.0f ) );
   FLOAT falloff2 = min(1.0f, max(0.0f, falloff/4.0f ) );
   return D3DXCOLOR(falloff1, falloff2, falloff1, falloff2);
}

//-----------------------------------------------------------------------------
// Name: CSpecularMap()
// Desc: A Texture used for specular lights.
//-----------------------------------------------------------------------------
CSpecularMap::CSpecularMap(DWORD size)
{
    m_dwWidth = size;
    m_Format = D3DFMT_L8;
}

D3DXCOLOR CSpecularMap::Function(D3DXVECTOR2* p, D3DXVECTOR2* s)
{
    if (p->x == 0 || (1/s->x) <= 1)return D3DXCOLOR(0, 0, 0, 0);
    // input is p->x = saturate(N.H * N.H)
    FLOAT fNH2 = (p->x-(1/m_dwWidth)/2)*m_dwWidth/(m_dwWidth-1);  // Rescale to be 0 to 1.
    FLOAT fNH  = sqrtf(fNH2);
    FLOAT fK   = 16;
    FLOAT fNHK = powf(fNH,fK);
    FLOAT fi = min(1.0f, max(0.0f, fNHK*.6f) );
    return D3DXCOLOR(fi, fi, fi, fi);
}


//-----------------------------------------------------------------------------
// Name: CPointFalloffMap()
// Desc: A Texture used for point light falloff.
//-----------------------------------------------------------------------------
CPointFalloffMap::CPointFalloffMap(DWORD size)
{
    m_dwWidth = size;
    m_Format = D3DFMT_A8R8G8B8;
}

D3DXCOLOR CPointFalloffMap::Function(D3DXVECTOR2* p, D3DXVECTOR2* s)
{
// BLAH
	return D3DXCOLOR(0,0,0,0);
}
