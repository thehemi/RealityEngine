//=========== (C) Copyright 2004, Artficial Studios. All rights reserved. =======
// Managed C++ Wrapper code to interface between C# and C++
//
//
//
//===============================================================================
#include "stdafx.h"
#include "Wrappers.h"
#include "NetworkActor.h"
#include "ScriptingEngine.h"
#include "ScriptingSystem.h"
#include "SkyController.h"
#include "IndoorVolume.h"
#include "..\GameDLL\Game\FX\Lasers.h"
#include "..\GameDLL\Game\FX\LaserParticles.h"

using namespace System::IO;
using namespace ScriptingSystem;
using namespace stdcli::language;

using namespace ScriptingSystem;


bool MInput::ControlDown(int ControlHandle)
{
    return Engine::Instance()->InputSys->ControlDown(ControlHandle);
}
bool MInput::ControlJustPressed(int ControlHandle)
{
    return Engine::Instance()->InputSys->ControlJustPressed(ControlHandle);
}
int MInput::GetControlHandle(String^ ControlName)
{
    return Engine::Instance()->InputSys->GetControlHandle(Helpers::ToCppString(ControlName));
}

bool MWorld::CollisionCheckRay(MActor^ sourceToIngore, MVector^ start, MVector^ end, MCheckType checkType, MCollisionInfo^ mResult)
{
    CollisionInfo result;

    Actor* ignoreActor = NULL;
    if(sourceToIngore != nullptr)
        ignoreActor = sourceToIngore->m_actor;

    bool retVal = m_world->CollisionCheckRay(ignoreActor, *start->m_vector, *end->m_vector, (CheckType)(int)checkType, result);

    if(mResult != nullptr)
    {
        mResult->actualDistance = result.actualDistance;
        mResult->normal = gcnew MVector(result.normal);
        mResult->point = gcnew MVector(result.point);
        mResult->touched = MActor::GetFromActor(result.touched);

        if(result.mat)
            mResult->mat = gcnew MMaterial(result.mat);
    }

    return retVal;
}

bool MPrecacher::Precache(String^ ClassName, bool hasCached)
{
    if(!hasCached)
        Precacher::PrecacheResources(Helpers::ToCppString(ClassName).c_str());

    return true;
}
void MPrecacher::PurgeCache(String^ ClassName)
{
    Precacher::PurgeCache(Helpers::ToCppString(ClassName));
}
void MPrecacher::PurgeTotalCache()
{
    Precacher::PurgeCache();
}

MModel^ MPrecacher::PrecacheModel(String^ ClassName,String^ FileName)
{
    int classID = -1;

    MModel^ model = gcnew MModel();
    Precacher::CacheMeFileName(Helpers::ToCppString(ClassName).c_str(),Helpers::ToCppString(FileName).c_str(),classID,model->m_model);
    return model;
}
MTexture^ MPrecacher::PrecacheTexture(String^ ClassName,String^ FileName)
{
    int classID = -1;

    MTexture^ texture = gcnew MTexture();
    texture->usesLOD = false;
    Precacher::CacheMeFileName(Helpers::ToCppString(ClassName),Helpers::ToCppString(FileName),classID,texture->m_texture);
    return texture;
}
MSound^ MPrecacher::PrecacheSound(String^ ClassName,String^ FileName, bool b2D)
{
    int classID = -1;

    MSound^ sound = gcnew MSound();
    Precacher::CacheMeFileName(Helpers::ToCppString(ClassName),Helpers::ToCppString(FileName),classID,sound->m_Sound,b2D);
    return sound;
}
MMaterial^ MPrecacher::PrecacheMaterial(String^ ClassName,String^ FileName)
{
    int classID = -1;

    Material* mat = NULL;
    Precacher::CacheMeFileName(Helpers::ToCppString(ClassName),Helpers::ToCppString(FileName),classID,&mat);

    MMaterial^ mMaterial = gcnew MMaterial(mat);

    return mMaterial;
}

