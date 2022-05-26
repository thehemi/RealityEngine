#ifndef __INVLOG_H
#define __INVLOG_H

#include "invobject.h"

namespace nv_sys
{

class INVLog : public INVObject
{
public:
	virtual unsigned long INTCALLTYPE AddRef() = 0;
	virtual unsigned long INTCALLTYPE Release() = 0;
	virtual bool INTCALLTYPE GetInterface(const NVGUID& name, void** ppObj) = 0;

	virtual unsigned int INTCALLTYPE GetDebugLevel() const = 0;
	virtual bool INTCALLTYPE SetDebugLevel(unsigned int Level) = 0;
	virtual void INTCALLTYPE DebugOut(const char* pszString) = 0;
};

SYS_API INVLog* GetLOGInterface();

#ifdef _DEBUG
#define NVLOG_DEBUG(a, b)										\
do																\
{																\
	if (a <= GetLOGInterface()->GetDebugLevel()) {				\
		std::ostringstream strStream;							\
		strStream << "NVLOG: " << b;							\
		GetLOGInterface()->DebugOut(strStream.str().c_str());	\
	}															\
} while(0)
#else
#define NVLOG_DEBUG(a, b) do { } while(0)
#endif

}; // namespace nv_sys

// {DFABBD1F-BB03-4030-A7F2-C67A0FA8AD99}
static const nv_sys::NVGUID IID_INVLog = 
{ 0xdfabbd1f, 0xbb03, 0x4030, { 0xa7, 0xf2, 0xc6, 0x7a, 0xf, 0xa8, 0xad, 0x99 } };

#endif __INVLOG