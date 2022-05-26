// Name: DXInput.cpp
// Desc: Mouse and keyboard Input processing
// Made: 28/June/2000 Tim
#include "stdafx.h"
#define DIRECTINPUT_VERSION 0x0800
#include "DXInput.h"
#include <fstream>
#include <stdio.h>
#include <dxerr9.h>

// Key state this frame
bool DXInput::GetKeystate(int key){
	return (diks[key] & 0x80);
}

bool DXInput::GetMousestate(int mouseButton){
	switch(mouseButton){
	case DIM_MOUSE1:
		return dims.rgbButtons[0] & 0x80;
	case DIM_MOUSE2:
		return dims.rgbButtons[1] & 0x80;
	case DIM_MOUSE3:
		return dims.rgbButtons[2] & 0x80;
	case DIM_MOUSE4:
		return dims.rgbButtons[3] & 0x80;
	default:
		Error("Invalid mouse state requested in DXInput class");
	}
	return 0;
}


void DXInput::SetExclusive(bool exclusive){
	SetAcquire(false);
	HRESULT hr;
	if(exclusive)
		hr = g_pMouse->SetCooperativeLevel( hWnd, 
                                     DISCL_EXCLUSIVE  | DISCL_FOREGROUND );
	else
		hr = g_pMouse->SetCooperativeLevel( hWnd, 
                                     DISCL_NONEXCLUSIVE  | DISCL_FOREGROUND );

	g_exclusiveMouse = exclusive;

	if(FAILED(hr))
		Error("Error setting mouse input mode");

	SetAcquire(true);
}


// Key state at last frame
bool DXInput::GetOldKeystate(int key){
	return (diksLast[key] & 0x80);
}

bool DXInput::GetOldMousestate(int mouseButton){
	switch(mouseButton){
	case DIM_MOUSE1:
		return dimsLast.rgbButtons[0] & 0x80;
	case DIM_MOUSE2:
		return dimsLast.rgbButtons[1] & 0x80;
	case DIM_MOUSE3:
		return dimsLast.rgbButtons[2] & 0x80;
	case DIM_MOUSE4:
		return dimsLast.rgbButtons[3] & 0x80;
	default:
		Error("Invalid mouse state requested in DXInput class");
	}
	return 0;
}

