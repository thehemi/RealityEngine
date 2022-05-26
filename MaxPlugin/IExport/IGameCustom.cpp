//-----------------------------------------------------------------------------
// File: IGameCustom
// Desc: Functions used to export max data to an XML File
// Author: Reauthored by Tim Johnson, 2004, as custom format
//
// 
// TODO:
// Binary meshes
// Tangents/Binormals

//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "..\Max HLSL\Tools\src\cgfxmax\Utility.h"
#include "..\Max HLSL\Tools\src\cgfxmax\RenderMesh.h"
#include "IGameCustom.h"
using namespace nv_sys;

extern float gWorldScale; // Converts to meters


//-----------------------------------------------------------------------------
// Helper
//-----------------------------------------------------------------------------
#define BITS_IN_ULONG (CHAR_BIT * sizeof(unsigned long))
unsigned long NearestPow2(unsigned long val)
{
	unsigned      b = BITS_IN_ULONG / 2;
	unsigned long q = 1lu<<b;
	unsigned long n = 0;
	/* one could do
	assert(q<<(b-1) != 0);
	here to abort if above premise does not hold
	*/
	if (val < 2)
		return 1;

	while (b != 0) {
		b >>= 1;
		if (val <= q) {
			n = q;
			q >>= b;
		} else {   q <<= b;
		}
	}
	return n;
}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void IGameExporter::DumpMatrix(Matrix3 tm,CComPtr<IXMLDOMNode> parent)
{
	CComPtr <IXMLDOMNode> tmNode;
//	AffineParts ap;
	Point3 rotAxis;
	Point3 scaleAxis;
	Matrix3 m = tm;
	TSTR Buf;
	
	AttribTo(parent,"Row0",m.GetRow(0));
	AttribTo(parent,"Row1",m.GetRow(1));
	AttribTo(parent,"Row2",m.GetRow(2));
	Point3 p = m.GetRow(3) * gWorldScale;
	AttribTo(parent,"Row3",p);

/*	decomp_affine(m, &ap);

	// Quaternions are dumped as angle axis.
	AngAxisFromQ(ap.q, &rotAngle, rotAxis);
	AngAxisFromQ(ap.u, &scaleAxAngle, scaleAxis);

//	CreateXMLNode(pXMLDoc,parent,_T("NodeTM"),&tmNodeParent);
	CreateXMLNode(pXMLDoc,parent,_T("Translation"),&tmNode);
	Buf.printf("%f %f %f",ap.t.x,ap.t.y,ap.t.z);
	AddXMLText(pXMLDoc,tmNode,Buf.data());
	tmNode = NULL;
	CreateXMLNode(pXMLDoc,parent,_T("Rotation"),&tmNode);
	if(!exportQuaternions)
		Buf.printf("%f %f %f %f",rotAxis.x, rotAxis.y, rotAxis.z, rotAngle);
	else
		Buf.printf("%f %f %f %f",ap.q.x, ap.q.y, ap.q.z, ap.q.w);

	AddXMLText(pXMLDoc,tmNode,Buf.data());
	tmNode = NULL;
	CreateXMLNode(pXMLDoc,parent,_T("Scale"),&tmNode);

	Buf.printf("%f %f %f %f %f %f",ap.k.x, ap.k.y, ap.k.z, scaleAxis.x,scaleAxis.y,scaleAxis.z);
	AddXMLText(pXMLDoc,tmNode,Buf.data());*/


}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void IGameExporter::DumpSHProperties(NodeData& data, CComPtr<IXMLDOMNode> parent)
{
	CreateNode(shRoot,parent,"SphericalHarmonics");
	AttribTo(shRoot,"ReceiverName",data.receiverGroup);

	// Indoor blockers
	CreateNode(blockers,shRoot,"InBlockers");
	AttribTo(blockers,"Count",(int)data.inBlockers.size());
	for(int i=0;i<data.inBlockers.size();i++)
	{
		Node(blockers,"Blocker");
		Attrib("Name",data.inBlockers[i]);
	}
	blockers = NULL;

	// Outdoor blockers
	CreateNode(blockers2,shRoot,"OutBlockers");
	AttribTo(blockers2,"Count",(int)data.outBlockers.size());
	for(int i=0;i<data.outBlockers.size();i++)
	{
		Node(blockers2,"Blocker");
		Attrib("Name",data.outBlockers[i]);
	}
	blockers2 = NULL;

	// Custom copy of simulator options that includes the global overrides
	SceneProperties* props = theMap.GetSceneProperties();

	// Temp will be modified by UpdateGlobalSettings
	NodeData temp = data;
	// If using or forced global settings, fetch them now
	if(!data.bUseCustom || props->bPRTOverride)
	{
		theSH.UpdateGlobalProperties(temp);
	}

	SIMULATOR_OPTIONS sh = temp.shOptions;
	// Multiply by lobal percentages
	sh.dwNumRays	 *= float(props->dwRays)/100.f;
	sh.dwNumRays++;
	sh.dwNumBounces  *= float(props->dwBounces)/100.f;
	sh.dwNumBounces++;
	sh.dwTextureSize *= float(props->dwTextureSize)/100.f;
	sh.dwTextureSize =  NearestPow2(sh.dwTextureSize);

	Node(shRoot,"Settings");
	Attrib("NumRays",sh.dwNumRays);
	Attrib("Order",sh.dwOrder);
	Attrib("NumChannels",sh.dwNumChannels);
	Attrib("NumBounces",sh.dwNumBounces);
	Attrib("SubsurfaceScattering",sh.bSubsurfaceScattering);
	Attrib("LengthScale",sh.fLengthScale);
	Attrib("Diffuse",*(Vector*)&sh.Diffuse);
	Attrib("Absoption",*(Vector*)&sh.Absoption);
	Attrib("ReducedScattering",*(Vector*)&sh.ReducedScattering);
	Attrib("RelativeIndexOfRefraction",sh.fRelativeIndexOfRefraction);
	Attrib("PredefinedMatIndex",sh.dwPredefinedMatIndex);
	Attrib("Adaptive",sh.bAdaptive);
	Attrib("RobustMeshRefine",sh.bRobustMeshRefine);
	Attrib("RobustMeshRefineMinEdgeLength",sh.fRobustMeshRefineMinEdgeLength);
	Attrib("RobustMeshRefineMaxSubdiv",sh.dwRobustMeshRefineMaxSubdiv);
	Attrib("AdaptiveDL",sh.bAdaptiveDL);
	Attrib("AdaptiveDLMinEdgeLength",sh.fAdaptiveDLMinEdgeLength);
	Attrib("AdaptiveDLThreshold",sh.fAdaptiveDLThreshold);
	Attrib("AdaptiveDLMaxSubdiv",sh.dwAdaptiveDLMaxSubdiv);
	Attrib("AdaptiveBounce",sh.bAdaptiveBounce);
	Attrib("AdaptiveBounceMinEdgeLength",sh.fAdaptiveBounceMinEdgeLength);
	Attrib("AdaptiveBounceThreshold",sh.fAdaptiveBounceThreshold);
	Attrib("AdaptiveBounceMaxSubdiv",sh.dwAdaptiveBounceMaxSubdiv);
	Attrib("NumClusters",sh.dwNumClusters);
	Attrib("NumPCA",sh.dwNumPCA);
	Attrib("PerPixel",sh.bPerPixel);
	Attrib("TextureSize",sh.dwTextureSize);

	tempNode = NULL;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
TSTR GetTimeStamp(SYSTEMTIME time)
{
	TSTR buf;
	buf.printf("%d %d %d %d %d %d %d %d",time.wYear,time.wMonth,time.wDayOfWeek,time.wDay,time.wHour,time.wMinute,time.wSecond,time.wMilliseconds);
	return buf;
}

//-----------------------------------------------------------------------------
// Custom Node/Entity Properties
//-----------------------------------------------------------------------------
void IGameExporter::DumpNodeProperties(INode* node, CComPtr<IXMLDOMNode> parent)
{
	NodeData nData;
	GetNodeData(node,nData);

	CComPtr <IXMLDOMNode> propNode;
	CreateXMLNode(pXMLDoc,parent,_T("CustomProperties"),&propNode);

	// Script Stuff
	if(nData.script.filename.length())
	{
		CreateNode(script,propNode,"Script");
		AttribTo(script,"File",nData.script.filename);
		AttribTo(script,"Class",nData.script.classname);
		AttribTo(script,"Parent",nData.script.parentclass);
		AttribTo(script,"IncludeModel",nData.script.bIncludeModel);

		for(int i=0;i<nData.script.parameters.size();i++){
			Node(script,"Param");
			Attrib("Name",nData.script.parameters[i]);
			Attrib("Value",nData.script.paramvalues[i]);
		}
	}

	propNode = NULL;
}

//-----------------------------------------------------------------------------
// A light with exclude list
//-----------------------------------------------------------------------------
void IGameExporter::DumpLight(IGameLight *lt, CComPtr<IXMLDOMNode> parent)
{
	IGameProperty * prop;
	CComPtr <IXMLDOMNode> propNode,targNode;

	CreateXMLNode(pXMLDoc,parent,_T("Properties"),&propNode);
	prop = lt->GetLightColor();
	DumpProperties(propNode,prop);
	prop = lt->GetLightMultiplier();
	DumpProperties(propNode,prop);
	prop = lt->GetLightAspectRatio();
	DumpProperties(propNode,prop);
	prop = lt->GetLightAttenEnd();
	DumpProperties(propNode,prop,true);
	prop = lt->GetLightAttenStart();
	DumpProperties(propNode,prop,true);
	prop = lt->GetLightAspectRatio();
	DumpProperties(propNode,prop);

	// TIM: Added
	prop = lt->GetLightFallOff();
	DumpProperties(propNode,prop);
	prop = lt->GetLightHotSpot();
	DumpProperties(propNode,prop);

	if(lt->GetLightType()==TSPOT_LIGHT )
	{
		CreateXMLNode(pXMLDoc,parent,_T("Target"),&targNode);
		ExportChildNodeInfo(targNode,lt->GetLightTarget());
	}

	GenLight* light = (GenLight*)lt->GetMaxObject();

	// Exclude/Include list export
	vector<string> excluded;
	// TimJohnson: Only export exclude list when applies to illumination, not shadowing
	for (int nameid = 0; nameid < lt->GetExcludedNodesCount(); nameid++) {
		IGameNode *n = lt->GetExcludedNode(nameid);
		if (n)
			excluded.push_back(n->GetName());
	}

	CComPtr <IXMLDOMNode> excludeList;
	CreateXMLNode(pXMLDoc, parent, _T("ExcludeList"), &excludeList);
	AddXMLAttribute(excludeList, _T("Count"), (int)excluded.size());
	AddXMLAttribute(excludeList, _T("Include"), (int)lt->IsExcludeListReversed());

	for(int i=0;i<excluded.size();i++){
		Node(excludeList,"Light");
		Attrib("Name",excluded[i]);
	}

	
	 // Does this have a projection map?
	 Texmap* tex = light->GetProjMap();
	 if (tex && tex->ClassID() == Class_ID(BMTEX_CLASS_ID, 0x00)) {
		 Node(parent,"ProjectionMap");
		 Attrib("Name",((BitmapTex *)tex)->GetMapName());
	 }

	 // Does this have a shadow map?
	 if(light->GetUseShadowColorMap(0))
		 tex = light->GetShadowProjMap();
	 else
		 tex = NULL;

	 if (tex && tex->ClassID() == Class_ID(BMTEX_CLASS_ID, 0x00)) {
		 Node(parent,"ShadowMap");
		 Attrib("Name",((BitmapTex *)tex)->GetMapName());
	 }

	tempNode = NULL;
}


 //----------------------------------------------------------------------------------
 // Dumps special shader material
 //----------------------------------------------------------------------------------
 void IGameExporter::DumpShader(IGameMaterial * mat, CComPtr<IXMLDOMNode> node)
 {
	 IDXShaderManagerInterface* IDXShaderMgr = GetDXShaderManager();
	 if(!IDXShaderMgr)
		 return;

	 CustAttrib * custattr = IDXShaderMgr->FindViewportShaderManager(mat->GetMaxMaterial());
	 ICGFXDataBridge * icgfx = 0;
	 if (custattr)
	 {
		 IViewportShaderManager *IViewportShaderMgr = (IViewportShaderManager*)custattr->GetInterface(VIEWPORT_SHADER_MANAGER_INTERFACE);
		 if (IViewportShaderMgr)
		 {
			 ReferenceTarget* reftarg = IViewportShaderMgr->GetActiveEffect();
			 if (reftarg)
				 icgfx = (ICGFXDataBridge *)reftarg->GetInterface(CGFX_DATA_CLIENT_INTERFACE);
		 }
	 }

	 if(!icgfx)
		 return;

	 CComPtr <IXMLDOMNode> material, tempNode;
	 CreateXMLNode(pXMLDoc,node,_T("Shader"),&material);

	 AddXMLAttribute(material,_T("File"),icgfx->GetCgFX());
	 AddXMLAttribute(material,_T("Technique"),icgfx->GetTechnique().c_str());

	 INVParameterList* pParamList  = icgfx->GetParameterList();
	 for (unsigned int i = 0; i < pParamList->GetNumParameters(); i++)
	 {
		 // Find out the active connection parameter
		 INVConnectionParameter* pParam = pParamList->GetConnectionParameter(i);

		 if (!pParam)
			 continue;

		 // TIM: Don't include vars which aren't annotated
		 // (Optionally, still include them if they have semantics)
		 if(pParam->GetDefaultValue().GetObjectSemantics()->GetNumAnnotations() == 0
			 /*&& pParam->GetDefaultValue().GetObjectSemantics()->GetSemanticID() == SEMANTICID_UNKNOWN*/ )
			 continue;

		 // Special case lights
		 eANNOTATIONVALUEID ValueID = pParam->GetDefaultValue().GetObjectSemantics()->FindAnnotationValue(ANNOTATIONNAMEID_OBJECT);
		 if (ValueID == ANNOTATIONVALUEID_DIRLIGHT ||ValueID == ANNOTATIONVALUEID_SPOTLIGHT ||ValueID == ANNOTATIONVALUEID_POINTLIGHT)
			 continue; // Ignore light data, we use scene lighting

		 Node(material,"Parameter");
		 Attrib("Name",pParam->GetDefaultValue().GetObjectSemantics()->GetName());

		 // TIM: FUCK! We can't do this optimization because we need default vars
		 // to overwrite other shaders that might have set them
		 // Filter out params that the user hasn't adjusted
		 /* NVType Comp = pParam->GetDefaultValue();
		 ULONG b1,b2;
		 KeyValue.GetValue(b1);
		 Comp.GetValue(b2);

		 if(b1 == b2 && memcmp(KeyValue.GetValue(b1),Comp.GetValue(b2),b1) == 0)
		 continue;*/

		 // Possible to write out the semantic name?  Could be useful.
		 /* SaveData(ConvertSemantic(pParam->GetSemanticID()))

		 // Write out the link info.  This is for mapping this parameter to a meaningful app thing,
		 // such as a light
		 IAppConnection pAppConnect = pParam->GetAppConnection();
		 if (pAppConnect)
		 {
		 // Write out the application's connection name
		 SaveDataString(pAppConnect->GetName())

		 // Write out the data items, preceeded by the number (0 is valid).
		 SaveDataDWORD(pAppConnect->GetConnectionDataNum());
		 for (unsigned int i = 0; i < pAppConnect->GetConnectionDataNum(); i++)
		 {
		 // Write out the bytes
		 SaveData(pAppConnect->GetConnectionData(i));
		 }
		 }*/ 

		 NVType KeyValue = pParam->GetValueAtTime(GetSYSInterface()->GetTime());
		 switch(KeyValue.GetType())
		 {
		 case NVTYPEID_VEC4:
			 {
				 Attrib("Type","Float4");
				 Attrib("Value",*(Vector4*)&KeyValue.GetVec4().x);
			 }
			 break;

		 case NVTYPEID_VEC3:
			 {
				 Attrib("Type","Float3");
				 Attrib("Value",Vector(KeyValue.GetVec3().x,KeyValue.GetVec3().y,KeyValue.GetVec3().z));
			 }
			 break;

		 case NVTYPEID_VEC2:
			 {
				 Attrib("Type","Float2");
				 Attrib("Value",Vector2(KeyValue.GetVec2().x,KeyValue.GetVec2().y));
			 }
			 break;

		 case NVTYPEID_TEXTURE:
		 case NVTYPEID_TEXTURE2D:
		 case NVTYPEID_TEXTURE3D:
		 case NVTYPEID_TEXTURECUBE:
			 {
				 Attrib("Type","Texture");
				 Attrib("Value",icgfx->GetTextureName(KeyValue.GetTexture()));
			 }
			 break;

		 case NVTYPEID_BOOL:
			 {
				 Attrib("Type","Bool");
				 Attrib("Value",KeyValue.GetBool());
			 }
			 break;

		 case NVTYPEID_INT:
			 {
				 Attrib("Type","Int");
				 Attrib("Value",(int)KeyValue.GetInt());
			 }
			 break;

		 case NVTYPEID_DWORD:
			 {
				 Attrib("Type","Int");
				 Attrib("Value",(int)KeyValue.GetDWORD());
			 }
			 break;

		 case NVTYPEID_FLOAT:
			 {
				 Attrib("Type","Float");
				 Attrib("Value",KeyValue.GetFloat());
			 }
			 break;
		 }
	 }


	 material = NULL;
	 tempNode = NULL;
 }

 //----------------------------------------------------------------------------------
 // 
 //----------------------------------------------------------------------------------
 void IGameExporter::ExportSceneInfo()
 {
	 TSTR buf;
	 CComPtr <IXMLDOMNode> sceneNode;

	 struct tm *newtime;
	 time_t aclock;

	 time( &aclock );
	 newtime = localtime(&aclock);

	 TSTR today = _tasctime(newtime);	// The date string has a \n appended.
	 today.remove(today.length()-1);		// Remove the \n

	 CreateXMLNode(pXMLDoc, pRoot, _T("IGame"), &iGameNode);
	 AddXMLAttribute(iGameNode, _T("Version"), _T("1.0"));
	 AddXMLAttribute(iGameNode, _T("Date"), today.data());

	 CreateXMLNode(pXMLDoc,iGameNode,_T("SceneInfo"),&sceneNode);

	 Node(sceneNode,"Info");
	 Attrib("FileName",const_cast<TCHAR*>(pIgame->GetSceneFileName()));

	 Node(sceneNode,"Info");
	 Attrib("StartFrame",pIgame->GetSceneStartTime() / pIgame->GetSceneTicks());

	 Node(sceneNode,"Info");
	 Attrib("EndFrame",pIgame->GetSceneEndTime() / pIgame->GetSceneTicks());

	 Node(sceneNode,"Info");
	 Attrib("FrameRate",GetFrameRate());

	 Node(sceneNode,"Info");
	 Attrib("TicksPerFrame",pIgame->GetSceneTicks());


	 Node(sceneNode,"Info");
	 if(cS == 0)
		 buf = TSTR("3dsmax");
	 if(cS == 1)
		 buf = TSTR("directx");
	 if(cS == 2)
		 buf = TSTR("opengl");

	 Attrib("CoordinateSystem",buf.data());

	 // Scene Properties...
	 SceneProperties* props = theMap.GetSceneProperties();

	 Node(sceneNode,"Clip");
	 Attrib("FarPlane",(int)props->clipPlane);

	 Node(sceneNode,"Fog");
	 Attrib("Color",Vector(GetRValue(props->fogColor)/255.f,GetGValue(props->fogColor)/255.f,GetBValue(props->fogColor)/255.f));
	 Attrib("Start",(int)props->fogStart);

	 Node(sceneNode,"MiniMap");
	 Attrib("Image",props->miniMap);
	 Attrib("Scale",props->miniMapScale);

	 sceneNode = NULL;
	 tempNode = NULL;
 }


 Point3 ToD3D(Point3& in){
	 Point3 out = in;
	 out.y = in.z;
	 out.z = in.y;
	 return out;
 }

void IGameExporter::DumpVertex(Vert3* test, IGameMesh* gM, FaceEx *fe, int i, CComPtr<IXMLDOMNode> faceData, bool smgrp, bool n, bool vc, bool tv)
{
	TSTR data;
	FaceEx *f = fe;
	CComPtr <IXMLDOMNode> prop = NULL;
	TSTR buf;

	int numNormals = gM->GetNumberOfNormals();



	if(/*numNormals > fe->meshFaceIndex &&*/ numNormals > f->vert[i]){
	
		/*Attrib("Norm",gM->GetNormal(fe,i));
		Attrib("Tangent",gM->GetTangent(f->vert[i]));
		Attrib("Binormal",gM->GetBinormal(f->vert[i]));*/
	}

	

	/*Tab<int> mapNums = gM->GetActiveMapChannelNum();
	int mapCount = mapNums.Count();
	for(int j=0;j < mapCount;j++)
	{
		DWORD  fIndex[3];
		// NOTE: meshFaceIndex could be wrong way to index!!!
		gM->GetMapFaceIndex(mapNums[j],fe->meshFaceIndex,fIndex);
		TSTR buf;
		buf.printf("Tex%d",j);
		Point3 p = gM->GetMapVertex(mapNums[j],fIndex[i]);
		Attrib(buf,Vector2(p.x,p.y));
	}*/
}

void IGameExporter::DumpMesh(IGameMesh * gm, CComPtr<IXMLDOMNode> geomData)
{
	CComPtr <IXMLDOMNode> vertData,normData,faceData,vcData,tvData;
	CComPtr <IXMLDOMNode> propNode;
	TSTR buf;
	bool vcSet = false;
	bool nSet = false;
	bool tvSet = false;

	std::vector<Vert3>		Verts;
	std::vector<Face3>		Faces;
	RenderMesh m;
	m.ConvertFaces(gm->GetMaxMesh(),0,Verts,Faces,false);

	Tab<int> mapNums = gm->GetActiveMapChannelNum();
	int mapCount = 0;//mapNums.Count();

	int mapVerts =0;
	while(mapVerts=gm->GetMaxMesh()->getNumMapVerts(mapCount+1))
		mapCount++;

	CreateXMLNode(pXMLDoc,geomData,_T("Vertices"),&faceData);
	AttribTo(faceData,"Count",(int)Verts.size());
	for(int i=0;i<Verts.size();i++)
	{
		Node(faceData,"Vertex");

		Attrib("Pos",ToD3D(Verts[i].m_Pos)*gWorldScale);
		Attrib("Norm",ToD3D(Verts[i].m_Normal));
		Attrib("Tangent",ToD3D(Verts[i].m_S));

		for(int j=0;j<mapCount;j++){
			TSTR buf;
			buf.printf("Tex%d",j);
			Attrib(buf,Verts[i].m_UV[j]);
		}
	}
	faceData = NULL;

	CreateXMLNode(pXMLDoc,geomData,_T("Faces"),&faceData);
	AttribTo(faceData,"Count",(int)Faces.size());
	for(int i=0;i<Faces.size();i++)
	{
		Node(faceData,"Face");

		Attrib("v0",Faces[i].m_Num[0]);
		Attrib("v1",Faces[i].m_Num[1]);
		Attrib("v2",Faces[i].m_Num[2]);
	}
	faceData = NULL;

	m.Destroy();


	vertData = NULL;
	normData = NULL;
	faceData = NULL;
}

bool NodeNeedsRecompiling(NodeData& node, string file)
{
	// Only generate SH if we haven't already generated it for current mesh+timestamp
	bool bRecompile = true;
	// Extract file stamp
	HANDLE hFile = CreateFile(file.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);  
	if(hFile == INVALID_HANDLE_VALUE)
		return true; // File doesn't exist, so yes we must create it!

	FILETIME writeTime, moveTime, modifyTime;
	GetFileTime(hFile,0,0,&writeTime);
	CloseHandle(hFile);
	// Compare
	SystemTimeToFileTime(&node.timeMoved,&moveTime);
	SystemTimeToFileTime(&node.timeModified,&modifyTime);
	if(CompareFileTime(&writeTime,&moveTime) == 1 && CompareFileTime(&writeTime,&modifyTime) == 1)
		return false; // Compiled file is newer, don't recompile

	return true;
}


void IGameExporter::DumpMeshBinary(IGameNode* gNode, CComPtr<IXMLDOMNode> geomData, INode* node, IGameMesh * gm, TSTR filename)
{
    DebugPrint("Dumping Binary Mesh..");
	CComPtr <IXMLDOMNode> vertData,normData,faceData,vcData,tvData;
	CComPtr <IXMLDOMNode> propNode;
	TSTR buf;
	bool vcSet = false;
	bool nSet = false;
	bool tvSet = false;
	
	GMatrix m1 = gNode->GetObjectTM(staticFrame);
 
	Matrix mat;
	for(int i=0;i<4;i++)
		mat[i] = *(Vector*)&m1.GetRow(i);

	Matrix rotMat = mat.GetRotationMatrix(); 
	Matrix3 m = m1.ExtractMatrix3();
	BOOL negScale = TMNegParity(m);

	Matrix tmMat;

	if(negScale) // If negative scale, no way to extract individual scales, so must use rotation too
		tmMat = mat;
	else
		tmMat = mat.GetScaleMatrix();

	tmMat[3] = Vector();

	Tab<int> mapNums = gm->GetActiveMapChannelNum();
	int mapCount = mapNums.Count();

	// No more than two map coords
	if(mapCount > 2)
		mapCount = 2;
	// Never 0, force 1, even if coords are garbage
	if(mapCount == 0)
		mapCount = 1;

	FILE* file = fopen(filename,"wb");
    if(!file)
    {
        MessageBox(0,"The specified save location is invalid. \nPlease update your output directories to valid locations\nThese are listed in the export tab.",0,0);
        return;
    }

	// Version
	BYTE ver = 11;
	fwrite(&ver,sizeof(ver),1,file);

	// Optimized
	bool bOptimized = false;
	fwrite(&bOptimized,sizeof(bOptimized),1,file);

	// Number of material segments
	int numSegments = 1;

	Tab<int> segs = gm->GetActiveMatIDs();

	bool useIDs = false;

	// Count segments that have faces
	if(gNode->GetNodeMaterial() && gNode->GetNodeMaterial()->IsMultiType()){
		// ONLY DONE FOR MULTI-MAT. Non-multi give random material indexes
		numSegments = segs.Count();
		useIDs = true;
	}

	fwrite(&numSegments,sizeof(numSegments),1,file);

	for(int i=0;i<numSegments;i++)
	{
		std::vector<Vert3>		Verts;
		std::vector<Face3>		Faces;

		int matID = 0;
		RenderMesh m;
		if(!useIDs)
			m.ConvertFaces(gm->GetMaxMesh(),0,Verts,Faces,negScale);
		else
		{
			matID = segs[i];
			m.ConvertFaces(gm->GetMaxMesh(),segs[i]+1,Verts,Faces,negScale);
		}

		fwrite(&matID,sizeof(matID),1,file);

		int numVertices = Verts.size();
		int vertexSize  = sizeof(float)*3*3 + sizeof(float)*2*mapCount;
		fwrite(&vertexSize,sizeof(vertexSize),1,file);
		fwrite(&numVertices,sizeof(numVertices),1,file);

		// Indices
		int indexSize = (gm->GetNumberOfVerts()*1.2f >= 65535 || gm->GetNumberOfFaces()*1.2f >= 65535)?sizeof(DWORD):sizeof(WORD);
		int numIndices = Faces.size()*3;
		fwrite(&indexSize,sizeof(indexSize),1,file);
		fwrite(&numIndices,sizeof(numIndices),1,file);

		if(Faces.size() == 0 || Verts.size() == 0)
			continue;

		// TIM: Baking scale only!
		for(int i=0;i<Verts.size();i++)
		{
			Verts[i].m_Pos = ToD3D(Verts[i].m_Pos)*gWorldScale;
			Verts[i].m_Pos = *(Point3*)&(tmMat * *(Vector*)&Verts[i].m_Pos);
			Verts[i].m_Normal = ToD3D(Verts[i].m_Normal);
			//Verts[i].m_Normal = *(Point3*)&(rotMat * *(Vector*)&Verts[i].m_Normal);
			Verts[i].m_S = ToD3D(Verts[i].m_S);
			//Verts[i].m_S = *(Point3*)&(rotMat * *(Vector*)&Verts[i].m_S);
			// If neg scale, flip normal and tangent
			if(negScale)
			{
				//Verts[i].m_Normal = -Verts[i].m_Normal;
				//Verts[i].m_S = -Verts[i].m_S;
			}

		}
		
		for(int i=0;i<numVertices;i++)
			fwrite(&Verts[i],vertexSize,1,file);

		if(indexSize == sizeof(DWORD)){
			for(int i=0;i<Faces.size();i++){
				DWORD inds[3] = {Faces[i].m_Num[2],Faces[i].m_Num[1],Faces[i].m_Num[0]};
				fwrite(&inds[0],indexSize,3,file);
			}
		}
		else{
			for(int i=0;i<Faces.size();i++){
				WORD inds[3] = {Faces[i].m_Num[2],Faces[i].m_Num[1],Faces[i].m_Num[0]};
				fwrite(&inds[0],indexSize,3,file);
			}
		}
		m.Destroy();
	}

	fclose(file);
    DebugPrint("Done.\n");
}