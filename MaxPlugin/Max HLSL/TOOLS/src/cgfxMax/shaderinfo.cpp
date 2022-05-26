/*********************************************************************NVMH4****
Path:  plugins/nvmax
File:  shaderinfo.cpp

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

A centralized manager of plugin setup.  The plugin is trying to manage several things
at once, so a central control was needed to clear up confusion about who does what 
when.  It could still do with improvement...

The plugin is trying to manage:
a) sync of connection editor GUI to the connection parameters and vice versa
b) sync of connection parameters to the max paramblock and vice versa
c) sync of the rendering/harware setup to the connection parameters.

The main work this code is doing is enabling swapping of effects and loading of the first
effect.  This is tricky because it involves copying tab entries in paramblocks around.
Note that the function assignpb2value is key here....

******************************************************************************/

#include "pch.h"
#include "material.h"
#include "RenderMesh.h"
#include "vertexshader.h"
#include "connectionpblock.h"
#include "effectmgr.h"
#include "connections.h"
#include <io.h> // access()

using namespace nv_fx;
using namespace nv_sys;

using namespace std;

#define SHADER_FILE "Diffuse.fx"

string BrowseForFile(HWND hwnd, string heading, char* filter, string startFile){
	string location;
	TCHAR szFileName[MAX_PATH];
	szFileName[0] = _T('\0');
	strcpy(szFileName,startFile.c_str());

	OPENFILENAME ofn;

	memset(&ofn,0,sizeof(ofn));
	ofn.lStructSize       = sizeof(ofn);
	ofn.hwndOwner         = hwnd;
	ofn.lpstrFilter       = filter;
	ofn.nFilterIndex      = 1;
	ofn.lpstrFile         = szFileName;
	ofn.lpstrTitle		  = heading.c_str();
	ofn.nMaxFile          = sizeof(szFileName);
	ofn.Flags			  = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	if(!GetOpenFileName(&ofn))
		return "";
	location = szFileName;
	return location;
}

void DoMissingFile(string& strFileName){
	char buf[512];
	sprintf(buf,"Can't find shader file '%s', you will be prompted to locate it.\n\nNote: In future you may want to add the location of your .fx directory to the MAX search paths\nTo do this:\nFrom MAX’s main menu; Customize, Configure Paths…, External Files tab, Add…, then add the paths\nTypically this will be something like: C:\\Helix Core\\Shaders\\Standard",strFileName.c_str());
	MessageBox(0,buf,"Missing shader",MB_ICONEXCLAMATION | MB_OK);
	strFileName = BrowseForFile(0,"Locate .fx file...","HLSL Files (*.fx)\0*.fx\0All Files (*.*)\0*.*\0\0",strFileName);	
}


ShaderInfo::ShaderInfo(CgFXMaterial* pMat)
: m_pMat(pMat)
{
	NVPROF_FUNC("ShaderInfo::ShaderInfo");
	m_itrCurrent = m_mapRefToShaderInfoData.end();
}

ShaderInfo::~ShaderInfo()
{
	NVPROF_FUNC("ShaderInfo::~ShaderInfo");
	tmapRefToShaderInfoData::iterator itrShaders = m_mapRefToShaderInfoData.begin();
	while (itrShaders != m_mapRefToShaderInfoData.end())
	{
		delete itrShaders->second;
		itrShaders++;
	}
	m_mapRefToShaderInfoData.clear();

}

void ShaderInfo::AddParamRef(int Ref)
{
	NVPROF_FUNC("ShaderInfo::AddParamRef");
	tmapRefToShaderInfoData::iterator itrPBlock = m_mapRefToShaderInfoData.find(Ref);
	if (itrPBlock != m_mapRefToShaderInfoData.end())
		return;

	m_mapRefToShaderInfoData[Ref] = new ShaderInfoData(this);
}

ShaderInfoData* ShaderInfo::GetShaderFromRef(int Ref)
{
	NVPROF_FUNC("ShaderInfo::GetShaderFromRef");
	tmapRefToShaderInfoData::iterator itrPBlock = m_mapRefToShaderInfoData.find(Ref);
	if (itrPBlock == m_mapRefToShaderInfoData.end())
		return NULL;

	return itrPBlock->second;
}

