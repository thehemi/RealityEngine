//=========== Copyright (c) 2004, Tim Johnson. All rights reserved. ===========
//
// Interface to assign spherical harmonics data to meshes or mesh groups
//
//=============================================================================
#include "stdafx.h"
#include "SphericalHarmonics.h"
#include "GlobalMain.h"
#include "GlobalSettings.h"
#include "resource.h"
#include "PRTOptionsDlg.h"
#include "EntityInterface.h"

SphericalHarmonics theSH;
CPRTOptionsDlg dlg;

bool AllNodesIdentical(vector<INode*>& nodes);

//----------------------------------------------------------------------------------
// Desc: Write states to global config
//----------------------------------------------------------------------------------
void SphericalHarmonics::WriteConfig()
{

}

//----------------------------------------------------------------------------------
// 
//----------------------------------------------------------------------------------
void SphericalHarmonics::EnableCustomUI(bool bEnable)
{
	Button_SetCheck(GetDlgItem(hwnd,IDC_USECUSTOM),bEnable);
	EnableWindow(GetDlgItem(hwnd,IDC_ORDER_SLIDER),bEnable);
	EnableWindow(GetDlgItem(hwnd,IDC_NUM_BOUNCES_SPIN),bEnable);
	EnableWindow(GetDlgItem(hwnd,IDC_NUM_RAYS_SPIN),bEnable);
	EnableWindow(GetDlgItem(hwnd,IDC_ADAPTIVE_CHECK),bEnable);
	EnableWindow(GetDlgItem(hwnd,IDC_NUM_RAYS_EDIT),bEnable);
	EnableWindow(GetDlgItem(hwnd,IDC_NUM_BOUNCES_EDIT),bEnable);
	EnableWindow(GetDlgItem(hwnd,IDC_TEXTURE_SIZE_COMBO),bEnable);
	EnableWindow(GetDlgItem(hwnd,IDC_PERPIXEL),bEnable);
	EnableWindow(GetDlgItem(hwnd,IDC_ADAPTIVE_SETTINGS),bEnable);
}

//----------------------------------------------------------------------------------
// Desc: Gets SH "custom" from global if not using custom
//----------------------------------------------------------------------------------
void SphericalHarmonics::UpdateGlobalProperties(NodeData& data)
{
	// If using global settings, fetch them now
	if(!data.bUseCustom)
	{
		SIMULATOR_OPTIONS sh = data.shOptions;
		// Get globals
		data.shOptions = GetGlobalOptions();

		// Restore node's non-overriden material options
		data.shOptions.Diffuse					= sh.Diffuse;
		data.shOptions.Absoption				= sh.Absoption;
		data.shOptions.ReducedScattering		= sh.ReducedScattering;
		data.shOptions.fRelativeIndexOfRefraction = sh.fRelativeIndexOfRefraction;
		data.shOptions.dwPredefinedMatIndex	= sh.dwPredefinedMatIndex;
		data.shOptions.bSubsurfaceScattering	= sh.bSubsurfaceScattering;
		data.shOptions.fLengthScale			= sh.fLengthScale;
	}
}


//----------------------------------------------------------------------------------
// Desc: Lets user pick a node
//----------------------------------------------------------------------------------
void SphericalHarmonics::EditBlockers(bool indoors){
	bool ok;
	if(indoors)
		ok=ip->DoExclusionListDialog(&inBlockers);
	else
		ok=ip->DoExclusionListDialog(&outBlockers);
}


//----------------------------------------------------------------------------------
// Saves node settings to NodeData
// *ALL* Saving should happen here
//----------------------------------------------------------------------------------
void SphericalHarmonics::SaveNodeSettings()
{
	for(int i=0;i<curNodes.size();i++)
	{
		// FIXME: Should be holding on to old data not doing this. (Why????)
		GetNodeData(curNodes[i],curData[i]);

		NodeData data = curData[i];
		NodeData oldData = data, test;

		
		// Only save data or query UI if window is open!
		if(!m_bWindowClosed && m_bRefreshedGUI){
			data.bSHEnabled = Button_GetCheck(GetDlgItem(hwnd,IDC_ENABLESH));
			data.bUseCustom = Button_GetCheck(GetDlgItem(hwnd,IDC_USECUSTOM));

			// NOTE: If multi-selection and the user didn't enable SH, don't touch the fucking nodes!!!!!
			// Otherwise an accidental selection would erase everything!!!!
			if(curNodes.size() > 1 && !data.bSHEnabled)
				return;

			// IMPORTANT: Only save this data if enabled, otherwise we may write data
			// to innocent nodes that were accidentally clicked
			if(data.bSHEnabled){
				// Set receiver group here too, to be safe
				if(data.receiverGroup.length() == 0)
					data.receiverGroup = curNodes[i]->GetName();
	 
				// Save blockers
				data.inBlockers.clear();
				for(int i=0;i<inBlockers.Count();i++)
					data.inBlockers.push_back(inBlockers[i]->GetName());
				data.outBlockers.clear();
				for(int i=0;i<outBlockers.Count();i++)
					data.outBlockers.push_back(outBlockers[i]->GetName());
				
				// Get custom settings from UI, then restore old settings to the global options file
				// so that it's never modified
				SIMULATOR_OPTIONS oldData = GetGlobalOptions();
				dlg.GetSettings();
				data.shOptions = GetGlobalOptions();
				GetGlobalOptionsFile().m_Options = oldData;

				// Override any of our custom settings with globals where applicable
				UpdateGlobalProperties(data);
			}

			if(data.bSHEnabled && data.CompareSH(oldData)){
				// Something has changed! Update date stamps
				//GetSystemTime(&data.timeMoved);
				GetSystemTime(&data.timeModified);
	#ifdef _DEBUG
				//Beep(5000,200);
	#endif
			}

			// Update our local copy, in case this functiong gets called again
			curData[i] = data;
		}
		
		SetNodeData(curNodes[i],data);
	} 
}




