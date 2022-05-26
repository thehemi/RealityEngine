#ifndef EXPORT_X_FILE_INCLUDED
#define EXPORT_X_FILE_INCLUDED

extern bool gbNan_found;
extern float gWorldScale;

TriObject *GetTriObjectFromObjRef
(
 Object* pObj, 
 BOOL *pbDeleteIt
 ) ;

// Macros for saving data to memory at DWORD* pbCur (this pointer is incremented)
#define WRITE_PTCHAR(pbCur, ptchar) {TCHAR** __pptchar = (TCHAR**)pbCur; *(__pptchar++) = ptchar;\
	pbCur = (PBYTE)__pptchar;}

/*#define WRITE_STRING(pbCur, pstring) {TCHAR* __pCurrDestChar = (TCHAR*)pbCur; TCHAR* __pCurrOrgChar = pstring;\
	while(NULL != *__pCurrOrgChar) { *(__pCurrDestChar++) = *(__pCurrOrgChar++); }\
	*(__pCurrDestChar++) = _T('\0'); pbCur = (PBYTE)__pCurrDestChar;}\
*/
#define WRITE_STRING(pbCur, ptr) {memcpy(pbCur,&ptr,sizeof(TCHAR**)); pbCur += sizeof(TCHAR**);}

#define WRITE_WORD(pbCur, word) {WORD* __pword = (WORD*)pbCur; *(__pword++) = word;\
	pbCur = (PBYTE)__pword;}

#define WRITE_DWORD(pbCur, dword) {DWORD* __pdword = (DWORD*)pbCur; *(__pdword++) = dword;\
	pbCur = (PBYTE)__pdword;}


void NAN_FLOAT_TEST(float *fValue);

#define WRITE_FLOAT(pbCur, _float_value) {float* __pfloat = (float*)pbCur; float _float = (float)_float_value; \
	NAN_FLOAT_TEST(&_float); \
	*(__pfloat++) = _float; pbCur = (PBYTE)__pfloat;}

//###danhoro $NOTE switched MAX Z-up RHS to D3D Z-front LHS by swapping element 1 and 2

#define WRITE_POINT3(pbCur, _point3) {Point3 _temp = (Point3)_point3; float __tempVal;\
	__tempVal = _temp[0]; WRITE_FLOAT(pbCur, __tempVal);\
	__tempVal = _temp[2]; WRITE_FLOAT(pbCur, __tempVal);\
	__tempVal = _temp[1]; WRITE_FLOAT(pbCur, __tempVal);}

// TIM: Added scaling to meters
#define WRITE_POINT3_SCALED(pbCur, _point3) {Point3 _temp = (Point3)_point3; float __tempVal;\
	__tempVal = _temp[0]*gWorldScale; WRITE_FLOAT(pbCur, __tempVal);\
	__tempVal = _temp[2]*gWorldScale; WRITE_FLOAT(pbCur, __tempVal);\
	__tempVal = _temp[1]*gWorldScale; WRITE_FLOAT(pbCur, __tempVal);}

#define WRITE_COLOR(pbCur, _color) {D3DXCOLOR _temp = (D3DXCOLOR)_color; float __tempVal;\
	__tempVal = _temp.r; WRITE_FLOAT(pbCur, __tempVal);\
	__tempVal = _temp.g; WRITE_FLOAT(pbCur, __tempVal);\
	__tempVal = _temp.b; WRITE_FLOAT(pbCur, __tempVal);\
	__tempVal = _temp.a; WRITE_FLOAT(pbCur, __tempVal);}

// Untransformed vec4
#define WRITE_VEC4(pbCur, _color) {D3DXVECTOR4 _temp = (D3DXVECTOR4)_color; float __tempVal;\
	__tempVal = _temp.x; WRITE_FLOAT(pbCur, __tempVal);\
	__tempVal = _temp.y; WRITE_FLOAT(pbCur, __tempVal);\
	__tempVal = _temp.z; WRITE_FLOAT(pbCur, __tempVal);\
	__tempVal = _temp.w; WRITE_FLOAT(pbCur, __tempVal);}

//###danhoro $NOTE switched MAX Z-up RHS to D3D Z-front LHS by swapping row 1 and 2
#define WRITE_MATRIX4_FROM_MATRIX3(pbCur, _matrix3) {Point3 __tempRow = ((Matrix3)_matrix3).GetRow(0);\
	WRITE_POINT3(pbCur, __tempRow); WRITE_FLOAT(pbCur, 0);\
	__tempRow = _matrix3.GetRow(2); WRITE_POINT3(pbCur, __tempRow); WRITE_FLOAT(pbCur, 0);\
	__tempRow = _matrix3.GetRow(1); WRITE_POINT3(pbCur, __tempRow); WRITE_FLOAT(pbCur, 0);\
	__tempRow = _matrix3.GetRow(3); WRITE_POINT3_SCALED(pbCur, __tempRow); WRITE_FLOAT(pbCur, 1);}

