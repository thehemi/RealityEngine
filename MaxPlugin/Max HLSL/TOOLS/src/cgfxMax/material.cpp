/*********************************************************************NVMH4****
Path:  
File:  material.cpp

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

The main material class.  Handles the parameter block, general control panel maintenance
and interaction with MAX.


******************************************************************************/


#include "pch.h"
#include "RenderMesh.h"
#include "Material.h"
#include "vertexshader.h"
#include "cgfxdatabridge.h"
#include "filesearch.h"
#include "connectionpblock.h"
#include "scenemgr.h"
#include "connections.h"
#include "modulever.h"

#pragma warning(disable: 4786)
#include <fstream>
#pragma warning(disable: 4786)

using namespace std;
using namespace nv_fx;
using namespace nv_sys;
using namespace nv_renderdevice;

DECLARE_NVPROFILE_MANAGER();

unsigned int g_DebugLevel = 0;
extern bool g_bShowGUI;
static bool g_bFirstMaterial = true;
CSceneManager theSceneManager;

// Report a debug console so the standard macros work.  Couldn't get the console window to
// work inside of max for some reason.
class dbg : public nv_gui::INVDebugConsole
{
public:
	virtual unsigned long INTCALLTYPE AddRef() { return 1;};
	virtual unsigned long INTCALLTYPE Release() { return 1; };
	virtual bool INTCALLTYPE GetInterface(const nv_sys::NVGUID& Interface, void** ppvObj) { return false; }

    virtual bool INTCALLTYPE OutputDebug(const char* pszChar) { OutputDebugString(pszChar); OutputDebugString("\n"); return true; };
	virtual bool INTCALLTYPE SetVisible(bool bHide) { return true; };
	virtual bool INTCALLTYPE IsVisible() { return true; };
	virtual bool INTCALLTYPE SetLogFile(const char* pszFile) { return true; }
	virtual bool INTCALLTYPE SetTitle(const char* pszTitle) { return true; }
};
static dbg dbgConsole;
nv_gui::INVDebugConsole* g_pDebugConsole = static_cast<nv_gui::INVDebugConsole*>(&dbgConsole);

	nv_gui::INVDebugConsole* nv_gui::INVDebugConsole::CreateInstance(){
		return g_pDebugConsole;
	}

static DefaultClassDesc		gDefaultShaderDesc;
PSCM_Accessor			gPSCMAccessor;

ClassDesc2* GetDefaultShaderDesc() 
{ 
	return(&gDefaultShaderDesc); 
}

//Function Publishing descriptor for Mixin interface
//*****************************************************
FPInterfaceDesc cgfx_interface(
	CGFXPLUGIN_INTERFACE, _T("cgfxpluginops"), 0, &gDefaultShaderDesc, FP_MIXIN,
		cgfxplugin_getnumtechniques, _T("getnumtechniques"), 0, TYPE_INT, 0, 0,
		cgfxplugin_getnumparameters, _T("getnumparameters"), 0, TYPE_INT, 0, 0,
		cgfxplugin_getparameter, _T("getparameter"), 0, TYPE_FPVALUE_BV, 0, 1,
			_T("index"), 0, TYPE_INT,
		cgfxplugin_setparameter, _T("setparameter"), 0, 0, 0, 2,
			_T("index"), 0, TYPE_INT,
			_T("value"), 0, TYPE_FPVALUE_BV,
		cgfxplugin_getparametername, _T("getparametername"), 0, TYPE_STRING, 0, 1,
			_T("index"), 0, TYPE_INT,
		cgfxplugin_getparametersemantic, _T("getparametersemantic"), 0, TYPE_STRING, 0, 1,
			_T("index"), 0, TYPE_INT,
	end
);

// Two almost identical param blocks.  One is a backup...
// Maintain the two together.
ParamBlockDesc2 MaterialParamBlk(0,_T("Params"),0,&gDefaultShaderDesc,  P_AUTO_CONSTRUCT + P_AUTO_UI, ref_pblock, 
	//rollout
	IDD_MAX_SHADER, IDS_PARAMS, 0, 0, NULL,

	// The CGFX Effect being used
	CGFX_FILE, _T("CgFX"), TYPE_FILENAME, 0, IDS_CGFX, 
		p_default, 		0, 
		p_accessor,		&gPSCMAccessor,
		end, 
 
 	// The teqnique number in the effect
    CGFX_TECHNIQUE, _T("Technique"), TYPE_INT, 0, IDS_TECHNIQUE, 
		p_default, 		0,
		p_accessor,		&gPSCMAccessor,
		end, 

	// The teqnique name in the effect
    CGFX_TECHNIQUENAME, _T("TechniqueName"), TYPE_STRING, 0, IDS_TECHNIQUENAME, 
		p_default, 		0,
		p_accessor,		&gPSCMAccessor,
		end, 

	// A button to edit the connections
	CGFX_CONNECTIONS, _T("Connections"), TYPE_BOOL,  P_RESET_DEFAULT, IDS_CONNECTIONS, 
		p_default, 		FALSE, 
		p_ui, 			TYPE_CHECKBUTTON, IDC_EDITCONNECTIONS,
		p_accessor,		&gPSCMAccessor,
		end, 

	CGFX_FLOAT,    _T("FloatTab"),  TYPE_FLOAT_TAB, 0, P_TV_SHOW_ALL+P_COMPUTED_NAME+P_ANIMATABLE+P_VARIABLE_SIZE, IDS_DYNMATFLOAT_DATA,
		p_accessor,		&gPSCMAccessor,
		p_refno, 10,
		end,

	CGFX_TEXTURE,    _T("TextureTab"),  TYPE_FILENAME_TAB, 0, P_TV_SHOW_ALL+P_COMPUTED_NAME+P_VARIABLE_SIZE, IDS_DYNMATFILENAME_DATA,
		p_accessor,		&gPSCMAccessor,
		p_refno, 11,
		end,

	CGFX_INT,    _T("IntTab"),  TYPE_INT_TAB, 0, P_TV_SHOW_ALL+P_COMPUTED_NAME+P_ANIMATABLE+P_VARIABLE_SIZE, IDS_DYNMATINT_DATA,
		p_accessor,		&gPSCMAccessor,
		p_refno, 12,
		end,

   	CGFX_SYNC_AMBIENT, _T("SyncAmbient"),	TYPE_BOOL,	0,	IDS_SYNC_AMBIENT,
	    p_default,		TRUE,
	    p_ui,			TYPE_SINGLECHEKBOX,	IDC_SYNC_AMBIENT,
	    p_accessor,		&gPSCMAccessor,
	    end,

   	CGFX_SYNC_DIFFUSE, _T("SyncDiffuse"),	TYPE_BOOL,	0,	IDS_SYNC_DIFFUSE,
	    p_default,		TRUE,
	    p_ui,			TYPE_SINGLECHEKBOX,	IDC_SYNC_DIFFUSE,
	    p_accessor,		&gPSCMAccessor,
	    end,
   	CGFX_SYNC_SPECULAR, _T("SyncSpecular"),	TYPE_BOOL,	0,	IDS_SYNC_SPECULAR,
	    p_default,		TRUE,
	    p_ui,			TYPE_SINGLECHEKBOX,	IDC_SYNC_SPECULAR,
	    p_accessor,		&gPSCMAccessor,
	    end,
   	CGFX_SYNC_GLOSS,    _T("SyncGloss"),	TYPE_BOOL,	0,	IDS_SYNC_GLOSS,
	    p_default,		TRUE,
	    p_ui,			TYPE_SINGLECHEKBOX,	IDC_SYNC_GLOSS,
	    p_accessor,		&gPSCMAccessor,
	    end,
   	CGFX_SYNC_SELFILLUMINATION, _T("SyncSelfIllumination"),	TYPE_BOOL,	0,	IDS_SYNC_SELFILLUMINATION,
	    p_default,		TRUE,
	    p_ui,			TYPE_SINGLECHEKBOX,	IDC_SYNC_SELFILLUMINATION,
	    p_accessor,		&gPSCMAccessor,
	    end,
   	CGFX_SYNC_OPACITY, _T("SyncOpacity"),	TYPE_BOOL,	0,	IDS_SYNC_OPACITY,
	    p_default,		TRUE,
	    p_ui,			TYPE_SINGLECHEKBOX,	IDC_SYNC_OPACITY,
	    p_accessor,		&gPSCMAccessor,
	    end,
   	CGFX_SYNC_FILTERCOLOR, _T("SyncFilterColor"), TYPE_BOOL,	0,	IDS_SYNC_FILTERCOLOR,
	    p_default,		TRUE,
	    p_ui,			TYPE_SINGLECHEKBOX,	IDC_SYNC_FILTERCOLOR,
	    p_accessor,		&gPSCMAccessor,
	    end,
   	CGFX_SYNC_BUMP, _T("SyncBump"),	TYPE_BOOL,	0,	IDS_SYNC_BUMP,
	    p_default,		TRUE,
	    p_ui,			TYPE_SINGLECHEKBOX,	IDC_SYNC_BUMP,
	    p_accessor,		&gPSCMAccessor,
	    end,
   	CGFX_SYNC_REFLECTION, _T("SyncReflection"),	TYPE_BOOL,	0,	IDS_SYNC_REFLECTION,
	    p_default,		TRUE,
	    p_ui,			TYPE_SINGLECHEKBOX,	IDC_SYNC_REFLECTION,
	    p_accessor,		&gPSCMAccessor,
	    end,
   	CGFX_SYNC_REFRACTION,  _T("SyncRefraction"),	TYPE_BOOL,	0,	IDS_SYNC_REFRACTION,
	    p_default,		TRUE,
	    p_ui,			TYPE_SINGLECHEKBOX,	IDC_SYNC_REFRACTION,
	    p_accessor,		&gPSCMAccessor,
	    end,
   	CGFX_SYNC_DISPLACEMENT, _T("SyncDisplacement"),	TYPE_BOOL,	0,	IDS_SYNC_DISPLACEMENT,
	    p_default,		TRUE,
	    p_ui,			TYPE_SINGLECHEKBOX,	IDC_SYNC_DISPLACEMENT,
	    p_accessor,		&gPSCMAccessor,
	    end,

	end

	);

