#include "pch.h"
#include "material.h"
#include "RenderMesh.h"
#include "vertexshader.h"
#include "connectionpblock.h"
#include "effectmgr.h"
#include <io.h> // access()

using namespace nv_fx;
using namespace nv_sys;

void DoMissingFile(std::string& strFileName);

ShaderInfoData::ShaderInfoData(ShaderInfo* pParent)
: m_pEffect(NULL),
m_pParams(NULL),
m_pConnectionManager(NULL),
m_pConnectionPBlock(NULL),
m_pParent(pParent),
m_pVSData(NULL)
{
	NVPROF_FUNC("ShaderInfoData::ShaderInfoData");
	ZeroMemory(&m_Desc, sizeof(D3DXEFFECT_DESC));
	m_pConnectionPBlock = new CConnectionPBlock;
}

ShaderInfoData::~ShaderInfoData()
{
	NVPROF_FUNC("ShaderInfoData::~ShaderInfoData");
	FreeEffect_Ver2(OWNER_NONE);
};

const char*  ShaderInfoData::GetTechniqueName(unsigned int Technique) const{
	assert(m_pEffect);
	NVFXHANDLE hTechnique = m_pEffect->GetTechnique(Technique);
	D3DXTECHNIQUE_DESC TechniqueDesc;
	m_pEffect->GetTechniqueDesc(hTechnique, &TechniqueDesc);
	return TechniqueDesc.Name;
}

bool ShaderInfoData::SetTechnique(OWNER_ID Owner, const char* Technique)
{
	NVPROF_FUNC("ShaderInfoData::SetTechnique");
	m_Technique = Technique;

	bool ret = m_pParent->GetMaterial()->GetVertexShader()->SetTechnique(Technique);
	
	if (Owner != OWNER_MATERIAL)
	{
		m_pParent->GetMaterial()->OnSetTechnique(Technique);
	}

	return ret;
}


bool ShaderInfoData::SetTechnique(OWNER_ID Owner, unsigned int Technique)
{
	return SetTechnique(Owner,(char*)GetTechniqueName(Technique));
}

bool ShaderInfoData::FreeEffect(OWNER_ID Owner, bool bStartup)
{
	NVPROF_FUNC("ShaderInfoData::FreeEffect");
	DISPDBG(1, "ShaderInfoData::FreeEffect");

	assert(m_pParent->GetMaterial()->GetFileVersion() < 2);

	// Free current textures, they'll reload if necessary
	m_pParent->GetMaterial()->GetVertexShader()->FreeTextures();

	m_pParent->GetMaterial()->GetVertexShader()->RemoveEffect();

	// Always a conneciton pblock.
	assert(m_pConnectionPBlock);

	// Clear the PBlock because we are changing effect
	if (!bStartup)
	{
		m_pConnectionPBlock->ClearItems(CConnectionPBlock::CLEAR_PBLOCK);
		SAFE_DELETE(m_pVSData);
	}

	if (m_pParams)
	{
		SAFE_RELEASE(m_pParams);
	}

	if (m_pConnectionManager)
	{
		if (m_pEffect)
		{
			m_pConnectionManager->RemoveEffect(m_pEffect);

			CEffectManager::GetSingletonPtr()->RemoveEffect(m_pEffect);
		}

		SAFE_RELEASE(m_pConnectionManager);
	}

	DISPDBG(1, "ShaderInfoData::FreeEffect - Leave");
	return true;
}

bool ShaderInfoData::LoadEffect(OWNER_ID Owner, const char* pszPath, bool bStartup)
{
	NVPROF_FUNC("ShaderInfoData::LoadEffect");

	DISPDBG(1, "ShaderInfoData::LoadEffect");

	assert(m_pParent->GetMaterial()->GetFileVersion() < 2);

	FreeEffect(Owner, bStartup);

	// Update the file
	m_strShaderFile = pszPath;

	// Load the effect
	LPD3DXEFFECT pEffect = m_pParent->GetMaterial()->GetVertexShader()->LoadEffect();
	m_pEffect = pEffect; 
	if (m_pEffect)
	{
		m_pEffect->GetDesc(&m_Desc);
	}
	else
	{
		ZeroMemory(&m_Desc, sizeof(D3DXEFFECT_DESC));
	}

	unsigned int i;
	if (m_pEffect)
	{
		m_pConnectionManager = INVConnectionManager::Create();
		m_pConnectionManager->Initialize();

		// Callback for us to load textures (potentially).
		m_pConnectionManager->SetEffectParamInitCallback(m_pParent->GetMaterial());

	
		for (i = 0; i < m_Desc.Techniques; i++)
		{
			if (m_pParent->GetMaterial()->GetVertexShader()->SetTechnique(i))
			{
				m_Technique = i;
				m_pParent->GetMaterial()->GetVertexShader()->SetupEffect();

				// Add the effect to the connection manager.
				// This will cause the connections to re-evaluate
				m_pConnectionManager->AddEffect(m_pEffect);

				m_pParams = m_pConnectionManager->CreateParameterList(m_pEffect); 

				m_pParent->GetMaterial()->GetVertexShader()->AddRefEffectTextures();
		
				// Build a new PBlock
				if (!bStartup)
				{
					m_pConnectionPBlock->AddItems(m_pParams, CConnectionPBlock::BUILD_PBLOCK);
					m_pConnectionPBlock->Synchronize(GetCOREInterface()->GetTime(), SYNC_PBLOCK, NULL);

				}
				else
				{
					m_pConnectionPBlock->AddItems(m_pParams, CConnectionPBlock::USE_CURRENT_PBLOCK);
					m_pConnectionPBlock->Synchronize(GetCOREInterface()->GetTime(), SYNC_CONNECTION, NULL);

				}


				
#ifdef _DEBUG
				m_pParent->DumpParameterList(m_pParams);
				//m_pConnectionManager->Dump();
#endif

				// Set technique afterwards so we don't cause a redraw before a setup effect.
				if (Owner != OWNER_MATERIAL)
				{
					m_pParent->GetMaterial()->OnLoadEffect(pszPath);
					m_pParent->GetMaterial()->OnSetTechnique(i);
				}
				return true;
			}
		}
		MessageBox(NULL, "Failed to find a valid technique in effect!\nReverting to Diffuse.fx", "Error", MB_ICONEXCLAMATION | MB_OK);
	}	

	DISPDBG(1, "ShaderInfoData::LoadEffect - Leave; failing to find technique");
	FreeEffect(Owner, bStartup);

	return false;
}


