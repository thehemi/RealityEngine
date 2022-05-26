//=========== Copyright (c) 2004, Tim Johnson. All rights reserved. ===========
//
// A utility to assign game classes to max objects
//
//=============================================================================
#include "stdafx.h"
#include "EntityInterface.h"
#include "GlobalMain.h"
#include "resource.h"
EntityInterface theEntityInterface;

Matrix3 mouseWorldPos(true);

//----------------------------------------------------------------------------------
// Desc: Also called externally to let other classes assign a node
//----------------------------------------------------------------------------------
void EntityInterface::AssignToNode(INode* node, string theClass)
{
	// Special-case StaticMesh in case the script isn't loaded
	// StaticMesh is so vital we never want it to fail
	if(theClass == "StaticMesh")
	{
		NodeData d;
		d.script.classname = d.script.filename = "StaticMesh";
		d.script.parameters.push_back("noclip");
		d.script.parameters.push_back("invisible");
		d.script.paramvalues.push_back("false");
		d.script.paramvalues.push_back("false");
		d.script.bIncludeModel = "true";
		SetNodeData(node,d);
		return;
	}

	int i = GetClassIndex(theClass.c_str());
	if(i != -1 && classes.size()>1)
	{
		NodeData d;
		GetNodeData(node,d);
		// Copy class data to this new node.
		// Explicit copy so we don't overwrite non-class data, such as SH
		d.script.classname = classes[i].script.classname;
		d.script.filename  = classes[i].script.filename;
		d.script.comments    = classes[i].script.comments;
		d.script.parentclass = classes[i].script.parentclass;
		d.script.parameters  = classes[i].script.parameters;
		d.script.paramvalues = classes[i].script.paramvalues;
		d.script.bIncludeModel = classes[i].script.bIncludeModel;
		SetNodeData(node,d);
	}
}

//----------------------------------------------------------------------------------
// Desc: Updates class based on new index
//----------------------------------------------------------------------------------
void EntityInterface::UpdateClass(int iIndex){
	curClassSelection = iIndex;

	if(curClassSelection == -1 || classes.size() == 0)
		return;

	// Set node properties
	for(int i=0;i<curNodes.size();i++){
		NodeData d;
		GetNodeData(curNodes[i],d);

		// If [None] class, clear data
		if(curClassSelection == 0){ 
			d.script.classname = "";
			d.script.filename  = "";
			d.script.comments  = "";
			d.script.parentclass = "";
			d.script.parameters.clear();
			d.script.paramvalues.clear();
			// Don't do this - would kill PRT data too
			//ClearNodeData(curNodes[i]);
			SetNodeData(curNodes[i],d);

		}
		// If this is a NEW node selection, copy over new data
		else if(d.script.classname != classes[curClassSelection].script.classname){
			AssignToNode(curNodes[i],classes[curClassSelection].script.classname);
		}
	}

	// Update the interface
	ProcessSelectedNodes();
}



//--------------------------------------------------------------------------
// Desc: Sets up our listview HWND
//--------------------------------------------------------------------------
HWND hListView;
void SetupDialog (HWND hwndDLV) {
	LVCOLUMN lvc;

	RECT r;
	GetClientRect(hwndDLV,&r);
	int width = r.right - r.left;
	int height = r.bottom - r.top;
	// define the first column

	lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT | LVCF_SUBITEM;
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = (width / 2) - 5;
	lvc.pszText= TEXT("Value");
	lvc.iSubItem = 0;
	SendMessage( hwndDLV, LVM_INSERTCOLUMN, 0, (LPARAM)&lvc);

	// define the second column

	lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT | LVCF_SUBITEM;
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx= (width / 2) - 5;
	lvc.pszText=TEXT ("Propertry");
	lvc.iSubItem=1;
	SendMessage( hwndDLV, LVM_INSERTCOLUMN, 1, (LPARAM)&lvc);

	// Re-Order them
	int iOrderArray [ ] = {1,0};
	ListView_SetColumnOrderArray(hListView,2,iOrderArray);
	return;

}

