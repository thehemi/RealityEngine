/**********************************************************************
 *<
	FILE: IGameExporter.cpp

	DESCRIPTION:	Sample to test the IGame Interfaces.  It follows a similar format
					to the Ascii Exporter.  However is does diverge a little in order
					to handle properties etc..

	TODO: Break the file down into smaller chunks for easier loading.

	CREATED BY:		Neil Hazzard	

	HISTORY:		Tim Johnson rewrites for Reality Engine

	TODO: Don't export materials that aren't used on the exported geometry!

 *>	Copyright (c) 2004, All Rights Reserved.
 **********************************************************************/
#include "stdafx.h"
#include "decomp.h"
#include "IGameCustom.h"

float gWorldScale = 1; // Converts to meters

//----------------------------------------------------------------------------------
// 
//----------------------------------------------------------------------------------
#define IGAMEEXPORTER_CLASS_ID	Class_ID(0x79d613a4, 0x4f21c3ad)

#define BEZIER	0
#define TCB		1
#define LINEAR	2
#define SAMPLE	3

HRESULT ExportFile
    (
    const TCHAR *szFilename,
    INode* pRootNode,
    Interface *pInterface, 
    BOOL bSuppressPrompts,
    BOOL bSaveSelection,
    SaveOptions& options
	)
{
	IGameExporter exp;
	exp.DoExport(szFilename,NULL,pInterface,bSuppressPrompts,bSaveSelection?SCENE_EXPORT_SELECTED:0);

	return S_OK;
}

 


void stripWhiteSpace(TSTR * buf, TCHAR &newBuf)
{

	TCHAR newb[256]={""};
	strcpy(newb,buf->data());

	int len = strlen(newb);

	int index = 0;

	for(int i=0;i<len;i++)
	{
		//if((newb[i] != ' ') && (!ispunct(newb[i])))
			(&newBuf)[index++] = newb[i];
	}
}

//--- IGameExporter -------------------------------------------------------
IGameExporter::IGameExporter()
{

	staticFrame = 0;
	framePerSample = 4;
	exportGeom = TRUE;
	exportNormals = TRUE;
	exportVertexColor = FALSE;
	exportControllers = FALSE;
	exportFaceSmgp = TRUE;
	exportTexCoords = TRUE;
	exportMappingChannel = TRUE;
	exportMaterials = TRUE;
	exportConstraints = FALSE;
	exportSplines = FALSE;
	exportModifiers = FALSE;
	forceSample = FALSE;
	exportSkin = TRUE;
	exportGenMod = TRUE; // skinning
	cS = IGameConversionManager::IGAME_D3D; // Coord system
	pRoot = NULL;
	pXMLDoc = NULL;
	splitFile = FALSE;
	exportQuaternions = FALSE;
	exportObjectSpace = TRUE;
	
//	int a = IGameConversionManager::IGAME_MAX;
//	int b = IGameConversionManager::IGAME_OGL;
//	int c = IGameConversionManager::IGAME_D3D;

}

IGameExporter::~IGameExporter() 
{
	iGameNode = NULL;
	if(pRoot)
		pRoot->Release(); 
	pRoot = NULL;
	if(pXMLDoc)
		pXMLDoc->Release();

	tempNode = NULL;
	pXMLDoc = NULL;
}

void IGameExporter::MakeSplitFilename(IGameNode * node, TSTR & buf)
{
	buf = splitPath;
	buf = buf + "\\" + node->GetName() + ".buf";
	
}