ArrayList^ MModelFrame::EnumerateMeshes()
{
    vector<ModelFrame*> frames;
    m_modelframe->EnumerateMeshes(frames);
    ArrayList^ arrayList = gcnew ArrayList();
    for(int i = 0; i < frames.size(); i++)
    {
        MModelFrame^ frame = gcnew MModelFrame(frames[i]);
        arrayList->Add(frame);
    }
    return arrayList;
}
ArrayList^ MModelFrame::EnumerateFrames()
{
    vector<ModelFrame*> frames;
    m_modelframe->EnumerateFrames(frames);
    ArrayList^ arrayList = gcnew ArrayList();
    for(int i = 0; i < frames.size(); i++)
    {
        MModelFrame^ frame = gcnew MModelFrame(frames[i]);
        arrayList->Add(frame);
    }
    return arrayList;
}

void MModelFrame::SetMaterial(int MatIndex,MMaterial^ material)
{
    if(!m_modelframe->GetMesh())
        return;

    m_modelframe->GetMesh()->m_Materials[MatIndex]->Release();

    m_modelframe->GetMesh()->m_Materials[MatIndex] = material->m_material;
    material->m_material->AddRef();
}
MMaterial^ MModelFrame::GetMaterial(int MatIndex)
{
    if(!m_modelframe->GetMesh())
        return nullptr;

    return gcnew MMaterial(m_modelframe->GetMesh()->m_Materials[MatIndex]);
}
void MMatrix::Rotate(MVector^ vectorDegrees)
{
    Matrix doRotX;
    doRotX.SetRotations(DEG2RAD(vectorDegrees->x),0,0);
    Matrix doRotY;
    doRotY.SetRotations(0,DEG2RAD(vectorDegrees->y),0);
    Matrix doRotZ;
    doRotZ.SetRotations(0,0,DEG2RAD(vectorDegrees->z));
    *m_matrix = (m_matrix->Inverse() * (doRotX*doRotY*doRotZ)).Inverse();
}
void MMatrix::Rotate(float PitchDeg, float YawDeg, float RollDeg)
{
    Matrix doRotX;
    doRotX.SetRotations(DEG2RAD(PitchDeg),0,0);
    Matrix doRotY;
    doRotY.SetRotations(0,DEG2RAD(YawDeg),0);
    Matrix doRotZ;
    doRotZ.SetRotations(0,0,DEG2RAD(RollDeg));
    *m_matrix = (m_matrix->Inverse() * (doRotX*doRotY*doRotZ)).Inverse();
}
void MModel::Draw(MWorld^ world)
{
    m_model->Draw(CameraHandler::Instance()->GetCamera(),world->m_world,false,DRAW_UPDATELIGHTING);
}
void MModel::DrawImmediate(MWorld^ world)
{
    m_model->Draw(CameraHandler::Instance()->GetCamera(),world->m_world,false,DRAW_UPDATELIGHTING | DRAW_IMMEDIATE);
}
void MModel::Draw(MWorld^ world, MActor^ owner)
{
    m_model->Draw(CameraHandler::Instance()->GetCamera(),world->m_world,false,DRAW_UPDATELIGHTING, owner->m_actor);
}
void MModel::DrawImmediate(MWorld^ world, MActor^ owner)
{
    m_model->Draw(CameraHandler::Instance()->GetCamera(),world->m_world,false,DRAW_UPDATELIGHTING | DRAW_IMMEDIATE, owner->m_actor);
}
int MModel::LoadAnimation(String^ animationFile, bool looping, ArrayList^ AffectedFrameNames)
{
    vector<string> frameNames;
    frameNames.resize(AffectedFrameNames->Count);
    for(int i = 0; i < AffectedFrameNames->Count; i++)
    {
        frameNames[i] = Helpers::ToCppString((String^)AffectedFrameNames[i]);
    }
    return (int)m_model->LoadAnimation(Helpers::ToCppString(animationFile),looping,&frameNames);
}
void MLight::AddKeyFrame(float Range, MFloatColor^ color, float Intensity, float TimeSeconds)
{
    if(!((Light*)m_actor)->GetNumKeyFrames())
    {
        LightState originalState = ((Light*)m_actor)->GetCurrentState();
        originalState.Position = Vector(-1,-1,-1);
        ((Light*)m_actor)->AddKeyframe(originalState,0,false);
    }

    LightState state;
    state.Diffuse = *color->m_color;
    state.Range = Range;
    state.Intensity = Intensity;
    ((Light*)m_actor)->AddKeyframe(state,TimeSeconds*1000.0f,false);
}

