#include "stdafx.h"
#include <sstream>
#include "windows.h"
#include "nvlog.h"

using namespace std;

namespace nv_sys
{

// The singleton for logging
NVLogSingleton* g_pLog = NULL;

SYS_API INVLog* GetLOGInterface() 
{
	// Ensure the global log is initialized.  Because it might be
	// called during construction time.
	if (!g_pLog)
		g_pLog = new NVLogSingleton;

	return NVLogSingleton::GetSingletonPtr(); 
}

NVLogSingleton::NVLogSingleton()
{
	
}

unsigned long NVLogSingleton::AddRef()
{
	return 1;
}

unsigned long NVLogSingleton::Release()
{
	return 1;
}

bool NVLogSingleton::GetInterface(const NVGUID& guid, void** ppObj)
{
	if (EqualNVGUID(guid, IID_INVObject))
	{
		*ppObj = static_cast<INVObject*>(this);
	}
	else if (EqualNVGUID(guid, IID_INVLog))
	{		
		*ppObj = static_cast<INVLog*>(this);
	}
	else
	{
		return false;
	}
		
	static_cast<INVObject*>(this)->AddRef();

	return true;
}

void NVLogSingleton::DebugOut(const char* pszOut)
{
	OutputDebugString(pszOut);
	OutputDebugString("\n");
}

unsigned int NVLogSingleton::GetDebugLevel() const
{
	return m_DebugLevel;
}

bool NVLogSingleton::SetDebugLevel(unsigned int Level)
{
	m_DebugLevel = Level;
	return true;
}

}; // nv_sys
