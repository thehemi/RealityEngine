//=========== Copyright (c) 2003, Tim Johnson. All rights reserved. ===========
//
//=============================================================================
#include "stdafx.h"
#include "RenderWrap.h"


BYTE* image; // Temp array holding greyscale height map
int bumpWidth, bumpHeight;
float scale = 1;

// Categories of normal decompression which happens in the pixel shader
#define SCALE_BIAS_TRIPLE                 0
#define TWOS_COMP_TRIPLE                  1
#define SCALE_BIAS_DOUBLE                 2
#define TWOS_COMP_DOUBLE                  3
#define NUM_PIXEL_SHADERS                 4
#define DXT5_BIAS_DOUBLE                  5

//////////////////////////////////////////////////////////////////////////
// Get a pixel from the image.
//////////////////////////////////////////////////////////////////////////
void 
ReadPixel (BYTE *image, BYTE *pix, int x, int y)
{
	x = x % bumpWidth;
	y = y % bumpHeight;

   *pix = *(image+bumpWidth*y+x);
}


void HeightmapToArray(const char* name, BYTE*& array, int& width, int& height){
	LPDIRECT3DTEXTURE9 tex;
	HRESULT hr;

	// Load the image as greyscale
	if((hr=D3DXCreateTextureFromFileEx( RenderWrap::dev, name, 
			D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT , 0, D3DFMT_A8R8G8B8 , 
				D3DPOOL_SYSTEMMEM , D3DX_DEFAULT, 
				D3DX_DEFAULT, 0, NULL, NULL, &tex ))!=S_OK)
			Warning("Couldn't create texture: %s (D3DErr: %s)", name,DXGetErrorString9(hr));

	// Lock the surface and read the greyscale
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

	width = desc.Width;
	height = desc.Height;

	array = new BYTE[width*height];

	for( DWORD y = 0; y < desc.Height; y++ )
	{
		DWORD* pPixel = (DWORD*)pBits;

		for( DWORD x = 0; x < desc.Width; x++ )
		{
			// ARGB
			array[y*width + x] = COLOR_GETRED(*pPixel);
			*pPixel++;
		}
		pBits += d3dlr.Pitch;
	}


	//unlock texture
	tex->UnlockRect( 0 );
	SAFE_RELEASE(tex);
}

#define ToFlt(pix) (((float)pix)/255.0f)
#define ToBYTE(pix) ((BYTE)(pix*255.0f))
#define FLOAT_ARGB(A,R,G,B) (COLOR_ARGB(ToBYTE(A),ToBYTE(R),ToBYTE(G),ToBYTE(B)))

