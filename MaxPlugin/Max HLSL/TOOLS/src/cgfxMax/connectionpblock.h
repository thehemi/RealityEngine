/*********************************************************************NVMH4****
Path:  plugins/nvmax
File:  connectionpblock.h

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

This class is responsible for syncing entries in the tab<> pblock locations
with INVConnectionParameter's.

It is called whenever the pblock changes or whenever a connection parameter changes.


******************************************************************************/
#ifndef __CONNECTIONPBLOCK_H
#define __CONNECTIONPBLOCK_H

#include "material.h"
// Material entries in the param block.
// Dynamic data is at the end.
enum { material_params };


class IParamBlock2;
class DynamicPBlockEntry
{
public:
	DynamicPBlockEntry(const DynamicPBlockEntry& rhs)
		: m_PBlockLocation(rhs.m_PBlockLocation),
		m_PBlockOffset(rhs.m_PBlockOffset),
		m_PBlockEntries(rhs.m_PBlockEntries)
	{}

	DynamicPBlockEntry()
	: m_PBlockLocation(CGFX_INVALID),
	m_PBlockOffset(0),
	m_PBlockEntries(0)
	{}

	eMaterialPBlock GetPBlockLocation() { return m_PBlockLocation; }
	unsigned int GetPBlockOffset() { return m_PBlockOffset; }
	unsigned int GetPBlockEntries() { return m_PBlockEntries; }

	void SetPBlockLocation(eMaterialPBlock Location) { m_PBlockLocation = Location; }
	void SetPBlockOffset(unsigned int Offset) { m_PBlockOffset = Offset; }
	void SetPBlockEntries(unsigned int Entries) { m_PBlockEntries = Entries; }

private:
	eMaterialPBlock m_PBlockLocation;
	unsigned int m_PBlockOffset;
	unsigned int m_PBlockEntries;
};

typedef std::map<nv_sys::INVConnectionParameter*, DynamicPBlockEntry*> tmapConnectionToPBlock;
typedef std::map<DynamicPBlockEntry*, nv_sys::INVConnectionParameter*> tmapPBlockToConnection;

typedef enum eSyncOption
{
	SYNC_CONNECTION,
	SYNC_PBLOCK
} eSyncOption;

class CConnectionPBlock
{
public:
	CConnectionPBlock()
		: m_pMat(NULL), 
		m_pParamBlock(NULL)
	{}

	typedef enum
	{
		BUILD_PBLOCK = 0,
		USE_CURRENT_PBLOCK
	} tPBlockOption;

	typedef enum
	{
		CLEAR_PBLOCK = 0,
		KEEP_PBLOCK
	} tPBlockOptionClear;

	void SetInfo(CgFXMaterial* pMat, IParamBlock2* pParamBlock)
	{
		m_pParamBlock = pParamBlock;
		m_pMat = pMat;
	}

