#ifndef __NVLOG_H
#define __NVLOG_H

#include "invlog.h"

namespace nv_sys
{

class NVLogSingleton : public INVLog, public Singleton<NVLogSingleton>
{
public:
	NVLogSingleton();

	virtual unsigned long INTCALLTYPE AddRef();
	virtual unsigned long INTCALLTYPE Release();
	virtual bool INTCALLTYPE GetInterface(const NVGUID& guid, void** ppObj);
	
	virtual unsigned int INTCALLTYPE GetDebugLevel() const;
	virtual bool INTCALLTYPE SetDebugLevel(unsigned int Level);
	virtual void INTCALLTYPE DebugOut(const char* pszOut);

private:
	unsigned int m_DebugLevel;

};


};  // namespace nv_sys

#endif