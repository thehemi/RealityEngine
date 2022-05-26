//----------------------------------------------------------------------------------
// Compiler.cpp - All data manipulation and preparation is done here before
// being passed off to be saved or loaded
//
// FIXME: Compiling Actor hierarchies fails. We need to collapse .X hierarchies if
// they are intended to be saved/used in the world
//
//----------------------------------------------------------------------------------
#include <stdafx.h>
#include <MMSystem.h>
#include "PRTMesh.h"
#include "PRTSimulator.h"
#include "Compiler.h"
#include "GUISystem.h"
#include "Serializer.h"
#include "Editor.h"
#include "Collision.h"

//-----------------------------------------------------------------------------
CPRTSimulator    g_Simulator;

//-----------------------------------------------------------------------------
// Nearest power of 2
//-----------------------------------------------------------------------------
#define BITS_IN_ULONG (CHAR_BIT * sizeof(unsigned long))
unsigned long NearestPow2(unsigned long val)
{
    unsigned      b = BITS_IN_ULONG / 2;
    unsigned long q = 1lu<<b;
    unsigned long n = 0;
    /* one could do
    assert(q<<(b-1) != 0);
    here to abort if above premise does not hold
    */
    if (val < 2)
        return 1;

    while (b != 0) {
        b >>= 1;
        if (val <= q) {
            n = q;
            q >>= b;
        } else {   q <<= b;
        }
    }
    return n;
}

