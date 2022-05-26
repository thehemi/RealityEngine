//=========== Copyright (c) 2004, Tim Johnson. All rights reserved. ===========
// The Light class
//
// You can subclass from this to make more advanced lights (like lights with flares)
// This class provides basic lighting and light animation capabilities
//
// NOTE: EnumerateElementsInSphere is REALLY slow in debug, but ultra-fast in release. Grr
//
//=============================================================================
#include "stdafx.h"
#include "Light.h"
#include "ispatialpartition.h"
#include "Collision\CollisionRoutines.h"
#include "ShadowMapping.h"

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
LightState::LightState(){ 
	memset(this,0,sizeof(LightState));
	Intensity	= 1;
	Diffuse = FloatColor(1,1,1,1);
	Range = 25;
    Spot_Size = 80;
    Spot_Falloff = 100;
}
//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
LightState::LightState(const Vector& pos, const FloatColor& diffuse, float range){
	memset(this,0,sizeof(LightState));
	Position	= pos;
	Diffuse		= diffuse;
	Range		= range;
	Intensity	= 1;
    Spot_Size = 80;
    Spot_Falloff = 100;
}
//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
LightState::LightState(const FloatColor& diffuse, float range){
	memset(this,0,sizeof(LightState));
	Position	= Vector(-1,-1,-1); // Used to tell the Light actor that this doesn't have a position
	Diffuse		= diffuse;
	Range		= range;
	Intensity	= 1;
    Spot_Size = 80;
    Spot_Falloff = 100;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Light::Initialize()
{
	MyWorld->m_Lights.push_back(this);
	m_ShadowMapSize = 512;
	m_bShadowProjector = false;
	m_ShadowMap = NULL;
	m_bDynamic	= true;
	m_Type		= LIGHT_OMNI;
	m_Method = LIGHT_METHOD_PERPIXEL;
	m_ForceSHMultiPass = true;
	m_IsExcludeList = true;
    m_HasIncludeExcludeList = true;
	m_ExcludeStaticTree = false;
	GhostObject = true;
	keyframeTime= 0;
	curTime		= 0;
	m_ExclusionListPreviousSize = 0;
	IgnoreKeyframePositions = true; //false
	m_PlaysKeyFrames = true;
	m_UpVec.Set(.1,.7,.2);
	curState.Position = Vector(-1,-1,-1); // Don't use this keyframe as a position frame

	EditorVars.push_back(EditorVar("Diffuse",&curState.Diffuse,"Lighting"));
	EditorVars.push_back(EditorVar("Range",&curState.Range,"Lighting"));
	EditorVars.push_back(EditorVar("SpotFalloff",&curState.Spot_Falloff,"Lighting"));
	EditorVars.push_back(EditorVar("SpotHotspot",&curState.Spot_Size,"Lighting"));	
	EditorVars.push_back(EditorVar("Intensity",&curState.Intensity,"Lighting"));	
	EditorVars.push_back(EditorVar("Type",(int*)&m_Type,"Lighting","1 = Omni, 2 = Spot, 4 = Directional, 8 = Omni projector."));	
	EditorVars.push_back(EditorVar("Force SH Multipass",&m_ForceSHMultiPass,"Lighting","Do Spherical Harmonics from this light as a separate pass. (forced False for Dir lights)"));
	EditorVars.push_back(EditorVar("Shadow Projector",&m_bShadowProjector,"Lighting","Project realtime shadows instead of light? Only valid for spotlights"));
	EditorVars.push_back(EditorVar("Shadow Map Size",&m_ShadowMapSize,"Lighting","Size of shadow map. Bigger is less aliased, but slower and uses more memory"));
	EditorVars.push_back(EditorVar("Lighting Method",(int*)&m_Method,"Lighting","0 = Per Pixel, 1 = PRT. Set automatically. Only override for Per-Pixel on PRT meshes"));

	// Textures
	EditorVars.push_back(EditorVar("Spot Projector Map",(Texture*)&m_tProjectionMap,"Lighting","Projection map for spotlights"));	
	EditorVars.push_back(EditorVar("OmniProjector Map (Cube)",(Texture*)&m_tOmniProjCubeMap,"Lighting","Projection cubemap for cube projector"));	
	EditorVars.push_back(EditorVar("OmniProjector Map Blurred (Cube)",(Texture*)&m_tOmniProjCubeMapBlur,"Lighting","Blurred (distance-based) projection cubemap for cube projector"));	
}

//-----------------------------------------------------------------------------
// Virtual clone, mainly used by editor. Doesn't copy per-actor data like handles
//-----------------------------------------------------------------------------
void Light::Clone(Actor* actor, bool bDeep)
{
	Actor::Clone(actor,bDeep);

	Light* dest = (Light*)actor;
	dest->m_Type				= m_Type;
	dest->m_bShadowProjector	= m_bShadowProjector;
	dest->m_ShadowMapSize		= m_ShadowMapSize;
	dest->m_IsExcludeList		= m_IsExcludeList;
	dest->m_ExcludeList			= m_ExcludeList;
	dest->m_ExcludeListHandles  = m_ExcludeListHandles;
	dest->m_Method				= m_Method;
	dest->keyframes				= keyframes;
	dest->curState				= curState;
	dest->m_ExcludeStaticTree   = m_ExcludeStaticTree;
	dest->IgnoreKeyframePositions = IgnoreKeyframePositions;
	dest->m_ForceSHMultiPass	= m_ForceSHMultiPass;

	dest->m_tProjectionMap.Load(m_tProjectionMap.filename);
	dest->m_tOmniProjCubeMap.Load(m_tOmniProjCubeMap.filename);
	dest->m_tOmniProjCubeMapBlur.Load(m_tOmniProjCubeMapBlur.filename);
}
	

//-----------------------------------------------------------------------------
// This one adds no keyframes or light state
//-----------------------------------------------------------------------------
Light::Light(World* world) : Actor(world){
	Initialize();
	
	if (LibsInitialized()) 
		SSystem_ActorWrap(this);
}

//-----------------------------------------------------------------------------
// alternate ctor used by Script Actors that doesn't add the base class to managed table, to avoid double-managing them
//-----------------------------------------------------------------------------
Light::Light(World *world, bool IsScriptActor) : Actor(world, IsScriptActor)
{
	Initialize();
}

//-----------------------------------------------------------------------------
// This one adds a primary keyframe
//-----------------------------------------------------------------------------
Light::Light(World* world, const FloatColor& col, float range): Actor(world){ 
	Initialize();

	if (LibsInitialized()) 
		SSystem_ActorWrap(this);

	IgnoreKeyframePositions = true;
	m_PlaysKeyFrames = true;
	keyframeTime = 0;

	curState.Position	= Location;
	curState.Diffuse	= col;
	curState.Range		= range;
	keyframes.push_back(curState);
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
Light::~Light(){ 
	SAFE_DELETE(m_ShadowMap);

	// Remove light from world
	if(MyWorld)
		vector_erase(MyWorld->m_Lights,this);
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void Light::SetColor(FloatColor& col){
	curState.Diffuse = col;
}

//-----------------------------------------------------------------------------
// View matrix for spots
// FIXME/TODO: Unify matrices.
//-----------------------------------------------------------------------------
Matrix Light::GetView(){
	Matrix MatLight;
	if(m_ShadowMap){
		D3DXVECTOR3 vUp( 0.0f, 1.0f, 0.0f );
		Vector		 vAt   = curState.Direction*5;
		Vector		 vFrom = curState.Position;
		vAt += vFrom;
		D3DXMatrixLookAtLH( (D3DXMATRIX*)&MatLight,(D3DXVECTOR3*)&vFrom, (D3DXVECTOR3*)&vAt, &vUp );
	}
	else{
		Vector Dir4 = curState.Direction;
		Vector Pos4 = curState.Position;
		Vector up = m_UpVec;
		up.Normalize();
		Vector dirX = Cross(up,Dir4);
		Vector dirY = Cross(Dir4, dirX);

		MatLight[0] = Vector4(dirX.x,dirY.x,Dir4.x,0);
		MatLight[1] = Vector4(dirX.y,dirY.y,Dir4.y,0);
		MatLight[2] = Vector4(dirX.z,dirY.z,Dir4.z,0);
		MatLight[3] = Vector4(-dirX.Dot(Pos4),-dirY.Dot(Pos4),-Dir4.Dot(Pos4),1);
	}

	return MatLight;
}


//-----------------------------------------------------------------------------
// Projection matrix for spots
//-----------------------------------------------------------------------------
Matrix Light::GetProjection(){
	D3DXMATRIX MatProj;
	D3DXMatrixPerspectiveFovLH( &MatProj, DEG2RAD(curState.Spot_Falloff), 1, 0.1f, curState.Range);
	return *(Matrix*)&MatProj;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void Light::SetRange(float Range){
	curState.Range = Range;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void Light::AddKeyframe(const LightState& light, float TimeMS, bool loopKeyFrames)
{
	// No position
	if(light.Position.x == -1){
		if(!IgnoreKeyframePositions && keyframes.size())
			Error("Light::AddKeyFrame: You used a mix of keyframes with and without positions\n\
				  This is not allowed, please use no keyframe positions (and let the engine use the light actor's position, \
				  or use all keyframe positions");
		IgnoreKeyframePositions = true;
	}

	keyframes.push_back(light);
	m_KeyFrameLooping = loopKeyFrames;
	m_PlaysKeyFrames = true;

	if(TimeMS >= keyframeTime)
		keyframeTime = TimeMS;
	else
		Error("Light::AddKeyframe() Keyframe added at time index that is < the previous keyframe time\nPrevious keyframe was at %fMS, requested one is %fMS",keyframeTime,TimeMS);
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void Light::Tick()
{
	// Check every frame, so in reality builder you can change to a shadow projector
	// Also checks to see if shadow map was resized
    if(!Engine::Instance()->IsDedicated())
    {
	    if(m_bShadowProjector && !m_ShadowMap || (m_ShadowMap && fabsf(m_ShadowMap->GetSize() - RenderDevice::Instance()->m_ShadowMapScale*m_ShadowMapSize) > 5))
	    {
		    delete m_ShadowMap;
            m_ShadowMap = new ::ShadowMap;
		    m_ShadowMap->Initialize(RenderDevice::Instance()->m_ShadowMapScale*m_ShadowMapSize);
		    // Shadow maps look crap with high intensities, so start at 0.7
		    //curState.Intensity = 0.7f;
	    }

	    // Shadowmap disabled, delete
	    if(!m_bShadowProjector && m_ShadowMap)
		    SAFE_DELETE(m_ShadowMap);
    }

	// Shadow maps do not work with intensities above 1
	if(m_bShadowProjector && curState.Intensity > 1)
		curState.Intensity = 1.0f;

	if(m_PlaysKeyFrames)
	{
		if(keyframes.size() > 1)
		{
			// Time as milliseconds
			curTime += GDeltaTime*1000;

			// Reset timer for looping
			if( curTime > keyframeTime )
			{
				if(m_KeyFrameLooping)
					curTime = 0;
				else 
					curTime = keyframeTime;
			}

			// Build the interpolated light for this frame
			LightState interpolated;
			const LightState curFrame = keyframes[1];
			const LightState prevFrame = keyframes[0];
			float interpValue = curTime / keyframeTime;
			interpolated = prevFrame + (curFrame-prevFrame)*interpValue;

			//FIXME: Spotlights don't show when we mess with their ranges for keyframing... why?
			if(m_Type != LIGHT_SPOT)
				curState = interpolated;
			else 
				curState.Diffuse = interpolated.Diffuse;

			if(!IgnoreKeyframePositions)
			{
				Location = curState.Position;
				//Update actor rotation. A bit slow, so we won't do this
				//Rotation = Rotation.LookAt(curState.Direction);
			}

		}
		else if(keyframes.size())
			curState = keyframes[0];
	}

	if(IgnoreKeyframePositions)
	{
		curState.Position  = Location;

		//FIXME: Why are spotlight/omniproj directions being offset from the Rotation by 90 degrees? we have to compensate...
		if(m_Type == LIGHT_SPOT || m_Type == LIGHT_OMNI_PROJ)
		{
			Matrix transRot = Rotation;
			Matrix doRotX;  
			doRotX.SetRotations(DEG2RAD(-90),0,0);  
			Matrix transformation;
			transRot = doRotX*Rotation;
			curState.Direction = -transRot[1].Normalized();
		}
		else
			curState.Direction = -Rotation[1].Normalized();
	}

	// HACKY: To feel safe ;p
	if(m_Type == LIGHT_DIR) 
    {
		m_ForceSHMultiPass = false;
        curState.Range = 100000;
    }

	// Check to see if light has moved and become dynamic
	if(!m_bDynamic && m_OldRange > 0 && (m_OldLocation != Location || m_OldRange > curState.Range))
	{
		m_bDynamic = true;
		IgnoreKeyframePositions = true;
		// Strip us from any static light lists we're on
		// 1. Strip from all prefabs
		for(int i=0;i<MyWorld->m_Actors.size();i++){
			Actor* a = MyWorld->m_Actors[i];
			// Strip
			if(a->IsPrefab && a->MyModel){
				for(int j=0;j<a->MyModel->m_TouchingLights.size();j++){
					if(a->MyModel->m_TouchingLights[j] == this){
						a->MyModel->m_TouchingLights.erase(a->MyModel->m_TouchingLights.begin()+j);
						a->MyModel->m_StaticLights--;
						break;
					}
				}
			}
		}
		// 2. Strip from static world tree
		if(MyWorld->GetTree())
			MyWorld->GetTree()->RemoveStaticLight(this);
	}
	m_OldRange    = curState.Range;
	m_OldLocation = curState.Position;

	Actor::Tick();
}

//-----------------------------------------------------------------------------
// Is model excluded by light exclude/include list?
// TODO: Cache handles so we don't do a string lookup each time?
//-----------------------------------------------------------------------------
/*
bool Light::IsExcluded(Model* model){
	bool found = false;

	// Check the handles first
	for(int i=0;i<m_ExcludeListHandles.size();i++){
		if(m_ExcludeListHandles[i] == model){
			found = true;
			break;
		}
	}

	// Ignore excluded meshes
	if((found && m_IsExcludeList) || (!found && !m_IsExcludeList)){
		return true;
	}
	return false;
}*/

bool Light::IsExcluded(Actor* actorIn){
	bool found = false;

	// Check the handles first
	for(int i=0;i<m_ExcludeListHandles.size();i++){
		if(m_ExcludeListHandles[i] == actorIn){
			found = true;
			break;
		}
	}

	// Ignore excluded meshes
	if( (found && m_IsExcludeList) || (!found && !m_IsExcludeList) )  // always allow exclusion lists for shadow projectors, since they're used for selective shadowing projecting
		if(!actorIn->MyModel || actorIn->m_AllowIncludeExclude || actorIn->m_AllowIncludeExcludeLight == this || (m_IsExcludeList && m_bShadowProjector))
			return true;

	return false;
}

//-----------------------------------------------------------------------------
/// All dynamic lights that affect an Actor must mark Actor every frame.
//-----------------------------------------------------------------------------
class MarkActorLightsEmumerator : public IPartitionEnumerator
{
public:
	Light* theLight;

	virtual IterationRetval_t EnumElement( IHandleEntity *pHandleEntity )
	{
		Actor* a = (Actor*)pHandleEntity;
		if(a->MyWorld != theLight->MyWorld)
			return ITERATION_CONTINUE; // Skip actors in different worlds!

		if(a->MyModel)
		{
			// NOTE: This code is bugged
			// If this is a spot, see if the box is in front of the cone
			//if(theLight->m_Type == LIGHT_SPOT){
			//	BBox box = a->MyModel->GetWorldBBox();
			//	float inFront = theLight->GetCurrentState().Direction.Dot( box.min - theLight->Location );
			//	if(inFront <= 0)
			//		inFront = theLight->GetCurrentState().Direction.Dot( box.max - theLight->Location  );

			//	if(inFront <= 0)
			//		return ITERATION_CONTINUE;
			//}

			bool bAdd = true;
			// If m_AllowIncludeExclude or is a prefab, check the exclude list
			// NOTE: Checking IsPrefab is redundancy -- m_AllowIncludeExclude is true for prefabs

			bool IsExcluded = false;

			if(!(a->Inside && theLight->Inside || a->Outside && theLight->Outside))
                bAdd = false;
            else if(a->m_IsExcludeList)
            {
                IsExcluded = theLight->IsExcluded(a);
                bAdd = !IsExcluded;
            }

            if(theLight->m_bShadowProjector)
            {
                //for shadow projectors, use the exclusion list as a way to denote a caster that does not receive (player drop-shadows)
				if(!a->IsHidden && ((bAdd && (!theLight->m_IsExcludeList || !theLight->HasExclusionList())) || (!bAdd && IsExcluded && theLight->m_IsExcludeList)))
                    theLight->m_ShadowMapItems.push_back(a->MyModel);
            }

            if(bAdd)
                a->MyModel->m_TouchingLights.push_back(theLight);
        }
        return ITERATION_CONTINUE;
	}
};

//-----------------------------------------------------------------------------
// Mark actors and prefabs this light touches
// 1. Static Lights <-> Static Prefabs (Well technically all prefabs, 
//    but the dynamic ones are reset every frame)
//
// One-off computation for static worlds/lights, so we can afford to do slow
// things like check include lists
//-----------------------------------------------------------------------------
void Light::MarkStaticItems(){
	// Ignore IsHidden, because this is done only once and light may become visible
	// later
	if(m_bDynamic || m_bShadowProjector) // TODO: Don't force shadow projectors to dynamic. Needed ATM, see comments
		return;

	// Cache all the exclude list meshes - must do this first, because it's used lower down in this function
	// WARNING: Any meshes added to the world after this won't be checked!
	m_ExcludeListHandles.clear();
	for(int k=0;k<m_ExcludeList.size();k++)
	{
		bool found = false;
		for(int i=0;i<MyWorld->m_Actors.size();i++)
		{
			///Model* model = MyWorld->m_Actors[i]->MyModel;
			Actor* model = MyWorld->m_Actors[i];
			if(MyWorld->m_Actors[i]->m_Name == m_ExcludeList.at(k))
			{
				found = true;
				m_ExcludeListHandles.push_back(model);
			};
			/*
			if(model){
				if(model->GetNodeHandle(m_ExcludeList[k]) || model->GetNodeHandle(m_ExcludeList[k]+"_Child")){
					found = true;
					// Store handle
					m_ExcludeListHandles.push_back(model);					
				}
			}*/
		}
		if(!found)
		{
			m_ExcludeList.erase(m_ExcludeList.begin() + k);
			k--;
		}
	}
	m_ExclusionListPreviousSize = m_ExcludeList.size();

	for(int i=0;i<MyWorld->m_Actors.size();i++){
		Actor* a = MyWorld->m_Actors[i];
		if(a->IsPrefab && a->MyModel){
			if(a->Inside != Inside && a->Outside != Outside)
				continue;

			//if(IsExcluded(a->MyModel))
			if(IsExcluded(a))
				continue;

			BBox b = a->MyModel->GetWorldBBox();
			if(IsBoxIntersectingSphere(b.min,b.max,curState.Position,curState.Range)){
				// Add to touching lights
				a->MyModel->m_TouchingLights.push_back(this);
				// Push static light marker forward one. All lights before this
				// marker are static
				a->MyModel->m_StaticLights = a->MyModel->m_TouchingLights.size();
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Mark actors and prefabs this light touches
// 1. Static/Dynamic Lights <-> Dynamic Objects
// 2. Dynamic Lights <-> Static Prefabs
// 3. Dynamic Lights <-> Static Tree
//
// TODO: Clear static lights if object becomes dynamic
// TODO: Cache static items for shadow projectors
//-----------------------------------------------------------------------------
void Light::MarkLitItems()
{
    bool bDynamic = m_bDynamic || m_bShadowProjector;

    m_ShadowMapItems.clear();

	if(!IsVisible() || (bDynamic && !RenderDevice::Instance()->GetDynamicLights()) )
		return;

	if((!m_ExcludeListHandles.size() && m_ExcludeList.size()) || m_ExcludeList.size() != m_ExclusionListPreviousSize)
	{
		m_ExcludeListHandles.clear();
		// Cache all the exclude list meshes - must do this first, because it's used lower down in this function
		// WARNING: Any meshes added to the world after this won't be checked!
		for(int k=0;k<m_ExcludeList.size();k++){
			bool found = false;
			for(int i=0;i<MyWorld->m_Actors.size();i++)
			{
				//Model* model = MyWorld->m_Actors[i]->MyModel;
				Actor* model = MyWorld->m_Actors[i];
				if(model->m_Name == m_ExcludeList.at(k))
				{
					found = true;
					// Store handle
					m_ExcludeListHandles.push_back(model);					
				}
				/*
				if(model)
				{
					if(model->GetNodeHandle(m_ExcludeList[k]) || model->GetNodeHandle(m_ExcludeList[k]+"_Child")){
						found = true;
						// Store handle
						m_ExcludeListHandles.push_back(model);
					}
				}*/
			}
			if(!found)
			{
				m_ExcludeList.erase(m_ExcludeList.begin() + k);
				k--;
			}
		}
	}
	m_ExclusionListPreviousSize = m_ExcludeList.size();

	// Handle dynamic directional lights as a special case, they never need to check ranges
	if(m_Type == LIGHT_DIR && bDynamic){
		for(int i=0;i<MyWorld->m_Actors.size();i++){
			Actor* a = MyWorld->m_Actors[i];
			if(a->MyModel){
				if(a->Inside && Inside || a->Outside && Outside)//if(!a->MyModel->m_AllowIncludeExclude || !IsExcluded(a->MyModel))
					a->MyModel->m_TouchingLights.push_back(this);
			}
		}
		return;
	}

	// 1. Static/Dynamic Lights <-> Dynamic Objects
	// HACK!! Because *tree* speed with large radius sucks so much
	if(curState.Range > 128){
		for(int i=0;i<MyWorld->m_Actors.size();i++){
			Actor* a = MyWorld->m_Actors[i];
			if(a->MyModel && !a->IsPrefab){ // Don't update prefabs, they aren't "Dynamic Objects"
				BBox b = a->MyModel->GetWorldBBox();
				if(IsBoxIntersectingSphere(b.min,b.max,curState.Position,curState.Range)){
					if((a->Inside && Inside || a->Outside && Outside) && (!a->m_AllowIncludeExclude || 
						//!IsExcluded(a->MyModel)))
						!IsExcluded(a)))
						a->MyModel->m_TouchingLights.push_back(this);
				}
			}
		}
	}
	else{
		MarkActorLightsEmumerator markActorLightsEmumerator;
		markActorLightsEmumerator.theLight = this;
		SpatialPartition()->EnumerateElementsInSphere( PARTITION_ENGINE_ACTORS, curState.Position, curState.Range, false, &markActorLightsEmumerator );
	}

	// This if() isn't just an optimization - if static lights cast on static items we'll have duplicate lighting
	// as we cache this info much earlier
	if(bDynamic){
		// 2.  Dynamic Lights <-> Static Prefabs
		MarkActorLightsEmumerator markActorLightsEmumerator;
		markActorLightsEmumerator.theLight = this;
		SpatialPartition()->EnumerateElementsInSphere( PARTITION_ENGINE_PREFABS, curState.Position, curState.Range, false, &markActorLightsEmumerator );

		// 3. Dynamic Lights <-> Static Tree
		// We only update tree if this light DOESN'T have an include list
		// Because if a light has an include list, we can't check the static scene
		// to see if it's included/excluded, so we must make the assumption that the light
		// has excluded the static scene and only wishes to light a few key prefabs
		// It's a hack, but it's better than nothing.
		if(!m_ExcludeStaticTree && (m_IsExcludeList == true || m_ExcludeList.size()==0)){
			vector<RenderableNode*> nodes;
			if(MyWorld->GetTree())
				MyWorld->GetTree()->GetNodesInSphere(curState.Position, curState.Range, nodes);
			for(int i=0;i<nodes.size();i++){
				nodes[i]->dynamicLights.push_back(this);
			}
		}
	}
}

bool Light::HasExclusionList()
{
	return m_ExcludeListHandles.size() && m_IsExcludeList;
}

bool Light::IsVisible()
{
	return !IsHidden && curState.Intensity > 0;
}