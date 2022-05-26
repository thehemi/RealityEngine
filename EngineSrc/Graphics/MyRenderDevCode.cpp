#define STRICT
#include "stdafx.h"
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <stdio.h>
#include <tchar.h>
#include "d3dcustom.h" 
#include "dxutil.h"
#include "D3DRenderDevice.h"
#include "Engine.h"
#include "HDR.h"

D3DRenderDevice* gRenderDev = 0; // For callback only

//-----------------------------------------------------------------------------
// TIM: Commented out conditionals, as there appear to be fringe cases
// where we need to set both
//-----------------------------------------------------------------------------
void D3DRenderDevice::ShowCursor(bool bShow){
	//if(m_bWindowed){
		if(!bShow)
			while(::ShowCursor(FALSE) >= 0);
		else
			while(::ShowCursor(TRUE) < 0);
	//}
	//else{
		m_pd3dDevice->ShowCursor( bShow );
	//}
	m_bCursorVisible = bShow;
}


//-----------------------------------------------------------------------------
// Name: ConfirmDeviceHelper()
// Desc: Static function used by D3DEnumeration
//-----------------------------------------------------------------------------
bool D3DRenderDevice::ConfirmDeviceHelper( D3DCAPS9* pCaps, VertexProcessingType vertexProcessingType, 
                         D3DFORMAT adapterFormat, D3DFORMAT backBufferFormat )
{
    DWORD dwBehavior;

	RenderWrap::dev = gRenderDev->m_pd3dDevice;
	RenderWrap::d3d = gRenderDev->m_pD3D;

    if (vertexProcessingType == SOFTWARE_VP)
        dwBehavior = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    else if (vertexProcessingType == MIXED_VP)
        dwBehavior = D3DCREATE_MIXED_VERTEXPROCESSING;
    else if (vertexProcessingType == HARDWARE_VP)
        dwBehavior = D3DCREATE_HARDWARE_VERTEXPROCESSING;
    else if (vertexProcessingType == PURE_HARDWARE_VP)
        dwBehavior = D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE;
    else
        Error("dwBehavior = 0; // TODO: throw exception");
    

	// Impure device will check duplicate states for us
	if(dwBehavior & D3DCREATE_PUREDEVICE)
		return false;

    // Don't accept hardware TNL if config doesn't want it, or this card
	// doesn't support vertex shaders
	if(!(dwBehavior & D3DCREATE_SOFTWARE_VERTEXPROCESSING) && 
		(D3DSHADER_VERSION_MAJOR(pCaps->VertexShaderVersion) == 0 || gRenderDev->WantHarwareTNL == false))
		return false;

	// FIXME: May disable hdr just because one device sucks?
	if(HDRSystem::Instance()->ConfirmDevice(pCaps,dwBehavior,adapterFormat,backBufferFormat) != S_OK){
		// Card doesn't support HDR, so disable it
		//LogPrintf("Error, no HDR support, disabling HDR");
		//RenderDevice::Instance()->SetHDR(false);
		return true;
	}

   return true;
}