//
// WARNING: This does not support DXT textures apparently
//
VOID NormalFill (FloatColor& col, int x, int y, LPVOID pData)
{
   // pData points to PackType
   int PackType =  *((int*)pData);

   // Sobel the image to get normals.
   float dX, dY, nX, nY, nZ, oolen;

   int off = 1;

   int gWidth = bumpWidth;
   int gHeight = bumpHeight;

   BYTE pix;

   // Do Y Sobel filter
   ReadPixel(image, &pix, (x-1+gWidth) , (y+1) );
   dY  = ToFlt(pix) * -1.0f;

   ReadPixel(image, &pix,   x   , (y+1) );
   dY += ToFlt(pix) * -2.0f;

   ReadPixel(image, &pix, (x+1) , (y+1) );
   dY += ToFlt(pix) * -1.0f;

   ReadPixel(image, &pix, (x-1+gWidth) , (y-1+gHeight) );
   dY += ToFlt(pix) *  1.0f;

   ReadPixel(image, &pix,   x   , (y-1+gHeight) );
   dY += ToFlt(pix) *  2.0f;

   ReadPixel(image, &pix, (x+1) , (y-1+gHeight) );
   dY += ToFlt(pix) *  1.0f;

   // Do X Sobel filter
   ReadPixel(image, &pix, (x-1+gWidth) , (y-1+gHeight) );
   dX  = ToFlt(pix) * -1.0f;

   ReadPixel(image, &pix, (x-1+gWidth) ,   y   );
   dX += ToFlt(pix) * -2.0f;

   ReadPixel(image, &pix, (x-1+gWidth) , (y+1) );
   dX += ToFlt(pix) * -1.0f;

   ReadPixel(image, &pix, (x+1) , (y-1+gHeight) );
   dX += ToFlt(pix) *  1.0f;

   ReadPixel(image, &pix, (x+1) ,   y   );
   dX += ToFlt(pix) *  2.0f;

   ReadPixel(image, &pix, (x+1) , (y+1) );
   dX += ToFlt(pix) *  1.0f;

   // Cross Product of components of gradient reduces to
   nX = -dX * scale;
   nY = -dY * scale;
   nZ = 1;

   // Normalize
   oolen = 1.0f/((float) sqrt(nX*nX + nY*nY + nZ*nZ));
   nX *= oolen;
   nY *= oolen;
   nZ *= oolen;

   // Properly pack the normal into the texel based upon the dimension and sign representation
   // NOTE: Must always fill the default alpha channel to 1, so we get a default spec/illum
   switch (PackType)
   {
   case SCALE_BIAS_TRIPLE:
	   col = FloatColor(nX*0.5f + 0.5f, nY*0.5f + 0.5f, nZ*0.5f + 0.5f, 1.0f);
	   break;
   case TWOS_COMP_TRIPLE:
	   col = FloatColor(nX, nY, nZ, 1.0f);
	   break;
   case SCALE_BIAS_DOUBLE:
	   col = FloatColor(nX*0.5f + 0.5f, nY*0.5f + 0.5f, 0.0f, 1.0f);
	   break;
   case TWOS_COMP_DOUBLE:
	   col = FloatColor(nX, nY, 0.0f, 1.0f);
	   break;
   case DXT5_BIAS_DOUBLE:
	   // A=X channel (4/6bit? interpolated). G=Y channel(3? bit explicit)
	   col = FloatColor(nX*0.5f + 0.5f, nY*0.5f + 0.5f, 1, nX*0.5f + 0.5f);
	   break;

   }

} // GetBumpMapFromHeightMap

#define PixType DWORD
void FillTex(LPDIRECT3DTEXTURE9 tex, LPVOID pData){
	try{
	D3DSURFACE_DESC desc;
	tex->GetLevelDesc(0,&desc);

	D3DLOCKED_RECT LockedRect;
	DXASSERT(tex->LockRect( 0, &LockedRect, 0, 0 ));

	PixType* pBits = (PixType*)LockedRect.pBits;

	for( UINT y=0; y<desc.Height; y++ )
	{
		PixType* pPixel = (PixType*)pBits;

		for( UINT x=0; x < desc.Width; x++ )
		{
			FloatColor col;
			NormalFill(col,x,y,pData);
			*pPixel++ = col.DWORDColor();
		}
		pBits += LockedRect.Pitch / sizeof(PixType);
	}

	tex->UnlockRect( 0 );
	}
	catch(...){
		Error("FillTex() failed. It means this function is faulty");
	}
}


void GenerateBumpMap(const TCHAR* heightMap, LPDIRECT3DTEXTURE9& normalMap, D3DFORMAT format, float bumpScale){
	scale = bumpScale;

	int packType;

	switch(format){
		case D3DFMT_A16B16G16R16:
			packType = SCALE_BIAS_TRIPLE;
			break;
		case D3DFMT_Q8W8V8U8:
			packType = TWOS_COMP_TRIPLE;
			break;
		case D3DFMT_A2W10V10U10:
			packType = TWOS_COMP_TRIPLE;
			break;
		case D3DFMT_V8U8:
			packType = TWOS_COMP_DOUBLE;
			break;
		case D3DFMT_DXT3:
			packType = SCALE_BIAS_TRIPLE;
			break;
		case D3DFMT_DXT5:
			packType = DXT5_BIAS_DOUBLE;
			break;
		case D3DFMT_DXT1:
			packType = SCALE_BIAS_TRIPLE;
			break;
		case D3DFMT_A8R8G8B8:
			packType = SCALE_BIAS_TRIPLE;
			break;
		default:
			Error("GenerateBumpMap: Input format is not recognized");
	}

	HeightmapToArray(heightMap,image,bumpWidth,bumpHeight);

//	DXASSERT(D3DXCreateTexture (RenderWrap::dev, bumpWidth, bumpHeight, D3DX_DEFAULT, 0,format, D3DPOOL_MANAGED , &normalMap));
	//DXASSERT(D3DXFillTexture (normalMap, NormalFill, (LPVOID) &packType));
	FillTex(normalMap, (LPVOID) &packType);

	delete[] image;
}
