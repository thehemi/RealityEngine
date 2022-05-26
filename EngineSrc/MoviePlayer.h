//------------------------------------------------------------------------------
/// File: PlayWnd.h
//
/// Desc: DirectShow sample code - header file for video in window movie
///      player application.
//
/// Copyright (c) 1998-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------
#ifndef  MOVIEPLAYER_H
#define MOVIEPLAYER_H

//
/// Function prototypes
//
enum PLAYSTATE {Stopped, Paused, Running, Closed};
extern PLAYSTATE g_psCurrent;

LRESULT CALLBACK VideoPlayerProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

/// Main
void OpenClip(char* fileName);
void Initialize(HWND app);
void Shutdown();


HRESULT InitPlayerWindow(void);
HRESULT InitVideoWindow(int nMultiplier, int nDivider);
HRESULT HandleGraphEvent(void);
HRESULT StepOneFrame(void);
HRESULT StepFrames(int nFramesToStep);
HRESULT ModifyRate(double dRateAdjust);
HRESULT SetRate(double dRate);

BOOL GetFrameStepInterface(void);
BOOL GetClipFileName(LPTSTR szName);

void PaintAudioWindow(void);
void MoveVideoWindow(void);
void CheckVisibility(void);
void CloseInterfaces(void);


void PauseClip(void);
void StopClip(void);
void CloseClip(void);


//
/// Constants
//
#define VOLUME_FULL     0L
#define VOLUME_SILENCE  -10000L

/// Begin default media search at root directory
#define DEFAULT_MEDIA_PATH  TEXT("\\\0")

/// Defaults used with audio-only files
#define DEFAULT_AUDIO_WIDTH     240
#define DEFAULT_AUDIO_HEIGHT    120
#define DEFAULT_VIDEO_WIDTH     320
#define DEFAULT_VIDEO_HEIGHT    240
#define MINIMUM_VIDEO_WIDTH     200
#define MINIMUM_VIDEO_HEIGHT    120

#define APPLICATIONNAME TEXT("PlayWnd Media Player")
#define CLASSNAME       TEXT("PlayWndMediaPlayer")

#define WM_GRAPHNOTIFY  WM_USER+13


//
/// Macros
//
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x) { if (x) x->Release(); x = NULL; }
#endif
#define JIF(x) if (FAILED(hr=(x))) \
    {Msg(TEXT("FAILED(hr=0x%x) in ") TEXT(#x) TEXT("\n"), hr); return hr;}

#define LIF(x) if (FAILED(hr=(x))) \
    {Msg(TEXT("FAILED(hr=0x%x) in ") TEXT(#x) TEXT("\n"), hr);}


#endif