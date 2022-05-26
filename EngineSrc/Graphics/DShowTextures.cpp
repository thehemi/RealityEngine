//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
// Desc: DirectShow code - adds support for DirectShow videos playing 
//       on a DirectX 9.0 texture surface. 
//
//====================================================================================
#include "stdafx.h"
#include <d3dx9.h>
#include <d3d9.h>
#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>

#include <d3d9types.h>
#include "DShowTextures.h"

#ifdef _DEBUG
#pragma comment(lib,"..\\DShowBase\\Debug\\strmbasd.lib")
#else
#pragma comment(lib,"..\\DShowBase\\Release\\STRMBASE.lib")
#endif
void Msg(TCHAR *szFormat, ...);

CComPtr<IGraphBuilder>  g_pGB;          // GraphBuilder
CComPtr<IMediaControl>  g_pMC;          // Media Control
CComPtr<IMediaPosition> g_pMP;          // Media Position
CComPtr<IMediaEvent>    g_pME;          // Media Event

// Define this if you want to render only the video component with no audio
//
//#define NO_AUDIO_RENDERER

// An application can advertise the existence of its filter graph
// by registering the graph with a global Running Object Table (ROT).
// The GraphEdit application can detect and remotely view the running
// filter graph, allowing you to 'spy' on the graph with GraphEdit.
//
// To enable registration in this sample, define REGISTER_FILTERGRAPH.
//
//#define REGISTER_FILTERGRAPH

//-----------------------------------------------------------------------------
// Global DirectShow pointers
//-----------------------------------------------------------------------------



TCHAR g_achCopy[]     = TEXT("Bitwise copy of the sample");
TCHAR g_achOffscr[]   = TEXT("Using offscreen surfaces and StretchCopy()");
TCHAR g_achDynTextr[] = TEXT("Using Dynamic Textures");
TCHAR* g_pachRenderMethod = NULL;

// Dummy, not used, could use if you care about this stuff
HRESULT UpgradeGeometry( LONG lActualW, LONG lTextureW,
                                LONG lActualH, LONG lTextureH )
{

	return S_OK;
}

//-----------------------------------------------------------------------------
// InitDShowTextureRenderer : Create DirectShow filter graph and run the graph
//-----------------------------------------------------------------------------
HRESULT CTextureRenderer::InitDShowTextureRenderer(CHAR* SOURCE_FILE, CTextureRenderer*& pCTR)
{
    HRESULT hr = S_OK;
    CComPtr<IBaseFilter>    pFSrc;          // Source Filter
    CComPtr<IPin>           pFSrcPinOut;    // Source Filter Output Pin   

    // Create the filter graph
	if(g_pGB.p == NULL)
		if (FAILED(g_pGB.CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC)))
			return E_FAIL;

#ifdef REGISTER_FILTERGRAPH
    // Register the graph in the Running Object Table (for debug purposes)
    AddToROT(g_pGB);
#endif
    
    // Create the Texture Renderer object
    pCTR = new CTextureRenderer(NULL, &hr);
    if (FAILED(hr) || !pCTR)
    {
        Msg(TEXT("Could not create texture renderer object!  hr=0x%x"), hr);
        return E_FAIL;
    }
    
    // Get a pointer to the IBaseFilter on the TextureRenderer, add it to graph
    if (FAILED(hr = g_pGB->AddFilter(pCTR, L"TEXTURERENDERER")))
    {
        Msg(TEXT("Could not add renderer filter to graph!  hr=0x%x"), hr);
        return hr;
    }
    
    // Determine the file to load based on DirectX Media path (from SDK)
    // Use a helper function included in DXUtils.cpp
    TCHAR strFileName[MAX_PATH];
    WCHAR wFileName[MAX_PATH];
    lstrcpyn( strFileName, SOURCE_FILE,MAX_PATH-1 );
    strFileName[MAX_PATH-1] = 0;  // NULL-terminate
    wFileName[MAX_PATH-1] = 0;    // NULL-terminate

    USES_CONVERSION;
    wcsncpy(wFileName, T2W(strFileName), NUMELMS(wFileName));

    // Add the source filter to the graph.
    hr = g_pGB->AddSourceFilter (wFileName, L"SOURCE", &pFSrc);
    
    // If the media file was not found, inform the user.
    if (hr == VFW_E_NOT_FOUND)
    {
        Msg(TEXT("Could not add source filter to graph!  (hr==VFW_E_NOT_FOUND)\r\n\r\n")
            TEXT("This sample reads a media file from the DirectX SDK's media path.\r\n")
            TEXT("Please install the DirectX 9 SDK on this machine."));
        return hr;
    }
    else if(FAILED(hr))
    {
        Msg(TEXT("Could not add source filter to graph!  hr=0x%x"), hr);
        return hr;
    }

    if (FAILED(hr = pFSrc->FindPin(L"Output", &pFSrcPinOut)))
    {
        Msg(TEXT("Could not find output pin!  hr=0x%x"), hr);
        return hr;
    }

