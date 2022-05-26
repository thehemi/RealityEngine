//-----------------------------------------------------------------------------
// File: ExportXFile.cpp
// Desc: Functions used to export max data to an X File
// Author: Reauthored by Tim Johnson, 2003, as custom format
//
// Change Log:
// * Added templates 'TextureMap' and 'MaterialMaps'
// * Added max scaling
// * Changed a million little things
// [2] Added template 'Light'
// [3] Addition of template for custom data stored per-mesh
// [4] Option to export ONLY animation data
// [5] Converted to util plugin lib
//
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "XSkinExp.h"
#include "MeshData.h"
#include "decomp.h"

#include <initguid.h>

#include "XSkinExpTemplates.h"
#include "dxfile.h"
#include "rmxfguid.h"
#include "rmxftmpl.h"

#define POWER_DEFAULT   0.0

bool gbNan_found= false;
float gWorldScale = 0;
TCHAR errstr[500 + _MAX_PATH]= "Undetermined Error!";

void NAN_FLOAT_TEST(float *fValue)
{

	if(0 != _isnan(*fValue) || *fValue != *fValue)
	{
		if(gbNan_found == false)
		{
			MessageBox(NULL, "Exporter found a NAN float value and has set it to 0.0f.  The resulting X-File may not be accurate.", "ERROR: NAN FLOAT", MB_OK|MB_ICONERROR);
		}

		gbNan_found= true;

		*fValue= 0.0f;
	}
}

VOID XSkinExp_OutputDebugString(LPCTSTR lpOutputString)
{
	_tcscpy(errstr, lpOutputString);
	OutputDebugString(lpOutputString);
}



const GUID* aIds[] = {&DXFILEOBJ_XSkinMeshHeader,
                &DXFILEOBJ_VertexDuplicationIndices,
				&DXFILEOBJ_SkinWeights};
int num_aIds = 3;


/*
BOOL CALLBACK XSkinExpOptionsDlgProc
    (
    HWND hWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    ) 
{
    static SDialogOptions *pDialogOptions = NULL;
    TCHAR buff[8];

    switch(message) 
    {
        case WM_INITDIALOG:
            pDialogOptions = (SDialogOptions *)lParam;

            CenterWindow(hWnd,GetParent(hWnd));

            CheckDlgButton(hWnd, IDC_ANIMATION, 
                    pDialogOptions->m_bSaveAnimationData ? BST_CHECKED : BST_UNCHECKED);

            CheckDlgButton(hWnd, IDC_LOOPINGANIMATION, 
                    pDialogOptions->m_bLoopingAnimationData ? BST_CHECKED : BST_UNCHECKED);

            CheckDlgButton(hWnd, IDC_PATCHDATA, 
                    pDialogOptions->m_bSavePatchData ? BST_CHECKED : BST_UNCHECKED);

            if (pDialogOptions == NULL)
                return FALSE;

            switch (pDialogOptions->m_xFormat)
            {
                case DXFILEFORMAT_BINARY:
                    CheckRadioButton(hWnd,IDC_TEXT,IDC_BINARYCOMPRESSED,IDC_BINARY);
                    break;

                case DXFILEFORMAT_TEXT:
                    CheckRadioButton(hWnd,IDC_TEXT,IDC_BINARYCOMPRESSED,IDC_TEXT);
                    break;

                case DXFILEFORMAT_BINARY | DXFILEFORMAT_COMPRESSED:
                    CheckRadioButton(hWnd,IDC_TEXT,IDC_BINARYCOMPRESSED,IDC_BINARYCOMPRESSED);
                    break;
            }

            _stprintf(buff,_T("%d"),pDialogOptions->m_cMaxBonesPerVertex);
            SetDlgItemText(hWnd,IDC_MAX_BONES_PER_VERTEX,buff);

            _stprintf(buff,_T("%d"),pDialogOptions->m_cMaxBonesPerFace);
            SetDlgItemText(hWnd,IDC_MAX_BONES_PER_FACE,buff);

            _stprintf(buff,_T("%d"),pDialogOptions->m_iAnimSamplingRate);
            SetDlgItemText(hWnd,IDC_SAMPLERATE,buff);

            return TRUE;

        case WM_CLOSE:
            pDialogOptions->m_bProceedWithExport = FALSE;

            EndDialog(hWnd, 0);
            return TRUE;

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
            case IDC_GO:
                pDialogOptions->m_bProceedWithExport = TRUE;

                EndDialog(hWnd,0);
                break;

            case IDC_CANCEL:
                pDialogOptions->m_bProceedWithExport = FALSE;

                EndDialog(hWnd,0);
                break;

            case IDC_TEXT:
                pDialogOptions->m_xFormat = DXFILEFORMAT_TEXT;
                break;

            case IDC_BINARY:
                pDialogOptions->m_xFormat = DXFILEFORMAT_BINARY;
                break;

            case IDC_BINARYCOMPRESSED:
                pDialogOptions->m_xFormat = DXFILEFORMAT_BINARY | DXFILEFORMAT_COMPRESSED;
                break;

            case IDC_ANIMATION:
                pDialogOptions->m_bSaveAnimationData = !pDialogOptions->m_bSaveAnimationData;
                break;

            case IDC_LOOPINGANIMATION:
                pDialogOptions->m_bLoopingAnimationData = !pDialogOptions->m_bLoopingAnimationData;
                break;

            case IDC_PATCHDATA:
                pDialogOptions->m_bSavePatchData = !pDialogOptions->m_bSavePatchData;
                break;

            case IDC_SAMPLERATE:
                GetDlgItemText(hWnd,IDC_SAMPLERATE,buff,sizeof(buff) / sizeof(TCHAR));

                pDialogOptions->m_iAnimSamplingRate = _ttoi(buff);
                break;

            default:
                break;
            }
    }
    return FALSE;
}*/

// ================================================== FindPhysiqueModifier()
// Find if a given node contains a Physique Modifier
// DerivedObjectPtr requires you include "modstack.h" from the MAX SDK
Modifier* FindPhysiqueModifier (INode* nodePtr)
{
    // Get object from node. Abort if no object.
    Object* ObjectPtr = nodePtr->GetObjectRef();
    

    if ( NULL == ObjectPtr)
    {
        return NULL;
    }

    // Is derived object ?
    if (ObjectPtr->SuperClassID() == GEN_DERIVOB_CLASS_ID)
    {
        // Yes -> Cast.
        IDerivedObject* DerivedObjectPtr = static_cast<IDerivedObject*>(ObjectPtr);

        // Iterate over all entries of the modifier stack.
        int ModStackIndex = 0;
        while (ModStackIndex < DerivedObjectPtr->NumModifiers())
        {
            // Get current modifier.
            Modifier* ModifierPtr = DerivedObjectPtr->GetModifier(ModStackIndex);
            Class_ID clsid = ModifierPtr->ClassID();
            // Is this Physique ?
            if (ModifierPtr->ClassID() == Class_ID(PHYSIQUE_CLASS_ID_A, PHYSIQUE_CLASS_ID_B))
            {
                // Yes -> Exit.
                return ModifierPtr;
            }

            // Next modifier stack entry.
            ModStackIndex++;
        }
    }

    // Not found.
    return NULL;
}

// ================================================== GetTriObjectFromObjRef
// Return a pointer to a TriObject given an object reference or return NULL
// if the node cannot be converted to a TriObject
TriObject *GetTriObjectFromObjRef
	(
	Object* pObj, 
	BOOL *pbDeleteIt
	) 
{


	TriObject *pTri;

	assert(pObj != NULL);
	assert(pbDeleteIt != NULL);

    *pbDeleteIt = FALSE;

	//###danhoro $NOTE: meshes were not properly evaluated
	pObj= pObj->Eval(0).obj;

 	assert(pObj != NULL);

    if (pObj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0))) 
	{ 
        pTri = (TriObject *) pObj->ConvertToType(0, Class_ID(TRIOBJ_CLASS_ID, 0));

        // Note that the TriObject should only be deleted
        // if the pointer to it is not equal to the object
        // pointer that called ConvertToType()
        if (pObj != pTri) 
			*pbDeleteIt = TRUE;

        return pTri;
    } 
	else 
	{
        return NULL;
    }
}

// ================================================== GetTriObjectFromObjRef
// Return a pointer to a TriObject given an object reference or return NULL
// if the node cannot be converted to a TriObject
PatchObject *GetPatchObjectFromObjRef
	(
	Object* pObj, 
	BOOL *pbDeleteIt
	) 
{
	PatchObject *pPatchObject;

	assert(pObj != NULL);
	assert(pbDeleteIt != NULL);

    *pbDeleteIt = FALSE;

	//###danhoro $NOTE: meshes were not properly evaluated
	pObj= pObj->Eval(0).obj;

    if (pObj->CanConvertToType(Class_ID(PATCHOBJ_CLASS_ID, 0))) 
	{ 
        pPatchObject = (PatchObject *) pObj->ConvertToType(0, Class_ID(PATCHOBJ_CLASS_ID, 0));

        // Note that the PatchObject should only be deleted
        // if the pointer to it is not equal to the object
        // pointer that called ConvertToType()
        if (pObj != pPatchObject) 
			*pbDeleteIt = TRUE;

        return pPatchObject;
    } 
	else 
	{
        return NULL;
    }
}

//extern char* ToLowerCase(string& str);


// Include entity model in export?
BOOL IncludeEntityModel(bool m_bSavePrefabData, INode* pNode){
	NodeData data;
	if(GetNodeData(pNode,data)){
		if(m_bSavePrefabData && data.classname == "Prefab")
			return TRUE;

		return data.includeModel;
	}
	return TRUE;
}


BOOL HasProperties(INode* pNode){
	NodeData data;
	return GetNodeData(pNode,data);
}

// ================================================== IsExportableMesh()
// TIM: NOT EXPORTABLE IF IT'S A PREFAB. Only its node data will be exported in such cases
//
BOOL IsExportableMesh(bool m_bSavePrefabData, INode* pNode, Object** pObj)
{
    ULONG superClassID;
	Object *pObjBase;

    assert(pNode);

	pObjBase = pNode->GetObjectRef();
    if (pObjBase == NULL)
        return FALSE;

	if(pObj)*pObj= pObjBase;

    superClassID = pObjBase->SuperClassID();
    //find out if mesh is renderable

    if( !pNode->Renderable() || pNode->IsNodeHidden())
    {
        return FALSE;
    }

    BOOL bFoundGeomObject = FALSE;
    //find out if mesh is renderable (more)
    switch(superClassID)
    {
    case GEN_DERIVOB_CLASS_ID:

        do
        {
            pObjBase = ((IDerivedObject*)pObjBase)->GetObjRef();
            superClassID = pObjBase->SuperClassID();
        }
        while( superClassID == GEN_DERIVOB_CLASS_ID );

        switch(superClassID)
        {
        case GEOMOBJECT_CLASS_ID:
            bFoundGeomObject = TRUE;
            break;
        }
        break;

    case GEOMOBJECT_CLASS_ID:
        bFoundGeomObject = TRUE;

    default:
        break;
    }

	// Return FALSE if it's an entity not wanting to include its model
	if(!IncludeEntityModel(m_bSavePrefabData,pNode))
		return FALSE;

	if(pObj)*pObj= pObjBase;

    return bFoundGeomObject || pObjBase->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0));
}

// ================================================== IsExportablePatchMesh()
BOOL IsExportablePatchMesh(INode* pNode, Object** pObj)
{
    //ULONG superClassID;
	Object *pObjBase;

    assert(pNode != NULL);

	TCHAR* chNodeName= pNode->GetName();

    if( !pNode->Renderable() || pNode->IsNodeHidden())
        return FALSE;

	pObjBase = pNode->GetObjectRef();

	if (pObjBase == NULL)
        return FALSE;

	//###danhoro $NOTE this does not get the final object type but eval does
	pObjBase= pObjBase->Eval(0).obj;

    if (pObjBase == NULL)
        return FALSE;

	if(pObj)*pObj= pObjBase;

	//###danhoro $NOTE tripatches dont seem to be caught properly 
	//so you must request if it can be converted to a patch object
	//a class is considered to be a subclass of itself according to IsSubClassOf docs
	if (pObjBase->IsSubClassOf(Class_ID(PATCHOBJ_CLASS_ID, 0))
		|| pObjBase->IsSubClassOf(Class_ID(PATCHGRID_CLASS_ID, 0))
		|| pObjBase->IsSubClassOf(Class_ID(NURBSOBJ_CLASS_ID, 0)))
	{
		// then check to make sure that the intervening steps don't disallow
		//   a patch object to be converted to
		return pObjBase->CanConvertToType(Class_ID(PATCHOBJ_CLASS_ID, 0));
	}
	else
		return FALSE;
}

// ================================================== dummyFn()
// used by 3DS progress bar
DWORD WINAPI dummyFn(LPVOID arg)
{
    return(0);
}



