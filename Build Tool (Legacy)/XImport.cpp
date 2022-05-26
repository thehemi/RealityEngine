//----------------------------------------------------------------------------------
// XImport.cpp - Reads modified X files and converts the data
// to C++ structures for use in Compiler.cpp
//
// TODO: Mixmap support
//----------------------------------------------------------------------------------
#include "stdafx.h"
#include <rmxfguid.h>

// Hacks, because templates shared
extern unsigned char D3DRM_XTEMPLATES[];
#define D3DRM_XTEMPLATE_BYTES 3278

#define MAP_COMPILER

unsigned long TotalFacesRead, TotalVerticesRead;
#define GETSTRING(x){ x = (CHAR*)pData; pData += strlen((CHAR*)pData)+1; }
#define GETDWORD(x){ x = *(DWORD*)pData; pData += sizeof(DWORD); }
#define GETOBJECT(x) memcpy((void*)&(x),(pData),sizeof(x)); (pData) += sizeof(x);
#define GETWORD(x){ x = *(WORD*)pData; pData += sizeof(WORD); }
#define GETCOLOR(x){ x = *(D3DXCOLOR*)pData; pData += sizeof(D3DXCOLOR); }
#define GETTIMESTAMP(x){ x = *(SYSTEMTIME*)pData; pData += sizeof(SYSTEMTIME); }
#define GETFLOAT(x){ x = *(float*)pData; pData += sizeof(float); }


//-----------------------------------------------------------------------------
// Name: DXUtil_ConvertAnsiStringToGeneric()
// Desc: This is a UNICODE conversion utility to convert a CHAR string into a
//       TCHAR string. 
//       cchDestChar is the size in TCHARs of tstrDestination.  Be careful not to 
//       pass in sizeof(strDest) on UNICODE builds
//-----------------------------------------------------------------------------
HRESULT DXUtil_ConvertAnsiStringToGenericCch( TCHAR* tstrDestination, const CHAR* strSource, 
											 int cchDestChar )
{
	if( tstrDestination==NULL || strSource==NULL || cchDestChar < 1 )
		return E_INVALIDARG;

#ifdef _UNICODE
	return DXUtil_ConvertAnsiStringToWideCch( tstrDestination, strSource, cchDestChar );
#else
	strncpy( tstrDestination, strSource, cchDestChar );
	tstrDestination[cchDestChar-1] = '\0';
	return S_OK;
#endif    
}

HRESULT DXUtil_ConvertAnsiStringToGenericCb( TCHAR* tstrDestination, const CHAR* strSource, int cbDestChar )
{
	return DXUtil_ConvertAnsiStringToGenericCch( tstrDestination, strSource, cbDestChar / sizeof(TCHAR) );
}


//-----------------------------------------------------------------------------
// Name: CalculateCombinedMatrices()
// Desc: Calculates the combined (world-space) matrices for all frames
// This is for static frames. Shouldn't use the data on animated meshes
//-----------------------------------------------------------------------------
void CalculateCombinedMatrices(ImportFrame* pFrame, Matrix* pParentMatrix)
{
	if(!pFrame)
		return;

	if (pParentMatrix != NULL)
		pFrame->CombinedTransformationMatrix = pFrame->TransformationMatrix * *pParentMatrix;
	else
		pFrame->CombinedTransformationMatrix = pFrame->TransformationMatrix;

	if (pFrame->m_pNext != NULL)
	{
		CalculateCombinedMatrices(pFrame->m_pNext, pParentMatrix);
	}

	if (pFrame->m_pChild != NULL)
	{
		CalculateCombinedMatrices(pFrame->m_pChild, &pFrame->CombinedTransformationMatrix);
	}
}

//-----------------------------------------------------------------------------
// Desc: Loads scene data
//-----------------------------------------------------------------------------
HRESULT LoadSceneProperties(LPD3DXFILEDATA pFileData, ImportFrame* pFrame ){
	DWORD cbSize;
	pFrame->sceneProperties = new SceneProperties;

	BYTE* pData;
	pFileData->Lock(&cbSize,(LPCVOID*)&pData);

	string skyworld;
	GETSTRING(skyworld);
	strcpy(pFrame->sceneProperties->skyworld,skyworld.c_str());

	GETCOLOR(pFrame->sceneProperties->fogColor);
	GETDWORD(pFrame->sceneProperties->fogStart);
	GETDWORD(pFrame->sceneProperties->clipPlane);
	string minimap;
	GETSTRING(minimap);
	strcpy(pFrame->sceneProperties->miniMap,skyworld.c_str());
	GETFLOAT(pFrame->sceneProperties->miniMapScale);
	string cubeMap;
	GETSTRING(cubeMap);
	strcpy(pFrame->sceneProperties->cubeMap,cubeMap.c_str());


	pFileData->Unlock();
	return S_OK;
}

