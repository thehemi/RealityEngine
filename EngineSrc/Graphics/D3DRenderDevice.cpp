//-----------------------------------------------------------------------------
// Changes from sample framework:
// Dynamic DLL loading
// Remove global inst
// Changed ToggleFullScreen to toggle all params without resetting device
// Remove wndproc
// D3DRenderDevice->D3DRenderDevice
// Removed IDM_CHANGEDEVICE part
// Delete D3DSettingsDialog class
// Remove WM_COMMAND branch
// Remove UserSelectNewDevice
// Change Create() first half
// Remove OneTimeSceneInit
// EVENT_CALLBACK switches with real functions
// add m_d3dEnumeration.UsesDepthBuffer   = TRUE;
// HACK_FORCE_ARGB_BACKBUFFER_HACK
// My variables sections
// Small mod to FindBestFullscreenMode
// WM_SETCURSOR  m_pd3dDevice->ShowCursor( m_bCursorVisible );
// m_pd3dDevice->ShowCursor( m_bCursorVisible ); for cursor creation
// SWP_SHOWWINDOW was replaced with NULL so window doesn't show until ready
// Removed WS_VISIBLE from SetWindowLong for fullscreen
//
//
// NOTE: Jerky movement = NVIDIA + Present() + Other apps taking GDI/CPU cycles (like Winamp non-minimized)
// NOTE: Because NVIDIA card is buffering frames?
// NOTE: Happens on ATI cards too
// WHAT DO WE DO?????
// NOTE: Added triple buffering
//
//
//-----------------------------------------------------------------------------
#define STRICT
#include "stdafx.h"
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <stdio.h>
#include <tchar.h>
#include "d3dcustom.h" 
#include "DXUtil.h"
#include "D3DUtil.h"
#include "D3DEnumeration.h"
#include "D3DSettings.h"
#include "D3DRenderDevice.h"

// This hack allows for ARGB support for destination alpha blending.
#define HACK_FORCE_ARGB_BACKBUFFER_HACK 0

extern D3DRenderDevice* gRenderDev; // For callback only

//-----------------------------------------------------------------------------
// Name: D3DRenderDevice()
// Desc: Constructor
//-----------------------------------------------------------------------------
D3DRenderDevice::D3DRenderDevice()
{
	gRenderDev = this;
    m_pD3D              = NULL;
    m_pd3dDevice        = NULL;
    m_hWnd              = NULL;
    m_hWndFocus         = NULL;
    m_hMenu             = NULL;
    m_bActive           = false;
    m_bDeviceLost       = false;
    m_bMinimized        = false;
    m_bMaximized        = false;
	m_bIgnoreSizeChange = false;
    m_bDeviceObjectsInited = false;
    m_bDeviceObjectsRestored = false;
    m_dwCreateFlags     = 0;

    m_bFrameMoving      = true;
    m_bSingleStep       = false;
    m_fTime             = 0.0f;
    m_fElapsedTime      = 0.0f;
    m_fFPS              = 0.0f;
    m_strDeviceStats[0] = _T('\0');
    m_strFrameStats[0]  = _T('\0');

    m_bShowCursorWhenFullscreen = true;
    m_bStartFullscreen  = true;

	// My variables
	m_bCursorVisible    = TRUE;
	m_bCenterCursor		= FALSE;
	WantHarwareTNL		= FALSE;
	m_bUseVSync			= FALSE;
	m_d3dEnumeration.AppUsesDepthBuffer   = TRUE;

    Pause( true ); // Pause until we're ready to render

    // When m_bClipCursorWhenFullscreen is true, the cursor is limited to
    // the device window when the app goes fullscreen.  This prevents users
    // from accidentally clicking outside the app window on a multimon system.
    // This flag is turned off by default for debug builds, since it makes 
    // multimon debugging difficult.
#if defined(_DEBUG) || defined(DEBUG)
    m_bClipCursorWhenFullscreen = false;
#else
    m_bClipCursorWhenFullscreen = true;
#endif
}



