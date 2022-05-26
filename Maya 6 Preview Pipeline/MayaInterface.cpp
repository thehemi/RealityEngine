//-----------------------------------------------------------------------------
// INCLUDE FILES
//-----------------------------------------------------------------------------
#include "dxstdafx.h"

#include "d3d9.h"
#include "d3dx9.h"

#pragma warning( push )
#pragma warning( disable: 4995 ) // strsafe triggers warnings for STL
#pragma warning( disable: 4005 ) // strsafe triggers warnings for tchar.h
#include "MayaInterface.h"
#pragma warning( pop )

#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <assert.h>
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <wchar.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>
#include <tchar.h>
#include <strsafe.h>

// Maya API
#include <maya/MPxNode.h>
#include <maya/MPxHwShaderNode.h>
#include <maya/MString.h>
#include <maya/MTypeId.h>
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MArrayDataHandle.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnLightDataAttribute.h>
#include <maya/MFnMessageAttribute.h>
#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFloatVector.h>
#include <maya/MFnPlugin.h>
#include <maya/MFnStringData.h>
#include <maya/MFnDoubleArrayData.h>
#include <maya/MSceneMessage.h>
#include <maya/MNodeMessage.h>
#include <maya/MDagMessage.h>
#include <maya/MDGMessage.h>
#include <maya/MUiMessage.h>
#include <maya/MEventMessage.h>
#include <maya/MModelMessage.h>
#include <maya/MSelectionList.h>
#include <maya/MItDag.h>
#include <maya/MDagPathArray.h>
#include <maya/MIntArray.h>
#include <maya/MItDependencyNodes.h>
#include <maya/MFnCamera.h>
#include <maya/MTime.h>
#include <maya/MMatrix.h>
#include <maya/MItSelectionList.h>
#include <maya/MItDependencyGraph.h>

#define DXCC_EXPORTER "X-File"
#define DXCC_SHADER "DirectXShader"
#define DXCC_PREVIEW_TOGGLE "DXCCPreviewToggle"
#define DXCC_GATHER_ANIMATION "DXCCGatherAnimation"
#define DXCC_REBUILD_SCENE "DXCCRebuildScene"
#define DXCC_REBUILD_SELECTED "DXCCRebuildSelected"
#define DXCC_REBUILD_DIRTY "DXCCRebuildDirty"
#define DXCC_EXPORT_SCENE "DXCCExportScene"
#define DXCC_EXPORT_OPTIONS "DXCCExportOptions"
#define DXCC_EXPORT_SELECTED "DXCCExportSelected"
#define DXCC_VIEW_FLOATING "DXCCFloatingView"
#define DXCC_VIEW_TOP "DXCCTopView"
#define DXCC_VIEW_SIDE "DXCCSideView"
#define DXCC_VIEW_FRONT "DXCCFrontView"
#define DXCC_VIEW_PERSP "DXCCPerspectiveView"

MTypeId  DirectXShader::id( 0x68038 );
MObject  DirectXShader::aDXCCCommand;
MObject  DirectXShader::aDXCCHandle;
MObject  DirectXShader::aColor;
MObject  DirectXShader::aColorR;
MObject  DirectXShader::aColorG;
MObject  DirectXShader::aColorB;
MObject  DirectXShader::aOutColor;
MObject  DirectXShader::aOutColorR;
MObject  DirectXShader::aOutColorG;
MObject  DirectXShader::aOutColorB;
MObject  DirectXShader::aNormalCamera;
MObject  DirectXShader::aNormalCameraX;
MObject  DirectXShader::aNormalCameraY;
MObject  DirectXShader::aNormalCameraZ;


HANDLE	DirectXShader::FxDialogEvent= NULL;

//-----------------------------------------------------------------------------
// MAYA PLUGIN FUNCTIONS
//-----------------------------------------------------------------------------

MString MakeNameExportable(MString &in)
{
	const char *inConv= in.asChar();
	char outConv[MAX_PATH];
	for(UINT i= 0; i < in.length() && i < MAX_PATH; i++)
	{
		if(isalnum( inConv[i] )//alphanumeric//A – Z, a – z, or 0 – 9
		|| (inConv[i] == '_'))
			outConv[i]= inConv[i];
		else
			outConv[i]= '_';
	}

	if(in.length() < MAX_PATH)
		outConv[in.length()]= '\0';
	else
		outConv[MAX_PATH-1]= '\0';
	
	return MString(outConv);
}



void CMayaPreviewPipeline::Callback_NodeParentingChange( MDagMessage::DagMessage dagMsg, MDagPath &child, MDagPath &parent, void * clientData )
{
	g_PreviewPipeline.Tag_Reparent( dagMsg, child, parent, true );
	return;
}

void CMayaPreviewPipeline::Callback_NodeAttributeChanged( MNodeMessage::AttributeMessage msg,
	MPlug & plug,
	MPlug & otherPlug,
	void* clientData )
{
	g_PreviewPipeline.Tag_Dirty(msg, plug, otherPlug);
	return;
}




