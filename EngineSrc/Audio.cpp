//=========== Copyright (c) 2003, Tim Johnson. All rights reserved. ===========
// File: Audio.cpp
//
// TODO URGENT: Don't update ALL buffers every frame
//
// NOTE: Must try to keep sounds in same format. No use offering crap 8bit,
// and there's no use lowering the primary buffer if you don't lower the secondary ones!!
//
// globalBuffers = Allocated buffers. Use GetCaps() to find out how many have been hardware-allocated
//
// WHAT THE HELL IS GOING ON WITH ATTENUATION
// Sound should be near-inaudiable at say 0.1 - 0.01
// 5 = full
// 10 = half
// 20 = quarter
// which is: 50 - 500
// but it takes nearer 1000 (0.001)
// a test in winamp shows ~1% is where it becomes inaudible
//=============================================================================
#include "stdafx.h"
#include <windows.h>
#include <basetsd.h>
#include <commdlg.h>
#include <commctrl.h>
#include <dmusicc.h>
#include <dmusici.h>
#include <cguid.h>
#include <dxerr9.h>
#include <tchar.h>
#include "DSUtil.h"
#include "StreamingOgg.h"
#include "Profiler.h"

static IDirectSound3DListener* listener = NULL; // The single listener object
static CSoundManager* pSoundManager = NULL;

//FIXME: Why so slow? Read up on Play() delays, and also debug to see if anything is created, and time just Play()

void Sound::Free(){
	ResourceManager::Instance()->Remove(this);
	AudioDevice* dev = Engine::Instance()->AudioSys;
	if(!dev || !pSoundManager) return;

	Stop();

	if(pSound){
		vector_erase(dev->sounds,this);
	}

    SAFE_DELETE( pSound );

	isLoaded = false;
}

Sound::Sound(){ 
	CurVolume = 1;
	isLoaded = false;
	pSound = NULL;
	FileName = "NOT LOADED";
}

void Sound::FadeTo(float fVolume, float fFadePerSec){ 
	FadePerSec = fFadePerSec;
	FadeVolume = fVolume;
	// Add to fading sounds only if not already there
	// If it's already there, then we've just updated the fade target volume, thus overriding the old fade
	if(std::find(AudioDevice::Instance()->fadingSounds.begin(),AudioDevice::Instance()->fadingSounds.end(),this) == AudioDevice::Instance()->fadingSounds.end())
		AudioDevice::Instance()->fadingSounds.push_back(this);
}

void Sound::Load2D(string FileName, bool variableFrequency){
	if(isLoaded) return;
	Load(FileName,false,variableFrequency);
}

void Sound::Load(string FileName, bool variableFrequency){
	if(isLoaded) return;
	Load(FileName,true,variableFrequency);
}

//-----------------------------------------------------------------------------
// Name: Sound::Load()
// Desc: Loads a sound file into a music segment
// Note: Music Segment has internal management to avoid duplicate loading. Cool huh?
//-----------------------------------------------------------------------------
void Sound::Load(string FileName, bool is3D, bool variableFrequency){
	string shortName = FileName;

	ResourceManager::Instance()->Add(this);

	if(Engine::Instance()->AudioSys == NULL) return;

	bIs3D = is3D;
	Free();

	// Get the right path for this file
	if(!FindMedia(FileName,"Sounds")){
		if(Engine::Instance()->MainConfig->GetBool("WarnOnMissingFiles"))
			Warning("Failed to find sound file: %s", FileName.c_str());
		else
			LogPrintf(LOG_MEDIUM,"Failed to find sound file: %s", FileName.c_str());
		return;
	}

	this->FileName = FileName;
	isLoaded = true;

	if(!pSoundManager){
		Error("Tried to load file before sound system was initialized!\nFile: %s",FileName.c_str());
	}

	// See if this sound has already been loaded...
	AudioDevice* dev = Engine::Instance()->AudioSys;
	for(int i=0;i<dev->sounds.size();i++){
		if(dev->sounds[i]->FileName == FileName){
			if(dev->sounds[i]->bIs3D != bIs3D) // Assert they are the same type
				continue;//Error("You've loaded the '%s' sound once with Load2D() and once with Load()!",FileName.c_str());
			LogPrintf(LOG_HIGH,"Loading Sound '%s' [From runtime cache]",shortName.c_str());

			// Yes! Duplicate it
			dev->sounds[i]->pSound->Duplicate(pSound);
			AudioDevice::Instance()->sounds.push_back(this);
			return;
		}
	}
	// Otherwise load the file normally...
	LogPrintf(LOG_HIGH,"Loading Sound '%s' [From disk]",shortName.c_str());
	AudioDevice::Instance()->LoadSoundBuffer((char*)FileName.c_str(),pSound,is3D,variableFrequency);
	if(pSound)
	{
		AudioDevice::Instance()->sounds.push_back(this);
	}
}





