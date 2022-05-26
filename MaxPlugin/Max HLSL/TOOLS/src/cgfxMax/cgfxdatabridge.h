/*********************************************************************NVMH3****
Path:  E:\nvidia\devrel\Tools\nvmax
File:  cgfxdatabridge.h

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

Class reported back to the flexporter plugin to give it access to the CgFX parameters.



******************************************************************************/

#ifndef __CGFXDATABRIDGE_H
#define __CGFXDATABRIDGE_H

#include "..\..\inc\cgfxMax\icgfxdatabridge.h"

class CgFXDataBridge : public ICGFXDataBridge
{
public:
   	CgFXDataBridge(MaxVertexShader* pVS);
	~CgFXDataBridge();

    // Query CgFX Data...
	// ICgFXDataBridge methods
	virtual string          GetTechnique();
	virtual TCHAR *         GetCgFX();
	virtual nv_sys::INVParameterList* GetParameterList();
    virtual const char*     GetTextureName(nv_renderdevice::INVTexture* pTexture);

public:
	MaxVertexShader* m_pVS;

};

#endif // __CGFXDATABRIDGE_H