// *********************************************************************
// Version 2 API
// *********************************************************************

bool ShaderInfoData::LoadEffect_Ver2(OWNER_ID Owner, const char* pszPath)
{
	NVPROF_FUNC("ShaderInfoData::LoadEffect_Ver2");

	DISPDBG(1, "ShaderInfoData::LoadEffect_Ver2");

	FreeEffect_Ver2(Owner);

	// Update the file
	m_strShaderFile = pszPath;

	// File doesn't exist, try to locate it
	if(access( m_strShaderFile.c_str(), 0 ) != 0){
		if(FindFile(m_strShaderFile).length())
			m_strShaderFile = FindFile(m_strShaderFile);
		else
			DoMissingFile(m_strShaderFile);
	}

	// Load the effect
	m_pEffect = NULL; // Stop it trying to do anything
	LPD3DXEFFECT pEffect = m_pParent->GetMaterial()->GetVertexShader()->LoadEffect();
	m_pEffect = pEffect; 
	if (m_pEffect)
	{
		m_pEffect->GetDesc(&m_Desc);
	}
	else
	{
		ZeroMemory(&m_Desc, sizeof(D3DXEFFECT_DESC));
	}

	unsigned int i;
	if (m_pEffect)
	{
		m_pConnectionManager = INVConnectionManager::Create();
		m_pConnectionManager->Initialize();

		// Callback for us to load textures (potentially).
		m_pConnectionManager->SetEffectParamInitCallback(m_pParent->GetMaterial());

	
		for (i = 0; i < m_Desc.Techniques; i++)
		{
			if (m_pParent->GetMaterial()->GetVertexShader()->SetTechnique(i))
			{
				m_Technique = i;
				m_pParent->GetMaterial()->GetVertexShader()->SetupEffect();

				// Add the effect to the connection manager.
				// This will cause the connections to re-evaluate
				m_pConnectionManager->AddEffect(m_pEffect);

				m_pParams = m_pConnectionManager->CreateParameterList(m_pEffect); 

				m_pParent->GetMaterial()->GetVertexShader()->AddRefEffectTextures();
		
				if (m_pParent->GetMaterial()->GetVertexShader()->GetLighting())
					m_pParent->GetMaterial()->GetVertexShader()->GetLighting()->SetLoadInfo(m_pParent->GetMaterial()->GetLightStreamInfo());

				// Build a new PBlock
				m_pConnectionPBlock->AddItems(m_pParams, CConnectionPBlock::BUILD_PBLOCK);
				m_pConnectionPBlock->Synchronize(GetCOREInterface()->GetTime(), SYNC_PBLOCK, NULL);
				
#ifdef _DEBUG
				m_pParent->DumpParameterList(m_pParams);
				//m_pConnectionManager->Dump();
#endif

				// Set technique afterwards so we don't cause a redraw before a setup effect.
				if (Owner != OWNER_MATERIAL)
				{
					m_pParent->GetMaterial()->OnLoadEffect(pszPath);
					m_pParent->GetMaterial()->OnSetTechnique(i);
				}
				return true;
			}
		}
		MessageBox(NULL, "Failed to find a valid technique in effect!\nReverting to Diffuse.fx", "Error", MB_ICONEXCLAMATION | MB_OK);
	}	

	DISPDBG(1, "ShaderInfoData::LoadEffect_Ver2 - Leave; failing to find technique");
	FreeEffect_Ver2(Owner);

	return false;
}

bool ShaderInfoData::FreeEffect_Ver2(OWNER_ID Owner)
{
	NVPROF_FUNC("ShaderInfoData::FreeEffect_Ver2");
	DISPDBG(1, "ShaderInfoData::FreeEffect_Ver2");

	// Free current textures, they'll reload if necessary
	m_pParent->GetMaterial()->GetVertexShader()->FreeTextures();

	m_pParent->GetMaterial()->GetVertexShader()->RemoveEffect();

	// Always a conneciton pblock.
	assert(m_pConnectionPBlock);

	// Clear the PBlock because we are changing effect
	m_pConnectionPBlock->ClearItems(CConnectionPBlock::CLEAR_PBLOCK);
	SAFE_DELETE(m_pVSData);

	if (m_pParams)
	{
		SAFE_RELEASE(m_pParams);
	}

	if (m_pConnectionManager)
	{
		if (m_pEffect)
		{
			m_pConnectionManager->RemoveEffect(m_pEffect);

			CEffectManager::GetSingletonPtr()->RemoveEffect(m_pEffect);
		}

		SAFE_RELEASE(m_pConnectionManager);
	}

	DISPDBG(1, "ShaderInfoData::FreeEffect - Leave");
	return true;
}
