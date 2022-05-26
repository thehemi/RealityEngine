/*********************************************************************NVMH4****
Path:  NVSDK\Common\include\nv_sys
File:  IEffectTemplateParameter.h

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

EffectTemplateParameter interface decl.  See effecttemplateparameter.cpp



******************************************************************************/

#ifndef __INVEFFECTTEMPLATEPARAMETER_H
#define __INVEFFECTTEMPLATEPARAMETER_H

namespace nv_sys
{
class INVConnectionManager;
class INVEffectParamInitCallback;
class INVEffectTemplateParameter
{
public:
	virtual unsigned long AddRef() = 0;
	virtual unsigned long Release() = 0;
	virtual bool Setup(INVConnectionManager* pManager, LPD3DXEFFECT pEffect, const unsigned int iParameter) = 0;
	virtual void Dump() = 0;
	virtual const D3DXPARAMETER_DESC* GetDesc() const = 0;
	virtual bool ApplyConnection(INVEffectParamInitCallback* pEffectInit, INVConnectionParameter* pParam) = 0;
	virtual INVConnectionParameter* CreateConnectionParameter() = 0;
};

}; // nv_sys

#endif // __INVEFFECTTEMPLATEPARAMTER_H