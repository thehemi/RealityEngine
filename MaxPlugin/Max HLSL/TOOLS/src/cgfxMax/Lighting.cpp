/*********************************************************************NVMH4****
Path:  plugins\nvmax
File:  lighting.cpp

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

Class to manage lights.  Keeps a list, adds/removes connections and manages UI for
Light selection.


******************************************************************************/
#include "pch.h"
#include "material.h"
#include "Lighting.h"
#include "RenderMesh.h"
#include "Material.h"
#include "vertexshader.h"
#include "scenemgr.h"

using namespace nv_renderdevice;
using namespace nv_sys;
using namespace nv_gui;
using namespace nv_fx;
using namespace nv_sys;
using namespace std;

static INode* FindNodeRef(ReferenceTarget *rt);
static INode* GetNodeRef(ReferenceMaker *rm) 
{
	NVPROF_FUNC("GetNodeRef");
	if (rm->SuperClassID()==BASENODE_CLASS_ID) 
		return (INode *)rm;
	else
		return rm->IsRefTarget()?FindNodeRef((ReferenceTarget *)rm):NULL;
}

static INode* FindNodeRef(ReferenceTarget *rt) 
{
	NVPROF_FUNC("FindNodeRef");
	DependentIterator di(rt);
	ReferenceMaker *rm;
	INode *nd = NULL;
	while (rm=di.Next()) 
	{	
		nd = GetNodeRef(rm);
		if (nd) return nd;
	}

	return NULL;
}

Lighting::Lighting(ShaderInfo* pShaderInfo)
: m_pShaderInfo(pShaderInfo)
{
	NVPROF_FUNC("Lighting::Lighting");
	DISPDBG(3, "Lighting::Lighting");
}

Lighting::~Lighting()
{
	NVPROF_FUNC("Lighting::~Lighting");
	DISPDBG(3, "Lighting::~Lighting");
}

void Lighting::FillLightOptions(nv_sys::INVConnectionParameter* pParam, NVGUIItem_ListBox* pListBox)
{
	NVPROF_FUNC("Lighting::FillLightOptions");	

	Interface		*Ip;
	Matrix3			Mat;
	int				i,Count;
	ObjectState		Os;
	TimeValue		Time;
	Interval		valid;
	LightObject*	pLight;

	Ip	  = GetCOREInterface();
	Time  = Ip->GetTime();
	INode * Root  = Ip->GetRootNode();
	Count = Root->NumberOfChildren();

	unsigned int NumDir = 0;
	unsigned int NumPoint = 0;
	unsigned int NumSpot = 0;
    unsigned int NumLights = 0;

	ULONG hSelected = 0;
	tmapParamToLight::iterator itrParams = m_mapParamToLight.find(pParam);
	if (itrParams != m_mapParamToLight.end())
	{
		hSelected = itrParams->second;
	}

	pListBox->AddString("<effect default>");
	pListBox->SetPrivateData(pListBox->GetNumStrings() - 1, (void*)NULL);

	int iSelected = 0;
	for(i=0; i < Count; i++) 
	{
		INode * pNode = Root->GetChildNode(i);
		Os   = pNode->EvalWorldState(Time);
		ULONG hNode = pNode->GetHandle();

		if(Os.obj && Os.obj->SuperClassID() == LIGHT_CLASS_ID) 
		{
            NumLights++;

			LightState Ls;
			pLight = (LightObject *)Os.obj;
			pLight->EvalLightState(Time,valid,&Ls);

			// Don't handle ambient
			if (Ls.type == AMBIENT_LGT)
				continue;

			tNodeInfo* pNodeInfo = new tNodeInfo;
			pNodeInfo->m_hNode = hNode;

			switch(Ls.type)
			{
				case OMNI_LGT:
				{
                    NumPoint++;
					pListBox->AddString(pNode->GetName());
					pListBox->SetPrivateData(pListBox->GetNumStrings() - 1, pNodeInfo);
				}
				break;

				case SPOT_LGT:
				{
                    NumSpot++;
					pListBox->AddString(pNode->GetName());
					pListBox->SetPrivateData(pListBox->GetNumStrings() - 1, pNodeInfo);
				}
				break;

				case DIRECT_LGT:
				{
                    NumDir++;
					pListBox->AddString(pNode->GetName());
					pListBox->SetPrivateData(pListBox->GetNumStrings() - 1, pNodeInfo);
				}
				break;

				default:
					continue;
			}

			if (hNode == hSelected)
			{
				iSelected = pListBox->GetNumStrings() - 1;
			}
		}
	}
	
	// Set the index - either 0 or the current selection
	pListBox->SetIndex(iSelected);

}

