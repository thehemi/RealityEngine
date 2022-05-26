#include "pch.h"
#include "RenderMesh.h"
#include "Material.h"
#include "vertexshader.h"

using namespace nv_sys;
using namespace std;
using namespace nv_renderdevice;

typedef struct tagConnectionHeader
{
	DWORD m_dwVersion;
	eSEMANTICID m_SemanticID;
	eNVTYPEID m_Type;

	DWORD m_dwNumAnnotations;
	DWORD m_dwNumValues;
	DWORD m_dwSize;
	DWORD m_dwRows;
	DWORD m_dwColumns;

	DWORD m_dwReserved;

} tConnectionHeader;

typedef struct tagAnnotationSaveInfo
{
	DWORD m_dwVersion;
	eANNOTATIONNAMEID m_NameID;
	eANNOTATIONVALUEID m_ValueID;
	eNVTYPEID m_NameType;
	eNVTYPEID m_ValueType;
	DWORD m_dwNameSize;
	DWORD m_dwValueSize;
	DWORD m_dwNameRows;
	DWORD m_dwNameColumns;
	DWORD m_dwValueRows;
	DWORD m_dwValueColumns;

	DWORD m_dwReserved;
} tAnnotationSaveInfo;

typedef struct tagKeySaveInfo
{
	DWORD m_dwVersion;
	TimeValue m_Time;
	DWORD m_dwSize;
	DWORD m_dwReserved;
} tKeySaveInfo;

bool SaveConnections(ISave* pSave, INVParameterList* pParams)
{
	unsigned long Written;

	if(!pSave || !pParams)
		return false;

	pSave->BeginChunk(CONNECTION_CHUNK);

	// Write the number of connections
	DWORD dwNumConnections = pParams->GetNumParameters();
	pSave->Write(&dwNumConnections, sizeof(DWORD), &Written);

	for (unsigned int i = 0; i < dwNumConnections; i++)
	{
		INVConnectionParameter* pParam = pParams->GetConnectionParameter(i);

		// Add a private string annotation to record the current texture in this parameter.
		if (pParam->GetDefaultValue().GetType() == NVTYPEID_TEXTURE ||
			pParam->GetDefaultValue().GetType() == NVTYPEID_TEXTURE2D ||
			pParam->GetDefaultValue().GetType() == NVTYPEID_TEXTURE3D ||
			pParam->GetDefaultValue().GetType() == NVTYPEID_TEXTURECUBE)
		{
			int Time = 0;
			NVType Key = pParam->GetValueAtTime(Time);
			if (Key.GetTexture() != 0)
			{
				std::string strName = TextureMgr::GetSingletonPtr()->GetTextureName(Key.GetTexture());
				pParam->GetDefaultValue().GetObjectSemantics()->AddAnnotation(ANNOTATIONNAMEID_CGMAXPLUGCURRENTTEXTURENAME_PRIVATE, NVType::CreateStringType(strName.c_str()));
			}
		}

		// Write the name of this connection
		DWORD dwStrLen = strlen(pParam->GetDefaultValue().GetObjectSemantics()->GetName()) + 1;
		pSave->Write(&dwStrLen, sizeof(DWORD), &Written);
		pSave->Write(pParam->GetDefaultValue().GetObjectSemantics()->GetName(), dwStrLen, &Written);
				
		// Write the header for this connection
		tConnectionHeader theHeader;
		theHeader.m_dwVersion = 0;
		theHeader.m_SemanticID = pParam->GetDefaultValue().GetObjectSemantics()->GetSemanticID();
		theHeader.m_Type = pParam->GetDefaultValue().GetType();
		theHeader.m_dwNumAnnotations = pParam->GetDefaultValue().GetObjectSemantics()->GetNumAnnotations();
		theHeader.m_dwReserved = 0;
		theHeader.m_dwRows = pParam->GetDefaultValue().GetRows();
		theHeader.m_dwColumns = pParam->GetDefaultValue().GetColumns();
		theHeader.m_dwNumValues = pParam->GetNumKeys();
		pParam->GetDefaultValue().GetValue(theHeader.m_dwSize);

		pSave->Write(&theHeader, sizeof(tConnectionHeader), &Written);

		// Write the annotations:
		for (unsigned int CurrentAnnotation = 0; CurrentAnnotation < theHeader.m_dwNumAnnotations; CurrentAnnotation++)
		{
			const tAnnotationInfo* pInfo = pParam->GetDefaultValue().GetObjectSemantics()->GetAnnotation(CurrentAnnotation);
			tAnnotationSaveInfo theAnnotation;
			theAnnotation.m_dwVersion = 0;
			theAnnotation.m_NameID = pInfo->m_NameID;
			theAnnotation.m_ValueID = pInfo->m_ValueID;
			theAnnotation.m_NameType = pInfo->m_Name.GetType();
			theAnnotation.m_ValueType = pInfo->m_Value.GetType();
			theAnnotation.m_dwNameRows = pInfo->m_Name.GetRows();
			theAnnotation.m_dwNameColumns = pInfo->m_Name.GetColumns();
			theAnnotation.m_dwValueRows = pInfo->m_Value.GetRows();
			theAnnotation.m_dwValueColumns = pInfo->m_Value.GetColumns();
			
			theAnnotation.m_dwReserved = 0;

			theAnnotation.m_dwNameSize = 0;
			const void* pNameData = pInfo->m_Name.GetValue(theAnnotation.m_dwNameSize);
			theAnnotation.m_dwValueSize = 0;
			const void* pValueData = pInfo->m_Value.GetValue(theAnnotation.m_dwValueSize);

			pSave->Write(&theAnnotation, sizeof(tAnnotationSaveInfo), &Written);
			
			// Write the annotations
			if (theAnnotation.m_NameType != NVTYPEID_UNKNOWN)
			{
				pSave->Write(pNameData, theAnnotation.m_dwNameSize, &Written);
			}

			if (theAnnotation.m_ValueType != NVTYPEID_UNKNOWN)
			{
				pSave->Write(pValueData, theAnnotation.m_dwValueSize, &Written);
			}

		}

		TimeValue theTime;


		// Now write each key value.
		for (unsigned int CurrentKey = 0; CurrentKey < pParam->GetNumKeys(); CurrentKey++)
		{
			const NVType* pKey = pParam->GetKeyFromIndex(CurrentKey, theTime);

			tKeySaveInfo theKey;
			theKey.m_dwVersion = 0;
			theKey.m_dwReserved = 0;
			theKey.m_Time = theTime;
			theKey.m_dwSize = 0;

			const void* pValue = pKey->GetValue(theKey.m_dwSize);

			pSave->Write(&theKey, sizeof(tKeySaveInfo), &Written);
			
			pSave->Write(pValue, theKey.m_dwSize, &Written);
		}
	}
	pSave->EndChunk();


	return true;
}