void MHelpers::SetViewportZ(float minZ, float maxZ)
{
    if(!RenderWrap::dev)
        return;

    D3DVIEWPORT9 viewport;
    viewport.X = 0;
    viewport.Y = 0;
    viewport.MinZ = minZ;
    viewport.MaxZ = maxZ;
    viewport.Height = Canvas::Instance()->Height;
    viewport.Width = Canvas::Instance()->Width;
    RenderWrap::dev->SetViewport(&viewport);
    RenderDevice::Instance()->MinViewportZ = minZ;
    RenderDevice::Instance()->MaxViewportZ = maxZ;
}

float MSkyController::GetDayTime()
{
    if(SkyController::Instance)
        return SkyController::Instance->GetDayTimeMinutes();

    return 0;
}
void MSkyController::SetDayTime(float DayTimeMinutes)
{
    if(SkyController::Instance)
        SkyController::Instance->SetDayTime(DayTimeMinutes);
}

void MSkyController::SetSpeed(float minutesPerGameSecond)
{
    if(SkyController::Instance)
        SkyController::Instance->SetMinutesPerGameSecond(minutesPerGameSecond);
}

unsigned long MSkyController::GetAmbientColor(MVector^ atLocation)
{
    if(SkyController::Instance)
    {
        FloatColor color = SkyController::Instance->AmbientLight->GetCurrentState().Diffuse;
        color.Clamp();

        return (unsigned long)color.DWORDColor();
    }

    return (unsigned long)COLOR_RGBA(255,255,255,255);
}

void MFXManager::SetParticleSystemProperties(FXSystemHash^ systemHash, unsigned long color, MBlendMode destblend,float spawnInterval, 
                                             int maxParticles, MVector^ centerpos, MVector^ spraydirection, MVector^ acceleration, 
                                             float particleSpeed, float particleDirVariance, float particleSpeedVariance, 
                                             float particleLifeTimeMS, float particleLifeTimeVarianceMS, float particleTimeFadeOutMS, 
                                             float particleSizeMeters, float particleSizeVarianceMeters, MVector^ posVariance, bool preventIndoors, float YaxisConstraint)
{
    ParticleSystem* ps = (ParticleSystem*)FXManager::Instance()->GetFXSystemFromHash(systemHash->HashIndex,systemHash->systemPointer);
    if(ps)
    {
        ps->blendMode = (BlendMode)(int)destblend;
        ps->msSpawn = spawnInterval;
        ps->variance = particleDirVariance;
        ps->particlesize = particleSizeMeters;
        ps->sizevariance = particleSizeVarianceMeters;
        ps->accel = *acceleration->m_vector;
        ps->speed = particleSpeed;
        ps->timefadeout = particleTimeFadeOutMS;
        ps->lifetime = particleLifeTimeMS;
        ps->lifetimevariance = particleLifeTimeVarianceMS;
        ps->speedvariance = particleSpeedVariance;
        ps->m_Color = color;
        ps->Location = *centerpos->m_vector;
        ps->sprayDir = *spraydirection->m_vector;
        ps->posVariance = *posVariance->m_vector;
        ps->SetMaxParticles(maxParticles);
        ps->preventIndoors = preventIndoors;
        ps->YaxisConstraint = YaxisConstraint;
    }
}
void MFXManager::SetParticleSystemColor(FXSystemHash^ systemHash, unsigned long color)
{
    ParticleSystem* ps = (ParticleSystem*)FXManager::Instance()->GetFXSystemFromHash(systemHash->HashIndex,systemHash->systemPointer);
    if(ps)
        ps->m_Color = color;
}
void MFXManager::SetParticleSystemAlphaMultiplier(FXSystemHash^ systemHash, float alphaMultiplier)
{
    ParticleSystem* ps = (ParticleSystem*)FXManager::Instance()->GetFXSystemFromHash(systemHash->HashIndex,systemHash->systemPointer);
    if(ps)
		ps->m_AlphaMultiplier = alphaMultiplier;
}

