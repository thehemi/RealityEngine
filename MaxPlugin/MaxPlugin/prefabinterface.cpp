#include "stdafx.h"
#include "prefabinterface.h"
#include "xrefutil.h"
#include "EntityInterface.h"
#include "ExportInterface.h"

PrefabInterface thePrefab;

//----------------------------------------------------------------------------------
// Desc: Helper
//----------------------------------------------------------------------------------
struct PACKEDMETA {
	WORD mm;
	WORD xExt;
	WORD yExt;
	WORD dummy;
};

//----------------------------------------------------------------------------------
// Desc: Loads a thumbnail with description into the below variables
//----------------------------------------------------------------------------------
string		text;
HMETAFILE	hMetaFile = NULL;
void GetPreview(const char* filename){
	if(hMetaFile)
		DeleteMetaFile(hMetaFile);

	HRESULT res;
	LPSTORAGE pIStorage=NULL;
	res = StgOpenStorage (WStr(filename),0,STGM_DIRECT|STGM_READ|STGM_SHARE_EXCLUSIVE,NULL,0,&pIStorage);
	if(res != S_OK)
		MessageBox(0,"StgOpenStorage Failed",0,0);

	IPropertySetStorage*	pPropertySetStorage=NULL;
	IPropertyStorage*		pPropertyStorage=NULL;

	// Get the Storage interface
	if (S_OK != pIStorage->QueryInterface(IID_IPropertySetStorage,(void**)&pPropertySetStorage))
		MessageBox(0,"GetPreview Failed Query",0,0);

	if (S_OK != pPropertySetStorage->Open(FMTID_SummaryInformation,STGM_READ|STGM_SHARE_EXCLUSIVE,&pPropertyStorage))
		MessageBox(0,"GetPreview Failed Open",0,0);

	PROPSPEC	PropSpec[6];
	PROPVARIANT	PropVar[6];

	PropSpec[0].ulKind = PRSPEC_PROPID;
	PropSpec[0].propid = PIDSI_TITLE;

	PropSpec[1].ulKind = PRSPEC_PROPID;
	PropSpec[1].propid = PIDSI_SUBJECT;

	PropSpec[2].ulKind = PRSPEC_PROPID;
	PropSpec[2].propid = PIDSI_AUTHOR;

	PropSpec[3].ulKind = PRSPEC_PROPID;
	PropSpec[3].propid = PIDSI_KEYWORDS;

	PropSpec[4].ulKind = PRSPEC_PROPID;
	PropSpec[4].propid = PIDSI_COMMENTS;

	PropSpec[5].ulKind = PRSPEC_PROPID;
	PropSpec[5].propid = PIDSI_THUMBNAIL;

	if (S_OK == pPropertyStorage->ReadMultiple(6, PropSpec, PropVar)) {

		if (PropVar[0].vt == VT_LPSTR)
			text += PropVar[0].pszVal + string("\n");
		if (PropVar[1].vt == VT_LPSTR)
			text += PropVar[1].pszVal + string("\n");
		if (PropVar[2].vt == VT_LPSTR)
			text += PropVar[2].pszVal + string("\n");
		if (PropVar[3].vt == VT_LPSTR)
			text += PropVar[3].pszVal + string("\n");
		if (PropVar[4].vt == VT_LPSTR)
			text += PropVar[4].pszVal + string("\n");

		if (PropVar[5].vt == VT_CF) {
			// Got clipboard format
			//Once found, MAX retrieves the data, in metafile format, and converts it to a DIB:

			CLIPDATA*	pClip = PropVar[5].pclipdata;
			PACKEDMETA*	pPackedMeta;
			if (pClip->ulClipFmt == -1) {
				BYTE* pb = pClip->pClipData;

				if (*pb == CF_METAFILEPICT) {
					// Got metafile format
					pb += sizeof(DWORD);		// Skip the clip type
					pPackedMeta = (PACKEDMETA*)pb;
					pb += sizeof(PACKEDMETA);	// Skip packed meta struct
					int	nMetaFileSize = pClip->cbSize - sizeof(pClip->ulClipFmt) -sizeof(DWORD) -sizeof(PACKEDMETA);

					HDC			hdc = GetDC(thePrefab.hwnd);
					void*		bits = NULL;
					//					BITMAPINFO*	lpbmi;
					int			nSizeImage = 4*pPackedMeta->xExt*pPackedMeta->yExt;

					//lpbmi = (BITMAPINFO*)malloc(sizeof(BITMAPINFO)+nSizeImage);
					//if (lpbmi) {
					hMetaFile = SetMetaFileBitsEx(nMetaFileSize, pb);
					//}
				}
			}
		}
	}

	pPropertySetStorage->Release();
	pIStorage->Release();
}



