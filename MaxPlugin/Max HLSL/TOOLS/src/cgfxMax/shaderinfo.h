/*********************************************************************NVMH4****
NVSDK not found!
Path:  
File:  shaderinfo.h

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

See shaderinfo.cpp



******************************************************************************/

#ifndef __SHADERINFO_H
#define __SHADERINFO_H

typedef enum
{
	OWNER_NONE,
	OWNER_MATERIAL
} OWNER_ID;

class ShaderInfo;
class CConnectionPBlock;
class ShaderInfoData
{
public:
	ShaderInfoData(ShaderInfo* pParent);
	~ShaderInfoData();

	LPD3DXEFFECT GetEffect() const { return m_pEffect; }
	const std::string& GetEffectFile() const { return m_strShaderFile; }
	nv_sys::INVConnectionManager* GetConnectionManager() const { return m_pConnectionManager; }
	const D3DXEFFECT_DESC& GetDesc() const { return m_Desc; }
	nv_sys::INVParameterList* GetParameterList() const { return m_pParams; }
	CConnectionPBlock* GetConnectionPBlock() const { return m_pConnectionPBlock; }
	string GetTechnique() const { return m_Technique; }
	unsigned int GetNumTechniques() const { return m_Desc.Techniques; }
	unsigned int GetNumParameters() const { if (m_pParams) return m_pParams->GetNumParameters(); else return 0;}
	const char* GetTechniqueName(unsigned int Technique) const;
	void* GetVSData() const { return m_pVSData; } 
	
	void SetVSData(void* pData) { m_pVSData = pData; }
	bool SetTechnique(OWNER_ID Owner, unsigned int Technique);
	bool SetTechnique(OWNER_ID Owner, const char* Technique);
	bool LoadEffect(OWNER_ID Owner, const char* pszPath, bool bStartup = false);
	bool FreeEffect(OWNER_ID Owner, bool bStartup = false);

	bool LoadEffect_Ver2(OWNER_ID Owner, const char* pszPath);
	bool FreeEffect_Ver2(OWNER_ID Owner);

private:
	std::string m_strShaderFile;
	std::string m_strShaderText;
	LPD3DXEFFECT m_pEffect;
	nv_sys::INVConnectionManager*	m_pConnectionManager;
	nv_sys::INVParameterList*	m_pParams;
	D3DXEFFECT_DESC m_Desc;
	CConnectionPBlock* m_pConnectionPBlock;
	string m_Technique;
	ShaderInfo* m_pParent;
	void* m_pVSData;
};

typedef std::map<int, ShaderInfoData*> tmapRefToShaderInfoData;

#ifdef _DEBUG
#define DUMPPBLOCK(a, b) DumpPBlock(a, b);
#define DUMPPARAMETERLIST(a) DumpParameterList(a);
#else
#define DUMPPBLOCK(a, b)
#define DUMPPARAMETERLIST(a)
#endif


class ShaderInfo
{
public:
	typedef enum eSwapOption
	{
		UPDATE_GUI,
		NO_UPDATE_GUI
	} eSwapOption;

	ShaderInfo(CgFXMaterial* pMat);
	~ShaderInfo();
	void AddParamRef(int Ref);
	ShaderInfoData* GetShaderFromRef(int Ref);
	bool SetShaderFromRef(int Ref);
	void DumpPBlock(IParamBlock2* pb, const char*);
	void DumpParameterList(nv_sys::INVParameterList* pParams);

	ShaderInfoData* GetCurrentShader() const { return m_itrCurrent->second; }
	CgFXMaterial* GetMaterial() { return m_pMat; }
	bool CopyPBlock(IParamBlock2* pDest, IParamBlock2* pSource);
	bool ResolvePBlocks(IParamBlock2* pDest, IParamBlock2* pSource);
	int GetCurrentRef();

	void UpdateEffect();
	bool SwapEffect(OWNER_ID Owner, std::string strPath, eSwapOption SwapOption);
	bool LoadEffect(OWNER_ID Owner, const std::string& strBackupFile, const std::string strFileName, string sTechnique, IParamBlock2* pDestBlock, IParamBlock2* pSourceBlock);
	bool FreeEffect(OWNER_ID Owner, bool bStartup);

	void UpdateEffect_Ver2();
	bool SwapEffect_Ver2(OWNER_ID Owner, std::string strPath, eSwapOption SwapOption);
	bool LoadEffect_Ver2(OWNER_ID Owner, const std::string strFileName, string sTechnique, IParamBlock2* pDestBlock);
	bool FreeEffect_Ver2(OWNER_ID Owner);
	

private:
	void FreeShader(ShaderInfoData* pData);
	void AssignPB2Value(IParamBlock2* pb, int i, int tabIndex, PB2Value& vs);

	unsigned int m_CurrentShader;
	tmapRefToShaderInfoData m_mapRefToShaderInfoData;
	tmapRefToShaderInfoData::iterator m_itrCurrent;
	CgFXMaterial* m_pMat;
	
};

#endif __SHADERINFO_H
