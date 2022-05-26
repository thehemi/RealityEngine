/*********************************************************************NVMH4****
Path:  plugins/nvmax
File:  tweakables.cpp

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

A class to manage communication with nv_gui.  Builds the UI based on the list of 
connection parameters, and receives and sends change notifications to nv_gui, the 
GUI dll.



******************************************************************************/

#include "pch.h"
#include "tweakables.h"
#include "RenderMesh.h"
#include "Material.h"
#include "vertexshader.h"

using namespace nv_renderdevice;
using namespace nv_sys;
using namespace nv_gui;
using namespace nv_fx;
using namespace nv_sys;
using namespace std;

DWORD MESSAGE_EVENT_CLOSEPANEL = WM_USER + 1000;
DWORD MESSAGE_EVENT_UPDATEITEM = WM_USER + 1001;

bool g_bShowGUI = false;

// create an image list for the specified BMP resource
void MakeImageList (UINT inBitmapID, UINT inMaskBitmapID, HIMAGELIST & outImageList, int ImageWidth, int ImageHeight, int NumImages)
{
    HBITMAP		bm;
    HBITMAP		bmMask;

	bm = (HBITMAP)::LoadImage ( g_hInstance,	MAKEINTRESOURCE (inBitmapID), IMAGE_BITMAP, 0, 0, (LR_DEFAULTSIZE| LR_CREATEDIBSECTION));
	bmMask = (HBITMAP)::LoadImage (g_hInstance, MAKEINTRESOURCE (inMaskBitmapID), IMAGE_BITMAP, 0, 0, (LR_DEFAULTSIZE | LR_CREATEDIBSECTION));

 	// create a 24 bit image list with the same dimensions and number
	// of buttons as the toolbar

    outImageList = ImageList_Create(ImageWidth, ImageHeight, ILC_COLOR24|ILC_MASK, NumImages, 0);
    // attach the bitmap to the image list
    ImageList_Add(outImageList, bm,bmMask);
}


CTweakables::CTweakables(CgFXMaterial* pMaterial)
: m_pGUI(NULL),
m_hGUILib(NULL),
m_pRootItem(NULL),
m_pMaterial(pMaterial),
m_hImageList(NULL)
{

   
}

CTweakables::~CTweakables()
{
	ClearGUI();
	SAFE_RELEASE(m_pGUI);

    if (m_hImageList)
        ImageList_Destroy(m_hImageList);
	
	if (m_hGUILib)
		FreeLibrary(m_hGUILib);
}

void CTweakables::OnShowTweakables()
{
	if (!m_pGUI)
	{
		m_pGUI = INVGUI::CreateInstance();

            char pszAppName[MAX_PATH];
            LoadString(g_hInstance,IDS_VSCLASS_NAME,pszAppName,MAX_PATH);
            m_pGUI->SetAppName(pszAppName);
		    m_pGUI->AddEventSink(this);

		    HIMAGELIST ImageList;
            HIMAGELIST ImageListDisabled;
		    MakeImageList(IDB_TWEAKABLE_TYPES,IDB_TWEAKABLE_TYPES_MASK,ImageList,16,16,7); 
            MakeImageList(IDB_TWEAKABLE_TYPES_DISABLED,IDB_TWEAKABLE_TYPES_MASK,ImageListDisabled,16,16,7); 
            m_pGUI->SetImageList(ImageList,ImageListDisabled);

            m_pGUI->SetStateImageIdx(nv_gui::INVGUIItem::kUnset,8);

		    BuildGUI();
		    m_pGUI->SetVisible(true);
			g_bShowGUI = true;
	}
	else
	{
		if (m_pGUI->IsVisible())
		{
			m_pGUI->SetVisible(false);
			g_bShowGUI = false;
		}
		else
		{
			m_pGUI->SetVisible(true);
			g_bShowGUI = true;
			RefreshGUI();
		}
	}
}

INVGUI* CTweakables::GetGUI()
{
	return m_pGUI;
}

DWORD CTweakables::GetMessage(const char* szMessage)
{
	if (stricmp(szMessage, EVENT_CLOSEPANEL) == 0)
		return MESSAGE_EVENT_CLOSEPANEL;
	else if (stricmp(szMessage, EVENT_UPDATEITEM) == 0)
		return MESSAGE_EVENT_UPDATEITEM;

	return 0;
}

bool CTweakables::PostMessage(const char* pszEvent, void* pEventData)
{
	OnEvent(pszEvent, pEventData);
	return true;
}

// Stop max taking over key presses during UI element tweaking.
void CTweakables::OnSetFocus()
{
	DisableAccelerators();
}

void CTweakables::OnKillFocus()
{
	EnableAccelerators();
}

