/*********************************************************************NVMH4****
Path:  NVSDK\Common\include\nv_sys
File:  connectionparameter.h

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


Implementation of IConnectionParameter and various derived versions.
e.g. IConnectonParameter_Bool

You cast like this:

  if (IConnectionParameter->GetConnectionType() == CONNECTIONTYPE_BOOL)
	static_cast<IConnectionParameter_Bool*>(IConnectionParameter)->DoBoolFunction()

Note that the connection parameter uses the template for many of it's features rather
than duplicating code.  The tricky public virtual 'I' causes the interface to go at the
top of the inheritance tree.


******************************************************************************/

#ifndef __NVCONNECTIONPARAMETER_H
#define __NVCONNECTIONPARAMETER_H

#include "invinterpolator.h"
namespace nv_sys
{

class NVConnectionParameter : public INVConnectionParameter, public INVClone
{
public:
	typedef std::map<int, unsigned int> tmapTimeToKeyIndex;
	typedef std::map<unsigned int, int> tmapKeyIndexToTime;
	typedef std::vector<nv_sys::NVType> tvecKeys;

	NVConnectionParameter();
	virtual ~NVConnectionParameter();

	static bool INTCALLTYPE CreateNVObject(const nv_sys::INVCreator* pCreator, const NVGUID& InterfaceID, void** ppObj);

	// INVObject
	virtual unsigned long INTCALLTYPE AddRef();
	virtual unsigned long INTCALLTYPE Release();
	virtual bool INTCALLTYPE GetInterface(const nv_sys::NVGUID& guid, void** ppObj);

	// IConnectionParameter
	virtual unsigned int INTCALLTYPE GetNumKeys() const;
	virtual const nv_sys::NVType* INTCALLTYPE GetKeyFromIndex(unsigned int Index, int& Time) const;
	virtual bool INTCALLTYPE GetSortedTimeIndices(unsigned int* pIndices, unsigned int NumIndices) const;
	virtual bool INTCALLTYPE DeleteAllKeys();
	virtual bool INTCALLTYPE SetKey(int Time, const nv_sys::NVType& Value);
	virtual nv_sys::NVType INTCALLTYPE GetValueAtTime(int Time) const;
	virtual void INTCALLTYPE Animate(bool bValue);
	virtual void INTCALLTYPE SetToDefaultValue();
	virtual const nv_sys::NVType& INTCALLTYPE GetDefaultValue() const;
	virtual bool INTCALLTYPE SetDefaultValue(const NVType& DefaultValue);
	virtual bool INTCALLTYPE SetInterpolator(INVInterpolator* pInterpolator);

	// INVClone
	virtual bool INTCALLTYPE Clone(const NVGUID& Interface, void** ppObj) const;

private:
	// An interpolator for this parameters
	nv_sys::INVInterpolator* m_pInterpolator;
	
	DWORD m_dwRefCount;

	// Animation control
	tvecKeys m_vecKeys;
	tmapTimeToKeyIndex m_mapTimeToKeyIndex;
	tmapKeyIndexToTime m_mapKeyIndexToTime;

	bool m_bAnimate;
	nv_sys::NVType m_DefaultValue;
};


}; // namespace nv_sys

#endif __NVCONNECTIONPARAMETER_H