#ifdef NO_AUDIO_RENDERER

    // If no audio component is desired, directly connect the two video pins
    // instead of allowing the Filter Graph Manager to render all pins.

    CComPtr<IPin> pFTRPinIn;      // Texture Renderer Input Pin

    // Find the source's output pin and the renderer's input pin
    if (FAILED(hr = pFSrc->FindPin(L"Video Renderer", &pFTRPinIn)))
    {
        Msg(TEXT("Could not find input pin!  hr=0x%x"), hr);
        return hr;
    }

    // Connect these two filters
    if (FAILED(hr = g_pGB->Connect(pCTR->pFSrcPinOut, pFTRPinIn)))
    {
        Msg(TEXT("Could not connect pins!  hr=0x%x"), hr);
        return hr;
    }

#else

    // Render the source filter's output pin.  The Filter Graph Manager
    // will connect the video stream to the loaded CTextureRenderer
    // and will load and connect an audio renderer (if needed).

    if (FAILED(hr = g_pGB->Render(pFSrcPinOut)))
    {
        Msg(TEXT("Could not render source output pin!  hr=0x%x"), hr);
        return hr;
    }

#endif
   
    // Get the graph's media control, event & position interfaces
	if(g_pMC.p == NULL)
	{
		g_pGB.QueryInterface(&g_pMC);
		g_pGB.QueryInterface(&g_pMP);
		g_pGB.QueryInterface(&g_pME);
	}
    
    // Start the graph running;
    if (FAILED(hr = g_pMC->Run()))
    {
        Msg(TEXT("Could not run the DirectShow graph!  hr=0x%x"), hr);
        return hr;
    }


    return S_OK;
}


//-----------------------------------------------------------------------------
// CTextureRenderer destructor
//-----------------------------------------------------------------------------
CTextureRenderer::~CTextureRenderer()
{
	/*if(g_pGB == NULL)
		return;
//	g_pGB->Disconnect (pFSrcPinOut);
	if(g_pGB)
		g_pGB->RemoveFilter(this);
	g_pGB = 0;

	m_pGraph = NULL;
	while(Release());*/
//	SAFE_RELEASE(m_pTexture);	
}


//-----------------------------------------------------------------------------
// CheckMovieStatus: If the movie has ended, rewind to beginning
//-----------------------------------------------------------------------------
void CTextureRenderer::CheckMovieStatus(void)
{
    long lEventCode;
    LONG_PTR lParam1, lParam2;
    HRESULT hr;

    if (!g_pME)
        return;
        
    // Check for completion events
    hr = g_pME->GetEvent(&lEventCode, &lParam1, &lParam2, 0);
    if (SUCCEEDED(hr))
    {
        // If we have reached the end of the media file, reset to beginning
        if (EC_COMPLETE == lEventCode) 
        {
            hr = g_pMP->put_CurrentPosition(0);
        }

        // Free any memory associated with this event
        hr = g_pME->FreeEventParams(lEventCode, lParam1, lParam2);
    }
}


