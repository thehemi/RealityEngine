//=========== Copyright (c) 2004, Tim Johnson. All rights reserved. ===========
//
// Handles quad and main menus for Tools
// Also handles global initialization of Tool classes
//
//=============================================================================
#include "stdafx.h"
#include "GlobalMain.h"
#include "ExportInterface.h"
#include "EntityInterface.h"
#include "PrefabInterface.h"
#include "SphericalHarmonics.h"
#include "MapSettings.h"
#include "xrefutil.h"
#include "resource.h"
#include "globalsettings.h"

Class_ID GLOBAL_MENU_CLASS_ID = 	Class_ID(0x385721ab, 0x55b664b5);

GlobalMenu theGlobalMenu;	//Singleton Instance
#define CFGFILENAME "Reality Tools.ini"

VOID DoPropertySheet(HWND hwndOwner, int startPage);

// lets register a callback so that we get notified of a color change so that we can re-register the 
// menu
static void CreateGameMenu();
static void InterfaceChanged(void *param, NotifyInfo *info) {
	CreateGameMenu();
}


BOOL CALLBACK HelpDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HBRUSH hBackground;
	switch (msg) {
case WM_INITDIALOG:
	// Set up toolbar icons & tooltips
	SetDlgItemUrl(hWnd,IDC_LINK,"http://artificialstudios.com/cgi-bin/twiki/bin/view/Main/PluginDocumentation");
	SetDlgItemUrl(hWnd,IDC_EMAIL,"mailto:tim@artificialstudios.com");
	hBackground = CreateSolidBrush(RGB(128,128,128)); // Create a brush
	return TRUE;
case WM_DESTROY:
case WM_CLOSE:
	EndDialog(hWnd,0);
	break;
default:
	break;
	}
	return FALSE;
}	

class GlobalMenuClassDescType:public ClassDesc2 {
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return &theGlobalMenu; }
	const TCHAR *	ClassName() { return GetString(IDS_CLASS_NAME); }
	SClass_ID		SuperClassID() { return GUP_CLASS_ID; }
	Class_ID		ClassID() { return GLOBAL_MENU_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_CLASS_CATEGORY); }
	const TCHAR*	InternalName() { return _T("GameMenu"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return g_hInstance; }				// returns owning module handle
};


static GlobalMenuClassDescType GlobalMenuClassDesc;
ClassDesc2* GetGlobalMenuDesc() { return &GlobalMenuClassDesc; }



// Function ids
enum {
	// Are these reserved?
	fps_GetInstance, fps_SetActionsEnabled, fps_SetActionsVisible, fps_SetActionsChecked,
	fps_DoThing,
	// Our function ids
	fnIdGlobalAction1,fnIdGlobalAction2,fnIdGlobalAction3,fnIdGlobalAction4,fnIdGlobalAction5,fnIdGlobalAction6,
	fnIdGlobalAction7,fnIdGlobalAction8,fnIdGlobalAction9,fnIdGlobalAction10,fnIdGlobalAction11,
	fnIdTrue, fnIdIsConvertToPrefabEnabled, fnIdIsEditEnabled
};

// Function entrypoints
class GlobalMenuActionsIMP:public GlobalMenuActions{
public:
	DECLARE_DESCRIPTOR(GlobalMenuActionsIMP) //Needed for static interfaces

	BEGIN_FUNCTION_MAP
		FN_ACTION( fnIdGlobalAction1, GlobalAction1 );
		FN_ACTION( fnIdGlobalAction2, GlobalAction2 );
		FN_ACTION( fnIdGlobalAction3, GlobalAction3 );
		FN_ACTION( fnIdGlobalAction4, GlobalAction4 );
		FN_ACTION( fnIdGlobalAction5, GlobalAction5 );
		FN_ACTION( fnIdGlobalAction6, GlobalAction6 );
		FN_ACTION( fnIdGlobalAction7, GlobalAction7 );
		FN_ACTION( fnIdGlobalAction8, GlobalAction4 );
		FN_ACTION( fnIdGlobalAction9, GlobalAction9 );
		FN_ACTION( fnIdGlobalAction10, GlobalAction10 );
		FN_PRED( fnIdIsConvertToPrefabEnabled, fnIsConvertToPrefabEnabled );
		FN_PRED( fnIdIsEditEnabled, fnIsEditEnabled );
		FN_PRED( fnIdTrue, fnTrue );
	END_FUNCTION_MAP