void IGameExporter::ExportChildNodeInfo(CComPtr<IXMLDOMNode> parent, IGameNode * child)
{
	IGameKeyTab poskeys;
	IGameKeyTab rotkeys;
	IGameKeyTab scalekeys;
	TSTR buf,data;

	CComPtr <IXMLDOMNode> prsNode,group;
	CComPtr <IXMLDOMNode> matIndex;
	CComPtr <IXMLDOMNode> geomData = NULL,splineData,ikData;
	CComPtr <IXMLDOMNode> tmNodeParent;

	IXMLDOMDocument * pSubDocMesh, * pSubDocCont;
	CComPtr <IXMLDOMNode> pRootSubCont,pRootSubMesh,pSubCont,pSubMesh;

	struct tm *newtime;
	time_t aclock;

	time( &aclock );
	newtime = localtime(&aclock);

	TSTR today = _tasctime(newtime);	// The date string has a \n appended.
	today.remove(today.length()-1);		// Remove the \n



	if(splitFile)
	{
		CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER,  IID_IXMLDOMDocument, (void**)&pSubDocCont);
		CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER,  IID_IXMLDOMDocument, (void**)&pSubDocMesh);
		pSubDocCont->QueryInterface(IID_IXMLDOMNode, (void **)&pRootSubCont);
		pSubDocMesh->QueryInterface(IID_IXMLDOMNode, (void **)&pRootSubMesh);
		CreateXMLNode(pXMLDoc, pRootSubCont, _T("IGame"), &pSubCont);
		AddXMLAttribute(pSubCont, _T("Version"), _T("1.0"));
		AddXMLAttribute(pSubCont, _T("Date"), today.data());

		CreateXMLNode(pXMLDoc, pRootSubMesh, _T("IGame"), &pSubMesh);
		AddXMLAttribute(pSubMesh, _T("Version"), _T("1.0"));
		AddXMLAttribute(pSubMesh, _T("Date"), today.data());

	}

	curNode ++;
	buf = TSTR("Processing: ") + TSTR(child->GetName());
	GetCOREInterface()->ProgressUpdate((int)((float)curNode/pIgame->GetTotalNodeCount()*100.0f),FALSE,buf.data()); 


	if(child->IsGroupOwner())
	{
		TSTR b(child->GetName());
		TCHAR nb[256]={""};
		stripWhiteSpace(&b,*nb);
		AddXMLAttribute(parent,_T("GroupName"),nb);
		buf.printf("%d",child->GetNodeID());
		AddXMLAttribute(parent,_T("NodeID"),buf.data());
		buf.printf("%d",child->GetChildCount());
		AddXMLAttribute(parent,_T("NumberInGroup"),buf.data());
	}
	else if(!child->IsNodeHidden())
	{
		TSTR b(child->GetName());
		TCHAR nb[256]={""};
		stripWhiteSpace(&b,*nb);
		AddXMLAttribute(parent,_T("Name"),nb);
		buf.printf("%d",child->GetNodeID());
		AddXMLAttribute(parent,_T("NodeID"),buf.data());

		IGameNode * p = child->GetNodeParent();
		if(p)
		{
			buf.printf("%d",p->GetNodeID());
			AddXMLAttribute(parent,_T("ParentNodeID"),buf.data());
		}

		DumpNodeProperties(child->GetMaxNode(),parent);

		CreateXMLNode(pXMLDoc,parent,_T("NodeTM"),&tmNodeParent);

		IGameObject * obj = child->GetIGameObject();
		Matrix3 mat = child->/*GetWorldTM*/GetObjectTM(staticFrame).ExtractMatrix3();

		// TIM: No scale on matrices that are exported, all built in
		if(obj->GetIGameType() == IGameObject::IGAME_MESH)
		{
			BOOL negScale = mat.Parity();//TMNegParity(mat);
			if(!negScale)
			{
				// Remove scale
				mat.Orthogonalize();
				mat.NoScale();
			}
			else{
				// Remove scale+rot
				mat = TransMatrix(mat.GetRow(3));
			}
		}

		DumpMatrix(mat,tmNodeParent);

		ULONG  handle = child->GetMaxNode()->GetHandle();

		if(child->GetMaterialIndex() !=-1){
			CreateXMLNode(pXMLDoc,parent,_T("MaterialIndex"),&matIndex);
			buf.printf("%d",m_MatOffset[child->GetMaterialIndex()]);
			AddXMLText(pXMLDoc,matIndex,buf.data());
		}
		

		IGameObject::MaxType T = obj->GetMaxType();

		bool xref = obj->IsObjectXRef();

		if(xref)
		{
			AddXMLAttribute(parent,_T("XRefObject"),_T("True"));
		}

		switch(obj->GetIGameType())
		{
			case IGameObject::IGAME_BONE:
			{
	//			CComPtr <IXMLDOMNode> boneNode;
	//			CreateXMLNode(pXMLDoc,parent,_T("BoneData"),&boneNode);
				AddXMLAttribute(parent,_T("NodeType"),_T("Bone"));
				IGameSupportObject * hO = (IGameSupportObject*)obj;
				IGameMesh * hm = hO->GetMeshObject();
				if(hm->InitializeData())
				{
					CreateXMLNode(pXMLDoc,parent,_T("GeomData"),&geomData);
					if(splitFile)
					{
						TSTR filename;
						MakeSplitFilename(child,filename);
						CComPtr<IXMLDOMNode>subMeshNode;						
						AddXMLAttribute(geomData, _T("Include"),filename.data());
						CreateXMLNode(pSubDocMesh,pSubMesh,_T("GeomData"),&subMeshNode);
						AddXMLAttribute(subMeshNode,_T("Node"),child->GetName());
						geomData = subMeshNode;

					}
					DumpMesh(hm,geomData);
				}

				break;
			}

			case IGameObject::IGAME_HELPER:
			{
	//			CComPtr <IXMLDOMNode> boneNode;
	//			CreateXMLNode(pXMLDoc,parent,_T("BoneData"),&boneNode);
				AddXMLAttribute(parent,_T("NodeType"),_T("Helper"));
				IGameSupportObject * hO = (IGameSupportObject*)obj;
				IPropertyContainer * cc = hO->GetIPropertyContainer();
				IGameProperty * prop = cc->QueryProperty(101);
				if(prop)
				{
					TCHAR * buf;
					prop->GetPropertyValue(buf);
				}
				prop = cc->QueryProperty(_T("IGameTestString"));

				if(prop)
				{
					TCHAR * buf;
					prop->GetPropertyValue(buf);
				}
				prop = cc->QueryProperty(_T("IGameTestString"));

				IGameMesh * hm = hO->GetMeshObject();
				if(hm->InitializeData())
				{
					CreateXMLNode(pXMLDoc,parent,_T("GeomData"),&geomData);
					if(splitFile)
					{
						TSTR filename;
						MakeSplitFilename(child,filename);
						CComPtr<IXMLDOMNode>subMeshNode;						
						AddXMLAttribute(geomData, _T("Include"),filename.data());
						CreateXMLNode(pSubDocMesh,pSubMesh,_T("GeomData"),&subMeshNode);
						AddXMLAttribute(subMeshNode,_T("Node"),child->GetName());
						geomData = subMeshNode;

					}
					DumpMesh(hm,geomData);
				}


				break;
			}
			case IGameObject::IGAME_LIGHT:
			{
				CComPtr <IXMLDOMNode> lightNode;
				CreateXMLNode(pXMLDoc,parent,_T("LightData"),&lightNode);

				AddXMLAttribute(parent,_T("NodeType"),_T("Light"));
				IGameLight * l = (IGameLight*)obj;
				if(l->GetLightType()==IGameLight::IGAME_OMNI)
					AddXMLAttribute(lightNode,_T("Type"),_T("Omni"));
				else if(l->GetLightType()==IGameLight::IGAME_TSPOT)
					AddXMLAttribute(lightNode,_T("Type"),_T("Targeted"));
				else if(l->GetLightType()==IGameLight::IGAME_FSPOT)
					AddXMLAttribute(lightNode,_T("Type"),_T("Free"));
				else if(l->GetLightType()==IGameLight::IGAME_TDIR)
					AddXMLAttribute(lightNode,_T("Type"),_T("TargetedDirectional"));
				else
					AddXMLAttribute(lightNode,_T("Type"),_T("Directional"));
				DumpLight(l,lightNode);
				break;
			}
			case IGameObject::IGAME_CAMERA:
			{
				CComPtr <IXMLDOMNode> camNode;
				CreateXMLNode(pXMLDoc,parent,_T("CameraData"),&camNode);
				AddXMLAttribute(parent,_T("NodeType"),_T("Camera"));
				IGameCamera * c = (IGameCamera*)obj;
				if(c->GetCameraTarget())
					AddXMLAttribute(camNode,_T("Type"),_T("Targeted"));
				else
					AddXMLAttribute(camNode,_T("Type"),_T("Free"));
				DumpCamera(c,camNode);
				break;
			}

			case IGameObject::IGAME_MESH:
				if(exportGeom )
				{
					bool NodeNeedsRecompiling(NodeData& node, string file);
					AddXMLAttribute(parent,_T("NodeType"),_T("GeomObject"));
					IGameMesh * gM = (IGameMesh*)obj;
					CComPtr<IXMLDOMNode>subMeshNode;
					// Get node data 
					NodeData nData;
					bool hasScript = GetNodeData(child->GetMaxNode(),nData);
					TSTR filename;
					MakeSplitFilename(child,filename);
					// Save Model?
					if(hasScript && nData.script.bIncludeModel == false){
						// Do nothing
					}
					// TIM: FUCK YOU STUPID MOTHER FUCKER COCKSUCKER
					// See if node needs recompiling
					/*else if(!NodeNeedsRecompiling(nData,filename.data())){
						// Doesn't, just write out include
						CreateXMLNode(pXMLDoc,parent,_T("GeomData"),&geomData);
						//AddXMLAttribute(geomData, _T("Optimized"),false);
						AddXMLAttribute(geomData, _T("Include"),filename.data());
					}*/
					else if(gM->InitializeData())
					{
						CreateXMLNode(pXMLDoc,parent,_T("GeomData"),&geomData);
						//AddXMLAttribute(geomData, _T("Optimized"),false);
						if(splitFile)
						{
							AddXMLAttribute(geomData, _T("Include"),filename.data());
							CreateXMLNode(pSubDocMesh,pSubMesh,_T("GeomData"),&subMeshNode);
							AddXMLAttribute(subMeshNode,_T("Node"),child->GetName());
							geomData = subMeshNode;
							DumpMesh(gM,geomData);
						}
						else{
							AddXMLAttribute(geomData, _T("Include"),filename.data());
							DumpMeshBinary(child,geomData,child->GetMaxNode(),gM,filename);
							//DumpMesh(gM,geomData);
						}
					}
					else
					{
						CreateXMLNode(pXMLDoc,parent,_T("GeomData"),&geomData);
						AddXMLAttribute(geomData,_T("Error"),_T("BadObject"));

					}

					if(geomData){
						extern TSTR GetTimeStamp(SYSTEMTIME time);
						NodeData nData;
						GetNodeData(child->GetMaxNode(),nData);
						Node(geomData,"Time");
						Attrib("Moved",GetTimeStamp(nData.timeMoved));
						Attrib("Modified",GetTimeStamp(nData.timeModified));

						// SH Stuff
						if(nData.bSHEnabled)
							DumpSHProperties(nData,geomData);
					}
						
				
				}
				break;
			case IGameObject::IGAME_SPLINE:
				if(exportSplines)
				{
					AddXMLAttribute(parent,_T("NodeType"),_T("SplineObject"));
					IGameSpline * sp = (IGameSpline*)obj;
					sp->InitializeData();
					CreateXMLNode(pXMLDoc,parent,_T("SplineData"),&splineData);
					DumpSpline(sp,splineData);
				}
				break;

			case IGameObject::IGAME_IKCHAIN:
				{
				AddXMLAttribute(parent,_T("NodeType"),_T("IKChainObject"));
				IGameIKChain * ikch = (IGameIKChain*)obj;
				CreateXMLNode(pXMLDoc,parent,_T("IKChain"), & ikData);
				DumpIKChain(ikch,ikData);
				}
				break;
			default:
				{
				AddXMLAttribute(parent,_T("NodeType"),_T("Unknown"));
				}
				break;
				
		}

		if(splitFile)
		{
			TSTR buf;
			MakeSplitFilename(child,buf);
			pSubDocMesh->save(CComVariant(buf.data()));

		}
		//dump PRS Controller data
		prsNode = NULL;

		// In our "Game Engine" we deal with Bezier Position, Scale and TCB Rotation controllers !!
		// Only list controllers on position, rotation...
		if(exportControllers)
		{
			CComPtr<IXMLDOMNode>subContNode;
			bool exportBiped = false;
			CreateXMLNode(pXMLDoc,parent,_T("PRSData"),&prsNode);
			IGameControl * pGameControl = child->GetIGameControl();
			ExportControllers(prsNode,pGameControl);

/*			IGameControl::MaxControlType T = pGameControl->GetControlType(IGAME_POS);

			//position
			if(T==IGameControl::IGAME_MAXSTD && pGameControl->GetBezierKeys(poskeys,IGAME_POS) && !forceSample)
				DumpBezierKeys(IGAME_POS,poskeys,prsNode);
			else if(T==IGameControl::IGAME_POS_CONSTRAINT && !forceSample)
			{
				IGameConstraint * cnst = pGameControl->GetConstraint(IGAME_POS);
				DumpConstraints(prsNode,cnst);
			}
			else if(T==IGameControl::IGAME_LIST && !forceSample)
			{
				DumpListController(pGameControl,prsNode);

			}
			else if(T==IGameControl::IGAME_BIPED)
				exportBiped = true;
			else
			{
				if(forceSample || T==IGameControl::IGAME_UNKNOWN)
					DumpSampleKeys(child->GetIGameControl(),prsNode,IGAME_POS);
			}
				
		
			//rotation
			T = pGameControl->GetControlType(IGAME_ROT);

			if(T==IGameControl::IGAME_MAXSTD && pGameControl->GetTCBKeys(rotkeys,IGAME_ROT) && !forceSample)
				DumpTCBKeys(IGAME_ROT,rotkeys,prsNode);

			else if(T==IGameControl::IGAME_ROT_CONSTRAINT && !forceSample)
			{
				IGameConstraint * cnst = pGameControl->GetConstraint(IGAME_ROT);
				DumpConstraints(prsNode,cnst);
			}
			else if(T==IGameControl::IGAME_LIST&& !forceSample)
			{
				DumpListController(pGameControl,prsNode);

			}
			else if(T==IGameControl::IGAME_EULER&& !forceSample)
			{
				DumpEulerController(pGameControl,prsNode);
			}

			else if(T==IGameControl::IGAME_BIPED)
				exportBiped = true;

			else
			{
				if(forceSample || T==IGameControl::IGAME_UNKNOWN)
					DumpSampleKeys(child->GetIGameControl(),prsNode, IGAME_ROT);
			}

			//scale
			T = pGameControl->GetControlType(IGAME_SCALE);

			if(T==IGameControl::IGAME_MAXSTD && pGameControl->GetBezierKeys(scalekeys,IGAME_SCALE) && !forceSample)
				DumpBezierKeys(IGAME_SCALE,scalekeys,prsNode);
			else if(T==IGameControl::IGAME_BIPED)
				exportBiped = true;
			else
			{
				if(forceSample || T==IGameControl::IGAME_UNKNOWN)
					DumpSampleKeys(child->GetIGameControl(),prsNode, IGAME_SCALE);
			}

			if(exportBiped)
				DumpSampleKeys(child->GetIGameControl(),prsNode,IGAME_TM);

  */
		}

		if(exportModifiers)
		{
			int numMod = obj->GetNumModifiers();
			if(numMod > 0)
			{
				CComPtr <IXMLDOMNode> mod;
				CreateXMLNode(pXMLDoc,parent,_T("Modifiers"),&mod);
				TSTR Buf;
				buf.printf("%d",numMod);
				AddXMLAttribute(mod,_T("count"),buf.data());

				for(int i=0;i<numMod;i++)
				{
					IGameModifier * m = obj->GetIGameModifier(i);
					DumpModifiers(mod,m);
				}
			}
		}
	}	
	for(int i=0;i<child->GetChildCount();i++)
	{
		CComPtr <IXMLDOMNode> cNode = NULL;
		IGameNode * newchild = child->GetNodeChild(i);
		TSTR name;
		if(newchild->IsGroupOwner())
			name = TSTR("Group");
		else
			name = TSTR("Node");

//		stripWhiteSpace(&name,*buf);
		CreateXMLNode(pXMLDoc,parent,name.data(),&cNode);

		// we deal with targets in the light/camera section
		if(newchild->IsTarget())
			continue;

		ExportChildNodeInfo(cNode,newchild);
	}

	child->ReleaseIGameObject();

	prsNode = NULL;
	group = NULL;

}