//-----------------------------------------------------------------------------
// Name: SetVideoMode()
// Desc: Changes the video mode on the spot
//-----------------------------------------------------------------------------
HRESULT D3DRenderDevice::SetVideoMode(int SizeX, int SizeY, int ColorDepth, bool FullScreen, bool VSync){
    
	HRESULT hr;

	// Get access to the newly selected adapter, device, and mode
    if(!PrepareSettings(SizeX,SizeY,ColorDepth,FullScreen,VSync))
		return -1;

    // Release all scene objects that will be re-created for the new device
    //EventCallback(RE_INVALIDATE);
    //EventCallback(RE_DELETE);

	// Inform the display class of the change. It will internally
	// re-create valid surfaces, a d3ddevice, etc.
	//if( FAILED( hr = Initialize3DEnvironment(true) ) )
	//	return DisplayErrorMsg( hr, MSGERR_APPMUSTEXIT );


	if( FAILED( hr = ToggleFullscreen() ) )
			return DisplayErrorMsg( hr, MSGERR_APPMUSTEXIT );

    // Release display objects, so a new device can be created
	/*int i;
    if( i = m_pd3dDevice->Release() > 0 )
		Error("%i D3D objects were not released",i);

    // Inform the display class of the change. It will internally
    // re-create valid surfaces, a d3ddevice, etc.
    if( FAILED( hr = Initialize3DEnvironment() ) )
        return DisplayErrorMsg( hr, MSGERR_APPMUSTEXIT );
*/
    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: PrepareSettings()
// Desc: Internally prepares the class to use these settings on a scene (re)initialization
//-----------------------------------------------------------------------------
bool D3DRenderDevice::PrepareSettings(int SizeX, int SizeY, int ColorDepth, bool FullScreen, bool VSync){
//	D3DAdapterInfo* pAdapterInfo = &m_Adapters[m_dwAdapter];

	// Will be applied during Initialize3DEnvironment()
	m_bUseVSync = VSync;

	bool forceRefRast = Engine::Instance()->MainConfig->GetBool("ForceREFRAST");

	if(FullScreen){
		if(!FindBestFullscreenMode(!forceRefRast,forceRefRast,SizeX,SizeY,ColorDepth))
			Error("Couldn't find fullscren mode matching requirements\nThis is probably because HDREnable is set to true in 'Helix Core.ini' and your card does not support HDR\nPlease edit the ini to set this to false");
	}
	else{
		if(!FindBestWindowedMode(!forceRefRast,forceRefRast))
			Error("Couldn't find windowed mode matching requirements\nThis is probably because HDREnable is set to true in 'Helix Core.ini' and your card does not support HDR\nPlease edit the ini to set this to false");
	}

	return true;
}


//-----------------------------------------------------------------------------
// Name: BeginRender()
// Desc: Begins Scene drawing
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Name: Render3DEnvironment()
// Desc: Draws the scene.
//-----------------------------------------------------------------------------
HRESULT D3DRenderDevice::BeginRender()
{
    HRESULT hr;

    if( m_bDeviceLost )
    {
        // Test the cooperative level to see if it's okay to render
        if( FAILED( hr = m_pd3dDevice->TestCooperativeLevel() ) )
        {
            // If the device was lost, do not render until we get it back
			if( D3DERR_DEVICELOST == hr ){
                return S_FALSE;
			}

            // Check if the device needs to be reset.
            if( D3DERR_DEVICENOTRESET == hr )
            {
                // If we are windowed, read the desktop mode and use the same format for
                // the back buffer
                if( m_bWindowed )
                {
                    D3DAdapterInfo* pAdapterInfo = m_d3dSettings.PAdapterInfo();
                    m_pD3D->GetAdapterDisplayMode( pAdapterInfo->AdapterOrdinal, &m_d3dSettings.Windowed_DisplayMode );
                    m_d3dpp.BackBufferFormat = m_d3dSettings.Windowed_DisplayMode.Format;
                }

                if( FAILED( hr = Reset3DEnvironment() ) )
                    return hr;
            }
            return hr;
        }
        m_bDeviceLost = false;
    }


	// Update mouse
	// FIXME: Excessive?
	if( m_bActive && m_pd3dDevice != NULL )
	{
		POINT ptCursor;
		GetCursorPos( &ptCursor );
		if( !m_bWindowed )
			ScreenToClient( m_hWnd, &ptCursor );

		// Center cursor if not shown
		if( m_bCenterCursor ){
			
			POINT ptCenter;
			ptCenter.x = m_d3dsdBackBuffer.Width*0.5f;
			ptCenter.y = m_d3dsdBackBuffer.Height*0.5f;
			ClientToScreen(m_hWnd,&ptCenter);
			if(m_bWindowed){
				// Only center the cursor if it's moved
				if(ptCursor.x != ptCenter.x || ptCursor.y != ptCenter.y)
					SetCursorPos(ptCenter.x,ptCenter.y);
			}
			else{
				m_pd3dDevice->SetCursorPosition( ptCenter.x, ptCenter.y, 0L );
			}
		}

		m_pd3dDevice->SetCursorPosition( ptCursor.x, ptCursor.y, 0L );
	}

	//
	// This is an attempt to stop variable framerates
	//
	/*LPDIRECT3DSURFACE9 buffer;
	m_pd3dDevice->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&buffer);
	RECT r;
	r.left = 0;
	r.top = 0;
	r.right = 1;
	r.bottom = 1;

	D3DLOCKED_RECT d3dr;
	buffer->LockRect(&d3dr,&r,D3DLOCK_READONLY|D3DLOCK_DONOTWAIT);
	buffer->UnlockRect();
	buffer->Release();*/

	// Render the scene as normal
    if( FAILED( hr = m_pd3dDevice->BeginScene() ) )
		return hr;

    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: EndRender()
// Desc: Marks the end of scene drawing. Displays result
//-----------------------------------------------------------------------------
HRESULT D3DRenderDevice::EndRender(){
	if(m_bDeviceLost)
		return S_OK; // Waiting for device to return to focus

	HRESULT hr;

	if( FAILED( hr = m_pd3dDevice->EndScene() ) )
		return hr;

    // Show the frame on the primary surface.
	//m_pd3dDevice->GetSwapChain(0,&chain);
	//hr = chain->Present( NULL, NULL, NULL, NULL, (PRESENT_DONOTWAIT?D3DPRESENT_DONOTWAIT:0)/* | D3DPRESENT_LINEAR_CONTENT*/  );
	//chain->Release();

	hr = m_pd3dDevice->Present( NULL, NULL, NULL, NULL );
	//else
	//  hr = m_pd3dDevice->Present( NULL, NULL, NULL, NULL );

    if( D3DERR_DEVICELOST == hr )
        m_bDeviceLost = true;

    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: Destroy()
// Desc: Cleanup scene objects
//-----------------------------------------------------------------------------
VOID D3DRenderDevice::Destroy()
{
    Cleanup3DEnvironment();
	if(m_pD3D)
		while( m_pD3D->Release() ); // Keep releasing to force a dump of any possible leaks
	m_pD3D = NULL;
}