		BOOL fnTrue(){ return true; }
		BOOL fnIsConvertToPrefabEnabled(){ return true; }
		BOOL fnIsEditEnabled(){ return theEntityInterface.NodeSelected(); }
		
		FPStatus GlobalAction1() { DoPropertySheet(GetCOREInterface()->GetMAXHWnd(), 3); return FPS_OK; }
		FPStatus GlobalAction2() { return FPS_OK; }
		// Add entity...
		FPStatus GlobalAction3() {
			theEntityInterface.LoadAllScripts(false);
			theEntityInterface.CreateObject(); 
			DoPropertySheet(GetCOREInterface()->GetMAXHWnd(), 1);
			return FPS_OK; 
		}
		// Entity Menu
		FPStatus GlobalAction4() { DoPropertySheet(GetCOREInterface()->GetMAXHWnd(), 1); return FPS_OK; }
		// Export Menu
		FPStatus GlobalAction5() { DoPropertySheet(GetCOREInterface()->GetMAXHWnd(), 3); return FPS_OK; }
		// World properties
		FPStatus GlobalAction6() { DoPropertySheet(GetCOREInterface()->GetMAXHWnd(), 2); return FPS_OK; } 
		// Prefab manager
		FPStatus GlobalAction7() { DoPropertySheet(GetCOREInterface()->GetMAXHWnd(), 0); return FPS_OK; } 
		// Spherical Harmonics manager
		FPStatus GlobalAction10() { DoPropertySheet(GetCOREInterface()->GetMAXHWnd(), 4); return FPS_OK; } 
		// Help
		FPStatus GlobalAction9() { 
			DialogBoxParam(g_hInstance, 
				MAKEINTRESOURCE(IDD_HELP), 
				GetCOREInterface()->GetMAXHWnd(), 
				HelpDlgProc, 
				NULL);
			return FPS_OK; 
		} 
};

#define MENU_NAME _T("Reality menu")

// Menu action table
static GlobalMenuActionsIMP globalmenuai( 
	GLOBAL_MENU_INTERFACE_ID, _T("Global_Interface"), 0, &GlobalMenuClassDesc, FP_ACTIONS,
	kActionMainUIContext,

	// Quad Menu
	fnIdGlobalAction1,	_T("Add prefab..."), IDS_ACTION1_DESC, 0,
	f_category, MENU_NAME, IDS_FPACTIONS_CATEGORY,
	f_menuText,	IDS_ACTION1_DESC,
	f_isEnabled, fnIdTrue,
	end,
	fnIdGlobalAction2,	_T("Convert to prefab..."), IDS_ACTION2_DESC, 0,
	f_category, MENU_NAME, IDS_FPACTIONS_CATEGORY,
	f_menuText,	IDS_ACTION2_DESC,
	f_isEnabled, fnIdIsConvertToPrefabEnabled,
	end,
	fnIdGlobalAction3,	_T("Add entity..."), IDS_ACTION3_DESC, 0,
	f_category, MENU_NAME, IDS_FPACTIONS_CATEGORY,
	f_menuText,	IDS_ACTION3_DESC,
	f_isEnabled, fnIdTrue,
	end,
	fnIdGlobalAction4,	_T("Edit properties..."), IDS_ACTION4_DESC, 0,
	f_category, MENU_NAME, IDS_FPACTIONS_CATEGORY,
	f_menuText,	IDS_ACTION4_DESC,
	f_isEnabled, fnIdIsEditEnabled,
	end,

	// Main Menu
	fnIdGlobalAction5,	_T("Exporting..."), IDS_ACTION5_DESC, 0,
	f_category, MENU_NAME, IDS_FPACTIONS_CATEGORY,
	f_menuText,	IDS_ACTION5_DESC,
	f_isEnabled, fnIdTrue,
	end,
	fnIdGlobalAction6,	_T("Scene Settings..."), IDS_ACTION6_DESC, 0,
	f_category, MENU_NAME, IDS_FPACTIONS_CATEGORY,
	f_menuText,	IDS_ACTION6_DESC,
	f_isEnabled, fnIdTrue,
	end,
	fnIdGlobalAction7,	_T("Prefabs..."), IDS_ACTION7_DESC, 0,
	f_category, MENU_NAME, IDS_FPACTIONS_CATEGORY,
	f_menuText,	IDS_ACTION7_DESC,
	f_isEnabled, fnIdTrue,
	end,
	fnIdGlobalAction8,	_T("Entities..."), IDS_ACTION8_DESC, 0,
	f_category, MENU_NAME, IDS_FPACTIONS_CATEGORY,
	f_menuText,	IDS_ACTION8_DESC,
	f_isEnabled, fnIdTrue,
	end,
	fnIdGlobalAction9,	_T("Help..."), IDS_ACTION9_DESC, 0,
	f_category, MENU_NAME, IDS_FPACTIONS_CATEGORY,
	f_menuText,	IDS_ACTION9_DESC,
	f_isEnabled, fnIdTrue,
	end,
	fnIdGlobalAction10,	_T("Spherical Harmonics..."), IDS_ACTION10_DESC, 0,
	f_category, MENU_NAME, IDS_FPACTIONS_CATEGORY,
	f_menuText,	IDS_ACTION10_DESC,
	f_isEnabled, fnIdTrue,
	end,
	end
	);