void Lighting::AutoLightUpdate(ULONG hExclude)
{
	NVPROF_FUNC("Lighting::AutoLightUpdate");	

	Interface		*Ip;
	Matrix3			Mat;
	int				i,Count;
	ObjectState		Os;
	TimeValue		Time;
	Interval		valid;
	LightObject*	pLight;

	Ip	  = GetCOREInterface();
	Time  = Ip->GetTime();
	INode * Root  = Ip->GetRootNode();
	Count = Root->NumberOfChildren();

	// Walk the effect light list
	INVParameterList* pParams = m_pShaderInfo->GetCurrentShader()->GetParameterList();

	// Walk the parameters
	for (unsigned int iParam = 0; iParam < pParams->GetNumParameters(); iParam++)
	{
		// For each param
		INVConnectionParameter* pParam = pParams->GetConnectionParameter(iParam);

		// We are tracking it, so don't try to assign
		if (m_mapParamToLight.find(pParam) != m_mapParamToLight.end())
			continue;

		// Is it a param we could connect?
		eANNOTATIONVALUEID ValueID = pParam->GetDefaultValue().GetObjectSemantics()->FindAnnotationValue(ANNOTATIONNAMEID_OBJECT);
		if ((ValueID != ANNOTATIONVALUEID_POINTLIGHT) &&
			(ValueID != ANNOTATIONVALUEID_SPOTLIGHT) &&
			(ValueID != ANNOTATIONVALUEID_DIRLIGHT))
			continue;

		for(i=0; i < Count; i++) 
		{
			INode * pNode = Root->GetChildNode(i);
			Os   = pNode->EvalWorldState(Time);
			ULONG hNode = pNode->GetHandle();
			if (hNode == hExclude)
				continue;

			if(Os.obj && Os.obj->SuperClassID() == LIGHT_CLASS_ID) 
			{
				LightState Ls;
				pLight = (LightObject *)Os.obj;
				pLight->EvalLightState(Time,valid,&Ls);

				// Don't handle ambient
				if (Ls.type == AMBIENT_LGT)
					continue;

				switch(Ls.type)
				{
					case OMNI_LGT:
					{
						if (ValueID == ANNOTATIONVALUEID_POINTLIGHT)
							m_mapParamToLight[pParam] = pNode->GetHandle();
					}
					break;

					case SPOT_LGT:
					{
						if (ValueID == ANNOTATIONVALUEID_SPOTLIGHT)
							m_mapParamToLight[pParam] = pNode->GetHandle();
					}
					break;

					case DIRECT_LGT:
					{
						if (ValueID == ANNOTATIONVALUEID_DIRLIGHT)
							m_mapParamToLight[pParam] = pNode->GetHandle();
					}
					break;

					default:
						continue;
				}

			}
		}
	}
	
}

void Lighting::SwitchLight(INVConnectionParameter* pParam, INVGUIItem_ListBox* pListBox)
{
	NVPROF_FUNC("Lighting::SwitchLight");

	tNodeInfo* pNodeInfo = (tNodeInfo*)pListBox->GetPrivateData(pListBox->GetIndex());

	tmapParamToLight::iterator itrFound = m_mapParamToLight.find(pParam);
	if (pNodeInfo == NULL)
	{

		if (itrFound != m_mapParamToLight.end())
		{
			// Reset the param to the default value
			m_mapParamToLight.erase(itrFound);
			pParam->SetToDefaultValue();
		}
	}
	else
	{
		if (itrFound != m_mapParamToLight.end())
		{
			itrFound->second = pNodeInfo->m_hNode;
		}
		else
		{
			m_mapParamToLight[pParam] = pNodeInfo->m_hNode;
		}
	}

	return;

}