void MFXManager::SetParticleSystemTransformation(FXSystemHash^ systemHash, MVector^ centerpos, MVector^ spraydirection)
{
    ParticleSystem* ps = (ParticleSystem*)FXManager::Instance()->GetFXSystemFromHash(systemHash->HashIndex,systemHash->systemPointer);
    if(ps)
    {
        ps->Location = *centerpos->m_vector;
        ps->sprayDir = *spraydirection->m_vector;
    }
}

FXSystemHash^ MFXManager::AddLaser(MWorld^ world,MTexture^ tex,unsigned long color,float width, MVector^ startPos, MVector^ endPos, MBlendMode destblend)
{
    FXSystemHash^ hash = gcnew FXSystemHash();

    Laser* laser = new Laser(world->m_world,color,*startPos->m_vector,*endPos->m_vector,width,tex->m_texture,(BlendMode)(int)destblend);
    laser->RegisterWithHash();
    hash->HashIndex = laser->HashIndex;
    hash->systemPointer = laser;

    return hash;
}

FXSystemHash^ MFXManager::AddLightBeam(MWorld^ world,MTexture^ tex,unsigned long color,int NumUnits, float DistanceMultiplier, float StartSize, float SizeMultiplier,float TotalScalingFactor)
{
    FXSystemHash^ hash = gcnew FXSystemHash();

    LightBeam* lightBeam = new LightBeam(world->m_world, tex->m_texture, NumUnits, DistanceMultiplier, StartSize, SizeMultiplier, TotalScalingFactor, color);
    lightBeam->RegisterWithHash();
    hash->HashIndex = lightBeam->HashIndex;
    hash->systemPointer = lightBeam;

    return hash;
}

void MFXManager::SetLightBeamTransformation(FXSystemHash^ systemHash, MVector^ Pos, MVector^ Dir)
{
    LightBeam* lightBeam = (LightBeam*)FXManager::Instance()->GetFXSystemFromHash(systemHash->HashIndex,systemHash->systemPointer);
    if(lightBeam)
        lightBeam->setPosDir(*Pos->m_vector,*Dir->m_vector);
}

void MFXManager::SetLightBeamProperties(FXSystemHash^ systemHash, unsigned long color,int NumUnits, float DistanceMultiplier, float StartSize, float SizeMultiplier,float TotalScalingFactor)
{
    LightBeam* lightBeam = (LightBeam*)FXManager::Instance()->GetFXSystemFromHash(systemHash->HashIndex,systemHash->systemPointer);
    if(lightBeam)
    {
        lightBeam->m_NumUnits = NumUnits;
        lightBeam->m_Color = color;
        lightBeam->m_DistanceMultiplier = DistanceMultiplier;
        lightBeam->m_StartSize = StartSize;
        lightBeam->m_SizeMultiplier = SizeMultiplier;
        lightBeam->m_TotalScalingFactor = TotalScalingFactor;
        lightBeam->UpdateBeamsInfos();
    }
}
void MFXManager::SetLightBeamColor(FXSystemHash^ systemHash, unsigned long color)
{
    LightBeam* lightBeam = (LightBeam*)FXManager::Instance()->GetFXSystemFromHash(systemHash->HashIndex,systemHash->systemPointer);
    if(lightBeam)
        lightBeam->setColor(color);
}

FXSystemHash^ MFXManager::AddSurfaceDecalHash(MWorld^ world,MTexture^ tex,unsigned long color, MBlendMode destblend,MVector^ pos,MVector^ normal, float size,float lifeTime,float fadeTime)
{
    FXSystemHash^ hash = gcnew FXSystemHash();

    SurfaceDecal* decal = new SurfaceDecal(world->m_world,tex->m_texture, *pos->m_vector, *normal->m_vector, color, size, lifeTime, fadeTime,(BlendMode)(int)destblend);
    decal->RegisterWithHash();

    hash->HashIndex = decal->HashIndex;
    hash->systemPointer = decal;

    return hash;
}