HRESULT AddNormals
    (
    SSaveContext *psc,
    SMeshData *pMeshData, 
    BOOL bSwapTriOrder,
    LPDIRECTXFILEDATA pParent
    )
{
    HRESULT hr = S_OK;
    LPDIRECTXFILEDATA pDataObject = NULL;
    PBYTE         pbData = NULL;
    PBYTE         pbCur;        
    DWORD          cbSize;
    DWORD cNormals;
    DWORD cFaces;
    DWORD iFace;
    DWORD iVertex;

    assert(psc != NULL);
    assert(pParent != NULL);
    assert(pMeshData != NULL);

    cNormals = pMeshData->m_cVertices;
    cFaces = pMeshData->m_cFaces;

    cbSize = sizeof(DWORD) // nNormals
             + 3*sizeof(float)*cNormals // normals
             + sizeof(DWORD) // nFaces
             + cFaces* // MeshFace array
                (sizeof(DWORD) //nFaceVertexIndices (number of normal indices)
                 + 3*sizeof(DWORD)); // faceVertexIndices (normal indices)

    pbCur = pbData = new BYTE[cbSize];
    if (pbData == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }

    // nNormals
    WRITE_DWORD(pbCur, cNormals);

    // normals
    for (iVertex = 0; iVertex < pMeshData->m_cVertices; iVertex++)
    {
        WRITE_POINT3(pbCur, pMeshData->m_rgVertices[iVertex].vNormal);
    }

    // nFaces
    WRITE_DWORD(pbCur, cFaces);


    // MeshFace array
    for( iFace = 0; iFace < cFaces; iFace++ )
    {
        WRITE_DWORD(pbCur, 3); // nFaceVertexIndices (number of normal indices)

        // faceVertexIndices (indices to normals - same as indices to vertices for us)
        if( bSwapTriOrder )
        {
            WRITE_DWORD(pbCur, pMeshData->m_rgFaces[iFace].index[0]);
            WRITE_DWORD(pbCur, pMeshData->m_rgFaces[iFace].index[2]);
            WRITE_DWORD(pbCur, pMeshData->m_rgFaces[iFace].index[1]);
        }
        else
        {
            WRITE_DWORD(pbCur, pMeshData->m_rgFaces[iFace].index[0]);
            WRITE_DWORD(pbCur, pMeshData->m_rgFaces[iFace].index[1]);
            WRITE_DWORD(pbCur, pMeshData->m_rgFaces[iFace].index[2]);
        }
    }

    hr = psc->m_pxofsave->CreateDataObject(TID_D3DRMMeshNormals,
                                    NULL,
                                    NULL,
                                    cbSize,
                                    pbData,
                                    &pDataObject
                                    );
    if (FAILED(hr))
    {
        XSkinExp_OutputDebugString("Failed to create x file data object!");
        goto e_Exit;
    }

    hr = pParent->AddDataObject(pDataObject);
    if (FAILED(hr))
    {
        XSkinExp_OutputDebugString("Failed to add x file data object!");
        goto e_Exit;
    }

    // falling through
e_Exit:
    RELEASE(pDataObject);
    delete []pbData;

    return hr;
}

HRESULT AddTextureCoordinates
    (
    SSaveContext *psc,
    SMeshData *pMeshData, 
    SCropInfo *rgCropInfo,
    Mesh* pMesh,
    LPDIRECTXFILEDATA pParent
    )
{
    LPDIRECTXFILEDATA pDataObject = NULL;
    PBYTE         pbData = NULL;
    PBYTE         pbCur = NULL;
    DWORD         cbSize;
    DWORD         cTexCoords;
    DWORD         iVertex;
    DWORD         iTextureIndex;
    HRESULT    hr = S_OK;
    float         fX;
    float         fY;
    DWORD         iMaterial;

    assert( psc );
    assert( pParent );
    assert( pMesh );
    assert( pMeshData );

    // if no tex coords, then don't add them
    if( !pMeshData->m_bTexCoordsPresent )
        goto e_Exit;

    cTexCoords = pMeshData->m_cVertices;

    cbSize = sizeof(DWORD) //nTextureCoords
             + cTexCoords*2*sizeof(float); //texture coords

    pbCur = pbData = new BYTE[cbSize];
    if (pbData == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }

    WRITE_DWORD(pbCur, cTexCoords); //nTextureCoords

    for (iVertex = 0; iVertex < pMeshData->m_cVertices; iVertex++)
    {
        iTextureIndex  = pMeshData->m_rgVertices[iVertex].iTextureIndex;
        iMaterial = pMeshData->m_rgVertices[iVertex].iMaterial;
        if( iTextureIndex == 0xFFFFFFFF ) // none present, or bad data
        {
            WRITE_FLOAT(pbCur, 0); //u
            WRITE_FLOAT(pbCur, 0); //v
        }
        else
        {
            fX = pMesh->tVerts[iTextureIndex].x;
            fY = (1.0f - pMesh->tVerts[iTextureIndex].y);

			//###danhoro $REMOVE: fixed texture tiling
            //fX = (fX > 1.0f) ? 1.0f : ((fX < 0.0f) ? 0.0f : fX);
            //fY = (fY > 1.0f) ? 1.0f : ((fY < 0.0f) ? 0.0f : fY);

            fX = (fX * rgCropInfo[iMaterial].fClipW) + rgCropInfo[iMaterial].fClipU;
            fY = (fY * rgCropInfo[iMaterial].fClipH) + rgCropInfo[iMaterial].fClipV;

            WRITE_FLOAT(pbCur, fX); //u
            WRITE_FLOAT(pbCur, fY); //v
        }
    }

    hr = psc->m_pxofsave->CreateDataObject(TID_D3DRMMeshTextureCoords,
                                    NULL,
                                    NULL,
                                    cbSize,
                                    pbData,
                                    &pDataObject
                                    );
    if (FAILED(hr))
    {
        XSkinExp_OutputDebugString("Failed to create x file data object!");
        goto e_Exit;
    }

    hr = pParent->AddDataObject(pDataObject);
    if (FAILED(hr))
    {
        XSkinExp_OutputDebugString("Failed to add x file data object!");
        goto e_Exit;
    }

    // falling through
e_Exit:
    RELEASE(pDataObject);

    delete []pbData;
    return hr;
}

HRESULT AddVertexDuplicationIndices
    (
    SSaveContext *psc,
    SMeshData *pMeshData, 
    LPDIRECTXFILEDATA pParent
    )
{
    HRESULT    hr = S_OK;
    LPDIRECTXFILEDATA pDataObject = NULL;
    PBYTE         pbData = NULL;
    PBYTE         pbCur = NULL;
    DWORD         cbSize;
    DWORD         cIndices;
    DWORD         cVerticesBeforeDuplication;
    DWORD         iVertex;

    assert(psc != NULL);
    assert(pParent != NULL);
    assert(pMeshData != NULL);

    cIndices = pMeshData->m_cVertices;
    cVerticesBeforeDuplication = pMeshData->m_cVerticesBeforeDuplication;

	// if no new vertices, then don't add a record to the file
	if (pMeshData->m_cVerticesBeforeDuplication == pMeshData->m_cVertices)
		goto e_Exit;

    cbSize = sizeof(DWORD) //nIndices
             + sizeof(DWORD) //nVerticesBeforeDuplication
             + cIndices*sizeof(DWORD); // array of indices

    pbCur = pbData = new BYTE[cbSize];
    if (pbData == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }

    WRITE_DWORD(pbCur, cIndices) // nIndices;
    WRITE_DWORD(pbCur, cVerticesBeforeDuplication) // nVerticesBeforeDuplication
    
    for (iVertex = 0; iVertex < cIndices; iVertex++)
    {
        WRITE_DWORD(pbCur, pMeshData->m_rgVertices[iVertex].iPointRep); // index to original vertex without duplication.
    }

    hr = psc->m_pxofsave->CreateDataObject(DXFILEOBJ_VertexDuplicationIndices,
                                    NULL,
                                    NULL,
                                    cbSize,
                                    pbData,
                                    &pDataObject
                                    );
    if (FAILED(hr))
    {
        XSkinExp_OutputDebugString("Failed to create x file data object!");
        goto e_Exit;
    }

    hr = pParent->AddDataObject(pDataObject);
    if (FAILED(hr))
    {
        XSkinExp_OutputDebugString("Failed to add x file data object!");
        goto e_Exit;
    }

    // falling through
e_Exit:
    RELEASE(pDataObject);

    delete []pbData;
    return hr;
}


HRESULT
AddWireframeMaterial(
    SSaveContext *psc,
    INode *pNode,
    LPDIRECTXFILEDATA pParent
    )
{
    HRESULT hr = S_OK;
    DWORD cbSize;
    LPDIRECTXFILEDATA pDataObject = NULL;
    PBYTE pbData = NULL;
    PBYTE pbCur;
    DWORD dwWFColor;
    D3DXCOLOR colorWF;

    cbSize = 4*sizeof(float) // colorRGBA
             + sizeof(float) //power
             + 3*sizeof(float) //specularColor
             + 3*sizeof(float); //emissiveColor

    pbData = pbCur = new BYTE[cbSize];
    if (pbCur == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }
    
    // get the wireframe color
    dwWFColor = pNode->GetWireColor();
    //###danhoro $REMOVE not needed
	//dwWFColor |= 0xff000000;  // set alpha to fully opaque

    // convert to floating point
	//###danhoro $REMOVE incorrect conversion COLORREF to D3DXCOLOR
    //colorWF = D3DXCOLOR(dwWFColor);

	//###danhoro $NOTE added to convert COLORREF to D3DXCOLOR
	//XBGR: 0x00bbggrr
	colorWF.r= ((dwWFColor >>  0) & 0xff) / 256.0f;
	colorWF.g= ((dwWFColor >>  8) & 0xff) / 256.0f;
	colorWF.b= ((dwWFColor >> 16) & 0xff) / 256.0f;
	colorWF.a= 1.0f;


    //RGBA
    WRITE_COLOR(pbCur, colorWF);

    // Wireframe doesn't have an explicit specular power, so output our default.
    WRITE_FLOAT(pbCur, POWER_DEFAULT);

    // Set the specular color identical to diffuse, as recommended in 3DSMax docs.
    WRITE_FLOAT(pbCur, colorWF.r);
    WRITE_FLOAT(pbCur, colorWF.g);
    WRITE_FLOAT(pbCur, colorWF.b);

    // Set the luminence to 0: the material isn't glowing.
    WRITE_FLOAT(pbCur, 0.0f);
    WRITE_FLOAT(pbCur, 0.0f);
    WRITE_FLOAT(pbCur, 0.0f);

    hr = psc->m_pxofsave->CreateDataObject(TID_D3DRMMaterial,
                                    NULL,
                                    NULL,
                                    cbSize,
                                    pbData,
                                    &pDataObject
                                    );

    hr = pParent->AddDataObject(pDataObject);
    if (FAILED(hr))
        goto e_Exit;

e_Exit:
    delete []pbData;
    RELEASE(pDataObject);

    return hr;
}

HRESULT
AddTextureFilename(
    SSaveContext *psc,
    TCHAR *szName,
    LPDIRECTXFILEDATA pParent
    )
{
    HRESULT hr = S_OK;
    LPDIRECTXFILEDATA pDataObject = NULL;
    DWORD cbSize;

    cbSize = sizeof(TCHAR**);

    hr = psc->m_pxofsave->CreateDataObject(TID_D3DRMTextureFilename,
                                    NULL,
                                    NULL,
                                    cbSize,
                                    &szName,
                                    &pDataObject
                                    );
    if (FAILED(hr))
        goto e_Exit;

    hr = pParent->AddDataObject(pDataObject);
    if (FAILED(hr))
        goto e_Exit;

e_Exit:
    RELEASE(pDataObject);

    return hr;
}

// ================================================== FindTextureFileName
// callback called by 3DSMAX
class FindTextureFilename : public NameEnumCallback
{
public:
    FindTextureFilename()
        :m_szTextureName(NULL) {}

    virtual void RecordName(TCHAR *szName)
    {
        m_szTextureName = szName;
    }

    TCHAR *m_szTextureName;    
};


HRESULT
AddMaterial(
    SSaveContext *psc,
    CUSTOMD3DXMATERIAL *pMaterial,
    LPDIRECTXFILEDATA pParent
    )
{
    HRESULT hr = S_OK;
    LPDIRECTXFILEDATA pDataObject = NULL;
    PBYTE pbData = NULL;
    PBYTE pbCur;
    DWORD cbSize;

    cbSize = 4*sizeof(float) // colorRGBA
             + sizeof(float) //power
             + 3*sizeof(float) //specularColor
             + 3*sizeof(float); //emissiveColor

    pbData = pbCur = new BYTE[cbSize];
    if (pbCur == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }
    
    // diffuse - write white RGBA
    WRITE_FLOAT(pbCur, pMaterial->MatD3D.Diffuse.r);
    WRITE_FLOAT(pbCur, pMaterial->MatD3D.Diffuse.g);
    WRITE_FLOAT(pbCur, pMaterial->MatD3D.Diffuse.b);
    WRITE_FLOAT(pbCur, pMaterial->MatD3D.Diffuse.a);

    // specular power
    WRITE_FLOAT(pbCur, POWER_DEFAULT);

    // specular - write white RGBA
    WRITE_FLOAT(pbCur, pMaterial->MatD3D.Specular.r);
    WRITE_FLOAT(pbCur, pMaterial->MatD3D.Specular.g);
    WRITE_FLOAT(pbCur, pMaterial->MatD3D.Specular.b);

    // emmissive - write white RGBA
    WRITE_FLOAT(pbCur, pMaterial->MatD3D.Emissive.r);
    WRITE_FLOAT(pbCur, pMaterial->MatD3D.Emissive.g);
    WRITE_FLOAT(pbCur, pMaterial->MatD3D.Emissive.b);

    hr = psc->m_pxofsave->CreateDataObject(TID_D3DRMMaterial,
                                    NULL,
                                    NULL,
                                    cbSize,
                                    pbData,
                                    &pDataObject
                                    );
    if (FAILED(hr))
        goto e_Exit;

    // if there is a bitmap texture, then add the filename
    if (pMaterial->pTextureFilename != NULL)
    {
        hr = AddTextureFilename(psc, pMaterial->pTextureFilename, pDataObject);
        if (FAILED(hr))
            goto e_Exit;
    }

    hr = pParent->AddDataObject(pDataObject);
    if (FAILED(hr))
        goto e_Exit;

e_Exit:
    delete []pbData;
    RELEASE(pDataObject);

    return hr;
}

