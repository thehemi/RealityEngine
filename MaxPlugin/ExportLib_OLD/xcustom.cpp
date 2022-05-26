//-----------------------------------------------------------------------------
// File: XCustom
// Desc: Functions used to export max data to an X File
// Author: Reauthored by Tim Johnson, 2003, as custom format
//
// Change Log (All code changed is labelled with below numbers):
// [1] Added templates 'CustomTextureMap' and 'CustomMaterial'
// [2] Added template 'Light'
// [3] Addition of template for custom data stored per-mesh
// [4] Option to export ONLY animation data
// [5] Converted to util plugin lib
//
// NOTE: Remember to use gWorldScale to scale elements that aren't scaled in the macros
//
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "..\..\Shared\Shared.h"
#include <initguid.h>
#include "XSkinExp.h"
#include "MeshData.h"
#include "decomp.h"
#include "XSkinExpTemplates.h"
#include "dxfile.h"
#include "Shaders.h"
#include "rmxfguid.h"
#include <sstream>
#include <fstream>
#include "IDXMaterial.h"
#include "..\Max HLSL\LIBS\inc\nv_sys\nv_sys.h"
#include "..\Max HLSL\TOOLS\inc\cgfxMax\icgfxdatabridge.h"
#include "IViewportManager.h" // IDXShaderManagerInterface
#include "MapSettings.h"
#include "PRTOptionsDlg.h"
#include "SphericalHarmonics.h"

using namespace nv_sys;

// Creates and asserts an xfile object
#define CREATEOBJECT(x) \
	LPDIRECTXFILEDATA pDataObject = NULL; \
	DWORD cbSize = pbCur - pbData; \
	DXASSERT(psc->m_pxofsave->CreateDataObject(x, \
	NULL, \
	NULL, \
	cbSize, \
	pbData, \
	&pDataObject \
	)); 


#define ADDOBJECT() \
	DXASSERT(pParent->AddDataObject(pDataObject)); \
	RELEASE(pDataObject); \
	delete []pbData; 
	

#define WRITESTRING(x) \
{  \
	TCHAR* str = psc->m_stStrings.CreateNiceFilename((TCHAR*)x.c_str()); \
	WRITE_STRING(pbCur,str); \
}


#define WRITENICESTRING(x) \
{  \
	TCHAR* str = psc->m_stStrings.CreateNiceString((TCHAR*)x.c_str()); \
	WRITE_STRING(pbCur,str); \
}

