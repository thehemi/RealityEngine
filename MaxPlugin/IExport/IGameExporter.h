/**********************************************************************
 *<
	FILE: IGameExporter.h

	DESCRIPTION:	Includes for Plugins

	CREATED BY:	

	HISTORY:

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#ifndef __IGAMEEXPORTER__H
#define __IGAMEEXPORTER__H

//----------------------------------------------------------------------------------
// Headers
//----------------------------------------------------------------------------------
#include <CGUID.H> 
#include "msxml2.h"
#include "XMLUtility.h"
#include "IGame.h"
#include "IGameObject.h"
#include "IGameProperty.h"
#include "IGameControl.h"
#include "IGameModifier.h"
#include "IConversionManager.h"
#include "IGameError.h"

//----------------------------------------------------------------------------------
// 
//----------------------------------------------------------------------------------
struct SaveOptions
{
	BOOL                        m_bSaveAnimationData;
	BOOL                        m_bLoopingAnimationData;
	DWORD                       m_iAnimSamplingRate;
};

//----------------------------------------------------------------------------------
// 
//----------------------------------------------------------------------------------
HRESULT ExportFile
    (
    const TCHAR *szFilename,
    INode* pRootNode,
    Interface *pInterface, 
    BOOL bSuppressPrompts,
    BOOL bSaveSelection,
    SaveOptions& options
	);




//----------------------------------------------------------------------------------
// Exporter Class
//----------------------------------------------------------------------------------
class IGameExporter {
	public:

		IGameScene * pIgame;

		IXMLDOMDocument * pXMLDoc;
		IXMLDOMNode * pRoot;		//this is our root node 	
		CComPtr<IXMLDOMNode> iGameNode;	//the IGame child - which is the main node
		CComPtr<IXMLDOMNode> rootNode;
		CComPtr <IXMLDOMNode> tempNode;
		static HWND hParams;

		int curNode;

		int staticFrame;
		int framePerSample;
		BOOL exportGeom;
		BOOL exportNormals;
		BOOL exportVertexColor;
		BOOL exportControllers;
		BOOL exportFaceSmgp;
		BOOL exportTexCoords;
		BOOL exportMappingChannel;
		BOOL exportConstraints;
		BOOL exportMaterials;
		BOOL exportSplines;
		BOOL exportModifiers;
		BOOL exportSkin;
		BOOL exportGenMod;
		BOOL forceSample;
		BOOL splitFile;
		BOOL exportQuaternions;
		BOOL exportObjectSpace;
		int cS;
		int exportCoord;
		bool showPrompts;
		bool exportSelected;

		TSTR splitPath;

		BOOL SupportsOptions(int ext, DWORD options);
		int	DoExport(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts=FALSE, DWORD options=0);

		
		void ExportSceneInfo();
		void ExportNodeInfo(IGameNode * node);
		void ExportChildNodeInfo(CComPtr<IXMLDOMNode> parent, IGameNode * child);
		void ExportMaterials();
		void ExportPositionControllers(CComPtr<IXMLDOMNode> node, IGameControl * cont);
		void ExportRotationControllers(CComPtr<IXMLDOMNode> node, IGameControl * cont);
		void ExportScaleControllers(CComPtr<IXMLDOMNode> node, IGameControl * cont);
		void ExportControllers(CComPtr<IXMLDOMNode> node, IGameControl * cont);

		void DumpMaterial(CComPtr<IXMLDOMNode> node,IGameMaterial * mat, int& index, int matID = -1);
		void DumpTexture(CComPtr<IXMLDOMNode> node,IGameMaterial * mat);
		void DumpBezierKeys(DWORD Type, IGameKeyTab Keys, CComPtr<IXMLDOMNode> prsData);
		void DumpTCBKeys(DWORD Type, IGameKeyTab Keys, CComPtr<IXMLDOMNode> prsData);
		void DumpLinearKeys(DWORD Type, IGameKeyTab Keys, CComPtr<IXMLDOMNode> prsData);
		void DumpConstraints(CComPtr<IXMLDOMNode> prsData, IGameConstraint * c);
		void DumpModifiers(CComPtr<IXMLDOMNode> prsData, IGameModifier * m);
		void DumpSkin(CComPtr<IXMLDOMNode> modNode, IGameSkin * s);
		void DumpIKChain(IGameIKChain * ikch, CComPtr<IXMLDOMNode> ikData);

		void DumpEulerController(IGameControl * sc,CComPtr<IXMLDOMNode> prsNode);
		void DumpProperties(CComPtr<IXMLDOMNode> node, IGameProperty * prop, bool bCoordinate = false);
		void DumpMesh(IGameMesh *gm,CComPtr<IXMLDOMNode> geomData);
		void DumpSpline(IGameSpline *sp,CComPtr<IXMLDOMNode> splineData);
		void DumpLight(IGameLight *lt, CComPtr<IXMLDOMNode> parent);
		void DumpCamera(IGameCamera *ca, CComPtr<IXMLDOMNode> parent);
		void DumpVertex(struct Vert3* test, IGameMesh* gM, FaceEx *fe, int vertex, CComPtr<IXMLDOMNode> faceData, bool smgrp, bool n, bool vc, bool tv);
		void DumpSampleKeys(IGameControl * sc,CComPtr<IXMLDOMNode> prsNode, DWORD Type, bool quick = false);
		void DumpListController(IGameControl * sc,CComPtr<IXMLDOMNode> listNode);
		void DumpMatrix(Matrix3 tm,CComPtr<IXMLDOMNode> parent);

		// New custom extensions (Tim Johnson)
		void DumpMeshBinary(IGameNode* gNode, CComPtr<IXMLDOMNode> geomData, INode* node, IGameMesh *gm, TSTR filename);
		void DumpShader(IGameMaterial * mat, CComPtr<IXMLDOMNode> node);
		void DumpNodeProperties(INode* node, CComPtr<IXMLDOMNode> parent);
		void DumpSHProperties(NodeData& data, CComPtr<IXMLDOMNode> parent);
		// New Vars
		int m_MatOffset[200]; // Multi-map ID to global offset

		void MakeSplitFilename(IGameNode * node, TSTR & buf);
		BOOL ReadConfig();
		void WriteConfig();
		TSTR GetCfgFilename();
		IGameExporter();
		~IGameExporter();		
};

#endif // __IGAMEEXPORTER__H
