//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
// Animated Day/Night Sky
//
//
// TODO: Get single-pass sun working properly (correct timings, correct angle, correct size)
//===================================================================================

#include "stdafx.h"
#include "SkyController.h"
#include "NetworkClient.h"
//#include "RainFXsys.h"
#include <d3d9.h>
#include "Python.h"
#include "HDR.h"
#include "IndoorVolume.h"

#define ENV_FVF (D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0))
#define FVF_LVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE |D3DFVF_TEX1)
typedef struct ENV_VERTEX
{
    Vector m_vecPos;
    Vector2 m_vecTex;
} ENV_VERTEX;
 
//statics
ENGINE_API SkyController* SkyController::Instance = NULL;
Script* SkyController::m_ControllerScript = NULL;
vector<BBox> SkyController::InteriorVolumes;
ENV_VERTEX vert[4];


//--------------------------------------------------------------------------------------
// Creates a new SkyController from the specified Script file and "Map.ini" file. 
// RetainTime == true will have the new sky start at the same time as the old sky, if any, for sky reload
//--------------------------------------------------------------------------------------
void SkyController::IntitializeNewSky(World* world,string ScriptClassName, bool RetainTime)
{
    float prevDayTime = -1;
    // delete any previous skycontroller, can only have one at a time
    if(Instance)
    {
        // store the previous sky's time in case we want to set back to that time for sky reloading
        prevDayTime = Instance->GetDayTimeMinutes();
        delete Instance;
    }

    if(!ScriptClassName.size())
        return;
    //Error("Need 'data\\%s.ini' for this map in order to use SkyController in %s.",g_GameState.m_CurrentMap.substr(0,g_GameState.m_CurrentMap.length()-4).c_str(),g_GameState.m_CurrentMap.c_str());

    string ScriptFile = ScriptClassName + ".py";

    if(!FindMedia(ScriptFile,"Scripts"))
        SeriousWarning("SkyController script '%s.py' not found in your Scripts directory! Sky will not appear correctly without the script.",ScriptClassName.c_str());

    // create a new script controller according to the specified classname
    m_ControllerScript = new Script();
    m_ControllerScript->Create(ScriptClassName,ScriptClassName);

    // only create the SkyController NetworkActor itself if the Server. 
    // Clients will receive the synchronized NetworkActor from the Server when they joins the game.
    if(world->m_IsServer)
        new SkyController(world);

    // if we want to retain the previous sky's time (for sky reloading), set to that time now
    if(RetainTime && prevDayTime != -1)
        Instance->SetDayTime(prevDayTime);
}

//--------------------------------------------------------------------------------------
//  Resets the core Sky State, starting the Sky back at its initial time with its initial fader values
//--------------------------------------------------------------------------------------
void SkyController::ResetSkyState()
{
    m_Server_LastSentDayTime = -BIG_NUMBER;
    m_SkyState = 0;
    m_WeatherState = 0;
    //time starts at midnight
    DayTimeMinutes = 0;
    // clear all faders
    for(int i = 0; i < NUM_SKY_FADES; i++)
    {
        SkyBGMixFades[i].Active = false;
        SkyBGMix[i] = 0;
    }
    m_FadeFog = false;
    // sets up the initial (midnight) sky state within the script
    PyObject_CallMethod(m_ControllerScript->classObj,"InitSkyMidnightState",NULL);
}

//--------------------------------------------------------------------------------------
// Server-side generates replication of initial spawn state for all Clients. 
// Actors inheriting from NetworkActor can override this if they have custom state information to replicate.
//--------------------------------------------------------------------------------------
void SkyController::Server_MakeSpawnMessages()
{
    // write time of day to all networkclients
    WriteMessageType(MSGID_SKYCONTROLLER_TIME,PACKET_SPAWN);
    WriteFloat(DayTimeMinutes,PACKET_SPAWN);

    // write speed of day cycle (minutes per game second) to all networkclients
    WriteMessageType(MSGID_SKYCONTROLLER_MINUTES_SPEED_SCALE,PACKET_SPAWN);
    WriteFloat(SkyMinutesPerGameSecond,PACKET_SPAWN);
}

//--------------------------------------------------------------------------------------
// Server-side generates replication of full state for newly-joining Clients
//--------------------------------------------------------------------------------------
NetworkActorPackets* SkyController::Server_MakeOnJoinSynchMessages(NetworkClient* client)
{
    NetworkActorPackets* packets = client->GetOrCreateNetworkActorPackets(this);

    // write time of day to a particular networkclient
    WriteMessageType(&packets->m_Packets[PACKET_SPAWN],MSGID_SKYCONTROLLER_TIME);
    WriteFloat(&packets->m_Packets[PACKET_SPAWN],DayTimeMinutes);

    // write speed of day cycle to a particular networkclient
    WriteMessageType(&packets->m_Packets[PACKET_SPAWN],MSGID_SKYCONTROLLER_MINUTES_SPEED_SCALE);
    WriteFloat(&packets->m_Packets[PACKET_SPAWN],SkyMinutesPerGameSecond);

    return packets;
}

//--------------------------------------------------------------------------------------
// Processes a spawn-state message
//--------------------------------------------------------------------------------------
void SkyController::Client_HandleSpawnMessage(MessageType message, ReadPacketBuffer* packetBuffer)
{
    switch(message)
    {
        //read and set time of day
    case MSGID_SKYCONTROLLER_TIME:
        SetDayTime(ReadFloat(packetBuffer));
        break;
        //read and set day cycle speed
    case MSGID_SKYCONTROLLER_MINUTES_SPEED_SCALE:
        SetMinutesPerGameSecond(ReadFloat(packetBuffer));
        break;

    default:
        NetworkActor::Client_HandleSpawnMessage(message,packetBuffer);
    }
}