//--------------------------------------------------------------------------
// Desc: Adds an item to the listview
//--------------------------------------------------------------------------
int AddItem(int index, const char* text){
	LVITEM lvi;
	lvi.mask=LVIF_TEXT;
	lvi.iItem=index;
	lvi.iSubItem=0;
	lvi.pszText=(char*)text;

	int realIndex = ListView_InsertItem (hListView, &lvi);
//	ListView_SetItemText (hListView, count, 1, cData);
	return realIndex;
}

//--------------------------------------------------------------------------
// Desc: Sets a 'property' field
//--------------------------------------------------------------------------
void SetPropText(int index, const char* text){
	ListView_SetItemText (hListView, index, 1, (char*)text);
}

//--------------------------------------------------------------------------
// Desc: Sets a 'value' field
//--------------------------------------------------------------------------
void SetValText(int index, const char* text){
	ListView_SetItemText (hListView, index, 0, (char*)text);
}

//--------------------------------------------------------------------------
// Desc: Fills listview with class data
//--------------------------------------------------------------------------
void EntityInterface::PopulateListView(NodeData& cd){
	ListView_DeleteAllItems(hListView);

	for(int i=0;i<cd.script.parameters.size();i++){
		int index = AddItem(i,cd.script.paramvalues[i].c_str());
		SetPropText(index,cd.script.parameters[i].c_str());
	}
}

/*
*	void CheckMessages(void* param, NotifyInfo* info)
{
MSG msg;
IPoint2 mousePos;
// Make sure any mouse messages go to us.
while(GetMessage( &msg, NULL, 0U, 0U )){
// Get mouse pos so we know where to create entity
if(msg.message == WM_RBUTTONDOWN){
GetCursorPos((POINT*)&mousePos);
ScreenToClient(GetCOREInterface()->GetActiveViewport()->GetHWnd(),(POINT*)&mousePos);
mouseWorldPos.IdentityMatrix();
mouseWorldPos.SetTrans( GetCOREInterface()->GetActiveViewport()->SnapPoint(mousePos,mousePos,NULL,SNAP_IN_3D) );
}
if(msg.message == WM_QUIT)
GetCOREInterface()->CheckMAXMessages();

GetCOREInterface()->TranslateAndDispatchMAXMessage(msg);
}
}
 */