void Sound::SetFrequency(LONG frequencyHz){
	if(!pSound)
		return;

	for(int i=0;i<pSound->GetNumBuffers();i++)
		pSound->GetBuffer(i)->SetFrequency( frequencyHz );
}


void Sound::SetVolume(float fVolume){
	if(CurVolume == fVolume)
		return;

	if(fVolume < 0.0001f)
		fVolume = 0.0001f;

	LONG volume;// = 5000 * log10((vol + 1) / 100.0);//(LONG)(100.0 * 20.0 * log10(fVolume));

	if (fVolume >= 1.0f) volume =  0;
	else if (fVolume <= 0.0f) volume = DSBVOLUME_MIN;
	else {
		static const float adj=3321.928094887F;  // 1000/log10(2)
		volume = int(float(log10(fVolume)) * adj);
	}


	if (volume < DSBVOLUME_MIN) 
		volume = DSBVOLUME_MIN;
	else if (volume > DSBVOLUME_MAX) 
		volume = DSBVOLUME_MAX;

	CurVolume = fVolume;

	if(!pSound)
		return;

	for(int i=0;i<pSound->GetNumBuffers();i++)
		DXASSERT(pSound->GetBuffer(i)->SetVolume( volume ));
}

//-----------------------------------------------------------------------------
// Name: Play()
// Desc: Plays a sound into an emitter
//-----------------------------------------------------------------------------
void Sound::Play(SoundEmitter& Emitter, SoundFlags flags, float volume){
	if(!pSound) return;

	Emitter.sounds.push_back(this);

	HRESULT hr;
	if( FAILED( hr = pSound->Play(0, /*DSBPLAY_TERMINATEBY_DISTANCE|*/(flags & SOUND_LOOP)?DSBPLAY_LOOPING:0 ) ) )
	{
		Error("Sound::Play() Error playing sound '%s'. ErrCode: %s",FileName,DXGetErrorString9(hr));
	}

	SetVolume(volume);
	Update3DParameters(Emitter.pos,Emitter.vel,Emitter.range);

    SetFrequency(pSound->m_pWaveFile->m_pwfx->nSamplesPerSec*AudioDevice::Instance()->m_PlaybackSpeed);
}

//-----------------------------------------------------------------------------
// Name: Play()
// Desc: Plays a sound
//-----------------------------------------------------------------------------
void Sound::Play(Vector& position, Vector& velocity, float Range, SoundFlags flags, float volume){
	if(!pSound) return;

	if(!bIs3D)
		Error("Trying to play 2D sound as 3D sound! File '%s'",FileName.c_str());

	HRESULT hr;
	if( FAILED( hr = pSound->Play( 0, (flags == SOUND_LOOP)?DSBPLAY_LOOPING:0 ) ) )
	{
		Error("Sound::Play() Error playing sound '%s'. ErrCode: %s",FileName,DXGetErrorString9(hr));
	}

	SetVolume(volume);
	Update3DParameters(position,velocity,Range);

    SetFrequency(pSound->m_pWaveFile->m_pwfx->nSamplesPerSec*AudioDevice::Instance()->m_PlaybackSpeed);
}


void Sound::Play2D(DWORD flags, float volume){
	if(!pSound) return;

	if(bIs3D)
		Error("Trying to play 3D sound as 2D sound! This means you called PlayNo3D() but called Load() instead of Load2D()\n File '%s'",FileName.c_str());

	HRESULT hr;
	if( FAILED( hr = pSound->Play( 0, (flags == SOUND_LOOP)?DSBPLAY_LOOPING:0 ) ) )
	{
		Error("Sound::Play() Error playing sound '%s'. ErrCode: %s",FileName,DXGetErrorString9(hr));
	}

    SetFrequency(pSound->m_pWaveFile->m_pwfx->nSamplesPerSec*AudioDevice::Instance()->m_PlaybackSpeed);
	SetVolume(volume);
}