void CMayaPreviewPipeline::Callback_NodeNameChange( MObject & node, void* clientData )
{
	g_PreviewPipeline.Tag_Rename(node);
	return;
}

void CMayaPreviewPipeline::Callback_SceneIdle( void* clientData )
{
	g_PreviewPipeline.Scene_Update(false);
	return;
}

//add resourced on newed item (not loaded)
void CMayaPreviewPipeline::Callback_NodeAdded( MObject & node, void* clientData )
{
	g_PreviewPipeline.Tag_Add(  node );
	return;
}


//void CMayaPreviewPipeline::Callback_NodeRemoved( MObject & node, MDGModifier & modifier, void* clientData )
void CMayaPreviewPipeline::Callback_NodeRemoved( MObject & node, void* clientData )
{
	g_PreviewPipeline.Tag_Remove(node);
	return;
}

void CMayaPreviewPipeline::Callback_SceneSave(void* clientData )
{
	CPipelineLock SceneLock;
	g_PreviewPipeline.SceneReadLock(true, SceneLock);

	for(MItDependencyNodes iter; !iter.isDone(); iter.next())
	{
		MFnDependencyNode depNode(iter.item());
		if(depNode.typeId() == DirectXShader::id)
		{
			g_PreviewPipeline.Material_Save(depNode.object());
		}
	}

	g_PreviewPipeline.SceneReadUnlock(SceneLock);
}

void CMayaPreviewPipeline::Callback_ScenePreload(void* clientData )
{
	g_PreviewPipeline.UI_SetPreviewState(false);
	g_PreviewPipeline.Scene_Deregister();

	g_PreviewPipeline.CallbackId_AfterOpen= MSceneMessage::addCallback( MSceneMessage::kAfterOpen, CMayaPreviewPipeline::Callback_ScenePostload, NULL, NULL);
}

//add resourced on scene update or reload
void CMayaPreviewPipeline::Callback_ScenePostload(void* clientData )
{
	MMessage::removeCallback(g_PreviewPipeline.CallbackId_AfterOpen);

	g_PreviewPipeline.Scene_Register();

	CPipelineLock SceneLock;
	g_PreviewPipeline.SceneWriteLock(true, SceneLock);

	for(MItDependencyNodes iter; !iter.isDone(); iter.next())
	{
		MFnDependencyNode depNode(iter.item());
		if(depNode.typeId() == DirectXShader::id)
		{
			g_PreviewPipeline.Material_Restore(depNode.object());
		}
	}

	g_PreviewPipeline.Scene_Update(TRUE);

	g_PreviewPipeline.SceneWriteUnlock(SceneLock);


};


MStatus DCCRebuildDirtyCommand::doIt ( const MArgList& Arg)
{
	MStatus stat;

	g_PreviewPipeline.Scene_Update(false);

	return stat;
}


MStatus DCCRebuildSelectedCommand::doIt ( const MArgList& Arg)
{

	MStatus stat;
	MSelectionList	list;
	MGlobal::getActiveSelectionList(list);
	
	DXCC_DPF_REPORT(TEXT("REFRESH SELECTION"));
	MGlobal::displayInfo("DirectX Rebuilding Selected (currently previewed items only)");

	for (UINT i=0; i < list.length(); i++)
	{
		MDagPath dagPath;
		UINT iRemove;

		list.getDagPath(i, dagPath);

		if(g_PreviewPipeline.TagContainer.Find(dagPath, &iRemove, NULL))
		{
			g_PreviewPipeline.TagContainer.Erase(iRemove);
		}

		g_PreviewPipeline.TagContainer.Add(dagPath, NULL, NULL);

	}
	g_PreviewPipeline.Scene_Update(true);


	return stat;
}

MStatus DXCCRebuildSceneCommand::doIt ( const MArgList& Arg)
{

	MStatus stat = MS::kSuccess;

	DXCC_DPF_REPORT(TEXT("REFRESH SCENE"));
	MGlobal::displayInfo("DirectX Rebuilding Scene");

	g_PreviewPipeline.Scene_Deregister();
	g_PreviewPipeline.Scene_Register();



	return stat;
}

MStatus CMayaPreviewPipeline::BindViewerToPanel (const char* strView)
{
	//HRESULT hr= S_OK;
	HWND renderwnd= NULL;
	MDagPath MayaCamera;

	if(strView == NULL)
		strView= "";

	StringCchCopyA(m_ViewerBinding, MAX_PATH, strView);

	if(strView && (strView[0] != '\0'))
	{
		if(0 == lstrcmpiA(strView, "floating"))
		{
			g_Viewer.BindToWindow(NULL, true); 
		}
		else
		{
			M3dView ourView;
			M3dView::get3dView(0,ourView);

			for(UINT iView= 0; iView < M3dView::numberOf3dViews(); iView++)
			{
				M3dView::get3dView(iView, ourView);
				ourView.getCamera(MayaCamera);
				MayaCamera.pop();
				if(MayaCamera.partialPathName() == MString(strView))
				{
					renderwnd= (HWND)ourView.window();
					g_Viewer.BindToWindow(ourView.window(), true); 
					break;
				}
			}
		}
	}

//e_Exit:

	return MS::kSuccess;
}

