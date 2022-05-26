//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
// GUI Engine
//
//====================================================================================
#include "stdafx.h"
#include "dxstdafx.h"
#include "GUISystem.h"
#include "StreamingOgg.h"
#include "Editor.h"

// Default window colors
COLOR a = D3DCOLOR_ARGB(240, 20, 23, 90); 
COLOR b = D3DCOLOR_ARGB(240, 76, 77, 99);
COLOR c = D3DCOLOR_ARGB(240, 20, 55, 70);
COLOR d = D3DCOLOR_ARGB(240, 81,  88, 102);
// Transparent window colors, when working with controls overlaying game
COLOR a1 = D3DCOLOR_ARGB(180, 20, 23, 120); 
COLOR b1 = D3DCOLOR_ARGB(180, 76, 124, 129);
COLOR c1 = D3DCOLOR_ARGB(180, 20, 100, 120);
COLOR d1 = D3DCOLOR_ARGB(180, 81,  104, 122);

/*COLOR a = D3DCOLOR_ARGB(240, 77, 99, 120); 
COLOR b = D3DCOLOR_ARGB(240, 44, 88, 111);
COLOR c = D3DCOLOR_ARGB(240, 44, 88, 111);
COLOR d = D3DCOLOR_ARGB(240, 10,  73, 133);*/


//-----------------------------------------------------------------------------
// Desktop button IDs
//-----------------------------------------------------------------------------
#define IDC_BACKGROUND_LEFT	   300
#define IDC_BACKGROUND_RIGHT   200
#define IDC_STATIC             100
#define IDC_BACK				1
#define IDC_GAME_SETTINGS		2
#define IDC_VIDEO_SETTINGS		3
#define IDC_ABOUT				4
#define IDC_EXIT				5
#define IDC_PLAY_GAME			6
#define IDC_LEVELMAP_BUTTON		7
#define IDC_CHANGE_GAMMA		8
#define IDC_MESSAGEBOX_OK		9
#define IDC_CREDITS_OK			10
#define IDC_GAME_CONTROLS		11
#define IDC_GRAPHICS_SETTINGS	12

bool m_MessageBoxBlocked = false;


//-----------------------------------------------------------------------------
// Window Constructor
//-----------------------------------------------------------------------------
CGUIWindow::CGUIWindow(CDXUTDialog* dialog, string ConfigIdentifier /*= "Auto"*/){
	m_Dialog			= dialog;
	m_Dialog->SetBackgroundColors(a,b,c,d);
	m_bVisible			= false;
	m_bEnabled			= true;
	m_ConfigIdentifier	= ConfigIdentifier;
	// If auto identifier, give us a unique identifier based on loading order,
	// which should hold until someone adds a new window further up the loading chain
	if(m_ConfigIdentifier == "Auto"){
		static int autoCounter = 0;
		m_ConfigIdentifier = "Window_"+ToStr(autoCounter);
		autoCounter++;
	}

	// Trigger an update of the position from the config
	m_Dialog->SetLocation(GUISystem::Instance()->GetX(this),GUISystem::Instance()->GetY(this));
}

//-----------------------------------------------------------------------------
// Picture Label
//-----------------------------------------------------------------------------
CPictureLabel::CPictureLabel(int ID, int x, int y, int width, int height, string textureFile, D3DBLEND src, D3DBLEND dest)
{
	m_rcBoundingBox.bottom = m_rcBoundingBox.top = m_rcBoundingBox.right = 0;
	m_Source = src;
	m_Dest	 = dest;
	SetID( ID ); 
    SetLocation( x, y );
    SetSize( width, height );

	m_Texture = new Texture;
	m_Texture->Load(textureFile);
}

//-----------------------------------------------------------------------------
// Change Texture
//-----------------------------------------------------------------------------
void CPictureLabel::ChangeTexture(string newTexture){
	m_Texture->Load(newTexture);
}

