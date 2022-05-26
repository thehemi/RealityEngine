/*********************************************************************NVMH4****
Path:  SDK\LIBS\inc\shared
File:  NV_Error.h

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

No longer requires separate .cpp file.  Functions defined in the .h with the
magic of the static keyword.



******************************************************************************/



#ifndef	__MYERROR__
#define	__MYERROR__


#include    <stdlib.h>          // for exit()
#include    <stdio.h>
#include    <windows.h>


#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000




void OkMsgBox( char * szCaption, char * szFormat, ... );

void FDebug( char * szFormat, ... );
void FMsg( char * szFormat, ... );

//////////////////////////////////////////////////////////////

#ifndef OUTPUT_POINTER
#define OUTPUT_POINTER(p) { FMsg("%35s = %8.8x\n", #p, p ); }
#endif


#ifndef NULLCHECK
#define NULLCHECK(q, msg,quit) {if(q==NULL) { DoError(msg, quit); }}
#endif

#ifndef IFNULLRET
#define IFNULLRET(q, msg)	   {if(q==NULL) { FDebug(msg); return;}}
#endif

#ifndef FAILRET
#define FAILRET(hres, msg) {if(FAILED(hres)){FDebug("*** %s   HRESULT: %d\n",msg, hres);return hres;}}
#endif

#ifndef HRESCHECK
#define HRESCHECK(q, msg)	 {if(FAILED(q)) { FDebug(msg); return;}}
#endif

#ifndef NULLASSERT
#define NULLASSERT( q, msg,quit )   {if(q==NULL) { FDebug(msg); assert(false); if(quit) exit(0); }}
#endif

/////////////////////////////////////////////////////////////////


static void OkMsgBox( char * szCaption, char * szFormat, ... )
{
	char buffer[256];

    va_list args;
    va_start( args, szFormat );
    _vsnprintf( buffer, 256, szFormat, args );
    va_end( args );

	buffer[256-1] = '\0';			// terminate in case of overflow
	MessageBox( NULL, buffer, szCaption, MB_OK );
}


#ifdef _DEBUG
	static void FDebug ( char * szFormat, ... )
	{	
		static char buffer[2048];

		va_list args;
		va_start( args, szFormat );
		_vsnprintf( buffer, 2048, szFormat, args );
		va_end( args );

		buffer[2048-1] = '\0';			// terminate in case of overflow
		OutputDebugString ( buffer );

		Sleep( 2 );		// OutputDebugString sometimes misses lines if
						//  called too rapidly in succession.
	}

	static void NullFunc( char * szFormat, ... ) {}

	#if 0
		#define WMDIAG(str) { OutputDebugString(str); }
	#else
		#define WMDIAG(str) {}
	#endif
#else
	static void FDebug( char * szFormat, ... )		{}
	static void NullFunc( char * szFormat, ... )	{}

	#define WMDIAG(str) {}
#endif


static void FMsg( char * szFormat, ... )
{	
	static char buffer[2048];

	va_list args;
	va_start( args, szFormat );
	_vsnprintf( buffer, 2048, szFormat, args );
	va_end( args );

	buffer[2048-1] = '\0';			// terminate in case of overflow
	OutputDebugString ( buffer );

	Sleep( 2 );		// OutputDebugString sometimes misses lines if
					//  called too rapidly in succession.
}


#endif
