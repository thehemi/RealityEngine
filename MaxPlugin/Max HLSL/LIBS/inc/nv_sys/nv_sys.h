/*********************************************************************NVMH4****
Path:  SDK\LIBS\inc\nv_sys
File:  nv_sys.h

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

#ifndef __NV_SYS_H
#define __NV_SYS_H

#pragma warning (disable : 4786)
#include <string>
#pragma warning (disable : 4786)
#include <vector>
#pragma warning (disable : 4786)
#include <map>
#pragma warning (disable : 4786)
#include <sstream>
#pragma warning (disable : 4786)
#include <list>
#pragma warning (disable : 4786)

#include "nv_typedef.h"
#include <dxerr9.h>
#include <d3dx9.h>


//#ifndef SYS_BUILD
//#define SYS_API __declspec(dllimport)
//#else
//#define SYS_API __declspec(dllexport)
//#endif
// TIM: No DLLs
#define SYS_API

#include "nv_math\nv_math.h"
#include "shared\singleton.h"

#include "invobject.h"
#include "invclone.h"
#include "invproperties.h"

#include "nv_graphics\nv_graphics.h"

#include "nvtype.h"
#include "invobjectsemantics.h"

#include "nvexception.h"
#include "invlog.h"
#include "invsystem.h"
#include "invcreatorarray.h"
#include "invcreator.h"


#include "nv_fx\nv_fx.h"
#include "invconnectionparameter.h"
#include "invparameterlist.h"
#include "invconnectionmanager.h"
#include "invinterpolator.h"

// Objects that can be created in the system namespace

// {B4F814DA-2D6E-4333-9816-CE5E49264F9E}
static const nv_sys::NVGUID CLSID_NVObjectSemantics = 
{ 0xb4f814da, 0x2d6e, 0x4333, { 0x98, 0x16, 0xce, 0x5e, 0x49, 0x26, 0x4f, 0x9e } };

// {974E3D2C-75C5-450e-887B-760AE46B5941}
static const nv_sys::NVGUID CLSID_NVConnectionManager = 
{ 0x974e3d2c, 0x75c5, 0x450e, { 0x88, 0x7b, 0x76, 0xa, 0xe4, 0x6b, 0x59, 0x41 } };

// {252C2754-9E03-41af-B090-3873529467CE}
//static const nv_sys::NVGUID CLSID_NVInterpolator = 
//{ 0x252c2754, 0x9e03, 0x41af, { 0xb0, 0x90, 0x38, 0x73, 0x52, 0x94, 0x67, 0xce } };

// {C337F35B-6E24-4e1f-A299-6FE82377A517}
static const nv_sys::NVGUID CLSID_NVParameterList = 
{ 0xc337f35b, 0x6e24, 0x4e1f, { 0xa2, 0x99, 0x6f, 0xe8, 0x23, 0x77, 0xa5, 0x17 } };

// {E26E31F3-DE46-4cdf-9A3A-BFEEBB280FB9}
//static const nv_sys::NVGUID CLSID_NVCreatorArray = 
//{ 0xe26e31f3, 0xde46, 0x4cdf, { 0x9a, 0x3a, 0xbf, 0xee, 0xbb, 0x28, 0xf, 0xb9 } };

#endif // __NV_SYS_H

