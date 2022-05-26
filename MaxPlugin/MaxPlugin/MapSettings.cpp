//=========== Copyright (c) 2004, Tim Johnson. All rights reserved. ===========
//
// Map Configuration Tab Window
//
//=============================================================================
#include "stdafx.h"
#include "MapSettings.h"
#include "hsv.h"
#include "globalsettings.h"
#include "PRTOptionsDlg.h"
MapSettings theMap;


//----------------------------------------------------------------------------------
// Gets properties, or initializes if first run
//----------------------------------------------------------------------------------
SceneProperties* MapSettings::GetSceneProperties(){
	if(!theGlobalSettings->InitGlobalSettings() || !sceneData.bInitialized){
		sceneData.bInitialized = true;
		// Settings are blank, so read in from config
		strcpy(sceneData.skyworld, theConfig.GetString("SkyWorld").c_str());
		strcpy(sceneData.miniMap, theConfig.GetString("MiniMap").c_str());
		strcpy(sceneData.skyworld, theConfig.GetString("SkyWorld").c_str());
		strcpy(sceneData.mapfile, theConfig.GetString("LevelFilename").c_str());
		strcpy(sceneData.modelfile, theConfig.GetString("ModelFilename").c_str());
		strcpy(sceneData.cubeMap, theConfig.GetString("CubeMap").c_str());

		sceneData.fogColor		= theConfig.GetColor("FogColor");
		sceneData.miniMapScale	= theConfig.GetFloat("MiniMapScale");
		sceneData.fogStart		= theConfig.GetInt("FogStart");
		sceneData.clipPlane		= theConfig.GetInt("ClipPlane");
		if(sceneData.clipPlane < 50) // Should never happen, but had a few probs where it did
			sceneData.clipPlane = 50;

		sceneData.bPRTOverride		= theConfig.GetBool("PRTOverride");
		sceneData.dwRays			= theConfig.GetInt("Rays");
		sceneData.dwBounces			= theConfig.GetInt("Bounces");
		sceneData.dwTextureSize		= theConfig.GetInt("TextureSize");
	}

	return &sceneData;
}

//----------------------------------------------------------------------------------
// Puts the map settings in the global structure, which is serialized on file save/load
//----------------------------------------------------------------------------------
void MapSettings::CommitSettings(){
	if(!hwnd)
		return;

	SceneProperties* props = GetSceneProperties();

	props->fogColor			= fogColor->GetColor();
	props->fogStart			= SendMessage(GetDlgItem(hwnd,IDC_FOGSTART),  TBM_GETPOS, NULL, NULL);
	props->clipPlane		= SendMessage(GetDlgItem(hwnd,IDC_CLIPPLANE), TBM_GETPOS, NULL, NULL);
	if(props->clipPlane < 50) // Should never happen, but had a few probs where it did
		props->clipPlane = 50;
	strcpy(props->miniMap,GetText(hwnd,IDC_MINIMAP).c_str());
	props->miniMapScale  = atof(GetText(hwnd,IDC_MINIMAP_SCALE).c_str());
	strcpy(props->skyworld,GetText(hwnd,IDC_SKYBOX).c_str());
	strcpy(props->cubeMap,GetText(hwnd,IDC_SH_CUBEMAP).c_str());

	props->bPRTOverride = Button_GetCheck(GetDlgItem(hwnd,IDC_GLOBALOVERRIDE));
	props->dwBounces = (DWORD) SendMessage( GetDlgItem( hwnd, IDC_PER_BOUNCES_SPIN ), UDM_GETPOS, 0, 0 );
	props->dwRays = (DWORD) SendMessage( GetDlgItem( hwnd, IDC_PER_RAYS_SPIN ), UDM_GETPOS, 0, 0 );

	int nIndex = (int) SendMessage( GetDlgItem( hwnd, IDC_TEXTURE_PERCENT_COMBO ), CB_GETCURSEL, 0, 0 );
	LRESULT lResult = SendMessage( GetDlgItem( hwnd, IDC_TEXTURE_PERCENT_COMBO ), CB_GETITEMDATA, nIndex, 0 );
	props->dwTextureSize = (DWORD) lResult;

	// Update config
	theConfig.SetString("SkyWorld",props->skyworld);
	theConfig.SetInt("FogStart",props->fogStart);
	theConfig.SetInt("ClipPlane",props->clipPlane);
	theConfig.SetColor("FogColor",props->fogColor);
	theConfig.SetString("MiniMap",props->miniMap);
	theConfig.SetFloat("MiniMapScale",props->miniMapScale);
	theConfig.SetString("CubeMap",props->cubeMap);

	theConfig.SetBool("PRTOverride",props->bPRTOverride);
	theConfig.SetInt("Rays",props->dwRays);
	theConfig.SetInt("Bounces",props->dwBounces);
	theConfig.SetInt("TextureSize",props->dwTextureSize);
}

