#ifndef _DYNAMICVB_H_
#define _DYNAMICVB_H_

#include "d3dcustom.h" 

/// /// /// /// /// /// /// /// /// //
/// D. Sim Dietrich Jr.
/// sim.dietrich@nvidia.com
/// /// /// /// /// /// /// /

class DynamicVB 
{
	private :

		LPDIRECT3DVERTEXBUFFER9 mpVB;

		unsigned int mVertexCount;
		unsigned int mPosition;

		bool		 mbLocked;
		bool		 mbFlush;

		unsigned int VertexTypeSize;

		D3DVERTEXBUFFER_DESC mDesc;

		enum LOCK_FLAGS
		{
			LOCKFLAGS_FLUSH  = D3DLOCK_NOSYSLOCK | D3DLOCK_DISCARD,
			LOCKFLAGS_APPEND = D3DLOCK_NOSYSLOCK | D3DLOCK_NOOVERWRITE
		};

	public :

		DynamicVB( const LPDIRECT3DDEVICE9 pD3D, const DWORD& theFVF, const unsigned int& theVertexCount, const unsigned int theVertexTypeSize )
		{
			mpVB = 0;
			mPosition = 0;
			VertexTypeSize = theVertexTypeSize;

			mbFlush = true;

			mbLocked = false;

			mVertexCount = theVertexCount;

		
			memset( &mDesc, 0x00, sizeof( mDesc ) );
			mDesc.Format = D3DFMT_VERTEXDATA;
			mDesc.Size = theVertexCount * VertexTypeSize;
			mDesc.Type = D3DRTYPE_VERTEXBUFFER;
			mDesc.Pool = D3DPOOL_DEFAULT;
			mDesc.FVF = theFVF;

			mDesc.Usage = D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC;

			HRESULT hr = pD3D->CreateVertexBuffer( mVertexCount * VertexTypeSize,
				                                   mDesc.Usage,
												   mDesc.FVF,
												   mDesc.Pool,
												   &mpVB,0);
			if(hr != D3D_OK)
				Error("CreateVB failed with: '%s'. Vertices: %d",DXGetErrorString9(hr),mVertexCount);
			assert( ( mpVB ) );
		}

		LPDIRECT3DVERTEXBUFFER9 GetInterface() const { return mpVB; }

		/// Use at beginning of frame to force a flush of VB contents on first draw
		void FlushAtFrameStart() { mbFlush = true; }


		void* Lock( const unsigned int& theLockCount, unsigned int& theStartVertex )
		{
			theStartVertex = 0;
			void* pLockedData = 0;

			/// Ensure there is enough space in the VB for this data
			if ( theLockCount > mVertexCount ) { assert( false ); return 0; }

			if ( mpVB )
			{
				DWORD dwFlags = LOCKFLAGS_APPEND;

				/// If either user forced us to flush,
				/// or there is not enough space for the vertex data,
				/// then flush the buffer contents
				//
				if ( mbFlush || ( ( theLockCount + mPosition ) > mVertexCount ) )
				{
					mbFlush = false;
					mPosition = 0;
					dwFlags = LOCKFLAGS_FLUSH;
				}

				DWORD dwSize = 0;
				HRESULT hr = mpVB->Lock( mPosition * VertexTypeSize, 
										 theLockCount * VertexTypeSize, 
										 (void**)( &pLockedData ), 
										 dwFlags );

				assert( hr == D3D_OK );
				if ( hr == D3D_OK )
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
			if ( ( mbLocked ) && ( mpVB ) )
			{
				HRESULT hr = mpVB->Unlock();				
				assert( hr == D3D_OK );
				mbLocked = false;
			}
		}

		~DynamicVB()
		{
			Unlock();
			if ( mpVB )
			{
				mpVB->Release();
			}
		}
	
};

#endif  _DYNAMICVB_H_
