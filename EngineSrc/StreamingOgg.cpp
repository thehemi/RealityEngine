//=========== (C) Copyright 2003, Tim Johnson. All rights reserved. ===========
// Name: OggPlayer
// Desc: Streams Ogg files! Uses a notify thread to write to a circular buffer
//=============================================================================
#include <stdafx.h>
#include "streamingogg.h"
#include <mmsystem.h>
#include <dsound.h>
#include "vorbis\vorbisfile.h"
#include <stdio.h>
#include <dxerr9.h>

OggPlayer oggPlayer;

OggPlayer::OggPlayer()
{
	bLocked			= false;
	bFileOpened     = false;
	bInitialized    = false;
	bReleaseDS      = false;
	pDS             = NULL;
	pDSB            = NULL;
	bLoop           = false;
	bDone           = true;
	bAlmostDone     = false;
	vf = new OggVorbis_File;
	fVolume = 1;
}

OggPlayer::~OggPlayer()
{
	if (bFileOpened)
		Close();

	if(vf)
		delete vf;
}

void OggPlayer::SetVolume(float volume){
	fVolume = volume;
	if(fVolume < 0.001f){
		bPaused = true;
		if(pDSB)
			pDSB->Stop();
		return;
	}

	if(pDSB){
		LONG lVolume = (LONG)(100.0 * 20.0 * log10(fVolume));
		if (lVolume < DSBVOLUME_MIN) 
			lVolume = DSBVOLUME_MIN;
		else if (lVolume > DSBVOLUME_MAX) 
			lVolume = DSBVOLUME_MAX;
		pDSB->SetVolume(lVolume);
	}

	if(bPaused){
		bPaused = false;
		if(pDSB)
			DXASSERT(pDSB->Play(0,0,DSBPLAY_LOOPING));
	}
}


