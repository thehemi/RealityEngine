/*********************************************************************NVMH4****
Path:  NVSDK\Common\include\nv_sys
File:  ConnectionManager.h

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


See connectionmanager.cpp


******************************************************************************/

#ifndef __NVCONNECTIONMANAGER_H
#define __NVCONNECTIONMANAGER_H

namespace nv_sys
{

typedef std::vector<INVEffectTemplateParameter*> tvecEffectTemplateParameters;
typedef std::map<LPD3DXEFFECT, tvecEffectTemplateParameters> tmapEffectTemplateParameters;

class NVConnectionManager : public INVConnectionManager
{
public:
	NVConnectionManager();
	~NVConnectionManager();

	static bool INTCALLTYPE CreateNVObject(const nv_sys::INVCreator* pCreator, const nv_sys::NVGUID& InterfaceID, void** ppObj);

	// INVObject
	virtual unsigned long INTCALLTYPE AddRef();
	virtual unsigned long INTCALLTYPE Release();
	virtual bool INTCALLTYPE GetInterface(const nv_sys::NVGUID& guid, void** ppObj);

	// IConnectionManager
	virtual bool INTCALLTYPE Initialize();
	virtual bool INTCALLTYPE AddEffect(LPD3DXEFFECT pEffect);
	virtual bool INTCALLTYPE RemoveEffect(LPD3DXEFFECT pEffect);
	virtual bool INTCALLTYPE ApplyEffect(LPD3DXEFFECT pEffect, INVParameterList* pList);
	virtual INVParameterList* INTCALLTYPE CreateParameterList(LPD3DXEFFECT pEffect);

	virtual INVEffectParamInitCallback* INTCALLTYPE GetEffectParamInitCallback();
	virtual bool INTCALLTYPE SetEffectParamInitCallback(INVEffectParamInitCallback* pCallback);

	// Useful debug stuff
	virtual void INTCALLTYPE Dump();

private:
	tmapEffectTemplateParameters m_mapEffectTemplateParameters;
	INVConnectionManager* m_pConnectionManager;
	INVEffectParamInitCallback* m_pCallback;

	unsigned long m_dwRefCount;
};

}; // nv_sys

#endif //__NVCONNECTIONMANAGER_H