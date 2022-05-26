//=========== Copyright (c) 2003, Tim Johnson. All rights reserved. ===========
// Name: Input
// Desc: Sits on top of Input to map Input to game-controlNames
//=============================================================================
#include "stdafx.h"
#define DIRECTINPUT_VERSION 0x0800
#include "Input.h"
#include "DXInput.h"


// Returns a singleton instance
Input* Input::Instance () 
{
    static Input inst;
    return &inst;
}


// Just for the mouse
string strSensitivity = "sensitivity";


int keymappings[100][2]; // 2 keys per command MAX


#define MWHEEL_DOWN 600
#define MWHEEL_UP   601

/// An input key's given name and its handle
struct keyinfo{
	CHAR* keyCodeName;
	int keycode;
};

// Must be lowercase
// Can assign multiple names to a key
keyinfo keycodes[]={
	// Letters
	{"a",DIK_A},{"b",DIK_B},{"c",DIK_C},{"d",DIK_D},{"e",DIK_E},{"f",DIK_F},{"g",DIK_G},{"h",DIK_H},{"i",DIK_I},
	{"j",DIK_J},{"k",DIK_K},{"l",DIK_L},{"m",DIK_M},{"n",DIK_N},{"o",DIK_O},{"p",DIK_P},{"q",DIK_Q},{"r",DIK_R},
	{"s",DIK_S},{"t",DIK_T},{"u",DIK_U},{"v",DIK_V},{"w",DIK_W},{"x",DIK_X},{"y",DIK_Y},{"z",DIK_Z},
	// Misc
	{".",DIK_PERIOD},{",",DIK_COMMA},{"backspace",DIK_BACK},{"spacebar",DIK_SPACE},{"capslock",DIK_CAPITAL},
	{"left shift",DIK_LSHIFT},{"right shift",DIK_RSHIFT},{"escape",DIK_ESCAPE},{"tab",DIK_TAB},
	{"enter",DIK_RETURN},{"\\",DIK_BACKSLASH},{"/",DIK_SLASH},{";",DIK_SEMICOLON},{"'",DIK_APOSTROPHE},{"@",DIK_GRAVE},
	{"-",DIK_MINUS},{"=",DIK_EQUALS},{"[",DIK_LBRACKET},{"]",DIK_RBRACKET},
	{"alt",DIK_LMENU},{"control",DIK_LCONTROL},{"right control",DIK_RCONTROL}, {"Page Up",DIK_PRIOR},{"Page Down",DIK_NEXT},
	{"delete",DIK_DELETE},
	// F-Keys
	{"f1",DIK_F1},{"f2",DIK_F2},{"f3",DIK_F3},{"f4",DIK_F4},{"f5",DIK_F5},{"f6",DIK_F6},{"f7",DIK_F7},
	{"f8",DIK_F8},{"f9",DIK_F9},{"f10",DIK_F10},{"f11",DIK_F11},{"f12",DIK_F12},
	// Arrows
	{"up arrow",DIK_UP},{"down arrow",DIK_DOWN},{"left arrow",DIK_LEFT},{"right arrow",DIK_RIGHT},
	//Numbers
	{"1",DIK_1},{"2",DIK_2},{"3",DIK_3},{"4",DIK_4},{"5",DIK_5},{"6",DIK_6},{"7",DIK_7},{"8",DIK_8},{"9",DIK_9},{"0",DIK_0},
	// Mice
	{"mouse1",DIM_MOUSE1},{"mouse2",DIM_MOUSE2},{"mouse3",DIM_MOUSE3},{"mouse4",DIM_MOUSE4},
	{"mousewheelup",MWHEEL_UP},{"mousewheeldown",MWHEEL_DOWN},
	// Numpad
	{"NUMPAD1",DIK_NUMPAD1},{"NUMPAD2",DIK_NUMPAD2},{"NUMPAD3",DIK_NUMPAD3},{"NUMPAD4",DIK_NUMPAD4},{"NUMPAD5",DIK_NUMPAD5},{"NUMPAD6",DIK_NUMPAD6},{"NUMPAD7",DIK_NUMPAD7},{"NUMPAD8",DIK_NUMPAD8},{"NUMPAD9",DIK_NUMPAD9},{"NUMPAD0",DIK_NUMPAD0},

};