//-----------------------------------------------------------------------------
// Name: InitDirectDXInput()
// Desc: Initialize the DirectDXInput variables.
//-----------------------------------------------------------------------------
HRESULT DXInput::Initialize( HWND hWnd, bool exclusiveMouse )
{
    HRESULT hr;
	this->hWnd = hWnd;

    // Register with the DirectDXInput subsystem and get a pointer
    // to a IDirectDXInput interface we can use.
        // Create a DDXInput object
    if( FAILED( hr = DirectInput8Create( GetModuleHandle(NULL), DIRECTINPUT_VERSION, 
		IID_IDirectInput8, (VOID**)&g_pDI, NULL ) ) ){
		Error("Input Creation failed: %s",DXGetErrorString9(hr));
	}

	//-------------
	// Keyboard
	//-------------
    // Obtain an interface to the system keyboard device.
    hr = g_pDI->CreateDevice( GUID_SysKeyboard, &g_pKeyboard, NULL );
    if( FAILED(hr) ) 
        Error("Keyboard Input Creation failed: %s",DXGetErrorString9(hr));


    // This tells DirectDXInput that we will be passing an array
    // of 256 bytes to IDirectDXInputDevice::GetDeviceState.
    hr = g_pKeyboard->SetDataFormat( &c_dfDIKeyboard );
    if( FAILED(hr) ) 
        Error("Keyboard Input SetDataFormat() failed: %s",DXGetErrorString9(hr));

	 // Set the cooperativity level to let DirectDXInput know how
    // this device should interact with the system and with other
    // DirectDXInput applications.
    hr = g_pKeyboard->SetCooperativeLevel( hWnd, 
                                     DISCL_NONEXCLUSIVE  | DISCL_FOREGROUND );
    if( FAILED(hr) ) 
        Error("Keyboard Input SetCooperativeLevel() failed: %s",DXGetErrorString9(hr));


	//-------------
	// Mouse
	//-------------
	// Obtain an interface to the system mouse device.
    hr = g_pDI->CreateDevice( GUID_SysMouse, &g_pMouse, NULL );
    if ( FAILED(hr) ) 
        Error("Mouse Input Creation failed: %s",DXGetErrorString9(hr));

    // Set the data format to "mouse format" - a predefined data format 
    // A data format specifies which controls on a device we
    // are interested in, and how they should be reported.
    hr = g_pMouse->SetDataFormat( &c_dfDIMouse );
    if ( FAILED(hr) ) 
        Error("Mouse Input SetDataFormat() failed: %s",DXGetErrorString9(hr));

	SetExclusive(exclusiveMouse);

	// Don't want to trigger undefined behaviour
	ZeroMemory(diks,sizeof(diks));
	ZeroMemory(&dims,sizeof(dims));

	// Set aquire depending on whether windows has set to active or not
	// Now, windows notifies us of our window being activated, but never if
	// our window starts in the background. So the default for g_bActive is off
	SetAcquire(g_bActive);

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: SetAcquire()
// Desc: Acquire or unacquire the keyboard, depending on if the app is active
//       DXInput device must be acquired before the GetDeviceState is called
//-----------------------------------------------------------------------------
void DXInput::SetAcquire(bool acquire )
{
	g_bActive = acquire;
    // Nothing to do if g_pKeyboard is NULL
    if( NULL == g_pKeyboard || NULL == g_pMouse )
        return;

    if( acquire ) 
    {
        // Acquire the DXInput device 
        g_pKeyboard->Acquire();
		g_pMouse->Acquire();
    } 
    else 
    {
        // Unacquire the DXInput device 
        g_pKeyboard->Unacquire();
		g_pMouse->Unacquire();
    }
}




//-----------------------------------------------------------------------------
// Name: UpdateDXInputState()
// Desc: Get the DXInput device's state and display it.
//-----------------------------------------------------------------------------
void DXInput::UpdateInput()
{
    if( g_pKeyboard ) 
    {
        HRESULT hr;

		memcpy(diksLast,diks,256);

        // Get the DXInput's device state, and put the state in dims
        hr = g_pKeyboard->GetDeviceState( sizeof(diks), &diks );
        if( FAILED(hr) )  
        {
            // DirectDXInput is telling us that the DXInput stream has been
            // interrupted.  We aren't tracking any state between polls, so
            // we don't have any special reset that needs to be done.
            // We just re-acquire and try again.

            // If DXInput is lost then acquire and keep trying 
			while( hr == DIERR_INPUTLOST ) 
				hr = g_pKeyboard->Acquire();

			// Reset diks in the mean-time, so no garbage data is given to the game
			ZeroMemory(diks,sizeof(diks));
        }

    }

	if (g_pMouse) 
    {
        HRESULT hr;

        hr = DIERR_INPUTLOST;

		memcpy(&dimsLast,&dims,sizeof(DIMOUSESTATE));

        // get the DXInput's device state, and put the state in dims
        hr = g_pMouse->GetDeviceState( sizeof(DIMOUSESTATE), &dims );

		if( FAILED(hr) ) 
		{
			// DirectInput may be telling us that the input stream has been
			// interrupted.  We aren't tracking any state between polls, so
			// we don't have any special reset that needs to be done.
			// We just re-acquire and try again.

			// Fixme: Hack to get control of mouse even if windows wasn't properly notified
			// This is rather rude
			// See sysmain.cpp for more info on the problem
			// Which is caused by windows not sending an activated message when app starts
			// in background
			//if(!g_exclusiveMouse)
				SetAcquire(true);
        
			// If input is lost then acquire and keep trying 
            while( hr == DIERR_INPUTLOST ) 
				 hr = g_pMouse->Acquire();

			// hr may be DIERR_OTHERAPPHASPRIO or other errors.  This
			// may occur when the app is minimized or in the process of 
			// switching, so just try again later
			//if( hr == DIERR_OTHERAPPHASPRIO || hr == DIERR_NOTACQUIRED ) 
				//Error("Unaquired");//SetDlgItemText( hDlg, IDC_DATA, TEXT("Unacquired") );

			// Reset dims in the mean-time, so no garbage data is given to the game
			ZeroMemory(&dims,sizeof(dims));
			return;
        }
    }
}



//-----------------------------------------------------------------------------
// Name: FreeDirectDXInput()
// Desc: Clear everything up
//-----------------------------------------------------------------------------
HRESULT DXInput::Shutdown()
{
    // Unacquire and release any DirectDXInputDevice objects.
    if( g_pKeyboard ) 
    {
        // Unacquire the device one last time just in case 
        // the app tried to exit while the device is still acquired.
        g_pKeyboard->Unacquire();
        g_pKeyboard->Release();
        g_pKeyboard = NULL;
    }

	if( g_pMouse ){
		// Unacquire the device one last time just in case 
        // the app tried to exit while the device is still acquired.
        g_pMouse->Unacquire();
        g_pMouse->Release();
        g_pMouse = NULL;
	}

    // Release any DirectDXInput objects.
    if( g_pDI ) 
    {
        g_pDI->Release();
        g_pDI = NULL;
    }

    return S_OK;
}




