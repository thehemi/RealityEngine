#ifndef __INVCREATOR_H
#define __INVCREATOR_H

#include "invobject.h"

namespace nv_sys
{

class INVCreator;

//! A pointer to a creation function in an object
typedef bool (INTCALLTYPE* pfnCreateNVObject)(const INVCreator* pCreator, const nv_sys::NVGUID& InterfaceID, void** pObj);

//! Each object registers itself with this interface type.  There is one instance of it per object type.
/*! The Creator class is used by the core system to find out what the object can do and create it on behalf of clients.
\sa DECLARE_NV_OBJECT
*/
class INVCreator
{
public:
	INVCreator(const nv_sys::NVGUID& ClassID,
			pfnCreateNVObject pfnCreate,
			const char* pszFriendlyName,
			const char* pszShortName,
			const char* pszCategory = "anonymous")
		: m_ClassID(ClassID),
		m_pszFriendlyName(pszFriendlyName),
		m_pszShortName(pszShortName),
		m_pszCategoryName(pszCategory),
		m_pfnCreate(pfnCreate)
	{
		GetSYSInterface()->RegisterObject(this);
	}
	
	~INVCreator()
	{
		GetSYSInterface()->UnRegisterObject(this);
	}
	virtual pfnCreateNVObject INTCALLTYPE GetCreateFunction() const { return m_pfnCreate; }
	virtual const NVGUID& INTCALLTYPE GetClass() const { return m_ClassID; }
	virtual const char* INTCALLTYPE GetFriendlyName() const { return m_pszFriendlyName; }
	virtual const char* INTCALLTYPE GetCategoryName() const { return m_pszCategoryName; }
	virtual const char* INTCALLTYPE GetShortName() const { return m_pszShortName; }
private:
	const char* m_pszFriendlyName;
	const char* m_pszShortName;
	const char* m_pszCategoryName;
	pfnCreateNVObject m_pfnCreate;
	const NVGUID& m_ClassID;
};


#define DECLARE_NVOBJECT(__ClassName, __ClassID, __FriendlyName, __ShortName)						\
/*__declspec(dllexport)*/ INVCreator __ClassName##__ClassID(__ClassID, __ClassName##::CreateNVObject, __FriendlyName, __ShortName);

#define DECLARE_NVOBJECT_CATEGORY(__ClassName, __ClassID, __FriendlyName, __ShortName, __Category)	\
/*__declspec(dllexport)*/ INVCreator __ClassName##__ClassID(__ClassID, __ClassName##::CreateNVObject, __FriendlyName, __ShortName, __Category);

}; // nv_sys namespace

#endif