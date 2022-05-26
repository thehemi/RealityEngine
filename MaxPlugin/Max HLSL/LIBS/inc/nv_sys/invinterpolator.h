/*********************************************************************NVMH4****
Path:  NVSDK\Common\include\nv_sys
File:  iinterpolator.h

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

#ifndef __INVINTERPOLATOR_H
#define __INVINTERPOLATOR_H

#include "invconnectionparameter.h"
namespace nv_sys
{

class INVInterpolator : public nv_sys::INVObject
{
public:
	virtual unsigned long INTCALLTYPE AddRef() = 0;
	virtual unsigned long INTCALLTYPE Release() = 0;
	virtual bool INTCALLTYPE GetInterface(const nv_sys::NVGUID& name, void** ppObj) = 0;

	virtual nv_sys::NVType INTCALLTYPE Interpolate(const INVConnectionParameter* pParam, unsigned int Time) const = 0;
};

}; // nv_sys

// {DBAC964C-448F-443b-808E-56650137AF4D}
static const nv_sys::NVGUID IID_INVInterpolator = 
{ 0xdbac964c, 0x448f, 0x443b, { 0x80, 0x8e, 0x56, 0x65, 0x1, 0x37, 0xaf, 0x4d } };

#endif __INVINTERPOLATOR