//--------------------------------------------------------------------------------------
//  Processes a state-update replication message
//--------------------------------------------------------------------------------------
void SkyController::Client_HandleTickMessage(MessageType message, ReadPacketBuffer* packetBuffer)
{
    switch(message)
    {
    case MSGID_SKYCONTROLLER_TIME:
        {
            //only set time of day if there is a discrepancy of more than 20 min between Client & Server
            float time = ReadFloat(packetBuffer);
            if(fabsf(time - DayTimeMinutes) > 20.f)
                SetDayTime(time);
            break;
        }
        //read and set day cycle speed
    case MSGID_SKYCONTROLLER_MINUTES_SPEED_SCALE:
        SetMinutesPerGameSecond(ReadFloat(packetBuffer));
        break;

    default:
        NetworkActor::Client_HandleTickMessage(message,packetBuffer);
    }
}

//--------------------------------------------------------------------------------------
// Iterates through Clients and calls Server_MakeTickMessages() to replicate changes in state
//--------------------------------------------------------------------------------------
void SkyController::Server_NetworkTick()
{
    if(GSeconds - m_Server_LastSentDayTime > 30.f)
    {
        m_Server_LastSentDayTime = GSeconds;
        //send time of day
        WriteMessageType(MSGID_SKYCONTROLLER_TIME);
        WriteFloat(DayTimeMinutes);
    }

    NetworkActor::Server_NetworkTick();
}

//--------------------------------------------------------------------------------------
// Sets the current sky day time, will jump the sky instantly to the new day time
//--------------------------------------------------------------------------------------
void SkyController::SetDayTime(float NewDayTimeMinutes)
{
    if(!m_ControllerScript)
        return;

    ResetSkyState();

    while(DayTimeMinutes < NewDayTimeMinutes)
    {
        float DeltaMinutes = 10;
        DayTimeMinutes += DeltaMinutes;

        if(DayTimeMinutes > NewDayTimeMinutes)
        {
            DeltaMinutes -= (DayTimeMinutes - NewDayTimeMinutes);
            DayTimeMinutes = NewDayTimeMinutes;
        }

        m_ControllerScript->Tick();

        UpdateFades(DeltaMinutes/SkyMinutesPerGameSecond);
    }
    if(MyWorld->m_IsServer)
    {
        m_Server_LastSentDayTime = GSeconds;
        //send time of day
        WriteMessageType(MSGID_SKYCONTROLLER_TIME);
        WriteFloat(DayTimeMinutes);
    }
}

//--------------------------------------------------------------------------------------
// Begins interpolation of current fog values to destination values over the course of a number of Sky Minutes
// Using 0 for MinutesDuration will result in an instant Set of the destination values.
//--------------------------------------------------------------------------------------
void SkyController::FadeFog(float newDensity, float newRed,float newGreen, float newBlue, float MinutesDuration)
{
    m_FadeFog = false;

    if(!MinutesDuration)
    {
        FogDensity = newDensity;
        FogRed = newRed;
        FogGreen = newGreen;
        FogBlue = newBlue;
    }
    else
    {
        m_FadeFog = true;
        FadeFogTotalTime = MinutesDuration/SkyMinutesPerGameSecond;
        FadeFogCurTime = 0;
        FadeFogRedStep = newRed - FogRed;
        FadeFogGreenStep = newGreen - FogGreen;
        FadeFogBlueStep = newBlue - FogBlue;
        FadeFogDensityStep = newDensity - FogDensity;
    }
}

//--------------------------------------------------------------------------------------
// Begins interpolation of the specified Sky Fader index float (controlling sky blend and color values)
// to a destination value over the course of a number of Sky Minutes
// Using 0 for MinutesDuration will result in an instant Set of the destination values.
//--------------------------------------------------------------------------------------
void SkyController::FadeSky(int SkyIndex,float Dest,float MinutesDuration)
{
    SkyBGMixFades[SkyIndex].Active = false;

    if(!MinutesDuration)
        SkyBGMix[SkyIndex] = Dest;
    else
    {
        SkyBGMixFades[SkyIndex].Active = true;
        SkyBGMixFades[SkyIndex].TotalTime = MinutesDuration/SkyMinutesPerGameSecond;
        SkyBGMixFades[SkyIndex].TotalStep = Dest - SkyBGMix[SkyIndex];
        SkyBGMixFades[SkyIndex].CurTime = 0;
    }

}

//--------------------------------------------------------------------------------------
// Updates the Sky's core logic every frame
//--------------------------------------------------------------------------------------
void SkyController::Tick()
{
    DayTimeMinutes += GDeltaTime*SkyMinutesPerGameSecond;
    if(DayTimeMinutes > 1440)
        DayTimeMinutes -= 1440;

    UpdateFades(GDeltaTime);
    NetworkActor::Tick();
    m_ControllerScript->Tick();
}

//--------------------------------------------------------------------------------------
// Updates the interpolation of the Sky faders 
//--------------------------------------------------------------------------------------
void SkyController::UpdateFades(float TimeInterval)
{
    for(int i = 0; i < NUM_SKY_FADES; i++)
    {
        if(SkyBGMixFades[i].Active)
        {
            float DeltaTime = TimeInterval;
            SkyBGMixFades[i].CurTime += TimeInterval;
            if(SkyBGMixFades[i].CurTime >= SkyBGMixFades[i].TotalTime)
            {
                SkyBGMixFades[i].Active = false;
                //subtract overrun
                DeltaTime -= (SkyBGMixFades[i].CurTime - SkyBGMixFades[i].TotalTime);
            }
            SkyBGMix[i] += SkyBGMixFades[i].TotalStep * (DeltaTime/SkyBGMixFades[i].TotalTime);
        }
    }

    if(m_FadeFog)
    {
        float DeltaTime = TimeInterval;
        FadeFogCurTime += TimeInterval;
        if(FadeFogCurTime >= FadeFogTotalTime)
        {
            m_FadeFog = false;
            //subtract overrun
            DeltaTime -= (FadeFogCurTime - FadeFogTotalTime);
        }

        FogRed += FadeFogRedStep * (DeltaTime/FadeFogTotalTime);
        FogGreen += FadeFogGreenStep * (DeltaTime/FadeFogTotalTime);
        FogBlue += FadeFogBlueStep * (DeltaTime/FadeFogTotalTime);
        FogDensity += FadeFogDensityStep * (DeltaTime/FadeFogTotalTime);
    }

    MyWorld->SetFog(FloatColor(FogRed,FogGreen,FogBlue,1.f),FogDensity);
    //MyWorld->SetFog(FloatColor(FogRed,FogGreen,FogBlue,1.f),FogDensity*LODManager::Instance()->VisibleRange*LODManager::Instance()->VisibleRange);
}