//-----------------------------------------------------------------------------
// Tweaks input options to what we need on this run, such as fast low quality
//-----------------------------------------------------------------------------
PRTSettings Compiler::TweakOptions(PRTSettings& o)
{
    PRTSettings pOptions = o;

    // Tweak for speed
    pOptions.dwNumRays		*= percentRays/100.f;
    pOptions.dwNumBounces	*= percentBounces/100.f;
    if(pOptions.dwNumBounces == 0)
        pOptions.dwNumBounces = 1;

    // Set these settings, they aren't set by default..
    pOptions.bSaveCompressedResults = true;
    pOptions.Quality = D3DXSHCQUAL_SLOWHIGHQUALITY;//bFast?D3DXSHCQUAL_FASTLOWQUALITY:D3DXSHCQUAL_SLOWHIGHQUALITY;

    if(pOptions.bPerPixel || bFast)
        pOptions.bAdaptive = false;

    return pOptions;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Compiler::FindFrameAndActor(string name, Actor*& actor, ModelFrame*& pFrame)
{
    for(int i=0;i<m_pWorld->m_Actors.size();i++){
        if(m_pWorld->m_Actors[i]->MyModel){
            ModelFrame* frame = m_pWorld->m_Actors[i]->MyModel->GetFrame(m_pWorld->m_Actors[i]->MyModel->GetNodeHandle(name));
            if(frame && frame->GetMesh()){
                actor = m_pWorld->m_Actors[i];
                pFrame = frame;
                return;
            }
        }
    }
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
bool Compiler::MeshNeedsRecompiling(Mesh* mesh, string file)
{
    // Only generate SH if we haven't already generated it for current mesh+timestamp
    bool bRecompile = true;
    // Extract file stamp
    HANDLE hFile = CreateFile(file.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);  
    if(hFile == INVALID_HANDLE_VALUE)
        return true; // File doesn't exist, so yes we must create it!

    FILETIME writeTime, moveTime, modifyTime;
    GetFileTime(hFile,0,0,&writeTime);
    CloseHandle(hFile);
    // Compare
    SystemTimeToFileTime(&mesh->m_TimeMoved,&moveTime);
    SystemTimeToFileTime(&mesh->m_TimeModified,&modifyTime);
    if(CompareFileTime(&writeTime,&moveTime) == 1 && CompareFileTime(&writeTime,&modifyTime) == 1)
        return false; // Compiled file is newer, don't recompile

    return true;
}



#ifdef GAME_EXPORTS
#define GAME_API __declspec(dllexport)
#else
#define GAME_API __declspec(dllimport)
#endif
extern "C" {          // we need to export the C interface
    void GAME_API Game_Tick();
}

//----------------------------------------------------------------------------------
string Compiler::GetFile(string objectName, string ext)
{
    string parentFile;
    if(m_pWorld)
        parentFile = m_pWorld->GetFileName();
    else if(m_pModel)
        parentFile = m_pModel->m_FileName;

    assert(parentFile.length());

    // Read PRT filename via MapName_Data\MeshName.prt
    string world = parentFile.substr(0,parentFile.find_last_of("."));
    if(world.find_last_of("\\") != -1)
        world = world.substr(0,world.find_last_of("\\"));
    world += "\\" + objectName + ext;
    return world;
}

bool bCompiling = false;

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
bool Compiler::CompileMesh(ModelFrame* frame)
{
    if(bCompiling)
    {
        SeriousWarning("Re-entrant compiler. Tell tim@artificialstudios.com. Frame=%s",frame->Name.c_str());
        return false;
    }

    ResetCurrentDirectory();

    Mesh* mesh = frame->GetMesh();
    Matrix worldTM = frame->CombinedTransformationMatrix;

    // Remove scaling and rotation for prefabs
    // 1. Scaling is evil for everything
    // 2. Rotation is bad because it requires instancing the PRT coefficients for each mesh
    Matrix tm;
    // Only remove (bake) rotation for Prefabs. 
    // Dynamic Actors may have a Rotation that determines functionality
    // For example, tokamak objects NEED their rotations not to be baked
    if(m_CurrectActor && m_CurrectActor->StaticObject)
        tm = m_CurrectActor->Rotation;
    else if(m_CurrectActor)// Just remove scaling for non-prefabs...
        tm = m_CurrectActor->Rotation.GetScaleMatrix();


    if(mesh && !tm.IsIdentity())
    {
        tm[3] = Vector();
        m_CurrectActor->BakedTM *= tm;
        MeshOps::Convert(mesh,MeshOps::BakeTM,tm);

        if(frame->collisionMesh)
        {
            frame->collisionMesh->Destroy();
            frame->collisionMesh->Initialize(mesh);
        }
    }

    if(mesh && !Engine::Instance()->IsDedicated())
    {
        // Just load PRT if not recompiling
        if(mesh->m_SHOptions.Enabled && !bCompilePRT){
            mesh->LoadPRT(GetFile(frame->Name,".prt"));
        }

        // Check for a lightmap file
        if(!mesh->m_LightMap && !mesh->m_SHOptions.Enabled)
        {
            string file = GetFile(frame->Name,".dds");
            if(FileExists(file.c_str()))
            {
                mesh->m_LightMap = new Texture;
                mesh->m_LightMap->m_bAllowExternalPath = true;
                mesh->m_LightMap->Load(file);
            }
        }
        else if(mesh->m_SHOptions.Enabled)
            SAFE_DELETE(mesh->m_LightMap);

        // Run PRT now if enabled
        if(mesh->m_SHOptions.Enabled && bCompilePRT && !Engine::Instance()->IsDedicated())
        {

            // Needs recompiling if node mesh, or any blockers are modified
            string file = GetFile(frame->Name,".prt");

            // Make sure directory exists
            string dir = file.substr(0,file.find_last_of("\\"));
            CreateDirectory(dir.c_str(),0);

            bool bRecompile = m_bForceRecompile;
            if(MeshNeedsRecompiling(mesh,file))
                bRecompile = true; 

            // Find inBlocker list
            vector<LPD3DXMESH>	inBlockers;
            vector<Matrix>		inBlockMats;

            for(int i=0;i<mesh->m_SHOptions.InBlockers.size();i++){
                ModelFrame* iFrame = NULL;
                Actor*		pActor = NULL;
                FindFrameAndActor(mesh->m_SHOptions.InBlockers[i], pActor, iFrame);
                if(!iFrame){ // Must be outdated blocker list
                    //bRecompile = true;
                    continue;
                }

                if(iFrame->GetMesh() == mesh)
                    continue; // Can't block self!!

                // See if blocker has moved, causing recompile
                // We compare blocker date with our PRT file date
                if(!bRecompile && MeshNeedsRecompiling(iFrame->GetMesh(),file))
                    bRecompile = true; 

                for(int x=0;x<inBlockers.size();x++)
                    if(inBlockers[x] == iFrame->GetMesh()->GetHardwareMesh())
                        SeriousWarning("WHAT THE HOLY JESUS? CALL AN EXORCIST WE HAVE A GHOST IN THE MACHINE");

                if(iFrame->GetMesh())
                    inBlockers.push_back(iFrame->GetMesh()->GetHardwareMesh());
                else
                    continue;

                // Put blocker into object space of receiver mesh
                Matrix objectSpace = iFrame->CombinedTransformationMatrix * worldTM.Inverse();
                inBlockMats.push_back(objectSpace);
            }

            // TODO: Out blockers!!

            if(bRecompile)
            {
                if(mesh->m_SHOptions.bPerPixel)
                    LogPrintf("Generating Per-Pixel PRT for %s ...",frame->Name.c_str());
                else
                    LogPrintf("Generating Per-Vertex PRT for %s ...",frame->Name.c_str());

                // Concanceate all blocker meshes into a single blocker we can pass to simulator
                LPD3DXMESH pCombinedMesh = NULL;
                if(inBlockers.size())
                {
                    // Note: Combined mesh is made 32-bit
                    DXASSERT(D3DXConcatenateMeshes(&inBlockers[0],inBlockers.size(),D3DXMESH_MANAGED|D3DXMESH_32BIT,
                        (D3DXMATRIX*)&inBlockMats[0],NULL,NULL,RenderWrap::dev,&pCombinedMesh));
                }

                // HACK: Workaround for DX9.2004 Bug where D3DXConcatenateMeshes() holds lock!
                for(int i=0;i<inBlockers.size();i++){
                   // inBlockers[i]->UnlockVertexBuffer();
                   // inBlockers[i]->UnlockIndexBuffer();
                }


                LPD3DXMESH pSrcMesh = mesh->GetHardwareMesh();
                bool bDeleteSrcMesh	= false;
                //
                // Flip coordinates temporarily so per-pixel uses the correct channel
                //
                int nB = pSrcMesh->GetNumBytesPerVertex();
                int nB2 = sizeof(VertexT2);
                if(mesh->m_SHOptions.bPerPixel){
                    if(nB < nB2){
                        SeriousWarning("Per-Pixel PRT requires two texture coordinate sets, where the second is the [0,1] mapping. %s appears to have one texture coordinate set only",frame->Name.c_str());
                        SAFE_RELEASE(pCombinedMesh);
                        return false;
                    }
                    else if(nB > nB2)
                    {
                        //
                        // Argh, we need to implement "Texture mapping progressive meshes. H. Hoppes" to do this automatically
                        //
                        SeriousWarning("Once you have generated per-vertex PRT data, the second UV set required for per-pixel PRT is lost. You must re-import your source mesh with two UV sets to use per-pixel. Mesh is '%s'",frame->Name.c_str());
                        SAFE_RELEASE(pCombinedMesh);
                        return false;
                    }

                    //
                    // Clone a temp mesh with special coords to generate per-pixel data
                    //
                    bool b32Bit = pCombinedMesh&&(pCombinedMesh->GetOptions()&D3DXMESH_32BIT);
                    LPD3DXMESH newMesh;
                    DXASSERT(pSrcMesh->CloneMesh(D3DXMESH_MANAGED|(b32Bit?D3DXMESH_32BIT:0),VertexFormats::Instance()->FindFormat(sizeof(VertexT2))->element,RenderWrap::dev,&newMesh));

                    BYTE* VerticesBuffer;
                    newMesh->LockVertexBuffer(0, (LPVOID*)&VerticesBuffer);
                    int verts = newMesh->GetNumVertices();
                    for(int i=0;i<verts;i++){
                        VertexT2* v = (VertexT2*)&VerticesBuffer[i*sizeof(VertexT2)];
                        // Flip coordinates
                        Vector2 t0 = v->tex[0];
                        v->tex[0] = v->tex[1];
                        v->tex[1] = t0;
                        if(v->tex[0].x > 1 || v->tex[0].y > 1 || v->tex[0].x < 0 || v->tex[0].y < 0){
                            SeriousWarning("Mesh '%s' UVs at Channel #2 are outside [0,1] range. Must be [0,1] for per-pixel PRT.",frame->Name.c_str());
                            SAFE_RELEASE(pCombinedMesh);
                            return false;
                        }
                    }
                    newMesh->UnlockVertexBuffer();
                    pSrcMesh = newMesh;
                    bDeleteSrcMesh = true;
                }

                //
                // If blocker has 32-bit vertices, so must the source mesh.
                //
                bool bRevertSourceMesh = false;
                if(pCombinedMesh && pCombinedMesh->GetNumVertices() > 65535 && !(pSrcMesh->GetOptions() & D3DXMESH_32BIT) && !bDeleteSrcMesh)
                {
                    D3DVERTEXELEMENT9 decl[32];
                    pSrcMesh->GetDeclaration (decl);
                    LPD3DXMESH newMesh;
                    DXASSERT(pSrcMesh->CloneMesh(D3DXMESH_MANAGED|D3DXMESH_32BIT,decl,RenderWrap::dev,&newMesh));
                    //SAFE_RELEASE(pSrcMesh);
                    pSrcMesh = newMesh;
                    bRevertSourceMesh = true;
                    bDeleteSrcMesh = true;
                }


                CPRTMesh prtMesh;
                pSrcMesh->AddRef(); // Stop PRTMesh from destroying source mesh!
                prtMesh.SetMesh(RenderWrap::dev,pSrcMesh);
                //
                // Load normal map_PRT.dds
                //
                string prtTex;
                for(int i=0;i<mesh->m_Materials[0]->m_Parameters.size();i++){
                    ShaderVar* v = mesh->m_Materials[0]->m_Parameters[i];
                    if(v->type == EditorVar::TEXTURE){
                        Texture* t = (Texture*)v->data;
                        if(t->type == TT_NORMALMAP){
                            prtTex = t->filename.substr(0,t->filename.find_last_of("."));
                            prtTex += "_PRT.dds";
                            break;
                        }
                    }
                }
                Texture t;
                if(prtTex.length() && prtTex.find("NoBump_PRT.dds") == -1){
                    if(t.Load(prtTex,TT_NORMALMAP))
                        prtMesh.m_pNormalMap = t.GetTexture();
                }

                PRTSettings pOptions = TweakOptions(mesh->m_SHOptions);
                if(pOptions.dwNumRays < 5){
                    SeriousWarning("Your mesh %s has only %d SH rays. The normal number of rays is 1024.\nCheck 1)Under Scene Settings you don't have Global Percentage set to 0% or something very low 2) That your object settings do in fact have a high number of rays, and that you have not got rays and bounces mixed up!"
                        ,frame->Name.c_str(),pOptions.dwNumRays);
                }

                // Set SSS scale
                if(pOptions.fLengthScale == 25)
                    pOptions.fLengthScale = 1;

                pOptions.fLengthScale *= (mesh->m_LocalBox.max -  mesh->m_LocalBox.min).Length()*500;
                // Round texture size to pow2
                pOptions.dwTextureSize = NearestPow2(pOptions.dwTextureSize);

                // Receiver group will be filename
                wcscpy( pOptions.strResultsFile, ToUnicode(file).c_str() );

                // Currently compiled mesh can't render, so set the flag
                mesh->m_bDontRender = true;

                bCompiling = true;

                GUISystem::Instance()->ShowDesktop(true);
                RenderDevice::Instance()->ShowCursor(true,false);
                LogPrintf("Starting compile thread for %s...",frame->Name.c_str());
                // Run!!
                g_Simulator.Run(RenderWrap::dev,&pOptions,&prtMesh,pCombinedMesh);



                // Use this as an opportunity to
                // process windows messages while deep in processing
                while(g_Simulator.IsRunning()){
                    CHAR sz[256];
                    if( g_Simulator.GetPercentComplete() >= 0.0f )
                        sprintf( sz, "Step %d of %d: %0.1f%% done", g_Simulator.GetCurrentPass(), g_Simulator.GetNumPasses(), g_Simulator.GetPercentComplete() );
                    else
                        sprintf( sz, "Step %d of %d (progress n/a)", g_Simulator.GetCurrentPass(), g_Simulator.GetNumPasses() );

                    Editor::Instance()->SetCompiling(true,ToAnsi(g_Simulator.GetCurrentPassName())+" for "+frame->Name,sz);

                    if (m_Callback)
                    {
                        if(m_Callback->OnCallback())
                        {
                            g_Simulator.Stop();
                            break;
                        }
                    }

                    //GUISystem::Instance()->DoMessageStrip();

                    Sleep(200);
                    // Needed for progress updates AFAIK
                    //if(Editor::Instance()->GetEditorMode())
                    //    Engine::Instance()->Game_Tick();
                    //if(Editor::Instance()->Callback)
                    //	Editor::Instance()->Callback();
                }
                bCompiling = false;

                mesh->m_bDontRender = false;
                LogPrintf("..done");


                GUISystem::Instance()->m_Strip->SetVisible(false);
                GUISystem::Instance()->ShowDesktop(false);
                Editor::Instance()->SetCompiling(false);

                m_bCompiling = true;

                if(g_Simulator.m_bFailed)
                    SeriousWarning(("Compiling Aborted/Failed: "+string(g_Simulator.strError)).c_str());

                m_bCompiling = false;
                SAFE_RELEASE(pCombinedMesh);

                LogPrintf("done!\n");

                // Update source mesh
                /* if(bRevertSourceMesh)
                {
                // Copy into non-32bit mesh
                D3DVERTEXELEMENT9 decl[32];
                pSrcMesh->GetDeclaration (decl);
                LPD3DXMESH newMesh;
                DWORD flags = D3DXMESH_MANAGED;
                if(pSrcMesh->GetNumVertices() >= 65535)
                flags |= D3DXMESH_32BIT;
                DXASSERT(pSrcMesh->CloneMesh(flags,decl,RenderWrap::dev,&newMesh));

                mesh->SetMesh(newMesh,false);
                }*/

                if(bDeleteSrcMesh)
                {
                    assert(mesh->GetHardwareMesh() != pSrcMesh);
                    SAFE_RELEASE(pSrcMesh);
                }

                // Get the result mesh, in case of adaptive tessellation
                if(mesh->m_SHOptions.bAdaptive){
                    mesh->SetMesh(prtMesh.GetMesh());
                }
            }
            mesh->LoadPRT(GetFile(frame->Name,".prt"));
        }
    }
    return true;
}

//----------------------------------------------------------------------------------
// Desc: Pre-processes imported meshes and items
// TODO: Apply scaling factor to all items
//----------------------------------------------------------------------------------
bool Compiler::CompileFrame(ModelFrame* frame){
    if(frame->pFrameFirstChild)
        if(!CompileFrame(frame->pFrameFirstChild))
            return false;
    if(frame->pFrameSibling)
        if(!CompileFrame(frame->pFrameSibling))
            return false;

    if(frame->GetMesh())
        CompileMesh(frame);

    return true;
}

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
bool Compiler::CompileActor(Actor* actor)
{
    m_CurrectActor = actor;
    if(actor->MyModel && actor->MyModel->m_pFrameRoot)
    {
        if(!CompileFrame(actor->MyModel->m_pFrameRoot))
            return false;

        // We collapse rotation for directional SH, so clear matrix here, once all frames have baked it
        // Only do this for Prefabs, dynamic Actors may have a Rotation that determines functionality
        if(m_CurrectActor->StaticObject)
            m_CurrectActor->Rotation = Matrix();
        // However, still strip scaling for dynamic actors
        else
            m_CurrectActor->Rotation = m_CurrectActor->Rotation.GetRotationMatrix();

        // Must update, as compile may be just before a save or on a static object
        m_CurrectActor->MyModel->SetTransform(m_CurrectActor->Rotation,m_CurrectActor->Location);
    }
    return true;
}

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void Compiler::SetProgress(int percent)
{

}

//--------------------------------------------------------------------------------------
// Recompile specific actors. Forces compile on objects, even if apparently up-to-date
//--------------------------------------------------------------------------------------
bool Compiler::CompileActors(std::vector<Actor*> actors)
{
    m_bForceRecompile = true;
    m_pWorld = actors[0]->MyWorld;
    bool ret = true;
    for(int i=0;i<actors.size();i++){
        if(!CompileActor(actors[i])){
            ret = false;
        }
    }
    return ret;

}
//--------------------------------------------------------------------------------------
// Desc: Compiles a model
//--------------------------------------------------------------------------------------
bool Compiler::CompileModel(Model* model){
    m_pModel = model;
    return CompileFrame(model->m_pFrameRoot);
}

//--------------------------------------------------------------------------------------
// Desc: Compiles a world including scene tree
//--------------------------------------------------------------------------------------
bool Compiler::CompileWorld(World* world){

    m_pWorld = world;
    LogPrintf("Compiling...\n");
    for(int i=0;i<world->m_Actors.size();i++){
        if(!CompileActor(world->m_Actors[i]))
            return false;
    }

    bool ret = true;//Serializer::Instance()->SaveWorld(world->m_FileName,world,false);
    LogPrintf("Compiled\n");
    return ret;
}


Compiler::~Compiler(){
}























