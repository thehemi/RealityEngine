//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
/// SkyController: Manages & renders Animated Day/Night Sky system, networked
//====================================================================================
#ifndef SKYCONTROLLER_H
#define SKYCONTROLLER_H
#include "NetworkActor.h"


#define NUM_SKY_FADES 36

//--------------------------------------------------------------------------------------
/// SkyController: Manages & renders Animated Day/Night Sky system, networked
//--------------------------------------------------------------------------------------
class ENGINE_API SkyController : public NetworkActor
{
	CLASS_NAME(SkyController);

public:

	//jump forward by 10 ID's in between classes to provide some room to add more message types
	const static MessageType MSGID_SKYCONTROLLER_TIME = 26;
	const static MessageType MSGID_SKYCONTROLLER_MINUTES_SPEED_SCALE = 27;

	/// we can only have 1 SkyController in the World at a time, and this is a pointer to it. NULL if no sky.
	static SkyController* Instance;
	/// controller script that adjusts the sky visuals over the course of the sky's day
	static Script* m_ControllerScript;
	/// Creates a new SkyController from the specified Script file and "Map.ini" file. 
	/// RetainTime == true will have the new sky start at the same time as the old sky, if any, for sky reloading.
	static void IntitializeNewSky(World* world,string ScriptClassName, bool RetainTime = false);

	SkyController(World* world);
	virtual ~SkyController();
	/// Updates the Sky's core logic every frame
	virtual void Tick();
	/// Iterates through Clients and calls Server_MakeTickMessages() to replicate changes in state
	virtual void Server_NetworkTick();
	/// Processes a spawn-state message
	virtual void Client_HandleSpawnMessage(MessageType message, ReadPacketBuffer* packetBuffer);
	/// Processes a state-update replication message
	virtual void Client_HandleTickMessage(MessageType message, ReadPacketBuffer* packetBuffer);
	/// Server-side generates replication of full state for newly-joining Clients
	virtual NetworkActorPackets* Server_MakeOnJoinSynchMessages(class NetworkClient* client);
	/// Server-side generates replication of initial spawn state for all Clients. 
	/// Actors inheriting from NetworkActor can override this if they have custom state information to replicate.
	virtual void Server_MakeSpawnMessages();
	/// Resets the core Sky State, starting the Sky back at its initial time with its initial fader values
	void ResetSkyState();

	/// Draws the Sky (behind the World, hence in PreRender)
	virtual void DrawSky(class Camera* cam);
	/// Draws the Sky Time HUD element if DisplayTime == true
	virtual void PostRender(class Camera* cam);
	/// Begins interpolation of the specified Sky Fader index float (controlling sky blend and color values)
	/// to a destination value over the course of a number of Sky Minutes
	/// Using 0 for MinutesDuration will result in an instant Set of the destination values.
	void FadeSky(int SkyIndex,float Dest,float MinutesDuration);
	/// Begins interpolation of current fog values to destination values over the course of a number of Sky Minutes
	/// Using 0 for MinutesDuration will result in an instant Set of the destination values.
	void FadeFog(float newDensity, float newRed,float newGreen, float newBlue, float MinutesDuration);
	/// Sets the sky speed, in sky minutes (24 hr day) per game second
	void SetMinutesPerGameSecond(float MinutesPerGameSecond);
	/// Gets the sky speed, in sky minutes (24 hr day) per game second
	float GetMinutesPerGameSecond(){return SkyMinutesPerGameSecond;}
	/// Gets the current Sky day time, in minutes (60 min * 24 hr = 1440 min per day)
	float GetDayTimeMinutes(){return DayTimeMinutes;}
	/// Sets the current sky day time, will jump the sky instantly to the new day time
	void SetDayTime(float NewDayTimeMinutes);
	/// Gets the current lightning intensity (no lightning = 0), for scaling other effects (such as brightening the rain) during lightning flashes
	float GetLightningIntensity(){return CurLightningIntensity;}

	/// whether the sky writes the current time on screen HUD
	bool DisplayTime;
	/// Current sky state for tracking in script, denoting period of day
	int m_SkyState;
	/// Current weather state for tracking in script, denoting which weather system is currently active
	int m_WeatherState;

	/// These Sky Mixers are floats linked up to key SkyController variables whose indices are listed in ScriptInterface.h
	/// The SkyController Script can set and interpolate these values to adjust the transition of the Sky graphics throughout the day cycle
	float SkyBGMix[NUM_SKY_FADES];

	/// adds an interior volume in which the sky ambience will not have an effect
	static void AddSkyVolumeBox(BBox& BoxVolume);

	/// Sun parameters shader can use in its own calculations
	float	fSunPosAlpha;
	float	fSunPosTheta;
	float	fSunShininess;
	float	fSunStrength;

	/// Returns i'th sky texture used for blending
	Texture* GetSkyBGTexture(int index){ return &m_SkyBGTextures[index]; }
	/// Returns sky material
	Material* GetSkyMaterial(){ return m_SkyBGMaterial; }
	/// Gets ambient light(s)
	vector<Light*> GetLights(){ vector<Light*> l;if(AmbientLight)l.push_back(AmbientLight); return l; }