void IGameExporter::ExportControllers(CComPtr<IXMLDOMNode> node, IGameControl * pGameControl)
{
	ExportPositionControllers(node,pGameControl);
	ExportRotationControllers(node,pGameControl);
	ExportScaleControllers(node,pGameControl);
}

void IGameExporter::ExportPositionControllers(CComPtr<IXMLDOMNode> node, IGameControl * pGameControl)
{
	IGameKeyTab poskeys;
	bool exportBiped = false;


	IGameControl::MaxControlType T = pGameControl->GetControlType(IGAME_POS);

	//position
	if(T==IGameControl::IGAME_MAXSTD && pGameControl->GetBezierKeys(poskeys,IGAME_POS) && !forceSample)
		DumpBezierKeys(IGAME_POS,poskeys,node);
	else if(T==IGameControl::IGAME_POS_CONSTRAINT && !forceSample)
	{
		IGameConstraint * cnst = pGameControl->GetConstraint(IGAME_POS);
		DumpConstraints(node,cnst);
	}
	else if(T==IGameControl::IGAME_LIST && !forceSample)
	{
		DumpListController(pGameControl,node);

	}
	else if(T==IGameControl::IGAME_BIPED)
		exportBiped = true;
	else
	{
		if(forceSample || T==IGameControl::IGAME_UNKNOWN)
			DumpSampleKeys(pGameControl,node,IGAME_POS);
	}
	if(exportBiped)
		DumpSampleKeys(pGameControl,node,IGAME_TM);
		
}

