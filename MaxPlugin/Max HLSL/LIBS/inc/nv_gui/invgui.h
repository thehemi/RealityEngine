/*********************************************************************NVMH4****
Path:  NVSDK\Common\include\nv_gui
File:  invgui.h

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


  This is the interface to the control panel.
  There is also an event callback interface which you can pass to the panel to get
  notifications when the panel is changed.  Notably, you get the info that a GUI 
  item has changed this way.

  The panel interface is pretty straight forward.  It enables creation/deletion of a panel
  and refreshing of the panel's interfaces.
******************************************************************************/

#ifndef __INVGUI_H
#define __INVGUI_H

#include "invguidata.h"

namespace nv_gui
{
static const char* EVENT_CLOSEPANEL = "OnClosePanel";
static const char* EVENT_UPDATEITEM = "OnUpdateItem";

//! An event callback interface
class INVGUIEvent
{
public:
	//! Called to pass a message to the client, about gui item changes.
	virtual bool INTCALLTYPE PostMessage(const char* pszEvent, void* pEventData) = 0;

	//! Called to tell the client that nv_gui has been given the focus.
	virtual void INTCALLTYPE OnSetFocus() = 0;

	//! Called to tell the client that nv_gui has lost the focus.
	virtual void INTCALLTYPE OnKillFocus() = 0;
};

//! A complex interface allowing a client app to display a list of gui controls and receive notifications when they change
/*! The MAX plugin uses nv_gui to extend the MAX UI in a dynamic way, something which is difficult to do in version 5
*/
class INVGUI : public nv_sys::INVObject
{
public:
	static INVGUI* CreateInstance();

	//! Set the app client name.
	/*! Used to setup the registry key used to keep settings. */
    virtual void INTCALLTYPE SetAppName(const char * pAppName) = 0;

	//! Setup the window that owns this window.
    virtual void INTCALLTYPE SetParentWindow(HWND hWnd) = 0;

	//! Refresh an item in the gui because it has been changed externally.
	virtual bool INTCALLTYPE RefreshGUI(INVGUIItem* pItem) = 0;

	//! Clear all items in the GUI
	virtual bool INTCALLTYPE ClearGUI() = 0;

	//! Add an item to the GUI
	virtual bool INTCALLTYPE AddItem(nv_gui::INVGUIItem* pItem) = 0;

	//! Remove an item from the GUI
	virtual bool INTCALLTYPE RemoveItem(nv_gui::INVGUIItem* pItem) = 0;

	//! Add a child item to the GUI, displayed like a treeview.
	virtual bool INTCALLTYPE AddChild(nv_gui::INVGUIItem* pParent, nv_gui::INVGUIItem* pItem) = 0;

	//! Remove all children below this item.
	virtual bool INTCALLTYPE RemoveChildren(nv_gui::INVGUIItem* pParent) = 0;
	
	//! Set the visibility of the dialog.
	virtual bool INTCALLTYPE SetVisible(bool bHide) = 0;

	//! Check the visibility of the dialog.
	virtual bool INTCALLTYPE IsVisible() = 0;

	//! Add an event sink, which will be called back when gui items change.
	virtual bool INTCALLTYPE AddEventSink(INVGUIEvent* pSink) = 0;

	//! Update the images used by the GUI items.  An OS dependent call.
    virtual bool INTCALLTYPE SetImageList(HIMAGELIST hImageList,HIMAGELIST hImageListDisabled) = 0;

	//! Update the image used by an individual gui item.
    virtual unsigned int INTCALLTYPE SetStateImageIdx(nv_gui::INVGUIItem::ItemState state, unsigned int idx) = 0;

	//! Get the image used by an individual gui item.
    virtual unsigned int INTCALLTYPE GetStateImageIdx(nv_gui::INVGUIItem::ItemState state ) = 0;

	//! Called to give the GUI chance to catch messages passed to the main app which are intended for it
    virtual bool INTCALLTYPE PreTranslateMessage(WORD hWnd, WORD Message, WORD wParam, DWORD lParam) = 0;

	//! Expand all items in the gui.
    virtual bool INTCALLTYPE ExpandAll() = 0;
};

}; // namespace nv_gui

// {8ECA95B2-9805-41f5-8DC0-2309942FA3C9}
static const nv_sys::NVGUID IID_INVGUI = 
{ 0x8eca95b2, 0x9805, 0x41f5, { 0x8d, 0xc0, 0x23, 0x9, 0x94, 0x2f, 0xa3, 0xc9 } };

#endif //__IDYNAMICPAN_H