//----------------------------------------------------------------------------------
// Draws the thumbnail
//----------------------------------------------------------------------------------
void PaintBmp(HDC dc,RECT *r)
{
	if(!hMetaFile)
		return;
	HDC cdc = CreateCompatibleDC(dc);
	if(!cdc)
		return;

	POINT old;
	SetViewportOrgEx(dc,140,50,&old);
	PlayMetaFile(dc, hMetaFile);
	SetViewportOrgEx(dc,old.x,old.y,NULL);
	DeleteDC(cdc);
}


//----------------------------------------------------------------------------------
// Desc: Reads config
//----------------------------------------------------------------------------------
void PrefabInterface::GetSettings(){
	rootFolder = theConfig.GetString("RootFolder");
	curFile    = theConfig.GetInt("CurrentFile");
	curFolder  = theConfig.GetInt("CurrentFolder");
}

//----------------------------------------------------------------------------------
// Desc: Writes config
//----------------------------------------------------------------------------------
void PrefabInterface::CommitSettings(){
	theConfig.SetString("RootFolder",rootFolder);
	theConfig.SetInt("CurrentFile",curFile);
	theConfig.SetInt("CurrentFolder",curFolder);
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void PrefabInterface::SavePrefab(){
	if(curFolder != -1){
		// Set file to a good starting path
		string path = folders[curFolder].path + string("\\") + string(folders[curFolder].name) + "\\";
		string file = path;
		INode* head = theXrefutil.ConvertSelectedToXrefObject(file,hwnd);
		if(head){
			MakePrefab(file,head);

			file = StripExtension(StripPath(file));
			theExport.Save(hwnd,file,true,path,ExportInterface::TYPE_MODEL,true,true);
		}
	}
	else
		MessageBox(0,"Please select a folder from the drop-down list before saving a prefab","Notice",MB_ICONSTOP);

}

//----------------------------------------------------------------------------------
// So we can enable/disable "save selected" button
//----------------------------------------------------------------------------------
void PrefabInterface::SelectionSetChanged(void* param, NotifyInfo* info){
	HWND hButtonWnd=GetDlgItem(thePrefab.hwnd, IDC_SAVEASPREFAB);
	EnableWindow(hButtonWnd, (GetCOREInterface()->GetSelNodeCount()>0));
}


//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void PrefabInterface::MakePrefab(string filename, INode* node){
	theEntityInterface.AssignToNode(node,"Prefab");
	// Now add in filename = [file] to the properties
	SetNodeProperty(node,"filename",filename);

	// Re-select it to trigger the entity utility to register the new settings
	GetCOREInterface()->ClearNodeSelection();
	theHold.Begin();
	GetCOREInterface()->SelectNode(node);
	TSTR undostr; undostr.printf("Select");
	theHold.Accept(undostr);
}

//----------------------------------------------------------------------------------
// Desc: Updates GUI and settings for file index
//----------------------------------------------------------------------------------
void PrefabInterface::FileSelected(int index){
	if(index >= files.size() || files.size() == 0)
		return;

	curFile = index;
	string file = files[index].path + string(files[index].name);
	GetPreview(file.c_str());
	InvalidateRect(hwnd,NULL,FALSE);
	UpdateWindow(hwnd);
}

//----------------------------------------------------------------------------------
// Desc: Updates GUI and settings for folder index
//----------------------------------------------------------------------------------
void PrefabInterface::PopulateFiles(int index){
	if(index >= folders.size())
		return;

	curFolder = index;
	string path = folders[index].path + string("\\") + string(folders[index].name) + "\\";

	// Reset file list
	files.clear();

	recurseDir(path.c_str(),files,true,"*.max");

	SendMessage(GetDlgItem(hwnd,IDC_FILES),LB_RESETCONTENT,0,0);
	for(int i=0;i<files.size();i++){
		string s = string(files[i].name);
		SendMessage(GetDlgItem(hwnd,IDC_FILES),LB_ADDSTRING,0,(LPARAM)s.substr(0,AsLower(s).find(".max")).c_str());
	}

	// Select last chosen file
	if(curFile < files.size())
		SendMessage(GetDlgItem(hwnd,IDC_FILES),LB_SETCURSEL,curFile,0);
}

