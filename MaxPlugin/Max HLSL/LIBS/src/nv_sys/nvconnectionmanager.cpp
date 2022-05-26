/*********************************************************************NVMH4****
Path:  NVSDK\Common\src\nv_sys
File:  NVConnectionManager.cpp

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


The connection manager is intended to simplify connection of application parameters
to effect parameters in an automated way.  It's a work in progress...

The principal interface/classes are:

  	INVConnectionManager - the Manager interface
  
	INVConnectionParameter - a parameter with a one-one mapping to an effect parameter
	 
	INVParameterList - a collection of parameters associated with a given effect.

	IEffectTemplateParameter - a template for an individual effect parameter.  Contains info
		about how to build a connection parameter, and short-cut stuff such as auto-mapped 
		annotation strings to ID's.

  Usage scenario:

  INVConnectionManager->AddEffect...

  INVConnectionManager->CreateParameterList...

  INVConnectionManager->ApplyEffect(IEffect*, INVParameterList*).

******************************************************************************/

#include "stdafx.h"
#include <algorithm>

using namespace std;
using namespace nv_fx;

namespace nv_sys
{

INVConnectionManager* INVConnectionManager::Create(){
	void* pObj;
	NVConnectionManager::CreateNVObject(NULL,IID_INVConnectionManager,&pObj);
	return (INVConnectionManager*)pObj;
}

bool INTCALLTYPE NVConnectionManager::CreateNVObject(const nv_sys::INVCreator* pCreator, const NVGUID& InterfaceID, void** ppObj)
{
	NVConnectionManager* pObj = new NVConnectionManager;
	
	if (pObj)
	{
		if (pObj->GetInterface(InterfaceID, ppObj))
		{
			pObj->Release();
			return true;
		}
	
		delete pObj;
	}

	return false;
}

NVConnectionManager::NVConnectionManager()
: m_dwRefCount(1)
{
}

NVConnectionManager::~NVConnectionManager()
{
	// Should all be gone
	NVLOG_DEBUG(3, "~NVConnectionManager:");
	
#ifdef _DEBUG
	Dump();
	
	// Check that all template params have gone
	assert(m_mapEffectTemplateParameters.empty());
#endif
}

unsigned long NVConnectionManager::AddRef()
{
	m_dwRefCount++;
	return m_dwRefCount;
}

unsigned long NVConnectionManager::Release()
{
	unsigned long RefNew = --m_dwRefCount;
	if (RefNew == 0)
		delete this;
	return RefNew;
}

bool NVConnectionManager::Initialize()
{
	return true;
}


// Remove this effect from the list of effect templates.
bool NVConnectionManager::RemoveEffect(LPD3DXEFFECT pEffect)
{
	if (!pEffect)
		return false;

	NVLOG_DEBUG(3, "NVConnectionManager::RemoveEffect");

	tmapEffectTemplateParameters::iterator itrEffect = m_mapEffectTemplateParameters.find(pEffect);
	if (itrEffect == m_mapEffectTemplateParameters.end())
	{
		return false;
	}
	tvecEffectTemplateParameters& vecEffectParams = itrEffect->second;
	tvecEffectTemplateParameters::iterator itrParams = vecEffectParams.begin();
	while (itrParams != vecEffectParams.end())
	{
		SAFE_RELEASE((*itrParams));
		itrParams++;
	}

	m_mapEffectTemplateParameters.erase(itrEffect);

	return true;
}

// Apply the effect and the parameter list
bool NVConnectionManager::ApplyEffect(LPD3DXEFFECT pEffect, INVParameterList* pParamList)
{
	if (!pEffect || !pParamList)
		return false;

	tmapEffectTemplateParameters::iterator itrEffect = m_mapEffectTemplateParameters.find(pEffect);
	if (itrEffect == m_mapEffectTemplateParameters.end())
	{
		return false;
	}

	tvecEffectTemplateParameters& vecEffectParams = itrEffect->second;
	assert(vecEffectParams.size() >= pParamList->GetNumParameters());

	if (vecEffectParams.size() < pParamList->GetNumParameters())
		return false;

	for (unsigned int i = 0; i < pParamList->GetNumParameters(); i++)
	{
		INVConnectionParameter* pParam = pParamList->GetConnectionParameter(i);
		if (pParam && vecEffectParams[i])
			vecEffectParams[i]->ApplyConnection(m_pCallback, pParam);
	}

	return true;
}

// For setting/retrieving the callback info
INVEffectParamInitCallback* NVConnectionManager::GetEffectParamInitCallback()
{
	return m_pCallback;
}

bool NVConnectionManager::SetEffectParamInitCallback(INVEffectParamInitCallback* pCallback)
{
	m_pCallback = pCallback;
	return true;
}

bool NVConnectionManager::GetInterface(const nv_sys::NVGUID& guid, void** ppObj)
{
	if (EqualNVGUID(guid, IID_INVObject))
	{
		*ppObj = static_cast<INVConnectionManager*>(this);
	}
	else if (EqualNVGUID(guid, IID_INVConnectionManager))
	{		
		*ppObj = static_cast<INVConnectionManager*>(this);
	}
	else
	{
		return false;
	}
		
	static_cast<INVConnectionManager*>(this)->AddRef();

	return true;
}
// Create a parameter list instance.  A unique set of values for this effect, based 
// on the start of day loaded values.
INVParameterList* NVConnectionManager::CreateParameterList(LPD3DXEFFECT pEffect)
{
	tmapEffectTemplateParameters::iterator itrEffect = m_mapEffectTemplateParameters.find(pEffect);
	if (itrEffect == m_mapEffectTemplateParameters.end())
	{
		assert(!"Didn't find effect!");
		return 0;
	}

	NVParameterList* pParamList = new NVParameterList;


	// First build up our list of effect parameters, and pointers to the template effect param
	tvecEffectTemplateParameters& vecEffectTemplateParams = itrEffect->second;
	tvecEffectTemplateParameters::iterator itrParams = vecEffectTemplateParams.begin();
	while (itrParams != vecEffectTemplateParams.end())
	{
		// Create a parameter from the template
		pParamList->AddConnectionParameter((*itrParams)->CreateConnectionParameter());

		itrParams++;
	}


	return static_cast<INVParameterList*>(pParamList);
}

// Create a template for an effect
bool NVConnectionManager::AddEffect(LPD3DXEFFECT pEffect)
{
	if (!pEffect)
		return false;

	// Add the effect to the template list so we can  
	// generate parameter lists for it.
	NVLOG_DEBUG(4, "CEffectTemplate::AddEffect");

	tmapEffectTemplateParameters::iterator itrEffect = m_mapEffectTemplateParameters.find(pEffect);
	if (itrEffect != m_mapEffectTemplateParameters.end())
	{
		// Don't think you should ever add an effect twice
		assert(!"Added an effect template twice, with the same pointer");
		return false;
	}

	D3DXEFFECT_DESC edesc;
	if (pEffect->GetDesc(&edesc) != S_OK)
		return false;

	unsigned int i;
	tvecEffectTemplateParameters vecEffectParameters;
	for (i=0;i<edesc.Parameters;++i)
	{
		// TIM: Don't include params that aren't annotated
		NVFXHANDLE hParam = pEffect->GetParameter(NULL, i);
		D3DXPARAMETER_DESC pdesc;
		if (FAILED(pEffect->GetParameterDesc(hParam,&pdesc)))
		{
			assert(!"Could not get parameter desc!");
			continue;
		}
		// No semantic, no annotation, no service
		// NOTE: The GUI hides items if they have no annotations, period
		// But the plugin needs semantic objects for things like world matrices
		if(pdesc.Annotations == 0 && pdesc.Semantic == 0 )
			continue;
 
        INVEffectTemplateParameter* pParam = NVEffectTemplateParameter::CreateInstance();
		if (pParam->Setup(this, pEffect, i))
		{
			vecEffectParameters.push_back(pParam);
		}
	}

	// Insert the parameters into the manager's map.
	m_mapEffectTemplateParameters.insert(tmapEffectTemplateParameters::value_type(pEffect, vecEffectParameters));

	return true;
}

// Useful dumping function
void NVConnectionManager::Dump()
{
	NVLOG_DEBUG(0, "NVConnectionManager::Dump");

	// Dump our list of parameters
	tmapEffectTemplateParameters::iterator itrEffect = m_mapEffectTemplateParameters.begin();
	while (itrEffect != m_mapEffectTemplateParameters.end())
	{
		D3DXEFFECT_DESC Desc;
		itrEffect->first->GetDesc(&Desc);

		NVLOG_DEBUG(0, "Effect: " << itrEffect->first << ", Params: " << Desc.Parameters);
		
		tvecEffectTemplateParameters::iterator itrParams = itrEffect->second.begin();
		while (itrParams != itrEffect->second.end())
		{
            INVEffectTemplateParameter* pParam = (*itrParams);
			pParam->Dump();
			itrParams++;
		}
		itrEffect++;
	}
}

}; // nv_sys