bool ShaderInfo::SetShaderFromRef(int Ref)
{
	NVPROF_FUNC("ShaderInfo::SetShaderFromRef");
	tmapRefToShaderInfoData::iterator itrPBlock = m_mapRefToShaderInfoData.find(Ref);
	if (itrPBlock == m_mapRefToShaderInfoData.end())
		return false;

	m_itrCurrent = itrPBlock;

	// Tell the pblock syncer which param block it syncs to.
	itrPBlock->second->GetConnectionPBlock()->SetInfo(m_pMat, (IParamBlock2*)m_pMat->GetReference(Ref));

	return true;
}

int ShaderInfo::GetCurrentRef()
{
	NVPROF_FUNC("ShaderInfo::GetCurrentRef");
	if (m_itrCurrent == m_mapRefToShaderInfoData.end())
		return ref_pblock;

	return m_itrCurrent->first;
}

bool ShaderInfo::CopyPBlock(IParamBlock2* pDest, IParamBlock2* pSource)
{
	NVPROF_FUNC("ShaderInfo::CopyPBlock");
	assert(pDest != pSource);
	assert(pDest->NumParams() == pSource->NumParams());

	assert(GetMaterial()->GetFileVersion() < 2);

	for (int i = 0; i < pSource->NumParams(); i++)
	{
		ParamDef& SourceDef = pSource->GetParamDef(i);
		ParamDef& DestDef = pDest->GetParamDef(i);

		assert(pDest->GetParameterType(i) == pSource->GetParameterType(i));

		// Walk the tabs, or copy across.  Either way we make a duplicate from 
		// table 1 to table 2.
		if((SourceDef.type & TYPE_TAB) && (DestDef.type & TYPE_TAB))
		{
			pDest->Resize(i, pSource->Count(i));
			assert(pSource->Count(i) == pDest->Count(i));

			for (int tabindex = 0; tabindex < pSource->Count(i); tabindex++)
			{
				AssignPB2Value(pDest, i, tabindex, pSource->GetPB2Value(i, tabindex));
			}
		}
		else
		{
			// Don't copy non-tab entries
			//AssignPB2Value(pDest, i, 0, pSource->GetPB2Value(i, 0));
		}
	}

	return true;
}

void ShaderInfo::DumpParameterList(INVParameterList* pParams)
{
	NVPROF_FUNC("ShaderInfo::DumpParameterList");
	for (unsigned int i = 0; i < pParams->GetNumParameters(); i++)
	{
		INVConnectionParameter* pParam = pParams->GetConnectionParameter(i);
		CConnectionPBlock* pBlockCurrent = GetCurrentShader()->GetConnectionPBlock();
		DynamicPBlockEntry* pDynamic = pBlockCurrent->GetPBlockEntry(pParam);
		if (pDynamic)
		{
			DISPDBG_ALWAYS(1, "ConnectionParam: " << i << ", Name=" << pParam->GetDefaultValue().GetObjectSemantics()->GetName() << ", location, index: " << pDynamic->GetPBlockLocation() << ", " << pDynamic->GetPBlockOffset());
			if ((pParam->GetDefaultValue().GetType() == NVTYPEID_TEXTURE) ||
				(pParam->GetDefaultValue().GetType() == NVTYPEID_TEXTURE2D) ||
				(pParam->GetDefaultValue().GetType() == NVTYPEID_TEXTURE3D) ||
				(pParam->GetDefaultValue().GetType() == NVTYPEID_TEXTURECUBE))
			{
				DISPDBG(1, "TEXTURE: " << pParam->GetValueAtTime(GetCOREInterface()->GetTime()).GetTexture() << ", :" << TextureMgr::GetSingletonPtr()->GetTextureName((pParam->GetValueAtTime(GetCOREInterface()->GetTime()).GetTexture())));
			}
		}
		else
		{
			DISPDBG_ALWAYS(0, "ConnectionParam (NO PBLOCK ENTRY): " << i << ", Name=" << pParam->GetDefaultValue().GetObjectSemantics()->GetName());
		}
	}
}