HRESULT
GatherMeshMaterials(
    SSaveContext *psc,
    INode *pNode,
    Mesh *pMesh,
    DWORD **prgdwMeshMaterials,
    CUSTOMD3DXMATERIAL **prgMaterials,
    SCropInfo **prgCropInfo,
    DWORD *pcMaterials
    )
{
    HRESULT hr = S_OK;
    Mtl *pMtlMain;
    Mtl *pMtlCur;
    DWORD iCurMaterial;
    DWORD cSubMaterials;
    DWORD iFace;
    DWORD cFaces;
    BOOL bNoSubMaterials;
    BOOL bWireframeColor;
    DWORD dwCurMatID;
    DWORD *rgdwMeshMaterials = NULL;
    SCropInfo *rgCropInfo = NULL;
    IParamBlock2 *pParamBlock;
    Texmap *pTexMap;
    CUSTOMD3DXMATERIAL *rgMaterials = NULL;
    TCHAR *szFilename;
    DWORD dwWFColor;
    D3DXCOLOR colorWF;
    BOOL bDetailMap;
    FindTextureFilename findTextureFilename;

    pMtlMain = pNode->GetMtl();
    cFaces = pMesh->getNumFaces();
    cSubMaterials = 0;
    bNoSubMaterials = FALSE;
    bWireframeColor = FALSE;

    rgdwMeshMaterials = new DWORD[cFaces];
    if (rgdwMeshMaterials == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }

    // The color of a given mesh can be provided by three different sources:
    //   1) applied texture maps, as part of a material
    //   2) explicitly defined & applied materials without texture maps
    //   3) a 'wireframe' color.
    
    // For our purposes, we will output these in this order of preference, ignoring
    // processing past the first one we find.

    if (pMtlMain != NULL)
    {
        // There is at least one material. We're in case 1) or 2)

        cSubMaterials = pMtlMain->NumSubMtls();
        if (cSubMaterials < 1)
        {
            // Count the material itself as a submaterial.
            cSubMaterials = 1;
            bNoSubMaterials = TRUE;
        }
    }
    else  // no material, then we'll create a material corresponding to the default wire color.
    {
        bWireframeColor = TRUE;
        cSubMaterials = 1;
    }

    for (iFace=0; iFace < cFaces; iFace++)
    {
        if (bWireframeColor || bNoSubMaterials) 
        {
            // If we're using wireframe color, it's our only material
            rgdwMeshMaterials[iFace] = 0;
        }
        else
        {
            // Otherwise we have at least one material to process.

            dwCurMatID = pMesh->faces[iFace].getMatID();

            if (cSubMaterials == 1)
            {
                dwCurMatID = 0;
            }
            else
            {
                // SDK recommends mod'ing the material ID by the valid # of materials, 
                // as sometimes a material number that's too high is returned.
                dwCurMatID %= cSubMaterials;
            }

            // output the appropriate material color

            rgdwMeshMaterials[iFace] = dwCurMatID;

        } 

    } 

    rgCropInfo = new SCropInfo[cSubMaterials];
    rgMaterials = new CUSTOMD3DXMATERIAL[cSubMaterials];
    if ((rgCropInfo == NULL) || (rgMaterials == NULL))
    {
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }
    memset(rgMaterials, 0, sizeof(CUSTOMD3DXMATERIAL) * cSubMaterials);

    for (iCurMaterial = 0; iCurMaterial < cSubMaterials; iCurMaterial++)
    {
        rgCropInfo[iCurMaterial].fClipU = 0.0f;
        rgCropInfo[iCurMaterial].fClipV = 0.0f;
        rgCropInfo[iCurMaterial].fClipW = 1.0f;
        rgCropInfo[iCurMaterial].fClipH = 1.0f;
    }

    if (!bWireframeColor)
    {
        // 3DSMax allows multiple materials to be used on a single mesh via
        // 'submaterials'. When the first submaterial is defined, the main material
        // is copied into submaterial #1, and the new submaterial because submaterial #2.
        // 
        // We have to catch the case where there's a material, but no submaterials. For this
        // case, set NumSubMaterials to 1 which would never happen otherwise. It's imperative
        // that the first material be set to MtlMain, rather than trying to GetSubMtl() to 
        // allow this logic to work.

        // Loop through the materials (if any) and output them.

        // first collect the crop info and the list of materials
        for (iCurMaterial = 0; iCurMaterial < cSubMaterials; iCurMaterial++)
        {
            if (bNoSubMaterials)
            {
                // We're on the parent material, possibly the ONLY material.
                // We won't be able to get it with GetSubMtl() if it's the only one, and
                // the data in the first submaterial is identical to the main material,
                // so just use the main material.

                pMtlCur = pMtlMain;
            }
            else
            {
                // We're into the true submaterials. Safe to get with 'GetSubMtl'

                pMtlCur = pMtlMain->GetSubMtl(iCurMaterial);
            }

            // if using texture as a detail map, get the crop info
			bDetailMap = FALSE;
            pTexMap = pMtlCur->GetSubTexmap(ID_DI);
            if ((pTexMap != NULL) && (pTexMap->ClassID() == Class_ID(BMTEX_CLASS_ID, 0x00)))
            {
                pParamBlock = pTexMap->GetParamBlock(0);

                if (pParamBlock != NULL)
                {
                    pParamBlock->GetValue(bmtex_clipu, 0, rgCropInfo[iCurMaterial].fClipU, FOREVER);
                    pParamBlock->GetValue(bmtex_clipv, 0, rgCropInfo[iCurMaterial].fClipV, FOREVER);
                    pParamBlock->GetValue(bmtex_clipw, 0, rgCropInfo[iCurMaterial].fClipW, FOREVER);
                    pParamBlock->GetValue(bmtex_cliph, 0, rgCropInfo[iCurMaterial].fClipH, FOREVER);       
                }

                bDetailMap = TRUE;
            }

            // 'Ambient color' is specified in Max docs as "the color of the object in shadows," and 
            //                 is usually a darker version of the diffuse color.
            // 'Diffuse color' is specified in the Max docs as "the color of the object in 
            //                  good lighting," usually referred to as the objects' color.
            // 'Specular color' is specified as the color of reflection highlights in direct lighting,
            //                  and according to Max docs is usually the same as diffuse.

           /* if (!bDetailMap)
            {
                // We're going to use the 'Diffuse' color as the object color for DirectX
                rgMaterials[iCurMaterial].MatD3D.Diffuse.r = pMtlCur->GetDiffuse().r;
                rgMaterials[iCurMaterial].MatD3D.Diffuse.g = pMtlCur->GetDiffuse().g;
                rgMaterials[iCurMaterial].MatD3D.Diffuse.b = pMtlCur->GetDiffuse().b;

                //Alpha
                rgMaterials[iCurMaterial].MatD3D.Diffuse.a = 1.0f - pMtlCur->GetXParency();

                // 3DSMax specular power is comprised of shininess, and shininess strength.
                // TODO - figure out a mapping from shininess to power
                rgMaterials[iCurMaterial].MatD3D.Power = pMtlCur->GetShininess();

                // Specular
                rgMaterials[iCurMaterial].MatD3D.Specular.r = pMtlCur->GetSpecular().r;
                rgMaterials[iCurMaterial].MatD3D.Specular.g = pMtlCur->GetSpecular().g;
                rgMaterials[iCurMaterial].MatD3D.Specular.b = pMtlCur->GetSpecular().b;
                rgMaterials[iCurMaterial].MatD3D.Specular.a = 1.0f;

                // Emmissive
                rgMaterials[iCurMaterial].MatD3D.Emissive.r = pMtlCur->GetSelfIllumColor().r;
                rgMaterials[iCurMaterial].MatD3D.Emissive.g = pMtlCur->GetSelfIllumColor().g;
                rgMaterials[iCurMaterial].MatD3D.Emissive.b = pMtlCur->GetSelfIllumColor().b;
                rgMaterials[iCurMaterial].MatD3D.Emissive.a = 1.0f;
            }
            else*/  // if a detail map, then don't write the color
            {
                // diffuse - write white RGBA
                rgMaterials[iCurMaterial].MatD3D.Diffuse.r = 1.0f;
                rgMaterials[iCurMaterial].MatD3D.Diffuse.g = 1.0f;
                rgMaterials[iCurMaterial].MatD3D.Diffuse.b = 1.0f;
                rgMaterials[iCurMaterial].MatD3D.Diffuse.a = 1.0f;

				rgMaterials[iCurMaterial].MatD3D.Power = pMtlCur->GetShininess();

                // specular - write white RGBA
                rgMaterials[iCurMaterial].MatD3D.Specular.r = 1.0f;
                rgMaterials[iCurMaterial].MatD3D.Specular.g = 1.0f;
                rgMaterials[iCurMaterial].MatD3D.Specular.b = 1.0f;
                rgMaterials[iCurMaterial].MatD3D.Specular.a = 1.0f;

                // emmissive - write white RGBA
                rgMaterials[iCurMaterial].MatD3D.Emissive.r = 0.0f;
                rgMaterials[iCurMaterial].MatD3D.Emissive.g = 0.0f;
                rgMaterials[iCurMaterial].MatD3D.Emissive.b = 0.0f;
                rgMaterials[iCurMaterial].MatD3D.Emissive.a = 1.0f;
            }

            if (bDetailMap)
            {
                pTexMap->EnumAuxFiles(findTextureFilename, FILE_ENUM_ALL);

                if (findTextureFilename.m_szTextureName == NULL)
                {
                    XSkinExp_OutputDebugString("AddMaterial: Bitmap texture found, but no texture name\n");
                    szFilename = NULL;
                }
                else // texture filename found
                {
                    // allocate a new string, doing the '\' fixup if neccessary
                    if (psc->m_xFormat == DXFILEFORMAT_TEXT)
                        szFilename = psc->m_stStrings.CreateNiceFilename(findTextureFilename.m_szTextureName); /*expand \ char to \\ */
                    else
                        szFilename = psc->m_stStrings.CreateNormalFilename(findTextureFilename.m_szTextureName); /*DON'T! expand \ char to \\ */

			        if (szFilename == NULL)
			        {
				        hr = E_OUTOFMEMORY;
				        goto e_Exit;
			        }
                }

                rgMaterials[iCurMaterial].pTextureFilename = szFilename;
            }

			// TIMJOHNSON: Custom material additions
			GatherCustomMaterialProperties(psc, &rgMaterials[iCurMaterial],pMtlCur);
        } 
    }
    else // wireframe color
    {
       // get the wireframe color
        dwWFColor = pNode->GetWireColor();
		//###danhoro $REMOVE no need for this
        //dwWFColor |= 0xff000000;  // set alpha to fully opaque

        // convert to floating point
		//###danhoro $REMOVE this was wrong and inverted the colors
         //colorWF = D3DXCOLOR(dwWFColor);

		//###danhoro $NOTE added to convert COLORREF to D3DXCOLOR
		//XBGR: 0x00bbggrr
		colorWF.r= ((dwWFColor >>  0) & 0xff) / 256.0f;
		colorWF.g= ((dwWFColor >>  8) & 0xff) / 256.0f;
		colorWF.b= ((dwWFColor >> 16) & 0xff) / 256.0f;
		colorWF.a= 1.0f;

        //RGBA
        rgMaterials[0].MatD3D.Diffuse.r = colorWF.r;
        rgMaterials[0].MatD3D.Diffuse.g = colorWF.g;
        rgMaterials[0].MatD3D.Diffuse.b = colorWF.b;
        rgMaterials[0].MatD3D.Diffuse.a = 1.0f;

        // Wireframe doesn't have an explicit specular power, so output our default.
        rgMaterials[0].MatD3D.Power = POWER_DEFAULT;

        // Set the specular color identical to diffuse, as recommended in 3DSMax docs.
        rgMaterials[0].MatD3D.Specular.r = colorWF.r;
        rgMaterials[0].MatD3D.Specular.g = colorWF.g;
        rgMaterials[0].MatD3D.Specular.b = colorWF.b;
        rgMaterials[0].MatD3D.Specular.a = 1.0f;

        // Set the luminence to 0: the material isn't glowing.
        rgMaterials[0].MatD3D.Emissive.r = 0.0f;
        rgMaterials[0].MatD3D.Emissive.g = 0.0f;
        rgMaterials[0].MatD3D.Emissive.b = 0.0f;
        rgMaterials[0].MatD3D.Emissive.a = 1.0f;

    }

    *prgdwMeshMaterials = rgdwMeshMaterials;
    rgdwMeshMaterials = NULL;

    *prgCropInfo = rgCropInfo;
    rgCropInfo = NULL;

    *prgMaterials = rgMaterials;
    rgMaterials = NULL;

    *pcMaterials = cSubMaterials;

e_Exit:
    delete []rgdwMeshMaterials;
    delete []rgMaterials;
    delete []rgCropInfo;
    return hr;
}