void MFXManager::SetSurfaceDecalTransformation(FXSystemHash^ systemHash, MVector^ Pos, MVector^ Dir)
{
    SurfaceDecal* decal = (SurfaceDecal*)FXManager::Instance()->GetFXSystemFromHash(systemHash->HashIndex,systemHash->systemPointer);
    if(decal)
        decal->setPosDir(*Pos->m_vector,*Dir->m_vector);
}

void MFXManager::SetLaserEndPoints(FXSystemHash^ systemHash,MVector^ startPos, MVector^ endPos)
{
    Laser* laser = (Laser*)FXManager::Instance()->GetFXSystemFromHash(systemHash->HashIndex,systemHash->systemPointer);
    if(laser)
        laser->SetEndPoints(*startPos->m_vector,*endPos->m_vector);
}
void MFXManager::SetLaserColor(FXSystemHash^ systemHash, unsigned long color)
{
    Laser* laser = (Laser*)FXManager::Instance()->GetFXSystemFromHash(systemHash->HashIndex,systemHash->systemPointer);
    if(laser)
        laser->SetColor(color);
}
void MFXManager::SetEffectHidden(FXSystemHash^ systemHash, bool IsHidden)
{
    FXSystem* system = FXManager::Instance()->GetFXSystemFromHash(systemHash->HashIndex,systemHash->systemPointer);
    if(system)
        system->IsHidden = IsHidden;
}

void MFXManager::DestroyEffect(FXSystemHash^ systemHash)
{
    FXSystem* system = FXManager::Instance()->GetFXSystemFromHash(systemHash->HashIndex,systemHash->systemPointer);
    if(system)
        delete system;
}

void MFXManager::SetEffectWorld(FXSystemHash^ systemHash,MWorld^ world)
{
	FXSystem* system = FXManager::Instance()->GetFXSystemFromHash(systemHash->HashIndex,systemHash->systemPointer);
    if(system)
	{
		if(world != nullptr)
			system->MyWorld = world->m_world;
		else
			system->MyWorld = NULL;
	}
}

void MFXManager::AddLaserParticleSystem(MWorld^ world,MTexture^ tex,MBlendMode destblend,bool bouncyParticles,
                                        float laserLength,float spawnInterval,int numParticles, int maxParticles,
                                        MVector^ centerpos, MVector^ spraydirection, MVector^ acceleration, 
                                        float particleSpeed, float particleDirVariance, float particleSpeedVariance,
                                        float particleLifeTimeMS, float particleLifeTimeVarianceMS, 
                                        float particleTimeFadeOutMS, float particleSizeMeters, float particleSizeVarianceMeters, 
                                        float emitterLifeTimeMS, bool StopEmittingImmediately)
{
    LaserParticleSystem* ps = new LaserParticleSystem(laserLength,world->m_world,spawnInterval,numParticles,maxParticles,*centerpos->m_vector,*spraydirection->m_vector,*acceleration->m_vector,particleSpeed,particleDirVariance,particleSpeedVariance,particleLifeTimeMS,particleLifeTimeVarianceMS,particleTimeFadeOutMS,particleSizeMeters,particleSizeVarianceMeters,false,bouncyParticles,tex->m_texture,(BlendMode)(int)destblend);			
    ps->LifeTime = emitterLifeTimeMS/1000.0f;
    if(StopEmittingImmediately)
        ps->StopSpawningNow();
}

bool MWorld::IsIndoors(MVector^ location)
{
    return IndoorVolume::IsIndoors(m_world,*location->m_vector);
}
bool MWorld::IsInVolumeType(MVector^ location, MVolumeType volumeType)
{
	return IndoorVolume::IsInVolumeType(m_world,*location->m_vector,(int)volumeType);
}
unsigned long MWorld::GetAverageLighting(MVector^ location, bool Indoors, bool Outdoors)
{
    FloatColor col = m_world->GetAverageLighting(*location->m_vector,Indoors,Outdoors);
    col.Clamp(); 
    return (unsigned long)col.DWORDColor();
}