const static int KEY_CODES = sizeof(keycodes) / sizeof(keycodes[0]);


long Input::GetMWheelChange(){
	return input->GetMouseZChange();
}

Input::Input(){
	mouseYaw = 0;
	mousePitch = 0;
	sensitivity = 1;
}

Input::~Input(){
}

void Input::Shutdown(){
	input->Shutdown();
	SAFE_DELETE(input);
}

string Input::GetControlName(CONTROLHANDLE handle){
	return controlNames[handle];
}

CONTROLHANDLE Input::GetControlHandle(string ControlName){
	ToLowerCase(ControlName);

	for(int i=0;i<controlNames.size();i++){
		if(controlNames[i] == ControlName)
			return i;
	}
	Warning("GetControlHandle(): Control: '%s' wasn't found in the config file",ControlName.c_str());
	return -1;
}

//--------------------------------------------------------------------------------
// Non-Member Helper functions
//--------------------------------------------------------------------------------
int keyIndex(string keyName){
	ToLowerCase(keyName);
	for(int i=0;i<KEY_CODES;i++){
		if(keyName == keycodes[i].keyCodeName)
			return keycodes[i].keycode;
	}
	return -1;
}

string keyName(int index){
	for(int i=0;i<KEY_CODES;i++){
		if(index == keycodes[i].keycode)
			return keycodes[i].keyCodeName;
	}
	return "";
}



int Input::controlIndex(string& control){
	string lower = control;
	ToLowerCase(lower);
	for(int i=0;i<controlNames.size();i++){
		string lower2 = controlNames[i];
		ToLowerCase(lower2);
		if(lower == lower2)
			return i;
	}
	return -1;
}





void Input::SetExclusive(bool exclusive){
	input->SetExclusive(exclusive);
}


//--------------------------------------------------------------------------------
// GetBoundKeys - Returns the bound keys for command control
//--------------------------------------------------------------------------------
void Input::GetBoundKeys(string control, string& key1, string& key2){
	// Get the controlcode for the control name
	int cIndex = controlIndex(control);

	// Invalid keyname or commandname
	if(cIndex == -1)
		Error("GetBoundKeys(): Invalid keyname or command name for key mapping in config. ");

	// Assign key identifier to keycode
	key1 = keyName(keymappings[cIndex][0]);
	key2 = keyName(keymappings[cIndex][1]);
}

//--------------------------------------------------------------------------------
// This has to find the namecode for the control string, then link the namecode
// to the keycode
//--------------------------------------------------------------------------------
void Input::Bind(string control, string keyName, int keySet){
	if(control.length() == 0 || keyName.length() == 0)
		return;

	// Get the keycode for the keyname
	int kIndex = keyIndex(keyName);

	// Get the controlcode for the control name
	int cIndex = controlIndex(control);

	// Invalid keyname or commandname
	if(kIndex == -1|| cIndex == -1)
		Error("Bind(): Invalid keyname or command name for key mapping in config.\nCommand: %s\nKey Name: %s",control.c_str(),keyName.c_str());

	// Assign key identifier to keycode
	keymappings[cIndex][keySet] = kIndex;
}