ParamBlockDesc2 MaterialParamBlk_Backup(1,_T("Params_backup"),0,&gDefaultShaderDesc,  P_AUTO_CONSTRUCT, ref_pblock_backup, 
	//rollout
	//IDD_PANEL, IDS_PARAMS, 0, 0, NULL,

	// The CGFX Effect being used
	CGFX_FILE, _T("CgFX"), TYPE_FILENAME, 0, IDS_CGFX, 
		p_default, 		0, 
		p_accessor,		&gPSCMAccessor,
		end, 
 
 	// The teqnique number in the effect
    CGFX_TECHNIQUE, _T("Technique"), TYPE_INT, 0, IDS_TECHNIQUE, 
		p_default, 		0,
		p_accessor,		&gPSCMAccessor,
		end, 

		// The teqnique name in the effect
    CGFX_TECHNIQUENAME, _T("TechniqueName"), TYPE_STRING, 0, IDS_TECHNIQUENAME, 
		p_default, 		0,
		p_accessor,		&gPSCMAccessor,
		end, 

	// A button to edit the connections
	CGFX_CONNECTIONS, _T("Connections"), TYPE_BOOL,  P_RESET_DEFAULT, IDS_CONNECTIONS, 
		p_default, 		FALSE, 
		p_ui, 			TYPE_CHECKBUTTON, IDC_EDITCONNECTIONS,
		p_accessor,		&gPSCMAccessor,
		end, 

	CGFX_FLOAT,    _T("FloatTab"),  TYPE_FLOAT_TAB, 0, P_TV_SHOW_ALL+P_COMPUTED_NAME+P_ANIMATABLE+P_VARIABLE_SIZE, IDS_DYNMATFLOAT_DATA,
		p_accessor,		&gPSCMAccessor,
		p_refno, 10,
		end,

	CGFX_TEXTURE,    _T("TextureTab"),  TYPE_FILENAME_TAB, 0, P_TV_SHOW_ALL+P_COMPUTED_NAME+P_VARIABLE_SIZE, IDS_DYNMATFILENAME_DATA,
		p_accessor,		&gPSCMAccessor,
		p_refno, 11,
		end,

	CGFX_INT,    _T("IntTab"),  TYPE_INT_TAB, 0, P_TV_SHOW_ALL+P_COMPUTED_NAME+P_ANIMATABLE+P_VARIABLE_SIZE, IDS_DYNMATINT_DATA,
		p_accessor,		&gPSCMAccessor,
		p_refno, 12,
		end,

   	CGFX_SYNC_AMBIENT, _T("SyncAmbient"),	TYPE_BOOL,	0,	IDS_SYNC_AMBIENT,
	    p_default,		TRUE,
	    p_ui,			TYPE_SINGLECHEKBOX,	IDC_SYNC_AMBIENT,
	    p_accessor,		&gPSCMAccessor,
	    end,

   	CGFX_SYNC_DIFFUSE, _T("SyncDiffuse"),	TYPE_BOOL,	0,	IDS_SYNC_DIFFUSE,
	    p_default,		TRUE,
	    p_ui,			TYPE_SINGLECHEKBOX,	IDC_SYNC_DIFFUSE,
	    p_accessor,		&gPSCMAccessor,
	    end,
   	CGFX_SYNC_SPECULAR, _T("SyncSpecular"),	TYPE_BOOL,	0,	IDS_SYNC_SPECULAR,
	    p_default,		TRUE,
	    p_ui,			TYPE_SINGLECHEKBOX,	IDC_SYNC_SPECULAR,
	    p_accessor,		&gPSCMAccessor,
	    end,
   	CGFX_SYNC_GLOSS,    _T("SyncGloss"),	TYPE_BOOL,	0,	IDS_SYNC_GLOSS,
	    p_default,		TRUE,
	    p_ui,			TYPE_SINGLECHEKBOX,	IDC_SYNC_GLOSS,
	    p_accessor,		&gPSCMAccessor,
	    end,
   	CGFX_SYNC_SELFILLUMINATION, _T("SyncSelfIllumination"),	TYPE_BOOL,	0,	IDS_SYNC_SELFILLUMINATION,
	    p_default,		TRUE,
	    p_ui,			TYPE_SINGLECHEKBOX,	IDC_SYNC_SELFILLUMINATION,
	    p_accessor,		&gPSCMAccessor,
	    end,
   	CGFX_SYNC_OPACITY, _T("SyncOpacity"),	TYPE_BOOL,	0,	IDS_SYNC_OPACITY,
	    p_default,		TRUE,
	    p_ui,			TYPE_SINGLECHEKBOX,	IDC_SYNC_OPACITY,
	    p_accessor,		&gPSCMAccessor,
	    end,
   	CGFX_SYNC_FILTERCOLOR, _T("SyncFilterColor"), TYPE_BOOL,	0,	IDS_SYNC_FILTERCOLOR,
	    p_default,		TRUE,
	    p_ui,			TYPE_SINGLECHEKBOX,	IDC_SYNC_FILTERCOLOR,
	    p_accessor,		&gPSCMAccessor,
	    end,
   	CGFX_SYNC_BUMP, _T("SyncBump"),	TYPE_BOOL,	0,	IDS_SYNC_BUMP,
	    p_default,		TRUE,
	    p_ui,			TYPE_SINGLECHEKBOX,	IDC_SYNC_BUMP,
	    p_accessor,		&gPSCMAccessor,
	    end,
   	CGFX_SYNC_REFLECTION, _T("SyncReflection"),	TYPE_BOOL,	0,	IDS_SYNC_REFLECTION,
	    p_default,		TRUE,
	    p_ui,			TYPE_SINGLECHEKBOX,	IDC_SYNC_REFLECTION,
	    p_accessor,		&gPSCMAccessor,
	    end,
   	CGFX_SYNC_REFRACTION,  _T("SyncRefraction"),	TYPE_BOOL,	0,	IDS_SYNC_REFRACTION,
	    p_default,		TRUE,
	    p_ui,			TYPE_SINGLECHEKBOX,	IDC_SYNC_REFRACTION,
	    p_accessor,		&gPSCMAccessor,
	    end,
   	CGFX_SYNC_DISPLACEMENT, _T("SyncDisplacement"),	TYPE_BOOL,	0,	IDS_SYNC_DISPLACEMENT,
	    p_default,		TRUE,
	    p_ui,			TYPE_SINGLECHEKBOX,	IDC_SYNC_DISPLACEMENT,
	    p_accessor,		&gPSCMAccessor,
	    end,



	end

	);

FPInterfaceDesc* ICgFXPlugin::GetDesc()
{
	return &cgfx_interface;
}

CgFXMaterial::CgFXMaterial()
: m_PBlock(NULL),
m_PBlockBackup(NULL),
m_pCgFXDataBridge(NULL),
m_pShaderInfo(NULL),
m_refMap(NULL),
m_bValidParams(false),
m_bPluginCommand(false),
m_dwVersion(CG_PLUGIN_FILEVERSION),
m_pLoadedParams(NULL),
m_pConsole(NULL)
{
	bLoaded = false;
	NVPROF_FUNC("CgFXMaterial::CgFXMaterial");

	// Add the material to the scene manager
	CSceneManager::GetSingletonPtr()->AddSink(this);

	m_pTweakables = new CTweakables(this);

	// Our vertex shader.
	m_pVS = new MaxVertexShader(this);

	m_pShaderInfo = new ShaderInfo(this);

		// Max Stuff
	gDefaultShaderDesc.MakeAutoParamBlocks(this);

	// The data bridge for passing info to the plugin
    m_pCgFXDataBridge = new CgFXDataBridge(m_pVS);

	// Call us when the time changes.
	GetCOREInterface()->RegisterTimeChangeCallback(this);

	// Search for CgFX files...  Not currently used.
#if 0
	FileSearch Searcher;
	for (i=0; i<TheManager->GetMapDirCount(); i++) 
	{
		TCHAR* dir = TheManager->GetMapDir(i);
		DISPDBG(3, "Searching dir: " << dir);
		Searcher.FindFile("*.fx", dir, false);
	}
#endif


}

CgFXMaterial::~CgFXMaterial()
{
	NVPROF_FUNC("CgFXMaterial::~CgFXMaterial");
	
	CSceneManager::GetSingletonPtr()->RemoveSink(this);
    
	DeleteAllRefsFromMe();

    SAFE_DELETE(m_pVS);
	SAFE_DELETE(m_pTweakables);
	SAFE_RELEASE(m_pConsole);

	GetCOREInterface()->UnRegisterTimeChangeCallback(this);


}

void CgFXMaterial::Reset()
{
	NVPROF_FUNC("CgFXMaterial::Reset");
	DeleteAllRefsFromMe();
}

bool CgFXMaterial::PreNewScene(void* pValue)
{
	g_bFirstMaterial = true;
	return true;
}

bool CgFXMaterial::NotifyRemoveNode(void* pValue)
{
	NVPROF_FUNC("CgFXMaterial::NotifyRemoveNode");

	if (pValue)
	{
		if (GetVertexShader())
		{
			if (GetVertexShader()->GetLighting())
			{
				GetVertexShader()->GetLighting()->CheckRemoveNode((ULONG)pValue);
			}
		}
	}
	else
	{
		// Post delete means we update
		if (GetTweakables())
			GetTweakables()->BuildGUI();
	}

	return true;
}

bool CgFXMaterial::NotifyAddNode(void* pValue)
{
	NVPROF_FUNC("CgFXMaterial::NotifyAddNode");

	bool bUpdated = false;
	if (GetVertexShader())
	{
		if (GetVertexShader()->GetLighting())
		{
			bUpdated = GetVertexShader()->GetLighting()->CheckAddNode((ULONG)pValue);
		}
	}
	
	if (bUpdated && GetTweakables())
	{
		GetTweakables()->BuildGUI();
	}

	return true;
}


// Returns the name of a parameter in the pblock.  Makes params appear correctly in the trackview.
TSTR PSCM_Accessor::GetLocalName(ReferenceMaker* owner, ParamID id, int tabIndex) 
{
	NVPROF_FUNC("PSCM_Accessor::GetLocalName");
	CgFXMaterial* pMat = (CgFXMaterial*)owner;
	ShaderInfoData* pShader = pMat->GetShaderInfo()->GetCurrentShader();
	return pShader->GetConnectionPBlock()->GetParamName(id, tabIndex).c_str();
}


