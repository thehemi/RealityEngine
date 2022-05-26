/*********************************************************************NVMH4****
Path:  plugins\nvmax
File:  DllEntry.cpp

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

Main entry point for dll.


******************************************************************************/
#include "pch.h"
#include "RenderMesh.h"
#include "Material.h"
#include "vertexshader.h"
#include "scenemgr.h"

extern ClassDesc2* GetDefaultShaderDesc();

HINSTANCE	g_hInstance = NULL;
HHOOK       g_hHookMsg = NULL;
int			controlsInit = FALSE;
//nv_plugin::CPluginManager g_PluginManager;

/*
class CFileSearcher : public nv_plugin::ISearchFile
{
public:
	// ISearchFile
	virtual std::string	INTCALLTYPE FindFile(const std::string& strFile)
	{
		return ::FindFile(strFile);
	}
};
static CFileSearcher theSearcher;
*/

//_____________________________________________________________________________
//
//	Functions	
//
//_____________________________________________________________________________

void NotifyHandler(void *param, NotifyInfo *info) 
{
	NVPROF_FUNC("CgFXMaterial::NotifyHandler");

	// Node can be added from an undo/redo etc.. At this time not all the node
	// data can be accessed, such as Object state and TM etc...So in this case we have to tell the 
	// lighting manager to reload the light list.	

	if (info->callParam)
	{
		INode * newNode = (INode*)info->callParam;
		switch(info->intcode)
		{
			case NOTIFY_SCENE_ADDED_NODE:
				DISPDBG(3, "NOTIFY_SCENE_ADDED_NODE");
				CSceneManager::GetSingletonPtr()->Notify(SCENEMGR_ADDNODE, (void*)newNode->GetHandle());
				break;
			case NOTIFY_SCENE_PRE_DELETED_NODE:
				DISPDBG(3, "NOTIFY_SCENE_PRE_DELETED_NODE");
				CSceneManager::GetSingletonPtr()->Notify(SCENEMGR_REMOVENODE, (void*)newNode->GetHandle());
				break;
			default:
				break;
		}
	}

	switch (info->intcode)
	{
		case NOTIFY_SCENE_POST_DELETED_NODE:
			CSceneManager::GetSingletonPtr()->Notify(SCENEMGR_REMOVENODE, NULL);
			break;
		case NOTIFY_FILE_PRE_OPEN:
			CSceneManager::GetSingletonPtr()->Notify(SCENEMGR_PRENEWSCENE, NULL);
			break;
	}

}

// We hook max messages to enable a posted message to our thread to get back to us.
// Used by the auto-fx file update mechanism (in vertexshader.cpp).
LRESULT CGetMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{

	NVPROF_FUNC("GetMsgProc");
    LPMSG lpMsg = (LPMSG) lParam;

    if ( nCode >= 0 && PM_REMOVE == wParam )
    {
        // Don't translate non-input events.
        if ( (lpMsg->message == MESSAGE_FILECHANGE) )
        {
            
            if (GetCOREInterface()->GetMAXHWnd() != NULL)
            {
                //if (::IsChild(GetCOREInterface()->GetMAXHWnd(),lpMsg->hwnd) ||
                //    (GetCOREInterface()->GetMAXHWnd() == lpMsg->hwnd))
                {
                    MaxVertexShader *pVS = (MaxVertexShader*) lpMsg->lParam;
            		pVS->SetReload(TRUE);
                    lpMsg->message = WM_NULL;
                    lpMsg->lParam  = 0;
                    lpMsg->wParam  = 0;
                    return true;
                }
            }
        }
    }

	try{
	   LRESULT l = CallNextHookEx(g_hHookMsg, nCode, wParam, lParam);
	   return l;
	}
	catch(...){
		MessageBox(0,"Caught HLSL plugin error, recovering...",0,0);
		return 0;
	}
} 