#define WRITE_MATRIX(pbCur, mat) { *(D3DXMATRIX*)pbCur = mat;\
	pbCur = (PBYTE)pbCur + sizeof(D3DXMATRIX);}




struct SBoneInfo
{
	INode                       *m_pBoneNode;
	DWORD                       m_cVertices;
};

// structure used to map an mesh to a bone info structure
struct SSkinMap
{
	SSkinMap()
		:m_pMeshNode(NULL), m_rgbiBones(NULL), m_cbiBones(0), m_cbiBonesMax(0) {}
		~SSkinMap()
		{ delete []m_rgbiBones; }

		INode                       *m_pMeshNode;

		SBoneInfo                   *m_rgbiBones;
		DWORD                       m_cbiBones;
		DWORD                       m_cbiBonesMax;

		SBoneInfo *FindBone(INode *pBoneNode)
		{
			SBoneInfo *pbi = NULL;
			DWORD iBone;

			for (iBone = 0; iBone < m_cbiBones; iBone++)
			{
				if (pBoneNode == m_rgbiBones[iBone].m_pBoneNode)
				{
					pbi = &m_rgbiBones[iBone];
					break;
				}
			}

			return pbi;
		}

		HRESULT AddBone(INode *pBoneNode, SBoneInfo **ppbiBoneInfo)
		{
			HRESULT hr = S_OK;
			SBoneInfo *rgbiTemp;

			// reallocate if neccessary
			if (m_cbiBones == m_cbiBonesMax)
			{
				m_cbiBonesMax = max(1, m_cbiBonesMax);
				m_cbiBonesMax *= 2;

				rgbiTemp = m_rgbiBones;
				m_rgbiBones = new SBoneInfo[m_cbiBonesMax];
				if (m_rgbiBones == NULL)
				{
					m_rgbiBones = rgbiTemp;
					hr = E_OUTOFMEMORY;
					goto e_Exit;
				}

				if (m_cbiBones > 0)
				{
					memcpy(m_rgbiBones, rgbiTemp, m_cbiBones * sizeof(SBoneInfo));
				}

				delete []rgbiTemp;
			}

			// not initialize the next bone in the array and return a pointer to it

			m_rgbiBones[m_cbiBones].m_cVertices = 0;
			m_rgbiBones[m_cbiBones].m_pBoneNode = pBoneNode;

			*ppbiBoneInfo = &m_rgbiBones[m_cbiBones];

			m_cbiBones += 1;

e_Exit:
			return hr;
		}
};

struct SPreprocessContext
{
	SPreprocessContext()
		:m_pInterface(NULL),
		m_rgpsmSkinMaps(NULL), 
		m_cpsmSkinMaps(NULL), 
		m_cpsmSkinMapsMax(0), 
		m_cMaxWeightsPerVertex(0), 
		m_cMaxWeightsPerFace(0),
		m_cNodes(0) {}

		~SPreprocessContext()
		{ if(m_rgpsmSkinMaps) delete []m_rgpsmSkinMaps; }

		BOOL                        m_bSaveSelection;

		Interface                   *m_pInterface;
		SSkinMap                    **m_rgpsmSkinMaps;
		DWORD                       m_cpsmSkinMaps;
		DWORD                       m_cpsmSkinMapsMax;

		DWORD                       m_cMaxWeightsPerVertex;
		DWORD                       m_cMaxWeightsPerFace;

		DWORD                       m_cNodes;
		BOOL						m_bSavePrefabData;
};

const int x_cbStringBufferMax = 4088;

struct SStringBlock
{
	SStringBlock()
		:m_psbNext(NULL), m_cbData(0) {}
		~SStringBlock()
		{
			delete m_psbNext;
		}

		SStringBlock				*m_psbNext;
		DWORD						m_cbData;

		TCHAR						szData[x_cbStringBufferMax];
};

class CStringTable
{
public:
	CStringTable()
		:m_psbHead(NULL) {}

		~CStringTable()
		{
			delete m_psbHead;
		}

		// allocate a string out of the data blocks to be free'd later, and make it a valid
		//   x-file name at the same time
		TCHAR *CreateNiceString(TCHAR *szString)
		{
			TCHAR* szNewString = NULL;
			BOOL bFirstCharIsDigit;
			DWORD cbLength;
			SStringBlock *psbNew;

			if (szString == NULL)
				return NULL;

			cbLength = _tcslen(szString) + 1;

			bFirstCharIsDigit = _istdigit(*szString);
			if (bFirstCharIsDigit)
			{
				cbLength += 1;
			}

			// if no string blocks or the current doesn't have enough space, then allocate one
			if ((m_psbHead == NULL) || ((x_cbStringBufferMax - m_psbHead->m_cbData) < cbLength))
			{
				psbNew = new SStringBlock();
				if (psbNew == NULL)
					return NULL;

				psbNew->m_psbNext = m_psbHead;
				m_psbHead = psbNew;
			}

			// allocate a string out of the data block
			szNewString = m_psbHead->szData + m_psbHead->m_cbData;
			m_psbHead->m_cbData += cbLength;

			// deal with the fact that the string can't start with digits
			*szNewString = _T('\0');
			if( bFirstCharIsDigit ) 
			{
				_tcscat(szNewString, _T("_"));
			}

			_tcscat(szNewString, szString);

			TCHAR* pchCur = szNewString;
			while( NULL != *pchCur )
			{
				if( *pchCur != _T('_') && !_istalnum(*pchCur) )
				{
					*pchCur = _T('_');
				}
				pchCur++;
			}
			return szNewString;
		}