void IGameExporter::ExportRotationControllers(CComPtr<IXMLDOMNode> node, IGameControl * pGameControl)
{

	IGameKeyTab rotkeys;
	bool exportBiped = false;
	bool exported = false;		//this will at least export something !!

	//rotation
	IGameControl::MaxControlType T = pGameControl->GetControlType(IGAME_ROT);

	if(T==IGameControl::IGAME_MAXSTD && pGameControl->GetTCBKeys(rotkeys,IGAME_ROT) && !forceSample)
	{
		DumpTCBKeys(IGAME_ROT,rotkeys,node);
		exported = true;
	}

	else if(T==IGameControl::IGAME_ROT_CONSTRAINT && !forceSample)
	{
		IGameConstraint * cnst = pGameControl->GetConstraint(IGAME_ROT);
		DumpConstraints(node,cnst);
		exported = true;
	}
	else if(T==IGameControl::IGAME_LIST&& !forceSample)
	{
		DumpListController(pGameControl,node);
		exported = true;

	}
	else if(T==IGameControl::IGAME_EULER&& !forceSample)
	{
		DumpEulerController(pGameControl,node);
		exported = true;
	}

	else if(T==IGameControl::IGAME_BIPED)
	{
		exportBiped = true;
		exported = true;
	}

	else
	{
		if(forceSample || !exported || T==IGameControl::IGAME_UNKNOWN)
			DumpSampleKeys(pGameControl,node, IGAME_ROT);
	}
	if(exportBiped)
		DumpSampleKeys(pGameControl,node,IGAME_TM);
}

void IGameExporter::ExportScaleControllers(CComPtr<IXMLDOMNode> node, IGameControl * pGameControl)
{
	IGameKeyTab scalekeys;
	bool exportBiped = false;


	//scale
	IGameControl::MaxControlType T= pGameControl->GetControlType(IGAME_SCALE);

	if(T==IGameControl::IGAME_MAXSTD && pGameControl->GetBezierKeys(scalekeys,IGAME_SCALE) && !forceSample)
		DumpBezierKeys(IGAME_SCALE,scalekeys,node);
	else if(T==IGameControl::IGAME_BIPED)
		exportBiped = true;
	else
	{
		if(forceSample || T==IGameControl::IGAME_UNKNOWN)
			DumpSampleKeys(pGameControl,node, IGAME_SCALE);
	}

	if(exportBiped)
		DumpSampleKeys(pGameControl,node,IGAME_TM);


	
}

void IGameExporter::ExportNodeInfo(IGameNode * node)
{
		
	rootNode = NULL;
	TCHAR buf[256]={""};
	TSTR name;
	if(node->IsGroupOwner())
		name = TSTR("Group");
	else
		name = TSTR("Node");
	
	stripWhiteSpace(&name,*buf);
	CreateXMLNode(pXMLDoc,iGameNode,name.data(),&rootNode);
	ExportChildNodeInfo(rootNode,node);

}


void GetKeyTypeName(TSTR &name, DWORD Type)
{
	if(Type==IGAME_POS)
		name.printf("Position");
	else if(Type==IGAME_ROT)
		name.printf("Rotation");
	else if(Type==IGAME_POINT3)
		name.printf("Point3");
	else if(Type==IGAME_FLOAT)
		name.printf("float");
	else if(Type==IGAME_SCALE)
		name.printf("Scale");
	else if(Type == IGameConstraint::IGAME_PATH)
		name.printf("Path");
	else if(Type == IGameConstraint::IGAME_POSITION)
		name.printf("Position");
	else if(Type == IGameConstraint::IGAME_ORIENTATION)
		name.printf("Orientation");
	else if (Type == IGameConstraint::IGAME_LOOKAT)
		name.printf("lookAt");
	else if(Type == IGAME_TM)
		name.printf("NodeTM");
	else if(Type == IGAME_EULER_X)
		name.printf("EulerX");
	else if(Type == IGAME_EULER_Y)
		name.printf("EulerY");
	else if(Type == IGAME_EULER_Z)
		name.printf("EulerZ");


	else
		name.printf("Huh!!");

}

void IGameExporter::DumpEulerController(IGameControl * sc,CComPtr<IXMLDOMNode> prsNode)
{
	TSTR data;
	CComPtr <IXMLDOMNode> eulerNode = NULL;

	IGameKeyTab xCont,yCont,zCont;

	if(sc->GetBezierKeys(xCont,IGAME_EULER_X))
	{
		if(eulerNode ==NULL)
			CreateXMLNode(pXMLDoc,prsNode,_T("EulerController"),&eulerNode);
		DumpBezierKeys(IGAME_EULER_X,xCont,eulerNode);
	}

	if(sc->GetBezierKeys(yCont,IGAME_EULER_Y))
	{
		if(eulerNode ==NULL)
			CreateXMLNode(pXMLDoc,prsNode,_T("EulerController"),&eulerNode);
		DumpBezierKeys(IGAME_EULER_Y,yCont,eulerNode);
	}

	if(sc->GetBezierKeys(zCont,IGAME_EULER_Z))
	{
		if(eulerNode ==NULL)
			CreateXMLNode(pXMLDoc,prsNode,_T("EulerController"),&eulerNode);
		DumpBezierKeys(IGAME_EULER_Z,zCont,eulerNode);
	}

}

void IGameExporter::DumpListController(IGameControl * sc,CComPtr<IXMLDOMNode> prsNode)
{
	TSTR data;
	CComPtr <IXMLDOMNode> listNode;
	CreateXMLNode(pXMLDoc,prsNode,_T("ListController"),&listNode);

	int subNum = sc->GetNumOfListSubControls(IGAME_ROT);
	data.printf("%d",subNum);
	AddXMLAttribute(listNode,_T("NumSubController"),data.data());

	for(int ii=0;ii<subNum;ii++)
	{
		IGameKeyTab SubCont;
		IGameControl * sub = sc->GetListSubControl(ii,IGAME_ROT);
		ExportRotationControllers(listNode,sub);

/*		if(sub->GetBezierKeys(SubCont,IGAME_ROT))
		{
			DumpBezierKeys(IGAME_ROT,SubCont,listNode);
		}
		if(sub->GetTCBKeys(SubCont,IGAME_ROT))
		{
			DumpTCBKeys(IGAME_ROT,SubCont,listNode);
		}
*/

	}


}

