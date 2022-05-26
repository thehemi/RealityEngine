//=========== (C) Copyright 2004, Artficial Studios. All rights reserved. =======
// Editor Item Class, base for any editable item properties dialog
// Author: Mostafa Mohamed
//===============================================================================
#include "StdAfx.h"
#include "EditorItem.h"
#include "Editor.h"
#include "dxcommon\dxstdafx.h"

//-----------------------------------------------------------------------------
// Conversion Functions Needed for the editors (FROM STRING TO INT)
//-----------------------------------------------------------------------------
int ConvertToInt(LPCWSTR inString,int oldInt)
{
	int newInt=_wtoi(inString);
	if (!newInt && inString[0]!='0')
		return oldInt;
	return newInt;
}
//-----------------------------------------------------------------------------
// Conversion Functions Needed for the editors (FROM STRING TO FLOAT)
//-----------------------------------------------------------------------------
float ConvertToFloat(LPCWSTR inString,float oldFloat)
{
	float newFloat=(float)_wtof(inString);
	if (!newFloat && inString[0]!='0')
		return oldFloat;
	return newFloat;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
EditorItem::EditorItem(Editor * editor, void * item)
{
	m_Editor=editor;
	m_Item=item;
	m_PrevMouse.x = 0;
	m_PrevMouse.y = 0;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
EditorItem::~EditorItem(void)
{
}

//-----------------------------------------------------------------------------
// UI Events for specific item
//-----------------------------------------------------------------------------
void EditorItem::OnEvent( UINT nEvent, int nControlID, CDXUTControl* pControl )
{
}

//-----------------------------------------------------------------------------
// UI Widgets for specific item
//-----------------------------------------------------------------------------
void EditorItem::FillPropsDialog(CDXUTDialog * dialog)
{
}


//-----------------------------------------------------------------------------
// Handles dragging of items in the editor
//-----------------------------------------------------------------------------
void EditorItem::Update()
{

}

//====================================================================
//--------------------------------------------------------------------
// ACTOR EDITOR
//--------------------------------------------------------------------
//====================================================================
#define IDC_STATIC			-1
#define IDC_GHOST			1
#define IDC_BBOX			2
#define IDC_TOUCHINGLIGHTS	3
#define IDC_MATERIALS		4
#define IDC_PROPERTIES		5
ActorEditor::ActorEditor(Editor * editor,Actor * item): EditorItem(editor,item)
{

}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
ActorEditor::~ActorEditor(void)
{
}

//-----------------------------------------------------------------------------
// UI Events for specific item
//-----------------------------------------------------------------------------
void ActorEditor::OnEvent( UINT nEvent, int nControlID, CDXUTControl* pControl )
{
	Actor * actor=(Actor*)m_Item;
	switch (nControlID)
	{
	case IDC_GHOST:
		actor->GhostObject=((CDXUTCheckBox*)pControl)->GetChecked();
		break;
	case IDC_BBOX:
		((Editor*)m_Editor)->m_DrawBox=((CDXUTCheckBox*)pControl)->GetChecked();
		break;
	case IDC_MATERIALS:
		{
			CDXUTListBox * listBox=(CDXUTListBox*)pControl;
			Material * mat= MaterialManager::Instance()->FindMaterial(ToAnsi( listBox->GetSelectedItem()->strText));
		((Editor*)m_Editor)->SelectMaterial(mat);
		}
	}
}

//-----------------------------------------------------------------------------
// UI Widgets for specific item
//-----------------------------------------------------------------------------
void ActorEditor::FillPropsDialog(CDXUTDialog * dialog)
{
	Actor * actor=(Actor*)m_Item;

	

	//CREATE THE PROPERTY GRID CONTROL
	CDXUTPropertiesList * propGrid;
	dialog->AddPropertiesList(IDC_PROPERTIES,1,1,168,200,0,&propGrid);
	propGrid->SetLabelWidth( 75);

	//STATISTICS
	propGrid->AddTitle(L"",-1,L"Statistics",NULL);
	if (actor->MyModel)
		propGrid->AddStatic(L"Name",-1,ToUnicode(actor->MyModel->m_pFrameRoot->Name).c_str(),NULL);

	ActorInfo info;
	GetInfo(&info);

	char tString[25];

	sprintf(tString,"%i",info.TouchingLights);
	string tStr = tString;
	propGrid->AddStatic(L"T Lights",-1,ToUnicode(tStr).c_str(),NULL);

	sprintf(tString,"%i",info.PolyCount);
	tStr = tString;
	propGrid->AddStatic(L"Polygons",-1,ToUnicode(tStr).c_str(),NULL);

	sprintf(tString,"%i",info.VerticesCount);
	tStr = tString;
	propGrid->AddStatic(L"Vertices",-1,ToUnicode(tStr).c_str(),NULL);

	sprintf(tString,"%i",info.NumMeshes);
	tStr = tString;
	propGrid->AddStatic(L"Meshes",-1,ToUnicode(tStr).c_str(),NULL);

	//OPTIONS
	propGrid->AddTitle(L"",-1,L"Options",NULL);
	propGrid->AddCheckBox(L"Ghost",IDC_GHOST,actor->GhostObject,NULL);
	propGrid->AddCheckBox(L"BBOX",IDC_BBOX,((Editor*)m_Editor)->m_DrawBox,NULL);

	//MATERIALS
	CDXUTListBox * listBox;
	dialog->AddListBox(IDC_MATERIALS,1,210,168,100,0,&listBox);
	for (int i=0;i<info.Materials.size();i++)
		listBox->AddItem(ToUnicode(info.Materials[i]).c_str(),NULL);
}

//-----------------------------------------------------------------------------
// Gathers actor stats for UI
//-----------------------------------------------------------------------------
void  ActorEditor::GetInfo(ModelFrame * frame, ActorEditor::ActorInfo * info)
{
	if(!frame)
		return;

	GetInfo(frame->pFrameFirstChild,info);
	GetInfo(frame->pFrameSibling,info);

	if(!frame->mesh)
		return;
	info->NumMeshes++;

	for (int i=0;i<frame->mesh->m_Materials.size();i++)
		info->Materials.push_back(frame->mesh->m_Materials[i]->m_Name);
	for (int i=0;i<frame->mesh->m_AttribTable.size();i++)
	{
		info->PolyCount+=frame->mesh->m_AttribTable[i].FaceCount;
		info->VerticesCount+=frame->mesh->m_AttribTable[i].VertexCount;
	}
}

//-----------------------------------------------------------------------------
// Gathers actor stats for UI
//-----------------------------------------------------------------------------
void  ActorEditor::GetInfo(ActorEditor::ActorInfo * info)
{
	Actor * actor=(Actor*)m_Item;

	info->PolyCount=0;
	info->NumMeshes=0;
	info->VerticesCount=0;
	info->TouchingLights=0;

	if (actor->MyModel)
	{
		info->TouchingLights=actor->MyModel->m_TouchingLights.size();
		GetInfo(actor->MyModel->m_pFrameRoot,info);
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ActorEditor::Update(void)
{
	EditorItem::Update();
}



//====================================================================
//--------------------------------------------------------------------
// LIGHTS EDITOR
//--------------------------------------------------------------------
//====================================================================
#define IDC_STATIC			-1
#define IDC_DYNAMIC			1
#define IDC_LIGHTTYPE		2
#define IDC_RANGE			3
#define IDC_SPOTSIZE		4
#define IDC_SPOTFALLOFF		5
#define IDC_DIFFUSE			6
#define IDC_SPECULAR		7
LightEditor::LightEditor(Editor * editor, Actor * item): EditorItem(editor,item)
{
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
LightEditor::~LightEditor()
{

}


//-----------------------------------------------------------------------------
// UI
//-----------------------------------------------------------------------------
void LightEditor::OnEvent( UINT nEvent, int nControlID, CDXUTControl* pControl )
{
	Light * light = (Light*)m_Item;
	switch (nControlID)
	{
	case IDC_LIGHTTYPE:
		{
			CDXUTComboBox * lightCombo = ((CDXUTComboBox*)pControl);
			switch (lightCombo->GetSelectedIndex())
			{
			case 0:
				light->m_Type=LIGHT_OMNI;
				break;
			case 1:
				light->m_Type=LIGHT_SPOT;
				break;
			case 2:
				light->m_Type=LIGHT_DIR;
				break;
			}
		}
		break;
	case IDC_RANGE:
		if (nEvent == EVENT_EDITBOX_CHANGE)
		{
			CDXUTEditBox* rangeEdit = ((CDXUTEditBox*)pControl); 
			light->GetCurrentState().Range=ConvertToFloat(rangeEdit->GetText(),light->GetCurrentState().Range);
		}
		break;
	case IDC_SPOTSIZE:
		if (nEvent == EVENT_EDITBOX_CHANGE)
		{
			CDXUTEditBox* editBox = ((CDXUTEditBox*)pControl); 
			light->GetCurrentState().Spot_Size=ConvertToFloat(editBox->GetText(),light->GetCurrentState().Spot_Size);
		}
		break;
	case IDC_SPOTFALLOFF:
		if (nEvent == EVENT_EDITBOX_CHANGE)
		{
			CDXUTEditBox* editBox = ((CDXUTEditBox*)pControl); 
			light->GetCurrentState().Spot_Falloff=ConvertToFloat(editBox->GetText(),light->GetCurrentState().Spot_Falloff);
		}
		break;
	case IDC_DIFFUSE:
		this->m_Editor->CreateValueDialog(DIALOG_COLOR,this,&light->GetCurrentState().Diffuse);
		break;
	case IDC_SPECULAR:
		this->m_Editor->CreateValueDialog(DIALOG_COLOR,this,&light->GetCurrentState().Specular);
		break;
	}
}

//-----------------------------------------------------------------------------
// UI Widgets for Light Statistics
//-----------------------------------------------------------------------------
void LightEditor::FillPropsDialog(CDXUTDialog * dialog)
{
	Light * light = (Light*)m_Item;

	//CREATE THE PROPERTY GRID
	CDXUTPropertiesList * propGrid;
	dialog->AddPropertiesList(IDC_PROPERTIES,1,1,168,310,0,&propGrid);
	propGrid->SetLabelWidth( 75);

	//PROPERTIES
	propGrid->AddTitle(L"",-1,L"Properties",NULL);

	propGrid->AddComboBox(L"LightType",IDC_LIGHTTYPE,NULL);
	CDXUTComboBox * lightTypeCombo = (CDXUTComboBox*)propGrid->GetControlByID(IDC_LIGHTTYPE) ;
	lightTypeCombo->AddItem(L"Omni",NULL);
	lightTypeCombo->AddItem(L"Spot",NULL);
	lightTypeCombo->AddItem(L"Directional",NULL);
	switch (light->m_Type)
	{
	case LIGHT_OMNI:
		lightTypeCombo->SetSelectedByIndex(0);
		break;
	case LIGHT_SPOT:
		lightTypeCombo->SetSelectedByIndex(1);
		break;
	case LIGHT_DIR:
		lightTypeCombo->SetSelectedByIndex(2);
		break;
	}


	propGrid->AddStatic(L"Dynamic ",IDC_DYNAMIC,light->IsDynamic() ? L"TRUE" :L"FALSE",NULL);
	propGrid->AddItem(L"Range",IDC_RANGE,NULL,ToUnicode( ToStr((int)(light->GetCurrentState().Range))).c_str());
	propGrid->AddItem(L"SpotSize",IDC_SPOTSIZE,NULL,ToUnicode(ToStr((int)(light->GetCurrentState().Spot_Size))).c_str());
	propGrid->AddItem(L"SpotFallOff",IDC_SPOTFALLOFF,NULL, ToUnicode(ToStr((int)(light->GetCurrentState().Spot_Falloff))).c_str());
	propGrid->AddButton(L"Diffuse",IDC_DIFFUSE,NULL, L"Edit");
	propGrid->AddButton(L"Specular",IDC_SPECULAR,NULL, L"Edit");
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void LightEditor::Update(void)
{
	EditorItem::Update();
}

//====================================================================
//--------------------------------------------------------------------
// MATERIAL EDITOR
//--------------------------------------------------------------------
//====================================================================
//#define IDC_EMISSIVE    1
#define IDC_VARS_START  10
MaterialEditor::MaterialEditor(Editor * editor, Material * item): EditorItem(editor,item)
{
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
MaterialEditor::~MaterialEditor()
{

}


//-----------------------------------------------------------------------------
// UI
//-----------------------------------------------------------------------------
void MaterialEditor::OnEvent( UINT nEvent, int nControlID, CDXUTControl* pControl )
{
	Material * material = (Material*)m_Item;
	if (nControlID>=IDC_VARS_START)
	{
		int pIndex=nControlID-IDC_VARS_START;
		ShaderVar * var=material->m_Parameters[pIndex];
		switch (var->type)
		{
		case PARAM_BOOL:
			
			break;
		case PARAM_FLOAT:
			if (nEvent == EVENT_EDITBOX_CHANGE)
		{
			CDXUTEditBox* editBox = ((CDXUTEditBox*)pControl); 
			*(float*)var->data=ConvertToFloat(editBox->GetText(),*(float*)var->data);
		}
			break;

		case PARAM_FLOAT3:
			break;
		case PARAM_FLOAT4:
			this->m_Editor->CreateValueDialog(DIALOG_VECTOR4,this,var->data);
			break;
		}
	}
}

//-----------------------------------------------------------------------------
// UI Widgets for Materials
//-----------------------------------------------------------------------------
void MaterialEditor::FillPropsDialog(CDXUTDialog * dialog)
{
	
	Material * material = (Material*)m_Item;
	//CREATE THE PROPERTY GRID
	CDXUTPropertiesList * propGrid;
	dialog->AddPropertiesList(IDC_PROPERTIES,1,1,168,310,0,&propGrid);
	propGrid->SetLabelWidth( 75);
	//PROPERTIES
	propGrid->AddTitle(L"",-1,L"Properties",NULL);
	propGrid->AddStatic(L"Name",-1, ToUnicode( material->m_Name).c_str(),NULL);
	propGrid->AddStatic(L"AlphaTest ",-1,material->m_AlphaTest ? L"TRUE" :L"FALSE",NULL);
	propGrid->AddStatic(L"Opaque",-1,material->m_Opaque ? L"TRUE" :L"FALSE",NULL);
	//propGrid->AddStatic(L"PRTEnabled",-1,material->m_bPRTEnabled ? L"TRUE" :L"FALSE",NULL);
	//propGrid->AddButton(L"Emissive",IDC_EMISSIVE,NULL, L"Edit");
	//PARAMETERS
	propGrid->AddTitle(L"",-1,L"Parameters",NULL);
	
	for (int i=0;i<material->m_Parameters.size();i++)
	{
		ShaderVar * var=material->m_Parameters[i];
		switch (var->type)
		{
		case PARAM_BOOL:
			propGrid->AddCheckBox(ToUnicode(var->name).c_str(),IDC_VARS_START + i,false,NULL);
			break;
		case PARAM_FLOAT:
			propGrid->AddItem(ToUnicode(var->name).c_str(),IDC_VARS_START+i,NULL,ToUnicode( ToStr((float)*(float*)var->data)).c_str());
			break;
		case PARAM_FLOAT3:
			propGrid->AddButton(ToUnicode(var->name).c_str(),IDC_VARS_START+i,NULL, L"Edit");
			break;
		case PARAM_FLOAT4:
			propGrid->AddButton(ToUnicode(var->name).c_str(),IDC_VARS_START+i,NULL, L"Edit");
			break;
		}
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void MaterialEditor::Update(void)
{
	Material * material = (Material*)m_Item;
	for (int i=0;i<material->m_Parameters.size();i++)
	{
		ShaderVar * var=material->m_Parameters[i];
		material->m_Shader->SetVar(*var,var->data);
	}
	// Don't do this, will cast material to an actor
	//EditorItem::Update();
}


#define IDC_OK	0
#define IDC_CANCEL	1
#define IDC_EDITX 2
#define IDC_EDITY 3
#define IDC_EDITZ 4
#define IDC_EDITW 5

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
EditorDialog::EditorDialog(Editor * editor, EditorItem * item,CDXUTDialog * dialog,void * valueToEdit)
{
	m_Editor= editor;
	m_EditorItem = item;
	m_ValueToEdit = valueToEdit;
	m_Dialog = dialog;

	//Default Buttons
	dialog->AddButton(IDC_OK,L"Ok",90,155,40,20,0,true);
	dialog->AddButton(IDC_CANCEL,L"Cancel",135,155,60,20,0,false);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void EditorDialog::OnEvent( UINT nEvent, int nControlID, CDXUTControl* pControl )
{
	switch (nControlID)
	{
	case IDC_OK:
		this->m_Editor->DialogFinished();
		break;
	case IDC_CANCEL:
		this->m_Editor->DialogFinished();
		break;
	}
}

//-----------------------------------------------------------------------------
// Constructor for vector 2 editor
//-----------------------------------------------------------------------------
Vector2_Editor::Vector2_Editor(Editor * editor, EditorItem * item,CDXUTDialog * dialog,void * valueToEdit) : EditorDialog(editor,item,dialog,valueToEdit)
{
	CDXUTEditBox * edit;
	Vector2 * value=(Vector2*)valueToEdit;

	dialog->AddStatic(IDC_STATIC,L"X :",10,20,40,20,false);
	dialog->AddEditBox(IDC_EDITX,ToUnicode( ToStr((float)value->x)).c_str(),45,20,100,20,false,&edit);
	edit->SetBorderWidth(0);

	dialog->AddStatic(IDC_STATIC,L"Y :",10,45,40,20,false);
	dialog->AddEditBox(IDC_EDITY,ToUnicode( ToStr((float)(value->y))).c_str(),45,45,100,20,false,&edit);
	edit->SetBorderWidth(0);
}

//-----------------------------------------------------------------------------
// Event handler for vector 2
//-----------------------------------------------------------------------------
void Vector2_Editor::OnEvent( UINT nEvent, int nControlID, CDXUTControl* pControl )
{
	
	switch (nControlID)
	{
	case IDC_OK:
		{
		Vector2 * value=(Vector2*)m_ValueToEdit;
		CDXUTEditBox* editBox = ((CDXUTEditBox*) m_Dialog->GetControl(IDC_EDITX)); 
		value->x=ConvertToFloat(editBox->GetText(),value->x);
		editBox = ((CDXUTEditBox*) m_Dialog->GetControl(IDC_EDITY)); 
		value->y=ConvertToFloat(editBox->GetText(),value->y);
		}
		break;
	}
	EditorDialog::OnEvent(nEvent,nControlID,pControl);
}

//-----------------------------------------------------------------------------
// Constructor for FloatColor editor
//-----------------------------------------------------------------------------
FloatColor_Editor::FloatColor_Editor(Editor * editor, EditorItem * item,CDXUTDialog * dialog,void * valueToEdit) : EditorDialog(editor,item,dialog,valueToEdit)
{
	CDXUTEditBox * edit;
	FloatColor * value=(FloatColor*)valueToEdit;
	m_OldValue=*value;

	dialog->AddStatic(IDC_STATIC,L"R :",10,20,40,20,false);
	dialog->AddEditBox(IDC_EDITX,ToUnicode( ToStr((float)value->r)).c_str(),45,20,100,20,false,&edit);
	edit->SetBorderWidth(0);

	dialog->AddStatic(IDC_STATIC,L"G :",10,45,40,20,false);
	dialog->AddEditBox(IDC_EDITY,ToUnicode( ToStr((float)(value->g))).c_str(),45,45,100,20,false,&edit);
	edit->SetBorderWidth(0);

	dialog->AddStatic(IDC_STATIC,L"B :",10,70,40,20,false);
	dialog->AddEditBox(IDC_EDITZ,ToUnicode( ToStr((float)(value->b))).c_str(),45,70,100,20,false,&edit);
	edit->SetBorderWidth(0);

	dialog->AddStatic(IDC_STATIC,L"A :",10,95,40,20,false);
	dialog->AddEditBox(IDC_EDITW,ToUnicode( ToStr((float)(value->a))).c_str(),45,95,100,20,false,&edit);
	edit->SetBorderWidth(0);
}

//-----------------------------------------------------------------------------
// Event handler for FloatColor editor
//-----------------------------------------------------------------------------
void FloatColor_Editor::OnEvent( UINT nEvent, int nControlID, CDXUTControl* pControl )
{
	FloatColor * value=(FloatColor*)m_ValueToEdit;
	CDXUTEditBox* editBox;
	switch (nControlID)
	{
		case IDC_EDITX:
		editBox = ((CDXUTEditBox*)pControl); 
		value->r=ConvertToFloat(editBox->GetText(),value->r);
		break;
		case IDC_EDITY:
		editBox = ((CDXUTEditBox*)pControl); 
		value->g=ConvertToFloat(editBox->GetText(),value->g);
		break;
		case IDC_EDITZ:
		editBox = ((CDXUTEditBox*)pControl); 
		value->b=ConvertToFloat(editBox->GetText(),value->b);
		break;
		case IDC_EDITW:
		editBox = ((CDXUTEditBox*)pControl); 
		value->a=ConvertToFloat(editBox->GetText(),value->a);
		break;
		case IDC_CANCEL:
			*value=m_OldValue;
	}
	EditorDialog::OnEvent(nEvent,nControlID,pControl);
}


//-----------------------------------------------------------------------------
// Constructor for Vector4 editor
//-----------------------------------------------------------------------------
Vector4_Editor::Vector4_Editor(Editor * editor, EditorItem * item,CDXUTDialog * dialog,void * valueToEdit) : EditorDialog(editor,item,dialog,valueToEdit)
{
	dialog->SetCaptionText(L"FLOAT4 Editor");

	CDXUTEditBox * edit;
	Vector4 * value=(Vector4*)valueToEdit;
	m_OldValue=*value;

	dialog->AddStatic(IDC_STATIC,L"X :",10,20,40,20,false);
	dialog->AddEditBox(IDC_EDITX,ToUnicode( ToStr((float)value->x)).c_str(),45,20,100,20,false,&edit);
	edit->SetBorderWidth(0);

	dialog->AddStatic(IDC_STATIC,L"Y :",10,45,40,20,false);
	dialog->AddEditBox(IDC_EDITY,ToUnicode( ToStr((float)(value->y))).c_str(),45,45,100,20,false,&edit);
	edit->SetBorderWidth(0);

	dialog->AddStatic(IDC_STATIC,L"Z :",10,70,40,20,false);
	dialog->AddEditBox(IDC_EDITZ,ToUnicode( ToStr((float)(value->z))).c_str(),45,70,100,20,false,&edit);
	edit->SetBorderWidth(0);

	dialog->AddStatic(IDC_STATIC,L"W :",10,95,40,20,false);
	dialog->AddEditBox(IDC_EDITW,ToUnicode( ToStr((float)(value->w))).c_str(),45,95,100,20,false,&edit);
	edit->SetBorderWidth(0);
}

//-----------------------------------------------------------------------------
// Event handler for Vector4 editor
//-----------------------------------------------------------------------------
void Vector4_Editor::OnEvent( UINT nEvent, int nControlID, CDXUTControl* pControl )
{
	Vector4 * value=(Vector4*)m_ValueToEdit;
	CDXUTEditBox* editBox;
	switch (nControlID)
	{
		case IDC_EDITX:
		editBox = ((CDXUTEditBox*)pControl); 
		value->x=ConvertToFloat(editBox->GetText(),value->x);
		break;
		case IDC_EDITY:
		editBox = ((CDXUTEditBox*)pControl); 
		value->y=ConvertToFloat(editBox->GetText(),value->y);
		break;
		case IDC_EDITZ:
		editBox = ((CDXUTEditBox*)pControl); 
		value->z=ConvertToFloat(editBox->GetText(),value->z);
		break;
		case IDC_EDITW:
		editBox = ((CDXUTEditBox*)pControl); 
		value->w=ConvertToFloat(editBox->GetText(),value->w);
		break;
		case IDC_CANCEL:
			*value=m_OldValue;
	}
	EditorDialog::OnEvent(nEvent,nControlID,pControl);
}


//-----------------------------------------------------------------------------
// Constructor for Vector editor
//-----------------------------------------------------------------------------
Vector_Editor::Vector_Editor(Editor * editor, EditorItem * item,CDXUTDialog * dialog,void * valueToEdit) : EditorDialog(editor,item,dialog,valueToEdit)
{
	CDXUTEditBox * edit;
	Vector * value=(Vector*)valueToEdit;
	m_OldValue=*value;

	dialog->AddStatic(IDC_STATIC,L"X :",10,20,40,20,false);
	dialog->AddEditBox(IDC_EDITX,ToUnicode( ToStr((float)value->x)).c_str(),45,20,100,20,false,&edit);
	edit->SetBorderWidth(0);

	dialog->AddStatic(IDC_STATIC,L"Y :",10,45,40,20,false);
	dialog->AddEditBox(IDC_EDITY,ToUnicode( ToStr((float)(value->y))).c_str(),45,45,100,20,false,&edit);
	edit->SetBorderWidth(0);

	dialog->AddStatic(IDC_STATIC,L"Z :",10,70,40,20,false);
	dialog->AddEditBox(IDC_EDITZ,ToUnicode( ToStr((float)(value->z))).c_str(),45,70,100,20,false,&edit);
	edit->SetBorderWidth(0);
}

//-----------------------------------------------------------------------------
// Event handler for Vector editor
//-----------------------------------------------------------------------------
void Vector_Editor::OnEvent( UINT nEvent, int nControlID, CDXUTControl* pControl )
{
	Vector * value=(Vector*)m_ValueToEdit;
	CDXUTEditBox* editBox;
	switch (nControlID)
	{
		case IDC_EDITX:
		editBox = ((CDXUTEditBox*)pControl); 
		value->x=ConvertToFloat(editBox->GetText(),value->x);
		break;
		case IDC_EDITY:
		editBox = ((CDXUTEditBox*)pControl); 
		value->y=ConvertToFloat(editBox->GetText(),value->y);
		break;
		case IDC_EDITZ:
		editBox = ((CDXUTEditBox*)pControl); 
		value->z=ConvertToFloat(editBox->GetText(),value->z);
		break;
		case IDC_CANCEL:
			*value=m_OldValue;
	}
	EditorDialog::OnEvent(nEvent,nControlID,pControl);
}