    /// Render Sky?
    bool bRender;
	/// weather the sky system will draw cloud formations
	bool HasClouds;

	/// unitary skycontroller light, interpolates between sun and moon states
	Light* AmbientLight;

	/// "position" of the sun in the World, some miles out
	Vector SunPosition;

private:
	/// a multiplier for the skybox background shaders' colour intensity
	float SkyBGIntensityFactor;

	/// the bounding boxes that define the interior volumes
	static vector<BBox> InteriorVolumes;

	/// when the sky is not actual visible in interiors, don't bother drawing it
	bool IsSkyVisible(Camera* camera);

	/// stores the results of above function for use in various places of the sky update/render process
	bool m_bIsSkyVisible;

	/// size in meters of sky box to draw
	float SkyBoxSize;

	/// cloud dome model on which to render the cloud technique
	Model CloudDome;
	/// Cloud Material using Sky Shader
	Material* CloudMaterial;
	/// Technique handle of Clouds in Sky Shader
	D3DXHANDLE CloudShaderTechnique;

	/// SkyBox face Material
	Material* m_SkyBGMaterial;
	/// one Texture for each SkyBox face
	Texture m_SkyBGTextures[4];
	ShaderVar m_SkyBGSamplerVars[4];

	/// Skybox vertices
	LPDIRECT3DVERTEXBUFFER9		m_SkyVertices;
	/// Skybox indices
	LPDIRECT3DINDEXBUFFER9		m_SkyIndices;
	/// Number of faces used in skybox
	int m_SkyboxDetail;

	/// Technique handle of SkyBox faces in Sky Shader
	D3DXHANDLE SkyBoxBGShaderTechnique;

	/// X distance of sun billboard from view
	float SunXDistance;
	/// Y distance of sun billboard from view
	float SunYDistance;
	/// Minutes the sin/cos sun movement is offset from the midnight
	float SunCycleMinuteOffset;
	/// Sun Billboard Material using Sky Shader
	Material* SunMaterial;
	/// Technique handle of Sun billboard in Sky Shader
	D3DXHANDLE SunShaderTechnique;

	/// X distance of moon billboard from view
	float MoonXDistance;
	/// Y distance of moon billboard from view
	float MoonYDistance;
	/// Minutes the sin/cos moon movement is offset from the midnight
	float MoonCycleMinuteOffset;
	/// Moon Billboard Material using Sky Shader
	Material* MoonMaterial;
	/// Technique handle of Moon billboard in Sky Shader
	D3DXHANDLE MoonShaderTechnique;

	/// Dir Light created for Sun/Moon ambience
	LightState SunLightState;
	LightState MoonLightState;

	/// whether the fog is currently fading to a value
	bool m_FadeFog;
	float FadeFogRedStep;
	float FadeFogGreenStep;
	float FadeFogBlueStep;
	float FadeFogDensityStep;
	float FadeFogTotalTime;
	float FadeFogCurTime;

	/// current fog values
	float FogDensity;
	float FogRed;
	float FogGreen;
	float FogBlue;

	/// Server sends time updates every few game minutes to ensure long-term synchronization
	float m_Server_LastSentDayTime;

	/// speed of the day cycle, in day minutes per game second
	float SkyMinutesPerGameSecond;
	/// the current Sky day time, in minutes (60 min * 24 hr = 1440 min per day)
	float DayTimeMinutes;

	/// the handles to the Spherical Harmonics probe environment maps, each one corresponding to a skybox up to 8
	int SkySHProbes[8];
	/// scales all the Spherical Harmonics probe environment map intensities by a factor, for adjustment of ambience intensity
	float SH_EnvironmentMapMultiplier;
	float SunLightIntensityMultiplier;
	float MoonLightIntensityMultiplier;

	/// how many skyboxes the system will blend between. 4 is default, 8 is max.
	int NumSkies;
	/// Stores interpolation state of the Sky Mixer values
	struct MixFade
	{
		bool Active;
		float TotalStep;
		float CurTime;
		float TotalTime;
	};
	/// One interpolation structure for each Sky Mixer
	MixFade SkyBGMixFades[NUM_SKY_FADES];

	/// whether the sky does lightning effects
	bool m_HasLightning;
	/// the time that lightning starts
	float LightningStartMinutes;
	/// the time that lightning ends
	float LightningEndMinutes;
	/// the current intensity of the lightning
	float CurLightningIntensity;
	/// Countdown timer to the next lightning flash
	float LightningFlashTimer;
	/// Lightning flash frequency multiplier
	float LightningFlashFrequency;

	/// whether moon is drawn, taken from map INI
	bool m_bDrawMoon;
	/// whether sun is drawn, taken from map INI
	bool m_bDrawSun;
	/// Draws the Moon billboard and updates the Moon ambient Light
	void DrawMoon(Vector& pos);
	/// Draws the Sun billboard and updates the Sun ambient Light
	void DrawSun(Vector& pos);

	/// Draws the Cloud FX
	void DrawClouds(Vector& pos);
	/// Updates the interpolation of the Sky faders
	void UpdateFades(float TimeInterval);

	Sound LightningSounds[2];
};

#endif