void IGameExporter::DumpSampleKeys(IGameControl * sc,CComPtr<IXMLDOMNode> prsNode, DWORD Type, bool quick)
{
	Tab<Matrix3>sKey;  
	Tab<GMatrix>gKey;
	IGameKeyTab Key;
	IGameControl * c = sc;
	CComPtr<IXMLDOMNode> sampleData;
	TSTR Buf;

	TSTR name;
	GetKeyTypeName(name,Type);
	sampleData = NULL;

	if(!c)
		return;

	if(!quick && c->GetFullSampledKeys(Key,framePerSample,IGameControlType(Type)) )
	{

		CreateXMLNode(pXMLDoc,prsNode,name,&sampleData);
		Buf.printf("%d",Key.Count());
		AddXMLAttribute(sampleData,_T("KeyCount"),Buf.data());
		AddXMLAttribute(sampleData,_T("Type"),_T("FullSampled"));
		Buf.printf("%d",framePerSample);
		AddXMLAttribute(sampleData,_T("SampleRate"),Buf.data());

		for(int i=0;i<Key.Count();i++)
		{
			CComPtr<IXMLDOMNode> data;
			CreateXMLNode(pXMLDoc,sampleData,_T("Sample"),&data);
			int fc = Key[i].t;
			Buf.printf("%d",fc);
			AddXMLAttribute(data,_T("frame"),Buf.data());

			if(Type ==IGAME_POS)
			{
				Point3 k = Key[i].sampleKey.pval;
				Buf.printf("%f %f %f",k.x,k.y,k.z); 
				AddXMLText(pXMLDoc,data,Buf.data());
			}
			if(Type == IGAME_ROT)
			{
				Quat q = Key[i].sampleKey.qval;
				AngAxis a(q);
				if(!exportQuaternions)
					Buf.printf("%f %f %f %f",a.axis.x, a.axis.y, a.axis.z, a.angle);
				else
					Buf.printf("%f %f %f %f",q.x,q.y, q.z, q.w);

				AddXMLText(pXMLDoc,data,Buf.data());

			}
			if(Type == IGAME_SCALE)
			{
				ScaleValue sval = Key[i].sampleKey.sval;
				Point3 s = sval.s;
				AngAxis a(sval.q);
				if(!exportQuaternions)
					Buf.printf("%f %f %f %f %f %f %f",sval.s.x,sval.s.y,sval.s.z,a.axis.x, a.axis.y, a.axis.z, a.angle);
				else
					Buf.printf("%f %f %f %f %f %f %f",sval.s.x,sval.s.y,sval.s.z,sval.q.x, sval.q.y,sval.q.z, sval.q.w);
				AddXMLText(pXMLDoc,data,Buf.data());

			}
			if(Type == IGAME_FLOAT)
			{
				float f = Key[i].sampleKey.fval;
				Buf.printf("%f",f);	
				AddXMLText(pXMLDoc,data,Buf.data());
			}
			if(Type == IGAME_TM)
			{
				//Even though its a 4x4 we dump it as a 4x3 ;-)
				GMatrix m;
				DumpMatrix(Key[i].sampleKey.gval.ExtractMatrix3(),data);
			}
		}
	}

	//mainly for the IK On/Off controller
	if(quick && c->GetQuickSampledKeys(Key,IGameControlType(Type)) )
	{

		CreateXMLNode(pXMLDoc,prsNode,name,&sampleData);
		Buf.printf("%d",Key.Count());
		AddXMLAttribute(sampleData,_T("KeyCount"),Buf.data());
		AddXMLAttribute(sampleData,_T("Type"),_T("QuickSampled"));

		for(int i=0;i<Key.Count();i++)
		{
			CComPtr<IXMLDOMNode> data;
			CreateXMLNode(pXMLDoc,sampleData,_T("Sample"),&data);
			int fc = Key[i].t;
			Buf.printf("%d",fc);
			AddXMLAttribute(data,_T("frame"),Buf.data());
			if(Type == IGAME_FLOAT)
			{
				float f = Key[i].sampleKey.fval;
				Buf.printf("%f",f);	
				AddXMLText(pXMLDoc,data,Buf.data());
			}

		}
	}

}
void IGameExporter::DumpSkin(CComPtr<IXMLDOMNode> modNode, IGameSkin * s)
{
	CComPtr <IXMLDOMNode> skinNode;
	TSTR Buf;

	if(s->GetSkinType()== IGameSkin::IGAME_PHYSIQUE)
		Buf.printf("%s",_T("Physique"));
	else
		Buf.printf("%s",_T("MaxSkin"));

	AddXMLAttribute(modNode,_T("skinType"),Buf.data());

	GMatrix skinTM;

	s->GetInitSkinTM(skinTM);

	for(int x=0; x<s->GetNumOfSkinnedVerts();x++)
	{
		int type = s->GetVertexType(x);
		if(type==IGameSkin::IGAME_RIGID)
		{
			CComPtr <IXMLDOMNode> boneNode;
			skinNode = NULL;
			CreateXMLNode(pXMLDoc,modNode,_T("Skin"),&skinNode);
			Buf.printf("%d",x);
			AddXMLAttribute(skinNode,_T("vertexID"),Buf.data());
			AddXMLAttribute(skinNode,_T("Type"),_T("Rigid"));
			CreateXMLNode(pXMLDoc,skinNode,_T("Bone"),&boneNode);
			ULONG id = s->GetBoneID(x,0);
			Buf.printf("%d",id);
			AddXMLAttribute(boneNode,_T("BoneID"),Buf.data());

		}
		else //blended
		{
			CComPtr <IXMLDOMNode> boneNode;
			skinNode = NULL;
			CreateXMLNode(pXMLDoc,modNode,_T("Skin"),&skinNode);
			Buf.printf("%d",x);
			AddXMLAttribute(skinNode,_T("vertexID"),Buf.data());
			AddXMLAttribute(skinNode,_T("Type"),_T("Blended"));

			for(int y=0;y<s->GetNumberOfBones(x);y++)
			{
				boneNode = NULL;
				CreateXMLNode(pXMLDoc,skinNode,_T("Bone"),&boneNode);
				ULONG id = s->GetBoneID(x,y);
				Buf.printf("%d",id);
				AddXMLAttribute(boneNode,_T("BoneID"),Buf.data());
				float weight = s->GetWeight(x,y);
				Buf.printf("%f",weight);
				AddXMLAttribute(boneNode,_T("Weight"),Buf.data());
			}
		}
	}
}

void IGameExporter::DumpModifiers(CComPtr<IXMLDOMNode> modNode, IGameModifier * m)
{
	CComPtr <IXMLDOMNode> propNode;
	TSTR buf;
	if(exportSkin || exportGenMod)
	{
	
		CreateXMLNode(pXMLDoc,modNode,_T("Modifier"),&propNode);
		AddXMLAttribute(propNode,_T("modName"),m->GetInternalName());
		bool bS = m->IsSkin();
		if(bS)
			buf.printf(_T("true"));
		else
			buf.printf(_T("false"));
		AddXMLAttribute(propNode,_T("IsSkin"),buf.data());
		
		if(m->IsSkin() && exportSkin)
		{
			IGameSkin * skin = (IGameSkin*)m;
			DumpSkin(propNode,skin);
		}
	}

}

void IGameExporter::DumpCamera(IGameCamera *ca, CComPtr<IXMLDOMNode> parent)
{

	IGameProperty * prop;
	CComPtr <IXMLDOMNode> propNode,targNode;


	CreateXMLNode(pXMLDoc,parent,_T("Properties"),&propNode);
	prop = ca->GetCameraFOV();
	DumpProperties(propNode,prop);
	prop = ca->GetCameraFarClip();
	DumpProperties(propNode,prop);
	prop = ca->GetCameraNearClip();
	DumpProperties(propNode,prop);
	prop = ca->GetCameraTargetDist();
	DumpProperties(propNode,prop);
	
	if(ca->GetCameraTarget())
	{
		CreateXMLNode(pXMLDoc,parent,_T("Target"),&targNode);
		ExportChildNodeInfo(targNode,ca->GetCameraTarget());
	}
	

}

void IGameExporter::DumpIKChain(IGameIKChain * ikch, CComPtr<IXMLDOMNode> ikData)
{
	CComPtr <IXMLDOMNode> ikRoot,ikNode, ikEnabled;
	TSTR buf;

	CreateXMLNode(pXMLDoc,ikData,_T("IKNodes"),&ikRoot);
	buf.printf("%d",ikch->GetNumberofBonesinChain());
	AddXMLAttribute(ikRoot,_T("NumOfNodesInChain"),buf.data());

	for(int i=0;i<ikch->GetNumberofBonesinChain();i++)
	{
		ikNode = NULL;
		IGameNode * node = ikch->GetIGameNodeInChain(i);
		CreateXMLNode(pXMLDoc,ikRoot,_T("ChainNode"),&ikNode);
		buf.printf("%d",node->GetNodeID());
		AddXMLAttribute(ikNode,_T("NodeID"),buf.data());
		
	}

	IGameControl * cont = ikch->GetIKEnabledController();
	if(cont)
	{
		CreateXMLNode(pXMLDoc,ikData,_T("IKEnabled"),&ikEnabled);
		DumpSampleKeys(cont,ikEnabled,IGAME_FLOAT,true);
	}

	
}

