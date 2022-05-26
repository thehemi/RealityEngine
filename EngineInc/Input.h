//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
/// Name: Core Engine
///
/// Name: Input
/// Desc: Input and control mappings
///
/// Author: Tim Johnson
///
//====================================================================================
#pragma once

#define CONTROLHANDLE int

//-----------------------------------------------------------------------------
/// \brief High-level input class
/// Encapsulates a low-level input class such as DXInput
//-----------------------------------------------------------------------------
class ENGINE_API Input {
protected:
	/// Input class
	class DXInput* input; 
	/// Mouse sensitivity
	float sensitivity;

	Input();
	Input(const Input&);
	Input& operator= (const Input&);
	~Input();

	int controlIndex(string& control);

	vector<string> controlNames;
	string kbBuf, kbCmdBuf;

	/// Engine calls
	friend class Engine;
	void Initialize(ConfigFile& InputConfig, HWND hWnd, bool exclusiveMouse );
	void Shutdown();
	void Update();
	void MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

public:

	static Input* Instance ();
	void SetAcquire(bool b);
	void SetExclusive(bool exclusive);

	/// Public interface
	void SetSensitivity(float sens);
	bool GetSensitivity(){ return sensitivity;}

	float mouseYaw;
	float mousePitch;

	bool m_bInvertPitch;

	void FlushKeyboardBuffer(){ kbBuf = ("");kbCmdBuf = ("");}

	/// Holds keys pressed by user since last frame. You can use this for reading typing
	/// for chat or gui edit boxes etc
	string GetKeyboardBuffer(){ return kbBuf; } 
	string GetKeyboardCmdBuffer(){ return kbCmdBuf; }

	/// Returns a control handle which can be used to see if the control was pressed
	/// Example usage: const static CONTROLHANDLE STRAFE_LEFT = input->GetControlHandle("STRAFE_LEFT");
	CONTROLHANDLE GetControlHandle(string ControlName);

	string GetControlName(CONTROLHANDLE handle);

	/// Control is pressed
	bool ControlDown(CONTROLHANDLE cmd); 
	/// Was pressed this frame
	bool ControlJustPressed(CONTROLHANDLE cmd); 
	/// Was released this frame
	bool ControlJustReleased(CONTROLHANDLE cmd); 
	//void ControlClearFrame(CONTROLHANDLE cmd); /// Clears a 'Just' message from this frame

	void GetBoundKeys(string Control, string& key1, string& key2);

	long GetMWheelChange();
	void GetMouseMovement(int&x,int&y);

	/// Bind a control to a Key.
	/// A control can be bound to up to two keys (sets)
	void Bind(string ControlName, string KeyName, int Keyset);
};