MStatus DXCCGatherAnimationCommand::doIt ( const MArgList& Arg )
{
	MStatus stat= MS::kSuccess;

	DXCC_ASSERT_EXCEPTIONS_BEGIN()


	g_PreviewPipeline.Scene_Update_Animation();

	DXCC_ASSERT_EXCEPTIONS_END()

	return stat;
}


MStatus DXCCPreviewToggleCommand::doIt ( const MArgList& Arg )
{
	MStatus stat= MS::kSuccess;

	DXCC_ASSERT_EXCEPTIONS_BEGIN()


	if(1 <= Arg.length())
	{
		bool bPreview= Arg.asBool(0);
		g_PreviewPipeline.UI_SetPreviewState(bPreview);
	}

	DXCC_ASSERT_EXCEPTIONS_END()

	return stat;
}

MStatus DXCCExportSelectedCommand::doIt ( const MArgList& Arg)
{
	HRESULT hr= S_OK;
	OPENFILENAMEA open;
	ZeroMemory(&open, sizeof(open));
	char szFilename[MAX_PATH]= "";
	open.lStructSize= sizeof(OPENFILENAME); 
	open.hwndOwner= (HWND)M3dView::applicationShell();
	open.hInstance= (HINSTANCE)GetWindowInstance(open.hwndOwner); 
	open.lpstrFilter= "X-File\0*.x\0"; 
	open.lpstrFile= szFilename;
	open.nMaxFile= sizeof(szFilename);
	open.Flags=OFN_CREATEPROMPT|OFN_OVERWRITEPROMPT;



	if(GetSaveFileNameA(&open))
	{
		char* pExtension= strrchr(szFilename, '.');
		if(!pExtension || (0 != _stricoll(".x", pExtension)))
		{
			StringCchCatA(szFilename,MAX_PATH, ".x");
		};

		hr= g_PreviewPipeline.Scene_Export(szFilename, NULL, MPxFileTranslator::kSaveAccessMode);
	}

	return MS::kSuccess;
}

MStatus DXCCExportOptionsCommand::doIt ( const MArgList& Arg)
{
	g_PreviewPipeline.GetExportOptions(true);
	return MS::kSuccess;
}

MStatus DXCCExportSceneCommand::doIt ( const MArgList& Arg)
{

	HRESULT hr= S_OK;
	OPENFILENAMEA open;
	ZeroMemory(&open, sizeof(open));
	char szFilename[MAX_PATH]= "";
	open.lStructSize= sizeof(OPENFILENAME); 
	open.hwndOwner= (HWND)M3dView::applicationShell();
	open.hInstance= (HINSTANCE)GetWindowInstance(open.hwndOwner); 
	open.lpstrFilter= "X-File\0*.x\0"; 
	open.lpstrFile= szFilename;
	open.nMaxFile= sizeof(szFilename);
	open.Flags=OFN_CREATEPROMPT|OFN_OVERWRITEPROMPT;



	if(GetSaveFileNameA(&open))
	{
		char* pExtension= strrchr(szFilename, '.');
		if(!pExtension || (0 != _stricoll(".x", pExtension)))
		{
			StringCchCatA(szFilename, MAX_PATH, ".x");
		};

		hr= g_PreviewPipeline.Scene_Export(szFilename, NULL, MPxFileTranslator::kSaveAccessMode);
	}

	return MS::kSuccess;
}

MStatus DXCCFrontViewCommand::doIt ( const MArgList& Arg)
{
	MStatus stat;
	stat= g_PreviewPipeline.BindViewerToPanel( "front");
	return stat;
}

MStatus DXCCSideViewCommand::doIt ( const MArgList& Arg)
{
	MStatus stat;
	stat= g_PreviewPipeline.BindViewerToPanel( "side");
	return stat;
}

MStatus DXCCTopViewCommand::doIt ( const MArgList& Arg)
{
	MStatus stat;
	stat= g_PreviewPipeline.BindViewerToPanel( "top");
	return stat;
}

MStatus DXCCPerspectiveViewCommand::doIt ( const MArgList& Arg)
{
	MStatus stat;
	stat= g_PreviewPipeline.BindViewerToPanel( "persp");
	return stat;
}

MStatus DXCCFloatingViewCommand::doIt ( const MArgList& Arg)
{
	MStatus stat;
	stat= g_PreviewPipeline.BindViewerToPanel("floating");
	return stat;
}

XFileTranslator::~XFileTranslator()
{
	return;
};	