//--------------------------------------------------------------------------------
// Initialize function
// Needs a config to load bindings from
// Acts as a frontend for input initialization
//--------------------------------------------------------------------------------
void Input::Initialize(ConfigFile& inputConfig, HWND hWnd, bool exclusiveMouse){
	input = new DXInput;
	input->Initialize(hWnd,exclusiveMouse);

	// Set the mouse sensitivity
	// It's scaled to give a more user-friendly settings choice
	// 1-10 is usually a good choice
	sensitivity = inputConfig.GetFloat(strSensitivity) * 0.05f;
	m_bInvertPitch = inputConfig.GetBool("InvertPitch");

	vector<string> KeyNames;

	// Read in all control names (every key in the inputcontrols section) and values
	inputConfig.GetSection("InputControls",controlNames,KeyNames);

	// Init keymappings with dummy values, so we know if a key mapping is unbound
	for(int i=0;i<100;i++){
		keymappings[i][0] = -1;
		keymappings[i][1] = -1;
	}

	// Loop through all command names looking for keys for them
	for(int i=0;i<controlNames.size();i++){
		string keyName = KeyNames[i];
		trimLeft(keyName);
		trimRight(keyName);

		if(keyName.find(",") != -1 && keyName.length() > 2){
			// Two keys listed for this command

			// Check for the awful case of a comma in our comma-delineated pair!!
			string check = keyName;
			check = check.substr(check.find(",")+1);
			if(check.find(",")!=-1){
				int lastcomma = check.find(",");
				// Comma is last key
				if(lastcomma == check.length()-1){
					string k1 = keyName.substr(0,keyName.find(","));
					trimRight(k1);
					Bind(controlNames[i],k1,0);
					Bind(controlNames[i],",",1);
				}
				else{ // Comma is first key
					Bind(controlNames[i],",",0);
					string k2 = keyName.substr(keyName.find_last_of(",")+1);
					trimLeft(k2);
					Bind(controlNames[i],k2,1);
				}
			}
			else{
				// Normal pair, no comma
				string k1 = keyName.substr(0,keyName.find(","));
				trimRight(k1);
				string k2 = keyName.substr(keyName.find(",")+1);
				trimLeft(k2);
				Bind(controlNames[i],k1,0);
				Bind(controlNames[i],k2,1);
			}
		}
		else{
			Bind(controlNames[i],keyName,0);
		}
	}
}

void Input::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
	switch (uMsg) 
	{
		case WM_ACTIVATE:
			if (WA_INACTIVE == wParam){
				// This is only needed for fullscreen/exclusive
				// Otherwise, windows snatches focus from us automatically
				SetAcquire(false);
			}
			else{
				SetAcquire(true);
			}
		break;

		case WM_CHAR:
			kbBuf += wParam;
			break;

		case WM_KEYDOWN:
			// add commands
			if(!isprint(wParam))
				kbCmdBuf += wParam;
			break;
	}
}

//--------------------------------------------------------------------------------
// Update the input controlNames once a tick
//--------------------------------------------------------------------------------
void Input::Update(){
	input->UpdateInput();

	if(!input->Active())
		return;

	mouseYaw += input->GetMouseX()*sensitivity;
	mousePitch += input->GetMouseY()*(sensitivity * (!m_bInvertPitch*2 - 1));

	// Neck can only bend 180 degrees up and down, unless this becomes a jelloman game
	if(mousePitch>89)  mousePitch = 89;
	if(mousePitch<-89) mousePitch =-89;	
}


void Input::SetSensitivity(float sens){
	sensitivity = sens;
}


void Input::SetAcquire(bool b){ input->SetAcquire(b); }



void Input::GetMouseMovement(int&x,int&y){
	x = input->GetMouseX();
	y = input->GetMouseY();
}

/*
Mouse is just another Key.
MouseState Input::GetMouseState(int button){
	// Return up + (up and was down)
	// 1 if up, 2 if just released
	bool UpState = !input->GetMousestate(DIM_MOUSE1) + 
		(!input->GetMousestate(DIM_MOUSE1)&&input->GetOldMousestate(DIM_MOUSE1));

	// Return down + (down and was up)
	// Returns 1 if down, 2 if just clicked
	bool DownState = input->GetMousestate(DIM_MOUSE1) + 
		(input->GetMousestate(DIM_MOUSE1)&&!input->GetOldMousestate(DIM_MOUSE1));

	if(!input->GetMousestate(DIM_MOUSE1))
		return MOUSE_UP;

	if((!input->GetMousestate(DIM_MOUSE1)&&input->GetOldMousestate(DIM_MOUSE1)))
		return MOUSE_JUST_UP;

}
*/