//-----------------------------------------------------------------------------
// Name: Create()
// Desc:
//-----------------------------------------------------------------------------
HRESULT D3DRenderDevice::Create( HINSTANCE hInstance, HWND window, int SizeX, int SizeY, 
		int ColorDepth, bool FullScreen, bool VSync, function NewEventCallback )
{
	EventCallback = NewEventCallback;

	m_hWnd = window;

    HRESULT hr;

	HMODULE hModule=LoadLibrary("d3d9.dll");

	if(hModule==NULL)
		Error("This game requires Microsoft® DirectX® 9.0 or later. You can download the latest version from http://www.microsoft.com/directx");

	// get function pointer and create a D3D8 interface
	typedef LPDIRECT3D9 (WINAPI * FD3DCREATE) (UINT SDKVersion);
	FD3DCREATE pCreateD3D = (FD3DCREATE) GetProcAddress(hModule, "Direct3DCreate9");

	// Create the Direct3D object
	m_pD3D =  pCreateD3D(D3D_SDK_VERSION);
    //m_pD3D = Direct3DCreate9( D3D_SDK_VERSION );

    if( m_pD3D == NULL )
        return DisplayErrorMsg( D3DAPPERR_NODIRECT3D, MSGERR_APPMUSTEXIT );

    // Build a list of Direct3D adapters, modes and devices. The
    // ConfirmDevice() callback is used to confirm that only devices that
    // meet the app's requirements are considered.
    m_d3dEnumeration.SetD3D( m_pD3D );
    m_d3dEnumeration.ConfirmDeviceCallback = ConfirmDeviceHelper;
    if( FAILED( hr = m_d3dEnumeration.Enumerate() ) )
    {
        SAFE_RELEASE( m_pD3D );
        return DisplayErrorMsg( hr, MSGERR_APPMUSTEXIT );
    }

	// Set the adapter/device/mode to use during Initialize3DEnvironment()
	if(!PrepareSettings(SizeX,SizeY,ColorDepth,FullScreen,VSync))
		return S_FALSE;

    // The focus window can be a specified to be a different window than the
    // device window.  If not, use the device window as the focus window.
    if( m_hWndFocus == NULL )
        m_hWndFocus = m_hWnd;

    // Save window properties
    m_dwWindowStyle = GetWindowLong( m_hWnd, GWL_STYLE );
    GetWindowRect( m_hWnd, &m_rcWindowBounds );
    GetClientRect( m_hWnd, &m_rcWindowClient );

    //ChooseInitialD3DSettings();

    // Initialize the application timer
    DXUtil_Timer( TIMER_START );

    // Initialize the 3D environment for the app
    if( FAILED( hr = Initialize3DEnvironment() ) )
    {
        SAFE_RELEASE( m_pD3D );
        return DisplayErrorMsg( hr, MSGERR_APPMUSTEXIT );
    }

    // The app is ready to go
    Pause( false );

	// Added these to stop curious loss of mouse focus when going to fullscreen
	// on my machine
	SetFocus(m_hWndFocus);
	SetActiveWindow(m_hWndFocus);
	ShowWindow(m_hWndFocus,SW_SHOW);
	if(!m_bWindowed)
		ShowWindow(m_hWndFocus,SW_MAXIMIZE);

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: FindBestWindowedMode()
// Desc: Sets up m_d3dSettings with best available windowed mode, subject to 
//       the bRequireHAL and bRequireREF constraints.  Returns false if no such
//       mode can be found.
//-----------------------------------------------------------------------------
bool D3DRenderDevice::FindBestWindowedMode( bool bRequireHAL, bool bRequireREF )
{
      // Get display mode of primary adapter (which is assumed to be where the window 
    // will appear)
    D3DDISPLAYMODE primaryDesktopDisplayMode;
    m_pD3D->GetAdapterDisplayMode(0, &primaryDesktopDisplayMode);

    D3DAdapterInfo* pBestAdapterInfo = NULL;
    D3DDeviceInfo* pBestDeviceInfo = NULL;
    D3DDeviceCombo* pBestDeviceCombo = NULL;

    for( UINT iai = 0; iai < m_d3dEnumeration.m_pAdapterInfoList->Count(); iai++ )
    {
        D3DAdapterInfo* pAdapterInfo = (D3DAdapterInfo*)m_d3dEnumeration.m_pAdapterInfoList->GetPtr(iai);
        for( UINT idi = 0; idi < pAdapterInfo->pDeviceInfoList->Count(); idi++ )
        {
            D3DDeviceInfo* pDeviceInfo = (D3DDeviceInfo*)pAdapterInfo->pDeviceInfoList->GetPtr(idi);
            if (bRequireHAL && pDeviceInfo->DevType != D3DDEVTYPE_HAL)
                continue;
            if (bRequireREF && pDeviceInfo->DevType != D3DDEVTYPE_REF)
                continue;
            for( UINT idc = 0; idc < pDeviceInfo->pDeviceComboList->Count(); idc++ )
            {
                D3DDeviceCombo* pDeviceCombo = (D3DDeviceCombo*)pDeviceInfo->pDeviceComboList->GetPtr(idc);
                bool bAdapterMatchesBB = (pDeviceCombo->BackBufferFormat == pDeviceCombo->AdapterFormat);
                if (!pDeviceCombo->IsWindowed)
                    continue;
                if (pDeviceCombo->AdapterFormat != primaryDesktopDisplayMode.Format)
                    continue;
                // If we haven't found a compatible DeviceCombo yet, or if this set
                // is better (because it's a HAL, and/or because formats match better),
                // save it
                if( pBestDeviceCombo == NULL || 
                    pBestDeviceCombo->DevType != D3DDEVTYPE_HAL && pDeviceCombo->DevType == D3DDEVTYPE_HAL ||
                    pDeviceCombo->DevType == D3DDEVTYPE_HAL && bAdapterMatchesBB )
                {
                    pBestAdapterInfo = pAdapterInfo;
                    pBestDeviceInfo = pDeviceInfo;
                    pBestDeviceCombo = pDeviceCombo;
                    if( pDeviceCombo->DevType == D3DDEVTYPE_HAL && bAdapterMatchesBB )
                    {
                        // This windowed device combo looks great -- take it
                        goto EndWindowedDeviceComboSearch;
                    }
                    // Otherwise keep looking for a better windowed device combo
                }
            }
        }
    }
EndWindowedDeviceComboSearch:
    if (pBestDeviceCombo == NULL )
        return false;

    m_d3dSettings.pWindowed_AdapterInfo = pBestAdapterInfo;
    m_d3dSettings.pWindowed_DeviceInfo = pBestDeviceInfo;
    m_d3dSettings.pWindowed_DeviceCombo = pBestDeviceCombo;
    m_d3dSettings.IsWindowed = true;
    m_d3dSettings.Windowed_DisplayMode = primaryDesktopDisplayMode;
    m_d3dSettings.Windowed_Width = m_rcWindowClient.right - m_rcWindowClient.left;
    m_d3dSettings.Windowed_Height = m_rcWindowClient.bottom - m_rcWindowClient.top;
    if (m_d3dEnumeration.AppUsesDepthBuffer)
        m_d3dSettings.Windowed_DepthStencilBufferFormat = *(D3DFORMAT*)pBestDeviceCombo->pDepthStencilFormatList->GetPtr(0);
    m_d3dSettings.Windowed_MultisampleType = *(D3DMULTISAMPLE_TYPE*)pBestDeviceCombo->pMultiSampleTypeList->GetPtr(0);
    m_d3dSettings.Windowed_MultisampleQuality = 0;
    m_d3dSettings.Windowed_VertexProcessingType = *(VertexProcessingType*)pBestDeviceCombo->pVertexProcessingTypeList->GetPtr(0);
    m_d3dSettings.Windowed_PresentInterval = *(UINT*)pBestDeviceCombo->pPresentIntervalList->GetPtr(0);
    return true;
}




//-----------------------------------------------------------------------------
// Name: FindBestFullscreenMode()
// Desc: Sets up m_d3dSettings with best available fullscreen mode, subject to 
//       the bRequireHAL and bRequireREF constraints.  Returns false if no such
//       mode can be found.
//-----------------------------------------------------------------------------
bool D3DRenderDevice::FindBestFullscreenMode( bool bRequireHAL, bool bRequireREF, int width, int height, int depth )
{
    // For fullscreen, default to first HAL DeviceCombo that supports the current desktop 
    // display mode, or any display mode if HAL is not compatible with the desktop mode, or 
    // non-HAL if no HAL is available
    D3DDISPLAYMODE adapterDesktopDisplayMode;
    D3DDISPLAYMODE bestAdapterDesktopDisplayMode;
    D3DDISPLAYMODE bestDisplayMode;
    bestAdapterDesktopDisplayMode.Width = 0;
    bestAdapterDesktopDisplayMode.Height = 0;
    bestAdapterDesktopDisplayMode.Format = D3DFMT_UNKNOWN;
    bestAdapterDesktopDisplayMode.RefreshRate = 0;

    D3DAdapterInfo* pBestAdapterInfo = NULL;
    D3DDeviceInfo* pBestDeviceInfo = NULL;
    D3DDeviceCombo* pBestDeviceCombo = NULL;

    for( UINT iai = 0; iai < m_d3dEnumeration.m_pAdapterInfoList->Count(); iai++ )
    {
        D3DAdapterInfo* pAdapterInfo = (D3DAdapterInfo*)m_d3dEnumeration.m_pAdapterInfoList->GetPtr(iai);
        m_pD3D->GetAdapterDisplayMode( pAdapterInfo->AdapterOrdinal, &adapterDesktopDisplayMode );

		// Tim: Build our own preference
		if(width && height){
			adapterDesktopDisplayMode.Height = height;
			adapterDesktopDisplayMode.Width  = width;
		}


       for( UINT idi = 0; idi < pAdapterInfo->pDeviceInfoList->Count(); idi++ )
        {
            D3DDeviceInfo* pDeviceInfo = (D3DDeviceInfo*)pAdapterInfo->pDeviceInfoList->GetPtr(idi);
            if (bRequireHAL && pDeviceInfo->DevType != D3DDEVTYPE_HAL)
                continue;
            if (bRequireREF && pDeviceInfo->DevType != D3DDEVTYPE_REF)
                continue;
            for( UINT idc = 0; idc < pDeviceInfo->pDeviceComboList->Count(); idc++ )
            {
                D3DDeviceCombo* pDeviceCombo = (D3DDeviceCombo*)pDeviceInfo->pDeviceComboList->GetPtr(idc);
				bool bAdapterMatchesBB = (pDeviceCombo->BackBufferFormat == pDeviceCombo->AdapterFormat);
				bool bAdapterMatchesDesktop = (pDeviceCombo->AdapterFormat == adapterDesktopDisplayMode.Format);
				if (pDeviceCombo->IsWindowed)
					continue;
				// If we haven't found a compatible set yet, or if this set
				// is better (because it's a HAL, and/or because formats match better),
				// save it
				if (pBestDeviceCombo == NULL ||
					pBestDeviceCombo->DevType != D3DDEVTYPE_HAL && pDeviceInfo->DevType == D3DDEVTYPE_HAL ||
					pDeviceCombo->DevType == D3DDEVTYPE_HAL && pBestDeviceCombo->AdapterFormat != adapterDesktopDisplayMode.Format && bAdapterMatchesDesktop ||
					pDeviceCombo->DevType == D3DDEVTYPE_HAL && bAdapterMatchesDesktop && bAdapterMatchesBB )
				{
					bestAdapterDesktopDisplayMode = adapterDesktopDisplayMode;
					pBestAdapterInfo = pAdapterInfo;
					pBestDeviceInfo = pDeviceInfo;
					pBestDeviceCombo = pDeviceCombo;
					if (pDeviceInfo->DevType == D3DDEVTYPE_HAL && bAdapterMatchesDesktop && bAdapterMatchesBB)
					{
						// This fullscreen device combo looks great -- take it
						goto EndFullscreenDeviceComboSearch;
					}
					// Otherwise keep looking for a better fullscreen device combo
				}
            }
        }
    }
EndFullscreenDeviceComboSearch:
    if (pBestDeviceCombo == NULL)
        return false;

    // Need to find a display mode on the best adapter that uses pBestDeviceCombo->AdapterFormat
    // and is as close to bestAdapterDesktopDisplayMode's res as possible
    bestDisplayMode.Width = 0;
    bestDisplayMode.Height = 0;
    bestDisplayMode.Format = D3DFMT_UNKNOWN;
    bestDisplayMode.RefreshRate = 0;
    for( UINT idm = 0; idm < pBestAdapterInfo->pDisplayModeList->Count(); idm++ )
    {
        D3DDISPLAYMODE* pdm = (D3DDISPLAYMODE*)pBestAdapterInfo->pDisplayModeList->GetPtr(idm);
        if( pdm->Format != pBestDeviceCombo->AdapterFormat )
            continue;
        if( pdm->Width == bestAdapterDesktopDisplayMode.Width &&
            pdm->Height == bestAdapterDesktopDisplayMode.Height && 
            pdm->RefreshRate == bestAdapterDesktopDisplayMode.RefreshRate )
        {
            // found a perfect match, so stop
            bestDisplayMode = *pdm;
            break;
        }
        else if( pdm->Width == bestAdapterDesktopDisplayMode.Width &&
                 pdm->Height == bestAdapterDesktopDisplayMode.Height && 
                 pdm->RefreshRate > bestDisplayMode.RefreshRate )
        {
            // refresh rate doesn't match, but width/height match, so keep this
            // and keep looking
            bestDisplayMode = *pdm;
        }
        else if( pdm->Width == bestAdapterDesktopDisplayMode.Width )
        {
            // width matches, so keep this and keep looking
            bestDisplayMode = *pdm;
        }
        else if( bestDisplayMode.Width == 0 )
        {
            // we don't have anything better yet, so keep this and keep looking
            bestDisplayMode = *pdm;
        }
    }

    m_d3dSettings.pFullscreen_AdapterInfo = pBestAdapterInfo;
    m_d3dSettings.pFullscreen_DeviceInfo = pBestDeviceInfo;
    m_d3dSettings.pFullscreen_DeviceCombo = pBestDeviceCombo;
    m_d3dSettings.IsWindowed = false;
    m_d3dSettings.Fullscreen_DisplayMode = bestDisplayMode;
    if (m_d3dEnumeration.AppUsesDepthBuffer)
        m_d3dSettings.Fullscreen_DepthStencilBufferFormat = *(D3DFORMAT*)pBestDeviceCombo->pDepthStencilFormatList->GetPtr(0);
    m_d3dSettings.Fullscreen_MultisampleType = *(D3DMULTISAMPLE_TYPE*)pBestDeviceCombo->pMultiSampleTypeList->GetPtr(0);
    m_d3dSettings.Fullscreen_MultisampleQuality = 0;
    m_d3dSettings.Fullscreen_VertexProcessingType = *(VertexProcessingType*)pBestDeviceCombo->pVertexProcessingTypeList->GetPtr(0);
    m_d3dSettings.Fullscreen_PresentInterval = *(UINT*)pBestDeviceCombo->pPresentIntervalList->GetPtr(0);
    return true;
}


//-----------------------------------------------------------------------------
// Name: ToggleFullScreen()
// Desc: Called when user toggles between fullscreen mode and windowed mode
//-----------------------------------------------------------------------------
HRESULT D3DRenderDevice::ToggleFullscreen()
{
	HRESULT hr;
	int AdapterOrdinalOld = m_d3dSettings.AdapterOrdinal();
	D3DDEVTYPE DevTypeOld = m_d3dSettings.DevType();

	Pause( true );
	m_bIgnoreSizeChange = true;

	m_bWindowed = m_d3dSettings.IsWindowed;
	// Toggle the windowed state
	//m_bWindowed = !m_bWindowed;
	//m_d3dSettings.IsWindowed = m_bWindowed;

	// Prepare window for windowed/fullscreen change
	AdjustWindowForChange();
	SetFocus(m_hWndFocus);
	SetActiveWindow(m_hWndFocus);

	// If AdapterOrdinal and DevType are the same, we can just do a Reset().
	// If they've changed, we need to do a complete device teardown/rebuild.
	if (true)//m_d3dSettings.AdapterOrdinal() == AdapterOrdinalOld &&
		//m_d3dSettings.DevType() == DevTypeOld)
	{
		// Reset the 3D device
		BuildPresentParamsFromSettings();
		hr = Reset3DEnvironment();
	}
	else
	{
		Cleanup3DEnvironment();
		hr = Initialize3DEnvironment();
	}
	if( FAILED( hr ) )
	{
		if( hr != D3DERR_OUTOFVIDEOMEMORY )
			hr = D3DAPPERR_RESETFAILED;
		m_bIgnoreSizeChange = false;
		if( !m_bWindowed )
		{
			// Restore window type to windowed mode
			m_bWindowed = !m_bWindowed;
			m_d3dSettings.IsWindowed = m_bWindowed;
			AdjustWindowForChange();
			SetWindowPos( m_hWnd, HWND_NOTOPMOST,
				m_rcWindowBounds.left, m_rcWindowBounds.top,
				( m_rcWindowBounds.right - m_rcWindowBounds.left ),
				( m_rcWindowBounds.bottom - m_rcWindowBounds.top ),
				SWP_SHOWWINDOW );
		}
		return DisplayErrorMsg( hr, MSGERR_APPMUSTEXIT );
	}

	m_bIgnoreSizeChange = false;

	// When moving from fullscreen to windowed mode, it is important to
	// adjust the window size after resetting the device rather than
	// beforehand to ensure that you get the window size you want.  For
	// example, when switching from 640x480 fullscreen to windowed with
	// a 1000x600 window on a 1024x768 desktop, it is impossible to set
	// the window size to 1000x600 until after the display mode has
	// changed to 1024x768, because windows cannot be larger than the
	// desktop.
	if( m_bWindowed )
	{
		SetWindowPos( m_hWnd, HWND_NOTOPMOST,
			m_rcWindowBounds.left, m_rcWindowBounds.top,
			( m_rcWindowBounds.right - m_rcWindowBounds.left ),
			( m_rcWindowBounds.bottom - m_rcWindowBounds.top ),
			SWP_SHOWWINDOW );
	}

	GetClientRect( m_hWnd, &m_rcWindowClient );  // Update our copy

	// Added these to stop curious loss of mouse focus when going to fullscreen
	// on my machine
	SetFocus(m_hWndFocus);
	SetActiveWindow(m_hWndFocus);
	ShowWindow(m_hWndFocus,SW_SHOW);
	if(!m_bWindowed)
		ShowWindow(m_hWndFocus,SW_MAXIMIZE);

	Pause( false );
	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: ChooseInitialD3DSettings()
// Desc: 
//-----------------------------------------------------------------------------
/*bool D3DRenderDevice::ChooseInitialD3DSettings()
{
    bool bFoundFullscreen = FindBestFullscreenMode( false, false );
    bool bFoundWindowed = FindBestWindowedMode( false, false );

    if( m_bStartFullscreen && bFoundFullscreen )
        m_d3dSettings.IsWindowed = false;

    if( !bFoundFullscreen && !bFoundWindowed )
        return D3DAPPERR_NOCOMPATIBLEDEVICES;

    return S_OK;
}
*/



//-----------------------------------------------------------------------------
// Name: MsgProc()
// Desc: Message handling function.  Here's what this function does:
//       - WM_PAINT: calls Render() and Present() is called if !m_bReady
//       - WM_EXITSIZEMOVE: window size recalc'd and calls HandlePossibleSizeChange()
//       - WM_CLOSE: calls Cleanup3dEnvironment(), DestroyMenu(), DestroyWindow(), PostQuitMessage()
//       - WM_COMMAND: IDM_CHANGEDEVICE calls UserSelectNewDevice() to select a new device
//       - WM_COMMAND: IDM_TOGGLEFULLSCREEN calls ToggleFullScreen() to toggle 
//                  between fullscreen and windowed
//       - WM_COMMAND: IDM_EXIT: shuts down the app with a WM_CLOSE 
//       - anything not handled goes to DefWindowProc()     
//-----------------------------------------------------------------------------
LRESULT D3DRenderDevice::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam,
								 LPARAM lParam )
{
	switch( uMsg )
	{
	case WM_PAINT:
		// Handle paint messages when the app is paused
		if( m_pd3dDevice && !m_bActive && 
			m_bDeviceObjectsInited && m_bDeviceObjectsRestored )
		{
			HRESULT hr;
			//Render();
			//hr = m_pd3dDevice->Present( NULL, NULL, NULL, NULL );
			//if( D3DERR_DEVICELOST == hr )
			//	m_bDeviceLost = true;
		}
		break;

	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 100;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 100;
		break;

	case WM_ENTERSIZEMOVE:
		// Halt frame movement while the app is sizing or moving
		Pause( true );
		break;

	case WM_SIZE:
		// Pick up possible changes to window style due to maximize, etc.
		if( m_bWindowed && m_hWnd != NULL )
			m_dwWindowStyle = GetWindowLong( m_hWnd, GWL_STYLE );

		if( SIZE_MINIMIZED == wParam )
		{
			if( m_bClipCursorWhenFullscreen && !m_bWindowed )
				ClipCursor( NULL );
			Pause( true ); // Pause while we're minimized
			m_bMinimized = true;
			m_bMaximized = false;
		}
		else if( SIZE_MAXIMIZED == wParam )
		{
			if( m_bMinimized )
				Pause( false ); // Unpause since we're no longer minimized
			m_bMinimized = false;
			m_bMaximized = true;
			HandlePossibleSizeChange();
		}
		else if( SIZE_RESTORED == wParam )
		{
			if( m_bMaximized )
			{
				m_bMaximized = false;
				HandlePossibleSizeChange();
			}
			else if( m_bMinimized)
			{
				Pause( false ); // Unpause since we're no longer minimized
				m_bMinimized = false;
				HandlePossibleSizeChange();
			}
			else
			{
				// If we're neither maximized nor minimized, the window size 
				// is changing by the user dragging the window edges.  In this 
				// case, we don't reset the device yet -- we wait until the 
				// user stops dragging, and a WM_EXITSIZEMOVE message comes.
			}
		}
		break;

	case WM_EXITSIZEMOVE:
		Pause( false );
		HandlePossibleSizeChange();
		break;

	case WM_SETCURSOR:
		// Turn off Windows cursor in fullscreen mode
		if( m_bActive && !m_bWindowed )
		{
			SetCursor( NULL );
			if( m_bShowCursorWhenFullscreen )
				m_pd3dDevice->ShowCursor( true );
			return true; // prevent Windows from setting cursor to window class cursor
		}
		break;

	case WM_MOUSEMOVE:
		if( m_bActive && m_pd3dDevice != NULL )
		{
			POINT ptCursor;
			GetCursorPos( &ptCursor );
			if( !m_bWindowed )
				ScreenToClient( m_hWnd, &ptCursor );
			m_pd3dDevice->SetCursorPosition( ptCursor.x, ptCursor.y, 0 );
		}
		break;

	case WM_ENTERMENULOOP:
		// Pause the app when menus are displayed
		Pause(true);
		break;

	case WM_EXITMENULOOP:
		Pause(false);
		break;

	case WM_NCHITTEST:
		// Prevent the user from selecting the menu in fullscreen mode
		if( !m_bWindowed )
			return HTCLIENT;
		break;

	case WM_POWERBROADCAST:
		switch( wParam )
		{
#ifndef PBT_APMQUERYSUSPEND
#define PBT_APMQUERYSUSPEND 0x0000
#endif
	case PBT_APMQUERYSUSPEND:
		// At this point, the app should save any data for open
		// network connections, files, etc., and prepare to go into
		// a suspended mode.
		return true;

#ifndef PBT_APMRESUMESUSPEND
#define PBT_APMRESUMESUSPEND 0x0007
#endif
	case PBT_APMRESUMESUSPEND:
		// At this point, the app should recover any data, network
		// connections, files, etc., and resume running from when
		// the app was suspended.
		return true;
		}
		break;

	case WM_SYSCOMMAND:
		// Prevent moving/sizing and power loss in fullscreen mode
		switch( wParam )
		{
		case SC_MOVE:
		case SC_SIZE:
		case SC_MAXIMIZE:
		case SC_KEYMENU:
		case SC_MONITORPOWER:
			if( false == m_bWindowed )
				return 1;
			break;
		}
		break;

	case WM_CLOSE:
		Cleanup3DEnvironment();
		SAFE_RELEASE(m_pD3D);
		FinalCleanup();
		HMENU hMenu;
		hMenu = GetMenu(hWnd);
		if( hMenu != NULL )
			DestroyMenu( hMenu );
		DestroyWindow( hWnd );
		PostQuitMessage(0);
		m_hWnd = NULL;
		return 0;
	}

	return DefWindowProc( hWnd, uMsg, wParam, lParam );
}





//-----------------------------------------------------------------------------
// Name: HandlePossibleSizeChange()
// Desc: Reset the device if the client area size has changed.
//-----------------------------------------------------------------------------
HRESULT D3DRenderDevice::HandlePossibleSizeChange()
{
	HRESULT hr = S_OK;
	RECT rcClientOld;
	rcClientOld = m_rcWindowClient;

	if( m_bIgnoreSizeChange )
		return S_OK;

	// Update window properties
	GetWindowRect( m_hWnd, &m_rcWindowBounds );
	GetClientRect( m_hWnd, &m_rcWindowClient );

	if( rcClientOld.right - rcClientOld.left !=
		m_rcWindowClient.right - m_rcWindowClient.left ||
		rcClientOld.bottom - rcClientOld.top !=
		m_rcWindowClient.bottom - m_rcWindowClient.top)
	{
		// A new window size will require a new backbuffer
		// size, so the 3D structures must be changed accordingly.
		Pause( true );

		m_d3dpp.BackBufferWidth  = m_rcWindowClient.right - m_rcWindowClient.left;
		m_d3dpp.BackBufferHeight = m_rcWindowClient.bottom - m_rcWindowClient.top;

		m_d3dSettings.Windowed_Width = m_d3dpp.BackBufferWidth;
		m_d3dSettings.Windowed_Height = m_d3dpp.BackBufferHeight;

		if( m_pd3dDevice != NULL )
		{
			// Reset the 3D environment
			if( FAILED( hr = Reset3DEnvironment() ) )
			{
				if( hr == D3DERR_DEVICELOST )
				{
					m_bDeviceLost = true;
					hr = S_OK;
				}
				else
				{
					if( hr != D3DERR_OUTOFVIDEOMEMORY )
						hr = D3DAPPERR_RESETFAILED;

					DisplayErrorMsg( hr, MSGERR_APPMUSTEXIT );
				}
			}
		}
		Pause( false );
	}
	return hr;
}



//-----------------------------------------------------------------------------
// Name: Initialize3DEnvironment()
// Desc:
//-----------------------------------------------------------------------------
HRESULT D3DRenderDevice::Initialize3DEnvironment(bool bReset)
{
	    HRESULT hr;

    D3DAdapterInfo* pAdapterInfo = m_d3dSettings.PAdapterInfo();
    D3DDeviceInfo* pDeviceInfo = m_d3dSettings.PDeviceInfo();

    m_bWindowed = m_d3dSettings.IsWindowed;

    // Prepare window for possible windowed/fullscreen change
    AdjustWindowForChange();

    // Set up the presentation parameters
    BuildPresentParamsFromSettings();

    if( pDeviceInfo->Caps.PrimitiveMiscCaps & D3DPMISCCAPS_NULLREFERENCE )
    {
        // Warn user about null ref device that can't render anything
        DisplayErrorMsg( D3DAPPERR_NULLREFDEVICE, 0 );
    }

    DWORD behaviorFlags;
    if (m_d3dSettings.GetVertexProcessingType() == SOFTWARE_VP)
        behaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    else if (m_d3dSettings.GetVertexProcessingType() == MIXED_VP)
        behaviorFlags = D3DCREATE_MIXED_VERTEXPROCESSING;
    else if (m_d3dSettings.GetVertexProcessingType() == HARDWARE_VP)
        behaviorFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING;
    else if (m_d3dSettings.GetVertexProcessingType() == PURE_HARDWARE_VP)
        behaviorFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE;
    else
        behaviorFlags = 0; // TODO: throw exception

    // Create the device
	if(bReset)
		// Reset the device
		hr = m_pd3dDevice->Reset( &m_d3dpp );
	else
		 hr = m_pD3D->CreateDevice( m_d3dSettings.AdapterOrdinal(), pDeviceInfo->DevType,
                               m_hWndFocus, behaviorFlags, &m_d3dpp,
                               &m_pd3dDevice );

    if( SUCCEEDED(hr) )
    {
        // When moving from fullscreen to windowed mode, it is important to
        // adjust the window size after recreating the device rather than
        // beforehand to ensure that you get the window size you want.  For
        // example, when switching from 640x480 fullscreen to windowed with
        // a 1000x600 window on a 1024x768 desktop, it is impossible to set
        // the window size to 1000x600 until after the display mode has
        // changed to 1024x768, because windows cannot be larger than the
        // desktop.
        if( m_bWindowed )
        {
            SetWindowPos( m_hWnd, HWND_NOTOPMOST,
                          m_rcWindowBounds.left, m_rcWindowBounds.top,
                          ( m_rcWindowBounds.right - m_rcWindowBounds.left ),
                          ( m_rcWindowBounds.bottom - m_rcWindowBounds.top ),
                          NULL );
        }

        // Store device Caps
        m_pd3dDevice->GetDeviceCaps( &m_d3dCaps );
        m_dwCreateFlags = behaviorFlags;

        // Store device description
        if( pDeviceInfo->DevType == D3DDEVTYPE_REF )
            lstrcpy( m_strDeviceStats, TEXT("REF") );
        else if( pDeviceInfo->DevType == D3DDEVTYPE_HAL )
            lstrcpy( m_strDeviceStats, TEXT("HAL") );
        else if( pDeviceInfo->DevType == D3DDEVTYPE_SW )
            lstrcpy( m_strDeviceStats, TEXT("SW") );

        if( behaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING &&
            behaviorFlags & D3DCREATE_PUREDEVICE )
        {
            if( pDeviceInfo->DevType == D3DDEVTYPE_HAL )
                lstrcat( m_strDeviceStats, TEXT(" (pure hw vp)") );
            else
                lstrcat( m_strDeviceStats, TEXT(" (simulated pure hw vp)") );
        }
        else if( behaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING )
        {
            if( pDeviceInfo->DevType == D3DDEVTYPE_HAL )
                lstrcat( m_strDeviceStats, TEXT(" (hw vp)") );
            else
                lstrcat( m_strDeviceStats, TEXT(" (simulated hw vp)") );
        }
        else if( behaviorFlags & D3DCREATE_MIXED_VERTEXPROCESSING )
        {
            if( pDeviceInfo->DevType == D3DDEVTYPE_HAL )
                lstrcat( m_strDeviceStats, TEXT(" (mixed vp)") );
            else
                lstrcat( m_strDeviceStats, TEXT(" (simulated mixed vp)") );
        }
        else if( behaviorFlags & D3DCREATE_SOFTWARE_VERTEXPROCESSING )
        {
            lstrcat( m_strDeviceStats, TEXT(" (sw vp)") );
        }

        if( pDeviceInfo->DevType == D3DDEVTYPE_HAL )
        {
            // Be sure not to overflow m_strDeviceStats when appending the adapter 
            // description, since it can be long.  Note that the adapter description
            // is initially CHAR and must be converted to TCHAR.
            lstrcat( m_strDeviceStats, TEXT(": ") );
            const int cchDesc = sizeof(pAdapterInfo->AdapterIdentifier.Description);
            TCHAR szDescription[cchDesc];
            DXUtil_ConvertAnsiStringToGenericCch( szDescription, 
                pAdapterInfo->AdapterIdentifier.Description, cchDesc );
            int maxAppend = sizeof(m_strDeviceStats) / sizeof(TCHAR) -
                lstrlen( m_strDeviceStats ) - 1;
            _tcsncat( m_strDeviceStats, szDescription, maxAppend );
        }

        // Store render target surface desc
        LPDIRECT3DSURFACE9 pBackBuffer = NULL;
        m_pd3dDevice->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer );
        pBackBuffer->GetDesc( &m_d3dsdBackBuffer );
        pBackBuffer->Release();

        // Set up the fullscreen cursor
        if( m_bShowCursorWhenFullscreen && !m_bWindowed )
        {
            HCURSOR hCursor;
#ifdef _WIN64
            hCursor = (HCURSOR)GetClassLongPtr( m_hWnd, GCLP_HCURSOR );
#else
            hCursor = (HCURSOR)ULongToHandle( GetClassLong( m_hWnd, GCL_HCURSOR ) );
#endif
            D3DUtil_SetDeviceCursor( m_pd3dDevice, hCursor, false );
            m_pd3dDevice->ShowCursor( m_bCursorVisible );
        }

        // Confine cursor to fullscreen window
        if( m_bClipCursorWhenFullscreen )
        {
            if (!m_bWindowed )
            {
                RECT rcWindow;
                GetWindowRect( m_hWnd, &rcWindow );
                ClipCursor( &rcWindow );
            }
            else
            {
                ClipCursor( NULL );
            }
        }

        // Initialize the app's device-dependent objects if not a reset
		if(!bReset)
		    hr = InitDeviceObjects();
        if( !bReset && FAILED(hr) )
        {
            DeleteDeviceObjects();
        }
        else
        {
            m_bDeviceObjectsInited = true;
            hr = RestoreDeviceObjects();
            if( FAILED(hr) )
            {
                InvalidateDeviceObjects();
            }
            else
            {
                m_bDeviceObjectsRestored = true;
                return S_OK;
            }
        }

        // Cleanup before we try again
        Cleanup3DEnvironment();
    }

    // If that failed, fall back to the reference rasterizer
	Warning("Your machine sucks so badly it can only run the game in software. This may not work at all....");
    if( hr != D3DAPPERR_MEDIANOTFOUND && 
        hr != HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND ) && 
        pDeviceInfo->DevType == D3DDEVTYPE_HAL )
    {
        if (FindBestWindowedMode(false, true))
        {
            m_bWindowed = true;
            AdjustWindowForChange();
            // Make sure main window isn't topmost, so error message is visible
            SetWindowPos( m_hWnd, HWND_NOTOPMOST,
                          m_rcWindowBounds.left, m_rcWindowBounds.top,
                          ( m_rcWindowBounds.right - m_rcWindowBounds.left ),
                          ( m_rcWindowBounds.bottom - m_rcWindowBounds.top ),
                          NULL );

            // Let the user know we are switching from HAL to the reference rasterizer
            DisplayErrorMsg( hr, MSGWARN_SWITCHEDTOREF );

            hr = Initialize3DEnvironment();
        }
    }
    return hr;
}



