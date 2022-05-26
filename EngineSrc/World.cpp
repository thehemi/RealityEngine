//=========== (C) Copyright 2003, Tim Johnson. All rights reserved. ===========
// Game world. Encapsulates a full game world.
// Contains the world's actor list and geometry information
//
// TODO: Move render tree to own file
//
// Optimization TODO: Shadow frustums should not add to view-render batches, only shadow-render
//
//=============================================================================
#include "stdafx.h"
#include "Serializer.h"
#include "BatchRenderer.h"
#include "Collision.h"
#include "Collision\CollisionRoutines.h"
#include "ispatialpartition.h"
#include "Compiler\Compiler.h"
#include "ScriptSystem.h"
#include "Editor.h"
#include "ScriptEngine.h"
#include "SkyController.h"
#include "PostProcess.h"
#include "Profiler.h"
#include "OcclusionCulling.h"
#include "HDR.h"
#include "FXManager.h"
#include "WaterSurface.h"
#include "ShadowMapping.h"

World::RegisterWorld World::m_RegisterWorld = 0;
World::Calllback_AddImpulse World::m_AddTouchedImpulse = NULL;

//-----------------------------------------------------------------------------
// Finds an actor from a name
//-----------------------------------------------------------------------------
Actor* World::FindActor(string& name)
{
	for(int i=0;i<m_Actors.size();i++)
		if(m_Actors[i]->m_Name == name)
			return m_Actors[i];
	return NULL;
}