void ShaderInfo::DumpPBlock(IParamBlock2* pb, const char* pszName)
{
	NVPROF_FUNC("ShaderInfo::DumpPBlock");
	DISPDBG_ALWAYS(0, " ");
	DISPDBG_ALWAYS(0, "** ShaderInfo::DumpPBlock ** : " << pszName);

	CConnectionPBlock* pBlockConnection = GetCurrentShader()->GetConnectionPBlock();
	for (int i = 0; i < pb->NumParams(); i++)
	{
		ParamDef& pd = pb->GetParamDef(i);

		if (pd.type & TYPE_TAB)
		{
			DISPDBG_ALWAYS(3, "tab table size: " << pb->Count(i));
			for (int tab = 0; tab < pb->Count(i); tab++)
			{
				PB2Value& val = pb->GetPB2Value(i, tab);

				if (!(pd.flags & P_ANIMATABLE) || !pb->GetController(i, tab))
				{
					switch (base_type(pd.type))
					{
						case TYPE_STRING:
						case TYPE_FILENAME:
							if (val.s)
							{
								DISPDBG_ALWAYS(0, "Param: " << i << ", Tab: " << tab << ", Value=STRING: " << val.s);
							}
							break;
						case TYPE_FLOAT:
							DISPDBG_ALWAYS(0, "Param: " << i << ", Tab: " << tab << ", Value=FLOAT: " << val.f);
							break;

						default:
							break;
					}
				}
				else
				{
					Control* pControl = pb->GetController(i, tab);
					if (pControl)
					{
						TSTR ClassName;
						pControl->GetClassName(ClassName);
						DISPDBG_ALWAYS(0, "Controller: " << i << ", Tab: " << tab << ", Class=" << (char*)ClassName);
					}
				}
			}
		}
	}
	DISPDBG_ALWAYS(0, "** ShaderInfo::~DumpPBlock **");
	DISPDBG_ALWAYS(0, " ");

} 

void ShaderInfo::AssignPB2Value(IParamBlock2* pb, int i, int tabIndex, PB2Value& vs)
{
	assert(GetMaterial()->GetFileVersion() < 2);

	NVPROF_FUNC("ShaderInfo::AssignPB2Value");
	Interval ivalid;
	ParamDef& pd = pb->GetParamDef(i);
	ParamType2 type = pd.type;
	PB2Value& vd = pb->GetPB2Value(i, tabIndex);

	DISPDBG(3, "AssignPB2Value: Index: " << i << ", tab: " << tabIndex);

	// Constant, so copy
	if (vs.is_constant())
	{
		// source constant
		switch (base_type(type))
		{
			// copy appropriately
			case TYPE_HSV:
			case TYPE_RGBA:
			case TYPE_POINT3:
				vd.p = new Point3(*vs.p);
				break;
			case TYPE_MATRIX3:
				vd.m = new Matrix3(*vs.m);
				break;

			case TYPE_STRING:
			case TYPE_FILENAME:
				vd.s = (vs.s == NULL) ? NULL : _tcscpy((TCHAR*)malloc(strlen(vs.s) + sizeof(TCHAR)), vs.s);
				break;

			case TYPE_BITMAP:
				vd.bm = (vs.bm == NULL) ? NULL : vs.bm->Clone();
				break;

			case TYPE_MTL:
			case TYPE_TEXMAP:
			case TYPE_INODE:
			case TYPE_REFTARG:
			case TYPE_PBLOCK2:
				if (vs.flags & P_NO_REF)
					vd.r = vs.r;
				else
					pb->ReplaceReference(pb->GetRefNum(i, tabIndex), vs.r);
				break;
			default:
				vd = vs;
		}
		vd.flags = vs.flags;
	}
	else
	{
		if (pd.flags & P_ANIMATABLE)
		{
			// is_constant has checked for a non-controller.
			DISPDBG(3, "REFERENCING CONTROL: ");
			vd = vs;
			pb->MakeRefByID(FOREVER, pb->GetControllerRefNum(i, tabIndex), vs.control);
		}
		else
		{
			// Not animatable
			switch (base_type(type))
			{
				case TYPE_FLOAT:
				case TYPE_ANGLE:
				case TYPE_PCNT_FRAC:
				case TYPE_WORLD:
				case TYPE_COLOR_CHANNEL:
					DISPDBG(3, "Copying FLOAT: " << vs.f);
					vs.control->GetValue(0, &vd.f, ivalid);
					break;
				case TYPE_INT:
				case TYPE_BOOL:
				case TYPE_TIMEVALUE:
				case TYPE_RADIOBTN_INDEX:
				case TYPE_INDEX:
					vs.control->GetValue(0, &vd.i, ivalid);
					break;
				case TYPE_POINT3:
			    case TYPE_RGBA:
				case TYPE_HSV:
				{
					Point3 p;
					vs.control->GetValue(0, &p, ivalid);
					vd.p = new Point3 (p);
				}
				break;

				default:
					break;
			}
			
		}
	}		

}