//-----------------------------------------------------------------------------
// Name: BuildPresentParamsFromSettings()
// Desc:
//-----------------------------------------------------------------------------
void D3DRenderDevice::BuildPresentParamsFromSettings()
{
    m_d3dpp.Windowed               = m_d3dSettings.IsWindowed;
	m_d3dpp.BackBufferCount        = tripleBuffering?2:1;//1;
    m_d3dpp.MultiSampleType        = m_d3dSettings.MultisampleType();
    m_d3dpp.MultiSampleQuality     = m_d3dSettings.MultisampleQuality();
    m_d3dpp.EnableAutoDepthStencil = m_d3dEnumeration.AppUsesDepthBuffer;
   
    m_d3dpp.hDeviceWindow          = m_hWnd;
	if( m_d3dEnumeration.AppUsesDepthBuffer ){
        m_d3dpp.Flags              = /*D3DPRESENTFLAG_LOCKABLE_BACKBUFFER |*/ D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
		m_d3dpp.AutoDepthStencilFormat = m_d3dSettings.DepthStencilBufferFormat();
	}
    else
        m_d3dpp.Flags              = 0;//D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;

    if( m_bWindowed )
    {
        m_d3dpp.BackBufferWidth  = m_rcWindowClient.right - m_rcWindowClient.left;
        m_d3dpp.BackBufferHeight = m_rcWindowClient.bottom - m_rcWindowClient.top;
        m_d3dpp.BackBufferFormat = m_d3dSettings.PDeviceCombo()->BackBufferFormat; // D3DFMT_X8R8G8B8;//
        m_d3dpp.FullScreen_RefreshRateInHz = 0;
        m_d3dpp.PresentationInterval = m_d3dSettings.PresentInterval();
    }
    else
    {
        m_d3dpp.BackBufferWidth  = m_d3dSettings.DisplayMode().Width;
        m_d3dpp.BackBufferHeight = m_d3dSettings.DisplayMode().Height;
        m_d3dpp.BackBufferFormat = m_d3dSettings.PDeviceCombo()->BackBufferFormat;
        m_d3dpp.FullScreen_RefreshRateInHz = m_d3dSettings.Fullscreen_DisplayMode.RefreshRate;
        m_d3dpp.PresentationInterval = m_d3dSettings.PresentInterval();
    }

	if(m_bUseVSync && !m_bWindowed){
		m_d3dpp.PresentationInterval   = D3DPRESENT_INTERVAL_ONE;
		m_d3dpp.SwapEffect             = D3DSWAPEFFECT_COPY;
	}
	else{
		m_d3dpp.PresentationInterval   = D3DPRESENT_INTERVAL_IMMEDIATE;
		m_d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
	}
	
}




