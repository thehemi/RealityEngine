/*********************************************************************NVMH4****
Path:  NVSDK\Common\include\nv_sys
File:  iconnectionmanager.h

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


INVConnectionManager declaration.  Note the callback which gives the app the opportunity
to intialise effect params and convert effect vector spaces when necessary.


******************************************************************************/

#ifndef __INVCONNECTIONMANAGER_H
#define __INVCONNECTIONMANAGER_H

namespace nv_sys
{

class INVEffectParamInitCallback
{
public:
	virtual bool EffectParamInit(LPD3DXEFFECT pEffect, NVType& Param) = 0;
};

class INVConnectionManager : public nv_sys::INVObject
{
public:
	virtual unsigned long INTCALLTYPE AddRef() = 0;
	virtual unsigned long INTCALLTYPE Release() = 0;
	virtual bool INTCALLTYPE Initialize() = 0;
	virtual bool INTCALLTYPE AddEffect(LPD3DXEFFECT pEffect) = 0;
	virtual bool INTCALLTYPE ApplyEffect(LPD3DXEFFECT pEffect, INVParameterList* pList) = 0;
	virtual bool INTCALLTYPE RemoveEffect(LPD3DXEFFECT pEffect) = 0;

	virtual INVEffectParamInitCallback* INTCALLTYPE GetEffectParamInitCallback() = 0;
	virtual bool INTCALLTYPE SetEffectParamInitCallback(INVEffectParamInitCallback* pCallback) = 0;

	virtual INVParameterList* INTCALLTYPE CreateParameterList(LPD3DXEFFECT pEffect) = 0;
	virtual void INTCALLTYPE Dump() = 0;

	static INVConnectionManager* Create();

};

}; // nv_sys

// {15851097-A4C3-48b0-BB83-6447D67DF253}
static const nv_sys::NVGUID IID_INVConnectionManager = 
{ 0x15851097, 0xa4c3, 0x48b0, { 0xbb, 0x83, 0x64, 0x47, 0xd6, 0x7d, 0xf2, 0x53 } };

#endif // __NVCONNECTIONMANAGER_H