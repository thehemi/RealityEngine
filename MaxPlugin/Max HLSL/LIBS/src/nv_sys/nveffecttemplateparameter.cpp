/*********************************************************************NVMH4****
Path:  NVSDK\Common\src\nv_sys
File:  EffectTemplateParameter.cpp

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


Template parameters are created as the effect is added to the connection manager
They can be used to create conneciton parameters based on effect params.
Template parameter is also useful because it always holds the default value of 
an effect parameter.
The template parameter holds per-effect-param-global data, so the connectionparamters
are really lightweight and have a back-pointer to the template param.

******************************************************************************/

#include "stdafx.h"

using namespace std;
using namespace nv_fx;
using namespace nv_renderdevice;

namespace nv_sys
{

INVEffectTemplateParameter* NVEffectTemplateParameter::CreateInstance()
{
	return static_cast<INVEffectTemplateParameter*>(new NVEffectTemplateParameter);
}

NVEffectTemplateParameter::NVEffectTemplateParameter()
: m_dwRefCount(1),
m_pEffect(NULL)
{
	INVObjectSemantics* pSemantics = INVObjectSemantics::Create();
	m_Value.SetObjectSemantics(pSemantics);
	SAFE_RELEASE(pSemantics);
}

NVEffectTemplateParameter::~NVEffectTemplateParameter()
{
	NVLOG_DEBUG(6, "~NVEffectTemplateParameter");
}

unsigned long NVEffectTemplateParameter::AddRef()
{
	m_dwRefCount++;
	return m_dwRefCount;
}

unsigned long NVEffectTemplateParameter::Release()
{
	unsigned long RefNew = --m_dwRefCount;
	if (RefNew == 0)
		delete this;
	return RefNew;
}

INVConnectionParameter* NVEffectTemplateParameter::CreateConnectionParameter()
{
	INVConnectionParameter* pParam = NULL;
	INVInterpolator* pInterpolator = NULL;
	
	switch(m_Value.GetType())
	{
		case NVTYPEID_TEXTURE:
		case NVTYPEID_TEXTURE2D:
		case NVTYPEID_TEXTURE3D:
		case NVTYPEID_TEXTURECUBE:
		case NVTYPEID_SAMPLER:
		case NVTYPEID_SAMPLER2D:
		case NVTYPEID_SAMPLER3D:
		case NVTYPEID_SAMPLERCUBE:
		case NVTYPEID_VERTEXSHADER:
		case NVTYPEID_PIXELSHADER:
		case NVTYPEID_VERTEXFRAGMENT:
		case NVTYPEID_PIXELFRAGMENT:
		case NVTYPEID_STRING:
		case NVTYPEID_VOID:
			pInterpolator = static_cast<INVInterpolator*>(new nv_sys::NVStandardInterpolator(INTERPOLATOR_CONSTANT));
			break;
		case NVTYPEID_MATRIX:
			pInterpolator = static_cast<INVInterpolator*>(new nv_sys::NVStandardInterpolator(INTERPOLATOR_STEP));
			break;
		default:
			pInterpolator = static_cast<INVInterpolator*>(new nv_sys::NVStandardInterpolator(INTERPOLATOR_LINEAR));
			break;
	}

	// CM for now, a single key.
	pParam = INVConnectionParameter::Create();

	pParam->SetDefaultValue(m_Value);
	pParam->SetInterpolator(pInterpolator);
	SAFE_RELEASE(pInterpolator);

	pParam->SetToDefaultValue();

	return static_cast<INVConnectionParameter*>(pParam);
}

const D3DXPARAMETER_DESC* NVEffectTemplateParameter::GetDesc() const
{
	return m_pParamDesc.get();
}

bool NVEffectTemplateParameter::ApplyConnection(INVEffectParamInitCallback* pCallback, INVConnectionParameter* pParam)
{
	assert(m_pEffect);
	assert(pParam);
	assert(m_pParamDesc.get());


	NVType KeyValue = pParam->GetValueAtTime(GetSYSInterface()->GetTime());

	//Don't send strings.	
	//HLSL doesn't seem to like it
	if (pParam->GetDefaultValue().GetType() == NVTYPEID_STRING)
		return true;

	// We don't handle arrays
	if(m_pParamDesc->Elements > 1) // 0
		return true;

	// TIM: Switched to handles
	switch(KeyValue.GetType())
	{
		case NVTYPEID_DWORD:
			m_pEffect->SetInt(m_pParamHandle, KeyValue.GetDWORD());
			break;
		case NVTYPEID_FLOAT:
			m_pEffect->SetFloat(m_pParamHandle, KeyValue.GetFloat());
			break;
		case NVTYPEID_VEC4:
			m_pEffect->SetVector(m_pParamHandle, (D3DXVECTOR4*)&KeyValue.GetVec4().x);
			break;
		case NVTYPEID_VEC3:
			m_pEffect->SetVector(m_pParamHandle, &D3DXVECTOR4(KeyValue.GetVec3().x,KeyValue.GetVec3().y,KeyValue.GetVec3().z,0));
			break;
		case NVTYPEID_VEC2:
			m_pEffect->SetVector(m_pParamHandle, &D3DXVECTOR4(KeyValue.GetVec2().x,KeyValue.GetVec2().y,0,0));
			break;
		case NVTYPEID_BOOL:
			m_pEffect->SetBool(m_pParamHandle, KeyValue.GetBool());
			break;
		case NVTYPEID_MATRIX:
			m_pEffect->SetMatrix(m_pParamHandle, (D3DXMATRIX*)KeyValue.GetMatrix());
			break;
		case NVTYPEID_TEXTURE:
			if (KeyValue.GetTexture())
				m_pEffect->SetTexture(m_pParamHandle, (LPDIRECT3DBASETEXTURE9)KeyValue.GetTexture()->GetTextureHandle());
			break;
		case NVTYPEID_TEXTURE2D:
			if (KeyValue.GetTexture())
				m_pEffect->SetTexture(m_pParamHandle, (LPDIRECT3DBASETEXTURE9)KeyValue.GetTexture()->GetTextureHandle());
			break;
		case NVTYPEID_TEXTURE3D:
			if (KeyValue.GetTexture())
				m_pEffect->SetTexture(m_pParamHandle, (LPDIRECT3DBASETEXTURE9)KeyValue.GetTexture()->GetTextureHandle());
			break;
		case NVTYPEID_TEXTURECUBE:
			if (KeyValue.GetTexture())
				m_pEffect->SetTexture(m_pParamHandle, (LPDIRECT3DBASETEXTURE9)KeyValue.GetTexture()->GetTextureHandle());
			break;
		default:
			break;
	}

	return true;
}

bool NVEffectTemplateParameter::Setup(INVConnectionManager* pConnectionManager, LPD3DXEFFECT pEffect, const unsigned int iParameter)
{
	NVLOG_DEBUG(5, "NVEffectTemplateParameter::Setup, Param: " << iParameter);

	m_pEffect = pEffect;

	NVFXHANDLE hParam = pEffect->GetParameter(NULL, iParameter);

	D3DXPARAMETER_DESC pdesc;
	//NVFXPARAMETER_DESC pdesc;
	if (FAILED(pEffect->GetParameterDesc(hParam,&pdesc)))
	{
		assert(!"Could not get parameter desc!");
		return false;
	}

	// At least 1 D.
	if (pdesc.Columns == 0)
		pdesc.Columns = 1;

	m_pParameter = auto_ptr<NVCgFXType>(new NVCgFXType(pdesc, NULL));

	m_pParamDesc = auto_ptr<D3DXPARAMETER_DESC>(new D3DXPARAMETER_DESC);
	memcpy(m_pParamDesc.get(), &pdesc, sizeof(D3DXPARAMETER_DESC));

	m_pParamHandle = pEffect->GetParameterByName(0,m_pParamDesc->Name);

	int Rows = m_pParamDesc->Rows;//Dimension[0];
	int Columns = m_pParamDesc->Columns;//Dimension[1];

	eNVTYPEID TypeID; 
	switch(m_pParamDesc->Type)
	{
		case D3DXPT_INT:
			// NOTE: Using DWORD because NVIDIA has poor support for ints

			// Ints
			if (m_pParamDesc->Columns == 1)
			{
				TypeID = NVTYPEID_DWORD;
			}
			else
			{
				assert(!"Not implemented int arrays");
			}
			break;
		case D3DXPT_STRING:
		{
			// Ints
			if (m_pParamDesc->Columns == 1)
			{
				TypeID = NVTYPEID_STRING;
			}
			else
			{
				assert(!"Not implemented string arrays");
			}
		}
		break;

/*		case D3DXPT_STRUCT:
		{
			assert(!"Not implemented structs");
			break;
		}
		break;*/

/*		case D3DXPT_DWORD:
		{
			assert(m_pParamDesc->Columns == 1);
			TypeID = NVTYPEID_DWORD;
		}
		break;*/

		case D3DXPT_FLOAT:
			// Float?
			if (m_pParamDesc->Rows == 1)
			{
				if (m_pParamDesc->Columns == 1)
				{
					TypeID = NVTYPEID_FLOAT;
				}
				else if (m_pParamDesc->Columns == 2)
				{
					TypeID = NVTYPEID_VEC2;
				}
				else if (m_pParamDesc->Columns == 3)
				{
					TypeID = NVTYPEID_VEC3;
				}
				else if (m_pParamDesc->Columns == 4)
				{
					TypeID = NVTYPEID_VEC4;
				}
				else
				{
					assert(!"Not implemented type!");
				}
			}	
			// Matrix
			else
			{
				TypeID = NVTYPEID_MATRIX;
			}
			break;
		case D3DXPT_BOOL:
			// Bool?
			if (m_pParamDesc->Columns == 1)
			{
				TypeID = NVTYPEID_BOOL;
			}
			else
			{
				assert(!"Unknown bool array!");
			}
			break;
		case D3DXPT_TEXTURE:
			// Texture?
			if (m_pParamDesc->Columns == 1)
			{
				TypeID = NVTYPEID_TEXTURE2D;
			}
			else
			{
				assert(!"Unknown texture array!");
			}
			break;

		case D3DXPT_TEXTURECUBE:
			// Texture?
			if (m_pParamDesc->Columns == 1)
			{
				TypeID = NVTYPEID_TEXTURECUBE;
			}
			else
			{
				assert(!"Unknown texture array!");
			}
			break;
		case D3DXPT_TEXTURE3D:
			// Texture?
			if (m_pParamDesc->Columns == 1)
			{
				TypeID = NVTYPEID_TEXTURE3D;
			}
			else
			{
				assert(!"Unknown texture array!");
			}
			break;
		case D3DXPT_VERTEXSHADER:
			// Vertex shader?
			if (m_pParamDesc->Columns == 1)
			{
				TypeID = NVTYPEID_VERTEXSHADER;
			}
			else
			{
				assert(!"Unknown vertex shader array!");
			}
			break;
		case D3DXPT_PIXELSHADER:
			// Vertex shader?
			if (m_pParamDesc->Columns == 1)
			{
				TypeID = NVTYPEID_PIXELSHADER;
			}
			else
			{
				assert(!"Unknown pixel shader array!");
			}
			break;
		case D3DXPT_SAMPLER:
		case D3DXPT_SAMPLER1D:
			// Sampler?
			if (m_pParamDesc->Columns == 1)
			{
				TypeID = NVTYPEID_SAMPLER;
			}
			else
			{
				assert(!"Unknown pixel shader array!");
			}
			break;

		case D3DXPT_SAMPLER2D:
			// Sampler?
			if (m_pParamDesc->Columns == 1)
			{
				TypeID = NVTYPEID_SAMPLER2D;
			}
			else
			{
				assert(!"Unknown pixel shader array!");
			}
			break;

		case D3DXPT_SAMPLER3D:
			// Sampler?
			if (m_pParamDesc->Columns == 1)
			{
				TypeID = NVTYPEID_SAMPLER3D;
			}
			else
			{
				assert(!"Unknown pixel shader array!");
			}
			break;

		case D3DXPT_SAMPLERCUBE:
			// Sampler?
			if (m_pParamDesc->Columns == 1)
			{
				TypeID = NVTYPEID_SAMPLERCUBE;
			}
			else
			{
				assert(!"Unknown pixel shader array!");
			}
			break;
		case D3DXPT_PIXELFRAGMENT:
			// Sampler?
			if (m_pParamDesc->Columns == 1)
			{
				TypeID = NVTYPEID_PIXELFRAGMENT;
			}
			else
			{
				assert(!"Unknown pixel fragment array!");
			}
			break;
		case D3DXPT_VERTEXFRAGMENT:
			// Sampler?
			if (m_pParamDesc->Columns == 1)
			{
				TypeID = NVTYPEID_VERTEXFRAGMENT;
			}
			else
			{
				assert(!"Unknown vertex fragment array!");
			}
			break;

		default:
			NVLOG_DEBUG(0, "WARNING: Unknown type in connection manager");
			return false;

			//assert(!"Unrecognized type!");
			break;
	}


	m_Value.GetObjectSemantics()->SetSemanticID(ConvertSemantic(pdesc.Semantic));
	m_Value.GetObjectSemantics()->SetName(m_pParamDesc->Name);

	if (TypeID == NVTYPEID_STRING)
	{
		const char* pString = NULL;
		pEffect->GetString(pdesc.Name, &pString);
		if (pString)
		{
			StreamNVType(m_Value, TypeID, (void*)pString, Rows, Columns);
		}
	}
	else
	{
		pEffect->GetValue(pdesc.Name, m_pParameter->m_pType, m_pParameter->m_dwSize);

		// Get the value into our locally copied parameter.
		// If the app updated it during create, then we will automatically pick up the copy.
		StreamNVType(m_Value, TypeID, m_pParameter->m_pType, Rows, Columns);
	}

	
	// For each of the internal connection parameters.
	for (unsigned int j=0;j<pdesc.Annotations;++j)
	{
		D3DXPARAMETER_DESC adesc;
		pEffect->GetParameterDesc(pEffect->GetAnnotation(hParam,j),&adesc);

		// Annotations we care about are strings.
		// Will only add if the annotation is something we understand...
		switch(adesc.Type)
        {
            case D3DXPT_STRING:
		    {
				LPCSTR Value;
				pEffect->GetString(pEffect->GetAnnotation(hParam,j),&Value);

			    eANNOTATIONVALUEID ValueID = ConvertAnnotationValue(Value);
			    eANNOTATIONNAMEID NameID = ConvertAnnotationName(adesc.Name);

			    if (NameID != ANNOTATIONNAMEID_UNKNOWN)
			    {
				    if (ValueID == ANNOTATIONVALUEID_UNKNOWN)
				    {
						m_Value.GetObjectSemantics()->AddAnnotation(NameID, NVType::CreateStringType(Value));
				    }
				    else
				    {
					    m_Value.GetObjectSemantics()->AddAnnotation(NameID, ValueID);
				    }
			    }
                break;
		    }
            case D3DXPT_FLOAT:
            {
			    eANNOTATIONVALUEID ValueID = ANNOTATIONVALUEID_FLOAT;
			    eANNOTATIONNAMEID NameID = ConvertAnnotationName(adesc.Name);
			    if (NameID != ANNOTATIONNAMEID_UNKNOWN)
			    {
					float Value;
					pEffect->GetFloat(pEffect->GetAnnotation(hParam,j),&Value);

                    m_Value.GetObjectSemantics()->AddAnnotation(NameID, NVType::CreateFloatType(Value));
                }
                break;
            }
            default:
                break;
        }
	}

	INVEffectParamInitCallback* pCallback = pConnectionManager->GetEffectParamInitCallback();
	if (pCallback)
		pCallback->EffectParamInit(pEffect, m_Value);

	return true;
}

void NVEffectTemplateParameter::Dump()
{
	NVLOG_DEBUG(0, "EffectTemplateParam: Name=" << m_pParamDesc->Name << ", Semantic=" << ConvertSemantic(m_Value.GetObjectSemantics()->GetSemanticID()));
}

}; // namespace nv_sys