//-----------------------------------------------------------------------------
// Desc: Loads light + subframes
//-----------------------------------------------------------------------------
HRESULT LoadLight(LPD3DXFILEDATA pFileData, ImportFrame* pFrame ){
	DWORD cbSize;
	BYTE* pData;
	pFileData->Lock(&cbSize,(LPCVOID*)&pData);
	
	pFrame->light.name = pFrame->Name;
	GETDWORD(pFrame->light.type);
	DWORD numExcluded;
	GETDWORD(numExcluded);
	for(int i=0;i<numExcluded;i++){
		string item;
		GETSTRING(item);
		// Replace spaces with underscores, so the include list matches the format .x files use
		findandreplace(item," ","_");
		pFrame->light.excludeIncludeList.push_back(item);
	}
	GETDWORD(pFrame->light.IsExcludeList);
	pFrame->light.IsExcludeList = !pFrame->light.IsExcludeList;
	GETSTRING(pFrame->light.projectionMap);
	GETSTRING(pFrame->light.shadowMap);
 
	// Enumerate to find light setting frames
	SIZE_T cChildren;
    pFileData->GetChildren(&cChildren);
    for (UINT iChild = 0; iChild < cChildren; iChild++)
    {
		// Query the child for its FileData
		LPD3DXFILEDATA pChildData;
        DXASSERT(pFileData->GetChild(iChild, &pChildData ));
		// Sanity check
		GUID Guid;
		pChildData->GetType( &Guid );
		assert(Guid == DXFILEOBJ_CustomLightSettings);

		// Extract map...
		LightFrame* settings, settings2;
		pChildData->Lock(&cbSize,(LPCVOID*)&settings);
		// Copy the valid data (up until matrix)
		memcpy(&settings2,settings,sizeof(LightFrame)-sizeof(Matrix));
		pFrame->light.keyframes.push_back(settings2);
	
		pChildData->Unlock();
		SAFE_RELEASE(pChildData);
	}

	if(pFrame->type != TYPE_ENTITYORPREFAB)
		pFrame->type = TYPE_LIGHT;
	pFileData->Unlock();
	return S_OK;
}

//-----------------------------------------------------------------------------
// Desc: Reads entity/prefab property buffer from node
//-----------------------------------------------------------------------------
HRESULT LoadProperties(LPD3DXFILEDATA pFileData, ImportFrame* pFrame ){
	DWORD cbSize;
	BYTE* pData;
	pFileData->Lock(&cbSize,(LPCVOID*)&pData);

	// Attributes
	GETSTRING(pFrame->nodeData.filename);
	GETSTRING(pFrame->nodeData.classname);
	GETSTRING(pFrame->nodeData.parentclass);

	// Only set entity flag if it has a class, because sometimes we get junk entities with no class
	// that should really be world geometry/lights/etc
	if(pFrame->nodeData.classname.length())
		pFrame->type = TYPE_ENTITYORPREFAB;

	// Class params
	DWORD numParameters;
	GETDWORD(numParameters);
	for(int i=0;i<numParameters;i++){
		string param;
		GETSTRING(param);
		pFrame->nodeData.parameters.push_back(param);
	}

	for(int i=0;i<numParameters;i++){
		string param;
		GETSTRING(param);
		pFrame->nodeData.paramvalues.push_back(param);
	}

	// Spherical Harmonics data
	GETWORD(pFrame->nodeData.shEnabled);

	// Extract timestamp objects
	SIZE_T cChildren;
    pFileData->GetChildren(&cChildren);
    for (UINT iChild = 0; iChild < cChildren; iChild++)
    {
		// Little hack to make macro work (macro uses pFileData only)
		LPD3DXFILEDATA temp = pFileData;
		DXASSERT(pFileData->GetChild(iChild, &pFileData ));
		GUID Guid;
		pFileData->GetType( &Guid );

		BYTE* pData;
		pFileData->Lock(&cbSize,(LPCVOID*)&pData);

		if(iChild == 0 && Guid == DXFILEOBJ_TimeStamp)
			GETTIMESTAMP(pFrame->nodeData.timeMoved);
		if(iChild == 1 && Guid == DXFILEOBJ_TimeStamp)
			GETTIMESTAMP(pFrame->nodeData.timeModified);

		// Spherical Harmonics Data
		//
		//
		if(Guid == DXFILEOBJ_SHProperties)
		{
			GETSTRING(pFrame->nodeData.receiverGroup);
			GETDWORD(numParameters);
			for(int i=0;i<numParameters;i++){
				string val;
				GETSTRING(val);
				pFrame->nodeData.inBlockers.push_back(val);
			}
			GETDWORD(numParameters);
			for(int i=0;i<numParameters;i++){
				string val;
				GETSTRING(val);
				pFrame->nodeData.outBlockers.push_back(val);
			}

			GETOBJECT(pFrame->nodeData.sh);
		}


		SAFE_RELEASE(pFileData);
		pFileData = temp;
	}
			
	pFileData->Unlock();
	return S_OK;
}