// Remove all child elements from a parent GUI item.
void ClearChildren(INVGUIItem* pParent)
{
	if (!pParent)
		return;

	for (unsigned int i = 0; i < pParent->GetNumChildren(); i++)
	{
		INVGUIItem* pChild = pParent->GetChild(i);

		// Delete any private data in the list box.
		if (pChild->GetType() == GUITYPE_LISTBOX)
		{
			INVGUIItem_ListBox* pLB = static_cast<INVGUIItem_ListBox*>(pChild);
			for (unsigned int Entry = 0; Entry < pLB->GetNumStrings(); Entry++)
			{
				void* pData = pLB->GetPrivateData(Entry);
				delete pData;
			}
		}

		if (pChild)
		{
			ClearChildren(pChild);
			SAFE_RELEASE(pChild);
		}
	}
}

// Completely empty the GUI
bool CTweakables::ClearGUI()
{
	ClearChildren(m_pRootItem);
	SAFE_RELEASE(m_pRootItem);

	if (m_pGUI)
		m_pGUI->ClearGUI();

	m_mapGUIItemTweakableInfo.clear();
	DISPDBG(1, "Cleared GUIItemTweaks");

	return true;
}

// Build the GUI from scratch, based on the current settings and connection parameters
bool CTweakables::BuildGUI()
{
	ClearGUI();
	if (!m_pGUI)
		return true;	

    ShaderInfo * sinfo = m_pMaterial->GetShaderInfo();

    m_pRootItem = static_cast<INVGUIItem*>(new NVGUIItem_Branch("Properties"));
	m_pGUI->AddItem(m_pRootItem);
    
    if (sinfo)
    {
	    ShaderInfoData* pShader = sinfo->GetCurrentShader();

        INVGUIItem_FilePath* pFilePath = static_cast<INVGUIItem_FilePath*>(new NVGUIItem_FilePath("HLSL Shader"));
		pFilePath->SetInfoText(_T("HLSL Filename"));
	    pFilePath->SetTruncatedDisplay(TRUE);
	    pFilePath->SetExtensionString("HLSL (*.fx)|*.fx||");
        pFilePath->SetPath(pShader->GetEffectFile().c_str());
        pFilePath->SetState(nv_gui::INVGUIItem::kReadOnly,false);
				
    	// Store a pointer to the material so we can get it back when the item changes.
	    tTweakableInfo TweakInfo;
	    ZeroMemory(&TweakInfo, sizeof(tTweakableInfo));
	    TweakInfo.m_Type = TWEAKTYPE_CGFXSHADER;
	    m_mapGUIItemTweakableInfo[pFilePath] = TweakInfo;

	    m_pGUI->AddChild(m_pRootItem, pFilePath);

        // Create a static item
        FillGUI(pFilePath, pShader->GetParameterList());
    }

    m_pGUI->ExpandAll();

	return true;
}

