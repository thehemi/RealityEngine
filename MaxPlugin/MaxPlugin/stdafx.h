#include <string>
#include <vector>
using namespace std;

#include <initguid.h>
#include "Max.h"
#include "..\Max HLSL\LIBS\src\nv_gui\resource.h"
#include "utilapi.h"
#include "istdplug.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"
#include "IXref.h"
#include "tvnode.h"
#include "modstack.h" // Object creation
#include "stdmat.h"   // Texture assignment
#include "utilapi.h"
#include "guplib.h"
#include "MaxIcon.h"
#include "Notify.h"
#include "iFnPub.h"
#include "phyexp.h"
#include "ActionTable.h"
#include "iMenuMan.h"
#include "MAXScrpt\MAXScrpt.h"
#include "MAXScrpt\Listener.h"
//#include "MAXScrpt\MAXObj.h"
#include "imacroscript.h"
#include <d3dx9.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <vector>
#include "d3dx9.h"
#include "dxfile.h"
#include "d3d9types.h"
#include "stdmat.h"
#include <dxerr9.h>
#include <string>
using namespace std;

#define ENGINE_API

#include "..\..\shared\shared.h"
#include "..\..\EngineInc\SharedStructures.h"
#include "..\IExport\IGameExporter.h"
#include "Helpers.h"
#include "..\Max HLSL\LIBS\inc\nv_sys\nv_sys.h"
#include "..\Max HLSL\TOOLS\inc\cgfxMax\icgfxdatabridge.h"
#include "IViewportManager.h" // IDXShaderManagerInterface
#include "MapSettings.h"
#include "..\..\dxcommon\PRTOptionsDlg.h"
#include "SphericalHarmonics.h"


extern HINSTANCE g_hInstance;

void Error(const char *fmt, ...);
#define DXASSERT(x) {HRESULT hr;if(FAILED(hr=(x))){ Error(" Error: %s, in: "###x,DXGetErrorString9(hr));}}


extern ConfigFile theConfig;

void Warning(const char *fmt, ...);
void Error(const char *fmt, ...);
//bool ResolvePath(string& str, bool LastDirectoriesMustReallyMatch = true);

extern HINSTANCE g_hInstance;



class INode;
class Mesh;
class Mtl;
class Object;

// Node manipulation
void   SetNodeData(INode* node, NodeData& data);
bool   GetNodeData(INode* node, NodeData& data);
void   ClearNodeData(INode* node);
string GetNodeProperty(INode* node, string prop);
bool   SetNodeProperty(INode* node, string prop, string val);

#define DATA_SLOT 1234 // For app data


// Misc
class Mesh* SplitMeshForLights(INode* InNode, Mesh* InMesh);
bool IsExportableLight(INode* node);