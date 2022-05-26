/*********************************************************************NVMH4****
Path:  NVSDK\Common\src\nv_gui
File:  CGFXView.cpp

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

// CGFXView.cpp : implementation file
//

#include "stdafx.h"
#include "nv_gui\gui.h"
#include "CGFXView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static LPTSTR s_apszCppNumberList[] =
{
	_T("true"),
	_T("false"),
    _T("none"),
    _T("linear"),
    _T("point"),
    _T("clamp"),
    _T("repeat"),
    NULL
};


//	C++ keywords (MSVC5.0 + POET5.0)
static LPTSTR s_apszCppKeywordList[] =
{
	_T("bool"),
	_T("const"),
	_T("uniform"),
	_T("varying"),
	_T("in"),
	_T("out"),
	_T("void"),
	_T("do"),
	_T("while"),
	_T("for"),
	_T("if"),
	_T("else"),
	_T("typedef"),
	_T("struct"),
	_T("discard"),
	_T("return"),
	_T("fixed"),
	_T("fixed2"),
	_T("fixed3"),
	_T("fixed4"),
	_T("half"),
	_T("half2"),
	_T("half3"),
	_T("half4"),
	_T("float"),
	_T("float2"),
	_T("float3"),
	_T("float4"),
	_T("float1x1"),
	_T("float1x2"),
	_T("float1x3"),
	_T("float1x4"),
	_T("float2x1"),
	_T("float2x2"),
	_T("float2x3"),
	_T("float2x4"),
	_T("float3x1"),
	_T("float3x2"),
	_T("float3x3"),
	_T("float3x4"),
	_T("float4x1"),
	_T("float4x2"),
	_T("float4x3"),
	_T("float4x4"),
	_T("abs"),
	_T("acos"),
	_T("asin"),
	_T("atan"),
	_T("atan2"),
	_T("ceil"),
	_T("clamp"),
	_T("cos"),
	_T("cosh"),
	_T("cross"),
	_T("ddx"),
	_T("ddy"),
	_T("degrees"),
	_T("dot"),
	_T("exp"),
	_T("exp2"),
	_T("floor"),
	_T("fmod"),
	_T("frexp"),
	_T("frac"),
	_T("isfinite"),
	_T("isinf"),
	_T("isnan"),
	_T("ldexp"),
	_T("log"),
	_T("log2"),
	_T("log10"),
    _T("mul"),
	_T("max"),
	_T("min"),
	_T("mix"),
	_T("lerp"),
	_T("modf"),
	_T("noise"),
	_T("pow"),
	_T("radians"),
	_T("round"),
	_T("rsqrt"),
	_T("sign"),
	_T("sin"),
	_T("sinh"),
	_T("smoothstep"),
	_T("step"),
	_T("sqrt"),
	_T("tan"),
	_T("tanh"),
	_T("distance"),
	_T("fresnel"),
	_T("length"),
	_T("normalize"),
	_T("reflect"),
	_T("reflectn"),
	_T("refract"),
	_T("refractn"),
	_T("f1tex1D"),
	_T("f2tex1D"),
	_T("f3tex1D"),
	_T("f4tex1D"),
	_T("h1tex1D"),
	_T("h2tex1D"),
	_T("h3tex1D"),
	_T("h4tex1D"),
	_T("x1tex1D"),
	_T("x2tex1D"),
	_T("x3tex1D"),
	_T("x4tex1D"),
	_T("f1tex1Dproj"),
	_T("f2tex1Dproj"),
	_T("f3tex1Dproj"),
	_T("f4tex1Dproj"),
	_T("h1tex1Dproj"),
	_T("h2tex1Dproj"),
	_T("h3tex1Dproj"),
	_T("h4tex1Dproj"),
	_T("x1tex1Dproj"),
	_T("x2tex1Dproj"),
	_T("x3tex1Dproj"),
	_T("x4tex1Dproj"),
	_T("f1tex2D"),
	_T("f2tex2D"),
	_T("f3tex2D"),
	_T("f4tex2D"),
	_T("h1tex2D"),
	_T("h2tex2D"),
	_T("h3tex2D"),
	_T("h4tex2D"),
	_T("x1tex2D"),
	_T("x2tex2D"),
	_T("x3tex2D"),
	_T("x4tex2D"),
	_T("f1tex2Dproj"),
	_T("f2tex2Dproj"),
	_T("f3tex2Dproj"),
	_T("f4tex2Dproj"),
	_T("h1tex2Dproj"),
	_T("h2tex2Dproj"),
	_T("h3tex2Dproj"),
	_T("h4tex2Dproj"),
	_T("x1tex2Dproj"),
	_T("x2tex2Dproj"),
	_T("x3tex2Dproj"),
	_T("x4tex2Dproj"),
	_T("f1tex3D"),
	_T("f2tex3D"),
	_T("f3tex3D"),
	_T("f4tex3D"),
	_T("h1tex3D"),
	_T("h2tex3D"),
	_T("h3tex3D"),
	_T("h4tex3D"),
	_T("x1tex3D"),
	_T("x2tex3D"),
	_T("x3tex3D"),
	_T("x4tex3D"),
	_T("f1tex3Dproj"),
	_T("f2tex3Dproj"),
	_T("f3tex3Dproj"),
	_T("f4tex3Dproj"),
	_T("h1tex3Dproj"),
	_T("h2tex3Dproj"),
	_T("h3tex3Dproj"),
	_T("h4tex3Dproj"),
	_T("x1tex3Dproj"),
	_T("x2tex3Dproj"),
	_T("x3tex3Dproj"),
	_T("x4tex3Dproj"),
	_T("f1texCUBE"),
	_T("f2texCUBE"),
	_T("f3texCUBE"),
	_T("f4texCUBE"),
	_T("h1texCUBE"),
	_T("h2texCUBE"),
	_T("h3texCUBE"),
	_T("h4texCUBE"),
	_T("x1texCUBE"),
	_T("x2texCUBE"),
	_T("x3texCUBE"),
	_T("x4texCUBE"),
	_T("f1texCUBEproj"),
	_T("f2texCUBEproj"),
	_T("f3texCUBEproj"),
	_T("f4texCUBEproj"),
	_T("h1texCUBEproj"),
	_T("h2texCUBEproj"),
	_T("h3texCUBEproj"),
	_T("h4texCUBEproj"),
	_T("x1texCUBEproj"),
	_T("x2texCUBEproj"),
	_T("x3texCUBEproj"),
	_T("x4texCUBEproj"),
	_T("f1texCUBE"),
	_T("f2texCUBE"),
	_T("f3texCUBE"),
	_T("f4texCUBE"),
	_T("h1texCUBE"),
	_T("h2texCUBE"),
	_T("h3texCUBE"),
	_T("h4texCUBE"),
	_T("x1texCUBE"),
	_T("x2texCUBE"),
	_T("x3texCUBE"),
	_T("x4texCUBE"),
	_T("f1texRECT"),
	_T("f2texRECT"),
	_T("f3texRECT"),
	_T("f4texRECT"),
	_T("h1texRECT"),
	_T("h2texRECT"),
	_T("h3texRECT"),
	_T("h4texRECT"),
	_T("x1texRECT"),
	_T("x2texRECT"),
	_T("x3texRECT"),
	_T("x4texRECT"),
	_T("f1texRECTproj"),
	_T("f2texRECTproj"),
	_T("f3texRECTproj"),
	_T("f4texRECTproj"),
	_T("h1texRECTproj"),
	_T("h2texRECTproj"),
	_T("h3texRECTproj"),
	_T("h4texRECTproj"),
	_T("x1texRECTproj"),
	_T("x2texRECTproj"),
	_T("x3texRECTproj"),
	_T("x4texRECTproj"),
	_T("f1texRECT"),
	_T("f2texRECT"),
	_T("f3texRECT"),
	_T("f4texRECT"),
	_T("h1texRECT"),
	_T("h2texRECT"),
	_T("h3texRECT"),
	_T("h4texRECT"),
	_T("x1texRECT"),
	_T("x2texRECT"),
	_T("x3texRECT"),
	_T("x4texRECT"),
	_T("f1texcompare2D"),
	_T("f1texcompare2D"),
	_T("f1texcompare2D"),
	_T("h1texcompare2D"),
	_T("h1texcompare2D"),
	_T("h1texcompare2D"),
	_T("x1texcompare2D"),
	_T("x1texcompare2D"),
	_T("x1texcompare2D"),
	_T("pack_2half"),
	_T("unpack_2half"),
	_T("pack_4clamp1s"),
	_T("unpack_4clamp1s"),
	_T("application2vertex"),
	_T("vertex2fragment"),
	_T("HPOS"),
	_T("PSIZ"),
	_T("WPOS"),
	_T("COL0"),
	_T("COL1"),
	_T("BCOL0"),
	_T("BCOL1"),
	_T("FOGP"),
	_T("FOGC"),
	_T("NRML"),
	_T("TEX0"),
	_T("TEX1"),
	_T("TEX2"),
	_T("TEX3"),
	_T("TEX4"),
	_T("TEX5"),
	_T("TEX6"),
	_T("TEX7"),
	_T("DEPR"),
	_T("ATTR0"),
	_T("ATTR1"),
	_T("ATTR2"),
	_T("ATTR3"),
	_T("ATTR4"),
	_T("ATTR5"),
	_T("ATTR6"),
	_T("ATTR7"),
	_T("ATTR8"),
	_T("ATTR9"),
	_T("ATTR10"),
	_T("ATTR11"),
	_T("ATTR12"),
	_T("ATTR13"),
	_T("ATTR14"),
	_T("ATTR15"),
	_T("sincos"),
    // CgFX
    _T("effect"),
	_T("technique"),
	_T("pass"),
	_T("sampler1D"),
	_T("sampler2D"),
	_T("sampler3D"),
	_T("samplerCUBE"),
    _T("samplerRECT"),
    _T("sampler_state"),
    _T("compile"),
	_T("asm"),
    _T("texture"),
    _T("VertexShader"),
    _T("PixelShader"),
	NULL
};

/////////////////////////////////////////////////////////////////////////////
// CCGFXView

IMPLEMENT_DYNCREATE(CCGFXView, CCrystalEditView)

CCGFXView::CCGFXView()
{
}

CCGFXView::~CCGFXView()
{
}


BEGIN_MESSAGE_MAP(CCGFXView, CCrystalEditView)
	//{{AFX_MSG_MAP(CSampleView)
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CCrystalEditView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CCrystalEditView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CCrystalEditView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCGFXView diagnostics

#ifdef _DEBUG
void CCGFXView::AssertValid() const
{
	CCrystalEditView::AssertValid();
}

void CCGFXView::Dump(CDumpContext& dc) const
{
	CCrystalEditView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CCGFXView message handlers
CCGFXDocument* CCGFXView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CCGFXDocument)));
	return (CCGFXDocument*)m_pDocument;
}

BOOL CCGFXView::PreCreateWindow(CREATESTRUCT& cs) 
{
	return CCrystalEditView::PreCreateWindow(cs);
}

CCrystalTextBuffer *CCGFXView::LocateTextBuffer()
{
	return &GetDocument()->m_xTextBuffer;
}

void CCGFXView::OnInitialUpdate() 
{
	CCrystalEditView::OnInitialUpdate();

	SetFont(GetDocument()->m_lf);
}

void CCGFXView::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	AfxMessageBox("Build your own context menu!");
}

static BOOL IsCppKeyword(LPCTSTR pszChars, int nLength)
{
	for (int L = 0; s_apszCppKeywordList[L] != NULL; L ++)
	{
		if (strncmp(s_apszCppKeywordList[L], pszChars, nLength) == 0
				&& s_apszCppKeywordList[L][nLength] == 0)
			return TRUE;
	}
	return FALSE;
}

static BOOL IsCppNumber(LPCTSTR pszChars, int nLength)
{
    CString test= pszChars;
    test.MakeLower();
	for (int L = 0; s_apszCppNumberList[L] != NULL; L ++)
	{
		if (strncmp(s_apszCppNumberList[L], (LPCTSTR)test, nLength) == 0
				&& s_apszCppNumberList[L][nLength] == 0)
			return TRUE;
	}

    if (nLength > 2 && pszChars[0] == '0' && pszChars[1] == 'x')
	{
		for (int I = 2; I < nLength; I++)
		{
			if (isdigit(pszChars[I]) || (pszChars[I] >= 'A' && pszChars[I] <= 'F') ||
										(pszChars[I] >= 'a' && pszChars[I] <= 'f'))
				continue;
			return FALSE;
		}
		return TRUE;
	}
	if (! isdigit(pszChars[0]))
		return FALSE;
	for (int I = 1; I < nLength; I++)
	{
		if (! isdigit(pszChars[I]) && pszChars[I] != '+' &&
			pszChars[I] != '-' && pszChars[I] != '.' && pszChars[I] != 'e' &&
			pszChars[I] != 'E')
			return FALSE;
	}
	return TRUE;
}

#define DEFINE_BLOCK(pos, colorindex)	\
	ASSERT((pos) >= 0 && (pos) <= nLength);\
	if (pBuf != NULL)\
	{\
		if (nActualItems == 0 || pBuf[nActualItems - 1].m_nCharPos <= (pos)){\
		pBuf[nActualItems].m_nCharPos = (pos);\
		pBuf[nActualItems].m_nColorIndex = (colorindex);\
		nActualItems ++;}\
	}

#define COOKIE_COMMENT			0x0001
#define COOKIE_PREPROCESSOR		0x0002
#define COOKIE_EXT_COMMENT		0x0004
#define COOKIE_STRING			0x0008
#define COOKIE_CHAR				0x0010

DWORD CCGFXView::ParseLine(DWORD dwCookie, int nLineIndex, TEXTBLOCK *pBuf, int &nActualItems)
{
	int nLength = GetLineLength(nLineIndex);
	if (nLength <= 0)
		return dwCookie & COOKIE_EXT_COMMENT;

	LPCTSTR pszChars    = GetLineChars(nLineIndex);
	BOOL bFirstChar     = (dwCookie & ~COOKIE_EXT_COMMENT) == 0;
	BOOL bRedefineBlock = TRUE;
	BOOL bDecIndex  = FALSE;
	int nIdentBegin = -1;
	for (int I = 0; ; I++)
	{
		if (bRedefineBlock)
		{
			int nPos = I;
			if (bDecIndex)
				nPos--;
			if (dwCookie & (COOKIE_COMMENT | COOKIE_EXT_COMMENT))
			{
				DEFINE_BLOCK(nPos, COLORINDEX_COMMENT);
			}
			else
			if (dwCookie & (COOKIE_CHAR | COOKIE_STRING))
			{
				DEFINE_BLOCK(nPos, COLORINDEX_STRING);
			}
			else
			if (dwCookie & COOKIE_PREPROCESSOR)
			{
				DEFINE_BLOCK(nPos, COLORINDEX_PREPROCESSOR);
			}
			else
			{
				DEFINE_BLOCK(nPos, COLORINDEX_NORMALTEXT);
			}
			bRedefineBlock = FALSE;
			bDecIndex      = FALSE;
		}

		if (I == nLength)
			break;

		if (dwCookie & COOKIE_COMMENT)
		{
			DEFINE_BLOCK(I, COLORINDEX_COMMENT);
			dwCookie |= COOKIE_COMMENT;
			break;
		}

		//	String constant "...."
		if (dwCookie & COOKIE_STRING)
		{
			if (pszChars[I] == '"' && (I == 0 || pszChars[I - 1] != '\\'))
			{
				dwCookie &= ~COOKIE_STRING;
				bRedefineBlock = TRUE;
			}
			continue;
		}

		//	Char constant '..'
		if (dwCookie & COOKIE_CHAR)
		{
			if (pszChars[I] == '\'' && (I == 0 || pszChars[I - 1] != '\\'))
			{
				dwCookie &= ~COOKIE_CHAR;
				bRedefineBlock = TRUE;
			}
			continue;
		}

		//	Extended comment /*....*/
		if (dwCookie & COOKIE_EXT_COMMENT)
		{
			if (I > 0 && pszChars[I] == '/' && pszChars[I - 1] == '*')
			{
				dwCookie &= ~COOKIE_EXT_COMMENT;
				bRedefineBlock = TRUE;
			}
			continue;
		}

		if (I > 0 && pszChars[I] == '/' && pszChars[I - 1] == '/')
		{
			DEFINE_BLOCK(I - 1, COLORINDEX_COMMENT);
			dwCookie |= COOKIE_COMMENT;
			break;
		}

		//	Preprocessor directive #....
		if (dwCookie & COOKIE_PREPROCESSOR)
		{
			if (I > 0 && pszChars[I] == '*' && pszChars[I - 1] == '/')
			{
				DEFINE_BLOCK(I - 1, COLORINDEX_COMMENT);
				dwCookie |= COOKIE_EXT_COMMENT;
			}
			continue;
		}

		//	Normal text
		if (pszChars[I] == '"')
		{
			DEFINE_BLOCK(I, COLORINDEX_STRING);
			dwCookie |= COOKIE_STRING;
			continue;
		}
		if (pszChars[I] == '\'')
		{
			DEFINE_BLOCK(I, COLORINDEX_STRING);
			dwCookie |= COOKIE_CHAR;
			continue;
		}
		if (I > 0 && pszChars[I] == '*' && pszChars[I - 1] == '/')
		{
			DEFINE_BLOCK(I - 1, COLORINDEX_COMMENT);
			dwCookie |= COOKIE_EXT_COMMENT;
			continue;
		}

		if (bFirstChar)
		{
			if (pszChars[I] == '#')
			{
				DEFINE_BLOCK(I, COLORINDEX_PREPROCESSOR);
				dwCookie |= COOKIE_PREPROCESSOR;
				continue;
			}
			if (! isspace(pszChars[I]))
				bFirstChar = FALSE;
		}

		if (pBuf == NULL)
			continue;	//	We don't need to extract keywords,
						//	for faster parsing skip the rest of loop

		if (isalnum(pszChars[I]) || pszChars[I] == '_' || pszChars[I] == '.')
		{
			if (nIdentBegin == -1)
				nIdentBegin = I;
		}
		else
		{
			if (nIdentBegin >= 0)
			{
				if (IsCppKeyword(pszChars + nIdentBegin, I - nIdentBegin))
				{
					DEFINE_BLOCK(nIdentBegin, COLORINDEX_KEYWORD);
				}
				else
				if (IsCppNumber(pszChars + nIdentBegin, I - nIdentBegin))
				{
					DEFINE_BLOCK(nIdentBegin, COLORINDEX_NUMBER);
				}
				bRedefineBlock = TRUE;
				bDecIndex = TRUE;
				nIdentBegin = -1;
			}
		}
	}

	if (nIdentBegin >= 0)
	{
		if (IsCppKeyword(pszChars + nIdentBegin, I - nIdentBegin))
		{
			DEFINE_BLOCK(nIdentBegin, COLORINDEX_KEYWORD);
		}
		else
		if (IsCppNumber(pszChars + nIdentBegin, I - nIdentBegin))
		{
			DEFINE_BLOCK(nIdentBegin, COLORINDEX_NUMBER);
		}
	}

	if (pszChars[nLength - 1] != '\\')
		dwCookie &= COOKIE_EXT_COMMENT;
	return dwCookie;
}
