#ifndef __PCH_H
#define __PCH_H

//_____________________________________________________________________________
//
//	Forward declare
//
//_____________________________________________________________________________


class CgFXMaterial;
class MaxVertexShader;

//_____________________________________________________________________________
//
//	Include files	
//
//_____________________________________________________________________________
#include "afx.h"
#include "Max.h"
#include "..\..\LIBS\src\nv_gui\resource.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"
#include "stdmat.h"
#include "imtl.h"
#include "macrorec.h"
#include "ID3D9GraphicsWindow.h"
#include "IDX9VertexShader.h"
#include "IStdDualVS.h"
#include "IViewportManager.h"
#include "IHardwareShader.h"
#include "IHardwareRenderer.h"
#include "IMtlEdit.h"
#include "ICustAttribContainer.h"
#include "IViewportManager.h"
#include "IHardwareShader.h"
#include "bmmlib.h"
#include "CustAttrib.h"
#include "Notify.h"
#include "sctex.h"
#include "gamma.h"

#include <string>
using namespace std;

#include <d3dx9.h>
#include <d3d9.h>
#include <Dxerr9.h>

#include <map>
#pragma warning(disable:4530)
#include <vector>
#pragma warning(disable:4530)
#include <string>
#pragma warning(disable:4530)
#include <sstream>
#pragma warning(disable:4530)
#include <list>
#pragma warning(disable:4530)

#include "nv_prof\nv_prof.h"
#include "nv_sys\nv_sys.h"
#include "singleton.h"
#include "TextureMgr.h"
#include "utility.h"
 
#include "nv_math\nv_math.h"
#include "nv_renderdevice\nv_renderdevice.h"
//#include "nv_plugin\nv_plugin.h"

#include "filekey.h"
#include "filesearch.h"

//#include "resource.h"
#include "MaxIcon.h"
#include "Color.h"
#include "shaders.h"

#include "nv_gui\nv_gui.h"
#include "nv_fx\nv_fx.h"

#include "scenemgr.h"

#endif __PCH_H