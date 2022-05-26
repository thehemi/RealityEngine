/*********************************************************************NVMH4****
Path:  NVSDK\Common\include\nv_sys
File:  EffectTemplateParameter.h

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


see effecttemplateparameter.cpp


******************************************************************************/

#ifndef __NVEFFECTTEMPLATEPARAMETER_H
#define __NVEFFECTTEMPLATEPARAMETER_H

namespace nv_sys
{

class NVEffectTemplateParameter : public INVEffectTemplateParameter
{
public:
	NVEffectTemplateParameter();
	virtual ~NVEffectTemplateParameter();

	virtual unsigned long AddRef();
	virtual unsigned long Release();
	virtual bool Setup(INVConnectionManager* pConfigurationManager, LPD3DXEFFECT pEffect, const unsigned int iParameter);
	virtual void Dump();
	virtual const D3DXPARAMETER_DESC* GetDesc() const;
	virtual INVConnectionParameter* CreateConnectionParameter();
	virtual bool ApplyConnection(INVEffectParamInitCallback* pEffectParamInit, INVConnectionParameter* pParam);
	static INVEffectTemplateParameter* CreateInstance(); 
private:
	// A pointer to the parameter info, the annotations (decoded), and the semantic
	std::auto_ptr<D3DXPARAMETER_DESC> m_pParamDesc;
	// TIM: Handle for fast assignment
	D3DXHANDLE						  m_pParamHandle;
	std::auto_ptr<NVCgFXType> m_pParameter;
	DWORD m_dwRefCount;
	LPD3DXEFFECT m_pEffect;
	nv_sys::NVType m_Value;
};

}; // nv_sys

#endif // __NVEFFECTTEMPLATEPARAMTER_H