// Get info on how to restore lights
void Lighting::GetSaveInfo(tvecLightStreamInfo& LightSave)
{
	NVPROF_FUNC("Lighting::GetSaveInfo");
	LightSave.clear();

	tmapParamToLight::iterator itrParams = m_mapParamToLight.begin();
	while (itrParams != m_mapParamToLight.end())
	{
		// Remember this light in the parameter list.
		tLightStreamInfo StreamInfo;
		StreamInfo.Reserved = 0;
		StreamInfo.SemanticID = itrParams->first->GetDefaultValue().GetObjectSemantics()->GetSemanticID();
		StreamInfo.SceneLightHandle = itrParams->second;
		StreamInfo.ObjectID = itrParams->first->GetDefaultValue().GetObjectSemantics()->FindAnnotationValue(ANNOTATIONNAMEID_OBJECT);
		ZeroMemory(&StreamInfo.szName[0], MAX_NAME);
		unsigned int Length = strlen(itrParams->first->GetDefaultValue().GetObjectSemantics()->GetName());
		if (Length > (MAX_NAME - 1))
			Length = (MAX_NAME - 1);
		memcpy(StreamInfo.szName, itrParams->first->GetDefaultValue().GetObjectSemantics()->GetName(), Length);
		LightSave.push_back(StreamInfo);

		itrParams++;
	}
}

// Restore the lights from the info.
void Lighting::SetLoadInfo(tvecLightStreamInfo& LightLoadStream)
{
	NVPROF_FUNC("Lighting::SetLoadInfo");

	m_mapParamToLight.clear();

	// For each loaded light info
	tvecLightStreamInfo::iterator itrStream = LightLoadStream.begin();
	while (itrStream != LightLoadStream.end())
	{
		INode* pNode = GetCOREInterface()->GetINodeByHandle(itrStream->SceneLightHandle);
		if (!pNode)
		{
			itrStream++;
			continue;
		}

		// Walk the effect light list
		INVParameterList* pParams = m_pShaderInfo->GetCurrentShader()->GetParameterList();

		// Walk the parameters
		for (unsigned int i = 0; i < pParams->GetNumParameters(); i++)
		{
			// For each param
			INVConnectionParameter* pParam = pParams->GetConnectionParameter(i);

			if ((pParam->GetDefaultValue().GetObjectSemantics()->GetSemanticID() == itrStream->SemanticID) &&
				(pParam->GetDefaultValue().GetObjectSemantics()->FindAnnotationValue(ANNOTATIONNAMEID_OBJECT) == itrStream->ObjectID) &&
				(stricmp(pParam->GetDefaultValue().GetObjectSemantics()->GetName(), itrStream->szName) == 0))
			{
				m_mapParamToLight[pParam] = itrStream->SceneLightHandle;
				break;
			}
		}
		itrStream++;
	}

	// Auto update the remaining
	AutoLightUpdate(0);
}

bool Lighting::CheckRemoveNode(ULONG hNode)
{
	bool bUpdated = false;

	// Current time
	TimeValue Time  = GetCOREInterface()->GetTime();
	ObjectState		Os;

	// Can't find it, better update the UI.
	INode* pNode = GetCOREInterface()->GetINodeByHandle(hNode);
	if (!pNode)
		return true;

	// Current time
	Os   = pNode->EvalWorldState(Time);
	
	// Is it an active light?
	if(!pNode || !((Os.obj) && (Os.obj->SuperClassID() == LIGHT_CLASS_ID)) )
	{
		return false;
	}

	// Updated because it's a light and we want to update the UI
	bUpdated = true;

	// Walk all the nodes we are watching
	tmapParamToLight::iterator itrParams = m_mapParamToLight.begin();
	while (itrParams != m_mapParamToLight.end())
	{
		// If it's this node, forget about it.
		if (itrParams->second == hNode)
		{
			// Reset the parameter to default because we just killed it's attached light.
			itrParams->first->SetToDefaultValue();
			
			m_mapParamToLight.erase(itrParams);
			
			itrParams = m_mapParamToLight.begin();
			continue;
		}

		itrParams++;
	}

	// Update lights, but exclude this one because it might not have gone away completely.
	AutoLightUpdate(hNode);

	return bUpdated;
}

