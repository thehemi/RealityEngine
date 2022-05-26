//=========== (C) Copyright 2004, Artficial Studios. All rights reserved. =======
// Editor Class, for in-game editing capabilities
// Author: Mostafa Mohamed, David Sleeper, Tim Johnson
//
//
//===============================================================================
#ifndef EDITOR_INCLUDED
#define EDITOR_INCLUDED
#pragma once
#include "EditorGizmo.h"

//current view modes
#define	EDITOR_ORBIT_VIEW	1

class CompilerCallback;

enum Camera_Views
{
    PERSPECTIVE_VIEW,	
    LEFT_VIEW,        
    RIGHT_VIEW,       
    TOP_VIEW,         
    BOTTOM_VIEW,       
    FRONT_VIEW,        
    BACK_VIEW,       
};

Vector2 getMousePos();

typedef void (*COMPILING_CALLBACK)();

class ENGINE_API ActorSelectedList
{
public:
	string			ListName;
	vector<Actor*>	m_SelectedActors;

	ActorSelectedList(int x)
	{
		ListName.append("SelectedList_");
		char number[10];
		sprintf(number, "%d",x);
		ListName.append(number);
	};
};

/// \brief Editor Class, for in-game editing capilities
/// Also serves as backend for Reality Builder
class ENGINE_API Editor : public RenderBase
{
	friend class EditorItem;
	friend class ActorEditor;
	friend class LightEditor;
	friend class MaterialEditor;
	friend class EditorDialog;

	//Variables
protected:
	/// Editor floating cam speed
	float			m_CamSpeed;
	/// Quick handle to input
	Input*			m_Input;
	/// Current cursor position
	Vector2			m_MousePosition;
	/// Handles used for input
	int				MOVEFORWARD,MOVEBACKWARD,STRAFELEFT,STRAFERIGHT,SETCAMERA,
						SHOWEDITOR,FREELOOK,MOUSEDOWN, MULTISELECT,CLONEDRAG,ZOOMOUT,ZOOMIN;
	/// Cursor. Hidden when moving view around
	bool			m_CursorVisible;

	/// Cone used for spotlight
	LPD3DXMESH		m_ConeMesh;
	/// Sphere used for omni
	LPD3DXMESH		m_SphereMesh;
	/// Texture used for scripts
	Texture			m_ScriptTex;

public:
	/// For orgagonal, zooming in?
    float           m_Zooming;
    float           m_ScreenHeight;
    Camera_Views    m_CurrentView;
	Camera			m_Camera;
	Gizmo			m_Gizmo;
	World*			m_World;
	RenderDevice*   m_RenderDevice;
	bool			m_EditorMode;
	bool			m_DrawBox;
	bool			m_InputLocked;
	bool			m_bDragging;
	bool			m_bSelectionChanged;
	bool			m_MouseInBounds;
	bool			m_bMouseDragging;
	bool			m_bHasFocus;
	int				m_ViewMode;
	bool			m_bSnapToGrid;	
	bool			m_bInPopupMenu;
	bool			m_bWireFrameMode;

	float			m_SnapToGridValue;
	
	// Temp functiont to get the angles for the first actor selected
	void GrabAngles(float &x, float &y, float &z);

	/// Force camera to re-create projection on window resize
	virtual HRESULT OnResetDevice(){ m_Camera.bUpdateProjection = true; return S_OK; };

protected:

	/// Selection System
	Vector2		m_MouseDownPos;
	void		HandleSelections();

	/// Drawing
	void DrawModel(Model * model,Camera* camera);
	void DrawFrame(ModelFrame* pFrame);
	void DrawLight(Light * light);
	void DrawScript(Actor* actor);

	/// Maths & Hit Checks
	bool	CheckDragBox(BBox& worldBox);
	bool	CursorInWindow(class CGUIWindow* wnd);
	Vector	Unproject(Vector screenPoint);

	Actor* GetUnderMouse(World * world,Vector& start,Vector& end);
	Actor* CheckLights(World * world,Vector & start,Vector & end);
	Actor* CheckActors(World * world,Vector & start,Vector & end, float& distance);
	Actor* CheckScripts(World * world,Vector & start,Vector & end);
	bool CheckBox(BBox box,Vector & start,Vector & end);

		
public:

	//-----------------------------------------------------------------------------
	/// Undo engine
	//-----------------------------------------------------------------------------
	struct Undo
	{
		enum UndoType
		{
			Transform,
			Delete,
			Selection,
			Hide,
			UnHide,
			Freeze,
			UnFreeze,
			ApplySelectionAsset,
		};

		UndoType	type;

		bool		isUndo;

		/// Data depending on undo type
		vector<Matrix> tms;
		/// Actor who has undo op
		vector<Actor*> objects;
		/// Undo Settings for Materials
		vector<Material*> mats;
		/// SubMaterial Numbers
		int	subMat;

		/// Constructor
		Undo(vector<Actor*>& objects, UndoType t){ 
			this->objects = objects; 
			type = t;
			/// Store TM data if TM undo 
			if(type == Transform){
				for(int i=0;i<objects.size();i++){
					tms.push_back(objects[i]->Rotation);
					tms.back()[3] = objects[i]->Location;
				}
			}
		}

