//--------------------------------------------------------------------------------------
/// File: DXUTgui.h
//
/// Desc: 
//
/// Copyright (c) Artificial Studios. All rights reserved.
//--------------------------------------------------------------------------------------
#pragma once
#pragma warning( disable : 4240 ) //disable this silly 'wcsncpy' was declared deprecated warnning
#ifndef DXUT_GUI_H
#define DXUT_GUI_H

#include <usp10.h>
#include <dimm.h>


//--------------------------------------------------------------------------------------
/// Defines and macros 
//--------------------------------------------------------------------------------------
#define MAX_CONTROL_STATES     16

#define EVENT_BUTTON_CLICKED                0x0101
#define EVENT_TAB_CLICKED					0x0102
#define EVENT_COMBOBOX_SELECTION_CHANGED    0x0201
#define EVENT_RADIOBUTTON_CHANGED           0x0301
#define EVENT_CHECKBOX_CHANGED              0x0401
#define EVENT_SLIDER_VALUE_CHANGED          0x0501
#define EVENT_EDITBOX_STRING                0x0601
/// EVENT_EDITBOX_CHANGE is sent when the listbox content changes
/// due to user input.
#define EVENT_EDITBOX_CHANGE                0x0602
#define EVENT_LISTBOX_ITEM_DBLCLK           0x0701
/// EVENT_LISTBOX_SELECTION is fired off when the selection changes in
/// a single selection list box.
#define EVENT_LISTBOX_SELECTION             0x0702


//--------------------------------------------------------------------------------------
/// Forward declarations
//--------------------------------------------------------------------------------------
class CDXUTControl;
class CDXUTButton;
class CDXUTStatic;
class CDXUTCheckBox;
class CDXUTRadioButton;
class CDXUTComboBox;
class CDXUTSlider;
class CDXUTEditBox;
class CDXUTIMEEditBox;
class CDXUTListBox;
class CDXUTScrollBar;
class CDXUTElement;
class CDXUTPropertiesList;
class CDXUTTabPages;
class CDXUTab;
struct DXUTElementHolder;
struct DXUTTextureNode;
struct DXUTFontNode;
typedef VOID (CALLBACK *PCALLBACKDXUTGUIEVENT) ( UINT nEvent, int nControlID, CDXUTControl* pControl );

//--------------------------------------------------------------------------------------
/// Enums for pre-defined control types
//--------------------------------------------------------------------------------------
enum DXUT_CONTROL_TYPE 
{ 
    DXUT_CONTROL_BUTTON, 
    DXUT_CONTROL_STATIC, 
    DXUT_CONTROL_CHECKBOX,
    DXUT_CONTROL_RADIOBUTTON,
    DXUT_CONTROL_COMBOBOX,
    DXUT_CONTROL_SLIDER,
    DXUT_CONTROL_EDITBOX,
    DXUT_CONTROL_IMEEDITBOX,
    DXUT_CONTROL_LISTBOX,
    DXUT_CONTROL_SCROLLBAR,
	DXUT_CONTROL_PROPERTIESLIST,
    DXUT_CONTROL_TABPAGES,
	DXUT_CONTROL_TAB,
};

enum DXUT_CONTROL_STATE
{
    DXUT_STATE_NORMAL,
    DXUT_STATE_DISABLED,
    DXUT_STATE_HIDDEN,
    DXUT_STATE_FOCUS,
    DXUT_STATE_MOUSEOVER,
    DXUT_STATE_PRESSED,
};


struct DXUTBlendColor
{
    void Init( D3DCOLOR defaultColor, D3DCOLOR disabledColor = D3DCOLOR_ARGB(200, 128, 128, 128), D3DCOLOR hiddenColor = 0 );
    void Blend( UINT iState, float fElaspedTime, float fRate = 0.7f );
    
    D3DCOLOR  States[ MAX_CONTROL_STATES ]; /// Modulate colors for all possible control states
    D3DXCOLOR Current;
};


//-----------------------------------------------------------------------------
/// Contains all the display tweakables for a sub-control
//-----------------------------------------------------------------------------
class ENGINE_API CDXUTElement
{
public:
    void SetTexture( UINT iTexture, RECT* prcTexture, D3DCOLOR defaultTextureColor = D3DCOLOR_ARGB(255, 255, 255, 255) );
    void SetFont( UINT iFont, D3DCOLOR defaultFontColor = D3DCOLOR_ARGB(255, 255, 255, 255), DWORD dwTextFormat = DT_CENTER | DT_VCENTER );
    
    void Refresh();
    
    UINT iTexture;          /// Index of the texture for this Element 
    UINT iFont;             /// Index of the font for this Element
    DWORD dwTextFormat;     /// The format argument to DrawText 

    RECT rcTexture;         /// Bounding rect of this element on the composite texture
    
    DXUTBlendColor TextureColor;
    DXUTBlendColor FontColor;
};


//-----------------------------------------------------------------------------
/// All controls must be assigned to a dialog, which handles
/// input and rendering for the controls.
//-----------------------------------------------------------------------------
class ENGINE_API CDXUTDialog
{
public:
	/// TIM: These vars needed for draggable windows
	bool bDrag;
	bool bDraggable;
	bool m_bDisplayBG;
	POINT clickPoint;
	POINT windowStart;

    int managedIndex;

    CDXUTDialog();
    ~CDXUTDialog();

    /// Windows message handler
    bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

    /// Control creation
    HRESULT AddStatic( int ID, LPCWSTR strText, int x, int y, int width, int height, bool bIsDefault=false, CDXUTStatic** ppCreated=NULL );
    HRESULT AddButton( int ID, LPCWSTR strText, int x, int y, int width, int height, UINT nHotkey=0, bool bIsDefault=false, CDXUTButton** ppCreated=NULL );
    HRESULT AddCheckBox( int ID, LPCWSTR strText, int x, int y, int width, int height, bool bChecked=false, UINT nHotkey=0, bool bIsDefault=false, CDXUTCheckBox** ppCreated=NULL );
    HRESULT AddRadioButton( int ID, UINT nButtonGroup, LPCWSTR strText, int x, int y, int width, int height, bool bChecked=false, UINT nHotkey=0, bool bIsDefault=false, CDXUTRadioButton** ppCreated=NULL );
    HRESULT AddComboBox( int ID, int x, int y, int width, int height, UINT nHotKey=0, bool bIsDefault=false, CDXUTComboBox** ppCreated=NULL );
    HRESULT AddSlider( int ID, int x, int y, int width, int height, int min=0, int max=100, int value=50, bool bIsDefault=false, CDXUTSlider** ppCreated=NULL, float UnitScale = 1 );
    HRESULT AddEditBox( int ID, LPCWSTR strText, int x, int y, int width, int height, bool bIsDefault=false, CDXUTEditBox** ppCreated=NULL );
    HRESULT AddIMEEditBox( int ID, LPCWSTR strText, int x, int y, int width, int height, bool bIsDefault=false, CDXUTIMEEditBox** ppCreated=NULL );
    HRESULT AddListBox( int ID, int x, int y, int width, int height, DWORD dwStyle=0, CDXUTListBox** ppCreated=NULL );
	HRESULT AddPropertiesList( int ID, int x, int y, int width, int height, DWORD dwStyle=0, CDXUTPropertiesList** ppCreated=NULL );
    HRESULT AddTabPagesControl( int ID, int x, int y, int width, int height, CDXUTTabPages** ppCreated = NULL );
	HRESULT AddTabControl( int ID,  LPCWSTR strText, int x, int y, 
								   int width, int height, CDXUTab** ppCreated = NULL );
	
    HRESULT AddControl( CDXUTControl* pControl );
    HRESULT InsertControl( int index, CDXUTControl* pControl );
    HRESULT InitControl( CDXUTControl* pControl );

