/*********************************************************************NVMH4****
Path:  NVSDK\Common\include\nv_gui
File:  DynamicDlg.h

Copyright NVIDIA Corporation 2002
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED
*AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS
BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES
WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS,
BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS)
ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS
BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.



Comments:





******************************************************************************/

#if !defined(AFX_CGFXDLG_H__13B6ACF4_E94A_484C_ABCA_C06F4695AB45__INCLUDED_)
#define AFX_CGFXDLG_H__13B6ACF4_E94A_484C_ABCA_C06F4695AB45__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DynamicDlg.h : header file
//
namespace nv_gui
{

    // these constants represent the dimensions and number of buttons in
// the default MFC-generated toolbar. If you need something different,
// feel free to change them. For extra credit, you can load the
// toolbar's existing image list at runtime and copy the parameters from
// there.
static const int	kImageWidth (32);
static const int	kImageHeight (32);
static const int	kNumImages (1);

static const UINT	kToolBarBitDepth (ILC_COLOR24);

// this color will be treated as transparent in the loaded bitmaps --
// in other words, any pixel of this color will be set at runtime to
// the user's button color. The Visual Studio toolbar editor defaults
// to 192, 192, 192 (light gray).
static const RGBTRIPLE	kBackgroundColor = {200, 208, 212};

/////////////////////////////////////////////////////////////////////////////
// CDynamicDlg dialog
typedef std::map<CPropTreeItem*, nv_gui::INVGUIItem*> tmapTreeItemGUIItem;
typedef std::map<nv_gui::INVGUIItem*, CPropTreeItem*> tmapGUIItemTreeItem;

class CDynamicDlg : public CDialog, public nv_gui::INVGUI
{
// Construction
public:
	CDynamicDlg(CWnd* pWnd = NULL);   // standard constructor

	static bool INTCALLTYPE CreateNVObject(const nv_sys::INVCreator* pCreator, const nv_sys::NVGUID& InterfaceID, void** ppObj);

	virtual unsigned long INTCALLTYPE AddRef();
	virtual unsigned long INTCALLTYPE Release();
	virtual bool INTCALLTYPE GetInterface(const nv_sys::NVGUID& InterfaceID, void** pObj);

	virtual bool INTCALLTYPE RefreshGUI(nv_gui::INVGUIItem* pItem);
	virtual bool INTCALLTYPE ClearGUI();
	virtual bool INTCALLTYPE AddEventSink(nv_gui::INVGUIEvent* pSink);
    virtual bool INTCALLTYPE SetImageList(HIMAGELIST hImageList,HIMAGELIST hImageListDisabled);
    virtual unsigned int INTCALLTYPE    SetStateImageIdx(INVGUIItem::ItemState state, unsigned int idx);
    virtual unsigned int INTCALLTYPE    GetStateImageIdx(INVGUIItem::ItemState state);
	virtual bool INTCALLTYPE SetVisible(bool bHide);
	virtual bool INTCALLTYPE IsVisible();
	virtual bool INTCALLTYPE AddItem(nv_gui::INVGUIItem* pItem);
	virtual bool INTCALLTYPE RemoveItem(nv_gui::INVGUIItem* pItem);
	virtual bool INTCALLTYPE AddChild(nv_gui::INVGUIItem* pParent, nv_gui::INVGUIItem* pChild);
	virtual bool INTCALLTYPE RemoveChildren(nv_gui::INVGUIItem* pParent);
    virtual void INTCALLTYPE SetAppName(const char * pAppName);
    virtual void INTCALLTYPE SetParentWindow(HWND hWnd);
    virtual bool INTCALLTYPE PreTranslateMessage(WORD hWnd, WORD Message, WORD wParam, DWORD lParam);
    virtual bool INTCALLTYPE ExpandAll();
	INVGUIEvent* GetEventSink() { return m_pEventSink; }
// Dialog Data
	//{{AFX_DATA(CDynamicDlg)
	enum { IDD = IDD_PANEL };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDynamicDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL
	//virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
private:
	CPropTreeItem* SetupItem(CPropTreeItem* pRoot, INVGUIItem* pItem);

	typedef enum
	{
		COPY_TREEITEM_TO_GUIITEM, 
		COPY_GUIITEM_TO_TREEITEM
	} tSyncDirection;

	bool SyncData(INVGUIItem* pGUIItem, CPropTreeItem* pTreeItem, tSyncDirection SyncDirection);

	std::auto_ptr<nv_gui::INVGUIData>   m_pGUIData;
	nv_gui::INVGUIEvent *               m_pEventSink;
	unsigned long                       m_dwRefCount;
	CPropTree                           m_Tree;
	tmapTreeItemGUIItem	                m_mapTreeItemGUIItem;
	tmapGUIItemTreeItem                 m_mapGUIItemTreeItem;
    WINDOWPLACEMENT                     m_wndpos;
    std::string                         m_AppName;
    CDocument *                         m_pDoc;

    // Toolbar
    void            SetupToolBar(CToolBar& ToolBar, 
					    		  UINT BarImageID,
						    	  UINT BarDisabledImageID,
							      UINT BarHotImageID, 
							      CImageList& Image,
							      CImageList& ImageDisabled,
							      CImageList& ImageHot);

    CImageList                          m_ToolBarImagesDisabled;
	CImageList                          m_ToolBarImagesHot;
	CImageList                          m_ToolBarImages;

    CDlgToolBar                         m_wndToolBar;

	// Generated message map functions
	//{{AFX_MSG(CDynamicDlg)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClose();
    afx_msg void OnDestroy();
    afx_msg void OnItemChanged(NMHDR* pNotifyStruct, LRESULT* plResult);
    afx_msg void OnFileEdit(NMHDR* pNotifyStruct, LRESULT* plResult);
    afx_msg void OnReadOnlyAttributes();
    afx_msg void OnUpdateReadOnlyAttributes( CCmdUI* pCmdUI );
    afx_msg void OnExpandAttributes();
    afx_msg void OnUpdateExpandAttributes( CCmdUI* pCmdUI );
    afx_msg void OnWindowPosChanged( WINDOWPOS* lpwndpos );
    //}}AFX_MSG
	DECLARE_MESSAGE_MAP()
//public:
	virtual BOOL OnInitDialog();
};

}; // namespace nvgui

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CGFXDLG_H__13B6ACF4_E94A_484C_ABCA_C06F4695AB45__INCLUDED_)
