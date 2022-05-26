//=========== Copyright (c) 2003, Tim Johnson. All rights reserved. ===========
//
// Shader Object
//
// A wrapper around the D3DX shaders files and a cached texture manager.
// Also sets many of the common constants that are used in vertex and 
// pixel shaders.
//
//
//=============================================================================

#include "stdafx.h"
#include <tchar.h>
#include <dxerr9.h>
#include "FnMap8.h"
#include <map>
#include <fstream>
#include "shlwapi.h"
#include "HDR.h"
#include "ShadowMapping.h"
#include "Profiler.h"
#include "PostProcess.h"

int COMPILE_FLAGS = D3DXSHADER_PARTIALPRECISION;//D3DXSHADER_DEBUG; //| D3DXSHADER_SKIPOPTIMIZATION )

bool m_UseConstantTable;

// Data used by shaders, etc
CNormalizerMap8		m_spNormalizerMap;
CFalloffMap			m_pFalloffMap(32);
Texture				atten2D;
float fSeconds = 0;

#define CHECKSET(v) if(v.handle == NULL){ CreateHandle(v); if(!v.handle) return; }

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void ShaderVar::Set(string newName, EditorVar::Type newType, DeleteType newDel, void* newData){ 
	type = newType;
	name = newName;
	del  = newDel;
	data = newData;
}
//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
ShaderVar::ShaderVar(string newName, EditorVar::Type newType, DeleteType newDel, void* newData){
	data   = NULL;
	handle = NULL; 
	allocated = false; 
	allocatedArray = false;
	Set(newName,newType,newDel,newData);
}
//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void ShaderVar::Destroy(){
	// If these bomb, you were setting a local var in a Set call, but this
	// class was initialized with a global var
	if(del == VAR_DELETEARRAY){
		SAFE_DELETE_ARRAY(data);
	}
	else if(del == VAR_DELETE){
		SAFE_DELETE(data);
	}
	else if(del == VAR_DELETETEXTURE)
		delete ((Texture*)data);
}


//-----------------------------------------------------------------------------
//
// Shader Manager
//
//-----------------------------------------------------------------------------
ShaderManager::ShaderManager()
{
}

void ShaderManager::Tick()
{
	fSeconds += GDeltaTime;
}


//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void ShaderManager::UpdateRenderSettings(){
	SetSharedInt("texFilter",RenderDevice::Instance()->GetTexFilter());
	SetSharedInt("maxAnisotropy",RenderDevice::Instance()->GetAnisotropyLevel());
	SetSharedBool("useSRGB",RenderDevice::Instance()->GetSRGB());
}

//-----------------------------------------------------------------------------
// TODO: Replace these with ID3DXEffectPool usage
//-----------------------------------------------------------------------------
void ShaderManager::SetSharedBool(string name, bool b){
	for (int i = 0; i < m_Shaders.size(); i++)
	{
		if(m_Shaders[i].shader->effect)
			m_Shaders[i].shader->effect->SetBool(name.c_str(),b);
	}

}

//-----------------------------------------------------------------------------
// TODO: Replace these with ID3DXEffectPool usage
//-----------------------------------------------------------------------------
void ShaderManager::SetSharedInt(string name, int number){
	for (int i = 0; i < m_Shaders.size(); i++)
	{
		if(m_Shaders[i].shader->effect)
			m_Shaders[i].shader->effect->SetInt(name.c_str(),number);
	}

}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void ShaderManager::ReloadShaders(){
	D3DInvalidate();
	D3DDelete();
	D3DRestore();
}
//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
ShaderManager* ShaderManager::Instance () 
{
	static ShaderManager inst;
	return &inst;
}