//--------------------------------------------------------------------------
// Desc: Handles dialog messages
//--------------------------------------------------------------------------
BOOL CALLBACK EntityInterfaceDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam){
	BOOL ret;
	IPoint2 mousePos;
	switch (msg) {
		case WM_INITDIALOG:{
			// Set up toolbar icons & tooltips
			HICON hU = (HICON)LoadImage( g_hInstance,MAKEINTRESOURCE( IDI_OPEN ),IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR );
			SendMessage( GetDlgItem(hWnd,IDC_SETSCRIPTFOLDER), BM_SETIMAGE, IMAGE_ICON,(LPARAM) (DWORD) hU );

			hU = (HICON)LoadImage( g_hInstance,MAKEINTRESOURCE( IDI_EDIT ),IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR );
			SendMessage( GetDlgItem(hWnd,IDC_EDITSCRIPT), BM_SETIMAGE, IMAGE_ICON,(LPARAM) (DWORD) hU );
			
			hU = (HICON)LoadImage( g_hInstance,MAKEINTRESOURCE( IDI_REFRESH ),IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR );
			SendMessage( GetDlgItem(hWnd,IDC_REFRESHSCRIPTS), BM_SETIMAGE, IMAGE_ICON,(LPARAM) (DWORD) hU );
			
			CreateMyTooltip(GetDlgItem(hWnd,IDC_EDITSCRIPT),"Edit script...");
			CreateMyTooltip(GetDlgItem(hWnd,IDC_SETSCRIPTFOLDER),"Change script folder...");
			CreateMyTooltip(GetDlgItem(hWnd,IDC_REFRESHSCRIPTS),"Refresh scripts");
			CreateMyTooltip(GetDlgItem(hWnd,IDC_APPDATA_CLASSES),"Applies script to selected item(s)");
			
			theEntityInterface.hwnd = hWnd;
			hListView = GetDlgItem(hWnd,IDC_LISTVIEW);
			SetupDialog(hListView);
			theEntityInterface.LoadAllScripts(false);
			RegisterNotification(EntityInterface::SelectionSetChanged,&theEntityInterface,NOTIFY_SELECTIONSET_CHANGED);
			// Add grid styles to listbox
			DWORD dwStyle = SendMessage(hListView,LVM_GETEXTENDEDLISTVIEWSTYLE,0,0);
			dwStyle |= LVS_EX_HEADERDRAGDROP| LVS_EX_GRIDLINES 
				| LVS_EX_FULLROWSELECT | LVS_EX_ONECLICKACTIVATE | LVS_EX_TRACKSELECT  ;
			SendMessage(hListView,LVM_SETEXTENDEDLISTVIEWSTYLE, 0, (LPARAM)dwStyle);
						   }
			break;

		case WM_DESTROY:
		case WM_CLOSE:
			theEntityInterface.CloseDialog();
			break;


		// Catch notifications for listbox
		case WM_NOTIFY:
			LPNMHDR hdr;
			NMLVDISPINFO *di;
			hdr=(LPNMHDR)lParam;
			di=(NMLVDISPINFO *)lParam;
			if (hdr->hwndFrom == hListView) {
				switch (hdr->code) {
					// IMPORTANT: Makes it editable immediately
					case NM_CLICK:{
						LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE) lParam;
						ListView_EditLabel(hListView,lpnmitem->iItem);
						}
						break;
					case LVN_BEGINLABELEDIT:
						DisableAccelerators();
						SetFocus(hListView);
						return FALSE; // Allow user to edit
					case LVN_ENDLABELEDIT:
						if (di->item.pszText != NULL) {
							// Get the property name
							char propName[512];
							ListView_GetItemText(hListView,di->item.iItem,1,propName,512);
							// Get new value
							string val = di->item.pszText;
							// Get the 'old' property value
							string oldVal = GetNodeProperty(theEntityInterface.curNodes[0],propName);

							// If old property was a string, force "quotes" around the new value
							if(oldVal.find("\"") != -1){
								if(val[0] != '\"' && val[0] != '\'')
									val = "\"" + val;
								if(val[val.length()-1] != '\"' && val[val.length()-1] != '\'')
									val = val + "\"";

							}
							// If old property was object, strip any "quotes"
							else{
								if(val[0] == '\"' || val[0] == '\'')
									val = val.substr(1);
								if(val[val.length()-1] == '\"' || val[val.length()-1] == '\'')
									val = val.substr(0,val.length()-1);
							}

							// Update the node info and the listbox view
							for(int i=0;i<theEntityInterface.curNodes.size();i++)
								SetNodeProperty(theEntityInterface.curNodes[i],propName,val);
							SetValText(di->item.iItem,val.c_str());

							return TRUE;
						} 
						else
							return FALSE;
					default: break;
				}
			}
			ret = HandlePropertySheet(lParam);
			if(!ret && theEntityInterface.hwnd){
				// Window is probably changing (but maybe not closing), so let's store the settings
			}
			return ret;
			return HandlePropertySheet(lParam);

		case WM_COMMAND:
			if(HandleFocus(wParam))
				return TRUE;
			switch (LOWORD(wParam)) {
			case IDC_SPAWNENTITY:
				theEntityInterface.CreateObject(true); 
			break;
			case IDC_APPDATA_CLASSES:
				// Class selection changed -- Assign class to selected object
				if(HIWORD(wParam) == CBN_SELCHANGE){
					// Get cur selection
					int iIndex = SendMessage(GetDlgItem(hWnd,IDC_APPDATA_CLASSES),CB_GETCURSEL,0,0);
					theEntityInterface.UpdateClass(iIndex);
				};
				break;

			case IDC_SETSCRIPTFOLDER:
				theEntityInterface.SetScriptsFolder();
				break;
			case IDC_EDITSCRIPT:
				theEntityInterface.EditScript();
				break;
			case IDC_REFRESHSCRIPTS:
				theEntityInterface.LoadAllScripts(true);
				break;
			}

		default:
			return FALSE;
	}
	return TRUE; 
}