    /// Control retrieval
    CDXUTStatic*      GetStatic( int ID ) { return (CDXUTStatic*) GetControl( ID, DXUT_CONTROL_STATIC ); }
    CDXUTButton*      GetButton( int ID ) { return (CDXUTButton*) GetControl( ID, DXUT_CONTROL_BUTTON ); }
    CDXUTCheckBox*    GetCheckBox( int ID ) { return (CDXUTCheckBox*) GetControl( ID, DXUT_CONTROL_CHECKBOX ); }
    CDXUTRadioButton* GetRadioButton( int ID ) { return (CDXUTRadioButton*) GetControl( ID, DXUT_CONTROL_RADIOBUTTON ); }
    CDXUTComboBox*    GetComboBox( int ID ) { return (CDXUTComboBox*) GetControl( ID, DXUT_CONTROL_COMBOBOX ); }
    CDXUTSlider*      GetSlider( int ID ) { return (CDXUTSlider*) GetControl( ID, DXUT_CONTROL_SLIDER ); }
    CDXUTEditBox*     GetEditBox( int ID ) { return (CDXUTEditBox*) GetControl( ID, DXUT_CONTROL_EDITBOX ); }
    CDXUTIMEEditBox*  GetIMEEditBox( int ID ) { return (CDXUTIMEEditBox*) GetControl( ID, DXUT_CONTROL_IMEEDITBOX ); }
    CDXUTListBox*     GetListBox( int ID ) { return (CDXUTListBox*) GetControl( ID, DXUT_CONTROL_LISTBOX ); }

    CDXUTControl* GetControl( int ID );
    CDXUTControl* GetControl( int ID, UINT nControlType );
    CDXUTControl* GetControlAtPoint( POINT pt );

    bool GetControlEnabled( int ID );
    void SetControlEnabled( int ID, bool bEnabled );

    void ClearRadioButtonGroup( UINT nGroup );
    void ClearComboBox( int ID );

    /// Access the default display Elements used when adding new controls
    HRESULT       SetDefaultElement( UINT nControlType, UINT iElement, CDXUTElement* pElement );
    CDXUTElement* GetDefaultElement( UINT nControlType, UINT iElement );

    /// Methods called by controls
    void SendEvent( UINT nEvent, bool bTriggeredByUser, CDXUTControl* pControl );
    void RequestFocus( CDXUTControl* pControl );

    /// Render helpers
    HRESULT DrawRect( RECT* pRect, D3DCOLOR color );
    HRESULT DrawPolyLine( POINT* apPoints, UINT nNumPoints, D3DCOLOR color );
    HRESULT DrawSprite( CDXUTElement* pElement, RECT* prcDest );
    HRESULT CalcTextRect( LPCWSTR strText, CDXUTElement* pElement, RECT* prcDest, int nCount = -1 );
    HRESULT DrawText( LPCWSTR strText, CDXUTElement* pElement, RECT* prcDest, bool bShadow = false, int nCount = -1 );

    /// Attributes
    void SetBackgroundColors( D3DCOLOR colorAllCorners ) { SetBackgroundColors( colorAllCorners, colorAllCorners, colorAllCorners, colorAllCorners ); }
    void SetBackgroundColors( D3DCOLOR colorTopLeft, D3DCOLOR colorTopRight, D3DCOLOR colorBottomLeft, D3DCOLOR colorBottomRight );
    void EnableCaption( bool bEnable ) { m_bCaption = bEnable; }
    void SetCaptionHeight( int nHeight ) { m_nCaptionHeight = nHeight; }
    void SetCaptionText( const WCHAR *pwszText ) { wcsncpy( m_wszCaption, pwszText, sizeof(m_wszCaption)/sizeof(m_wszCaption[0]) ); m_wszCaption[sizeof(m_wszCaption)/sizeof(m_wszCaption[0])-1] = 0; }
    void SetLocation( int x, int y ) { m_x = x; m_y = y; }
    void SetSize( int width, int height ) { m_width = width; m_height = height;  }
    int GetWidth() { return m_width; }
    int GetHeight() { return m_height; }
	int GetX(){ return m_x; }
	int GetY(){ return m_y; }
	bool IsMinimized(){ return m_bMinimized;}
	int GetCaptionHeight(){ return m_nCaptionHeight; }
	bool GetCaptionEnabled(){ return m_bCaption; }
	void SetBGDisplay(bool Enable) { m_bDisplayBG = Enable; };

    void SetNextDialog( CDXUTDialog* pNextDialog );

    static void SetRefreshTime( float fTime ){ s_fTimeRefresh = fTime; }

    static CDXUTControl* GetNextControl( CDXUTControl* pControl );
    static CDXUTControl* GetPrevControl( CDXUTControl* pControl );

    void RemoveControl( int ID );
    void RemoveAllControls();
     void RemoveAt( int index );

    bool Contains( CDXUTControl* pControl );
    int GetControlsCount()
    {
        return m_Controls.GetSize();
    }
    int IndexOfControl(CDXUTControl* control)
    {
        return m_Controls.IndexOf(control);
    }

    /// Sets the callback used to notify the app of control events
    void SetCallback( PCALLBACKDXUTGUIEVENT pCallback ) { m_pCallbackEvent = pCallback; }
    void EnableNonUserEvents( bool bEnable ) { m_bNonUserEvents = bEnable; }
    void EnableKeyboardInput( bool bEnable ) { m_bKeyboardInput = bEnable; }
    void EnableMouseInput( bool bEnable ) { m_bMouseInput = bEnable; }

    /// Device state notification
    void Refresh();
    HRESULT OnRender( float fElapsedTime );    

    /// Shared resource access. Indexed fonts and textures are shared among
    /// all the controls.
    HRESULT       SetFont( UINT index, LPCWSTR strFaceName, LONG height, LONG weight );
    DXUTFontNode* GetFont( UINT index );

    HRESULT          SetTexture( UINT index, LPCWSTR strFilename );
    DXUTTextureNode* GetTexture( UINT index );

    static void ClearFocus();
    void FocusDefaultControl();

    bool m_bNonUserEvents;
    bool m_bKeyboardInput;
    bool m_bMouseInput;

private:
    int m_nDefaultControlID;

    static double s_fTimeRefresh;
    double m_fTimeLastRefresh;

    /// Initialize default Elements
    void InitDefaultElements();

    /// Windows message handlers
    void OnMouseMove( POINT pt );
    void OnMouseDown( POINT pt );
    void OnMouseUp( POINT pt );

    bool OnKeyDown( UINT nChar );
    void OnSpacebarDown();
    void OnSpacebarUp();

    /// Control events
    void OnCycleFocus( bool bForward );
    void OnMouseEnter( CDXUTControl* pControl );
    void OnMouseLeave( CDXUTControl* pControl );
    void OnMouseClick( CDXUTControl* pControl );

    static CDXUTControl* s_pControlFocus;        /// The control which has focus
    static CDXUTControl* s_pControlPressed;      /// The control currently pressed

    CDXUTControl* m_pControlMouseOver;           /// The control which is hovered over

    bool m_bCaption;
    bool m_bMinimized;
    WCHAR m_wszCaption[256];

    int m_x;
    int m_y;
    int m_width;
    int m_height;
    int m_nCaptionHeight;

    D3DCOLOR m_colorTopLeft;
    D3DCOLOR m_colorTopRight;
    D3DCOLOR m_colorBottomLeft;
    D3DCOLOR m_colorBottomRight;

    PCALLBACKDXUTGUIEVENT m_pCallbackEvent;

    CGrowableArray< int > m_Textures;   /// Index into m_TextureCache;
    CGrowableArray< int > m_Fonts;      /// Index into m_FontCache;

    CGrowableArray< CDXUTControl* > m_Controls;
    CGrowableArray< DXUTElementHolder* > m_DefaultElements;

    CDXUTElement m_CapElement;  /// Element for the caption

    CDXUTDialog* m_pNextDialog;
    CDXUTDialog* m_pPrevDialog;
};


//--------------------------------------------------------------------------------------
/// Structs for shared resources
//--------------------------------------------------------------------------------------
struct DXUTTextureNode
{
    WCHAR strFilename[MAX_PATH];
    IDirect3DTexture9* pTexture;
    DWORD dwWidth;
    DWORD dwHeight;
};

struct DXUTFontNode
{
    WCHAR strFace[MAX_PATH];
    ID3DXFont* pFont;
    LONG  nHeight;
    LONG  nWeight;
};


//-----------------------------------------------------------------------------
/// Manages shared resources of dialogs
/// Use DXUTGetGlobalDialogResourceManager() to get access to the global instance
//-----------------------------------------------------------------------------
class ENGINE_API CDXUTDialogResourceManager
{
public:
    int AddFont( LPCWSTR strFaceName, LONG height, LONG weight );
    int AddTexture( LPCWSTR strFilename );