//-----------------------------------------------------------------------------
// Name: SetObjectProperties()
// Desc: Sets the position and velocity on the 3D buffer
//-----------------------------------------------------------------------------
VOID Sound::Update3DParameters( Vector& pos, Vector& vel, float Range)
{
	if(!pSound) return;

	// Build the buffer once
	DS3DBUFFER ds;
	ZeroMemory(&ds,sizeof(DS3DBUFFER));
	ds.dwSize = sizeof(DS3DBUFFER);
	ds.dwOutsideConeAngle = 360;
	ds.dwInsideConeAngle  = 360;
	ds.vConeOrientation.z = 1;
	ds.vPosition = *(D3DVECTOR*)&pos;
	ds.vVelocity = *(D3DVECTOR*)&vel;
	ds.flMinDistance = Range * 0.01f;
	ds.flMaxDistance = DS3D_DEFAULTMAXDISTANCE;
	ds.dwMode = DS3DMODE_NORMAL;

    // Every change to 3-D sound buffer and listener settings causes 
    // DirectSound to remix, at the expense of CPU cycles. 
    // To minimize the performance impact of changing 3-D settings, 
    // use the DS3D_DEFERRED flag in the dwApply parameter of any of 
    // the IDirectSound3DListener or IDirectSound3DBuffer methods that 
    // change 3-D settings. Then call the IDirectSound3DListener::CommitDeferredSettings 
    // method to execute all of the deferred commands at once.
	for(int i=0;i<pSound->GetNumBuffers();i++){
		// FIXME: This is just test code
		// It's terribly slow as it gets/releases all the buffers every frame
		LPDIRECTSOUND3DBUFFER ds3DBuffer;
		pSound->Get3DBufferInterface(i,&ds3DBuffer);
		ds3DBuffer->SetAllParameters(&ds,DS3D_DEFERRED);

		/*HRESULT hr;
		if(FAILED(hr=ds3DBuffer->SetAllParameters(&ds,DS3D_IMMEDIATE))){
			Error("%s Vel: %f Loc: %f/%f/%f Range: %f",
				DXGetErrorString9(hr)
				,vel.Length(),
				pos.x,pos.y,pos.z,Range);
		}*/

		ds3DBuffer->Release();
	}
}



//-----------------------------------------------------------------------------
// Name: IsPlaying()
// Desc: Returns the state of the sound
// Note: Based on ear latency. Don't base timing on it
//-----------------------------------------------------------------------------
bool Sound::IsPlaying(){
	if(!pSound) return false;
	return pSound->IsSoundPlaying();
}

//-----------------------------------------------------------------------------
// Name: Stop()
// Desc: Stops a sound
//-----------------------------------------------------------------------------
void Sound::Stop(){
	if(pSound)
		pSound->Stop();
}


//-----------------------------------------------------------------------------
// Name: SoundEmitter()
//-----------------------------------------------------------------------------
SoundEmitter::SoundEmitter(){
	range = 1;
}


void SoundEmitter::Update3DParameters(Vector& Position, Vector& Velocity, float Range){
	if(Engine::Instance()->AudioSys == NULL) return;

	pos = Position;
	vel = Velocity;
	range = Range;

	for(int i=0;i<sounds.size();i++){
		// Erase sounds that aren't playing
		if(!sounds[i]->IsPlaying()){
			sounds.erase(sounds.begin()+i);
			i--;
			continue;
		}
		sounds[i]->Update3DParameters(Position,Velocity,Range);
	}


}


SoundEmitter::~SoundEmitter(){
}



AudioDevice* AudioDevice::Instance () 
{
    static AudioDevice inst;
    return &inst;
}



void AudioDevice::Update(Camera* cam){
	SetListenerProperties(cam->Location,cam->Velocity,cam->Direction);

	// Fading sound management
	for(int i=0;i<fadingSounds.size();i++){
		// Fade up or down
		if(fadingSounds[i]->FadeVolume < fadingSounds[i]->CurVolume)
			fadingSounds[i]->CurVolume -= fadingSounds[i]->FadePerSec * GDeltaTime;
		else
			fadingSounds[i]->CurVolume += fadingSounds[i]->FadePerSec * GDeltaTime;

		fadingSounds[i]->SetVolume(fadingSounds[i]->CurVolume);

		// Done fading, remove from array
		if(fadingSounds[i]->CurVolume <= fadingSounds[i]->FadeVolume){
			fadingSounds.erase(fadingSounds.begin()+i);
			i--;
		}
	}

    static int count = 0;
	// Remix sound buffer using adaptive degredation
    if(count > 10)
    {
        StartMiniTimer();
	    HRESULT hr;
	    if(FAILED(hr = listener->CommitDeferredSettings()))
		    Error("AudioDevice.Update() listener->CommitDeferredSettings() failed: %s. Vel: %f Loc: %f/%f/%f Dir:%f",DXGetErrorString9(hr),cam->Velocity.Length(),cam->Location.x,cam->Location.y,cam->Location.z,cam->Direction.Length());
        float AudioMS = StopMiniTimer();
        Profiler::Get()->AudioMS += AudioMS;

        // Decide how quickly to update depending on how much the audio is costing
        if(AudioMS < 1)
            count = 10;
        else if(AudioMS < 2)
            count = 5;
        else // Eek!! > 2MS!! Update only every 10 frames
            count = 0;
    }
    count++;
}