// Fill the gui below a node based on a connection parameter list.
void CTweakables::FillGUI(INVGUIItem* pRoot, INVParameterList* pParamList)
{
    unsigned int i;
    tTweakableInfo TweakInfo;
    INVGUIItem* pTweak = NULL;

    if (!pParamList)
        return;

	ShaderInfoData* pShader = m_pMaterial->GetShaderInfo()->GetCurrentShader();

	TimeValue t = GetCOREInterface()->GetTime();

    // Add the technique
    NVGUIItem_ListBox* pListBox = new NVGUIItem_ListBox("Technique");
    //pListBox->HasImage(TRUE,6);

	LPD3DXEFFECT pEffect = pShader->GetEffect();
	if (pEffect)
	{
		for (i = 0; i < pShader->GetDesc().Techniques; i++)
		{
			NVFXHANDLE hTechnique = pEffect->GetTechnique(i);
			D3DXTECHNIQUE_DESC TechniqueDesc;
			pEffect->GetTechniqueDesc(hTechnique, &TechniqueDesc);
	        pListBox->AddString(TechniqueDesc.Name);

			// Found technique name matching old technique
			if(string(TechniqueDesc.Name) == pShader->GetTechnique()){
				pListBox->SetIndex(i);
			}

		}
    }
	else
	{
		pListBox->AddString("<invalid>");
	}

    //pListBox->SetIndex(pShader->GetTechnique());
	pTweak = static_cast<INVGUIItem*>(pListBox);
	TweakInfo.m_Type = TWEAKTYPE_TECHNIQUE;
	m_mapGUIItemTweakableInfo[pTweak] = TweakInfo;
	pTweak->SetInfoText("Technique Number");
    pTweak->SetState(nv_gui::INVGUIItem::kReadOnly,false);
	m_pGUI->AddChild(pRoot, pTweak);

	m_pTechniqueTweak = pTweak;
    
	// Walk through the parameters for this material's tweakables
	for (i = 0; i < pParamList->GetNumParameters(); i++)
	{
		// Find out the active connection parameter
		INVConnectionParameter* pParam = pParamList->GetConnectionParameter(i);

		// Is this an application owned connection?
		bool bCanEdit = m_pMaterial->GetVertexShader()->IsEditableParam(pParam);

		if (!pParam)
			continue;

		// TIM: Don't include vars which aren't annotated in the GUI
		// (Optionally, still include them if they have semantics)
		if(pParam->GetDefaultValue().GetObjectSemantics()->GetNumAnnotations() == 0
			/*&& pParam->GetDefaultValue().GetObjectSemantics()->GetSemanticID() == SEMANTICID_UNKNOWN*/ )
			continue;

		tTweakableInfo TweakInfo;

		// Special case lights
		eANNOTATIONVALUEID ValueID = pParam->GetDefaultValue().GetObjectSemantics()->FindAnnotationValue(ANNOTATIONNAMEID_OBJECT);
		if (ValueID == ANNOTATIONVALUEID_DIRLIGHT ||
			ValueID == ANNOTATIONVALUEID_SPOTLIGHT ||
			ValueID == ANNOTATIONVALUEID_POINTLIGHT)
		{
			TweakInfo.m_Type = TWEAKTYPE_LIGHT;
			if(pParam->GetDefaultValue().GetObjectSemantics()->GetSemanticID() != SEMANTICID_POSITION)
				continue; // Ignore non-position attributes, we'll keep the GUI nice and clean by only showing pos

			NVGUIItem_ListBox* pListBox = new NVGUIItem_ListBox(pParam->GetDefaultValue().GetObjectSemantics()->GetName());
			switch(pParam->GetDefaultValue().GetObjectSemantics()->GetSemanticID())
			{
				case SEMANTICID_DIFFUSE:
					pListBox->HasImage(TRUE,3);
					break;
				default:
					pListBox->HasImage(TRUE,1);
					break;
			}

			m_pMaterial->GetVertexShader()->GetLighting()->FillLightOptions(pParam, pListBox);
			pTweak = static_cast<INVGUIItem*>(pListBox);
		}
		else
		{
			TweakInfo.m_Type = TWEAKTYPE_CONNECTION;

			// Get the tweakable type
			switch(pParam->GetDefaultValue().GetType())
			{
				case NVTYPEID_DWORD:
				{
					NVGUIItem_Dword* pGUIItem = new NVGUIItem_Dword(pParam->GetDefaultValue().GetObjectSemantics()->GetName());
					pGUIItem->HasImage(TRUE, 2);
					pGUIItem->SetDword(pParam->GetValueAtTime(t).GetDWORD());
					pTweak = static_cast<INVGUIItem*>(pGUIItem);
				}
				break;

				case NVTYPEID_VEC4:
				{
					if (pParam->GetDefaultValue().GetObjectSemantics()->GetSemanticID() == SEMANTICID_DIFFUSE || 
						pParam->GetDefaultValue().GetObjectSemantics()->GetSemanticID() == SEMANTICID_AMBIENT ||
						pParam->GetDefaultValue().GetObjectSemantics()->GetSemanticID() == SEMANTICID_SPECULAR ||
						pParam->GetDefaultValue().GetObjectSemantics()->GetSemanticID() == SEMANTICID_EMISSIVE)
					{
						NVGUIItem_Color* pColor = new NVGUIItem_Color(pParam->GetDefaultValue().GetObjectSemantics()->GetName());
						pColor->SetAlpha(true);
						pColor->SetColor(pParam->GetValueAtTime(t).GetVec4());
						pColor->HasImage(TRUE,3);
						pTweak = static_cast<INVGUIItem*>(pColor);
					}
					else
					{
						NVGUIItem_Vector4* pVec = new NVGUIItem_Vector4(pParam->GetDefaultValue().GetObjectSemantics()->GetName());
						pVec->HasImage(TRUE,1);
						pVec->SetVector(pParam->GetValueAtTime(t).GetVec4());
						pTweak = static_cast<INVGUIItem*>(pVec);
					}
				}
				break;

				case NVTYPEID_VEC3:
				{
					if (pParam->GetDefaultValue().GetObjectSemantics()->GetSemanticID() == SEMANTICID_DIFFUSE || 
						pParam->GetDefaultValue().GetObjectSemantics()->GetSemanticID() == SEMANTICID_AMBIENT ||
						pParam->GetDefaultValue().GetObjectSemantics()->GetSemanticID() == SEMANTICID_SPECULAR ||
						pParam->GetDefaultValue().GetObjectSemantics()->GetSemanticID() == SEMANTICID_EMISSIVE)
					{
						NVGUIItem_Color* pColor = new NVGUIItem_Color(pParam->GetDefaultValue().GetObjectSemantics()->GetName());
						pColor->SetAlpha(false);
						vec4 col = vec4(pParam->GetValueAtTime(t).GetVec3().x, pParam->GetValueAtTime(t).GetVec3().y, pParam->GetValueAtTime(t).GetVec3().z, 1.0f);
						pColor->SetColor(col);
						pColor->HasImage(TRUE,3);
						pTweak = static_cast<INVGUIItem*>(pColor);
					}
					else
					{
						NVGUIItem_Vector3* pVec = new NVGUIItem_Vector3(pParam->GetDefaultValue().GetObjectSemantics()->GetName());
						pVec->HasImage(TRUE,1);
						pVec->SetVector(pParam->GetValueAtTime(t).GetVec3());
						pTweak = static_cast<INVGUIItem*>(pVec);
					}
				}
				break;

				case NVTYPEID_VEC2:
				{
					NVGUIItem_Vector2* pVec = new NVGUIItem_Vector2(pParam->GetDefaultValue().GetObjectSemantics()->GetName());
					pVec->HasImage(TRUE,1);
					pVec->SetVector(pParam->GetValueAtTime(t).GetVec2());
					pTweak = static_cast<INVGUIItem*>(pVec);
				}
				break;

				case NVTYPEID_TEXTURE:
				case NVTYPEID_TEXTURE2D:
				case NVTYPEID_TEXTURE3D:
				case NVTYPEID_TEXTURECUBE:
				{
					// Add it to the list.
					NVGUIItem_FilePath* pFilePath = new NVGUIItem_FilePath(pParam->GetDefaultValue().GetObjectSemantics()->GetName());
					pTweak = static_cast<INVGUIItem*>(pFilePath);
					pFilePath->HasImage(TRUE,4);
                
					const char* pszPath = NULL;
					pszPath = TextureMgr::GetSingletonPtr()->GetTextureName((pParam->GetValueAtTime(t).GetTexture()));
					if (pszPath == NULL || (strlen(pszPath) == 0))
					{
						pTweak->SetState(nv_gui::INVGUIItem::kUnset,true);
						pFilePath->SetPath("");
					}
					else
					{
						pTweak->SetState(nv_gui::INVGUIItem::kUnset,false);
						pFilePath->SetPath(pszPath);
					}

					pFilePath->SetTruncatedDisplay(TRUE);
					pFilePath->SetExtensionString(GetString(IDS_TEXTURE_TYPES));//"DirectDrawSurface textures (*.dds)|*.dds||");
					pFilePath->SetPreview(TRUE);
				}
				break;

				case NVTYPEID_BOOL:
				{
					NVGUIItem_Bool* pBoolItem = new NVGUIItem_Bool(pParam->GetDefaultValue().GetObjectSemantics()->GetName());
					pBoolItem->HasImage(TRUE,6);
					pBoolItem->SetBool(pParam->GetValueAtTime(t).GetBool());
					pTweak = static_cast<INVGUIItem*>(pBoolItem);
				}
				break;

				case NVTYPEID_FLOAT:
				{
					BOOL bSetRange = TRUE;
					float fMin = 0.0f;
					float fMax = 1.0f;
					float fStep = 1.0f;
					NVGUIItem_Float* pFloatItem = new NVGUIItem_Float(pParam->GetDefaultValue().GetObjectSemantics()->GetName());
					pFloatItem->SetFloat(pParam->GetValueAtTime(t).GetFloat());
                
					const tAnnotationInfo * AnInfo = pParam->GetDefaultValue().GetObjectSemantics()->FindAnnotationInfo(ANNOTATIONNAMEID_UIMIN);
					if (AnInfo)
					{
						if (AnInfo->m_Value.GetType() == NVTYPEID_STRING)
							fMin = atof(AnInfo->m_Value.GetString());
						else if (AnInfo->m_Value.GetType() == NVTYPEID_INT)
							fMin = AnInfo->m_Value.GetInt();
						else if (AnInfo->m_Value.GetType() == NVTYPEID_FLOAT)
							fMin = AnInfo->m_Value.GetFloat();
						else
							bSetRange = FALSE;
						AnInfo = pParam->GetDefaultValue().GetObjectSemantics()->FindAnnotationInfo(ANNOTATIONNAMEID_UIMAX);
						if (AnInfo && bSetRange)
						{
							if (AnInfo->m_Value.GetType() == NVTYPEID_STRING)
								fMax = atof(AnInfo->m_Value.GetString());
							else if (AnInfo->m_Value.GetType() == NVTYPEID_INT)
								fMax = AnInfo->m_Value.GetInt();
							else if (AnInfo->m_Value.GetType() == NVTYPEID_FLOAT)
								fMax = AnInfo->m_Value.GetFloat();
							else
								bSetRange = FALSE;
						}
						AnInfo = pParam->GetDefaultValue().GetObjectSemantics()->FindAnnotationInfo(ANNOTATIONNAMEID_UISTEP);
						if (AnInfo && bSetRange)
						{
							if (AnInfo->m_Value.GetType() == NVTYPEID_STRING)
								fStep = atof(AnInfo->m_Value.GetString());
							else if (AnInfo->m_Value.GetType() == NVTYPEID_INT)
								fStep = AnInfo->m_Value.GetInt();
							else if (AnInfo->m_Value.GetType() == NVTYPEID_FLOAT)
								fStep = AnInfo->m_Value.GetFloat();
							else
								bSetRange = FALSE;
						}
					}
					else
						bSetRange = FALSE;

					pFloatItem->HasImage(TRUE,2);
					if (bSetRange)
						pFloatItem->SetMinMaxStep(fMin,fMax,fStep);
					pTweak = static_cast<INVGUIItem*>(pFloatItem);
				}
				break;

				case NVTYPEID_MATRIX:
				{
					NVGUIItem_Matrix* pMatrix = new NVGUIItem_Matrix(pParam->GetDefaultValue().GetObjectSemantics()->GetName(), pParam->GetDefaultValue().GetRows(), pParam->GetDefaultValue().GetColumns());
					pMatrix->HasImage(TRUE,0);
					pMatrix->SetArray(pParam->GetValueAtTime(t).GetMatrix());
					pTweak = static_cast<INVGUIItem*>(pMatrix);
				}
				break;

				case NVTYPEID_STRING:
				{
					NVGUIItem_Text* pTextItem = new NVGUIItem_Text(pParam->GetDefaultValue().GetObjectSemantics()->GetName());
					pTextItem->SetString(pParam->GetValueAtTime(t).GetString());
					pTextItem->HasImage(TRUE,0);
					pTweak = static_cast<INVGUIItem*>(pTextItem);
				}
				break;

				default:
				{
					// Add it to the list.
					pTweak = new NVGUIItem_Branch(pParam->GetDefaultValue().GetObjectSemantics()->GetName());
				}
				break;
			}
		}

		if (pTweak)
		{
			TweakInfo.m_pConnection = pParam;
			m_mapGUIItemTweakableInfo[pTweak] = TweakInfo;
			DISPDBG(1, "Added GUIItemTweak: " << pTweak);

			ostringstream strStream;
			strStream << "Parameter Info: Semantic=" << ConvertSemantic(pParam->GetDefaultValue().GetObjectSemantics()->GetSemanticID());
			if (TweakInfo.m_Type == TWEAKTYPE_LIGHT)
			{
                strStream << ", App Connection=Yes (Light)";
                pTweak->SetState(nv_gui::INVGUIItem::kReadOnly,false);
			}
			else
			{
				if (!bCanEdit)
				{
					strStream << ", App Connection=Yes";
					pTweak->SetState(nv_gui::INVGUIItem::kReadOnly,true);
				}
				else
				{
					strStream << ", App Connection=No";
					pTweak->SetState(nv_gui::INVGUIItem::kReadOnly,false);
				}
			}
			pTweak->SetInfoText(strStream.str().c_str());
			m_pGUI->AddChild(pRoot, pTweak);
		}
	}
}