//--------------------------------------------------------------------------
// Desc: Updates the config with the scripts folder
//--------------------------------------------------------------------------
void EntityInterface::SetScriptsFolder(){
	char fld[MAX_PATH];
	ip->ChooseDirectory(hwnd,_T("Choose scripts folder"),fld,NULL);
	string folder = fld;
	scriptsFolder = folder;

	// Open our config file
	theConfig.SetString("ScriptsFolder",folder);

	// Reload everything
	LoadAllScripts(true);
}


//--------------------------------------------------------------------------
// Desc: Gets array index from name
//--------------------------------------------------------------------------
int EntityInterface::GetClassIndex(const char* name){
	for(int i=0;i<classes.size();i++){
		string s1 = classes[i].script.classname, s2 = name;
		ToLowerCase(s1);
		ToLowerCase(s2);
		if(s1 == s2){
			return i;
		}
	}
	return 0; // 0 = [None] class
}

//--------------------------------------------------------------------------
// Pseudo-code:
// 1. Get a list of all scripts in directory
// 2. Foreach script
// 2.1 If this is a valid script
// 2.2 Open file
// 2.3	Get class name
// 2.4  get parameters
//--------------------------------------------------------------------------
void EntityInterface::ProcessScripts(string folder){
	// 1
	vector<FileList> scripts;
	recurseDir((char*)folder.c_str(),scripts,true,"*.py");

	// 2
	for(int i=0;i<scripts.size();i++){
		bool addComments = true; // Add comments until we hit a non-comment line
		// 2.1
		// Only read .PY files (temp, to check)
		assert(string(scripts[i].name).find(".pyc") == -1);

		// 2.2
		string str;
		string filePath = scripts[i].path;
		filePath += "\\";
		filePath += string(scripts[i].name);

		ifstream scriptIn(filePath.c_str());
		NodeData data;
		data.script.filename = filePath;
		do {
			char temp[512];
			scriptIn.getline(temp,512);
			str = temp;

			// Stop adding comments as soon as we hit a non-comment line
			if(str.find("#") == -1)
				addComments = false;
				
			// Skip full-line comments, but add them to comments string
			if(str.find("#") == 0 && addComments){
				data.script.comments += str + "\r\n";
				continue;
			}

			// Strip comments from lines
			if(str.find("#") != -1){
				str = str.substr(0,str.find("#"));
			}

			// 2.3
			if(str.find("class") == 0){
				// Skip strings
				if(str.find("\"\"")!=-1)
					continue;
				// Chop the end off
				if(str.find("(") !=-1){
					// Get the parent name first
					data.script.parentclass = str.substr(str.find("(")+1);
					data.script.parentclass = data.script.parentclass.substr(0,data.script.parentclass.find(")"));
					str = str.substr(0,str.find("("));
				}
				else
					str = str.substr(0,str.find(":"));

				// Remove any remaining spaces
				trimRight(str);

				// Chop the front off
				str = str.substr(str.find_last_of(" ")+1);

				data.script.classname = str;
			}
			// Break if we've passed all params
			else if(str.find("def ")!=-1)
				break;

			// 2.4
			else if(str.find("=")!=-1){
				// Remove all the whitespace except any in strings
				char temp[512];
				if(str.find("\"") == -1){
					strcpy(temp,str.c_str());
					remove_whitespace(temp);
					str = temp;
				}
				else{
					// only remove the whitespace outside the string.
					strcpy(temp,str.substr(0,str.find("\"")).c_str());
					remove_whitespace(temp);
					string str2 = str;
					str = temp;
					str += str2.substr(str2.find("\""));

					// Erase the quotes
					//strcpy(temp,str.c_str());
					//remove_quotes(temp);
					//str = temp;
				}

				string param	= str.substr(0,str.find("="));
				string paramVal = str.substr(str.find("=")+1);

				// Check for special fixed param - 'UsesModel'
				// If we have it, set bool and keep off param list
				if(AsLower(param) == "usesmodel"){
					data.script.bIncludeModel = AsLower(paramVal) == "true";
					continue;
				}

				// Store the param
				data.script.parameters.push_back(str.substr(0,str.find("=")));
				// Store its default value
				data.script.paramvalues.push_back(str.substr(str.find("=")+1));

			}
		} while(!scriptIn.eof());

		// If this had a valid class, push it
		if(data.script.classname.length())
			classes.push_back(data);
	}

	// Add all parent parameters to the classes
	// Pseudo-code:
	// 1. Foreach class
	// 1.1 While (hasParent)
	// 1.2 Get parent params
	// 1.3 Add parent params to inherited param list
	// 1.4 parent = parent.script.parentclass;

	// Temp storage for inherited param list
	NodeData* inheritedData = new NodeData[classes.size()];

	for(i=0;i<classes.size();i++){
		string parent = classes[i].script.parentclass;
		// 1.1
		while(parent != ""){
			// 1.2
			int index = GetClassIndex(parent.c_str());
			if(index == -1)
				break;
			NodeData cParent = classes[index];

			// 1.3
			for(int j=0;j<cParent.script.parameters.size();j++){
				inheritedData[i].script.parameters.push_back(cParent.script.parameters[j]);
				inheritedData[i].script.paramvalues.push_back(cParent.script.paramvalues[j]);
			}

			// 1.4
			parent = cParent.script.parentclass;
		}
	}

	// Merge inherited and normal params
	for(i=0;i<classes.size();i++){
		for(int j=0;j<inheritedData[i].script.parameters.size();j++){
			classes[i].script.parameters.push_back(inheritedData[i].script.parameters[j]);
			classes[i].script.paramvalues.push_back(inheritedData[i].script.paramvalues[j]);
		}
	}

	// Delete temp storage
	delete[] inheritedData;

	// Put the scripts on the combo
	if(hwnd){
		SendMessage(GetDlgItem(hwnd,IDC_APPDATA_CLASSES),CB_RESETCONTENT,0,0);
		for(i=0;i<classes.size();i++)
			SendMessage(GetDlgItem(hwnd,IDC_APPDATA_CLASSES),CB_ADDSTRING,0,(LPARAM)classes[i].script.classname.c_str());
	}
}