//--------------------------------------------------------------------------------------
// Draws the Sky Time HUD element if DisplayTime == true
//--------------------------------------------------------------------------------------
void SkyController::PostRender(Camera* cam)
{
    if(!bRender)
        return;

    if(DisplayTime)
    {
        string Median = " AM";

        string HoursString;
        string MinutesString;

        float Hours = DayTimeMinutes/60.f;
        if(Hours >= 12)
        {
            Median = " PM";

            if(Hours > 12)
                Hours -= 12;
        }

        if(Hours < 1)
            Hours += 12;

        int HoursInt = Hours;
        HoursString = ToStr(HoursInt);

        int MinutesInt = (Hours - HoursInt)*60.f;
        MinutesString = ToStr(MinutesInt);

        if(MinutesInt < 10)
            MinutesString = ToStr("0") + MinutesString;

        string FinalTime = HoursString + ToStr(":") + MinutesString + Median;

        Canvas::Instance()->Textf(MediumFont,COLOR_RGBA(255,255,255,255),920,746,FinalTime.c_str());
    }
}

//--------------------------------------------------------------------------------------
// dtor
//--------------------------------------------------------------------------------------
SkyController::~SkyController()
{
    if(AmbientLight)
        delete AmbientLight;

    Instance = NULL;
    delete m_ControllerScript;
    m_ControllerScript = NULL;

    SAFE_RELEASE(SunMaterial);
    SAFE_RELEASE(MoonMaterial);
    SAFE_RELEASE(m_SkyBGMaterial);

    for(int i = 0; i < MyWorld->m_SHProbes.size(); i++)
    {
        delete MyWorld->m_SHProbes[i];
    }
    MyWorld->m_SHProbes.clear();
    InteriorVolumes.clear();
}

