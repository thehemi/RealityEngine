/*********************************************************************NVMH4****
Path:  SDK\LIBS\inc\shared
File:  NV_Timer.h

Copyright NVIDIA Corporation 2002
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED
*AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS
BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES
WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS,
BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS)
ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS
BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.



Comments:
10/23/2002 - Greg J. Adapted from DXUtil_Timer() function to be API agnostic.

Uses either QuerryPerformanceCounter() (good) or if that is not supported, 
it uses timeGetTime() (not good for short times).

Returned times are always in seconds.


Example Usage:

NV_Timer timer;
float secs;		// time in seconds
timer.Start( true ); 		// reset = true to start from zero
secs = timer.GetTime();		// timer still runs
secs = timer.Stop();		// returns time since last reset
secs = timer.Start(false)	// time will be added to previous time
...

******************************************************************************/


#ifndef __NVTIMER_H_GJ_
#define __NVTIMER_H_GJ_

#include <shared/NV_Common.h>
#include <shared/NV_Error.h>


#include <mmsystem.h>
#pragma comment( lib , "winmm.lib" )



class NV_Timer
{
protected:
	int		m_nUsingQPF;
	bool	m_bTimerStopped;

	LONGLONG m_llQPFTicksPerSec;

	LONGLONG m_llStartTime;
	LONGLONG m_llStopTime;
//@    LONGLONG m_llLastElapsedTime	= 0;
    LONGLONG m_llPreviousTime;

	double m_fStartTime;
    double m_fStopTime;
    double m_fPreviousTime;

	float	m_fLastTime;	// last returned time

public:

//////////////////////////////////////////////////////////////////
// Interface:
//	float	Start( bool reset );	// true to start from 0 time
//	float	Stop();					// timer stops, time is preserved
//	float	GetTime();				// timer keeps running
//	float	GetAbsoluteTime();		// get absolute system time
//
//	bool	UsingQPF();				// true if using it
//
//	void	GetTextClock( char * buf, int buf_sz );
//	void	GetTextDate( char * buf, int buf_sz );

	

public:

	NV_Timer()
	{
		// Use QueryPerformanceFrequency() to get frequency of timer.
		// If QPF is not supported, use timeGetTime() which itself returns
		//   milliseconds that we convert to float seconds

		LARGE_INTEGER qwTicksPerSec;
		m_nUsingQPF = QueryPerformanceFrequency( &qwTicksPerSec );

		if( m_nUsingQPF != 0 )
		{
			m_llQPFTicksPerSec = qwTicksPerSec.QuadPart;
		}

		m_bTimerStopped		= true;
		m_llStartTime		= 0;
		m_llStopTime        = 0;
		m_llPreviousTime	= 0;

		m_fStartTime		= 0.0f;
		m_fStopTime			= 0.0f;
		m_fPreviousTime		= 0.0f;

		m_fLastTime			= 0.0f;
	}


	float	Start( bool reset )
	{
		if( m_nUsingQPF != 0 )
		{
			LARGE_INTEGER qwTime;

			if( reset || m_bTimerStopped )
			{
	            QueryPerformanceCounter( &qwTime );
				m_llStartTime = qwTime.QuadPart;

				if( reset )
					m_llPreviousTime = 0;
			}
			else
			{
				m_fLastTime = GetTime();
			}
		}
		else		// using timeGetTime()
		{

			if( reset || m_bTimerStopped )
			{
				m_fStartTime = timeGetTime() * 0.001;

				if( reset )
					m_fPreviousTime = 0.0f;
			}
			else
			{
				m_fLastTime = GetTime();
			}
		}

		m_bTimerStopped = false;
		return( m_fLastTime );
	}

	float	Stop()
	{
		// timer stops, time is preserved

		if( m_bTimerStopped == true )
		{
			// return previous time
		}
		else
		{
			if( m_nUsingQPF != 0 )
			{

				// Get time and add to running total
				LARGE_INTEGER qwTime;
				QueryPerformanceCounter( &qwTime );

				m_llPreviousTime = m_llPreviousTime + ( qwTime.QuadPart - m_llStartTime );

				m_fLastTime = (float)((double) m_llPreviousTime / (double) m_llQPFTicksPerSec);
			}
			else
			{
				assert( false );
				return( -2.0f );
			}
		}

		m_bTimerStopped = true;
		return( m_fLastTime );
	}

	float	GetTime()
	{
		if( m_nUsingQPF != 0 )
		{
			if( m_bTimerStopped == false )
			{
				LARGE_INTEGER qwTime;
				QueryPerformanceCounter( &qwTime );

				m_fLastTime = (float)((double) ( qwTime.QuadPart - m_llStartTime + m_llPreviousTime ) / (double) m_llQPFTicksPerSec);
			}

			return( m_fLastTime );
		}
		else
		{
			//@@@
			assert( false );
			return( -2.0f );
		}
	}

	
	float	GetAbsoluteTime()
	{
		assert( false );
		return( -3.0f );		
	}


	bool	UsingQPF()	{ return( m_nUsingQPF != 0 ); }				// true if using it

	void	GetTextClock( char * buf, int buf_sz )
	{
		assert( false );
		
	}

	void	GetTextDate( char * buf, int buf_sz )
	{
		assert( false );		
		
	}

};


#endif 