void IGameExporter::DumpSpline(IGameSpline * sp,CComPtr<IXMLDOMNode> splineData)
{
	CComPtr <IXMLDOMNode> spline,knotData;
	TSTR buf;

	buf.printf("%d",sp->GetNumberOfSplines());
	AddXMLAttribute(splineData,_T("NumOfSplines"),buf.data());

	for(int i=0;i<sp->GetNumberOfSplines();i++)
	{
		IGameSpline3D * sp3d = sp->GetIGameSpline3D(i);
		spline=NULL;
		CreateXMLNode(pXMLDoc,splineData,_T("Spline"),&spline);
		buf.printf("%d",i+1);
		AddXMLAttribute(spline,_T("index"),buf.data());
		int num = sp3d->GetIGameKnotCount();
		buf.printf("%d",num);
		AddXMLAttribute(spline,_T("NumOfKnots"),buf.data());
		for(int j=0;j<num;j++)
		{
			TSTR data;
			Point3 v;
			CComPtr <IXMLDOMNode> point,invec,outvec;
			knotData=NULL;
			IGameKnot * knot = sp3d->GetIGameKnot(j);
			CreateXMLNode(pXMLDoc,spline,_T("knot"),&knotData);
			CreateXMLNode(pXMLDoc,knotData,_T("Point"),&point);
			v = knot->GetKnotPoint();
			data.printf("%f %f %f",v.x,v.y,v.z);
			AddXMLText(pXMLDoc,point,data.data());
			CreateXMLNode(pXMLDoc,knotData,_T("inVec"),&invec);
			v = knot->GetInVec();
			data.printf("%f %f %f",v.x,v.y,v.z);
			AddXMLText(pXMLDoc,invec,data.data());
			CreateXMLNode(pXMLDoc,knotData,_T("outVec"),&outvec);
			v = knot->GetOutVec();
			data.printf("%f %f %f",v.x,v.y,v.z);
			AddXMLText(pXMLDoc,outvec,data.data());

		}
	}

	IPropertyContainer * cc = sp->GetIPropertyContainer();
	IGameProperty * prop = cc->QueryProperty(_T("IGameTestString"));
	prop = cc->QueryProperty(_T("IGameTestString"));

	if(prop)
	{
		TCHAR * name;
		prop->GetPropertyValue(name);

	}

		

}




void IGameExporter::DumpConstraints(CComPtr<IXMLDOMNode> prsData, IGameConstraint * c)
{
	TSTR buf,name;
	CComPtr <IXMLDOMNode> prsChild;
	CComPtr <IXMLDOMNode> constNode;

	if(exportConstraints)
	{
	
		CreateXMLNode(pXMLDoc,prsData,_T("Constrainst"),&prsChild);
		GetKeyTypeName(name, c->GetConstraintType());
		AddXMLAttribute(prsChild,_T("Type"),name.data());
		buf.printf("%d",c->NumberOfConstraintNodes());
		AddXMLAttribute(prsChild,_T("NodeCount"),buf.data());

		for(int i=0;i<c->NumberOfConstraintNodes();i++ )
		{
			constNode = NULL;
			CreateXMLNode(pXMLDoc,prsChild,_T("ConstNode"),&constNode);
			float w = c->GetConstraintWeight(i);
			int id = c->GetConstraintNodes(i)->GetNodeID();
			
			buf.printf("%d", id);
			AddXMLAttribute(constNode,_T("NodeID"),buf.data());

			buf.printf("%f", w);
			AddXMLAttribute(constNode,_T("Weight"),buf.data());

		}
		for(i=0;i<c->GetIPropertyContainer()->GetNumberOfProperties();i++)
		{
			DumpProperties(prsChild,c->GetIPropertyContainer()->GetProperty(i));
		}
	}
}