//-----------------------------------------------------------------------------
// Name: initializePlugin()
// Desc: Initializes the Maya XFile Exporter plugin
//-----------------------------------------------------------------------------

MStatus	
initializePlugin(MObject obj)
{


	MStatus stat = MS::kSuccess;

	DXCC_ASSERT_EXCEPTIONS_BEGIN()


	TCHAR szPlugin[MAX_PATH];
	TCHAR PluginFilePath[MAX_PATH];
	TCHAR* PluginFileName= NULL;

	HMODULE hPluginModule= GetModuleHandle(TEXT(THISFILENAME));
	DWORD OldLength= GetEnvironmentVariable(TEXT("PATH"),NULL,0);
	DWORD ExtLength= GetModuleFileName(hPluginModule, szPlugin, MAX_PATH);
	GetFullPathName(szPlugin, MAX_PATH, PluginFilePath, &PluginFileName);
	PluginFileName[0]= TEXT('\0');
	ExtLength= _tcslen(PluginFilePath)+1;

	DWORD NewLength= OldLength + ExtLength + 2;
	LPTSTR szPath= (LPTSTR)_alloca( sizeof(TCHAR) * NewLength );

	if(NewLength > GetEnvironmentVariable(TEXT("PATH") , szPath , NewLength))
	{
		if(szPath[OldLength-2] != TEXT(';'))
		{
			szPath[OldLength-1]= TEXT(';');
			szPath[OldLength]= TEXT('\0');
			OldLength++;
		}

		if(StringCchCopy( szPath+OldLength-1, ExtLength, PluginFilePath))
		{
			szPath[OldLength+ExtLength-1]= TEXT(';');
			szPath[OldLength+ExtLength]= TEXT('\0');
		}


   		SetEnvironmentVariable(TEXT("PATH"), szPath);
	}




	MFnPlugin plugin(obj, "Microsoft", "5.0", "Any");
	const MString DirectXShader_UserClassify( "shader/surface" );
	MArgList args;

#ifdef _DEBUG
	MGlobal::startErrorLogging("c:\\D3DMayaExportXErrorLog.txt");
#endif

	g_PreviewPipeline.CallbackId_Exiting= MSceneMessage::addCallback( MSceneMessage::kMayaExiting, CMayaPreviewPipeline::Callback_MayaExiting, NULL, NULL);
	
	//Menu Creation
	MGlobal::executeCommand("source DirectX", true, true);


	HWND hShell= (HWND)M3dView::applicationShell();
	HMENU hMenu= GetMenu(hShell);
	for(int iMenu= 0; iMenu < GetMenuItemCount(hMenu); iMenu++)
	{
		char szMenu[MAX_PATH];
		MENUITEMINFOA info;
		ZeroMemory(&info, sizeof(MENUITEMINFO));
		info.cbSize= sizeof(MENUITEMINFO);
		info.fMask= MIIM_STRING|MIIM_SUBMENU;
		info.dwTypeData= szMenu;
		info.cch= MAX_PATH;

		GetMenuItemInfoA(hMenu, iMenu, TRUE, &info);
		szMenu[MAX_PATH-1]= '\0';

		if(0 == lstrcmpiA(szMenu, "DirectX"))
		{
			HMENU hDirectXMenu= info.hSubMenu ;
			for(iMenu= 0; iMenu < GetMenuItemCount(hDirectXMenu); iMenu++)
			{
				ZeroMemory(&info, sizeof(MENUITEMINFO));
				info.cbSize= sizeof(MENUITEMINFO);
				info.fMask= MIIM_STRING|MIIM_SUBMENU;
				info.dwTypeData= szMenu;
				info.cch= MAX_PATH;

				GetMenuItemInfoA(hDirectXMenu, iMenu, TRUE, &info);
				szMenu[MAX_PATH-1]= '\0';

				if(0 == lstrcmpiA(szMenu, "Rebuild Dirty")) 
				{
					g_PreviewPipeline.m_RebuildDirty_wparam= (WPARAM)MAKELONG(GetMenuItemID(hDirectXMenu, iMenu), 0);
					g_PreviewPipeline.m_RebuildDirtyTimer= SetTimer(NULL, NULL, 66, CMayaPreviewPipeline::TimerProc);
					break;
				}
			}
			break;
		}
	}

	g_PreviewPipeline.Create();

	stat= plugin.registerNode( 
		DXCC_SHADER, 
		DirectXShader::id, 
		DirectXShader::creator, 
		DirectXShader::initialize, 
		MPxNode::kDependNode, 
		&DirectXShader_UserClassify );
	CHECK_MSTATUS(stat);

	// register the translator
	stat = plugin.registerFileTranslator(DXCC_EXPORTER,			// name
										 NULL,		// icon
										 XFileTranslator::creator,	
										 NULL,		// script
										 NULL,
										 false);
	CHECK_MSTATUS(stat);

	CHECK_MSTATUS(stat= plugin.registerCommand( DXCC_PREVIEW_TOGGLE ,  DXCCPreviewToggleCommand::creator ));
	CHECK_MSTATUS(stat= plugin.registerCommand( DXCC_GATHER_ANIMATION,  DXCCGatherAnimationCommand::creator ));
	CHECK_MSTATUS(stat= plugin.registerCommand( DXCC_REBUILD_SCENE,  DXCCRebuildSceneCommand::creator ));
	CHECK_MSTATUS(stat= plugin.registerCommand( DXCC_REBUILD_SELECTED,  DCCRebuildSelectedCommand::creator ));
	CHECK_MSTATUS(stat= plugin.registerCommand( DXCC_REBUILD_DIRTY,  DCCRebuildDirtyCommand::creator ));
	CHECK_MSTATUS(stat= plugin.registerCommand( DXCC_EXPORT_SCENE,  DXCCExportSceneCommand::creator ));
	CHECK_MSTATUS(stat= plugin.registerCommand( DXCC_EXPORT_OPTIONS,  DXCCExportOptionsCommand::creator ));
	CHECK_MSTATUS(stat= plugin.registerCommand( DXCC_EXPORT_SELECTED,  DXCCExportSelectedCommand::creator ));
	CHECK_MSTATUS(stat= plugin.registerCommand( DXCC_VIEW_FLOATING,  DXCCFloatingViewCommand::creator ));
	CHECK_MSTATUS(stat= plugin.registerCommand( DXCC_VIEW_FRONT,  DXCCFrontViewCommand::creator ));
	CHECK_MSTATUS(stat= plugin.registerCommand( DXCC_VIEW_SIDE,  DXCCSideViewCommand::creator ));
	CHECK_MSTATUS(stat= plugin.registerCommand( DXCC_VIEW_TOP,  DXCCTopViewCommand::creator ));
	CHECK_MSTATUS(stat= plugin.registerCommand( DXCC_VIEW_PERSP,  DXCCPerspectiveViewCommand::creator ));


	//Scene Registration ... 
	g_PreviewPipeline.Scene_Register();

	D3DPRESENT_PARAMETERS pp = {0};
	pp.BackBufferWidth = 640;
	pp.BackBufferHeight = 480;
	pp.BackBufferCount = 1;
	pp.MultiSampleType = D3DMULTISAMPLE_NONE;
	pp.MultiSampleQuality = 0;
	pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	pp.hDeviceWindow = NULL;
	pp.Windowed = true;
	pp.EnableAutoDepthStencil = true;
	pp.AutoDepthStencilFormat = D3DFMT_D16;
	pp.FullScreen_RefreshRateInHz = 0;
	pp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

	g_PreviewPipeline.g_Viewer.Create( &g_PreviewPipeline, &pp );


	g_PreviewPipeline.LoadUIOptions();

	
	DXCC_ASSERT_EXCEPTIONS_END()

	return stat;
};


