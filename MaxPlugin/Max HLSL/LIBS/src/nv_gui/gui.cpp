/*********************************************************************NVMH4****
Path:  NVSDK\Common\src\nv_gui
File:  gui.cpp

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


Main nv_gui file, responsible for creation of INVGUI panels...


******************************************************************************/

// mvgui.cpp : Defines the initialization routines for the DLL.
//
//
//	Note!
//
//		If this DLL is dynamically linked against the MFC
//		DLLs, any functions exported from this DLL which
//		call into MFC must have the AFX_MANAGE_STATE macro
//		added at the very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

#include "stdafx.h"
#include "nv_gui\gui.h"
#include "nv_gui\dynamicdlg.h"
#include "nv_gui\nvguidata.h"
#include "CGFXDocument.h"
#include "CGFXmainframe.h"
#include "CGFXview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace nv_gui;
using namespace nv_sys;

//! The nv_gui namespace contains useful external GUI dialogs, which can be created and used independant of the main app
/*! nv_gui is designed to make it easy to use MFC in apps that may not have easy access to it.  Since the access to MFC
is entirely contained inside the nv_gui plugin, the client app doesn't have to link to it.
*/
namespace nv_gui
{
	CNVGUIApp theApp;
    HHOOK hHookMsg;
}
LRESULT FAR PASCAL GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam);

/////////////////////////////////////////////////////////////////////////////
// CNVGUIApp

BEGIN_MESSAGE_MAP(CNVGUIApp, CWinApp)
	//{{AFX_MSG_MAP(CNVGUIApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////
// CNVGUIApp construction

CNVGUIApp::CNVGUIApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CNVGUIApp object



void CNVGUIApp::AddDialog(CDialog* pDlg)
{
	m_setDlg.insert(pDlg);
}

void CNVGUIApp::RemoveDialog(CDialog* pDlg)
{
	m_setDlg.erase(m_setDlg.find(pDlg));
}

void CNVGUIApp::UpdateDialogs()
{
	try{
		std::set<CDialog*>::iterator itrDlgs = m_setDlg.begin();
		while (itrDlgs != m_setDlg.end())
		{
			if ((*itrDlgs)->m_hWnd)
			{
				(*itrDlgs)->SendMessageToDescendants(WM_IDLEUPDATECMDUI, TRUE, 0);
			}
			itrDlgs++;
		}
	}catch(...){
		MessageBox(0,"Error in UpdateDialogs",0,0);
	}
}

BOOL WINAPI CDllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved);

extern HHOOK       g_hHookMsg;

BOOL CNVGUIApp::InitInstance() 
{
	// TIM: Init dll inside initinstance
	
    ilInit();
    m_AppName = "<default CgFX GUI>";

   	// Register document templates
	m_pDocTemplate = new CSingleDocTemplate(
		IDR_CGFXMAINFRAME,
		RUNTIME_CLASS(CCGFXDocument),
		RUNTIME_CLASS(CCGFXMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CCGFXView));

    AddDocTemplate(m_pDocTemplate);
    
    hHookMsg = SetWindowsHookEx(WH_GETMESSAGE, (HOOKPROC)GetMsgProc, m_hInstance, (DWORD)GetCurrentThreadId());  
	// TIM: Add hook
	g_hHookMsg = hHookMsg;

	CDllMain(m_hInstance,DLL_PROCESS_ATTACH,0);

    ReadInitialPathRegKey();
	
	return CWinApp::InitInstance();
}

int CNVGUIApp::ExitInstance() 
{
	CDllMain(m_hInstance,DLL_PROCESS_DETACH,0);
    ilShutDown();
    UnhookWindowsHookEx( hHookMsg );
	return CWinApp::ExitInstance();
}

void CNVGUIApp::SetAppName(const char * pAppName)
{
    m_AppName = pAppName;
    ReadInitialPathRegKey();
}

const char * CNVGUIApp::GetAppName()
{
    return m_AppName.c_str();
}

const char * CNVGUIApp::GetInitialPath(const char * ext)
{
    std::map<std::string, std::string>::iterator it = m_PathExtMap.find(ext);
    if (it != m_PathExtMap.end())
        return it->second.c_str();
    return NULL;
}

bool CNVGUIApp::SetInitialPath(const char * ext, const char * path)
{
    std::map<std::string, std::string>::iterator it = m_PathExtMap.find(ext);
    if (it != m_PathExtMap.end())
    {
        it->second = path;
    }
    else
    {
        m_PathExtMap.insert(std::map<std::string, std::string>::value_type(ext,path));
        // make the setting persistent
        WriteInitialPathRegKey();
    }
    return true;
}

