#include "stdafx.h"
#include <fstream>
#include <iostream>

using namespace std;

namespace nv_plugin
{

CPluginManager::CPluginManager()
{

}

CPluginManager::~CPluginManager()
{

}


void error(LPSTR lpszFunction) 
{ 
    CHAR szBuf[80]; 
    DWORD dw = GetLastError(); 
 
    sprintf(szBuf, "%s failed: GetLastError returned %u\n", 
        lpszFunction, dw); 
 
	OutputDebugString(szBuf);
   // MessageBox(NULL, szBuf, "Error", MB_OK); 
}


bool CPluginManager::LoadPlugins(const char* pszInfFile, ISearchFile* pSearch)
{

	ifstream inputFile(pszInfFile);

	if (!inputFile.is_open())
	{
		MessageBox(NULL, "Could not find plugin config file", "Error", MB_ICONEXCLAMATION | MB_OK);
        return FALSE;
	}

	char line[256];

	do
	{
		ZeroMemory(line, 256 * sizeof(char));
		inputFile.getline(&line[0], 255);

		if (strlen(line) != 0)
		{
			// Trim right.
			int i = strlen(line - 1);
			while (line[i] == ' ' || line[i] == '\t' || line[i] == '\n' || line[i] == '\r')
			{
				line[i] = 0;
				i--;
				if (i < 0)
					break;
			}

			if (strlen(line) != 0)
			{
				std::string strLibPath;
				if (pSearch)
				{
					strLibPath = pSearch->FindFile(line);
				}
				else
				{
					strLibPath = line;
				}

				/* While there are tokens in "string" */
				HINSTANCE hLib = LoadLibrary((LPCSTR)strLibPath.c_str());
				if (hLib != NULL)
				{
					tPluginInfo PlugInfo;
					PlugInfo.m_hLib = hLib;
					m_listPluginInfo.push_back(PlugInfo);
				}
				else
					error("LoadLibrary nv_gui_mtdll.dll");

			}
		}
		
	} while (!inputFile.eof());
	


	return true;
}



void CPluginManager::FreePlugins()
{
	// Free Plugins
	tlistPluginInfo::iterator itrPlugins = m_listPluginInfo.begin();
	while (itrPlugins != m_listPluginInfo.end())
	{
		tPluginInfo& PluginInfo = *itrPlugins;

		FreeLibrary(PluginInfo.m_hLib);

		itrPlugins++;
	}
}

}; // nv_plugin namespace