HRESULT
AddMeshMaterials(
    SSaveContext *psc,
    INode *pNode,
    Mesh *pMesh,
    DWORD *rgdwMeshMaterials,
    CUSTOMD3DXMATERIAL *rgMaterials,
    DWORD& cMaterials,
    LPDIRECTXFILEDATA pParent
    )
{
    HRESULT hr = S_OK;
    LPDIRECTXFILEDATA pDataObject = NULL;
    DWORD iFace;
    DWORD cFaces;
    PBYTE pbCur;
    PBYTE pbData = NULL;
    DWORD cbSize;
    DWORD iCurMaterial;
    BOOL bFound;
    DWORD cUniqueMaterials;
    DWORD iSearchMaterial;
    DWORD iPrevMaterial;
    DWORD iDestMaterial;
    DWORD *rgdwMaterialRemap = NULL;

    cFaces = pMesh->getNumFaces();


    rgdwMaterialRemap = new DWORD[cMaterials];
    if (rgdwMaterialRemap == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }

    // first go through and remove redundant materials (might not have had the same crop info!)
    cUniqueMaterials = 1;
    rgdwMaterialRemap[0] = 0;
    for (iCurMaterial = 1; iCurMaterial < cMaterials; iCurMaterial++)
    {
        bFound = FALSE;
        for (iSearchMaterial = 0; iSearchMaterial < iCurMaterial; iSearchMaterial++)
        {
            if (iSearchMaterial == iCurMaterial)
                continue;

			// TODO: Add this filtering back, but we must check every single value
			// -- Perhaps not. Seems to be only per-mesh filtering
            // if both the material and texture name are the same, then it is a redundant material
            /*if ((memcmp(&(rgMaterials[iCurMaterial].MatD3D), &(rgMaterials[iSearchMaterial].MatD3D), sizeof(D3DMATERIAL8)) == 0)
                && ( (rgMaterials[iCurMaterial].pTextureFilename == rgMaterials[iSearchMaterial].pTextureFilename)
                      || ((rgMaterials[iCurMaterial].pTextureFilename != NULL) 
                             && (rgMaterials[iSearchMaterial].pTextureFilename != NULL)
                             && (strcmp(rgMaterials[iCurMaterial].pTextureFilename, rgMaterials[iSearchMaterial].pTextureFilename) == 0))))
            {
                bFound = TRUE;
                break;
            }*/

        }

        // if found, just remap to the material that was found
        if (bFound)
        {
            rgdwMaterialRemap[iCurMaterial] = rgdwMaterialRemap[iSearchMaterial];
        }
        else  // not found, another unique material
        {
            rgdwMaterialRemap[iCurMaterial] = cUniqueMaterials;
            cUniqueMaterials += 1;
        }
    }

    // now remap the materials
    iPrevMaterial = 0;
    for (iCurMaterial = 1; iCurMaterial < cMaterials; iCurMaterial++)
    {
        iDestMaterial = rgdwMaterialRemap[iCurMaterial];

        // if a unique material, then move it
        if (iDestMaterial > iPrevMaterial)
        {
            iPrevMaterial += 1;
            assert(iDestMaterial == iPrevMaterial);

            // if not staying in the same place, then copy it
            if (iCurMaterial != iDestMaterial)
            {
                memcpy(&rgMaterials[iDestMaterial], &rgMaterials[iCurMaterial], sizeof(CUSTOMD3DXMATERIAL));
            }
        }
    }


    cbSize = sizeof(DWORD) // nMaterials
                + sizeof(DWORD) // nFaceIndexes
                + cFaces*sizeof(DWORD); // face indexes

    pbData = pbCur = new BYTE[cbSize];
    if (pbCur == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }

    WRITE_DWORD(pbCur, cUniqueMaterials);
    WRITE_DWORD(pbCur, cFaces);


    // For each face, output the index of the material which applies to it, 
    // starting from  0

    for (iFace=0; iFace < cFaces; iFace++)
    {
        // don't forget to remap the matrerial before writing it to the file
        WRITE_DWORD(pbCur, rgdwMaterialRemap[rgdwMeshMaterials[iFace]];);
    } 

    // now finally create the mesh material list
    hr = psc->m_pxofsave->CreateDataObject(TID_D3DRMMeshMaterialList,
                                    NULL,
                                    NULL,
                                    cbSize,
                                    pbData,
                                    &pDataObject
                                    );
    if (FAILED(hr))
        goto e_Exit;

    hr = pParent->AddDataObject(pDataObject);
    if (FAILED(hr))
        goto e_Exit;

    for (iCurMaterial = 0; iCurMaterial < cUniqueMaterials; iCurMaterial++)
    {
        hr = AddMaterial(psc, &rgMaterials[iCurMaterial], pDataObject);
        if (FAILED(hr))
            goto e_Exit;
    } 

	cMaterials = cUniqueMaterials;

e_Exit:
    delete []pbData;
    delete []rgdwMaterialRemap;
    RELEASE(pDataObject);


    return hr;
} 

// structe used by AddSkinData to convert skin data from per vertex to per bone
struct SBoneState
{
    SBoneState()
        :m_pbData(NULL), m_szBoneName(NULL) {}
    ~SBoneState()
        { 
            delete []m_pbData; 

			// Bone name is allocated out of a string pool because it has to be deleted after
			//   the data has been saved to disk
            //delete []m_szBoneName; 
        }

    // info computed up front (note m_rgdwIndices, m_rgfWeights and m_pmatOFfset point into pbData)
    INode *m_pBoneNode;
    DWORD m_cVertices;
    DWORD m_cbData;
    PBYTE m_pbData;
    char *m_szBoneName;
    DWORD *m_rgdwIndices;
    float *m_rgfWeights;
    D3DXMATRIX *m_pmatOffset;

    // current index of last added vertex index
    DWORD m_iCurIndex;
};

SBoneState *FindBoneData
    (
    INode *pBoneNode, 
    SBoneState *rgbsBoneData, 
    DWORD cBones
    )
{
    DWORD iBone;
    SBoneState *pbs = NULL;

    for (iBone = 0; iBone < cBones; iBone++)
    {
        if (rgbsBoneData[iBone].m_pBoneNode == pBoneNode)
        {
            pbs = &rgbsBoneData[iBone];
            break;
        }
    }

    return pbs;
}

HRESULT AddSkinData
    (
    SSaveContext *psc,
    INode *pNode,
    SMeshData *pMeshData,
    LPDIRECTXFILEDATA pParent
    )
{
    HRESULT hr = S_OK;
    IPhyVertexExport *pVertexExport;
    IPhyRigidVertex* pRigidVertexExport;
    IPhyBlendedRigidVertex *pBlendedRigidVertexExport;
    INode* pBoneNode;
    SSkinMap *psmSkinMap = NULL;
    Modifier* pPhyMod = NULL;
    IPhysiqueExport* pPhyExport = NULL;
    IPhyContextExport* pPhyContextExport = NULL;
    SBoneState *rgbsBoneData = NULL;
    SBoneState *rgbsBoneDataDups = NULL;
    SBoneState *pbsCurData = NULL;
    SBoneState *pbsOldData = NULL;
    DWORD iVertex;
    DWORD cVertices;
    DWORD iVertexType;
    DWORD cTotalBones;
    DWORD iBone;
    PBYTE pbCur;
    float fWeight;
    LPDIRECTXFILEDATA pDataObject = NULL;
    DWORD iIndex;
    DWORD iNextWedge;
    Matrix3 matMesh;
    Matrix3 matOffset;
    Matrix3 matZero;
	PBYTE pbHeaderData = NULL;
	DWORD cbHeaderData;
    static bool shownError = false;

    matZero.Zero();
    pPhyMod = FindPhysiqueModifier(pNode);
    if (pPhyMod != NULL)
    {
        // Get a Physique Export interface
        pPhyExport = (IPhysiqueExport *)pPhyMod->GetInterface(I_PHYINTERFACE);
        if (pPhyExport != NULL)
        { 
			pPhyExport->SetInitialPose(TRUE);
        }
		else
		{  // not all interfaces present, so ignore
            goto e_NoPhysique;
		}

        // get the init matrix for the mesh (used to mult by the bone matrices)
		matMesh= pNode->GetObjTMAfterWSM(0);

		//###danhoro $REMOVE this doesnt seem to really work as well as setting the physique to initial pose
		//###we don't want the node TM anyways!  we want the objTM After WSM
		//int rval = pPhyExport->GetInitNodeTM(pNode, matMesh);
        //if (rval || matMesh.Equals(matZero, 0.0))
        //{
        //    if (!shownError)
        //    {
        //        shownError = true;
        //        MessageBox(NULL, "This mesh was skinned using an older version of Character Studio. Mesh may not be exported correctly",
        //                   "X File export Warning", MB_OK);
        //    }
        //}

        // For a given Object's INode get a
        // ModContext Interface from the Physique Export Interface:
        pPhyContextExport = (IPhyContextExport *)pPhyExport->GetContextInterface(pNode);
        if(pPhyContextExport == NULL)
        {
            // not all interfaces present, so ignore
            goto e_NoPhysique;
        }

        // should already have been converted to rigid with blending by Preprocess
            // convert to rigid with blending
            pPhyContextExport->ConvertToRigid(TRUE);
            pPhyContextExport->AllowBlending(TRUE);

        psmSkinMap = psc->GetSkinMap(pNode);
        if (psmSkinMap == NULL)
        {
            XSkinExp_OutputDebugString("AddSkinData: Found physique info at save time, but not preprocess\n");
            hr = E_FAIL;
            goto e_Exit;
        }

        // now setup state used to fill arrays for output 
        rgbsBoneData = new SBoneState[psmSkinMap->m_cbiBones];
        if (rgbsBoneData == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto e_Exit;
        }

        // intialize each of the bone structures 
        for (iBone = 0; iBone < psmSkinMap->m_cbiBones; iBone++)
        {
            pbsCurData = &rgbsBoneData[iBone];

            pbsCurData->m_iCurIndex = 0;
            pbsCurData->m_cVertices = psmSkinMap->m_rgbiBones[iBone].m_cVertices;
            pbsCurData->m_pBoneNode = psmSkinMap->m_rgbiBones[iBone].m_pBoneNode;

            // allocate memory to pass to D3DXOF lib
            pbsCurData->m_cbData = sizeof(TCHAR*)
                                    + sizeof(DWORD) // numWeights
                                    + sizeof(DWORD) * pbsCurData->m_cVertices // array of vertex indices
                                    + sizeof(float) * pbsCurData->m_cVertices // array of weights
                                    + sizeof(float) * 16; // offset matrix

            pbCur = pbsCurData->m_pbData = new BYTE[pbsCurData->m_cbData];
            if (pbsCurData->m_pbData == NULL)
            {
                hr = E_OUTOFMEMORY;
                goto e_Exit;
            }

            // fill first few entries, and remember pointers to the other arrays/matrix
            WRITE_PTCHAR(pbCur, pbsCurData->m_szBoneName);
            WRITE_DWORD(pbCur, pbsCurData->m_cVertices);

            pbsCurData->m_rgdwIndices = (DWORD*)pbCur;
            pbCur += sizeof(DWORD) * pbsCurData->m_cVertices;

            pbsCurData->m_rgfWeights = (float*)pbCur;
            pbCur += sizeof(float) * pbsCurData->m_cVertices;

            pbsCurData->m_pmatOffset = (D3DXMATRIX*)pbCur;

            // compute the mat offset
			matOffset= pbsCurData->m_pBoneNode->GetNodeTM(0);

			//###danhoro $REMOVE this doesnt seem to really work as well as setting the physique to initial pose
            //int rval = pPhyExport->GetInitNodeTM(pbsCurData->m_pBoneNode, matOffset);
            //if (rval)
            //{
            //    MessageBox(NULL, "This mesh was skinned using an older version of Character Studio. Mesh may not be exported correctly",
            //               "X File export Warning", MB_OK);
            //}

            matOffset.Invert();
            matOffset = matMesh * matOffset;

            WRITE_MATRIX4_FROM_MATRIX3(pbCur, matOffset);
        }

        cVertices = pPhyContextExport->GetNumberVertices();
        for (iVertex = 0; iVertex < cVertices; iVertex++ )
        {
            pVertexExport = (IPhyVertexExport *)pPhyContextExport->GetVertexInterface(iVertex);    
            if (pVertexExport == NULL)
            {
				XSkinExp_OutputDebugString("AddSkinData: Couldn't get physique vertex export interface!");
                hr = E_FAIL;
                goto e_Exit;
            }
        
            // What kind of vertices are these?
            iVertexType = pVertexExport->GetVertexType();


            if( iVertexType == RIGID_TYPE )
            {
                pRigidVertexExport = (IPhyRigidVertex *)pVertexExport;//pPhyContextExport->GetVertexInterface(iVertex);

				// Get the vertex info!
            
                pBoneNode = pRigidVertexExport->GetNode();

                pbsCurData = FindBoneData(pBoneNode, rgbsBoneData, psmSkinMap->m_cbiBones);
                if (pbsCurData == NULL)
                {
                    assert(0);
                    XSkinExp_OutputDebugString("AddSkinData: Bone node not found on second pass\n");
                    hr = E_FAIL;
                    goto e_Exit;
                }

                pbsCurData->m_rgdwIndices[pbsCurData->m_iCurIndex] = iVertex;
                pbsCurData->m_rgfWeights[pbsCurData->m_iCurIndex] = 1.0f;
                pbsCurData->m_iCurIndex += 1;

            }
            else
            {
                assert( iVertexType == RIGID_BLENDED_TYPE );

                pBlendedRigidVertexExport = (IPhyBlendedRigidVertex *)pVertexExport;//pPhyContextExport->GetVertexInterface(iVertex);


                // How many nodes affect his vertex?
                cTotalBones = pBlendedRigidVertexExport->GetNumberNodes();
                for (iBone = 0; iBone < cTotalBones; iBone++ )
                {
                    pBoneNode = pBlendedRigidVertexExport->GetNode(iBone);

                    pbsCurData = FindBoneData(pBoneNode, rgbsBoneData, psmSkinMap->m_cbiBones);
                    if (pbsCurData == NULL)
                    {
                        assert(0);
                        XSkinExp_OutputDebugString("AddSkinData: Bone node not found on second pass\n");
                        hr = E_FAIL;
                        goto e_Exit;
                    }

                    fWeight = pBlendedRigidVertexExport->GetWeight(iBone);

                    // first see if it is a repeat weight, is so just add to previous
                    if ((pbsCurData->m_iCurIndex > 0) && (pbsCurData->m_rgdwIndices[pbsCurData->m_iCurIndex-1] == iVertex))
                    {
                        pbsCurData->m_rgfWeights[pbsCurData->m_iCurIndex-1] += fWeight;
                    }
                    else
                    {
                        pbsCurData->m_rgdwIndices[pbsCurData->m_iCurIndex] = iVertex;
                        pbsCurData->m_rgfWeights[pbsCurData->m_iCurIndex] = fWeight;
                        pbsCurData->m_iCurIndex += 1;
                    }                    
                }
            }

			pPhyContextExport->ReleaseVertexInterface( pVertexExport );    
        }
    }
e_NoPhysique:



    if (rgbsBoneData != NULL)
    {
        assert(psmSkinMap != NULL);

// now deal with the wonderful world of duplicated indices
        rgbsBoneDataDups = new SBoneState[psmSkinMap->m_cbiBones];
        if (rgbsBoneDataDups == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto e_Exit;
        }

        // first calculate new lengths for the bone weight arrays
        for (iBone = 0; iBone < psmSkinMap->m_cbiBones; iBone++)
        {
            pbsOldData = &rgbsBoneData[iBone];
            pbsCurData = &rgbsBoneDataDups[iBone];

            pbsCurData->m_cVertices = pbsOldData->m_cVertices;
            for (iIndex = 0; iIndex < pbsOldData->m_cVertices; iIndex++)
            {
                iVertex = pbsOldData->m_rgdwIndices[iIndex];

                iNextWedge = pMeshData->m_rgVertices[iVertex].iWedgeList;
                while (iVertex != iNextWedge)
                {
                    pbsCurData->m_cVertices += 1;

                    iNextWedge = pMeshData->m_rgVertices[iNextWedge].iWedgeList;
                }
            }
        }

        // next build 
        for (iBone = 0; iBone < psmSkinMap->m_cbiBones; iBone++)
        {
            pbsOldData = &rgbsBoneData[iBone];
            pbsCurData = &rgbsBoneDataDups[iBone];

            pbsCurData->m_pBoneNode = pbsOldData->m_pBoneNode;
            pbsCurData->m_iCurIndex = 0;

            // allocate memory to pass to D3DXOF lib
            pbsCurData->m_cbData = sizeof(TCHAR*)
                                    + sizeof(DWORD) // numWeights
                                    + sizeof(DWORD) * pbsCurData->m_cVertices // array of vertex indices
                                    + sizeof(float) * pbsCurData->m_cVertices // array of weights
                                    + sizeof(float) * 16; // offset matrix

            pbCur = pbsCurData->m_pbData = new BYTE[pbsCurData->m_cbData];
            pbsCurData->m_szBoneName = psc->m_stStrings.CreateNiceString(pbsCurData->m_pBoneNode->GetName());
            if ((pbsCurData->m_pbData == NULL) || (pbsCurData->m_szBoneName == NULL))
            {
                hr = E_OUTOFMEMORY;
                goto e_Exit;
            }

            // fill first few entries, and remember pointers to the other arrays/matrix
            WRITE_PTCHAR(pbCur, pbsCurData->m_szBoneName);
            WRITE_DWORD(pbCur, pbsCurData->m_cVertices);

            pbsCurData->m_rgdwIndices = (DWORD*)pbCur;
            pbCur += sizeof(DWORD) * pbsCurData->m_cVertices;

            pbsCurData->m_rgfWeights = (float*)pbCur;
            pbCur += sizeof(float) * pbsCurData->m_cVertices;

            pbsCurData->m_pmatOffset = (D3DXMATRIX*)pbCur;

            // already loaded above, copy from the original state data
            *pbsCurData->m_pmatOffset = *pbsOldData->m_pmatOffset;


            // now that we have the new states set up, copy the data from the old states
            for (iIndex = 0; iIndex < pbsOldData->m_cVertices; iIndex++)
            {
                iVertex = pbsOldData->m_rgdwIndices[iIndex];

                pbsCurData->m_rgdwIndices[pbsCurData->m_iCurIndex] = iVertex;
                pbsCurData->m_rgfWeights[pbsCurData->m_iCurIndex] = pbsOldData->m_rgfWeights[iIndex];
                pbsCurData->m_iCurIndex += 1;

                iNextWedge = pMeshData->m_rgVertices[iVertex].iWedgeList;
                while (iVertex != iNextWedge)
                {
                    pbsCurData->m_rgdwIndices[pbsCurData->m_iCurIndex] = iNextWedge;
                    pbsCurData->m_rgfWeights[pbsCurData->m_iCurIndex] = pbsOldData->m_rgfWeights[iIndex];
                    pbsCurData->m_iCurIndex += 1;

                    iNextWedge = pMeshData->m_rgVertices[iNextWedge].iWedgeList;
                }
            }
        }

		// now that we do have skin data to put in the file, add a skinning header record
		cbHeaderData = sizeof(WORD) * 3;
		pbCur = pbHeaderData = new BYTE[cbHeaderData];
		if (pbHeaderData == NULL)
		{
			hr = E_OUTOFMEMORY;
			goto e_Exit;
		}

		WRITE_WORD(pbCur, psc->m_cMaxWeightsPerVertex);
		WRITE_WORD(pbCur, psc->m_cMaxWeightsPerFace);
		WRITE_WORD(pbCur, psmSkinMap->m_cbiBones);

        hr = psc->m_pxofsave->CreateDataObject(DXFILEOBJ_XSkinMeshHeader,
                                        NULL,
                                        NULL,
                                        cbHeaderData,
                                        pbHeaderData,
                                        &pDataObject
                                        );
        hr = pParent->AddDataObject(pDataObject);
        if (FAILED(hr))
            goto e_Exit;

        RELEASE(pDataObject);

// now actually output the prepared buffers
        for (iBone = 0; iBone < psmSkinMap->m_cbiBones; iBone++)
        {
            assert(rgbsBoneData[iBone].m_iCurIndex == rgbsBoneData[iBone].m_cVertices);
            assert(rgbsBoneDataDups[iBone].m_iCurIndex == rgbsBoneDataDups[iBone].m_cVertices);

            hr = psc->m_pxofsave->CreateDataObject(DXFILEOBJ_SkinWeights,
                                            NULL,
                                            NULL,
                                            rgbsBoneDataDups[iBone].m_cbData,
                                            rgbsBoneDataDups[iBone].m_pbData,
                                            &pDataObject
                                            );
            if (FAILED(hr))
                goto e_Exit;

            hr = pParent->AddDataObject(pDataObject);
            if (FAILED(hr))
                goto e_Exit;

            RELEASE(pDataObject);
        }
    }

e_Exit:
    if (pPhyExport != NULL)
 	{
		pPhyExport->SetInitialPose(FALSE);
	}


	delete []pbHeaderData;
    delete []rgbsBoneData;
    delete []rgbsBoneDataDups;
    return hr;
}