    DXUTFontNode*     GetFontNode( int iIndex )     { return m_FontCache.GetAt( iIndex ); };
    DXUTTextureNode*  GetTextureNode( int iIndex )  { return m_TextureCache.GetAt( iIndex ); };    
    IDirect3DDevice9* GetD3DDevice()                { return m_pd3dDevice; }

    /// Shared between all dialogs
    IDirect3DStateBlock9* m_pStateBlock;
    ID3DXSprite*          m_pSprite;          /// Sprite used for drawing

    ~CDXUTDialogResourceManager();

protected:
    friend CDXUTDialogResourceManager* DXUTGetGlobalDialogResourceManager();
    friend HRESULT DXUTInitialize3DEnvironment();
    friend HRESULT DXUTReset3DEnvironment();
    friend void DXUTCleanup3DEnvironment( bool bReleaseSettings );

    /// Use DXUTGetGlobalDialogResourceManager() to get access to the global instance
    CDXUTDialogResourceManager();

    /// The sample framework uses the global instance and automatically calls the device events
    HRESULT     OnCreateDevice( LPDIRECT3DDEVICE9 pd3dDevice );
    HRESULT     OnResetDevice();
    void        OnLostDevice();
    void        OnDestroyDevice();

    CGrowableArray< DXUTTextureNode* > m_TextureCache;   /// Shared textures
    CGrowableArray< DXUTFontNode* > m_FontCache;         /// Shared fonts
    
    IDirect3DDevice9* m_pd3dDevice;

    /// Resource creation helpers
    HRESULT CreateFont( UINT index );
    HRESULT CreateTexture( UINT index );
};

CDXUTDialogResourceManager* DXUTGetGlobalDialogResourceManager();


//-----------------------------------------------------------------------------
/// Base class for controls
//-----------------------------------------------------------------------------
class ENGINE_API CDXUTControl
{
public:
    CDXUTControl( CDXUTDialog *pDialog = NULL );
    virtual ~CDXUTControl();

    virtual HRESULT OnInit() { return S_OK; }
    virtual void Refresh();
    virtual void Render( IDirect3DDevice9* pd3dDevice, float fElapsedTime ) { };

    /// Windows message handler
    virtual bool MsgProc( UINT uMsg, WPARAM wParam, LPARAM lParam ) { return false; }

    virtual bool HandleKeyboard( UINT uMsg, WPARAM wParam, LPARAM lParam ) { return false; }
    virtual bool HandleMouse( UINT uMsg, POINT pt, WPARAM wParam, LPARAM lParam ) { return false; }

    virtual bool CanHaveFocus() { return false; }
    virtual void OnFocusIn() { m_bHasFocus = true; }
    virtual void OnFocusOut() { m_bHasFocus = false; }
    virtual void OnMouseEnter() { m_bMouseOver = true; }
    virtual void OnMouseLeave() { m_bMouseOver = false; }
    virtual void OnHotkey() {}

    virtual BOOL ContainsPoint( POINT pt ) { return PtInRect( &m_rcBoundingBox, pt ); }

    virtual void SetEnabled( bool bEnabled ) { m_bEnabled = bEnabled; }
    virtual bool GetEnabled() { return m_bEnabled; }
    virtual void SetVisible( bool bVisible ) { m_bVisible = bVisible; }
    virtual bool GetVisible() { return m_bVisible; }
	

    UINT GetType() const { return m_Type; }

    int  GetID() const { return m_ID; }
    void SetID( int ID ) { m_ID = ID; }
	

    void SetLocation( int x, int y ) { m_x = x; m_y = y; UpdateRects(); }
    void SetSize( int width, int height ) { m_width = width; m_height = height; UpdateRects(); }

    void SetHotkey( UINT nHotkey ) { m_nHotkey = nHotkey; }
    UINT GetHotkey() { return m_nHotkey; }

    void SetUserData( void *pUserData ) { m_pUserData = pUserData; }
    void *GetUserData() const { return m_pUserData; }

    CDXUTElement* GetElement( UINT iElement ) { return m_Elements.GetAt( iElement ); }
    HRESULT SetElement( UINT iElement, CDXUTElement* pElement);

    bool m_bVisible;                /// Shown/hidden flag
    bool m_bMouseOver;              /// Mouse pointer is above control
    bool m_bHasFocus;               /// Control has input focus
    bool m_bIsDefault;              /// Is the default control

    /// Size, scale, and positioning members
    int m_x, m_y;
    int m_width, m_height;

    /// These members are set by the container
    CDXUTDialog* m_pDialog;    /// Parent container
    UINT m_Index;              /// Index within the control list
    
    CGrowableArray< CDXUTElement* > m_Elements;  /// All display elements

protected:
    virtual void UpdateRects();

    int  m_ID;                 /// ID number
    DXUT_CONTROL_TYPE m_Type;  /// Control type, set once in constructor  
    UINT m_nHotkey;            /// Virtual key code for this control's hotkey
    void *m_pUserData;         /// Data associated with this control that is set by user.
    
    bool m_bEnabled;           /// Enabled/disabled flag
    
    RECT m_rcBoundingBox;      /// Rectangle defining the active region of the control
};


//-----------------------------------------------------------------------------
/// Contains all the display information for a given control type
//-----------------------------------------------------------------------------
struct DXUTElementHolder
{
    UINT nControlType;
    UINT iElement;

    CDXUTElement Element;
};


//-----------------------------------------------------------------------------
/// Static control
//-----------------------------------------------------------------------------
class ENGINE_API CDXUTStatic : public CDXUTControl
{
public:
    CDXUTStatic( CDXUTDialog *pDialog = NULL );

    virtual void Render( IDirect3DDevice9* pd3dDevice, float fElapsedTime );
    
    HRESULT GetTextCopy( LPWSTR strDest, UINT bufferCount );
    LPCWSTR GetText() { return m_strText; }
    HRESULT SetText( LPCWSTR strText );


protected:
    WCHAR m_strText[MAX_PATH];      /// Window text  
};


//-----------------------------------------------------------------------------
/// Button control
//-----------------------------------------------------------------------------
class ENGINE_API CDXUTButton : public CDXUTStatic
{
public:
    CDXUTButton( CDXUTDialog *pDialog = NULL );

    virtual bool HandleKeyboard( UINT uMsg, WPARAM wParam, LPARAM lParam );
    virtual bool HandleMouse( UINT uMsg, POINT pt, WPARAM wParam, LPARAM lParam );
    virtual void OnHotkey() { m_pDialog->SendEvent( EVENT_BUTTON_CLICKED, true, this ); }
    
    virtual bool CanHaveFocus() { return (m_bVisible && m_bEnabled); }

    virtual void Render( IDirect3DDevice9* pd3dDevice, float fElapsedTime );

protected:
    bool m_bPressed;
};


//-----------------------------------------------------------------------------
/// CheckBox control
//-----------------------------------------------------------------------------
class ENGINE_API CDXUTCheckBox : public CDXUTButton
{
public:
    CDXUTCheckBox( CDXUTDialog *pDialog = NULL );

    virtual bool HandleKeyboard( UINT uMsg, WPARAM wParam, LPARAM lParam );
    virtual bool HandleMouse( UINT uMsg, POINT pt, WPARAM wParam, LPARAM lParam );
    virtual void OnHotkey() { SetCheckedInternal( !m_bChecked, true ); }

    virtual BOOL ContainsPoint( POINT pt ); 
    virtual void UpdateRects(); 

    virtual void Render( IDirect3DDevice9* pd3dDevice, float fElapsedTime );

    bool GetChecked() { return m_bChecked; }
    void SetChecked( bool bChecked ) { SetCheckedInternal( bChecked, false ); }
    
protected:
    virtual void SetCheckedInternal( bool bChecked, bool bFromInput );

    bool m_bChecked;
    RECT m_rcButton;
    RECT m_rcText;
};


//-----------------------------------------------------------------------------
/// RadioButton control
//-----------------------------------------------------------------------------
class ENGINE_API CDXUTRadioButton : public CDXUTCheckBox
{
public:
    CDXUTRadioButton( CDXUTDialog *pDialog = NULL );

