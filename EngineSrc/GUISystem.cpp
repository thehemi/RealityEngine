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
COLOR a = D3DCOLOR_ARGB(130, 20, 23, 90); 
COLOR b = D3DCOLOR_ARGB(130, 76, 77, 99);
COLOR c = D3DCOLOR_ARGB(130, 20, 55, 70);
COLOR d = D3DCOLOR_ARGB(130, 81,  88, 102);
// Transparent window colors, when working with controls overlaying game
COLOR a1 = D3DCOLOR_ARGB(120, 20, 23, 120); 
COLOR b1 = D3DCOLOR_ARGB(120, 76, 124, 129);
COLOR c1 = D3DCOLOR_ARGB(120, 20, 100, 120);
COLOR d1 = D3DCOLOR_ARGB(120, 81,  104, 122);

/*COLOR a = D3DCOLOR_ARGB(240, 77, 99, 120); 
COLOR b = D3DCOLOR_ARGB(240, 44, 88, 111);
COLOR c = D3DCOLOR_ARGB(240, 44, 88, 111);
COLOR d = D3DCOLOR_ARGB(240, 10,  73, 133);*/


//-----------------------------------------------------------------------------
// Desktop button IDs
//-----------------------------------------------------------------------------
#define IDC_STATIC             -1
#define IDC_BACK				1
#define IDC_MESSAGEBOX_OK		400

bool m_MessageBoxBlocked = false;
bool DelayedMessageBox = false;
string DelayedMessageBoxTitle;
string DelayedMessageBoxText;
int DelayedMessageBoxX = 0;
int DelayedMessageBoxY = 0;