// See if we the new node can be attached to us, because we have a light in the shader currently unconnected.
bool Lighting::CheckAddNode(ULONG hNode)
{
	bool bUpdated = false;

	INode* pNode = GetCOREInterface()->GetINodeByHandle(hNode);
	if (!pNode)
		return false;

	// Current time
	TimeValue Time  = GetCOREInterface()->GetTime();
	ObjectState Os   = pNode->EvalWorldState(Time);
	
	// Is it an active light?
	if(!pNode || !((Os.obj) && (Os.obj->SuperClassID() == LIGHT_CLASS_ID)) )
	{
		return false;
	}

	LightState Ls;
	Interval valid;
	LightObject* pLight = (LightObject *)Os.obj;
	pLight->EvalLightState(Time,valid,&Ls);

	// Don't handle ambient
	if (Ls.type == AMBIENT_LGT)
		return false;

	AutoLightUpdate(0);

	// Need to update UI because we found a light we might care about.
	return true;
}

INode* Lighting::FindAttachedNode(INVConnectionParameter* pParam)
{
	NVPROF_FUNC("Lighting::FindAttachedNode");

	tmapParamToLight::iterator itrFound = m_mapParamToLight.find(pParam);
	if (itrFound == m_mapParamToLight.end())
		return false;

	INode* pNode = GetCOREInterface()->GetINodeByHandle(itrFound->second);
	if (!pNode)
	{
		m_mapParamToLight.erase(itrFound);
		return NULL;
	}	

	return pNode;
}

string StripPath(const string& fileName);/*{
	string ret = fileName;
	if(fileName.find_last_of("\\")!=-1){
		int i = fileName.find_last_of("\\");
		ret = fileName.substr(i+1);
	}
	else if(fileName.find_last_of("/")!=-1){
		int i = fileName.find_last_of("\\");
		ret = fileName.substr(i+1);
	}
	
	return ret;
}*/

