#include "stdafx.h"
#include "CTimer.h"

#include <mmsystem.h>
//--------------------------------------------------------
// Update the frame and world delta
//--------------------------------------------------------
void CTimer::Update(){
	/*static double OldDeltaTime = 0;
	LARGE_INTEGER PerformanceCount;
	
	QueryPerformanceCounter (&PerformanceCount);
	unsigned int temp = ((unsigned int)PerformanceCount.LowPart >> lowshift) |
		((unsigned int)PerformanceCount.HighPart << (32 - lowshift));
	
	unsigned int t2 = temp - oldtime;
	double time = (double)t2 * pfreq;
	oldtime = temp;
	
	DeltaTime = time;*/
	static double old = timeGetTime();
	double t = timeGetTime();
	DeltaTime =  (t - old)/1000.;
	old = t;
	LogPrintf("CTimer: %f",DeltaTime);
	
	// 2 FPS - was probably a hiccup
	//if(DeltaTime > 0.5f)
	//	DeltaTime = OldDeltaTime;
	Seconds += DeltaTime;

//	OldDeltaTime = DeltaTime;
}



// Mini-timer, for timing functions etc
void CTimer::StartMiniTimer(){
	LARGE_INTEGER PerformanceCount;
	
	QueryPerformanceCounter (&PerformanceCount);
	unsigned int temp = ((unsigned int)PerformanceCount.LowPart >> lowshift) |
		((unsigned int)PerformanceCount.HighPart << (32 - lowshift));
	
	unsigned int t2 = temp - oldtime;
	miniTime = (double)t2 * pfreq;
}

double CTimer::StopMiniTimer(){
	LARGE_INTEGER PerformanceCount;
	
	QueryPerformanceCounter (&PerformanceCount);
	unsigned int temp = ((unsigned int)PerformanceCount.LowPart >> lowshift) |
		((unsigned int)PerformanceCount.HighPart << (32 - lowshift));
	
	unsigned int t2 = temp - oldtime;
	double time = (double)t2 * pfreq;
	return (time - miniTime);
}


//--------------------------------------------------------
// Start up the game timers
//--------------------------------------------------------
void CTimer::Start(){
	Seconds = 0;
	DeltaTime = 0;
	LARGE_INTEGER PerformanceFreq;
	if (!QueryPerformanceFrequency (&PerformanceFreq))
		Error ("ERROR: No hardware timer available!");
	
	unsigned int lowpart,highpart;
	// get 32 out of the 64 time bits such that we have around
	// 1 microsecond resolution
	lowpart = (unsigned int)PerformanceFreq.LowPart;
	highpart = (unsigned int)PerformanceFreq.HighPart;
	lowshift = 0;
	
	while (highpart || (lowpart > 2000000.0))
	{
		lowshift++;
		lowpart >>= 1;
		lowpart |= (highpart & 1) << 31;
		highpart >>= 1;
	}
	pfreq = 1.0 / (double)lowpart;
}
