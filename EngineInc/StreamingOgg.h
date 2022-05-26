//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
/// Name: OggPlayer
/// Desc: Streams Ogg files! Uses a notify thread to write to a circular buffer
///
/// Author: Tim Johnson
//====================================================================================
#ifndef STREAMINGOGG_H 
#define STREAMINGOGG_H 

/// Streams Ogg files. Uses a notify thread to write to a circular buffer
class ENGINE_API OggPlayer
{
protected:
	friend class AudioDevice;
	bool RestoreBuffer();
	void    Update();
	void	FinalShutdown();
	void	Initialize(LPDIRECTSOUND pDS);
	static DWORD WINAPI NotificationProc( LPVOID lpParameter );

	/// for thread safety
	bool		bLocked; 
	long		BUFSIZE;
	float		fVolume;

	/// initialized?
    bool        bInitialized;        
	/// have we opened an ogg yet?
    bool        bFileOpened;     
	/// release ds by ourselves?
    bool        bReleaseDS;             

	/// the directsound 8 object
    LPDIRECTSOUND pDS;		
	/// the buffer
    LPDIRECTSOUNDBUFFER pDSB;	

	/// ID for streaming thread that'll call NotificationProc
	DWORD g_dwNotifyThreadID; 
	HANDLE g_hNotifyThread;
	HANDLE g_hNotificationEvent;

	/// for the vorbisfile interface   
    struct OggVorbis_File* vf;              

	/// which half of the buffer are/were
    int         nLastSection,    
		/// we playing?
                nCurSection;    
	/// only one half of the buffer to play
    bool        bAlmostDone;            
	/// done playing
    bool        bDone;                  
	/// loop?
    bool        bLoop;         
	/// Paused
	bool		bPaused;				

public:

    OggPlayer();
    ~OggPlayer();
	/// this opens an oggvorbis for playing 
    bool   OpenOgg( string file, long bufSize=65536);
	 /// and this one closes it :)
    void   Close();     
	/// play it again sam
    void  Play(bool loop = false);     
	/// Stop it
    void  Stop();     
	/// 1 to 0. Smart enough to stop using CPU cycles at 0 volume!!
	void  SetVolume(float volume); 
    
    inline bool IsPlaying(){return !bDone;}
};

extern ENGINE_API OggPlayer oggPlayer;

#endif
