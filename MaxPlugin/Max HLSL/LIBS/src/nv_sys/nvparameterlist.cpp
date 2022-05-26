/*********************************************************************NVMH4****
Path:  NVSDK\Common\src\nv_sys
File:  ParameterList.cpp

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

Maintains a list of connection parameters.



******************************************************************************/
#include "stdafx.h"
using namespace nv_fx;
namespace nv_sys
{

INVParameterList* INVParameterList::Create(){
	void* pObj;
	NVParameterList::CreateNVObject(NULL,IID_INVParameterList,&pObj);
	return (INVParameterList*)pObj;
}
/*
	INVParameterList* INVParameterList::Create(){
		return new NVParameterList();
	}
*/
bool INTCALLTYPE NVParameterList::CreateNVObject(const nv_sys::INVCreator* pCreator, const NVGUID& InterfaceID, void** ppObj)
{
	NVParameterList* pObj = new NVParameterList;
	
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

NVParameterList::NVParameterList()
: m_dwRefCount(1)
{

}

NVParameterList::~NVParameterList()
{
	NVLOG_DEBUG(3, "~NVParameterList");
	tvecParameterInfo::iterator itrParams = m_vecParameterInfo.begin();
	while (itrParams != m_vecParameterInfo.end())
	{
		// Release the connection parameter
		SAFE_RELEASE(*itrParams);
		itrParams++;
	}	
}

unsigned long NVParameterList::AddRef()
{
	m_dwRefCount++;
	return m_dwRefCount;
}

unsigned long NVParameterList::Release()
{
	unsigned long RefNew = --m_dwRefCount;
	if (RefNew == 0)
		delete this;
	return RefNew;
}

// Create a copy of self.
bool NVParameterList::Clone(const NVGUID& Interface, void** ppObj) const
{
	INVParameterList* pParamList = INVParameterList::Create();
 
	for (unsigned int i = 0; i < GetNumParameters(); i++)
	{
		INVConnectionParameter* pConnectionParameter = GetConnectionParameter(i);
		
		// Get a clone interface from the connection parameter
		INVClone* pClone = NULL;
		pConnectionParameter->GetInterface(IID_INVClone, (void**)&pClone);
		
		// Clone it
		INVConnectionParameter* pNewConnectionParameter = NULL;
		if (pClone->Clone(IID_INVConnectionParameter, (void**)&pNewConnectionParameter))
		{
			pParamList->AddConnectionParameter(pNewConnectionParameter);		
		}

		SAFE_RELEASE(pClone);
		
	}

	// Return the new interface.
	bool bRet = pParamList->GetInterface(Interface, ppObj);

	SAFE_RELEASE(pParamList);

	return bRet;
}

bool NVParameterList::GetInterface(const NVGUID& guid, void** ppObj)
{
	if (EqualNVGUID(guid, IID_INVObject))
	{
		*ppObj = static_cast<INVParameterList*>(this);
	}
	else if (EqualNVGUID(guid, IID_INVParameterList))
	{		
		*ppObj = static_cast<INVParameterList*>(this);
	}
	else if (EqualNVGUID(guid, IID_INVClone))
	{		
		*ppObj = static_cast<INVClone*>(this);
	}
	else
	{
		return false;
	}
		
	static_cast<INVParameterList*>(this)->AddRef();
	return true;
}

unsigned int NVParameterList::GetNumParameters() const
{
	return m_vecParameterInfo.size();
}

INVConnectionParameter* NVParameterList::GetConnectionParameter(unsigned int ParamNum) const
{
	if (m_vecParameterInfo.size() <= ParamNum)
		return NULL;

	return m_vecParameterInfo[ParamNum];
}

bool NVParameterList::AddConnectionParameter(INVConnectionParameter* pParam)
{
	m_vecParameterInfo.push_back(pParam);
	return true;
}

}; // namespace nv_sys