AudioDevice::~AudioDevice(){
}

//-----------------------------------------------------------------------------
// Name: Shutdown()
// Desc: Does what it says on the label
//-----------------------------------------------------------------------------
void AudioDevice::Shutdown(){
	waveOutSetVolume(NULL,originalVolume);

	oggPlayer.FinalShutdown();

	// Free any sounds that haven't been released
	for(int i=0;i<sounds.size();i++){
		sounds[0]->Free(); // Each Free() will delete itself from the array
		i--;
	}

	SAFE_RELEASE( listener );
	SAFE_DELETE( pSoundManager );
}


//-----------------------------------------------------------------------------
// Name: Initialize()
// Desc: Initializes DSound & DMusic
//-----------------------------------------------------------------------------
void AudioDevice::Initialize(HWND hDlg)
{
    HRESULT hr; 
	InitCommonControls();
	waveOutGetVolume(NULL,&originalVolume);

    m_PlaybackSpeed = 1;

	// Create a static IDirectSound in the CSound class.  
    // Set coop level to DSSCL_PRIORITY, and set primary buffer 
    // format to stereo, 22-44kHz and 8/16-bit output.
    pSoundManager = new CSoundManager();

    hr = pSoundManager->Initialize( hDlg, DSSCL_PRIORITY );
    
	SetMasterParameters();

    // Get the 3D listener, so we can control its params
    hr |= pSoundManager->Get3DListenerInterface( &listener );

    if( FAILED(hr) )
    {
		Error("Error initializing DirectSound: %s",DXGetErrorString9(hr));
        return;
    }

    listener->SetRolloffFactor(Engine::Instance()->MainConfig->GetFloat("3DAudioRolloff"),DS3D_IMMEDIATE);

	oggPlayer.Initialize(pSoundManager->GetDirectSound());
}


void AudioDevice::SetMasterParameters(float fVolume/*=-1*/, LONG frequencyKhz/*=-1*/){
	ConfigFile* cfg = Engine::Instance()->MainConfig;

	if(frequencyKhz == -1){
		if(cfg->GetInt("AudioFrequency") == 22)
			frequencyKhz = 22050;
		else if(cfg->GetInt("AudioFrequency") == 44)
			frequencyKhz = 44100;
	}
	else{
		if(frequencyKhz == 22)
			frequencyKhz = 22050;
		else if(frequencyKhz == 44)
			frequencyKhz = 44100;
	}

	if(fVolume == -1)
		fVolume = cfg->GetFloat("AudioVolume");

	// Use waveOut, setting the primary buffer volume sucks balls, works terribly
	WORD max = (WORD) 0xFFFF * fVolume;
	DWORD volume = MAKELONG(max,max);
	waveOutSetVolume(NULL,volume);

	HRESULT hr;
	hr = pSoundManager->SetPrimaryBufferFormat( 2, frequencyKhz, 16);
    
	if(FAILED(hr))
		Error("SetPrimaryBufferFormat() failed. frequencyKhz=%d sampleBits=16\nYour soundcard may not support these settings, your drivers may be out of date, or you may not have the soundcard installed correctly.",frequencyKhz);
}

//-----------------------------------------------------------------------------
// Name: 
//-----------------------------------------------------------------------------
void AudioDevice::SetPlaybackSpeed(float fSpeed)
{
    m_PlaybackSpeed = fSpeed;
    for(int i=0;i<sounds.size();i++)
    {
        if(sounds[i]->pSound->GetNumBuffers() == 0)
            continue;

        DWORD freq = sounds[i]->pSound->m_pWaveFile->m_pwfx->nSamplesPerSec;
        sounds[i]->SetFrequency(freq*fSpeed);
    }
}