//----------------------------------------------------------------------------------
// Takes settings out of structure and into GUI
//----------------------------------------------------------------------------------
void MapSettings::ApplySettings(){
	if(!hwnd)
		return;

	SceneProperties* props = GetSceneProperties();

	fogColor->SetColor(props->fogColor);
	SendMessage(GetDlgItem(hwnd,IDC_FOGSTART), TBM_SETPOS, TRUE, props->fogStart);
	SendMessage(GetDlgItem(hwnd,IDC_CLIPPLANE),TBM_SETPOS, TRUE, props->clipPlane);
	SetText(hwnd,props->skyworld,IDC_SKYBOX);
	SetText(hwnd,props->miniMap,IDC_MINIMAP);
	SetText(hwnd,props->cubeMap,IDC_SH_CUBEMAP);
	SetText(hwnd,ToStr(props->miniMapScale),IDC_MINIMAP_SCALE);

	HWND hNumBouncesSpin = GetDlgItem( hwnd, IDC_PER_BOUNCES_SPIN );
	SendMessage( hNumBouncesSpin, UDM_SETRANGE, 0, (LPARAM) MAKELONG( 100, 1 ) );
	SendMessage( hNumBouncesSpin, UDM_SETPOS, 0, (LPARAM) MAKELONG( props->dwBounces, 0) );  

	HWND hNumRaysSpin = GetDlgItem( hwnd, IDC_PER_RAYS_SPIN );
	SendMessage( hNumRaysSpin, UDM_SETRANGE, 0, (LPARAM) MAKELONG( 100, 1 ) );
	SendMessage( hNumRaysSpin, UDM_SETPOS, 0, (LPARAM) MAKELONG( props->dwRays, 0) );  

	Button_SetCheck(GetDlgItem(hwnd,IDC_GLOBALOVERRIDE),props->bPRTOverride);

	// Set percentage values in texture combo
	SendMessage(GetDlgItem(hwnd,IDC_TEXTURE_PERCENT_COMBO),CB_RESETCONTENT,0,0);
	HWND hTexSizeCombo = GetDlgItem( hwnd, IDC_TEXTURE_PERCENT_COMBO );
	int nIndex = 0;
	int nSelection = -1;
	for( int i=1; i<11; i++ )
	{                
		DWORD nSize = (DWORD)powf(2.0f,(float)i);
		char buf[256];
		sprintf(buf,"%d%%",i*10);
		nIndex = (int) SendMessage( hTexSizeCombo, CB_ADDSTRING, 0, (LPARAM) buf );                
		if( nIndex >= 0 ) 
			SendMessage( hTexSizeCombo, CB_SETITEMDATA, nIndex, (LPARAM) i*10 );
	}
	SendMessage( hTexSizeCombo, CB_SETCURSEL, props->dwTextureSize/10 -1, 0 );
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
BOOL CALLBACK MapSettingsDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	BOOL ret;
	switch (msg) {
		case WM_INITDIALOG:
			{
			theMap.hwnd = hWnd;
			theMap.fogColor = GetIColorSwatch(GetDlgItem(hWnd, IDC_COLOR_SWATCH),  RGB(255,255,255), _T("New Wireframe Color"));
			SendMessage(GetDlgItem(hWnd,IDC_FOGSTART),TBM_SETRANGE,TRUE,MAKELONG(50, 5000));
			SendMessage(GetDlgItem(hWnd,IDC_CLIPPLANE),TBM_SETRANGE,TRUE,MAKELONG(50, 5000));
			CenterWindow(hWnd,GetParent(hWnd));
			theMap.ApplySettings();
			return TRUE;
			}
		case WM_DESTROY: 
		case WM_CLOSE:
			theMap.CloseDialog();
			break;
		case WM_NOTIFY:
			ret = HandlePropertySheet(lParam);
			if(ret == 0  && theMap.hwnd){
				// Window is changing (but maybe not closing), so let's store the settings
				theMap.CommitSettings();
			}
			else{
				SetText(hWnd,ToStr(SendMessage(GetDlgItem(hWnd,IDC_FOGSTART),  TBM_GETPOS, NULL, NULL)),IDC_FOGVALUE);
				SetText(hWnd,ToStr(SendMessage(GetDlgItem(hWnd,IDC_CLIPPLANE), TBM_GETPOS, NULL, NULL)),IDC_CLIPVALUE);
			}
			return ret;

		case WM_COMMAND:
			if(HandleFocus(wParam))
				return TRUE;

			switch (LOWORD(wParam)) {
				case IDC_EDIT_DEFAULTS:
				{
					
					ShellExecuteW(hWnd, L"open", L"NOTEPAD.EXE", GetGlobalOptionsFile().m_strFile,L"",1);
					break;
				}
				case IDC_RELOAD_DEFAULTS:
				{
					GetGlobalOptionsFile().LoadOptions(GetGlobalOptionsFile().m_strFile);
					break;
				}
				case IDC_STRIPSCENE:
				{
					theGlobalSettings->ClearGlobalSettings();
					break;
				}
				case IDC_BROWSE_SKYBOX:
					SetText(hWnd,BrowseForFile(hWnd,"Locate sky world...","Map Files\0*.mpc\0"),IDC_SKYBOX);
					break;
				case IDC_BROWSE_CUBEMAP:
					{
					string str = BrowseForFile(hWnd,"Locate .HDR image probe or .DDS cubemap","HDR Vertical-Cross (*.hdr)\0*.hdr\0DDS Cube Maps (*.dds)\0*.dds\0All Files (*.*)\0*.*\0\0");
					SetText(hWnd,str,IDC_SH_CUBEMAP);
					break;
					}
			}
		default:
			break;
	}
	return FALSE;
}	

MapSettings::MapSettings()
{
	ip = NULL;	
	hwnd = NULL;
}

MapSettings::~MapSettings()
{
}

void MapSettings::Init(){
	ip = GetCOREInterface();
}


void MapSettings::CloseDialog() 
{
	if(!hwnd) // We've already closed once
		return;

//	EndDialog(hwnd,0);
	hwnd = NULL;
}