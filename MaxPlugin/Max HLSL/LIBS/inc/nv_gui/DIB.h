/*********************************************************************NVMH4****
Path:  NVSDK\Common\include\nv_gui
File:  DIB.h

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

#ifndef __DIB
#define __DIB

namespace DIBUtils
{

class CDIB 
{
public:
	enum BitmapType
	{
		BMP,
		GIF,
		TIFF
	};
	CDIB(HANDLE hDib=NULL,int nBits=8);
	virtual ~CDIB();
	CDIB& operator=(CDIB& dib);
	BOOL IsValid() { return (m_pVoid && Width() && Height());};
	void UseGamma(float fg,BOOL bUse=TRUE);
	BOOL CreateFromHandle(HANDLE hDib,int nBits);
	BOOL Create(int width,int height,int bits=24);
	BOOL Create(BITMAPINFOHEADER& bmInfo);
	BOOL CopyDIB(CDIB& dib);
	BOOL OpenDIB(CString& fileName);
	BOOL SaveDIB(CString& fileName,BitmapType type);
	void ReplaceColor(unsigned char oldColor,unsigned char newColor);
	HANDLE GetDIBits(int nStartX=-1,int nStartY=-1,int nCx=-1,int nCy=-1);
	CBitmap *GetBitmap(CDC& dc);
	CBitmap *GetTempBitmap(CDC& dc);
	DWORD GetDIBSize();
	int GetPaletteSize(BITMAPINFOHEADER& bmInfo);
	int GetPaletteSize();
	int CountColors();
	int EnumColors(BYTE *colors);
	void InitDIB(COLORREF color);
	void CopyLine(int source,int dest);
	void DestroyDIB();
	void SetPalette(unsigned char *palette);
	void SetPalette(RGBQUAD *pRGB);
	COLORREF PaletteColor(int index);
	void SetPixel(int x,int y,COLORREF color);
	void SetPixel8(int x,int y,unsigned char color);
	COLORREF GetPixel(int x,int y);
	void GetPixel(UINT x,UINT y,int& pixel);
	void BitBlt(HDC hDest,int nXDest,int nYDest,int nWidth,int nHeight,int xSrc,int ySrc);
	void BitBlt(int nXDest,int nYDest,int nWidth,int nHeight,CDIB& dibSrc,int nSrcX,int nSrcY,BYTE *colors=NULL);
	void StretchBlt(HDC hDest,int nXDest,int nYDest,int nDWidth,int nDHeight,int xSrc,int ySrc,int  nSWidth,int nSHeight);
	void StretchBlt(int nXDest,int nYDest,int nDWidth,int nDHeight,CDIB& dibSrc,int xSrc,int ySrc,int  nSWidth,int nSHeight);
	void ExpandBlt(int nXDest,int nYDest,int xRatio,int yRatio,CDIB& dibSrc,int xSrc,int ySrc,int  nSWidth,int nSHeight);
	void SetFlags(int flag) { m_nFlags = flag; };
	int Height() { return height ; };
	int Width() { return width ; };
	unsigned char *GetLinePtr(int line);	
	inline int GetBitCount() {return m_pInfo->bmiHeader.biBitCount;};
	BOOL Make8Bit(CDIB& dib);
	BOOL SwitchFromOne(CDIB& dib);
	BOOL SwitchFromFour(CDIB& dib);
	BOOL SwitchFrom24(CDIB& dib);
	BOOL SwitchPalette(CDIB& dib);
	int  ClosestColor(RGBQUAD *pRgb);
	LPBITMAPINFO GetBitmapInfo() { return m_pInfo;};
	static unsigned int Distance(RGBQUAD& rgb1,RGBQUAD& rgb2);
protected :
	HANDLE DIBHandle();
	BOOL OpenBMP(CString& csFileName);
	BOOL OpenGIF(CString& csFileName);
	BOOL OpenTIFF(CString& csFileName);
	BOOL SaveBMP(CString& csFileName);
	BOOL SaveGIF(CString& csFileName);
	BOOL SaveTIFF(CString& csFileName);
	void CreateGammaCurve();
	void Expand(int nXDest,int nYDest,int xRatio,int yRatio,CDIB& dibSrc,int xSrc,int ySrc,int  nSWidth,int nSHeight);
	unsigned char *m_pBits;
	PBITMAPINFO m_pInfo;
	RGBQUAD *m_pRGB;
	void *m_pVoid; 
	BYTE **m_pLinePtr;
	int height,bytes,width,m_nBits;
	int m_nFlags;
	BOOL m_bUseGamma;
	float m_fGamma;
	float m_fOldGamma;
	unsigned char Gamma[256];
	RGBQUAD CacheQuad[256];
	char CachePtr[256];
};

}; // namespace DIBUtils

#endif
