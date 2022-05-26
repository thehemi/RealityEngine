#ifndef __INVSYSTEM_H
#define __INVSYSTEM_H

#include "invobject.h"

namespace nv_sys
{

class INVCreator;
class INVCreatorArray;
//!The INVSystem interface is core interface for the runtime, and is created first.
/*!Supports registering of components, creation of objects, management of exceptions and logging.
Also has functions to get/set the current system animation time.
*/
class INVSystem : public INVObject
{
public:
	//!Called to indicate an exception has occured.
	virtual bool INTCALLTYPE Exception(const NVException& except) = 0;

	//!Returns the current system animation time.
	virtual unsigned int INTCALLTYPE GetTime() const = 0;

	//!Sets the current system animation time.
	virtual void INTCALLTYPE SetTime(unsigned int Time) = 0;

	//!Register an object in the system
	virtual bool INTCALLTYPE RegisterObject(INVCreator* pCreator) = 0;

	//!UnRegisters an object in the system
	virtual bool INTCALLTYPE UnRegisterObject(INVCreator* pCreator) = 0;

	//!Returns an array of creators in a certain category
	virtual bool INTCALLTYPE GetCreatorsInCategory(const char* pszCategory, INVCreatorArray** ppArray) = 0;

	//!Creates an object based on the unique ID of the object and the required interface to it.
	/*
	\param ObjectClass The Unique ID of the object.
	\param ObjectInterface The Unique interface ID of the interface requested from the object
	\param ppObj A pointer to the returned object's pointer.
	\return TRUE if the object creation succeeded.
	*/
	//virtual bool INTCALLTYPE CreateObject(const NVGUID& ObjectClass, const NVGUID& ObjectInterface, void** ppObj) = 0;

	//!Gets the creator class for a CLSID
	/*
	\param ClassID
	\return The creator class pointer.
	*/
	virtual const INVCreator* INTCALLTYPE GetCreatorClass(const NVGUID& ClassID) const = 0;

};

//!Call to get the INVSystem interface singleton.
SYS_API INVSystem* GetSYSInterface();

SYS_API FinalSYSShutdown();


}; // nv_sys namespace

//!The ID of the system interface
// {2E754DA8-68D8-46b2-8E9C-DC02DBA9EF33}
static const nv_sys::NVGUID IID_INVSystem = 
{ 0x2e754da8, 0x68d8, 0x46b2, { 0x8e, 0x9c, 0xdc, 0x2, 0xdb, 0xa9, 0xef, 0x33 } };

#endif