	bool AddItems(nv_sys::INVParameterList* pParams, tPBlockOption Option)
	{
		if (!pParams)
			return false;

		TimeValue t = GetCOREInterface()->GetTime();

		ClearItems(Option == BUILD_PBLOCK ? CLEAR_PBLOCK : KEEP_PBLOCK);

		unsigned int iCurrentCGFX_FLOAT = 0;
		unsigned int iCurrentCGFX_VEC2 = 0;
		unsigned int iCurrentCGFX_VEC3 = 0;
		unsigned int iCurrentCGFX_VEC4 = 0;
		unsigned int iCurrentCGFX_TEXTURE = 0;

	    // Build the PBlock.
	    for (unsigned int i = 0; i < pParams->GetNumParameters(); i++)
		{
			nv_sys::INVConnectionParameter* pItem = pParams->GetConnectionParameter(i);
		
			DynamicPBlockEntry PBlockEntry;

			nv_sys::NVType NVValue = pItem->GetValueAtTime(t);
			unsigned long Bytes;

			// Because MAX doesn't have const types for appending to PBlocks, even though 
			// it doesn't mess with them.
			void* pValue = const_cast<void*>(NVValue.GetValue(Bytes));

			switch(pItem->GetDefaultValue().GetType())
			{
				case nv_sys::NVTYPEID_FLOAT:
				{
					PBlockEntry.SetPBlockLocation(CGFX_FLOAT);
					PBlockEntry.SetPBlockEntries(1);
					if (Option == BUILD_PBLOCK)
					{
						m_pParamBlock->Append(PBlockEntry.GetPBlockLocation(), PBlockEntry.GetPBlockEntries(), static_cast<float*>(pValue));
						PBlockEntry.SetPBlockOffset(m_pParamBlock->Count(PBlockEntry.GetPBlockLocation()) - PBlockEntry.GetPBlockEntries());
					}
					else
					{
						// Check we have room for it.
						PBlockEntry.SetPBlockOffset(iCurrentCGFX_FLOAT);
						assert(m_pParamBlock->Count(CGFX_FLOAT) > iCurrentCGFX_FLOAT);
						iCurrentCGFX_FLOAT++;						
					}
				}
				break;

				case nv_sys::NVTYPEID_VEC2:
				{
					PBlockEntry.SetPBlockLocation(CGFX_FLOAT);
					PBlockEntry.SetPBlockEntries(2);
					if (Option == BUILD_PBLOCK)
					{
						m_pParamBlock->Append(PBlockEntry.GetPBlockLocation(), PBlockEntry.GetPBlockEntries(), static_cast<float*>(pValue));
						PBlockEntry.SetPBlockOffset(m_pParamBlock->Count(PBlockEntry.GetPBlockLocation()) - PBlockEntry.GetPBlockEntries());
					}
					else
					{
						PBlockEntry.SetPBlockOffset(iCurrentCGFX_FLOAT);
						assert(m_pParamBlock->Count(CGFX_FLOAT) > (iCurrentCGFX_FLOAT + 1));
						iCurrentCGFX_FLOAT += 2;
					}
				}
				break;

				case nv_sys::NVTYPEID_VEC3:
				{
					PBlockEntry.SetPBlockLocation(CGFX_FLOAT);
					PBlockEntry.SetPBlockEntries(3);
					if (Option == BUILD_PBLOCK)
					{
						m_pParamBlock->Append(PBlockEntry.GetPBlockLocation(), PBlockEntry.GetPBlockEntries(), static_cast<float*>(pValue));
						PBlockEntry.SetPBlockOffset(m_pParamBlock->Count(PBlockEntry.GetPBlockLocation()) - PBlockEntry.GetPBlockEntries());
					}
					else
					{
						PBlockEntry.SetPBlockOffset(iCurrentCGFX_FLOAT);
						assert(m_pParamBlock->Count(CGFX_FLOAT) > (iCurrentCGFX_FLOAT + 2));
						iCurrentCGFX_FLOAT += 3;
					}

				}
				break;

				case nv_sys::NVTYPEID_VEC4:
				{
					PBlockEntry.SetPBlockLocation(CGFX_FLOAT);
					PBlockEntry.SetPBlockEntries(4);
					if (Option == BUILD_PBLOCK)
					{
						m_pParamBlock->Append(PBlockEntry.GetPBlockLocation(), PBlockEntry.GetPBlockEntries(), static_cast<float*>(pValue));
						PBlockEntry.SetPBlockOffset(m_pParamBlock->Count(PBlockEntry.GetPBlockLocation()) - PBlockEntry.GetPBlockEntries());
					}
					else
					{
						PBlockEntry.SetPBlockOffset(iCurrentCGFX_FLOAT);
						assert(m_pParamBlock->Count(CGFX_FLOAT) > (iCurrentCGFX_FLOAT + 3));
						iCurrentCGFX_FLOAT += 4;
					}
				}
				break;

				case nv_sys::NVTYPEID_TEXTURE:
				case nv_sys::NVTYPEID_TEXTURE2D:
				case nv_sys::NVTYPEID_TEXTURE3D:
				case nv_sys::NVTYPEID_TEXTURECUBE:
				{
					std::string strName = TextureMgr::GetSingletonPtr()->GetTextureName(pItem->GetValueAtTime(t).GetTexture());
					char* pName = const_cast<char*>(strName.c_str());
					PBlockEntry.SetPBlockLocation(CGFX_TEXTURE);
					PBlockEntry.SetPBlockEntries(1);
					if (Option == BUILD_PBLOCK)
					{
						m_pParamBlock->Append(PBlockEntry.GetPBlockLocation(), PBlockEntry.GetPBlockEntries(), &pName);
						PBlockEntry.SetPBlockOffset(m_pParamBlock->Count(PBlockEntry.GetPBlockLocation()) - PBlockEntry.GetPBlockEntries());
					}
					else
					{
						PBlockEntry.SetPBlockOffset(iCurrentCGFX_TEXTURE);
						assert(m_pParamBlock->Count(CGFX_TEXTURE) > iCurrentCGFX_TEXTURE);
						iCurrentCGFX_TEXTURE++;
					}
				}
				break;

				default:
					continue;
			}
			

			AddLink(pItem, PBlockEntry);
		}
		return true;
	}