//-----------------------------------------------------------------------------
// Picture Label
//-----------------------------------------------------------------------------
void CPictureLabel::Render( IDirect3DDevice9* pd3dDevice, float fElapsedTime )
{
	if( m_bVisible == false )
		return;

	DXUT_CONTROL_STATE iState = DXUT_STATE_NORMAL;

	if( m_bEnabled == false )
		iState = DXUT_STATE_DISABLED;

	GetElement(0)->TextureColor.States[DXUT_STATE_NORMAL] = 0xFFFFFFFF;
	GetElement(0)->TextureColor.Blend(DXUT_STATE_NORMAL,fElapsedTime);
	UpdateRects();
	RECT rcWindow = m_rcBoundingBox;
	OffsetRect(&rcWindow, m_pDialog->GetX(), m_pDialog->GetY()+m_pDialog->GetCaptionHeight());

	// Capture the states, because we alter the shaders in a second
	LPDIRECT3DSTATEBLOCK9 states;
	RenderWrap::dev->CreateStateBlock(D3DSBT_ALL,&states);
	states->Capture();
	RenderWrap::SetSS( 0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
	RenderWrap::SetSS( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	RenderWrap::SetSS( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	RenderWrap::SetRS(D3DRS_DITHERENABLE,FALSE);
	RenderDevice::Instance()->GetCanvas()->Box(GetElement(0)->TextureColor.Current,rcWindow.left,rcWindow.top,m_width,m_height,m_Texture,(BlendMode)m_Source,(BlendMode)m_Dest);
	
	states->Apply();
	states->Release();
	RenderWrap::ClearTextureLevel(0);
	
}

CPictureLabel::~CPictureLabel(){
	SAFE_DELETE(m_Texture);
}


// Returns a singleton instance
GUISystem* GUISystem::Instance () 
{
    static GUISystem inst;
    return &inst;
}

//--------------------------------------------------------------------------------------
// Handles the GUI events
// You should probably have one of these per window
//--------------------------------------------------------------------------------------
void CALLBACK OnDesktopEvent( UINT nEvent, int nControlID, CDXUTControl* pControl )
{
	GUISystem::Instance()->OnDesktopEvent(nEvent,nControlID,pControl);
}
void CALLBACK VideoSettingsCallBack(UINT nEvent, int nControlID, CDXUTControl* pControl)
{
	((CD3DSettingsDlg*)GUISystem::Instance()->m_Video->m_Dialog)->OnEvent(nEvent,nControlID,pControl);
	switch( nControlID )
	{
	case DXUTSETTINGSDLG_CANCEL:
		GUISystem::Instance()->m_Video->SetVisible(false);
		GUISystem::Instance()->m_Desktop->SetEnabled(true);
		break;
	case DXUTSETTINGSDLG_OK:
		GUISystem::Instance()->m_Video->m_Dialog->SetSize(500,500);
		GUISystem::Instance()->m_Video->SetVisible(false);
		GUISystem::Instance()->m_Desktop->SetEnabled(true);
		break;
	}
}

void GUISystem::OnDesktopEvent( UINT nEvent, int nControlID, CDXUTControl* pControl )
{
	switch( nControlID )
	{
	case IDC_BACK:
		{
			for(int i = 1; i < m_Windows.size();i++)
			{
				m_Windows[i]->SetVisible(false);
			}
			m_Desktop->SetEnabled(true);
			break;
		}
	case IDC_PLAY_GAME:
		m_NewGame->SetVisible(true);
		BringToTop(m_NewGame);
		break;
	case IDC_VIDEO_SETTINGS:
		m_Video->SetVisible(true);
		BringToTop(m_Video);
		break;
	case IDC_GRAPHICS_SETTINGS:
		m_Graphics->SetVisible(true);
		BringToTop(m_Graphics);
		break;
	case IDC_GAME_SETTINGS:
		//m_Game->SetVisible(true);
		//BringToTop(m_Game);
		break;
	case IDC_GAME_CONTROLS:
		if(m_Controls)
		{
			m_Controls->SetVisible(true);
			BringToTop(m_Controls);
		}
		break;
	case IDC_ABOUT:
		m_Credits->SetVisible(true);
		BringToTop(m_Credits);
		break;
	case IDC_EXIT:
		// This is the 'proper' way to force quit without causing any exceptions
		SendMessage(Engine::Instance()->hWnd,WM_SYSCOMMAND,SC_CLOSE,0);
		exit(0);
		break;
	case IDC_MESSAGEBOX_OK:
		m_MessageBox->SetVisible(false);
		break;
	case IDC_CREDITS_OK:
		m_Credits->SetVisible(false);
		break;
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
int GUISystem::GetX(CGUIWindow* window){
	if(m_Config.KeyExists(window->m_ConfigIdentifier+"_X"))
		return m_Config.GetInt(window->m_ConfigIdentifier+"_X");
	else
		return 100;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
int GUISystem::GetY(CGUIWindow* window){
	if(m_Config.KeyExists(window->m_ConfigIdentifier+"_Y"))
		return m_Config.GetInt(window->m_ConfigIdentifier+"_Y");
	else
		return 100;
}

//-----------------------------------------------------------------------------
// Window resize, probably need to resize/reposition controls
//-----------------------------------------------------------------------------
void GUISystem::OnResize(){
	if(!m_Desktop)
		return;

	int windowX = RenderDevice::Instance()->GetViewportX();
	int windowY = RenderDevice::Instance()->GetViewportY();

	// Desktop covers entire screen
	m_Desktop->m_Dialog->SetSize(windowX,windowY);

	/*CDXUTControl* bg = m_Desktop->m_Dialog->GetControl(IDC_BACKGROUND_LEFT);
	bg->SetLocation(0,0);
	bg->SetSize(430,windowY);
	*/

	CDXUTControl* bg = m_Desktop->m_Dialog->GetControl(IDC_BACKGROUND_RIGHT);
	bg->SetSize(293,108);
	bg->SetLocation(windowX-bg->m_width,0);
	
	for(int i=0;i<m_Windows.size();i++){
		if(m_Windows[i]->m_Dialog->GetX() >= windowX || m_Windows[i]->m_Dialog->GetY() > windowY)
			m_Windows[i]->m_Dialog->SetLocation(100,100);
	}

	// Let game do some resizing
	if(m_ResizeCallback)
		m_ResizeCallback(windowX,windowY);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void GUISystem::InitializeSettings(){
	m_Video = new CGUIWindow(DXUTGetSettingsDialog()->GetDialogControl());
	m_Windows.push_back(m_Video);
	CDXUTDialog* window = m_Video->m_Dialog;
	window->SetCallback(VideoSettingsCallBack);
	window->SetSize(500,500);
	//window->SetBackgroundColors(D3DCOLOR_RGBA(32,32,55,255));
	window->SetBackgroundColors(a,b,c,d);
	window->EnableCaption(true);
	window->SetCaptionText(L"Graphics Settings");
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void GUISystem::InitializeDesktop(){
	m_Desktop = new CGUIWindow(new CDXUTDialog);
	CDXUTDialog* desktop = m_Desktop->m_Dialog;
	desktop->SetCallback(::OnDesktopEvent);
	// Desktop covers entire screen
	desktop->SetSize(RenderDevice::Instance()->GetViewportX(),RenderDevice::Instance()->GetViewportY());
	desktop->SetLocation(0,0);
	//desktop->SetBackgroundColors(D3DCOLOR_RGBA(0,0,0,255));
	COLOR a = D3DCOLOR_ARGB(100, 40, 23, 90); 
	COLOR b = D3DCOLOR_ARGB(100, 56, 24, 72);
	COLOR c = D3DCOLOR_ARGB(100, 30, 00, 67);
	COLOR d = D3DCOLOR_ARGB(100, 51,  04, 60);

	desktop->SetBackgroundColors( a,b,c,d );

	desktop->EnableCaption(true);
	desktop->SetCaptionText(ToUnicode("RealityEngine Main Menu -- Build "+ToStr(BUILD_VERSION)).c_str());
/*
	RECT r;
	SetRect( &r, 0, 0, 136, 54 );
	
	e.SetTexture(1,&r);
	desktop->SetTexture(1,L"particle.dds");
	//desktop->SetDefaultElement(DXUT_CONTROL_BUTTON,DXUT_STATE_MOUSEOVER,&e);
	desktop->SetDefaultElement(DXUT_CONTROL_BUTTON,1,&e);
*/
	int buttonWidth  = 100;
	int buttonHeight = 25;
	int curX = 50;
	int curY = 20;
	CDXUTButton* button;
	desktop->AddButton( IDC_PLAY_GAME, L"New Game...",curX-buttonWidth/2,curY-buttonHeight/2,buttonWidth,buttonHeight,0,false,&button);
	curY += buttonHeight+5;
	//button->SetElement(0,&e);
	desktop->AddButton( IDC_VIDEO_SETTINGS, L"Video",curX-buttonWidth/2,curY-buttonHeight/2,buttonWidth,buttonHeight,0,false,&button);
	curY += buttonHeight+5;
	desktop->AddButton( IDC_GRAPHICS_SETTINGS, L"Graphics",curX-buttonWidth/2,curY-buttonHeight/2,buttonWidth,buttonHeight,0,false,&button);
	curY += buttonHeight+5;
	desktop->AddButton( IDC_GAME_CONTROLS, L"Controls",curX-buttonWidth/2,curY-buttonHeight/2,buttonWidth,buttonHeight,0,false,&button);
	curY += buttonHeight+5;
	desktop->AddButton( IDC_ABOUT, L"Credits",curX-buttonWidth/2,curY-buttonHeight/2,buttonWidth,buttonHeight,0,false,&button);
	curY += buttonHeight+5;
	desktop->AddButton( IDC_EXIT, L"Exit",curX-buttonWidth/2,curY-buttonHeight/2,buttonWidth,buttonHeight,0,false,&button);

	// Add the background last, so it's under everything
	/*CPictureLabel* p = new CPictureLabel(IDC_BACKGROUND_LEFT,0,0,0,0,"background_left.tga",D3DBLEND_SRCCOLOR,D3DBLEND_INVSRCCOLOR);
	desktop->AddControl(p);
	*/

	// Add logo
	CPictureLabel* p = new CPictureLabel(IDC_BACKGROUND_RIGHT,0,0,0,0,"background_right.tga",D3DBLEND_SRCCOLOR,D3DBLEND_INVSRCCOLOR);
	desktop->AddControl(p);



	// Add music!
	string music = m_Config.GetString("BackgroundMusic");
	if(music != "NONE" && FindMedia(music,"Music") && !Editor::Instance()->GetEditorMode())
	{
		if(oggPlayer.IsPlaying())
			oggPlayer.Stop();
		oggPlayer.OpenOgg(music.c_str());
		oggPlayer.Play(true);
		oggPlayer.SetVolume(Engine::Instance()->MainConfig->GetFloat("MusicVolume"));
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void GUISystem::Initialize(){
	if(m_Desktop)
		return;

	// Load config containing all the window positions
	ResetCurrentDirectory();
	if(!FileExists("GUIConfig.ini")){
		m_Config.Create("GUIConfig.ini");
		m_Config.SetString("BackgroundMusic","NONE");
	}
	m_Config.Load("GUIConfig.ini");

	InitializeDesktop();
	InitializeSettings(); 

	// Init messagebox
	m_MessageBox = RegisterAsWindow(new CDXUTDialog);
	m_MessageBox->m_Dialog->SetCallback(::OnDesktopEvent);
	m_MessageBox->m_Dialog->EnableCaption(true);
	m_MessageBox->m_Dialog->SetCaptionText(L"");
	m_MessageBox->m_Dialog->SetSize(270,120);
	m_MessageBox->m_Dialog->AddButton(IDC_MESSAGEBOX_OK, L"OK",270/2 - 70/2, 120/2 - 31/2, 70, 31,VK_RETURN,true);
	m_MessageBox->m_Dialog->AddStatic(IDC_STATIC,L"This is a message box",0,0,270,70,false,NULL);
	m_MessageBox->m_Dialog->SetBackgroundColors(a,b,c,d);

	// Init strip
	m_Strip = RegisterAsWindow(new CDXUTDialog);
	m_Strip->m_Dialog->SetCallback(::OnDesktopEvent);
	m_Strip->m_Dialog->EnableCaption(false);
	m_Strip->m_Dialog->SetSize(500,25);
	m_Strip->m_Dialog->AddStatic(IDC_STATIC,L"Status Strip",0,0,500,25,false,NULL);
	m_Strip->m_Dialog->SetBackgroundColors(a,b,c,d);

	// Trigger size update
	OnResize();
}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
CGUIWindow* GUISystem::RegisterAsWindow(CDXUTDialog* dialog){
	CGUIWindow* window = new CGUIWindow(dialog);
	m_Windows.push_back(window);
	return window;
}


//-----------------------------------------------------------------------------
// Finds window from dialog
//-----------------------------------------------------------------------------
CGUIWindow* GUISystem::FindWindow(CDXUTDialog* dialog){
	for(int i=0;i<m_Windows.size();i++){
		if(m_Windows[i]->m_Dialog == dialog){
			return m_Windows[i];
		}
	}
	return 0;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void GUISystem::RemoveWindow(CGUIWindow* window){
	for(int i=0;i<m_Windows.size();i++){
		if(m_Windows[i] == window){
			m_Windows.erase(m_Windows.begin() + i);
			// This is an opportunity to save the window properties to the config....
			m_Config.SetInt(window->m_ConfigIdentifier+"_X",window->m_Dialog->GetX());
			m_Config.SetInt(window->m_ConfigIdentifier+"_Y",window->m_Dialog->GetY());
			break;
		}
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool GUISystem::DesktopVisible(){
	return m_Desktop->GetVisible();
}

//-----------------------------------------------------------------------------
// Sets visibility of not only desktop, but all desktop windows
//-----------------------------------------------------------------------------
void GUISystem::ShowDesktop(bool bShow){
	if(!m_Desktop)
		return;

	m_Desktop->SetVisible(bShow);

	// FIXME: Should only be done once, when all windows are loaded.
	if(m_Video)
		m_Video->SetDesktopWindow(true);
	if(m_Game)
		m_Game->SetDesktopWindow(true);
	if(m_Controls)
		m_Controls->SetDesktopWindow(true);
	if(m_Credits)
		m_Credits->SetDesktopWindow(true);
	if(m_NewGame)
		m_NewGame->SetDesktopWindow(true);
}

//-----------------------------------------------------------------------------
// Moves the window to the top of the ordering array
//-----------------------------------------------------------------------------
void GUISystem::BringToTop(CGUIWindow* window){
	// Remove the window temporarily
	for(int i=0;i<m_Windows.size();i++){
		if(m_Windows[i] == window){
			m_Windows.erase(m_Windows.begin() + i);
			break;
		}
	}
	// Add it back to the top
	m_Windows.push_back(window);
}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void GUISystem::Render(){

	// Little trick - make main menu dialogs more transparent when the desktop is minmized
	for(int i=0;i<m_Windows.size();i++){
		if(m_Windows[i]->GetDesktopWindow()){
			if(m_Desktop->m_Dialog->IsMinimized())
				m_Windows[i]->m_Dialog->SetBackgroundColors(a1,b1,c1,d1);
			else
				m_Windows[i]->m_Dialog->SetBackgroundColors(a,b,c,d);
		}
	}

	// Capture the states, because we alter the shaders in a second
	LPDIRECT3DSTATEBLOCK9 states;
	RenderWrap::dev->CreateStateBlock(D3DSBT_ALL,&states);
	states->Capture();

	RenderWrap::SetRS( D3DRS_FOGENABLE, FALSE );
	RenderWrap::SetSS( 0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
	RenderWrap::SetSS( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	RenderWrap::SetSS( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	DWORD rsDith = RenderWrap::SetRS(D3DRS_DITHERENABLE,FALSE);

	Canvas* canvas = RenderDevice::Instance()->GetCanvas();
	canvas->SetScaling(1,1);
	if(m_Desktop->GetVisible())
		m_Desktop->m_Dialog->OnRender(GDeltaTime);

	for(int i=0;i<m_Windows.size();i++){
		// If desktop is hidden and this is a desktop window, hide it too
		if(m_Windows[i]->GetDesktopWindow() && !m_Desktop->GetVisible())
			continue;

		// Draw if visible
		if(m_Windows[i]->GetVisible())
			m_Windows[i]->m_Dialog->OnRender(GDeltaTime);
	}
	canvas->SetScaling(canvas->Width/1024.f,canvas->Height/768.f);

	RenderWrap::SetSS( 0, D3DSAMP_MIPFILTER,1);
	RenderWrap::SetRS(D3DRS_DITHERENABLE,rsDith);
	RenderWrap::SetRS(D3DRS_FOGENABLE, TRUE );

	states->Apply();
	SAFE_RELEASE(states);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void GUISystem::Update(){
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void GUISystem::Shutdown(){
	LogPrintf("Saving %d windows",m_Windows.size());
	while(m_Windows.size()) RemoveWindow(m_Windows[0]);
}

//-----------------------------------------------------------------------------
// Handles messages, obeying z-order
//-----------------------------------------------------------------------------
LRESULT GUISystem::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
	if(!m_Desktop || m_MessageBoxBlocked)
		return 0;

	// Handle possible focus changes
	if(uMsg == WM_LBUTTONDOWN){
		// Check for window click that would alter the focus window
		POINT mousePoint = { short(LOWORD(lParam)), short(HIWORD(lParam)) };

		// Go in reverse, so we check the top window first
		for(int i=m_Windows.size()-1;i>=0;i--){
			if(!m_Windows[i]->GetVisible() || !m_Windows[i]->GetEnabled())
				continue; // Ignore invisible/disabled

			CDXUTDialog* dlg = m_Windows[i]->m_Dialog;
			// Is cursor in window area? 

			if( // If minimized, only check titlebar
				(dlg->IsMinimized() && 
				( mousePoint.x >= dlg->GetX() && mousePoint.x < dlg->GetX() + dlg->GetWidth() &&
                mousePoint.y >= dlg->GetY() && mousePoint.y < dlg->GetY() +  dlg->GetCaptionHeight() ))
				||
				// Otherwise, check full window area
				(!dlg->IsMinimized() && (mousePoint.x >= dlg->GetX() && mousePoint.x < dlg->GetX() + dlg->GetWidth() &&
				mousePoint.y >= dlg->GetY() && mousePoint.y < dlg->GetY() + dlg->GetHeight() )))
			{
				// This window wants focus!!
				BringToTop(m_Windows[i]);
				break;
			}
		}
	}

	// Handle input _ONLY_ from the top focus window
	// Must check visibility again
	if(m_Windows.size() && m_Windows[m_Windows.size()-1]->GetVisible()){
		CDXUTDialog* dlg = m_Windows[m_Windows.size()-1]->m_Dialog;
		// Give the dialogs a chance to handle the message first
		if(dlg->MsgProc( hWnd, uMsg, wParam, lParam))
			return 0; // Only one dialog can be active at a time. It returns > 0 if it's the active one
	}

    if(m_Desktop->GetVisible() && m_Desktop->GetEnabled() && !Editor::Instance()->GetEditorMode())
		m_Desktop->m_Dialog->MsgProc( hWnd, uMsg, wParam, lParam);
	return 0;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void GUISystem::DoMessageBox(string title, string text, int x, int y){
	if(!m_Desktop)
		Initialize(); // Hacky, but a messagebox may be triggered before initialize

	if(x == -1)
		x = RenderDevice::Instance()->GetViewportX()/2;
	if(y == -1)
		y = RenderDevice::Instance()->GetViewportY()/2;

	m_MessageBox->SetVisible(true);
	((CDXUTButton*)m_MessageBox->m_Dialog->GetControl(IDC_MESSAGEBOX_OK))->SetVisible(true);
	m_MessageBox->m_Dialog->SetCaptionText(ToUnicode(title).c_str());
	CDXUTStatic* s = (CDXUTStatic*)m_MessageBox->m_Dialog->GetControl(IDC_STATIC);
	s->SetText(ToUnicode(text).c_str());
	m_MessageBox->m_Dialog->SetLocation(x - m_MessageBox->m_Dialog->GetWidth()/2,y - m_MessageBox->m_Dialog->GetHeight()/2);

	BringToTop(m_MessageBox);

	m_MessageBoxBlocked = false;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void GUISystem::DoMessageStrip(string text){
	if(!m_Desktop)
		Initialize(); // Hacky, but a messagebox may be triggered before initialize

	m_Strip->SetVisible(true);
	//m_MessageBox->m_Dialog->SetCaptionText(ToUnicode(title).c_str());
	CDXUTStatic* s = (CDXUTStatic*)m_Strip->m_Dialog->GetControl(IDC_STATIC);
	s->SetText(ToUnicode(text).c_str());
	m_Strip->m_Dialog->SetLocation(0,0);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void GUISystem::DoMessageBoxBlocking(string title, string text, float waitTime, MessageBox_Callback callback){

	int x = RenderDevice::Instance()->GetViewportX()/2;

	int y = RenderDevice::Instance()->GetViewportY()/2;

	m_MessageBox->SetVisible(true);
	((CDXUTButton*)m_MessageBox->m_Dialog->GetControl(IDC_MESSAGEBOX_OK))->SetVisible(false);
	m_MessageBox->m_Dialog->SetCaptionText(ToUnicode(title).c_str());
	CDXUTStatic* s = (CDXUTStatic*)m_MessageBox->m_Dialog->GetControl(IDC_STATIC);
	s->SetText(ToUnicode(text).c_str());
	m_MessageBox->m_Dialog->SetLocation(x - m_MessageBox->m_Dialog->GetWidth()/2,y - m_MessageBox->m_Dialog->GetHeight()/2);

	BringToTop(m_MessageBox);

	m_MessageBoxBlocked = true;

	//Sleep(waitTime);
	//callback();
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void GUISystem::CloseMessageBoxBlocking()
{
	m_MessageBoxBlocked = false;
	m_MessageBox->SetVisible(false);
}