//-----------------------------------------------------------------------------
// Desc: Reads a material linked to a mesh. There may be many of these per mesh
//-----------------------------------------------------------------------------
HRESULT LoadMaterial(LPD3DXFILEDATA pFileData, ImportFrame* pFrame){
	DWORD cbSize;
	BYTE* pData;
	pFileData->Lock(&cbSize,(LPCVOID*)&pData);

	Material mat;
	// Hacky way to give materials unique ids for lookup
	static int unique_id = 0;
	mat.id = unique_id;
	unique_id++;
	GETSTRING(mat.name);
	GETSTRING(mat.shader);
	GETSTRING(mat.technique);

	// Enumerate to find texture maps and shader parameters
	SIZE_T cChildren;
    pFileData->GetChildren(&cChildren);
    for (UINT iChild = 0; iChild < cChildren; iChild++)
    {
		// Little hack to make macro work (macro uses pFileData only)
		LPD3DXFILEDATA temp = pFileData;
		DXASSERT(pFileData->GetChild(iChild, &pFileData ));
		GUID Guid;
		pFileData->GetType( &Guid );

		BYTE* pData;
		pFileData->Lock(&cbSize,(LPCVOID*)&pData);

		// Look for params
		if(Guid == DXFILEOBJ_EffectParamDWord){
			ShaderParam p;
			p.type = PARAM_INT;
			GETSTRING(p.name);
			GETDWORD(p.iVal);
			mat.params.push_back(p);
		}
		else if(Guid == DXFILEOBJ_EffectParamFloats){
			ShaderParam p;
			
			GETSTRING(p.name);
			DWORD numFloats;
			GETDWORD(numFloats);
			vector<float> floats;
			for(int i=0;i<numFloats;i++){
				float aFloat;
				GETFLOAT(aFloat);
				floats.push_back(aFloat);
			}

			if(numFloats == 4){
				p.type = PARAM_FLOAT4;
				p.vVal = *(D3DXVECTOR4*)&floats[0];
			}
			if(numFloats == 1){
				p.type = PARAM_FLOAT;
				p.fVal = floats[0];
			}

			mat.params.push_back(p);
		}
		else if(Guid == DXFILEOBJ_EffectParamString){
			ShaderParam p;
			p.type = PARAM_TEXTURE;
			GETSTRING(p.name);
			GETSTRING(p.sVal);
			mat.params.push_back(p);
		}
		else if(Guid == DXFILEOBJ_CustomTextureMap){
			// Extract map...
			TexMap tmap;
			GETDWORD(tmap.type);
			GETSTRING(tmap.texVar);
			GETSTRING(tmap.transformVar);
			GETSTRING(tmap.filename);
			GETFLOAT(tmap.amount);
			GETFLOAT(tmap.uTile);
			GETFLOAT(tmap.vTile);
			GETFLOAT(tmap.uOff);
			GETFLOAT(tmap.vOff);
			// Add to list
			mat.maps.push_back(tmap);
		}
		else 
			Error("Unknown template found in material!\n");

		SAFE_RELEASE(pFileData);
		pFileData = temp;
	}
	pFrame->materials.push_back(mat);

	pFileData->Unlock();
	return S_OK;
}