//--------------------------------------------------------------------------------------
// ctor
//--------------------------------------------------------------------------------------
SkyController::SkyController(World* world) : NetworkActor(world)
{
    Instance = this;

    SetNetworkTickRate(0);
    GhostObject = true;
    IsHidden = true;
    bRender = true;

    LightningSounds[0].Load2D("thunder1.wav");
    LightningSounds[1].Load2D("thunder2.wav");

    m_Server_LastSentDayTime = 0;

    // Hack, temp values for sun
    fSunPosAlpha = PI/2;
    fSunPosTheta = 0.88f;
    fSunShininess=	1076.0f;
    fSunStrength = 5100.0f;

    // load some key settings from the ini
    DisplayTime = MyWorld->m_Config.GetBool("DisplayTime","SkyController");
    SkyBoxSize = MyWorld->m_Config.GetFloat("SkyBoxSize","SkyController");
    NumSkies = MyWorld->m_Config.GetFloat("NumSkies","SkyController");
    m_bDrawMoon = MyWorld->m_Config.GetBool("DrawMoon","SkyController");
    m_bDrawSun = MyWorld->m_Config.GetBool("DrawSun","SkyController");
    SH_EnvironmentMapMultiplier = MyWorld->m_Config.GetFloat("EnvironmentMapMultiplier","SkyController");

    if(MyWorld->m_Config.KeyExists("SunLightIntensityMultiplier","SkyController"))
        SunLightIntensityMultiplier = MyWorld->m_Config.GetFloat("SunLightIntensityMultiplier","SkyController");
    else
        SunLightIntensityMultiplier = 1;

    if(MyWorld->m_Config.KeyExists("MoonLightIntensityMultiplier","SkyController"))
        MoonLightIntensityMultiplier = MyWorld->m_Config.GetFloat("MoonLightIntensityMultiplier","SkyController");
    else
        MoonLightIntensityMultiplier = 1;

    if(MyWorld->m_Config.KeyExists("SkyBGIntensity","SkyController"))
        SkyBGIntensityFactor = MyWorld->m_Config.GetFloat("SkyBGIntensity","SkyController");
    else
        SkyBGIntensityFactor = 1;

    SkyMinutesPerGameSecond =  MyWorld->m_Config.GetFloat("SkyMinutesPerGameSecond","SkyController");


    m_HasLightning = MyWorld->m_Config.GetBool("HasLightning","SkyController");
    LightningStartMinutes = MyWorld->m_Config.GetFloat("LightningStartMinutes","SkyController");
    LightningEndMinutes = MyWorld->m_Config.GetFloat("LightningEndMinutes","SkyController");
    LightningFlashFrequency = MyWorld->m_Config.GetFloat("LightningFlashFrequency","SkyController");
    CurLightningIntensity = 0;
    LightningFlashTimer = 30;

    // create the day outdoor amb light, if any name is specified
    string AmbientLightName = MyWorld->m_Config.GetString("SunAmbientLightName","SkyController");
    if(AmbientLightName.size())
    {
        AmbientLight = new Light(MyWorld);
        AmbientLight->m_Name = AmbientLightName;
        AmbientLight->m_Type = LIGHT_DIR;
        AmbientLight->GetCurrentState().Range = 10000;
        AmbientLight->GetCurrentState().Spot_Falloff = 180;
        AmbientLight->GetCurrentState().Spot_Size = 180;
        AmbientLight->m_Method = LIGHT_METHOD_SH;
        AmbientLight->m_ForceSHMultiPass = false;
        AmbientLight->Outside = true;
        AmbientLight->Inside = false;
    }
    else
        AmbientLight = NULL;

    SunXDistance = MyWorld->m_Config.GetFloat("SunXDistance","SkyController");
    SunYDistance = MyWorld->m_Config.GetFloat("SunYDistance","SkyController");
    SunCycleMinuteOffset = MyWorld->m_Config.GetFloat("SunCycleMinuteOffset","SkyController");

    MoonXDistance = MyWorld->m_Config.GetFloat("MoonXDistance","SkyController");
    MoonYDistance = MyWorld->m_Config.GetFloat("MoonYDistance","SkyController");
    MoonCycleMinuteOffset = MyWorld->m_Config.GetFloat("MoonCycleMinuteOffset","SkyController");

    SunMaterial = NULL;
    MoonMaterial = NULL;
    m_SkyBGMaterial = NULL;

    if(!Engine::Instance()->IsDedicated())
    {
        // create SkyBox bg mats

        // SH environment maps (probes)
        for(int i = 0; i < NumSkies; i++)
        {
            SkySHProbes[i] = MyWorld->LoadSHProbe(MyWorld->m_Config.GetString("SkyTextureNameEnv","SkyController") + ToStr("_") + ToStr(i) + ToStr(".dds"));
        }


        string SkyTextureName = MyWorld->m_Config.GetString("SkyTextureName","SkyController");

        for(int i = 0; i < 4; i++)
        {
            string filename = SkyTextureName + ToStr("_") + ToStr(i) + ToStr(".dds");
            bool result = m_SkyBGTextures[i].Load(filename);
        }

        for(int n=0;n<NumSkies;n++)
        {
            string VarName = ToStr("tDiffuse") + ToStr(n);
            //set dummy texture
            m_SkyBGSamplerVars[n].Set(VarName,EditorVar::TEXTURE,ShaderVar::VAR_DONTDELETE,&m_SkyBGTextures[0]);
        }

        m_SkyBGMaterial = new Material("SkyBG");
        m_SkyBGMaterial->m_ID = 0;
        m_SkyBGMaterial->Initialize(MyWorld->m_Config.GetString("SkyShaderFile","SkyController").c_str(),MyWorld->m_Config.GetString("SkyBGShaderTechnique","SkyController").c_str());
        m_SkyBGMaterial->m_Parameters.resize(4);
        for(int n=0;n<4;n++)
        {
            ShaderVar* var = new ShaderVar;
            string VarName = ToStr("SkyMix") + ToStr(n);
            var->name = VarName;
            m_SkyBGMaterial->m_Parameters[n] = var;
            m_SkyBGMaterial->m_Shader->SetVar(*var,&SkyBGMix[28+n]);
            m_SkyBGMaterial->m_Shader->GetEffect()->SetFloat("Overbright_Sky",SkyBGIntensityFactor);
        }
        SkyBoxBGShaderTechnique = m_SkyBGMaterial->m_Shader->GetTechnique("SkyBox_BlendedBG");

        //create sun and moon mats
        if(true)
        {
            MoonMaterial = new Material("MoonMaterial");
            MoonMaterial->m_ID = 0;
            MoonMaterial->Initialize(MyWorld->m_Config.GetString("SkyShaderFile","SkyController").c_str(),MyWorld->m_Config.GetString("MoonShaderTechnique","SkyController").c_str());

            MoonMaterial->m_Parameters.resize(2);

            MoonMaterial->m_Parameters[0] = new ShaderVar;

            // Allocate a texture
            Texture* tex = new Texture;
            bool result = tex->Load(MyWorld->m_Config.GetString("MoonTexture","SkyController"));
            string VarName = ToStr("tDiffuse0");
            MoonMaterial->m_Parameters[0]->Set(VarName,EditorVar::TEXTURE,ShaderVar::VAR_DELETETEXTURE,tex);


            ShaderVar* var = new ShaderVar;
            var->name = "MoonAlphaFactor";
            MoonMaterial->m_Parameters[1] = var;
            MoonMaterial->m_Shader->SetVar(*var,&SkyBGMix[4]);
            MoonShaderTechnique = MoonMaterial->m_Shader->GetTechnique(MyWorld->m_Config.GetString("MoonShaderTechnique","SkyController"));
        }
        if(true)
        {
            SunMaterial = new Material("SunMaterial");
            SunMaterial->m_ID = 0;
            SunMaterial->Initialize(MyWorld->m_Config.GetString("SkyShaderFile","SkyController").c_str(),MyWorld->m_Config.GetString("SunShaderTechnique","SkyController").c_str());

            SunMaterial->m_Parameters.resize(2);

            SunMaterial->m_Parameters[0] = new ShaderVar;

            // Allocate a texture
            Texture* tex = new Texture;
            bool result = tex->Load(MyWorld->m_Config.GetString("SunTexture","SkyController"));
            string VarName = ToStr("tDiffuse0");
            SunMaterial->m_Parameters[0]->Set(VarName,EditorVar::TEXTURE,ShaderVar::VAR_DELETETEXTURE,tex);

            ShaderVar* var = new ShaderVar;
            var->name = "SunAlphaFactor";
            SunMaterial->m_Parameters[1] = var;
            SunMaterial->m_Shader->SetVar(*var,&SkyBGMix[6]);

            SunShaderTechnique = SunMaterial->m_Shader->GetTechnique(MyWorld->m_Config.GetString("SunShaderTechnique","SkyController"));
        }

        HasClouds = MyWorld->m_Config.GetBool("HasClouds","SkyController");

           CloudDome.Load(MyWorld->m_Config.GetString("CloudModel","SkyController").c_str());
		   // grab cloud mat from model
           CloudMaterial = CloudDome.FindMaterial(MyWorld->m_Config.GetString("CloudMaterialName","SkyController"));
           CloudShaderTechnique = CloudMaterial->m_Shader->GetTechnique(MyWorld->m_Config.GetString("CloudShaderTechnique","SkyController"));

            ShaderVar* var = CloudMaterial->FindVar("CloudScrollSpeed");
            if(var)
                SunMaterial->m_Shader->SetVar(*var,&SkyBGMix[8]);

            var = CloudMaterial->FindVar("CloudIntensityFactor");
            if(var)
                SunMaterial->m_Shader->SetVar(*var,&SkyBGMix[9]);

            var = CloudMaterial->FindVar("CloudSolidity");
            if(var)
                SunMaterial->m_Shader->SetVar(*var,&SkyBGMix[10]);

            var = CloudMaterial->FindVar("MaxCloudIntensity");
            if(var)
                SunMaterial->m_Shader->SetVar(*var,&SkyBGMix[11]);

            var = CloudMaterial->FindVar("MinCloudIntensity");
            if(var)
                SunMaterial->m_Shader->SetVar(*var,&SkyBGMix[12]);

            var = CloudMaterial->FindVar("CloudAddRed");
            if(var)
                SunMaterial->m_Shader->SetVar(*var,&SkyBGMix[13]);

            var = CloudMaterial->FindVar("CloudAddGreen");
            if(var)
                SunMaterial->m_Shader->SetVar(*var,&SkyBGMix[14]);

            var = CloudMaterial->FindVar("CloudAddBlue");
            if(var)
                SunMaterial->m_Shader->SetVar(*var,&SkyBGMix[15]);

            var = CloudMaterial->FindVar("CloudAlphaFactor");
            if(var)
                SunMaterial->m_Shader->SetVar(*var,&SkyBGMix[16]);

        //set up vertex tex coords
        float f = 0.5f / 512.0f;
        vert[0].m_vecTex = Vector2(0.0f + f, 0.0f + f);
        vert[1].m_vecTex = Vector2(0.0f + f, 1.0f - f);
        vert[2].m_vecTex = Vector2(1.0f - f, 0.0f + f);
        vert[3].m_vecTex = Vector2(1.0f - f, 1.0f - f);

        ///
        // Create the skydome buffers!
        ///
        m_SkyboxDetail = 32;

        // create the skydome vertexbuffer
        if( FAILED( RenderWrap::dev->CreateVertexBuffer( m_SkyboxDetail*m_SkyboxDetail*sizeof(Vertex),
            D3DUSAGE_WRITEONLY, /*FVF_VERTEX*/0,
            D3DPOOL_MANAGED, &m_SkyVertices, NULL ) ) )
        {
            return ;
        }
        Vertex* pdVertices;
        if( FAILED( m_SkyVertices->Lock( 0, 0, (void**)&pdVertices, 0 ) ) )
            return ;
        {
            for(int v=0; v<m_SkyboxDetail; v++){
                for(int u=0; u<m_SkyboxDetail; u++){
                    float	al = -2*3.14159265*((float)u/(m_SkyboxDetail-1.0f)),
                        th = 0.6*3.14159265*((float)v/(m_SkyboxDetail-1.0f));
                    pdVertices[v*m_SkyboxDetail+u].position.x = sin(th)*sin(al);
                    pdVertices[v*m_SkyboxDetail+u].position.y = cos(th);
                    pdVertices[v*m_SkyboxDetail+u].position.z = sin(th)*cos(al);
                }
            }
        }
        m_SkyVertices->Unlock();

        // create/fill the indexbuffer
        unsigned int *indexbuffer;

        // create/fill the indexbuffer
        if(	FAILED( RenderWrap::dev->CreateIndexBuffer(	sizeof(unsigned int) * 6 * (m_SkyboxDetail-1)*(m_SkyboxDetail-1),
            D3DUSAGE_WRITEONLY,			
            D3DFMT_INDEX32,	D3DPOOL_MANAGED,&m_SkyIndices,NULL)))
        {
            return ;
        }

        if( FAILED( m_SkyIndices->Lock(0,0,(void**)&indexbuffer,0 ) ) )
            return ;

        {
            int i = 0;
            for(int v=0; v<m_SkyboxDetail-1; v++){
                for(int u=0; u<m_SkyboxDetail-1; u++){
                    // face 1 |/
                    indexbuffer[i++]	= v*m_SkyboxDetail + u;
                    indexbuffer[i++]	= v*m_SkyboxDetail + u + 1;
                    indexbuffer[i++]	= (v+1)*m_SkyboxDetail + u;

                    // face 2 /|
                    indexbuffer[i++]	= (v+1)*m_SkyboxDetail + u;
                    indexbuffer[i++]	= v*m_SkyboxDetail + u + 1;
                    indexbuffer[i++]	= (v+1)*m_SkyboxDetail + u + 1;
                }
            }
        }
        m_SkyIndices->Unlock();

    }
    m_ControllerScript->Initialize(NULL,MyWorld,Matrix());

}