MVector^ MCamera::GetCoords_ScreenFromWorld(MVector^ posInWorld, bool doNotCull)
{
    if(doNotCull || (*posInWorld->m_vector - m_camera->Location).Normalized().Dot(m_camera->view.Inverse().GetDir().Normalized()) > 0)
    {
        Matrix worldMatrix;
        D3DVIEWPORT9 viewport;
        viewport.Height = 768;
        viewport.Width = 1024;
        viewport.MaxZ = RenderDevice::Instance()->MaxViewportZ;
        viewport.MinZ = RenderDevice::Instance()->MinViewportZ;
        viewport.X= 0;
        viewport.Y = 0;
        Vector screenCoords;
        D3DXVec3Project((D3DXVECTOR3*)&screenCoords,(D3DXVECTOR3*)&(m_camera->Location + (*posInWorld->m_vector - m_camera->Location).Normalized()*m_camera->Location.Length()*.5),&viewport,(D3DXMATRIX*)&m_camera->projection,(D3DXMATRIX*)&m_camera->view,(D3DXMATRIX*)&worldMatrix);
        return gcnew MVector(screenCoords);
    }
    return gcnew MVector(-2000,0,0);
}

MVector^ MCamera::GetCoords_WorldFromScreen(MVector^ posOnScreen)
{
    Matrix worldMatrix;
    D3DVIEWPORT9 viewport;
    viewport.Height = 768;
    viewport.Width = 1024;
    viewport.MaxZ = RenderDevice::Instance()->MaxViewportZ;
    viewport.MinZ = RenderDevice::Instance()->MinViewportZ;
    viewport.X= 0;
    viewport.Y = 0;
    Vector worldCoords;
    D3DXVec3Unproject((D3DXVECTOR3*)&worldCoords,(D3DXVECTOR3*)posOnScreen->m_vector,&viewport,(D3DXMATRIX*)&m_camera->projection,(D3DXMATRIX*)&m_camera->view,(D3DXMATRIX*)&worldMatrix);
    return gcnew MVector(worldCoords);
}

MLight^ MSkyController::GetAmbientLight()
{
    if(!SkyController::Instance)
        return nullptr;

    return (MLight^)MActor::GetFromActor(SkyController::Instance->AmbientLight);
}
float MSkyController::GetLightningIntensity()
{
    if(!SkyController::Instance)
        return 0;

	return SkyController::Instance->GetLightningIntensity();
}
bool MSkyController::HasSky(MWorld^ world)
{
    return SkyController::Instance != NULL && SkyController::Instance->MyWorld == world->m_world;
}
MVector^  MSkyController::GetSunPosition()
{
    if(!SkyController::Instance)
        return gcnew MVector();

    return gcnew MVector(SkyController::Instance->SunPosition);
}

void MWorld::RunPhysics(MActor^ actor)
{
    m_world->RunPhysics(actor->m_actor);
}

MCamera^ MCameraHandler::GetCamera()
{
    return gcnew MCamera(CameraHandler::Instance()->GetCamera());
}

		ArrayList^ MHelpers::EnumerateDirectories(String^ path, int depth)
		{
			ArrayList^ arrayDirs = gcnew ArrayList();
			vector<string> dirs;
			enumerateDirectories(Helpers::ToCppString(path).c_str(),dirs,depth);
			for(int i = 0; i < dirs.size();i++)
				arrayDirs->Add(Helpers::ToCLIString(dirs[i]));
			return arrayDirs;
		}

		ArrayList^ MHelpers::EnumerateFiles(String^ path, String^ fileExtension, int depth)
		{
			ArrayList^ arrayFiles = gcnew ArrayList();
			vector<string> files;
			enumerateFiles(Helpers::ToCppString(path).c_str(),files,depth,Helpers::ToCppString(fileExtension).c_str());
			for(int i = 0; i < files.size();i++)
				arrayFiles->Add(Helpers::ToCLIString(files[i]));
			return arrayFiles;
		}