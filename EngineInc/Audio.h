//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
/// Name: Audio
/// Desc: Audio manager and sound classes
/// Author: Tim Johnson
//====================================================================================
#ifndef AUDIO_H
#define AUDIO_H

/// Audio device powers audio subsystems
class ENGINE_API AudioDevice {
protected:
    // Master frequency at current scaling
    float m_PlaybackSpeed;
	DWORD originalVolume; /// User's volume before game was launched
	/// For resource management
	friend class Sound;
	vector<Sound*> sounds; 
	/// For sounds fading in/out
	vector<Sound*> fadingSounds; 

	AudioDevice(){}
	AudioDevice(const AudioDevice&);
	AudioDevice& operator= (const AudioDevice&);
	~AudioDevice();

	void SetListenerProperties(Vector& position, Vector& velocity, Vector& direction );
	void LoadSoundBuffer(char *strFileName, class CSound*& pSound, bool is3D, bool variableFrequency);
 
	friend class Engine;

	void Initialize(HWND hwnd);
	void Shutdown();
	/// Remixes sounds and updates from sound Listener's position, orientation, and velocity
	void Update(class Camera* Listener);

public:

	static AudioDevice* Instance();
	/// Master audio parameters. -1 = Config Default
	void SetMasterParameters(float fVolume = -1, LONG frequencyKhz = -1); 
    /// Playback speed
    void SetPlaybackSpeed(float fSpeed=1);
};



/// \brief 3D positional sound source to play sounds through
//
/// This is a pseudo-object which just keeps a list of all sounds that play through it
/// The purpose of having it is a convenience, and for the future, as it may be a better design methodology
/// (sharing sound buffers between sound data)
class ENGINE_API SoundEmitter {
private:
	friend class Sound;
	vector<Sound*> sounds;
	Vector pos, vel;
	float range;
public:
	void Update3DParameters(Vector& Position, Vector& Velocity, float Range);
	
	SoundEmitter();
	~SoundEmitter();
};



/// Sound flags
enum SoundFlags{
	NONE,
	SOUND_LOOP  /// looping sound
};

/// A sound object
class ENGINE_API Sound {
private:
    friend class AudioDevice;
	bool bIs3D;
	CSound* pSound;
	string FileName;
	bool isLoaded;

	void Load(string FileName, bool is3D, bool variableFrequency=false);

public:
	bool IsLoaded(){ return isLoaded; }
	void Load(string FileName, bool variableFrequency=false);
	void Load2D(string FileName, bool variableFrequency=false);

	Sound();
	~Sound(){ Free(); }
	
	/// Emitter properties
	void Update3DParameters(Vector& Position, Vector& Velocity, float MinSoundDist);
	/// fVolume should be from 0 to 1
	void SetVolume(float fVolume=1.0f);
	/// Frequency in Hz (Pitch). Set to '0' to reset to default
	void SetFrequency(LONG frequencyHz);
 
	/// Playing
	void Play(SoundEmitter& Emitter, SoundFlags flags = NONE, float volume = 0.75f);
	void Play(Vector& position, Vector& velocity, float Range, SoundFlags flags = NONE, float volume = 0.75f);
	void Play2D(DWORD flags = 0, float volume = 0.35f);
	bool IsPlaying();
	void Stop();
	/// Release resources
	void Free(); 

	/// Fading
	float FadeVolume;
	float FadePerSec;
	float CurVolume;
	void FadeTo(float fVolume, float fFadePerSec = 0.3f);
};



//=========== (C) Copyright 2004, Artificial Studios.All rights reserved. ===========
/// Name:    Sound Capturing & Streaming Classes
/// Author : Mostafa Mohamed
//===============================================================================

//TypeDefs for DSOUND 
typedef struct IDirectSoundCapture			*LPDIRECTSOUNDCAPTURE;
typedef struct IDirectSoundCaptureBuffer	*LPDIRECTSOUNDCAPTUREBUFFER;
typedef struct IDirectSoundNotify			*LPDIRECTSOUNDNOTIFY;
#include <mmreg.h>


//void* : User Info
//int   : Buffer Length
//void* : Buffer Data
typedef void (*SoundCapture_Callback)(void*,int,void*);

//---------------------------------------------------------------------------
/// Sound Capturing Class for network voice chat
//---------------------------------------------------------------------------
class ENGINE_API SoundCapture{
	//VARIABLES
public:
	LPDIRECTSOUNDCAPTURE       m_DSCapture;
	LPDIRECTSOUNDCAPTUREBUFFER m_DSCaptureBuffer;
	LPDIRECTSOUNDNOTIFY        m_DSNotify;
protected:
	WAVEFORMATEX * m_WaveFormat;

	HANDLE m_NotifyThread;
	DWORD  m_NotifyThreadID;

	SoundCapture_Callback m_callBack; 

	short m_NotifySize;
	short m_BufferSize;

	int m_nextCaptureOffset;

	void * m_UserInfo;
public:
	HANDLE m_NotificationEvent;

	/// FUNCTIONS
public:
	/// Default Constructor
	SoundCapture()
	{
		m_WaveFormat= NULL;
		m_NotifyThreadID = 0;
		m_NotifyThread = NULL;
		m_DSCapture = NULL;
		m_DSCaptureBuffer = NULL;
		m_DSNotify = NULL;
		m_UserInfo = NULL;
	}

	void Initialize(short samplesPerSecond,short bitsPerSample,short blockSize,char channels,SoundCapture_Callback callBack,void* userInfo);
	void ShutDown();

	static DWORD WINAPI NotificationProc( LPVOID lpParameter );
	void Update();
	void Start();
	void Stop();
};

//===========================================================================
//void* : User Info
//int   : Buffer Length
//void* : Buffer Data
typedef void (*SoundStreaming_Callback)(void*,int,void*);

//---------------------------------------------------------------------------
/// Sound Playback Class for network voice chat
//---------------------------------------------------------------------------
class ENGINE_API SoundStreaming{
	//VARIABLES
public:
	LPDIRECTSOUNDBUFFER m_DSBuffer;
	LPDIRECTSOUNDNOTIFY m_DSNotify;
protected:
	WAVEFORMATEX * m_WaveFormat;

	HANDLE m_NotifyThread;
	DWORD  m_NotifyThreadID;

	SoundStreaming_Callback m_callBack; 

	short m_NotifySize;
	short m_BufferSize;

	int m_nextPlayOffset;

	void * m_UserInfo;
public:
	HANDLE m_NotificationEvent;

	//FUNCTIONS
public:
	/// Default Constructor
	SoundStreaming()
	{
		m_WaveFormat= NULL;
		m_NotifyThreadID = 0;
		m_NotifyThread = NULL;
		m_DSBuffer = NULL;
		m_DSNotify = NULL;
		m_UserInfo = NULL;
	}

	void Initialize(short samplesPerSecond,short bitsPerSample,short blockSize,char channels,SoundStreaming_Callback callBack,void* userInfo);
	void ShutDown();

	static DWORD WINAPI NotificationProc( LPVOID lpParameter );
	void Update();
	void Start();
	void Stop();
};
#endif

