//=========== Copyright (c) 2004, Tim Johnson. All rights reserved. ===========
//
// Handles quad and main menus for Tools
//
//=============================================================================
#pragma once
#ifndef __MENUUTIL__H
#define __MENUUTIL__H

//Usefull little helper for printing into the Listner
#define ScriptPrint (the_listener->edit_stream->printf)
#define GLOBAL_MENU_INTERFACE_ID Interface_ID(0x7206659e, 0x1caf65e3)
#define PBLOCK_REFNO	0
#define kMenuUtilContext 0x1761733d //<random number> makes your actions contexual

extern TCHAR *GetString(int id);
extern HINSTANCE g_hInstance;
extern Class_ID GLOBAL_MENU_CLASS_ID;

enum { MenuUtil_params }; 

enum { 
	pb_ActionsEnabled, pb_ActionsVisible, pb_ActionsChecked,
};


class GlobalMenuActions : public FPStaticInterface {
public:

	virtual FPStatus GlobalAction1() =0;
	virtual FPStatus GlobalAction2() =0;

};

class GlobalMenu : public GUP {
public:
	static HWND hParams;

	// GUP Methods
	DWORD	Start();
	void	Stop();
	DWORD	Control( DWORD parameter );

	FPInterfaceDesc* GetDesc();

	//Constructor/Destructor
	GlobalMenu(){};
	~GlobalMenu(){};		
};
extern GlobalMenu theGlobalMenu;

#endif // __MENUUTIL__H