void IGameExporter::DumpBezierKeys(DWORD Type, IGameKeyTab Keys, CComPtr<IXMLDOMNode> prsData)
{
	TSTR buf,name;
	CComPtr <IXMLDOMNode> prsChild;
	GetKeyTypeName(name,Type);

	if(Keys.Count()==0)
		return;

	CreateXMLNode(pXMLDoc,prsData,name,&prsChild);

	buf.printf("%d",Keys.Count());
	AddXMLAttribute(prsChild,_T("KeyCount"),buf.data());
	AddXMLAttribute(prsChild,_T("Type"),_T("Bezier"));

	for(int i = 0;i<Keys.Count();i++)
	{
		CComPtr <IXMLDOMNode> key = NULL;
		TSTR data;
		CreateXMLNode(pXMLDoc,prsChild,_T("Key"),&key);
		buf.printf("%d",Keys[i].t);
		AddXMLAttribute(key,_T("time"),buf.data());
		if(Type==IGAME_POS || Type==IGAME_POINT3)
		{
			Point3 k = Keys[i].bezierKey.pval;
			data.printf("%f %f %f",k.x,k.y,k.z); 
		}
		else if (Type == IGAME_ROT)
		{
			Quat q = Keys[i].bezierKey.qval;
			AngAxis a(q);
			if(!exportQuaternions)
				data.printf("%f %f %f %f",a.axis.x, a.axis.y, a.axis.z, a.angle);
			else
				data.printf("%f %f %f %f",q.x, q.y, q.z, q.w);

		}
		else if ( Type == IGAME_FLOAT || Type==IGAME_EULER_X || Type == IGAME_EULER_Y || Type==IGAME_EULER_Z)
		{
			float f = Keys[i].bezierKey.fval;
			data.printf("%f",f);
		}
		else{
			ScaleValue sval = Keys[i].bezierKey.sval;
			Point3 s = sval.s;
			AngAxis a(sval.q);
			if(!exportQuaternions)
				data.printf("%f %f %f %f %f %f %f",sval.s.x,sval.s.y,sval.s.z,a.axis.x, a.axis.y, a.axis.z, a.angle);
			else
				data.printf("%f %f %f %f %f %f %f",sval.s.x,sval.s.y,sval.s.z,sval.q.x, sval.q.y,sval.q.z, sval.q.w);

		}
		AddXMLText(pXMLDoc,key,data.data());
		
	}

	prsChild = NULL;

}
void IGameExporter::DumpLinearKeys(DWORD Type, IGameKeyTab Keys, CComPtr<IXMLDOMNode> prsData)
{
	TSTR buf,name;
	CComPtr <IXMLDOMNode> prsChild;
	GetKeyTypeName(name,Type);

	if(Keys.Count()==0)
		return;

	CreateXMLNode(pXMLDoc,prsData,name.data(),&prsChild);

	buf.printf("%d",Keys.Count());
	AddXMLAttribute(prsChild,_T("KeyCount"),buf.data());
	AddXMLAttribute(prsChild,_T("Type"),_T("Linear"));

	for(int i = 0;i<Keys.Count();i++)
	{
		CComPtr <IXMLDOMNode> key = NULL;
		TSTR data;
		CreateXMLNode(pXMLDoc,prsChild,_T("Key"),&key);
		buf.printf("%d",Keys[i].t);
		AddXMLAttribute(key,_T("time"),buf.data());
		if(Type==IGAME_POS)
		{
			Point3 k = Keys[i].linearKey.pval;
			data.printf("%f %f %f",k.x,k.y,k.z); 
		}
		else if (Type == IGAME_ROT)
		{
			Quat q = Keys[i].linearKey.qval;
			AngAxis a(q);
			if(!exportQuaternions)
				data.printf("%f %f %f %f",a.axis.x, a.axis.y, a.axis.z, a.angle);
			else
				data.printf("%f %f %f %f",q.x, q.y, q.z, q.w);

		}
		else if ( Type == IGAME_FLOAT || Type==IGAME_EULER_X || Type == IGAME_EULER_Y || Type==IGAME_EULER_Z)
		{
			float f = Keys[i].linearKey.fval;
			data.printf("%f",f);	
		}
		else{
			ScaleValue sval = Keys[i].linearKey.sval;
			Point3 s = sval.s;
			AngAxis a(sval.q);
			if(exportQuaternions)
				data.printf("%f %f %f %f %f %f %f",sval.s.x,sval.s.y,sval.s.z,a.axis.x, a.axis.y, a.axis.z, a.angle);
			else
				data.printf("%f %f %f %f %f %f %f",sval.s.x,sval.s.y,sval.s.z,sval.q.x, sval.q.y,sval.q.z, sval.q.w);

		}
		AddXMLText(pXMLDoc,key,data.data());
		
	}

	prsChild = NULL;


}
void IGameExporter::DumpTCBKeys(DWORD Type, IGameKeyTab Keys, CComPtr<IXMLDOMNode> prsData)
{
	TSTR buf,name;
	CComPtr <IXMLDOMNode> prsChild;

	if(Keys.Count()==0)
		return;

	GetKeyTypeName(name,Type);
	CreateXMLNode(pXMLDoc,prsData,name.data(),&prsChild);

	buf.printf("%d",Keys.Count());
	AddXMLAttribute(prsChild,_T("KeyCount"),buf.data());
	AddXMLAttribute(prsChild,_T("Type"),_T("TCB"));

	for(int i = 0;i<Keys.Count();i++)
	{
		CComPtr <IXMLDOMNode> key = NULL;
		TSTR data;
		CreateXMLNode(pXMLDoc,prsChild,_T("Key"),&key);
		buf.printf("%d",Keys[i].t);
		AddXMLAttribute(key,_T("time"),buf.data());
		if(Type==IGAME_POS)
		{
			Point3 k = Keys[i].tcbKey.pval;
			data.printf("%f %f %f",k.x,k.y,k.z); 
		}
		else if (Type == IGAME_ROT)
		{
			AngAxis a = Keys[i].tcbKey.aval;
			data.printf("%f %f %f %f",a.axis.x, a.axis.y, a.axis.z, a.angle);

		}
		else{
			ScaleValue sval = Keys[i].tcbKey.sval;
			Point3 s = sval.s;
			AngAxis a(sval.q);
			if(!exportQuaternions)
				data.printf("%f %f %f %f %f %f %f",sval.s.x,sval.s.y,sval.s.z,a.axis.x, a.axis.y, a.axis.z, a.angle);
			else
				data.printf("%f %f %f %f %f %f %f",sval.s.x,sval.s.y,sval.s.z,sval.q.x, sval.q.y,sval.q.z, sval.q.w);

		}
		AddXMLText(pXMLDoc,key,data.data());
		
	}

	prsChild = NULL;


}

void IGameExporter::DumpProperties(CComPtr<IXMLDOMNode> node, IGameProperty * prop, bool bCoordinate)
{
	TSTR Buf;
	IGameKeyTab keys;
	CComPtr <IXMLDOMNode> propData;
	CComPtr <IXMLDOMNode> keyNode;

	if(!prop)	//fix me NH...
		return;
	CreateXMLNode(pXMLDoc,node,_T("Prop"),&propData);
	AddXMLAttribute(propData,_T("name"),prop->GetName());

	if(prop->GetType() == IGAME_POINT3_PROP)
	{
		Point3 p; 
		prop->GetPropertyValue(p);
		if(bCoordinate)
			p *= gWorldScale;
		Buf.printf("%f %f %f",p.x,p.y,p.z);
	}
	else if( prop->GetType() == IGAME_FLOAT_PROP)
	{
		float f;
		prop->GetPropertyValue(f);
		if(bCoordinate)
			f *= gWorldScale;
		Buf.printf("%f", f);
	}
	else if(prop->GetType()==IGAME_STRING_PROP)
	{
		TCHAR * b;
		prop->GetPropertyValue(b);
		Buf.printf("$s",b);
	}
	else
	{
		int i;
		prop->GetPropertyValue(i);
		Buf.printf("%f", i);

	}
	AddXMLAttribute(propData,_T("value"),Buf.data());

	if(prop->IsPropAnimated() && exportControllers)
	{
		IGameControl * c = prop->GetIGameControl();
		CreateXMLNode(pXMLDoc,propData,_T("PropKeyData"),&keyNode);


		if(prop->GetType() == IGAME_POINT3_PROP)
		{
			if(	c->GetBezierKeys(keys,IGAME_POINT3 )){
				DumpBezierKeys(IGAME_POINT3,keys,keyNode);
			}
		}
		if(prop->GetType()==IGAME_FLOAT_PROP)
		{
			if(	c->GetBezierKeys(keys,IGAME_FLOAT )){
				DumpBezierKeys(IGAME_FLOAT,keys,keyNode);
			}
		}
	}
	propData = NULL;
	keyNode = NULL;
}	


void IGameExporter::DumpTexture(CComPtr<IXMLDOMNode> node,IGameMaterial * mat)
{
	CComPtr <IXMLDOMNode> textureRoot;
	TSTR buf;
	int texCount = mat->GetNumberOfTextureMaps();

	if(texCount>0)
	{
		CreateXMLNode(pXMLDoc,node,_T("TextureMaps"),&textureRoot);
		buf.printf("%d",texCount);
		AddXMLAttribute(textureRoot,_T("Count"),buf.data());
	}
	
	for(int i=0;i<texCount;i++)
	{
		CComPtr <IXMLDOMNode> texture;
		IGameTextureMap * tex = mat->GetIGameTextureMap(i);
		CreateXMLNode(pXMLDoc,textureRoot,_T("Texture"),&texture);
		buf.printf("%d",i);
		AddXMLAttribute(texture,_T("index"),buf.data());
		TCHAR * name = tex->GetTextureName();
		AddXMLAttribute(texture,_T("name"),name);
		int slot = tex->GetStdMapSlot();
		buf.printf("%d",slot);
		AddXMLAttribute(texture,_T("StdSlotType"),buf.data());

		if(tex->IsEntitySupported())	//its a bitmap texture
		{
			CComPtr <IXMLDOMNode> bitmapTexture;
			CreateXMLNode(pXMLDoc,texture,_T("BitmapTexture"),&bitmapTexture);

			CComPtr <IXMLDOMNode> bitmapName;
			CreateXMLNode(pXMLDoc,bitmapTexture,_T("Name"),&bitmapName);
			AddXMLText(pXMLDoc,bitmapName,tex->GetBitmapFileName());

			IGameProperty * prop = tex->GetClipHData();
			DumpProperties(bitmapTexture,prop);

			prop = tex->GetClipUData();
			DumpProperties(bitmapTexture,prop);

			prop = tex->GetClipVData();
			DumpProperties(bitmapTexture,prop);

			prop = tex->GetClipWData();
			DumpProperties(bitmapTexture,prop);

		}

	}

}