    virtual bool HandleKeyboard( UINT uMsg, WPARAM wParam, LPARAM lParam );
    virtual bool HandleMouse( UINT uMsg, POINT pt, WPARAM wParam, LPARAM lParam );
    virtual void OnHotkey() { SetCheckedInternal( !m_bChecked, true, true ); }
    
    void SetChecked( bool bChecked, bool bClearGroup=true ) { SetCheckedInternal( bChecked, bClearGroup, false ); }
    void SetButtonGroup( UINT nButtonGroup ) { m_nButtonGroup = nButtonGroup; }
    UINT GetButtonGroup() { return m_nButtonGroup; }
    
protected:
    virtual void SetCheckedInternal( bool bChecked, bool bClearGroup, bool bFromInput );
    UINT m_nButtonGroup;
};


//-----------------------------------------------------------------------------
/// Scrollbar control
//-----------------------------------------------------------------------------
class ENGINE_API CDXUTScrollBar : public CDXUTControl
{
public:
    CDXUTScrollBar( CDXUTDialog *pDialog = NULL );
    virtual ~CDXUTScrollBar();

    virtual bool    HandleKeyboard( UINT uMsg, WPARAM wParam, LPARAM lParam );
    virtual bool    HandleMouse( UINT uMsg, POINT pt, WPARAM wParam, LPARAM lParam );

    virtual void Render( IDirect3DDevice9* pd3dDevice, float fElapsedTime );
    virtual void    UpdateRects();

    void SetTrackRange( int nStart, int nEnd );
    int GetTrackPos() { return m_nPosition; }
    void SetTrackPos( int nPosition ) { m_nPosition = nPosition; Cap(); UpdateThumbRect(); }
    int GetPageSize() { return m_nPageSize; }
    void SetPageSize( int nPageSize ) { m_nPageSize = nPageSize; Cap(); UpdateThumbRect(); }

    void Scroll( int nDelta );    /// Scroll by nDelta items (plus or minus)
    void ShowItem( int nIndex );  /// Ensure that item nIndex is displayed, scroll if necessary

protected:
    void UpdateThumbRect();
    void Cap();  /// Clips position at boundaries. Ensures it stays within legal range.

    bool m_bShowThumb;
    RECT m_rcUpButton;
    RECT m_rcDownButton;
    RECT m_rcTrack;
    RECT m_rcThumb;
    int m_nPosition;  /// Position of the first displayed item
    int m_nPageSize;  /// How many items are displayable in one page
    int m_nStart;     /// First item
    int m_nEnd;       /// The index after the last item
    public:
    int GetTrackStart()
    {
        return m_nStart;
    }
    int GetTrackEnd()
    {
        return m_nEnd;
    }
};


//-----------------------------------------------------------------------------
/// ListBox control
//-----------------------------------------------------------------------------
struct DXUTListBoxItem
{
    WCHAR strText[256];
    void*  pData;

    RECT  rcActive;
    bool  bSelected;
};

/// ListBox control
class ENGINE_API CDXUTListBox : public CDXUTControl
{
public:
    CDXUTListBox( CDXUTDialog *pDialog = NULL );
    virtual ~CDXUTListBox();

    virtual HRESULT OnInit() { return m_pDialog->InitControl( &m_ScrollBar ); }
    virtual bool    CanHaveFocus() { return (m_bVisible && m_bEnabled); }
    virtual bool    HandleKeyboard( UINT uMsg, WPARAM wParam, LPARAM lParam );
    virtual bool    HandleMouse( UINT uMsg, POINT pt, WPARAM wParam, LPARAM lParam );

    virtual void Render( IDirect3DDevice9* pd3dDevice, float fElapsedTime );
    virtual void    UpdateRects();

    DWORD GetStyle() const { return m_dwStyle; }
    int GetSize() const { return m_Items.GetSize(); }
    void SetStyle( DWORD dwStyle ) { m_dwStyle = dwStyle; }
    void SetScrollBarWidth( int nWidth ) { m_nSBWidth = nWidth; UpdateRects(); }
    void SetBorder( int nBorder, int nMargin ) { m_nBorder = nBorder; m_nMargin = nMargin; }
    HRESULT AddItem( const WCHAR *wszText, void *pData );
    HRESULT InsertItem( int nIndex, const WCHAR *wszText, void *pData );
    void RemoveItem( int nIndex );
    void RemoveItemByText( WCHAR *wszText );
    void RemoveItemByData( void *pData );
    void RemoveAllItems();

    DXUTListBoxItem *GetItem( int nIndex );
    int GetSelectedIndex( int nPreviousSelected = -1 );
    DXUTListBoxItem *GetSelectedItem( int nPreviousSelected = -1 ) { return GetItem( GetSelectedIndex( nPreviousSelected ) ); }
    void SelectItem( int nNewIndex );

    enum STYLE { MULTISELECTION = 1 };

protected:
    RECT m_rcText;      /// Text rendering bound
    RECT m_rcSelection; /// Selection box bound
    CDXUTScrollBar m_ScrollBar;
    int m_nSBWidth;
    int m_nBorder;
    int m_nMargin;
    int m_nTextHeight;  /// Height of a single line of text
    DWORD m_dwStyle;    /// List box style
    int m_nSelected;    /// Index of the selected item for single selection list box
    int m_nSelStart;    /// Index of the item where selection starts (for handling multi-selection)
    bool m_bDrag;       /// Whether the user is dragging the mouse to select

    CGrowableArray< DXUTListBoxItem* > m_Items;
};


//-----------------------------------------------------------------------------
/// Properties List control
//-----------------------------------------------------------------------------
struct DXUTPropListItem
{
    WCHAR strText[256];
	CDXUTControl * editControl;

	int height;
    void*  pData;

    RECT  rcActive;
    bool  bSelected;
	bool bIsTitle;
};

/// Properties List control
class ENGINE_API CDXUTPropertiesList : public CDXUTControl
{
public:
    CDXUTPropertiesList( CDXUTDialog *pDialog = NULL );
    virtual ~CDXUTPropertiesList();

    virtual HRESULT OnInit() { return m_pDialog->InitControl( &m_ScrollBar ); }
    virtual bool    CanHaveFocus() { return (m_bVisible && m_bEnabled); }
    virtual bool    HandleKeyboard( UINT uMsg, WPARAM wParam, LPARAM lParam );
    virtual bool    HandleMouse( UINT uMsg, POINT pt, WPARAM wParam, LPARAM lParam );

    virtual void Render( IDirect3DDevice9* pd3dDevice, float fElapsedTime );
    virtual void    UpdateRects();

	CDXUTControl * GetControlByID(int ID);
    DWORD GetStyle() const { return m_dwStyle; }
    int GetSize() const { return m_Items.GetSize(); }
    void SetStyle( DWORD dwStyle ) { m_dwStyle = dwStyle; }
    void SetScrollBarWidth( int nWidth ) { m_nSBWidth = nWidth; UpdateRects(); }
    void SetBorder( int nBorder, int nMargin ) { m_nBorder = nBorder; m_nMargin = nMargin; }
    void SetLabelWidth( int newWidth ) { m_labelWidth = newWidth; UpdateRects(); }

	HRESULT AddItem( const WCHAR *wszText,int ID, void *pData ,LPCWSTR text);
	HRESULT AddItem( const WCHAR *wszText, void *pData );
	HRESULT AddButton( const WCHAR *wszText,int ID, void *pData ,LPCWSTR text);
	HRESULT AddComboBox( const WCHAR *wszText,int ID, void *pData );
	HRESULT AddSlider( const WCHAR *wszText,int ID,int rangeMin,int rangeMax,int intialValue, void *pData );
	HRESULT AddCheckBox( const WCHAR *wszText,int ID,bool checked, void *pData );
	HRESULT AddStatic( const WCHAR *wszText,int ID,LPCWSTR text, void *pData );
	HRESULT AddTitle( const WCHAR *wszText,int ID,LPCWSTR text, void *pData );
    
	HRESULT InsertItem( int nIndex, const WCHAR *wszText, void *pData );

    void RemoveItem( int nIndex );
    void RemoveItemByText( WCHAR *wszText );
    void RemoveItemByData( void *pData );
    void RemoveAllItems();

