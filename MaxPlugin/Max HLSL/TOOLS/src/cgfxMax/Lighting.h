/*********************************************************************NVMH4****
Path:  plugins/nvmax
File:  Lighting.h

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


See lighting.cpp


******************************************************************************/

#ifndef __LIGHTING_H
#define __LIGHTING_H

#if _MSC_VER >= 1000
#pragma once
#endif 

class Mesh;
class RenderMesh;
class ShaderInfo;

typedef std::map<nv_sys::INVConnectionParameter*, ULONG> tmapParamToLight;
typedef struct tagNodeInfo
{
	ULONG m_hNode;
} tNodeInfo;

// Manages scene lighting
class Lighting
{
public:
	Lighting(ShaderInfo* pShaderInfo);
	~Lighting();
	INode*	FindAttachedNode(nv_sys::INVConnectionParameter* pParam);
	bool	ApplyAttachedNode(nv_sys::INVConnectionParameter* pParam, CgFXMaterial* projMapHelp = NULL);
	void	FillLightOptions(nv_sys::INVConnectionParameter* pParam, nv_gui::NVGUIItem_ListBox* pListBox);
	void	SwitchLight(nv_sys::INVConnectionParameter* pParam, nv_gui::INVGUIItem_ListBox* pListBox);
	void	GetSaveInfo(std::vector<tLightStreamInfo>& LightSave);
	void	SetLoadInfo(std::vector<tLightStreamInfo>& LightLoad);
	void	AutoLightUpdate(ULONG hExcludeNode);
	bool	CheckRemoveNode(ULONG hNode);
	bool	CheckAddNode(ULONG hNode);

private:
	tmapParamToLight m_mapParamToLight;
	ShaderInfo* m_pShaderInfo;

};

#endif __LIGHTING_H

