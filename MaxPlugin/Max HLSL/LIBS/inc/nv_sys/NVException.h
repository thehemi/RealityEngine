/*********************************************************************NVMH4****
Path:  NVSDK\Common\src\nv_fx
File:  nvexception.h

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

******************************************************************************/
#ifndef __NVEXCEPTION_H
#define __NVEXCEPTION_H

namespace nv_sys
{

typedef enum eNVEXCEPTIONID
{
	NVEXCEPTIONID_TYPEMISMATCH = 0,
	NVEXCEPTIONID_UNKNOWNTYPE = 1,
	NVEXCEPTIONID_INVALIDOPERATION = 2,
	NVEXCEPTIONID_CREATEFAILED_UNEXPECTEDLY = 3
} eNVEXCEPTIONID;

class NVException
{
public:
	NVException(eNVEXCEPTIONID ExceptionID, const char* pszExceptionString)
		: m_ExceptionID(ExceptionID),
		m_strException(pszExceptionString)
	{
		
	}

	static const char* INTCALLTYPE ExceptionIDToString(eNVEXCEPTIONID ID)
	{
		switch(ID)
		{
			case NVEXCEPTIONID_TYPEMISMATCH:
				return "(Type Mismatch)";
			case NVEXCEPTIONID_UNKNOWNTYPE:
				return "(Type Unknown)";
			case NVEXCEPTIONID_INVALIDOPERATION:
				return "(Invalid Operation)";
			case NVEXCEPTIONID_CREATEFAILED_UNEXPECTEDLY:
				return "(Object creation failed unexpectedly)";
			default:
				break;
		}
		return "(Unknown Exception)";
	}
		
	const char* INTCALLTYPE GetExceptionString() const { return m_strException.c_str(); } 
	eNVEXCEPTIONID INTCALLTYPE GetExceptionID() const { return m_ExceptionID; }

private:
	eNVEXCEPTIONID m_ExceptionID;
	std::string m_strException;
};

}; // namespace nv_type

#endif