float PrevSkyBGMix[4];
//--------------------------------------------------------------------------------------
// Draws the Sky
//--------------------------------------------------------------------------------------
void SkyController::DrawSky(Camera* cam)
{
    if(!bRender)
        return;

    if(m_HasLightning)
    {
        if(CurLightningIntensity > 0)
        {
            CurLightningIntensity -= GDeltaTime*20.0f;
            if(CurLightningIntensity <= 0)
                CurLightningIntensity = 0;
        }
        else if(DayTimeMinutes > LightningStartMinutes && DayTimeMinutes < LightningEndMinutes)
        {
            LightningFlashTimer -= GDeltaTime * LightningFlashFrequency * 35.0f * RANDF();
            if(LightningFlashTimer < 0)
            {
                if(rand()%3 == 1)
                {
                    // FLASH LIGHTNING!
                    float volume = 1;
                    if(IndoorVolume::IsIndoors(MyWorld,cam->Location))
                        volume = .5f;

                    LightningSounds[rand()%2].Play2D(0,volume);

                    CurLightningIntensity = 10;
                    LightningFlashTimer = 60;
                }
                else // it's a dud
                    LightningFlashTimer = 60;
            }
        }
    }

    m_bIsSkyVisible = IsSkyVisible(cam);
    DWORD rsFog	= 0;
    DWORD rsAlpha = 0;
    DWORD rsZWrite = 0;
    DWORD rsCull = 0;
	DWORD rsZEnable = 0;
    Vector pos;

    if(m_bIsSkyVisible)
    {
        for(int n = 0; n < 4; n++)
        {
            PrevSkyBGMix[n] = SkyBGMix[28+n];
            SkyBGMix[28+n] *= (1 + CurLightningIntensity);
        }

        RenderWrap::SetView(cam->view);
        pos = cam->Location;

        float f = SkyBoxSize * 0.5f;

        Shader* shader = m_SkyBGMaterial->m_Shader;
        shader->SetTechnique(SkyBoxBGShaderTechnique);
        shader->SetWorld(Matrix());
        RenderWrap::dev->SetFVF(ENV_FVF);
        m_SkyBGMaterial->Apply(false);
        shader->Begin();
        shader->BeginPass(0);
        shader->SetFog(MyWorld->GetFogDistance(),MyWorld->GetFogColor());

        /// FIXME: Set all skies
        //m_SkyBGSamplerVars[0].data = &m_SkyBGTextures[0];
        //shader->SetVar(m_SkyBGSamplerVars[0]);

        // build a 'fake' viweproj with the distance vector set to 0,0,0
        Camera cam2 = *cam;
        /// EVIL!!!
        /*cam2.NearClip = 0.1f;
        cam2.NearClip = 10000.0f;
        cam2.Location = Vector();
        cam2.Update();*/
        D3DXMATRIXA16 fvproj(*(D3DXMATRIX*)&cam2.view);
        fvproj._41 = 0;
        fvproj._42 = 0;
        fvproj._43 = 0;
        fvproj = fvproj * *(D3DXMATRIX*)&cam2.projection;

        shader->GetEffect()->SetMatrix("mViewProj",&fvproj);
        for(int i=0;i<4;i++)
            shader->GetEffect()->SetTexture(("EnvironmentMap"+ToStr(i+1)).c_str(),m_SkyBGTextures[i].GetTexture());

        /*shader->GetEffect()->SetFloat("sun_alfa", fSunPosAlpha);
        shader->GetEffect()->SetFloat("sun_theta", fSunPosTheta);
        shader->GetEffect()->SetFloat("sun_shininess", 4*fSunShininess);
        shader->GetEffect()->SetFloat("sun_strength", fSunStrength);*/

        shader->CommitChanges();

                //draw SkyBox
        rsFog	  = RenderWrap::SetRS( D3DRS_FOGENABLE, FALSE); 
        rsAlpha	  = RenderWrap::SetRS(D3DRS_ALPHABLENDENABLE,FALSE);
        rsZWrite  = RenderWrap::SetRS(D3DRS_ZWRITEENABLE,FALSE);
        rsZEnable = RenderWrap::SetRS(D3DRS_ZENABLE,FALSE);
        RenderWrap::dev->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE  );
        RenderWrap::SetRS(D3DRS_ALPHABLENDENABLE, FALSE );
        RenderWrap::SetRS(D3DRS_FOGENABLE, FALSE );

        RenderWrap::dev->SetStreamSource( 0, m_SkyVertices, 0, sizeof(Vertex) );
        RenderWrap::dev->SetVertexDeclaration(VertexFormats::Instance()->FindFormat(sizeof(Vertex))->decl);
        RenderWrap::dev->SetIndices(m_SkyIndices);
        RenderWrap::dev->DrawIndexedPrimitive(	D3DPT_TRIANGLELIST,	0,	0, m_SkyboxDetail*m_SkyboxDetail, 0, 2*(m_SkyboxDetail-1)*(m_SkyboxDetail-1) );
        shader->EndPass();
        shader->End();

        for(int n = 0; n < 4; n++)
            SkyBGMix[28+n] = PrevSkyBGMix[n];
    }

    //flip target once for both sun and moon
    if(m_bDrawMoon || m_bDrawSun)
        RenderDevice::Instance()->FlipHDRTargets();

    DrawMoon(pos);
    DrawSun(pos);

    if(AmbientLight)
    {
        AmbientLight->GetCurrentState().Position = SunLightState.Position;
        AmbientLight->GetCurrentState().Direction = SunLightState.Direction;
        AmbientLight->GetCurrentState().Intensity = 1 + CurLightningIntensity/2.5f;
        AmbientLight->Location = SunLightState.Position;
        AmbientLight->Rotation = Matrix::LookTowards(SunLightState.Direction);

        static float NightTimeEnd = 370;
        static float NightTimeEndTransition = 430;
        static float NightTimeTransitionDuration = 60;
        static float DayTimeEnd = 1220;
        static float DayTimeEndTransition = 1280;
        static float DayTimeTransitionDuration = 60;

        if(DayTimeMinutes < NightTimeEnd || DayTimeMinutes >= DayTimeEndTransition)
            AmbientLight->GetCurrentState().Diffuse = MoonLightState.Diffuse;
        else if(DayTimeMinutes < NightTimeEndTransition)
        {
            float lerpVal = 1.0f - (NightTimeEndTransition - DayTimeMinutes)/NightTimeTransitionDuration;
            AmbientLight->GetCurrentState().Diffuse = MoonLightState.Diffuse + (SunLightState.Diffuse - MoonLightState.Diffuse)*lerpVal;
        }
        else if(DayTimeMinutes < DayTimeEnd)
            AmbientLight->GetCurrentState().Diffuse = SunLightState.Diffuse;
        else if(DayTimeMinutes < DayTimeEndTransition)
        {
            float lerpVal = 1.0f - (DayTimeEndTransition - DayTimeMinutes)/DayTimeTransitionDuration;
            AmbientLight->GetCurrentState().Diffuse = SunLightState.Diffuse + (MoonLightState.Diffuse - SunLightState.Diffuse)*lerpVal;
        }

    }

    if(m_bIsSkyVisible)
    {
        if(HasClouds)
            DrawClouds(pos);

        //RenderWrap::SetRS(D3DRS_CULLMODE, rsCull );
        RenderWrap::SetRS(D3DRS_ALPHABLENDENABLE, rsAlpha );
        RenderWrap::SetRS(D3DRS_ZWRITEENABLE,rsZWrite);
		RenderWrap::SetRS(D3DRS_ZENABLE,rsZEnable);
        RenderWrap::SetRS(D3DRS_FOGENABLE, rsFog );
    }

    for(int i = 0; i < NumSkies;i++)
    {
        MyWorld->m_SHProbes[SkySHProbes[i]]->fBlendFactor = SkyBGMix[28+i]*SH_EnvironmentMapMultiplier;
        //if(MyWorld->m_SHProbes.size()>SkySHProbes[i])
        //  MyWorld->m_SHProbes[SkySHProbes[i]]->fBlendFactor = SkyBGMix[28+i]*SH_EnvironmentMapMultiplier;
    }
}