//-----------------------------------------------------------------------------
// Name: SetObjectProperties()
// Desc: Sets the position and velocity on the 3D buffer
//-----------------------------------------------------------------------------
void AudioDevice::SetListenerProperties(Vector& position, Vector& velocity, Vector& direction )
{
	if(FAILED(listener->SetPosition( position.x,position.y,position.z, DS3D_DEFERRED  )))
		Error("Failed setting DirectSound listener position.\nThe position of the camera was (%f,%f,%f)",position.x,position.y,position.z);
	
	if(FAILED(listener->SetVelocity( velocity.x,velocity.y,velocity.z, DS3D_DEFERRED  )))
		Error("Failed setting DirectSound listener velocity");
	
	listener->SetOrientation( direction.x, direction.y, direction.z,
		0,1,0,// Top orientation. DX should auto-calculate the correct orientation if looking up
		DS3D_DEFERRED );
}




void AudioDevice::LoadSoundBuffer(char *strFileName, CSound*& pSound, bool is3D, bool variableFrequency)
{
    variableFrequency = true;
    GUID    guid3DAlgorithm = DS3DALG_DEFAULT;
    HRESULT hr; 

    if( pSound )
    {
        pSound->Stop();
        pSound->Reset();
    }

    // Free any previous sound, and make a new one
    SAFE_DELETE( pSound );

    CWaveFile waveFile;
    waveFile.Open( strFileName, NULL, WAVEFILE_READ );
    WAVEFORMATEX* pwfx = waveFile.GetFormat();
    if( pwfx == NULL )
    {
        Error("Invalid wave file format for '%s'",strFileName);
        return;
    }

    if( pwfx->nChannels > 1 && is3D )
    {
        // Too many channels in wave.  Sound must be mono when using DSBCAPS_CTRL3D
		LogPrintf("ERROR: Sound file '%s': Wave file must be mono for 3D control. File will NOT be loaded.",strFileName);
        return;
    }

    if( pwfx->wFormatTag != WAVE_FORMAT_PCM )
    {
        // Sound must be PCM when using DSBCAPS_CTRL3D
        Error("Sound file '%s': Wave file must be PCM for 3D control.",strFileName);
        return;
    }

    // Get the software DirectSound3D emulation algorithm to use (if software)
	switch(Engine::Instance()->MainConfig->GetInt("SoftwareSoundQuality")){
		case 0: // User selected DS3DALG_NO_VIRTUALIZATION  
			guid3DAlgorithm = DS3DALG_NO_VIRTUALIZATION;
			break;

		case 1: // User selected DS3DALG_HRTF_LIGHT
			guid3DAlgorithm = DS3DALG_HRTF_LIGHT;
			break;
		case 2: // User selected DS3DALG_HRTF_FULL  
			guid3DAlgorithm = DS3DALG_HRTF_FULL;
			break;
    }

	if(!is3D)
		guid3DAlgorithm = GUID_NULL;

    // Load the wave file into a DirectSound buffer
	DWORD flags = (is3D?DSBCAPS_CTRL3D|DSBCAPS_MUTE3DATMAXDISTANCE:0) | DSBCAPS_CTRLVOLUME | 
		(variableFrequency?DSBCAPS_CTRLFREQUENCY:0) | DSBCAPS_LOCDEFER | DSBCAPS_GETCURRENTPOSITION2   ;


	hr = pSoundManager->Create( &pSound, strFileName, flags , guid3DAlgorithm, 1 );  
    if( FAILED( hr ) || hr == DS_NO_VIRTUALIZATION )
    {
        if( DS_NO_VIRTUALIZATION == hr )
        {
            Error( "The 3D virtualization algorithm requested is not supported under this "
                        "operating system.  It is available only on Windows 2000, Windows ME, and Windows 98 with WDM "
                        "drivers and beyond.  Please change config 'SoftwareSoundQuality' value to '0'");
        }
		else
			Error("CreateSound(%s) failed: %s",strFileName,DXGetErrorString9(hr));

        return; 
    }
}



//============================================================================
//----------------------------------------------------------------------------
//Sound Capturing
//----------------------------------------------------------------------------
//============================================================================


