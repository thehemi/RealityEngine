//=========== (C) Copyright 2004, Artficial Studios. All rights reserved. =======
/// Editor Item Class, base for any editable item properties dialog
/// Author: Mostafa Mohamed
//===============================================================================
#pragma once

enum EditorDialogType
{
	DIALOG_VECTOR2,
	DIALOG_VECTOR3,
	DIALOG_VECTOR4,
	DIALOG_COLOR,
};

//-----------------------------------------------------------------------------
/// Editor Item Class, base for any editable item properties dialog
//-----------------------------------------------------------------------------
class EditorItem
{
protected:
	class Editor *	m_Editor;
	void *	m_Item;

	/// For moving items
	Vector2	m_PrevPick;
	Matrix  m_StartRot;
	Vector2	m_PrevMouse;
public:

	EditorItem(Editor * editor, void * item);
	~EditorItem(void);
	virtual void FillPropsDialog(CDXUTDialog * dialog);
	virtual void OnEvent( UINT nEvent, int nControlID, CDXUTControl* pControl );
	virtual void Update(void);
};




//-----------------------------------------------------------------------------
/// Mesh-Actor extension for the Editor
//-----------------------------------------------------------------------------
class ActorEditor : public EditorItem
{
public:
	/// Structure containing information about this Actor for the Editor
	struct ActorInfo
	{
		int PolyCount;
		int VerticesCount;
		int NumMeshes;
		int TouchingLights;
		vector<string> Materials;
	};

	ActorEditor(Editor * editor, Actor * item);
	~ActorEditor(void);
	virtual void FillPropsDialog(CDXUTDialog * dialog);
	virtual void OnEvent( UINT nEvent, int nControlID, CDXUTControl* pControl );
	virtual void Update(void);
	void  GetInfo(ModelFrame * frame,ActorInfo * info);
	void  GetInfo(ActorInfo * info);
};

//-----------------------------------------------------------------------------
/// Light-Actor Extension
//-----------------------------------------------------------------------------
class LightEditor : public EditorItem
{
public:
	LightEditor(Editor * editor, Actor * item);
	~LightEditor(void);
	virtual void FillPropsDialog(CDXUTDialog * dialog);
	virtual void OnEvent( UINT nEvent, int nControlID, CDXUTControl* pControl );
	virtual void Update(void);
};

//-----------------------------------------------------------------------------
/// Editor Material Extension
//-----------------------------------------------------------------------------
class MaterialEditor : public EditorItem
{
public:
	MaterialEditor(Editor * editor, Material * item);
	~MaterialEditor(void);
	virtual void FillPropsDialog(CDXUTDialog * dialog);
	virtual void OnEvent( UINT nEvent, int nControlID, CDXUTControl* pControl );
	virtual void Update(void);
};

//-----------------------------------------------------------------------------
/// Editor Dialog Class, base for any custom value editor like colors, vectors etc...
//-----------------------------------------------------------------------------
class EditorDialog
{
protected:
	class Editor *	m_Editor;
	EditorItem *	m_EditorItem;
	void *			m_ValueToEdit;
	CDXUTDialog *	m_Dialog;
public:

	EditorDialog(Editor * editor, EditorItem * item,CDXUTDialog * dialog,void * valueToEdit);
	virtual void OnEvent( UINT nEvent, int nControlID, CDXUTControl* pControl );
	virtual void Update(void){}
};

//-----------------------------------------------------------------------------
/// Editor Dialog Class, For Editor Vector2 Type
//-----------------------------------------------------------------------------
class Vector2_Editor : public EditorDialog
{
public:

	Vector2_Editor(Editor * editor, EditorItem * item,CDXUTDialog * dialog,void * valueToEdit);
	~Vector2_Editor(void);
	virtual void OnEvent( UINT nEvent, int nControlID, CDXUTControl* pControl );
};

//-----------------------------------------------------------------------------
/// Editor Dialog Class, For Editor Vector2 Type
//-----------------------------------------------------------------------------
class FloatColor_Editor : public EditorDialog
{
private:
	FloatColor m_OldValue;
public:
	FloatColor_Editor(Editor * editor, EditorItem * item,CDXUTDialog * dialog,void * valueToEdit);
	~FloatColor_Editor(void);
	virtual void OnEvent( UINT nEvent, int nControlID, CDXUTControl* pControl );
};

//-----------------------------------------------------------------------------
/// Editor Dialog Class, For Editor Vector4 Type
//-----------------------------------------------------------------------------
class Vector4_Editor : public EditorDialog
{
private:
	Vector4 m_OldValue;
public:
	Vector4_Editor(Editor * editor, EditorItem * item,CDXUTDialog * dialog,void * valueToEdit);
	~Vector4_Editor(void);
	virtual void OnEvent( UINT nEvent, int nControlID, CDXUTControl* pControl );
};

//-----------------------------------------------------------------------------
/// Editor Dialog Class, For Editor Vector Type
//-----------------------------------------------------------------------------
class Vector_Editor : public EditorDialog
{
private:
	Vector m_OldValue;
public:
	Vector_Editor(Editor * editor, EditorItem * item,CDXUTDialog * dialog,void * valueToEdit);
	~Vector_Editor(void);
	virtual void OnEvent( UINT nEvent, int nControlID, CDXUTControl* pControl );
};