//-----------------------------------------------------------------------------
// Name: uninitializePlugin()
// Desc: Uninitializes the Maya XFile Exporter plugin
//-----------------------------------------------------------------------------


MStatus	
uninitializePlugin(MObject obj) 
{
	MStatus stat = MS::kSuccess;
	try
	{
		MFnPlugin plugin(obj);

		CMayaPreviewPipeline::Callback_MayaExiting(NULL);

		stat = plugin.deregisterNode( DirectXShader::id );
		stat = plugin.deregisterFileTranslator(DXCC_EXPORTER);
		stat = plugin.deregisterCommand(DXCC_PREVIEW_TOGGLE);
		stat = plugin.deregisterCommand(DXCC_GATHER_ANIMATION);
		stat = plugin.deregisterCommand(DXCC_REBUILD_SCENE);
		stat = plugin.deregisterCommand(DXCC_REBUILD_SELECTED);
		stat = plugin.deregisterCommand(DXCC_REBUILD_DIRTY);
		stat = plugin.deregisterCommand(DXCC_EXPORT_SCENE);
		stat = plugin.deregisterCommand(DXCC_EXPORT_SELECTED);
		stat = plugin.deregisterCommand(DXCC_VIEW_FLOATING);
		stat = plugin.deregisterCommand(DXCC_VIEW_TOP);
		stat = plugin.deregisterCommand(DXCC_VIEW_SIDE);
		stat = plugin.deregisterCommand(DXCC_VIEW_FRONT);
		stat = plugin.deregisterCommand(DXCC_VIEW_PERSP);
	
		MGlobal::executeCommand("DirectX_RemoveMenu()");

	}
	catch( ... )
	{
	}

	return stat;
}


void CMayaPreviewPipeline::Callback_MayaExiting( void* clientData )
{
		CPipelineLock Lock;

		g_PreviewPipeline.SaveUIOptions();


		//if we get this lock then we are ready to end because we 
		g_PreviewPipeline.SceneWriteLock(true, Lock); 
		
		g_PreviewPipeline.g_Viewer.Destroy();

		g_PreviewPipeline.Destroy();
	
		g_PreviewPipeline.SceneWriteUnlock(Lock); 


	return;
}


