#include "stdafx.h"

using namespace std;
using namespace nv_fx;

namespace nv_sys
{

	INVConnectionParameter* INVConnectionParameter::Create(){
		void* pObj;
		NVConnectionParameter::CreateNVObject(NULL,IID_INVConnectionParameter,&pObj);
		return (INVConnectionParameter*)pObj;
	}

NVConnectionParameter::NVConnectionParameter()
	: m_dwRefCount(1),
	m_pInterpolator(NULL),
	m_bAnimate(false)
{
}

NVConnectionParameter::~NVConnectionParameter()
{
	NVLOG_DEBUG(6, "~NVConnectionParameter");
	SAFE_RELEASE(m_pInterpolator);
}

bool INTCALLTYPE NVConnectionParameter::CreateNVObject(const nv_sys::INVCreator* pCreator, const NVGUID& InterfaceID, void** ppObj)
{
	NVConnectionParameter* pObj = new NVConnectionParameter;
	
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

// Create a copy of self.
bool NVConnectionParameter::Clone(const NVGUID& Interface, void** ppObj) const
{
	INVConnectionParameter* pParam = INVConnectionParameter::Create();

	// Copy the default value.
	pParam->SetDefaultValue(GetDefaultValue());

	// Addref the interpolator
	pParam->SetInterpolator(m_pInterpolator);
	SAFE_ADDREF(m_pInterpolator);

	// Copy the keys
	tmapTimeToKeyIndex::const_iterator itrTimeToKey = m_mapTimeToKeyIndex.begin();
	while (itrTimeToKey != m_mapTimeToKeyIndex.end())
	{
		pParam->SetKey(itrTimeToKey->first, m_vecKeys[itrTimeToKey->second]);
		itrTimeToKey++;
	}

	bool bRet = pParam->GetInterface(Interface, ppObj);

	SAFE_RELEASE(pParam);

	return bRet;
}


unsigned long NVConnectionParameter::AddRef()
{
	m_dwRefCount++;
	return m_dwRefCount;
}

unsigned long NVConnectionParameter::Release()
{
	unsigned long RefNew = --m_dwRefCount;
	if (RefNew == 0)
		delete this;
	return RefNew;
}

bool NVConnectionParameter::GetInterface(const NVGUID& guid, void** ppObj)
{
	if (EqualNVGUID(guid, IID_INVObject))
	{
		*ppObj = static_cast<INVConnectionParameter*>(this);
	}
	else if (EqualNVGUID(guid, IID_INVConnectionParameter))
	{		
		*ppObj = static_cast<INVConnectionParameter*>(this);
	}
	else if (EqualNVGUID(guid, IID_INVClone))
	{		
		*ppObj = static_cast<INVClone*>(this);
	}
	else
	{
		return false;
	}
		
	static_cast<INVConnectionParameter*>(this)->AddRef();
	return true;
}

// Animation support
unsigned int NVConnectionParameter::GetNumKeys() const
{
	return m_vecKeys.size();
}

const nv_sys::NVType* NVConnectionParameter::GetKeyFromIndex(unsigned int Index, int& Time) const
{
	if (m_vecKeys.empty() || (m_vecKeys.size() <= Index))
		return NULL;

	tmapKeyIndexToTime::const_iterator itrFound = m_mapKeyIndexToTime.find(Index);
	assert(itrFound != m_mapKeyIndexToTime.end());

	Time = itrFound->second;

	return &m_vecKeys[Index];
}

bool NVConnectionParameter::GetSortedTimeIndices(unsigned int* pIndices, unsigned int NumIndices) const
{
	unsigned int CurrentIndex = 0;

	tmapTimeToKeyIndex::const_iterator itrTime = m_mapTimeToKeyIndex.begin();
	while ((CurrentIndex < NumIndices) && itrTime != m_mapTimeToKeyIndex.end())
	{
		pIndices[CurrentIndex] = itrTime->second;
		itrTime++;
		CurrentIndex++;
	}
	return true;
}

bool NVConnectionParameter::DeleteAllKeys() 
{
	if (m_vecKeys.empty())
		return true;

	m_vecKeys.clear();
	m_mapTimeToKeyIndex.clear();
	m_mapKeyIndexToTime.clear();

	return true;
}

bool NVConnectionParameter::SetKey(int Time, const nv_sys::NVType& Value)
{
	if (!m_bAnimate)
		Time = 0;

	// Found it.
	tmapTimeToKeyIndex::iterator itrFound = m_mapTimeToKeyIndex.find(Time);
	if (itrFound != m_mapTimeToKeyIndex.end())
	{
		m_vecKeys[itrFound->second] = Value;
		return true;
	}

	// Add it.
	m_vecKeys.push_back(Value);
	m_mapTimeToKeyIndex[Time] = m_vecKeys.size() - 1;
	m_mapKeyIndexToTime[m_vecKeys.size() - 1] = Time;

	return true;
}

nv_sys::NVType NVConnectionParameter::GetValueAtTime(int Time) const
{
	if (m_vecKeys.empty())
	{
		return m_DefaultValue;
	}

	if (!m_bAnimate)
	{
		tmapTimeToKeyIndex::const_iterator itrFound = m_mapTimeToKeyIndex.find(0);
		if (itrFound == m_mapTimeToKeyIndex.end())
		{
			return m_DefaultValue;
		}
		else
		{
			return m_vecKeys[itrFound->second];
		}
	}
	
	return m_pInterpolator->Interpolate(this, Time);
}

void NVConnectionParameter::Animate(bool bValue) 
{
	m_bAnimate = bValue;
}

void NVConnectionParameter::SetToDefaultValue()
{
	if (m_bAnimate)
	{
		SetKey(GetSYSInterface()->GetTime(), m_DefaultValue);
	}
	else
	{
		SetKey(0, m_DefaultValue);
	}
}

const NVType& NVConnectionParameter::GetDefaultValue() const
{
	return m_DefaultValue;
}

bool NVConnectionParameter::SetDefaultValue(const NVType& DefaultValue)
{
	m_DefaultValue = DefaultValue;
	return true;
}

bool NVConnectionParameter::SetInterpolator(INVInterpolator* pInterpolator)
{
	SAFE_RELEASE(m_pInterpolator);

	m_pInterpolator = pInterpolator;
	if (pInterpolator)
		pInterpolator->AddRef();

	return true;
}

}; // namespace nv_sys