//-----------------------------------------------------------------------------
// CleanupDShow
//-----------------------------------------------------------------------------
void CTextureRenderer::CleanupDShow(void)
{
#ifdef REGISTER_FILTERGRAPH
    // Pull graph from Running Object Table (Debug)
    RemoveFromROT();
#endif

    // Shut down the graph
    if (!(!g_pMC)) g_pMC->Stop();

    if (!(!g_pMC)) g_pMC.Release();
    if (!(!g_pME)) g_pME.Release();
    if (!(!g_pMP)) g_pMP.Release();
    if (!(!g_pGB)) g_pGB.Release();
}
    

//-----------------------------------------------------------------------------
// CTextureRenderer constructor
//-----------------------------------------------------------------------------
CTextureRenderer::CTextureRenderer( LPUNKNOWN pUnk, HRESULT *phr )
                                  : CBaseVideoRenderer(__uuidof(CLSID_TextureRenderer), 
                                    NAME("Texture Renderer"), pUnk, phr),
                                    m_bUseDynamicTextures(FALSE)
{
    // Store and AddRef the texture for our use.
    ASSERT(phr);
    if (phr)
        *phr = S_OK;
}


//-----------------------------------------------------------------------------
// CheckMediaType: This method forces the graph to give us an R8G8B8 video
// type, making our copy to texture memory trivial.
//-----------------------------------------------------------------------------
HRESULT CTextureRenderer::CheckMediaType(const CMediaType *pmt)
{
    HRESULT   hr = E_FAIL;
    VIDEOINFO *pvi=0;
    
    CheckPointer(pmt,E_POINTER);

    // Reject the connection if this is not a video type
    if( *pmt->FormatType() != FORMAT_VideoInfo ) {
        return E_INVALIDARG;
    }
    
    // Only accept RGB24 video
    pvi = (VIDEOINFO *)pmt->Format();

    if(IsEqualGUID( *pmt->Type(),    MEDIATYPE_Video)  &&
       IsEqualGUID( *pmt->Subtype(), MEDIASUBTYPE_RGB24))
    {
        hr = S_OK;
    }
    
    return hr;
}