//--------------------------------------------------------------------------------------
// when the sky is not actual visible in interiors, don't bother drawing it
//--------------------------------------------------------------------------------------
bool SkyController::IsSkyVisible(Camera* camera)
{
    Vector position = camera->Location;
    for(int i = 0; i < InteriorVolumes.size(); i++)
    {
        BBox bbox = InteriorVolumes[i];
        if(position.x < bbox.max.x && position.x > bbox.min.x && position.y < bbox.max.y && position.y > bbox.min.y && position.z < bbox.max.z && position.z > bbox.min.z)
            return false;
    }
    return true;
}

//--------------------------------------------------------------------------------------  
// Sun/Moon direction vector based on time of day  
// Cycles from -X degrees to +X degrees over hemisphere  
//--------------------------------------------------------------------------------------  
Vector GetDir(float dayMinutes)  
{  
    float angle = DEG2RAD(140);  

    float dawn = 7*60;  
    float dusk = 21*60;  

    float dayLength = dusk-dawn;  
    float nightLength = 24*60-dayLength;  

    float frac = 0;  

    // Day  
    if(dayMinutes > dawn && dayMinutes < dusk)  
    {  
        frac = dayMinutes - dawn;  
        // Put fraction [0,1]  
        frac /= dayLength;  
    }  
    // Gone midnight  
    else if(dayMinutes < dawn)  
    {  
        frac = dayMinutes + (1440-dusk);  
        // Put fraction [0,1]  
        frac /= nightLength;  
    }  
    // Evening  
    else  
    {  
        frac = dayMinutes - dusk;  
        // Put fraction [0,1]  
        frac /= nightLength;  
    }  

    assert(frac >= 0 && frac <= 1);  


    // Arc goes opposite way during day  
    if(dayMinutes > dawn && dayMinutes < dusk)  
        frac = 1-frac;  

    // Fraction to [-0.5,0.5]  
    frac = (frac*2-1)*0.5f;  

    Vector dir;  
    dir.x = cosf(frac*angle + DEG2RAD(90));  
    dir.y = -sinf(frac*angle + DEG2RAD(90));  
    dir.z = 0.12f;//-cosf(frac*angle*0.2f);  

    return dir.Normalized();
}