void SoundCapture::Initialize(short samplesPerSecond, short bitsPerSample,short blockSize,
							  char channels, SoundCapture_Callback callBack,
							  void* userInfo)
{
	//Already Intialized??
	if (m_DSCapture)
		return;

	//Assign the m_callback
	m_callBack=callBack;
	m_UserInfo=userInfo;

	//Create Direct Capture
	if (FAILED(DirectSoundCaptureCreate( NULL, &m_DSCapture, NULL ) ))
		Error("Can't Create sound capture device.");

	//Create the wave format
	m_WaveFormat=new WAVEFORMATEX();
	ZeroMemory((void*)m_WaveFormat,sizeof(m_WaveFormat));
	m_WaveFormat->wFormatTag = WAVE_FORMAT_PCM;
	m_WaveFormat->nSamplesPerSec = samplesPerSecond;
	m_WaveFormat->nChannels = channels;
	m_WaveFormat->wBitsPerSample=bitsPerSample * sizeof(short);
	m_WaveFormat->nBlockAlign = channels * (m_WaveFormat->wBitsPerSample / 8);
	m_WaveFormat->nAvgBytesPerSec = m_WaveFormat->nBlockAlign * samplesPerSecond;
   
	m_NotifySize = blockSize; 
    m_BufferSize = m_NotifySize * 2;

	//Create the buffer description
	DSCBUFFERDESC dsBufferDesc;
    ZeroMemory( &dsBufferDesc, sizeof(dsBufferDesc) );
    dsBufferDesc.dwSize        = sizeof(dsBufferDesc);
    dsBufferDesc.dwBufferBytes = m_BufferSize;
    dsBufferDesc.lpwfxFormat   = m_WaveFormat; 

	//Create capture buffer
	if( FAILED(m_DSCapture->CreateCaptureBuffer( &dsBufferDesc,
												 &m_DSCaptureBuffer, 
												 NULL ) ) )
	{
       Error("Can't create capture buffer.");
	}


	//--CREATE EVENT--
	//Create notification event
	m_NotificationEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
	if (!m_NotificationEvent)
		Error("Can't create notification event.");

	//Create notification thread
	m_NotifyThread  = CreateThread( NULL, 0, NotificationProc, this, 0, &m_NotifyThreadID );
	if (!m_NotifyThread)
		Error("Can't create notification thread.");
	//----------------

	//--CREATE NOTIFICATIONS--
	//Query Notification Interface
    if( FAILED(m_DSCaptureBuffer->QueryInterface( IID_IDirectSoundNotify, 
                                                    (VOID**)&m_DSNotify ) ) )
	{
       Error("Can't query direct sound notify interface.");
	}

    // Setup the notification positions
	DSBPOSITIONNOTIFY positionNotify[2];
    for( int i = 0; i < 2; i++ )
    {
        positionNotify[i].dwOffset = (m_NotifySize * i) + m_NotifySize - 1;
        positionNotify[i].hEventNotify = m_NotificationEvent;             
    }
    
    if( FAILED( m_DSNotify->SetNotificationPositions( 2,  positionNotify ) ) )
		Error("Can't create notification positions for direct sound capture.");

	m_nextCaptureOffset=0;
}


//Notification Procedure
DWORD WINAPI SoundCapture::NotificationProc( LPVOID lpParameter )
{
	SoundCapture *soundCapture = (SoundCapture*)lpParameter;
	MSG     msg;
	DWORD   dwResult;
	BOOL    bDone = FALSE;

	while( !bDone ) 
	{ 
		dwResult = MsgWaitForMultipleObjects( 1, &soundCapture->m_NotificationEvent, FALSE, INFINITE, QS_ALLEVENTS );

		switch( dwResult )
		{
		case WAIT_OBJECT_0 + 0:
			soundCapture->Update();
			break;

		case WAIT_OBJECT_0 + 1:
			// Messages are available
			while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) ) 
			{ 
				if( msg.message == WM_QUIT )
					bDone = TRUE;
			}
			break;
		}

	}

	return 0;
}