		// Allocate a new string with '\\' in place of '\' characters
		TCHAR* CreateNiceFilename(TCHAR *szString)
		{
			TCHAR* szNewString = NULL;
			DWORD cbNameLength;
			DWORD cbLength;
			TCHAR* pchCur;
			TCHAR* pchOrig;
			SStringBlock *psbNew;

			if( NULL == szString )
			{
				return NULL;
			}

			cbNameLength = _tcslen(szString);
			cbLength = cbNameLength*2 + 1;


			// if no string blocks or the current doesn't have enough space, then allocate one
			if ((m_psbHead == NULL) || ((x_cbStringBufferMax - m_psbHead->m_cbData) < cbLength))
			{
				psbNew = new SStringBlock();
				if (psbNew == NULL)
					return NULL;

				psbNew->m_psbNext = m_psbHead;
				m_psbHead = psbNew;
			}

			// allocate a string out of the data block
			szNewString = m_psbHead->szData + m_psbHead->m_cbData;
			m_psbHead->m_cbData += cbLength;

			pchCur = szNewString;
			pchOrig = szString;
			while (NULL != *pchOrig)
			{
				if( _T('\\') == *pchOrig )
				{
					*(pchCur++) = _T('\\');
					*(pchCur++) = _T('\\');
				}
				else
				{
					*(pchCur++) = *pchOrig;
				}
				pchOrig++;
			}
			*pchCur = _T('\0');

			return szNewString;
		}

		// Allocate a new string without fiddling with the '\' characters
		TCHAR* CreateNormalFilename(TCHAR *szString)
		{
			TCHAR* szNewString = NULL;
			DWORD cbNameLength;
			DWORD cbLength;
			SStringBlock *psbNew;

			if( NULL == szString )
			{
				return NULL;
			}

			cbNameLength = _tcslen(szString);
			cbLength = cbNameLength + 1;


			// if no string blocks or the current doesn't have enough space, then allocate one
			if ((m_psbHead == NULL) || ((x_cbStringBufferMax - m_psbHead->m_cbData) < cbLength))
			{
				psbNew = new SStringBlock();
				if (psbNew == NULL)
					return NULL;

				psbNew->m_psbNext = m_psbHead;
				m_psbHead = psbNew;
			}

			// allocate a string out of the data block
			szNewString = m_psbHead->szData + m_psbHead->m_cbData;
			m_psbHead->m_cbData += cbLength;

			memcpy(szNewString, szString, cbLength);

			return szNewString;
		}
private:
	SStringBlock *m_psbHead;
};

struct SSaveContext
{
	SSaveContext()
		:m_rgpsmSkinMaps(NULL), m_pAnimationSet(NULL) {}

		~SSaveContext()
		{ delete []m_rgpsmSkinMaps; }

		LPDIRECTXFILESAVEOBJECT     m_pxofsave;
		DXFILEFORMAT                m_xFormat;
		DWORD                       m_iTime;
		BOOL                        m_bSaveSelection;
		BOOL                        m_bSavePatchData;
		BOOL                        m_bSaveAnimationData;
		BOOL                        m_bLoopingAnimationData;
		DWORD                       m_iAnimSamplingRate;
		DWORD                       m_cMaxWeightsPerVertex;
		DWORD                       m_cMaxWeightsPerFace;

		SSkinMap                    **m_rgpsmSkinMaps;
		DWORD                       m_cpsmSkinMaps;

		LPDIRECTXFILEDATA           m_pAnimationSet;
		Interface                   *m_pInterface;

		DWORD                       m_cNodes;
		DWORD                       m_cNodesCur;
		INode                       **m_rgpnNodes;

		// TIM
		INode						*rootNode;
		vector<INode*>				splitLights;
		vector<string>				m_SavedMaterials; // Used so we know if we can create a reference instead of saving material again
		BOOL						m_bSavePrefabData;


		CStringTable				m_stStrings;

		SSkinMap *GetSkinMap(INode *pMeshNode)
		{
			SSkinMap *psm = NULL;
			DWORD iMesh;

			for (iMesh = 0; iMesh < m_cpsmSkinMaps; iMesh++)
			{
				if (pMeshNode == m_rgpsmSkinMaps[iMesh]->m_pMeshNode)
				{
					psm = m_rgpsmSkinMaps[iMesh];
					break;
				}
			}

			return psm;
		}

};


#endif //EXPORT_X_FILE_INCLUDED