bool EntityInterface::NodeSelected(){
	int numSelected = GetCOREInterface()->GetSelNodeCount();

	return true;//numSelected == 1;
}

void EntityInterface::SelectionSetChanged(void *param, NotifyInfo *info) {
	theEntityInterface.ProcessSelectedNodes();
}


bool AllNodesIdentical(vector<INode*>& nodes){
	if(nodes.size() == 0)
		return false;

	NodeData n1, n2;
	// Fill first node
	GetNodeData(nodes[0],n1);

	// Compare all nodes
	for(int i=1;i<nodes.size();i++){
		// Compare nodes
		if(GetNodeData(nodes[i],n2) && n2 == n1){
			n1 = n2;
			continue; // They match, continue checking next node pair
		}

		return false;
	}
	return true;
}

//--------------------------------------------------------------------------
// Desc: Reads selected node(s) and fills dialog fields
//--------------------------------------------------------------------------
void EntityInterface::ProcessSelectedNodes(){
	if(!ip)
		return;

	int numSelected = ip->GetSelNodeCount();
	curNodes.resize(0);

	// Initially set to [None] and defaults
	SendMessage(GetDlgItem(hwnd,IDC_APPDATA_CLASSES),CB_SETCURSEL,0,0);
	ListView_DeleteAllItems(hListView);
	SetText(hwnd,"",IDC_COMMENTS);

	// If nothing selected, we're done
	if(numSelected == 0 || classes.size() == 0){
		return;
	}

	// Figure out which nodes to use
	for(int i=0;i<numSelected;i++){
		// Use all nodes that aren't group members (we'll just assign to their group heads instead)
		if(!ip->GetSelNode(i)->IsGroupMember())
			curNodes.push_back(ip->GetSelNode(i));
	}

	// Only populate the view if we have one node, or all nodes have matching data
	if(curNodes.size()){// == 1 || AllNodesIdentical(curNodes)){
		CStr str;
		NodeData temp;

		// Update the selection for the combo box
		if(GetNodeData(curNodes[0],temp)){
			SendMessage(GetDlgItem(hwnd,IDC_APPDATA_CLASSES),CB_SELECTSTRING,0,(LPARAM)temp.script.classname.c_str());

			// Update the param list
			PopulateListView(temp);

			// Update the comments field by looking up the original node data from the current node name
			SetText(hwnd,classes[GetClassIndex(temp.script.classname.c_str())].script.comments.c_str(),IDC_COMMENTS);
		}
	}		
}