void PSCM_Accessor::Set(PB2Value &v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)
{
	NVPROF_FUNC("PSCM_Accessor::Set");
	CgFXMaterial		*pMat;
	bool				Update = false;
	
	pMat = (CgFXMaterial*)owner;
	if(!pMat)
	{
		return;
	}

	ShaderInfoData* pShader = pMat->GetShaderInfo()->GetCurrentShader();

	DISPDBG(4, "PB:Set, Time: " << t << ", Index: " << tabIndex);

	switch(id)
	{
		case CGFX_FILE:
		{
			DISPDBG(4,"PB::Set - CGFX_FILE");

			if (v.s != NULL && (strlen(v.s) != 0) )
			{
				DISPDBG(4, "CGFX_FILE: " << v.s);

				if (!pMat->IsPluginCommand())
				{
					std::string strEffectRealPath = FindFile(v.s);
					if (strEffectRealPath.empty())
					{
						std::ostringstream strStream;
						strStream << "Failed to Find: " << v.s;
						MessageBox(NULL, strStream.str().c_str(), "Warning", MB_ICONEXCLAMATION | MB_OK);
					}
					else
					{
						pShader->LoadEffect_Ver2(OWNER_MATERIAL, strEffectRealPath.c_str());
					}
				}
			}
		}
		break;

		case CGFX_TECHNIQUE:
		{
			DISPDBG(4, "CGFX_TECHNIQUE: " << v.i);

			if (!pMat->IsPluginCommand())
				pShader->SetTechnique(OWNER_MATERIAL, v.i);
		}
		break;
		case CGFX_TECHNIQUENAME:
		{
			DISPDBG(4, "CGFX_TECHNIQUENAME: " << v.i);

			if (!pMat->IsPluginCommand())
				pShader->SetTechnique(OWNER_MATERIAL, v.s);
		}
		break;

		case CGFX_FLOAT:
		{
			DISPDBG(4, "CGFX_FLOAT: " << v.f);
		}
		break;

		case CGFX_INT:
		{
			DISPDBG(4, "CGFX_INT: " << v.i);
		}
		break;

		case CGFX_TEXTURE:
		{
			if (v.s && strlen(v.s))
				DISPDBG(4, "CGFX_TEXTURE: " << v.s);
			else
				DISPDBG(4, "CGFX_TEXTURE: <not valid>");
		}
		break;

		case CGFX_SYNC_DIFFUSE:
		{
			DISPDBG(4, "CGFX_SYNC_DIFFUSE: " << v.i);
			pMat->OverrideMaterial();
		}
		break;

		case CGFX_SYNC_SPECULAR:
		{
			DISPDBG(4, "CGFX_SYNC_SPECULAR: " << v.i);
			pMat->OverrideMaterial();
		}
		break;

		case CGFX_SYNC_BUMP:
		{
			DISPDBG(4, "CGFX_SYNC_BUMP: " << v.i);
			pMat->OverrideMaterial();
		}
		break;

		default:
		break;
	}
}

void PSCM_Accessor::Get(PB2Value &v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t, Interval &valid)
{
	NVPROF_FUNC("PSCM_Accessor::Get");
	CgFXMaterial		*pMat;
	bool				Update = false;
	
	pMat = (CgFXMaterial*)owner;
	if(!pMat)
	{
		return;
	}

	DISPDBG(4, "PB:Get, Time: " << t << ", Index: " << tabIndex);

	switch(id)
	{
		case CGFX_FILE:
		{
			DISPDBG(4,"CGFX_FILE");
		}
		break;

		case CGFX_TECHNIQUE:
		{
			DISPDBG(4, "CGFX_TECHNIQUE: " << v.i);
		}
		break;

		case CGFX_FLOAT:
		{
			DISPDBG(4, "CGFX_FLOAT: " << v.f);
		}
		break;

		case CGFX_INT:
		{
			DISPDBG(4, "CGFX_INT: " << v.i);
		}
		break;

       	case CGFX_TEXTURE:
		{
			if (v.s && strlen(v.s))
				DISPDBG(4, "CGFX_TEXTURE: " << v.s);
			else
				DISPDBG(4, "CGFX_TEXTURE: <not valid>");
		}
		break;

		default:
		break;
	}
}

// Accessors.
MaxVertexShader* CgFXMaterial::GetVertexShader()
{
	NVPROF_FUNC("CgFXMaterial::GetVertexShader");
	return m_pVS;
}

CTweakables* CgFXMaterial::GetTweakables()
{
	NVPROF_FUNC("CgFXMaterial::GetTweakables");
	return m_pTweakables;
}

// Given a connection into an effect, synchronise the parameter with the paramblock in max.
void CgFXMaterial::SyncMaterialToConnection(nv_sys::INVConnectionParameter* pParam)
{
	NVPROF_FUNC("CgFXMaterial::SyncMaterialToConnection");
	ShaderInfoData* pShader = m_pShaderInfo->GetCurrentShader();
	pShader->GetConnectionPBlock()->Synchronize(GetCOREInterface()->GetTime(), SYNC_PBLOCK, pParam);
}

void CgFXMaterial::EnableDialogItems()
{
	NVPROF_FUNC("CgFXMaterial::EnableDialogItems");
	if (!m_PBlock)
		return;

	bool bHaveTexture = false;
	bool bHaveMaterial = false;

	IParamMap2* pParamMap = m_PBlock->GetMap();
    if (!pParamMap)
        return;

    HWND hWndDlg = pParamMap->GetHWnd();
    HWND hWndText = GetDlgItem(hWndDlg, IDC_CGFXFILE);

    std::string strEffectName;
	ShaderInfoData* pShader = m_pShaderInfo->GetCurrentShader();
	if (!pShader->GetEffectFile().empty())
		strEffectName = GetFileNameAndExtension(pShader->GetEffectFile().c_str());

    SetWindowText(hWndText, strEffectName.c_str());
}

// Callbacks from the connection manager to ask us to orient positions/directions from one space to another
void CgFXMaterial::ConvertSpace(INVConnectionParameter* pParam, eANNOTATIONVALUEID InSpace, eANNOTATIONVALUEID OutSpace, eSEMANTICID SemanticID, const vec3& InVec, vec3& OutVec)
{
	NVPROF_FUNC("CgFXMaterial::ConvertSpace");
    vec4 Temp;
    ConvertSpace(pParam, InSpace, OutSpace, SemanticID, vec4(InVec.x, InVec.y, InVec.z, 1.0f), Temp);
    OutVec = vec3(Temp.x, Temp.y, Temp.z);
}

// Callbacks from the connection manager to ask us to orient positions/directions from one space to another
void CgFXMaterial::ConvertSpace(INVConnectionParameter* pParam, eANNOTATIONVALUEID InSpace, eANNOTATIONVALUEID OutSpace, eSEMANTICID SemanticID, const vec4& InVec, vec4& OutVec)
{
	NVPROF_FUNC("CgFXMaterial::ConvertSpace");
    if (InSpace == OutSpace)
    {
        OutVec = InVec;
        return;
    }

    mat4 matWorld = m_pVS->GetWorld();
    mat4 matView = m_pVS->GetView();
    mat4 matProjection = m_pVS->GetProjection();

    if (SemanticID != SEMANTICID_POSITION && 
        SemanticID != SEMANTICID_DIRECTION)
    {
        DISPDBG(1, "ERROR: Don't understand semantic for space conversion: " << pParam->GetDefaultValue().GetObjectSemantics()->GetName());
        OutVec = InVec;
        return;
    }

    switch(InSpace)
    {
        // This is the most common, because the viewer's lights are in device light space.
        case ANNOTATIONVALUEID_DEVICELIGHTSPACE:
        {
            switch(OutSpace)
            {
                case ANNOTATIONVALUEID_WORLD:
                {
                    // world->world in d3d
                    OutVec = InVec;
                    return;
                }
                break;

                case ANNOTATIONVALUEID_VIEW:
                {
                    // world->view in d3d
                    if (SemanticID == SEMANTICID_POSITION)
                    {
                        OutVec = matView * InVec;
                    }
                    else
                    {
                        mat4 matViewIT;
                        invert(matViewIT, matView);
                        //transpose(matViewIT, matViewIT);
                        OutVec = matViewIT * InVec;
                    }
                    return;
                }
                break;

                case ANNOTATIONVALUEID_PROJECTION:
                {
                    DISPDBG(1, "Not implemented space conversion for devicespace -> projection");
                }
                break;
            }
        }
        break;

        case ANNOTATIONVALUEID_WORLD:
        {
            switch(OutSpace)
            {
                case ANNOTATIONVALUEID_VIEW:
                {
                    if(SemanticID == SEMANTICID_POSITION)
                    {
                        OutVec = matView * InVec;
                    }
                    else
                    {
                        mat4 matViewIT;
                        invert(matViewIT, matView);
                        //transpose(matViewIT);
                        OutVec = matViewIT * InVec;
                    }
                    return;
                }
                break;

                case ANNOTATIONVALUEID_PROJECTION:
                {
                    mat4 matViewProjection;
                    matViewProjection = matProjection * matView;

                    if (SemanticID == SEMANTICID_POSITION)
                    {
                        OutVec = matViewProjection * InVec;
                    }
                    else
                    {
                        mat4 matViewProjectionIT;
                        invert(matViewProjectionIT, matViewProjection);
                        //transpose(matViewProjectionIT);
                        OutVec = matViewProjectionIT * InVec;
                    }
                    return;
                }
                break;

                case ANNOTATIONVALUEID_SCREEN:
                {
                    DISPDBG(1, "Not implemented space conversion for world -> screen");
                }
                break;

                case ANNOTATIONVALUEID_DEVICELIGHTSPACE:
                {
                    OutVec = InVec;
                }
                break;        
            }
        }
        break;

        case ANNOTATIONVALUEID_VIEW:
        {
            switch(OutSpace)
            {
                case ANNOTATIONVALUEID_WORLD:
                {
                    if (SemanticID == SEMANTICID_POSITION)
                    {
                        mat4 matViewI;
                        invert(matViewI, matView);

                        OutVec = matViewI * InVec;
                    }
                    else
                    {
                        mat4 matViewT = matView;
                        //transpose(matViewT);
                        OutVec = matViewT * InVec;
                    }
                    return;
                }
                break;

                case ANNOTATIONVALUEID_PROJECTION:
                {
                    if (SemanticID == SEMANTICID_POSITION)
                    {
                        OutVec = matProjection * InVec;
                    }
                    else
                    {
                        mat4 ProjectionIT;
                        invert(ProjectionIT, matProjection);
                        //transpose(ProjectionIT);
                        OutVec = ProjectionIT * InVec;
                    }
                    return;
                }
                break;

                case ANNOTATIONVALUEID_SCREEN:
                {
                    DISPDBG(1, "Not implemented space conversion for view -> screen");
                }
                break;

                case ANNOTATIONVALUEID_DEVICELIGHTSPACE:
                {
                    // In D3D Light space is world space
                    mat4 matViewI;
                    invert(matViewI, matView);

                    if (SemanticID == SEMANTICID_POSITION)
                    {
                        OutVec = matViewI * InVec;
                    }
                    else
                    {
                        mat4 matConvert = matView;
                        //transpose(matConvert);
                        OutVec = matConvert * InVec;
                    }
                }
                break;        
            }
        }
        break;

        case ANNOTATIONVALUEID_PROJECTION:
        {
            DISPDBG(1, "Not implemented space conversion for projection ->");
            switch(OutSpace)
            {
                case ANNOTATIONVALUEID_WORLD:
                {
                }
                break;

                case ANNOTATIONVALUEID_VIEW:
                {
                }
                break;

                case ANNOTATIONVALUEID_SCREEN:
                {
                }
                break;

                case ANNOTATIONVALUEID_DEVICELIGHTSPACE:
                {
                }
                break;        
            }
        }
        break;
        
        case ANNOTATIONVALUEID_SCREEN:
        {
            DISPDBG(1, "Not implemented space conversion for screen space ->");
            switch(OutSpace)
            {
                case ANNOTATIONVALUEID_WORLD:
                {
                }
                break;

                case ANNOTATIONVALUEID_VIEW:
                {
                }
                break;

                case ANNOTATIONVALUEID_PROJECTION:
                {
                   
                }
                break;

                case ANNOTATIONVALUEID_DEVICELIGHTSPACE:
                {
                }
                break;        
            }
        }
        break;

    }

    DISPDBG(1, "ERROR: Don't understand space conversion: " << pParam->GetDefaultValue().GetObjectSemantics()->GetName());
    OutVec = InVec;
}