INVParameterList* LoadConnections(MaxVertexShader* pVS, ILoad* pLoad)
{
	unsigned long Read;

	INVParameterList* pParamList = INVParameterList::Create();
	
	// Read the number of connections
	DWORD dwNumConnections;
	pLoad->Read(&dwNumConnections, sizeof(DWORD), &Read);

	for (unsigned int i = 0; i < dwNumConnections; i++)
	{
		TCHAR* pszParamName = NULL;
		DWORD dwStrLength;
		INVConnectionParameter* pParam = INVConnectionParameter::Create();

		pParam->Animate(true);
		pLoad->Read(&dwStrLength, sizeof(DWORD), &Read);
		
		if (dwStrLength)
		{
			pszParamName = (TCHAR*)malloc(dwStrLength);
			pLoad->Read(pszParamName, dwStrLength, &Read);
		}

		// Read the header for this connection
		tConnectionHeader theHeader;
		pLoad->Read(&theHeader, sizeof(tConnectionHeader), &Read);

		INVObjectSemantics* pSemantics = INVObjectSemantics::Create();

		pSemantics->SetSemanticID(theHeader.m_SemanticID);
		
		if (pszParamName)
		{
			pSemantics->SetName(pszParamName);
			free(pszParamName);
		}

		void* pData;

		// Read the annotations:
		for (unsigned int CurrentAnnotation = 0; CurrentAnnotation < theHeader.m_dwNumAnnotations; CurrentAnnotation++)
		{
			tAnnotationSaveInfo theAnnotation;
			pLoad->Read(&theAnnotation, sizeof(tAnnotationSaveInfo), &Read);

			NVType theValue;
			NVType theName;

			// Read the annotations back
			if (theAnnotation.m_NameType != NVTYPEID_UNKNOWN)
			{
				pData = malloc(theAnnotation.m_dwNameSize);
				pLoad->Read(pData, theAnnotation.m_dwNameSize, &Read);

				StreamNVType(theName, theAnnotation.m_NameType, pData, theAnnotation.m_dwNameRows, theAnnotation.m_dwNameColumns);
				free(pData);
			}

			if (theAnnotation.m_ValueType != NVTYPEID_UNKNOWN)
			{
				pData = malloc(theAnnotation.m_dwValueSize);
				pLoad->Read(pData, theAnnotation.m_dwValueSize, &Read);

				StreamNVType(theValue, theAnnotation.m_ValueType, pData, theAnnotation.m_dwValueRows, theAnnotation.m_dwValueColumns);
				free(pData);
			}

			if ((theName.GetType() == NVTYPEID_STRING) && (theValue.GetType() != NVTYPEID_UNKNOWN))
			{
				pSemantics->AddAnnotation(theName.GetString(), theValue);
			}
			else
			{
				pSemantics->AddAnnotation(theAnnotation.m_NameID, theAnnotation.m_ValueID);
			}

		}

		// Now read each key value.
		for (unsigned int CurrentKey = 0; CurrentKey < theHeader.m_dwNumValues; CurrentKey++)
		{
			tKeySaveInfo theKey;
			pLoad->Read(&theKey, sizeof(tKeySaveInfo), &Read);

			if (theKey.m_dwSize)
			{
				pData = malloc(theKey.m_dwSize);
				pLoad->Read(pData, theKey.m_dwSize, &Read);

				NVType theValue;
				if ((theHeader.m_Type != NVTYPEID_TEXTURE) &&
					(theHeader.m_Type != NVTYPEID_TEXTURE2D) &&
					(theHeader.m_Type != NVTYPEID_TEXTURE3D) &&
					(theHeader.m_Type != NVTYPEID_TEXTURECUBE))
				{
					StreamNVType(theValue, theHeader.m_Type, pData, theHeader.m_dwRows, theHeader.m_dwColumns);
				}
				else
				{
					switch(theHeader.m_Type)
					{
						case NVTYPEID_TEXTURE:
							theValue.SetTexture(0);
							break;
						case NVTYPEID_TEXTURE2D:
							theValue.SetTexture2D(0);
							break;
						case NVTYPEID_TEXTURE3D:
							theValue.SetTexture3D(0);
							break;
						case NVTYPEID_TEXTURECUBE:
							theValue.SetTextureCube(0);
							break;
					}
				}

				theValue.SetObjectSemantics(pSemantics);
				if (CurrentKey == 0)
				{
					pParam->SetDefaultValue(theValue);
				}

				pParam->SetKey(theKey.m_Time, theValue);
				theValue.Dump();

				free(pData);
			}
		}

		SAFE_RELEASE(pSemantics);
		
		pParam->Animate(false);

		const tAnnotationInfo* pInfo = pParam->GetDefaultValue().GetObjectSemantics()->FindAnnotationInfo(ANNOTATIONNAMEID_CGMAXPLUGCURRENTTEXTURENAME_PRIVATE);
		if (pInfo && (pInfo->m_Value.GetType() == NVTYPEID_STRING))
		{
			INVTexture* pTexture = pVS->LoadTexture(pInfo->m_Value.GetString(), pParam);
			//assert(pTexture);

            pParam->SetKey(0, NVType::CreateTextureType(pTexture));
		}

		pParamList->AddConnectionParameter(pParam);
		
	}

	return pParamList;
}