//-----------------------------------------------------------------------------
// Window Constructor
//-----------------------------------------------------------------------------
CGUIWindow::CGUIWindow(CDXUTDialog* dialog, string ConfigIdentifier /*= "Auto"*/){
	m_Dialog			= dialog;
	m_Dialog->SetBackgroundColors(a,b,c,d);
	m_bVisible			= false;
	m_bEnabled			= true;
	m_bDesktop = false;
	m_bDesktopWindow = false;
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
	m_Texture->usesLOD = false;
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
	DXUTGetGlobalDialogResourceManager()->m_pSprite->End();

	RenderWrap::SetSS( 0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
	RenderWrap::SetSS( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	RenderWrap::SetSS( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	RenderWrap::SetRS(D3DRS_DITHERENABLE,FALSE);
	RenderDevice::Instance()->GetCanvas()->Box(GetElement(0)->TextureColor.Current,rcWindow.left,rcWindow.top,m_width,m_height,m_Texture,(BlendMode)m_Source,(BlendMode)m_Dest);

	DXUTGetGlobalDialogResourceManager()->m_pSprite->Begin( D3DXSPRITE_DONOTSAVESTATE );

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

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
int GUISystem::GetX(CGUIWindow* window){
	if(m_Config.KeyExists(window->m_ConfigIdentifier+"_X"))
		return m_Config.GetInt(window->m_ConfigIdentifier+"_X");
	else
		return window->m_Dialog->GetX();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
int GUISystem::GetY(CGUIWindow* window){
	if(m_Config.KeyExists(window->m_ConfigIdentifier+"_Y"))
		return m_Config.GetInt(window->m_ConfigIdentifier+"_Y");
	else
		return window->m_Dialog->GetY();
}

//-----------------------------------------------------------------------------
// Window resize, probably need to resize/reposition controls
//-----------------------------------------------------------------------------
void GUISystem::OnResize(){
	if(!m_Desktop)
		return;

	int windowX = RenderDevice::Instance()->GetViewportX();
	int windowY = RenderDevice::Instance()->GetViewportY();

	for(int i=0;i<m_Windows.size();i++){
		if(m_Windows[i]->m_Dialog->GetX() >= windowX || m_Windows[i]->m_Dialog->GetY() > windowY)
			m_Windows[i]->m_Dialog->SetLocation(100,100);
	}

	m_Desktop->m_Dialog->SetSize(windowX,windowY);

	// Let game do some resizing
	if(m_ResizeCallback)
		m_ResizeCallback(windowX,windowY);
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void GUISystem::Initialize()
{
	if(m_Desktop)
		return;

	a = Engine::Instance()->MainConfig->GetColor("WindowTopLeft");
	b = Engine::Instance()->MainConfig->GetColor("WindowTopRight");
	c = Engine::Instance()->MainConfig->GetColor("WindowBottomLeft");
	d = Engine::Instance()->MainConfig->GetColor("WindowBottomRight");

	// Load config containing all the window positions
	ResetCurrentDirectory();
	if(!FileExists("GUIConfig.ini")){
		m_Config.Create("GUIConfig.ini");
		m_Config.SetString("BackgroundMusic","NONE");
	}
	m_Config.Load("GUIConfig.ini");

	m_Desktop = new CGUIWindow(new CDXUTDialog);
	CDXUTDialog* desktop = m_Desktop->m_Dialog;
	// Desktop covers entire screen
	desktop->SetSize(RenderDevice::Instance()->GetViewportX(),RenderDevice::Instance()->GetViewportY());
	desktop->SetLocation(0,0);
	desktop->SetBackgroundColors(D3DCOLOR_RGBA(0,0,0,0));
	desktop->SetBGDisplay(false);

	// Init strip
	m_Strip = RegisterAsWindow(new CDXUTDialog);
	m_Strip->m_Dialog->EnableCaption(false);
	m_Strip->m_Dialog->SetSize(500,25);
	m_Strip->m_Dialog->AddStatic(IDC_STATIC,L"Status Strip",0,0,500,25,false,NULL);

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
void GUISystem::Render()
{
	if(DelayedMessageBox)
	{
		DelayedMessageBox = false;
		DoMessageBox(DelayedMessageBoxTitle,DelayedMessageBoxText,DelayedMessageBoxX,DelayedMessageBoxY);
	}

	if(!m_bDisplayGUI)
		return;

	Canvas* canvas = RenderDevice::Instance()->GetCanvas();
	bool SetStates = false;

	if(m_Desktop->GetVisible())
	{
		if(!SetStates)
		{
			SetStates = true;
			RenderWrap::SetRS( D3DRS_FOGENABLE, FALSE );
			RenderWrap::SetSS( 0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
			RenderWrap::SetSS( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
			RenderWrap::SetSS( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
			DWORD rsDith = RenderWrap::SetRS(D3DRS_DITHERENABLE,FALSE);
			canvas->SetScaling(1,1);
		}
		m_Desktop->m_Dialog->OnRender(GDeltaTime);
	}

	for(int i=0;i<m_Windows.size();i++){
		// If desktop is hidden and this is a desktop window, hide it too
		if(m_Windows[i]->GetDesktopWindow() && !m_Desktop->GetVisible())
			continue;

		// Draw if visible
		if(m_Windows[i]->GetVisible())
		{
			if(!SetStates)
			{
				SetStates = true;
				RenderWrap::SetRS( D3DRS_FOGENABLE, FALSE );
				RenderWrap::SetSS( 0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
				RenderWrap::SetSS( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
				RenderWrap::SetSS( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
				DWORD rsDith = RenderWrap::SetRS(D3DRS_DITHERENABLE,FALSE);
				canvas->SetScaling(1,1);
			}
			if(m_Windows[i]->GetDesktop())
				m_Windows[i]->m_Dialog->SetSize(canvas->Width,canvas->Height);

			m_Windows[i]->m_Dialog->OnRender(GDeltaTime);
		}
	}
	if(SetStates)
		canvas->SetScaling(canvas->Width/1024.f,canvas->Height/768.f);
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
			if((m_Windows[i]->GetDesktopWindow() && !m_Desktop->GetVisible()) || !m_Windows[i]->GetVisible() || !m_Windows[i]->GetEnabled() || m_Windows[i]->GetDesktop())
				continue; // Ignore invisible/disabled/desktop

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
	if(m_Windows.size())
	{
		int i = m_Windows.size()-1;
		if(!((m_Windows[i]->GetDesktopWindow() && !m_Desktop->GetVisible()) || !m_Windows[i]->GetVisible() || !m_Windows[i]->GetEnabled() || m_Windows[i]->GetDesktop()))
		{
		CDXUTDialog* dlg = m_Windows[m_Windows.size()-1]->m_Dialog;
		// Give the dialogs a chance to handle the message first
		if(dlg->MsgProc( hWnd, uMsg, wParam, lParam))
			return 0; // Only one dialog can be active at a time. It returns > 0 if it's the active one
		}
	}

	if(!Editor::Instance()->GetEditorMode() && m_Desktop->GetVisible())
	{
		POINT mousePoint = { short(LOWORD(lParam)), short(HIWORD(lParam)) };
		for(int i=m_Windows.size()-1;i>=0;i--)
		{
			if(!m_Windows[i]->GetVisible() || m_Windows[i]->GetDesktop())
				continue; // Ignore invisible/desktop
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
				return 0;
			}
		}
		for(int n = 0; n < m_Windows.size(); n++)
		{
			if(m_Windows[n]->GetDesktop() && m_Windows[n]->GetVisible() && m_Windows[n]->GetEnabled())
				m_Windows[n]->m_Dialog->MsgProc( hWnd, uMsg, wParam, lParam);
		}
		if(m_Desktop->GetVisible() && m_Desktop->GetEnabled())
			m_Desktop->m_Dialog->MsgProc( hWnd, uMsg, wParam, lParam);
	}

	return 0;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void GUISystem::DoMessageBox(string title, string text, int x, int y){
	if(!m_Desktop)
	{
		DelayedMessageBox = true;
		DelayedMessageBoxTitle = title;
		DelayedMessageBoxText = text;
		DelayedMessageBoxX = x;
		DelayedMessageBoxY = y;
		return;
	}

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

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
CDXUTDialog* GUISystem::GetVideoSettingsDialog()
{
	return DXUTGetSettingsDialog()->GetDialogControl();
}

void GUISystem::OnVideoSettingsEvent(UINT nEvent, int nControlID, CDXUTControl* pControl)
{
	((CD3DSettingsDlg*)GetVideoSettingsDialog())->OnEvent(nEvent,nControlID,pControl);
}