// Callback from the connection manager to do any app-side initialisation of effect parameters.
// We use it to load textures specified in annotations
bool CgFXMaterial::EffectParamInit(LPD3DXEFFECT pEffect, NVType& Value )
{
	NVPROF_FUNC("CgFXMaterial::EffectParamInit");

	DISPDBG(5, "CCgFXView::EffectParamInit") ;

	// TIM: !! HACK ATTACK !!
	// Load fixed textures here for ps11 if the params exist
	// Case 1. Normalizer map
	if(Value.GetObjectSemantics()->GetSemanticID() == SEMANTICID_NORMALIZATIONMAP){
		//NVType TextureValue(Value);
		//TextureValue.SetTexture(0);
		//if (Value.GetTexture() != 0){
		//	TextureMgr::GetSingletonPtr()->Release((Value.GetTexture()));
			// Reset the texture
			//pParam->SetKey(t, TextureValue);
		//}
		INVTexture* pTex = m_pVS->LoadTexture("Normalizer.dds",NVTEXTURE_CUBE);
		if (pTex){
			Value.SetTexture(pTex);
			//SAFE_RELEASE(pTex);
			//pParam->SetKey(t, TextureValue);
		}
	} 
	// Case 2. Attenuation map
	if(Value.GetObjectSemantics()->GetSemanticID() == SEMANTICID_ATTENUATIONMAP){
		//NVType TextureValue(Value);
		//TextureValue.SetTexture(0);
		//if (Value.GetTexture() != 0){
		//	TextureMgr::GetSingletonPtr()->Release((Value.GetTexture()));
			// Reset the texture
//			pParam->SetKey(t, TextureValue);
		//}
		INVTexture* pTex = m_pVS->LoadTexture("Attenuation.dds",NVTEXTURE_3D);
		if (pTex){ 
			Value.SetTexture(pTex);
			//SAFE_RELEASE(pTex);
			//pParam->SetKey(t, TextureValue);
		}
	} 
 
	// Early out if we don't understand the type, or can't help.
	if ((Value.GetType() != NVTYPEID_TEXTURE) &&
		(Value.GetType() != NVTYPEID_TEXTURE2D) &&
		(Value.GetType() != NVTYPEID_TEXTURE3D) &&
		(Value.GetType() != NVTYPEID_TEXTURECUBE))
		return false;

	ostringstream strStream;
	const tAnnotationInfo* pFileInfo = Value.GetObjectSemantics()->FindAnnotationInfo(ANNOTATIONNAMEID_FILE);
	const tAnnotationInfo* pTextureType = Value.GetObjectSemantics()->FindAnnotationInfo(ANNOTATIONNAMEID_TEXTURETYPE);

	if (!pFileInfo)
		return false;

	// If the file isn't a string there's not much we can do.
	if (pFileInfo->m_Value.GetType() != NVTYPEID_STRING)
	{
		return false;
	}

	// Error if empty file specified
	std::string strFile = pFileInfo->m_Value.GetString();
	if (strFile.empty())
	{
		strStream << "FILE = ? in effect param: " << Value.GetObjectSemantics()->GetName();
		MessageBox(NULL, strStream.str().c_str(), "ERROR", MB_ICONEXCLAMATION | MB_OK);
		return false;
	}

	// Error if couldn't find the file
	std::string strTextureFilePath = FindFile(strFile);
	if (strTextureFilePath.empty())
	{
		strStream << "Couldn't find texture: " << strFile;
		MessageBox(NULL, strStream.str().c_str(), "ERROR", MB_ICONEXCLAMATION | MB_OK);
		return false;
	}

	NVTEXTURETARGETTYPE TargetType = NVTEXTURE_UNKNOWN;

	// Check the texture type annotation
	if (pTextureType && (pTextureType->m_Value.GetType() == NVTYPEID_STRING))
	{
		// Empty texture type annotation
		std::string strTextureType = pTextureType->m_Value.GetString();
		if (strTextureType.empty())
		{
			strStream << "TextureType = ? in effect param: " << Value.GetObjectSemantics()->GetName();
			MessageBox(NULL, strStream.str().c_str(), "ERROR", MB_ICONEXCLAMATION | MB_OK);
			return false;
		}

		if (!stricmp(strTextureType.c_str(), "1D"))
		{
			TargetType = NVTEXTURE_1D;
		}
		else if (!stricmp(strTextureType.c_str(), "2D"))
		{
			TargetType = NVTEXTURE_2D;
		}
		else if (!stricmp(strTextureType.c_str(), "3D"))
		{
			TargetType = NVTEXTURE_3D;
		}
		else if (!stricmp(strTextureType.c_str(), "CUBE"))
		{
			TargetType = NVTEXTURE_CUBE;
		}
		else if (!stricmp(strTextureType.c_str(), "RECT"))
		{
			TargetType = NVTEXTURE_CUBE;
		}
	}

	if (TargetType == NVTEXTURE_UNKNOWN)
	{
		switch(Value.GetType())
		{
			case NVTYPEID_TEXTURE:
				// Guess at 2d if no type specified.
				TargetType = NVTEXTURE_2D;
				break;
			case NVTYPEID_TEXTURE2D:
				TargetType = NVTEXTURE_2D;
				break;
			case NVTYPEID_TEXTURE3D:
				TargetType = NVTEXTURE_3D;
				break;
			case NVTYPEID_TEXTURECUBE:
				TargetType = NVTEXTURE_CUBE;
				break;
		}
	}

	if (!strTextureFilePath.empty() && (TargetType != NVTEXTURE_UNKNOWN))
	{
		INVTexture* pTex = m_pVS->LoadTexture(strTextureFilePath.c_str(), TargetType);
		Value.SetTexture(pTex);
		SAFE_RELEASE(pTex);
	}
	else
	{
		Value.SetTexture(NULL);
	}

	return true;
}

// Returns the validity of the paramblock, including the tab entries.
void CgFXMaterial::GetValidity(TimeValue t, Interval &valid)
{
	NVPROF_FUNC("CgFXMaterial::GetValidity");
	DISPDBG(5, "CgFXMaterial::GetValidity");
	ShaderInfoData* pShader = m_pShaderInfo->GetCurrentShader();
	pShader->GetConnectionPBlock()->GetValidity(t, valid);

	// Force validity failure
	if (!m_bValidParams)
	{
		m_bValidParams = true;
		valid = NEVER;
	}

}

// For dialog creation (which happens when the user brings up the panel in MAX)
ParamDlg* CgFXMaterial::CreateEffectDlg(HWND hwMtlEdit, IMtlParams *imp) 
{
	NVPROF_FUNC("CgFXMaterial::CreateEffectDlg");
	IAutoMParamDlg	*MasterDlg;
	IParamMap2		*Map;
	HWND			Wnd;
	DISPDBG(3, "CgFXMaterial::CreateEffectDlg");
	
	CgFXMaterialDlg * paramDlg = new CgFXMaterialDlg(this);
	ip = imp;

	MasterDlg  = gDefaultShaderDesc.CreateParamDlgs(hwMtlEdit,imp,this);
	MaterialParamBlk.SetUserDlgProc(paramDlg);

	Map	= MasterDlg->GetMap();
	Wnd	= Map->GetHWnd();

	ShaderInfoData* pShader = m_pShaderInfo->GetCurrentShader();

	TCHAR* pszFile;
	m_PBlock->GetValue(CGFX_FILE, m_pVS->GetTime(), pszFile, FOREVER);

	// Load the shader
	if (!pszFile || (strlen(pszFile) == 0))
	{
		std::string strEffectFile = "Diffuse.fx";
		pszFile = const_cast<TCHAR*>(strEffectFile.c_str());
    	
		m_vecLightStreamInfo.clear();

		m_bPluginCommand = true;
		m_PBlock->SetValue(CGFX_FILE, m_pVS->GetTime(), pszFile);
		m_bPluginCommand = false;
		m_pShaderInfo->GetCurrentShader()->LoadEffect_Ver2(OWNER_MATERIAL, pszFile);
	}
	if (g_bShowGUI && m_pTweakables)
		OnEditConnections();

	EnableDialogItems();

	// Get references to texmaps we want to watch
	IMtlEditInterface *mtlEdit = (IMtlEditInterface *)GetCOREInterface(MTLEDIT_INTERFACE);
	StdMat2 *mtl = (StdMat2*)mtlEdit->GetCurMtl();
	if (mtl)
	{
		for (int i = 0; i < mtl->NumSubs(); i++)
		{
			DISPDBG(1, (char*)mtl->SubAnimName(i));
			if (!stricmp((char*)mtl->SubAnimName(i), "Maps"))
			{
				MakeRefByID(FOREVER, ref_maps, reinterpret_cast<ReferenceTarget*>(mtl->SubAnim(i)));
				break;
			}
		}
	}

	MasterDlg->InvalidateUI();
	MasterDlg->MtlChanged();

	return(MasterDlg);	
}

// Helper function to get the D3D device.
LPDIRECT3DDEVICE9 CgFXMaterial::GetDevice()
{
	NVPROF_FUNC("CgFXMaterial::GetDevice");
	DISPDBG(3, "CgFXMaterial::GetDevice");	

	GraphicsWindow		*GW;
	ViewExp				*View;
	LPDIRECT3DDEVICE9	Device;

	View = GetCOREInterface()->GetActiveViewport();

	if(View)
	{
		GW = View->getGW();

		if(GW)
		{
			ID3D9GraphicsWindow *D3DGW = (ID3D9GraphicsWindow *)GW->GetInterface(D3D9_GRAPHICS_WINDOW_INTERFACE_ID);
		
			if(D3DGW)
			{
				Device = D3DGW->GetDevice();

				return(Device);
			}
		}
	}

	MessageBox(NULL, "Could not get DX9 Device?\nRunning with DX9 viewport in MAX required for Cg plugin.\nChange the MAX renderer to DX9 and restart.", "Error", MB_ICONEXCLAMATION | MB_OK);

	return(NULL);
}