bool ShaderInfo::ResolvePBlocks(IParamBlock2* pDestPBlock, IParamBlock2* pSourcePBlock)
{
	assert(GetMaterial()->GetFileVersion() < 2);

	NVPROF_FUNC("ShaderInfo::ResolvePBlocks");
	DISPDBG(3, "ShaderInfo::ResolvePBlocks");

	assert(pDestPBlock != pSourcePBlock);

	// Get the current shader info for source and dest.
	ShaderInfoData* pSourceData = GetShaderFromRef(pSourcePBlock->ID());
	ShaderInfoData* pDestData = GetShaderFromRef(pDestPBlock->ID());
	
	// Get the parameter lists for the source and dest.
	INVParameterList* pSourceParams = pSourceData->GetParameterList();
    if (pSourceParams==NULL)
        return false;
	INVParameterList* pDestParams = pDestData->GetParameterList();

	// Get the pblock<->connection syncer for the source/dest.
	CConnectionPBlock* pDestConnectionPBlock = pDestData->GetConnectionPBlock();
	CConnectionPBlock* pSourceConnectionPBlock = pSourceData->GetConnectionPBlock();

	// A list of the source parameters that were succefully taken up.
	std::vector<bool> taken_source(pSourceParams->GetNumParameters(), false);
	std::vector<bool> taken_dest(pDestParams->GetNumParameters(), false);

	unsigned int iSrc, iDest;

	for (unsigned int PassNum = 0; PassNum < 2; PassNum++)
	{
		DISPDBG(3, "Pass: " << PassNum);

		// For all destination parameters...
		for (iDest = 0; iDest < pDestParams->GetNumParameters(); iDest++)
		{
			// Get the equivalent connection parameter.
			INVConnectionParameter* pDestConnection = pDestParams->GetConnectionParameter(iDest);
			if (!pDestConnection || taken_dest[iDest])
				continue;

			DISPDBG(3, "Checking Param: " << pDestConnection->GetDefaultValue().GetObjectSemantics()->GetName());

			// Find the dynamic mapping for the dest
			// Skip it if we don't handle syncing of it.
			DynamicPBlockEntry* pDestEntry = pDestConnectionPBlock->GetPBlockEntry(pDestConnection);
			if (!pDestEntry)
				continue;

			// We only copy tab table entries
			if (!is_tab(pDestPBlock->GetParamDef(pDestEntry->GetPBlockLocation()).type))
				continue;

			// For each source param...
			for (iSrc = 0; iSrc < pSourceParams->GetNumParameters(); iSrc++)
			{
				// Get the connection
				INVConnectionParameter* pSourceConnection = pSourceParams->GetConnectionParameter(iSrc);
				if (!pSourceConnection || taken_source[iSrc])
					continue;

				// Find its location in the PBlock
				// Skip it if we don't handle syncing it.
				DynamicPBlockEntry* pSourceEntry = pSourceConnectionPBlock->GetPBlockEntry(pSourceConnection);
				if (!pSourceEntry)
					continue;

				// We only copy tab table entries
				if (!is_tab(pSourcePBlock->GetParamDef(pSourceEntry->GetPBlockLocation()).type))
					continue;

				bool bMatched = false;
				if (PassNum == 0)
				{
					// Check for name, semantic and type match
					if ((pSourceConnection->GetDefaultValue().GetType() == pDestConnection->GetDefaultValue().GetType()) &&
						(pSourceConnection->GetDefaultValue().GetObjectSemantics()->GetSemanticID() == pDestConnection->GetDefaultValue().GetObjectSemantics()->GetSemanticID()) &&
						(stricmp(pSourceConnection->GetDefaultValue().GetObjectSemantics()->GetName(), pDestConnection->GetDefaultValue().GetObjectSemantics()->GetName()) == 0))
						bMatched = true;
				}
				else if (PassNum == 1)
				{
					// Check for semantic and type match, but with no 'unknown' semantics.
					// No name check either.
					if ((pSourceConnection->GetDefaultValue().GetObjectSemantics()->GetSemanticID() != nv_sys::SEMANTICID_UNKNOWN) &&
						(pSourceConnection->GetDefaultValue().GetType() == pDestConnection->GetDefaultValue().GetType()) &&
						(pSourceConnection->GetDefaultValue().GetObjectSemantics()->GetSemanticID() == pDestConnection->GetDefaultValue().GetObjectSemantics()->GetSemanticID()) )
						bMatched = true;
				}
				/*
				else if (PassNum == 2)
				{
					// Check for semantic (and possibly unknown semantic) and type match
					// No name check either.  This is last-chance, and I'm not sure how effective this will be....
					if ((pSourceConnection->GetDefaultValue().GetType() == pDestConnection->GetDefaultValue().GetType()) &&
						(pSourceConnection->GetSemanticID() == pDestConnection->GetSemanticID()) )
						bMatched = true;
				}
				*/

				if (bMatched)
				{
					// Copy all entries in the matched parameter.
					for (unsigned int i = 0; i < pDestEntry->GetPBlockEntries(); i++)
					{
						AssignPB2Value(pDestPBlock, pDestEntry->GetPBlockLocation(), pDestEntry->GetPBlockOffset() + i, pSourcePBlock->GetPB2Value(pSourceEntry->GetPBlockLocation(), pSourceEntry->GetPBlockOffset() + i));
					}
					taken_source[iSrc] = true;
					taken_dest[iDest] = true;

					DISPDBG(3, "Found match for param name: " << pDestConnection->GetDefaultValue().GetObjectSemantics()->GetName() << ", Semantic = " << ConvertSemantic(pDestConnection->GetDefaultValue().GetObjectSemantics()->GetSemanticID()) << ", iDest=" << iDest << ", iSrc =" << iSrc << 	", Semantic = " << ConvertSemantic(pSourceConnection->GetDefaultValue().GetObjectSemantics()->GetSemanticID()));
					

					DISPDBG(3, "Dest Location: " << pDestEntry->GetPBlockLocation() << ", Source Location: " << pSourceEntry->GetPBlockLocation());
					DISPDBG(3, "Dest Offset: " << pDestEntry->GetPBlockOffset() << ", Source Offset: " << pSourceEntry->GetPBlockOffset());
	
					break;
				}			
			}
		}
	}

	std::vector<bool>::iterator itrTaken = taken_source.begin();
	while (itrTaken != taken_source.end())
	{
		if (*itrTaken != true)
		{
			DISPDBG(3, "DID NOT match for SOURCE param name: " << pSourceParams->GetConnectionParameter(itrTaken - taken_source.begin())->GetDefaultValue().GetObjectSemantics()->GetName());
		}
		itrTaken++;
	}

	itrTaken = taken_dest.begin();
	while (itrTaken != taken_dest.end())
	{
		if (*itrTaken != true)
		{
			DISPDBG(3, "DID NOT match for DEST param name: " << pDestParams->GetConnectionParameter(itrTaken - taken_dest.begin())->GetDefaultValue().GetObjectSemantics()->GetName());
		}
		itrTaken++;
	}

	return true;
}