HRESULT AddMesh
    (
    SSaveContext *psc,
    INode *pNode, 
    Object* pObj,
    LPDIRECTXFILEDATA pParent
    )
{
    HRESULT hr = S_OK;
    BOOL bDeleteTriObject = false;
    TriObject *pTriObject = NULL;
    Mesh *pMesh;
    BOOL bSwapTriOrder;
    PBYTE pbData = NULL;
    PBYTE pbCur;
    DWORD cbSize;
    DWORD cVertices;
    DWORD cFaces;
    DWORD iFace;
    Matrix3 matNodeTM;
    SMeshData MeshData;
    LPDIRECTXFILEDATA pDataObject = NULL;
    DWORD iVertex;
    DWORD *rgdwMeshMaterials = NULL;
    SCropInfo *rgCropInfo = NULL;
    CUSTOMD3DXMATERIAL *rgMaterials = NULL;
    DWORD cMaterials;
	Modifier* pPhyMod= NULL;
	IPhysiqueExport* pPhyExport= NULL;

    // Retrieve the TriObject from the node

    pPhyMod = FindPhysiqueModifier(pNode);
    if (pPhyMod != NULL)
    {
        // Get a Physique Export interface
        pPhyExport = (IPhysiqueExport *)pPhyMod->GetInterface(I_PHYINTERFACE);
		
		if(pPhyExport != NULL)
		{
			pPhyExport->SetInitialPose(TRUE);
		}
	}

	//###danhoro $NOTE: meshes were not properly evaluated
	//mesh must be properly evaluated
	const ObjectState &pWorldObjState = pNode->EvalWorldState(0);
    pTriObject = GetTriObjectFromObjRef(pWorldObjState.obj, &bDeleteTriObject);

    // If no TriObject then skip
    if (pTriObject == NULL) 
        goto e_Exit;

    pMesh = &(pTriObject->mesh);

	// TIM: Code for light splitting
	Mesh* mesh1 = pMesh;
	pMesh = SplitMeshForLights(psc, pNode, pMesh);

	bool bDeleteMesh = false;
	if(mesh1 != pMesh)
		bDeleteMesh = true;

	//###danhoro $NOTE: added to fix dirty mesh
	//make sure the mesh set is clean
    pMesh->RemoveIllegalFaces();
    pMesh->RemoveDegenerateFaces();
    pMesh->DeleteIsoVerts();
    pMesh->DeleteIsoMapVerts();
    pMesh->checkNormals(TRUE); //just in case

    matNodeTM = pNode->GetObjTMAfterWSM(0);

	//###danhoro $NOTE parity is inverted because we are switching from RHS to LHS
    bSwapTriOrder = !matNodeTM.Parity();

    hr = GatherMeshMaterials(psc, pNode, pMesh, &rgdwMeshMaterials, &rgMaterials, &rgCropInfo, &cMaterials);
    if (FAILED(hr))
        goto e_Exit;

    hr = GenerateMeshData(pMesh, pNode, &MeshData, rgdwMeshMaterials);
    if (FAILED(hr))
        goto e_Exit;

    cVertices = MeshData.m_cVertices;
    cFaces = MeshData.m_cFaces;
    cbSize = sizeof(DWORD) // nVertices
             + cVertices*sizeof(float)*3 // vertices
             + sizeof(DWORD) // nFaces
             + cFaces*(sizeof(DWORD) /*nFaceVertexIndices*/ 
                            + sizeof(DWORD)*3 /*faceVertexIndices*/); // faces

    pbCur = pbData = new BYTE[cbSize];
    if (pbData == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }

    // write nVertices
    WRITE_DWORD(pbCur, cVertices);

    // write vertices
    for (iVertex = 0; iVertex < MeshData.m_cVertices; iVertex++)
    {
        WRITE_POINT3_SCALED(pbCur, pMesh->verts[MeshData.m_rgVertices[iVertex].iPointRep]);
    }

    // write nFaces
    WRITE_DWORD(pbCur, cFaces);
    
    // write faces
    for( iFace = 0; iFace < cFaces; iFace++ )
    {
        WRITE_DWORD(pbCur, 3); //nFaceVertexIndices

        // face vertex indices
        if( bSwapTriOrder )
        {
            WRITE_DWORD(pbCur, MeshData.m_rgFaces[iFace].index[0]);
            WRITE_DWORD(pbCur, MeshData.m_rgFaces[iFace].index[2]);
            WRITE_DWORD(pbCur, MeshData.m_rgFaces[iFace].index[1]);
        }
        else
        {
            WRITE_DWORD(pbCur, MeshData.m_rgFaces[iFace].index[0]);
            WRITE_DWORD(pbCur, MeshData.m_rgFaces[iFace].index[1]);
            WRITE_DWORD(pbCur, MeshData.m_rgFaces[iFace].index[2]);
        }
    }

    hr = psc->m_pxofsave->CreateDataObject(TID_D3DRMMesh,
                                    NULL,
                                    NULL,
                                    cbSize,
                                    pbData,
                                    &pDataObject
                                    );
    if (FAILED(hr))
        goto e_Exit;

    // add the normals to the file
    hr = AddNormals(psc, &MeshData, bSwapTriOrder, pDataObject);
    if (FAILED(hr))
        goto e_Exit;

    // write texture coordinates
    hr = AddTextureCoordinates(psc, &MeshData, rgCropInfo, pMesh, pDataObject);
    if (FAILED(hr))
        goto e_Exit;

    hr = AddVertexDuplicationIndices(psc, &MeshData, pDataObject);
    if (FAILED(hr))
        goto e_Exit;

    hr = AddMeshMaterials(psc, pNode, pMesh, rgdwMeshMaterials, rgMaterials, cMaterials, pDataObject);
    if (FAILED(hr))
        goto e_Exit;

    hr = AddSkinData(psc, pNode, &MeshData, pDataObject);
    if (FAILED(hr))
        goto e_Exit;

    hr = pParent->AddDataObject(pDataObject);
    if (FAILED(hr))
        goto e_Exit;

	// TIMJOHNSON: Add our custom materials after the mesh container is closed
	hr = AddCustomMaterials(psc,rgMaterials,cMaterials,pParent);
	if (FAILED(hr))
		goto e_Exit;

e_Exit:
	if(pPhyExport != NULL)
	{
		pPhyExport->SetInitialPose(FALSE);
	}

    if (bDeleteTriObject)
    {
        delete pTriObject;
    }

	if(bDeleteMesh)
		SAFE_DELETE(pMesh);
    RELEASE(pDataObject);
	SAFE_DELETE_ARRAY(rgdwMeshMaterials);
	SAFE_DELETE_ARRAY(rgMaterials);
	SAFE_DELETE_ARRAY(rgCropInfo);

    return hr;
}


