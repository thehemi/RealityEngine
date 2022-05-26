/*********************************************************************NVMH3****
Path:  E:\nvidia\devrel\Tools\nvmax
File:  icgfxdatabridge.h

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

The databridge interface exported to the flexporter plugin.


******************************************************************************/
#ifndef __ICGFXDATABRIDGE_H
#define __ICGFXDATABRIDGE_H

#define CGFX_DATA_CLIENT_INTERFACE Interface_ID(0x43d2493, 0xe4c17a4)

class ICGFXDataBridge : public BaseInterface
{
public:
	virtual Interface_ID	    GetID()             { return CGFX_DATA_CLIENT_INTERFACE; }

	// Interface Lifetime
	virtual LifetimeType	    LifetimeControl()   { return noRelease; }

    // Query CgFX Data...
	virtual string             GetTechnique()    = 0;
	virtual TCHAR *            GetCgFX()                       = 0;
	virtual nv_sys::INVParameterList* GetParameterList()  = 0;
    virtual const char*         GetTextureName(nv_renderdevice::INVTexture* ) = 0;
};

#endif // __ICGFXDATABRIDGE_H