//--------------------------------------------------------------------------
// Desc: Allows manual editing of the current class file
//--------------------------------------------------------------------------
void EntityInterface::EditScript(){
	if(curNodes.size() == 1){
		NodeData cd;
		if(!GetNodeData(curNodes[0],cd))
			MessageBox(0,"You haven't selected a class for this node to edit",0,0);
		else
			ShellExecute (hwnd, "open", "NOTEPAD.EXE", cd.script.filename.c_str(),"",1);
	}
	else if(curNodes.size() > 1)
		Error("You can't edit a script whilst you have multiple entities selected");
}

//--------------------------------------------------------------------------
// 1. If we are uninitialized, try to load settings file to get scripts path
// 1.1 Load and display all scripts [ProcessScripts()]
// 2. If already initialized, just reload the combo box with the classes data
// 3. Check for any newly selected nodes, and load their listbox data [ProcessSelectedNode]
//--------------------------------------------------------------------------
void EntityInterface::LoadAllScripts(bool reload){
	// Try to find our settings file
	if(reload || classes.size() == 0){
		classes.clear();
		NodeData dummy;
		dummy.script.classname = "[None]";
		classes.push_back(dummy);

		string str;
		scriptsFolder = theConfig.GetString("ScriptsFolder");

		// Try to process all scripts
		if(scriptsFolder.length())
			ProcessScripts(scriptsFolder);

		if(hwnd)
			SendMessage(GetDlgItem(hwnd,IDC_APPDATA_CLASSES),CB_SETCURSEL,0,0);
	}

	// Put the scripts on the combo
	if(hwnd){
		SendMessage(GetDlgItem(hwnd,IDC_APPDATA_CLASSES),CB_RESETCONTENT,0,0);
		for(int i=0;i<classes.size();i++)
			SendMessage(GetDlgItem(hwnd,IDC_APPDATA_CLASSES),CB_ADDSTRING,0,(LPARAM)classes[i].script.classname.c_str());
	}

	// Process any newly selected nodes
	ProcessSelectedNodes();
}

