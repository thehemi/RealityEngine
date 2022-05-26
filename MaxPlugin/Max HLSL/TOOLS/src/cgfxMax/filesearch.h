/******************************************************************************

  Copyright (C)	1999, 2000 NVIDIA Corporation
  This file is provided without support, instruction, or implied warranty of any
  kind.  NVIDIA makes no guarantee of its fitness for a particular purpose and is
  not liable under any circumstances for any damages or loss whatsoever arising
  from the use or inability to use this file or items derived from it.
  
    Comments:

	Simple class to search for files on Win32.

	Call:
	
	  Search->FindFile("*.*", "c:\", true);
	  to find every file on a user's hard disk.

  cmaughan@nvidia.com
      
        
******************************************************************************/
#ifndef __FILESEARCH_H
#define __FILESEARCH_H

#include "nv_common.h"
#include <string>
#include <list>
#include <windows.h>

typedef struct tagFileLocation
{
	WIN32_FIND_DATA m_FindData;
	std::string m_strDirectory;
} tFileLocation;

typedef std::vector<tFileLocation> tvecSearchList;

class FileSearch
{
public:
	FileSearch::FileSearch()
	{
		ZeroMemory(&m_FindData, sizeof(WIN32_FIND_DATA));
	}

	FileSearch::~FileSearch()
	{

	}

	virtual bool FileFoundCallback(const WIN32_FIND_DATA& FindData, const std::string& strDirectory)
	{
		tFileLocation Location;
		Location.m_FindData = FindData;
		Location.m_strDirectory = strDirectory;

		m_vecSearchList.push_back(Location);

		return false;
	}

	virtual void FindFile(const std::string& strFileString, const std::string& strDirectoryStart, bool bRecurse)
	{
		std::string strDirectory;
		strDirectory.resize(MAX_PATH);
		DWORD dwNewSize = GetCurrentDirectory(MAX_PATH, &strDirectory[0]);
		strDirectory.resize(dwNewSize);
		GetCurrentDirectory(dwNewSize, &strDirectory[0]);

		SetCurrentDirectory(strDirectoryStart.c_str());

		WalkDirectory(strFileString, bRecurse);

		SetCurrentDirectory(strDirectory.c_str());
	}
	const tvecSearchList& GetFileList() const { return m_vecSearchList; }
protected:
	
	virtual void WalkDirectory(const std::string& strFileString, bool bRecurse)
	{
		HANDLE hFind;

		m_bRecurse = bRecurse;

		std::string strDirectory;
		strDirectory.resize(MAX_PATH);
		DWORD dwNewSize = GetCurrentDirectory(MAX_PATH, &strDirectory[0]);
		strDirectory.resize(dwNewSize);
		GetCurrentDirectory(dwNewSize, &strDirectory[0]);

		hFind = FindFirstFile(strFileString.c_str(), &m_FindData);
		
		if (hFind == INVALID_HANDLE_VALUE)
			m_bOK = false;
		else
			m_bOK = true;

		while (m_bOK)
		{
			if (!(m_FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				// Grab the current directory, so that we can reset it after the callback.
				// Otherwise, if the callback changes the current directory in any way,
				// our search will abort prematurely.
				// Alternately, we could require the callback to not have any current
				// directory changes, but this is an easy fix, and performance is not
				// likely an issue.
				std::string strCurrentDirectory;
				strCurrentDirectory.resize(MAX_PATH);
				dwNewSize = GetCurrentDirectory(MAX_PATH, &strCurrentDirectory[0]);
				strCurrentDirectory.resize(dwNewSize);
				GetCurrentDirectory(dwNewSize, &strCurrentDirectory[0]);

				FileFoundCallback(m_FindData, strDirectory);

				// Reset the current directory to what it was before the callback.
				SetCurrentDirectory(strCurrentDirectory.c_str());
			}

			m_bOK = FindNextFile(hFind, &m_FindData);
		}

		if (hFind != INVALID_HANDLE_VALUE)
			FindClose(hFind);

		if (m_bRecurse)
		{
			hFind = FindFirstChildDir();

			if (hFind == INVALID_HANDLE_VALUE)
				m_bOK = false;
			else 
				m_bOK = true;

			while (m_bOK)
			{
				if (SetCurrentDirectory(m_FindData.cFileName))
				{
					WalkDirectory(strFileString, true);
				
					SetCurrentDirectory(_T(".."));				
				}
				m_bOK = FindNextChildDir(hFind);
			}

			if (hFind != INVALID_HANDLE_VALUE)
				FindClose(hFind);
		}
	}

	virtual BOOL IsChildDir()
	{
		return ((m_FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && (m_FindData.cFileName[0] != _T('.')));
	}

	virtual BOOL FindNextChildDir(HANDLE hFindFile)
	{
		BOOL bFound = FALSE;
		do
		{
			bFound = FindNextFile(hFindFile, &m_FindData);
		} while (bFound && !IsChildDir());

		return bFound;
	}

	virtual HANDLE FindFirstChildDir()
	{
		BOOL bFound;
		HANDLE hFindFile = FindFirstFile("*.*", &m_FindData);

		if (hFindFile != INVALID_HANDLE_VALUE)
		{
			bFound = IsChildDir();

			if (!bFound)
			{
				bFound = FindNextChildDir(hFindFile);
			}

			if (!bFound)
			{
				FindClose(hFindFile);
				hFindFile = INVALID_HANDLE_VALUE;
			}
		}

		return hFindFile;
	}

protected:
	tvecSearchList m_vecSearchList;
	BOOL m_bRecurse;
	BOOL m_bOK;
	BOOL m_bIsDir;
	WIN32_FIND_DATA m_FindData;
};

#endif	 //__FILESEARCH_H