HRESULT AddTransform
    (
    SSaveContext *psc,
    INode *pNode, 
    LPDIRECTXFILEDATA pParent
    )
{
    HRESULT hr = S_OK;
    PBYTE pbData = NULL;
    PBYTE pbCur;
    DWORD cbSize;
    Matrix3 matNodeTM;
    Matrix3 matRelativeTM;
    LPDIRECTXFILEDATA pDataObject = NULL;

    cbSize = 16*sizeof(float);

    pbCur = pbData = new BYTE[cbSize];
    if (pbData == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }


    // Node transform
	matNodeTM = pNode->GetNodeTM(0);

    if( pNode->IsRootNode() )
    {
        // scene root
        matRelativeTM = matNodeTM;
    }
    else
    {
		INode *pParentNode= pNode->GetParentNode();
		assert(pParentNode);
		pParentNode->EvalWorldState(0);
		Matrix3 matParentTM= pParentNode->GetNodeTM(0);

        if( matNodeTM == matParentTM )
        {
            matRelativeTM.IdentityMatrix();
        }
        else
        {
            matRelativeTM = matNodeTM * Inverse(matParentTM);
        }
    }

    WRITE_MATRIX4_FROM_MATRIX3(pbCur, matRelativeTM);

    hr = psc->m_pxofsave->CreateDataObject(TID_D3DRMFrameTransformMatrix,
                                    NULL,
                                    NULL,
                                    cbSize,
                                    pbData,
                                    &pDataObject
                                    );
    if (FAILED(hr))
        goto e_Exit;

    hr = pParent->AddDataObject(pDataObject);
    if (FAILED(hr))
        goto e_Exit;

e_Exit:
    delete []pbData;
    RELEASE(pDataObject);

    return hr;
}
/*
HRESULT AddPatchInverseTransform
    (
    SSaveContext *psc,
	INode *pNode,
    LPDIRECTXFILEDATA pParent,
    LPDIRECTXFILEDATA *ppMeshParent
    )
{
    HRESULT hr = S_OK;
    PBYTE pbData = NULL;
    PBYTE pbCur;
    DWORD cbSize;
    LPDIRECTXFILEDATA pDataObject = NULL;
	Matrix3 matNodeTM;
	Matrix3 matObjTMAfterWSM;
	Matrix3 matObjectOffset;
    LPDIRECTXFILEDATA pObjectOffset = NULL;


	matNodeTM = pNode->GetObjTMAfterWSM(0);
	if(matNodeTM.Parity())
	{
		hr = psc->m_pxofsave->CreateDataObject(TID_D3DRMFrame,
										NULL,
										NULL,
										0,
										NULL,
										&pObjectOffset
										);

		matObjectOffset.IdentityMatrix();
		matObjectOffset.Scale(Point3(-1.0f,-1.0f,-1.0f), TRUE);

		cbSize = 16*sizeof(float);

		pbCur = pbData = new BYTE[cbSize];
		if (pbData == NULL)
		{
			hr = E_OUTOFMEMORY;
			goto e_Exit;
		}

		WRITE_MATRIX4_FROM_MATRIX3(pbCur, matObjectOffset);

		hr = psc->m_pxofsave->CreateDataObject(TID_D3DRMFrameTransformMatrix,
										NULL,
										NULL,
										cbSize,
										pbData,
										&pDataObject
										);
		if (FAILED(hr))
			goto e_Exit;

		hr = pObjectOffset->AddDataObject(pDataObject);
		if (FAILED(hr))
			goto e_Exit;

		hr = pParent->AddDataObject(pObjectOffset);
		if (FAILED(hr))
			goto e_Exit;


		*ppMeshParent = pObjectOffset;
	}
	else  // identity object offset, mesh should use node as parent
	{
		*ppMeshParent = pParent;
	}


e_Exit:
    delete []pbData;
    RELEASE(pDataObject);
	RELEASE(pObjectOffset);

    return hr;
}*/


HRESULT AddObjectOffsetTransform
    (
    SSaveContext *psc,
	INode *pNode,
    LPDIRECTXFILEDATA pParent,
    LPDIRECTXFILEDATA *ppMeshParent
    )
{
    HRESULT hr = S_OK;
    PBYTE pbData = NULL;
    PBYTE pbCur;
    DWORD cbSize;
    LPDIRECTXFILEDATA pDataObject = NULL;
	Matrix3 matNodeTM;
	Matrix3 matObjTMAfterWSM;
	Matrix3 matObjectOffset;
    LPDIRECTXFILEDATA pObjectOffset = NULL;

	// check to see if the node has an object offset matrix
	matNodeTM = pNode->GetNodeTM(0);
	matObjTMAfterWSM = pNode->GetObjTMAfterWSM(0);

	if( !(matObjTMAfterWSM == matNodeTM) )
	{
		// the mesh is positioned offset from the node, so add another
		//   frame (unnamed) to offset the mesh without affecting the node's children
		//   and/or animation attached to the node
		hr = psc->m_pxofsave->CreateDataObject(TID_D3DRMFrame,
										NULL,
										NULL,
										0,
										NULL,
										&pObjectOffset
										);

		matObjectOffset = matObjTMAfterWSM * Inverse(matNodeTM);

		cbSize = 16*sizeof(float);

		pbCur = pbData = new BYTE[cbSize];
		if (pbData == NULL)
		{
			hr = E_OUTOFMEMORY;
			goto e_Exit;
		}

		WRITE_MATRIX4_FROM_MATRIX3(pbCur, matObjectOffset);

		hr = psc->m_pxofsave->CreateDataObject(TID_D3DRMFrameTransformMatrix,
										NULL,
										NULL,
										cbSize,
										pbData,
										&pDataObject
										);
		if (FAILED(hr))
			goto e_Exit;

		hr = pObjectOffset->AddDataObject(pDataObject);
		if (FAILED(hr))
			goto e_Exit;

		hr = pParent->AddDataObject(pObjectOffset);
		if (FAILED(hr))
			goto e_Exit;


		*ppMeshParent = pObjectOffset;
	}
	else  // identity object offset, mesh should use node as parent
	{
		*ppMeshParent = pParent;
	}


e_Exit:
    delete []pbData;
    RELEASE(pDataObject);
	RELEASE(pObjectOffset);

    return hr;
}

// TODO: Only save children containing selected.
HRESULT EnumTree
    (SSaveContext *psc,INode *pNode, LPDIRECTXFILEDATA *ppDataObjectNew, bool& containedSelected)
{
    HRESULT hr = S_OK;
    DWORD cChildren;
    DWORD iChild;
    LPDIRECTXFILEDATA pChildDataObject;
    LPDIRECTXFILEDATA pDataObject = NULL;
    LPDIRECTXFILEDATA pMeshParent = NULL;
    Object* pObj;
    char *szName = NULL;
    
    szName = psc->m_stStrings.CreateNiceString(pNode->GetName());
    if (szName == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }

    OutputDebugString(szName);
    OutputDebugString("\n");

    // add the node to the array for anim purposes
    assert( psc->m_cNodesCur < psc->m_cNodes );
    psc->m_rgpnNodes[psc->m_cNodesCur] = pNode;
    psc->m_cNodesCur += 1;

    hr = psc->m_pxofsave->CreateDataObject(TID_D3DRMFrame,
                                    szName,
                                    NULL,
                                    0,
                                    NULL,
                                    &pDataObject
                                    );

    hr = AddTransform(psc, pNode, pDataObject);
    if (FAILED(hr))
        goto e_Exit;

	// Tim: Don't write data when saving animations
	if(!psc->m_bSaveAnimationData && !(psc->m_bSaveSelection && !pNode->Selected())){
		containedSelected = true;

		if (psc->m_bSavePatchData && IsExportablePatchMesh(pNode, &pObj))
		{
			MessageBox(0,"Patch meshes not natively supported. TODO: Ask Tim to add a mesh collapse here",0,0);
		}
		else if (IsExportableMesh(psc->m_bSavePrefabData,pNode, &pObj))
		{
			hr = AddObjectOffsetTransform(psc, pNode, pDataObject, &pMeshParent);
			if (FAILED(hr))
				goto e_Exit;
			
			hr = AddMesh(psc, pNode, pObj, pMeshParent);
			if (FAILED(hr))
				goto e_Exit;
		}
		else if(IsExportableLight(pNode)){
			hr = AddObjectOffsetTransform(psc, pNode, pDataObject, &pMeshParent);
			if (FAILED(hr))
				goto e_Exit;

			AddLight(psc, pNode, pMeshParent);
		}

		// All nodes even lights or group heads can have custom properties
		if(HasProperties(pNode) && !pNode->IsNodeHidden()){
			// Create offset transform if it hasn't been added (i.e. it's not a mesh/light)
			if(!pMeshParent){
				hr = AddObjectOffsetTransform(psc, pNode, pDataObject, &pMeshParent);
				if (FAILED(hr))
					goto e_Exit;
			}

			hr = AddCustomProperties(psc, pNode, pMeshParent);
			if(FAILED(hr))
				goto e_Exit;
		}
	}

    cChildren = pNode->NumberOfChildren();

	// Some entity children should be ignored, as we sometimes only need to save
	// the entity info, not its attached model
	if(!IncludeEntityModel(psc->m_bSavePrefabData,pNode))
		cChildren = 0;

    for (iChild = 0; iChild < cChildren; iChild++)
    {
        // enumerate the child

		// Set this flag so we can filter redundant frames when we have selections
		// Animations and skinned objects often need frames that aren't selected, so still must export everything
		// TODO: Only save relevant animation data. How can we tell what this is? i.e. how do we know
		// which bones are used
		bool containedSelected = psc->m_bSaveAnimationData || (psc->m_cMaxWeightsPerVertex > 0);
        hr = EnumTree(psc, pNode->GetChildNode(iChild), &pChildDataObject, containedSelected);
        if (FAILED(hr))
            goto e_Exit;
        
		if(containedSelected){
			hr = pDataObject->AddDataObject(pChildDataObject);
			if (FAILED(hr))
				goto e_Exit;
		}

        RELEASE(pChildDataObject);
    }

    *ppDataObjectNew = pDataObject;
e_Exit:
    return hr;
}