//----------------------------------------------------------------------------------
// Desc: Fills folder combo box
//----------------------------------------------------------------------------------
void PrefabInterface::PopulateFolders(){
	if(rootFolder.length() == 0)
		return;

	// Reset folder list
	folders.clear();

	recurseDir(rootFolder.c_str(),folders,false);

	if(folders.size() == 0){
		MessageBox(0,"The prefabs folder you selected has no sub-folders.\nPlease select a folder that contains sub-folders with prefabs, preferably sorted by category.",0,0);
	}

	SendMessage(GetDlgItem(hwnd,IDC_FOLDERS),CB_RESETCONTENT,0,0);
	// Add all child folders
	for(int i=0;i<folders.size();i++)
		SendMessage(GetDlgItem(hwnd,IDC_FOLDERS),CB_ADDSTRING,0,(LPARAM)folders[i].name);

	// Select last chosen folder
	if(curFolder < folders.size())
		SendMessage(GetDlgItem(hwnd,IDC_FOLDERS),CB_SETCURSEL,curFolder,0);
}

BOOL CALLBACK PrefabInterfaceDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	BOOL ret;

	switch (msg) {
case WM_INITDIALOG:{
	thePrefab.hwnd = hWnd;
	// Set up toolbar icons & tooltips
	HICON hU = (HICON)LoadImage( g_hInstance,MAKEINTRESOURCE( IDI_OPEN ),IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR );
	SendMessage( GetDlgItem(hWnd,IDC_PATH), BM_SETIMAGE, IMAGE_ICON,(LPARAM) (DWORD) hU );

	CreateMyTooltip(GetDlgItem(hWnd,IDC_PATH),"Prefabs path...");

	thePrefab.GetSettings();
	SetText(hWnd,text,IDC_INFO);
	CenterWindow(hWnd,GetParent(hWnd));
	thePrefab.PopulateFolders();
	thePrefab.PopulateFiles(thePrefab.curFolder);
	thePrefab.FileSelected(thePrefab.curFile);

	// Set selection notification and trigger it immediately
	RegisterNotification(PrefabInterface::SelectionSetChanged,&thePrefab,NOTIFY_SELECTIONSET_CHANGED);
	thePrefab.SelectionSetChanged(NULL,NULL);
	return TRUE;
				   }
case WM_NOTIFY :
	ret = HandlePropertySheet(lParam);
	if(!ret && thePrefab.hwnd){
		// Window is changing (but maybe not closing), so let's store the settings
		thePrefab.CommitSettings(); 
	}
	return ret;

case WM_DESTROY:
case WM_CLOSE:
	thePrefab.CloseDialog();
	break;
case WM_PAINT:
	{
		PAINTSTRUCT ps;
		BeginPaint(hWnd,&ps);
		PaintBmp(ps.hdc,&ps.rcPaint);
		EndPaint(hWnd,&ps);
	}
	break;
case WM_COMMAND:
	if(HandleFocus(wParam))
		return TRUE;
	switch (LOWORD(wParam)) {

case IDC_PATH:
	thePrefab.rootFolder = BrowseForDir(hWnd,"Locate prefabs folder...");
	thePrefab.PopulateFolders();
	break;

case IDC_ADD:
	// Import prefab via x-ref!
	if(thePrefab.curFile != -1 && thePrefab.files.size()){
		string file = thePrefab.files[thePrefab.curFile].path + string(thePrefab.files[thePrefab.curFile].name);
		INode* head = theXrefutil.AddNewXrefObject(file);
		// Make it a prefab
		if(head)
			thePrefab.MakePrefab(file,head);
	}
	//EndDialog (hWnd, 0);
	return (TRUE);

case IDC_SAVEASPREFAB:
	thePrefab.SavePrefab();
	return (TRUE);

case IDC_FOLDERS:
	if(HIWORD(wParam) == CBN_SELCHANGE){
		// Get cur selection
		int iIndex = SendMessage(GetDlgItem(hWnd,IDC_FOLDERS),CB_GETCURSEL,0,0);
		thePrefab.PopulateFiles(iIndex);
	}
	break;

case IDC_FILES:
	if(HIWORD(wParam) == LBN_SELCHANGE){
		// Get cur selection
		int iIndex = SendMessage(GetDlgItem(hWnd,IDC_FILES),LB_GETCURSEL,0,0);
		thePrefab.FileSelected(iIndex);
	}
	break;

case IDCANCEL:
	EndDialog(hWnd,0);
	return (TRUE);
	}
default:
	break;
	}
	return FALSE;
}	

PrefabInterface::PrefabInterface()
{
	curFile = curFolder = -1;
	ip = NULL;	
	hwnd = NULL;
}

PrefabInterface::~PrefabInterface()
{
}

void PrefabInterface::Init(){
	ip = GetCOREInterface();
}


void PrefabInterface::CloseDialog() 
{
	if(!hwnd) // We've already closed once
		return;

	CommitSettings();
	//	EndDialog(hwnd,0);
	hwnd = NULL;
}