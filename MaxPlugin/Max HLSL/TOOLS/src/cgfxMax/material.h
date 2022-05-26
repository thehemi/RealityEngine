/*********************************************************************NVMH4****
NVSDK not found!
Path:  
File:  material.h

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


See material.cpp


******************************************************************************/

#ifndef _CGFXMATERIAL_H
#define _CGFXMATERIAL_H

#include "tweakables.h"
#include "icgfx.h"

#if _MSC_VER >= 1000
#pragma once
#endif 

#define	 MAX_SHADER_CLASS_ID	Class_ID(0x3a689b7b, 0xdbaa6dd)
#define	 EFFECT_CHUNK 5000
#define  SHADER_CHUNK 5010
#define  LIGHT_CHUNK 5020
#define  CONNECTION_CHUNK 5040
#define  VERSION_CHUNK 5050

#define MAX_NAME 256
typedef struct tagLightStreamInfo
{
	DWORD Reserved;
	nv_sys::eSEMANTICID SemanticID;
	nv_sys::eANNOTATIONVALUEID ObjectID;
	char szName[MAX_NAME];
	DWORD SceneLightHandle;
} tLightStreamInfo;	


typedef struct tagConnectionHeaderInfo
{
	nv_sys::eSEMANTICID SemanticID;
	char szName[MAX_NAME];
	unsigned int NumValues;

} tConnectionStreamInfo;

typedef std::vector<tLightStreamInfo> tvecLightStreamInfo;

static const WORD MESSAGE_FILECHANGE = WM_USER + 1000;
typedef enum 
{ 
	CGFX_FILE = 0,
	CGFX_TECHNIQUE = 1,
	CGFX_CONNECTIONS = 2,
	CGFX_FLOAT = 3,
	CGFX_TEXTURE = 4,
	CGFX_INT = 5,
	CGFX_SYNC_DIFFUSE = 6,
    CGFX_SYNC_SPECULAR = 7,
	CGFX_SYNC_BUMP = 8,
    CGFX_SYNC_GLOSS = 9,
	CGFX_SYNC_SELFILLUMINATION = 10,
	CGFX_SYNC_OPACITY = 11,
	CGFX_SYNC_FILTERCOLOR = 12,
	CGFX_SYNC_AMBIENT = 13, 
	CGFX_SYNC_REFLECTION = 14,
	CGFX_SYNC_REFRACTION = 15,
	CGFX_SYNC_DISPLACEMENT = 16,
	CGFX_TECHNIQUENAME = 17,
	CGFX_INVALID = 0x7FFFFFFF
} eMaterialPBlock;

enum
{
	ref_pblock = 0,
	ref_pblock_backup = 1,
	ref_maps = 2	
};

#define		SLIDER_SCALE			1000.0f

class CgFXDataBridge;
class CConnectionPBlock;

typedef std::map<FileKey, LPD3DXEFFECT> tmapFileEffect;

#define CG_PLUGIN_FILEVERSION 2

class CgFXMaterialDlg : public ParamMap2UserDlgProc
{
	CgFXMaterial * shader;
public:
	CgFXMaterialDlg(CgFXMaterial * ms){shader = ms;}
	void Update(TimeValue t){};
	BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
	void DeleteThis(){delete this;}

};

