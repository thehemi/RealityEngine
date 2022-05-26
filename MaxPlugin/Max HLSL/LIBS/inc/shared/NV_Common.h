/*********************************************************************NVMH4****
Path:  SDK\LIBS\inc\shared
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

Define these to output messages on delete and release:
#define NVMSG_SAFE_ARRAY_DELETE
#define NVMSG_SAFE_DELETE
#define NVMSG_SAFE_RELEASE



******************************************************************************/


#ifndef	__NV_COMMON_H
#define	__NV_COMMON_H

#define INLINE __forceinline


#ifndef MULTI_THREAD
#include <windows.h>
#endif

#include <assert.h> 
#include "NV_Error.h"


typedef unsigned short		USHORT;
typedef unsigned short		ushort;


//////////////////////////////////////////////////////////////////

#ifndef	ASSERT_MSG_RET
#define	ASSERT_MSG_RET( p, m, r )								\
{																\
	if( !(p) )	{ FMsg(m); assert( false ); return( r );	}	\
}
#endif


#ifndef MSG_RET_IF_FAILED
#define MSG_RET_IF_FAILED( h, m, r )								\
{																	\
	if( FAILED(h) )	{ FMsg(m); assert( false ); return( r );	}	\
}
#endif


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
#define ASSERT_AND_RET_IF_NULL(p)			\
{											\
	if( (p) == NULL )						\
	{	assert( false );					\
		return;								\
	}										\
}
#endif


#ifndef RET_NULL_IF_NULL
#define RET_NULL_IF_NULL(p)					\
{											\
	if( (p) == NULL )						\
	{										\
		return(NULL);						\
	}										\
}
#endif


#ifndef RET_IF_NULL
#define RET_IF_NULL( p )						\
	{ if( (p) == NULL ) { return; } }
#endif


#ifndef FAIL_IF_NULL
#define FAIL_IF_NULL( x )						\
	{ if( (x)==NULL ) { return(E_FAIL); } }
#endif



#ifndef MSG_AND_RET_IF_NULL
#define MSG_AND_RET_IF_NULL( p, m )				\
{												\
	if( (p) == NULL )							\
	{	FDebug("%s\n", m );						\
		return;									\
	}											\
}
#endif


#ifndef IFNULL_MSG_FAIL
#define IFNULL_MSG_FAIL( p, m )	{if((p)==NULL) { FDebug(m); return(E_FAIL);}}
#endif




	// FDebug defined in NV_Error.h
#ifndef ASSERT_MSG
	#define ASSERT_MSG( v, m )			\
	{									\
		if( !(v) )						\
		{								\
			FDebug( m );				\
		}								\
		assert(v);						\
	}
#endif



#ifndef SAFE_DELETE_ARRAY
#ifdef	NVMSG_SAFE_ARRAY_DELETE
	#define SAFE_DELETE_ARRAY(p)  { FMsg("SAFE_DELETE_ARRAY: %35s = 0x%8.8X\n", #p, p );	\
									if(p) { delete [] (p);  p=NULL; }				}
#else
	#define SAFE_DELETE_ARRAY(p)  { if(p) { delete [] (p);  p=NULL; } }
#endif
#endif


#ifndef SAFE_ARRAY_DELETE
#ifdef	NVMSG_SAFE_ARRAY_DELETE
	#define SAFE_ARRAY_DELETE(p)  { FMsg("SAFE_ARRAY_DELETE: %35s = 0x%8.8X\n", #p, p );	\
									if(p) { delete [] (p);  p=NULL; }				}
#else
	#define SAFE_ARRAY_DELETE(p)  { if(p) { delete [] (p);  p=NULL; } }
#endif
#endif


// deletes all pointers in a vector of pointers, and clears the vector
#ifndef SAFE_VECTOR_DELETE
#define SAFE_VECTOR_DELETE( v )							\
{	for( UINT svecdel = 0; svecdel < (v).size(); svecdel++ ) \
	{	if( (v).at( svecdel ) != NULL )						\
		{	delete( (v).at( svecdel ));						\
			(v).at( svecdel ) = NULL;						\
		}													\
	}														\
	(v).clear();											\
}
#endif


#ifndef SAFE_DELETE
#ifdef	NVMSG_SAFE_DELETE
//	#pragma message( "Outputting messages for SAFE_DELETE() macro\n" )
	#define SAFE_DELETE( p )	{	FMsg("SAFE_DELETE: %35s = 0x%8.8X\n", #p, p );		\
								if((p) != NULL) { delete(p); (p) = NULL; }	}
#else
	#define SAFE_DELETE( p )  { if((p) != NULL ) { delete(p); (p) = NULL; } }
#endif
#endif



// deletes all pointers in a vector of pointers, and clears the vector
#ifndef SAFE_VECTOR_RELEASE
#define SAFE_VECTOR_RELEASE( v )							\
{	for( UINT svecrel = 0; svecrel < (v).size(); svecrel++ ) \
	{	if( (v).at( svecrel ) != NULL )						\
		{	(v).at( svecrel )->Release();					\
			(v).at( svecrel ) = NULL;						\
		}													\
	}														\
	(v).clear();											\
}
#endif

#ifndef SAFE_RELEASE
#ifdef	NVMSG_SAFE_RELEASE
#define SAFE_RELEASE(p) {	FMsg("SAFE_RELEASE: %35s = 0x%8.8X\n", #p, p );		\
							if( (p) != NULL )	{ (p)->Release(); (p) = NULL; }	}
#else
	#define SAFE_RELEASE(p) { if( (p) != NULL ) { (p)->Release(); (p) = NULL; } }
#endif
#endif


#ifndef SAFE_ADDREF
#ifdef	NVMSG_SAFE_ADDREF
	#define SAFE_ADDREF(p)	{	FMsg("SAFE_ADDREF: %35s = 0x%8.8X\n", #p, p );	\
								if((p) != NULL ) { (p)->AddRef(); }					}
#else
	#define SAFE_ADDREF(p) { if((p) != NULL ) { (p)->AddRef(); } }
#endif
#endif




#ifndef CHECK_BOUNDS
#define CHECK_BOUNDS( v, n, x )					\
	if( (v < n) || (v > x) )					\
	{	FDebug("Variable out of bounds!\n");	\
		assert( false ); return;				\
	}
#endif


#ifndef CHECK_BOUNDS_NULL
#define CHECK_BOUNDS_NULL( v, n, x )			\
	if( (v < n) || (v > x) )					\
	{	FDebug("Variable out of bounds!\n");	\
		assert( false ); return(NULL);			\
	}
#endif

#ifndef CHECK_BOUNDS_HR
#define CHECK_BOUNDS_HR( v, n, x )				\
	if( (v < n) || (v > x) )					\
	{	FDebug("Variable out of bounds!\n");	\
		assert( false ); return(E_FAIL);		\
	}
#endif

///////////////////////////////////////////////////////////////


#define ifnot(x)  if (!(x))
#define until(x) while(!(x))
#define ever          (;;)
#define wait        do {}
#define nothing     {}


///////////////////////////////////////////////////////////////
//@@ still needed?  and by what?
/* //@@ removed 3/17/2003
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
*/


#endif