//--------------------------------------------------------------------------
// Desc: Handles dialog messages
//--------------------------------------------------------------------------
BOOL CALLBACK SphericalHarmonicsDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam){
	BOOL ret = dlg.DlgProc(hWnd,msg,wParam,lParam);
	BOOL ret2;
	IPoint2 mousePos;
	switch (msg) {
		case WM_INITDIALOG:
			{
				theSH.hwnd = hWnd;
				theSH.ProcessSelectedNodes();
			}
			break;
		case WM_DESTROY:
		case WM_CLOSE:
			theSH.SaveNodeSettings();
			theSH.m_bWindowClosed = true;
			theSH.m_bRefreshedGUI = false;
            g_OptionsFile.SaveOptions();
			theSH.CloseDialog();
			
			break;
		case WM_NOTIFY :
			{
			bool bEnabled;
			ret2 = HandlePropertySheet(lParam,&bEnabled);
			if(bEnabled){
				theSH.m_bWindowClosed = false;
				theSH.ProcessSelectedNodes();
			}

			if(ret2 == 0 && theSH.hwnd){
				theSH.WriteConfig();
				theSH.SaveNodeSettings();

				// Window is probably changing (but maybe not closing), so let's store the settings
				theSH.m_bRefreshedGUI		  = false; // To stop trying to gather from UI
				theSH.m_bWindowClosed = true;
			}
			}
			return ret||ret2;
		case WM_COMMAND:
			if(HandleFocus(wParam))
				return TRUE;
			switch (LOWORD(wParam)) 
			{
			case IDC_SELECTALL:
				{
					INodeTab nodes;
					for(int i=0;i<theSH.curNodes.size();i++)
						nodes.Append(1,&theSH.curNodes[i]);
					theSH.ip->SelectNodeTab(nodes,TRUE);
					break;
				}
			case IDC_USECUSTOM:
				{
					bool bEnable = Button_GetCheck(GetDlgItem(hWnd,IDC_USECUSTOM));
					theSH.EnableCustomUI(bEnable);
					break;
				}
			case IDC_ADDINBLOCKER:
				{
					theSH.EditBlockers(true);
					break;
				}
			case IDC_ADDOUTBLOCKER:
				{
					theSH.EditBlockers(false);
					break;
				}
			}

		default:
			return ret;
	}
	return TRUE; 
}


//----------------------------------------------------------------------------------
// Redundant
//----------------------------------------------------------------------------------
bool SphericalHarmonics::NodeSelected(){
	int numSelected = GetCOREInterface()->GetSelNodeCount();

	return true;//numSelected == 1;
}

//----------------------------------------------------------------------------------
// Pass-Through
//----------------------------------------------------------------------------------
void SphericalHarmonics::SelectionSetChanged(void *param, NotifyInfo *info) {
	theSH.ProcessSelectedNodes();
}