bool CopyConnection(INVConnectionParameter* pDest, INVConnectionParameter* pSource)
{
	pDest->Animate(true);
	pSource->Animate(true);

	pDest->DeleteAllKeys();

	int Time;
	for (unsigned int KeyNum = 0; KeyNum < pSource->GetNumKeys(); KeyNum++)
	{
		const nv_sys::NVType* pKey = pSource->GetKeyFromIndex(KeyNum, Time);
		pDest->SetKey(Time, *pKey);
	}

	pDest->Animate(false);
	pSource->Animate(false);

	return true;
}

bool ResolveConnections(INVParameterList* pDest, INVParameterList* pSource, bool bLoading)
{
	enum
	{
		PASS_0 = 0,
		PASS_1 = 1,
		PASS_END = 2
	};

	typedef set<unsigned int> tSourceNumTaken;
	tSourceNumTaken theTakenSources;

	for (unsigned int Pass = 0; Pass < PASS_END; Pass++)
	{
		for (unsigned int NumDest = 0; NumDest < pDest->GetNumParameters(); NumDest++)
		{
			INVConnectionParameter* pDestParam = pDest->GetConnectionParameter(NumDest);
			
			for (unsigned int NumSource = 0; NumSource < pSource->GetNumParameters(); NumSource++)
			{
				// Don't do this one.
				if (theTakenSources.find(NumSource) != theTakenSources.end())
					continue;

				INVConnectionParameter* pSourceParam = pSource->GetConnectionParameter(NumSource);

				// Types always have to match.
				if (pSourceParam->GetDefaultValue().GetType() != 
					pDestParam->GetDefaultValue().GetType())
					continue;

				// Semantics have to match or be not present on both sides.
				INVObjectSemantics* pSourceSemantics = pSourceParam->GetDefaultValue().GetObjectSemantics();
				INVObjectSemantics* pDestSemantics = pDestParam->GetDefaultValue().GetObjectSemantics();
				if ( ((pSourceSemantics->GetSemanticID() != SEMANTICID_UNKNOWN) || (pDestSemantics->GetSemanticID() != SEMANTICID_UNKNOWN)) &&
					 (pSourceSemantics->GetSemanticID() != pDestSemantics->GetSemanticID()))
				{
					continue;
				}

				// TIM: Why were we doing this???
				// Checked above for not equal
				// If not loading, never copy semantics that are unknown.
				//if ((pDestSemantics->GetSemanticID() == SEMANTICID_UNKNOWN) && !bLoading)
				//{
				//	continue;
//
				//}

				// Object annotations have to match or not be present on both sides.
				eANNOTATIONVALUEID SourceObjectID = pSourceSemantics->FindAnnotationValue(ANNOTATIONNAMEID_OBJECT);
				eANNOTATIONVALUEID DestObjectID = pDestSemantics->FindAnnotationValue(ANNOTATIONNAMEID_OBJECT);
				if ( ((SourceObjectID != ANNOTATIONVALUEID_UNKNOWN) || (DestObjectID != ANNOTATIONVALUEID_UNKNOWN)) &&
					 (SourceObjectID != DestObjectID))
				{
					continue;
				}

				// targettype annotations have to match or not be present on both sides.
				eANNOTATIONVALUEID SourceTextureID = pSourceSemantics->FindAnnotationValue(ANNOTATIONNAMEID_TEXTURETYPE);
				eANNOTATIONVALUEID DestTextureID = pDestSemantics->FindAnnotationValue(ANNOTATIONNAMEID_TEXTURETYPE);
				if ( ((SourceTextureID != ANNOTATIONVALUEID_UNKNOWN) || (DestTextureID != ANNOTATIONVALUEID_UNKNOWN)) &&
					 (SourceTextureID != DestTextureID))
				{
					continue;
				}

				// enable annotations have to match or not be present on both sides.
				eANNOTATIONVALUEID SourceEnableID = pSourceSemantics->FindAnnotationValue(ANNOTATIONNAMEID_ENABLE);
				eANNOTATIONVALUEID DestEnableID = pDestSemantics->FindAnnotationValue(ANNOTATIONNAMEID_ENABLE);
				if ( ((SourceEnableID != ANNOTATIONVALUEID_UNKNOWN) || (DestEnableID != ANNOTATIONVALUEID_UNKNOWN)) &&
					 (SourceEnableID != DestEnableID))
				{
					continue;
				}

				// First pass, check for matching name
				// In the case where we loaded or reloaded an effect this will always be the same of course.
				if (PASS_0 == Pass)
				{
					if (strcmp(pSourceSemantics->GetName(), pDestSemantics->GetName()) == 0)
					{
						// Found it.  A perfect match.
						theTakenSources.insert(NumSource);

						CopyConnection(pDestParam, pSourceParam);
						
						break;
					}
				}
				else if (PASS_1 == Pass)
				{
					// We got here so we found a type with matching semantics, object, and type, but
					// not name.  Which is good enough for us....
					// Check that semantics are non-null.
					// We have to be a little bit careful about copying parameters
					if ((pSourceSemantics->GetSemanticID() != SEMANTICID_UNKNOWN) && (pDestSemantics->GetSemanticID() != SEMANTICID_UNKNOWN))
					{
						theTakenSources.insert(NumSource);

						CopyConnection(pDestParam, pSourceParam);
					}
					break;
				}
			}
		}
	}

	return true;

}