//--------------------------------------------------------------------------
// Desc: Spawns an 'entity' object
//--------------------------------------------------------------------------
void EntityInterface::CreateObject(bool spawnAtCenter){
	// Create a new object through the CreateInstance() API
	Object *obj = (Object*)ip->CreateInstance(GEOMOBJECT_CLASS_ID,Class_ID(CONE_CLASS_ID,0));
	assert(obj);
	// Get a hold of the parameter block
	IParamArray *iCylParams = obj->GetParamBlock();
	assert(iCylParams);
	int radius2 = obj->GetParamBlockIndex(CONE_RADIUS2);
	assert(radius2>=0);
	iCylParams->SetValue(radius2,TimeValue(0),0);

	// Get the scaling needed to put coordinates in meters
	float worldScale = GetMasterScale(UNITS_METERS);

	// Set the values
	int radius1 = obj->GetParamBlockIndex(CONE_RADIUS1);
	assert(radius1>=0);
	iCylParams->SetValue(radius1,TimeValue(0),0.5f / worldScale);

	int height = obj->GetParamBlockIndex(CONE_HEIGHT);
	assert(height>=0);
	iCylParams->SetValue(height,TimeValue(0),3 / worldScale);

	int segs = obj->GetParamBlockIndex(CONE_SEGMENTS);
	assert(segs>=0);
	iCylParams->SetValue(segs,TimeValue(0),2);

	int sides = obj->GetParamBlockIndex(CONE_SIDES);
	assert(sides>=0);
	iCylParams->SetValue(sides,TimeValue(0),8);

	INode *node = ip->CreateObjectNode(obj);
	// Name the node and make the name unique.
	TSTR name(_T("Entity"));
	ip->MakeNameUnique(name);
	node->SetName(name);

	// Node has no rotation to start
	node->SetObjOffsetRot(IdentQuat());

	// Get node pos from viewport
	if(!spawnAtCenter){
		IPoint2 mousePos;
		GetCursorPos((POINT*)&mousePos);
		ScreenToClient(GetCOREInterface()->GetActiveViewport()->GetHWnd(),(POINT*)&mousePos);
		mouseWorldPos.IdentityMatrix();
		mouseWorldPos.SetTrans( GetCOREInterface()->GetActiveViewport()->SnapPoint(mousePos,mousePos,NULL,SNAP_IN_3D) );
		ip->SetNodeTMRelConstPlane(node,mouseWorldPos);
	}
	else{
		node->SetNodeTM(0,Matrix3(true));
	}

	//------------------------------------------------
	// Create the material for the object
	//-------------------------------------------------
	//if (! ip->GetSelNodeCount()) return; // Nothing to assign.
	//INode *node = ip->GetSelNode(0);
	// Create a new Standard material.
	/*StdMat2 *m = NewDefaultStdMat();
	// Set its properties...
	m->SetName(_T("EntityMap"));
	m->SetAmbient(Color(1.0f,0.0f,0.0f),0); // Pure Red
	m->SetDiffuse(Color(1.0f,0.0f,0.0f),0);
	m->SetSpecular(Color(1.0f,1.0f,1.0f),0);
	m->SetShininess(0.5f,0);
	m->SetShinStr(.7f,0);
	m->SetSelfIllumColor(Color(1,0,0),0);
	m->SetSelfIllumColorOn(1);*/

	// Texture
/*	BitmapTex *bmt = NewDefaultBitmapTex();
	if(!DoesFileExist(TSTR("maps\\entity_map.bmp"))){
		MessageBox(NULL,"Couldn't find texture for entity object: entity_map.bmp - This should be in your 3dsmax\\maps folder","Texture load",MB_OK);
	}
	else{
		bmt->SetMapName(TSTR("entity_map.bmp"));
		bmt->GetUVGen()->SetCoordMapping(UVMAP_SCREEN_ENV);
		bmt->ActivateTexDisplay(TRUE);
		m->SetSubTexmap(ID_DI,bmt);
		m->EnableMap(ID_DI,TRUE);
		// Assign it to the node.
		node->SetMtl(m);

		// Turn it on in the viewport
		ip->ActivateTexture(bmt,m);
	}*/

	// Assign entity to current class
	SetNodeData(node,theEntityInterface.classes[theEntityInterface.curClassSelection]);

	// Select this node (triggers SelectionSetChanged() callback which initializes object)
	theHold.Begin();
	ip->ClearNodeSelection(FALSE);
	ip->SelectNode(node);
	TSTR undostr; undostr.printf("Select");
	theHold.Accept(undostr);

	// Redraw the viewports
	ip->RedrawViews(ip->GetTime());
}


EntityInterface::EntityInterface()
{
	ip = NULL;	
	hwnd = NULL;	
	spin = NULL;
	curClassSelection = 0;
}


void EntityInterface::Init(){
	ip = GetCOREInterface();
	theEntityInterface.LoadAllScripts(false); // Must do this early
}



void EntityInterface::CloseDialog() 
{
	if(!hwnd) // We've already closed once
		return;
//	EndDialog(hwnd,0);
	hwnd = NULL;
}