    DXUTPropListItem *GetItem( int nIndex );
    int GetSelectedIndex( int nPreviousSelected = -1 );
	
    DXUTPropListItem *GetSelectedItem( int nPreviousSelected = -1 ) { return GetItem( GetSelectedIndex( nPreviousSelected ) ); }
    void SelectItem( int nNewIndex );

    enum STYLE { MULTISELECTION = 1 };

protected:
    RECT m_rcText;      /// Text rendering bound
    RECT m_rcSelection; /// Selection box bound
    CDXUTScrollBar m_ScrollBar;
    int m_nSBWidth;
    int m_nBorder;
    int m_nMargin;
    int m_nTextHeight;  /// Height of a single line of text
    DWORD m_dwStyle;    /// List box style
    int m_nSelected;    /// Index of the selected item for single selection list box
    int m_nSelStart;    /// Index of the item where selection starts (for handling multi-selection)
    bool m_bDrag;       /// Whether the user is dragging the mouse to select
	int m_cntID;
	int m_labelWidth;
    CGrowableArray< DXUTPropListItem* > m_Items;
};


class CDXUTab : public CDXUTStatic
{	
public:
	// this is needed because if a tab is pressed parent needs to know
	// to hide other pages controls
	// control however is in parent it is a part of CDXUTDialog
	CDXUTTabPages *parentPages;

public:
    CDXUTab( CDXUTDialog *pDialog = NULL );

    virtual bool HandleKeyboard( UINT uMsg, WPARAM wParam, LPARAM lParam );
    virtual bool HandleMouse( UINT uMsg, POINT pt, WPARAM wParam, LPARAM lParam );        
    virtual void Render( IDirect3DDevice9* pd3dDevice, float fElapsedTime );

	virtual bool CanHaveFocus() { return (m_bVisible && m_bEnabled); }

protected:
    bool m_bPressed;
};
// individual page of the Tab Pages
struct ENGINE_API DXUTPage
{
	// Name of Page Also shows up in button
    WCHAR strText[256];
	// all controls that deal with a tab page
	// buttons, edit box, etc. (disable all controls on inactive pages)...
    CGrowableArray< CDXUTControl* > m_Controls;	
	// whether or not it is selected
    bool  bSelected;
	// Tab for Page, set Tabpages function to autocreate and link
    CDXUTab *m_Tab;	
};
//template class ENGINE_API CGrowableArray<DXUTPage>;
/// Tab pages control
class ENGINE_API CDXUTTabPages : public CDXUTControl
{
public:
	CGrowableArray< DXUTPage > m_Pages;

    CDXUTTabPages( CDXUTDialog *pDialog = NULL );

	int Displacement;
	int CurIndex;

    virtual bool CanHaveFocus() { return (m_bVisible && m_bEnabled); }
	virtual void AddTabPage(const WCHAR *wszText, int width);
    virtual void Render( IDirect3DDevice9* pd3dDevice, float fElapsedTime );

	// a simple method to avoid mouse over, due to the fact it will
	// stop checking mouse over for other parts if it his the tab...
	virtual BOOL ContainsPoint( POINT pt ) { return false; }

	DXUTPage *GetTabPage(int i)
	{
		return &m_Pages.GetAt(i);
	};

	bool isSelected(CDXUTab *tabIn)
	{
		for(int i=0;i<m_Pages.GetSize();i++)
		{
			if(tabIn == m_Pages.GetAt(i).m_Tab && m_Pages.GetAt(i).bSelected)
				return true;			
		}
		return false;
	};

	// Link In a control to this page
	void AddControlToTabPage(int i, CDXUTControl* controlIn) 
	{ 
		//localize contorls for tab page
		controlIn->SetLocation(controlIn->m_x+this->m_x, controlIn->m_y+this->m_y);	
		m_Pages.GetAt(i).m_Controls.Add(controlIn); 
	};

	// sets all controls visible or hidden then selects first tab
	virtual void ResetToFirstTab()
	{
		for(int i=0;i<m_Pages.GetSize();i++)
		{			
			if(i == 0)
			{
			for(int j=0;j<m_Pages.GetAt(i).m_Controls.GetSize();j++)
			{
				m_Pages.GetAt(i).m_Controls.GetAt(j)->SetEnabled(true);
				m_Pages.GetAt(i).m_Controls.GetAt(j)->SetVisible(true);
				m_Pages.GetAt(i).bSelected=true;
			}
			}
			else
			{			
			for(int j=0;j<m_Pages.GetAt(i).m_Controls.GetSize();j++)
			{
				m_Pages.GetAt(i).m_Controls.GetAt(j)->SetEnabled(false);
				m_Pages.GetAt(i).m_Controls.GetAt(j)->SetVisible(false);
				m_Pages.GetAt(i).bSelected=false;
			}
			};
		};
	};

	void SetAllChildrenLocation( int x, int y ) 
	{ 	
		int differenceX = x - this->m_x;
		int differenceY = y - this->m_y;
		
		this->SetLocation(x,y);
				
		int nX, nY;
		for(int i=0;i<m_Pages.GetSize();i++)
		{				
			nX=m_Pages.GetAt(i).m_Tab->m_x+differenceX;
			nY=m_Pages.GetAt(i).m_Tab->m_y+differenceY;
			m_Pages.GetAt(i).m_Tab->SetLocation(nX,nY);

			for(int j=0;j<m_Pages.GetAt(i).m_Controls.GetSize();j++)
			{
				nX=m_Pages.GetAt(i).m_Controls.GetAt(j)->m_x+differenceX;
				nY=m_Pages.GetAt(i).m_Controls.GetAt(j)->m_y+differenceY;

				m_Pages.GetAt(i).m_Controls.GetAt(j)->SetLocation(nX,nY);				
			}			
		};
	};

	// no need for mouse or keyboard control, done outside of this control and
	// tab pressed is hit once a tab is pressed with this as a parent

	// hide page's controls not associated with tab, and unhide associated page's controls
	virtual void TabPressed(CDXUTab *tabIn)
	{
		for(int i=0;i<m_Pages.GetSize();i++)
		{
			if(tabIn == m_Pages.GetAt(i).m_Tab)
			{
			for(int j=0;j<m_Pages.GetAt(i).m_Controls.GetSize();j++)
			{
				m_Pages.GetAt(i).m_Controls.GetAt(j)->SetEnabled(true);
				m_Pages.GetAt(i).m_Controls.GetAt(j)->SetVisible(true);
				m_Pages.GetAt(i).bSelected=true;
			}
			}
			else
			{
			for(int j=0;j<m_Pages.GetAt(i).m_Controls.GetSize();j++)
			{
				m_Pages.GetAt(i).m_Controls.GetAt(j)->SetEnabled(false);
				m_Pages.GetAt(i).m_Controls.GetAt(j)->SetVisible(false);
				m_Pages.GetAt(i).bSelected=false;
			}
			};
		};

	};

	//CDXUTControl * GetControlByID(int ID);
   // DWORD GetStyle() const { return m_dwStyle; }
   // void SetStyle( DWORD dwStyle ) { m_dwStyle = dwStyle; } 
	
};


//-----------------------------------------------------------------------------
/// ComboBox control
//-----------------------------------------------------------------------------
struct DXUTComboBoxItem
{
    WCHAR strText[256];
    void*  pData;

    RECT  rcActive;
    bool  bVisible;
};

/// ComboBox control
class ENGINE_API CDXUTComboBox : public CDXUTButton
{
public:
    CDXUTComboBox( CDXUTDialog *pDialog = NULL );
    virtual ~CDXUTComboBox();
    
    virtual HRESULT OnInit() { return m_pDialog->InitControl( &m_ScrollBar ); }

    virtual bool HandleKeyboard( UINT uMsg, WPARAM wParam, LPARAM lParam );
    virtual bool HandleMouse( UINT uMsg, POINT pt, WPARAM wParam, LPARAM lParam );
    virtual void OnHotkey();

    virtual bool CanHaveFocus() { return (m_bVisible && m_bEnabled); }
    virtual void OnFocusOut();
    virtual void Render( IDirect3DDevice9* pd3dDevice, float fElapsedTime );

    virtual void UpdateRects(); 