bool Lighting::ApplyAttachedNode(INVConnectionParameter* pParam, CgFXMaterial* projMapHelp)
{
	NVPROF_FUNC("Lighting::ApplyAttachedNode");

	ObjectState		Os;
	TimeValue		Time;
	Point3	Pos,Target, Dir, Color;
	Matrix3 Mat;
	LightObject* pLight = NULL;


	INode* pNode = FindAttachedNode(pParam);
	if (!pNode)
		return false;

	Time = GetCOREInterface()->GetTime();
	Os  = pNode->EvalWorldState(Time);
	if(Os.obj && Os.obj->SuperClassID() == LIGHT_CLASS_ID) 
	{
		pLight = (LightObject *)Os.obj;
	}
	
	nv_sys::eSEMANTICID SemanticID = pParam->GetDefaultValue().GetObjectSemantics()->GetSemanticID();
	if (!pLight)
	{
	    // If no light were found, we enable a
	    // default light
		switch(SemanticID)
		{
			case SEMANTICID_POSITION:
				pParam->SetKey(Time, NVType::CreateVec4Type(vec4_null));
				break;
			case SEMANTICID_DIRECTION:
				pParam->SetKey(Time, NVType::CreateVec4Type(vec4_z));
				break;
			case SEMANTICID_DIFFUSE:
				pParam->SetKey(Time, NVType::CreateVec4Type(vec4_one));
				break;
			case SEMANTICID_SPECULAR:
				pParam->SetKey(Time, NVType::CreateVec4Type(vec4_null));
				break;
			default: 
				break;
		}
		return true;
	}

	LightState Ls;
	Interval valid;
	pLight->EvalLightState(Time,valid,&Ls);

	if (SemanticID == SEMANTICID_POSITION || SemanticID == SEMANTICID_DIRECTION || SemanticID == SEMANTICID_LIGHTPROJECTION || SemanticID == SEMANTICID_PROJECTIONMAP)
	{
		Mat				= pNode->GetNodeTM(Time);			
		Pos				= Mat.GetTrans();
		Dir				= Mat.GetRow(2);

		vec4 Dir4 = -vec4(Dir.x, Dir.y, Dir.z, 0.0f);
		vec4 Pos4 = vec4(Pos.x, Pos.y, Pos.z, 1.0f);
		

		normalize(Dir4);

		switch (SemanticID)
		{
			case SEMANTICID_POSITION:
				pParam->SetKey(Time, NVType::CreateVec4Type(Pos4));
				break;
			case SEMANTICID_DIRECTION:
				pParam->SetKey(Time, NVType::CreateVec4Type(Dir4));
				break;
			case SEMANTICID_PROJECTIONMAP:
			{
				if(projMapHelp == NULL)
					break;

				TimeValue t = GetCOREInterface()->GetTime();
				Texmap* tex = pLight->GetProjMap();
				string name;
				if (tex && tex->ClassID() == Class_ID(BMTEX_CLASS_ID, 0x00))
					 name = ((BitmapTex *)tex)->GetMapName();

				if(name.length() == 0)
					break; // Don't continue if no projection map to use. FIXME: Need to be able to unset a map

				// Update the GUI item with the light's new texture..

				// First compare, see if we really need to update
				TSTR strPath = TextureMgr::GetSingletonPtr()->GetTextureName((pParam->GetValueAtTime(t).GetTexture()));
				if(strPath.data() && string(strPath).find(StripPath(name)) != -1)
					break; // No change

				NVType TextureValue(pParam->GetDefaultValue());
				TextureValue.SetTexture(0);
				if (pParam->GetValueAtTime(t).GetTexture() != 0){
					TextureMgr::GetSingletonPtr()->Release((pParam->GetValueAtTime(t).GetTexture()));
					// Reset the texture
					pParam->SetKey(t, TextureValue);
				}
				INVTexture* pTex = projMapHelp->GetVertexShader()->LoadTexture(name.c_str(), pParam);
				if (pTex){
					TextureValue.SetTexture((pTex));
					//pItem->SetState(nv_gui::INVGUIItem::kUnset,false);
					pParam->SetKey(t, TextureValue);
				}
				else{
                    //pItem->SetState(nv_gui::INVGUIItem::kUnset,true);
					//pPath->SetPath("");
				}

				break;
			}
			case SEMANTICID_LIGHTPROJECTION:
			{
				vec3 up   = vec3(0,1,0);
				vec3 dirX = normalize(cross(vec3(),up, Dir4));
				vec3 dirY = cross(vec3(),Dir4, dirX);
				mat4 MatLight;
				MatLight.set_row(0,vec4(dirX.x,dirY.x,Dir4.x,0));
				MatLight.set_row(1,vec4(dirX.y,dirY.y,Dir4.y,0));
				MatLight.set_row(2,vec4(dirX.z,dirY.z,Dir4.z,0));
				MatLight.set_row(3,vec4(-dot(dirX,Pos4),-dot(dirY,Pos4),-dot(Dir4,Pos4),1));

				D3DXMATRIX MatProj;
				D3DXMatrixPerspectiveFovLH(&MatProj,DEG_RAD(Ls.fallsize),1.0f,1.0f,50.0f);

				MatLight = MatLight * *(mat4*)&MatProj;
				MatLight = transpose(MatLight);
				pParam->SetKey(Time, NVType::CreateMatrixType(MatLight.mat_array,4,4));
			} 
			break;
			default:
				assert(!"Unknown type");
				break;
		}
	}
	else
	{
		Color = Ls.color * Ls.intens;
		vec4 Color4 = vec4(Color.x, Color.y, Color.z, 1.0f);
		switch(SemanticID)
		{
			case SEMANTICID_DIFFUSE:
				pParam->SetKey(Time, NVType::CreateVec4Type(Color4));
				break;
			case SEMANTICID_SPECULAR:
				pParam->SetKey(Time, NVType::CreateVec4Type(vec4_null));
				break;
			case SEMANTICID_UMBRA:
			{
				float fValue = Ls.hotsize * nv_to_rad;
				fValue = cos(fValue/2.f); // WTF am I doing
				pParam->SetKey(Time, NVType::CreateFloatType(fValue));
			}
			break;
			case SEMANTICID_PENUMBRA:
			{
				float fValue = Ls.hotsize * nv_to_rad;
				fValue = cos(fValue/2.f); // WTF am I doing
				pParam->SetKey(Time, NVType::CreateFloatType(fValue));
			}
			break;
			case SEMANTICID_LIGHTRANGE:
			{
				float fValue = Ls.attenEnd;
				pParam->SetKey(Time, NVType::CreateFloatType(fValue));
			}
			break;
			case SEMANTICID_FALLOFF:
			{
				float fValue = Ls.fallsize * nv_to_rad;
				fValue = cos(fValue/2.f); // WTF am I doing
				pParam->SetKey(Time, NVType::CreateFloatType(fValue));
			}
			break;
			default:
				assert(!"Unknown type!");
				break;
		}
	}

	return true;
}

