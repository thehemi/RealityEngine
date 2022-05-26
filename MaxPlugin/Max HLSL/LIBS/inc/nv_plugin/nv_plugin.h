#ifndef __NV_PLUGIN_H
#define __NV_PLUGIN_H

#include "nv_sys\nv_sys.h"
#include <list>

namespace nv_plugin
{

typedef struct tagtPluginInfo
{
	HINSTANCE m_hLib;
} tPluginInfo;

typedef std::list<tPluginInfo> tlistPluginInfo;

class ISearchFile
{
public:
	virtual std::string INTCALLTYPE FindFile(const std::string& strSearch) = 0;
};

class CPluginManager
{
public:
	CPluginManager();
	~CPluginManager();

	bool LoadPlugins(const char* pszInfFile, ISearchFile* pSearch = NULL);
	void FreePlugins();

	const tlistPluginInfo& GetPluginInfo() const { return m_listPluginInfo; }
private:
	tlistPluginInfo m_listPluginInfo;

};


}; // namespace nv_plugin

#endif // __NV_PLUGIN_H