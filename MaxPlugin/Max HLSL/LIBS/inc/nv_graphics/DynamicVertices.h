/*********************************************************************NVMH4****
Path:  NVSDK\Common\include\nv_renderdevice
File:  DynamicVertices.h

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


A version of Dynamic vertices originally written by Sim, and ported here to the
INVRenderDevice interfaces.  Allows API independant manipulation of dynamic vertex data.


******************************************************************************/

#ifndef _DYNAMICVERTICES_H_
#define _DYNAMICVERTICES_H_

#include <assert.h>

namespace nv_graphics
{

/////////////////////////////
// D. Sim Dietrich Jr.
// sim.dietrich@nvidia.com
// CMaughan - modified for the API independant vertex buffer
//////////////////////

template < class VertexType >
class NVDynamicVertices
{
	private :

		nv_renderdevice::INVRenderVertices* mpVertices;

		unsigned int mVertexCount;
		unsigned int mPosition;

		bool		 mbLocked;
		bool		 mbFlush;

		enum LOCK_FLAGS
		{
			LOCKFLAGS_FLUSH  = NVVERTEXFLAG_DISCARD,
			LOCKFLAGS_APPEND = NVVERTEXFLAG_NOOVERWRITE
		};

	public :

        enum USAGE_FLAGS
        {
            HW = NVCREATEFLAG_WRITEONLY | NVCREATEFLAG_DYNAMIC,
            SW = NVCREATEFLAG_WRITEONLY | NVCREATEFLAG_DYNAMIC | NVCREATEFLAG_SOFTWAREPROCESSING
        };

		NVDynamicVertices( nv_renderdevice::INVRenderDevice* pDevice, const unsigned long& theFVF, const unsigned int& theVertexCount, const USAGE_FLAGS& Usage = HW )
		{
			mpVertices = 0;
			mPosition = 0;

			mbFlush = true;

			mbLocked = false;

			mVertexCount = theVertexCount;

			bool bSuccess = pDevice->CreateRenderVertices(theVertexCount * sizeof(VertexType), Usage, &mpVertices);

			assert( ( bSuccess ) && ( mpVertices ) );
		}

		nv_renderdevice::INVRenderVertices* GetInterface() const { return mpVertices; }

		// Use at beginning of frame to force a flush of VB contents on first draw
		void FlushAtFrameStart() { mbFlush = true; }

		VertexType* Lock( const unsigned int& theLockCount, unsigned int& theStartVertex )
		{
			theStartVertex = 0;
			VertexType* pLockedData = 0;

			// Ensure there is enough space in the VB for this data
			if ( theLockCount > mVertexCount ) 
			{

#ifdef _DEBUG
				OutputDebugString("Warning: Couldn't get enough room in dynamic VB\n");
#endif
				return 0;
			}

			if ( mpVertices )
			{
				DWORD dwFlags = LOCKFLAGS_APPEND;

				// If either user forced us to flush,
				//  or there is not enough space for the vertex data,
				//  then flush the buffer contents
				//
				if ( mbFlush || ( ( theLockCount + mPosition ) > mVertexCount ) )
				{
					mbFlush = false;
					mPosition = 0;
					dwFlags = LOCKFLAGS_FLUSH;
				}

				DWORD dwSize = 0;
				bool bSuccess = mpVertices->Lock( mPosition * sizeof( VertexType ), 
										 theLockCount * sizeof( VertexType ), 
										 reinterpret_cast< BYTE** >( &pLockedData ), 
										 dwFlags );

				assert( bSuccess == true );
				if ( bSuccess == true)
				{
					assert( pLockedData != 0 );
					mbLocked = true;
					theStartVertex = mPosition;
					mPosition += theLockCount;
				}
			}

			return pLockedData;
		}

		void Unlock()
		{
			if ( ( mbLocked ) && ( mpVertices ) )
			{
				bool bSuccess = mpVertices->Unlock();				
				assert( bSuccess == true );
				mbLocked = false;
			}
		}

		~NVDynamicVertices()
		{
			Unlock();
			SAFE_RELEASE(mpVertices);
		}
	
};

}; // nv_graphics

#endif  //_DYNAMICVERTICES_H_
