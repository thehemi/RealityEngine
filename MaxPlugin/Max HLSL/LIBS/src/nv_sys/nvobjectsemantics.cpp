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
#ifndef __NVOBJECTSEMANTICS_H
#define __NVOBJECTSEMANTICS_H

#include "stdafx.h"
#include "nvobjectsemantics.h"

namespace nv_sys
{

INVObjectSemantics* INVObjectSemantics::Create(){
	void* pObj;
	NVObjectSemantics::CreateNVObject(NULL,IID_INVObjectSemantics,&pObj);
	return (INVObjectSemantics*)pObj;
}
/*
	INVObjectSemantics* INVObjectSemantics::Create(){
		return new NVObjectSemantics();
	}
*/
bool INTCALLTYPE NVObjectSemantics::CreateNVObject(const nv_sys::INVCreator* pCreator, const NVGUID& InterfaceID, void** ppObj)
{
	NVObjectSemantics* pObj = new NVObjectSemantics;
	
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

NVObjectSemantics::NVObjectSemantics()
: m_dwRefCount(1)
{

}

unsigned long NVObjectSemantics::AddRef()
{
	m_dwRefCount++;
	return m_dwRefCount;
}

unsigned long NVObjectSemantics::Release()
{
	unsigned long dwVal = --m_dwRefCount;
	if (dwVal == 0)
		delete this;
	return dwVal;
}

bool NVObjectSemantics::GetInterface(const NVGUID& guid, void** ppObj)
{
	if (EqualNVGUID(guid, IID_INVObject))
	{
		*ppObj = static_cast<INVObjectSemantics*>(this);
	}
	else if (EqualNVGUID(guid, IID_INVObjectSemantics))
	{		
		*ppObj = static_cast<INVObjectSemantics*>(this);
	}
	else if (EqualNVGUID(guid, IID_INVClone))
	{
		*ppObj = static_cast<INVClone*>(this);
	}
	else
	{
		return false;
	}
		
	static_cast<INVObjectSemantics*>(this)->AddRef();

	return true;
}

bool NVObjectSemantics::Clone(const NVGUID& Interface, void** ppObj) const
{
	NVObjectSemantics* pObject= new NVObjectSemantics();
		pObject->m_SemanticID = m_SemanticID;
		pObject->m_strName = m_strName;
		pObject->m_mapNameToAnnotationInfo = m_mapNameToAnnotationInfo;

		bool bRet = pObject->GetInterface(Interface, ppObj);

		SAFE_RELEASE(pObject);

		return bRet;

	return false;

}

const tAnnotationInfo* NVObjectSemantics::GetAnnotation(unsigned int Num) const
{
	if (Num >= m_mapNameToAnnotationInfo.size())
		return NULL;

	tmapNameToAnnotationInfo::const_iterator itrAnnotation = m_mapNameToAnnotationInfo.begin();
	for (unsigned int i = 0; i < Num; i++)
	{
		itrAnnotation++;
	}

	return &itrAnnotation->second;
}


eSEMANTICID NVObjectSemantics::GetSemanticID() const
{
	return m_SemanticID;
}

bool NVObjectSemantics::SetSemanticID(eSEMANTICID SemanticID)
{
	m_SemanticID = SemanticID;
	return true;
}

const char* NVObjectSemantics::GetName() const
{
	return m_strName.c_str();
}

bool NVObjectSemantics::SetName(const char* pszName)
{
	m_strName = pszName;
	return true;
}

unsigned int NVObjectSemantics::GetNumAnnotations() const
{
	return m_mapNameToAnnotationInfo.size();
}

eANNOTATIONVALUEID NVObjectSemantics::FindAnnotationValue(eANNOTATIONNAMEID NameID) const
{
	// Find an ID for a named annotation
	std::string strName = ConvertAnnotationName(NameID);
	tmapNameToAnnotationInfo::const_iterator itrAnnotations = m_mapNameToAnnotationInfo.find(strName);
	if (itrAnnotations != m_mapNameToAnnotationInfo.end())
	{
		return itrAnnotations->second.m_ValueID;
	}
	return ANNOTATIONVALUEID_UNKNOWN;
}

bool NVObjectSemantics::AddAnnotation(eANNOTATIONNAMEID NameID, eANNOTATIONVALUEID ValueID)
{
	// Add an annotation with name ID and value ID
	std::string strName = ConvertAnnotationName(NameID);
	tmapNameToAnnotationInfo::iterator itrAnnotation = m_mapNameToAnnotationInfo.find(strName);
	if (itrAnnotation == m_mapNameToAnnotationInfo.end())
	{
		tAnnotationInfo theAnnotation;
		itrAnnotation = m_mapNameToAnnotationInfo.insert(tmapNameToAnnotationInfo::value_type(strName, theAnnotation)).first;
	}
	else
	{
		itrAnnotation->second.m_Name.SetNULL();
		itrAnnotation->second.m_Value.SetNULL();
	}
	
	itrAnnotation->second.m_NameID = NameID;
	itrAnnotation->second.m_ValueID = ValueID;
	itrAnnotation->second.m_Name = NVType::CreateStringType(strName.c_str());
	itrAnnotation->second.m_Value = NVType::CreateStringType(ConvertAnnotationValue(ValueID));

	return true;
}

bool NVObjectSemantics::AddAnnotation(const char* pszName, const NVType& Value)
{
	// Add with NameID and value type
	tmapNameToAnnotationInfo::iterator itrAnnotation = m_mapNameToAnnotationInfo.find(pszName);
	if (itrAnnotation == m_mapNameToAnnotationInfo.end())
	{
		tAnnotationInfo theAnnotation;
		itrAnnotation = m_mapNameToAnnotationInfo.insert(tmapNameToAnnotationInfo::value_type(pszName, theAnnotation)).first;
	}
	else
	{
		itrAnnotation->second.m_Name.SetNULL();
		itrAnnotation->second.m_Value.SetNULL();
	}
	
	itrAnnotation->second.m_NameID = ConvertAnnotationName(pszName);
	if (Value.GetType() == NVTYPEID_STRING)
		itrAnnotation->second.m_ValueID = ConvertAnnotationValue(Value.GetString());
	else
		itrAnnotation->second.m_ValueID = ANNOTATIONVALUEID_UNKNOWN;

	itrAnnotation->second.m_Name = NVType::CreateStringType(pszName);
	itrAnnotation->second.m_Value = Value;


	return true;
}

bool NVObjectSemantics::AddAnnotation(eANNOTATIONNAMEID NameID, const NVType& Value)
{
	std::string strName = ConvertAnnotationName(NameID);
	tmapNameToAnnotationInfo::iterator itrAnnotation = m_mapNameToAnnotationInfo.find(strName);
	if (itrAnnotation == m_mapNameToAnnotationInfo.end())
	{
		tAnnotationInfo theAnnotation;
		itrAnnotation = m_mapNameToAnnotationInfo.insert(tmapNameToAnnotationInfo::value_type(strName, theAnnotation)).first;
	}
	else
	{
		itrAnnotation->second.m_Name.SetNULL();
		itrAnnotation->second.m_Value.SetNULL();
	}
	
	itrAnnotation->second.m_NameID = NameID;
	if (Value.GetType() == NVTYPEID_STRING)
		itrAnnotation->second.m_ValueID = ConvertAnnotationValue(Value.GetString());
	else
		itrAnnotation->second.m_ValueID = ANNOTATIONVALUEID_UNKNOWN;

	itrAnnotation->second.m_Name = NVType::CreateStringType(strName.c_str());
	itrAnnotation->second.m_Value = Value;

	return true;
}



const tAnnotationInfo* NVObjectSemantics::FindAnnotationInfo(eANNOTATIONNAMEID NameID) const
{
	std::string strName = ConvertAnnotationName(NameID);
	tmapNameToAnnotationInfo::const_iterator itrAnnotations = m_mapNameToAnnotationInfo.find(strName);
	while (itrAnnotations != m_mapNameToAnnotationInfo.end())
	{
		if (itrAnnotations->second.m_NameID == NameID)
			return &(itrAnnotations->second);

		itrAnnotations++;
	}
	return NULL;
}

}; // namespace nv_sys


