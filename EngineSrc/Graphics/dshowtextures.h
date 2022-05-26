//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
/// Desc: DirectShow code - adds support for DirectShow videos playing 
///       on a DirectX 9.0 texture surface. 
///
//====================================================================================
typedef char TCHAR, *PTCHAR; 

#include <streams.h>
#include <atlbase.h>

//-----------------------------------------------------------------------------
/// Define GUID for Texture Renderer
/// {71771540-2017-11cf-AE26-0020AFD79767}
//-----------------------------------------------------------------------------
struct __declspec(uuid("{71771540-2017-11cf-ae26-0020afd79767}")) CLSID_TextureRenderer;



//-----------------------------------------------------------------------------
/// CTextureRenderer Class Declarations
//-----------------------------------------------------------------------------
class CTextureRenderer : public CBaseVideoRenderer
{
public:
    CTextureRenderer(LPUNKNOWN pUnk,HRESULT *phr);
    virtual ~CTextureRenderer();

	D3DFORMAT               g_TextureFormat; // Texture format

public:

	/// Our texture
	LPDIRECT3DTEXTURE9      m_pTexture;   

	/// Format acceptable?
    HRESULT CheckMediaType(const CMediaType *pmt );    
	/// Video format notification
    HRESULT SetMediaType(const CMediaType *pmt );    
	/// New video sample
    HRESULT DoRenderSample(IMediaSample *pMediaSample); 
    /// Restore for non-managed textures
	HRESULT Restore();
	/// Lost for non-managed textures
	HRESULT OnLostDevice();
    BOOL m_bUseDynamicTextures;
	/// Video width
    LONG m_lVidWidth;   
	/// Video Height
    LONG m_lVidHeight;  
	/// Video Pitch
    LONG m_lVidPitch;  


	static HRESULT InitDShowTextureRenderer(CHAR* SOURCE_FILE, CTextureRenderer*& pCTR);

	void CheckMovieStatus(void);
	static void CleanupDShow(void);

	HRESULT AddToROT(IUnknown *pUnkGraph); 
	void RemoveFromROT(void);
};