bool ShaderInfo::FreeEffect(OWNER_ID Owner, bool bStartup)
{
	assert(GetMaterial()->GetFileVersion() < 2);

	NVPROF_FUNC("ShaderInfo::FreeEffect");
	bool bRet;
	bRet = GetCurrentShader()->FreeEffect(Owner, bStartup);
	return bRet;
}

bool ShaderInfo::LoadEffect(OWNER_ID Owner, const std::string& strBackupFile, std::string strFileName, string sTechnique, IParamBlock2* pDestBlock, IParamBlock2* pSourceBlock)
{
	NVPROF_FUNC("ShaderInfo::LoadEffect");
	HANDLE hFile = INVALID_HANDLE_VALUE;
	
	assert(GetMaterial()->GetFileVersion() < 2);

	if (!strFileName.empty())
		hFile = CreateFile(strFileName.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		MessageBox(0,"WTF!!! Can't find file - TODO: Search for it",0,0);
		// If no file, then setup the default.fx
		/*strFileName = FindFile("default.fx");
		if (strFileName.empty())
		{
			MessageBox(NULL, "Failed to find default.fx!", "Error", MB_ICONEXCLAMATION | MB_OK);
			return false;
		}*/
		return false;
	}

	// Don't use the handle, just needed to try opening it.
	CloseHandle(hFile);

	m_pMat->GetVertexShader()->StopWatching();

	DISPDBG(3, "Loading Effect: " << strFileName);

	assert(pSourceBlock != pDestBlock);
	if (!strBackupFile.empty())
	{
		bool bBadBackup = false;

		m_pMat->InDrawSetup(true);

		pDestBlock->EnableNotifications(false);
		pSourceBlock->EnableNotifications(false);

		DUMPPBLOCK(pDestBlock, "DestBlock");

		// Copy the pblock for the dest into the source, so we can load it.
		// This is because MAX reloaded it straight into the first PBlock
		// The source block in this case is really the backup pblock.
		// The dest block in this case is really the loaded pblock from file.
		CopyPBlock(pSourceBlock, pDestBlock);

		// Lets have a look at how we did...
		DUMPPBLOCK(pSourceBlock, "SourceBlock");

		// Load the original effect into the backup, with startup flag to remap the paramblock
		SetShaderFromRef(pSourceBlock->ID());
		if (!GetCurrentShader()->LoadEffect(Owner, const_cast<char*>(strBackupFile.c_str()), true))
		{
			bBadBackup = true;
			MessageBox(NULL, "Could not load all of old effect data\nMay need to re-assign parameters to material effect...", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		}
		DUMPPBLOCK(pSourceBlock, "SourceBlock");
		
		// Load the new effect into the main pblock.
		SetShaderFromRef(pDestBlock->ID());
		if (!GetCurrentShader()->LoadEffect(Owner, const_cast<char*>(strFileName.c_str()), false))
		{
			// Revert the default.
			/*strFileName = FindFile("default.fx");
			if (!GetCurrentShader()->LoadEffect(Owner, strFileName.c_str()))
			{
				return false;
			}*/
			return false;
		}
		DUMPPBLOCK(pDestBlock, "DestBlock");

		// If the backup was OK, resolve the blocks
		if (!bBadBackup)
		{
			ResolvePBlocks(pDestBlock, pSourceBlock);
			DUMPPBLOCK(pDestBlock, "DestBlock");
		
			// Kill the old one.
			SetShaderFromRef(pSourceBlock->ID());
			GetCurrentShader()->FreeEffect(Owner, false);
		}
		
		// Back to the current one
		SetShaderFromRef(pDestBlock->ID());

		if (m_pMat->GetVertexShader()->GetLighting())
			m_pMat->GetVertexShader()->GetLighting()->SetLoadInfo(m_pMat->GetLightStreamInfo());

		m_pMat->InDrawSetup(false);

		pDestBlock->EnableNotifications(true);
		pSourceBlock->EnableNotifications(true);
		pDestBlock->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		pSourceBlock->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);

	}
	else
	{
		SetShaderFromRef(pDestBlock->ID());
		GetCurrentShader()->LoadEffect(Owner, const_cast<char*>(strFileName.c_str()), false);
	}

	// Set the technique
	GetCurrentShader()->SetTechnique(OWNER_MATERIAL, sTechnique.c_str());

	// Rebuild the tweakables.  We know that the tweakables will never call load effect!
	m_pMat->GetTweakables()->BuildGUI();

	m_pMat->GetVertexShader()->StartWatching();

	return true;
}