//_____________________________________
//
//	Clone 
//
//_____________________________________
RefTargetHandle CgFXMaterial::Clone(RemapDir &remap) 
{
	// Trigger Deferred Load, if not yet loaded
	TriggerLoad();

	NVPROF_FUNC("CgFXMaterial::Clone");
	CgFXMaterial		*MNew;

	DISPDBG(3, "CgFXMaterial::Clone");	

	// Create a new material
	MNew = new CgFXMaterial();

	MNew->m_dwVersion = m_dwVersion;

	MNew->ReplaceReference(ref_pblock,remap.CloneRef(m_PBlock));
	MNew->ReplaceReference(ref_pblock_backup, remap.CloneRef(m_PBlockBackup));
	MNew->ReplaceReference(ref_maps, remap.CloneRef(m_refMap));
	
	MNew->m_pVS->SetDevice(m_pVS->GetDevice());
	
	// If we didn't find the device, get it from MAX.
	if(!MNew->m_pVS->GetDevice())
	{	
		MNew->m_pVS->SetDevice(GetDevice());
	}

	MNew->m_pVS->m_CurCache		= 0;
	MNew->m_pVS->m_MaxCache		= 0;
	MNew->m_pVS->SetTime(GetCOREInterface()->GetTime());

	ShaderInfoData* pThisShader = GetShaderInfo()->GetCurrentShader();
	string strEmpty;
	if (pThisShader)
	{
		MNew->GetShaderInfo()->SetShaderFromRef(MNew->m_PBlock->ID());
		MNew->GetShaderInfo()->GetCurrentShader()->LoadEffect_Ver2(OWNER_MATERIAL, pThisShader->GetEffectFile().c_str());
		MNew->GetShaderInfo()->GetCurrentShader()->SetTechnique(OWNER_MATERIAL, (char*)pThisShader->GetTechnique().c_str());

		// Rebuild the tweakables.  We know that the tweakables will never call load effect!
		MNew->GetTweakables()->BuildGUI();

		MNew->GetVertexShader()->StartWatching();
	}
	else
	{
		MNew->GetShaderInfo()->LoadEffect_Ver2(OWNER_MATERIAL, strEmpty, 0, MNew->m_PBlock);
	}


	GetCOREInterface()->RedrawViews(MNew->m_pVS->GetTime());

	return((RefTargetHandle)MNew);

}

	 
//_____________________________________
//
//	NotifyRefChanged 
//
//_____________________________________

RefResult CgFXMaterial::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
									   PartID& partID, RefMessage message ) 
{
	NVPROF_FUNC("CgFXMaterial::NotifyRefChanged");
	DISPDBG(3, "CgFXMaterial::NotifyRefChanged");	

	// If not ready (perhaps haven’t triggered deferred load), return
	if(!Ready())
		return (REF_SUCCEED);


	switch(message)
	{
		case REFMSG_NODE_MATERIAL_CHANGED:
		{
			DISPDBG(4, "REFMSG_NODE_MATERIAL_CHANGED");	
		}
		break;

		case REFMSG_SUBANIM_STRUCTURE_CHANGED:
		{
			DISPDBG(4, "REFMSG_NODE_MATERIAL_CHANGED");	
		}
		break;

		case REFMSG_NODE_WSCACHE_UPDATED:
		{
			DISPDBG(4, "REFMSG_NODE_WSCACHE_UPDATED");	
			/*
			if(m_pVS)
			{
				m_pVS->m_RMesh[m_pVS->m_CurCache].Invalidate();
			}*/
		}
		break;

		case REFMSG_TARGET_DELETED:
		{
			DISPDBG(4, "REFMSG_TARGET_DELETED");	
			if(m_pVS)
			{
				m_pVS->m_Mesh[m_pVS->m_CurCache] = NULL;
				m_pVS->m_Node[m_pVS->m_CurCache] = NULL;
			}
		}
		break;

		case REFMSG_CHANGE:
		{
			DISPDBG(4, "REFMSG_CHANGE");	

			// Did a texmap change?  If so, update any synchronised textures.
			if((hTarget == m_refMap) && (partID & PART_TEXMAP))
			{
				DISPDBG(3, "Changed TexMaps");	
				OverrideMaterial();
			}

			/*if (!m_bInDrawSetup)
				GetCOREInterface()->ForceCompleteRedraw();	
				*/
		}
		break;

		case REFMSG_WANT_SHOWPARAMLEVEL:
		{
			DISPDBG(4, "REFMSG_WANT_SHOWPARAMLEVEL");	

			if(hTarget == m_PBlock)
			{

				BOOL * pb = (BOOL*)(partID);
				*pb = TRUE;


				return REF_STOP;
			}
		}
		break;

		default:
			DISPDBG(4, "REFMSG_UNKNOWN" << message);	

			return REF_SUCCEED;
			break;
	}

	return(REF_SUCCEED);
}


// Save any non-pblock data.
// We store the shader text (for safe reload reasons), the shader file, and info on how to remap lights.
IOResult CgFXMaterial::Save(ISave *isave) 
{	
	NVPROF_FUNC("CgFXMaterial::Save");
	// Must be loaded or save will fail!!!
	TriggerLoad();

	ShaderInfoData* pShader = m_pShaderInfo->GetCurrentShader();

	if (!pShader->GetEffectFile().empty())
	{
		unsigned long Written;

		std::string strFile;
		const char* pszEffectPath = pShader->GetEffectFile().c_str();

		isave->BeginChunk(EFFECT_CHUNK);
		isave->WriteWString(pszEffectPath);
		isave->EndChunk();

		isave->BeginChunk(VERSION_CHUNK);
		DWORD dwVersion = CG_PLUGIN_FILEVERSION;
		isave->Write(&dwVersion, sizeof(DWORD), &Written);
		isave->EndChunk();

		
		vector<tLightStreamInfo> vecLightSave;
		if (m_pVS->GetLighting())
		{
			m_pVS->GetLighting()->GetSaveInfo(vecLightSave);

			vector<tLightStreamInfo>::iterator itrSave = vecLightSave.begin();
			while (itrSave != vecLightSave.end())
			{
				isave->BeginChunk(LIGHT_CHUNK);
				isave->Write(&(*itrSave), sizeof(tLightStreamInfo), &Written);
				isave->EndChunk();

				itrSave++;
			}
		}

		// Fill the connections with key info.
		pShader->GetConnectionPBlock()->FillConnectionKeys();
		INVParameterList* pParams = pShader->GetParameterList();

		SaveConnections(isave, pParams);
	}
	
	return(IO_OK);
}

// Called once the load is completed.
// Here the pblock is full, we've figured out the shader, have access to the text of the old shader file, and
// are ready to setup.
void CgFXMaterial::proc(ILoad *iload)
{ 
	NVPROF_FUNC("CgFXMaterial::Proc");
	EnableDialogItems();
}