    HRESULT AddItem( const WCHAR* strText, void* pData );
    void    RemoveAllItems();
    void    RemoveItem( UINT index );
    bool    ContainsItem( const WCHAR* strText );
    void*   GetItemData( const WCHAR* strText );
    void    SetDropHeight( UINT nHeight ) { m_nDropHeight = nHeight; UpdateRects(); }
    void    SetScrollBarWidth( int nWidth ) { m_nSBWidth = nWidth; UpdateRects(); }

    void*   GetSelectedData();
    DXUTComboBoxItem* GetSelectedItem();

	int    GetSelectedIndex() { return m_iSelected; }
    UINT    GetNumItems() { return m_Items.GetSize(); }
    DXUTComboBoxItem* GetItem( UINT index ) { return m_Items.GetAt( index ); }

    HRESULT SetSelectedByIndex( UINT index );
    HRESULT SetSelectedByText( const WCHAR* strText );
    HRESULT SetSelectedByData( void* pData );  

protected:
    int     m_iSelected;
    int     m_iFocused;
    int     m_nDropHeight;
    CDXUTScrollBar m_ScrollBar;
    int     m_nSBWidth;

    bool    m_bOpened;

    RECT m_rcText;
    RECT m_rcButton;
    RECT m_rcDropdown;
    RECT m_rcDropdownText;

    int FindItem( const WCHAR* strText );

    CGrowableArray< DXUTComboBoxItem* > m_Items;
};


//-----------------------------------------------------------------------------
/// Slider control
//-----------------------------------------------------------------------------
class ENGINE_API CDXUTSlider : public CDXUTControl
{
public:
    CDXUTSlider( CDXUTDialog *pDialog = NULL );

    virtual BOOL ContainsPoint( POINT pt ); 
    virtual bool CanHaveFocus() { return true; }
    virtual bool HandleKeyboard( UINT uMsg, WPARAM wParam, LPARAM lParam );
    virtual bool HandleMouse( UINT uMsg, POINT pt, WPARAM wParam, LPARAM lParam );
    
    virtual void UpdateRects(); 

    virtual void Render( IDirect3DDevice9* pd3dDevice, float fElapsedTime );

    void SetValue( int nValue ) { SetValueInternal( nValue, false ); }
    int  GetValue() { return m_nValue; };

    void SetRange( int nMin, int nMax );

	float m_UnitScale;

protected:
    void SetValueInternal( int nValue, bool bFromInput );
    int  ValueFromPos( int x ); 

    int m_nValue;

    int m_nMin;
    int m_nMax;

    int m_nDragX;      /// Mouse position at start of drag
    int m_nDragOffset; /// Drag offset from the center of the button
    int m_nButtonX;

    bool m_bPressed;
    RECT m_rcButton;
    public:
    int GetMin()
    {return m_nMin;}

    int GetMax()
    {return m_nMax;}
};


//-----------------------------------------------------------------------------
/// EditBox control
//-----------------------------------------------------------------------------
#define UNISCRIBE_DLLNAME L"usp10.dll"

