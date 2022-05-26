//-----------------------------------------------------------------------------
// File: XSkinExp.h
//
// Desc: Export interface to max definition
//
// Copyright (C) 1998-2000 Microsoft Corporation. All Rights Reserved.
//-----------------------------------------------------------------------------
#ifndef __XSKINEXP__H
#define __XSKINEXP__H

#include "xcustom.h"
#define VERSION_NUMBER 1
#define RELEASE(x) if(x) {x->Release(); x = NULL;}

#define MAX_OFFSET_VECTORS 20

extern TCHAR *GetString(int id);
//extern HINSTANCE g_hInstance;

extern TCHAR errstr[500 + _MAX_PATH];
VOID XSkinExp_OutputDebugString(LPCTSTR lpOutputString);


typedef DWORD DXFILEFORMAT;
struct SDialogOptions
{
	SceneProperties				sceneData; // Header
	DXFILEFORMAT                m_xFormat;
	DWORD                       m_cMaxBonesPerVertex;
	DWORD                       m_cMaxBonesPerFace;

	BOOL                        m_bSavePatchData;
	BOOL                        m_bSaveAnimationData;
	BOOL                        m_bLoopingAnimationData;
	DWORD                       m_iAnimSamplingRate;
	BOOL						m_bSavePrefabData; // Rarely save prefab data, except when exporting prefabs from scene
};

HRESULT ExportXFile
(
 const TCHAR *szFilename,
 INode* pRootNode,
 Interface *pInterface, 
 BOOL bSuppressPrompts,
 BOOL bSaveSelection,
 HWND hwndParent,
 SDialogOptions DlgOptions
 );



#endif // __XSKINEXP__H
