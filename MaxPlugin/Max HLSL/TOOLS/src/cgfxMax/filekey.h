/*********************************************************************NVMH4****
Path:  NVSDK\Common\include
File:  filekey.h

Copyright NVIDIA Corporation 2002
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED
*AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS
BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES
WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS,
BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS)
ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS
BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.



Comments:


A simple key representing a file.  It will return 'true' to the updated function
if the file has changed since Updated() was last called.

Operator < is provided for stl compatibility


******************************************************************************/

#ifndef __FILEKEY_H
#define __FILEKEY_H

class FileKey
{
public:
	FileKey()
	{
		ZeroMemory(&m_FileTime, sizeof(FILETIME));
	}

	FileKey(const std::string& strFileName)
		: m_strFileName(strFileName)
	{
		Updated();
	}

	// Return true if this key's file has changed
	bool Updated() const
	{
		WIN32_FILE_ATTRIBUTE_DATA FileData;
		GetFileAttributesEx(m_strFileName.c_str(), GetFileExInfoStandard, &FileData);

		if ((m_FileTime.dwHighDateTime != FileData.ftLastWriteTime.dwHighDateTime) ||
			(m_FileTime.dwLowDateTime != FileData.ftLastWriteTime.dwLowDateTime))
		{
			m_FileTime.dwHighDateTime = FileData.ftLastWriteTime.dwHighDateTime;
			m_FileTime.dwLowDateTime = FileData.ftLastWriteTime.dwLowDateTime;
			return true;
		}

		return false;
	}

	// We use the key in a map, but we don't want multiple keys with different dates.
	// so we only compare the string.
	// It's the client's responsibility to call Update() to check that the file is out of date.
	bool operator < (const FileKey& rhs) const
	{
		return (m_strFileName < rhs.m_strFileName);
	}

	const char* GetFileName() const { return m_strFileName.c_str(); }
private:
	mutable FILETIME m_FileTime;
	std::string m_strFileName;

};

#endif