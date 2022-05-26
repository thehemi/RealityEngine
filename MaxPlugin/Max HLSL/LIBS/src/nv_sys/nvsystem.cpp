#include "stdafx.h"
#include <sstream>
#include "windows.h"
#include "nvobjectsemantics.h"
#include "nvlog.h"

using namespace std;

namespace nv_sys
{

// The singleton for system management
NVSystemSingleton* g_pSystem = NULL;
extern NVLogSingleton* g_pLog;

SYS_API INVSystem* GetSYSInterface() 
{
	// Ensure the global system is initialized.  Because it might be
	// called during construction time.
	if (!g_pSystem)
		g_pSystem = new NVSystemSingleton;

	return NVSystemSingleton::GetSingletonPtr(); 
}

SYS_API FinalSYSShutdown()
{
	
	delete g_pSystem;
	g_pSystem = NULL;

	delete g_pLog;
	g_pLog = NULL;
}

NVSystemSingleton::NVSystemSingleton()
: m_Time(0)
{
	
}

bool NVSystemSingleton::Exception(const NVException& except)
{
	ostringstream strStream;
	strStream << NVException::ExceptionIDToString(except.GetExceptionID()) << " : " << except.GetExceptionString() << endl;
	OutputDebugString(strStream.str().c_str());
	assert(0);
	return true;
}

unsigned int NVSystemSingleton::GetTime() const
{
	return m_Time;
}

void NVSystemSingleton::SetTime(unsigned int Time)
{
	m_Time = Time;
}

// System singleton doesn't die until the app is finished.
unsigned long NVSystemSingleton::AddRef()
{
	return 1;
}

unsigned long NVSystemSingleton::Release()
{
	return 1;
}

bool NVSystemSingleton::GetInterface(const NVGUID& guid, void** ppObj)
{
	if (EqualNVGUID(guid, IID_INVObject))
	{
		*ppObj = static_cast<INVObject*>(this);
	}
	else if (EqualNVGUID(guid, IID_INVSystem))
	{		
		*ppObj = static_cast<INVSystem*>(this);
	}
	else
	{
		return false;
	}
		
	static_cast<INVObject*>(this)->AddRef();

	return true;
}

bool NVSystemSingleton::RegisterObject(INVCreator* pCreator)
{
	m_mapNVGUIDToCreate[pCreator->GetClass()] = pCreator;
	NVLOG_DEBUG(0, "Registered Object: " << pCreator->GetFriendlyName());

	if (pCreator->GetCategoryName() && !(strlen(pCreator->GetCategoryName()) == 0) )
	{
		// Add the creator to the category list
		tmapCategoryNameToCreators::iterator itrFound = m_mapCategoryNameToCreators.find(pCreator->GetCategoryName());
		if (itrFound != m_mapCategoryNameToCreators.end())
		{
			itrFound->second.push_back(pCreator);
		}
		else
		{
			tlistCreators listCreators;
			listCreators.push_back(pCreator);
			m_mapCategoryNameToCreators[pCreator->GetCategoryName()] = listCreators;
		}

		NVLOG_DEBUG(0, "Category: " << pCreator->GetCategoryName());
	}
	return true;
}

bool NVSystemSingleton::UnRegisterObject(INVCreator* pCreator)
{
	tmapNVGUIDToCreate::iterator itrFound = m_mapNVGUIDToCreate.find(pCreator->GetClass());
	if (itrFound != m_mapNVGUIDToCreate.end())
	{
		// Erase the creator from the category list
		tmapCategoryNameToCreators::iterator itrFoundCategory = m_mapCategoryNameToCreators.find(pCreator->GetCategoryName());
		if (itrFoundCategory != m_mapCategoryNameToCreators.end())
		{
			tlistCreators& listCreators = itrFoundCategory->second;
			tlistCreators::iterator itrCreators = listCreators.begin();
			while (itrCreators != listCreators.end())
			{
				if ((*itrCreators) == pCreator)
				{
					listCreators.erase(itrCreators);
					break;
				}
				itrCreators++;
			}
		}

		NVLOG_DEBUG(0, "UnRegistered Object: " << itrFound->second->GetFriendlyName());
		m_mapNVGUIDToCreate.erase(itrFound);
		return true;
	}
	return false;
}
/*
bool NVSystemSingleton::CreateObject(const NVGUID& ObjectClass, const NVGUID& ObjectInterface, void** ppObj) 
{
	*ppObj = NULL;

	if(EqualNVGUID(ObjectClass,  CLSID_NVConnectionManager))
		*ppObj = new NVConnectionManager;
	if(EqualNVGUID(ObjectClass, CLSID_NVObjectSemantics))
		*ppObj = new NVObjectSemantics;
	if(EqualNVGUID(ObjectClass, CLSID_NVParameterList))
		*ppObj = new NVParameterList;

	/*tmapNVGUIDToCreate::iterator itrFound = m_mapNVGUIDToCreate.find(ObjectClass);
	if (itrFound != m_mapNVGUIDToCreate.end())
	{
		// TIM: Annoying when creating 50 semantics each update
		//NVLOG_DEBUG(3, "Creating Object: " << itrFound->second->GetFriendlyName());
		return itrFound->second->GetCreateFunction()(itrFound->second, ObjectInterface, ppObj);
	}*/
	//return (*ppObj) != NULL;
//}*/

bool NVSystemSingleton::GetCreatorsInCategory(const char* pszCategory, INVCreatorArray** ppArray)
{
	MessageBox(0,"GetCreatorsInCategory() is Unsupported","",0);
	/*
	tmapCategoryNameToCreators::const_iterator itrFoundCategory = m_mapCategoryNameToCreators.find(pszCategory);	
	if (itrFoundCategory == m_mapCategoryNameToCreators.end())
		return false;

	if (!CreateObject(CLSID_NVCreatorArray, IID_INVCreatorArray, (void**)ppArray))
		return false;

	tlistCreators::const_iterator itrCreators = itrFoundCategory->second.begin();
	while (itrCreators != itrFoundCategory->second.end())
	{
		(*ppArray)->AddCreator(*itrCreators);
		itrCreators++;
	}
*/
	return true;
}

const INVCreator* NVSystemSingleton::GetCreatorClass(const NVGUID& ClassID) const
{
	tmapNVGUIDToCreate::const_iterator itrFound = m_mapNVGUIDToCreate.find(ClassID);
	if (itrFound != m_mapNVGUIDToCreate.end())
	{
		return itrFound->second;
	}

	return NULL;
}

}; // nv_sys