// Define a function that matches the prototype of LPD3DXFILL3D
VOID WINAPI ColorFill (D3DXVECTOR4* pOut, const D3DXVECTOR2* pTexCoord, 
const D3DXVECTOR2* pTexelSize, LPVOID pData)
{
   *pOut = D3DXVECTOR4(0, 0, 0.0f, 0.0f);
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
HRESULT CTextureRenderer::OnLostDevice()
{
	if( m_bUseDynamicTextures )
    {
		//SAFE_RELEASE(m_pTexture);
	}
	return S_OK;
}
//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
HRESULT CTextureRenderer::Restore()
{
	D3DCAPS9 caps;
	ZeroMemory( &caps, sizeof(D3DCAPS9));
    RenderWrap::dev->GetDeviceCaps( &caps );

	UINT uintWidth=2,uintHeight=2;
	if( caps.TextureCaps & D3DPTEXTURECAPS_POW2 )
    {
        while( (LONG)uintWidth < m_lVidWidth )
        {
            uintWidth = uintWidth << 1;
        }
        while( (LONG)uintHeight < m_lVidHeight )
        {
            uintHeight = uintHeight << 1;
        }
        UpgradeGeometry( m_lVidWidth, uintWidth, m_lVidHeight, uintHeight);
    }
    else
    {
        uintWidth = m_lVidWidth;
        uintHeight = m_lVidHeight;
    }

	if( m_bUseDynamicTextures )
    {
		HRESULT hr;
        hr = RenderWrap::dev->CreateTexture(uintWidth, uintHeight, 1, D3DUSAGE_DYNAMIC, 
                                         D3DFMT_X8R8G8B8,D3DPOOL_DEFAULT,
                                         &m_pTexture, NULL);

		// Fill the texture with black using D3DXFillTexture
		if (FAILED (hr = D3DXFillTexture (m_pTexture, ColorFill, NULL)))
		{
			return hr;
		}


        g_pachRenderMethod = g_achDynTextr;
        if( FAILED(hr))
        {
            m_bUseDynamicTextures = FALSE;
        }
    }
	return S_OK;
}

//-----------------------------------------------------------------------------
// SetMediaType: Graph connection has been made. 
//-----------------------------------------------------------------------------
HRESULT CTextureRenderer::SetMediaType(const CMediaType *pmt)
{
    HRESULT hr;

    UINT uintWidth = 2;
    UINT uintHeight = 2;

    // Retrive the size of this media type
    D3DCAPS9 caps;
    VIDEOINFO *pviBmp;                      // Bitmap info header
    pviBmp = (VIDEOINFO *)pmt->Format();

    m_lVidWidth  = pviBmp->bmiHeader.biWidth;
    m_lVidHeight = abs(pviBmp->bmiHeader.biHeight);
    m_lVidPitch  = (m_lVidWidth * 3 + 3) & ~(3); // We are forcing RGB24

    // here let's check if we can use dynamic textures
    ZeroMemory( &caps, sizeof(D3DCAPS9));
    hr = RenderWrap::dev->GetDeviceCaps( &caps );
    if( caps.Caps2 & D3DCAPS2_DYNAMICTEXTURES )
    {
        m_bUseDynamicTextures = TRUE;
    }

    if( caps.TextureCaps & D3DPTEXTURECAPS_POW2 )
    {
        while( (LONG)uintWidth < m_lVidWidth )
        {
            uintWidth = uintWidth << 1;
        }
        while( (LONG)uintHeight < m_lVidHeight )
        {
            uintHeight = uintHeight << 1;
        }
        UpgradeGeometry( m_lVidWidth, uintWidth, m_lVidHeight, uintHeight);
    }
    else
    {
        uintWidth = m_lVidWidth;
        uintHeight = m_lVidHeight;
    }

	// FIXME: Use dynamic textures, but need to update shader texture pointers!!
	m_bUseDynamicTextures=false;

    // Create the texture that maps to this media type
    hr = E_UNEXPECTED;
    if( m_bUseDynamicTextures )
    {
        hr = RenderWrap::dev->CreateTexture(uintWidth, uintHeight, 1, D3DUSAGE_DYNAMIC, 
                                         D3DFMT_X8R8G8B8,D3DPOOL_DEFAULT,
                                         &m_pTexture, NULL);

		m_pTexture->AddRef(); // HACK: So texture doesn't delete, we do
		// Fill the texture with black using D3DXFillTexture
		if (FAILED (hr = D3DXFillTexture (m_pTexture, ColorFill, NULL)))
		{
			return hr;
		}


        g_pachRenderMethod = g_achDynTextr;
        if( FAILED(hr))
        {
            m_bUseDynamicTextures = FALSE;
        }
    }
    if( FALSE == m_bUseDynamicTextures )
    {
        hr = RenderWrap::dev->CreateTexture(uintWidth, uintHeight, 1, 0, 
                                         D3DFMT_X8R8G8B8,D3DPOOL_MANAGED,
                                         &m_pTexture, NULL);
        g_pachRenderMethod = g_achCopy;
    }
    if( FAILED(hr))
    {
        Msg(TEXT("Could not create the D3DX texture!  hr=0x%x"), hr);
        return hr;
    }

    // CreateTexture can silently change the parameters on us
    D3DSURFACE_DESC ddsd;
    ZeroMemory(&ddsd, sizeof(ddsd));

    if ( FAILED( hr = m_pTexture->GetLevelDesc( 0, &ddsd ) ) ) {
        Msg(TEXT("Could not get level Description of D3DX texture! hr = 0x%x"), hr);
        return hr;
    }


    CComPtr<IDirect3DSurface9> pSurf; 

    if (SUCCEEDED(hr = m_pTexture->GetSurfaceLevel(0, &pSurf)))
        pSurf->GetDesc(&ddsd);

    // Save format info
    g_TextureFormat = ddsd.Format;

    if (g_TextureFormat != D3DFMT_X8R8G8B8 &&
        g_TextureFormat != D3DFMT_A1R5G5B5) {
        Msg(TEXT("Texture is format we can't handle! Format = 0x%x"), g_TextureFormat);
        return VFW_E_TYPE_NOT_ACCEPTED;
    }

    return S_OK;
}


//-----------------------------------------------------------------------------
// DoRenderSample: A sample has been delivered. Copy it to the texture.
//-----------------------------------------------------------------------------
HRESULT CTextureRenderer::DoRenderSample( IMediaSample * pSample )
{
    BYTE  *pBmpBuffer, *pTxtBuffer; // Bitmap buffer, texture buffer
    LONG  lTxtPitch;                // Pitch of bitmap, texture

    BYTE  * pbS = NULL;
    DWORD * pdwS = NULL;
    DWORD * pdwD = NULL;
    UINT row, col, dwordWidth;

	if(RenderDevice::Instance()->IsDeviceLost() || !m_pTexture)
		return S_OK;
    
    CheckPointer(pSample,E_POINTER);
    CheckPointer(m_pTexture,E_UNEXPECTED);

    // Get the video bitmap buffer
    pSample->GetPointer( &pBmpBuffer );

    // Lock the Texture
    D3DLOCKED_RECT d3dlr;
    if( m_bUseDynamicTextures )
    {
		try{
        if( FAILED(m_pTexture->LockRect(0, &d3dlr, 0, D3DLOCK_DISCARD)))
            return E_FAIL;
		}
		catch(...){
			m_pTexture = NULL;
			return E_FAIL;
		}
    }
    else
    {
		try
		{
			if(RenderDevice::Instance()->IsDeviceLost() || !m_pTexture)
				return S_OK;

			if (FAILED(m_pTexture->LockRect(0, &d3dlr, 0, 0)))
				return E_FAIL;
		}
		catch(...){
			m_pTexture = NULL;
			return E_FAIL;
		}
    }
    // Get the texture buffer & pitch
    pTxtBuffer = static_cast<byte *>(d3dlr.pBits);
    lTxtPitch = d3dlr.Pitch;
    
    
    // Copy the bits    

    if (g_TextureFormat == D3DFMT_X8R8G8B8) 
    {
        // Instead of copying data bytewise, we use DWORD alignment here.
        // We also unroll loop by copying 4 pixels at once.
        //
        // original BYTE array is [b0][g0][r0][b1][g1][r1][b2][g2][r2][b3][g3][r3]
        //
        // aligned DWORD array is     [b1 r0 g0 b0][g2 b2 r1 g1][r3 g3 b3 r2]
        //
        // We want to transform it to [ff r0 g0 b0][ff r1 g1 b1][ff r2 g2 b2][ff r3 b3 g3]
        // below, bitwise operations do exactly this.

        dwordWidth = m_lVidWidth / 4; // aligned width of the row, in DWORDS
                                      // (pixel by 3 bytes over sizeof(DWORD))

        for( row = 0; row< (UINT)m_lVidHeight; row++)
        {
            pdwS = ( DWORD*)pBmpBuffer;
            pdwD = ( DWORD*)pTxtBuffer;

            for( col = 0; col < dwordWidth; col ++ )
            {
                pdwD[0] =  pdwS[0] | 0xFF000000;
                pdwD[1] = ((pdwS[1]<<8)  | 0xFF000000) | (pdwS[0]>>24);
                pdwD[2] = ((pdwS[2]<<16) | 0xFF000000) | (pdwS[1]>>16);
                pdwD[3] = 0xFF000000 | (pdwS[2]>>8);
                pdwD +=4;
                pdwS +=3;
            }

            // we might have remaining (misaligned) bytes here
            pbS = (BYTE*) pdwS;
            for( col = 0; col < (UINT)m_lVidWidth % 4; col++)
            {
                *pdwD = 0xFF000000     |
                        (pbS[2] << 16) |
                        (pbS[1] <<  8) |
                        (pbS[0]);
                pdwD++;
                pbS += 3;           
            }

            pBmpBuffer  += m_lVidPitch;
            pTxtBuffer += lTxtPitch;
        }// for rows
    }

    if (g_TextureFormat == D3DFMT_A1R5G5B5) 
    {
        for(int y = 0; y < m_lVidHeight; y++ ) 
        {
            BYTE *pBmpBufferOld = pBmpBuffer;
            BYTE *pTxtBufferOld = pTxtBuffer;   

            for (int x = 0; x < m_lVidWidth; x++) 
            {
                *(WORD *)pTxtBuffer = (WORD)
                    (0x8000 +
                    ((pBmpBuffer[2] & 0xF8) << 7) +
                    ((pBmpBuffer[1] & 0xF8) << 2) +
                    (pBmpBuffer[0] >> 3));

                pTxtBuffer += 2;
                pBmpBuffer += 3;
            }

            pBmpBuffer = pBmpBufferOld + m_lVidPitch;
            pTxtBuffer = pTxtBufferOld + lTxtPitch;
        }
    }

    // Unlock the Texture
    if (FAILED(m_pTexture->UnlockRect(0)))
        return E_FAIL;
    
    return S_OK;
}


#ifdef REGISTER_FILTERGRAPH

//-----------------------------------------------------------------------------
// Running Object Table functions: Used to debug. By registering the graph
// in the running object table, GraphEdit is able to connect to the running
// graph. This code should be removed before the application is shipped in
// order to avoid third parties from spying on your graph.
//-----------------------------------------------------------------------------
DWORD dwROTReg = 0xfedcba98;

HRESULT CTextureRenderer::AddToROT(IUnknown *pUnkGraph) 
{
    IMoniker * pmk;
    IRunningObjectTable *pROT;
    if (FAILED(GetRunningObjectTable(0, &pROT))) {
        return E_FAIL;
    }

    WCHAR wsz[256];
    wsprintfW(wsz, L"FilterGraph %08x  pid %08x\0", (DWORD_PTR) 0, GetCurrentProcessId());

    HRESULT hr = CreateItemMoniker(L"!", wsz, &pmk);
    if (SUCCEEDED(hr)) 
    {
        // Use the ROTFLAGS_REGISTRATIONKEEPSALIVE to ensure a strong reference
        // to the object.  Using this flag will cause the object to remain
        // registered until it is explicitly revoked with the Revoke() method.
        //
        // Not using this flag means that if GraphEdit remotely connects
        // to this graph and then GraphEdit exits, this object registration 
        // will be deleted, causing future attempts by GraphEdit to fail until
        // this application is restarted or until the graph is registered again.
        hr = pROT->Register(ROTFLAGS_REGISTRATIONKEEPSALIVE, pUnkGraph, 
                            pmk, &dwROTReg);
        pmk->Release();
    }

    pROT->Release();
    return hr;
}


void CTextureRenderer::RemoveFromROT(void)
{
    IRunningObjectTable *pirot=0;

    if (SUCCEEDED(GetRunningObjectTable(0, &pirot))) 
    {
        pirot->Revoke(dwROTReg);
        pirot->Release();
    }
}

#endif


//-----------------------------------------------------------------------------
// Msg: Display an error message box if needed
//-----------------------------------------------------------------------------
void Msg(TCHAR *szFormat, ...)
{
    TCHAR szBuffer[1024];  // Large buffer for long filenames or URLs
    const size_t NUMCHARS = sizeof(szBuffer) / sizeof(szBuffer[0]);
    const int LASTCHAR = NUMCHARS - 1;

    // Format the input string
    va_list pArgs;
    va_start(pArgs, szFormat);

    // Use a bounded buffer size to prevent buffer overruns.  Limit count to
    // character size minus one to allow for a NULL terminating character.
    _vsntprintf(szBuffer, NUMCHARS - 1, szFormat, pArgs);
    va_end(pArgs);

    // Ensure that the formatted string is NULL-terminated
    szBuffer[LASTCHAR] = TEXT('\0');

    MessageBox(NULL, szBuffer, TEXT("Movie Texture Error"), 
               MB_OK | MB_ICONERROR);
}


