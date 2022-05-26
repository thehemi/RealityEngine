/*********************************************************************NVMH4****
Path:  NVSDK\Common\src\nv_gui
File:  bitmapctrl.cpp

Copyright NVIDIA Corporation 2002
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED
*AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS
BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES
WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS,
BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS)
ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS
BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.



Comments:





******************************************************************************/

// bitmapctrl.cpp : implementation file
//

#include "stdafx.h"
#include "nv_gui\gui.h"
#include "nv_gui\bitmapctrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBitmapCtrl

CBitmapCtrl::CBitmapCtrl()
{
    LoadDefault();
}

CBitmapCtrl::~CBitmapCtrl()
{
}


BEGIN_MESSAGE_MAP(CBitmapCtrl, CWnd)
	//{{AFX_MSG_MAP(CBitmapCtrl)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBitmapCtrl message handlers

void CBitmapCtrl::LoadDefault() 
{
    m_Bitmap.DeleteObject();
  	VERIFY (m_Bitmap.Attach (::LoadImage (::AfxFindResourceHandle(
	MAKEINTRESOURCE (IDB_DEFAULT), RT_BITMAP),
	MAKEINTRESOURCE (IDB_DEFAULT), IMAGE_BITMAP, 0, 0,
	(LR_DEFAULTSIZE| LR_CREATEDIBSECTION))));
    m_Bitmap.SetBitmapDimension(128,128);

    m_Width = 0;
    m_Height = 0;
    m_Depth = 0;
    m_Format = "";
    m_Compression = "";
    m_Cube = FALSE;
    m_NumMipMaps = 0;
    m_NumBitsPerPixels = 0;

    if (IsWindow(GetSafeHwnd()))
        Invalidate();
}

void CBitmapCtrl::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
    CBitmap * pOldBitmap;
    CDC memdc;
    CRect rect;

    GetClientRect(rect);

    memdc.CreateCompatibleDC( &dc );
	
    pOldBitmap = memdc.SelectObject( &m_Bitmap );

    CSize size = m_Bitmap.GetBitmapDimension();

    dc.StretchBlt(0,0,rect.Width(), rect.Height(), &memdc, 0, 0, size.cx, size.cy, SRCCOPY);

    memdc.SelectObject( pOldBitmap );
}


const char * ConvertILCompressionToString(const ILint& compress)
{
	switch(compress)
	{
		case IL_COMPRESS_NONE: 
			return "Uncompressed";
		case IL_COMPRESS_RLE:
			return "RLE";		
		case IL_COMPRESS_LZO:
			return "LZO";
		case IL_COMPRESS_ZLIB:
			return "ZLIB";
		case IL_LUMINANCE:
			return "Luminance";
		case IL_DXT1:
			return "DXT1";
		case IL_DXT2:
			return "DXT2";
		case IL_DXT3:
			return "DXT3";
		case IL_DXT4:
			return "DXT4";
		case IL_DXT5:
			return "DXT5";
        case IL_DXT_NO_COMP:
            return "None";
		default:
			return "Unknown";
	}		
}

const char * ConvertILFormatToString(const ILint& format)
{
	switch(format)
	{
		case IL_RGB: 
			return "RGB";
		case IL_RGBA:
			return "RGBA";		
		case IL_BGR:
			return "BGR";
		case IL_BGRA:
			return "BGRA";
		case IL_LUMINANCE:
			return "Luminance";
        case IL_COLOR_INDEX:
            return "Paletted";
		default:
			return "Unknown";
	}		
}

unsigned int ConvertILFormatToBitCount(const ILint& format)
{
	switch(format)
	{
		case IL_RGB: 
			return 24;
		case IL_RGBA:
			return 32;		
		case IL_BGR:
			return 24;
		case IL_BGRA:
			return 32;
		case IL_LUMINANCE:
			return 8;
		default:
			return 1;
	}		
}

void CBitmapCtrl::SetBitmapFile(const char * filename)
{
    m_Filename = filename;

   	ilGetError(); // temporary till il init bug is fixed.
	
    // Generate the main image name to use.
	unsigned int imgId;	
	ilGenImages(1, &imgId);
	
    // Bind this image name.
	ilBindImage(imgId);

    // Set origin location:
	ilEnable(IL_ORIGIN_SET);
    ilSetInteger(IL_KEEP_DXTC_DATA,IL_TRUE);
	ilOriginFunc(IL_ORIGIN_LOWER_LEFT);

	// Loads the image specified by File into the ImgId image.
	ilLoadImage((char* const)filename);
	if (ilGetError() != IL_NO_ERROR)
    {
        LoadDefault();
        return;
    }
		

	// Check if its a cube map:
    if (ilGetInteger(IL_NUM_IMAGES) < 0)
    {
        LoadDefault();
        return;
    }

	ilBindImage(imgId);

    m_Format = ConvertILFormatToString(ilGetInteger(IL_IMAGE_FORMAT));
    m_Compression = ConvertILCompressionToString(ilGetInteger(IL_DXTC_DATA_FORMAT));
    m_Cube = ilGetInteger(IL_NUM_IMAGES) ? TRUE : FALSE;

    m_Width = ilGetInteger(IL_IMAGE_WIDTH);
	m_Height = ilGetInteger(IL_IMAGE_HEIGHT);
    m_Depth = ilGetInteger(IL_IMAGE_DEPTH);
    m_NumMipMaps = ilGetInteger(IL_NUM_MIPMAPS);
    m_NumBitsPerPixels = ilGetInteger(IL_IMAGE_BITS_PER_PIXEL);

    ilConvertImage(IL_BGRA,IL_UNSIGNED_BYTE);
    iluImageParameter(ILU_FILTER,ILU_BILINEAR);
    int resize = 128;
    iluScale(resize,resize,1);
		
    if (ilGetError() != IL_NO_ERROR)
    {
        LoadDefault();
        return;
    }

	unsigned int ExternalFormat = ConvertILFormatToBitCount(ilGetInteger(IL_IMAGE_FORMAT));
	void * pData = ilGetData();

    m_Bitmap.DeleteObject();

    CClientDC dc(this);
    BITMAPINFO bmInfo;
    memset(&bmInfo,0,sizeof(BITMAPINFO));
    bmInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmInfo.bmiHeader.biWidth = resize;
    bmInfo.bmiHeader.biHeight = resize;
    bmInfo.bmiHeader.biPlanes = 1;
    bmInfo.bmiHeader.biBitCount = 32;
    bmInfo.bmiHeader.biCompression = BI_RGB;
    bmInfo.bmiHeader.biSizeImage = ((((bmInfo.bmiHeader.biWidth * bmInfo.bmiHeader.biBitCount) + 31) & ~31) >> 3) * bmInfo.bmiHeader.biHeight;

    HBITMAP hBitmap = ::CreateDIBitmap(dc.GetSafeHdc(),&bmInfo.bmiHeader,CBM_INIT,pData,&bmInfo,DIB_RGB_COLORS);
    m_Bitmap.Attach(hBitmap);
    m_Bitmap.SetBitmapDimension(resize,resize);

	// Reset origin location:
	ilDisable(IL_ORIGIN_SET);

    ilDeleteImages(1,&imgId);

    Invalidate();
}