/*
int Input::MouseUp(){
	// Return up + (up and was down)
	return !input->GetMousestate(DIM_MOUSE1) + 
		(!input->GetMousestate(DIM_MOUSE1)&&input->GetOldMousestate(DIM_MOUSE1));
}

// Returns 1 if down, 2 if just clicked
int Input::MouseDown(){
	// Return down + (down and was up)
	return input->GetMousestate(DIM_MOUSE1) + 
		(input->GetMousestate(DIM_MOUSE1)&&!input->GetOldMousestate(DIM_MOUSE1));
}
*/

//--------------------------------------------------------------------------------
// Returns true if command is down
//--------------------------------------------------------------------------------
bool Input::ControlDown(int ControlName){
	if(ControlName == -1)
		return false;
	// Is this a mouse command or keyboard command?
	
	// Loop for each keymapping set
	bool result[2];
	for(int i=0;i<2;i++){
		if(keymappings[ControlName][i] == -1)
			result[i] = false;
		else if(keymappings[ControlName][i] >= DIM_MOUSE1 && keymappings[ControlName][i] <= DIM_MOUSE4)
			result[i] = input->GetMousestate(keymappings[ControlName][i]);
		else
			result[i] = input->GetKeystate(keymappings[ControlName][i]);
	}

	return (result[0]||result[1]);
}

//--------------------------------------------------------------------------------
// Returns true if commandstate has changed
//--------------------------------------------------------------------------------
bool Input::ControlJustReleased(int ControlName){
	if(ControlName == -1)
		return false;

#ifdef _DEBUG
	if(ControlName < 0 || ControlName > 99)
		Error("ControlHandle for ControlJustReleased was %d",ControlName);
	assert(input);
#endif

	// Is this a mouse command or keyboard command?

	// Loop for each keymapping set
	bool result[2];
	for(int i=0;i<2;i++){
		if(keymappings[ControlName][i] == -1)
			result[i] = false;
		else if(keymappings[ControlName][i] >= DIM_MOUSE1 && keymappings[ControlName][i] <= DIM_MOUSE4)
			result[i] = input->GetOldMousestate(keymappings[ControlName][i])&&!input->GetMousestate(keymappings[ControlName][i]);
		else
		// Return true if the keystate has changed
			result[i] =  (input->GetOldKeystate(keymappings[ControlName][i])) &&
				 (!input->GetKeystate(keymappings[ControlName][i]));
	}

	return (result[0]||result[1]);
}


//--------------------------------------------------------------------------------
// Returns true if commandstate has changed
//--------------------------------------------------------------------------------
bool Input::ControlJustPressed(int ControlName){
	if(ControlName == -1)
		return false;
	
#ifdef _DEBUG
	if(ControlName < 0 || ControlName > 99)
		Error("ControlHandle for ControlJustPressed was %d",ControlName);
	assert(input);
#endif

	// Is this a mouse command or keyboard command?

	// Loop for each keymapping set
	bool result[2];
	for(int i=0;i<2;i++){
		if(keymappings[ControlName][i] == MWHEEL_DOWN)
			result[i] = GetMWheelChange() < 0;
		else if(keymappings[ControlName][i] == MWHEEL_UP)
			result[i] = GetMWheelChange() > 0;
		else if(keymappings[ControlName][i] == -1)
			result[i] = false;
		else if(keymappings[ControlName][i] >= DIM_MOUSE1 && keymappings[ControlName][i] <= DIM_MOUSE4)
			result[i] = !input->GetOldMousestate(keymappings[ControlName][i])&&input->GetMousestate(keymappings[ControlName][i]);
		else
		// Return true if the keystate has changed
			result[i] =  (!input->GetOldKeystate(keymappings[ControlName][i])) &&
				 (input->GetKeystate(keymappings[ControlName][i]));
	}

	return (result[0]||result[1]);
}