HRESULT Preprocess
    (
    SPreprocessContext *ppc,
    INode *pNode
    )
{
    HRESULT hr = S_OK;
    DWORD cChildren;
    DWORD iChild;
    Object* pObj;
    SSkinMap **rgpsmTemp = NULL;
    IPhyVertexExport *pVertexExport = NULL;
    IPhyRigidVertex* pRigidVertexExport = NULL;
    IPhyBlendedRigidVertex *pBlendedRigidVertexExport = NULL;
    INode* pBoneNode;
    SSkinMap *psmSkinMap;
    Modifier* pPhyMod = NULL;
    IPhysiqueExport* pPhyExport = NULL;
    IPhyContextExport* pPhyContextExport = NULL;
    DWORD iVertex;
    DWORD cVertices;
    DWORD iVertexType;
    SBoneInfo *pbi;
    DWORD cTotalBones;
    DWORD iBone;
    DWORD cpnBonesSeen;
    DWORD cpnBonesSeenMax;
    INode **rgpnBonesSeen = NULL;
    INode **rgpnTemp;
    BOOL bBoneSeen;
    DWORD iBoneSeen;
    BOOL bDeleteTriObject = false;
    TriObject *pTriObject = NULL;
    Mesh *pMesh;
	DWORD iFace;
	DWORD iPoint;

    ppc->m_cNodes += 1;

    if (IsExportableMesh(ppc->m_bSavePrefabData,pNode, &pObj) && !(ppc->m_bSaveSelection && !pNode->Selected()))
    {
        // first see if physique is present
        pPhyMod = FindPhysiqueModifier(pNode);
        if (pPhyMod != NULL)
        {
            // Get a Physique Export interface
            pPhyExport = (IPhysiqueExport *)pPhyMod->GetInterface(I_PHYINTERFACE);
            if (pPhyExport == NULL)
            {   // not all interfaces present, so ignore
                goto e_NoPhysique;
            }
            // For a given Object's INode get a
            // ModContext Interface from the Physique Export Interface:
            pPhyContextExport = (IPhyContextExport *)pPhyExport->GetContextInterface(pNode);
            if(pPhyContextExport == NULL)
            {
                // not all interfaces present, so ignore
                goto e_NoPhysique;
            }

            // convert to rigid with blending
            pPhyContextExport->AllowBlending(TRUE);
            pPhyContextExport->ConvertToRigid(TRUE);

            psmSkinMap = new SSkinMap;
            if (psmSkinMap == NULL)
            {
                hr = E_OUTOFMEMORY;
                goto e_Exit;
            }

            // realloc if necessary
            if (ppc->m_cpsmSkinMaps == ppc->m_cpsmSkinMapsMax)
            {
                rgpsmTemp = ppc->m_rgpsmSkinMaps;

                ppc->m_cpsmSkinMapsMax = max(1, ppc->m_cpsmSkinMapsMax);
                ppc->m_cpsmSkinMapsMax = ppc->m_cpsmSkinMapsMax * 2;
                ppc->m_rgpsmSkinMaps = new SSkinMap*[ppc->m_cpsmSkinMapsMax];
                if (ppc->m_rgpsmSkinMaps == NULL)
                {
                    ppc->m_rgpsmSkinMaps = rgpsmTemp;
                    hr = E_OUTOFMEMORY;
                    goto e_Exit;
                }

                if (ppc->m_cpsmSkinMaps > 0)
                {
                    memcpy(ppc->m_rgpsmSkinMaps, rgpsmTemp, sizeof(SSkinMap*) * ppc->m_cpsmSkinMaps);
                }

                delete []rgpsmTemp;
            }
            ppc->m_rgpsmSkinMaps[ppc->m_cpsmSkinMaps] = psmSkinMap;
            ppc->m_cpsmSkinMaps += 1;

            // init the map
            psmSkinMap->m_pMeshNode = pNode;
            psmSkinMap->m_cbiBonesMax = 30;
            psmSkinMap->m_rgbiBones = new SBoneInfo[psmSkinMap->m_cbiBonesMax];
            if (psmSkinMap->m_rgbiBones == NULL)
            {
                hr = E_OUTOFMEMORY;
                goto e_Exit;
            }

            // init duplication removal for redundant weights
            cpnBonesSeenMax = 30;
            cpnBonesSeen = 0;
            rgpnBonesSeen = new INode*[cpnBonesSeenMax];
            if (rgpnBonesSeen == NULL)
            {
                hr = E_OUTOFMEMORY;
                goto e_Exit;
            }

            cVertices = pPhyContextExport->GetNumberVertices();
            for (iVertex = 0; iVertex < cVertices; iVertex++ )
            {
                pVertexExport = (IPhyVertexExport *)pPhyContextExport->GetVertexInterface(iVertex);    
                if (pVertexExport == NULL)
                {
                    XSkinExp_OutputDebugString("Preprocess: Couldn't get physique vertex export interface!");
                    hr = E_FAIL;
                    goto e_Exit;
                }
				else
				{    
					// What kind of vertices are these?
					iVertexType = pVertexExport->GetVertexType();

					//pPhyContextExport->ReleaseVertexInterface( pVertexExport );    

					if( iVertexType == RIGID_TYPE )
					{
						pRigidVertexExport = (IPhyRigidVertex *)pVertexExport;//pPhyContextExport->GetVertexInterface(iVertex);

		            
						pBoneNode = pRigidVertexExport->GetNode();

						pbi = psmSkinMap->FindBone(pBoneNode);
						if (pbi == NULL)
						{
							hr = psmSkinMap->AddBone(pBoneNode, &pbi);
							if (FAILED(hr))
								goto e_Exit;
						}

						pbi->m_cVertices += 1;

						ppc->m_cMaxWeightsPerVertex = max(1, ppc->m_cMaxWeightsPerVertex);

					}
					else
					{
						assert( iVertexType == RIGID_BLENDED_TYPE );

						pBlendedRigidVertexExport = (IPhyBlendedRigidVertex *)pVertexExport;//pPhyContextExport->GetVertexInterface(iVertex);

						// How many nodes affect his vertex?
						cTotalBones = pBlendedRigidVertexExport->GetNumberNodes();
						cpnBonesSeen = 0;
						for (iBone = 0; iBone < cTotalBones; iBone++ )
						{
							pBoneNode = pBlendedRigidVertexExport->GetNode(iBone);

							// first see if the bone has already been seen
							bBoneSeen = FALSE;
							for (iBoneSeen = 0; iBoneSeen < cpnBonesSeen; iBoneSeen++)
							{
								if (rgpnBonesSeen[iBoneSeen] == pBoneNode)
								{
									bBoneSeen = TRUE;
									break;
								}
							}
		                    
							// if not seen, collect stats and add to already seen
							if (!bBoneSeen)
							{
								pbi = psmSkinMap->FindBone(pBoneNode);
								if (pbi == NULL)
								{
									hr = psmSkinMap->AddBone(pBoneNode, &pbi);
									if (FAILED(hr))
										goto e_Exit;
								}
								pbi->m_cVertices += 1;

								if (cpnBonesSeen >= cpnBonesSeenMax)
								{
									rgpnTemp = rgpnBonesSeen;
									cpnBonesSeenMax *= 2;

									rgpnBonesSeen = new INode*[cpnBonesSeenMax];
									if (rgpnBonesSeen == NULL)
									{
										rgpnBonesSeen = rgpnTemp;
										hr = E_OUTOFMEMORY;
										goto e_Exit;
									}

									memcpy(rgpnBonesSeen, rgpnTemp, cpnBonesSeen * sizeof(INode*));
									delete []rgpnTemp;
								}

								rgpnBonesSeen[cpnBonesSeen] = pBoneNode;
								cpnBonesSeen += 1;
							}
						}

						// actualNum... accounts for duplicated weights to same transform node
						// that are combined automatically above
						ppc->m_cMaxWeightsPerVertex = max(cpnBonesSeen, ppc->m_cMaxWeightsPerVertex);
		        
					}

					pPhyContextExport->ReleaseVertexInterface( pVertexExport);


				}
            }


		// now figure out the max number of weights per face

			pTriObject = GetTriObjectFromObjRef(pObj, &bDeleteTriObject);
            if (pTriObject == NULL) 
            {
                XSkinExp_OutputDebugString("Preprocess: Physique info, but no mesh");
                hr = E_FAIL;
                goto e_Exit;
            }

            pMesh = &(pTriObject->mesh);

            for (iFace = 0; iFace < pMesh->numFaces; iFace++)
            {
				cpnBonesSeen = 0;
				for (iPoint = 0; iPoint < 3; iPoint++ )
				{
					iVertex = pMesh->faces[iFace].v[iPoint];

					pVertexExport = (IPhyVertexExport *)pPhyContextExport->GetVertexInterface(iVertex);    
					if (pVertexExport == NULL)
					{
						XSkinExp_OutputDebugString("Preprocess: Couldn't get physique vertex export interface!");
						hr = E_FAIL;
						goto e_Exit;
					}
            
					// What kind of vertices are these?
					iVertexType = pVertexExport->GetVertexType();

  

					if( iVertexType == RIGID_TYPE )
					{
						pRigidVertexExport = (IPhyRigidVertex *)pVertexExport;//pPhyContextExport->GetVertexInterface(iVertex);

						// Get the vertex info!
                
						pBoneNode = pRigidVertexExport->GetNode();

						pbi = psmSkinMap->FindBone(pBoneNode);
						if (pbi == NULL)
						{
							hr = psmSkinMap->AddBone(pBoneNode, &pbi);
							if (FAILED(hr))
								goto e_Exit;
						}

						// first see if the bone has already been seen
						bBoneSeen = FALSE;
						for (iBoneSeen = 0; iBoneSeen < cpnBonesSeen; iBoneSeen++)
						{
							if (rgpnBonesSeen[iBoneSeen] == pBoneNode)
							{
								bBoneSeen = TRUE;
								break;
							}
						}
                    
						// if not seen, collect stats and add to already seen
						if (!bBoneSeen)
						{
							if (cpnBonesSeen >= cpnBonesSeenMax)
							{
								rgpnTemp = rgpnBonesSeen;
								cpnBonesSeenMax *= 2;

								rgpnBonesSeen = new INode*[cpnBonesSeenMax];
								if (rgpnBonesSeen == NULL)
								{
									rgpnBonesSeen = rgpnTemp;
									hr = E_OUTOFMEMORY;
									goto e_Exit;
								}

								memcpy(rgpnBonesSeen, rgpnTemp, cpnBonesSeen * sizeof(INode*));
								delete []rgpnTemp;
							}

							rgpnBonesSeen[cpnBonesSeen] = pBoneNode;
							cpnBonesSeen += 1;
						}

					}
					else
					{
						assert( iVertexType == RIGID_BLENDED_TYPE );

						pBlendedRigidVertexExport = (IPhyBlendedRigidVertex *)pVertexExport;//pPhyContextExport->GetVertexInterface(iVertex);


						// How many nodes affect his vertex?
						cTotalBones = pBlendedRigidVertexExport->GetNumberNodes();
						for (iBone = 0; iBone < cTotalBones; iBone++ )
						{
							pBoneNode = pBlendedRigidVertexExport->GetNode(iBone);

							// first see if the bone has already been seen
							bBoneSeen = FALSE;
							for (iBoneSeen = 0; iBoneSeen < cpnBonesSeen; iBoneSeen++)
							{
								if (rgpnBonesSeen[iBoneSeen] == pBoneNode)
								{
									bBoneSeen = TRUE;
									break;
								}
							}
                        
							// if not seen, collect stats and add to already seen
							if (!bBoneSeen)
							{
								if (cpnBonesSeen >= cpnBonesSeenMax)
								{
									rgpnTemp = rgpnBonesSeen;
									cpnBonesSeenMax *= 2;

									rgpnBonesSeen = new INode*[cpnBonesSeenMax];
									if (rgpnBonesSeen == NULL)
									{
										rgpnBonesSeen = rgpnTemp;
										hr = E_OUTOFMEMORY;
										goto e_Exit;
									}

									memcpy(rgpnBonesSeen, rgpnTemp, cpnBonesSeen * sizeof(INode*));
									delete []rgpnTemp;
								}

								rgpnBonesSeen[cpnBonesSeen] = pBoneNode;
								cpnBonesSeen += 1;
							}
						}
					}
					pPhyContextExport->ReleaseVertexInterface( pVertexExport );  
				}

				ppc->m_cMaxWeightsPerFace = max(cpnBonesSeen, ppc->m_cMaxWeightsPerFace);
            }
        }

e_NoPhysique:;
    }

    cChildren = pNode->NumberOfChildren();
    for (iChild = 0; iChild < cChildren; iChild++)
    {
        // enumerate the child
        hr = Preprocess(ppc, pNode->GetChildNode(iChild));
        if (FAILED(hr))
            goto e_Exit;        
    }

e_Exit:
    if (bDeleteTriObject)
    {
        delete pTriObject;
    }

    delete []rgpnBonesSeen;
    return hr;
}


struct SAnimInfo
{
    DWORD dwTime;
    DWORD dwNumValues;
    float rgfValues[16];
};

BOOL BFloatsEqual
    (
    float f1,
    float f2
    )
{
    // first do a bitwise compare
    if ((*(DWORD*)&f1) == (*(DWORD*)&f2))
        return TRUE;

    // next do an epsilon compare
    float fDiff = (f1 - f2);
    return (fDiff <= 1e-6f) && (fDiff >= -1e-6f);
}

BOOL BEqualMatrices(float *rgfMat1, float *rgfMat2)
{
    DWORD iFloat;

    for (iFloat = 0; iFloat < 16; iFloat++)
    {
        if (!BFloatsEqual(rgfMat1[iFloat], rgfMat2[iFloat]))
            return FALSE;
    }

    return TRUE;
}