bool WriteRegString(HKEY root, LPCSTR lpszSection, LPCSTR lpszEntry, LPCSTR lpszValue)
{
	HKEY hSectionKey;
	long Err;
    DWORD disposition;
    std::string subkey;

    RegCreateKeyEx(root, lpszSection, 0, "", REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hSectionKey, &disposition);

	if(hSectionKey)
    {
		Err = RegSetValueEx(hSectionKey, lpszEntry, 0, REG_SZ, (const unsigned char *)lpszValue, strlen(lpszValue));
		RegCloseKey(hSectionKey);
		return (Err == ERROR_SUCCESS);
	}
	return false;
}

bool GetRegString(HKEY root, LPCSTR lpszSection, LPCSTR lpszEntry, std::string *value)
{
	HKEY hSectionKey;
	long Err;
	DWORD Type = REG_SZ;
    DWORD Size = 0;
    DWORD disposition;
    std::string subkey;
    char *data;
    
    RegCreateKeyEx(root, lpszSection, 0, "", REG_OPTION_NON_VOLATILE, KEY_READ, NULL, &hSectionKey, &disposition);

    if(hSectionKey) 
    {
		RegQueryValueEx(hSectionKey, lpszEntry, 0, &Type, NULL, &Size);
        data = new char[Size];
        Err = RegQueryValueEx(hSectionKey, lpszEntry, 0, &Type, (unsigned char *)data, &Size);
		RegCloseKey(hSectionKey);
        *value = data;
        delete [] data;
        data = 0;
        return ((Err == ERROR_SUCCESS) && (Type == REG_SZ));
    }
	return false;
}

bool CNVGUIApp::WriteInitialPathRegKey()
{
    std::string         RegKey;
    RegKey = "Software\\NVIDIA Corporation\\" + m_AppName;

    std::string initial_paths;
    std::map<std::string, std::string>::iterator it = m_PathExtMap.begin();
    while (it != m_PathExtMap.end())
    {
        initial_paths += it->first;
        initial_paths += '|';
        initial_paths += it->second;
        initial_paths += '|';
        ++it;
    }
    initial_paths += '|';
    return WriteRegString(HKEY_CURRENT_USER, RegKey.c_str(), "DefaultPaths", initial_paths.c_str());
}

bool CNVGUIApp::ReadInitialPathRegKey()
{
    std::string         RegKey;
    RegKey = "Software\\NVIDIA Corporation\\" + m_AppName;

    std::string buf;
    if (GetRegString(HKEY_CURRENT_USER, RegKey.c_str(), "DefaultPaths", &buf) == false)
        return false;
    CString initial_paths = buf.c_str();

    m_PathExtMap.clear();
    int idx = initial_paths.Find('|');
    while (idx > 0)
    {
        CString ext = initial_paths;
        ext.SetAt(idx,'\0');

        initial_paths.Delete(0,idx + 1);
        CString path = initial_paths;
        idx = initial_paths.Find('|');
        path.SetAt(idx,'\0');
        initial_paths.Delete(0,idx + 1);
        idx = initial_paths.Find('|');
        SetInitialPath(ext,path);
    }
    return true;
}

bool CNVGUIApp::WriteWindowPosRegKey(const char * KeyName, HWND hwnd)
{
    std::string         RegKey;
    HKEY                hKey; 
    WINDOWPLACEMENT     wndpos;
    int                 bVisible;

    ::GetWindowPlacement(hwnd,&wndpos);
    bVisible = ::IsWindowVisible(hwnd);
    // Force the showCmd since it doesn't seem to be doing the right thing
    wndpos.showCmd = bVisible ? SW_SHOW : SW_HIDE;
    
    RegKey = "Software\\NVIDIA Corporation\\" + m_AppName;

    if (RegCreateKey(HKEY_CURRENT_USER, RegKey.c_str(), &hKey)) 
    {
        TRACE0("Could not create the registry key."); 
        return false;
    }

	RegKey = KeyName;
    RegKey += "WNDPLT";

    if (RegSetValueEx(hKey,         // subkey handle 
        RegKey.c_str(),				// value name 
        0,                          // must be zero 
        REG_BINARY,                 // value type 
        (LPBYTE) &wndpos,           // pointer to value data 
        sizeof(WINDOWPLACEMENT)))   // length of value data 
    {
        TRACE0("Could not create the WindowPlacement registry key."); 
        return false;
    }

    RegCloseKey(hKey);
    return true;
}