//--------------------------------------------------------------------------
// Desc: Reads selected node(s) and fills dialog fields
//--------------------------------------------------------------------------
void SphericalHarmonics::ProcessSelectedNodes(){
	if(!ip)
		return;

	// Save current node, before switching to new node
	// NOTE: Saves even if window not open, so we can maintain data across stack (complex issue)
	if(curNodes.size())
		SaveNodeSettings();

	int numSelected = ip->GetSelNodeCount();
	curNodes.resize(0);	
	curData.resize(0);

	// Reset GUI here
	inBlockers.SetCount(0);
	outBlockers.SetCount(0);
	EnableWindow(GetDlgItem(hwnd,IDC_ENABLESH),true);
	Button_SetCheck(GetDlgItem(hwnd,IDC_ENABLESH),false);
	EnableCustomUI(false);


	// Figure out which nodes to use
	for(int i=0;i<numSelected;i++){
		// Use all nodes that aren't group members (we'll just assign to their group heads instead)
		if(!ip->GetSelNode(i)->IsGroupMember()){
			curNodes.push_back(ip->GetSelNode(i));
			// Collect node data here too
			NodeData d;
			GetNodeData(ip->GetSelNode(i),d);
			// If not yet a class, auto-assign "StaticMesh", which is required for spherical harmonics
			if(d.script.classname.length() == 0 && ip->GetSelNode(i)->EvalWorldState(ip->GetTime()).obj->SuperClassID()==GEOMOBJECT_CLASS_ID)
			{
				theEntityInterface.AssignToNode(ip->GetSelNode(i),"StaticMesh");
			}
			else if(d.script.classname.length() == 0 && ip->GetSelNode(i)->EvalWorldState(ip->GetTime()).obj->SuperClassID()==LIGHT_CLASS_ID)
			{
				theEntityInterface.AssignToNode(ip->GetSelNode(i),"Light");
			}

			curData.push_back(d);
		}
	}


	// If GUI not open, exit now (we still want to do the above, in case dialog is closed
	// before saving, or so we can do our trick of saving data across stack collapses when closed
	if(m_bWindowClosed)
		return; 

	m_bRefreshedGUI = true;

	NodeData data;

	// Only populate the view if we have one node, or all nodes have matching data
	bool nodesIdentical = AllNodesIdentical(curNodes);
	if(curNodes.size() == 1 || nodesIdentical)
	{
		// Update the GUI
		data = curData[0];

		// SH Enabled checkbox
		Button_SetCheck(GetDlgItem(hwnd,IDC_ENABLESH),data.bSHEnabled);
		if(data.bUseCustom)
			EnableCustomUI(true);

		// Only update these params if SH is enabled, or we'll be getting junk values
		if(data.bSHEnabled)
		{
			UpdateGlobalProperties(data);

			// Indoor Blockers
			for(int i=0;i<data.inBlockers.size();i++){
				INode* node = ip->GetINodeByName(data.inBlockers[i].c_str());
				if(node)
					inBlockers.AddNode(node);
				else{
					data.inBlockers.erase(data.inBlockers.begin()+i);
					i--;
				}
			}
			// Outdoor Blockers
			for(int i=0;i<data.outBlockers.size();i++){
				INode* node = ip->GetINodeByName(data.outBlockers[i].c_str());
				if(node)
					outBlockers.AddNode(node);
				else{
					data.outBlockers.erase(data.outBlockers.begin()+i);
					i--;
				}
			}

			dlg.UpdateControlsWithSettings(&data.shOptions,dlg.m_hDlg);
		}
		else
			dlg.UpdateControlsWithSettings(&GetGlobalOptions(),dlg.m_hDlg);
	}
	else if(curNodes.size() == 0)
	{
		// Invalid selection group, don't allow enabling
		EnableWindow(GetDlgItem(hwnd,IDC_ENABLESH),false);
		dlg.UpdateControlsWithSettings(&GetGlobalOptions(),dlg.m_hDlg);
	}

	// Update pane
	string str = "Selected:";
	if(curNodes.size() == 1)
		str += "'"+string(curNodes[0]->GetName()) + "'\n";
	else
		str += "[" + ToStr(curNodes.size()) + " selected]\n";
	str += "Group: ";
	// Group if all nodes 
	if(!nodesIdentical || (data.receiverGroup.find("SH_") != 0))
		str += "[None]\n";
	else
		str += "'" + data.receiverGroup + "'\n";
	str += "Blockers(Ind): " + ToStr(data.inBlockers.size()) + "\n";
	str += "Blockers(Out): " + ToStr(data.outBlockers.size()) + "\n";

	SetText(hwnd,str,IDC_SHINFO);
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
SphericalHarmonics::SphericalHarmonics()
{
	ip = NULL;	
	hwnd = NULL;	
	spin = NULL;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void SphericalHarmonics::Init(){
	ip = GetCOREInterface();
	theSH.m_bRefreshedGUI		  = false;
	theSH.m_bWindowClosed = false;
	RegisterNotification(SphericalHarmonics::SelectionSetChanged,&theSH,NOTIFY_SELECTIONSET_CHANGED);
	//theSH.LoadAllScripts(false); // Must do this early
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void SphericalHarmonics::CloseDialog() 
{
	if(!hwnd) // We've already closed once
		return;
	//	EndDialog(hwnd,0);
	hwnd = NULL;
}

