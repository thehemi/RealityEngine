/*********************************************************************NVMH3****
Path:  E:\nvidia\devrel\Tools\nvmax
File:  cgfxdatabridge.cpp

Copyright NVIDIA Corporation 2002
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED
*AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS
BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES
WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS,
BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS)
ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS
BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:

An exported class used by the flexporter plugin to get the CgFX parameters.


******************************************************************************/

#include "pch.h"
#include "RenderMesh.h"
#include "Material.h"
#include "vertexshader.h"
#include "cgfxdatabridge.h"

using namespace nv_sys;
using namespace nv_renderdevice;

CgFXDataBridge::CgFXDataBridge(MaxVertexShader* pVS)
: m_pVS(pVS)
{
}

CgFXDataBridge::~CgFXDataBridge()
{
    
}

// Query CgFX Data...
TCHAR * CgFXDataBridge::GetCgFX()
{
	// TIM: Trigger load when saving, so we can get data
	m_pVS->m_pMaterial->TriggerLoad();

	ShaderInfoData* pShader = m_pVS->GetMaterial()->GetShaderInfo()->GetCurrentShader();
    return (TCHAR*)pShader->GetEffectFile().c_str();
}

INVParameterList* CgFXDataBridge::GetParameterList()
{
	ShaderInfoData* pShader = m_pVS->GetMaterial()->GetShaderInfo()->GetCurrentShader();
	return pShader->GetParameterList();
}

string CgFXDataBridge::GetTechnique()
{
	ShaderInfoData* pShader = m_pVS->GetMaterial()->GetShaderInfo()->GetCurrentShader();
	return pShader->GetTechnique();
}

const char* CgFXDataBridge::GetTextureName(INVTexture* pTex)
{
	return TextureMgr::GetSingletonPtr()->GetTextureName(pTex);
}

/* Here's how you walk the parameters:
for (unsigned int i = 0; i < pParamList->GetNumParameters(); i++)
{
	// Find out the active connection parameter
	INVConnectionParameter* pParam = pParamList->GetActiveConnection(i);

	if (!pParam)
		continue;

	// Write out the data same for all connection types:

	// The Name of hte parameter
	SaveData(pParam->GetName());

	// Possible to write out the semantic name?  Could be useful.
	SaveData(ConvertSemantic(pParam->GetSemanticID()))

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
	}
	

	switch(pParam->GetDefaultValue().GetType())
	{
		case nv_sys::NVTYPEID_VEC4:
		{
			IConnection_Vec4* pVec4 = static_cast<IConnection_Vec4*>(pParam);
			SaveData(pVec4->GetVec4())
		}
		break;

		case nv_sys::NVTYPEID_VEC3:
		{
			IConnection_Vec3* pVec3 = static_cast<IConnection_Vec3*>(pParam);
			SaveData(pVec3->GetVec3())
		}
		break;

		case nv_sys::NVTYPEID_VEC2:
		{
			IConnection_Vec2* pVec2 = static_cast<IConnection_Vec2*>(pParam);
			SaveData(pVec2->GetVec2())
		}
		break;

		case nv_sys::NVTYPEID_TEXTURE:
		{
			IConnection_Texture* pTexture = static_cast<IConnection_Texture*>(pParam);
			std::string strTexture = m_pVS->GetTextureName(pTexture->GetTexture());
			SaveData(strTexture);

		}
		break;

		case nv_sys::NVTYPEID_BOOL:
		{
			IConnection_Bool* pBool = static_cast<IConnection_Bool*>(pParam);
			SaveData(pBool->GetBool())
		}
		break;

		case nv_sys::NVTYPEID_FLOAT:
		{
			IConnection_Float* pFloat = static_cast<IConnection_Float*>(pParam);
			SaveData(pFloat->GetFloat());
		}
		break;
		.......

	}
}

  */