//-----------------------------------------------------------------------------
// Name: Reset3DEnvironment()
// Desc:
//-----------------------------------------------------------------------------
HRESULT D3DRenderDevice::Reset3DEnvironment()
{
  HRESULT hr;

    // Release all vidmem objects
    if( m_bDeviceObjectsRestored )
    {
        m_bDeviceObjectsRestored = false;
        InvalidateDeviceObjects();
    }
    // Reset the device
    if( FAILED( hr = m_pd3dDevice->Reset( &m_d3dpp ) ) )
        return hr;

    // Store render target surface desc
    LPDIRECT3DSURFACE9 pBackBuffer;
    m_pd3dDevice->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer );
    pBackBuffer->GetDesc( &m_d3dsdBackBuffer );
    pBackBuffer->Release();

    // Set up the fullscreen cursor
    if( m_bShowCursorWhenFullscreen && !m_bWindowed )
    {
        HCURSOR hCursor;
#ifdef _WIN64
        hCursor = (HCURSOR)GetClassLongPtr( m_hWnd, GCLP_HCURSOR );
#else
        hCursor = (HCURSOR)ULongToHandle( GetClassLong( m_hWnd, GCL_HCURSOR ) );
#endif
        D3DUtil_SetDeviceCursor( m_pd3dDevice, hCursor, false );
        m_pd3dDevice->ShowCursor( m_bCursorVisible );
    }

    // Confine cursor to fullscreen window
    if( m_bClipCursorWhenFullscreen )
    {
        if (!m_bWindowed )
        {
            RECT rcWindow;
            GetWindowRect( m_hWnd, &rcWindow );
            ClipCursor( &rcWindow );
        }
        else
        {
            ClipCursor( NULL );
        }
    }

    // Initialize the app's device-dependent objects
    hr = RestoreDeviceObjects();
    if( FAILED(hr) )
    {
        InvalidateDeviceObjects();
        return hr;
    }
    m_bDeviceObjectsRestored = true;

    // If the app is paused, trigger the rendering of the current frame
    if( false == m_bFrameMoving )
    {
        m_bSingleStep = true;
        DXUtil_Timer( TIMER_START );
        DXUtil_Timer( TIMER_STOP );
    }

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: AdjustWindowForChange()
// Desc: Prepare the window for a possible change between windowed mode and
//       fullscreen mode.  This function is virtual and thus can be overridden
//       to provide different behavior, such as switching to an entirely
//       different window for fullscreen mode (as in the MFC sample apps).
//-----------------------------------------------------------------------------
HRESULT D3DRenderDevice::AdjustWindowForChange()
{
	#if HACK_FORCE_ARGB_BACKBUFFER_HACK
		m_d3dEnumeration.AppMinAlphaChannelBits = 8; //<-changew
	#endif

    if( m_bWindowed )
    {
        // Set windowed-mode style
        SetWindowLong( m_hWnd, GWL_STYLE, m_dwWindowStyle );
        if( m_hMenu != NULL )
        {
            SetMenu( m_hWnd, m_hMenu );
            m_hMenu = NULL;
        }
    }
    else
    {
        // Set fullscreen-mode style
        SetWindowLong( m_hWnd, GWL_STYLE, WS_POPUP|WS_SYSMENU );
        if( m_hMenu == NULL )
        {
            m_hMenu = GetMenu( m_hWnd );
            SetMenu( m_hWnd, NULL );
        }
    }
    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: Pause()
// Desc: Called in to toggle the pause state of the app.
//-----------------------------------------------------------------------------
void D3DRenderDevice::Pause( bool bPause )
{
    static DWORD dwAppPausedCount = 0;

    dwAppPausedCount += ( bPause ? +1 : -1 );
    m_bActive         = ( dwAppPausedCount ? false : true );

    // Handle the first pause request (of many, nestable pause requests)
    if( bPause && ( 1 == dwAppPausedCount ) )
    {
        // Stop the scene from animating
        if( m_bFrameMoving )
            DXUtil_Timer( TIMER_STOP );
    }

    if( 0 == dwAppPausedCount )
    {
        // Restart the timers
        if( m_bFrameMoving )
            DXUtil_Timer( TIMER_START );
    }
}




//-----------------------------------------------------------------------------
// Name: Cleanup3DEnvironment()
// Desc: Cleanup scene objects
//-----------------------------------------------------------------------------
void D3DRenderDevice::Cleanup3DEnvironment()
{
    if( m_pd3dDevice != NULL )
    {
        if( m_bDeviceObjectsRestored )
        {
            m_bDeviceObjectsRestored = false;
            InvalidateDeviceObjects();
        }
        if( m_bDeviceObjectsInited )
        {
            m_bDeviceObjectsInited = false;
            DeleteDeviceObjects();
        }

		int i;
		if( i = m_pd3dDevice->Release() > 0 ){
            Warning("%i D3D objects were not released",i);
			while(m_pd3dDevice->Release());
		}
        m_pd3dDevice = NULL;
    }
}




//-----------------------------------------------------------------------------
// Name: DisplayErrorMsg()
// Desc: Displays error messages in a message box
//-----------------------------------------------------------------------------
HRESULT D3DRenderDevice::DisplayErrorMsg( HRESULT hr, DWORD dwType )
{
    static bool s_bFatalErrorReported = false;
    TCHAR strMsg[512];

    // If a fatal error message has already been reported, the app
    // is already shutting down, so don't show more error messages.
    if( s_bFatalErrorReported )
        return hr;

    switch( hr )
    {
        case D3DAPPERR_NODIRECT3D:
            _tcscpy( strMsg, _T("Could not initialize Direct3D. You may\n")
                             _T("want to check that the latest version of\n")
                             _T("DirectX is correctly installed on your\n")
                             _T("system. This game requires DirectX9 Release Candidate 1 or later.") );
            break;

        case D3DAPPERR_NOCOMPATIBLEDEVICES:
            _tcscpy( strMsg, _T("Could not find any compatible Direct3D\n")
                             _T("devices.") );
            break;

        case D3DAPPERR_NOWINDOWABLEDEVICES:
            _tcscpy( strMsg, _T("This game cannot run in a desktop\n")
                             _T("window with the current display settings.\n")
                             _T("Please change your desktop settings to a\n")
                             _T("16- or 32-bit display mode and re-run this\n")
                             _T("sample.") );
            break;

        case D3DAPPERR_NOHARDWAREDEVICE:
            _tcscpy( strMsg, _T("No hardware-accelerated Direct3D devices\n")
                             _T("were found.") );
            break;

        case D3DAPPERR_HALNOTCOMPATIBLE:
            _tcscpy( strMsg, _T("This game requires functionality that is\n")
                             _T("not available on your Direct3D hardware\n")
                             _T("accelerator.") );
            break;

        case D3DAPPERR_NOWINDOWEDHAL:
            _tcscpy( strMsg, _T("Your Direct3D hardware accelerator cannot\n")
                             _T("render into a window.\n")
                             _T("Press F2 while the app is running to see a\n")
                             _T("list of available devices and modes.") );
            break;

        case D3DAPPERR_NODESKTOPHAL:
            _tcscpy( strMsg, _T("Your Direct3D hardware accelerator cannot\n")
                             _T("render into a window with the current\n")
                             _T("desktop display settings.\n")
                             _T("Press F2 while the app is running to see a\n")
                             _T("list of available devices and modes.") );
            break;

        case D3DAPPERR_NOHALTHISMODE:
            _tcscpy( strMsg, _T("This game requires functionality that is\n")
                             _T("not available on your Direct3D hardware\n")
                             _T("accelerator with the current desktop display\n")
                             _T("settings.\n")
                             _T("Press F2 while the app is running to see a\n")
                             _T("list of available devices and modes.") );
            break;

        case D3DAPPERR_MEDIANOTFOUND:
        case HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND ):
            _tcscpy( strMsg, _T("Could not load required media." ) );
            break;

        case D3DAPPERR_RESETFAILED:
            _tcscpy( strMsg, _T("Could not reset the Direct3D device." ) );
            break;

        case D3DAPPERR_NONZEROREFCOUNT:
            _tcscpy( strMsg, _T("A D3D object has a non-zero reference\n")
                             _T("count (meaning things were not properly\n")
                             _T("cleaned up).") );
            break;

        case D3DAPPERR_NULLREFDEVICE:
            _tcscpy( strMsg, _T("Warning: Nothing will be rendered.\n")
                             _T("The reference rendering device was selected, but your\n")
                             _T("computer only has a reduced-functionality reference device\n")
                             _T("installed.  Install the DirectX SDK to get the full\n")
                             _T("reference device.\n") );
            break;

        case E_OUTOFMEMORY:
            _tcscpy( strMsg, _T("Not enough memory.") );
            break;

        case D3DERR_OUTOFVIDEOMEMORY:
            _tcscpy( strMsg, _T("Not enough video memory.") );
            break;

        default:
            _tcscpy( strMsg, _T("Generic application error. Enable\n")
                             _T("debug output for detailed information.") );
    }

    if( MSGERR_APPMUSTEXIT == dwType )
    {
        s_bFatalErrorReported = true;
        _tcscat( strMsg, _T("\n\nThis game will now exit.") );
        Error( strMsg );

        // Close the window, which shuts down the app
        if( m_hWnd )
            SendMessage( m_hWnd, WM_CLOSE, 0, 0 );
    }
    else
    {
        if( MSGWARN_SWITCHEDTOREF == dwType )
            _tcscat( strMsg, _T("\n\nSwitching to the reference rasterizer,\n")
                             _T("a software device that implements the entire\n")
                             _T("Direct3D feature set, but runs very slowly.") );
        Error( strMsg );
    }

    return hr;
}
