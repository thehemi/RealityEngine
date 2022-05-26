//=========== Copyright (c) 2003, Tim Johnson. All rights reserved. ===========
// Material:
//
// Materials & graphical properties
//
// Materials can have sounds and effects associated with them. 
// This is at the game level - see MaterialHelper.h
//
//
// New Shader Logic:
// -
//
//=============================================================================
#include "stdafx.h"
#include "Material.h"
#include "RenderWrap.h"
#include "Serializers\XMLSerializer.h"


//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
Technique::Technique()
{ 
    MaxLights = 1; 
    PerPixel = true; 
    PRT = false; 
    LightTypes = -1; 
    PixelShaderVersion = 1.1f; 
    ShadowProjector = false; 
    LightMapping = false;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
Material::Material(string name){
	m_Name			= name;
	m_Shader		= NULL;
	m_ParamBlock	= NULL;
	m_RefCount		= 1;
	// Try to set our type base on the name
	m_Type = MaterialManager::Instance()->GetType(name);
	MaterialManager::Instance()->m_Materials.push_back(this);
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
Material::Material(){
	m_Shader		= NULL;
	m_ParamBlock	= NULL;
	m_RefCount		= 1;
	MaterialManager::Instance()->m_Materials.push_back(this);
}

//-----------------------------------------------------------------------------
// IUnknown-like ref system
//-----------------------------------------------------------------------------
void Material::AddRef(){
	m_RefCount++;
	MaterialManager::Instance()->m_TotalRefs++;
}

//-----------------------------------------------------------------------------
// IUnknown-like ref system
//-----------------------------------------------------------------------------
DWORD Material::Release(){
	if(m_RefCount == 1){ // Final instance
		delete this;
		return 0;
	}
	else{
		MaterialManager::Instance()->m_TotalRefs--;
		m_RefCount--;
		return m_RefCount;
	}
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
Technique*		Material::GetTechnique(LightType tech, bool bPRTEnabled, bool shadowProjector, bool lightMapping)
{
    for(int i=0;i<m_Techniques.size();i++)
    {
        Technique& t = m_Techniques[i];
        if(t.LightTypes & tech && (t.PRT || !bPRTEnabled) && (t.PerPixel || (bPRTEnabled||lightMapping)) 
            && (t.LightMapping || !lightMapping) && (t.ShadowProjector == shadowProjector))
            return &t;
    }
    if(!m_Techniques.size())
        Error("Material %s has no valid techniques! '%s' may no longer exist in the shader '%s'",m_Name.c_str(),m_Token.c_str(),m_ShaderName.c_str());

    // If no technique, object won't be rendered
    return 0;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
Material::~Material(){
	if(m_RefCount != 1)
		Error("Material '%s' was deleted with a ref count above 1 (%d). Please use the Release() method instead.",m_Name.c_str(),m_RefCount);

	ResourceManager::Instance()->Remove(this);
	SAFE_DELETE_VECTOR(m_Parameters);
	SAFE_RELEASE(m_Shader);
	vector_erase(MaterialManager::Instance()->m_Materials,this);
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
ShaderVar* Material::FindVar(string name){
	for(int i=0;i<m_Parameters.size();i++){
		if(m_Parameters[i]->name == name)
			return m_Parameters[i];
	}
	return NULL;
}


//-----------------------------------------------------------------------------
// 
//
//-----------------------------------------------------------------------------
void Material::UpdateStates()
{
	m_Shader->GetEffect()->BeginParameterBlock();
    bool foundAnything = false;

	// Set all parameters
	for(int i=0;i<m_Parameters.size();i++)
    {
        if(m_Parameters[i]->GetHandle() == NULL)
            m_Shader->CreateHandle(*m_Parameters[i]);

        // Erase non-tweakables if they somehow got on our list
        if(!Tweakable(m_Parameters[i]->GetHandle()))
        {
            m_Parameters.erase(m_Parameters.begin() + i);
            i--;
            continue;
        }

        // Only capture params that are used by current material
        bool found = false;
        for(int j=0;j<m_Techniques.size();j++)
        {
            if(m_Shader->GetEffect()->IsParameterUsed(m_Parameters[i]->GetHandle(),m_Techniques[j].Handle))
            {
                found = true;
                foundAnything = true;
                break;
            }
        }
        if(found)
            m_Shader->SetVar(*m_Parameters[i]);
	}
    if(foundAnything)
	    m_ParamBlock = m_Shader->GetEffect()->EndParameterBlock();
    else
        m_ParamBlock = 0;
}

//-----------------------------------------------------------------------------
// Fills our technique handles that contain a certain substring
//
//-----------------------------------------------------------------------------
void Material::SetTechnique(string token/*=""*/)
{
    m_Techniques.clear();
    //
    // This is legacy support for when we get pumped in incorrect tokens in multi-technique files
    //
    if(token.find("_Ambient") != -1) // Strip 
		token.erase(token.find("_Ambient"));
	if(token.find("_Point") != -1)   // Strip 
		token.erase(token.find("_Point"));
	if(token.find("_Spot") != -1)   // Strip 
		token.erase(token.find("_Spot"));
	if(token.find("_OmniProj") != -1)   // Strip 
		token.erase(token.find("_OmniProj"));

    m_Token = token;
    //
    // Enumerate all techniques in this shader, parsing annotations
    //
    // Example Annotation:
    // <    string LightMethods="PerPixel,PRT";
    //      string MaxLights="99";
    //      string LightTypes="Point,Spot,Ambient";
    // >
    int numTechniques = m_Shader->GetNumTechniques();
    for(int i=0;i<numTechniques;i++)
    {
        Technique tech;
        LPD3DXEFFECT effect = m_Shader->GetEffect();
        D3DXHANDLE hTech = effect->GetTechnique(i);
        D3DXTECHNIQUE_DESC techDesc;
        effect->GetTechniqueDesc(hTech,&techDesc);
        if(token.length() && string(techDesc.Name).find(token) == -1 && techDesc.Name[0] != '_')
            continue;

        tech.Handle = hTech;

        tech.Name = techDesc.Name;
        // Extract light types
        D3DXHANDLE temp = effect->GetAnnotationByName(hTech,"LightTypes");
        if(temp){
            tech.LightTypes = 0;
            string str = AsLower(m_Shader->GetString(temp));
            if(str.find("omniprojector") != -1)
                tech.LightTypes |= LIGHT_OMNI_PROJ;
            if(str.find("omni,") != -1 || str.find("point") != -1)
                tech.LightTypes |= LIGHT_OMNI;
            if(str.find("spot") != -1)
                tech.LightTypes |= LIGHT_SPOT;
            if(str.find("ambient") != -1 || str.find("dir") != -1)
                tech.LightTypes |= LIGHT_DIR;
        }

        // Extract Max Lights
        temp = effect->GetAnnotationByName(hTech,"MaxLights");
        if(temp){
            tech.MaxLights = atoi(m_Shader->GetString(temp).c_str());
        }

        // Extract methods
        temp = effect->GetAnnotationByName(hTech,"LightMethods");
        if(temp){
            string str = AsLower(m_Shader->GetString(temp));
            tech.PRT        = (str.find("prt") != -1);
            tech.PerPixel   = (str.find("perpixel") != -1);
            tech.ShadowProjector   = (str.find("shadowproject") != -1);
            tech.LightMapping = (str.find("lightmap") !=-1);
        }

        // Extract PS version
        D3DXPASS_DESC pDesc;
        effect->GetPassDesc(effect->GetPass(hTech,0),&pDesc);

        if(pDesc.pPixelShaderFunction)
        {
            DWORD ver = D3DXGetShaderVersion(pDesc.pPixelShaderFunction);
            if(ver == D3DPS_VERSION(3,0))
                tech.PixelShaderVersion = 3;
            if(ver == D3DPS_VERSION(2,0))
                tech.PixelShaderVersion = 2;
            if(ver == D3DPS_VERSION(1,4))
                tech.PixelShaderVersion = 1.4f;
            if(ver == D3DPS_VERSION(1,3))
                tech.PixelShaderVersion = 1.3f;
            if(ver == D3DPS_VERSION(1,2))
                tech.PixelShaderVersion = 1.2f;
            if(ver == D3DPS_VERSION(1,1))
                tech.PixelShaderVersion = 1.1f;
        }

        // Default if nothing specified
        if(tech.LightTypes == -1)
            tech.LightTypes = LIGHT_OMNI;

        // Technique annotation for optional grouping
        temp = effect->GetAnnotationByName(hTech,"TechGroup");
        if(temp)
            tech.Group = AsLower(m_Shader->GetString(temp));;

        RenderWrap::SetRS(D3DRS_ALPHABLENDENABLE,FALSE); // Set default
        RenderWrap::SetRS(D3DRS_ALPHATESTENABLE,FALSE); // Set default
        m_Shader->SetTechnique(tech.Handle);
        /// Apply params. Do NOT use a stateblock here because it would be outdated
        Apply(false);
        m_Shader->Begin();
        m_Shader->BeginPass(0);

        // Get some states from this pass. We'll reapply them between items when they get altered
        // Blend states
        RenderWrap::dev->GetRenderState(D3DRS_SRCBLEND,(DWORD*)&tech.m_SrcBlend);
        RenderWrap::dev->GetRenderState(D3DRS_DESTBLEND,(DWORD*)&tech.m_DestBlend);
        DWORD dw;
        RenderWrap::dev->GetRenderState(D3DRS_ALPHABLENDENABLE,&dw);
        // Opaque if no alpha and no color buffer reading
        // FIXME: Only indicates that should go in alpha batch, not real blending
		tech.m_AlphaBlend = dw==1 || m_Shader->m_Flags & Shader::ReadsColorBuffer  || m_Shader->m_Flags & Shader::ReadsGlobalColorBuffer;// || m_Shader->m_Flags & Shader::ReadsLDRBuffer;
        ///
        RenderWrap::dev->GetRenderState(D3DRS_ALPHATESTENABLE,&dw);
        tech.m_AlphaTest = dw;

        m_Shader->EndPass();
        m_Shader->End();

        // Add compatible techniques
        if(tech.PixelShaderVersion <= RenderDevice::Instance()->PixelShaderVersion)
            m_Techniques.push_back(tech);
    }

    //
    // Remove duplicate techniques that are inferior (choose superior only)
    // N.B. Inferior usually means lower shader versions than this card supports
Start:
    for(int i=0;i<m_Techniques.size();i++)
    {
        Technique& a = m_Techniques[i];
        // Search for techniques inferior to a
        for(int j=0;j<m_Techniques.size();j++)
        {
            if(j == i)
                continue;

            Technique& b = m_Techniques[j];

            // A is superior to B in every way
            if(a.Group == b.Group && 
                a.LightTypes & b.LightTypes && 
                (a.PerPixel || !b.PerPixel) &&
                (a.PRT || !b.PRT) && 
                (a.LightMapping || !b.LightMapping) && 
                a.MaxLights >= b.MaxLights&& 
                a.PixelShaderVersion >= b.PixelShaderVersion
                && (a.ShadowProjector == b.ShadowProjector))
            {
                // Delete B
                m_Techniques.erase(m_Techniques.begin()+j);
                //if(i>=j)
                //    i=0;
                //j--;
                goto Start;
            }
        }
    }

    // Update param block based on the new techniques and used params
    UpdateStates();
}        

//-----------------------------------------------------------------------------
// Fills our technique handles
//
// THIS FUNCTION IS LEGACY!! EnumerateTechniques() is preferred
//-----------------------------------------------------------------------------
/*bool Material::SetTechnique_OLD(string technique)
{
	if(technique.find("_Ambient") != -1) // Strip 
		technique.erase(technique.find("_Ambient"));
	if(technique.find("_Point") != -1)   // Strip 
		technique.erase(technique.find("_Point"));
	if(technique.find("_Spot") != -1)   // Strip 
		technique.erase(technique.find("_Spot"));
	if(technique.find("_OmniProj") != -1)   // Strip 
		technique.erase(technique.find("_OmniProj"));

	m_Technique[LIGHT_OMNI]   = m_Shader->GetTechnique(technique+"_Point");
	m_Technique[LIGHT_DIR] = m_Shader->GetTechnique(technique+"_Ambient");
	m_Technique[LIGHT_SPOT]    = m_Shader->GetTechnique(technique+"_Spot");
	m_Technique[LIGHT_OMNI_PROJ]    = m_Shader->GetTechnique(technique+"_OmniProj");

	m_TechniqueSH[LIGHT_OMNI]   = m_Shader->GetTechnique(technique+"_PointSH");
	m_TechniqueSH[LIGHT_DIR] = m_Shader->GetTechnique(technique+"_AmbientSH");
	m_TechniqueSH[LIGHT_SPOT]    = m_Shader->GetTechnique(technique+"_SpotSH");
	m_TechniqueSH[LIGHT_OMNI_PROJ]    = m_Shader->GetTechnique(technique+"_OmniProjSH");

	m_TechniqueSH_SinglePass[LIGHT_OMNI]   = m_Shader->GetTechnique(technique+"_PointSH_SP");
	m_TechniqueSH_SinglePass[LIGHT_DIR] = m_Shader->GetTechnique(technique+"_AmbientSH_SP");
	m_TechniqueSH_SinglePass[LIGHT_SPOT]    = m_Shader->GetTechnique(technique+"_SpotSH_SP");
	m_TechniqueSH_SinglePass[LIGHT_OMNI_PROJ]    = m_Shader->GetTechnique(technique+"_OmniProjSH_SP");

	if(!m_TechniqueSH_SinglePass[LIGHT_OMNI])
		m_TechniqueSH_SinglePass[LIGHT_OMNI] = m_TechniqueSH[LIGHT_OMNI];
	if(!m_TechniqueSH_SinglePass[LIGHT_DIR])
		m_TechniqueSH_SinglePass[LIGHT_DIR] = m_TechniqueSH[LIGHT_DIR];

	// If no light tech found, we'll request an 'unlit' version, using the raw technique string we were passed to begin with
	bool found = false;
	for(int i=0;i<NUM_LIGHT_TYPES;i++){
		if(m_Technique[i] || m_TechniqueSH[i] || m_TechniqueSH_SinglePass[i]){
			found = true;
			break;
		}
	}
	if(!found){
		D3DXHANDLE h = m_Shader->GetTechnique(technique);
		if(h)
			found = true;
		for(int i=0;i<NUM_LIGHT_TYPES;i++){
			m_Technique[i]   = h;
			m_TechniqueSH[i] = h;
			m_TechniqueSH_SinglePass[i] = h;
		}
	}
	// If any light techniques are NULL, copy over one of the valid techniques to stop them dying at runtime  
	for(int i=0;i<NUM_LIGHT_TYPES;i++)  
	{  
		// Try to find light within same SH  
		if(!m_Technique[i])  
			m_Technique[i] = m_Technique[LIGHT_OMNI];  
		if(!m_TechniqueSH[i])  
			m_TechniqueSH[i] = m_TechniqueSH[LIGHT_OMNI];  
		if(!m_TechniqueSH_SinglePass[i])  
			m_TechniqueSH_SinglePass[i] = m_TechniqueSH_SinglePass[LIGHT_OMNI];  

		// If not within same SH, search SH modes  
		if(!m_TechniqueSH_SinglePass[i])  
			m_TechniqueSH_SinglePass[i] = m_TechniqueSH[i];  
		if(!m_TechniqueSH_SinglePass[i])  
			m_TechniqueSH_SinglePass[i] = m_Technique[i];
		if(!m_TechniqueSH[i])  
			m_TechniqueSH[i] = m_TechniqueSH_SinglePass[i];
		if(!m_Technique[i])  
			m_Technique[i] = m_TechniqueSH_SinglePass[i];
	}   

	if(!found){
		SeriousWarning("Could not find suitable technique for technique '%s' on shader '%s', material is '%s'. Shader may not contain fallback, or may lack appropriate lighting version.\nContact the person who created the shader. Attempting to fall back to previous technique.",technique.c_str(),m_Shader->GetFilename().c_str(),m_Name.c_str());
		if(m_TechniqueName != technique)
			SetTechnique(m_TechniqueName);
		return false;
	}

	m_TechniqueName	= technique;

    Refresh();

	return true;
}
*/

//-----------------------------------------------------------------------------
/// Get technique based on group tag
//-----------------------------------------------------------------------------
Technique*  Material::GetTechnique(string groupTag)
{
    for(int i=0;i<m_Techniques.size();i++)
    {
        if(m_Techniques[i].Group == groupTag || m_Techniques[i].Name == groupTag)
        {
            return &m_Techniques[i];
        }
    }
    return 0;
}

//-----------------------------------------------------------------------------
// Refresh if altered, in editor or on load
//-----------------------------------------------------------------------------
void Material::Refresh()
{
    // Do this to update stored alpha techniques
    SetTechnique(m_Token);
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void Material::SetEditorVars()
{
	EditorVars.clear();
	// Fill vars
	//EditorVars.push_back(EditorVar("Name",&m_Name)); // Name done via widget
	EditorVars.push_back(EditorVar("Shader",&m_ShaderName));
	EditorVars.back().type = EditorVar::FILENAME;

	//EditorVars.push_back(EditorVar("Technique",&m_TechniqueName)); // Done via combo

	for(int i=0;i<m_Parameters.size();i++)
	{
		m_Parameters[i]->Reset();
		EditorVar var;
		var.name = m_Parameters[i]->name;
		var.type = m_Parameters[i]->type;
		var.data = m_Parameters[i]->data;
		var.desc = m_Parameters[i]->desc;
		var.category = "Parameters";
		EditorVars.push_back(var);
	}
}

//-----------------------------------------------------------------------------
// TODO: bForceRecompile should really recompile all instances of the shader
// instead of instancing a new one, but that would require remapping ALL handles, ick
//-----------------------------------------------------------------------------
bool Material::Initialize(const char* cshader, const char*  ctechnique, bool bForceRecompile)
{
    string technique = ctechnique;
	m_ShaderName = cshader;
	SAFE_RELEASE(m_Shader);
	EditorVars.clear();

	// Load shader file
	if(!bForceRecompile)
		m_Shader = ShaderManager::Instance()->GetShader(m_ShaderName);

	if(m_Shader == NULL){ // Not cached
		m_Shader		 = new Shader; 
		if(!m_Shader->Load(m_ShaderName.c_str())){
			SAFE_DELETE(m_Shader);
			return false;
		}
	}

	// Get parameters from file if recompiled or not already stored
	//if(m_Parameters.size() == 0 || bForceRecompile)

    // TIM: Always extract params again, in case shader was modified after material was last saved.
		ExtractParameters();
	//else
		SetEditorVars();

	// Check for invalid params that no longer exist in the shader. We'll print a warning then ignore them
	for(int i=0;i<m_Parameters.size();i++){
		D3DXHANDLE handle = m_Shader->GetEffect()->GetParameterByName(NULL,m_Parameters[i]->name.c_str());
		if(handle == NULL){
			Warning("The parameter '%s' on material '%s' does not exist on shader '%s'. Please re-export the model or map that is using this material soon, because it was compiled with outdated shaders and may have visual problems.",
				m_Parameters[i]->name.c_str(),m_Name.c_str(),m_ShaderName.c_str());
			m_Parameters.erase(m_Parameters.begin()+i);
			i--;
		}
	}

	// Get a list of techniques, WITHOUT the lighting extension
	// THis is used mainly in the editor
	LPD3DXEFFECT pEffect = m_Shader->GetEffect();
	m_TechniqueNames.clear();
	if (pEffect)
	{
		D3DXEFFECT_DESC desc;
		pEffect->GetDesc(&desc);
		for (int i = 0; i < desc.Techniques; i++)
		{
			D3DXHANDLE hTechnique = pEffect->GetTechnique(i);
			D3DXTECHNIQUE_DESC TechniqueDesc;
			pEffect->GetTechniqueDesc(hTechnique, &TechniqueDesc);
			string tech = TechniqueDesc.Name;
            // Underscored techniques are not listed
            if(tech[0] == '_')
                continue;

			if(tech.find("_Ambient") != -1) // Strip 
				tech.erase(tech.find("_Ambient"));
			if(tech.find("_Point") != -1)   // Strip 
				tech.erase(tech.find("_Point"));
			if(tech.find("_Spot") != -1)   // Strip 
				tech.erase(tech.find("_Spot"));
			if(tech.find("_OmniProj") != -1)   // Strip 
				tech.erase(tech.find("_OmniProj"));
			if(tech.find("_PS") != -1)   // Strip PSXX
				tech.erase(tech.find("_PS"));

			// See if added yet
			bool found = false;
			for(int j=0;j<m_TechniqueNames.size();j++){
				if(m_TechniqueNames[j] == tech)
					found = true;
			}
			if(!found)
				m_TechniqueNames.push_back(tech);
		}
	}

	//if(technique.length())
		SetTechnique(technique);
	//else if(m_TechniqueNames.size())
	//	SetTechnique(m_TechniqueNames[0]);
	//else
	//	SeriousWarning("No techniques found in shader");

	m_Emissive = FindVar("MatEmissive");

	//ExtractParameters();
	return true;
}


//-----------------------------------------------------------------------------
// Apply material
//-----------------------------------------------------------------------------
bool Material::Apply(bool useParamBlock)
{
	// HACK: Apply default properties in case this material doesn't have them
	// TODO: Replace with proper var setup in 3dsmax
	//if(this != MaterialManager::Instance()->GetDefaultMaterial())
	//	MaterialManager::Instance()->GetDefaultMaterial()->Apply();

	if(!useParamBlock)
    {
	    for(int i=0;i<m_Parameters.size();i++){
	    	m_Shader->SetVar(*m_Parameters[i]);
	    }
    }
	else if(m_ParamBlock && m_Parameters.size())
		(m_Shader->GetEffect()->ApplyParameterBlock(m_ParamBlock));

	return true;
}

//-----------------------------------------------------------------------------
// Is parameter tweakable? Must conform to FX Composer, RenderMonkey, etc
// Non-tweakables won't be stored - doing so could even corrupt engine-set vars!
//-----------------------------------------------------------------------------
bool Material::Tweakable(LPCSTR hParam)
{
    LPD3DXEFFECT pEffect = m_Shader->GetEffect();
    D3DXPARAMETER_DESC pdesc;
    if (FAILED(pEffect->GetParameterDesc(hParam,&pdesc)))
    {
        return false;
    }
    // Material parameters are only tweakable things, and tweakable things always have annotations, so skip this
    // if it's not annotated.
    if(pdesc.Annotations == 0)
        return false;

    // Check widget
    D3DXHANDLE hDesc = pEffect->GetAnnotationByName(hParam,"UIWidget");
    if(hDesc)
    {
        // No widget, thus not displayed
        if(m_Shader->GetString(hDesc).find("None") != -1)
            return false;
    }
    // Object annotations are not editable. They are system things such as lights
    if(pEffect->GetAnnotationByName(hParam,"Object"))
        return false;

    return true;
}

//-----------------------------------------------------------------------------
// Clones a material+parameters
//-----------------------------------------------------------------------------
void Material::CloneTo(Material* material)
{
    // Initialize mateiral with same shader (technique is irrelevant)
    material->Initialize(m_ShaderName.c_str(),"");

    // Copy the values of all matching parameters (array order may be different, so must equate names)
    for(int i=0;i<m_Parameters.size();i++)
    {
        for(int j=0;j<material->m_Parameters.size();j++)
        {
            if(material->m_Parameters[j]->name == m_Parameters[i]->name)
			{
				if(m_Parameters[i]->type != EditorVar::TEXTURE)
					material->m_Parameters[j]->Alloc(m_Parameters[i]->data,m_Parameters[i]->dataLen);
				else
					((Texture*)material->m_Parameters[j]->data)->Load(((Texture*)m_Parameters[i]->data)->filename);
			}
        }
    }

	material->SetEditorVars();
}

//-----------------------------------------------------------------------------
// Extracts annotated parameters from an effect at runtime. Used for
// loading new shaders, reloading shaders when params change, etc
//-----------------------------------------------------------------------------
void Material::ExtractParameters()
{
	vector<ShaderVar*>	newParams; // All effects parameters
	LPD3DXEFFECT pEffect = m_Shader->GetEffect();
	D3DXEFFECT_DESC edesc;
	if (pEffect->GetDesc(&edesc) != S_OK)
		return;

	for (int i=0;i<edesc.Parameters;++i)
	{
		D3DXHANDLE  hParam = pEffect->GetParameter(NULL, i);
		D3DXPARAMETER_DESC pdesc;
		if (FAILED(pEffect->GetParameterDesc(hParam,&pdesc)))
		{
			assert(!"Could not get parameter desc!");
			continue;
		}
		if(!Tweakable(hParam))
            continue;

		ShaderVar* var = new ShaderVar;

		if (pdesc.Columns > 1 && pdesc.Type != D3DXPT_FLOAT)
			SeriousWarning("Material engine cannot parse arrays/tuples of any values but floats. %s does not meet this criteria",pdesc.Name);

		// Get UI Desc if exists
		D3DXHANDLE hDesc = pEffect->GetAnnotationByName(hParam,"UIHelp");
		if(hDesc)
		{
            var->desc = m_Shader->GetString(hDesc);
		}

    
        var->name = pdesc.Name;
        assert(1024 >  pdesc.Bytes);
		BYTE data[1024];
		DXASSERT(pEffect->GetValue(pdesc.Name, data, D3DX_DEFAULT));
		
		// Parse the type!
		switch(pdesc.Type){
			case D3DXPT_INT:
				var->type = EditorVar::INT;
				var->Alloc(data,pdesc.Bytes);
				break;
			case D3DXPT_STRING:
				var->type = EditorVar::STRING;
				break;

			case D3DXPT_FLOAT:
				var->Alloc(data,pdesc.Bytes);
				// Float?
				if (pdesc.Rows == 1)
				{
					if (pdesc.Columns == 1)
					{
						var->type = EditorVar::FLOAT;
					}
					else if (pdesc.Columns == 2)
					{
						var->type = EditorVar::FLOAT2;
					}
					else if (pdesc.Columns == 3)
					{
						var->type = EditorVar::FLOAT3;
					}
					else if (pdesc.Columns == 4)
					{
						var->type = EditorVar::FLOAT4;
					}
					else
					{
						SeriousWarning("Can't do above float4. %s is a float%d",pdesc.Name,pdesc.Columns);
					}
				}	
				// Matrix
				else
				{
					SAFE_DELETE(var);
					continue;

					var->type = EditorVar::MATRIX;
				}
				break;
			case D3DXPT_BOOL:
				var->type = EditorVar::BOOL;
				var->Alloc(data,pdesc.Bytes);
				break;
			case D3DXPT_TEXTURE:
			case D3DXPT_TEXTURE1D:
			case D3DXPT_TEXTURE2D:
			case D3DXPT_TEXTURECUBE:
			case D3DXPT_TEXTURE3D:
				{
				var->type = EditorVar::TEXTURE;
				}
				break;
			
			default:
				SeriousWarning("WARNING: Unknown type in connection manager");
				break;
		}

		//
		// See if we can map old param data to this param
		//
		for(int j=0;j<m_Parameters.size();j++)
		{
			if(m_Parameters[j] && m_Parameters[j]->name == var->name && m_Parameters[j]->type == var->type)
			{
				// Only copy integral types
				if(pdesc.Type == D3DXPT_BOOL || pdesc.Type == D3DXPT_FLOAT || pdesc.Type == D3DXPT_INT)
					memcpy(var->data,m_Parameters[j]->data,pdesc.Bytes);

				// If a texture var, just use the previous one!
				if(var->type == EditorVar::TEXTURE)
				{
					delete var;
					var = m_Parameters[j];
					m_Parameters[j] = NULL; // To avoid deletion
					//var->data = m_Parameters[j]->data;
					//m_Parameters[j]->del = ShaderVar::VAR_DONTDELETE; // This is our memory now!
				}
			}
		}

		// Load texture if not already loaded
		if(var->type == EditorVar::TEXTURE && var->data == NULL)
		{
			Texture* tex = new Texture;
			tex->Load(tex->filename,TT_AUTO); // TT_AUTO will force loading of default texture
			var->Set(var->name,tex);
		}
		newParams.push_back(var);
	}
 
	SAFE_DELETE_VECTOR(m_Parameters);
	m_Parameters = newParams;

	// Remap effect vars
	SetEditorVars();
}



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool Material::Load(const char* filename)
{
	m_FileName = filename;

	ResourceManager::Instance()->Add(this);

	if(Engine::Instance()->IsDedicated())
		return true;

	//string sGuid = ReadString(pMat,"GUID");
	//UuidFromString((unsigned char*)sGuid.c_str(),&m_GUID);

	XMLSystem m_XML;
	if(!m_XML.Load(filename)){
		return false;
	}

	// use the tree walker to recurse the nodes
	DOMTreeWalker* walker = m_XML.GetWalker();
	DOMElement* pMat = (DOMElement*)m_XML.m_Parser->getDocument()->getDocumentElement();//(DOMElement*)walker->nextNode();

	m_Name				= ReadString(pMat,"Name");
	m_Category			= ReadString(pMat,"Category");

	DOMElement* shader		= m_XML.FindFirstNode(pMat,"Shader");
	if(shader)
	{
		m_ShaderName		= ReadString(shader,"File");
		m_Token         	= ReadString(shader,"Technique");

		DOMNodeList* params		= shader->getElementsByTagName(L"Parameter");
		if(!params || params->getLength() == 0)
			params		= shader->getElementsByTagName(L"Param");

		for(int j=0;j<params->getLength();j++){
			ShaderVar* p	= new ShaderVar;

			BYTE* data = new BYTE[1024]; // Temp, until we know real size
			ZeroMemory(&data[0],1024);
			int   size = 0;
			m_XML.ReadParameter(p->type,(BYTE*)data,params->item(j),size,p->name);

			if(p->type == EditorVar::TEXTURE){
				string value	= ((Texture*)data)->filename;
				// Allocate a texture
				Texture* tex = new Texture;
				tex->filename = value;
				tex->Load(value);
				p->Set(p->name,tex);

				// Try to set the material type from this map if it's not already been found
				if(m_Type == "unknown")
					m_Type = MaterialManager::Instance()->GetType(value);
			}
			else{
				// ShaderVar will alloc exact memory and copy from buffer
				p->Alloc(data,size);
			}

			// Buffer was temp, so delete
			delete[] data;

			//
			m_Parameters.push_back(p);
		}

        Initialize(m_ShaderName.c_str(),m_Token.c_str());
	}
	else
	{
		SeriousWarning("Material '%s' lacks shader!",m_Name.c_str());
	}

	
	return true;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool Material::Save(string filename)
{
	m_FileName = filename;
	
	XMLSystem m_XML;
	xercesc_2_5::DOMDocument* doc = m_XML.CreateDocument("Material");
	DOMElement* matNode = doc->getDocumentElement();

	BYTE* buf;
	UuidToStringA(&m_GUID,&buf);
	m_XML.Attrib("GUID",buf);
	m_XML.Attrib("Name",m_Name);
	m_XML.Attrib("Category",m_Category);

	// TODO: Submaterials

	DOMElement* shader = m_XML.CreateNode(matNode,"Shader");
	m_XML.Attrib("File",m_Shader->GetFilename());
	m_XML.Attrib("Technique",m_Token);
	for(int j=0;j<m_Parameters.size();j++)
	{
		ShaderVar* var = m_Parameters[j];
		m_XML.WriteParameter((TCHAR*)m_Parameters[j]->name.c_str(),m_Parameters[j]->type,(BYTE*)m_Parameters[j]->data,shader);
	}

	m_XML.Save(m_FileName);
	return true;
}


//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
MaterialManager* MaterialManager::Instance () 
{
	static MaterialManager inst;
	return &inst;
}

//-----------------------------------------------------------------------------
// Finds a material in the global array
//-----------------------------------------------------------------------------
Material* MaterialManager::FindMaterial(string name){
	for(int i=0;i<m_Materials.size();i++){
		if(name == m_Materials[i]->m_Name)
			return m_Materials[i];
	}
	return NULL;
}

//-----------------------------------------------------------------------------
// Build default material
//-----------------------------------------------------------------------------
void MaterialManager::Initialize(){
	if(m_DefaultMaterial)
		return; // Already initialized

	// Build default material
	m_DefaultMaterial = new Material("(Internal) Default Material");

	// Create diffuse 
	Texture* t = new Texture;
	t->Load("DefaultTexture.dds");
	if(!t->IsValid())
		Error("Couldn't find/load DefaultTexture.dds\nYour install may be corrupt");
	// Add to params as 'tDiffuse0'
	m_DefaultMaterial->m_Parameters.push_back(
		new ShaderVar("tDiffuse0",EditorVar::TEXTURE,ShaderVar::VAR_DELETE,t));

	// Create bump
	t = new Texture;
	t->Load("DefaultNormal.dds",TT_NORMALMAP);
	if(!t->IsValid())
		Error("Couldn't find/load DefaultNormal.dds\nYour install may be corrupt");
	// Add to params as 'tBump0'
	m_DefaultMaterial->m_Parameters.push_back(
		new ShaderVar("tBump0",EditorVar::TEXTURE,ShaderVar::VAR_DELETE,t));

	// Fixed emissive param, needed by all shaders
	D3DXCOLOR* col = new D3DXCOLOR;
	*col = D3DXCOLOR(1,1,1,1);
	m_DefaultMaterial->m_Parameters.push_back(
		new ShaderVar("MatEmissive",EditorVar::FLOAT4,ShaderVar::VAR_DELETE,col));


	m_DefaultMaterial->Initialize("Diffuse.fx","Diffuse");

	//-----------------------------------------------------------------------------
	// Keyword strings identifying materials
	//-----------------------------------------------------------------------------
	m_MaterialTypes.push_back("water");
	m_MaterialTypes.push_back("metal");
	m_MaterialTypes.push_back("wood");
	m_MaterialTypes.push_back("marble");
	m_MaterialTypes.push_back("gravel");
	m_MaterialTypes.push_back("grass");
	m_MaterialTypes.push_back("mud");
	m_MaterialTypes.push_back("earth");
	m_MaterialTypes.push_back("stone");
	m_MaterialTypes.push_back("glass");
	m_MaterialTypes.push_back("snow");
	m_MaterialTypes.push_back("skin");
	m_MaterialTypes.push_back("carpet");

	//-----------------------------------------------------------------------------
	// Load all material libraries
	// Could do this selectively at load-time if it gets too slow
	//-----------------------------------------------------------------------------
	/*vector<string> files;
	enumerateFiles("..\\Materials\\",files);
	for(int i=0;i<files.size();i++)
	{
		if(AsLower(files[i]).find(".xml") == -1)
			continue;

		MaterialLibrary* lib = new MaterialLibrary();
		lib->Load(files[i]);
		m_Libraries.push_back(lib);
	}*/
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void MaterialManager::Shutdown(){
	m_DefaultMaterial->Release();
	SAFE_DELETE_VECTOR(m_Libraries);
}



//-----------------------------------------------------------------------------
// Sets the material type based on texture name lookup
// For example if the path or name contains grass, the material type
// will be MT_Grass
// This should be the default fallback if the designer didn't set a type in the editor
//-----------------------------------------------------------------------------
string MaterialManager::GetType(string texName){
	ToLowerCase(texName);
	string type = "unknown";
	for(int i=0;i<m_MaterialTypes.size();i++){
		if(texName.find(m_MaterialTypes[i]) != -1){
			type = m_MaterialTypes[i];
		}
	}
	return type;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
Material* MaterialLibrary::GetMaterial(int index)
{
	Material* mat = Materials[index];
	if(mat->m_Shader == NULL)
	{
		for(int j=0;j<mat->m_Parameters.size();j++){
			if(mat->m_Parameters[j]->type == EditorVar::TEXTURE){
				Texture* t = (Texture*)mat->m_Parameters[j]->data;
				t->Load(t->filename);
			}
		}
        mat->Initialize(mat->m_ShaderName.c_str(),mat->m_Token.c_str());
	}
	return mat;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
Material* MaterialLibrary::GetMaterial(GUID guid)
{
	for(int i=0;i<Materials.size();i++)
	{
		Material* mat = Materials[i];
		if(guid == mat->m_GUID)
		{
			if(mat->m_Shader == NULL)
			{
				for(int j=0;j<mat->m_Parameters.size();j++){
					if(mat->m_Parameters[j]->type == EditorVar::TEXTURE){
						Texture* t = (Texture*)mat->m_Parameters[j]->data;
						t->Load(t->filename);
					}
				}
                mat->Initialize(mat->m_ShaderName.c_str(),mat->m_Token.c_str());
			}
			return mat;
		}
	}
	return 0;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
MaterialLibrary::MaterialLibrary()
{
	MaterialManager::Instance()->m_Libraries.push_back(this);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
MaterialLibrary::~MaterialLibrary()
{
	vector_erase(MaterialManager::Instance()->m_Libraries,this);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool MaterialLibrary::Load(string filename)
{
	FileName = filename;
	MaterialSerializer s;
	return s.Load(filename,this);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool MaterialLibrary::Save(string filename)
{
	FileName = filename;
	MaterialSerializer s;
	return s.Save(filename,this);
}