	void ClearItems(tPBlockOptionClear Option)
	{
		tmapPBlockToConnection::iterator itrConnection = m_mapPBlockToConnection.begin();
		while (itrConnection != m_mapPBlockToConnection.end())
		{
			delete itrConnection->first;
			itrConnection++;
		}

		m_mapPBlockToConnection.clear();
		m_mapConnectionToPBlock.clear();

		// If we are building the pblock, nuke the existing one.
		if (Option == CLEAR_PBLOCK)
		{
			if (m_pParamBlock)
			{
				if (m_pParamBlock->Count(CGFX_FLOAT))
				{
					m_pParamBlock->Delete(CGFX_FLOAT, 0, m_pParamBlock->Count(CGFX_FLOAT));
					m_pParamBlock->ZeroCount(CGFX_FLOAT);
				}

				if (m_pParamBlock->Count(CGFX_TEXTURE))
				{
					m_pParamBlock->Delete(CGFX_TEXTURE, 0, m_pParamBlock->Count(CGFX_TEXTURE));
					m_pParamBlock->ZeroCount(CGFX_TEXTURE);
				}


				if (m_pParamBlock->Count(CGFX_INT))
				{
					m_pParamBlock->Delete(CGFX_INT, 0, m_pParamBlock->Count(CGFX_INT));
					m_pParamBlock->ZeroCount(CGFX_INT);
				}
			}
		}
	}

	bool AddLink(nv_sys::INVConnectionParameter* pItem, const DynamicPBlockEntry& PBlockEntry)
	{
		DynamicPBlockEntry* pEntry = new DynamicPBlockEntry(PBlockEntry);
		m_mapConnectionToPBlock[pItem] = pEntry;
		m_mapPBlockToConnection[pEntry] = pItem;
		return true;
	}

	DynamicPBlockEntry* GetPBlockEntry(nv_sys::INVConnectionParameter* pItem)
	{
		tmapConnectionToPBlock::const_iterator itrPBlock = m_mapConnectionToPBlock.find(pItem);
		if (itrPBlock == m_mapConnectionToPBlock.end())
			return NULL;

		return itrPBlock->second;
	}

	std::string GetParamName(ParamID id, int tabIndex)
	{
		tmapPBlockToConnection::iterator itrConnection = m_mapPBlockToConnection.begin();
		while (itrConnection != m_mapPBlockToConnection.end())
		{
			nv_sys::INVConnectionParameter* pConnection = itrConnection->second;
			DynamicPBlockEntry* pPBlock = itrConnection->first;

			if ((id == CGFX_FLOAT) && (pPBlock->GetPBlockLocation() == id) && 
				(pPBlock->GetPBlockOffset() <= tabIndex) && 
				((pPBlock->GetPBlockOffset() + pPBlock->GetPBlockEntries()) > tabIndex ))
			{
				int Offset = tabIndex - pPBlock->GetPBlockOffset();
				
				switch(Offset)
				{
					case 0:
						return std::string(pConnection->GetDefaultValue().GetObjectSemantics()->GetName()) + std::string(".x");
						break;
					case 1:
						return std::string(pConnection->GetDefaultValue().GetObjectSemantics()->GetName()) + std::string(".y");
						break;
					case 2:
						return std::string(pConnection->GetDefaultValue().GetObjectSemantics()->GetName()) + std::string(".z");
						break;
					case 3:
						return std::string(pConnection->GetDefaultValue().GetObjectSemantics()->GetName()) + std::string(".w");
						break;
					default:
						return pConnection->GetDefaultValue().GetObjectSemantics()->GetName();
						break;
				}
			}
			else if (id == CGFX_TEXTURE)
			{
				return pConnection->GetDefaultValue().GetObjectSemantics()->GetName();
			}

			itrConnection++;
		}

		return _T("");
	}

