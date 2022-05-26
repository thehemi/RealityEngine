/*********************************************************************NVMH4****
Path:  NVSDK\Common\include\nv_sys
File:  interpolator.h

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

******************************************************************************/
#include "invinterpolator.h"

namespace nv_sys
{

typedef enum eINTERPOLATOR
{
	INTERPOLATOR_LINEAR = 0,
	INTERPOLATOR_CONSTANT = 1,
	INTERPOLATOR_STEP = 2
} eINTERPOLATOR;


#define lerp(t, a, b) ( a + ((b - a) * t))

class NVStandardInterpolator : public INVInterpolator
{
public:
	NVStandardInterpolator(eINTERPOLATOR eInterpolatorType)
		: m_InterpolatorType(eInterpolatorType),
		m_dwRef(1)
	{}

	virtual unsigned long INTCALLTYPE AddRef() 
	{
		m_dwRef++; 
		return m_dwRef; 
	}
	virtual unsigned long INTCALLTYPE Release() 
	{
		unsigned long NewRef = --m_dwRef;
		if (NewRef == 0) 
			delete this; 

		return NewRef;
	}

	bool INTCALLTYPE GetInterface(const nv_sys::NVGUID& guid, void** ppObj)
	{
		if (EqualNVGUID(guid, IID_INVObject))
		{
			*ppObj = static_cast<INVInterpolator*>(this);
		}
		else if (EqualNVGUID(guid, IID_INVInterpolator))
		{		
			*ppObj = static_cast<INVInterpolator*>(this);
		}
		else
		{
			return false;
		}
			
		static_cast<INVInterpolator*>(this)->AddRef();
		return true;	
	}

	// IInterpolator
	nv_sys::NVType INTCALLTYPE Interpolate(const INVConnectionParameter* pParam, unsigned int Time) const
	{
		int Time1;
		int Time2;

		unsigned int NumKeys = pParam->GetNumKeys();
		if (NumKeys == 0)
		{
			return pParam->GetDefaultValue();
		}

		std::vector<unsigned int> Indices(NumKeys);
		pParam->GetSortedTimeIndices(&Indices[0], NumKeys);


		// Constant has one key, the first one.
		if (m_InterpolatorType == INTERPOLATOR_CONSTANT)
		{
			return *pParam->GetKeyFromIndex(Indices[0], Time1);
		}


		const nv_sys::NVType* pFirstKey;
		const nv_sys::NVType* pSecondKey;
		if (m_InterpolatorType == INTERPOLATOR_STEP)
		{
			Time2 = -1;
			pFirstKey = pParam->GetKeyFromIndex(Indices[0], Time1);
			pSecondKey = pFirstKey;
			for (unsigned int KeyNum = 1; KeyNum < NumKeys; KeyNum++)
			{
				pSecondKey = pParam->GetKeyFromIndex(Indices[KeyNum], Time2);
				if (Time2 >= Time)
					break;

				Time1 = Time2;
				pFirstKey = pSecondKey;
			}
			
			if (Time2 == Time)
				return *pSecondKey;

			return *pFirstKey;
		}
		else if (m_InterpolatorType == INTERPOLATOR_LINEAR)
		{
			pFirstKey = pParam->GetKeyFromIndex(Indices[0], Time1);
			Time2 = Time1;
			pSecondKey = pFirstKey;
			for (unsigned int KeyNum = 1; KeyNum < NumKeys; KeyNum++)
			{
				pSecondKey = pParam->GetKeyFromIndex(Indices[KeyNum], Time2);
				if (Time2 >= Time)
					break;

				Time1 = Time2;
				pFirstKey = pSecondKey;
			}
			
			if (Time2 == Time1)
			{
				return *pFirstKey;
			}

			int TimeDiff = Time2 - Time1;
			float fLerp = (((float)(Time - Time1)) / (float)TimeDiff);

			#define lerp(t, a, b) ( a + ((b - a) * t))
			nv_sys::NVType bminusa = *pSecondKey - *pFirstKey;
			nv_sys::NVType bminusatimest = bminusa * nv_sys::NVType::CreateFloatType(fLerp);

			nv_sys::NVType Ret = lerp(nv_sys::NVType::CreateFloatType(fLerp), *pFirstKey, *pSecondKey);
			return Ret;
		}		

		assert(0);
		return NVType::CreateNULLType();
	}

private:
	unsigned long m_dwRef;
	eINTERPOLATOR m_InterpolatorType;
};


}; // nv_sys namespace 