void SoundCapture::Update()
{
	//Intialized???
	if(!m_DSCaptureBuffer)
       return;

    void*   CaptureData    = NULL;
    DWORD   CaptureLength;
    void*   CaptureData2   = NULL;
    DWORD   CaptureLength2;

    DWORD   ReadPosition;
    DWORD   CapturePosition;

    long LockSize=0;

	//Get Current Position from the capture buffer
    if( FAILED(m_DSCaptureBuffer->GetCurrentPosition( &CapturePosition, &ReadPosition ) ) )
        Error("Can't get Sound Capture buffer position.");

	//Calculate the lock size
    LockSize = ReadPosition - m_nextCaptureOffset;
    if( LockSize < 0 )
        LockSize += m_BufferSize;
    LockSize -= (LockSize % m_NotifySize);
    if( LockSize == 0 )
        return;

    // Lock the capture buffer
    if( FAILED(m_DSCaptureBuffer->Lock( m_nextCaptureOffset, LockSize, 
                                          &CaptureData, &CaptureLength, 
                                          &CaptureData2, &CaptureLength2, 0L ) ) )
        Error("Can't lock Capture Buffer");

	//raise the callback
	if (m_callBack)
		m_callBack(m_UserInfo,CaptureLength,CaptureData);

    // Move the capture offset
    m_nextCaptureOffset += CaptureLength; 
    m_nextCaptureOffset %= m_BufferSize;

    if(CaptureData2)
    {
		//raise the callback
		if (m_callBack)
			m_callBack(m_UserInfo,CaptureLength2,CaptureData2);

        // Move the capture offset
        m_nextCaptureOffset += CaptureLength2; 
        m_nextCaptureOffset %= m_BufferSize; 
    }

    // Unlock the capture buffer
    m_DSCaptureBuffer->Unlock( CaptureData,  CaptureLength, 
                           CaptureData2, CaptureLength2 );
}

//Start Capturing Sound
void SoundCapture::Start()
{
	if (m_DSCaptureBuffer)
		m_DSCaptureBuffer->Start(DSCBSTART_LOOPING);
}

//Stop Capturing Sound
void SoundCapture::Stop()
{
	if (m_DSCaptureBuffer)
		m_DSCaptureBuffer->Stop();
}

//Shutdown Sound Capture
void SoundCapture::ShutDown()
{
	SAFE_DELETE(m_WaveFormat);
	if(m_NotifyThread)
	{
		PostThreadMessage( m_NotifyThreadID , WM_QUIT, 0, 0 );
		WaitForSingleObject( m_NotifyThread , INFINITE );
		CloseHandle( m_NotifyThread );
		if (m_NotificationEvent)
			CloseHandle( m_NotificationEvent );
	}

	SAFE_RELEASE(m_DSNotify);
	SAFE_RELEASE(m_DSCaptureBuffer);
}



//============================================================================
//----------------------------------------------------------------------------
//Sound Streaming
//----------------------------------------------------------------------------
//============================================================================


void SoundStreaming::Initialize(short samplesPerSecond, short bitsPerSample,short blockSize,
							  char channels, SoundStreaming_Callback callBack,
							  void* userInfo)
{
	if (!pSoundManager)
		Error("Sound Engine must be intialized first.");
	//Assign the m_callback
	m_callBack=callBack;
	m_UserInfo=userInfo;

	//Create the wave format
	m_WaveFormat=new WAVEFORMATEX();
	ZeroMemory((void*)m_WaveFormat,sizeof(m_WaveFormat));
	m_WaveFormat->wFormatTag = WAVE_FORMAT_PCM;
	m_WaveFormat->nSamplesPerSec = samplesPerSecond;
	m_WaveFormat->nChannels = channels;
	m_WaveFormat->wBitsPerSample=bitsPerSample * sizeof(short) ;
	m_WaveFormat->nBlockAlign = channels * (m_WaveFormat->wBitsPerSample / 8);
	m_WaveFormat->nAvgBytesPerSec = m_WaveFormat->nBlockAlign * samplesPerSecond;

	m_NotifySize =blockSize;
    m_BufferSize = m_NotifySize * 2;

	//Create the buffer description
	DSBUFFERDESC dsBufferDesc;
    ZeroMemory( &dsBufferDesc, sizeof(dsBufferDesc) );
	dsBufferDesc.guid3DAlgorithm = GUID_NULL;
    dsBufferDesc.dwSize        = sizeof(dsBufferDesc);
    dsBufferDesc.dwBufferBytes = m_BufferSize;
	dsBufferDesc.dwFlags        = DSBCAPS_GLOBALFOCUS  | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLVOLUME ;
	dsBufferDesc.dwReserved     = 0;
    dsBufferDesc.lpwfxFormat   = m_WaveFormat; 

	//Create the sound buffer
	if( FAILED(pSoundManager->GetDirectSound()->CreateSoundBuffer( &dsBufferDesc,
												 &m_DSBuffer, 
												 NULL ) ) )
	{
       Error("Can't create straming buffer.");
	}


	//--CREATE EVENT--
	//Create notification event
	m_NotificationEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
	if (!m_NotificationEvent)
		Error("Can't create notification event.");

	//Create notification thread
	m_NotifyThread  = CreateThread( NULL, 0, NotificationProc, this, 0, &m_NotifyThreadID );
	if (!m_NotifyThread)
		Error("Can't create notification thread.");
	//----------------

	//--CREATE NOTIFICATIONS--
	//Query Notification Interface
    if( FAILED(m_DSBuffer->QueryInterface( IID_IDirectSoundNotify, 
                                                    (VOID**)&m_DSNotify ) ) )
	{
       Error("Can't query direct sound notify interface.");
	}

    // Setup the notification positions
	DSBPOSITIONNOTIFY positionNotify[2];
    for( int i = 0; i < 2; i++ )
    {
        positionNotify[i].dwOffset = (m_NotifySize * i) + m_NotifySize - 1;
        positionNotify[i].hEventNotify = m_NotificationEvent;             
    }
    
    if( FAILED( m_DSNotify->SetNotificationPositions( 2,  positionNotify ) ) )
		Error("Can't create notification positions for direct sound.");

	m_nextPlayOffset=0;
}