	void GetValidity(TimeValue t, Interval &valid)
	{
		tmapPBlockToConnection::iterator itrConnection = m_mapPBlockToConnection.begin();
		while (itrConnection != m_mapPBlockToConnection.end())
		{
			nv_sys::INVConnectionParameter* pConnection = itrConnection->second;
			DynamicPBlockEntry* pPBlock = itrConnection->first;

			int i = 0;
			float f = 0.0f;
			TCHAR* pString = NULL;

			switch(pConnection->GetDefaultValue().GetType())
			{
				case nv_sys::NVTYPEID_FLOAT:
				{
					m_pParamBlock->GetValue(pPBlock->GetPBlockLocation(), t, f, valid, pPBlock->GetPBlockOffset());
				}
				break;
				case nv_sys::NVTYPEID_VEC2:
				{
					m_pParamBlock->GetValue(pPBlock->GetPBlockLocation(), t, f, valid, pPBlock->GetPBlockOffset());
					m_pParamBlock->GetValue(pPBlock->GetPBlockLocation(), t, f, valid, pPBlock->GetPBlockOffset() + 1);
				}
				break;
				case nv_sys::NVTYPEID_VEC3:
				{
					m_pParamBlock->GetValue(pPBlock->GetPBlockLocation(), t, f, valid, pPBlock->GetPBlockOffset());
					m_pParamBlock->GetValue(pPBlock->GetPBlockLocation(), t, f, valid, pPBlock->GetPBlockOffset() + 1);
					m_pParamBlock->GetValue(pPBlock->GetPBlockLocation(), t, f, valid, pPBlock->GetPBlockOffset() + 2);
				}
				break;
				case nv_sys::NVTYPEID_VEC4:
				{
					m_pParamBlock->GetValue(pPBlock->GetPBlockLocation(), t, f, valid, pPBlock->GetPBlockOffset());
					m_pParamBlock->GetValue(pPBlock->GetPBlockLocation(), t, f, valid, pPBlock->GetPBlockOffset() + 1);
					m_pParamBlock->GetValue(pPBlock->GetPBlockLocation(), t, f, valid, pPBlock->GetPBlockOffset() + 2);
					m_pParamBlock->GetValue(pPBlock->GetPBlockLocation(), t, f, valid, pPBlock->GetPBlockOffset() + 3);
				}
				break;

				case nv_sys::NVTYPEID_TEXTURE:
				case nv_sys::NVTYPEID_TEXTURE2D:
				case nv_sys::NVTYPEID_TEXTURE3D:
				case nv_sys::NVTYPEID_TEXTURECUBE:
				{
					m_pParamBlock->GetValue(pPBlock->GetPBlockLocation(), t, pString, valid, pPBlock->GetPBlockOffset());
				}
				break;

				default:
					break;
			}
			itrConnection++;
		}

	}