//--------------------------------------------------------------------------------------
// Draws the Moon billboard and updates the Moon ambient Light
//--------------------------------------------------------------------------------------
void SkyController::DrawMoon(Vector& pos)
{
    MoonLightState.Direction = GetDir(DayTimeMinutes);
    MoonLightState.Position = -6000.f*MoonLightState.Direction;
    MoonLightState.Diffuse = FloatColor(SkyBGMix[20],SkyBGMix[21],SkyBGMix[22],1)*MoonLightIntensityMultiplier;

    if(!m_bIsSkyVisible || SkyBGMix[4] < .005 || !m_bDrawMoon)
        return;

    float size = SkyBGMix[5];//1.1f

    float timeCycle = DayTimeMinutes-MoonCycleMinuteOffset;

    float xDist = cos(timeCycle*(2.f*PI/1440.f));
    float yDist = sin(timeCycle*(2.f*PI/1440.f));

    Vector location = Vector(MoonXDistance*xDist, MoonYDistance*yDist, 0);

    // Calculate billboard data for a camera-facing quad using the view matrix
    Matrix mat = Matrix::LookTowards(-location);

    Vector loc	=  location + pos;
    Vector right = mat.GetRight() * size;
    Vector up = mat.GetUp() * size;

    if(xDist < 0)
    {
        up *= -1;
        right *= -1;
    }

    vert[0].m_vecPos = (loc-right)-up;
    vert[1].m_vecPos = (loc+right)-up;
    vert[2].m_vecPos = (loc-right)+up;
    vert[3].m_vecPos = (loc+right)+up;

    Shader* shader = MoonMaterial->m_Shader;
    shader->SetTechnique(MoonShaderTechnique);
    shader->SetColorBuffer(HDRSystem::Instance()->GetColorBuffer());
    shader->SetWorld(Matrix());
    MoonMaterial->Apply(false);
    RenderWrap::dev->SetFVF(ENV_FVF);
    shader->CommitChanges();
    shader->Begin();
    shader->BeginPass(0);
    if(!RenderDevice::Instance()->GetHDR())
        RenderWrap::SetRS(D3DRS_ALPHABLENDENABLE,TRUE);
    RenderWrap::DrawPrim(true,2,vert,sizeof(ENV_VERTEX));
    shader->UnbindHDRTarget();
    shader->EndPass();
    shader->End();

    RenderWrap::SetRS(D3DRS_ALPHABLENDENABLE,FALSE);
}

