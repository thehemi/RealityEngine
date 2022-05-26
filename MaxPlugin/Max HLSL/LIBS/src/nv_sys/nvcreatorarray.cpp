/*********************************************************************NVMH4****
Path:  NVSDK\Common\src\nv_fx
File:  nvrenderdevice_fx.cpp

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

******************************************************************************/
#ifndef __NVCREATORARRAY_H
#define __NVCREATORARRAY_H

#include "stdafx.h"
#include "nvcreatorarray.h"

namespace nv_sys
{

//DECLARE_NVOBJECT(NVCreatorArray, CLSID_NVCreatorArray, "system.creatorarray", "Creator Array");
/*
INVCreatorArray* CreateNVObject::CreateInstance(){
	void* pObj;
	CreateNVObject(NULL,CLSID_CreateNVObject,&pObj);
	return pObj;
}
*/
bool INTCALLTYPE NVCreatorArray::CreateNVObject(const nv_sys::INVCreator* pCreator, const NVGUID& InterfaceID, void** ppObj)
{
	NVCreatorArray* pObj = new NVCreatorArray;
	
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

NVCreatorArray::NVCreatorArray()
: m_dwRefCount(1)
{

}

unsigned long NVCreatorArray::AddRef()
{
	m_dwRefCount++;
	return m_dwRefCount;
}

unsigned long NVCreatorArray::Release()
{
	unsigned long dwVal = --m_dwRefCount;
	if (dwVal == 0)
		delete this;
	return dwVal;
}

bool NVCreatorArray::GetInterface(const NVGUID& guid, void** ppObj)
{
	if (EqualNVGUID(guid, IID_INVObject))
	{
		*ppObj = static_cast<INVCreatorArray*>(this);
	}
	else if (EqualNVGUID(guid, IID_INVCreatorArray))
	{		
		*ppObj = static_cast<INVCreatorArray*>(this);
	}
	else
	{
		return false;
	}
		
	static_cast<INVCreatorArray*>(this)->AddRef();

	return true;
}

bool NVCreatorArray::AddCreator(INVCreator* pCreator)
{
	m_vecCreator.push_back(pCreator);
	return true;
}

INVCreator* NVCreatorArray::GetCreator(unsigned int Index)
{
	if (Index < m_vecCreator.size())
	{
		return m_vecCreator[Index];
	}
	return NULL;
}

unsigned int NVCreatorArray::GetNumCreators()
{
	return m_vecCreator.size();
}

}; // namespace nv_sys