//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void ShaderManager::Shutdown()
{

}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void ShaderManager::Initialize()
{
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void ShaderManager::D3DInitialize(){
	m_UseConstantTable = Engine::Instance()->MainConfig->GetBool("UseConstantTable");
	// Even though textures are saved, we still need to refill them if the device
	// is lost
	StartMiniTimer();

	m_spNormalizerMap.Initialize();
	RegisterSharedTexture( "tNormalizer", &m_spNormalizerMap.texture );

	// Create procedural volume falloff texture
	if(!RenderDevice::Instance()->GetFFP()){
		m_pFalloffMap.Initialize();
		RegisterSharedTexture( "tPointFalloff", &m_pFalloffMap.texture );
	}
	
	//bool f = t.Load("LobbyCube.dds",0,0,1,1,0,TT_CUBE);
	//RegisterSharedTexture("tEnv",&t);

	// Create 1D light falloff texture
	//m_p1DFalloffMap.Initialize();
	//RegisterSharedTexture( "t1DPointFalloff", &m_p1DFalloffMap.texture );

	// Create Specular highlight texture
	//m_pSpecularMap.Initialize();
	//RegisterSharedTexture( "tSpecular", &m_pSpecularMap.texture );

	//atten2D.Load("Atten2D.tga");
	//RegisterSharedTexture( "tAtten2D",&atten2D);

	//createcube();
	//CShaderObject::RegisterSharedTexture( "tEnv",&cube);

	LogPrintf(_U("\tShader resource load took %f MS"),(float)StopMiniTimer());
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void ShaderManager::D3DRestore()
{
    //
    // Parse settings on restore in case user changed settings
    //
    string file = "ShaderSettings.ini";
    ResetCurrentDirectory();
    bool bRecompile = false;
    
    if(!FileExists(file))
    {
        m_Config.Create(file);
		m_Config.InsertRawLine("// This is a procedural file. Do not modify.");
        bRecompile = true;
    }

    m_Config.Load(file);

    
    // Pixel shader version changed
    if(m_Config.KeyExists("PixelShaderVersion") && m_Config.GetString("PixelShaderVersion") != RenderDevice::Instance()->PixelShaderString)
        bRecompile = true;
    // HDR Changed
    if(m_Config.KeyExists("HDR") && m_Config.GetBool("HDR") != RenderDevice::Instance()->GetHDR())
        bRecompile = true;
    // Compression CHanged
    if(m_Config.KeyExists("CompressNormalMaps") && m_Config.GetBool("CompressNormalMaps") != RenderDevice::Instance()->GetCompressNormalMaps())
        bRecompile = true;


    // Delete all compiled files, so we don't accidentally load a file that needs recompiling
    // (Imagine if user exists before all shaders have recompiled)
    if(bRecompile)
    {
        ResetCurrentDirectory();
        vector<string> files;
        enumerateFiles("..\\Shaders\\Compiled\\",files);
        for(int i=0;i<files.size();i++)
            DeleteFile((string("..\\Shaders\\Compiled\\")+files[i]).c_str());
    }
        

    m_Config.SetBool("CompressNormalMaps",RenderDevice::Instance()->GetCompressNormalMaps());
    m_Config.SetBool("HDR",RenderDevice::Instance()->GetHDR());
    m_Config.SetString("PixelShaderVersion",RenderDevice::Instance()->PixelShaderString);


    for (int i = 0; i < m_Shaders.size(); i++)
    {
		Shader* shader = m_Shaders[i].shader;
		if(!shader->effect)
		{
			SeriousWarning("Shader #%d's (%s) effect is NULL",i,shader->filename.c_str());
			return;
		}

		shader->effect->OnResetDevice();
		//shader->curTechniqueHandle = shader->GetTechnique(shader->curTechniqueName.c_str());
		//shader->SetTechnique(shader->curTechniqueHandle);
		shader->MapTables();
        SetUsedTextures(i);
    }

	UpdateRenderSettings();
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void ShaderManager::D3DInvalidate()
{
    for (int i = 0; i < m_Shaders.size(); i++)
    {
		m_Shaders[i].shader->effect->OnLostDevice();
    }

}

//-----------------------------------------------------------------------------
// TODO: This deletion is slow (~0.5 seconds per shader). How can we speed it up?
//-----------------------------------------------------------------------------
void ShaderManager::D3DDelete()
{
	for (int i = 0; i < m_Shaders.size(); i++)
    {
		SAFE_RELEASE(m_Shaders[i].shader->effect);
    }
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void ShaderManager::SetUsedTextures(DWORD effect)
{
    // Set shared textures
    for (int i = 0; i < m_sharedTextures.size(); i++)
    {
		if (m_sharedTextures[i].name!=""){
			D3DXHANDLE h = m_Shaders[effect].shader->effect->GetParameterByName(NULL,m_sharedTextures[i].name.c_str());
            if(h)
				m_Shaders[effect].shader->effect->SetTexture(h,m_sharedTextures[i].texture->GetTexture());
		}
    }
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
VOID ShaderManager::RegisterSharedTexture(CHAR* name, Texture* pTexture)
{
	ShaderTex st;
	st.name = name;
	st.texture = pTexture;

	m_sharedTextures.push_back(st);
}

//-----------------------------------------------------------------------------
// See if shader is loaded. Increments ref count!!!
//-----------------------------------------------------------------------------
Shader* ShaderManager::GetShader(string& sFile){
	FindMedia(sFile,"Shaders");
	sFile = StripPath(sFile);
	for(int i=0;i<m_Shaders.size();i++){
		if(AsLower(sFile) == AsLower(StripPath(m_Shaders[i].filename))){
			m_Shaders[i].shader->AddRef();
			return m_Shaders[i].shader;
		}
	}
	return NULL;
}


//=====================================================================
//
//
// Shader
//
//
//=====================================================================
Shader::Shader(){ 
    m_Flags = 0;
	m_RefCount = 1;
	begun = false;
	m_bOverridesEngineMultipass = false;
	for(int i=0;i<MAX_PASSES;i++){
		for(int j=0;j<MAX_TECHNIQUES;j++){
			m_CurPixelTable[j][i]	= NULL;
			m_CurVertexTable[j][i]	= NULL;
		}
	}

	// Common shader constants
	CC_WorldIMatrix.Set(":WorldInverse",EditorVar::MATRIX);
	CC_mWV.Set (":WORLDVIEW", EditorVar::MATRIX);
	CC_mP.Set  (":PROJECTION", EditorVar::MATRIX);
	CC_mWVP.Set(":WorldViewProjection",EditorVar::MATRIX);
	CC_mW.Set  (":World",  EditorVar::MATRIX);
	CC_mVI.Set  (":ViewInverse",  EditorVar::MATRIX);
	CC_mVP.Set (":ViewProjection", EditorVar::MATRIX);
	CC_fSeconds.Set(":TIME", EditorVar::FLOAT);
	// Light
	CC_LightPos.Set(":POSITION",EditorVar::FLOAT3);
	CC_LightRange.Set(":LightRange",EditorVar::FLOAT);
	CC_LightColor.Set("LightColor",EditorVar::FLOAT4);
	CC_LightDir.Set(":DIRECTION",EditorVar::FLOAT3);
	CC_LightFalloff.Set(":FALLOFF",EditorVar::FLOAT);
	CC_LightHotspot.Set(":UMBRA",EditorVar::FLOAT);
	CC_LightProjection.Set(":LIGHTPROJECTION",EditorVar::MATRIX);
	CC_LightProjectionMap.Set(":PROJECTIONMAP",EditorVar::TEXTURE);
	CC_LightOmniProjectionMap.Set(":OMNIPROJECTIONMAP",EditorVar::TEXTURE);
	CC_LightOmniProjectionMapBlur.Set(":OMNIPROJECTIONMAPBLUR",EditorVar::TEXTURE);

    // Light Arrays
    CC_NumLights.Set("NumLights",EditorVar::INT);
	CC_LightPosArray.Set(":POSITIONArray",EditorVar::FLOAT3);
	CC_LightRangeArray.Set(":LightRangeArray",EditorVar::FLOAT);
	CC_LightColorArray.Set("LightColorArray",EditorVar::FLOAT4);
	CC_LightDirArray.Set(":DIRECTIONArray",EditorVar::FLOAT3);
	CC_LightFalloffArray.Set(":FALLOFFArray",EditorVar::FLOAT);
	CC_LightHotspotArray.Set(":UMBRAArray",EditorVar::FLOAT);
	CC_LightProjectionArray.Set(":LIGHTPROJECTIONArray",EditorVar::MATRIX);
	CC_LightProjectionMapArray.Set(":PROJECTIONMAPArray",EditorVar::TEXTURE);
	CC_LightOmniProjectionMapArray.Set(":OMNIPROJECTIONMAPArray",EditorVar::TEXTURE);
	CC_LightOmniProjectionMapBlurArray.Set(":OMNIPROJECTIONMAPBLURArray",EditorVar::TEXTURE);
	// Misc
	CC_fogDensity.Set("FogDensity",EditorVar::FLOAT);
	CC_fogColor.Set("FogColor",EditorVar::FLOAT3);
	// HDR
	CC_bHDRBlend.Set("bHDRBlend",EditorVar::BOOL);
	CC_bHDRalphaTestPass.Set("bHDRalphaTestPass",EditorVar::BOOL);
	CC_tColorBuffer.Set("tColorBuffer",EditorVar::D3DTEXTURE);
	CC_fScaleBias.Set("fScaleBias",EditorVar::FLOAT4);
    CC_bDOF.Set("bDOF",EditorVar::INT);
	// Skinning
	CC_CurNumBones.Set("CurNumBones",EditorVar::INT);

    // PRT/Lighting
    CC_bHasPRT.Set("bHasPRT",EditorVar::BOOL);
    CC_bPerPixelPRT.Set("bPerPixelSH",EditorVar::BOOL);
    CC_bBakedLighting.Set("bBakedLighting",EditorVar::BOOL);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
Shader::~Shader(){
	assert(m_RefCount == 1);
	// Delete from manager array
	for(int i=0;i<ShaderManager::Instance()->m_Shaders.size();i++){
		if(this == ShaderManager::Instance()->m_Shaders[i].shader){
			ShaderManager::Instance()->m_Shaders.erase(ShaderManager::Instance()->m_Shaders.begin()+i);
			break;
		}
	}
	SAFE_RELEASE(effect);
}

//-----------------------------------------------------------------------------
// IUnknown-like ref system
//-----------------------------------------------------------------------------
void Shader::AddRef(){
	m_RefCount++;
}

//-----------------------------------------------------------------------------
// IUnknown-like ref system
//-----------------------------------------------------------------------------
DWORD Shader::Release(){
	if(m_RefCount == 1){ // Final instance
		delete this;
		return 0;
	}
	else{
		m_RefCount--;
		return m_RefCount;
	}
}
//-----------------------------------------------------------------------------
// Returns num passes it wants
//-----------------------------------------------------------------------------
UINT Shader::Begin(bool saveState)
{
	// Set some common vars here

	// OPTIMIZE NOTE: Really only needs to be done per shader switch, not per Begin()
	D3DXMATRIX matProj = *(D3DXMATRIX*)&(RenderWrap::GetProjection());
	D3DXMATRIX matView = *(D3DXMATRIX*)&(RenderWrap::GetView());
	D3DXMATRIX matViewProj=matProj, matViewInv;
	D3DXMatrixMultiply( &matViewProj, &matView, &matProj ); 
    D3DXMatrixInverse( &matViewInv, NULL, &matView );

	SetVar(CC_fSeconds,&fSeconds);
	SetVar(CC_mVI,&matViewInv);
	SetVar(CC_mVP,&matViewProj);
	SetVar(CC_mP,&matProj);

    

	begun = true;
	UINT passes;    
	DXASSERT(effect->Begin(&passes,saveState?0:D3DXFX_DONOTSAVESTATE ));

	if(m_UseConstantTable)
		effect->BeginPass(0);

	return passes;
}
//-----------------------------------------------------------------------------
// Prepares for pass
//-----------------------------------------------------------------------------
void Shader::BeginPass(UINT pass){
	curPass = pass;
	if(!m_UseConstantTable)
		(effect->BeginPass(pass));

    int dof = PostProcess::Instance()->m_bUsingDOF;
    SetVar(CC_bDOF,&dof);
}

//-----------------------------------------------------------------------------
// Ends pass
//-----------------------------------------------------------------------------
void Shader::EndPass(){
	DXASSERT(effect->EndPass());
}

//-----------------------------------------------------------------------------
// Ends technique
//-----------------------------------------------------------------------------
void Shader::End(){
	begun = false;
	effect->End();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool Shader::Load(const CHAR* filename){
	if(!Engine::Instance()->MainConfig->GetBool("ShaderPrecomputing"))
		COMPILE_FLAGS &= D3DXSHADER_NO_PRESHADER;

     // Version is for ps2+, if ps11 is set it would not compile ps2 shaders, even if not used
	 string pShader = RenderDevice::Instance()->PixelShaderString;
	 string vShader = RenderDevice::Instance()->VertexShaderString;

	 if(pShader.find("1_") != -1)
		 pShader = "ps_2_0";

	 D3DXMACRO macros[6];  
     string pName = ("VERSION_"+AsUpper(pShader));  
     string vName = ("VERSION_"+AsUpper(vShader)); 
	 string hdrName;
	 if(HDRSystem::Instance()->m_bEnabled)
		hdrName = "HDR_ENABLED";
	 else
		hdrName = "HDR_DISABLED";

     // We want targets to only be ps2.x, so force off ps3 (but still use above)
     if(pShader.find("3_") != -1)
         pShader = "ps_2_a";
     if(vShader.find("3_") != -1)
         vShader = "vs_2_0";

     macros[0].Name = pName.c_str();  
     macros[0].Definition = "";  
     macros[1].Name = vName.c_str();  
     macros[1].Definition = "";  
     macros[2].Name = "PS_COMPILER_TARGET";  
     macros[2].Definition = pShader.c_str();  
     macros[3].Name = "VS_COMPILER_TARGET";  
     macros[3].Definition = vShader.c_str();
	 macros[4].Name = hdrName.c_str();
     macros[4].Definition = "";
     macros[5].Name = NULL;          // NULL-Terminate  
     macros[5].Definition = NULL; // NULL-Terminate   

	ShaderManager* manager = ShaderManager::Instance();

	if(!RenderWrap::dev)
		Error(_U("Effect load attempted before device was ready"));

	string sFile = filename;

	// See if the uncompiled file exists. We need either this or the fxc file
	bool fxExists = FindMedia(sFile,"Shaders");

	// Get the filename of the compiled effect. If it doesn't exist, then make
	// the filename what it will be once created
	string sFileCompiled;
	if(fxExists) sFileCompiled = sFile;
	else sFileCompiled = filename;

	// Insert /Compiled/ into end of path.
	int ls = FindLastSlash(sFileCompiled);
	if(ls!=-1)
		sFileCompiled = sFileCompiled.substr(0,ls) + _U("\\Compiled") + sFileCompiled.substr(ls);
	else
		sFileCompiled = _U("Compiled\\") + sFileCompiled;

	if(!PathIsDirectory(sFileCompiled.substr(0,FindLastSlash(sFileCompiled)).c_str())){
		CreateDirectory(sFileCompiled.substr(0,FindLastSlash(sFileCompiled)).c_str(),NULL);
	}

	sFileCompiled = sFileCompiled.substr(0,sFileCompiled.find_last_of(_U(".")));
	sFileCompiled += _U(".fxc");
	string temp = sFileCompiled;
	bool fxcExists = FindMedia(sFileCompiled,"Shaders");
	if(fxExists && !fxcExists){
		// Change sFileCompiled back to where we expect to find it once created
		// (because FindResource mangled it)
		sFileCompiled = temp;
	}

	// If we don't have either the FX file or a compiled version, throw an error
	if(!fxExists && !fxcExists){
		SeriousWarning(_U("Effect file '%s' or its compiled version couldn't be found."),filename);
		return false;
	}

	bool recompile = true;

	// If we have a compiled version, check to see if it's up to date
	// If it is, use it. If not, recompile it.
	if(fxcExists){
		recompile = false;

		if(fxExists){
			WIN32_FILE_ATTRIBUTE_DATA fxCompiled;
			GetFileAttributesEx( sFileCompiled.c_str(), GetFileExInfoStandard, &fxCompiled);

			int ls = FindLastSlash(sFile);
			string path = sFile.substr(0,ls+1);

			vector<string> list;
			enumerateFiles(path.c_str(),list,0,_U(".fx"));

			// See if any of the .fx files in this dir are newer, if so, rebuild
			WIN32_FILE_ATTRIBUTE_DATA fxUncompiled;
			for(int i=0;i<list.size();i++){
				string file = path+list[i];
				GetFileAttributesEx( file.c_str(), GetFileExInfoStandard, &fxUncompiled);
				if(CompareFileTime(&fxCompiled.ftLastWriteTime,&fxUncompiled.ftLastWriteTime) == -1)
					recompile = true; // Compiled file is older
			}

		}
	}

    StartMiniTimer();

	HRESULT hr;
	LPD3DXBUFFER errorBuffer;

    //if(Engine::Instance()->MainConfig->GetBool("ForceShaderRecompile"))
      //  sFileCompiled = sFile;

    if(Engine::Instance()->MainConfig->GetBool("ForceShaderRecompile") || recompile)
	{
		LPD3DXEFFECTCOMPILER effectCompiler;
		LPD3DXBUFFER effectBuffer;

		// Compile the effect, inside a loop to give user chance
		// to fix broken shader
        while(true)
        {
            // Create an effect compiler
            if(FAILED(hr=D3DXCreateEffectCompilerFromFile(sFile.c_str(),
                macros,NULL,COMPILE_FLAGS,&effectCompiler,&errorBuffer)))
            {
                string error;
                error += _U("Error creating effect compiler from file, ");
                error += sFile + _U("\n");
                if(errorBuffer && errorBuffer->GetBufferSize() > 1){
                    error += (CHAR*) errorBuffer->GetBufferPointer();
                    SAFE_RELEASE(errorBuffer);
                }

                error += "\nClick 'Retry' to recompile again, or 'Cancel' to abort";

                if(IDCANCEL == MessageBox(Engine::Instance()->hWnd,error.c_str(),"Error Compiling Shader",MB_RETRYCANCEL))
                {
                    effect = NULL;
                    return false;
                }
                else
                    continue;
            }

            //
            // Set Literals
            //
            bool isLiteral = !Engine::Instance()->MainConfig->GetBool("ForceDynamicVars");
            // NOTE: Forcing HDR blending off for cards that don't need it. However this'll break depth of field on
            // these cards, as they need the alpha=z trick!!!
            bool hdrBlending = RenderDevice::Instance()->GetHDR() && RenderDevice::Instance()->PixelShaderVersion <= 2;
            bool compress    = RenderDevice::Instance()->GetCompressNormalMaps()&&RenderDevice::Instance()->PixelShaderVersion>=2;
            

            effectCompiler->SetLiteral("bHDRBlend",!hdrBlending);
            effectCompiler->SetLiteral("SwizzleBump",isLiteral);
            effectCompiler->SetLiteral("CalcBumpZ",isLiteral);

            effectCompiler->SetBool("bHDRBlend",hdrBlending);
            effectCompiler->SetBool("CalcBumpZ",compress);
            effectCompiler->SetBool("SwizzleBump",compress && !RenderDevice::Instance()->Supports3DC());

			if(FAILED(hr=effectCompiler->CompileEffect(COMPILE_FLAGS,&effectBuffer,&errorBuffer)))
            {
				SAFE_RELEASE(effectCompiler);

				string error;
				error += _U("Error compiling effect\n");
				if(errorBuffer && errorBuffer->GetBufferSize() > 1){
					error += (CHAR*) errorBuffer->GetBufferPointer();
					SAFE_RELEASE(errorBuffer);
				}

				error += "\nClick 'Retry' to recompile again, or 'Cancel' to abort";

				if(IDCANCEL == MessageBox(Engine::Instance()->hWnd,error.c_str(),"Error Compiling Shader",MB_RETRYCANCEL))
				{
					effect = NULL;
					return false;
				}
			}
			else
				break;
		}

		// Write the effect to file;
		ofstream fxc;
		fxc.open(sFileCompiled.c_str(),ios::binary);

		fxc.write((const char*)effectBuffer->GetBufferPointer(),effectBuffer->GetBufferSize());

		fxc.close();

		SAFE_RELEASE(effectBuffer);
		SAFE_RELEASE(effectCompiler);
	}

	// Load compiled shaders file (basically just a file read into memory)
	if ( FAILED (hr = D3DXCreateEffectFromFile( RenderWrap::dev, sFileCompiled.c_str(),0,0,COMPILE_FLAGS,0, &effect, &errorBuffer) ) )
	{
		string error;
		error += _U("Error loading compiled effect, ");
		error += sFileCompiled + _U("\n");
		if(errorBuffer && errorBuffer->GetBufferSize() > 1){
			error += (CHAR*) errorBuffer->GetBufferPointer();
			SAFE_RELEASE(errorBuffer);
		}
		Error(error.c_str());
	}

	this->filename = sFile;
	assert(effect);

	manager->m_Shaders.push_back(ShaderManager::ShaderInfo(this,sFile));

    ProcessGlobals();

	// FIXME: Hacky
	ShaderManager::Instance()->SetUsedTextures(manager->m_Shaders.size()-1);
	ShaderManager::Instance()->UpdateRenderSettings();
	MapTables();

    Profiler::Get()->ShaderLoadSecs += StopMiniTimer()/1000.f;
	return true;
}



//-----------------------------------------------------------------------------
// Gets the technique, and handles the _PS flag for the required target
//-----------------------------------------------------------------------------
D3DXHANDLE Shader::GetTechnique(string technique){
	RenderDevice* rdev = RenderDevice::Instance();

	
	// Strip tag if already there, we will figure out the tag afterwards
	if(technique.find("_PS11") != -1)
		technique.erase(technique.find("_PS11"),5);
	if(technique.find("_PS20") != -1)
		technique.erase(technique.find("_PS20"),5);
	if(technique.find("_PS30") != -1)
		technique.erase(technique.find("_PS30"),5);

	// Add the tag based on pixel shader version
	string tech = technique;
	if(rdev->PixelShaderVersion < 1.1)
		tech = tech + "_FFP";
	else if(rdev->PixelShaderVersion < 2)
		tech = tech + "_PS11";
	else if(rdev->PixelShaderVersion < 3)
		tech = tech + "_PS20";
	else
		tech = tech + "_PS30";

	D3DXHANDLE techID = effect->GetTechniqueByName(tech.c_str());
	// Tag not found. Perhaps there is no version for this card? Try a lower version
	if(techID == NULL && rdev->PixelShaderVersion >= 2)
		techID = effect->GetTechniqueByName((technique+"_PS20").c_str());
	if(techID == NULL && rdev->PixelShaderVersion >= 1.1)
		techID = effect->GetTechniqueByName((technique+"_PS11").c_str());
	if(techID == NULL)
		techID = effect->GetTechniqueByName((technique+"_FFP").c_str());

	//if(techID == NULL)
	//	Error("Technique %s does not exist in %s",name,filename.c_str());
#if _DEBUG
	if(techID)
		effect->ValidateTechnique(techID);
#endif

	return techID;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Shader::SetTechnique(const D3DXHANDLE technique)
{
	if(!technique)
		return;//Error("Shader tried to set NULL Technique!");

	curTechniqueHandle = technique;

	D3DXTECHNIQUE_DESC desc;
	if(SUCCEEDED(effect->GetTechniqueDesc(curTechniqueHandle,&desc))){
		curTechniqueName = desc.Name;
	}
	else 
		Error("GetTechniqueDesc failed! Handle is %x",curTechniqueHandle);

	HRESULT hr;
	if( FAILED( hr = effect->SetTechnique(curTechniqueHandle) ) ){
		Error("SetTechnique failed for '%s' (Handle %d)",desc.Name,curTechniqueHandle);
	}

	D3DXTECHNIQUE_DESC d;
	effect->GetTechniqueDesc(curTechniqueHandle,&d);
    m_bOverridesEngineMultipass = false;
	if(d.Annotations){
		D3DXHANDLE annotation = effect->GetAnnotationByName(curTechniqueHandle,"OverrideEngineMultipass");
		if(annotation){
			m_bOverridesEngineMultipass = true;
		}
	}

	// Figure out index for technique
	// TODO: Optimize!
	if(m_UseConstantTable){
		D3DXEFFECT_DESC desc;
		effect->GetDesc(&desc);
		for(int i=0;i<desc.Techniques;i++){
			if(effect->GetTechnique(i) == curTechniqueHandle){
				m_CurTechniqueIndex = i;
				return;
			}
		}
		Error("Impossible");
	}
}

//-----------------------------------------------------------------------------
// Called only when a Set function finds an unset invalid ShaderVar
// _NO NEED TO CALL THIS EXPLICITLY_
// Gets a var using the ID3DXEffects system
//-----------------------------------------------------------------------------
void Shader::CreateHandle(ShaderVar& var)
{
	// Gets a var using the D3D9 system
	// Searches the vertex and pixel shader constant tables to get a param constant index for
	// super-fast access
	if(m_UseConstantTable){
		for(int i=0;i<MAX_PASSES;i++){
			if(var.name[0] == ':')
				var.handle = effect->GetParameterBySemantic(NULL,var.name.substr(1).c_str());
			else
				var.handle = effect->GetParameterByName(NULL,var.name.c_str());

			D3DXPARAMETER_DESC desc1;
			effect->GetParameterDesc(var.handle,&desc1);

			D3DXEFFECT_DESC desc;
			effect->GetDesc(&desc);

			// Store a handle for *each* technique
			for(int j=0;j<desc.Techniques;j++){
				D3DXHANDLE vHandle = 0;
				D3DXHANDLE pHandle = 0;
				if(m_CurVertexTable[j][i])
					vHandle = m_CurVertexTable[j][i]->GetConstantByName(NULL,desc1.Name);
				if(m_CurPixelTable[j][i])
					pHandle = m_CurPixelTable[j][i]->GetConstantByName(NULL,desc1.Name);

				ID3DXConstantTable *table;
				D3DXHANDLE handle;
				if(vHandle)		 {
					table = m_CurVertexTable[j][i]; handle = vHandle; 
				}
				else if(pHandle) {
					table = m_CurPixelTable[j][i];  handle = pHandle; 
				}
				else {
					var.Reg[j][i] = (D3DXHANDLE)9999;
					continue;
				}

				//D3DXCONSTANT_DESC desc;
				//table->GetConstantDesc(handle,&desc,0);
				if(pHandle)
					var.Reg[j][i]			= pHandle;//desc.RegisterIndex;
				else
					var.Reg[j][i]			= vHandle;
				var.PixelConst[j][i]	= pHandle?true:false;
			}
		}
	}
	else{
		if(var.name[0] == ':')
			var.handle = effect->GetParameterBySemantic(NULL,var.name.substr(1).c_str());
		else
			var.handle = effect->GetParameterByName(NULL,var.name.c_str());
		
		if(var.handle == NULL){
			//Warning(_U("The variable '%s' does not exist in the file '%s', but the game tried to set it. This is a programmer error\n"),var.name.c_str(),filename.c_str());
		}
	}

}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
string Shader::GetString(D3DXHANDLE handle)
{
    LPCSTR pStr;
	effect->GetString(handle, (LPCSTR*)&pStr);
    return pStr;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
int Shader::GetNumTechniques()
{
    D3DXEFFECT_DESC desc;
    effect->GetDesc(&desc);
    return desc.Techniques;
}


//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void Shader::CommitChanges()
{
	if(!m_UseConstantTable)
		effect->CommitChanges();
}

//-----------------------------------------------------------------------------
// Get the vertex and pixel constant tables for all techniques and passes
//-----------------------------------------------------------------------------
void Shader::MapTables()
{
	if(!m_UseConstantTable)
		return;

	D3DXEFFECT_DESC desc;
	effect->GetDesc(&desc);

	for(int i=0;i<desc.Techniques;i++){
		effect->SetTechnique(effect->GetTechnique(i));
		int passes = Begin();
		for(int j=0;j<passes;j++){
			assert(j < MAX_PASSES);
			BeginPass(j);
			LPDIRECT3DVERTEXSHADER9 vs = 0;
			LPDIRECT3DPIXELSHADER9 ps = 0;
			RenderWrap::dev->GetVertexShader(&vs);
			RenderWrap::dev->GetPixelShader(&ps);
			if(ps){
				DWORD psData[4012];
				UINT dataSize = 4012;
				ps->GetFunction(psData,&dataSize); 
				D3DXGetShaderConstantTable(psData,&m_CurPixelTable[i][j]);
			}

			if(vs){
				DWORD vsData[4012];
				UINT dataSize = 4012;
				vs->GetFunction(vsData,&dataSize);
				D3DXGetShaderConstantTable(vsData,&m_CurVertexTable[i][j]);
			}

			EndPass();
		}
		End();
	}
}

//-----------------------------------------------------------------------------
// Sets a pixel or vertex shader constant for the current effect
//-----------------------------------------------------------------------------
VOID Shader::SetVar(ShaderVar& c, void* data){
	void* oldData = c.data;
	c.data = data;
	SetVar(c);
	c.del = ShaderVar::VAR_DONTDELETE; // HACK
}

void Blah(){
	void* v = 0;
	v=v;
}

#define Apply(X) { \
	if(bPixel){ \
		if(FAILED(m_CurPixelTable[m_CurTechniqueIndex][curPass]->X)) { \
			Blah(); \
		} \
	} \
	else{ \
		if(FAILED(m_CurVertexTable[m_CurTechniqueIndex][curPass]->X)) { \
			Blah(); \
		} \
	} \
}

//-----------------------------------------------------------------------------
// Sets a pixel or vertex shader constant for the current effect
//-----------------------------------------------------------------------------
VOID Shader::SetVar(ShaderVar& c){
	if(c.data == NULL)
		return;

	CHECKSET(c);
	if(begun && m_UseConstantTable/* && c.Reg[curPass]*/){
		bool bPixel = c.PixelConst[m_CurTechniqueIndex][curPass];
		D3DXHANDLE reg	= c.Reg[m_CurTechniqueIndex][curPass];

		if(reg == (D3DXHANDLE)9999) // Invalid reg, param doesn't map to current technique
			return;

		switch(c.type){
			case EditorVar::FLOAT3:{
				Apply(SetFloatArray(RenderWrap::dev,reg,(float*)c.data,3));
				break;
				}
			case EditorVar::FLOAT4:
				Apply(SetFloatArray(RenderWrap::dev,reg,(float*)c.data,4));
				break;
			case EditorVar::BOOL:
				{
				BOOL b = *(bool*)c.data;
				Apply(SetBool(RenderWrap::dev,reg,b));
				break;
				}
			case EditorVar::INT:
				Apply(SetInt(RenderWrap::dev,reg,*(int*)c.data));
				break;
			case EditorVar::FLOAT:
				Apply(SetFloat(RenderWrap::dev,reg,*(float*)c.data));
				break;
			case EditorVar::TEXTURE:
				if(((Texture*)c.data)->IsValid())
					RenderWrap::dev->SetTexture(0,((Texture*)c.data)->GetTexture());//effect->SetTexture(c.handle,((Texture*)c.data)->GetTexture());
				break;
			case EditorVar::D3DTEXTURE:
				RenderWrap::dev->SetTexture(0,((Texture*)c.data)->GetTexture());
				//effect->SetTexture(c.handle,(PDIRECT3DTEXTURE9)c.data);
				break;
			case EditorVar::MATRIX:
				Apply(SetMatrix(RenderWrap::dev,reg,(D3DXMATRIX*)c.data));
				break;
		}
	}
	else if(!begun || !m_UseConstantTable)
	{
		switch(c.type){
			case EditorVar::FLOAT3:{
				D3DXVECTOR3* v = (D3DXVECTOR3*)c.data;
				effect->SetVector(c.handle,&D3DXVECTOR4(v->x,v->y,v->z,1));
				break;
							}
			case EditorVar::FLOAT4:
				effect->SetVector(c.handle,(D3DXVECTOR4*)c.data);
				break;
			case EditorVar::BOOL:
				effect->SetBool(c.handle,*(bool*)c.data);
				break;
			case EditorVar::INT:
				effect->SetInt(c.handle,*(int*)c.data);
				break;
			case EditorVar::FLOAT:
				effect->SetFloat(c.handle,*(float*)c.data);
				break;
			case EditorVar::TEXTURE:
				if(((Texture*)c.data)->IsValid())
					effect->SetTexture(c.handle,((Texture*)c.data)->GetTexture());
				break;
			case EditorVar::D3DTEXTURE:
				effect->SetTexture(c.handle,(PDIRECT3DTEXTURE9)c.data);
				break;
			case EditorVar::MATRIX:
				effect->SetMatrix(c.handle,(D3DXMATRIX*)c.data);
				break;
		}
	}
}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
VOID Shader::SetWorld(const Matrix& world){
	// Warp matrix allows us to do cool things like water reflections or acid trips
    //D3DXMatrixMultiply((D3DXMATRIX*)&curWorldMat,(D3DXMATRIX*)&world,(D3DXMATRIX*)&ShaderManager::Instance()->GetWorldWarp());
	curWorldMat = world * ShaderManager::Instance()->GetWorldWarp();

	RenderWrap::SetWorld(curWorldMat);
	SetCommonShaderConstants();
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
VOID Shader::SetFog(float Density, FloatColor color){
	CC_fogDensity.data = &Density;
	CC_fogColor.data = &color;
	SetVar(CC_fogDensity);
	SetVar(CC_fogColor);
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
VOID Shader::SetCommonShaderConstants() 
{
    // Set up vertex shader constants
	D3DXMATRIX matWorld = *(D3DXMATRIX*)&curWorldMat,
		matView = *(D3DXMATRIX*)&(RenderWrap::GetView()),
		matProj = *(D3DXMATRIX*)&(RenderWrap::GetProjection());
    D3DXMATRIX matWorldView, matWorldViewProj;
    D3DXMATRIX matWorldInv, matViewInv;

    //matWorldView = matWorld * matView;
    //matWorldViewProj = matWorldView * matProj;
    D3DXMatrixMultiply( &matWorldView, &matWorld, &matView ); 
    D3DXMatrixMultiply( &matWorldViewProj, &matWorldView, &matProj ); 
    D3DXMatrixInverse( &matWorldInv, NULL, &matWorld );
	curMatWorldInv = *(Matrix*)&matWorldInv;

	SetVar(CC_mWVP,&matWorldViewProj);
	SetVar(CC_WorldIMatrix,&matWorldInv);
	SetVar(CC_mW,&matWorld);
	SetVar(CC_mWV,&matWorldView);
}


VOID Shader::SetSkinning(int iInfluences, int iMatrices, D3DXMATRIX* matrices){
	SetVar(CC_CurNumBones,&iInfluences);
    effect->SetBool("HasSkinning",iInfluences>0);
	if(iInfluences > 0){
		effect->SetMatrixArray( "mWorldMatrixArray", matrices, iMatrices);
	}
}

VOID Shader::UnbindHDRTarget(){
	SetVar(CC_tColorBuffer,NULL);
}

VOID Shader::SetMeshParams(bool hasPRT, bool perPixel, bool bakedLighting)
{
    SetVar(CC_bHasPRT,&hasPRT);
    SetVar(CC_bBakedLighting,&bakedLighting);
    SetVar(CC_bPerPixelPRT,&perPixel);
}

VOID Shader::SetHDR(bool enable, bool blend,bool alphaTestPass){
	if(!HDRSystem::Instance()->m_bEnabled)
		return; // Never set vars if HDR is disabled

	SetVar(CC_bHDRBlend,&blend);
	SetVar(CC_tColorBuffer,HDRSystem::Instance()->GetColorBuffer());
	if(enable)
	{
		SetVar(CC_fScaleBias,&D3DXVECTOR4(0.5f + 0.5f/RenderDevice::Instance()->GetViewportX(),0.5f + 0.5f/RenderDevice::Instance()->GetViewportY(),RenderDevice::Instance()->GetViewportX(),RenderDevice::Instance()->GetViewportY()));
		SetVar(CC_bHDRalphaTestPass,&alphaTestPass);
	}
}

VOID Shader::SetColorBuffer(PDIRECT3DTEXTURE9 colorBufferTexture)
{
	SetVar(CC_tColorBuffer,colorBufferTexture);
	SetVar(CC_fScaleBias,&D3DXVECTOR4(0.5f + 0.5f/RenderDevice::Instance()->GetViewportX(),0.5f + 0.5f/RenderDevice::Instance()->GetViewportY(),RenderDevice::Instance()->GetViewportX(),RenderDevice::Instance()->GetViewportY()));
}


VOID Shader::SetUnlit(){
	// Calculate Light constants
	// Light pos
	SetVar(CC_LightPos, &Vector(100000,0,0));
	SetVar(CC_LightColor,&FloatColor(0,0,0));
}

VOID Shader::SetLightShaderConstants(vector<Light*>& lights)
{
    int numLights = lights.size();
    SetVar(CC_NumLights,&numLights);

    if(lights.size())// == 1)
    {
        Light* pLight = lights[0];
	    if(pLight->m_Type == LIGHT_SPOT){
		    // Light dir
		    SetVar(CC_LightDir,&pLight->curState.Direction);
		    // Falloff
		    float falloff = cos(DEG2RAD(pLight->curState.Spot_Falloff)/2.f);
		    SetVar(CC_LightFalloff,&falloff);
		    // Hotspot
		    float spot = cosf(DEG2RAD(pLight->curState.Spot_Size)/2.f);
		    SetVar(CC_LightHotspot,&spot);
		    // Projection
		    SetVar(CC_LightProjection,&pLight->GetViewProjection());
		    // (Dynamic) Shadow Map
		    if(pLight->m_bShadowProjector && pLight->m_ShadowMap)
            {
			    effect->SetFloat("fShadowMapSize",pLight->m_ShadowMap->GetSize());
			    CC_LightProjectionMap.type = EditorVar::D3DTEXTURE;
			    SetVar(CC_LightProjectionMap,pLight->m_ShadowMap->GetMap());

               // effect->SetTexture("Jitter",pLight->m_ShadowMap->GetJitterMap());

                // Set special texture matrix for shadow mapping
                float fOffsetX = 0.5f + (0.5f / (float)pLight->m_ShadowMap->GetSize()); // Width
                float fOffsetY = 0.5f + (0.5f / (float)pLight->m_ShadowMap->GetSize()); // Height
                unsigned int range = 1;//pLight->GetCurrentState().Range;//1;            //note different scale in DX9!
                float fBias    = -0.001f * (float)range;
               // float fBias    = 0.0f;
                D3DXMATRIX texScaleBiasMat( 0.5f,     0.0f,     0.0f,         0.0f,
                    0.0f,    -0.5f,     0.0f,         0.0f,
                    0.0f,     0.0f,     (float)range, 0.0f,
                    fOffsetX, fOffsetY, fBias,        1.0f );

                //D3DXMATRIX texMat = worldMat * m_LightViewProj * texScaleBiasMat
                effect->SetMatrix("mTexProj",&texScaleBiasMat);

		    }
		    else{
		    // Light Map
			    CC_LightProjectionMap.type = EditorVar::TEXTURE;
			    SetVar(CC_LightProjectionMap,&pLight->m_tProjectionMap);
		    }
    		
	    }
	    if(pLight->m_Type == LIGHT_OMNI_PROJ)
	    {
		    // View
		    SetVar(CC_LightProjection,&pLight->GetView());

		    // Omni projection cube map
		    CC_LightOmniProjectionMap.type = EditorVar::TEXTURE;
		    SetVar(CC_LightOmniProjectionMap,&pLight->m_tOmniProjCubeMap);

		    // Omni projection cube map blurred
		    CC_LightOmniProjectionMapBlur.type = EditorVar::TEXTURE;
		    SetVar(CC_LightOmniProjectionMapBlur,&pLight->m_tOmniProjCubeMapBlur);
	    }
	    if(pLight->m_Type == LIGHT_DIR){
		    // Light dir
		    SetVar(CC_LightDir,&pLight->curState.Direction);
	    }
	    if(pLight->m_Type == LIGHT_OMNI){

	    }

        // Range
	    SetVar(CC_LightRange,&pLight->curState.Range);
	    // Light pos
	    SetVar(CC_LightPos, &pLight->curState.Position);
	    // Light color
	    FloatColor col = *(FloatColor*)&pLight->curState.Diffuse*pLight->curState.Intensity;
	    col.a = 1; // Keep alpha to a default
	    SetVar(CC_LightColor,&col);
    }
    // DEMO HACK!!: Only enable multi-light support for Plants.fx
    if(lights.size()>1 && filename.find("lants") != -1)
    {

        CHECKSET(CC_LightRangeArray);
        CHECKSET(CC_LightColorArray);
        CHECKSET(CC_LightPosArray);
        CHECKSET(CC_LightDirArray);
        CHECKSET(CC_LightFalloffArray);
        CHECKSET(CC_LightHotspotArray);

        // Ranges
        float* ranges = new float[numLights];
        for(int i=0;i<numLights;i++)
            ranges[i] = lights[i]->curState.Range;
        effect->SetFloatArray(CC_LightRangeArray.handle,ranges,numLights);
        SAFE_DELETE_ARRAY(ranges);
        
        // Positions
        D3DXVECTOR4* positions = new D3DXVECTOR4[numLights];
        for(int i=0;i<numLights;i++)
            positions[i] = D3DXVECTOR4(lights[i]->curState.Position.x,lights[i]->curState.Position.y,lights[i]->curState.Position.z,1);
        effect->SetVectorArray(CC_LightPosArray.handle,positions,numLights);
        SAFE_DELETE_ARRAY(positions);

        // Colors
        FloatColor* colors = new FloatColor[numLights];
        for(int i=0;i<numLights;i++)
            colors[i] = *(FloatColor*)&lights[i]->curState.Diffuse*lights[i]->curState.Intensity;
        effect->SetVectorArray(CC_LightColorArray.handle,(D3DXVECTOR4*)colors,numLights);
        SAFE_DELETE_ARRAY(colors);

        // Light dir
        D3DXVECTOR4* dirs = new D3DXVECTOR4[numLights];
        for(int i=0;i<numLights;i++)
            dirs[i] = D3DXVECTOR4(lights[i]->curState.Direction.x,lights[i]->curState.Direction.y,lights[i]->curState.Direction.z,1);
        effect->SetVectorArray(CC_LightDirArray.handle,(D3DXVECTOR4*)dirs,numLights);
        SAFE_DELETE_ARRAY(dirs);

        // Falloff
        float* falloffs = new float[numLights];
        for(int i=0;i<numLights;i++)
            falloffs[i] = cos(DEG2RAD(lights[i]->curState.Spot_Falloff)/2.f);
        effect->SetFloatArray(CC_LightFalloffArray.handle,falloffs,numLights);
        SAFE_DELETE_ARRAY(falloffs);

        // Falloff
        float* hotspots = new float[numLights];
        for(int i=0;i<numLights;i++)
            hotspots[i] = cos(DEG2RAD(lights[i]->curState.Spot_Size)/2.f);
        effect->SetFloatArray(CC_LightHotspotArray.handle,hotspots,numLights);
        SAFE_DELETE_ARRAY(hotspots);
    }
}