//
// TIM: Triggers late load, used when user clicks material or displays it, etc
//
void CgFXMaterial::TriggerLoad(){
	if(bLoaded)
		return;
	bLoaded = true;

	int iTechnique;
	TCHAR* pszFile;

	if (g_bFirstMaterial)
	{
		if (m_dwVersion > CG_PLUGIN_FILEVERSION)
		{
			MessageBox(NULL, "This scene has been created with a later version of the Cg plugin!\nPlease install an updated Cg Plugin.", "Information", MB_ICONINFORMATION | MB_OK);
		}
		else if (m_dwVersion < CG_PLUGIN_FILEVERSION)
		{
			MessageBox(NULL, "This scene has been created with an earlier version of the Cg Plugin!\nPlease save the scene as soon as possible to update it.", "Information", MB_ICONINFORMATION | MB_OK);
		}
		g_bFirstMaterial = false;
	}

	// Old format plugin
	if (m_dwVersion < 2)
	{
		// Write a temporary file
		if (!m_strEffectText.empty())
		{
			assert(m_dwVersion < 1);
			DWORD dwWritten;
			
			char TempPath[MAX_PATH];
			GetTempPath(MAX_PATH, TempPath);

			std::string strPath = string(TempPath) + BACKUP_FILEPATH;
			HANDLE hFile = CreateFile(strPath.c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			WriteFile(hFile, m_strEffectText.c_str(), m_strEffectText.size(), &dwWritten, NULL);
			m_strBackupFilePath = strPath;
			CloseHandle(hFile);
		}
	}

    // Get the loaded technique
	char* tech = NULL;
	m_PBlock->GetValue(CGFX_TECHNIQUENAME, m_pVS->GetTime(), tech, FOREVER);
	string sTechnique;
	
	if(tech && strlen(tech)){
		sTechnique = tech;
	}
	else {
		// If no technique string (old file format, etc) use index
		m_PBlock->GetValue(CGFX_TECHNIQUE, m_pVS->GetTime(), iTechnique, FOREVER);

		// FIXME: Hack to get index into string, until we can dump indexes all together
		char buf[32];
		sprintf(buf,":%d",iTechnique);
		sTechnique = buf;
	}


	m_PBlock->GetValue(CGFX_FILE, m_pVS->GetTime(), pszFile, FOREVER);

	std::string strEffectRealPath = FindFile(pszFile);
	if(strEffectRealPath.empty())
		strEffectRealPath = pszFile;

	// Load the effect, taking into account the loaded text file, the technique, the required shader file, etc.
	if (m_dwVersion < 2)
	{
		m_pShaderInfo->LoadEffect(OWNER_MATERIAL, m_strBackupFilePath, strEffectRealPath, sTechnique, m_PBlock, m_PBlockBackup);
	}
	else
	{
		m_pShaderInfo->LoadEffect_Ver2(OWNER_MATERIAL, strEffectRealPath, sTechnique, m_PBlock);

		// If we loaded parameters, copy them across.
		if (m_pLoadedParams && m_pShaderInfo->GetCurrentShader() && m_pShaderInfo->GetCurrentShader()->GetParameterList())
		{
			ResolveConnections(m_pShaderInfo->GetCurrentShader()->GetParameterList(), m_pLoadedParams, true);
			
			SuspendAnimate();
			AnimateOn();
			m_pShaderInfo->GetCurrentShader()->GetConnectionPBlock()->SyncConnections(m_pShaderInfo->GetCurrentShader()->GetParameterList());
			AnimateOff();
			ResumeAnimate();
		}
		SAFE_RELEASE(m_pLoadedParams);
	}

	m_bPluginCommand = true;
	m_PBlock->SetValue(CGFX_FILE, m_pVS->GetTime(), const_cast<char*>(m_pShaderInfo->GetCurrentShader()->GetEffectFile().c_str()));
	m_PBlock->SetValue(CGFX_TECHNIQUENAME, m_pVS->GetTime(), static_cast<char*>((char*)m_pShaderInfo->GetCurrentShader()->GetTechnique().c_str()));
	m_bPluginCommand = false;

	// Kill the loaded light stream
	m_vecLightStreamInfo.clear();
}
 
//_____________________________________
//
//	Load 
//
//_____________________________________

IOResult CgFXMaterial::Load(ILoad *iload) 
{ 
	NVPROF_FUNC("CgFXMaterial::Load");
	//TCHAR			*Buf;
	TCHAR*			pszFilePath = NULL;
	TCHAR*          pszEffectPath = NULL;
	IOResult		Ret;
	Ret = IO_OK;
	 
	DISPDBG(3, "CgFXMaterial::Load");

	m_dwVersion = 0;

	unsigned long Read;
	m_strEffectText = "";
	assert(m_pVS);
	m_vecLightStreamInfo.clear();
	while(IO_OK == (Ret = iload->OpenChunk())) 
	{
		switch(iload->CurChunkID()) 
		{
			
			case EFFECT_CHUNK:
				Ret = iload->ReadWStringChunk(&pszEffectPath);
				m_bPluginCommand = true;
				m_PBlock->SetValue(CGFX_FILE, m_pVS->GetTime(), pszEffectPath, 0);
				m_bPluginCommand = false;
				break;
			case SHADER_CHUNK:
				TCHAR* pszShader;
				iload->ReadWStringChunk(&pszShader);
				m_strEffectText = pszShader;
				DISPDBG(3, "Loaded Shader for material: " << pszShader);
				break;
			case VERSION_CHUNK:
				iload->Read(&m_dwVersion, sizeof(DWORD), &Read);
				DISPDBG(3, "Version Cg file saved with: " << m_dwVersion);
				break;
			case LIGHT_CHUNK:
				{
					tLightStreamInfo NewInfo;
					iload->Read(&NewInfo, sizeof(tLightStreamInfo), &Read);
					m_vecLightStreamInfo.push_back(NewInfo);
				}
				break;
			case CONNECTION_CHUNK:
			{
				SAFE_RELEASE(m_pLoadedParams);
				m_pLoadedParams = LoadConnections(m_pVS, iload);
			}
			default:
				DISPDBG(1, "WARNING: Unrecognised Chunk: " << iload->CurChunkID());
				break;

		}


		iload->CloseChunk();

		if(Ret != IO_OK)
		{
			return(Ret);
		}
	}				


	if(!m_pVS->GetDevice())
	{	
		m_pVS->SetDevice(GetDevice());
	}

	iload->RegisterPostLoadCallback(this);

	return(IO_OK);
}

BaseInterface *CgFXMaterial::GetInterface(Interface_ID id)
{
	NVPROF_FUNC("CgFXMaterial::GetInterface");
	if (id == VIEWPORT_SHADER_CLIENT_INTERFACE) {
		return static_cast<IDXDataBridge*>(this);
	}
	else if (id == VIEWPORT_SHADER9_CLIENT_INTERFACE) {
		return static_cast<IDX9DataBridge*>(this);
	}
	else if(id == DX9_VERTEX_SHADER_INTERFACE_ID) 
	{
		return((IDX9VertexShader *)m_pVS);
	}
	else if (id == CGFX_DATA_CLIENT_INTERFACE) 
	{
		return static_cast<ICGFXDataBridge*>(m_pCgFXDataBridge);
	}
	else if (id == CGFXPLUGIN_INTERFACE)
	{
		return static_cast<ICgFXPlugin*>(this);
	}
	else
	{
		return(FPMixinInterface::GetInterface(id));
	}
}


//______________________________________
//
//	DlgProc 
//
//______________________________________

BOOL CgFXMaterialDlg::DlgProc(TimeValue t,IParamMap2 *parammap,HWND hwndDlg,UINT msg,WPARAM wParam,LPARAM lParam)		
{	
	NVPROF_FUNC("CgFXMaterial::DlgProc");
	CgFXMaterial* pMat;
	
	if (parammap && parammap->GetParamBlock())
	{
		pMat = (CgFXMaterial*)parammap->GetParamBlock()->GetOwner();
	}
	else
	{
		pMat = NULL;
	}
	
    switch(msg)
	{
		case WM_CLOSE:
		{
			assert(pMat);
			if (pMat)
				return pMat->OnCloseDialog(hwndDlg);
		}
		break;

		case WM_INITDIALOG:
		{
			assert(pMat);
			if (pMat)
				return pMat->OnInitDialog(hwndDlg);
		}
		break;

		case WM_COMMAND:
		{
			switch (LOWORD(wParam)) 
			{
				case IDC_EDITCONNECTIONS:
				{
					assert(pMat);
					if (pMat){
						pMat->OnEditConnections();
					}
				}
				break;

				case IDC_ABOUT:
				{
					assert(pMat);
					if (pMat)
						pMat->OnAbout();
				}
				break;

				case IDC_HELP:
				{
					assert(pMat);
					if (pMat)
						pMat->OnHelp();
				}
				break;

			}
		}
		break;

	}
	return false;
}

BOOL CgFXMaterial::OnCloseDialog(HWND hWndDlg)
{
	NVPROF_FUNC("CgFXMaterial::OnCloseDialog");
	assert(m_pTweakables);
	m_pTweakables->OnEvent("OnClosePanel", NULL);
	
	return TRUE;
}

BOOL CgFXMaterial::OnInitDialog(HWND hWndDlg)
{
	NVPROF_FUNC("CgFXMaterial::OnInitDialog");
	return TRUE;
}

void CgFXMaterial::OnEditConnections()
{
	NVPROF_FUNC("CgFXMaterial::OnEditConnections");
	// Trigger Deferred Load, if not yet loaded
	TriggerLoad();

	assert(m_pTweakables);
	m_pTweakables->OnShowTweakables();
}

long GetRegKey(HKEY key, LPCTSTR subkey, LPTSTR retdata)
{
	NVPROF_FUNC("GetRegKey");
    HKEY hkey;
    long retval = RegOpenKeyEx(key, subkey, 0, KEY_QUERY_VALUE, &hkey);

    if (retval == ERROR_SUCCESS) {
        long datasize = MAX_PATH;
        TCHAR data[MAX_PATH];
        RegQueryValue(hkey, NULL, data, &datasize);
        lstrcpy(retdata,data);
        RegCloseKey(hkey);
    }

    return retval;
}

HINSTANCE GotoURL(LPCTSTR url, int showcmd)
{
	NVPROF_FUNC("GotoURL");
    TCHAR key[MAX_PATH + MAX_PATH];

    // First try ShellExecute()
    HINSTANCE result = 0;//ShellExecute(NULL, _T("open"), url, NULL,NULL, showcmd);

    // If it failed, get the .htm regkey and lookup the program
    if ((UINT)result <= HINSTANCE_ERROR) {

        if (GetRegKey(HKEY_CLASSES_ROOT, _T(".htm"), key) == ERROR_SUCCESS) {
            lstrcat(key, _T("\\shell\\open\\command"));

            if (GetRegKey(HKEY_CLASSES_ROOT,key,key) == ERROR_SUCCESS) {
                TCHAR *pos;
                pos = _tcsstr(key, _T("\"%1\""));
                if (pos == NULL) {                     // No quotes found
                    pos = strstr(key, _T("%1"));       // Check for %1, without quotes 
                    if (pos == NULL)                   // No parameter at all...
                        pos = key+lstrlen(key)-1;
                    else
                        *pos = '\0';                   // Remove the parameter
                }
                else
                    *pos = '\0';                       // Remove the parameter

                lstrcat(pos, _T(" "));
                lstrcat(pos, url);
                result = (HINSTANCE) WinExec(key,showcmd);
            }
        }
    }

    return result;
}

void CgFXMaterial::OnAbout()
{
	NVPROF_FUNC("CgFXMaterial::OnAbout");
	if (!m_pConsole)
	{
		m_pConsole = nv_gui::INVDebugConsole::CreateInstance();

			m_pConsole->SetTitle("About Reality HLSL Plugin");
			m_pConsole->OutputDebug("Reality HLSL MAX plugin");
			m_pConsole->OutputDebug("Feedback to: tim@artificialstudios.com");
			
			m_pConsole->OutputDebug(" ");

			m_pConsole->SetVisible(true);
	}
	else
	{
		m_pConsole->SetVisible(true);
	}

}

void CgFXMaterial::OnHelp()
{
	NVPROF_FUNC("CgFXMaterial::OnHelp");
	std::string strHelpFile = FindFile("CgFX_maxplugin_v12.htm");
	if (strHelpFile.empty())
	{
		MessageBox(NULL, "Couldn't find help file: CgFX_maxplugin_v12.htm", "Error", MB_OK | MB_ICONEXCLAMATION);
		return;
	}

	GotoURL(strHelpFile.c_str(), SW_SHOW);
}

// Need to know about time changes for redraws if the shader is dependant on changing 
// parameters with time.
void CgFXMaterial::TimeChanged(TimeValue t) 
{
	// If not ready (perhaps haven’t triggered deferred load), return
	if(!Ready())
		return;

	NVPROF_FUNC("CgFXMaterial::TimeChanged");
	Interval ivalid = FOREVER;

	GetSYSInterface()->SetTime(t);

	ShaderInfoData* pShader = m_pShaderInfo->GetCurrentShader();
	pShader->GetConnectionPBlock()->Synchronize(t, SYNC_CONNECTION, NULL);

	assert(m_pTweakables);
	m_pTweakables->RefreshGUI();

	GetValidity(t,ivalid);
	if(ivalid.Start()==ivalid.End() || m_pVS->IsTimeDependant())
	{
		NotifyDependents(FOREVER,PART_MTL,REFMSG_CHANGE);
	}

/*
	std::ostringstream strStream;

	g_NVProfManager.StreamData(strStream);

	// Output to a file
	ofstream dbgLog("g:\\stats.log", std::ios::out | std::ios::trunc);	// Open a log file for debug messages
	dbgLog << strStream.str() << std::ends;
	dbgLog.flush();
	*/
}

// Sample code provided by discreet for finding the viewport manager's owner.
#define MSEMULATOR_CLASS_ID	Class_ID(0x6de34e16, 0x4796bc9a)
#define VIEWPORTLOADER_CLASS_ID Class_ID(0x5a06293c, 0x30420c1e)
MtlBase * CgFXMaterial::GetMaterialOwner(ReferenceTarget * targ)
{
	NVPROF_FUNC("CgFXMaterial::GetMaterialOwner");
	ICustAttribContainer * cc = NULL;
	CustAttrib * vl = NULL;
	RefList &list = targ->GetRefList();
	RefListItem *ptr = list.first;

	// find the viewport manager first
	while (ptr) 
	{
		if (ptr->maker) 
		{
			  if (ptr->maker->ClassID()==VIEWPORTLOADER_CLASS_ID ) 
			  {
					vl = (CustAttrib*)ptr->maker;
			  }
		}
		ptr = ptr->next;
	}

	// now find the ViewportManager owner - which should be a Custom Attribute Container
	if(!vl) 
		return NULL;

	RefList &nlist = vl->GetRefList();
	RefListItem *nptr = nlist.first;

	while (nptr) 
	{
		if (nptr->maker) 
		{
			if (nptr->maker->ClassID()==Class_ID(0x5ddb3626, 0x23b708db) ) 
			{
				cc = (ICustAttribContainer*)nptr->maker;
			}
		}
		nptr = nptr->next;
	}

	if(cc)
		return (MtlBase *) cc->GetOwner();        // the owner will be the material
	return NULL;
}

// Access to the standard material
StdMat2* CgFXMaterial::GetStandardMaterial()
{
	NVPROF_FUNC("CgFXMaterial::GetStandardMaterial");
	return static_cast<StdMat2*>(GetMaterialOwner(this));
}

// Override the CgFX material parameters with the stdmaterial if enabled.
// Includes auto-conversion of bump maps to normal maps.
void CgFXMaterial::OverrideMaterial()
{
	NVPROF_FUNC("CgFXMaterial::OverrideMaterial");
	Texmap* pTexMap = NULL;

   	BOOL SyncDiffuse, SyncSpecular, SyncBump, SyncAmbient, SyncGloss, SyncSelfIllumination, SyncOpacity, SyncFilterColor,SyncReflection,SyncRefraction,SyncDisplacement;
	m_PBlock->GetValue(CGFX_SYNC_AMBIENT,0,SyncAmbient,FOREVER);
    m_PBlock->GetValue(CGFX_SYNC_DIFFUSE,0,SyncDiffuse,FOREVER);
    m_PBlock->GetValue(CGFX_SYNC_SPECULAR,0,SyncSpecular,FOREVER);
	m_PBlock->GetValue(CGFX_SYNC_GLOSS,0,SyncGloss,FOREVER);
	m_PBlock->GetValue(CGFX_SYNC_SELFILLUMINATION,0,SyncSelfIllumination,FOREVER);
	m_PBlock->GetValue(CGFX_SYNC_OPACITY,0,SyncOpacity,FOREVER);
	m_PBlock->GetValue(CGFX_SYNC_FILTERCOLOR,0,SyncFilterColor,FOREVER);
    m_PBlock->GetValue(CGFX_SYNC_BUMP,0,SyncBump,FOREVER);
	m_PBlock->GetValue(CGFX_SYNC_REFLECTION,0,SyncReflection,FOREVER);
	m_PBlock->GetValue(CGFX_SYNC_REFRACTION,0,SyncRefraction,FOREVER);
	m_PBlock->GetValue(CGFX_SYNC_DISPLACEMENT,0,SyncDisplacement,FOREVER);

	TCHAR* pszAmbientMap = NULL;
    TCHAR* pszDiffuseMap = NULL;
    TCHAR* pszSpecularMap = NULL;
	TCHAR* pszGlossMap = NULL;
	TCHAR* pszSelfIlluminationMap = NULL;
	TCHAR* pszOpacityMap = NULL;
	TCHAR* pszFilterColorMap = NULL;
    TCHAR* pszBumpMap = NULL;
	TCHAR* pszReflectionMap = NULL;
	TCHAR* pszRefractionMap = NULL;
	TCHAR* pszDisplacementMap = NULL;

	StdMat2* mtl = GetStandardMaterial();
	if(!mtl)
		return;

  	macroRecorder->Disable();
	TimeValue t = GetCOREInterface()->GetTime();

	int AmbientChannel = mtl->StdIDToChannel(ID_AM);
	int DiffuseChannel = mtl->StdIDToChannel(ID_DI);
	int SpecularChannel = mtl->StdIDToChannel(ID_SP);
	int GlossChannel = mtl->StdIDToChannel(ID_SH);
	int SelfIlluminationChannel = mtl->StdIDToChannel(ID_SI);
	int OpacityChannel = mtl->StdIDToChannel(ID_OP);
	int FilterColorChannel = mtl->StdIDToChannel(ID_FI);
	int BumpChannel = mtl->StdIDToChannel(ID_BU);
	int ReflectionChannel = mtl->StdIDToChannel(ID_RL);
	int RefractionChannel = mtl->StdIDToChannel(ID_RR);
	int DisplacementChannel = mtl->StdIDToChannel(ID_DP);

    // Ambient Texture
    if(AmbientChannel != -1)
	{
		pTexMap = mtl->GetSubTexmap(AmbientChannel);
		if (pTexMap && pTexMap->ClassID() == Class_ID(BMTEX_CLASS_ID, 0))
		{
			BitmapTex* pBitmap = static_cast<BitmapTex*>(pTexMap);
			pszAmbientMap = pBitmap->GetMapName();
		}
	}

    // Diffuse Texture
    if(DiffuseChannel != -1)
	{
		pTexMap = mtl->GetSubTexmap(DiffuseChannel);
		if (pTexMap && pTexMap->ClassID() == Class_ID(BMTEX_CLASS_ID, 0))
		{
			BitmapTex* pBitmap = static_cast<BitmapTex*>(pTexMap);
			pszDiffuseMap = pBitmap->GetMapName();
		}
	}

    // Specular Texture
    if(SpecularChannel != -1)
	{
		pTexMap = mtl->GetSubTexmap(SpecularChannel);
		if (pTexMap && pTexMap->ClassID() == Class_ID(BMTEX_CLASS_ID, 0))
		{
			BitmapTex* pBitmap = static_cast<BitmapTex*>(pTexMap);
			pszSpecularMap = pBitmap->GetMapName();
		}
	}

    // Gloss Texture
    if(GlossChannel != -1)
	{
		pTexMap = mtl->GetSubTexmap(GlossChannel);
		if (pTexMap && pTexMap->ClassID() == Class_ID(BMTEX_CLASS_ID, 0))
		{
			BitmapTex* pBitmap = static_cast<BitmapTex*>(pTexMap);
			pszGlossMap = pBitmap->GetMapName();
		}
	}

    // Self Illum
    if(SelfIlluminationChannel != -1)
	{
		pTexMap = mtl->GetSubTexmap(SelfIlluminationChannel);
		if (pTexMap && pTexMap->ClassID() == Class_ID(BMTEX_CLASS_ID, 0))
		{
			BitmapTex* pBitmap = static_cast<BitmapTex*>(pTexMap);
			pszSelfIlluminationMap = pBitmap->GetMapName();
		}
	}

    // Opacity
    if(OpacityChannel != -1)
	{
		pTexMap = mtl->GetSubTexmap(OpacityChannel);
		if (pTexMap && pTexMap->ClassID() == Class_ID(BMTEX_CLASS_ID, 0))
		{
			BitmapTex* pBitmap = static_cast<BitmapTex*>(pTexMap);
			pszOpacityMap = pBitmap->GetMapName();
		}
	}

    // FilterColor
    if(FilterColorChannel != -1)
	{
		pTexMap = mtl->GetSubTexmap(FilterColorChannel);
		if (pTexMap && pTexMap->ClassID() == Class_ID(BMTEX_CLASS_ID, 0))
		{
			BitmapTex* pBitmap = static_cast<BitmapTex*>(pTexMap);
			pszFilterColorMap = pBitmap->GetMapName();
		}
	}

    // Bump Texture
    if(BumpChannel != -1)
	{
		pTexMap = mtl->GetSubTexmap(BumpChannel);
		if (pTexMap && pTexMap->ClassID() == Class_ID(BMTEX_CLASS_ID, 0))
		{
			BitmapTex* pBitmap = static_cast<BitmapTex*>(pTexMap);
			pszBumpMap = pBitmap->GetMapName();
		}
	}

	// Reflection texture
    if(ReflectionChannel != -1)
	{
		pTexMap = mtl->GetSubTexmap(ReflectionChannel);
		if (pTexMap && pTexMap->ClassID() == Class_ID(BMTEX_CLASS_ID, 0))
		{
			BitmapTex* pBitmap = static_cast<BitmapTex*>(pTexMap);
			pszReflectionMap = pBitmap->GetMapName();
		}
	}

	// Refraction texture
    if(RefractionChannel != -1)
	{
		pTexMap = mtl->GetSubTexmap(RefractionChannel);
		if (pTexMap && pTexMap->ClassID() == Class_ID(BMTEX_CLASS_ID, 0))
		{
			BitmapTex* pBitmap = static_cast<BitmapTex*>(pTexMap);
			pszRefractionMap = pBitmap->GetMapName();
		}
	}

	// Displacement texture
    if(DisplacementChannel != -1)
	{
		pTexMap = mtl->GetSubTexmap(DisplacementChannel);
		if (pTexMap && pTexMap->ClassID() == Class_ID(BMTEX_CLASS_ID, 0))
		{
			BitmapTex* pBitmap = static_cast<BitmapTex*>(pTexMap);
			pszDisplacementMap = pBitmap->GetMapName();
		}
	}
 
	ShaderInfoData* pShader = m_pShaderInfo->GetCurrentShader();
	INVParameterList* pParams = pShader->GetParameterList();

	if (!pParams)
		return;

    for (unsigned int i = 0; i < pParams->GetNumParameters(); i++)
    {
        INVConnectionParameter* pParam = pParams->GetConnectionParameter(i);

        if ((pParam->GetDefaultValue().GetType() == NVTYPEID_TEXTURE) ||
			(pParam->GetDefaultValue().GetType() == NVTYPEID_TEXTURE2D) ||
			(pParam->GetDefaultValue().GetType() == NVTYPEID_TEXTURE3D) ||
			(pParam->GetDefaultValue().GetType() == NVTYPEID_TEXTURECUBE))
        {
            switch(pParam->GetDefaultValue().GetObjectSemantics()->GetSemanticID())
            {
                case SEMANTICID_AMBIENTMAP:
                    if (SyncAmbient && pszAmbientMap)
                    {
                        INVTexture* pTex = m_pVS->LoadTexture(pszAmbientMap, pParam);
                        pParam->SetKey(t, NVType::CreateTextureType(pTex));
                    }
                    break;
                case SEMANTICID_DIFFUSEMAP:
                    if (SyncDiffuse && pszDiffuseMap)
                    {
                        INVTexture* pTex = m_pVS->LoadTexture(pszDiffuseMap, pParam);
                        pParam->SetKey(t, NVType::CreateTextureType(pTex));
                    }
                    break;
                case SEMANTICID_SPECULARMAP:
                    if (SyncSpecular && pszSpecularMap)
                    {
                        INVTexture* pTex = m_pVS->LoadTexture(pszSpecularMap, pParam);
                        pParam->SetKey(t, NVType::CreateTextureType(pTex));
                    }
                    break;
                case SEMANTICID_GLOSSMAP:
                    if (SyncGloss && pszGlossMap)
                    {
                        INVTexture* pTex = m_pVS->LoadTexture(pszGlossMap, pParam);
                        pParam->SetKey(t, NVType::CreateTextureType((pTex)));
                    }
                    break;
                case SEMANTICID_SELFILLUMINATIONMAP:
                    if (SyncSelfIllumination && pszSelfIlluminationMap)
                    {
                        INVTexture* pTex = m_pVS->LoadTexture(pszSelfIlluminationMap, pParam);
                        pParam->SetKey(t, NVType::CreateTextureType((pTex)));
                    }
                    break;
                case SEMANTICID_OPACITYMAP:
                    if (SyncOpacity && pszOpacityMap)
                    {
                        INVTexture* pTex = m_pVS->LoadTexture(pszOpacityMap, pParam);
                        pParam->SetKey(t, NVType::CreateTextureType((pTex)));
                    }
                    break;
                case SEMANTICID_FILTERCOLORMAP:
                    if (SyncFilterColor && pszFilterColorMap)
                    {
                        INVTexture* pTex = m_pVS->LoadTexture(pszFilterColorMap, pParam);
                        pParam->SetKey(t, NVType::CreateTextureType((pTex)));
                    }
                    break;

					// Map the bump map to a normal map
                case SEMANTICID_NORMAL:
                    if (SyncBump && pszBumpMap)
                    {
                        INVTexture* pTex = m_pVS->LoadBumpTexture(pszBumpMap, pParam);
                        pParam->SetKey(t, NVType::CreateTextureType((pTex)));
                    }
                    break;
                case SEMANTICID_ENVMAP:
                    if (SyncReflection && pszReflectionMap)
                    {
                        INVTexture* pTex = m_pVS->LoadTexture(pszReflectionMap, pParam);
                        pParam->SetKey(t, NVType::CreateTextureType((pTex)));
                    }
                    break;
                case SEMANTICID_REFRACTIONMAP:
                    if (SyncRefraction && pszRefractionMap)
                    {
                        INVTexture* pTex = m_pVS->LoadTexture(pszRefractionMap, pParam);
                        pParam->SetKey(t, NVType::CreateTextureType((pTex)));
                    }
                    break;
            }
        }
    }

	// Copy the updated connection params into the pblock
	pShader->GetConnectionPBlock()->Synchronize(t, SYNC_PBLOCK, NULL);

	macroRecorder->Enable();
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);

}

