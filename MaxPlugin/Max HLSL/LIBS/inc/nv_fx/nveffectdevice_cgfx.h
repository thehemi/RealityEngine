/*********************************************************************NVMH4****
Path:  NVSDK\Common\include\nveffects_cgfx
File:  nveffects_cgfx.h

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

cgfx effects implementation



******************************************************************************/

#ifndef __NVEFFECTDEVICE_CGFX_H
#define __NVEFFECTDEVICE_CGFX_H

namespace nv_fx
{

class NVEffectDevice_CgFX : public INVEffectDevice_CgFX, public nv_gui::INVInfoDump
{
public:
	static bool INTCALLTYPE CreateNVObject(const nv_sys::INVCreator* pCreator, const nv_sys::NVGUID& InterfaceID, void** ppObj);

	// INVObject	
	virtual unsigned long INTCALLTYPE AddRef();
	virtual unsigned long INTCALLTYPE Release();
	virtual bool INTCALLTYPE GetInterface(const nv_sys::NVGUID& name, void** ppObj);

	// INVEffects
	virtual bool INTCALLTYPE Initialize(nv_renderdevice::INVRenderDevice* pRenderDevice);
	virtual bool INTCALLTYPE UnInitialize();
	virtual bool INTCALLTYPE CreateEffectFromFile(LPCSTR pSrcFile, DWORD Flags, LPD3DXEFFECT* pEffect, const char** ppCompilationErrors);
	virtual bool INTCALLTYPE GetErrors(const char** ppErrors);
	virtual bool INTCALLTYPE GetRenderDevice(nv_renderdevice::INVRenderDevice** ppDevice);

	// INVInfoDump
	virtual bool INTCALLTYPE InfoDump(nv_gui::INVDebugConsole* pConsole);

private:
    // Internal functions 
	DWORD m_dwRefCount;
	std::string m_strDeviceName;
	UINT_PTR m_DeviceHandle;

    // Internal constructor
    NVEffectDevice_CgFX();

	nv_renderdevice::INVRenderDevice* m_pRenderDevice;
};

}; //namespace nv_fx

#endif // NVEFFECTS_CGFX_H