		/// Constructor for Materials
		Undo(vector<Actor*>& objects, vector<Material*> &matsIn, 
			int subMatIn, UndoType t){ 
			this->objects = objects; 
			type = t;
			
			// no need for translation info, since we not a mat

			mats=matsIn;
			subMat=subMatIn;
		}
	};

	// no need for an array, linked lists will work better here for from
	// side deletions when hits max site
	list<Undo>	m_Undo;
	//
	list<Undo>	m_Redo;
	/// 'Undo' something, and remove it from undo stack
	bool	PopUndo();
	/// Add an Undo op
	void PushUndo(Undo& undo);

	bool  PopRedo();

	void  PushRedo(Undo& undo);


	bool Editor::PopUndoRedo(list<Undo> *unrePop, list<Undo> *unrePush);
		

		
	/// Called by compiler
	bool m_bCompiling;
	/// Called by compiler
	string curObject, curMsg;
	/// Set a compiling message/state. Compiler calls this and editor exe checks it
	void SetCompiling(bool compiling, string curObject="", string curMsg=""){ m_bCompiling=compiling; this->curObject=curObject; this->curMsg = curMsg; }

	Editor(void);
	~Editor(void);
	/// Initialize editor system
	void Intialize();
	/// Update all editor/world logic
	void Update(World * world);
	/// Update editor floating camera
	void UpdateCamera();
	/// All editor rendering, including world, which editor is responsible for
	void Render(World * world);
	/// Camera speed
	void SetCamSpeed(float speed){ m_CamSpeed = speed; }

	/// Editor running, and editor is in editing mode as opposed to Play mode?
	bool GetEditorMode(){ return m_EditorMode; }
	/// Set the editor state
	void SetEditorMode(bool mode){ m_EditorMode = mode; m_CursorVisible	= mode; }

	static Editor* Instance();

	//
	/// Standalone Editor Hooks
	//
	void DeleteSelected();
	/// Merge another scene or multi-mesh into scene
	void	MergeScene(const char* file);		

	void	MergeSceneUsingCam(string filename);

	//
	void	MergeSceneUsingPosAndRotMat(string filename, Vector &Position, Matrix &Rotation);
	/// Add prefab to scene
	Prefab* AddPrefab(std::string& filename);		
	/// Save scene
	bool SaveScene(const char* file="");	
	/// Save m_SelectedActors
	bool SaveSelected(const char* file="");	
	/// Compiles entire scene, date-checking to avoid recompiling where not necessary
	bool CompileScene(bool fast,  CompilerCallback * callback);
	/// *Forces* recompile of selected
	bool Compile(vector<Actor*>& actors, bool fast,  CompilerCallback * callback);
	/// Checks to see if if an Actor is valid actor of the world
	bool IsActor(Actor *actor);
	///
	void SelectActor(Actor* actor);
	///
	void UnSelectActor(Actor* actor);
	///

	///
	private: vector<Actor*> HideFreezeHelper(vector<Actor*> actor, bool HideNotFreeze, 
				 bool SetTrue);
	public:
	///
	void HideActors(vector<Actor*>);
	///
	void UnHideActors(vector<Actor*>);
	///
	void FreezeActors(vector<Actor*>);
	///
	void UnFreezeActors(vector<Actor*>);
	/// Actor selected? If we do this too often it could be a bool in actor for speed
	bool IsSelected(Actor* actor);
	/// Always use this instead of m_SelectedActors.clear()
	void UnSelectAll();
	/// Clones any sort of actor
	void CloneSelected();
	/// Generates a scene-unique name for an actor
	void FixName(Actor* actor);
	/// Actor selection changed, so we know to update stats, etc
	bool SelectionChanged(){ bool ret = m_bSelectionChanged; m_bSelectionChanged = false; return ret; }
	void SetFocus(bool f){ m_bHasFocus = f; }
	/// Output in editor log window. Engine writes to this
	vector<string>  m_Log; 
	/// Currently selected actors in editor, used extremely frequently
	vector<Actor*>	m_SelectedActors;
	/// Lists control for groups of selections
	vector<ActorSelectedList>	m_ActorLists;
	/// Unused, to be implemented to allow GUI to refresh during compiling
	COMPILING_CALLBACK Callback;
	/// Clears editor log
	void ClearLog(){ m_Log.clear(); }
	///
    std::vector<std::string> GetAllClasses();
	///Helper Functions for Returning Actors names by a Group Name with {name}
	vector<string> GetActorsFromGroup(char *GroupName); //{name}
	/// Helper Functions for Returning Model Frames by a Group Name with {name}
	vector<string> GetModelFrameNamesFromGroup(char *GroupName); //{name}
	/// Mesh Operation On Selected
	void MeshOpSelected(MeshOps::Op op);
	/// Clear undos (delete held resources etc)
	void ClearUndos();
    /// Combine prefabs
    void CombineSelected();
    /// Give an actor a unique scene name
    void UniqueName(Actor* node);
};

ENGINE_API void __stdcall Register(COMPILING_CALLBACK call);


#endif