    bool Synchronize(TimeValue t, eSyncOption SyncOption, nv_sys::INVConnectionParameter* pParam)
	{

		tmapPBlockToConnection::iterator itrConnection = m_mapPBlockToConnection.begin();
		while (itrConnection != m_mapPBlockToConnection.end())
		{
			nv_sys::INVConnectionParameter* pConnection = itrConnection->second;
			DynamicPBlockEntry* pPBlock = itrConnection->first;

            // If NULL sync all, otherwise just the parameter in question
            if (pParam == NULL || pConnection == pParam)
            {
				unsigned long Bytes;
				nv_sys::NVType Value = pConnection->GetValueAtTime(t);
				const void* pValue = Value.GetValue(Bytes);

			    switch(pConnection->GetDefaultValue().GetType())
			    {
				    case nv_sys::NVTYPEID_FLOAT:
				    {
						float fVal;
					    if (SyncOption == SYNC_CONNECTION)
					    {
						    m_pParamBlock->GetValue(pPBlock->GetPBlockLocation(), t, fVal, FOREVER, pPBlock->GetPBlockOffset());
							pConnection->SetKey(t, nv_sys::NVType::CreateFloatType(fVal));

					    }
					    else
					    {
							fVal = pConnection->GetValueAtTime(t).GetFloat();
						    m_pParamBlock->SetValue(pPBlock->GetPBlockLocation(), t, fVal, pPBlock->GetPBlockOffset());
					    }
				    }
				    break;

				    case nv_sys::NVTYPEID_VEC2:
				    {
						vec2 vec2Val;
					    if (SyncOption == SYNC_CONNECTION)
					    {
						    m_pParamBlock->GetValue(pPBlock->GetPBlockLocation(), t, vec2Val.x, FOREVER, pPBlock->GetPBlockOffset());
						    m_pParamBlock->GetValue(pPBlock->GetPBlockLocation(), t, vec2Val.y, FOREVER, pPBlock->GetPBlockOffset() + 1);
							pConnection->SetKey(t, nv_sys::NVType::CreateVec2Type(vec2Val));
					    }
					    else
					    {
							vec2Val = pConnection->GetValueAtTime(t).GetVec2();
						    m_pParamBlock->SetValue(pPBlock->GetPBlockLocation(), t, vec2Val.x, pPBlock->GetPBlockOffset());
						    m_pParamBlock->SetValue(pPBlock->GetPBlockLocation(), t, vec2Val.y, pPBlock->GetPBlockOffset() + 1);
					    }
				    }
				    break;

				    case nv_sys::NVTYPEID_VEC3:
				    {
						vec3 vec3Val;
					    if (SyncOption == SYNC_CONNECTION)
					    {
						    m_pParamBlock->GetValue(pPBlock->GetPBlockLocation(), t, vec3Val.x, FOREVER, pPBlock->GetPBlockOffset());
						    m_pParamBlock->GetValue(pPBlock->GetPBlockLocation(), t, vec3Val.y, FOREVER, pPBlock->GetPBlockOffset() + 1);
						    m_pParamBlock->GetValue(pPBlock->GetPBlockLocation(), t, vec3Val.z, FOREVER, pPBlock->GetPBlockOffset() + 2);
							pConnection->SetKey(t, nv_sys::NVType::CreateVec3Type(vec3Val));
					    }
					    else
					    {
							vec3Val = pConnection->GetValueAtTime(t).GetVec3();
						    m_pParamBlock->SetValue(pPBlock->GetPBlockLocation(), t, vec3Val.x, pPBlock->GetPBlockOffset());
						    m_pParamBlock->SetValue(pPBlock->GetPBlockLocation(), t, vec3Val.y, pPBlock->GetPBlockOffset() + 1);
						    m_pParamBlock->SetValue(pPBlock->GetPBlockLocation(), t, vec3Val.z, pPBlock->GetPBlockOffset() + 2);
					    }
				    }
				    break;

				    case nv_sys::NVTYPEID_VEC4:
				    {
						vec4 vec4Val;
					    if (SyncOption == SYNC_CONNECTION)
					    {
						    m_pParamBlock->GetValue(pPBlock->GetPBlockLocation(), t, vec4Val.x, FOREVER, pPBlock->GetPBlockOffset());
						    m_pParamBlock->GetValue(pPBlock->GetPBlockLocation(), t, vec4Val.y, FOREVER, pPBlock->GetPBlockOffset() + 1);
						    m_pParamBlock->GetValue(pPBlock->GetPBlockLocation(), t, vec4Val.z, FOREVER, pPBlock->GetPBlockOffset() + 2);
						    m_pParamBlock->GetValue(pPBlock->GetPBlockLocation(), t, vec4Val.w, FOREVER, pPBlock->GetPBlockOffset() + 3);
							pConnection->SetKey(t, nv_sys::NVType::CreateVec4Type(vec4Val));
					    }
					    else
					    {
							vec4Val = pConnection->GetValueAtTime(t).GetVec4();
						    m_pParamBlock->SetValue(pPBlock->GetPBlockLocation(), t, vec4Val.x, pPBlock->GetPBlockOffset());
						    m_pParamBlock->SetValue(pPBlock->GetPBlockLocation(), t, vec4Val.y, pPBlock->GetPBlockOffset() + 1);
						    m_pParamBlock->SetValue(pPBlock->GetPBlockLocation(), t, vec4Val.z, pPBlock->GetPBlockOffset() + 2);
						    m_pParamBlock->SetValue(pPBlock->GetPBlockLocation(), t, vec4Val.w, pPBlock->GetPBlockOffset() + 3);
					    }
				    }
				    break;

				    case nv_sys::NVTYPEID_TEXTURE:
					case nv_sys::NVTYPEID_TEXTURE2D:
					case nv_sys::NVTYPEID_TEXTURE3D:
					case nv_sys::NVTYPEID_TEXTURECUBE:
				    {
					    // Stored as ints.
					    if (SyncOption == SYNC_CONNECTION)
					    {
						    TCHAR* pName = NULL;
							nv_sys::NVType TextureValue(pConnection->GetDefaultValue());
						    m_pParamBlock->GetValue(pPBlock->GetPBlockLocation(), 0, pName, FOREVER, pPBlock->GetPBlockOffset());
						    if (pName && strlen(pName))
						    {
								// TIM: Free texture first to avoid caching modified files
								m_pMat->GetVertexShader()->FreeTexture((pConnection->GetValueAtTime(t).GetTexture()));
								TextureValue.SetTexture(m_pMat->GetVertexShader()->LoadTexture(pName, pConnection));
								pConnection->SetKey(t, TextureValue);
						    } 
						    else
						    {
								TextureValue.SetTexture(0);
							    m_pMat->GetVertexShader()->FreeTexture((pConnection->GetValueAtTime(t).GetTexture()));
								pConnection->SetKey(t, TextureValue);
						    }
					    }
					    else
					    {
						    std::string strName = TextureMgr::GetSingletonPtr()->GetTextureName(pConnection->GetValueAtTime(t).GetTexture());
						    char* pName = const_cast<char*>(strName.c_str());
	
						    m_pParamBlock->SetValue(pPBlock->GetPBlockLocation(), 0, pName, pPBlock->GetPBlockOffset());
					    }
				    }
				    break;

				    default:
					    break;
			    }
            }

			itrConnection++;
		}
		
		return true;
	}

