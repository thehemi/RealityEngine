/*********************************************************************NVMH4****
Path:  NVSDK\Common\include
File:  NV_Common.h

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

Usefull macros, etc.

******************************************************************************/


#ifndef	__NV_COMMON_H
#define	__NV_COMMON_H

#define INLINE __forceinline


#ifndef MULTI_THREAD
#include <windows.h>
#endif
#include <assert.h> 

//@@ nv_debug does not belong here
//@@ It is not useful to all apps which request NV_Common.h
//@@ Please add an extra include line to your own programs if you need nv_debug
//@@#include <nv_debug\idebugconsole.h>



#ifndef ASSERT_IF_FAILED
#define ASSERT_IF_FAILED( hres )	\
{									\
	if( FAILED(hres) )				\
	   assert( false );				\
}
#endif

#ifndef	ASSERT_AND_RET_IF_FAILED
#define ASSERT_AND_RET_IF_FAILED(hres)	\
{										\
	if( FAILED(hres) )					\
	{									\
		assert( false );				\
		return( hres );					\
	}									\
}
#endif


#ifndef ASSERT_AND_RET_IF_NULL
#define ASSERT_AND_RET_IF_NULL(pointer)		\
{											\
	if( pointer == NULL )					\
	{	assert( false );					\
		return;								\
	}										\
}
#endif


#ifndef RET_NULL_IF_NULL
#define RET_NULL_IF_NULL(pointer)			\
{											\
	if( pointer == NULL )					\
	{	assert( false );					\
		return(NULL);						\
	}										\
}
#endif


#ifndef MSG_AND_RET_IF_NULL
#define MSG_AND_RET_IF_NULL( pointer, msg )		\
{												\
	if( pointer == NULL )						\
	{	FDebug("%s\n", msg); assert(false);		\
		return;									\
	}											\
}
#endif


#ifndef RET_IF_NULL
#define RET_IF_NULL( ptr )							\
	{ if( ptr == NULL ) { assert(false); return; } }
#endif

#ifndef RET_NULL_IF_NULL
#define RET_NULL_IF_NULL( ptr )							\
	{ if( ptr == NULL ) { assert(false); return( NULL ); } }
#endif


#ifndef FAIL_IF_NULL
#define FAIL_IF_NULL( x )								\
	{ if( x==NULL ) { assert(false); return(E_FAIL); } }
#endif



	// FDebug defined in NV_Error.h
#ifndef ASSERT_MSG
	#define ASSERT_MSG( var, msg )	\
	{									\
		if( !(var) )					\
		{								\
			FDebug( msg );				\
		}								\
		assert(var);					\
	}
#endif



#ifndef SAFE_ARRAY_DELETE
#define SAFE_ARRAY_DELETE(p)  { if(p) { delete [] (p);  p=NULL; } }
#endif


// deletes all pointers in a vector of pointers, and clears the vector
#ifndef SAFE_VECTOR_DELETE
#define SAFE_VECTOR_DELETE( vec )							\
{	for( int svecdel = 0; svecdel < vec.size(); svecdel++ ) \
	{	if( vec.at( svecdel ) != NULL )						\
		{	delete( vec.at( svecdel ));						\
			vec.at( svecdel ) = NULL;						\
		}													\
	}														\
	vec.clear();											\
}
#endif


#ifndef SAFE_DELETE
#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }
#endif

#ifndef SAFE_ADDREF
#define SAFE_ADDREF(p) { if (p) { (p)->AddRef(); } }
#endif


#ifndef CHECK_BOUNDS
#define CHECK_BOUNDS( var, min, max )			\
	if( (var < min) || (var > max) )			\
	{	FDebug("Variable out of bounds!\n");	\
		assert( false ); return;				\
	}
#endif


#ifndef CHECK_BOUNDS_NULL
#define CHECK_BOUNDS_NULL( var, min, max )		\
	if( (var < min) || (var > max) )			\
	{	FDebug("Variable out of bounds!\n");	\
		assert( false ); return(NULL);			\
	}
#endif

///////////////////////////////////////////////////////////////


#define ifnot(x)  if (!(x))
#define until(x) while(!(x))
#define ever          (;;)
#define wait        do {}
#define nothing     {}


///////////////////////////////////////////////////////////////
// @@ extract more of these from ulCommon.h

typedef unsigned short		USHORT;
typedef unsigned short		ushort;


#define XX row[0].x
#define XY row[0].y
#define XZ row[0].z
#define XW row[0].w

#define YX row[1].x
#define YY row[1].y
#define YZ row[1].z
#define YW row[1].w

#define ZX row[2].x
#define ZY row[2].y
#define ZZ row[2].z
#define ZW row[2].w

#define WX row[3].x
#define WY row[3].y
#define WZ row[3].z
#define WW row[3].w


#endif