// Callback to tell us when the gui changes.
bool CTweakables::OnEvent(const char* pszEvent, void* pEventData1)
{
	DISPDBG(3, "CTweakables::OnEvent");
	if (!stricmp(pszEvent, EVENT_CLOSEPANEL))
	{
		DISPDBG(3, "EVENT_CLOSEPANEL");
		ClearGUI();
		SAFE_RELEASE(m_pGUI);
		return true;
	}
	else if (!stricmp(pszEvent, EVENT_UPDATEITEM))
	{
		DISPDBG(3, "EVENT_UPDATEITEM");
		INVGUIItem* pItem = reinterpret_cast<INVGUIItem*>(pEventData1);
		tmapGUIItemTweakableInfo::iterator itrGUIItem = m_mapGUIItemTweakableInfo.find(pItem);
		tTweakableInfo& TweakInfo = itrGUIItem->second;

		ShaderInfoData* pShader = m_pMaterial->GetShaderInfo()->GetCurrentShader();

   		if (TweakInfo.m_Type == TWEAKTYPE_CGFXSHADER)
		{
			INVGUIItem_FilePath* pPath = reinterpret_cast<INVGUIItem_FilePath*>(pItem);
			
			DISPDBG(3, "Changed CgFX file path: " << pPath->GetPath());

            // Set the new effect, which will rebuild the tree.
			// Correct redraw notifications should also occur.
			// NEVER call load effect from here, only ever swap.
			// Swap's contract says it won't alter the GUI items if you tell it not to...
			// Essential here because we haven't finished handling the notification from nv_gui
			// and things can get messy...
			m_pMaterial->GetShaderInfo()->SwapEffect_Ver2(OWNER_NONE, pPath->GetPath(), ShaderInfo::NO_UPDATE_GUI);

			
			// Update the .fx in case the swap didn't replace with the file we expected.
			pPath->SetPath(m_pMaterial->GetShaderInfo()->GetCurrentShader()->GetEffectFile().c_str());
			m_pGUI->RefreshGUI(pPath);

       	    // Walk the children, and remove them from our mapping.
	        // We don't care about them any more.
	        for (unsigned int i = 0; i < pPath->GetNumChildren(); i++)
	        {
		        INVGUIItem* pVictim = pPath->GetChild(i);
		        tmapGUIItemTweakableInfo::iterator itrVictim = m_mapGUIItemTweakableInfo.find(pVictim);
                DISPDBG(1, "Removed GUIItemTweak: " << pVictim << ", " << pVictim->GetName());
		        SAFE_RELEASE(pVictim);
		        m_mapGUIItemTweakableInfo.erase(itrVictim);
		        
	        }
	        m_pGUI->RemoveChildren(pPath);
	        
	        FillGUI(pPath, pShader->GetParameterList());

			GetCOREInterface()->ForceCompleteRedraw();
			
		}
        else if (TweakInfo.m_Type == TWEAKTYPE_TECHNIQUE)
        {
            // Change the technique
            INVGUIItem_ListBox* pListBox = reinterpret_cast<INVGUIItem_ListBox*>(pItem);
            pShader->SetTechnique(OWNER_NONE, pListBox->GetIndex());

			// Syncing the material should generate changed updates for redraws.
			m_pMaterial->SyncMaterialToConnection(TweakInfo.m_pConnection);

			GetCOREInterface()->ForceCompleteRedraw();
        }
		else if (TweakInfo.m_Type == TWEAKTYPE_LIGHT)
		{
			INVGUIItem_ListBox* pListBox = reinterpret_cast<INVGUIItem_ListBox*>(pItem);
			m_pMaterial->GetVertexShader()->GetLighting()->SwitchLight(TweakInfo.m_pConnection, pListBox);

			// TIM: Set all the other tweakable lights to the newly selected light
			INVParameterList* pParamList = pShader->GetParameterList();
			for(int i=0;i<pParamList->GetNumParameters();i++){
				// Find out the active connection parameter
				INVConnectionParameter* pParam = pParamList->GetConnectionParameter(i);
				eANNOTATIONVALUEID ValueID = pParam->GetDefaultValue().GetObjectSemantics()->FindAnnotationValue(ANNOTATIONNAMEID_OBJECT);
				if (ValueID == ANNOTATIONVALUEID_DIRLIGHT ||
					ValueID == ANNOTATIONVALUEID_SPOTLIGHT ||
					ValueID == ANNOTATIONVALUEID_POINTLIGHT)
				{
					m_pMaterial->GetVertexShader()->GetLighting()->SwitchLight(pParam, pListBox);

					if(pParam->GetDefaultValue().GetObjectSemantics()->GetSemanticID() == SEMANTICID_PROJECTIONMAP)
						m_pMaterial->GetVertexShader()->GetLighting()->ApplyAttachedNode(pParam, m_pMaterial);
				}
			}

			// TIM: Update the technique if we need to
			{
			/*	INVGUIItem* pItem = reinterpret_cast<INVGUIItem*>(m_pTechniqueTweak);
				tmapGUIItemTweakableInfo::iterator itrGUIItem = m_mapGUIItemTweakableInfo.find(pItem);
				tTweakableInfo& TweakInfo = itrGUIItem->second;

				string name = pShader->GetTechniqueName(pListBox->GetIndex());

				m_pMaterial->GetVertexShader()->GetTechnique(
				// Change the technique
				INVGUIItem_ListBox* pListBox = reinterpret_cast<INVGUIItem_ListBox*>(m_pTechniqueTweak);
				pShader->SetTechnique(OWNER_NONE, pListBox->GetIndex());
				// Syncing the material should generate changed updates for redraws.
				m_pMaterial->SyncMaterialToConnection(TweakInfo.m_pConnection);*/
			}
			
			// Refresh to update the new light shading info
			GetCOREInterface()->ForceCompleteRedraw();

		}
        else if (TweakInfo.m_Type == TWEAKTYPE_CONNECTION)
		{
			DISPDBG(3, "Changed Connection parameter: " << TweakInfo.m_pConnection->GetDefaultValue().GetObjectSemantics()->GetName() << ", " << ConvertSemantic(TweakInfo.m_pConnection->GetDefaultValue().GetObjectSemantics()->GetSemanticID()));
			
			// Copy the GUI Item into the connection parameter.
			SyncGUIItemToConnection(pItem, TweakInfo.m_pConnection, COPY_GUIITEM_TO_CONNECTION);
            m_pGUI->RefreshGUI(itrGUIItem->first);

			// Syncing the material should generate changed updates for redraws.
			m_pMaterial->SyncMaterialToConnection(TweakInfo.m_pConnection);

			// Refresh to update the new light shading info
			GetCOREInterface()->ForceCompleteRedraw();
		}

		return true;
	}

	DISPCONSOLE("Panel EVENT_UNKNOWN!");
	return false;
}