bool ShaderInfo::SwapEffect(OWNER_ID Owner, std::string strFileName, eSwapOption SwapOption)
{
	NVPROF_FUNC("ShaderInfo::SwapEffect");
	IParamBlock2* pCurrentBlock;
	IParamBlock2* pBackupBlock;

	assert(GetMaterial()->GetFileVersion() < 2);

	m_pMat->InDrawSetup(true);

	m_pMat->GetVertexShader()->StopWatching();

	// Swap one shader for another.
	pCurrentBlock = (IParamBlock2*)m_pMat->GetReference(ref_pblock);
	pBackupBlock = (IParamBlock2*)m_pMat->GetReference(ref_pblock_backup);

	pBackupBlock->EnableNotifications(false);
	pCurrentBlock->EnableNotifications(false);

	DUMPPBLOCK(pCurrentBlock, "CurrentBlock");

	tvecLightStreamInfo LightStream;
	if (m_pMat->GetVertexShader()->GetLighting())
		m_pMat->GetVertexShader()->GetLighting()->GetSaveInfo(LightStream);

	// Load the shader into the current backup block
	SetShaderFromRef(pBackupBlock->ID());
	if (!GetCurrentShader()->LoadEffect(Owner, const_cast<char*>(strFileName.c_str()),false))
	{
		strFileName = FindFile(SHADER_FILE);
		if (!GetCurrentShader()->LoadEffect(Owner, strFileName.c_str()))
		{
			MessageBox(NULL, "Fatal error occured trying to revert to Diffuse.fx", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		
			pBackupBlock->EnableNotifications(true);
			pCurrentBlock->EnableNotifications(true);

			return false;
		}
	}

	// Addref the backup textures because we are 'transfering' ownership of them.
	m_pMat->GetVertexShader()->AddRefEffectTextures();
	DUMPPBLOCK(pBackupBlock, "BackupBlock - new shader");

	// Resolve the current shader's parameters into the new one.
	ResolvePBlocks(pBackupBlock, pCurrentBlock);
	DUMPPBLOCK(pBackupBlock, "BackupBlock - integrated parameters");

	// Copy the params back into the current block
	CopyPBlock(pCurrentBlock, pBackupBlock);
	DUMPPBLOCK(pCurrentBlock, "CurrentBlock - integrated parameters");

	// Load the shader, this time into the current block, with startup condition so that the connections are valid
	SetShaderFromRef(pCurrentBlock->ID());

	// AddRef the textures first to make sure any we might immediately re-use once the shader is loaded aren't nuked
	// before it's created.
	GetCurrentShader()->LoadEffect(Owner, const_cast<char*>(strFileName.c_str()), true);
	DUMPPBLOCK(pCurrentBlock, "CurrentBlock - new shader");

	// Kill the old one.
	SetShaderFromRef(pBackupBlock->ID());
	GetCurrentShader()->FreeEffect(Owner, false);
	DUMPPBLOCK(pBackupBlock, "BackupBlock - after free");
	
	// Back to the current one
	SetShaderFromRef(pCurrentBlock->ID());
	DUMPPBLOCK(pCurrentBlock, "CurrentBlock - after free backup");

	// Replace the light stream
	if (m_pMat->GetVertexShader()->GetLighting())
		m_pMat->GetVertexShader()->GetLighting()->SetLoadInfo(LightStream);

	// Check material overrides.
	m_pMat->OverrideMaterial();
	DUMPPBLOCK(pCurrentBlock, "CurrentBlock - after override");

	// Update the GUI.
	if (SwapOption == UPDATE_GUI)
	{
		m_pMat->GetTweakables()->BuildGUI();
	}

	m_pMat->InDrawSetup(false);

	// Must come last....
	pBackupBlock->EnableNotifications(true);
	pCurrentBlock->EnableNotifications(true);

	m_pMat->GetVertexShader()->StartWatching();

	pBackupBlock->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
  	pCurrentBlock->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	
	return true;
}

void ShaderInfo::UpdateEffect()
{
	NVPROF_FUNC("ShaderInfo::UpdateEffect");
	DISPDBG(3, "ShaderInfo::UpdateEffect");
	ShaderInfoData* pShader = GetCurrentShader();

	// Reload the same effect by swapping it.  This will maintain the params.
	std::string strShaderFile = pShader->GetEffectFile();
	SwapEffect_Ver2(OWNER_NONE, strShaderFile, UPDATE_GUI);
}


// *******************************************************************************************************
// Version 2 API's.
// *******************************************************************************************************

bool ShaderInfo::FreeEffect_Ver2(OWNER_ID Owner)
{
	NVPROF_FUNC("ShaderInfo::FreeEffect");
	return GetCurrentShader()->FreeEffect_Ver2(Owner);
}


bool ShaderInfo::LoadEffect_Ver2(OWNER_ID Owner, std::string strFileName, string sTechnique, IParamBlock2* pDestBlock)
{
	NVPROF_FUNC("ShaderInfo::LoadEffect_Ver2");
	HANDLE hFile = INVALID_HANDLE_VALUE;

	if(FindFile(strFileName).length())
		strFileName = FindFile(strFileName);
	
	if (!strFileName.empty() && (access( strFileName.c_str(), 0 ) == 0))
		hFile = CreateFile(strFileName.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
	else
		hFile = INVALID_HANDLE_VALUE;

	if (hFile == INVALID_HANDLE_VALUE)
	{
		DoMissingFile(strFileName);

		hFile = CreateFile(strFileName.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);

		if (hFile == INVALID_HANDLE_VALUE)
			return false;
	}

	// Don't use the handle, just needed to try opening it.
	CloseHandle(hFile);

	m_pMat->GetVertexShader()->StopWatching();
	pDestBlock->EnableNotifications(false);
	m_pMat->InDrawSetup(true);

	DISPDBG(3, "Loading Effect: " << strFileName);

	SetShaderFromRef(pDestBlock->ID());
	GetCurrentShader()->LoadEffect_Ver2(Owner, const_cast<char*>(strFileName.c_str()));

	// Set the technique
	if(sTechnique[0] == ':'){ // HACK
		sTechnique = GetCurrentShader()->GetTechniqueName(atoi(sTechnique.substr(1).c_str()));
	}
	// Set the technique, but we may have changed shader file, so be prepared to set a default technique
	if(!GetCurrentShader()->SetTechnique(OWNER_MATERIAL, sTechnique.c_str())){
		GetCurrentShader()->SetTechnique(OWNER_MATERIAL, (UINT)0);
	}
 
	// Rebuild the tweakables.  We know that the tweakables will never call load effect!
	m_pMat->GetTweakables()->BuildGUI();
 
	m_pMat->GetVertexShader()->StartWatching();
	m_pMat->InDrawSetup(false);

	// Must come last....
	pDestBlock->EnableNotifications(true);

	return true;
}


bool ShaderInfo::SwapEffect_Ver2(OWNER_ID Owner, std::string strFileName, eSwapOption SwapOption)
{
	NVPROF_FUNC("ShaderInfo::SwapEffect");
	IParamBlock2* pCurrentBlock;

	m_pMat->InDrawSetup(true);

	m_pMat->GetVertexShader()->StopWatching();

	// Swap one shader for another.
	pCurrentBlock = (IParamBlock2*)m_pMat->GetReference(ref_pblock);
	pCurrentBlock->EnableNotifications(false);

	tvecLightStreamInfo LightStream;
	if (m_pMat->GetVertexShader()->GetLighting())
		m_pMat->GetVertexShader()->GetLighting()->GetSaveInfo(LightStream);

	m_pMat->GetVertexShader()->AddRefEffectTextures();

	string Technique = GetCurrentShader()->GetTechnique();
	// Clone the parameter list for the current effect.
	INVParameterList* pParams = GetCurrentShader()->GetParameterList();
	assert(pParams);
	INVClone* pClone = NULL;
	pParams->GetInterface(IID_INVClone, (void**)&pClone);

	INVParameterList* pParamsBackup = NULL;
	pClone->Clone(IID_INVParameterList, (void**)&pParamsBackup);
	SAFE_RELEASE(pClone);
	
	if (!pParamsBackup)
		return false;

	// Load the shader into the current backup block
	SetShaderFromRef(pCurrentBlock->ID());
	if (!GetCurrentShader()->LoadEffect_Ver2(Owner, const_cast<char*>(strFileName.c_str())))
	{
		strFileName = FindFile(SHADER_FILE);
		if (!GetCurrentShader()->LoadEffect_Ver2(Owner, strFileName.c_str()))
		{
			MessageBox(NULL, "Fatal error occured trying to revert to Diffuse.fx", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		
			pCurrentBlock->EnableNotifications(true);
			return false;
		}
	}

	// Resolve the backed up connections to the new shader connections
	ResolveConnections(GetCurrentShader()->GetParameterList(), pParamsBackup, false);
	SAFE_RELEASE(pParamsBackup);


	// TIM: Try to resolve the backed up technique to the new shader
	if(!GetCurrentShader()->SetTechnique(OWNER_MATERIAL, (char*)Technique.c_str())){
		// May have failed if we swapped shader, in which case apply a default
		GetCurrentShader()->SetTechnique(OWNER_MATERIAL, (UINT)0);
	}
	
	// Sync the updated connections to the paramblock
	SuspendAnimate();
	AnimateOn();
	GetCurrentShader()->GetConnectionPBlock()->SyncConnections(GetCurrentShader()->GetParameterList());
	AnimateOff();
	ResumeAnimate();

	// Replace the light stream
	if (m_pMat->GetVertexShader()->GetLighting())
		m_pMat->GetVertexShader()->GetLighting()->SetLoadInfo(LightStream);

	// Check material overrides.
	m_pMat->OverrideMaterial();

	// Update the GUI.
	if (SwapOption == UPDATE_GUI)
	{
		m_pMat->GetTweakables()->BuildGUI();
	}

	m_pMat->InDrawSetup(false);

	// Must come last....
	pCurrentBlock->EnableNotifications(true);
	m_pMat->GetVertexShader()->StartWatching();

	pCurrentBlock->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	
	return true;
}

void ShaderInfo::UpdateEffect_Ver2()
{
	NVPROF_FUNC("ShaderInfo::UpdateEffect_Ver2");
	
	ShaderInfoData* pShader = GetCurrentShader();

	// Reload the same effect by swapping it.  This will maintain the params.
	std::string strShaderFile = pShader->GetEffectFile();
	SwapEffect_Ver2(OWNER_NONE, strShaderFile, UPDATE_GUI);
}
