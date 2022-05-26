/*********************************************************************NVMH4****
NVSDK not found!
Path:  
File:  tweakables.h

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


See tweakables.cpp

******************************************************************************/
#if !defined(AFX_TWEAKABLES_H__1337C21D_0B62_48A4_8EFB_6798E22A7AEC__INCLUDED_)
#define AFX_TWEAKABLES_H__1337C21D_0B62_48A4_8EFB_6798E22A7AEC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "shaderinfo.h"

typedef enum tageTweakableType
{
	TWEAKTYPE_CONNECTION = 0,
    TWEAKTYPE_CGFXSHADER = 1,
    TWEAKTYPE_TECHNIQUE = 2,
	TWEAKTYPE_LIGHT = 3
} eTweakableType;

typedef struct tagTweakableInfo
{
	eTweakableType m_Type;
	nv_sys::INVConnectionParameter* m_pConnection;
} tTweakableInfo;

typedef std::map<nv_gui::INVGUIItem*, tTweakableInfo> tmapGUIItemTweakableInfo;

extern DWORD MESSAGE_EVENT_CLOSEPANEL;
extern DWORD MESSAGE_EVENT_UPDATEITEM;


class CgFXMaterial;
class CTweakables : public nv_gui::INVGUIEvent
{
public:
    CTweakables(CgFXMaterial* pMaterial);
	virtual ~CTweakables();

	void                                OnShowTweakables();
	bool                                BuildGUI();
	bool                                RefreshGUI();
	bool                                ClearGUI();
	nv_gui::INVGUI*                     GetGUI();

	// INVGUIEvent
	virtual bool INTCALLTYPE            PostMessage(const char* pszEvent, void* pEventData);
	virtual void INTCALLTYPE            OnSetFocus();
	virtual void INTCALLTYPE            OnKillFocus();
	
	virtual DWORD                       GetMessage(const char* szMessage);
	virtual bool                        OnEvent(const char* pszEvent, void* pEventData1);

private:

	typedef enum
	{
		COPY_CONNECTION_TO_GUIITEM,
		COPY_GUIITEM_TO_CONNECTION,
	} tSyncDirection;

	void                                FillGUI(nv_gui::INVGUIItem* pRoot, nv_sys::INVParameterList* pParameterList);
	void                                SyncGUIItemToConnection( nv_gui::INVGUIItem* pItem, 
                                                                nv_sys::INVConnectionParameter* pConnect, 
                                                                tSyncDirection SyncDirection);
	bool                                SetupGUI();
	
	nv_gui::INVGUIItem*					m_pTechniqueTweak;
	CgFXMaterial*                       m_pMaterial;
	nv_gui::INVGUIItem*                 m_pRootItem;
	HINSTANCE                           m_hGUILib;
	nv_gui::INVGUI*                     m_pGUI;
	tmapGUIItemTweakableInfo            m_mapGUIItemTweakableInfo;
    HIMAGELIST                          m_hImageList;
};

#endif __TWEAKABLES_H