//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
/// Name: DXInput.h
/// Desc: Mouse and keyboard Input processing
///
/// Author: 28/June/2000 Tim Johnson
//====================================================================================

#ifndef DXInput_H
#define DXInput_H
#include <windows.h>
#include <dinput.h>

/// Extended key codes
#define DIM_MOUSE1 400
#define DIM_MOUSE2 401
#define DIM_MOUSE3 402
#define DIM_MOUSE4 403

/// Encapsulates DirectInput to query mouse, keyboard, and joystick inputs states
class DXInput {
private:
	LPDIRECTINPUT       g_pDI ;         
	LPDIRECTINPUTDEVICE g_pKeyboard, g_pMouse;    
	BOOL                g_bActive;    
	BOOL				g_exclusiveMouse;
	BYTE    diks[256];   /// DirectDXInput keyboard state buffer
	BYTE    diksLast[256];   /// Old DirectDXInput keyboard state buffer 
	DIMOUSESTATE dims;
	DIMOUSESTATE dimsLast;
	HWND hWnd;

	//---------------------------------
	/// User DXInput data storage
	//---------------------------------

	/// Keyboard DXInput for a frame. DDXInput is no good for typing
	string windowsKeyboardDXInput;

public:
	long GetMouseX(){return dims.lX;}
	long GetMouseY(){return dims.lY;}
	long GetMouseZ(){return dims.lZ;}
	long GetMouseZChange(){ return dims.lZ; }

	bool Active(){ return g_bActive; }
	

	DXInput(){
		g_bActive = FALSE;
		g_pKeyboard = NULL;
		g_pMouse = NULL;
		g_pDI = NULL;
		g_exclusiveMouse = FALSE;
	}
	


	//---------------------------------
	/// DirectDXInput functions
	//---------------------------------
	void SetAcquire(bool acquire);
	HRESULT Initialize( HWND hWnd, bool exclusiveMouse );
	HRESULT Shutdown();
	void SetExclusive(bool exclusive);


	//---------------------------------
	/// Processes the DXInput for the frame and dispatches the commands
	//---------------------------------
	void UpdateInput();
	bool GetKeystate(int key);
	bool GetMousestate(int mouseButton);

	bool GetOldKeystate(int key);
	bool GetOldMousestate(int mouseButton);
	//static void ProcessKeyboard();
};


#endif //TD_DXInput

