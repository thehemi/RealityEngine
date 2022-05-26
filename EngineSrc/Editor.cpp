//=========== (C) Copyright 2004, Artficial Studios. All rights reserved. =======
// Editor Class, for in-game editing capabilities
// Author: Mostafa Mohamed, David Sleeper, Tim Johnson
//
//
//===============================================================================
#include "StdAfx.h"
#include ".\editor.h"
#include "Compiler\Compiler.h"
#include <crtdbg.h>
#include <d3dx9.h>
#include "Collision\CollisionRoutines.h"
#include "Serializer.h"
#include "BatchRenderer.h"
#include "classmap.h"
#include "Collision.h"
#include "HDR.h"
#include "Profiler.h"
#include "GUISystem.h"
#include "FXManager.h"

Editor * m_CurEditor = NULL;

typedef void (*FunctionPointer)(int i);

#define MAXUNDOSIZE	50

extern "C"  __declspec(dllexport) int __cdecl GetValue(int i)
{
	//MessageBox(0,"Hello",0,0);
	FunctionPointer f = reinterpret_cast<FunctionPointer>(i);
	f(42);
	return i;
}

ENGINE_API void __stdcall Register(COMPILING_CALLBACK call){
	Editor::Instance()->Callback = call;
	call();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Editor::CombineSelected()
{
    ModelFrame*  combinedFrame = 0;
    Actor*       combinedActor = 0;

    vector<LPD3DXMESH>	meshes;
	vector<Matrix>		matrices;
    int                 totalVerts = 0;

    // Attribute ids so we know how to remap/collapse subattributes
    vector<int>         ids;
    for(int j=0;j<m_SelectedActors.size();j++)
	{
		Actor* actor = m_SelectedActors[j];
		if(!actor || !actor->MyModel)
			return;

		vector<ModelFrame*> frames;
		actor->MyModel->m_pFrameRoot->EnumerateMeshes(frames);

		for(int i=0;i<frames.size();i++)
		{
            if(!frames[i]->GetMesh(0))
                continue;

            // First mesh will be the mesh we combine
            if(!combinedFrame)
            {
                combinedActor = m_SelectedActors[j];
                combinedFrame = frames[i];
                for(int k=0;k<combinedFrame->GetMesh()->m_Materials.size();k++)
                    ids.push_back(k);
            }
            else
            {
                // Add the materials from this mesh to the combined materials
                for(int p=0;p<frames[i]->GetMesh(0)->m_Materials.size();p++)
                {
                    frames[i]->GetMesh(0)->m_Materials[p]->AddRef();
                    
                    // See if material is already on list
                    int found = -1;
                    for(int k=0;k<combinedFrame->GetMesh(0)->m_Materials.size();k++)
                    {
                        if(combinedFrame->GetMesh(0)->m_Materials[k] == frames[i]->GetMesh(0)->m_Materials[p])
                        {
                            found = k;
                            break;
                        }
                    }
                   

                    if(found == -1)
                    {
                        // Not on list, so add
                        combinedFrame->GetMesh(0)->m_Materials.push_back(frames[i]->GetMesh(0)->m_Materials[p]);
                        ids.push_back(combinedFrame->GetMesh(0)->m_Materials.size()-1);
                    }
                    // On list, just store id for attribute buffer
                    else
                         ids.push_back(found);
                }
            }


            // Add, put in inverse source frame space
            meshes.push_back(frames[i]->GetMesh(0)->GetHardwareMesh());
            matrices.push_back(frames[i]->CombinedTransformationMatrix * combinedFrame->CombinedTransformationMatrix.Inverse());
            totalVerts += frames[i]->GetMesh(0)->GetHardwareMesh()->GetNumVertices();
		}
	}

    // Combine all
    LPD3DXMESH pCombinedMesh;
    DXASSERT(D3DXConcatenateMeshes(&meshes[0],meshes.size(),D3DXMESH_MANAGED|(totalVerts>=65535?D3DXMESH_32BIT:0),
					(D3DXMATRIX*)&matrices[0],NULL,NULL,RenderWrap::dev,&pCombinedMesh));

    // HACK: Workaround for DX9.2004 Bug where D3DXConcatenateMeshes() holds lock!
    for(int i=0;i<meshes.size();i++){
        meshes[i]->UnlockVertexBuffer();
        meshes[i]->UnlockIndexBuffer();
    }

    Mesh* combinedMesh = combinedFrame->GetMesh(0);

    int numMats = combinedFrame->GetMesh(0)->m_Materials.size();
    combinedMesh->SetMesh(pCombinedMesh);
    // SetMesh fills in attrib ids with dummy materials for unmapped ids
    // but we're about to optimize and remove the ids, so remove these dummy materials
    combinedMesh->m_Materials.resize(numMats);

    // Update & remap ids
    for(int i=0;i<combinedMesh->m_AttribTable.size();i++)
        combinedMesh->m_AttribTable[i].AttribId = ids[i];
    // Update attribute buffer
    combinedMesh->CalcData(false);
    // Optimize to force redundant attribute groups to be filtered
    combinedMesh->Optimize(false);

    // Update collision mesh with combined mesh
    if(combinedFrame->collisionMesh)
	{
		combinedFrame->collisionMesh->Destroy();
		combinedFrame->collisionMesh->Initialize(combinedMesh);
	}

    for(int i=0;i<m_SelectedActors.size();i++)
    {
        // Unselect everything
        m_SelectedActors[i]->IsSelected = false;
        // Delete merged prefabs
        if(m_SelectedActors[i] != combinedActor)
        {
            SAFE_DELETE(m_SelectedActors[i]);
        }
    }
    m_SelectedActors.clear();

    SelectActor(combinedActor);

    // Center pivot for this new combined mesh
    MeshOpSelected(MeshOps::CenterPivot);

    m_World->RegenerateOcclusionTree();
}


//-----------------------------------------------------------------------------
void Editor::MergeScene(const char* filename)
{
	UnSelectAll();
	m_bSelectionChanged = true;

	// Read in scene to merge
	World scene;
	Serializer load;
	load.LoadWorld(filename,&scene,false);

	// Move all actors from scene to our scene
	for(int i=0;i<scene.m_Actors.size();i++)
	{
		// Update world for actor
		scene.m_Actors[i]->SetWorld(m_World);

        UniqueName(scene.m_Actors[i]);

		// Add to actor array
		m_World->m_Actors.push_back(scene.m_Actors[i]);

		// If light, goes on light array too
		if(scene.m_Actors[i]->IsLight())
			m_World->m_Lights.push_back((Light*)scene.m_Actors[i]);

		// Select actor
		SelectActor(scene.m_Actors[i]);
	}

	FXManager::Instance()->ChangeSystemsWorld(&scene,m_World);

	// Remove from scene, so scene doesn't destroy them when unloaded
	scene.m_Actors.clear();
}


// Move Merged Actors based on Camera Position
void Editor::MergeSceneUsingCam(string filename)
{
	UnSelectAll();
	m_bSelectionChanged = true;

	Camera OldCamera=m_Camera;
	// Read in scene to merge
	World scene;
	Serializer load;
	load.LoadWorld(filename.c_str(),&scene,false);

	vector<Actor *> newActors;
	// Move all actors from scene to our scene
	for(int i=0;i<scene.m_Actors.size();i++)
	{
		newActors.push_back(scene.m_Actors.at(i));

        UniqueName(scene.m_Actors[i]);

		// Update world for actor
		scene.m_Actors[i]->SetWorld(m_World);

		// Add to actor array
		m_World->m_Actors.push_back(scene.m_Actors[i]);

		// If light, goes on light array too
		if(scene.m_Actors[i]->IsLight())
			m_World->m_Lights.push_back((Light*)scene.m_Actors[i]);

		// Select actor
		SelectActor(scene.m_Actors[i]);
	}

	FXManager::Instance()->ChangeSystemsWorld(&scene,m_World);
	
	m_Camera=OldCamera;

	BBox box;

	for(int i = 0; i < newActors.size(); i++)
	{ 
		if(newActors[i]->MyModel)
		{
			newActors[i]->MyModel->SetTransform(Matrix());
			box += newActors[i]->MyModel->GetWorldBBox();
		}
		else
			box += newActors[i]->CollisionBox;
	}

	Vector BoxMid = (box.min + box.max)/2;
	float SpereRad = (float) (BoxMid - box.max).Length();

	float ExtraDistance=2.0f;

	Vector NewPosition = (m_Camera.Location + 
		(m_Camera.Direction*(SpereRad*ExtraDistance)));

	for(int i=0;i<newActors.size();i++)
	{
		newActors.at(i)->Location = NewPosition;
	};

	// Remove from scene, so scene doesn't destroy them when unloaded
	scene.m_Actors.clear();
}


void Editor::MergeSceneUsingPosAndRotMat(string filename, Vector &Position, 
										 Matrix &Rotation)
{
	UnSelectAll();
	m_bSelectionChanged = true;

	Camera OldCamera=m_Camera;
	// Read in scene to merge
	World scene;
	Serializer load;
	load.LoadWorld(filename.c_str(),&scene,false);

	vector<Actor *> newActors;
	// Move all actors from scene to our scene
	for(int i=0;i<scene.m_Actors.size();i++)
	{
		newActors.push_back(scene.m_Actors.at(i));

        UniqueName(scene.m_Actors[i]);

		// Update world for actor
		scene.m_Actors[i]->SetWorld(m_World);

		// Add to actor array
		m_World->m_Actors.push_back(scene.m_Actors[i]);

		// If light, goes on light array too
		if(scene.m_Actors[i]->IsLight())
			m_World->m_Lights.push_back((Light*)scene.m_Actors[i]);

		// Select actor
		SelectActor(scene.m_Actors[i]);
	}

	FXManager::Instance()->ChangeSystemsWorld(&scene,m_World);
	
	m_Camera=OldCamera;

	BBox box;
	Vector Middle;

	for(int i = 0; i < newActors.size(); i++)
	{ 	
		Middle += (newActors[i]->Location / newActors.size());					
	}
		
	for(int i=0;i<newActors.size();i++)
	{
		//recenter and move
		newActors.at(i)->Location=(newActors.at(i)->Location - Middle) + Position;
		newActors.at(i)->Rotation.Orthonormalize();
		newActors.at(i)->Rotation*=Rotation;
	};

	// Remove from scene, so scene doesn't destroy them when unloaded
	scene.m_Actors.clear();
};
//-----------------------------------------------------------------------------
vector<string> Editor::GetAllClasses()
{
	vector<string> classes = Factory::GetClasses();

	// Enumerate the maps
	vector<string> files;
	enumerateFiles("..\\Scripts\\Spawnable\\",files,1,".cs");

	// Fill the listbox
	for(int i=0;i<files.size();i++)
	{
		classes.push_back(files[i].substr(0,files[i].find_last_of(".")));
	}
	return classes;
}

//-----------------------------------------------------------------------------
vector<string> Editor::GetModelFrameNamesFromGroup(char *GroupName)
{
	vector<string> ModelFramesOut;
	int NameLength = strlen(GroupName); 
	if(NameLength < 3 || NameLength > 50) return ModelFramesOut;
	char realName[50];
	memcpy(realName, GroupName+1, NameLength-2);
	//null terminated string
	realName[NameLength-2]='\0';
	string CompareNames(realName); 
	for(int i=0;i<m_ActorLists.size();i++)
	{
		if(m_ActorLists.at(i).ListName == CompareNames)
		{
			vector<ModelFrame*> tempModelFrames;
			for(int j=0;j<m_ActorLists.at(i).m_SelectedActors.size();j++)
			{ 
				m_ActorLists.at(i).m_SelectedActors[j]->MyModel->m_pFrameRoot->
					EnumerateMeshes(tempModelFrames);
			};
			// only one lists matches return here
			for(int j=0;j<tempModelFrames.size();j++)
				ModelFramesOut.push_back(tempModelFrames.at(j)->Name);
			return ModelFramesOut;
		};
	};
	// set here to avoid stupid warnings
	return ModelFramesOut;
};

//-----------------------------------------------------------------------------
void Editor::FixName(Actor* actor)
{
	string newName = actor->m_Name;

	while(m_World->FindActor(newName))
    {
        newName = GenerateNewName(newName);
    }

	actor->m_Name = newName;

    // Ensure unique frame names
    if(actor->MyModel && actor->MyModel->m_pFrameRoot)
        actor->MyModel->m_pFrameRoot->FixNames(m_World);
}

//-----------------------------------------------------------------------------
// Never call m_SelectedActors.clear(), always this
//-----------------------------------------------------------------------------
void Editor::UnSelectAll()
{
	for(int i=0;i<m_SelectedActors.size();i++)
	{
		m_SelectedActors[i]->IsSelected = false;
	}
	m_SelectedActors.clear();
}

//-----------------------------------------------------------------------------
// Not really a delete, because of undo stack. Just removes actors from world
//-----------------------------------------------------------------------------
void Editor::DeleteSelected()
{ 
	m_Gizmo.LostFocus();
	for(int i=0;i<m_SelectedActors.size();i++)
	{
		m_SelectedActors[i]->IsSelected = false;
		m_World->RemoveActor(m_SelectedActors[i]);
		for(int j=0;j<m_ActorLists.size();j++)
		{
			for(int k=0;k<m_ActorLists.at(j).m_SelectedActors.size();k++)
			{
				if(m_ActorLists.at(j).m_SelectedActors.at(k) == m_SelectedActors.at(i))
					m_ActorLists.at(j).m_SelectedActors.erase( m_ActorLists.at(j).m_SelectedActors.begin() + k );
			};
		};
	}
	PushUndo(Undo(m_SelectedActors,Undo::Delete));
	UnSelectAll();
	m_bSelectionChanged = true;
    m_World->RegenerateOcclusionTree();
}

//-----------------------------------------------------------------------------
//system similiar for both
bool Editor::PopUndoRedo(list<Undo> *unrePop, list<Undo> *unrePush)
{		
	if(unrePop->size() == 0)
		return false;

	Undo& u = unrePop->back();

	////////////////////////////////////////////////////////////////////

	//HANDLE DELETIONS
	// If undo and deleted, add actor back to world
	if(u.type == Undo::Delete && u.isUndo)
	{
		UnSelectAll();

		for(int i=0;i<u.objects.size();i++)
		{
			// Add to appropriate world array(s)
			u.objects[i]->SetWorld(m_World);
			m_World->m_Actors.push_back(u.objects[i]);
			if(u.objects[i]->IsLight())
				m_World->m_Lights.push_back((Light*)u.objects[i]);
			SelectActor(u.objects[i]);
		}
	}
	else 
		// If redo and actor was added remove him
		if(u.type == Undo::Delete && !u.isUndo)
		{
			UnSelectAll();

			for(int i=0;i<u.objects.size();i++)
			{
				m_World->RemoveActor(m_SelectedActors[i]);			
			}
		}


		//HANDLE TRANSLATIONS
		// If transform, set stored TM back
		if(u.type == Undo::Transform)
		{
			UnSelectAll();

			vector<Matrix> tms;
			for(int i=0;i<u.objects.size();i++)
			{
				//make sure this pointer is still valid
				if(!IsActor(u.objects[i]))continue;

				//store current for Redo/Undo
				tms.push_back(u.objects[i]->Rotation);
				tms.back()[3] = u.objects[i]->Location;

				u.objects[i]->Location = u.tms[i][3];
				u.tms[i][3] = Vector();
				u.objects[i]->Rotation = u.tms[i];


				SelectActor(u.objects[i]);
			}

			//set the U about to be poped to where it used to be
			//for redo/undo purposes
			u.tms=tms;
		}	

		//HANDLE SELECTIONS	
		//no Redo Support yet
		if(u.type == Undo::Selection)
		{
			//get the currently selected for redo/undo		
			vector<Actor*>	NewSelected=m_SelectedActors;

			//clear selected
			UnSelectAll();

			for(int i=0;i<u.objects.size();i++)
			{
				if(!IsActor(u.objects[i]))continue;

				SelectActor(u.objects[i]);
			};

			//get the currently selected for redo/undo
			u.objects=NewSelected;			
		} 	

		//HANDLE HIDDING AND FREEZING

		//Hiding
		if(u.type == Undo::Hide)
		{
			UnSelectAll();

			//undo to get here it hide something, so lets unhide it and
			//reset u saying to get there it unhide something
			u.type = Undo::UnHide;

			for(int i=0;i<u.objects.size();i++)
			{
				if(!IsActor(u.objects[i]))continue;			
				u.objects[i]->IsHidden=false;
			};
		}
		else if(u.type == Undo::UnHide)
		{
			UnSelectAll();

			u.type = Undo::Hide;

			for(int i=0;i<u.objects.size();i++)
			{
				if(!IsActor(u.objects[i]))continue;			
				u.objects[i]->IsHidden=true;
			};
		}
		//Freezing
		if(u.type == Undo::Freeze)
		{
			UnSelectAll();

			u.type = Undo::UnFreeze;

			for(int i=0;i<u.objects.size();i++)
			{
				if(!IsActor(u.objects[i]))continue;			
				u.objects[i]->IsFrozen=false;
			};
		}
		else if(u.type == Undo::UnFreeze)
		{	
			UnSelectAll();

			u.type = Undo::Freeze;

			for(int i=0;i<u.objects.size();i++)
			{
				if(!IsActor(u.objects[i]))continue;			
				u.objects[i]->IsFrozen=true;
			};
		}

		//
		// Handle Selections
		//
		if(u.type == Undo::ApplySelectionAsset && u.isUndo)
		{
			UnSelectAll();

			vector<ModelFrame*> selectedMeshes;
			//Editor* editor = Editor::Instance();
			for(int i=0;i<u.objects.size();i++)
			{
				if(u.objects[i]->MyModel && u.objects[i]->MyModel->m_pFrameRoot)
					u.objects[i]->MyModel->m_pFrameRoot->EnumerateMeshes(selectedMeshes);
			}

			vector<Material*> matsToUndo;

			for(int i=0;i<selectedMeshes.size();i++)
			{
				Mesh* m =  selectedMeshes[i]->GetMesh();

				int j=0;
				// Apply to submat
				if(u.subMat < m->m_Materials.size())
				{				
					matsToUndo.push_back(m->m_Materials[u.subMat]);

					//since undo release current, since we know it 
					//must have at least one other reference
					m->m_Materials[u.subMat]->Release();

					m->m_Materials[u.subMat] = u.mats[j];
					j++;
				}
			}		

			//set her back for redo
			u.mats=matsToUndo;
		} 
		else
			if(u.type == Undo::ApplySelectionAsset && !u.isUndo)
			{
				UnSelectAll();

				vector<ModelFrame*> selectedMeshes;
				//Editor* editor = Editor::Instance();
				for(int i=0;i<u.objects.size();i++)
				{
					if(u.objects[i]->MyModel && u.objects[i]->MyModel->m_pFrameRoot)
						u.objects[i]->MyModel->m_pFrameRoot->EnumerateMeshes(selectedMeshes);
				}

				vector<Material*> matsToUndo;

				for(int i=0;i<selectedMeshes.size();i++)
				{
					Mesh* m =  selectedMeshes[i]->GetMesh();

					int j=0;
					// Apply to submat
					if(u.subMat < m->m_Materials.size())
					{				
						matsToUndo.push_back(m->m_Materials[u.subMat]);

						//redo adds refs since we are going back and it re-enters
						//undo refs
						u.mats[j]->AddRef();

						m->m_Materials[u.subMat] = u.mats[j];
						j++;
					}
				}		

				//set her back for redo
				u.mats=matsToUndo;
			}

			////////////////////////////////////////////////////////////////////

			//flip since we are pushing it to the opposite
			u.isUndo=!u.isUndo;

			unrePush->push_back(u);
			unrePop->pop_back();

			return true;
};

bool  Editor::PopUndo()
{
	return PopUndoRedo(&m_Undo, &m_Redo);	
}
bool  Editor::PopRedo()
{
	return PopUndoRedo(&m_Redo, &m_Undo);	
}
//-----------------------------------------------------------------------------
void  Editor::PushUndo(Undo& undo)
{
	undo.isUndo=true;

	//if you add a new undo you must clear the redo's
	m_Redo.clear();

	m_Undo.push_back(undo);

	//we are hitting the limit start cleaning up
	if(m_Undo.size()>MAXUNDOSIZE)
	{
		//If it was a delete remove actors permantently
		if(m_Undo.begin()->type == Undo::Delete)
		{
			for(int i=0;i<m_Undo.begin()->objects.size();i++)
			{
                // FIXME: Undos broken!!
				//if(m_Undo.begin()->objects[i])
				//	delete m_Undo.begin()->objects[i];
				m_Undo.begin()->objects[i]=NULL;
			};
		};

		//If it was a ApplySelectionAsset release mats
		if(m_Undo.begin()->type == Undo::ApplySelectionAsset)
		{
			for(int i=0;i<m_Undo.begin()->mats.size();i++)		
				if(m_Undo.begin()->mats[i])m_Undo.begin()->mats[i]->Release();				
		};

		m_Undo.erase(m_Undo.begin());
	};
}

//-----------------------------------------------------------------------------
void Editor::ClearUndos()
{
	while(m_Undo.size())
	{
		// If it was a delete remove actors permantently
		if(m_Undo.begin()->type == Undo::Delete)
		{
			for(int j=0;j<m_Undo.begin()->objects.size();j++)
			{
				if(m_Undo.begin()->objects[j])
					delete m_Undo.begin()->objects[j];

				m_Undo.begin()->objects[j]=NULL;
			};

			// Dave: Might need to Revise this, it is assuming the undo
			// worked correctly, if not mem lose
			// If it was a ApplySelectionAsset release mats
			if(m_Undo.begin()->type == Undo::ApplySelectionAsset)
			{
				for(int i=0;i<m_Undo.begin()->mats.size();i++)			
					if(m_Undo.begin()->mats[i])m_Undo.begin()->mats[i]->Release();				

			};
		}
		m_Undo.erase(m_Undo.begin());
	}
}

void  Editor::PushRedo(Undo& undo)
{
	undo.isUndo=false;
	m_Redo.push_back(undo);
}

//-----------------------------------------------------------------------------
Editor* Editor::Instance () 
{
	static Editor inst;
	return &inst;
}
//-----------------------------------------------------------------------------
void Editor::CloneSelected()
{
	vector<Actor*> actors = m_SelectedActors;
	UnSelectAll();

	for(int i=0;i<actors.size();i++)
	{
		// Create
		Actor* act = Factory::create(actors[i]->ClassName(),m_World);
		// Clone
		actors[i]->Clone(act);
        // Ensure unique frame names
        if(act->MyModel && act->MyModel->m_pFrameRoot)
            act->MyModel->m_pFrameRoot->FixNames(m_World);
		// Select!
		SelectActor(act);
	}

	Beep(1000,100);
	//MessageBox(0,"Object Cloned Successfully.","Cloning Operation",MB_ICONINFORMATION);
}
//-----------------------------------------------------------------------------
bool Editor::IsActor(Actor *actor)
{
	bool found=false;
	for(int i=0;i<m_World->m_Actors.size();i++)
	{
		if(m_World->m_Actors[i]==actor)
		{
			found=true;
			break;
		};
	};

	return found;
};
//-----------------------------------------------------------------------------
bool Editor::IsSelected(Actor* actor)
{
	return actor->IsSelected;
	/*
	bool found = false;
	for(int i=0;i<m_SelectedActors.size();i++)
	{
	if(m_SelectedActors[i] == actor){
	found = true;
	break;
	}
	}
	return found;*/
}
//-----------------------------------------------------------------------------
void Editor::SelectActor(Actor* actor)
{
	if(!IsSelected(actor)){
		m_bSelectionChanged = true;
		m_SelectedActors.push_back(actor);
	}
	actor->IsSelected = true;
}
//-----------------------------------------------------------------------------
void Editor::UnSelectActor(Actor* actor)
{
	actor->IsSelected = false;
	for(int i=0;i<m_SelectedActors.size();i++)
	{
		m_bSelectionChanged = true;
		if(m_SelectedActors[i] == actor)
		{
			m_SelectedActors.erase(m_SelectedActors.begin()+i);
			return;
		}
	}
}

//-----------------------------------------------------------------------------
//USED BY EDITOR FOR UNDO/REDO
//-----------------------------------------------------------------------------

//helper function
vector<Actor*> Editor::HideFreezeHelper(vector<Actor*> actor, bool HideNotFreeze, bool SetTrue)
{
	for(int i=0;i<actor.size();i++)
	{
		//if it isn't an actor remove it and move i back
		if(!IsActor(actor[i]))
		{
			//go ahead and clean this on up so we can set it directly to objects
			actor.erase(actor.begin() + i);
			i--;
			continue;
		};

		if(HideNotFreeze)
			actor[i]->IsHidden=SetTrue;
		else
			actor[i]->IsFrozen=SetTrue;
	};

	return actor;
};

void Editor::HideActors(vector<Actor*> actor)
{	
	PushUndo(Undo(HideFreezeHelper(actor, true, true),Undo::Hide));	
};
//-----------------------------------------------------------------------------
void Editor::FreezeActors(vector<Actor*> actor)
{
	PushUndo(Undo(HideFreezeHelper(actor, false, true),Undo::Freeze));	
};
//-----------------------------------------------------------------------------
void Editor::UnHideActors(vector<Actor*> actor)
{
	PushUndo(Undo(HideFreezeHelper(actor, true, false),Undo::UnHide));	
};
//-----------------------------------------------------------------------------
void Editor::UnFreezeActors(vector<Actor*> actor)
{
	PushUndo(Undo(HideFreezeHelper(actor, false, false),Undo::UnFreeze));		
};
//-----------------------------------------------------------------------------
bool Editor::CompileScene(bool fast, CompilerCallback * callback)
{
	Compiler c; 
    c.m_Callback = callback;
	if(fast){
		c.bFast = true;
		c.percentRays = 33;
		c.percentBounces = 0;
	}
	return c.CompileWorld(m_World);
}
//-----------------------------------------------------------------------------
bool Editor::Compile(vector<Actor*>& actors, bool fast ,  CompilerCallback * callback)
{
	if(actors.size() == 0)
		return false;

	Compiler c; 
    c.m_Callback = callback;
	if(fast){
		c.bFast = true;
		c.percentRays = 33;
		c.percentBounces = 0;
	}
	return c.CompileActors(actors);
}

//-----------------------------------------------------------------------------
bool Editor::SaveSelected(const char* file)
{
    string filename = file;
	if(!m_SelectedActors.size() || !m_SelectedActors[0]->MyModel)
		return false;

	return Serializer::Instance()->Save(filename,m_SelectedActors);
}

void Editor::UniqueName(Actor* node)
{
    FixName(node);
}

string StripFolderPath(string str)
{
    if(str[str.length()-1] == '\\')
        str=str.substr(0,str.length()-1);

    str = str.substr(str.find_last_of("\\")+1);
    return str;
}


//-----------------------------------------------------------------------------
bool Editor::SaveScene(const char* file)
{
    string filename = file;
	Compiler c;
	c.bCompilePRT = false;
	c.CompileWorld(m_World);

	if(filename.length() == 0)
	{
		filename = m_World->m_FileName;
		// Sometimes filename lacks path, if so resolve it here
		if(filename.find("\\") == -1)
			FindMedia(filename,"Maps");
	}
	else
	{ 
		// Saving to new dir, so copy over old Prt files...
		ResetCurrentDirectory();

		// Get new dir
		string newDir = filename.substr(0,filename.find_last_of("\\")+1);

		// Get current (old) dir
		string oldDir = m_World->m_FileName;
		if(oldDir.find("\\") == -1)
			FindMedia(oldDir,"Maps");
        if(FileExists(oldDir))
        {
		    oldDir =  oldDir.substr(0,oldDir.find_last_of("\\")+1);
            // If saving to new dir, copy files over...
            if(filename.length() && oldDir.length())
            {
		        // Copy PRT files to new dir
		        vector<string> prtFiles;
		        enumerateFiles(oldDir.c_str(),prtFiles,2,"*.prt");
		        for(int i=0;i<prtFiles.size();i++)
		        {
			        string newFile = newDir + StripPath(prtFiles[i]);
			        string oldFile = oldDir + StripPath(prtFiles[i]);
			        CopyFile(oldFile.c_str(),newFile.c_str(),FALSE);
		        }

                // Get ini names
                string oldName = oldDir+StripFolderPath(oldDir)+".ini";
                string newName = newDir+StripFolderPath(newDir)+".ini";
                CopyFile(oldName.c_str(),newName.c_str(),FALSE);
            }
        }
	}

	return Serializer::Instance()->SaveWorld(filename,m_World,false);
}
//-----------------------------------------------------------------------------
Prefab* Editor::AddPrefab(std::string& filename)
{
	Prefab* node = new Prefab(m_World);
	node->MyModel = new Model;
	node->MyModel->Load(filename.c_str(),true);
	node->MyModel->bExportable = true;

	if(!node->MyModel->m_pFrameRoot)
	{
		delete node;
		SeriousWarning("Prefab load failed for %s (root frame is null)",filename.c_str());
		return 0;
	}
	UniqueName(node);


	// Copy node data!
	node->Rotation		= node->MyModel->m_pFrameRoot->CombinedTransformationMatrix.GetRotationMatrix();
	node->Location		= node->MyModel->m_pFrameRoot->CombinedTransformationMatrix[3];

	// Always remove root offset
	node->MyModel->m_pFrameRoot->CombinedTransformationMatrix = node->MyModel->m_pFrameRoot->TransformationMatrix = Matrix();

	// Remove the sub offsets only if it's a single mesh, otherwise we want to retain offsets
	//vector<ModelFrame*> frames;
	//node->MyModel->m_pFrameRoot->EnumerateMeshes(frames);
	//if(frames.size() == 1)
	//{
	//node->MyModel->RemoveSceneOffset();
	//}

	// Prefabs do not have their TMs updated automatically, so update once now
	node->MyModel->SetTransform(node->Rotation,node->Location);

	UnSelectAll();
	SelectActor(node);
	return node;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
Vector2 getMousePos()
{
	POINT p;
	RECT clientR;
	GetCursorPos(&p);
	HWND hWnd = Engine::Instance()->hWnd;
	GetClientRect(hWnd, &clientR);
	ClientToScreen(hWnd, (POINT *)&clientR.left);
	ClientToScreen(hWnd, (POINT *)&clientR.right);
	p.x-=clientR.left;
	p.y-=clientR.top;
	return Vector2(p.x,p.y);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
Editor::Editor(void)
{
	m_bWireFrameMode = false;
	m_bMouseDragging= false;
	m_CamSpeed		= 50.f;
	m_DrawBox		= true;
	m_EditorMode	= false;
	m_InputLocked	= false;
	m_bSelectionChanged = true;
	m_Input			= NULL;
	m_SphereMesh	= NULL;
	m_ConeMesh		= NULL;
	m_bCompiling	= false;
	m_Camera.Location.y = 5;
	m_World			= NULL;
	m_bInPopupMenu	= false;
	m_CurrentView   = PERSPECTIVE_VIEW;
	m_Zooming       = 1;
	m_ScreenHeight  = 0;

	//Grid Settings STG	
	m_bSnapToGrid=false;	
	m_SnapToGridValue=1.0f;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
Editor::~Editor(void)
{
	SAFE_RELEASE(m_ConeMesh);
	SAFE_RELEASE(m_SphereMesh);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Editor::Intialize()
{
	// CREATE MESHES NEEDED FOR LIGHTS
	RenderDevice * renderSys=Engine::Instance()->RenderSys;
	m_RenderDevice=renderSys;
	D3DXCreateCylinder(RenderWrap::dev,0.0f,0.2f,0.5f, 10,10,&m_ConeMesh,NULL);
	D3DXCreateSphere(RenderWrap::dev,0.2f,10,10,&m_SphereMesh,NULL);

	// INTIALIZE KEYS
	m_Input			= Input::Instance();
	MOVEFORWARD		= m_Input->GetControlHandle("WALK_FORWARDS");
	MOVEBACKWARD	= m_Input->GetControlHandle("WALK_BACKWARDS");
	STRAFELEFT		= m_Input->GetControlHandle("STRAFE_LEFT");
	STRAFERIGHT		= m_Input->GetControlHandle("STRAFE_RIGHT");
	FREELOOK		= m_Input->GetControlHandle("FREE_LOOK");
	MOUSEDOWN		= m_Input->GetControlHandle("GUIClick");
	MULTISELECT		= m_Input->GetControlHandle("MultiSelect");
	CLONEDRAG		= m_Input->GetControlHandle("CloneDrag");

	ZOOMIN          = m_Input->GetControlHandle("ZoomIn");
	ZOOMOUT         = m_Input->GetControlHandle("ZoomOut");

	m_ScriptTex.Load("Editor_Script.dds");
}

//-----------------------------------------------------------------------------
// Perform a mesh operation
//-----------------------------------------------------------------------------
void Editor::MeshOpSelected(MeshOps::Op op)
{
	for(int j=0;j<m_SelectedActors.size();j++)
	{
		Actor* actor = m_SelectedActors[j];
		if(!actor || !actor->MyModel)
			return;

		vector<ModelFrame*> frames;
		actor->MyModel->m_pFrameRoot->EnumerateMeshes(frames);

		for(int i=0;i<frames.size();i++)
		{
            Matrix tm;
			MeshOps::Convert(frames[i]->GetMesh(0),op,tm);
            if(op == MeshOps::CenterPivot && i == 0)
            {
                m_SelectedActors[j]->Location += tm.m3;
            }

			// Regenerate LODs by setting mesh again
			frames[i]->SetMesh(frames[i]->GetMesh(0));

			if(frames[i]->collisionMesh)
			{
				frames[i]->collisionMesh->Destroy();
				frames[i]->collisionMesh->Initialize(frames[i]->GetMesh(0));
			}
		}
	}
}


//-----------------------------------------------------------------------------
// Helper for ray picking meshes
//-----------------------------------------------------------------------------
Vector Editor::Unproject(Vector screenPoint)
{
	D3DXMATRIX worldMat,proj,view;
	D3DXMatrixIdentity(&worldMat);
	view=*(D3DXMATRIX*)&m_Camera.view;
	proj=*(D3DXMATRIX*)&m_Camera.projection;

	D3DVIEWPORT9 viewport;
	RenderWrap::dev->GetViewport(&viewport);

	Vector vec;
	D3DXVec3Unproject((D3DXVECTOR3*)&vec, (D3DXVECTOR3*)&screenPoint,&viewport, &proj,&view, &worldMat);
	return vec;
}

#define OVERLAPS(x0,y0,x1,y1,x2,y2,x3,y3) (	(!(   ((x0)<(x2) && (x1)<(x2))|| ((x0)>(x3) && (x1)>(x3)) || ((y0)<(y2) && (y1)<(y2)) || ((y0)>(y3) &&(y1)>(y3))   ))	)

//-----------------------------------------------------------------------------
// See if the user has selected a scene object
//-----------------------------------------------------------------------------
bool Editor::CheckDragBox(BBox& worldBox)
{
	// Find 2D min/max projections
	Vector min = m_Gizmo.To2D(worldBox.min);
	Vector max = m_Gizmo.To2D(worldBox.max);

	// Sort into min/max
	Vector vMin, vMax;
	vMin.x = smallest(max.x,min.x);
	vMin.y = smallest(max.y,min.y);

	vMax.x = largest(min.x,max.x);
	vMax.y = largest(min.y,max.y);

	// Min/Max for dragbox
	Vector dMin, dMax;
	dMin.x = smallest(m_MouseDownPos.x,m_MousePosition.x);
	dMin.y = smallest(m_MouseDownPos.y,m_MousePosition.y);

	dMax.x = largest(m_MouseDownPos.x,m_MousePosition.x);
	dMax.y = largest(m_MouseDownPos.y,m_MousePosition.y);

	// Compare the two drag box
	return OVERLAPS(vMin.x,vMin.y,vMax.x,vMax.y,dMin.x,dMin.y,dMax.x,dMax.y);

}
//-----------------------------------------------------------------------------
// See if the user has selected a scene object
//-----------------------------------------------------------------------------
void Editor::HandleSelections()
{
	if(m_ViewMode==EDITOR_ORBIT_VIEW) return;


	m_MousePosition = getMousePos();

	// Store mouse down pos for later
	if (m_Input->ControlJustPressed(MOUSEDOWN) && m_MouseInBounds)
	{
		m_MouseDownPos = getMousePos();
		m_bMouseDragging = true;
	}

	if (m_bMouseDragging && m_Input->ControlJustReleased(MOUSEDOWN))
	{
		m_bMouseDragging = false;
		Vector start1   = Unproject(Vector(m_MouseDownPos.x,m_MouseDownPos.y,0));
		Vector start2   = Unproject(Vector(m_MouseDownPos.x,m_MouseDownPos.y,1));

		Vector end1   = Unproject(Vector(m_MousePosition.x,m_MousePosition.y,0));
		Vector end2   = Unproject(Vector(m_MousePosition.x,m_MousePosition.y,1));

		// Get selected list
		vector<Actor*> selected;

		// Dragbox selection if mouse moved more than 5 pixels
		if((m_MouseDownPos-m_MousePosition).Length() > 5)
		{
			for(int i=0;i<m_World->m_Actors.size();i++)
			{
				if(m_World->m_Actors[i]->IsHidden || m_World->m_Actors[i]->IsFrozen)
					continue; // Can't drag-select hidden or frozen meshs

				// Check model actors
				if(m_World->m_Actors[i]->MyModel){
					if(CheckDragBox(m_World->m_Actors[i]->MyModel->GetWorldBBox()))
						selected.push_back(m_World->m_Actors[i]);
				}
				// Check scripts and lights
				else if(!m_World->m_Actors[i]->MyModel){
					Vector pos = m_World->m_Actors[i]->Location;
					BBox box = BBox(Vector(pos.x-0.5f,pos.y-0.5f,pos.z-0.5f),Vector(pos.x+0.5f,pos.y+0.5f,pos.z+0.5f));
					if(CheckDragBox(box))
						selected.push_back(m_World->m_Actors[i]);
				}
			}
		}

		// Build selection list. Not necessary when you can only click one point at a time, designed for when we support
		// dragbox selection
		Actor * touched = GetUnderMouse(m_World,end1, end2);
		if(touched)
			if(!touched->IsFrozen && !touched->IsHidden)
				selected.push_back(touched);

		bool bMultiSelect = m_Input->ControlDown(MULTISELECT);
		// If multi-selecting, don't reset current selection
		if(!bMultiSelect && m_SelectedActors.size())
		{
			//Selection changes 
			PushUndo(Undo(m_SelectedActors,Undo::Selection));
			m_bSelectionChanged = true;
			UnSelectAll();
		}

		for(int i=0;i<selected.size();i++){
			// If multi-selecting, clicking an object again unselects it
			if(bMultiSelect && IsSelected(selected[i]))
				UnSelectActor(selected[i]);
			else
				SelectActor(selected[i]);
		}


	}
}

//-----------------------------------------------------------------------------
// Is cursor in dialog? Used to avoid clicking world objects when dialog is in front
//-----------------------------------------------------------------------------
bool Editor::CursorInWindow(CGUIWindow* wnd)
{
	if(!wnd->GetVisible())
		return false;
	CDXUTDialog* dlg = wnd->m_Dialog;

	int width  = dlg->GetWidth();
	int height = dlg->GetHeight();

	if(dlg->IsMinimized())
		height = dlg->GetCaptionHeight();

	if (m_MousePosition.x < dlg->GetX() || 
		m_MousePosition.x > dlg->GetX() + width || 
		m_MousePosition.y < dlg->GetY() || 
		m_MousePosition.y > dlg->GetY() + height)
		return false;
	return true;
}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Editor::Update(World * world)
{
	if (!GetEditorMode())
		return;

	StartMiniTimer();

	// was mouse down last time in
	bool static wasMouseDown = false;

	m_World = world;
	Vector2 mDelta = m_MousePosition - getMousePos();
	m_MousePosition = getMousePos();
	// Is mouse in window?
	RECT r;
	HWND hWnd = Engine::Instance()->hWnd;
	GetClientRect(hWnd, &r);
	m_MouseInBounds = true;
	if (m_MousePosition.x < 0 || m_MousePosition.x > r.right || m_MousePosition.y < 0 || m_MousePosition.y > r.bottom)
		m_MouseInBounds = false;

	if (!m_InputLocked)
	{
		//Do Camera Movement
		UpdateCamera();

		// Check for an object selection
		if(!m_Gizmo.HasFocus())
			HandleSelections();

		// (If out of bounds, do not trigger mouse down)
		bool mouseDown = m_Input->ControlDown(MOUSEDOWN) && (m_MouseInBounds || m_bDragging);

        if (wasMouseDown && !mouseDown)
            world->RegenerateOcclusionTree();
		//
		// Update Gizmo
		//
		Matrix object;

		static vector<Vector> ActorsLocOnMD;		
		if(!mouseDown || !wasMouseDown)	ActorsLocOnMD.clear();		


		// Object pos will be average of all positions
		for(int i=0;i<m_SelectedActors.size();i++){
			object[3] += (m_SelectedActors[i]->Location / m_SelectedActors.size());

			//store the originals for grided controls
			if(mouseDown && !wasMouseDown)
				ActorsLocOnMD.push_back(m_SelectedActors[i]->Location);

		}

		Vector startLoc = object[3];	

		Vector2 mouse = m_MousePosition;
		if(!m_CursorVisible)
			mouse = Vector2(0,0);

		// Hacky: Set object size for scaling purposes
		BBox b;
		if(m_SelectedActors.size() && m_SelectedActors[0]->MyModel)
			b = m_SelectedActors[0]->MyModel->GetWorldBBox();
		m_Gizmo.fObjectSize = (b.max - b.min).Length()/2;

		// Run gizmo to get movement
		m_Gizmo.OnUpdate(mouseDown,mouse,m_Camera,object);

		// Extract Deltas
		Vector deltaLoc = m_Gizmo.GetLocation()-startLoc;
		Matrix deltaRot = m_Gizmo.GetRotation();
		deltaRot[3] = Vector();

		// Get The overall mouse movement for Grid's
		static Vector mouseMoveLock;

		if(wasMouseDown && mouseDown) mouseMoveLock += deltaLoc;
		else mouseMoveLock = deltaLoc;	

		// Anything inside here happens at the start of a drag
		if(!m_bDragging && (mDelta.x||mDelta.y) && mouseDown)
		{
			m_bDragging = true;
			// Just gone into drag mode, add undo
			PushUndo(Undo(m_SelectedActors,Undo::Transform));

			// Clone-dragging support!!
			if(m_Gizmo.m_Mode != Gizmo::MODE_SCALE) // No scale-cloning, too often happens accidentally
			{
				if(m_MouseInBounds && m_Input->ControlDown(CLONEDRAG))
				{
					CloneSelected();
				}
			}
		}

		// Update actors pos/rot/scale if moved/modified
		for(int i=0;i<m_SelectedActors.size();i++){
			Actor* actor = m_SelectedActors[i];

			//
			// Extract new location & see if actor location has changed
			//			
			bool bModified = false;

			if(mouseDown && m_Gizmo.m_Axis!=0){
				if(m_Gizmo.m_Mode == Gizmo::MODE_TRANSLATE){

					if(m_bSnapToGrid && m_SnapToGridValue > 0.00001f) //snap to grid					
					{
						if(fabs(mouseMoveLock.x)>(m_SnapToGridValue/2.0f))
						{
							bModified = true;
							int Divisor=(ActorsLocOnMD[i].x + mouseMoveLock.x)/m_SnapToGridValue;
							actor->Location.x = Divisor * m_SnapToGridValue;
						};
						if(fabs(mouseMoveLock.y)>(m_SnapToGridValue/2.0f))
						{
							bModified = true;							
							int Divisor=(ActorsLocOnMD[i].y + mouseMoveLock.y)/m_SnapToGridValue;
							actor->Location.y = Divisor * m_SnapToGridValue;
						};
						if(fabs(mouseMoveLock.z)>(m_SnapToGridValue/2.0f))
						{
							bModified = true;							
							int Divisor=(ActorsLocOnMD[i].z + mouseMoveLock.z)/m_SnapToGridValue;
							actor->Location.z = Divisor * m_SnapToGridValue;
						};	
					}
					else
					{
						bModified = deltaLoc.Length() > 0.01f;
						actor->Location += deltaLoc;
					};
				}
				else if(m_Gizmo.m_Mode == Gizmo::MODE_ROTATE){
					bModified = !deltaRot.IsIdentity();

					// Multiple objects pivot around group center. Single objects pivot around themselves
					if(m_SelectedActors.size() > 1)
					{
						Vector RotCenter=startLoc;								

						Matrix RotCenterMat;							
						RotCenterMat.SetTranslations(RotCenter.x,RotCenter.y,RotCenter.z);
						Matrix ActorCenterMat;

						ActorCenterMat.SetTranslations(actor->Location.x,actor->Location.y,actor->Location.z);

						Matrix mActor = actor->Rotation;
						mActor[3] = actor->Location;							

						mActor = mActor * (RotCenterMat.Inverse() * deltaRot * RotCenterMat);

						actor->Rotation = mActor.GetRotationMatrix();
						actor->Location = mActor[3];
					}
					else
					{
						actor->Rotation *= deltaRot;
					}					

				}
				else if(m_Gizmo.m_Mode == Gizmo::MODE_SCALE){
					bModified = !deltaRot.IsIdentity();

					Vector NewLocation = actor->Location - startLoc;					

					NewLocation = deltaRot * NewLocation;
					actor->Location = NewLocation + startLoc;
					actor->Rotation *= deltaRot;
				}
			}

			// Mouse has been modified, update actor node data
			if(actor->MyModel && bModified){
				vector<ModelFrame*> meshes;
				actor->MyModel->m_pFrameRoot->EnumerateMeshes(meshes);
				for(int i=0;i<meshes.size();i++)
					GetSystemTime(&meshes[i]->GetMesh(0)->m_TimeMoved);
			}
		}

		wasMouseDown = mouseDown;

		if(!mouseDown)
			m_bDragging = false;
	}

	Profiler::Get()->EditorMS += StopMiniTimer();

	// Tick the engine
	Engine::Instance()->Update(&m_Camera);
}

//-----------------------------------------------------------------------------
// Camera movement logic
//-----------------------------------------------------------------------------
void Editor::UpdateCamera()
{
	if (m_CurrentView) // IS Ortho?
	{
		m_Camera.bUpdateProjection = true;
		if (m_bHasFocus && (m_Input->ControlDown(FREELOOK) || m_Input->ControlJustPressed(ZOOMOUT) || m_Input->ControlJustPressed(ZOOMIN)))
		{
			Engine::Instance()->RenderSys->ShowCursor(false,false);
			m_CursorVisible=false;
			bool zoomed=false;

			if (m_Input->ControlJustPressed(ZOOMOUT))
			{
				m_Zooming *= 1.1f;
				zoomed=true;
			}

			if (m_Input->ControlJustPressed(ZOOMIN))
			{
				m_Zooming *= 0.9f;
				zoomed=true;
			}

			if (zoomed)
			{
				float aspRatio =(float) RenderDevice::Instance()->GetViewportX()/ RenderDevice::Instance()->GetViewportY();
				aspRatio *= m_Zooming;
				D3DXMATRIX matProj;
				D3DXMatrixOrthoLH(&matProj,m_ScreenHeight* aspRatio,m_ScreenHeight * m_Zooming ,1,10000);
				m_Camera.projection= *(Matrix*)&matProj;
				m_Camera.bUpdateProjection = false;
			}

			switch (m_CurrentView)
			{
			case LEFT_VIEW:
				if (m_Input->ControlDown(MOVEFORWARD))
					m_Camera.Location.y+= m_CamSpeed * GDeltaTime;
				if (m_Input->ControlDown(MOVEBACKWARD))
					m_Camera.Location.y-= m_CamSpeed * GDeltaTime;
				if (m_Input->ControlDown(STRAFERIGHT))
					m_Camera.Location.z-= m_CamSpeed  * GDeltaTime;
				if (m_Input->ControlDown(STRAFELEFT))
					m_Camera.Location.z+= m_CamSpeed  * GDeltaTime;
				break;
			case RIGHT_VIEW:
				if (m_Input->ControlDown(MOVEFORWARD))
					m_Camera.Location.y+= m_CamSpeed * GDeltaTime;
				if (m_Input->ControlDown(MOVEBACKWARD))
					m_Camera.Location.y-= m_CamSpeed * GDeltaTime;
				if (m_Input->ControlDown(STRAFERIGHT))
					m_Camera.Location.z+= m_CamSpeed  * GDeltaTime;
				if (m_Input->ControlDown(STRAFELEFT))
					m_Camera.Location.z-= m_CamSpeed  * GDeltaTime;
				break;
			case FRONT_VIEW:
				if (m_Input->ControlDown(MOVEFORWARD))
					m_Camera.Location.y+= m_CamSpeed * GDeltaTime;
				if (m_Input->ControlDown(MOVEBACKWARD))
					m_Camera.Location.y-= m_CamSpeed * GDeltaTime;
				if (m_Input->ControlDown(STRAFERIGHT))
					m_Camera.Location.x+= m_CamSpeed  * GDeltaTime;
				if (m_Input->ControlDown(STRAFELEFT))
					m_Camera.Location.x-= m_CamSpeed  * GDeltaTime;
				break;
			case BACK_VIEW:
				if (m_Input->ControlDown(MOVEFORWARD))
					m_Camera.Location.y+= m_CamSpeed * GDeltaTime;
				if (m_Input->ControlDown(MOVEBACKWARD))
					m_Camera.Location.y-= m_CamSpeed * GDeltaTime;
				if (m_Input->ControlDown(STRAFERIGHT))
					m_Camera.Location.x-= m_CamSpeed  * GDeltaTime;
				if (m_Input->ControlDown(STRAFELEFT))
					m_Camera.Location.x+= m_CamSpeed  * GDeltaTime;
				break;
			case TOP_VIEW:
				if (m_Input->ControlDown(MOVEFORWARD))
					m_Camera.Location.z+= m_CamSpeed * GDeltaTime;
				if (m_Input->ControlDown(MOVEBACKWARD))
					m_Camera.Location.z-= m_CamSpeed * GDeltaTime;
				if (m_Input->ControlDown(STRAFERIGHT))
					m_Camera.Location.x+= m_CamSpeed  * GDeltaTime;
				if (m_Input->ControlDown(STRAFELEFT))
					m_Camera.Location.x-= m_CamSpeed  * GDeltaTime;
				break;
			case BOTTOM_VIEW:
				if (m_Input->ControlDown(MOVEFORWARD))
					m_Camera.Location.z+= m_CamSpeed * GDeltaTime;
				if (m_Input->ControlDown(MOVEBACKWARD))
					m_Camera.Location.z-= m_CamSpeed * GDeltaTime;
				if (m_Input->ControlDown(STRAFERIGHT))
					m_Camera.Location.x-= m_CamSpeed  * GDeltaTime;
				if (m_Input->ControlDown(STRAFELEFT))
					m_Camera.Location.x+= m_CamSpeed  * GDeltaTime;
				break;
			}  
		}
		else
		{
			Engine::Instance()->RenderSys->ShowCursor(true,false);
			m_CursorVisible=true;
		}
		if (m_CurrentView == TOP_VIEW || m_CurrentView==BOTTOM_VIEW)
			D3DXMatrixLookAtLH((D3DXMATRIX*)& m_Camera.view,(D3DXVECTOR3*)&(m_Camera.Location),(D3DXVECTOR3*)&(m_Camera.Location + m_Camera.Direction),(D3DXVECTOR3*)&Vector(0,0,1));
		else 
			D3DXMatrixLookAtLH((D3DXMATRIX*)& m_Camera.view,(D3DXVECTOR3*)&(m_Camera.Location),(D3DXVECTOR3*)&(m_Camera.Location + m_Camera.Direction),(D3DXVECTOR3*)&Vector(0,1,0));
		m_Camera.CreateClipPlanes();
	}
	else
	{
		Matrix rotation=Matrix::LookTowards(m_Camera.Direction);
		static bool ViewLastUpdate=false;

		if(m_ViewMode==EDITOR_ORBIT_VIEW && (m_SelectedActors.empty() ||  !m_SelectedActors[0]->MyModel))
			m_ViewMode = 0;	

		if(m_Input->ControlJustPressed(FREELOOK) && m_ViewMode != EDITOR_ORBIT_VIEW && m_bHasFocus)
		{
			// Calculate yaw/pitch from current cam
			m_Input->mouseYaw   = RAD2DEG(atan2(m_Camera.Direction .z,-m_Camera.Direction .x)) - 90.0;
			m_Input->mousePitch = -(RAD2DEG(m_Camera.Direction.RadAngle(Vector(0,-1,0))) - 90);
		}


		if (m_bHasFocus && m_ViewMode==EDITOR_ORBIT_VIEW)
		{
			static float LastYaw=0;
			static float LastPitch=0;

			if(!ViewLastUpdate)
			{
				LastYaw=RAD2DEG(atan2(m_Camera.Direction .z,-m_Camera.Direction .x)) - 90.0;
				LastPitch=-(RAD2DEG(m_Camera.Direction.RadAngle(Vector(0,-1,0))) - 90);
			}
			else if(m_Input->ControlDown(FREELOOK))
			{
				LastYaw+=m_Input->mouseYaw;
				LastPitch+=m_Input->mousePitch;
			}

			LastYaw=fmod(LastYaw,360);
			LastPitch=fmod(LastPitch,360);

			m_Camera.Direction =Vector::MakeDirection(LastYaw,LastPitch,0);

			BBox box;

			for(int i = 0; i < m_SelectedActors.size(); i++)
			{ 
				if(m_SelectedActors[i]->MyModel)
					box += m_SelectedActors[i]->MyModel->GetWorldBBox();
			}

			Vector BoxMid = (box.min + box.max)/2;
			float SpereRad = (float) (BoxMid - box.max).Length();
			float Distance = (float) (m_Camera.Location - BoxMid).Length();

			if (m_Input->ControlDown(MOVEFORWARD)) Distance-= (SpereRad*3.5f) * GDeltaTime;
			if (m_Input->ControlDown(MOVEBACKWARD)) Distance+= (SpereRad*3.5f) * GDeltaTime;

			if (m_Input->ControlJustPressed(ZOOMIN)) Distance-= (SpereRad*0.35f);
			if (m_Input->ControlJustPressed(ZOOMOUT)) Distance+= (SpereRad*0.35f);

			if(Distance<.5*SpereRad)Distance=.5*SpereRad;

			Vector NewPosition = (BoxMid + (-m_Camera.Direction*(Distance)));

			m_Camera.Location = NewPosition;

			m_Input->mouseYaw=0;
			m_Input->mousePitch=0;

			ViewLastUpdate=true;
		} else 
			if (m_Input->ControlDown(FREELOOK) && m_bHasFocus)
			{
				if (m_Input->ControlDown(MOVEFORWARD))
					m_Camera.Location+= m_CamSpeed * m_Camera.Direction * GDeltaTime;
				if (m_Input->ControlDown(MOVEBACKWARD))
					m_Camera.Location-= m_CamSpeed * m_Camera.Direction * GDeltaTime;
				if (m_Input->ControlDown(STRAFERIGHT))
					m_Camera.Location+= m_CamSpeed * rotation.GetRight() * GDeltaTime;
				if (m_Input->ControlDown(STRAFELEFT))
					m_Camera.Location-= m_CamSpeed * rotation.GetRight() * GDeltaTime;

				Engine::Instance()->RenderSys->ShowCursor(false,true);
				m_CursorVisible=false;
				m_Camera.Direction =Vector::MakeDirection(m_Input->mouseYaw,m_Input->mousePitch,0);
			}
			else
			{
				ViewLastUpdate=false;
				Engine::Instance()->RenderSys->ShowCursor(true,false);
				m_CursorVisible=true;
			}

			m_Camera.Update();
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Editor::Render(World * world)
{
	if (!GetEditorMode())
		return;

	//Get the canvas
	Canvas* canvas = RenderDevice::Instance()->GetCanvas();
	canvas->SetScaling(1,1);

	//------------------------------
	//Render the World
	// Render in wireframe?
	if(m_bWireFrameMode)
	{
		// Clear in case no sky, etc
		// Also fixes a bug some ppl get with inproper clearing in wireframe (Why?)
		RenderDevice::Instance()->EndHDR();
		RenderWrap::dev->Clear( 0L, NULL, D3DCLEAR_ZBUFFER|D3DCLEAR_TARGET/*|D3DCLEAR_STENCIL*/,RenderDevice::Instance()->GetClearColor(), 1.0f, 0L );

		// Setup the states for a nice alpha-blended wireframe
		Matrix Identity;
		Identity.Identity();
		RenderWrap::SetWorld(Identity);
		RenderWrap::dev->SetPixelShader(0);
		RenderWrap::dev->SetVertexShader(0);
		RenderWrap::SetRS(D3DRS_LIGHTING,TRUE);
		RenderWrap::SetRS(D3DRS_FOGENABLE,FALSE);
		RenderWrap::SetRS(D3DRS_FILLMODE,D3DFILL_WIREFRAME);
		RenderWrap::SetRS(D3DRS_ANTIALIASEDLINEENABLE,FALSE);
		RenderWrap::SetRS(D3DRS_ALPHABLENDENABLE,FALSE);
		RenderWrap::SetRS(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA );
		RenderWrap::SetRS(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA );

		RenderWrap::dev->SetTextureStageState(0,D3DTSS_COLORARG1,D3DTA_DIFFUSE);
		RenderWrap::dev->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_SELECTARG1);
		RenderWrap::dev->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_DISABLE);

		RenderWrap::SetView(m_Camera.view);
		RenderWrap::SetProjection(m_Camera.projection);

		// Setup wireframe colors
		D3DMATERIAL9  selected, normal;
		ZeroMemory(&selected, sizeof(D3DMATERIAL9) );
		ZeroMemory(&normal, sizeof(D3DMATERIAL9) );	
		selected.Emissive.r =  selected.Diffuse.r = 1;
		selected.Emissive.g =  selected.Diffuse.g = 0;
		selected.Emissive.b =  selected.Diffuse.b = 0;
		selected.Emissive.a =  selected.Diffuse.a = 1;	

		normal.Emissive.r =  normal.Diffuse.r = 0.7f;
		normal.Emissive.g =  normal.Diffuse.g = 0.7f;
		normal.Emissive.b =  normal.Diffuse.b = 0.7f;
		normal.Emissive.a =  normal.Diffuse.a = 1;	

		for(int i=0;i<world->m_Actors.size();i++)
		{
			if(IsSelected(world->m_Actors[i]))
				RenderWrap::SetMaterial(&selected);
			else
				RenderWrap::SetMaterial(&normal);
			if(world->m_Actors[i]->MyModel && !world->m_Actors[i]->IsHidden)
				DrawModel(world->m_Actors[i]->MyModel,&m_Camera);	
		}
	}
	// Render normal
	else
	{
		world->Render(&m_Camera);
		RenderDevice::Instance()->EndHDR();
			// Set default states
	RenderDevice::Instance()->ResetAllStates();
	RenderWrap::SetView(m_Camera.view);
	RenderWrap::SetProjection(m_Camera.projection);
	RenderWrap::SetWorld(Matrix());
	RenderWrap::SetRS( D3DRS_ZWRITEENABLE, FALSE );
	RenderWrap::SetRS( D3DRS_ALPHABLENDENABLE,TRUE );
	RenderWrap::SetRS( D3DRS_LIGHTING,FALSE );
	RenderWrap::SetRS( D3DRS_FOGENABLE,FALSE );
	RenderWrap::SetSS( 0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
	RenderWrap::SetSS( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	RenderWrap::SetSS( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	RenderWrap::SetTSS(0,D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	RenderWrap::SetTSS(0,D3DTSS_ALPHAARG1,D3DTA_TEXTURE );
	RenderWrap::SetTSS(0,D3DTSS_ALPHAARG2,D3DTA_DIFFUSE );
	RenderWrap::SetTSS( 0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	RenderWrap::SetTSS( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	RenderWrap::SetTSS( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		world->PostRender(&m_Camera);
	}

	StartMiniTimer();


	//Capture the current device states
	LPDIRECT3DSTATEBLOCK9 states;
	RenderWrap::dev->CreateStateBlock(D3DSBT_ALL,&states);

    RenderDevice::Instance()->ResetAllStates();
    RenderWrap::SetView(m_Camera.view);
	RenderWrap::SetProjection(m_Camera.projection);

	//Render Lights
	RenderWrap::SetRS(D3DRS_LIGHTING,TRUE);
	RenderWrap::SetRS(D3DRS_FOGENABLE,FALSE);
	for (int i=0;i<world->m_Lights.size();i++)
	{
		if(!world->m_Lights[i]->IsHidden)
			DrawLight(world->m_Lights[i]);
	}

	// Render Scripts
	for (int i=0;i<world->m_Actors.size();i++)
		if(world->m_Actors[i]->script.filename.length() && world->m_Actors[i]->MyModel == 0 && !world->m_Actors[i]->IsLight())
			DrawScript(world->m_Actors[i]);

	//Draw the selected Actor Bounding Box
	RenderWrap::SetRS(D3DRS_LIGHTING,FALSE);
	Matrix Identity;
	RenderWrap::SetWorld(Identity);

	RenderWrap::SetRS(D3DRS_FILLMODE,D3DFILL_WIREFRAME);
	RenderWrap::SetRS(D3DRS_ANTIALIASEDLINEENABLE,FALSE);

	// Render box/wireframe for all selected meshes
	for(int i=0;i<m_SelectedActors.size();i++)
	{
		// Is box in view?
		//Vector objDir = (m_Camera.Location - m_SelectedActors[i]->Location).Normalized();
		//float deg = objDir.Dot(m_Camera.Direction.Normalized());

		if(m_SelectedActors[i]->MyModel)
		{
			if (m_DrawBox)
			{
				BBox box = m_SelectedActors[i]->MyModel->GetWorldBBox();
				//BBox box2 = BBox(m_SelectedActors[i]->MyModel->m_RootTransform.Inverse()*m_SelectedActors[i]->MyModel->GetWorldBBox().min,m_SelectedActors[i]->MyModel->m_RootTransform.Inverse()*m_SelectedActors[i]->MyModel->GetWorldBBox().max);
				// Grow box slightly so it doesn't alias against mesh
				box.max += Vector(0.1f,0.1f,0.1f);
				box.min -= Vector(0.1f,0.1f,0.1f);
				canvas->CubeWireframe(box,m_Gizmo.GetScreenMatrix(),0xFF00FF00);
			}
			else
			{
				//
				// Setup the states for a nice alpha-blended wireframe
				//

				RenderWrap::SetRS(D3DRS_ALPHABLENDENABLE,TRUE);
				RenderWrap::SetRS(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA );
				RenderWrap::SetRS(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA );

				RenderWrap::dev->SetTextureStageState(0,D3DTSS_COLORARG1,D3DTA_DIFFUSE);
				RenderWrap::dev->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_SELECTARG1);
				RenderWrap::dev->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_SELECTARG1);
				RenderWrap::dev->SetTextureStageState(0,D3DTSS_ALPHAARG1,D3DTA_TFACTOR);
				RenderWrap::SetRS(D3DRS_TEXTUREFACTOR, COLOR_ARGB(30,0,255,0));

				DrawModel(m_SelectedActors[i]->MyModel,&m_Camera);	
			}

		}
		else if (m_SelectedActors[i]->IsLight())
		{
			RenderWrap::SetRS(D3DRS_ALPHABLENDENABLE,FALSE);
			Vector pos=((Light*)m_SelectedActors[i])->GetCurrentState().Position;
			float size = 0.25f;
			canvas->CubeWireframe(BBox(Vector(pos.x-size,pos.y-size,pos.z-size),Vector(pos.x+size,pos.y+size,pos.z+size)),m_Gizmo.GetScreenMatrix(),0xff00ff00);

			//
			// Draw radius
			//
			// TODO: Support cone, etc
			//if(((Light*)m_SelectedActors[i])->m_Type == LIGHT_OMNI)
			{
				COLOR color = 0xFFFFFFFF;
				float radius = ((Light*)m_SelectedActors[i])->GetCurrentState().Range;
				// computed points
				vector<LVertex> X,Y,Z;
				int segsHalf = 16;
				X.push_back(LVertex(pos+Vector(0,radius,0),color));
				Y.push_back(LVertex(pos+Vector(radius,0,0),color));
				Z.push_back(LVertex(pos+Vector(radius,0,0),color));

				for (int i = 1; i < segsHalf*2; i++)
				{
					Vector x=pos,y=pos,z=pos;
					// Ring for each axis
					// Z
					z.x += (float)(radius * cosf(i * D3DX_PI / segsHalf));
					z.y += (float)(radius * sinf(i * D3DX_PI / segsHalf));
					// Y
					y.x += (float)(radius * cosf(i * D3DX_PI / segsHalf));
					y.z += (float)(radius * sinf(i * D3DX_PI / segsHalf));
					// X
					x.y += (float)(radius * cosf(i * D3DX_PI / segsHalf));
					x.z += (float)(radius * sinf(i * D3DX_PI / segsHalf));

					X.push_back(LVertex(x,color));
					Y.push_back(LVertex(y,color));
					Z.push_back(LVertex(z,color));
				}

				X.push_back(LVertex(pos+Vector(0,radius,0),color));
				Y.push_back(LVertex(pos+Vector(radius,0,0),color));
				Z.push_back(LVertex(pos+Vector(radius,0,0),color));

				Canvas::Instance()->DrawLines(X.size(),(LVertex*)&X[0],sizeof(LVertex),true);
				Canvas::Instance()->DrawLines(X.size(),(LVertex*)&Y[0],sizeof(LVertex),true);
				Canvas::Instance()->DrawLines(X.size(),(LVertex*)&Z[0],sizeof(LVertex),true);
			}
		}
	}

	RenderWrap::SetRS(D3DRS_FILLMODE,D3DFILL_SOLID);


	// Dragbox if dragging and if mouse moved more than 5 pixels
	if(!m_Gizmo.HasFocus() && m_bMouseDragging && (m_MouseDownPos-m_MousePosition).Length() > 5)
	{
		LPD3DXLINE line = Canvas::Instance()->m_Line;
		line->SetAntialias(true);
		line->SetWidth(1.0f);
		DXASSERT(line->Begin());
		D3DXVECTOR2 vLine[5];
		vLine[0] = D3DXVECTOR2(m_MouseDownPos.x,m_MouseDownPos.y);
		vLine[1] = D3DXVECTOR2(m_MouseDownPos.x,m_MousePosition.y);
		vLine[2] = D3DXVECTOR2(m_MousePosition.x,m_MousePosition.y);
		vLine[3] = D3DXVECTOR2(m_MousePosition.x,m_MouseDownPos.y);
		vLine[4] = D3DXVECTOR2(m_MouseDownPos.x,m_MouseDownPos.y);
		line->Draw(vLine,5,0xffffffff);
		line->End();
	}

	if(m_SelectedActors.size())
		m_Gizmo.OnRender();

	m_Gizmo.DrawAxisIcon(m_Camera);

	if(GUISystem::Instance()->m_Strip->GetVisible())
		GUISystem::Instance()->m_Strip->m_Dialog->OnRender(GDeltaTime);

	//Restore the captured states
	states->Apply();
	SAFE_RELEASE(states);

	Profiler::Get()->EditorMS += StopMiniTimer();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Editor::DrawFrame(ModelFrame* pFrame){
	if(!pFrame)
		return;

	DrawFrame(pFrame->pFrameFirstChild);
	DrawFrame(pFrame->pFrameSibling);

    Mesh* mesh = pFrame->GetMesh(0);
	if(!mesh)
		return;


	// Reset draw calls for frame here
	mesh->m_DrawCalls = 0;

	RenderWrap::SetWorld(pFrame->CombinedTransformationMatrix);
    LPDIRECT3DVERTEXDECLARATION9 oldDecl = mesh->m_pDeclaration;
    mesh->m_pDeclaration = VertexFormats::Instance()->FindFormat(sizeof(SimpleVertex))->decl;

	for (int i=0;i<mesh->m_AttribTable.size();i++)
		mesh->DrawSubset(mesh->m_AttribTable[i]);

    mesh->m_pDeclaration = oldDecl;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Editor::DrawScript(Actor* actor)
{
	RenderWrap::SetWorld(Matrix());

	Canvas* c = RenderDevice::Instance()->GetCanvas();
	RenderWrap::SetRS(D3DRS_ALPHABLENDENABLE,TRUE);
	RenderWrap::SetRS(D3DRS_LIGHTING,TRUE);
	RenderWrap::dev->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_SELECTARG1);
	RenderWrap::dev->SetTextureStageState(0,D3DTSS_COLORARG1,D3DTA_TEXTURE );
	RenderWrap::dev->SetTextureStageState(0,D3DTSS_COLORARG2,D3DTA_DIFFUSE);
	RenderWrap::dev->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_SELECTARG1);
	RenderWrap::dev->SetTextureStageState(0,D3DTSS_ALPHAARG1,D3DTA_TEXTURE);

	D3DMATERIAL9  mtl;
	ZeroMemory(&mtl, sizeof(D3DMATERIAL9) );
	mtl.Diffuse = D3DXCOLOR(1,0,0,1);
	RenderWrap::SetMaterial(&mtl);

	if(IsSelected(actor))
		c->BillBoard(actor->Location,0.25f,0xFF00FF00,&m_ScriptTex,BLEND_SRCALPHA,BLEND_INVSRCALPHA);
	else
		c->BillBoard(actor->Location,0.25f,0xFFFFFFFF,&m_ScriptTex,BLEND_SRCALPHA,BLEND_INVSRCALPHA);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Editor::DrawLight(Light * light)
{
	LightState curState=light->GetCurrentState();
	RenderWrap::dev->SetVertexShader(NULL);
	RenderWrap::dev->SetPixelShader(NULL);

	RenderWrap::SetRS(D3DRS_ALPHABLENDENABLE,FALSE);

	RenderWrap::dev->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_SELECTARG1);
	RenderWrap::dev->SetTextureStageState(0,D3DTSS_COLORARG1,D3DTA_DIFFUSE);
	RenderWrap::dev->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_DISABLE);

	D3DMATERIAL9  mtl;
	ZeroMemory(&mtl, sizeof(D3DMATERIAL9) );		
	mtl.Emissive.r =  curState.Diffuse.r;
	mtl.Emissive.g =  curState.Diffuse.g;
	mtl.Emissive.b =  curState.Diffuse.b;
	mtl.Emissive.a =1;	
	mtl.Diffuse = D3DXCOLOR(curState.Diffuse.r,curState.Diffuse.g,curState.Diffuse.b,1);
	RenderWrap::SetMaterial(&mtl);

	Matrix transformation;  
	transformation = light->Rotation;  
	transformation[3] = curState.Position;  
	RenderWrap::SetWorld(transformation);  

	if (light->m_Type==LIGHT_SPOT|| light->m_Type==LIGHT_OMNI_PROJ)  
		m_ConeMesh->DrawSubset(0);  
	else if (light->m_Type==LIGHT_OMNI)  
		m_SphereMesh->DrawSubset(0);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Editor::DrawModel(Model * model,Camera* camera){
	BBox worldBox = model->GetWorldBBox();
	if(!camera->BoxInFrustum(worldBox))
		return;
	DrawFrame( model->m_pFrameRoot);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
Actor* Editor::GetUnderMouse(World * world,Vector& start,Vector& end)
{
	float a = 99999999, b = 99999999, c = 99999999;

	Actor* actor = CheckActors(world,start, end,a);
	Actor* light = CheckLights(world,start, end);
	Actor* script = CheckScripts(world,start, end);

	// Which is nearer?
	if(light)
		b = (m_Camera.Location - light->Location).Length()-0.5f; // Fudge lights so easier to select
	if(script)
		c =  (m_Camera.Location - script->Location).Length()-0.5f; // Fudge scripts so easier to select

	if(a < b && a < c)
		return actor;
	if(b < a && b < c)
		return light;
	if(c < a && c < b)
		return script;

	return NULL;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
int ray_intersectBBox1(const Vector& min, const Vector& max,  const Vector& ro,const Vector& rd,float& tnear,float& tfar)
{
	float t1,t2,t;
	int ret=-1;

	tnear=-BIG_NUMBER;
	tfar=BIG_NUMBER;

	int a,b;
	for( a=0;a<3;a++ )
	{
		if (rd[a]>-KINDA_SMALL_NUMBER && rd[a]<KINDA_SMALL_NUMBER)
			if (ro[a]<min[a] || ro[a]>max[a])
				return -1;
			else ;
		else 
		{
			t1=(min[a]-ro[a])/rd[a];
			t2=(max[a]-ro[a])/rd[a];
			if (t1>t2)
			{ 
				t=t1; t1=t2; t2=t; 
				b=3+a;
			}
			else
				b=a;
			if (t1>tnear)
			{
				tnear=t1;
				ret=b;
			}
			if (t2<tfar)
				tfar=t2;
			if (tnear>tfar || tfar<KINDA_SMALL_NUMBER)
				return -1;
		}
	}

	if (tnear>tfar || tfar<KINDA_SMALL_NUMBER)
		return -1;

	return ret;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
Actor * Editor::CheckScripts(World * world,Vector & start,Vector & end)
{
	for (int i=0;i<world->m_Actors.size();i++)
	{
		if(world->m_Actors[i]->script.filename.length() && world->m_Actors[i]->MyModel == 0)
		{
			Vector pos=world->m_Actors[i]->Location;
			float size = 2;
			if (CheckBox(BBox(Vector(pos.x-size,pos.y-size,pos.z-size),Vector(pos.x+size,pos.y+size,pos.z+size)),start,end))
				return world->m_Actors[i];
		}
	}
	return NULL;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
Actor * Editor::CheckLights(World * world,Vector & start,Vector & end)
{
	for (int i=0;i<world->m_Lights.size();i++)
	{
		Vector pos=world->m_Lights[i]->GetCurrentState().Position;
		if (CheckBox(BBox(Vector(pos.x-0.5f,pos.y-0.5f,pos.z-0.5f),Vector(pos.x+0.5f,pos.y+0.5f,pos.z+0.5f)),start,end))
			return world->m_Lights[i];
	}
	return NULL;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
vector<Actor*> LastSelectedActors;
float LastSelectedDistance = 0;
float LastSelectedActorTime = 0;
Actor * Editor::CheckActors(World * world,Vector & start,Vector & end, float& distance)
{
	if(GSeconds - LastSelectedActorTime > 2.5f)
			LastSelectedActors.resize(0);
	CollisionInfo result;
	world->CollisionCheckRay(0,start,end,CHECK_VISIBLE_ACTORS_UNFROZEN,result,true,&LastSelectedActors);
	if(result.touched)
	{
		distance = result.actualDistance;
		if(LastSelectedDistance > distance)
			LastSelectedActors.resize(0);

		LastSelectedDistance = distance;
		LastSelectedActors.push_back(result.touched);
		LastSelectedActorTime = GSeconds;
	}
	else
	{
		LastSelectedActors.resize(0);
		LastSelectedDistance = 0;
	}
	return result.touched;
}

//-----------------------------------------------------------------------------
// Check collision between a bounding box and a ray
//-----------------------------------------------------------------------------
bool Editor::CheckBox(BBox box,Vector & start,Vector & end)
{
	float dirLength = (end - start).Length();
	Vector dir = (end - start).Normalized();

	float fNear,fFar;
	int fi = ray_intersectBBox1(box.min,box.max,start,dir,fNear,fFar);
	if(fi != -1 && fNear > 0 && fNear < dirLength)
		return true;
	return false;
}


//-----------------------------------------------------------------------------
void Editor::GrabAngles(float &x, float &y, float &z)
{	
	if(m_SelectedActors.size() > 0)
		m_SelectedActors.at(0)->Rotation.GetRotations(x, y, z);
};


//-----------------------------------------------------------------------------
vector<string> Editor::GetActorsFromGroup(char *GroupName)
{
	vector<string> ActorNamesOut;

	int NameLength = strlen(GroupName);


	if(NameLength < 3 || NameLength > 50) return ActorNamesOut;

	char realName[50];
	memcpy(realName, GroupName+1, NameLength-2);
	//null terminated string
	realName[NameLength-2]='\0';

	string CompareNames(realName); 

	for(int i=0;i<m_ActorLists.size();i++)
	{
		if(m_ActorLists.at(i).ListName == CompareNames)
		{
			for(int j=0;j<m_ActorLists.at(i).m_SelectedActors.size();j++)
			{
				ActorNamesOut.push_back(
					m_ActorLists.at(i).m_SelectedActors.at(j)->m_Name.c_str());				
			};
			// only one lists matches return here
			return ActorNamesOut;
		};
	};
	// set here to avoid stupid warnings
	return ActorNamesOut;
};