//-----------------------------------------------------------------------------
// XFILETRANSLATOR MEMBER FUNCTIONS
//-----------------------------------------------------------------------------
 void*		XFileTranslator::creator()					
{ 
	return new XFileTranslator(); 
};

MStatus		XFileTranslator::reader(const MFileObject& file, const MString& optionsString, MPxFileTranslator::FileAccessMode mode) 
{ 
	return MS::kSuccess; 
};

bool		XFileTranslator::haveReadMethod() const		
{ 
	return false; 
};

bool		XFileTranslator::haveWriteMethod() const		
{ 
	return true; 
};

MString 	XFileTranslator::defaultExtension() const	
{ 
	return "x"; 
}

MString 	XFileTranslator::filter() const				
{ 
	return "*.x"; 
}

MPxFileTranslator::MFileKind 
XFileTranslator::identifyFile(	
	const MFileObject& fileName,
	const char* buffer,
	short size) const
{ 
	char *extension= strchr(fileName.name().asChar(), '.');
	if(0==_stricoll(".x", extension))
	{
		return kIsMyFileType;
	}
	else
	{
		return kNotMyFileType; 
	}
}

MStatus	
XFileTranslator::writer(
			const MFileObject& mfoXFile,			// save file object
			const MString& msOptions,				// options string
			MPxFileTranslator::FileAccessMode mode)	// options string
{
	HRESULT hr = S_OK;
	MStatus stat = MS::kSuccess;

	MString	msFile= mfoXFile.fullName();
	char* pExtension= strrchr(msFile.asChar(), '.');
	if(!pExtension || (0 != _stricoll(".x", pExtension)))
	{
		msFile += ".x";
	};

	hr= g_PreviewPipeline.Scene_Export(msFile.asChar(), msOptions.asChar(), mode);
	if(FAILED(hr))
	{
		stat= MS::kFailure;
		goto e_Exit;
	};

e_Exit:
	return stat;
}


void* DirectXShader::creator()
{
	return new DirectXShader();
}


// The initialize method is called only once when the node is first
// registered with Maya. In this method you define the attributes of the
// node, what data comes in and goes out of the node that other nodes may
// want to connect to. 
//