BOOL WINAPI CDllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved)
{
	char	Buff[256];
	DWORD	Version;

	g_hInstance = hinstDLL;				

	Version = Get3DSMAXVersion();

	if(GET_MAX_RELEASE(Version) < MAX_RELEASE)
	{
		sprintf(Buff,"Need Max version:%d\n",MAX_RELEASE);
		MessageBox(NULL,Buff,"Error",MB_OK | MB_ICONWARNING | MB_SETFOREGROUND | MB_APPLMODAL);
		return(false);
	} 


	if(!controlsInit) 
	{
		controlsInit = TRUE;
		InitCustomControls(g_hInstance);	
		InitCommonControls();			

	}

	if(fdwReason == DLL_PROCESS_ATTACH || fdwReason == DLL_THREAD_ATTACH)
	{	
		AVIFileInit();
		//AfxSetResourceHandle (hinstDLL);
		if (fdwReason == DLL_PROCESS_ATTACH)
		{
			CHECK_NVPROFILE_EVENTS();

			//g_hHookMsg = SetWindowsHookEx(WH_GETMESSAGE, (HOOKPROC)GetMsgProc, g_hInstance, (DWORD)GetCurrentThreadId());  

			RegisterNotification(NotifyHandler,NULL,NOTIFY_SCENE_ADDED_NODE);
			RegisterNotification(NotifyHandler,NULL,NOTIFY_SCENE_POST_DELETED_NODE);
			RegisterNotification(NotifyHandler,NULL,NOTIFY_SCENE_PRE_DELETED_NODE);
			RegisterNotification(NotifyHandler,NULL,NOTIFY_FILE_PRE_OPEN);

		}
	}
	else if(fdwReason == DLL_PROCESS_DETACH || fdwReason == DLL_THREAD_DETACH)
	{
		AVIFileExit();
		if (fdwReason == DLL_PROCESS_ATTACH)
		{
			//UnhookWindowsHookEx( g_hHookMsg );

			// Remove notifications.
			UnRegisterNotification(NotifyHandler,NULL,NOTIFY_SCENE_ADDED_NODE);
			UnRegisterNotification(NotifyHandler,NULL,NOTIFY_SCENE_POST_DELETED_NODE);
			UnRegisterNotification(NotifyHandler,NULL,NOTIFY_SCENE_PRE_DELETED_NODE);
			UnRegisterNotification(NotifyHandler,NULL,NOTIFY_FILE_PRE_OPEN);
		}
	}

	CoInitialize(NULL);


	return(true);
}


//_____________________________________
//
//	LibDescription 
//
//_____________________________________

__declspec( dllexport ) const TCHAR* LibDescription()
{
	return GetString(IDS_LIBDESCRIPTION);
}

//_____________________________________
//
//	LibNumberClasses 
//
//_____________________________________

__declspec( dllexport ) int LibNumberClasses()
{
	return 4;
}

//_____________________________________
//
//	LibClassDesc 
//
//_____________________________________
extern ClassDesc2* GetGlobalMenuDesc();
extern ClassDesc2* GetDummyTVNDesc();
extern ClassDesc2* GetSimpleCustAttribDesc();
extern ClassDesc2* GetNodeTracker();

__declspec( dllexport ) ClassDesc* LibClassDesc(int i)
{
	switch(i) 
	{
		case 0: return GetGlobalMenuDesc();
		case 1: return GetDefaultShaderDesc();
		case 2: return GetDummyTVNDesc();
		case 3: return GetSimpleCustAttribDesc();
		//case 4: return GetNodeTracker();
		default: return 0;
	}
}

//_____________________________________
//
//	LibVersion
//
//_____________________________________

__declspec( dllexport ) ULONG LibVersion()
{
	return VERSION_3DSMAX;
}

// Let the plug-in register itself for deferred loading
__declspec( dllexport ) ULONG CanAutoDefer()
{
	// Don't defer, we want menus etc immediately
    return 0;
}