void IGameExporter::DumpMaterial(CComPtr<IXMLDOMNode> node,IGameMaterial * mat, int& index, int matID )
{
	TSTR buf;
	IGameProperty *prop;
	CComPtr <IXMLDOMNode> material;
	CComPtr <IXMLDOMNode> propNode;

	if(!mat->IsMultiType())
	{
		CreateXMLNode(pXMLDoc,node,_T("Material"),&material);
		buf.printf("%d",index);
		AddXMLAttribute(material,_T("index"),buf.data());

		if(matID !=-1)	// we are not a sub material
		{
			buf.printf("%d",matID);
			AddXMLAttribute(material,_T("MaterialID"),buf.data());
		}

		AddXMLAttribute(material,_T("Name"),mat->GetMaterialName());
		CComPtr <IXMLDOMNode> matData;
		//CreateXMLNode(pXMLDoc,material,_T("NumSubMtls"),&matData);
		//buf.printf("%d",mat->GetSubMaterialCount());
		//AddXMLText(pXMLDoc,matData,buf.data());

		//WE ONLY WANT THE PROPERTIES OF THE ACTUAL MATERIAL NOT THE CONTAINER - FOR NOW.....

		CreateXMLNode(pXMLDoc,material,_T("Properties"),&propNode);
		prop = mat->GetDiffuseData();
		DumpProperties(propNode,prop);
		prop = mat->GetAmbientData();
		DumpProperties(propNode,prop);
		prop = mat->GetSpecularData();
		DumpProperties(propNode,prop);
		prop = mat->GetOpacityData();
		DumpProperties(propNode,prop);
		prop = mat->GetGlossinessData();
		DumpProperties(propNode,prop);
		prop = mat->GetSpecularLevelData();
		DumpProperties(propNode,prop);
		//do the textures if they are there

		DumpTexture(material,mat);

		DumpShader(mat,material);
		index++;
	}
	else
	{
		//CComPtr <IXMLDOMNode> multi;
		//CreateXMLNode(pXMLDoc,material,_T("MultiMaterial"),&multi);
		for(int k=0;k<mat->GetSubMaterialCount();k++)
		{
			int mID = mat->GetMaterialID(k);
			IGameMaterial * subMat = mat->GetSubMaterial(k);
			DumpMaterial(node,subMat,index, mID);
		}
	}
}

int CountMaterials(IGameMaterial * mat){
	int count = 0;
	if(mat->IsMultiType())
	{
		for(int k=0;k<mat->GetSubMaterialCount();k++)
		{
			count += CountMaterials(mat->GetSubMaterial(k));
		}
	}
	else
		count++;
	return count;
}


void IGameExporter::ExportMaterials()
{
	CComPtr <IXMLDOMNode> matNode;
	TSTR buf;
	if(exportMaterials)
	{
	
		CreateXMLNode(pXMLDoc,iGameNode,_T("MaterialList"),&matNode);

		int totalMaterials = 0;
		int matCount = pIgame->GetRootMaterialCount();
		for(int j =0;j<matCount;j++)
		{
			totalMaterials += CountMaterials(pIgame->GetRootMaterial(j));
		}
		buf.printf("%d",totalMaterials);
		AddXMLAttribute(matNode,_T("Count"),buf.data());

		int index = 0;
		for(int j =0;j<matCount;j++)
		{
			m_MatOffset[j] = index;
			IGameMaterial * mat = pIgame->GetRootMaterial(j);
			if(mat)
				DumpMaterial(matNode,mat,index);
		}
	}
}

// Dummy function for progress bar
DWORD WINAPI fn(LPVOID arg)
{
	return(0);
}



class MyErrorProc : public IGameErrorCallBack
{
public:
	void ErrorProc(IGameError error)
	{
		TCHAR * buf = GetLastIGameErrorText();
		DebugPrint("ErrorCode = %d ErrorText = %s\n", error,buf);
	}
};


int	IGameExporter::DoExport(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts, DWORD options)
{
	HRESULT hr;

	Interface * ip = GetCOREInterface();

	MyErrorProc pErrorProc;

	UserCoord Whacky = {
		1,	//Right Handed
		1,	//X axis goes right
		3,	//Y Axis goes down
		4,	//Z Axis goes in.
		1,	//U Tex axis is right
		1,  //V Tex axis is Down
	};	

	SetErrorCallBack(&pErrorProc);

	ReadConfig();
	hr = CoInitialize(NULL); 
	// Check the return value, hr...
	hr = CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER,  IID_IXMLDOMDocument, (void**)&pXMLDoc);
	if(FAILED(hr))
		return false;
	// Check the return value, hr...
	hr = pXMLDoc->QueryInterface(IID_IXMLDOMNode, (void **)&pRoot);
	if(FAILED(hr))
		return false;

	// Set a global prompt display switch
	showPrompts = suppressPrompts ? FALSE : TRUE;

	exportSelected = (options & SCENE_EXPORT_SELECTED) ? true : false;

	curNode = 0;
	ip->ProgressStart(_T("Exporting Using IGame.."), TRUE, fn, NULL);
	
	pIgame = GetIGameInterface();

	//
	// TIM: All custom code below here
	//
	// Get the scaling needed to put coordinates in meters
	gWorldScale = GetMasterScale(UNITS_METERS);

	IGameConversionManager * cm = GetConversionManager();
	cm->SetCoordSystem((IGameConversionManager::CoordSystem)cS);

	TSTR filename;
	filename += GetCOREInterface()->GetDir(APP_PLUGCFG_DIR);
	filename += "\\";
	filename += "IGameProp_Reality.XML";

	if(!FileExists(filename.data())){
		MessageBox(0,"Couldn't find IGameProp_Reality.XML in plugcfg",0,0);
		return FALSE;
	}

	pIgame->SetPropertyFile(filename);
	pIgame->InitialiseIGame(exportSelected);
	pIgame->SetStaticFrame(staticFrame);
	
	TSTR path,file,ext;

	SplitFilename(TSTR(name),&path,&file,&ext);

	splitPath = path;
	
	ExportSceneInfo();
	ExportMaterials();


	for(int loop = 0; loop <pIgame->GetTopLevelNodeCount();loop++)
	{
		IGameNode * pGameNode = pIgame->GetTopLevelNode(loop);
		//check for selected state - we deal with targets in the light/camera section
		if(pGameNode->IsTarget())
			continue;
		ExportNodeInfo(pGameNode);

	}
	pIgame->ReleaseIGame();
	pXMLDoc->save(CComVariant(name));
//	pXMLDoc->Release();
	CoUninitialize();
	ip->ProgressEnd();	
	WriteConfig();
	return TRUE;
}


TSTR IGameExporter::GetCfgFilename()
{
	TSTR filename;
	
	filename += GetCOREInterface()->GetDir(APP_PLUGCFG_DIR);
	filename += "\\";
	filename += "IgameExport.cfg";
	return filename;
}

// NOTE: Update anytime the CFG file changes
#define CFG_VERSION 0x03

BOOL IGameExporter::ReadConfig()
{
	return FALSE; // not using config
}

void IGameExporter::WriteConfig()
{
	return; // not using config
}