//-----------------------------------------------------------------------------
// Finds an actor from its GUID
//-----------------------------------------------------------------------------
Actor* World::FindActor(GUID guid)
{
	for(int i=0;i<m_Actors.size();i++)
		if(m_Actors[i]->m_GUID == guid)
			return m_Actors[i];
	return NULL;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void World::EnumerateMeshes(vector<ModelFrame*>& meshFrames)
{
	for(int i=0;i<m_Actors.size();i++){
		if(m_Actors[i]->MyModel){
			m_Actors[i]->MyModel->m_pFrameRoot->EnumerateMeshes(meshFrames);
		}
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
ModelFrame*	World::FindMeshFrame(string name, ModelFrame* ignore)
{
	for(int i=0;i<m_Actors.size();i++){
		if(m_Actors[i]->MyModel){
			ModelFrame* frame = m_Actors[i]->MyModel->GetFrame(m_Actors[i]->MyModel->GetNodeHandle(name,ignore));
			if(frame && frame->GetMesh())
				return frame;
		}
	}
	return NULL;
}


//-----------------------------------------------------------------------------
// Removes dynamic lights each frame before they are added again
//-----------------------------------------------------------------------------
void RenderTree::FlushDynamicLights(){
	for(int i=0;i<data.size();i++){
		data[i]->dynamicLights.clear();
	}

	for(int i=0;i<numChildren;i++){
		children[i]->FlushDynamicLights();
	}
}

//-----------------------------------------------------------------------------
// Removes a static light that has probably become dynamic
// TODO: Examine why some lights get on this list twice, thus we must do j--
// Seemed to happen for flash. Related to being made dynamic?
//-----------------------------------------------------------------------------
void RenderTree::RemoveStaticLight(Light* light){
	for(int i=0;i<data.size();i++){
		for(int j=0;j<data[i]->staticLights.size();j++){
			if(data[i]->staticLights[j] == light){
				data[i]->staticLights.erase(data[i]->staticLights.begin()+j);
				j--;
			}
		}
	}

	for(int i=0;i<numChildren;i++){
		children[i]->RemoveStaticLight(light);
	}
}

//-----------------------------------------------------------------------------
// TODO: Include all nodes if a box is fully enclosed in the sphere
//-----------------------------------------------------------------------------
void RenderTree::GetNodesInSphere(Vector& center, float radius, vector<RenderableNode*>& nodes, bool bCheck){
	for(int i=0;i<data.size();i++){
		if(bCheck){
			// Skip pools that aren't in view
			if(!IsBoxIntersectingSphere(data[i]->box.min,data[i]->box.max,center,radius))
				continue;
		}

		nodes.push_back(data[i]);
	}

	for(int i=0;i<numChildren;i++){
		children[i]->GetNodesInSphere(center,radius,nodes,true);
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void RenderTree::GetNodesInView(Camera* camera, vector<RenderableNode*>& nodes, bool bCheck){
	if(bCheck){
		// Skip nodes that aren't in view
		int ret = camera->BoxInFrustum(box);
		if(ret == CULL_EXCLUSION)
			return; // Completely out of view

		if(ret == CULL_INCLUSION){
			GetNodesInView(camera,nodes,false); // Completely in view, get all nodes without further checking
			return;
		}

		// Partially in view, continue checking...
	}


	for(int i=0;i<data.size();i++){
		if(bCheck){
			// Skip pools that aren't in view
			if(!camera->BoxInFrustum(data[i]->box))
				continue;
		}

		nodes.push_back(data[i]);
	}

	for(int i=0;i<numChildren;i++){
		children[i]->GetNodesInView(camera,nodes,true);
	}
}

//-----------------------------------------------------------------------------
// Device-dependent world state.
//-----------------------------------------------------------------------------
void	World::OnLostDevice()
{
	// for(int i=0;i<m_Actors.size();i++)
	//    SAFE_RELEASE(m_Actors[i]->m_query);
}


//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
World::World()
{
    ProgressCallback = 0;
    m_fLODCullBias  = 1;
	m_Root			= NULL;
	m_CollisionMesh = NULL;
	m_ClipPlane		= 5000;
	m_FogDistance	= 5000;
	m_FogColor		= FloatColor(1,1,1,1);
	m_HasSky		= false;
	m_SkyWorld		= NULL;
	m_IsSky			= false;
	m_IsServer      = true;
	m_OcclusionCulling = new OcclusionCulling();
	m_RenderOcclusionBoxes = false;
	m_fConeRadius   = 50;
    m_DisableOcclusionCulling = false;

    m_bDrawMeshes   = true;
    m_bDoFlips      = true;
    m_bSetSHStates  = true;
    m_bPostRender   = true;

    m_EditorVars.push_back(EditorVar("Draw Meshes",&m_bDrawMeshes,"Debugging","Call DrawIndexedPrimitive?"));
	m_EditorVars.push_back(EditorVar("Update Color Buffers",&m_bDoFlips,"Debugging","Allow color buffer change"));
	m_EditorVars.push_back(EditorVar("Update Mesh States",&m_bSetSHStates,"Debugging","Updates lighting states for meshes"));
	m_EditorVars.push_back(EditorVar("World.PostRender()",&m_bPostRender,"Debugging","Do postrender"));
	

	// Initialize spatial partiion
	Vector buffer(1000,400,1000); // To stop weird static out of bounds errors, or statically empty levels barfing
	BBox worldBox; // TODO: Calculate!
	worldBox.min = Vector(0,0,0);
	worldBox.max = Vector(0,0,0);

	// Only init spatial tree once
	static bool initialized = false;
	if(!initialized)
		SpatialPartition()->Init(worldBox.min - buffer,worldBox.max + buffer);
	initialized = true;

	m_EditorVars.push_back(EditorVar("Fog Distance",&m_FogDistance," World","Distance at which fog reaches full white. Make this very high, or you will get fog very early"));
	m_EditorVars.push_back(EditorVar("Fog Color",&m_FogColor," World","Fog Color"));
	m_EditorVars.push_back(EditorVar("Clip Distance",&m_ClipPlane," World","Default clipping plane distance"));
	m_EditorVars.push_back(EditorVar("Sky Cone Radius",&m_fConeRadius," World","Cone radius for SH sky light"));
    m_EditorVars.push_back(EditorVar("LOD/Cull Bias",&m_fLODCullBias," World","Overall LOD/Culling bias for scene objects"));
    m_EditorVars.push_back(EditorVar("Disable Occlusion",&m_DisableOcclusionCulling," World","Always disable occlusion culling on this map?"));

	for(int i=0;i<m_DefaultSH.EditorVars.size();i++)
		m_EditorVars.push_back(m_DefaultSH.EditorVars[i]);
}


//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void World::NewWorld()
{
	UnLoad();
	if(m_RegisterWorld)
		m_RegisterWorld(this);
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void World::Load(string FileName)
{
    m_UseOcclusionCulling = !Engine::Instance()->IsDedicated() && RenderDevice::Instance()->GetOcclusionTesting();
	ResourceManager::Instance()->m_WorldBeingLoaded = this;
	ResourceManager::Instance()->Add(this);

	StartMiniTimer();
	if(!m_Actors.empty())
		UnLoad();

	FindMedia(FileName,"Maps");
	m_Config.Load(FileName.substr(0,FileName.find_last_of("."))+".ini");

	m_FileName = FileName;

	ReloadSkyController();

	// Register with C# system
	if(m_RegisterWorld)
		m_RegisterWorld(this);

	Serializer load;
	load.LoadWorld(FileName.c_str(),this,false);

	PostProcess::Instance()->CreateFXFromConfigFile(m_Config);

	if(!Editor::Instance()->GetEditorMode())
		InitializeScripts();

	// Precompute lighting for static prefabs
	for(int i=0;i<m_Lights.size();i++){
		m_Lights[i]->MarkStaticItems();
	}

	float loadTime = StopMiniTimer()/1000.0f;
	LogPrintf(_U("Entire world load took %f secs."),loadTime);

	float a = Profiler::Get()->TextureLoadSecs,
		b = Profiler::Get()->MeshLoadSecs,
		c = Profiler::Get()->ShaderLoadSecs,
        d = Profiler::Get()->MeshSetupSecs;

    StartMiniTimer();
	Compiler comp;
	if(Editor::Instance()->GetEditorMode() || !Engine::Instance()->MainConfig->GetBool("PRTAutoCompile"))
		comp.bCompilePRT = false;
	comp.CompileWorld(this);
    float compileSecs = StopMiniTimer()/1000.0f;

    LogPrintf("(Seconds) Textures: %f. Meshes (Load: %f Setup: %f). Shaders: %f. Compile/LoadPRT: %f Unaccounted time: %f",a,b,d,c,compileSecs,loadTime-(a+b+c));


	ResourceManager::Instance()->m_WorldBeingLoaded = NULL;

	// Building occlusion tree from world
	if(!Engine::Instance()->IsDedicated())
		m_OcclusionCulling->BuildFromWorld(*this);
}

void World::ReloadSkyController()
{
	if(m_Config.KeyExists("ScriptClass","SkyController"))
		SkyController::IntitializeNewSky(this,m_Config.GetString("ScriptClass","SkyController"),true);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void World::InitializeScripts()
{
	vector<Actor*> tempActors = m_Actors;
	for(int i = 0; i < tempActors.size();i++)
	{
		if(tempActors[i]->script.classname.length() && tempActors[i]->script.classname != "StaticMesh")
		{
			Matrix m = tempActors[i]->Rotation;
			m.m3 = tempActors[i]->Location;
			ScriptSystem::Instance()->RunScript(tempActors[i]->script,m,tempActors[i],this);
		}
	}
}


//-----------------------------------------------------------------------------
// Can override current sky world with this
//-----------------------------------------------------------------------------
void World::LoadSkyWorld(string worldFile){
	if(!FindMedia(worldFile,"Maps"))
		return;

	SAFE_DELETE(m_SkyWorld);
	m_SkyWorld = new World;
	m_HasSky = true;

	m_SkyWorld->m_IsSky = true;
	m_SkyWorld->m_FileName = worldFile;
	Serializer load;
	load.LoadWorld(worldFile.c_str(),m_SkyWorld,true);
}

World::~World()
{
	UnLoad(); 
	SAFE_DELETE(m_OcclusionCulling);
}


//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void World::UnLoad(){
	// Delete non-prefabs first, avoids touching-on-prefab pointers becoming outdated
	for(int i = 0; i < m_Actors.size(); i++)
	{
		if(m_Actors[i]->IsPrefab)
			continue;
		int beforeSize = m_Actors.size();
		delete m_Actors[i];
		int IndexDifference = beforeSize - m_Actors.size();
		if(IndexDifference > 0)
			i -= IndexDifference;
		else
			i--;
	}
	while(m_Actors.size()) 
		delete m_Actors[0];

	m_Actors.clear();

	// Similar to above, but doesn't expect actors to delete themselves, and doesn't
	// cover the case where deleting a light spawns another light

	// TIM: lights are actors, so why was I deleting again? And why did it work.
	//vector<Light*> tempLights = m_Lights;
	//SAFE_DELETE_VECTOR(tempLights);
	//m_Lights.clear();

	SAFE_DELETE_VECTOR(m_Scripts);
	SAFE_DELETE_VECTOR(m_BatchMeshes);
	SAFE_DELETE_VECTOR(m_Prefabs);
	SAFE_DELETE_VECTOR(m_SHProbes);

	SAFE_DELETE(m_Root);
	SAFE_DELETE(m_CollisionMesh);
	SAFE_DELETE(m_SkyWorld);
	m_HasSky   = false;
	m_FileName = ("MAP NOT LOADED");
	m_OcclusionCulling->Reset();

	if(m_Config.KeyExists("Effect0","PostProcessFX"))
		PostProcess::Instance()->RemoveAllEffects();

	FXManager::Instance()->Reset(this);
}

/*void World::RunPhysics(Actor* actor, float DeltaTime){
float oldDelta = GDeltaTime;
GDeltaTime = DeltaTime;
WorldPhysics::RunPhysics(this,actor);
GDeltaTime = oldDelta;
}*/

//-----------------------------------------------------------------------------
// Ticks world state forward once frame
//-----------------------------------------------------------------------------
void World::Tick()
{
	StartMiniTimer();    

	//Scripting
	if(LibsInitialized())
		SSystem_ScriptsTickAll(this);

    Profiler::Get()->ScriptMS += StopMiniTimer();

	// Only tick if not paused/zero-tick
	// This allows us to render a frozen world, matrix style
	if(GDeltaTime > 0)
	{
        StartMiniTimer();
		if(m_Root)
			m_Root->FlushDynamicLights();

		//if(m_HasSky) 
		//	m_SkyWorld->Tick();

		for(int i=0;i<m_Actors.size();i++){
			Actor* act = m_Actors[i];

			// don't Tick or update Model if about to expire
			if(act->LifeTime != 0)
			{
				// Ordering is correct:
				// 1. CollisionBox gets updated in Actor::PreTick() in preparation for
				// child classes and physics system
				// 2. Normal tick runs, so logic follows new physic state (DEFINITELY what we want. think projectiles hitting walls)
				if(!act->IsScriptActor())
					act->Tick();

				// 3. After Tick and Physics complete, the model
				// is updated in preparation for being rendered.
				// Update model transform now, so it's rendered at the current location
				if(act->MyModel)
				{
					act->MyModel->ClearLights();

					// Only update prefabs if they are selected, because this is slow to do 1000 times
					if(!act->StaticObject || act->IsSelected)
					{
						if(act->m_AutoUpdateModelTransformation)
						{
							act->MyModel->SetTransform(act->Rotation,act->Location);
							act->MyModel->Update();
						}
					}
				}
			}

			// Update actor's life count, and delete if dead
			// -1 = forever
			if(act->LifeTime != -1){
				act->LifeTime -= GDeltaTime;
				if(act->LifeTime <= 0)
				{
					int beforeSize = m_Actors.size();

					SAFE_DELETE(act)
					
					int IndexDifference = beforeSize - m_Actors.size();
					if(IndexDifference > 0)
						i -= IndexDifference;
					else
						i--;
				}
			}
		}
        Profiler::Get()->Actors += m_Actors.size();
        Profiler::Get()->Lights += m_Lights.size();

        Profiler::Get()->TickMS += StopMiniTimer();

        StartMiniTimer();
        if(!Engine::Instance()->IsDedicated())
        {
		    // Light all the scene objects
		    for(int i=0;i<m_Lights.size();i++){
			    m_Lights[i]->MarkLitItems();
		    }
        }
        Profiler::Get()->LightMS += StopMiniTimer();
        
		StartMiniTimer();
		for(int i=0;i<m_Scripts.size();i++){
			Script* script = m_Scripts[i];
			if(!script)
				Error(_U("A script was NULL"));

			bool deleted = false;
			// Update script's life count, and delete if dead
			if(script->LifeTime != -1){
				script->LifeTime -= GDeltaTime;
				if(script->LifeTime <= 0){
					// Remove it
					vector_erase(m_Scripts,script);
					deleted = true;
					SAFE_DELETE(script);
					i--;
				}
			}
			if(!deleted)
				script->Tick();
		}
        Profiler::Get()->ScriptMS += StopMiniTimer();
	}
    
}

//-----------------------------------------------------------------------------
// Renders world + all inhabitants
//-----------------------------------------------------------------------------
void World::Render(Camera* cam)
{
	StartMiniTimer();
	cam->ClearFrustums();

	// Tag any shadow light frustums to the camera frustum, so shadows don't get culled
	for(int i=0;i<m_Lights.size();i++){
		if(m_Lights[i]->m_ShadowMap && !m_Lights[i]->IsHidden)
		{
			// Check to see if shadow frustum is in cam frustum first!
			float r = m_Lights[i]->GetCurrentState().Range;
			BBox box(m_Lights[i]->Location-Vector(r,r,r),m_Lights[i]->Location+Vector(r,r,r));
			if(cam->BoxInFrustum(box))
			{
				cam->AddFrustum(m_Lights[i]->GetView(),m_Lights[i]->GetProjection());
			}
		}
	}

	//Mostafa: Scripting
	if(LibsInitialized())
		SSystem_ActorsPreRender(this, cam);

	// Pre-render stage
	Canvas* canvas = RenderDevice::Instance()->GetCanvas();
	for(int i=0;i<m_Actors.size();i++){
		if(!m_Actors[i]->IsPrefab){
			m_Actors[i]->PreRender(cam);
		}
	}


	// Draw the sky first, so alpha blended items go over it
	// *SHOULD* Draw the sky last, so the fancy Z occlusion hardware is working for us
    // TIM: Disabled - we now use SkyController class
	/*if(m_HasSky)
    {
		// Center camera in the middle of the skyworld
		Camera oldCam = *cam;
		// Update matrix directly. Must not rebuild matrix, or
		// it might have invalid values in (as it does when flying a starship)
		cam->view.m3 = Vector(0,0,0);
		cam->NearClip = 60;
		cam->FarClip =  5000;
		cam->projection = RenderWrap::BuildProjection(cam->Fov,cam->NearClip,cam->FarClip);
		cam->CreateClipPlanes();
		Profiler::Get()->RenderPreMS += StopMiniTimer();
		m_SkyWorld->Render(cam);
		StartMiniTimer(); 
		//RenderWrap::dev->Clear( 0L, NULL, D3DCLEAR_ZBUFFER,0, 1.0f, 0L );
		*cam = oldCam;
	}*/

	cam->FarClip = m_ClipPlane;
	RenderWrap::SetView(cam->view);
	RenderWrap::SetProjection(cam->projection);

	// if Using occlusion system
	if (m_UseOcclusionCulling)
        m_OcclusionCulling->PreRender(cam,this);

	RenderDevice::Instance()->UpdateViewport();

    // Always update sky before PRT
    // TIM: But why is doing it after messing everything up???!!!!!
    if(SkyController::Instance && SkyController::Instance->MyWorld == this)
        SkyController::Instance->DrawSky(cam);

    // Update PRT now, just before rendering
	UpdateGlobalPRT();

	Profiler::Get()->RenderPreMS += StopMiniTimer();

	StartMiniTimer();

	if(LibsInitialized())
		SSystem_ActorsOnRender(this,cam);

	// Render all models
    for(int i=0;i<m_Actors.size();i++)
    {
        if(!m_Actors[i]->m_isScriptActor)
            m_Actors[i]->OnRender(cam);

        if(!m_Actors[i]->IsHidden && m_Actors[i]->MyModel && 
            (!m_Actors[i]->IsOccluded || m_Actors[i]->IsSelected || !m_UseOcclusionCulling))
				m_Actors[i]->MyModel->Draw(cam,this,m_Actors[i]->IsPrefab);//m_Actors[i]->StaticObject
    }

    // Any custom or immediate renders may have altered important states
    RenderDevice::Instance()->SetDefaultStates();

	// Get visible static geometry
	if(m_Root)
	{
		vector<RenderableNode*> nodes;
		m_Root->GetNodesInView(cam,nodes);
		// Convert to batch items
		for(int i=0;i<nodes.size();i++){
			BatchItem* item = new BatchItem;
			item->lights		= nodes[i]->staticLights;
			item->lights.insert(item->lights.end(), nodes[i]->dynamicLights.begin(), nodes[i]->dynamicLights.end());
			item->box			= nodes[i]->box;
			item->mesh			= m_BatchMeshes[nodes[i]->meshID];
			item->subset		= nodes[i]->subset;
			item->matRef		= item->mesh->GetMaterial(item->subset.AttribId);
			item->world			= this;
			item->usesTransform	= false;
			BatchRenderer::Instance()->QueueBatchItem(item);
		}
	}

	RenderDevice::Instance()->UpdateViewport();
	BatchRenderer::Instance()->PrepareQueuedBatches(cam,this);
    ShadowEngine::Instance()->RenderShadowMaps(this,cam);

    Profiler::Get()->BatchMS += StopMiniTimer();

	/*for(int i=0;i<m_Actors.size();i++)
        if(!m_Actors[i]->IsOccluded)
		    m_Actors[i]->DrawRenderTarget(cam);*/
    // FIXME: Abstract this without having to iterate through ALL actors (above)
    if(WaterSurface::Instance() && !WaterSurface::Instance()->IsOccluded && WaterSurface::Instance()->MyWorld == this)
        WaterSurface::Instance()->DrawRenderTarget(cam);

	// Required after RT adjustments	
	RenderDevice::Instance()->UpdateViewport();

    StartMiniTimer();

	BatchRenderer::Instance()->RenderQueuedBatches(this);

	Profiler::Get()->RenderMS += StopMiniTimer();

	// Other dumb FFP rendering functions like this set
	RenderWrap::dev->SetPixelShader(0);
	RenderWrap::SetWorld(Matrix());
	RenderWrap::SetRS(D3DRS_ALPHABLENDENABLE,FALSE);
    
    StartMiniTimer();
    PostProcess::Instance()->PostRenderHDR();
    Profiler::Get()->RenderPostMS += StopMiniTimer();
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void World::SetFog(FloatColor Color, float Density){
	m_FogColor = Color;
	m_FogDistance = Density;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void World::PostRender(Camera* cam)
{
    if(!m_bPostRender)
        return;

    StartMiniTimer();

    LPDIRECT3DSTATEBLOCK9 states;
	RenderWrap::dev->CreateStateBlock(D3DSBT_ALL,&states);
    if (m_UseOcclusionCulling)
	{
		m_OcclusionCulling->PostRender(cam,this);
		if(m_RenderOcclusionBoxes)
			m_OcclusionCulling->RenderBoxes(cam,this);
	}
    ShadowEngine::Instance()->RenderDropShadows(this,cam);
	states->Apply();
    SAFE_RELEASE(states);

	FXManager::Instance()->PostRender(this,*cam);

	//Mostafa: Scripting
	if(LibsInitialized())
		SSystem_ActorsPostRender(this,cam);

	// Post-render stage
	for(int i=0;i<m_Actors.size();i++)
	{
		Actor* act = m_Actors[i];
		if(!act->IsPrefab)
			act->PostRender(cam);
	}

	Profiler::Get()->RenderPostMS += StopMiniTimer();
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
Actor* World::AddActor(Actor* actor){
	if(!this)
		Error(_U("World::AddActor() You tried to add an actor using a NULL world pointer. It is likely that the calling actor was never added to the world itself"));

	actor->SetWorld(this);
	m_Actors.push_back(actor);
	return actor;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
bool World::RemoveActor(Actor* actor)
{
	if(!this)
		Error(_U("World::RemoveActor() You tried to add an actor using a NULL world pointer. It is likely that the calling actor was never added to the world itself"));

	// Allows us to ignore if it shows up in collision tree, etc
	actor->SetWorld(NULL);

    // Remove from occlusion system
    m_OcclusionCulling->RemoveActor(actor);

	bool ret = false;

	// Check lights array
	for(int i=0;i<m_Lights.size();i++){
		if(m_Lights[i] == actor){
			m_Lights.erase(m_Lights.begin() + i);
			ret = true;
			break;
		}
	}

	// Check actors array
	for(int i=0;i<m_Actors.size();i++){
		if(m_Actors[i] == actor){
			m_Actors.erase(m_Actors.begin() + i);
			ret = true;
			break;
		}
	}
	return ret;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
Light* World::FindLight(string name)
{
	for(int i = 0; i < m_Lights.size();i++)
	{
		if(m_Lights[i]->m_Name == name)
			return m_Lights[i];
	}
	return 0;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
Actor* World::CreateScriptActor(string ClassName)
{
	return ScriptEngine::Instance()->CreateActor(ClassName.c_str(),this);
	//ScriptEngine::Instance()->RunScript((string("..\\scripts\\")+ClassName+string(".cs")).c_str(),NULL,this,Matrix());
}
\
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void World::RegenerateOcclusionTree()
{
	m_OcclusionCulling->BuildFromWorld(*this);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
Light* World::FindStrongestLight(Vector& pos, bool Indoors, bool Outdoors)
{
	float minDistFrac = 999;
	Light* bestLight = 0;
	for(int i=0;i<m_Lights.size();i++)
	{
		Light* light = m_Lights[i];

		if(light->m_bShadowProjector || light->LifeTime != -1)
			continue;

		if(light->Outside != Outdoors && light->Inside != Indoors)
			continue;

		Vector v = light->Location - pos;
		// Get fraction as a factor of how far within the radius point is
		float distFrac = v.DotSelf() / (light->GetCurrentState().Range*light->GetCurrentState().Range);
		// If > 1, is out of range
		if(distFrac < 1)
		{
			// Bias by intensity
			distFrac /= light->GetCurrentState().Intensity;
			if(distFrac < minDistFrac)
			{
				minDistFrac = distFrac;
				bestLight = m_Lights[i];
			}
		}
	}

	return bestLight;
}

//-----------------------------------------------------------------------------
// Returns average lighting at a position in space. Does not include directional (sky) lighting
//-----------------------------------------------------------------------------
FloatColor World::GetAverageLighting(Vector& pos, bool Indoors, bool Outdoors)
{
	FloatColor total;
	for(int i=0;i<m_Lights.size();i++)
	{
		Light* light = m_Lights[i];

		if((!Indoors && light->m_Type != LIGHT_DIR) || light->m_bShadowProjector || light->LifeTime != -1 || light->IsHidden || (!Outdoors && light->IsDynamic()))
			continue;

		if(light->Outside != Outdoors && light->Inside != Indoors)
			continue;

		// Get fraction as a factor of how far within the radius point is
		float atten;
		if(light->m_Type == LIGHT_DIR)
			atten = 1;
		else
			atten = 1.0 - (light->Location - pos).Length() / light->GetCurrentState().Range;

		// If < 0, is out of range
		if(atten > 0)
		{
			float intensity = light->GetCurrentState().Intensity;
			if(intensity > 1)
				intensity = 1;

			FloatColor c = light->GetCurrentState().Diffuse * intensity * atten;
			total = total + c;
		}
	}

	return total;
}


