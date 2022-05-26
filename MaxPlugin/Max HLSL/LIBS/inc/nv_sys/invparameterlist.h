/*********************************************************************NVMH4****
Path:  NVSDK\Common\include\nv_sys
File:  iparameterlist.h

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


See ParameterList.cpp


******************************************************************************/

#ifndef __INVPARAMETERLIST_H
#define __INVPARAMETERLIST_H

namespace nv_sys
{

class INVParameterList : public nv_sys::INVObject
{
public:
	static INVParameterList* Create();

	virtual unsigned long INTCALLTYPE AddRef() = 0;
	virtual unsigned long INTCALLTYPE Release() = 0;
	virtual bool INTCALLTYPE GetInterface(const nv_sys::NVGUID& name, void** ppObj) = 0;

	virtual unsigned int INTCALLTYPE GetNumParameters() const = 0;
	virtual INVConnectionParameter* INTCALLTYPE GetConnectionParameter(unsigned int ParamNum) const = 0;
	virtual bool INTCALLTYPE AddConnectionParameter(INVConnectionParameter* pParam) = 0;
};


}; // namespace nv_sys

// {8612A3BE-A5FB-47b0-A2C1-8D9520A2899E}
static const nv_sys::NVGUID IID_INVParameterList = 
{ 0x8612a3be, 0xa5fb, 0x47b0, { 0xa2, 0xc1, 0x8d, 0x95, 0x20, 0xa2, 0x89, 0x9e } };

#endif