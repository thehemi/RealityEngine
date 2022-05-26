/*********************************************************************NVMH4****
Path:  NVSDK\Common\include\nv_sys
File:  iconnectionparameter.h

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

IConnectionParameter declaration, as well as derived types.
Also enums for Semantic ID's and annotation ID's.  Strings for semantics and annotations
are converted ASAP to their enum counterparts for speed.



******************************************************************************/

#ifndef __INVCONNECTIONPARAMETER_H
#define __INVCONNECTIONPARAMETER_H

namespace nv_sys
{

class INVParameterList;
class INVInterpolator;

class INVConnectionParameter : public nv_sys::INVObject
{
public:
	virtual unsigned long INTCALLTYPE AddRef() = 0;
	virtual unsigned long INTCALLTYPE Release() = 0;
	virtual bool INTCALLTYPE GetInterface(const nv_sys::NVGUID& name, void** ppObj) = 0;

	virtual unsigned int INTCALLTYPE GetNumKeys() const = 0;
	virtual bool INTCALLTYPE DeleteAllKeys() = 0;
	virtual bool INTCALLTYPE SetKey(int Time, const nv_sys::NVType& Value) = 0;
	virtual nv_sys::NVType INTCALLTYPE GetValueAtTime(int Time) const = 0;
	virtual const nv_sys::NVType* INTCALLTYPE GetKeyFromIndex(unsigned int Index, int& Time) const = 0;
	virtual bool INTCALLTYPE GetSortedTimeIndices(unsigned int* pIndices, unsigned int NumIndices) const = 0;
	virtual void INTCALLTYPE Animate(bool bValue) = 0;
	virtual void INTCALLTYPE SetToDefaultValue() = 0;
	virtual const nv_sys::NVType& INTCALLTYPE GetDefaultValue() const = 0;
	virtual bool INTCALLTYPE SetDefaultValue(const NVType& DefaultValue) = 0;
	virtual bool INTCALLTYPE SetInterpolator(INVInterpolator* pInterpolator) = 0;

	static INVConnectionParameter* Create();

};

}; // namespace nv_sys

// {8144F85B-1C76-4958-99E8-F8BF7399DBB8}
static const nv_sys::NVGUID IID_INVConnectionParameter = 
{ 0x8144f85b, 0x1c76, 0x4958, { 0x99, 0xe8, 0xf8, 0xbf, 0x73, 0x99, 0xdb, 0xb8 } };

// {4644A49F-D633-4598-A2E1-4D855CCC616B}
static const nv_sys::NVGUID CLSID_NVConnectionParameter = 
{ 0x4644a49f, 0xd633, 0x4598, { 0xa2, 0xe1, 0x4d, 0x85, 0x5c, 0xcc, 0x61, 0x6b } };

#endif