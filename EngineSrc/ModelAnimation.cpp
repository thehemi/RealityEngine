//=========== Copyright (c) 2003, Tim Johnson. All rights reserved. ===========
// Model: Various Model classes.
// NOTE: Could use 2-bone IK for matching up to some surfaces such as stairs
// or weapons. Halo did
//
// Understanding bones:
// -Bones are just frames.
// -All frames can be animated via independent animation sets.
// -Skin weights (indices, offsets, weights, name) are used combined with these frames to skin the mesh
//
// -Base model is in local space.
//
// -Keyframe/AnimationController CombinedTransform = Straight Local (Hierarchical) TM
// -This must be converted to world space every frame after the animation controller
// updates it
//
// -invMatrix = Per-bone inverse transform to take model to bone space 
//   (if we inverse it again, we simply get the reference bone pos in world space)
//
// -so invMatrix * CombinedTransform gives us the animated bone
//=============================================================================
#include "stdafx.h"
#include "Frame.h"
#include <atlbase.h>
#include <initguid.h>
#include "dxfile.h"
#include "rmxfguid.h"
#include "rmxftmpl.h"
//unsigned char* D3DRM_XTEMPLATES

extern vector<StaticModel*> modelCache;

// From Export Plugin -- Keep in Sync!!!
// {3F487F18-AEC4-4234-9130-66DB022457F4}
DEFINE_GUID(DXFILEOBJ_AnimationSettings, 
            0x3f487f18, 0xaec4, 0x4234, 0x91, 0x30, 0x66, 0xdb, 0x2, 0x24, 0x57, 0xf4);

#ifndef DOXYGEN_IGNORE
//-----------------------------------------------------------------------------
/// Loads animation sets from animation files, which we parse directly
//-----------------------------------------------------------------------------
class Loader {
    struct Animation {
        /// Contains transformation and a time for a given node within an animation keyframe
        struct D3DXKEY_MATRIX{
            Matrix Value;
            float  Time;
        };
        string						name;
        vector<D3DXKEY_QUATERNION>	rotations;
        vector<D3DXKEY_VECTOR3>		scalings;
        vector<D3DXKEY_VECTOR3>		translations;
        vector<D3DXKEY_MATRIX>				matrices;
    };
    vector<Animation*> animations;


    BOOL ParseObject(ID3DXFileData *pDataObj,ID3DXFileData *pParentDataObj,
                 DWORD Depth,BOOL Reference);

    ID3DXFile*	m_pXFile;
    HRESULT LoadXFileFrame(ID3DXFileData* data);
    HRESULT LoadXFileAnimationKey(ID3DXFileData* data, Animation* anim);
    HRESULT LoadXFileAnimation(ID3DXFileData* data);
    HRESULT LoadXFileAnimationSet(ID3DXFileData* data);
    HRESULT RegisterTemplates(ID3DXFile* pXFile);
    HRESULT LoadXFile(LPVOID pSource, DXFILELOADOPTIONS options);

public:

    Loader(){
        AffectedBoneNames = 0;
        m_pXFile = NULL;
        bLooping = TRUE;
    }