// Code to sync between the GUI and the connection parameter.  Works in both directions.
void CTweakables::SyncGUIItemToConnection(INVGUIItem* pItem, INVConnectionParameter* pConnect, tSyncDirection SyncDirection)
{
	TimeValue t = GetCOREInterface()->GetTime();

	tmapGUIItemTweakableInfo::iterator itrGUIItem = m_mapGUIItemTweakableInfo.find(pItem);
	if (itrGUIItem != m_mapGUIItemTweakableInfo.end())
	{
	
	}
    
    // Only sync connection UI items
    if (itrGUIItem->second.m_Type != TWEAKTYPE_CONNECTION)
        return;

	switch (pItem->GetType())
	{
		case GUITYPE_FILEPATH:
		{
            INVGUIItem* pTweak = NULL;
			INVGUIItem_FilePath* pPath = static_cast<INVGUIItem_FilePath*>(pItem);
            
			if (SyncDirection == COPY_CONNECTION_TO_GUIITEM)
			{
				TSTR strPath = TextureMgr::GetSingletonPtr()->GetTextureName((pConnect->GetValueAtTime(t).GetTexture()));
				if (strPath.isNull())
                {
                    pItem->SetState(nv_gui::INVGUIItem::kUnset,true);
                    pPath->SetPath("");
                }
			    else
                {
                    pItem->SetState(nv_gui::INVGUIItem::kUnset,false);
					pPath->SetPath(strPath);
                }
			}
			else
			{
				NVType TextureValue(pConnect->GetDefaultValue());
				TextureValue.SetTexture(0);


				if (pConnect->GetValueAtTime(t).GetTexture() != 0)
				{
					TextureMgr::GetSingletonPtr()->Release((pConnect->GetValueAtTime(t).GetTexture()));

					// Reset the texture
					pConnect->SetKey(t, TextureValue);

				}

				INVTexture* pTex = m_pMaterial->GetVertexShader()->LoadTexture(pPath->GetPath(), pConnect);
				if (pTex)
				{
					TextureValue.SetTexture((pTex));

					pItem->SetState(nv_gui::INVGUIItem::kUnset,false);
					pConnect->SetKey(t, TextureValue);
				}
				else
				{
                    pItem->SetState(nv_gui::INVGUIItem::kUnset,true);
					pPath->SetPath("");
				}

			}
		}
		break;

		case GUITYPE_BOOL:
		{
			INVGUIItem_Bool* pBool = static_cast<INVGUIItem_Bool*>(pItem);
			assert(pConnect->GetDefaultValue().GetType() == NVTYPEID_BOOL);
			if (SyncDirection == COPY_CONNECTION_TO_GUIITEM)
			{
				pBool->SetBool(pConnect->GetValueAtTime(t).GetBool());
			}
			else
			{
				pConnect->SetKey(t, NVType::CreateBoolType(pBool->GetBool()));
			}			
		}
		break;

		case GUITYPE_FLOAT:
		{
			INVGUIItem_Float* pFloat = static_cast<INVGUIItem_Float*>(pItem);
			assert(pConnect->GetDefaultValue().GetType() == NVTYPEID_FLOAT);
			if (SyncDirection == COPY_CONNECTION_TO_GUIITEM)
			{
				pFloat->SetFloat(pConnect->GetValueAtTime(t).GetFloat());
			}
			else
			{
				pConnect->SetKey(t, NVType::CreateFloatType(pFloat->GetFloat()));
			}
		}
		break;

		case GUITYPE_DWORD:
		{
			INVGUIItem_Dword* pDword = static_cast<INVGUIItem_Dword*>(pItem);
			assert(pConnect->GetDefaultValue().GetType() == NVTYPEID_DWORD);
			if (SyncDirection == COPY_CONNECTION_TO_GUIITEM)
			{
				pDword->SetDword(pConnect->GetValueAtTime(t).GetDWORD());
			}
			else
			{
				pConnect->SetKey(t, NVType::CreateDWORDType(pDword->GetDword()));
			}
		}
		break;

   		case GUITYPE_VECTOR2:
		{
			INVGUIItem_Vector2* pVec2 = static_cast<INVGUIItem_Vector2*>(pItem);
			
			assert(pConnect->GetDefaultValue().GetType() == NVTYPEID_VEC2);
			if (SyncDirection == COPY_CONNECTION_TO_GUIITEM)
			{
				pVec2->SetVector(pConnect->GetValueAtTime(t).GetVec2());
			}
			else
			{
				pConnect->SetKey(t, NVType::CreateVec2Type(pVec2->GetVector()));
			}
		}
		break;

   		case GUITYPE_VECTOR3:
		{
			INVGUIItem_Vector3* pVec3 = static_cast<INVGUIItem_Vector3*>(pItem);
			
			assert(pConnect->GetDefaultValue().GetType() == NVTYPEID_VEC3);
			if (SyncDirection == COPY_CONNECTION_TO_GUIITEM)
			{
				pVec3->SetVector(pConnect->GetValueAtTime(t).GetVec3());
			}
			else
			{
				pConnect->SetKey(t, NVType::CreateVec3Type(pVec3->GetVector()));
			}
		}
		break;

   		case GUITYPE_VECTOR4:
		{
			INVGUIItem_Vector4* pVec4 = static_cast<INVGUIItem_Vector4*>(pItem);
			
			assert(pConnect->GetDefaultValue().GetType() == NVTYPEID_VEC4);
			if (SyncDirection == COPY_CONNECTION_TO_GUIITEM)
			{
				pVec4->SetVector(pConnect->GetValueAtTime(t).GetVec4());
			}
			else
			{
				pConnect->SetKey(t, NVType::CreateVec4Type(pVec4->GetVector()));
			}
		}
		break;

		case GUITYPE_MATRIX:
		{
			INVGUIItem_Matrix* pMat = static_cast<INVGUIItem_Matrix*>(pItem);
			
			assert(pConnect->GetDefaultValue().GetType() == NVTYPEID_MATRIX);
			if (SyncDirection == COPY_CONNECTION_TO_GUIITEM)
			{
				pMat->SetArray(pConnect->GetValueAtTime(t).GetMatrix());
			}
			else
			{
				pConnect->SetKey(t, NVType::CreateMatrixType(pMat->GetArray(), pMat->GetRows(), pMat->GetColumns()));
			}
		}
		break;

		case GUITYPE_COLOR:
		{
			// If it's a color, update it.
			INVGUIItem_Color* pColor = static_cast<INVGUIItem_Color*>(pItem);
			if (pConnect->GetDefaultValue().GetType() == NVTYPEID_VEC4)
			{
				if (SyncDirection == COPY_CONNECTION_TO_GUIITEM)
				{
					pColor->SetColor(pConnect->GetValueAtTime(t).GetVec4());
				}
				else
				{
					pConnect->SetKey(t, NVType::CreateVec4Type(pColor->GetColor()));
				}
			}
			else if (pConnect->GetDefaultValue().GetType() == NVTYPEID_VEC3)
			{
				if (SyncDirection == COPY_CONNECTION_TO_GUIITEM)
				{
					pColor->SetColor(pConnect->GetValueAtTime(t).GetVec3());
				}
				else
				{
                    pConnect->SetKey(t, NVType::CreateVec3Type(pColor->GetColor()));
				}
			}
			else
			{
				assert(!"Unknown connection!");
			}
		}
		break;

		default: 
			break;
	}
}

// Called to update the GUI display based on connection parameter changes.
bool CTweakables::RefreshGUI()
{
	DISPDBG(3, "Tweakables::RefreshGUI");
	if (m_pGUI && m_pGUI->IsVisible())
	{
		tmapGUIItemTweakableInfo::iterator itrGUIItem = m_mapGUIItemTweakableInfo.begin();
		while (itrGUIItem != m_mapGUIItemTweakableInfo.end())
		{
			if (itrGUIItem->second.m_pConnection)
			{
				SyncGUIItemToConnection(itrGUIItem->first, itrGUIItem->second.m_pConnection, COPY_CONNECTION_TO_GUIITEM);
			}
			m_pGUI->RefreshGUI(itrGUIItem->first);
			itrGUIItem++;
		}
	}
	return true;
}