bool CNVGUIApp::ReadWindowPosRegKey(const char * KeyName, HWND hwnd)
{
    std::string         RegKey;
    HKEY                hKey; 
    WINDOWPLACEMENT     wndpos;
    unsigned long       dwLenBuf = sizeof(WINDOWPLACEMENT);
    unsigned long       RegType = REG_BINARY;

    RegKey = "Software\\NVIDIA Corporation\\" + m_AppName;
    
    if (RegOpenKeyEx( HKEY_CURRENT_USER,
               RegKey.c_str(),
               0, KEY_QUERY_VALUE, &hKey ) != ERROR_SUCCESS)
    {
        return false;
    }
           
    RegKey = KeyName;
    RegKey += "WNDPLT";

    if (RegQueryValueEx( hKey, 
        RegKey.c_str(), 
        NULL, 
        &RegType,
        (LPBYTE)&wndpos, 
        &dwLenBuf))
    {
        TRACE0("Could not read the WindowPlacement registry key."); 
        return false;
    }
    
    if (dwLenBuf != sizeof(WINDOWPLACEMENT) || RegType != REG_BINARY)
        return false;

    RegCloseKey(hKey);

	// Make sure we can see the window
	HWND hDesk = GetDesktopWindow();
	if (hDesk)
	{
		CRect rcDesk;
		GetClientRect(hDesk, &rcDesk);

		int WindowWidth = wndpos.rcNormalPosition.right - wndpos.rcNormalPosition.left;
		int WindowHeight = wndpos.rcNormalPosition.bottom - wndpos.rcNormalPosition.top;

		if (wndpos.ptMinPosition.x > rcDesk.Width())
			wndpos.ptMinPosition.x = 0;
		if (wndpos.ptMinPosition.y > rcDesk.Height())
			wndpos.ptMinPosition.y = 0;
		
		if (wndpos.ptMaxPosition.x > rcDesk.Width())
			wndpos.ptMaxPosition.x = 0;
		if (wndpos.ptMaxPosition.y > rcDesk.Height())
			wndpos.ptMaxPosition.y = 0;
		
		if (wndpos.rcNormalPosition.left > rcDesk.right)
		{
			wndpos.rcNormalPosition.left = 0;
			wndpos.rcNormalPosition.right = WindowWidth;
		}

		if (wndpos.rcNormalPosition.top < rcDesk.top)
		{
			wndpos.rcNormalPosition.top = 0;
			wndpos.rcNormalPosition.bottom = WindowHeight;
		}
	}

    
    ::SetWindowPlacement(hwnd,&wndpos);
    return true;
}

LRESULT CGetMsgProc(int nCode, WPARAM wParam, LPARAM lParam);

LRESULT FAR PASCAL GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{

    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    LPMSG lpMsg = (LPMSG) lParam;

	theApp.UpdateDialogs();
	
    if ( nCode >= 0 && PM_REMOVE == wParam )
    {
	
        // Don't translate non-input events.
        if ( (lpMsg->message >= WM_KEYFIRST && lpMsg->message <= WM_KEYLAST))
        {
            if (theApp.GetMainWnd() != NULL)
            {
                if (theApp.GetMainWnd()->IsChild(CWnd::FromHandle(lpMsg->hwnd)) ||
                    theApp.GetMainWnd() == CWnd::FromHandle(lpMsg->hwnd))
                {
                    //theApp.GetMainWnd()->SendMessageToDescendants(lpMsg->message,lpMsg->wParam,lpMsg->lParam);
                    //if (theApp.GetMainWnd()->PreTranslateMessage(lpMsg))
                    HACCEL hAccel = ((CFrameWnd*)theApp.GetMainWnd())->m_hAccelTable;
                	return hAccel != NULL &&  ::TranslateAccelerator(theApp.GetMainWnd()->GetSafeHwnd(), hAccel, lpMsg);

                    {
                        lpMsg->message = WM_NULL;
                        lpMsg->lParam  = 0;
                        lpMsg->wParam  = 0;
                        return true;
                    }
                }
            }
        }
		
    }

	// TIM: New Proc
    return /*CGetMsgProc(nCode,wParam,lParam);*/CallNextHookEx(hHookMsg, nCode, wParam, lParam);
} 