//--------------------------------------------------------------------------
// Desc: Helper
//--------------------------------------------------------------------------
IMenuItem* InsertMenuItem(int actionId, IMenu* menu){
	ActionTable* pGlobalActionTable = globalmenuai.action_table;
	ActionItem* pGlobalAction1 = pGlobalActionTable->GetAction( actionId );
	assert(pGlobalAction1);
	IMenuItem* pMenuItem1 = GetIMenuItem();
	pMenuItem1->SetActionItem(pGlobalAction1);
	menu->AddItem( pMenuItem1 );
	menu->SetEnabled(false);
	return pMenuItem1;
}

DWORD GlobalMenu::Start()
{
	// Load our global config
	string filename = GetCOREInterface()->GetDir(APP_PLUGCFG_DIR);
	filename += "\\";
	filename += CFGFILENAME;
	if(!FileExists(filename)){
		//
		// Create a default config if one doesn't exist
		//
		theConfig.Create(filename);
		theConfig.InsertRawLine("// This is a procedural file. Do not modify.");
		theConfig.InsertRawLine("[PrefabManager]");
		theConfig.InsertRawLine("RootFolder = ");
		theConfig.InsertRawLine("CurrentFile = 0");
		theConfig.InsertRawLine("CurrentFolder = 0");

		theConfig.InsertRawLine("[MapSettings] // These are overriden by settings saved in the map");
		theConfig.InsertRawLine("SkyWorld = ");
		theConfig.InsertRawLine("FogStart = 5000");
		theConfig.InsertRawLine("ClipPlane = 5000");
		theConfig.InsertRawLine("FogColor = RGBA 255 255 255 255");
		theConfig.InsertRawLine("MiniMap = ");
		theConfig.InsertRawLine("MiniMapScale = 0.00");
		theConfig.InsertRawLine("PRTOverride = false");
		theConfig.InsertRawLine("CubeMap = ");
		theConfig.InsertRawLine("Rays = 100");
		theConfig.InsertRawLine("Bounces = 100");
		theConfig.InsertRawLine("TextureSize = 100");
		theConfig.InsertRawLine("[Entity]");
		theConfig.InsertRawLine("ScriptsFolder = C:\\Helix Core\\scripts");

		theConfig.InsertRawLine("[Exporter]");
		theConfig.InsertRawLine("// Misc");
		theConfig.InsertRawLine("CompilerPath = C:\\Cry Havoc\\maps\\BuildTool.exe");
		theConfig.InsertRawLine("GamePath = C:\\Cry Havoc\\Helix Core.exe");
		theConfig.InsertRawLine("SaveSelected = false");
		theConfig.InsertRawLine("// Animation");
		theConfig.InsertRawLine("IsLooping = true");
		theConfig.InsertRawLine("AnimationFilename = shoot");
		theConfig.InsertRawLine("AnimationPath = C:\\Cry Havoc\\models");
		theConfig.InsertRawLine("// Model");
		theConfig.InsertRawLine("UncompiledModelPath = C:\\Cry Havoc\\models");
		theConfig.InsertRawLine("CompiledModelPath = C:\\Cry Havoc\\models");
		theConfig.InsertRawLine("ModelFilename = ball");
		theConfig.InsertRawLine("// Level");
		theConfig.InsertRawLine("UncompiledLevelPath = C:\\Cry Havoc\\maps");
		theConfig.InsertRawLine("CompiledLevelPath = C:\\Cry Havoc\\maps");
		theConfig.InsertRawLine("LevelFilename = Test");
		theConfig.InsertRawLine("SaveBinary = true");
	}

	theConfig.Load(filename);

	// Load all the plugin classes
	if(!theGlobalSettings)
		theGlobalSettings = new GlobalSettings();
	theExport.Init();
	theEntityInterface.Init();
	theMap.Init();
	thePrefab.Init();
	theXrefutil.Init();
	theSH.Init();

	GetCOREInterface()->GetActionManager()->RegisterActionContext(kMenuUtilContext,GetString(IDS_CLASS_CATEGORY));
	// Register the callback
	RegisterNotification(InterfaceChanged,this, NOTIFY_COLOR_CHANGE);
	// When done, unregister the callback
	CreateGameMenu();	

	//RegisterNotification(CheckMessages,NULL, NOTIFY_SYSTEM_STARTUP);
	return GUPRESULT_KEEP;
}

