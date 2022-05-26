//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
/// 
/// \brief Reality GUI Manager
//
///
/// This is the GUI system for main menu (desktop) AND all game windows
///
/// If you're adding a desktop window you should call SetDesktopWindow(true)
/// so it will be hidden with the desktop
///
/// Features:
/// - Minimizing, moving
/// - Z-Ordering
/// - Retains window positions in a persistent file
///
///
/// Author: Tim Johnson
//====================================================================================
#pragma once
#ifndef _GUISYSTEM
#define _GUISYSTEM
#include "..\DXCommon\dxstdafx.h"

typedef void (*MessageBox_Callback)(void*);
typedef void (*Resize_Callback)(int width, int height);

#define BUILD_VERSION 11.1

//-----------------------------------------------------------------------------
/// Extended window class
//-----------------------------------------------------------------------------
class ENGINE_API CGUIWindow {
private:
	friend class GUISystem;
	bool m_bVisible;
	bool m_bEnabled;
	/// Will this be linked to the desktop visibility?
	bool m_bDesktopWindow;
	/// Will this Window be a desktop, always acting as if it has focus (unless occluded)
	bool m_bDesktop;
	/// Internal name used to identify windows in config
	string m_ConfigIdentifier;  

public:

	CGUIWindow(CDXUTDialog* dialog, string ConfigIdentifier = "Auto");

	class CDXUTDialog* m_Dialog;

	virtual void SetEnabled( bool bEnabled ) { m_bEnabled = bEnabled; }
    virtual bool GetEnabled() { return m_bEnabled; }
    virtual void SetVisible( bool bVisible ) { m_bVisible = bVisible; }
    virtual bool GetVisible() { return m_bVisible; }
	virtual void SetDesktopWindow( bool bDesktopWindow ) { m_bDesktopWindow = bDesktopWindow; }
    virtual bool GetDesktopWindow() { return m_bDesktopWindow; }
	virtual void SetDesktop( bool bDesktop ) { m_bDesktop = bDesktop; }
    virtual bool GetDesktop() { return m_bDesktop; }
};

//-----------------------------------------------------------------------------
/// Picture Label Control
//-----------------------------------------------------------------------------
class ENGINE_API CPictureLabel : public CDXUTControl
{
protected:
	class Texture* m_Texture;
public:
	D3DBLEND m_Source, m_Dest;
	~CPictureLabel();
	CPictureLabel(int ID, int x, int y, int width, int height, string textureFile, D3DBLEND src = D3DBLEND_SRCALPHA, D3DBLEND dest = D3DBLEND_INVSRCALPHA);
	virtual void Render( IDirect3DDevice9* pd3dDevice, float fElapsedTime );
	virtual BOOL ContainsPoint( POINT pt ) { return false; } /// Picture labels must never take messages away from other controls
	virtual void ChangeTexture(string newTexture);
	string GetTextureFilename(){return m_Texture->filename;}
};

//-----------------------------------------------------------------------------
/// \brief A manager for the entire Reality GUI
/// Provides useful functionality for controlling the desktop, messageboxes, etc
/// Sets up the default desktop buttons and layout
//-----------------------------------------------------------------------------
class ENGINE_API GUISystem {
protected:
	Resize_Callback m_ResizeCallback;
	ConfigFile		m_Config;
	GUISystem(){ m_Desktop = NULL; m_bDisplayGUI = true; }
	bool m_bDisplayGUI;
	/// Window Vars
	CGUIWindow* m_Desktop;
	CGUIWindow* m_MessageBox;

public:
	/// Window has been resized, reposition controls
	virtual void OnResize(); 

	/// Singleton
	static GUISystem* Instance ();
	
	/// System
	virtual void Initialize();
	virtual void Render();
	virtual void Shutdown();
	virtual void SetResizeCallback(Resize_Callback callback){ m_ResizeCallback = callback; }

	LRESULT MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	/// MessageBox routines
	virtual void DoMessageStrip(string text);
	virtual void DoMessageBox(string title, string text, int x=-1, int y=-1);
	virtual void DoMessageBoxBlocking(string title, string text, float waitTime, MessageBox_Callback callback);
	virtual void CloseMessageBoxBlocking();

	/// Desktop / Main Menu
	/// Sets visibility of not only desktop, but all desktop windows
	virtual void ShowDesktop(bool bShow);

	/// Sets visibility of entire GUI system. If not using GUI at all during gameplay, good to hide GUI to completely this way to eliminate processing time
	virtual void SetDisplayGUI(bool display){m_bDisplayGUI = display;}
	virtual bool GetDisplayGUI(){return m_bDisplayGUI;}

	virtual bool DesktopVisible();

	/// General window management
	virtual CGUIWindow* RegisterAsWindow(CDXUTDialog* dialog);
	virtual void RemoveWindow(CGUIWindow* window);
	virtual void BringToTop(CGUIWindow* window);

#undef FindWindow /// Stupid unicode defines
	virtual CGUIWindow* FindWindow(CDXUTDialog* dialog);

	/// Get old window positions from the config!!!
	virtual int GetX(CGUIWindow* window);
	virtual int GetY(CGUIWindow* window);

	vector<CGUIWindow*> m_Windows;

	CDXUTDialog* GetVideoSettingsDialog();
	void OnVideoSettingsEvent(UINT nEvent, int nControlID, CDXUTControl* pControl);
	void SetMessageBoxWindow(CGUIWindow* window){m_MessageBox = window;}
	CGUIWindow* GetDesktop(){return m_Desktop;}
	/// progress strip for in-game compilation of PRT
	CGUIWindow* m_Strip;
};

#undef UNICODE

#endif