    HRESULT	LoadFromFile(LPCSTR filename);
    /// Contains handle to d3dx animation and total length of animation
    struct AnimSet{
        float duration;
        LPD3DXKEYFRAMEDANIMATIONSET set;
    };
    vector<string>* AffectedBoneNames;
    vector<AnimSet> animationSets;
    BOOL			bLooping;
    float			fScale; // Scale for animations
};
#endif

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void Model::Destroy(){
    SAFE_RELEASE(m_AnimationController);
    StaticModel::Destroy();
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void Model::InitAfterLoad(){
    StaticModel::InitAfterLoad();
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
Model::Model(){
    paused		 = false;
    m_AnimationController = NULL;
    curAnimTrack = -1;
}

//-----------------------------------------------------------------------------
// Pass-through virtual
//-----------------------------------------------------------------------------
void Model::SetTransform(Matrix& transform){
    StaticModel::SetTransform(transform);
}

	/// Get duration of animation, in seconds
	float Model::GetDuration(ANIMATIONHANDLE anim){ if(anim==-1 || Engine::Instance()->IsDedicated()) return 0; return tracks[anim].duration; }
	/// Get remaining time, in seconds
	float Model::GetRemaining(ANIMATIONHANDLE anim){ if(anim==-1 || Engine::Instance()->IsDedicated()) return 0; return tracks[anim].endTime-m_AnimationController->GetTime() ; }
	/// Get looping state
	bool  Model::IsLooping(ANIMATIONHANDLE anim){ if(anim==-1 || Engine::Instance()->IsDedicated()) return false; return tracks[anim].bLooping; }


//-----------------------------------------------------------------------------
// Desc: Transforms the entire hierarchy, keeping relative offsets that were 
// saved with the file
//-----------------------------------------------------------------------------
void Model::SetTransform(Matrix& rotOffset, Vector& locOffset){
    if(!m_pFrameRoot)
        return;

    
    Matrix mat = rotOffset;
    mat.m3 = locOffset;
    m_RootTransform = mat;

    SetTransform(mat);
}

//-----------------------------------------------------------------------------
// Desc: Clones a model without copying the shared data
//-----------------------------------------------------------------------------
void Model::CreateNewInstance(StaticModel* newModel, bool bCloneMeshData){
    Model* pNew = (Model*)newModel;
    *pNew = *(Model*)this;
    pNew->m_OcclusionQuery = NULL; // We want a unique query for this instance

    // Clone the frame hierarchy
    if(m_pFrameRoot)
        pNew->m_pFrameRoot = m_pFrameRoot->Clone(bCloneMeshData);

    // Clone animation controller
    if(m_AnimationController){
        pNew->InitializeAnimationSystem(m_AnimationController);
    }

    if(pNew->m_pFrameRoot)
        pNew->m_pFrameRoot->UpdateMatrices(pNew->m_RootTransform);

    // Add instance to shared list
    m_pInstances->push_back((Model*)newModel);

    // We must add all models, even instances, to the cache, so that
    // if one copy is deleted we can still look up a fellow instance
    modelCache.push_back(newModel);
}

//-----------------------------------------------------------------------------
// Desc: Initializes or clones the animation controller 
//-----------------------------------------------------------------------------
void Model::InitializeAnimationSystem(LPD3DXANIMATIONCONTROLLER cloneController)
{
    if(!m_pFrameRoot)
    {
        SeriousWarning("%s load failed, yet InitializeAnimationSystem was called",this->m_FileName.c_str());
        return;
    }

    int matrices = m_pFrameRoot->CountFrames()+1;

    if(cloneController){
        cloneController->CloneAnimationController(matrices,100,100,100,&m_AnimationController);
    }
    else{
        D3DXCreateAnimationController(matrices,100,100,100,&m_AnimationController);
 
        D3DXTRACK_DESC desc;
        desc.Priority = D3DXPRIORITY_HIGH;
        desc.Weight = 0.0f; // Start from 0, will ramp up to 100% over a second or so
        desc.Speed = 1;		// Reset the speed to 100%
        desc.Position = 0;  // Start the animation now, from the beginning
        desc.Enable = true; // Enable track
        m_AnimationController->SetTrackDesc(0,&desc);
    }

    // Need to call this before working on tracks, or the next call will hang forever
    m_AnimationController->AdvanceTime(0.01f,0);

    // Register the new matrix pointers, it'll overwrite the ones from the old mesh
    m_pFrameRoot->Register(m_AnimationController);
    m_pFrameRoot->RegisterSkinMatrices(m_pFrameRoot);
}


//-----------------------------------------------------------------------------
// Desc: Loads animation for entire model hierarchy
// TODO: Support multiple animation sets
//-----------------------------------------------------------------------------
ANIMATIONHANDLE Model::LoadAnimation(string filename, bool bLooping, vector<string>* AffectedBoneNames)
{
	if(Engine::Instance()->IsDedicated())
	{
		tracks.resize(tracks.size() + 1);
		return tracks.size() - 1;
	}

    StartMiniTimer();

    if(!m_AnimationController)
        InitializeAnimationSystem();

    Loader load;
    load.bLooping = bLooping;
    load.AffectedBoneNames = AffectedBoneNames;
    load.fScale = GetLoadingScale();
    if(!FindMedia(filename,"Models"))
        return -1;

    if(load.LoadFromFile(filename.c_str()) != S_OK)
        return -1;
    assert(load.animationSets.size() == 1);

    // Grow tracks size +1, and add looping info
    tracks.resize(tracks.size()+1);
    tracks[tracks.size()-1].bLooping = load.bLooping;
    tracks[tracks.size()-1].duration = load.animationSets[0].duration;

    // Register animation set (contains all node animations for this file)
    int track = m_AnimationController->GetNumAnimationSets();
    DXASSERT(m_AnimationController->SetTrackEnable(track,false)); // Off until activated
    DXASSERT(m_AnimationController->RegisterAnimationSet(load.animationSets[0].set));
    DXASSERT(m_AnimationController->SetTrackAnimationSet(track,load.animationSets[0].set));

    LogPrintf("Anim '%s' load took %f seconds",filename.c_str(),StopMiniTimer()/1000.0f);
    return m_AnimationController->GetNumAnimationSets() - 1;
}

//-----------------------------------------------------------------------------
// Desc: Updates animation for entire model hierarchy
//-----------------------------------------------------------------------------
void Model::Update()
{
    StaticModel::Update();

    if(!paused && m_AnimationController)
	{
        m_AnimationController->AdvanceTime(GDeltaTime,0);
        // Compile the hierarchy!
        m_pFrameRoot->UpdateMatrices(m_RootTransform);
		m_BBox = m_pFrameRoot->GetWorldBBox();
    }
}

//-----------------------------------------------------------------------------
// Desc: 
//-----------------------------------------------------------------------------
void Model::SetAnimationWeight(ANIMATIONHANDLE animation, float weight, float transitionTime)
{
    if(!m_AnimationController || Engine::Instance()->IsDedicated())
        return;

    m_AnimationController->KeyTrackWeight(animation,weight,m_AnimationController->GetTime(), transitionTime, D3DXTRANSITION_EASEINEASEOUT);
}

//-----------------------------------------------------------------------------
// Desc: 
//-----------------------------------------------------------------------------
float Model::TransitionToAnimation(ANIMATIONHANDLE animation, float transitionTime, float weight, float startAtTime, float playSpeed)
{
    if(!m_AnimationController || animation == -1 || Engine::Instance()->IsDedicated())
        return 0;
	
	D3DXTRACK_DESC pDesc;
	m_AnimationController->GetTrackDesc(animation,&pDesc);

    D3DXTRACK_DESC desc;
    desc.Priority = D3DXPRIORITY_HIGH;
    desc.Weight = pDesc.Weight; // Start from current weighting, will ramp up to destination weighting (default 100%) over a second or so
    desc.Speed = playSpeed;		// Reset the speed to 100%

	if(IsLooping(animation) && IsPlaying(animation))
		desc.Position = pDesc.Position;  // Already playing, just continue onwards from the current position
    else
		desc.Position = startAtTime;  // Start the animation now, from the start time (default 0)

    desc.Enable = true; // Enable track

    m_AnimationController->UnkeyAllTrackEvents(animation);

    m_AnimationController->SetTrackDesc(animation,&desc);

    tracks[animation].endTime = m_AnimationController->GetTime()+tracks[animation].duration/playSpeed;

    if(curAnimTrack == -1 || (animation == curAnimTrack)){
        // Instant
        m_AnimationController->KeyTrackWeight(animation, weight,m_AnimationController->GetTime(), 0, D3DXTRANSITION_LINEAR);
    }
    else{
        // Slerp over 'transitionTime' seconds
		
		if(weight == 1)
			m_AnimationController->KeyTrackWeight(curAnimTrack, 1-weight,m_AnimationController->GetTime(), transitionTime, D3DXTRANSITION_EASEINEASEOUT);

        m_AnimationController->KeyTrackWeight(animation, weight,m_AnimationController->GetTime(), transitionTime, D3DXTRANSITION_EASEINEASEOUT);
        //don't add transition time, since it doesn't actually increase the time the animation takes
        //tracks[animation].endTime += transitionTime;
    }
    curAnimTrack = animation;

    tracks[curAnimTrack].bEverStarted = true;
    return tracks[curAnimTrack].duration;
}


//-----------------------------------------------------------------------------
// Desc: 
//-----------------------------------------------------------------------------
bool Model::IsPlaying(){
    return IsPlaying(curAnimTrack);
}

//-----------------------------------------------------------------------------
// Desc: 
//-----------------------------------------------------------------------------
bool Model::IsPlaying(ANIMATIONHANDLE animation)
{
	 if(!m_AnimationController || Engine::Instance()->IsDedicated())
        return 0;

    if(animation == -1)
        return false;

    if(tracks[animation].bLooping && tracks[animation].bEverStarted) // Looping animations are always playing
        return true;

    return (m_AnimationController->GetTime() < tracks[animation].endTime);
}

//-----------------------------------------------------------------------------
// Desc: 
//-----------------------------------------------------------------------------
ANIMATIONHANDLE Model::GetCurrentAnimation(){
    if(!IsPlaying() || Engine::Instance()->IsDedicated())
        return -1;
    else
        return curAnimTrack;
}

//-----------------------------------------------------------------------------
// Desc: 
//-----------------------------------------------------------------------------
void Model::UnloadAllAnimations(){
    LPD3DXANIMATIONSET set[100];
    for(int i=0;i<m_AnimationController->GetNumAnimationSets();i++){
        m_AnimationController->GetAnimationSet(i,&set[i]);
    }

    int num = m_AnimationController->GetNumAnimationSets();
    for(int i=0;i<num;i++){
        m_AnimationController->UnregisterAnimationSet(set[i]);
    }

    curAnimTrack = -1;
}


static LPCSTR	GetName(ID3DXFileData* data)
{
    DWORD nameLen = 0;
    data->GetName(NULL, &nameLen);	// 必要な長さをもらう
    if(nameLen > 0)
    {
        CHAR* name = new CHAR[nameLen+1];
        data->GetName(name, &nameLen);
        return name;
    }
    else
        return NULL;
}

static inline void	ReleaseName(LPCSTR name)
{
    delete[] name;
}

HRESULT Loader::LoadXFileAnimationKey(ID3DXFileData* data, Animation* anim)
{
    struct AnimationKey
    {
        DWORD	keyType;
        DWORD	numKeys;
        union
        {
            struct VectorKey
            {
                DWORD time;
                DWORD size;
                FLOAT x, y, z;
            } v[1];
            struct QuaternionKey
            {
                DWORD time;
                DWORD size;
                FLOAT w, x, y, z;	// D3DXQUATERNION
            } q[1];
            struct MatrixKey
            {
                DWORD time;
                DWORD size;
                FLOAT matrix[16];
            } m[1];
        };
    };
    DWORD size;
    AnimationKey* xkey;
    data->Lock(&size, (LPCVOID*)&xkey);

    switch(xkey->keyType)
    {
    case 0:	// Rotation
        {
            assert(size == 8 + (2 + 4) * 4 * xkey->numKeys);
            anim->rotations.resize(anim->rotations.size() + xkey->numKeys);
            for(DWORD i = 0; i < xkey->numKeys; i++)
            {
                anim->rotations[i].Time = xkey->q[i].time;
                anim->rotations[i].Value.x =  xkey->q[i].x;
                anim->rotations[i].Value.y =  xkey->q[i].y;
                anim->rotations[i].Value.z =  xkey->q[i].z;
                anim->rotations[i].Value.w = -xkey->q[i].w; // Flip quaternion w
            }
            return S_OK;
        }
    case 1:	// Scale
        {
            assert(size == 8 + (2 + 3) * 4 * xkey->numKeys);
            anim->scalings.resize(anim->scalings.size() + xkey->numKeys);
            for(DWORD i = 0; i < xkey->numKeys; i++)
            {
                anim->scalings[i].Time = xkey->v[i].time;
                anim->scalings[i].Value.x = xkey->v[i].x;
                anim->scalings[i].Value.y = xkey->v[i].y;
                anim->scalings[i].Value.z = xkey->v[i].z;
            }
            return S_OK;
        }
    case 2:	// Translation
        {
            assert(size == 8 + (2 + 3) * 4 * xkey->numKeys);
            anim->translations.resize(anim->translations.size() + xkey->numKeys);
            for(DWORD i = 0; i < xkey->numKeys; i++)
            {
                anim->translations[i].Time = xkey->v[i].time;
                anim->translations[i].Value.x = xkey->v[i].x;
                anim->translations[i].Value.y = xkey->v[i].y;
                anim->translations[i].Value.z = xkey->v[i].z;
            }
            return S_OK;
        }
    case 4:	// Full SRT matrix
        {
            assert(size == 8 + (2 + 16) * 4 * xkey->numKeys);
            anim->matrices.resize(anim->matrices.size() + xkey->numKeys);
            for(DWORD i = 0; i < xkey->numKeys; i++)
            {
                anim->matrices[i].Time = xkey->m[i].time;
                memcpy(&anim->matrices[i], xkey->m[i].matrix, sizeof(FLOAT)*16);
            }
            return S_OK;
        }
    default:
        Error("Invalid key type found. Corrupt animation file possible.");
        return S_OK;
    }

    data->Unlock();
}

//
HRESULT Loader::LoadXFileAnimation(ID3DXFileData* data)
{
    HRESULT hr = S_OK;
    LPCSTR frameName = NULL;
    Animation* animation = new Animation;

    SIZE_T ObjectNum;
    data->GetChildren(&ObjectNum);

    for(SIZE_T i=0;i<ObjectNum;i++)
    {
        CComPtr<ID3DXFileData> pNextData;
        data->GetChild(i,&pNextData);

        GUID type;
        pNextData->GetType(&type);

        if(type == TID_D3DRMFrame)
        {	//
            assert(frameName == NULL);
            frameName = GetName(pNextData);
            animation->name = frameName;
        }
        else if(type == TID_D3DRMAnimationKey)
        {	//
            hr = LoadXFileAnimationKey(pNextData,animation);
            if FAILED(hr)
                break;
        }
    }

    animations.push_back(animation);

    ReleaseName(frameName);
    return hr;
}

//
HRESULT Loader::LoadXFileAnimationSet(ID3DXFileData* data)
{
    HRESULT hr = S_OK;
    // BeginAnimation()
    LPCSTR animeName = GetName(data);


    // Load all animations in the set. One animation per node

    SIZE_T ObjectNum;
    data->GetChildren(&ObjectNum);

    for(SIZE_T i=0;i<ObjectNum;i++)
    {
        CComPtr<ID3DXFileData> pNextData;
        data->GetChild(i,&pNextData);

        GUID type;
        pNextData->GetType(&type);

        if(type == TID_D3DRMAnimation)
        {
            if FAILED(hr = LoadXFileAnimation(pNextData))
                break;
        }
    }


    //
    // Create and populate an animation set from the animations read above
    //
    LPD3DXKEYFRAMEDANIMATIONSET animationSet;
    DXASSERT(D3DXCreateKeyframedAnimationSet(animeName,4800,bLooping?D3DXPLAY_LOOP:D3DXPLAY_ONCE,animations.size(),0,0,&animationSet));

    float largestTime = 0;
    for(int i=0;i<animations.size();i++){
        Animation* a = animations[i];

        // If we have matrix keys we must convert to SRT
        if(a->matrices.size())
        {
            assert(a->rotations.size() == 0 && a->translations.size() == 0 && a->scalings.size() == 0);

            a->rotations.resize(a->matrices.size());
            a->scalings.resize(a->matrices.size());
            a->translations.resize(a->matrices.size());
            for(int k=0;k<a->matrices.size();k++){
                Matrix tm = a->matrices[k].Value;

                tm.m3 *= fScale;

                // Copy trans
                memcpy(&a->translations[k].Value,&tm.m3,sizeof(Vector));
                // Copy scale
                tm.GetScales(a->scalings[k].Value.x,a->scalings[k].Value.y,a->scalings[k].Value.z);
                // Copy rotation
				D3DXQuaternionRotationMatrix(&a->rotations[k].Value,(D3DXMATRIX*)&(tm.Orthonormalized()));

                // Flip quat. Same result, but affine transformation reconstruction will break if we don't do this
                a->rotations[k].Value = D3DXQUATERNION(
                    -a->rotations[k].Value.x,
                    -a->rotations[k].Value.y,
                    -a->rotations[k].Value.z,
                    a->rotations[k].Value.w);

                // When deriving a quaternion from a matrix, the sign will typically flip at 180 degrees
                // The inverse of a quaternion is exactly the same rotation, EXCEPT when interpolating
                // for obvious reasons.
                // So if we see a sign flip, we must force it back, and flip w too, as it represents
                // the cosine of half the angle, and the angle has now inversed
                if(k){
                    if(D3DXQuaternionDot(&a->rotations[k].Value,&a->rotations[k-1].Value) < 0){
                        a->rotations[k].Value = D3DXQUATERNION(
                            -a->rotations[k].Value.x,
                            -a->rotations[k].Value.y,
                            -a->rotations[k].Value.z,
                            -a->rotations[k].Value.w);
                    }
                }

                a->translations[k].Time = a->scalings[k].Time = a->rotations[k].Time = a->matrices[k].Time;

                if(a->matrices[k].Time > largestTime)
                    largestTime = a->matrices[k].Time;
            }
        }
        else if(a->translations.size())
        {
            for(int k=0;k<a->translations.size();k++)
            {
                a->translations[k].Value *= fScale;

                if(a->translations[k].Time > largestTime)
                    largestTime = a->translations[k].Time;
            }
        }

        // TODO ASAP: Hook this up
        DWORD pAnimationIndex;

        if(AffectedBoneNames == NULL)
        {
            // NOTE: Not allowing scaling. We encode scaling inherently in vertex data to avoid
            // a host of problems.
            DXASSERT(animationSet->RegisterAnimationSRTKeys(a->name.c_str(),
                a->scalings.size(),a->rotations.size(),a->translations.size(),/*NULL*/
                (D3DXKEY_VECTOR3*)a->scalings.size()==0?0:&a->scalings[0],
                (D3DXKEY_QUATERNION*)a->rotations.size()==0?0:&a->rotations[0],
                (D3DXKEY_VECTOR3*)a->translations.size()==0?0:&a->translations[0],&pAnimationIndex));
        }
        else
        {
            //passed list of affected bones
            bool found = false;
            for(int l = 0; l < AffectedBoneNames->size(); l++)
            {
                if(a->name == (*AffectedBoneNames)[l])
                {
                    found = true;
                    break;
                }
            }
            if(found)
            {
                //affect this bone
                DXASSERT(animationSet->RegisterAnimationSRTKeys(a->name.c_str(),
                    a->scalings.size(),a->rotations.size(),a->translations.size(),/*NULL*/
                    (D3DXKEY_VECTOR3*)a->scalings.size()==0?0:&a->scalings[0],
                    (D3DXKEY_QUATERNION*)a->rotations.size()==0?0:&a->rotations[0],
                    (D3DXKEY_VECTOR3*)a->translations.size()==0?0:&a->translations[0],&pAnimationIndex));
            }
        }
        delete a;
    }
    animations.resize(0);

    AnimSet as;
    as.set = animationSet;
    as.duration = largestTime  / animationSet->GetSourceTicksPerSecond();
    animationSets.push_back(as);

    ReleaseName(animeName);
    return hr;
}


HRESULT Loader::LoadXFile(LPVOID pSource, DXFILELOADOPTIONS options)
{
    assert(m_pXFile);
    HRESULT hr = S_OK;
    CComPtr<ID3DXFileEnumObject> pEnumObj;


    hr = m_pXFile->CreateEnumObject(pSource, options, &pEnumObj);
    assert(hr != DXFILEERR_BADFILETYPE);
    assert(hr != DXFILEERR_BADFILEVERSION);
    assert(hr != DXFILEERR_BADRESOURCE);
    assert(hr != DXFILEERR_BADVALUE);
    assert(hr != DXFILEERR_FILENOTFOUND);
    if(FAILED( hr ))
        return hr;
    
    BOOL Parse;
    SIZE_T ObjectNum=0;

    pEnumObj->GetChildren(&ObjectNum);

    for(SIZE_T i=0;i<ObjectNum;i++)
    {
        CComPtr<ID3DXFileData> pData;
        pEnumObj->GetChild(i,&pData);
        GUID guid;
        pData->GetType(&guid);

        if(guid == TID_D3DRMAnimationSet)
            LoadXFileAnimationSet(pData);

    }

    return hr;
}

HRESULT	Loader::LoadFromFile(LPCSTR filename)
{
    D3DXFileCreate(&m_pXFile);
    assert(m_pXFile);
    RegisterTemplates(m_pXFile);

    HRESULT hr = LoadXFile(const_cast<LPSTR>(filename), DXFILELOAD_FROMFILE);

    SAFE_RELEASE(m_pXFile);
    return hr;
}

HRESULT Loader::RegisterTemplates(ID3DXFile* pXFile)
{
    HRESULT hr;
    // D3DRMテンプレートの登録
    hr = pXFile->RegisterTemplates(D3DRM_XTEMPLATES, D3DRM_XTEMPLATE_BYTES);
    if FAILED(hr)
        return hr;
    // X8のテンプレート登録 from ウマイハナシ
    CHAR szTemplates[] =
        "template XSkinMeshHeader {"
        "	<3cf169ce-ff7c-44ab-93c0-f78f62d172e2>"
        "	WORD nMaxSkinWeightsPerVertex;"
        "	WORD nMaxSkinWeightsPerFace;"
        "	WORD nBones;"
        "}"
        "template VertexDuplicationIndices {"
        "	<b8d65549-d7c9-4995-89cf-53a9a8b031e3>"
        "	DWORD nIndices;"
        "	DWORD nOriginalVertices;"
        "	array DWORD indices[nIndices];"
        "}"
        "template SkinWeights {"
        "	<6f0d123b-bad2-4167-a0d0-80224f25fabb>"
        "	STRING transformNodeName;"
        "	DWORD nWeights;"
        "	array DWORD vertexIndices[nWeights];"
        "	array FLOAT weights[nWeights];"
        "	Matrix4x4 matrixOffset;"
        "}";
    hr = pXFile->RegisterTemplates(szTemplates, sizeof(szTemplates));
    if FAILED(hr)
        return hr;
    return S_OK;
}











