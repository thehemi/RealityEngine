#include "stdafx.h"

HWND sheetHwnd = NULL;

// Property pages
BOOL CALLBACK PrefabInterfaceDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK ExportDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK MapSettingsDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK EntityInterfaceDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK SphericalHarmonicsDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//--------------------------------------------------------------------------
// Property sheet must heed 3ds MAX notifications
//--------------------------------------------------------------------------
// Declare the callback function
static void NotificationMessage(void *param, NotifyInfo *info) {
	if(info->intcode == NOTIFY_PRE_PROGRESS)
		ShowWindow(sheetHwnd,SW_HIDE);
	if(info->intcode == NOTIFY_POST_PROGRESS)
		ShowWindow(sheetHwnd,SW_SHOW);
}


//--------------------------------------------------------------------------
// Default proc for modeless property page
//--------------------------------------------------------------------------
int CALLBACK PropSheetProc (HWND hwndDlg, UINT uMsg, LPARAM lParam) {
	switch(uMsg) {
  case PSCB_INITIALIZED:
	  //Property sheet must heed 3ds MAX notifications
	  SetText(hwndDlg,"Online Help",IDHELP);
	  RegisterNotification(NotificationMessage,NULL,NOTIFY_PRE_PROGRESS);
	  RegisterNotification(NotificationMessage,NULL,NOTIFY_POST_PROGRESS);
	  break;
  case PSCB_PRECREATE:
	  break;  
  default:
	  break;
	}

	return 0;
}

//--------------------------------------------------------------------------
// Default handling for all property sheet buttons (OK, CANCEL, HELP)
//--------------------------------------------------------------------------
BOOL HandlePropertySheet(LPARAM lParam, bool* bEnabled){
	LPNMHDR pnmh = (LPNMHDR)lParam;
	LPPSHNOTIFY pshntfy = (LPPSHNOTIFY)lParam;
	if(bEnabled)
		*bEnabled = false;

	switch(pnmh->code)
	{
	case PSN_HELP:
		ShellExecute(sheetHwnd,"open","http://artificialstudios.com/cgi-bin/twiki/bin/view/Main/PluginDocumentation",NULL,NULL,SW_SHOWNORMAL);
		SetWindowLong(sheetHwnd, DWL_MSGRESULT, PSNRET_NOERROR);
		break;
	// User clicked Cancel, OK, Apply
	case PSN_APPLY:
	case PSN_QUERYCANCEL:
		SetWindowLong(sheetHwnd, DWL_MSGRESULT, PSNRET_NOERROR);
		DestroyWindow(sheetHwnd);
		sheetHwnd = NULL;
		return FALSE;
	// User changed tab
	case PSN_SETACTIVE:
		if(bEnabled)
			*bEnabled = true;
		return TRUE;
	case PSN_KILLACTIVE:
		SetWindowLong(sheetHwnd, DWL_MSGRESULT, PSNRET_NOERROR) ;
		return FALSE;
	}
	return TRUE;
}

//--------------------------------------------------------------------------
// DoPropertySheet - creates a property sheet that
// contains two pages.
// hwndOwner - handle to the owner window of the
// property sheet.
//
//--------------------------------------------------------------------------
#define NUM_PAGES 5
VOID DoPropertySheet(HWND hwndOwner, int startPage)
{
	// Already property sheet running, just change its selection
	if(sheetHwnd != NULL){
		PropSheet_SetCurSel(sheetHwnd,startPage,startPage);
		return;
	}

	PROPSHEETPAGE psp[NUM_PAGES];
	// Set defaults for all pages
	for(int i=0;i<NUM_PAGES;i++){
		psp[i].dwSize = sizeof(PROPSHEETPAGE);
		psp[i].dwFlags = PSP_USETITLE | PSP_HASHELP ;
		psp[i].hInstance = g_hInstance;
		psp[i].pfnCallback = NULL;
		psp[i].lParam = 0;
	}

	
	psp[0].pszTemplate = MAKEINTRESOURCE(IDD_PREFABS);
	psp[0].pfnDlgProc = PrefabInterfaceDlgProc;
	psp[0].pszTitle = "Prefabs";

	psp[1].pszTemplate = MAKEINTRESOURCE(IDD_OBJECT_PROPERTIES);
	psp[1].pfnDlgProc = EntityInterfaceDlgProc;
	psp[1].pszTitle = "Entities";

	psp[2].pszTemplate = MAKEINTRESOURCE(IDD_MAPSETTINGS);
	psp[2].pfnDlgProc = MapSettingsDlgProc;
	psp[2].pszTitle = "Scene Settings";

	psp[3].pszTemplate = MAKEINTRESOURCE(IDD_EXPORT);
	psp[3].pfnDlgProc = ExportDlgProc;
	psp[3].pszTitle = "Exporting";

	psp[4].pszTemplate = MAKEINTRESOURCE(IDD_SH);
	psp[4].pfnDlgProc = SphericalHarmonicsDlgProc;
	psp[4].pszTitle = "Spherical Harmonics";

	
	PROPSHEETHEADER psh;
	psh.dwSize = sizeof(PROPSHEETHEADER);
	psh.dwFlags = PSH_PROPSHEETPAGE | PSH_MODELESS | PSH_HASHELP | PSH_NOAPPLYNOW | PSH_USECALLBACK; 
	psh.hwndParent = hwndOwner;
	psh.hInstance = g_hInstance;
	psh.pszCaption = (LPSTR) "Game Tools";
	psh.nPages = sizeof(psp) / sizeof(PROPSHEETPAGE);
	psh.nStartPage = startPage;
	psh.ppsp = (LPCPROPSHEETPAGE) &psp;
	psh.pfnCallback = PropSheetProc;
	sheetHwnd = (HWND)PropertySheet(&psh);
	return;
}