#define BACKUP_FILEPATH "xx123_backup_xx123.fx"
class CgFXMaterial : public ReferenceTarget, public IDX9DataBridge, public PostLoadCallback, public nv_sys::INVEffectParamInitCallback,
						 public TimeChangeCallback, public ICgFXPlugin, public INotificationCallback
{
public:
	bool bLoaded;
	bool Ready(){ return bLoaded; }
	void TriggerLoad();

	//Constructor/Destructor
	CgFXMaterial();
	~CgFXMaterial();	

	void DeleteThis() { delete this; }		

	// Loading/Saving chunks
	IOResult Load(ILoad *iload);
	IOResult Save(ISave *isave);

	// Callback when our dialog is created.
	BOOL OnInitDialog(HWND hWndDlg);
	BOOL OnCloseDialog(HWND hWndDlg);
	void OnEditConnections();
	void OnHelp();
	void OnAbout();


	// Published functions
	virtual int	fnGetNumTechniques();
	virtual int fnGetNumParameters();
	virtual FPValue fnGetParameter(int index);
	virtual void fnSetParameter(int index, FPValue Value);
	virtual TCHAR* fnGetParameterName(int index);
	virtual TCHAR* fnGetParameterSemantic(int index);


	//From Animatable
	Class_ID ClassID()							{ return(MAX_SHADER_CLASS_ID);}		
	SClass_ID SuperClassID()					{ return(REF_TARGET_CLASS_ID); }
	void GetClassName(std::string& s)			{ s = GetString(IDS_BPSCLASS_NAME);}
	
	// IDX9DataBridge
	virtual float GetDXVersion() { return 9.0f; }	// DX 8.1 or 9.0 etc...

	// Subanims
	virtual	int NumSubs()					
	{
		return 2; 
	}

	virtual	Animatable* SubAnim(int i)		
	{
		if (i == 0) 
			return m_PBlock;
		else if (i == 1)
			return m_PBlockBackup;

		return NULL; 
	}

	virtual TSTR SubAnimName(int i)			{ if (i == 0) return GetString(IDS_PARAMS); else return ""; } 
	int SubNumToRefNum(int subNum)			{ return subNum;	}

	virtual int NumRefs() { return 3; }
	virtual RefTargetHandle GetReference(int i);
	virtual void SetReference(int i, RefTargetHandle rtarg);

	int	NumParamBlocks() { return 2; }		
	IParamBlock2* GetParamBlock(int i)			
	{
		if (i == ref_pblock) 
			return m_PBlock; 
		else if (i == ref_pblock_backup)
			return m_PBlockBackup;
	
		return NULL;
	} 

	IParamBlock2* GetParamBlockByID(BlockID id) 
	{
		if (m_PBlock->ID() == id) 
			return m_PBlock;
		else if (m_PBlockBackup->ID() == id)
			return m_PBlockBackup;

		return NULL;
	}

	void EnableDialogItems();
	RefTargetHandle Clone( RemapDir &remap );
	RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
							   PartID& partID,  RefMessage message);

	void GetValidity(TimeValue t, Interval &valid);
    void SyncMaterialToConnection(nv_sys::INVConnectionParameter* pParam = NULL);
	virtual void Reset();
	
	LPDIRECT3DDEVICE9	GetDevice();
	BaseInterface		*GetInterface(Interface_ID id);

	StdMat2* GetStandardMaterial();
	MtlBase * GetMaterialOwner(ReferenceTarget * targ);

	void SetDXData(IHardwareMaterial * pHWMtl, Mtl * pMtl);
	ParamDlg * CreateEffectDlg(HWND hWnd, IMtlParams * imp);
	void DisableUI(){};

	TCHAR * GetName(){return _T("HLSL Material");}

	void TimeChanged(TimeValue t);
	void OverrideMaterial();

	MaxVertexShader* GetVertexShader();
	CTweakables* GetTweakables();

	ShaderInfo* GetShaderInfo() { return m_pShaderInfo; };

	// IEffectParamInitCallback
	virtual bool EffectParamInit(LPD3DXEFFECT pEffect, nv_sys::NVType& Value );
    virtual void ConvertSpace(nv_sys::INVConnectionParameter* pParam, nv_sys::eANNOTATIONVALUEID InSpace, nv_sys::eANNOTATIONVALUEID OutSpace, nv_sys::eSEMANTICID SemanticID, const vec3& InVec, vec3& OutVec);
    virtual void ConvertSpace(nv_sys::INVConnectionParameter* pParam, nv_sys::eANNOTATIONVALUEID InSpace, nv_sys::eANNOTATIONVALUEID OutSpace, nv_sys::eSEMANTICID SemanticID, const vec4& InVec, vec4& OutVec);
	virtual void proc(ILoad *iload);
	virtual void InvalidateParams() { m_bValidParams = true; }
	
	virtual void InDrawSetup(bool bInSetup) { m_bInDrawSetup = bInSetup; }

	// INotificationCallback
	virtual bool NotifyAddNode(void* pValue);
	virtual bool NotifyRemoveNode(void* pValue);
	virtual bool PreNewScene(void* pValue);

	bool NotifyNodeRemoved(ULONG hNode);

	bool OnSetTechnique(unsigned int Technique);
	bool OnSetTechnique(const char* Technique);
	bool OnLoadEffect(const char* pszPath);
	
	bool IsPluginCommand() const { return m_bPluginCommand; }

	tvecLightStreamInfo& GetLightStreamInfo() { return m_vecLightStreamInfo; }

	DWORD GetFileVersion() { return m_dwVersion; }
private:
	DWORD m_dwVersion;
	// Parameter block
	IParamBlock2		*m_PBlock;					
	IParamBlock2		*m_PBlockBackup;

	nv_gui::INVDebugConsole* m_pConsole;
	bool				m_bPluginCommand;
	tvecLightStreamInfo	m_vecLightStreamInfo;

	MaxVertexShader*    m_pVS;
	IMtlParams			*ip;
	CTweakables*		m_pTweakables;
    std::string         m_strLoadedEffectPath;

	RefTargetHandle		m_refMap;

	std::string			m_strEffectFile;
	std::string			m_strEffectText;

	ShaderInfo* m_pShaderInfo;
	// CgFX Data Bridge
	CgFXDataBridge* m_pCgFXDataBridge; 

	bool				m_bInDrawSetup;
	bool				m_bValidParams;
	std::string m_strBackupFilePath;

	nv_sys::INVParameterList* m_pLoadedParams;

};

extern FPInterfaceDesc cgfx_interface;

class DefaultClassDesc:public ClassDesc2 
{
	public:
	int 			IsPublic()					 { return(1); }
	void *			Create(BOOL loading = FALSE) { AddInterface(&cgfx_interface); return(new CgFXMaterial()); }
	const TCHAR*	ClassName()					 { return(GetString(IDS_BPSCLASS_NAME)); }
	SClass_ID		SuperClassID()				 { return(REF_TARGET_CLASS_ID); }
	Class_ID		ClassID()					 { return(MAX_SHADER_CLASS_ID); }
	const TCHAR* 	Category()					 { return(_T("DXViewportEffect"));} // MUST BE DXViewportEffect
	const TCHAR*	InternalName()				 { return(_T("Reality HLSL")); }	
	HINSTANCE		HInstance()					 { return(g_hInstance); }				

};

class PSCM_Accessor : public PBAccessor
{
	public:
		virtual TSTR GetLocalName(ReferenceMaker* owner, ParamID id, int tabIndex);
		virtual void Get(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t, Interval &valid);
		virtual void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t);

};


#endif 