    bool SyncConnections(nv_sys::INVParameterList* pParamList)
	{
		for (unsigned int i = 0; i < pParamList->GetNumParameters(); i++)
		{
			nv_sys::INVConnectionParameter* pParam = pParamList->GetConnectionParameter(i);
			
			pParam->Animate(true);
			
			// Walk the connection keys to get the times, and sync MAX to them.
			int Time;
			for (unsigned int KeyNum = 0; KeyNum < pParam->GetNumKeys(); KeyNum++)
			{
				const nv_sys::NVType* pKey = pParam->GetKeyFromIndex(KeyNum, Time);
				Synchronize(Time, SYNC_PBLOCK, pParam);
			}
			
			pParam->Animate(false);
		}
		return true;
	}



	nv_sys::INVConnectionParameter* GetConnection(DynamicPBlockEntry* pPBlock)
	{
		tmapPBlockToConnection::const_iterator itrConnection = m_mapPBlockToConnection.find(pPBlock);
		if (itrConnection == m_mapPBlockToConnection.end())
			return NULL;

		return itrConnection->second;
	}

	void FillConnectionKeys()
	{
		tmapPBlockToConnection::iterator itrConnection = m_mapPBlockToConnection.begin();
		while (itrConnection != m_mapPBlockToConnection.end())
		{
			nv_sys::INVConnectionParameter* pConnection = itrConnection->second;
			DynamicPBlockEntry* pPBlock = itrConnection->first;
	
			Control* pController = m_pParamBlock->GetController(pPBlock->GetPBlockLocation(), pPBlock->GetPBlockOffset());

			SuspendAnimate();
			if(pController)
			{
				pConnection->DeleteAllKeys();
				pConnection->Animate(true);
				AnimateOn();
				unsigned int NumKeys = pController->NumKeys();
				for(int Key=0;Key<NumKeys;Key++)
				{
					TimeValue kt = pController->GetKeyTime(Key);
					
					switch(pConnection->GetDefaultValue().GetType())
					{
						case nv_sys::NVTYPEID_FLOAT:
						{
							float fVal;
							m_pParamBlock->GetValue(pPBlock->GetPBlockLocation(), kt, fVal, FOREVER, pPBlock->GetPBlockOffset());
							pConnection->SetKey(kt, nv_sys::NVType::CreateFloatType(fVal));
						}
						break;

						case nv_sys::NVTYPEID_VEC2:
						{
							vec2 vec2Val;
							m_pParamBlock->GetValue(pPBlock->GetPBlockLocation(), kt, vec2Val.x, FOREVER, pPBlock->GetPBlockOffset());
							m_pParamBlock->GetValue(pPBlock->GetPBlockLocation(), kt, vec2Val.y, FOREVER, pPBlock->GetPBlockOffset() + 1);
							pConnection->SetKey(kt, nv_sys::NVType::CreateVec2Type(vec2Val));
						}
						break;

						case nv_sys::NVTYPEID_VEC3:
						{
							vec3 vec3Val;
							m_pParamBlock->GetValue(pPBlock->GetPBlockLocation(), kt, vec3Val.x, FOREVER, pPBlock->GetPBlockOffset());
							m_pParamBlock->GetValue(pPBlock->GetPBlockLocation(), kt, vec3Val.y, FOREVER, pPBlock->GetPBlockOffset() + 1);
							m_pParamBlock->GetValue(pPBlock->GetPBlockLocation(), kt, vec3Val.z, FOREVER, pPBlock->GetPBlockOffset() + 2);
							pConnection->SetKey(kt, nv_sys::NVType::CreateVec3Type(vec3Val));
						}
						break;

						case nv_sys::NVTYPEID_VEC4:
						{
							vec4 vec4Val;
							m_pParamBlock->GetValue(pPBlock->GetPBlockLocation(), kt, vec4Val.x, FOREVER, pPBlock->GetPBlockOffset());
							m_pParamBlock->GetValue(pPBlock->GetPBlockLocation(), kt, vec4Val.y, FOREVER, pPBlock->GetPBlockOffset() + 1);
							m_pParamBlock->GetValue(pPBlock->GetPBlockLocation(), kt, vec4Val.z, FOREVER, pPBlock->GetPBlockOffset() + 2);
							m_pParamBlock->GetValue(pPBlock->GetPBlockLocation(), kt, vec4Val.w, FOREVER, pPBlock->GetPBlockOffset() + 3);
							pConnection->SetKey(kt, nv_sys::NVType::CreateVec4Type(vec4Val));
						}
						break;

						case nv_sys::NVTYPEID_TEXTURE:
						case nv_sys::NVTYPEID_TEXTURE2D:
						case nv_sys::NVTYPEID_TEXTURE3D:
						case nv_sys::NVTYPEID_TEXTURECUBE:
						{
							// Stored as ints.
							TCHAR* pName = NULL;
							nv_sys::NVType TextureValue(pConnection->GetDefaultValue());
							m_pParamBlock->GetValue(pPBlock->GetPBlockLocation(), 0, pName, FOREVER, pPBlock->GetPBlockOffset());
							if (pName && strlen(pName))
							{
								// TIM: Free texture first to avoid caching modified files
								m_pMat->GetVertexShader()->FreeTexture((pConnection->GetValueAtTime(kt).GetTexture()));
								TextureValue.SetTexture(m_pMat->GetVertexShader()->LoadTexture(pName, pConnection));
								pConnection->SetKey(kt, TextureValue);
							}
							else
							{
								m_pMat->GetVertexShader()->FreeTexture((pConnection->GetValueAtTime(kt).GetTexture()));
								TextureValue.SetTexture(0);
								pConnection->SetKey(kt, TextureValue);
							}
						}
						break;

					}

				}
				pConnection->Animate(false);
				AnimateOff();
			}
			ResumeAnimate();
			itrConnection++;
		}
	}
private:
	tmapConnectionToPBlock m_mapConnectionToPBlock;
	tmapPBlockToConnection m_mapPBlockToConnection;
	IParamBlock2* m_pParamBlock;
	CgFXMaterial* m_pMat;
};

#endif __GUIPBLOCK_H // GUIPBLOCK