#define GETPROCADDRESS( Module, APIName, Temp ) \
    Temp = GetProcAddress( Module, #APIName ); \
    if( Temp ) \
        *(FARPROC*)&_##APIName = Temp

#define PLACEHOLDERPROC( APIName ) \
    _##APIName = Dummy_##APIName

/// EditBox control
class ENGINE_API CDXUTEditBox : public CDXUTControl
{
    friend class ENGINE_API CExternalApiInitializer;

public:
    CDXUTEditBox( CDXUTDialog *pDialog = NULL );
    virtual ~CDXUTEditBox();

    virtual bool HandleKeyboard( UINT uMsg, WPARAM wParam, LPARAM lParam );
    virtual bool HandleMouse( UINT uMsg, POINT pt, WPARAM wParam, LPARAM lParam );
    virtual bool MsgProc( UINT uMsg, WPARAM wParam, LPARAM lParam );
    virtual void UpdateRects();
    virtual bool CanHaveFocus() { return (m_bVisible && m_bEnabled); }
    virtual void Render( IDirect3DDevice9* pd3dDevice, float fElapsedTime );
    virtual void    OnFocusIn();

    void SetText( LPCWSTR wszText, bool bSelected = false );
    LPCWSTR GetText() { return m_Buffer.GetBuffer(); }
    int GetTextLength() { return m_Buffer.GetTextSize(); }  /// Returns text length in chars excluding NULL.
    HRESULT GetTextCopy( LPWSTR strDest, UINT bufferCount );
    void ClearText();
    void SetTextColor( D3DCOLOR Color ) { m_TextColor = Color; }  /// Text color
    void SetSelectedTextColor( D3DCOLOR Color ) { m_SelTextColor = Color; }  /// Selected text color
    void SetSelectedBackColor( D3DCOLOR Color ) { m_SelBkColor = Color; }  /// Selected background color
    void SetCaretColor( D3DCOLOR Color ) { m_CaretColor = Color; }  /// Caret color
    void SetBorderWidth( int nBorder ) { m_nBorder = nBorder; UpdateRects(); }  /// Border of the window
    int GetBorderWidth(  ) {return m_nBorder ; }
    void SetSpacing( int nSpacing ) { m_nSpacing = nSpacing; UpdateRects(); }
    void ParseFloatArray( float *pNumbers, int nCount );
    void SetTextFloatArray( const float *pNumbers, int nCount );

public:
    //-----------------------------------------------------------------------------
    /// CUniBuffer class for the edit control
    //-----------------------------------------------------------------------------
    class ENGINE_API CUniBuffer
    {
        friend class ENGINE_API CExternalApiInitializer;

        /// static member

        /// Empty implementation of the Uniscribe API
        static HRESULT WINAPI Dummy_ScriptApplyDigitSubstitution( const SCRIPT_DIGITSUBSTITUTE*, SCRIPT_CONTROL*, SCRIPT_STATE* ) { return E_NOTIMPL; }
        static HRESULT WINAPI Dummy_ScriptStringAnalyse( HDC, const void *, int, int, int, DWORD, int, SCRIPT_CONTROL*, SCRIPT_STATE*, const int*, SCRIPT_TABDEF*, const BYTE*, SCRIPT_STRING_ANALYSIS* ) { return E_NOTIMPL; }
        static HRESULT WINAPI Dummy_ScriptStringCPtoX( SCRIPT_STRING_ANALYSIS, int, BOOL, int* ) { return E_NOTIMPL; }
        static HRESULT WINAPI Dummy_ScriptStringXtoCP( SCRIPT_STRING_ANALYSIS, int, int*, int* ) { return E_NOTIMPL; }
        static HRESULT WINAPI Dummy_ScriptStringFree( SCRIPT_STRING_ANALYSIS* ) { return E_NOTIMPL; }
        static const SCRIPT_LOGATTR* WINAPI Dummy_ScriptString_pLogAttr( SCRIPT_STRING_ANALYSIS ) { return NULL; }
        static const int* WINAPI Dummy_ScriptString_pcOutChars( SCRIPT_STRING_ANALYSIS ) { return NULL; }

        /// Function pointers
        static HRESULT (WINAPI *_ScriptApplyDigitSubstitution)( const SCRIPT_DIGITSUBSTITUTE*, SCRIPT_CONTROL*, SCRIPT_STATE* );
        static HRESULT (WINAPI *_ScriptStringAnalyse)( HDC, const void *, int, int, int, DWORD, int, SCRIPT_CONTROL*, SCRIPT_STATE*, const int*, SCRIPT_TABDEF*, const BYTE*, SCRIPT_STRING_ANALYSIS* );
        static HRESULT (WINAPI *_ScriptStringCPtoX)( SCRIPT_STRING_ANALYSIS, int, BOOL, int* );
        static HRESULT (WINAPI *_ScriptStringXtoCP)( SCRIPT_STRING_ANALYSIS, int, int*, int* );
        static HRESULT (WINAPI *_ScriptStringFree)( SCRIPT_STRING_ANALYSIS* );
        static const SCRIPT_LOGATTR* (WINAPI *_ScriptString_pLogAttr)( SCRIPT_STRING_ANALYSIS );
        static const int* (WINAPI *_ScriptString_pcOutChars)( SCRIPT_STRING_ANALYSIS );

        static InitializeUniscribe();
        static UninitializeUniscribe();

        static HINSTANCE s_hDll;  /// Uniscribe DLL handle

    /// Instance data
    private:
        WCHAR *m_pwszBuffer;    /// Buffer to hold text
        int    m_nBufferSize;   /// Size of the buffer allocated, in characters
        int    m_nTextSize;     /// Size of the text in buffer excluding
                                ///   NULL terminator, in characters
        /// Uniscribe-specific
        HDC m_hDC;              /// DC used for Uniscribe analysis
        bool m_bAnalyseRequired;/// True if the string has changed since last analysis.
        SCRIPT_STRING_ANALYSIS m_Analysis;  /// Analysis for the current string

    private:
        bool Grow();

        /// Uniscribe -- Analyse() analyses the string in the buffer
        HRESULT Analyse();

    public:
        CUniBuffer( int nInitialSize = 1 );
        ~CUniBuffer();
        int GetBufferSize() { return m_nBufferSize; }
        bool SetBufferSize( int nSize );
        int GetTextSize() { return m_nTextSize; }
        const WCHAR *GetBuffer() { return m_pwszBuffer; }
        const WCHAR &operator[]( int n ) const { return m_pwszBuffer[n]; }
        WCHAR &operator[]( int n );
        HDC GetDC() { return m_hDC; }
        void SetDC( HDC hDC ) { m_hDC = hDC; }
        void Clear();

        /// Inserts the char at specified index.
        /// If nIndex == -1, insert to the end.
        bool InsertChar( int nIndex, WCHAR wChar );

        /// Removes the char at specified index.
        /// If nIndex == -1, remove the last char.
        bool RemoveChar( int nIndex );
        bool SetText( LPCWSTR wszText );

        /// Uniscribe
        HRESULT CPtoX( int nCP, BOOL bTrail, int *pX );
        HRESULT XtoCP( int nX, int *pCP, int *pnTrail );
        void GetPriorItemPos( int nCP, int *pPrior );
        void GetNextItemPos( int nCP, int *pPrior );
    };

protected:
    void PlaceCaret( int nCP );
    void DeleteSelectionText();
    void ResetCaretBlink();
    void CopyToClipboard();
    void PasteFromClipboard();

    CUniBuffer m_Buffer;     /// Buffer to hold text
    int      m_nBorder;      /// Border of the window
    int      m_nSpacing;     /// Spacing between the text and the edge of border
    RECT     m_rcText;       /// Bounding rectangle for the text
    RECT     m_rcRender[9];  /// Convenient rectangles for rendering elements
    double   m_dfBlink;      /// Caret blink time in milliseconds
    double   m_dfLastBlink;  /// Last timestamp of caret blink
    bool     m_bCaretOn;     /// Flag to indicate whether caret is currently visible
    int      m_nCaret;       /// Caret position, in characters
    bool     m_bInsertMode;  /// If true, control is in insert mode. Else, overwrite mode.
    int      m_nSelStart;    /// Starting position of the selection. The caret marks the end.
    int      m_nFirstVisible;/// First visible character in the edit control
    D3DCOLOR m_TextColor;    /// Text color
    D3DCOLOR m_SelTextColor; /// Selected text color
    D3DCOLOR m_SelBkColor;   /// Selected background color
    D3DCOLOR m_CaretColor;   /// Caret color

    /// Mouse-specific
    bool m_bMouseDrag;       /// True to indicate drag in progress

    /// Static
    static bool     s_bHideCaret;   /// If true, we don't render the caret.
};


//-----------------------------------------------------------------------------
/// IME-enabled EditBox control
//-----------------------------------------------------------------------------
#define IMM32_DLLNAME L"imm32.dll"
#define VER_DLLNAME L"version.dll"
#define MAX_CANDLIST 9
#define MAX_COMPSTRING_SIZE 256

/// IME-enabled EditBox control
class ENGINE_API CDXUTIMEEditBox : public CDXUTEditBox
{
    friend class ENGINE_API CExternalApiInitializer;

    static void InitializeImm();
    static void UninitializeImm();

    static HINSTANCE s_hDllImm32;  /// IMM32 DLL handle
    static HINSTANCE s_hDllVer;    /// Version DLL handle
    static HIMC      s_hImcDef;    /// Default input context

protected:
    /// Empty implementation of the IMM32 API
    static INPUTCONTEXT* WINAPI Dummy_ImmLockIMC( HIMC ) { return NULL; }
    static BOOL WINAPI Dummy_ImmUnlockIMC( HIMC ) { return FALSE; }
    static LPVOID WINAPI Dummy_ImmLockIMCC( HIMCC ) { return NULL; }
    static BOOL WINAPI Dummy_ImmUnlockIMCC( HIMCC ) { return FALSE; }
    static BOOL WINAPI Dummy_ImmDisableTextFrameService( DWORD ) { return TRUE; }
    static LONG WINAPI Dummy_ImmGetCompositionStringW( HIMC, DWORD, LPVOID, DWORD ) { return IMM_ERROR_GENERAL; }
    static DWORD WINAPI Dummy_ImmGetCandidateListW( HIMC, DWORD, LPCANDIDATELIST, DWORD ) { return 0; }
    static HIMC WINAPI Dummy_ImmGetContext( HWND ) { return NULL; }
    static BOOL WINAPI Dummy_ImmReleaseContext( HWND, HIMC ) { return FALSE; }
    static HIMC WINAPI Dummy_ImmAssociateContext( HWND, HIMC ) { return NULL; }
    static BOOL WINAPI Dummy_ImmGetOpenStatus( HIMC ) { return 0; }
    static BOOL WINAPI Dummy_ImmSetOpenStatus( HIMC, BOOL ) { return 0; }
    static BOOL WINAPI Dummy_ImmGetConversionStatus( HIMC, LPDWORD, LPDWORD ) { return 0; }
    static HWND WINAPI Dummy_ImmGetDefaultIMEWnd( HWND ) { return NULL; }
    static UINT WINAPI Dummy_ImmGetIMEFileNameA( HKL, LPSTR, UINT ) { return 0; }
    static UINT WINAPI Dummy_ImmGetVirtualKey( HWND ) { return 0; }
    static BOOL WINAPI Dummy_ImmNotifyIME( HIMC, DWORD, DWORD, DWORD ) { return FALSE; }
    static BOOL WINAPI Dummy_ImmSetConversionStatus( HIMC, DWORD, DWORD ) { return FALSE; }
    static BOOL WINAPI Dummy_ImmSimulateHotKey( HWND, DWORD ) { return FALSE; }
    static BOOL WINAPI Dummy_ImmIsIME( HKL ) { return FALSE; }

    /// Traditional Chinese IME
    static UINT WINAPI Dummy_GetReadingString( HIMC, UINT, LPWSTR, PINT, BOOL*, PUINT ) { return 0; }
    static BOOL WINAPI Dummy_ShowReadingWindow( HIMC, BOOL ) { return FALSE; }

    /// Verion library imports
    static BOOL APIENTRY Dummy_VerQueryValueA( const LPVOID, LPSTR, LPVOID *, PUINT ) { return 0; }
    static BOOL APIENTRY Dummy_GetFileVersionInfoA( LPSTR, DWORD, DWORD, LPVOID ) { return 0; }
    static DWORD APIENTRY Dummy_GetFileVersionInfoSizeA( LPSTR, LPDWORD ) { return 0; }

    /// Function pointers: IMM32
    static INPUTCONTEXT* (WINAPI * _ImmLockIMC)( HIMC );
    static BOOL (WINAPI * _ImmUnlockIMC)( HIMC );
    static LPVOID (WINAPI * _ImmLockIMCC)( HIMCC );
    static BOOL (WINAPI * _ImmUnlockIMCC)( HIMCC );
    static BOOL (WINAPI * _ImmDisableTextFrameService)( DWORD );
    static LONG (WINAPI * _ImmGetCompositionStringW)( HIMC, DWORD, LPVOID, DWORD );
    static DWORD (WINAPI * _ImmGetCandidateListW)( HIMC, DWORD, LPCANDIDATELIST, DWORD );
    static HIMC (WINAPI * _ImmGetContext)( HWND );
    static BOOL (WINAPI * _ImmReleaseContext)( HWND, HIMC );
    static HIMC (WINAPI * _ImmAssociateContext)( HWND, HIMC );
    static BOOL (WINAPI * _ImmGetOpenStatus)( HIMC );
    static BOOL (WINAPI * _ImmSetOpenStatus)( HIMC, BOOL );
    static BOOL (WINAPI * _ImmGetConversionStatus)( HIMC, LPDWORD, LPDWORD );
    static HWND (WINAPI * _ImmGetDefaultIMEWnd)( HWND );
    static UINT (WINAPI * _ImmGetIMEFileNameA)( HKL, LPSTR, UINT );
    static UINT (WINAPI * _ImmGetVirtualKey)( HWND );
    static BOOL (WINAPI * _ImmNotifyIME)( HIMC, DWORD, DWORD, DWORD );
    static BOOL (WINAPI * _ImmSetConversionStatus)( HIMC, DWORD, DWORD );
    static BOOL (WINAPI * _ImmSimulateHotKey)( HWND, DWORD );
    static BOOL (WINAPI * _ImmIsIME)( HKL );

    /// Function pointers: Traditional Chinese IME
    static UINT (WINAPI * _GetReadingString)( HIMC, UINT, LPWSTR, PINT, BOOL*, PUINT );
    static BOOL (WINAPI * _ShowReadingWindow)( HIMC, BOOL );

    /// Function pointers: Verion library imports
    static BOOL (APIENTRY * _VerQueryValueA)( const LPVOID, LPSTR, LPVOID *, PUINT );
    static BOOL (APIENTRY * _GetFileVersionInfoA)( LPSTR, DWORD, DWORD, LPVOID );
    static DWORD (APIENTRY * _GetFileVersionInfoSizeA)( LPSTR, LPDWORD );

public:
    CDXUTIMEEditBox( CDXUTDialog *pDialog = NULL );
    virtual ~CDXUTIMEEditBox();

    static  HRESULT StaticOnCreateDevice();
    static  bool StaticMsgProc( UINT uMsg, WPARAM wParam, LPARAM lParam );

    static  void EnableImeSystem( bool bEnable );

    virtual void Render( IDirect3DDevice9* pd3dDevice, float fElapsedTime );
    virtual bool MsgProc( UINT uMsg, WPARAM wParam, LPARAM lParam );
    virtual bool HandleMouse( UINT uMsg, POINT pt, WPARAM wParam, LPARAM lParam );
    virtual void UpdateRects();
    virtual void OnFocusIn();
    virtual void OnFocusOut();

    void PumpMessage();

    virtual void RenderCandidateReadingWindow( IDirect3DDevice9* pd3dDevice, float fElapsedTime, bool bReading );
    virtual void RenderComposition( IDirect3DDevice9* pd3dDevice, float fElapsedTime );
    virtual void RenderIndicator( IDirect3DDevice9* pd3dDevice, float fElapsedTime );

protected:
    static WORD GetLanguage() { return LOWORD( s_hklCurrent ); }
    static WORD GetPrimaryLanguage() { return PRIMARYLANGID( LOWORD( s_hklCurrent ) ); }
    static WORD GetSubLanguage() { return SUBLANGID( LOWORD( s_hklCurrent ) ); }
    static void SendKey( BYTE nVirtKey );
    static DWORD GetImeId( UINT uIndex = 0 );
    static void CheckInputLocale();
    static void CheckToggleState();
    static void SetupImeApi();
    static void ResetCompositionString();
    void TruncateCompString( bool bUseBackSpace = true, int iNewStrLen = 0 );
    void FinalizeString( bool bSend );
    static void GetReadingWindowOrientation( DWORD dwId );
    static void GetPrivateReadingString();

    void SendCompString();

protected:
    enum { INDICATOR_NON_IME, INDICATOR_CHS, INDICATOR_CHT, INDICATOR_KOREAN, INDICATOR_JAPANESE };
    enum IMESTATE { IMEUI_STATE_OFF, IMEUI_STATE_ON, IMEUI_STATE_ENGLISH };

	/// Candidate List
    struct CCandList
    {
        WCHAR awszCandidate[MAX_CANDLIST][256];
        CUniBuffer HoriCand; /// Candidate list string (for horizontal candidate window)
        int   nFirstSelected; /// First character position of the selected string in HoriCand
        int   nHoriSelectedLen; /// Length of the selected string in HoriCand
        DWORD dwCount;       /// Number of valid entries in the candidate list
        DWORD dwSelection;   /// Currently selected candidate entry relative to page top
        DWORD dwPageSize;
        int   nReadingError; /// Index of the error character
        bool  bShowWindow;   /// Whether the candidate list window is visible
        RECT  rcCandidate;   /// Candidate rectangle computed and filled each time before rendered
    };

	/// Input locale
    struct CInputLocale
    {
        HKL   m_hKL;            /// Keyboard layout
        WCHAR m_wszLangAbb[3];  /// Language abbreviation
        WCHAR m_wszLang[64];    /// Localized language name
    };

    /// Application-wide data
    static HKL     s_hklCurrent;          /// Current keyboard layout of the process
    static bool    s_bVerticalCand;       /// Indicates that the candidates are listed vertically
    static LPWSTR  s_wszCurrIndicator;    /// Points to an indicator string that corresponds to current input locale
    static WCHAR   s_aszIndicator[5][3];  /// String to draw to indicate current input locale
    static bool    s_bInsertOnType;       /// Insert the character as soon as a key is pressed (Korean behavior)
    static HINSTANCE s_hDllIme;           /// Instance handle of the current IME module
    static IMESTATE  s_ImeState;          /// IME global state
    static bool    s_bEnableImeSystem;    /// Whether the IME system is active
    static POINT   s_ptCompString;        /// Composition string position. Updated every frame.
    static int     s_nCompCaret;          /// Caret position of the composition string
    static int     s_nFirstTargetConv;    /// Index of the first target converted char in comp string.  If none, -1.
    static CUniBuffer s_CompString;       /// Buffer to hold the composition string (we fix its length)
    static BYTE    s_abCompStringAttr[MAX_COMPSTRING_SIZE];
    static DWORD   s_adwCompStringClause[MAX_COMPSTRING_SIZE];
    static WCHAR   s_wszReadingString[32];/// Used only with horizontal reading window (why?)
    static CCandList s_CandList;          /// Data relevant to the candidate list
    static bool    s_bShowReadingWindow;  /// Indicates whether reading window is visible
    static bool    s_bHorizontalReading;  /// Indicates whether the reading window is vertical or horizontal
    static bool    s_bChineseIME;
    static CGrowableArray< CInputLocale > s_Locale; /// Array of loaded keyboard layout on system

    /// Color of various IME elements
    D3DCOLOR       m_ReadingColor;        /// Reading string color
    D3DCOLOR       m_ReadingWinColor;     /// Reading window color
    D3DCOLOR       m_ReadingSelColor;     /// Selected character in reading string
    D3DCOLOR       m_ReadingSelBkColor;   /// Background color for selected char in reading str
    D3DCOLOR       m_CandidateColor;      /// Candidate string color
    D3DCOLOR       m_CandidateWinColor;   /// Candidate window color
    D3DCOLOR       m_CandidateSelColor;   /// Selected candidate string color
    D3DCOLOR       m_CandidateSelBkColor; /// Selected candidate background color
    D3DCOLOR       m_CompColor;           /// Composition string color
    D3DCOLOR       m_CompWinColor;        /// Composition string window color
    D3DCOLOR       m_CompCaretColor;      /// Composition string caret color
    D3DCOLOR       m_CompTargetColor;     /// Composition string target converted color
    D3DCOLOR       m_CompTargetBkColor;   /// Composition string target converted background
    D3DCOLOR       m_IndicatorImeColor;   /// Indicator text color for IME
    D3DCOLOR       m_IndicatorEngColor;   /// Indicator text color for English
    D3DCOLOR       m_IndicatorBkColor;    /// Indicator text background color

    /// Edit-control-specific data
    int            m_nIndicatorWidth;     /// Width of the indicator symbol
    RECT           m_rcIndicator;         /// Rectangle for drawing the indicator button
};



#endif /// DXUT_GUI_H