MStatus DirectXShader::initialize()
{
	MFnNumericAttribute nAttr; 
	MFnTypedAttribute tAttr; 

	MStatus status; 

	if(DirectXShader::FxDialogEvent == NULL)
	{
		DirectXShader::FxDialogEvent= CreateEventA(NULL, TRUE, TRUE, "FxDialogEvent");
		SetEvent(DirectXShader::FxDialogEvent);
	}


	// Input Attributes
	aColorR = nAttr.create( "colorR", "cr",MFnNumericData::kFloat, 0, &status );
	CHECK_MSTATUS( status );
	CHECK_MSTATUS( nAttr.setKeyable( true ) );
	CHECK_MSTATUS( nAttr.setStorable( true ) );
	CHECK_MSTATUS( nAttr.setDefault( 0.0f ) );

	aColorG = nAttr.create( "colorG", "cg", MFnNumericData::kFloat, 0, &status );
	CHECK_MSTATUS( status );
	CHECK_MSTATUS( nAttr.setKeyable( true ) );
	CHECK_MSTATUS( nAttr.setStorable( true ) );
	CHECK_MSTATUS( nAttr.setDefault( 0.58824f ) );

	aColorB = nAttr.create( "colorB", "cb",MFnNumericData::kFloat, 0, &status );
	CHECK_MSTATUS( status );
	CHECK_MSTATUS( nAttr.setKeyable( true ) );
	CHECK_MSTATUS( nAttr.setStorable( true ) );
	CHECK_MSTATUS( nAttr.setDefault( 0.644f ) );

	aColor = nAttr.create( "color", "c", aColorR, aColorG, aColorB, &status );
	CHECK_MSTATUS( status );
	CHECK_MSTATUS( nAttr.setKeyable( true ) );
	CHECK_MSTATUS( nAttr.setStorable( true ) );
	CHECK_MSTATUS( nAttr.setDefault( 0.0f, 0.58824f, 0.644f ) );
	CHECK_MSTATUS( nAttr.setUsedAsColor( true ) );

	aDXCCCommand = tAttr.create( "Command", "DXC",  MFnData::kString,  MObject::kNullObj , &status );
	CHECK_MSTATUS( status );
	CHECK_MSTATUS( tAttr.setKeyable( false ) );
	CHECK_MSTATUS( tAttr.setStorable( false ) );
	CHECK_MSTATUS( tAttr.setInternal( true ) );
#ifndef _DEBUG
	CHECK_MSTATUS( nAttr.setHidden( true ) );
#endif

	aDXCCHandle = nAttr.create( DX_LONG_FXHANDLE, DX_SHORT_FXHANDLE, MFnNumericData::kInt, 0, &status );
	CHECK_MSTATUS( status );
	CHECK_MSTATUS( nAttr.setKeyable( false ) );
	CHECK_MSTATUS( nAttr.setStorable( false ) );
	CHECK_MSTATUS( nAttr.setInternal( false ) );
	CHECK_MSTATUS( nAttr.setHidden( true ) );

	// Color Output
	aOutColorR = nAttr.create( "outColorR", "ocr", MFnNumericData::kFloat, 0, &status );
	CHECK_MSTATUS( status );
	
	aOutColorG = nAttr.create( "outColorG", "ocg", MFnNumericData::kFloat, 0, &status );
	CHECK_MSTATUS( status );
	
	aOutColorB = nAttr.create( "outColorB", "ocb", MFnNumericData::kFloat, 0, &status );
	CHECK_MSTATUS( status );
	
	aOutColor = nAttr.create( "outColor", "oc", aOutColorR, aOutColorG, aOutColorB, &status );
	CHECK_MSTATUS( status );
	
	CHECK_MSTATUS( nAttr.setHidden( false ) );
	CHECK_MSTATUS( nAttr.setReadable( true ) );
	CHECK_MSTATUS( nAttr.setWritable( false ) );

	// Camera Normals
	aNormalCameraX = nAttr.create( "normalCameraX", "nx", MFnNumericData::kFloat, 0, &status );
	CHECK_MSTATUS( status );
	CHECK_MSTATUS( nAttr.setStorable( false ) );
	CHECK_MSTATUS( nAttr.setDefault( 1.0f ) );

	aNormalCameraY = nAttr.create( "normalCameraY", "ny", MFnNumericData::kFloat, 0, &status );
	CHECK_MSTATUS( status );
	CHECK_MSTATUS( nAttr.setStorable( false ) );
	CHECK_MSTATUS( nAttr.setDefault( 1.0f ) );

	aNormalCameraZ = nAttr.create( "normalCameraZ", "nz", MFnNumericData::kFloat, 0, &status );
	CHECK_MSTATUS( status );
	CHECK_MSTATUS( nAttr.setStorable( false ) );
	CHECK_MSTATUS( nAttr.setDefault( 1.0f ) );

	aNormalCamera = nAttr.create( "normalCamera","n", aNormalCameraX, aNormalCameraY, aNormalCameraZ, &status );
	CHECK_MSTATUS( status );
	CHECK_MSTATUS( nAttr.setStorable( false ) );
	CHECK_MSTATUS( nAttr.setDefault( 1.0f, 1.0f, 1.0f ) );
	CHECK_MSTATUS( nAttr.setHidden( true ) );


	// Next we will add the attributes we have defined to the node
	CHECK_MSTATUS( addAttribute( aDXCCCommand ) );
	CHECK_MSTATUS( addAttribute( aDXCCHandle ) );
	CHECK_MSTATUS( addAttribute( aColor ) );
	CHECK_MSTATUS( addAttribute( aOutColor ) );
	CHECK_MSTATUS( addAttribute( aNormalCamera ) );

	CHECK_MSTATUS( attributeAffects( aColorR, aOutColor ) );
	CHECK_MSTATUS( attributeAffects( aColorG, aOutColor ) );
	CHECK_MSTATUS( attributeAffects( aColorB, aOutColor ) );
	CHECK_MSTATUS( attributeAffects( aColor, aOutColor ) );
	CHECK_MSTATUS( attributeAffects( aNormalCameraX, aOutColor ) );
	CHECK_MSTATUS( attributeAffects( aNormalCameraY, aOutColor ) );
	CHECK_MSTATUS( attributeAffects( aNormalCameraZ, aOutColor ) );
	CHECK_MSTATUS( attributeAffects( aNormalCamera, aOutColor ) );

	return( MS::kSuccess );
}

void DirectXShader::postConstructor( )
{
	HRESULT hr= S_OK; 
	MStatus status= MStatus::kSuccess; 
	int iHandle= 0;
	DXCCHANDLE hHandle= NULL;

	LPDXCCRESOURCE pResource= NULL;

	setMPSafe( true );

	MObject object= thisMObject();
	MPlug plug(object, aDXCCHandle);

	hr= g_PreviewPipeline.AccessManager()->CreateResource(NULL, IID_NULL, false, &pResource);
	if(DXCC_FAILED(hr))
		DXCC_GOTO_EXIT(e_Exit, TRUE);

	g_PreviewPipeline.TriggerManagerEvents.OnResourceAdd(pResource);

	CHECK_MSTATUS( plug.setValue((INT)pResource->GetHandle()) );

e_Exit:
	DXCC_RELEASE(pResource);

	return;
}

