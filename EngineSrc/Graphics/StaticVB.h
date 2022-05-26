#ifndef _StaticVB_H_
#define _StaticVB_H_

#include "d3dcustom.h" 
/// /// /// /// /// /// /// /// /// //
/// D. Sim Dietrich Jr.
/// sim.dietrich@nvidia.com
/// /// /// /// /// /// /// /

class StaticVB 
{
	private :

		LPDIRECT3DVERTEXBUFFER9 mpVB;

		unsigned int mVertexCount;

		unsigned int mPosition;

		bool		 mbLocked;

		unsigned int VertexTypeSize;

		D3DVERTEXBUFFER_DESC mDesc;

	public :

		unsigned int GetVertexCount() const 
		{ 
			return mVertexCount; 
		}

		StaticVB( const LPDIRECT3DDEVICE9 pD3D, const DWORD& theFVF, const unsigned int& theVertexCount, const unsigned int theVertexTypeSize )
		{
			mpVB = 0;
			mPosition = 0;
			VertexTypeSize = theVertexTypeSize;
			mbLocked = false;

			mVertexCount = theVertexCount;
		
			memset( &mDesc, 0x00, sizeof( mDesc ) );
			mDesc.Format = D3DFMT_VERTEXDATA;
			mDesc.Size = theVertexCount * VertexTypeSize;
			mDesc.Type = D3DRTYPE_VERTEXBUFFER;
			mDesc.Pool = D3DPOOL_DEFAULT;
			mDesc.FVF = theFVF;

			mDesc.Usage = D3DUSAGE_WRITEONLY;

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

		void* Lock( const unsigned int& theLockCount, unsigned int& theStartVertex )
		{
			theStartVertex = 0;
			void* pLockedData = 0;

			/// Ensure there is enough space in the VB for this data
			if ( theLockCount > mVertexCount ) { assert( false ); return 0; }

			if ( mpVB )
			{
				DWORD dwFlags = D3DLOCK_NOSYSLOCK;

				DWORD dwSize = 0;

				HRESULT hr = mpVB->Lock( mPosition * VertexTypeSize, 
										 theLockCount * VertexTypeSize, 
										 reinterpret_cast< void** >( &pLockedData ), 
										 dwFlags );

				if(hr != D3D_OK)
					Error("mpVB->Lock failed with: %s",DXGetErrorString9(hr));
				else
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

		~StaticVB()
		{
			Unlock();
			if ( mpVB )
			{
				mpVB->Release();
			}
		}
	
};

#endif  _StaticVB_H_