void CgFXMaterial::SetDXData(IHardwareMaterial * pHWMtl, Mtl * pMtl)
{
	NVPROF_FUNC("CgFXMaterial::SetDXData");
	TimeValue t = GetCOREInterface()->GetTime();
}

RefTargetHandle CgFXMaterial::GetReference(int i)
{
	NVPROF_FUNC("CgFXMaterial::GetReference");
	if (i == ref_pblock)
		return m_PBlock;
	else if (i == ref_pblock_backup)
		return m_PBlockBackup;
	
	return m_refMap; 
}

void CgFXMaterial::SetReference(int i, RefTargetHandle rtarg)
{ 
	NVPROF_FUNC("CgFXMaterial::SetReference");
	DISPDBG(3, "CgFXMaterial::SetReference: " << i << ", " << rtarg);
	if (i == ref_pblock) 
	{
		m_PBlock = (IParamBlock2*)rtarg; 
		m_pShaderInfo->AddParamRef(ref_pblock);

		// This is our current shader
		m_pShaderInfo->SetShaderFromRef(ref_pblock);

		if (m_PBlock)
			assert(m_PBlock->ID() == ref_pblock);
	}
	else if (i == ref_pblock_backup)
	{
		m_PBlockBackup = (IParamBlock2*)rtarg;
		
		m_pShaderInfo->AddParamRef(ref_pblock_backup);

		if (m_PBlockBackup)
			assert(m_PBlockBackup->ID() == ref_pblock_backup);
	}
	else if (i == ref_maps)
	{
		m_refMap = rtarg;
	}
	else
		assert(!"Setting a reference we don't know!"); 
}