//-----------------------------------------------------------------------------
// Desc: Loads a D3D Mesh from the file into the frame
//-----------------------------------------------------------------------------
HRESULT LoadMesh(LPD3DXFILEDATA pFileData, ImportFrame* pFrame )
{
	// Currently only allowing one mesh per frame
	if( pFrame->pMesh )
		Error("More than one mesh per frame... contact a programmer.");

	// Load the mesh from the DXFILEDATA object
	HRESULT hr = D3DXLoadSkinMeshFromXof( pFileData, D3DXMESH_SYSTEMMEM, g_pd3dDevice,
		&pFrame->pAdjacency, &pFrame->pMaterials,NULL,
		&pFrame->NumMaterials, &pFrame->pSkinInfo,&pFrame->pMesh );
 
	if(hr == D3DXERR_LOADEDMESHASNODATA){
		pFrame->pMaterials		= NULL;
		pFrame->NumMaterials	= NULL;
		pFrame->pSkinInfo		= NULL;
		pFrame->pMesh			= NULL;
		pFrame->pAdjacency		= NULL;
		Warning("Mesh %s has no polygon data. You should delete it from the level",pFrame->Name);
		return S_OK;
	}
	else if(hr != S_OK)
		Error("Error in LoadMesh(%s): %s", pFrame->Name, DXGetErrorString9(hr));

	// Stupid D3DX generates skin info for 0 bones. So delete it.
	if(pFrame->pSkinInfo && pFrame->pSkinInfo->GetNumBones() == 0){
		SAFE_RELEASE(pFrame->pSkinInfo);
	}

	// FIXME: Mem leak, but don't want meshes deleted until exporting completes 100%
	// or it fucks up include/exclude pointers
	pFrame->mesh = new Mesh;

	TotalVerticesRead += pFrame->pMesh->GetNumVertices();
	TotalFacesRead += pFrame->pMesh->GetNumFaces();

	if(pFrame->type != TYPE_ENTITYORPREFAB)
		pFrame->type = TYPE_STATIC_GEOMETRY;

	return S_OK;
}

//-----------------------------------------------------------------------------
// Desc: Reads a frame, then calls the appropirate object loading function
//-----------------------------------------------------------------------------
HRESULT LoadFrame(LPD3DXFILEDATA pFileData, ImportFrame* pParentFrame)
{
	LPD3DXFILEDATA   pChildData = NULL;
	LPDIRECTXFILEOBJECT pChildObj = NULL;
	GUID Guid;
	DWORD       cbSize;
	ImportFrame*  pCurrentFrame;
	HRESULT     hr;

	// Get the type of the object
	if( FAILED( hr = pFileData->GetType( &Guid ) ) )
		return hr;

	if( Guid == DXFILEOBJ_SceneProperties ){
		LoadSceneProperties( pFileData, pParentFrame);
	}
	if( Guid == DXFILEOBJ_CustomLight ){
		LoadLight( pFileData, pParentFrame);
	}
	if( Guid == DXFILEOBJ_NodeProperties ){
		LoadProperties( pFileData, pParentFrame);
	}
	if( Guid == DXFILEOBJ_CustomMaterial ){
		LoadMaterial( pFileData, pParentFrame);
	}
	if( Guid == TID_D3DRMMesh )
	{
		hr = LoadMesh( pFileData, pParentFrame );
		if( FAILED(hr) )
			return hr;
	}
	if( Guid == TID_D3DRMFrameTransformMatrix )
	{
		D3DXMATRIX* pmatMatrix;
        hr = pFileData->Lock(&cbSize, (LPCVOID*)&pmatMatrix );
        if( FAILED(hr) )
            return hr;

		// Update the parent's matrix with the new one
		pParentFrame->TransformationMatrix = *(Matrix*)pmatMatrix ;
	}
	if( Guid == TID_D3DRMFrame )
	{
		// Get the frame name
		CHAR  strAnsiName[512] = "";
		TCHAR strName[512];
		DWORD dwNameLength = 512;
		if( FAILED( hr = pFileData->GetName( strAnsiName, &dwNameLength ) ) )
			return hr;
		DXUtil_ConvertAnsiStringToGenericCb( strName, strAnsiName, sizeof(strName) );

		// Use parent name if name is ""
		string name = strName;
		if(name.length() == 0){
			name = string(pParentFrame->Name) + "_Child";
		}

		// Create the frame
		pCurrentFrame = new ImportFrame( name.c_str() );
		if( pCurrentFrame == NULL )
			return E_OUTOFMEMORY;

		pCurrentFrame->m_pNext = pParentFrame->m_pChild;
		pParentFrame->m_pChild = pCurrentFrame;
		pCurrentFrame->type = TYPE_UNKNOWN;

		// Enumerate child objects
		SIZE_T cChildren;
		pFileData->GetChildren(&cChildren);
		for (UINT iChild = 0; iChild < cChildren; iChild++)
		{
			// Query the child for its FileData
			hr = pFileData->GetChild(iChild, &pChildData );
			if( SUCCEEDED(hr) )
			{
				hr = LoadFrame( pChildData, pCurrentFrame );
				SAFE_RELEASE( pChildData );
			}
			// Probably a reference...
			else{
				if(pChildData->IsReference())
					Error("Bang!? What to do...");
			}

			if( FAILED(hr) )
				return hr;
		}

	}

	return S_OK;
}


