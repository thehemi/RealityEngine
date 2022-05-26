//=========== (C) Copyright 2003, Tim Johnson. All rights reserved. ===========
/// Evolution GUI Manager
//
/// This is the GUI system for main menu (desktop) AND all game windows
//
/// If you're adding a desktop window you should call SetDesktopWindow(true)
/// so it will be hidden with the desktop
//
/// Features:
/// - Minimizing, moving
/// - Z-Ordering
/// - Retains window positions in a persistent file
//
//=============================================================================
#pragma once
#ifndef _GUISYSTEM
#define _GUISYSTEM
#include "..\EngineSrc\DXCommon\dxstdafx.h"

typedef void (*MessageBox_Callback)(void*);
typedef void (*Resize_Callback)(int width, int height);

#define BUILD_VERSION 10.4

//-----------------------------------------------------------------------------
/// Extended window class
//-----------------------------------------------------------------------------
class ENGINE_API CGUIWindow {
private:
	friend class GUISystem;
	bool m_bVisible;
	bool m_bEnabled;
	bool m_bDesktopWindow;		/// Will this be linked to the desktop visibility?
	string m_ConfigIdentifier;  /// Internal name used to identify windows in config

public:

	CGUIWindow(CDXUTDialog* dialog, string ConfigIdentifier = "Auto");

	class CDXUTDialog* m_Dialog;

	virtual void SetEnabled( bool bEnabled ) { m_bEnabled = bEnabled; }
    virtual bool GetEnabled() { return m_bEnabled; }
    virtual void SetVisible( bool bVisible ) { m_bVisible = bVisible; }
    virtual bool GetVisible() { return m_bVisible; }
	virtual void SetDesktopWindow( bool bDesktopWindow ) { m_bDesktopWindow = bDesktopWindow; }
    virtual bool GetDesktopWindow() { return m_bDesktopWindow; }

};

//-----------------------------------------------------------------------------
/// Picture Label Control
//-----------------------------------------------------------------------------
class ENGINE_API CPictureLabel : public CDXUTControl
{
protected:
	class Texture* m_Texture;
	D3DBLEND m_Source, m_Dest;

public:

	~CPictureLabel();
	CPictureLabel(int ID, int x, int y, int width, int height, string textureFile, D3DBLEND src = D3DBLEND_SRCALPHA, D3DBLEND dest = D3DBLEND_INVSRCALPHA);
	virtual void Render( IDirect3DDevice9* pd3dDevice, float fElapsedTime );
	virtual BOOL ContainsPoint( POINT pt ) { return false; } /// Picture labels must never take messages away from other controls
	virtual void ChangeTexture(string newTexture);
};

//-----------------------------------------------------------------------------
/// GUI System. A manager for the entire GUI
/// Provides useful functionality for controlling the desktop, messageboxes, etc
/// Sets up the default desktop buttons and layout
//-----------------------------------------------------------------------------
class ENGINE_API GUISystem {
protected:
	virtual void InitializeDesktop();
	virtual void InitializeSettings();

	Resize_Callback m_ResizeCallback;
	ConfigFile		m_Config;

	GUISystem(){ }

public:
	virtual void OnResize(); /// Window has been resized, reposition controls

	/// Singleton
	static GUISystem* Instance ();
	
	/// System
	virtual void Initialize();
	virtual void Render();
	virtual void Update();
	virtual void Shutdown();
	virtual void SetResizeCallback(Resize_Callback callback){ m_ResizeCallback = callback; }

	LRESULT MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual void OnDesktopEvent( UINT nEvent, int nControlID, class CDXUTControl* pControl );

	/// MessageBox routines
	virtual void DoMessageBox(string title, string text);
	virtual void DoMessageBoxBlocking(string title, string text, float waitTime, MessageBox_Callback callback);
	virtual void CloseMessageBoxBlocking();

	/// Desktop / Main Menu
	/// Sets visibility of not only desktop, but all desktop windows
	virtual void ShowDesktop(bool bShow);
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

	/// Vars
	CGUIWindow* m_Desktop;
	CGUIWindow* m_Graphics;
	CGUIWindow* m_Video;
	CGUIWindow* m_Game;
	CGUIWindow* m_Controls;
	CGUIWindow* m_NewGame;
	CGUIWindow* m_Credits;
	CGUIWindow* m_MessageBox;
	vector<CGUIWindow*> m_Windows;
};

#undef UNICODE

#endif