void GlobalMenu::Stop()
{
	UnRegisterNotification(InterfaceChanged,this,NOTIFY_COLOR_CHANGE);
}


//--------------------------------------------------------------------------
// Desc: Set up our right-click menu and main menu
//--------------------------------------------------------------------------
void CreateGameMenu()
{
	MenuContextId kMyMenuContextId=0x4e79536c;  //<random number>
	IMenuManager* pMenuMan = GetCOREInterface()->GetMenuManager();
	//IQuadMenu* mainQuad = menuMan->GetViewportRightClickMenu( (IQuadMenuContext::RightClickContext)kMenuQuadMenu );
	IQuadMenu* mainQuad = pMenuMan->FindQuadMenu("Default Viewport Quad");
	IMenu* quadMenu = mainQuad->GetMenu(0);

	bool newlyRegistered = pMenuMan->RegisterMenuBarContext(kMyMenuContextId, "Reality Context");
	if(newlyRegistered) {
		// 
		// Register the Caffeine QUAD MENU
		//
		//InsertMenuItem(fnIdGlobalAction1, quadMenu);
		//InsertMenuItem(fnIdGlobalAction2, quadMenu);
		InsertMenuItem(fnIdGlobalAction3, quadMenu);
		InsertMenuItem(fnIdGlobalAction4, quadMenu);

		//
		// Register MAIN MENU
		// 
		IMenu *pMainMenu = pMenuMan->GetMainMenuBar();
		// Create + register menu
		IMenu* pCustomMenu = GetIMenu();
		pCustomMenu->SetTitle("Reality");
		pMenuMan->RegisterMenu(pCustomMenu, 0);
		// Populate
		InsertMenuItem(fnIdGlobalAction5, pCustomMenu);
		InsertMenuItem(fnIdGlobalAction6, pCustomMenu);
		InsertMenuItem(fnIdGlobalAction7, pCustomMenu);
		InsertMenuItem(fnIdGlobalAction8, pCustomMenu);
		InsertMenuItem(fnIdGlobalAction10, pCustomMenu);
		InsertMenuItem(fnIdGlobalAction9, pCustomMenu);
		// Add to main menu
		IMenuItem* pSubMenuItem1 = GetIMenuItem();
		pSubMenuItem1->SetSubMenu(pCustomMenu);
		pMainMenu->AddItem(pSubMenuItem1);

		pMenuMan->UpdateMenuBar();
	}
}

DWORD GlobalMenu::Control(DWORD parameter)
{
	return GUPRESULT_KEEP;
}

FPInterfaceDesc* GlobalMenu::GetDesc() { return &globalmenuai; }