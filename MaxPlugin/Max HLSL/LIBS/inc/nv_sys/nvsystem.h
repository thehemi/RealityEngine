#ifndef __NVSYSTEM_H
#define __NVSYSTEM_H

#include "nv_sys.h"

namespace nv_sys
{

typedef std::map<NVGUID, INVCreator*> tmapNVGUIDToCreate;

typedef std::list<INVCreator*> tlistCreators;
typedef std::map<std::string, tlistCreators> tmapCategoryNameToCreators;

class NVSystemSingleton : public INVSystem, public Singleton<NVSystemSingleton>
{
public:
	NVSystemSingleton();

	virtual unsigned long INTCALLTYPE AddRef();
	virtual unsigned long INTCALLTYPE Release();
	virtual bool INTCALLTYPE GetInterface(const NVGUID& guid, void** ppObj);

	virtual bool INTCALLTYPE Exception(const NVException& except);
	virtual unsigned int INTCALLTYPE GetTime() const;
	virtual void INTCALLTYPE SetTime(unsigned int Time);

	virtual bool INTCALLTYPE RegisterObject(INVCreator* pCreator);
	virtual bool INTCALLTYPE UnRegisterObject(INVCreator* pCreator);
	//virtual bool INTCALLTYPE CreateObject(const NVGUID& ObjectClass, const NVGUID& ObjectInterface, void** ppObj);

	virtual bool INTCALLTYPE GetCreatorsInCategory(const char* pszCategory, INVCreatorArray** ppArray);
	virtual const INVCreator* INTCALLTYPE GetCreatorClass(const NVGUID& ClassID) const;

private:
	unsigned int m_Time;
	tmapNVGUIDToCreate m_mapNVGUIDToCreate;
	tmapCategoryNameToCreators m_mapCategoryNameToCreators;
};


};

#endif