//-----------------------------------------------------------------------------
// Converts include strings to pointers for fast lookup later on
//-----------------------------------------------------------------------------
void ConvertIncludeLists(ImportFrame* root, ImportFrame* frame){
	for(int j=0;j<frame->light.excludeIncludeList.size();j++){
		ImportFrame* ret = root->FindMeshFrame(frame->light.excludeIncludeList[j]);
		// Ret can sometimes be NULL if the light include list is outdated. It happens!
		// The mesh that was in the light list could also be 'hidden'!
		if(ret)
			frame->light.excludeIncludeMeshPointers.push_back(ret->mesh);
	}

	if(frame->m_pChild){
		ConvertIncludeLists(root,frame->m_pChild);
	}
	if(frame->m_pNext){
		ConvertIncludeLists(root,frame->m_pNext);
	}
}


//-----------------------------------------------------------------------------
// Desc: Loads a D3D Mesh from the file into the frame
//-----------------------------------------------------------------------------
bool XImport::Import(string filename, bool& success){
	HRESULT hr;
	LPD3DXFILE			 dxFile = NULL;
	LPD3DXFILEENUMOBJECT pEnumObj = NULL;

	DXASSERT(D3DXFileCreate(&dxFile));

	// Register templates for d3drm and patch extensions.
	DXASSERT(dxFile->RegisterTemplates((void*)D3DRM_XTEMPLATES,D3DRM_XTEMPLATE_BYTES));
	DXASSERT(dxFile->RegisterTemplates((void*)XSKINEXP_TEMPLATES,strlen(XSKINEXP_TEMPLATES)));
	DXASSERT(dxFile->RegisterTemplates((void*)XCUSTOM_TEMPLATES,strlen(XCUSTOM_TEMPLATES)));
	(dxFile->CreateEnumObject((LPVOID)filename.c_str(),DXFILELOAD_FROMFILE,&pEnumObj));
 
	if(!pEnumObj){
		Error("Error loading file %s. It may be in an invalid format, or does not exist in the specified location. Aborting.",filename.c_str());
		return false;
	}
 
	LogPrintf("Parsing file...\n");

    // Enumerate top level objects (which are always frames)
	int count = 0;
	LPD3DXFILEDATA       pFileData = NULL;
	SIZE_T cChildren;
    pEnumObj->GetChildren(&cChildren);
    for (UINT iChild = 0; iChild < cChildren; iChild++)
    {
        hr = pEnumObj->GetChild(iChild, &pFileData);
        if (FAILED(hr))
            return hr;

        DXASSERT(LoadFrame( pFileData,&sceneRoot));
        SAFE_RELEASE( pFileData );
		count++;
    }

    SAFE_RELEASE( pFileData );
    SAFE_RELEASE( pEnumObj );
    SAFE_RELEASE( dxFile );

	//sceneRoot.PrintFrames();
	ConvertIncludeLists(&sceneRoot,&sceneRoot);
	CalculateCombinedMatrices(&sceneRoot,NULL);

	if(count == 0){
		Warning("No frames found in file %s",filename.c_str());
		return false;
	}

	
	LogPrintf("..completed. %d faces and %d vertices read.\n",TotalFacesRead,TotalVerticesRead);

	return true;
}