HRESULT GenerateAnimationSet
    (
    SSaveContext *psc
    )
{
    HRESULT hr = S_OK;
    DWORD cKeys;
    PBYTE pbData = NULL;
    PBYTE pbCur;
    DWORD cTicksPerFrame;
    DWORD iCurTime;
    DWORD iCurKey;
    Matrix3 matFirstTM;
    Matrix3 matTM;
    Matrix3 matRelativeTM;
    Matrix3 matParentTM;
	Interval interval;
    DWORD iNode;
    INode *pNode;
//    INode *pParentNode;
    DWORD cbSize;
    DWORD iKey;
    DWORD cCurrentKeys;
    SAnimInfo *rgaiAnimData = NULL;
    SAnimInfo *rgaiAnimDataCur;
    LPDIRECTXFILEDATA pAnimation = NULL;
    LPDIRECTXFILEDATA pAnimationKeys = NULL;
    char *szName;
    BOOL bAddEndKey = FALSE;
    DWORD iInterval;
    DWORD iStartTime;
    DWORD iEndTime;
    DWORD cFrameRate;

    // find the animation info from the interface
    interval = psc->m_pInterface->GetAnimRange();
    cTicksPerFrame = GetTicksPerFrame();
    cFrameRate = GetFrameRate();
    iStartTime = interval.Start();
    iEndTime = interval.End();

    iInterval = (cTicksPerFrame * cFrameRate) / psc->m_iAnimSamplingRate;

    cKeys = (iEndTime - iStartTime) / iInterval;

    // add a key for the last frame at iEndTime, or the one added if iEndTime is not directly touched
    cKeys += 1;

    // add a key to give the final frame a length (difference between Max and X file keyframe specification)
    cKeys += 1;

    // if the sampling frequency doesn't end directly on 
    //   on the end time, then add a partial end key
    if (((iEndTime - iStartTime) % iInterval) != 0)
    {
        bAddEndKey = TRUE;
    }

    rgaiAnimData = new SAnimInfo[psc->m_cNodes*cKeys];
    if (rgaiAnimData == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }

	for (iCurKey = 0, iCurTime = iStartTime; iCurTime <= iEndTime; iCurTime += iInterval, iCurKey++ )
	{
		for (iNode = 0; iNode < psc->m_cNodes; iNode++)
		{
			pNode = psc->m_rgpnNodes[iNode];

		    //iCurTime = iFrame * (cTicksPerFrame / 2);
			matTM = pNode->GetNodeTM(iCurTime);


            if (pNode->GetParentNode() == NULL)
            {
                matRelativeTM = matTM;

            }
            else
            {
				pNode->GetParentNode()->EvalWorldState(iCurTime);
				matParentTM = pNode->GetParentNode()->GetNodeTM(iCurTime);

                if( matTM == matParentTM )
                {
                    matRelativeTM.IdentityMatrix();
                }
                else
                {
                    matRelativeTM = matTM * Inverse(matParentTM);
                }
            }

            // set the current pointer to the correct buffer position
            pbCur = (PBYTE)&rgaiAnimData[iNode*cKeys + iCurKey];

            // first write the time and dword count            
            WRITE_DWORD(pbCur, iCurTime);
            WRITE_DWORD(pbCur, 16);

            // next write the matrix
            WRITE_MATRIX4_FROM_MATRIX3(pbCur, matRelativeTM);
        }
    }

    // if the sampling rate doesn't evenly end on the last frame, add a partial key frame
    if (bAddEndKey)
    {
        assert(((iEndTime - iStartTime) % iInterval) != 0);

        // just add the end time as a key frame
        for (iNode = 0; iNode < psc->m_cNodes; iNode++)
        {
            // set the current pointer to the correct buffer position
            pbCur = (PBYTE)&rgaiAnimData[iNode*cKeys +iCurKey];

			matTM = pNode->GetNodeTM(iEndTime);

            if (pNode->GetParentNode() == NULL)
            {
                matRelativeTM = matTM;
            }
            else
            {
				matParentTM = pNode->GetParentNode()->GetNodeTM(iEndTime);
                if( matTM == matParentTM )
                {
                    matRelativeTM.IdentityMatrix();
                }
                else
                {
                    matRelativeTM = matTM * Inverse(matParentTM);
                }
            }

            WRITE_DWORD(pbCur, iEndTime);
            WRITE_DWORD(pbCur, 16);

            // next write the matrix
            WRITE_MATRIX4_FROM_MATRIX3(pbCur, matRelativeTM);
        }

        // update iCurKey for the looping key frames
        iCurKey += 1;
    }


    
    // add an final animation frame to allow for looping - Max assumes that the last frame lasts for an interval...  X-files do not, so specify it explicitly
    //    if looping is not desired, then this should be the final frames data instead of the first frames data
    for (iNode = 0; iNode < psc->m_cNodes; iNode++)
    {
            // set the current pointer to the correct buffer position
            pbCur = (PBYTE)&rgaiAnimData[iNode*cKeys + iCurKey];
        if (psc->m_bLoopingAnimationData)
        {
            // copy the zero'th animations data, just update the time value
            memcpy(pbCur, (PBYTE)&rgaiAnimData[iNode*cKeys + 0], sizeof(SAnimInfo));
        }
        else
        {
            // copy the previous frames data to hold for a frames worth of time
            memcpy(pbCur, (PBYTE)&rgaiAnimData[iNode*cKeys + (iCurKey-1)], sizeof(SAnimInfo));
        }

        WRITE_DWORD(pbCur, iEndTime + cTicksPerFrame);
    }

    assert(iCurKey + 1 == cKeys);    

    // allocate memory to send to D3DXOF lib
    cbSize = sizeof(DWORD) + sizeof(DWORD) +
            (sizeof(DWORD) //time
                + sizeof(DWORD) //nValues
                + sizeof(FLOAT)*16) // x, y, z
               * cKeys; // number of keys

    pbData = new BYTE[cbSize];
    if (pbData == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }

    hr = psc->m_pxofsave->CreateDataObject(TID_D3DRMAnimationSet,
                                    NULL,
                                    NULL,
                                    0,
                                    NULL,
                                    &psc->m_pAnimationSet
                                    );
    if (FAILED(hr))
        goto e_Exit;

	for (iNode = 0; iNode < psc->m_cNodes; iNode++)
	{
		pbCur = pbData;

        szName = psc->m_stStrings.CreateNiceString(psc->m_rgpnNodes[iNode]->GetName());
        if (szName == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto e_Exit;
        }

		// write the key type
		WRITE_DWORD(pbCur, 4);

		// write the number of keys
		WRITE_DWORD(pbCur, cKeys);

        rgaiAnimDataCur = &rgaiAnimData[iNode*cKeys];
        cCurrentKeys = 0;
        for (iKey = 0; iKey < cKeys; iKey++)
        {
		    memcpy(pbCur, &rgaiAnimDataCur[iKey], sizeof(SAnimInfo));
            pbCur += sizeof(SAnimInfo);
            cCurrentKeys += 1;

            // if not last key, then check for start of a run of identical keys
            if (iKey < (cKeys-1))
            {
                // if equal to next, check for a run of equal matrices
                if (BEqualMatrices(rgaiAnimDataCur[iKey].rgfValues, rgaiAnimDataCur[iKey+1].rgfValues))
                {
                    // move to the next key, and skip all equal matrices
                    iKey += 1;
                    while ((iKey < (cKeys-1)) && BEqualMatrices(rgaiAnimDataCur[iKey].rgfValues, rgaiAnimDataCur[iKey+1].rgfValues))
                    {
                        iKey += 1;
                    }

		            memcpy(pbCur, &rgaiAnimDataCur[iKey], sizeof(SAnimInfo));
                    pbCur += sizeof(SAnimInfo);
                    cCurrentKeys += 1;
                }
            }
        }

        // update to current count of keys
        ((DWORD*)pbData)[1] = cCurrentKeys;

		hr = psc->m_pxofsave->CreateDataObject(TID_D3DRMAnimation,
										NULL,
										NULL,
										0,
										NULL,
										&pAnimation
										);

	// add the data to the file
		hr = psc->m_pxofsave->CreateDataObject(TID_D3DRMAnimationKey,
										NULL,
										NULL,
										cbSize,
										pbData,
										&pAnimationKeys
										);
		if (FAILED(hr))
			goto e_Exit;

		// add to the animation set
		hr = pAnimation->AddDataObject(pAnimationKeys);
		if (FAILED(hr))
			goto e_Exit;

		hr = pAnimation->AddDataReference(szName, NULL);
		if (FAILED(hr))
			goto e_Exit;

		hr = psc->m_pAnimationSet->AddDataObject(pAnimation);
		if (FAILED(hr))
			goto e_Exit;

        RELEASE(pAnimation);
        RELEASE(pAnimationKeys);
	}

e_Exit:
    delete []rgaiAnimData;
	delete []pbData;
    RELEASE(pAnimation);
    RELEASE(pAnimationKeys);

    return hr;
}


// ================================================== CDataSave::CDataSave()
HRESULT ExportXFile
    (
    const TCHAR *szFilename,
    INode* pRootNode,
    Interface *pInterface, 
    BOOL bSuppressPrompts,
    BOOL bSaveSelection,
    HWND hwndParent,
	SDialogOptions DlgOptions
    ) 
{
    LPDIRECTXFILE pxofapi = NULL;
    LPDIRECTXFILEDATA pRootData = NULL;
    LPDIRECTXFILESAVEOBJECT pxofsave = NULL; 
    HRESULT hr = S_OK;
    SSaveContext sc;
    SPreprocessContext pc;

	// Get the scaling needed to put coordinates in meters
	gWorldScale = GetMasterScale(UNITS_METERS);


    assert(szFilename && pInterface);

    // Extract scene information
    pInterface->ProgressStart(_T("Extracting skinning data"),TRUE,dummyFn,NULL);
    pInterface->ProgressUpdate(0);
    //pInterface->ProgressUpdate(100);

    // first find the root node
    //hr = FindRootNode(gup, &pRootNode);
    //if (FAILED(hr))
    //    goto e_Exit;

	sc.rootNode = pRootNode;

    // setup options for Preprocess stage
    pc.m_bSaveSelection = bSaveSelection;
	pc.m_pInterface = pInterface;
	pc.m_bSavePrefabData = DlgOptions.m_bSavePrefabData;

    // figure out bone counts, etc.
    hr = Preprocess(&pc, pRootNode);
    if (FAILED(hr))
        goto e_Exit;

    // move the skin map data from the preprocess context to the save context
    sc.m_cpsmSkinMaps = pc.m_cpsmSkinMaps;
    sc.m_rgpsmSkinMaps = pc.m_rgpsmSkinMaps;
    pc.m_rgpsmSkinMaps = NULL;

    pInterface->ProgressUpdate(25);

    DlgOptions.m_cMaxBonesPerVertex = pc.m_cMaxWeightsPerVertex;
    DlgOptions.m_cMaxBonesPerFace = pc.m_cMaxWeightsPerFace;

    // if prompts not suppressed, then check with the user on options
    if (!bSuppressPrompts)
    {
     /*   DialogBoxParam(g_hInstance, 
                       MAKEINTRESOURCE(IDD_PANEL), 
                        hwndParent, 
                        XSkinExpOptionsDlgProc, 
                        (LPARAM)&DlgOptions);*/

//        if (!DlgOptions.m_bProceedWithExport)
//            goto e_Exit;
    }

    pInterface->ProgressStart(_T("Exporting data"),TRUE,dummyFn,NULL);
    pInterface->ProgressUpdate(25);

    // Create xofapi object.
    DXASSERT(DirectXFileCreate(&pxofapi));
    // Register templates for d3drm.
    DXASSERT(pxofapi->RegisterTemplates((LPVOID)D3DRM_XTEMPLATES,
                                    D3DRM_XTEMPLATE_BYTES));
    DXASSERT(pxofapi->RegisterTemplates((LPVOID)XSKINEXP_TEMPLATES,
                                    strlen(XSKINEXP_TEMPLATES)));
	// TIMJOHNSON: Custom templates
	DXASSERT(pxofapi->RegisterTemplates((LPVOID)XCUSTOM_TEMPLATES,
						strlen(XCUSTOM_TEMPLATES)));
    // Create save object.
    if(FAILED(hr=pxofapi->CreateSaveObject(szFilename,    // filename
                                   DlgOptions.m_xFormat,  // binary or text
								   &pxofsave)))
	{
		Error("Failed to save file (%s). Make sure path '%s' is valid and that the file is not in use",DXGetErrorString9(hr),szFilename);
		goto e_Exit;
	}


    DXASSERT(pxofsave->SaveTemplates(num_aIds, aIds));

	sc.m_bSavePrefabData = DlgOptions.m_bSavePrefabData;
    sc.m_pxofsave = pxofsave;
    sc.m_xFormat = DlgOptions.m_xFormat;
    sc.m_bSaveAnimationData = DlgOptions.m_bSaveAnimationData;
    sc.m_bLoopingAnimationData = DlgOptions.m_bLoopingAnimationData;
    sc.m_iAnimSamplingRate = DlgOptions.m_iAnimSamplingRate;
	sc.m_bSavePatchData = DlgOptions.m_bSavePatchData;
    sc.m_iTime = pInterface->GetTime();
    sc.m_pInterface = pInterface;
    sc.m_bSaveSelection = bSaveSelection;
    sc.m_cMaxWeightsPerVertex = pc.m_cMaxWeightsPerVertex;
    sc.m_cMaxWeightsPerFace = pc.m_cMaxWeightsPerFace;

    sc.m_cNodes = pc.m_cNodes;
    sc.m_cNodesCur = 0;
    sc.m_rgpnNodes = new INode*[sc.m_cNodes];
    if (sc.m_rgpnNodes == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }

	// We only save scene _OR_ animation, never both in same file
	if (DlgOptions.m_bSaveAnimationData){
		// Write the header
		ExportAnimationHeader(&sc, &pRootData);
		// now save that file data to the file
		DXASSERT(pxofsave->SaveData(pRootData));
		RELEASE(pRootData);
	}
	else{
		// Write the header
		ExportHeader(&sc, &DlgOptions.sceneData, &pRootData);
		// now save that file data to the file
		DXASSERT(pxofsave->SaveData(pRootData));
		RELEASE(pRootData);
	}

	gbNan_found= false;
	// then write the whole tree out into file data's
	bool temp = false;
	hr = EnumTree(&sc, pRootNode, &pRootData,temp);
	if (FAILED(hr))
		goto e_Exit;

	pInterface->ProgressStart(_T("Saving data to file"),TRUE,dummyFn,NULL);
	pInterface->ProgressUpdate(50);

	// now save that file data to the file
	try {
		DXASSERT(pxofsave->SaveData(pRootData));
	}
	catch(...){
		MessageBox(0,"SaveData(): Saving the file failed. Unknown reason",0,0);
		goto e_Exit;
	}

    pInterface->ProgressUpdate(75);

    if (DlgOptions.m_bSaveAnimationData)
    {
        pInterface->ProgressStart(_T("Saving animation data to file"),TRUE,dummyFn,NULL);
        pInterface->ProgressUpdate(75);

        hr = GenerateAnimationSet(&sc);
        if (FAILED(hr))
            goto e_Exit;

        hr = pxofsave->SaveData(sc.m_pAnimationSet);
        if (FAILED(hr))
        {
			XSkinExp_OutputDebugString("ExportXFile: Failed to add animation set to X File\n");
			goto e_Exit;
        }            
    }

e_Exit:
    if (FAILED(hr))
    {
        OutputDebugString("File was not successfully exported.");
        {
             MessageBox(hwndParent,errstr,_T("Error!"),MB_ICONERROR|MB_OK);
        }
    }

    // falling through
    // Free up outstanding interfaces
    RELEASE(pxofsave);
    RELEASE(pRootData);
    RELEASE(pxofapi);

    pInterface->ProgressUpdate(100);
    pInterface->ProgressEnd();

    return hr;
}