bool DirectXShader::setInternalValue( const MPlug& plug, const MDataHandle& dataIn)
{
	HRESULT hr= S_OK;
	MStatus status= MStatus::kSuccess; 

	if((plug == aDXCCCommand) && GetSingleObject(false, DirectXShader::FxDialogEvent))
	{
		ResetEvent(DirectXShader::FxDialogEvent);

		//BOOL bModal;
		int iHandle;
		DXCCHANDLE hHandle;
		LPDIRECT3DDEVICE9 pDevice= NULL;
		LPDXCCRESOURCE pResource= NULL;
		MString& mayaCommand= dataIn.asString();
		const char* strCommand= mayaCommand.asChar();

		MPlug plugMaterial(plug.node(), aDXCCHandle);
		plugMaterial.getValue(iHandle);
		hHandle= (DXCCHANDLE)iHandle;

		hr= g_PreviewPipeline.AccessManager()->FindResourceByHandle(hHandle, &pResource);
		if(DXCC_FAILED(hr))
			DXCC_GOTO_EXIT(e_Exit, TRUE);


		if(0==lstrcmpiA(strCommand, "open"))
		{

			hr= g_PreviewPipeline.AccessEngine()->GetD3DDevice(&pDevice);
			if(DXCC_FAILED(hr))
				DXCC_GOTO_EXIT(e_Exit, TRUE);
	

			hr= DXCCShowEffectDialog(
					(HWND)M3dView::applicationShell(),
					pDevice,
					g_PreviewPipeline.AccessManager(),
					pResource,
					this, 
					TRUE,
					FALSE);
			if(DXCC_FAILED(hr))
			{
				SetEvent(DirectXShader::FxDialogEvent);
				DXCC_GOTO_EXIT(e_Exit, TRUE);
			}
		}
		else
		{
			SetEvent(DirectXShader::FxDialogEvent);
		}

e_Exit:
		DXCC_RELEASE(pDevice);
		DXCC_RELEASE(pResource);


		return TRUE;
	}
	else
	{
		return MPxNode::setInternalValue(plug, dataIn);
	}
}


HRESULT DirectXShader::OnPostLoad( 
	HWND hDialog, 
	LPDIRECT3DDEVICE9 pDevice,
	LPDXCCMANAGER pManager,
	LPDXCCRESOURCE pResource,//recycle me
	LPCSTR szNewEffectFilename) 
{
	CPipelineLock SceneLock;

	g_PreviewPipeline.SceneReadLock(true, SceneLock);

	bool FoundInDirtyBin= false;
	for(UINT i = 0; i < g_PreviewPipeline.DirtyShaders.size(); i++)
	{
		if(g_PreviewPipeline.DirtyShaders[i]==pResource->GetHandle())
		{
			FoundInDirtyBin=true;
			break;
		}
	}
	if(!FoundInDirtyBin)
	{
		g_PreviewPipeline.DirtyShaders.push_back(pResource->GetHandle());
	}

	g_PreviewPipeline.SceneReadUnlock(SceneLock);

	return S_OK;
}

HRESULT DirectXShader::OnClose(  
	HWND hDialog, 
 	LPDIRECT3DDEVICE9 pDevice,
 	LPDXCCMANAGER pManager,
 	LPDXCCRESOURCE pResource)
{
	SetEvent(DirectXShader::FxDialogEvent);

	return S_OK; 
}

MStatus DirectXShader::compute( const MPlug& plug, MDataBlock& block ) 
{ 
	// The plug parameter will allow us to determine which output attribute
	// needs to be calculated.
	//
	if( plug == aOutColor
			|| plug == aOutColorR
			|| plug == aOutColorG
			|| plug == aOutColorB)
	{
		MStatus status= MS::kSuccess;
		MFloatVector resultColor( 0.0, 0.0, 0.0 ); 

	
		// Get surface shading parameters from input block
		//
		MFloatVector& surfaceNormal = block.inputValue( aNormalCamera,
				&status ).asFloatVector();
		CHECK_MSTATUS( status );

		MFloatVector& surfaceColor = block.inputValue( aColor,
				&status ).asFloatVector();
		CHECK_MSTATUS( status );

		// Factor incident light with surface color and add incandescence
		//

		float dot= -surfaceNormal[1];
		if(dot < 0) dot = 0;
		resultColor[0] = dot * surfaceColor[0];
		resultColor[1] = dot * surfaceColor[1];
		resultColor[2] = dot * surfaceColor[2];


		if ( plug == aOutColor || plug == aOutColorR || plug == aOutColorG
				|| plug == aOutColorB)
		{
			// Get the handle to the attribute
			//
			MDataHandle outColorHandle = block.outputValue( aOutColor,
					&status );
			CHECK_MSTATUS( status );
			MFloatVector& outColor = outColorHandle.asFloatVector();
		
			outColor = resultColor;		// Set the output value
			outColorHandle.setClean(); // Mark the output value as EraseInvalidTags
		}
	} 
	else
	{
		return( MS::kUnknownParameter ); // We got an unexpected plug
	}

	return( MS::kSuccess );
}




