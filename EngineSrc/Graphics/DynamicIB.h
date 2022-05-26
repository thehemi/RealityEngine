#ifndef _DYNAMICIB_H_
#define _DYNAMICIB_H_

#include "d3dcustom.h" 

/// /// /// /// /// /// /// /// /// //
/// D. Sim Dietrich Jr.
/// sim.dietrich@nvidia.com
/// /// /// /// /// /// /// /


class DynamicIB 
{
	private :

		LPDIRECT3DINDEXBUFFER9 mpIB;

		unsigned int mIndexCount;
		unsigned int mPosition;

		unsigned int IndexTypeSize;

		bool		 mbLocked;
		bool		 mbFlush;

		D3DINDEXBUFFER_DESC mDesc;

		enum LOCK_FLAGS
		{
			LOCKFLAGS_FLUSH  = D3DLOCK_NOSYSLOCK | D3DLOCK_DISCARD,
			LOCKFLAGS_APPEND = D3DLOCK_NOSYSLOCK | D3DLOCK_NOOVERWRITE
		};

	public :

		DynamicIB( const LPDIRECT3DDEVICE9 pD3D, const unsigned int& theIndexCount, const unsigned int theIndexTypeSize )
		{
			mpIB = 0;
			mPosition = 0;
			IndexTypeSize = theIndexTypeSize;

			mbFlush = true;

			mbLocked = false;

			mIndexCount = theIndexCount;
		
			memset( &mDesc, 0x00, sizeof( mDesc ) );
			if ( IndexTypeSize == 2 )
			{
				mDesc.Format = D3DFMT_INDEX16;
			}
			else
			{
				assert( IndexTypeSize == 4 );
				mDesc.Format = D3DFMT_INDEX32;
			}

			mDesc.Size = IndexTypeSize * theIndexCount;
			mDesc.Type = D3DRTYPE_INDEXBUFFER;
			mDesc.Pool = D3DPOOL_DEFAULT;

			mDesc.Usage = D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC;

			HRESULT hr = pD3D->CreateIndexBuffer( mIndexCount * IndexTypeSize,
				                                   mDesc.Usage,
												   mDesc.Format,
												   mDesc.Pool,
												   &mpIB,0);

			if ( mpIB )
			{
				D3DINDEXBUFFER_DESC aDesc;
				mpIB->GetDesc( &aDesc );
				assert( memcmp( &aDesc, &mDesc, sizeof( mDesc ) ) == 0 );
			}

			if(hr != D3D_OK)
				Error("CreateIB failed with: '%s'. Indices: %d",DXGetErrorString9(hr),mIndexCount);
			assert( ( mpIB ) );
		}

		LPDIRECT3DINDEXBUFFER9 GetInterface() const { return mpIB; }

		/// Use at beginning of frame to force a flush of IB contents on first draw
		void FlushAtFrameStart() { mbFlush = true; }


		void* Lock( const unsigned int& theLockCount, unsigned int& theStartIndex )
		{
			theStartIndex = 0;
			void* pLockedData = 0;

			/// Ensure there is enough space in the IB for this data
			if ( theLockCount > mIndexCount ) { assert( false ); return 0; }

			if ( mpIB )
			{
				DWORD dwFlags = LOCKFLAGS_APPEND;

				/// If either user forced us to flush,
				/// or there is not enough space for the Index data,
				/// then flush the buffer contents
				//
				if ( mbFlush || ( ( theLockCount + mPosition ) > mIndexCount ) )
				{
					mbFlush = false;
					mPosition = 0;
					dwFlags = LOCKFLAGS_FLUSH;
				}

				DWORD dwSize = 0;
				HRESULT hr = mpIB->Lock( mPosition * IndexTypeSize, 
										 theLockCount * IndexTypeSize, 
										 reinterpret_cast< void** >( &pLockedData ), 
										 dwFlags );

				assert( hr == D3D_OK );
				if ( hr == D3D_OK )
				{
					assert( pLockedData != 0 );
					mbLocked = true;
					theStartIndex = mPosition;
					mPosition += theLockCount;
				}
			}

			return pLockedData;
		}

		void Unlock()
		{
			if ( ( mbLocked ) && ( mpIB ) )
			{
				HRESULT hr = mpIB->Unlock();				
				assert( hr == D3D_OK );
				mbLocked = false;
			}
		}

		~DynamicIB()
		{
			Unlock();
			if ( mpIB )
			{
				mpIB->Release();
			}
		}
	
};

#endif  _DYNAMICIB_H_
