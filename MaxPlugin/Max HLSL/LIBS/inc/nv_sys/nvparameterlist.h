/*********************************************************************NVMH4****
Path:  NVSDK\Common\include\nv_sys
File:  parameterlist.h

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


See parameterlist.cpp


******************************************************************************/

#ifndef __NVPARAMETERLIST_H
#define __NVPARAMETERLIST_H

namespace nv_sys
{
typedef std::vector<INVConnectionParameter*> tvecParameterInfo;

class NVParameterList : public INVParameterList, public INVClone
{
public:
	NVParameterList();
	virtual ~NVParameterList();

	static bool INTCALLTYPE CreateNVObject(const nv_sys::INVCreator* pCreator, const NVGUID& InterfaceID, void** ppObj);

	// INVObject
	virtual unsigned long INTCALLTYPE AddRef();
	virtual unsigned long INTCALLTYPE Release();
	virtual bool INTCALLTYPE GetInterface(const nv_sys::NVGUID& guid, void** ppObj);

	// IParameterList
	virtual unsigned int INTCALLTYPE GetNumParameters() const;
	virtual INVConnectionParameter* INTCALLTYPE GetConnectionParameter(unsigned int ParamNum) const;
	virtual bool INTCALLTYPE AddConnectionParameter(INVConnectionParameter* pParam);

	// INVClone
	virtual bool INTCALLTYPE Clone(const NVGUID& Interface, void** ppObj) const;


private:
	
	DWORD m_dwRefCount;
	tvecParameterInfo m_vecParameterInfo;

}; // namespace nv_sys

};

#endif __NVPARAMETERLIST_H