//Notification Procedure
DWORD WINAPI SoundStreaming::NotificationProc( LPVOID lpParameter )
{
	SoundStreaming *soundStreaming = (SoundStreaming*)lpParameter;
	MSG     msg;
	DWORD   dwResult;
	BOOL    bDone = FALSE;

	while( !bDone ) 
	{ 
		dwResult = MsgWaitForMultipleObjects( 1, &soundStreaming->m_NotificationEvent, FALSE, INFINITE, QS_ALLEVENTS );

		switch( dwResult )
		{
		case WAIT_OBJECT_0 + 0:
			soundStreaming->Update();
			break;

		case WAIT_OBJECT_0 + 1:
			// Messages are available
			while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) ) 
			{ 
				if( msg.message == WM_QUIT )
					bDone = TRUE;
			}
			break;
		}
	}

	return 0;
}

void SoundStreaming::Update()
{
	//Intialized???
	if(!m_DSBuffer)
       return;

    void*   StreamData    = NULL;
    DWORD   StreamLength;
    void*   StreamData2   = NULL;
    DWORD   StreamLength2;

    DWORD   ReadPosition;
    DWORD   StreamPosition;

    long LockSize=0;

	//Get Current Position from the streaming buffer
    if( FAILED(m_DSBuffer->GetCurrentPosition( &StreamPosition, &ReadPosition ) ) )
        Error("Can't get Sound Streaming buffer position.");

	//Calculate the lock size
    LockSize = ReadPosition - m_nextPlayOffset;
    if( LockSize < 0 )
        LockSize += m_BufferSize;
    LockSize -= (LockSize % m_NotifySize);
    if( LockSize == 0 )
        return;

    // Lock the capture buffer
    if( FAILED(m_DSBuffer->Lock( m_nextPlayOffset, LockSize, 
                                          &StreamData, &StreamLength, 
                                          &StreamData2, &StreamLength2, 0L ) ) )
        Error("Can't lock Stream Buffer");

	//raise the callback
	if (m_callBack)
		m_callBack(m_UserInfo,StreamLength,StreamData);

    // Move the streaming offset
    m_nextPlayOffset += StreamLength; 
    m_nextPlayOffset %= m_BufferSize;

    if(StreamData2)
    {
		//raise the callback
		if (m_callBack)
			m_callBack(m_UserInfo,StreamLength2,StreamData2);

        // Move the streaming offset
        m_nextPlayOffset += StreamLength2; 
        m_nextPlayOffset %= m_BufferSize; 
    }

    // Unlock the Streaming buffer
    m_DSBuffer->Unlock( StreamData,  StreamLength, 
                           StreamData2, StreamLength2 );
}

//Start Streaming Sound
void SoundStreaming::Start()
{
	if (m_DSBuffer)
		m_DSBuffer->Play(0,0,DSBPLAY_LOOPING);
}

//Stop Streaming Sound
void SoundStreaming::Stop()
{
	if (m_DSBuffer)
		m_DSBuffer->Stop();
}

//Shutdown Sound Stream
void SoundStreaming::ShutDown()
{
	SAFE_DELETE(m_WaveFormat);
	if(m_NotifyThread)
	{
		PostThreadMessage( m_NotifyThreadID , WM_QUIT, 0, 0 );
		WaitForSingleObject( m_NotifyThread , INFINITE );
		CloseHandle( m_NotifyThread );
		if (m_NotificationEvent)
			CloseHandle( m_NotificationEvent );
	}

	SAFE_RELEASE(m_DSNotify);
	SAFE_RELEASE(m_DSBuffer);
}