//-----------------------------------------------------------------------------
// A light can have many settings over a time period
//-----------------------------------------------------------------------------
void AddLightSettings(LightState* ls, GenLight* light, DWORD t,   SSaveContext *psc, LPDIRECTXFILEDATA pParent)
{
	HRESULT hr = S_OK;
	PBYTE pbData = NULL;
	PBYTE pbCur;
	pbData = pbCur = new BYTE[10240]; // 10K data should cover us

	WRITE_DWORD(pbCur, t);
	WRITE_COLOR(pbCur, D3DXCOLOR(ls->color.r,ls->color.g,ls->color.b,1));
	WRITE_FLOAT(pbCur, ls->intens);

	WRITE_FLOAT(pbCur, ls->hotsize);
	WRITE_FLOAT(pbCur, ls->fallsize);
	float start = ls->attenStart*gWorldScale;
	float end   = ls->attenEnd*gWorldScale;
	WRITE_FLOAT(pbCur, start);
	WRITE_FLOAT(pbCur, end);

	// NO MATRIX. Matrix will be exported as part of frame hierarchy. This function doesn't work anyhow
	//WRITE_MATRIX4_FROM_MATRIX3(pbCur,ls->tm);

	CREATEOBJECT(DXFILEOBJ_CustomLightSettings);
	ADDOBJECT();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void AddTimeStamp(SSaveContext *psc,SYSTEMTIME time, LPDIRECTXFILEDATA pParent)
{
	HRESULT hr = S_OK;
	PBYTE pbData = NULL;
	PBYTE pbCur;
	pbData = pbCur = new BYTE[1000];

	WRITE_WORD(pbCur,time.wYear);  
	WRITE_WORD(pbCur,time.wMonth);  
	WRITE_WORD(pbCur,time.wDayOfWeek); 
	WRITE_WORD(pbCur,time.wDay);  
	WRITE_WORD(pbCur,time.wHour);  
	WRITE_WORD(pbCur,time.wMinute);  
	WRITE_WORD(pbCur,time.wSecond);  
	WRITE_WORD(pbCur,time.wMilliseconds);

	CREATEOBJECT(DXFILEOBJ_TimeStamp);
	ADDOBJECT();
}

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
void AddSHProperties(SSaveContext *psc, NodeData& data, LPDIRECTXFILEDATA pParent)
{
	HRESULT hr = S_OK;
	PBYTE pbData = NULL;
	PBYTE pbCur;
	pbData = pbCur = new BYTE[10000];

	WRITENICESTRING(data.receiverGroup);
	// Indoor blockers
	DWORD num = data.inBlockers.size();
	WRITE_DWORD(pbCur,num);
	for(int i=0;i<num;i++)
		WRITENICESTRING(data.inBlockers[i]);

	// Outdoor blockers
	num = data.outBlockers.size();
	WRITE_DWORD(pbCur,num);
	for(int i=0;i<num;i++)
		WRITENICESTRING(data.outBlockers[i]);

	// Custom copy of simulator options that includes the global overrides
	SceneProperties* props = theMap.GetSceneProperties();

	// Temp will be modified by UpdateGlobalSettings
	NodeData temp = data;
	// If using or forced global settings, fetch them now
	if(!data.bUseCustom || props->bPRTOverride)
	{
		theSH.UpdateGlobalProperties(temp);
	}

	SIMULATOR_OPTIONS sh = temp.sh;

	// Multiply by lobal percentages
	sh.dwNumRays	 *= float(props->dwRays)/100.f;
	sh.dwNumBounces  *= float(props->dwBounces)/100.f;
	sh.dwTextureSize *= float(props->dwTextureSize)/100.f;
	sh.dwTextureSize =  NearestPow2(sh.dwTextureSize);

	memcpy(pbCur,&sh,sizeof(sh));
	pbCur += sizeof(sh);

	CREATEOBJECT(DXFILEOBJ_SHProperties);
	ADDOBJECT();
}

//-----------------------------------------------------------------------------
// Writes out our custom node/entity properties
//-----------------------------------------------------------------------------
HRESULT AddCustomProperties(SSaveContext *psc,INode* node,LPDIRECTXFILEDATA pParent)
{
	HRESULT hr = S_OK;
	PBYTE pbData = NULL;
	PBYTE pbCur;
	CStr buf;
	pbData = pbCur = new BYTE[10240]; // 10K data should cover us

	NodeData nData;
	GetNodeData(node,nData);

	WRITESTRING(nData.filename);
	WRITESTRING(nData.classname);
	WRITESTRING(nData.parentclass);

	// Write all param/paramvalue pairs
	DWORD num = nData.parameters.size();
	WRITE_DWORD(pbCur,num);
	for(int i=0;i<num;i++)
		WRITESTRING(nData.parameters[i]);
	for(int i=0;i<num;i++)
		WRITESTRING(nData.paramvalues[i]);

	WRITE_WORD(pbCur, nData.shEnabled);

	CREATEOBJECT(DXFILEOBJ_NodeProperties);

	AddTimeStamp(psc, nData.timeMoved,pDataObject);
	AddTimeStamp(psc, nData.timeModified,pDataObject);
	if(nData.shEnabled)
		AddSHProperties(psc, nData, pDataObject);

	ADDOBJECT();
	return hr;
}

//-----------------------------------------------------------------------------
// A light
//-----------------------------------------------------------------------------
HRESULT AddLight
(
 SSaveContext *psc,
 INode *node, 
 LPDIRECTXFILEDATA pParent
 ){
	 int t = 0;
	 HRESULT hr = S_OK;
	 PBYTE pbData = NULL;
	 PBYTE pbCur;
	 pbData = pbCur = new BYTE[20240]; // 20K data should cover us

	 ObjectState os = node->EvalWorldState(t);
	 if (!os.obj) {
		 Error("Light is invalid. Should never happen. tell Tim");
		 return S_OK;
	 }

	 GenLight* light = (GenLight*)os.obj;
	 struct LightState ls;
	 Interval valid = FOREVER;
	 Interval animRange = psc->m_pInterface->GetAnimRange();

	 light->EvalLightState(t, valid, &ls);

	 // This is part os the lightState, but it doesn't
	 // make sense to output as an animated setting so
	 // we dump it outside of ExportLightSettings()

	 DWORD type;
	 switch(light->Type()) {
	case OMNI_LIGHT:  type = LIGHT_OMNI; break;
	case TSPOT_LIGHT: type = LIGHT_SPOT;  break;
	case DIR_LIGHT:   type = LIGHT_DIR; break;
	case FSPOT_LIGHT: type = LIGHT_SPOT; break;
	 }

	 WRITE_DWORD(pbCur, type);

	 //fprintf(pStream,"%s\t%s %s\n", indent.data(), ID_LIGHT_SPOTSHAPE, 
	 //	light->GetSpotShape() == RECT_LIGHT ? ID_LIGHT_SHAPE_RECT : ID_LIGHT_SHAPE_CIRC);

	 ExclList* el = light->GetExclList();  // DS 8/31/00 . switched to NodeIDTab from NameTab
	 vector<TCHAR*> excluded;
	 // TimJohnson: Only export exclude list when applies to illumination, not shadowing
	 if (el->Count() && el->TestFlag(NT_AFFECT_ILLUM)) {
		 for (int nameid = 0; nameid < el->Count(); nameid++) {
			 INode *n = (*el)[nameid];	// DS 8/31/00
			 if (n){
				 TCHAR* name = psc->m_stStrings.CreateNiceFilename(n->GetName());
				 excluded.push_back(name);
			 }
		 }
	 }

	 DWORD numExcluded = excluded.size();
	 WRITE_DWORD(pbCur, numExcluded);
	 for(int i=0;i<excluded.size();i++)
		 WRITE_STRING(pbCur, excluded[i]);

	 // Is this an include list? Or exclude?
	 DWORD bIncludList = el->TestFlag(NT_INCLUDE);
	 WRITE_DWORD(pbCur, bIncludList);

	 // Does this have a projection map?
	 Texmap* tex = light->GetProjMap();
	 if (tex && tex->ClassID() == Class_ID(BMTEX_CLASS_ID, 0x00)) {
		 TCHAR* str = ((BitmapTex *)tex)->GetMapName();
		 TCHAR* name = psc->m_stStrings.CreateNiceFilename(str);
		 WRITE_STRING(pbCur, name);
	 }
	 else{
		 TCHAR* name = psc->m_stStrings.CreateNiceFilename("");
		 WRITE_STRING(pbCur, name);
	 }

	 // Does this have a shadow map?
	 if(light->GetUseShadowColorMap(0))
		 tex = light->GetShadowProjMap();
	 else
		 tex = NULL;

	 if (tex && tex->ClassID() == Class_ID(BMTEX_CLASS_ID, 0x00)) {
		 TCHAR* str = ((BitmapTex *)tex)->GetMapName();
		 TCHAR* name = psc->m_stStrings.CreateNiceFilename(str);
		 WRITE_STRING(pbCur, name);
	 }
	 else{
		 TCHAR* name = psc->m_stStrings.CreateNiceFilename("");
		 WRITE_STRING(pbCur, name);
	 }


	 CREATEOBJECT(DXFILEOBJ_CustomLight);

	 // Export light settings for frame 0
	 AddLightSettings(&ls, light, t, psc, pDataObject);

	 // Export animated light settings
	 if (!valid.InInterval(animRange)) {
		 TimeValue t = animRange.Start();

		 while (1) {
			 valid = FOREVER; // Extend the validity interval so the camera can shrink it.
			 light->EvalLightState(t, valid, &ls);

			 t = valid.Start() < animRange.Start() ? animRange.Start() : valid.Start();

			 // Export the light settings at this frame
			 AddLightSettings(&ls, light, t, psc, pDataObject);

			 if (valid.End() >= animRange.End()) {
				 break;
			 }
			 else {
				 // FIXME: This is completely wrong
				 t = (valid.End()/GetTicksPerFrame()+psc->m_iAnimSamplingRate) * GetTicksPerFrame();
			 }
		 }
	 }

	 // Close object
	 ADDOBJECT();
	 return S_OK;
 }

 //-----------------------------------------------------------------------------
 // Checks if node is used/valid light
 //-----------------------------------------------------------------------------
 bool IsExportableLight(INode* node){
	 if( node->IsNodeHidden()){
		 return false;
	 }

	 ObjectState os = node->EvalWorldState(0);
	 if (!os.obj || os.obj->SuperClassID() != LIGHT_CLASS_ID) {
		 return false;
	 }
	 GenLight* light = (GenLight*)os.obj;


	 // Don't export light if it's turned off!
	 if(!light->GetUseLight())
		 return false;

	 return true;
 }


 //-----------------------------------------------------------------------------
 // Dumps UV coords
 //-----------------------------------------------------------------------------
 void DumpUVGen(CustomTextureMap& map, StdUVGen* uvGen)
 {
	 //int mapType = uvGen->GetCoordMapping(0);
	 map.uOff = uvGen->GetUOffs(0);
	 map.vOff = uvGen->GetVOffs(0);
	 map.uTile = uvGen->GetUScl(0);
	 map.vTile = uvGen->GetVScl(0);
	 if(uvGen->GetAng(0) > 1 || uvGen->GetAng(0) < -1)
		 Error("Minor warning for '%s': UV Angle rotations aren't supported on individual texture maps (Diffuse, Bump, etc).\n Please remove the rotation (set to 0,0,0) from the map and apply it to the object base UVW instead.",map.name);
 }


 //-----------------------------------------------------------------------------
 // Dumps texture map + submaps
 // index is the slot#, used to resolve mixmaps (which are always the third slot)
 //-----------------------------------------------------------------------------
 void DumpTexture(SSaveContext *psc, CUSTOMD3DXMATERIAL* pMaterial, Texmap* tex, Class_ID cid, int slotID, int depth, float amt, int index)
 {
	 if (!tex) return;

	 TSTR className;
	 tex->GetClassName(className);

	 CustomTextureMap map;
	 map.amount = amt;

	 // Is this a bitmap texture?
	 // We know some extra bits 'n pieces about the bitmap texture
	 if (tex->ClassID() == Class_ID(BMTEX_CLASS_ID, 0x00)) {
		 TCHAR* str = ((BitmapTex *)tex)->GetMapName();
		 map.name = psc->m_stStrings.CreateNiceFilename(str);

		 StdUVGen* uvGen = ((BitmapTex *)tex)->GetUVGen();
		 if (uvGen) {
			 DumpUVGen(map,uvGen);
		 }

		 TextureOutput* texout = ((BitmapTex*)tex)->GetTexout();
		 if (texout->GetInvert()) {
			 Error("Warning: Map %s uses an Invert. This Invert will not be applied in the game.",tex->GetName());
		 }
	 }
	 else if(tex->ClassID() != Class_ID(MIX_CLASS_ID,0x00)){
		 Error("Map %s is not a bitmap or a mixmap.",tex->GetName());
	 }

	 depth++;
	 for (int i=0; i<tex->NumSubTexmaps(); i++) {
		 DumpTexture(psc, pMaterial, tex->GetSubTexmap(i), tex->ClassID(), slotID, depth, 1.0f,i);
	 }

	 // Don't save mixmap containers, just output their textures (above)
	 if(tex->ClassID() == Class_ID(MIX_CLASS_ID,0x00))
		 return;

	 // Set hard-coded variables
	 if(depth > 1 && index == 2){
		 map.type			= TT_DIFFUSE;
		 map.texVar			= "tMix"+ToStr(depth-2);
		 map.transformVar	= "fMix"+ToStr(depth-2)+"Transform";
	 }
	 else if(slotID == ID_DI){
		 map.type			= TT_DIFFUSE;
		 map.texVar			= "tDiffuse"+ToStr(index);
		 map.transformVar	= "fDiffuse"+ToStr(index)+"Transform";
	 }
	 else if(slotID == ID_BU){
		 map.type			= TT_BUMP;
		 map.texVar			= "tBump"+ToStr(index);
		 map.transformVar	= "fBump"+ToStr(index)+"Transform";
	 }
	 else if(slotID == ID_SP || slotID == ID_SH){
		 map.type			= TT_PUTINALPHA;
		 map.texVar			= "tBump"+ToStr(index);
		 map.transformVar	= "fBump"+ToStr(index)+"Transform";
	 }
	 else if(slotID == ID_SI){
		 map.type			= TT_PUTINALPHA;
		 map.texVar			= "tDiffuse"+ToStr(index);
		 map.transformVar	= "fDiffuse"+ToStr(index)+"Transform";
	 }
	 else if(slotID == ID_RL){
		 map.type			= TT_CUBE;
		 map.texVar			= "tReflection"+ToStr(index);
		 map.transformVar	= "fReflection"+ToStr(index)+"Transform";
	 }
	 else
		 Error("Error - texture '%s' is not one of: Diffuse, Specular Level, Self Illumination, Bump, Reflection. These are the only supported types.",tex->GetName());

	 pMaterial->maps.push_back(map);
 }





 //-----------------------------------------------------------------------------
 // Dumps special shader material
 //-----------------------------------------------------------------------------
 HRESULT DumpShaderMaterial(SSaveContext *psc, CUSTOMD3DXMATERIAL *pMaterial, Mtl* mtl){
	 IDxMaterial* idxm = (IDxMaterial*)mtl->GetInterface(IDXMATERIAL_INTERFACE);
	 if(!idxm)
		 return S_FALSE;

	 pMaterial->dxMaterial = true;

	 pMaterial->shader = StripPath(idxm->GetEffectFilename());

	 for(int i=0;i<mtl->NumParamBlocks();i++){
		 Interval interval = FOREVER;
		 IParamBlock2* block = mtl->GetParamBlock(i);
		 ParamBlockDesc2 *desc = mtl->GetParamBlock(i)->GetDesc();
		 if(string(desc->int_name) == "Technique"){
			 // NOTE: Neil Hazzard is a wanker, he didn't allow access to the technique string
			 // We must get the index and parse the file manually (ick!!)
			 int index;
			 block->GetValue(0,0,index,interval);
			 // Look up technique in file
			 ifstream file(idxm->GetEffectFilename());
			 char buf[512];
			 int techniqueNumber = 0;
			 while(!file.eof()){
				 file.getline(buf,512);
				 string line = buf;

				 if(AsLower(line).find("technique ") == 0){
					 // This is the technique we need!
					 if(techniqueNumber == index){
						 // strip "technique "
						 string str = line.substr(10);

						 // strip possible "{"
						 int fi = str.find("{");
						 if(fi != -1)
							 str = str.substr(0,fi);

						 // Remove any whitespace
						 trimLeft(str);
						 trimRight(str);

						 pMaterial->technique = str;
						 break;
					 }
					 techniqueNumber++;
				 }
			 }

			 file.close();

			 if(pMaterial->technique.length() == 0)
				 Error("There was an internal error trying to look up the chosen technique for shader %s\nPlease report this to tim@helixcore.com",pMaterial->shader.c_str());
		 }
		 if(i == 0){ // FIXME: use name

			 // Extract all parameters
			 for(int j=0;j<desc->Count();j++){
				 ShaderParam param;
				 param.type = block->GetParameterType(j);
				 param.name = desc->GetParamDef(j).int_name;

				 // Hacky way to avoid exporting these vars. SHould do by semantic.
				 if(AsLower(param.name) == "lightpos" ||  AsLower(param.name) == "lightdir")
					 continue;

				 bool found = true;
				 switch(param.type){
					case TYPE_FLOAT:
						param.type = PARAM_FLOAT;
						block->GetValue(j,0,param.fVal,interval);
						break;
					case TYPE_INT:{
						int i;
						param.type = PARAM_INT;
						block->GetValue(j,0,i,interval);
						param.iVal = i;
						break;
								  }
					case TYPE_FRGBA:
					case TYPE_RGBA:
						param.type = PARAM_FLOAT4;
						block->GetValue(j,0,*(AColor*)&param.vVal,interval);
						break;
					case TYPE_BITMAP:{
						param.type = PARAM_TEXTURE;
						PBBitmap* bit;
						block->GetValue(j,0,bit,interval);
						param.sVal = bit->bi.Filename();
						param.sVal = StripPath(param.sVal);
									 }
									 break;
					case TYPE_FILENAME:
					case TYPE_STRING:{
						param.type = PARAM_TEXTURE;
						TCHAR* str;
						block->GetValue(j,0,str,interval);
						param.sVal = StripPath(str);
									 }
									 break;
					default:
						found = false;
						break;
				 }

				 if(found)
					 pMaterial->params.push_back(param);
			 }
		 }
	 }

	 return S_OK;
 }

 //-----------------------------------------------------------------------------
 // Dumps entire material
 //-----------------------------------------------------------------------------
 HRESULT GatherCustomMaterialProperties(SSaveContext *psc, CUSTOMD3DXMATERIAL *pMaterial, Mtl* mtl){
	 int t = 0;
	 TSTR className;
	 mtl->GetClassName(className);
	 pMaterial->matname = psc->m_stStrings.CreateNiceString(mtl->GetName());

	// if(DumpHLSLMaterial(psc,pMaterial,mtl) == S_OK)
	//	 return S_OK; // Dumped HLSL material, we're done

	 if(DumpShaderMaterial(psc,pMaterial,mtl) == S_OK)
		 return S_OK; // Dumped DX9 material, we're done

	 // Set hard-coded shader properties for this material
	 pMaterial->shader			= "Max_Default.fx";
	 pMaterial->technique	= "Diffuse";

	 // We know the Standard material, so we can get some extra info
	 if (mtl->ClassID() == Class_ID(DMTL_CLASS_ID, 0)) {
		 StdMat* std = (StdMat*)mtl;

		 AColor specular = mtl->GetSpecular();
		 specular *= mtl->GetShinStr(); // Multiply specular color by level, so they are combined
		 specular.a = 1;

		 AColor diffuse = mtl->GetDiffuse();
		 diffuse.a = 1-mtl->GetXParency();

		 AColor emissive;
		 emissive.a = 1;
		 if(std->GetSelfIllumColorOn()){
			 emissive = std->GetSelfIllumColor();
		 }
		 else{
			 float amt = std->GetSelfIllum(t);
			 emissive = AColor(amt,amt,amt,1.0f);
		 }

		 int transType;
		 switch (std->GetTransparencyType()) {
			 // Not checking filter because it's "always" on
			 //case TRANSP_FILTER:		 transType = MAT_FILTER; break;
			case TRANSP_SUBTRACTIVE: transType = MAT_SUBTRACTIVE; break;
			case TRANSP_ADDITIVE:	 transType = MAT_ADDITIVE; break;
			default:				 
				if(mtl->GetXParency()>0.01f)
					transType = MAT_FILTER; 
				else
					transType = MAT_NONE;

				break;
		 }

		 pMaterial->params.push_back(ShaderParam("MatDiffuse",*(D3DXCOLOR*)&diffuse));
		 //pMaterial->params.push_back(ShaderParam("MatSpecular",*(D3DXCOLOR*)&specular));
		 pMaterial->params.push_back(ShaderParam("MatEmissive",*(D3DXCOLOR*)&emissive));
		 pMaterial->params.push_back(ShaderParam("SpecPower",(mtl->GetShininess() * 100.f)));
		 pMaterial->params.push_back(ShaderParam("AlphaBlend",(DWORD)(transType != MAT_NONE)));

		 if(transType == MAT_ADDITIVE){
			 pMaterial->params.push_back(ShaderParam("SourceBlend",(DWORD)D3DBLEND_ONE));
			 pMaterial->params.push_back(ShaderParam("DestBlend",(DWORD)D3DBLEND_ONE));
		 }
		 else if(transType == MAT_FILTER){
			 pMaterial->params.push_back(ShaderParam("SourceBlend",(DWORD)D3DBLEND_SRCALPHA));
			 pMaterial->params.push_back(ShaderParam("DestBlend",(DWORD)D3DBLEND_INVSRCALPHA));
		 }

		 // Use specular shader if we have any significant specular level in material
		 if(mtl->GetShinStr() > 0.05f)
			 pMaterial->technique	= "DiffuseSpecular";
	 }


	 for (int i=0; i<mtl->NumSubTexmaps(); i++) {
		 Texmap* subTex = mtl->GetSubTexmap(i);
		 float amt = 1.0f;
		 if (subTex) {
			 // If it is a standard material we can see if the map is enabled.
			 if (mtl->ClassID() == Class_ID(DMTL_CLASS_ID, 0)) {
				 if (!((StdMat*)mtl)->MapEnabled(i))
					 continue;
				 amt = ((StdMat*)mtl)->GetTexmapAmt(i, 0);

			 }
			 DumpTexture(psc, pMaterial, subTex, mtl->ClassID(), i, 0, amt,0);
		 }
	 }

	 return S_OK;
 }

 //-----------------------------------------------------------------------------
 // Writes out a shader paramteter
 //-----------------------------------------------------------------------------
 HRESULT AddEffectParam(SSaveContext *psc,ShaderParam& param,LPDIRECTXFILEDATA pParent)
 {
	 HRESULT hr = S_OK;
	 PBYTE pbData = NULL;
	 PBYTE pbCur;

	 pbData = pbCur = new BYTE[10240]; // 10K data should cover us

	 GUID paramTemplate;

	 WRITESTRING(param.name);

	 DWORD num;
	 switch(param.type){
		case PARAM_BOOL:
			// TODO: Make boolean export type
			paramTemplate = DXFILEOBJ_EffectParamDWord;
			num = param.bVal;
			WRITE_DWORD(pbCur,num);
			break;
		case PARAM_INT:
			paramTemplate = DXFILEOBJ_EffectParamDWord;
			WRITE_DWORD(pbCur,param.iVal);
			break;
		case PARAM_TEXTURE:
			paramTemplate = DXFILEOBJ_EffectParamString;
			WRITESTRING(param.sVal);
			break;
		case PARAM_FLOAT:
			paramTemplate = DXFILEOBJ_EffectParamFloats;
			num = 1;
			WRITE_DWORD(pbCur,num);
			WRITE_FLOAT(pbCur,param.fVal);
			break;
		case PARAM_FLOAT4:
			paramTemplate = DXFILEOBJ_EffectParamFloats;
			num = 4;
			WRITE_DWORD(pbCur,num);
			WRITE_VEC4(pbCur,param.vVal);
			break;
		default:
			Error("Shader type %d not found.\nThis is an internal error. Inform tim@helixcore.com",param.type);
	 }


	 CREATEOBJECT(paramTemplate);
	 ADDOBJECT();
	 return hr;
 }

 //-----------------------------------------------------------------------------
 // Writes out tex map
 //-----------------------------------------------------------------------------
 HRESULT
	 AddCustomTextureMap(
	 SSaveContext *psc,
	 CustomTextureMap& map,
	 LPDIRECTXFILEDATA pParent
	 )
 {
	 HRESULT hr = S_OK;
	 PBYTE pbData = NULL;
	 PBYTE pbCur;
	 pbData = pbCur = new BYTE[10240]; // 10K data should cover us


	 WRITE_DWORD(pbCur,map.type);
	 WRITESTRING(map.texVar);
	 WRITESTRING(map.transformVar);
	 WRITE_STRING(pbCur,map.name);
	 WRITE_FLOAT(pbCur,map.amount);
	 WRITE_FLOAT(pbCur,map.uTile);
	 WRITE_FLOAT(pbCur,map.vTile);
	 WRITE_FLOAT(pbCur,map.uOff);
	 WRITE_FLOAT(pbCur,map.vOff);

	 CREATEOBJECT(DXFILEOBJ_CustomTextureMap);
	 ADDOBJECT();

	 return hr;
 }

 //-----------------------------------------------------------------------------
 // Writes out entire material
 //-----------------------------------------------------------------------------
 HRESULT
	 AddCustomMaterial(
	 SSaveContext *psc,
	 CUSTOMD3DXMATERIAL *pMaterial,
	 LPDIRECTXFILEDATA pParent
	 )
 {
	 if(!pMaterial || !pMaterial->matname)
		 return S_OK;

	 // See if material has already been saved. If so, just save a reference to it
	 for(int i=0;i<psc->m_SavedMaterials.size();i++){
		 if(string(pMaterial->matname) == psc->m_SavedMaterials[i]){
			 pParent->AddDataReference(psc->m_stStrings.CreateNiceString(pMaterial->matname),NULL);
			 return S_OK;
		 }
	 }

	 HRESULT hr = S_OK;
	 PBYTE pbData = NULL;
	 PBYTE pbCur;
	 pbData = pbCur = new BYTE[10240]; // 10K data should cover us

	 WRITE_STRING(pbCur,pMaterial->matname);
	 WRITESTRING(pMaterial->shader);
	 WRITESTRING(pMaterial->technique);

	 // Create the object _WITH NAME_, so we can use references
	 LPDIRECTXFILEDATA pDataObject = NULL;
	 DWORD cbSize = pbCur - pbData;
	 DXASSERT(psc->m_pxofsave->CreateDataObject(DXFILEOBJ_CustomMaterial,
		 psc->m_stStrings.CreateNiceString(pMaterial->matname),
		 NULL,
		 cbSize,
		 pbData,
		 &pDataObject
		 )); 

	 // Insert our shader params...
	 for(int i=0;i<pMaterial->params.size();i++){
		 AddEffectParam(psc,pMaterial->params[i],pDataObject);
	 }

	 // Insert our maps...
	 for(int i=0;i<pMaterial->maps.size();i++)
		 AddCustomTextureMap(psc,pMaterial->maps[i],pDataObject);

	 // Add to root of file, then reference locally. That way we maintain a global
	 // material list so we can share references
	 psc->m_pxofsave->SaveData(pDataObject);
	 pParent->AddDataReference(psc->m_stStrings.CreateNiceString(pMaterial->matname),NULL);
			
	 //DXASSERT(pParent->AddDataObject(pDataObject));

	 delete []pbData;
	 RELEASE(pDataObject);

	 // Add to saved material list, so we avoid saving material again
	 psc->m_SavedMaterials.push_back(pMaterial->matname);

	 return hr;
 }

 //-----------------------------------------------------------------------------
 // Now export our *custom* material list to the parent frame node
 // (not to the above list/mesh node, which get eaten by the importer, grr)
 //-----------------------------------------------------------------------------
 HRESULT AddCustomMaterials(SSaveContext *psc,CUSTOMD3DXMATERIAL *rgMaterials,DWORD cMaterials,LPDIRECTXFILEDATA pFrameNode)
 {
	 for (int iCurMaterial = 0; iCurMaterial < cMaterials; iCurMaterial++)
	 {
		 DXASSERT(AddCustomMaterial(psc, &rgMaterials[iCurMaterial], pFrameNode));

	 } 
	 return S_OK;
 }

 
 HRESULT ExportAnimationHeader(SSaveContext *psc, LPDIRECTXFILEDATA *ppDataObjectNew){
	 HRESULT hr = S_OK;
	 PBYTE pbData = NULL;
	 PBYTE pbCur;
	 pbData = pbCur = new BYTE[10240]; // 10K data should cover us

	 DWORD bLoop = psc->m_bLoopingAnimationData;
	 WRITE_DWORD(pbCur,bLoop);

	 CREATEOBJECT(DXFILEOBJ_AnimationSettings);
	 *ppDataObjectNew = pDataObject;

	 delete []pbData;
	 return S_OK;
 }


 HRESULT ExportHeader(SSaveContext *psc, SceneProperties* mapData, LPDIRECTXFILEDATA *ppDataObjectNew){
	 HRESULT hr = S_OK;
	 PBYTE pbData = NULL;
	 PBYTE pbCur;
	 pbData = pbCur = new BYTE[10240]; // 10K data should cover us

	 WRITESTRING(string(mapData->skyworld));
	 D3DXCOLOR c(GetRValue(mapData->fogColor)/255.f,GetGValue(mapData->fogColor)/255.f,GetBValue(mapData->fogColor)/255.f,1);
	 WRITE_COLOR(pbCur,c);
	 WRITE_DWORD(pbCur,mapData->fogStart);
	 WRITE_DWORD(pbCur,mapData->clipPlane);
	 WRITESTRING(string(mapData->miniMap));
	 WRITE_FLOAT(pbCur,mapData->miniMapScale);
	 WRITESTRING(string(mapData->cubeMap));

	 // Put this global data in a frame, to be nice
	 //TCHAR* szName =  psc->m_stStrings.CreateNiceString("Scene Global");
	 //hr = psc->m_pxofsave->CreateDataObject(TID_D3DRMFrame,
	 //	szName,NULL,0,NULL,ppDataObjectNew);

	 CREATEOBJECT(DXFILEOBJ_SceneProperties);
	 *ppDataObjectNew = pDataObject;

	 delete []pbData;
	 return S_OK;
 }