// ShaderUpdateCallbacks
bool CgFXMaterial::OnSetTechnique(unsigned int Technique)
{
	NVPROF_FUNC("CgFXMaterial::OnSetTechnique");
	m_PBlock->SetValue(CGFX_TECHNIQUE, m_pVS->GetTime(), static_cast<int>(Technique));
	return true;
}

// ShaderUpdateCallbacks
bool CgFXMaterial::OnSetTechnique(const char* Technique)
{
	NVPROF_FUNC("CgFXMaterial::OnSetTechnique");
	m_PBlock->SetValue(CGFX_TECHNIQUENAME, m_pVS->GetTime(), const_cast<char*>(Technique));
	return true;
}

bool CgFXMaterial::OnLoadEffect(const char* pszPath)
{
	NVPROF_FUNC("CgFXMaterial::OnLoadEffect");
	m_bPluginCommand = true;
	m_PBlock->SetValue(CGFX_FILE, m_pVS->GetTime(), const_cast<char*>(pszPath));
	m_bPluginCommand = false;
	EnableDialogItems();
	return true;
}

int	CgFXMaterial::fnGetNumTechniques()
{
	NVPROF_FUNC("CgFXMaterial::fnGetNumTechniques");
	return m_pShaderInfo->GetCurrentShader()->GetNumTechniques();
}

int	CgFXMaterial::fnGetNumParameters()
{
	NVPROF_FUNC("CgFXMaterial::fnGetNumParameters");
	return m_pShaderInfo->GetCurrentShader()->GetNumParameters();
}

FPValue CgFXMaterial::fnGetParameter(int index)
{
	NVPROF_FUNC("CgFXMaterial::fnGetParameter");
	FPValue retval = FPValue();

	INVParameterList* pParamList = m_pShaderInfo->GetCurrentShader()->GetParameterList();
	if (!pParamList)
		throw MAXException("No valid parameters for this effect");

	if (index >= pParamList->GetNumParameters())
		throw MAXException("the index is not within the valid range");

	INVConnectionParameter* pParam = pParamList->GetConnectionParameter(index);

	NVType Value = pParam->GetValueAtTime(GetCOREInterface()->GetTime());

	switch (Value.GetType())
	{
		case NVTYPEID_TEXTURE:
		case NVTYPEID_TEXTURE2D:
		case NVTYPEID_TEXTURE3D:
		case NVTYPEID_TEXTURECUBE:
			retval.type = (ParamType2)TYPE_TSTR;
			retval.tstr = new CStr(TextureMgr::GetSingletonPtr()->GetTextureName(Value.GetTexture()));
			break;
		case NVTYPEID_INT:
			retval.type = (ParamType2)TYPE_INT;
			retval.i = Value.GetInt();
			break;
		case NVTYPEID_BOOL:
			retval.type = (ParamType2)TYPE_BOOL;
			retval.b = Value.GetBool();
			break;
		case NVTYPEID_STRING:
			retval.type = (ParamType2)TYPE_TSTR;
			retval.tstr = new CStr(Value.GetString());
			break;
		case NVTYPEID_DWORD:
			retval.type = (ParamType2)TYPE_DWORD;
			retval.d = Value.GetDWORD();
			break;
		case NVTYPEID_FLOAT: 
			retval.type = (ParamType2)TYPE_FLOAT;
			retval.f = Value.GetFloat();
			break;
		case NVTYPEID_VEC3: 
			retval.type = (ParamType2)TYPE_POINT3;
			retval.p = new Point3(Value.GetVec3().x, Value.GetVec3().y, Value.GetVec3().z);
			break;
		case NVTYPEID_VEC2: 
			retval.type = (ParamType2)TYPE_POINT2;
			retval.p2 = new Point2(Value.GetVec2().x, Value.GetVec2().y);
			break;
		default: 
			retval.type = (ParamType2)TYPE_TSTR;
			retval.tstr = new CStr("Can't retrieve value");
			break;
	}

	return retval;
}

void CgFXMaterial::fnSetParameter(int index, FPValue Value)
{
	NVPROF_FUNC("CgFXMaterial::fnSetParameter");
	INVParameterList* pParamList = m_pShaderInfo->GetCurrentShader()->GetParameterList();
	if (!pParamList)
		throw MAXException("No valid parameters for this effect");

	if (index >= pParamList->GetNumParameters())
		throw MAXException("the index is not within the valid range");

	INVConnectionParameter* pParam = pParamList->GetConnectionParameter(index);

	TimeValue t = GetCOREInterface()->GetTime();

	switch (pParam->GetDefaultValue().GetType())
	{
		case NVTYPEID_TEXTURE:
		case NVTYPEID_TEXTURE2D:
		case NVTYPEID_TEXTURE3D:
		case NVTYPEID_TEXTURECUBE:
			if (Value.type == TYPE_STRING)
			{
				if (GetVertexShader())
				{
					INVTexture* pTex = GetVertexShader()->LoadTexture(Value.s, pParam);
					if (pTex)
					{
						NVType TextureValue;
						TextureValue.SetTexture((pTex));
						pParam->SetKey(t, TextureValue);
					}
				}
			}
			break;
		case NVTYPEID_INT:
			if (Value.type == TYPE_INT)
			{
				pParam->SetKey(t, NVType::CreateIntType(Value.i));
			}
			break;
		case NVTYPEID_BOOL:
			if (Value.type == TYPE_BOOL)
			{
				pParam->SetKey(t, NVType::CreateBoolType(Value.b));
			}
			break;
		case NVTYPEID_STRING:
			if (Value.type == TYPE_STRING)
			{
				pParam->SetKey(t, NVType::CreateStringType((char*)Value.s));
			}
			break;
		case NVTYPEID_DWORD:
			if (Value.type == TYPE_DWORD)
			{
				pParam->SetKey(t, NVType::CreateDWORDType(Value.d));
			}
			break;
		case NVTYPEID_FLOAT: 
			if (Value.type == TYPE_FLOAT)
			{
				pParam->SetKey(t, NVType::CreateFloatType(Value.f));
			}
			break;
		case NVTYPEID_VEC3: 
			if (Value.type == TYPE_POINT3)
			{
				Point3 point = *Value.p;
				pParam->SetKey(t, NVType::CreateVec3Type(vec3(point[0], point[1], point[2])));
			}
			break;
		case NVTYPEID_VEC2: 
			if (Value.type == TYPE_POINT2)
			{
				Point2 point = *Value.p2;
				pParam->SetKey(t, NVType::CreateVec2Type(vec2(point[0], point[1])));
			}
			break;
		default: 
			break;
	}

	// Sync the pblock to the change
	if (GetShaderInfo()->GetCurrentShader())
	{
		GetShaderInfo()->GetCurrentShader()->GetConnectionPBlock()->Synchronize(t, SYNC_PBLOCK, pParam);
	}

	// Refresh the UI
	if (GetTweakables())
	{
		GetTweakables()->RefreshGUI();
	}
}

TCHAR* CgFXMaterial::fnGetParameterName(int index)
{
	NVPROF_FUNC("CgFXMaterial::fnGetParameterName");
	INVParameterList* pParamList = m_pShaderInfo->GetCurrentShader()->GetParameterList();
	if (!pParamList)
		throw MAXException("No valid parameters for this effect");

	if (index >= pParamList->GetNumParameters())
		throw MAXException("the index is not within the valid range");

	INVConnectionParameter* pParam = pParamList->GetConnectionParameter(index);
	return (TCHAR*)pParam->GetDefaultValue().GetObjectSemantics()->GetName();
}

TCHAR* CgFXMaterial::fnGetParameterSemantic(int index)
{
	NVPROF_FUNC("CgFXMaterial::fnGetParameterSemantic");
	INVParameterList* pParamList = m_pShaderInfo->GetCurrentShader()->GetParameterList();
	if (!pParamList)
		throw MAXException("No valid parameters for this effect");

	if (index >= pParamList->GetNumParameters())
		throw MAXException("the index is not within the valid range");

	INVConnectionParameter* pParam = pParamList->GetConnectionParameter(index);
	return (TCHAR*)ConvertSemantic(pParam->GetDefaultValue().GetObjectSemantics()->GetSemanticID());
}