// This thread loops forever until the sound is shut down
DWORD WINAPI OggPlayer::NotificationProc( LPVOID lpParameter )
{
	OggPlayer *player = (OggPlayer*)lpParameter;
	MSG     msg;
	DWORD   dwResult;
	BOOL    bDone = FALSE;

	while( !bDone ) 
	{ 
		dwResult = MsgWaitForMultipleObjects( 1, &player->g_hNotificationEvent, FALSE, INFINITE, QS_ALLEVENTS );

		//	if(!oggPlayer.IsPlaying()) continue; // Musn't handle any events if not ready
		player->bLocked = true;
		switch( dwResult )
		{
		case WAIT_OBJECT_0 + 0:
			player->Update();
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

		player->bLocked = false;
	}

	return 0;
}



bool OggPlayer::OpenOgg( string file, long bufSize )
{
	while(bLocked); // WAIT FOR ANY EXISTING THREAD TO FINISH PROCESSING

	if (!bInitialized)
		return false;

	BUFSIZE = bufSize;

	if (bFileOpened)
		Close();

	FILE    *f;
	FindMedia(file,"Music");
	const CHAR* filename = file.c_str();
	f = fopen(filename, _T("rb"));
	if (!f){ 
		LogPrintf(_T("Couldn't find ogg file '%s'"),filename);
		return false;
	}

	LogPrintf(_T("Opening OGG file '%s'"),filename);

	int ret = ov_open(f, vf, NULL, 0);	
	if(ret < 0)
		Error(_T("Couldn't read Ogg from file : %s"),file.c_str());

	// ok now the tricky part

	// the vorbis_info struct keeps the most of the interesting format info
	vorbis_info *vi = ov_info(vf,-1);

	// set the wave format
	WAVEFORMATEX        wfx;

	memset(&wfx, 0, sizeof(wfx));

	// Fill wave data
	if(vi->channels * 8 != 16)
		Error(_T("OGG file '%s' does not appear to be stereo 16-bit"),filename);

	wfx.wFormatTag      = WAVE_FORMAT_PCM; 
	wfx.nChannels       = vi->channels; 
	wfx.nSamplesPerSec  = vi->rate; 
	wfx.wBitsPerSample  = 16; 
	wfx.nBlockAlign     = (wfx.wBitsPerSample / 8) * wfx.nChannels;
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;


	// set up the buffer
	DSBUFFERDESC desc;

	desc.guid3DAlgorithm = GUID_NULL;
	desc.dwSize         = sizeof(desc);
	desc.dwFlags        = /*DSBCAPS_LOCDEFER | */DSBCAPS_GLOBALFOCUS  | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLVOLUME ;
	desc.lpwfxFormat    = &wfx;
	desc.dwReserved     = 0;

	desc.dwBufferBytes  = BUFSIZE*2;

	DXASSERT(pDS->CreateSoundBuffer(&desc, &pDSB, NULL ));

	// fill the buffer

	DWORD   pos = 0;
	int     sec = 0;
	ret = 1;
	DWORD   size = BUFSIZE*2;

	char    *buf;

	DXASSERT(pDSB->Lock(0, size, (LPVOID*)&buf, &size, NULL, NULL,DSBLOCK_ENTIREBUFFER));

	// now read in the bits
	while(ret && pos<size)
	{
		ret = ov_read(vf, buf+pos, size-pos, 0, 2, 1, &sec);
		pos += ret;
	}

	pDSB->Unlock( buf, size, NULL, NULL );

	//---------------------------------------
	// Create the notification events, so that we know when to fill
	// the buffer as the sound plays. 
	//---------------------------------------
	DWORD dwNotifyCount = 2;

	DSBPOSITIONNOTIFY*  aPosNotify     = NULL; 
	LPDIRECTSOUNDNOTIFY pDSNotify      = NULL;

	DXASSERT(pDSB->QueryInterface( IID_IDirectSoundNotify, (VOID**)&pDSNotify ) );

	aPosNotify = new DSBPOSITIONNOTIFY[ dwNotifyCount ];

	for( DWORD i = 0; i < dwNotifyCount; i++ )
	{
		aPosNotify[i].dwOffset     = (i==0?0:BUFSIZE+1000) ;
		aPosNotify[i].hEventNotify = g_hNotificationEvent;             
	}

	// Tell DirectSound when to notify us. The notification will come in the form 
	// of signaled events that are handled in WinMain()
	DXASSERT(pDSNotify->SetNotificationPositions( dwNotifyCount, aPosNotify ));

	SAFE_RELEASE( pDSNotify );
	SAFE_DELETE_ARRAY( aPosNotify );

	return bFileOpened = true;
}

void OggPlayer::Close()
{
	bFileOpened = false;

	nCurSection         =0;
	nLastSection        = 0;

	SAFE_RELEASE(pDSB);
}


void OggPlayer::Play(bool loop)
{
	if (!bFileOpened)
		return;

	// play looping because we will fill the buffer
	DXASSERT(pDSB->Play(0,0,DSBPLAY_LOOPING));    

	SetVolume(fVolume);

	bLoop = loop;
	bDone = false;
	bAlmostDone = false;
}

void OggPlayer::Stop()
{
	if (!bInitialized)
		return;

	if (!bFileOpened)
		return;

	if(pDSB)
		pDSB->Stop();
	bDone = true;
}

void OggPlayer::Update()
{
	if(!bFileOpened || !pDSB || bPaused)
		return;

	RestoreBuffer();

	DWORD pos;
	DXASSERT(pDSB->GetCurrentPosition(&pos, NULL));

	nCurSection = pos<BUFSIZE?0:1;

	// section changed?
	if (nCurSection != nLastSection)
	{
		if (bDone && !bLoop)
			Stop();

		// gotta use this trick 'cause otherwise there wont be played all bits
		if (bAlmostDone && !bLoop)
			bDone = true;

		DWORD   size = BUFSIZE;
		char    *buf;

		// fill the section we just left (i.e. the next section, since it's circular)
		DXASSERT(pDSB->Lock( nLastSection*BUFSIZE, size, (LPVOID*)&buf, &size,NULL, NULL, 0 ));

		DWORD   pos = 0;
		int     sec = 0;
		int     ret = 1;

		while(ret && pos<size)
		{
			ret = ov_read(vf, buf+pos, size-pos, 0, 2, 1, &sec);
			pos += ret;
		}

		// reached the and?
		if (!ret && bLoop)
		{
			// we are looping so restart from the beginning
			// NOTE: sound with sizes smaller than BUFSIZE may be cut off

			ret = 1;
			ov_pcm_seek(vf, 0);
			while(ret && pos<size)
			{
				ret = ov_read(vf, buf+pos, size-pos, 0, 2, 1, &sec);
				pos += ret;
			}
		}
		else if (!ret && !(bLoop))
		{
			// not looping so fill the rest with 0
			while(pos<size)
				*(buf+pos)=0; pos ++;

			// and say that after the current section no other sectin follows
			bAlmostDone = true;
		}

		pDSB->Unlock( buf, size, NULL, NULL );

		nLastSection = nCurSection;
	}
}


bool OggPlayer::RestoreBuffer( )
{
	HRESULT hr;

	if( pDSB == NULL )
		Error(_T("pDSB == NULL"));

	DWORD dwStatus;
	DXASSERT(pDSB->GetStatus( &dwStatus ) );

	if( dwStatus & DSBSTATUS_BUFFERLOST )
	{
		// Since the app could have just been activated, then
		// DirectSound may not be giving us control yet, so 
		// the restoring the buffer may fail.  
		// If it does, sleep until DirectSound gives us control.
		do 
		{
			hr = pDSB->Restore();
			if( hr == DSERR_BUFFERLOST )
				Sleep( 10 );
		}
		while( ( hr = pDSB->Restore() ) == DSERR_BUFFERLOST );

		return true;
	}

	return false;
}


//-----------------------------------------------------------------------------
// Name: StreamingOgg::Shutdown()
// Desc: Releases resources shared between all songs
//-----------------------------------------------------------------------------
void OggPlayer::FinalShutdown(){
	// Close any open file
	Close();

	// Thread
	if(g_hNotificationEvent != NULL){
		PostThreadMessage( g_dwNotifyThreadID, WM_QUIT, 0, 0 );
		WaitForSingleObject( g_hNotifyThread, INFINITE );
		CloseHandle( g_hNotifyThread );
		CloseHandle( g_hNotificationEvent );
	}
}

void OggPlayer::Initialize( LPDIRECTSOUND _pDS )
{
	if(bInitialized)
		return;

	g_hNotificationEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
	g_hNotifyThread = CreateThread( NULL, 0, NotificationProc, this, 0, &g_dwNotifyThreadID );
	pDS = _pDS; 
	bInitialized = true;
}