//--------------------------------------------------------------------------------------
// Draws the Sun billboard and updates the Sun ambient Light
//--------------------------------------------------------------------------------------
void SkyController::DrawSun(Vector& pos)
{
    SunLightState.Direction = GetDir(DayTimeMinutes);
    SunLightState.Position = -6000.f*SunLightState.Direction;
    SunLightState.Diffuse = FloatColor(SkyBGMix[17],SkyBGMix[18],SkyBGMix[19],1)*SunLightIntensityMultiplier;

    float size = SkyBGMix[7];//1.85f

    float timeCycle = DayTimeMinutes-SunCycleMinuteOffset;

    float xDist = cos(timeCycle*(2.f*PI/1440.f));
    float yDist = sin(timeCycle*(2.f*PI/1440.f));

    Vector location = Vector(SunXDistance*xDist, SunYDistance*yDist, 0);

    fSunPosTheta = timeCycle*(2.f*PI/1440.f);

    SunPosition = location*600;

    if(!m_bIsSkyVisible || SkyBGMix[6] < .005 || !m_bDrawSun)
        return;

    // Calculate billboard data for a camera-facing quad using the view matrix
    Matrix mat = Matrix::LookTowards(-location);

    Vector loc	=  location + pos;
    Vector right = mat.GetRight() * size;
    Vector up = mat.GetUp() * size;

    if(xDist < 0)
    {
        up *= -1;
        right *= -1;
    }

    vert[0].m_vecPos = (loc-right)-up;
    vert[1].m_vecPos = (loc+right)-up;
    vert[2].m_vecPos = (loc-right)+up;
    vert[3].m_vecPos = (loc+right)+up;

    Shader* shader = SunMaterial->m_Shader;
    shader->SetTechnique(SunShaderTechnique);
    shader->SetColorBuffer(HDRSystem::Instance()->GetColorBuffer());
    shader->SetWorld(Matrix());
    SunMaterial->Apply(false);
    RenderWrap::dev->SetFVF(ENV_FVF);
    shader->CommitChanges();
    shader->Begin();
    shader->BeginPass(0);
    if(!RenderDevice::Instance()->GetHDR())
        RenderWrap::SetRS(D3DRS_ALPHABLENDENABLE,TRUE);
    RenderWrap::DrawPrim(true,2,vert,sizeof(ENV_VERTEX));
    shader->UnbindHDRTarget();
    shader->EndPass();
    shader->End();

    RenderWrap::SetRS(D3DRS_ALPHABLENDENABLE,FALSE);
}

//--------------------------------------------------------------------------------------
// Draws the Cloud FX
//--------------------------------------------------------------------------------------
void SkyController::DrawClouds(Vector& pos)
{
    RenderDevice::Instance()->FlipHDRTargets();

    CloudDome.SetTransform(Matrix(),pos+Vector(0,-12,0));

    Shader* shader = CloudMaterial->m_Shader;
    shader->SetTechnique(CloudShaderTechnique);
    shader->SetColorBuffer(HDRSystem::Instance()->GetColorBuffer());
    shader->SetWorld(CloudDome.m_pFrameRoot->CombinedTransformationMatrix);
    CloudMaterial->Apply(false);
    shader->CommitChanges();
    shader->Begin();
    shader->BeginPass(0);

    shader->SetFog(MyWorld->GetFogDistance(),FloatColor());

    if(!RenderDevice::Instance()->GetHDR())
        RenderWrap::SetRS(D3DRS_ALPHABLENDENABLE,TRUE);

    RenderWrap::SetRS(D3DRS_ZENABLE,FALSE);
    RenderWrap::SetRS(D3DRS_ZWRITEENABLE,FALSE);

    CloudDome.m_pFrameRoot->Draw();
    shader->UnbindHDRTarget();
    shader->EndPass();
    shader->End();

    shader->SetWorld(Matrix());

    RenderWrap::SetRS(D3DRS_ALPHABLENDENABLE,FALSE);
}

//--------------------------------------------------------------------------------------
// Sets the sky speed, in sky minutes (24 hr day) per game second
//--------------------------------------------------------------------------------------
void SkyController::SetMinutesPerGameSecond(float MinutesPerGameSecond)
{
    if(MinutesPerGameSecond)
        SkyMinutesPerGameSecond = MinutesPerGameSecond;
    else
        SkyMinutesPerGameSecond = SMALL_NUMBER;

    if(MyWorld->m_IsServer)
    {
        // write speed of day cycle (minutes per game second) to all networkclients
        WriteMessageType(MSGID_SKYCONTROLLER_MINUTES_SPEED_SCALE);
        WriteFloat(SkyMinutesPerGameSecond);
    }

    SetDayTime(DayTimeMinutes);
}

void SkyController::AddSkyVolumeBox(BBox& BoxVolume)
{
    int index = InteriorVolumes.size();
    InteriorVolumes.resize(index + 1);
    InteriorVolumes[index] = BoxVolume;
}