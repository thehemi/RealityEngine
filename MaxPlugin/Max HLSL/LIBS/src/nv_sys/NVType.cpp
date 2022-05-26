#include "stdafx.h"
#include "nv_sys.h"
#include <sstream>

using namespace std;
using namespace nv_renderdevice;

namespace nv_sys
{

#define TYPEIDTOSTRING(a)		\
	case a:						\
		return #a;		\
		break;

const char* NVType::TypeIDToString(eNVTYPEID TypeID)
{
	switch(TypeID)
	{
		TYPEIDTOSTRING(NVTYPEID_VEC4)
		TYPEIDTOSTRING(NVTYPEID_VEC3)
		TYPEIDTOSTRING(NVTYPEID_VEC2)
		TYPEIDTOSTRING(NVTYPEID_FLOAT)
		TYPEIDTOSTRING(NVTYPEID_BOOL)
		TYPEIDTOSTRING(NVTYPEID_MATRIX)
		TYPEIDTOSTRING(NVTYPEID_TEXTURE)
		TYPEIDTOSTRING(NVTYPEID_TEXTURE2D)
		TYPEIDTOSTRING(NVTYPEID_TEXTURE3D)
		TYPEIDTOSTRING(NVTYPEID_TEXTURECUBE)
		TYPEIDTOSTRING(NVTYPEID_SAMPLER)
		TYPEIDTOSTRING(NVTYPEID_SAMPLER2D)
		TYPEIDTOSTRING(NVTYPEID_SAMPLER3D)
		TYPEIDTOSTRING(NVTYPEID_SAMPLERCUBE)
		TYPEIDTOSTRING(NVTYPEID_DWORD)
		TYPEIDTOSTRING(NVTYPEID_INT)
		TYPEIDTOSTRING(NVTYPEID_STRING)
		TYPEIDTOSTRING(NVTYPEID_INTERFACE)
		TYPEIDTOSTRING(NVTYPEID_UNKNOWN)
		TYPEIDTOSTRING(NVTYPEID_VOID)
		TYPEIDTOSTRING(NVTYPEID_VERTEXSHADER)
		TYPEIDTOSTRING(NVTYPEID_PIXELSHADER)
		TYPEIDTOSTRING(NVTYPEID_PIXELFRAGMENT)
		TYPEIDTOSTRING(NVTYPEID_VERTEXFRAGMENT)
		default:
		{
			assert(0);			
		}
		break;
	}
	return "";
}

// Public constructor
NVType::NVType()
: m_Type(NVTYPEID_UNKNOWN),
m_Rows(0),
m_Columns(0),
m_pObjectSemantics(NULL)
{
}

// Public copying
NVType::NVType(const NVType& rhs)
: m_Type(NVTYPEID_UNKNOWN),
m_Rows(0),
m_Columns(0),
m_pObjectSemantics(NULL)
{
	Alloc(rhs.m_Type, rhs.m_Rows, rhs.m_Columns);
	operator = (rhs);
}

static GetTypeAndValue(const NVType& theValue, ostringstream& strStreamType, ostringstream& strStreamValue)
{
	switch(theValue.GetType())
	{
		case NVTYPEID_VEC4:
		{
			strStreamType << "float4";
			strStreamValue << theValue.GetVec4().x << ", " << theValue.GetVec4().y << ", " << theValue.GetVec4().z << ", " << theValue.GetVec4().w;
		}
		break;
		case NVTYPEID_VEC3:
		{
			strStreamType << "float3";
			strStreamValue << theValue.GetVec3().x << ", " << theValue.GetVec3().y << ", " << theValue.GetVec3().z;
		}
		break;
		case NVTYPEID_VEC2:
		{
			strStreamType << "float2";
			strStreamValue << theValue.GetVec2().x << ", " << theValue.GetVec2().y;
		}
		break;
		case NVTYPEID_FLOAT:
		{
			strStreamType << "float";
			strStreamValue << theValue.GetFloat();
		}
		break;
		case NVTYPEID_BOOL:
		{
			strStreamType << "bool";
			strStreamValue << theValue.GetBool();
		}
		break;
		case NVTYPEID_MATRIX:
		{
			strStreamType << "float" << theValue.GetRows() << "x" << theValue.GetColumns();
			strStreamValue << "(MATRIX)";
		}
		break;
		case NVTYPEID_TEXTURE:
		{
			strStreamType << "texture";
			strStreamValue << theValue.GetTexture();
		}
		break;
		case NVTYPEID_TEXTURE2D:
		{
			strStreamType << "texture2d";
			strStreamValue << theValue.GetTexture2D();
		}
		break;
		case NVTYPEID_TEXTURE3D:
		{
			strStreamType << "texture3d";
			strStreamValue << theValue.GetTexture3D();
		}
		break;
		case NVTYPEID_TEXTURECUBE:
		{
			strStreamType << "texturecube";
			strStreamValue << theValue.GetTextureCube();
		}
		break;
		case NVTYPEID_VERTEXSHADER:
		{
			strStreamType << "vertexshader";
			strStreamValue << theValue.GetVertexShader();
		}
		break;
		case NVTYPEID_PIXELSHADER:
		{
			strStreamType << "pixelshader";
			strStreamValue << theValue.GetPixelShader();
		}
		break;
		case NVTYPEID_PIXELFRAGMENT:
		{
			strStreamType << "pixelfragment";
			strStreamValue << theValue.GetPixelFragment();
		}
		break;
		case NVTYPEID_VERTEXFRAGMENT:
		{
			strStreamType << "vertexfragment";
			strStreamValue << theValue.GetVertexFragment();
		}
		break;
		case NVTYPEID_SAMPLER:
		{
			strStreamType << "sampler";
			strStreamValue << theValue.GetSampler();
		}
		break;
		case NVTYPEID_SAMPLER2D:
		{
			strStreamType << "sampler2d";
			strStreamValue << theValue.GetSampler2D();
		}
		break;
		case NVTYPEID_SAMPLER3D:
		{
			strStreamType << "sampler3d";
			strStreamValue << theValue.GetSampler3D();
		}
		break;
		case NVTYPEID_SAMPLERCUBE:
		{
			strStreamType << "samplercube";
			strStreamValue << theValue.GetSamplerCube();
		}
		break;
		case NVTYPEID_DWORD:
		{
			strStreamType << "dword";
			strStreamValue << theValue.GetDWORD();
		}
		break;
		case NVTYPEID_INT:
		{
			strStreamType << "int";
			strStreamValue << theValue.GetInt();
		}
		break;
		case NVTYPEID_STRING:
		{
			strStreamType << "string";
			strStreamValue << theValue.GetString();
		}
		break;
		case NVTYPEID_UNKNOWN:
			strStreamType << "UNKNOWN";
			strStreamValue << "UNKNOWN";
		break;
		case NVTYPEID_VOID:
			strStreamType << "void";
			strStreamValue << theValue.GetVoid();
		break;
		default:
			assert(0);
			break;
	}
}


void NVType::Dump() const
{
	ostringstream strStream;
	ostringstream strStreamValue;
	ostringstream strStreamType;

	GetTypeAndValue(*this, strStreamType, strStreamValue);

	strStream << strStreamType.str() << " ";
	if (m_pObjectSemantics)
	{
		strStream << m_pObjectSemantics->GetName() << " : " << ConvertSemantic(m_pObjectSemantics->GetSemanticID()) << endl;
		strStream << "<" << endl;
		
		for (unsigned int i = 0; i < m_pObjectSemantics->GetNumAnnotations(); i++)
		{
			const tAnnotationInfo* pAnnotation = m_pObjectSemantics->GetAnnotation(i);
			if (pAnnotation)
			{
				strStream << "    ";

				if ((pAnnotation->m_NameID != ANNOTATIONNAMEID_UNKNOWN) &&
					(pAnnotation->m_ValueID != ANNOTATIONVALUEID_UNKNOWN))
				{
					strStream << ConvertAnnotationName(pAnnotation->m_NameID) << " = " << ConvertAnnotationValue(pAnnotation->m_ValueID) << endl;
				}
				else 
				{
					ostringstream strValueType;
					ostringstream strValueValue;
					ostringstream strNameType;
					ostringstream strNameValue;

					GetTypeAndValue(pAnnotation->m_Value, strValueType, strValueValue);
					GetTypeAndValue(pAnnotation->m_Name, strNameType, strNameValue);

					strStream << strValueType.str() << " " << strNameValue.str() << " = " << strValueValue.str() << endl;
				}
			}
		}
		strStream << "> = { " << strStreamValue.str() << " }; " << endl;
	}
	else
	{
		strStream << strStreamType << " = { " << strStreamValue.str() << " }; " << endl;
	}

	GetLOGInterface()->DebugOut(strStream.str().c_str());

}

// Creation
NVType NVType::CreateVec4Type(const vec4& rhs)
{
	NVType NewType;
	NewType.Alloc(NVTYPEID_VEC4);
	NewType.SetVec4(rhs);
	return NewType;
}

NVType NVType::CreateVec3Type(const vec3& rhs)
{
	NVType NewType;
	NewType.Alloc(NVTYPEID_VEC3);
	NewType.SetVec3(rhs);
	return NewType;
}

NVType NVType::CreateVec2Type(const vec2& rhs)
{
	NVType NewType;
	NewType.Alloc(NVTYPEID_VEC2);
	NewType.SetVec2(rhs);
	return NewType;
}

NVType NVType::CreateFloatType(const float& rhs)
{
	NVType NewType;
	NewType.Alloc(NVTYPEID_FLOAT);
	NewType.SetFloat(rhs);
	return NewType;
}

NVType NVType::CreateDWORDType(const unsigned long& rhs)
{
	NVType NewType;
	NewType.Alloc(NVTYPEID_DWORD);
	NewType.SetDWORD(rhs);
	return NewType;
}

NVType NVType::CreateStringType(const char* rhs)
{
	NVType NewType;
	NewType.Alloc(NVTYPEID_STRING);
	NewType.SetString(rhs);
	return NewType;
}

NVType NVType::CreateBoolType(const bool& rhs)
{
	NVType NewType;
	NewType.Alloc(NVTYPEID_BOOL);
	NewType.SetBool(rhs);
	return NewType;
}
	
NVType NVType::CreateIntType(const int&rhs)
{
	NVType NewType;
	NewType.Alloc(NVTYPEID_INT);
	NewType.SetInt(rhs);
	return NewType;
}

NVType NVType::CreateInterfaceType(INVObject* pObj)
{
	NVType NewType;
	NewType.Alloc(NVTYPEID_INTERFACE);
	NewType.SetInterface(pObj);
	return NewType;
}

NVType NVType::CreateTextureType(INVTexture* pTexture)
{
	NVType NewType;
	NewType.Alloc(NVTYPEID_TEXTURE);
	NewType.SetTexture(pTexture);
	return NewType;
}
NVType NVType::CreateTexture2DType(INVTexture* pTexture)
{
	NVType NewType;
	NewType.Alloc(NVTYPEID_TEXTURE2D);
	NewType.SetTexture(pTexture);
	return NewType;
}
NVType NVType::CreateTexture3DType(INVTexture* pTexture)
{
	NVType NewType;
	NewType.Alloc(NVTYPEID_TEXTURE3D);
	NewType.SetTexture(pTexture);
	return NewType;
}
NVType NVType::CreateTextureCubeType(INVTexture* pTexture)
{
	NVType NewType;
	NewType.Alloc(NVTYPEID_TEXTURECUBE);
	NewType.SetTexture(pTexture);
	return NewType;
}

NVType NVType::CreateVertexShaderType(const unsigned long& dwShader)
{
	NVType NewType;
	NewType.Alloc(NVTYPEID_VERTEXSHADER);
	NewType.SetVertexShader(dwShader);
	return NewType;
}
NVType NVType::CreatePixelShaderType(const unsigned long& dwShader)
{
	NVType NewType;
	NewType.Alloc(NVTYPEID_PIXELSHADER);
	NewType.SetPixelShader(dwShader);
	return NewType;
}
NVType NVType::CreatePixelFragmentType(const unsigned long& dwFragment)
{
	NVType NewType;
	NewType.Alloc(NVTYPEID_PIXELFRAGMENT);
	NewType.SetPixelShader(dwFragment);
	return NewType;
}
NVType NVType::CreateVertexFragmentType(const unsigned long& dwFragment)
{
	NVType NewType;
	NewType.Alloc(NVTYPEID_VERTEXFRAGMENT);
	NewType.SetPixelShader(dwFragment);
	return NewType;
}
NVType NVType::CreateSamplerType(const unsigned long& dwSampler)
{
	NVType NewType;
	NewType.Alloc(NVTYPEID_SAMPLER);
	NewType.SetSampler(dwSampler);
	return NewType;
}
NVType NVType::CreateSampler2DType(const unsigned long& dwSampler)
{
	NVType NewType;
	NewType.Alloc(NVTYPEID_SAMPLER2D);
	NewType.SetSampler(dwSampler);
	return NewType;
}
NVType NVType::CreateSampler3DType(const unsigned long& dwSampler)
{
	NVType NewType;
	NewType.Alloc(NVTYPEID_SAMPLER3D);
	NewType.SetSampler(dwSampler);
	return NewType;
}
NVType NVType::CreateSamplerCubeType(const unsigned long& dwSampler)
{
	NVType NewType;
	NewType.Alloc(NVTYPEID_SAMPLERCUBE);
	NewType.SetSampler(dwSampler);
	return NewType;
}

NVType NVType::CreateMatrixType(const float* pValue, unsigned int Rows, unsigned int Columns)
{
	NVType NewType;
	NewType.Alloc(NVTYPEID_MATRIX, Rows, Columns);
	NewType.SetMatrix(pValue, Rows, Columns);
	return NewType;
}

NVType NVType::CreateVoidType()
{
	NVType NewType;
	NewType.Alloc(NVTYPEID_VOID);
	return NewType;
}

NVType NVType::CreateNULLType()
{
	NVType NewType;
	NewType.Alloc(NVTYPEID_UNKNOWN);
	return NewType;
}


// Destructor
NVType::~NVType()
{
	if (m_pObjectSemantics)
		m_pObjectSemantics->Release();
	Free();
}

bool NVType::SetObjectSemantics(INVObjectSemantics* pSemantics)
{
	if (m_pObjectSemantics)
	{
		m_pObjectSemantics->Release();
	}

	m_pObjectSemantics = pSemantics;
	
	if (pSemantics)
		pSemantics->AddRef();

	return true;
}

INVObjectSemantics* NVType::GetObjectSemantics() const
{
	return m_pObjectSemantics;
}


// Allocation for type data
void NVType::Alloc(const NVType& rhs)
{
	Alloc(rhs.m_Type, rhs.m_Rows, rhs.m_Columns);
}

void NVType::Alloc(eNVTYPEID Type, unsigned int Rows, unsigned int Columns)
{
	if (m_Type == Type)
	{
		if (m_Rows == Rows && m_Columns == Columns)
			return;
	}
	else
	{
		// It's an exception to copy to a type from another type, unless this type is UNKNOWN
		if (m_Type != NVTYPEID_UNKNOWN)
		{
			GetSYSInterface()->Exception(NVException(NVEXCEPTIONID_TYPEMISMATCH, NVType::TypeIDToString(m_Type)));
			return;
		}


		Free();
	}

	m_Type = Type;
	m_Rows = Rows;
	m_Columns = Columns;

	switch(Type)
	{
		case NVTYPEID_UNKNOWN:
			return;

		case NVTYPEID_VEC4:
		{
			m_pValue = new vec4;
		}
		break;
		case NVTYPEID_VEC3:
		{
			m_pValue = new vec3;
		}
		break;
		case NVTYPEID_VEC2:
		{
			m_pValue = new vec2;
		}
		break;
		case NVTYPEID_MATRIX:
		{
			m_pFloatArray = (float*)malloc(m_Rows * m_Columns * sizeof(float));
		}
		break;
		case NVTYPEID_STRING:
		{
			m_pValue = new std::string;
		}
		default:
			break;
	}
}

void NVType::Free()
{
	switch(m_Type)
	{
		case NVTYPEID_TEXTURE:
		case NVTYPEID_TEXTURE2D:
		case NVTYPEID_TEXTURE3D:
		case NVTYPEID_TEXTURECUBE:
		{
			SAFE_RELEASE(m_pTexture);
		}
		break;

		case NVTYPEID_MATRIX:
		{
			if (m_pFloatArray)
			{
				free(m_pFloatArray);
			}
		}
		break;

		case NVTYPEID_STRING:
		{
			if (m_pValue)
			{
				std::string* pString = reinterpret_cast<std::string*>(m_pValue);
				delete pString;
			}
		}
		break;
		case NVTYPEID_VEC2:
		{
			vec2* pVec2 = reinterpret_cast<vec2*>(m_pValue);
			delete pVec2;
		}
		break;
		case NVTYPEID_VEC3:
		{
			vec3* pVec3 = reinterpret_cast<vec3*>(m_pValue);
			delete pVec3;
		}
		break;
		case NVTYPEID_VEC4:
		{
			vec4* pVec4 = reinterpret_cast<vec4*>(m_pValue);
			delete pVec4;
		}
		break;
		case NVTYPEID_INTERFACE:
		{
			if (m_pInterface)
				m_pInterface->Release();
			m_pInterface = NULL;
		}
		break;
	}
	m_pValue = NULL;
	m_Type = NVTYPEID_UNKNOWN;
}

unsigned int NVType::GetRows() const
{
	return m_Rows;
}

unsigned int NVType::GetColumns() const
{
	return m_Columns;
}

bool NVType::CheckTypeRead(eNVTYPEID Type, unsigned int Rows, unsigned int Columns) const
{
	if ((Type == NVTYPEID_TEXTURE) && 
		((m_Type == NVTYPEID_TEXTURE2D) ||
		 (m_Type == NVTYPEID_TEXTURE3D) ||
		 (m_Type == NVTYPEID_TEXTURECUBE)))
		 return true;

	if ((Type == NVTYPEID_SAMPLER) && 
		((m_Type == NVTYPEID_SAMPLER2D) ||
		 (m_Type == NVTYPEID_SAMPLER3D) ||
		 (m_Type == NVTYPEID_SAMPLERCUBE)))
		 return true;

	if (m_Type != Type)
	{
		ostringstream strStream;
		strStream << "types not the same : " << NVType::TypeIDToString(Type) << " , " << NVType::TypeIDToString(m_Type);
		GetSYSInterface()->Exception(NVException(NVEXCEPTIONID_TYPEMISMATCH, strStream.str().c_str()));
		return false;
	}

	if ((m_Type != NVTYPEID_STRING) && (m_Rows != Rows || m_Columns != Columns))
	{
		ostringstream strStream;
		strStream << "different dimensions : " << NVType::TypeIDToString(Type) << " , " << NVType::TypeIDToString(m_Type);
		GetSYSInterface()->Exception(NVException(NVEXCEPTIONID_TYPEMISMATCH, strStream.str().c_str()));
		return false;
	}

	return true;
}

bool NVType::CheckTypeWrite(eNVTYPEID Type, unsigned int Rows, unsigned int Columns)
{
	if (m_Type == NVTYPEID_UNKNOWN)
	{
		Alloc(Type, Rows, Columns);
		return true;
	}

	if ((Type == NVTYPEID_TEXTURE) && 
		((m_Type == NVTYPEID_TEXTURE2D) ||
		 (m_Type == NVTYPEID_TEXTURE3D) ||
		 (m_Type == NVTYPEID_TEXTURECUBE)))
		 return true;

	if ((Type == NVTYPEID_SAMPLER) && 
		((m_Type == NVTYPEID_SAMPLER2D) ||
		 (m_Type == NVTYPEID_SAMPLER3D) ||
		 (m_Type == NVTYPEID_SAMPLERCUBE)))
		 return true;

	if (m_Type != Type)
	{
		ostringstream strStream;
		strStream << "types not the same : " << NVType::TypeIDToString(Type) << " , " << NVType::TypeIDToString(m_Type);
		GetSYSInterface()->Exception(NVException(NVEXCEPTIONID_TYPEMISMATCH, strStream.str().c_str()));
		return false;
	}

	if ((m_Type != NVTYPEID_STRING) && (m_Rows != Rows || m_Columns != Columns))
	{
		ostringstream strStream;
		strStream << "different dimensions : " << NVType::TypeIDToString(Type) << " , " << NVType::TypeIDToString(m_Type);
		GetSYSInterface()->Exception(NVException(NVEXCEPTIONID_TYPEMISMATCH, strStream.str().c_str()));
		return false;
	}

	return true;
}

const NVType& NVType::Assign(const NVType& rhs)
{
	*this = rhs;
	return *this;
}

eNVTYPEID NVType::GetType() const { return m_Type; }

void NVType::SetDWORD(const unsigned long& rhs) { CheckTypeWrite(NVTYPEID_DWORD); m_dwValue = rhs; };
void NVType::SetVertexShader(const unsigned long& rhs) { CheckTypeWrite(NVTYPEID_VERTEXSHADER); m_dwValue = rhs; };
void NVType::SetPixelShader(const unsigned long& rhs) { CheckTypeWrite(NVTYPEID_PIXELSHADER); m_dwValue = rhs; };
void NVType::SetPixelFragment(const unsigned long& rhs) { CheckTypeWrite(NVTYPEID_PIXELFRAGMENT); m_dwValue = rhs; };
void NVType::SetVertexFragment(const unsigned long& rhs) { CheckTypeWrite(NVTYPEID_VERTEXFRAGMENT); m_dwValue = rhs; };
void NVType::SetSampler(const unsigned long& rhs) { CheckTypeWrite(NVTYPEID_SAMPLER); m_dwValue = rhs; };
void NVType::SetSampler2D(const unsigned long& rhs) { CheckTypeWrite(NVTYPEID_SAMPLER2D); m_dwValue = rhs; };
void NVType::SetSampler3D(const unsigned long& rhs) { CheckTypeWrite(NVTYPEID_SAMPLER3D); m_dwValue = rhs; };
void NVType::SetSamplerCube(const unsigned long& rhs) { CheckTypeWrite(NVTYPEID_SAMPLERCUBE); m_dwValue = rhs; };
void NVType::SetVec4(const vec4& rhs) { CheckTypeWrite(NVTYPEID_VEC4); *reinterpret_cast<vec4*>(m_pValue) = rhs; };
void NVType::SetVec3(const vec3& rhs) { CheckTypeWrite(NVTYPEID_VEC3); *reinterpret_cast<vec3*>(m_pValue) = rhs; };
void NVType::SetVec2(const vec2& rhs) { CheckTypeWrite(NVTYPEID_VEC2); *reinterpret_cast<vec2*>(m_pValue) = rhs;};
void NVType::SetFloat(const float& rhs) { CheckTypeWrite(NVTYPEID_FLOAT); m_fValue = rhs; };
void NVType::SetBool(const bool& rhs) { CheckTypeWrite(NVTYPEID_BOOL); m_bValue = rhs; };
void NVType::SetMatrix(const float* pMatrix, int Rows, int Columns) { assert(pMatrix); CheckTypeWrite(NVTYPEID_MATRIX, Rows, Columns); memcpy(m_pFloatArray, pMatrix, sizeof(float) * Rows * Columns); };
void NVType::SetTexture(INVTexture* pTexture) { CheckTypeWrite(NVTYPEID_TEXTURE); if (m_pTexture != pTexture) SAFE_RELEASE(m_pTexture); m_pTexture = pTexture; SAFE_ADDREF(m_pTexture); }
void NVType::SetTexture2D(INVTexture* pTexture) { CheckTypeWrite(NVTYPEID_TEXTURE2D); if (m_pTexture != pTexture) SAFE_RELEASE(m_pTexture); m_pTexture = pTexture; SAFE_ADDREF(m_pTexture); }
void NVType::SetTexture3D(INVTexture* pTexture) { CheckTypeWrite(NVTYPEID_TEXTURE3D); if (m_pTexture != pTexture) SAFE_RELEASE(m_pTexture); m_pTexture = pTexture; SAFE_ADDREF(m_pTexture); }
void NVType::SetTextureCube(INVTexture* pTexture) { CheckTypeWrite(NVTYPEID_TEXTURECUBE); if (m_pTexture != pTexture) SAFE_RELEASE(m_pTexture); m_pTexture = pTexture; SAFE_ADDREF(m_pTexture); }
void NVType::SetInt(const int& rhs) { CheckTypeWrite(NVTYPEID_INT); m_iValue = rhs; }
void NVType::SetString(const char* pszString)  { CheckTypeWrite(NVTYPEID_STRING, strlen(pszString)); *reinterpret_cast<std::string*>(m_pValue) = pszString; };
void NVType::SetInterface(INVObject* const prhs) { CheckTypeWrite(NVTYPEID_INTERFACE); m_pInterface = prhs; if (prhs) prhs->AddRef(); };
void NVType::SetNULL() { Free(); m_Type = NVTYPEID_UNKNOWN; };
void NVType::SetVoid(void* pVoid) { CheckTypeWrite(NVTYPEID_VOID); m_pValue = pVoid;};

const unsigned long& NVType::GetDWORD() const { CheckTypeRead(NVTYPEID_DWORD); return m_dwValue; };
const unsigned long& NVType::GetVertexShader() const { CheckTypeRead(NVTYPEID_VERTEXSHADER); return m_dwValue; };
const unsigned long& NVType::GetPixelShader() const { CheckTypeRead(NVTYPEID_PIXELSHADER); return m_dwValue; };
const unsigned long& NVType::GetVertexFragment() const { CheckTypeRead(NVTYPEID_VERTEXFRAGMENT); return m_dwValue; };
const unsigned long& NVType::GetPixelFragment() const { CheckTypeRead(NVTYPEID_PIXELFRAGMENT); return m_dwValue; };
const unsigned long& NVType::GetSampler() const { CheckTypeRead(NVTYPEID_SAMPLER); return m_dwValue; };
const unsigned long& NVType::GetSampler2D() const { CheckTypeRead(NVTYPEID_SAMPLER2D); return m_dwValue; };
const unsigned long& NVType::GetSampler3D() const { CheckTypeRead(NVTYPEID_SAMPLER3D); return m_dwValue; };
const unsigned long& NVType::GetSamplerCube() const { CheckTypeRead(NVTYPEID_SAMPLERCUBE); return m_dwValue; };
const vec4& NVType::GetVec4() const { CheckTypeRead(NVTYPEID_VEC4); return *reinterpret_cast<vec4*>(m_pValue); };
const vec3& NVType::GetVec3() const { CheckTypeRead(NVTYPEID_VEC3); return *reinterpret_cast<vec3*>(m_pValue); };
const vec2& NVType::GetVec2() const { CheckTypeRead(NVTYPEID_VEC2); return *reinterpret_cast<vec2*>(m_pValue); };
const float& NVType::GetFloat() const { CheckTypeRead(NVTYPEID_FLOAT); return m_fValue; };
const bool& NVType::GetBool() const { CheckTypeRead(NVTYPEID_BOOL); return m_bValue; };
const float* NVType::GetMatrix() const { CheckTypeRead(NVTYPEID_MATRIX, m_Rows, m_Columns); return m_pFloatArray; };
INVTexture* NVType::GetTexture() const { CheckTypeRead(NVTYPEID_TEXTURE); return m_pTexture; }
INVTexture* NVType::GetTexture2D() const { CheckTypeRead(NVTYPEID_TEXTURE2D); return m_pTexture; }
INVTexture* NVType::GetTexture3D() const { CheckTypeRead(NVTYPEID_TEXTURE3D); return m_pTexture; }
INVTexture* NVType::GetTextureCube() const { CheckTypeRead(NVTYPEID_TEXTURECUBE); return m_pTexture; }
const int& NVType::GetInt() const { CheckTypeRead(NVTYPEID_INT); return m_iValue; }
const void* NVType::GetVoid() const { CheckTypeRead(NVTYPEID_VOID); return m_pValue; }
const char* NVType::GetString() const { CheckTypeRead(NVTYPEID_STRING); return reinterpret_cast<std::string*>(m_pValue)->c_str(); };
INVObject* NVType::GetInterface() { CheckTypeRead(NVTYPEID_INTERFACE); if (m_pInterface) m_pInterface->AddRef(); return m_pInterface; };

const void* NVType::GetValue(unsigned long& Bytes) const
{
	switch(m_Type)
	{
		case NVTYPEID_VEC4:
		{
			Bytes = sizeof(vec4);
			return reinterpret_cast<vec4*>(m_pValue)->vec_array;

		}
		break;
		case NVTYPEID_VEC3:
		{
			Bytes = sizeof(vec3);
			return reinterpret_cast<vec3*>(m_pValue)->vec_array;
		}
		break;
		case NVTYPEID_VEC2:
		{
			Bytes = sizeof(vec2);
			return reinterpret_cast<vec2*>(m_pValue)->vec_array;
		}
		break;
		case NVTYPEID_FLOAT:
		{
			Bytes = sizeof(float);
			return &m_fValue;
		}
		break;
		case NVTYPEID_BOOL:
		{
			Bytes = sizeof(bool);
			return &m_bValue;
		}
		break;
		case NVTYPEID_MATRIX:
		{
			Bytes = m_Rows * m_Columns * sizeof(float);
			return m_pFloatArray;
		}
		break;
		case NVTYPEID_TEXTURE:
		case NVTYPEID_TEXTURE2D:
		case NVTYPEID_TEXTURE3D:
		case NVTYPEID_TEXTURECUBE:
		{
			// Get the value of the pointer is hardly useful,
			// but it's returned just in case.
			Bytes = sizeof(INVTexture*);
			return &m_pTexture;
		}
		break;
		case NVTYPEID_VERTEXSHADER:
		case NVTYPEID_PIXELSHADER:
		case NVTYPEID_PIXELFRAGMENT:
		case NVTYPEID_VERTEXFRAGMENT:
		case NVTYPEID_SAMPLER:
		case NVTYPEID_SAMPLER2D:
		case NVTYPEID_SAMPLER3D:
		case NVTYPEID_SAMPLERCUBE:
		case NVTYPEID_DWORD:
		{
			Bytes = sizeof(unsigned long);
			return &m_dwValue;
		}
		break;
		case NVTYPEID_INT:
		{
			Bytes = sizeof(int);
			return &m_iValue;
		}
		break;
		case NVTYPEID_STRING:
		{
			std::string* pString = reinterpret_cast<std::string*>(m_pValue);
			Bytes = pString->size() + 1;
			return pString->c_str();
		}
		break;
		case NVTYPEID_UNKNOWN:
		{
			Bytes = 0;
			return NULL;
		}
		break;
		case NVTYPEID_VOID:
		{
			Bytes = sizeof(m_pValue);
			return m_pValue;
		}
		break;

		default:
			assert(0);
			break;
	}
	return NULL;
}


const NVType& NVType::operator = (const NVType& rhs)
{
	// Copy the semantics if we don't have any.
	if (!m_pObjectSemantics && rhs.m_pObjectSemantics)
	{
		INVClone* pClone = NULL;
			
		if (rhs.m_pObjectSemantics->GetInterface(IID_INVClone, (void**)&pClone))
		{
			pClone->Clone(IID_INVObjectSemantics, (void**)&m_pObjectSemantics);
		}
		SAFE_RELEASE(pClone);
	}

	// Allocate the value if necessary, or check that we are correct.
	// Note that it's allowed to convert this type from UNKNOWN to a type using assignment
	CheckTypeWrite(rhs.m_Type, rhs.m_Rows, rhs.m_Columns);


	switch(m_Type)
	{
		case NVTYPEID_VEC4:
		{
			vec4* pVec4 = reinterpret_cast<vec4*>(m_pValue);
			switch(rhs.m_Type)
			{
				case NVTYPEID_VEC4:
				{
					*pVec4 = *reinterpret_cast<vec4*>(rhs.m_pValue);
					return *this;
				}
				break;
				case NVTYPEID_VEC3:
				{
					vec3* pVec3 = reinterpret_cast<vec3*>(rhs.m_pValue);
					pVec4->x = pVec3->x;
					pVec4->y = pVec3->y;
					pVec4->z = pVec3->z;
					pVec4->w = 1.0f;
					return *this;
				}
				break;
				case NVTYPEID_VEC2:
				{
					vec2* pVec2 = reinterpret_cast<vec2*>(rhs.m_pValue);
					pVec4->x = pVec2->x;
					pVec4->y = pVec2->y;
					pVec4->z = 0.0f;
					pVec4->w = 1.0f;
					return *this;
				}
				break;
				case NVTYPEID_FLOAT:
				{
					pVec4->x = rhs.m_fValue;
					pVec4->y = rhs.m_fValue;
					pVec4->z = rhs.m_fValue;
					pVec4->w = rhs.m_fValue;
					return *this;
				}
				break;
				case NVTYPEID_DWORD:
				{
					pVec4->x = rhs.m_dwValue;
					pVec4->y = rhs.m_dwValue;
					pVec4->z = rhs.m_dwValue;
					pVec4->w = rhs.m_dwValue;
					return *this;
				}
				break;
				case NVTYPEID_INT:
				{
					pVec4->x = rhs.m_iValue;
					pVec4->y = rhs.m_iValue;
					pVec4->z = rhs.m_iValue;
					pVec4->w = rhs.m_iValue;
					return *this;
				}
				break;
				case NVTYPEID_BOOL:
				{
					pVec4->x = rhs.m_bValue ? 1.0f : 0.0f;
					pVec4->y = rhs.m_bValue ? 1.0f : 0.0f;
					pVec4->z = rhs.m_bValue ? 1.0f : 0.0f;
					pVec4->w = rhs.m_bValue ? 1.0f : 0.0f;
					return *this;
				}
				break;
				case NVTYPEID_STRING:
				{
					// TODO
				}
				break;

				default:
					break;
			}
		}
		break;
		case NVTYPEID_VEC3:
		{
			vec3* pVec3 = reinterpret_cast<vec3*>(m_pValue);
			switch(rhs.m_Type)
			{
				case NVTYPEID_VEC4:
				{
					vec4* pVec4 = reinterpret_cast<vec4*>(rhs.m_pValue);
					pVec3->x = pVec4->x;
					pVec3->y = pVec4->y;
					pVec3->z = pVec4->z;
					return *this;
				}
				break;
				case NVTYPEID_VEC3:
				{
					*pVec3 = *reinterpret_cast<vec3*>(rhs.m_pValue);
					return *this;
				}
				break;
				case NVTYPEID_VEC2:
				{
					vec2* pVec2 = reinterpret_cast<vec2*>(rhs.m_pValue);
					pVec3->x = pVec2->x;
					pVec3->y = pVec2->y;
					pVec3->z = 0.0f;
					return *this;
				}
				break;
				case NVTYPEID_FLOAT:
				{
					pVec3->x = rhs.m_fValue;
					pVec3->y = rhs.m_fValue;
					pVec3->z = rhs.m_fValue;
					return *this;
				}
				break;
				case NVTYPEID_DWORD:
				{
					pVec3->x = rhs.m_dwValue;
					pVec3->y = rhs.m_dwValue;
					pVec3->z = rhs.m_dwValue;
					return *this;
				}
				break;
				case NVTYPEID_INT:
				{
					pVec3->x = rhs.m_iValue;
					pVec3->y = rhs.m_iValue;
					pVec3->z = rhs.m_iValue;
					return *this;
				}
				break;
				case NVTYPEID_BOOL:
				{
					pVec3->x = rhs.m_bValue ? 1.0f : 0.0f;
					pVec3->y = rhs.m_bValue ? 1.0f : 0.0f;
					pVec3->z = rhs.m_bValue ? 1.0f : 0.0f;
					return *this;
				}
				break;
				case NVTYPEID_STRING:
				{
					// TODO
				}
				break;

				default:
					break;
			}
		}
		break;
		case NVTYPEID_VEC2:
		{
			vec2* pVec2 = reinterpret_cast<vec2*>(m_pValue);
			switch(rhs.m_Type)
			{
				case NVTYPEID_VEC4:
				{
					vec4* pVec4 = reinterpret_cast<vec4*>(rhs.m_pValue);
					pVec2->x = pVec4->x;
					pVec2->y = pVec4->y;
					return *this;
				}
				break;
				case NVTYPEID_VEC3:
				{
					vec3* pVec3 = reinterpret_cast<vec3*>(rhs.m_pValue);
					pVec2->x = pVec2->x;
					pVec2->y = pVec2->y;
					return *this;
				}
				break;
				case NVTYPEID_VEC2:
				{
					*pVec2 = *reinterpret_cast<vec2*>(rhs.m_pValue);
					return *this;
				}
				break;
				case NVTYPEID_FLOAT:
				{
					pVec2->x = rhs.m_fValue;
					pVec2->y = rhs.m_fValue;
					return *this;
				}
				break;
				case NVTYPEID_DWORD:
				{
					pVec2->x = rhs.m_dwValue;
					pVec2->y = rhs.m_dwValue;
					return *this;
				}
				break;
				case NVTYPEID_INT:
				{
					pVec2->x = rhs.m_iValue;
					pVec2->y = rhs.m_iValue;
					return *this;
				}
				break;
				case NVTYPEID_BOOL:
				{
					pVec2->x = rhs.m_bValue ? 1.0f : 0.0f;
					pVec2->y = rhs.m_bValue ? 1.0f : 0.0f;
					return *this;
				}
				break;
				case NVTYPEID_STRING:
				{
					// TODO
				}
				break;
				default:
					break;
			}
		}
		break;
		case NVTYPEID_FLOAT:
		{
			switch(rhs.m_Type)
			{
				case NVTYPEID_VEC4:
				{
					vec4* pVec4 = reinterpret_cast<vec4*>(rhs.m_pValue);
					m_fValue = pVec4->x;
					return *this;
				}
				break;
				case NVTYPEID_VEC3:
				{
					vec3* pVec3 = reinterpret_cast<vec3*>(rhs.m_pValue);
					m_fValue = pVec3->x;
					return *this;
				}
				break;
				case NVTYPEID_VEC2:
				{
					vec2* pVec2 = reinterpret_cast<vec2*>(rhs.m_pValue);
					m_fValue = pVec2->x;
					return *this;
				}
				break;
				case NVTYPEID_FLOAT:
				{
					m_fValue = rhs.m_fValue;
					return *this;
				}
				break;
				case NVTYPEID_DWORD:
				{
					m_fValue = rhs.m_dwValue;
					return *this;
				}
				break;
				case NVTYPEID_INT:
				{
					m_fValue = rhs.m_iValue;
					return *this;
				}
				break;
				case NVTYPEID_BOOL:
				{
					m_fValue = rhs.m_bValue ? 1.0f : 0.0f;
					return *this;
				}
				break;
				case NVTYPEID_STRING:
				{
					// TODO
				}
				break;

				default:
					break;
			}
		}
		break;
		case NVTYPEID_BOOL:
		{
			switch(rhs.m_Type)
			{
				case NVTYPEID_VEC4:
				{
					vec4* pVec4 = reinterpret_cast<vec4*>(rhs.m_pValue);
					m_bValue = (pVec4->x != 0.0f) ? true : false;
					return *this;
				}
				break;
				case NVTYPEID_VEC3:
				{
					vec3* pVec3 = reinterpret_cast<vec3*>(rhs.m_pValue);
					m_bValue = (pVec3->x != 0.0f) ? true : false;
					return *this;
				}
				break;
				case NVTYPEID_VEC2:
				{
					vec2* pVec2 = reinterpret_cast<vec2*>(rhs.m_pValue);
					m_bValue = (pVec2->x != 0.0f) ? true : false;
					return *this;
				}
				break;
				case NVTYPEID_FLOAT:
				{
					m_bValue = (rhs.m_fValue != 0.0f) ? true : false;
					return *this;
				}
				break;
				case NVTYPEID_DWORD:
				{
					m_bValue = (rhs.m_dwValue != 0) ? true : false;
					return *this;
				}
				break;
				case NVTYPEID_INT:
				{
					m_bValue = (rhs.m_iValue != 0) ? true : false;
					return *this;
				}
				break;
				case NVTYPEID_BOOL:
				{
					m_bValue = rhs.m_bValue;
					return *this;
				}
				break;
				case NVTYPEID_TEXTURE:
				{
					m_bValue = rhs.m_pTexture ? true : false;
					return *this;
				}
				break;
				case NVTYPEID_STRING:
				{
					// TODO
				}
				break;

				default:
					break;
			}
		}
		break;
		case NVTYPEID_MATRIX:
		{
			if (CheckTypeRead(rhs.m_Type, rhs.m_Rows, rhs.m_Columns))
			{
				SetMatrix(reinterpret_cast<float*>(rhs.m_pFloatArray), rhs.m_Rows, rhs.m_Columns);
				return *this;
			}
		}
		break;
		case NVTYPEID_TEXTURE:
		{
			if (CheckTypeRead(rhs.m_Type, rhs.m_Rows, rhs.m_Columns))
			{
				SetTexture(rhs.m_pTexture);
				return *this;
			}
		}
		break;
		case NVTYPEID_TEXTURE2D:
		{
			if (CheckTypeRead(rhs.m_Type, rhs.m_Rows, rhs.m_Columns))
			{
				SetTexture2D(rhs.m_pTexture);
				return *this;
			}
		}
		break;
		case NVTYPEID_TEXTURE3D:
		{
			if (CheckTypeRead(rhs.m_Type, rhs.m_Rows, rhs.m_Columns))
			{
				SetTexture3D(rhs.m_pTexture);
				return *this;
			}
		}
		break;
		case NVTYPEID_TEXTURECUBE:
		{
			if (CheckTypeRead(rhs.m_Type, rhs.m_Rows, rhs.m_Columns))
			{
				SetTextureCube(rhs.m_pTexture);
				return *this;
			}
		}
		break;
		case NVTYPEID_VERTEXSHADER:
		{
			if (CheckTypeRead(rhs.m_Type, rhs.m_Rows, rhs.m_Columns))
			{
				SetVertexShader(rhs.m_dwValue);
				return *this;
			}
		}
		break;
		case NVTYPEID_PIXELSHADER:
		{
			if (CheckTypeRead(rhs.m_Type, rhs.m_Rows, rhs.m_Columns))
			{
				SetPixelShader(rhs.m_dwValue);
				return *this;
			}
		}
		break;
		case NVTYPEID_PIXELFRAGMENT:
		{
			if (CheckTypeRead(rhs.m_Type, rhs.m_Rows, rhs.m_Columns))
			{
				SetPixelFragment(rhs.m_dwValue);
				return *this;
			}
		}
		break;
		case NVTYPEID_VERTEXFRAGMENT:
		{
			if (CheckTypeRead(rhs.m_Type, rhs.m_Rows, rhs.m_Columns))
			{
				SetVertexFragment(rhs.m_dwValue);
				return *this;
			}
		}
		break;
		case NVTYPEID_SAMPLER:
		{
			if (CheckTypeRead(rhs.m_Type, rhs.m_Rows, rhs.m_Columns))
			{
				SetSampler(rhs.m_dwValue);
				return *this;
			}
		}
		break;
		case NVTYPEID_SAMPLER2D:
		{
			if (CheckTypeRead(rhs.m_Type, rhs.m_Rows, rhs.m_Columns))
			{
				SetSampler2D(rhs.m_dwValue);
				return *this;
			}
		}
		break;
		case NVTYPEID_SAMPLER3D:
		{
			if (CheckTypeRead(rhs.m_Type, rhs.m_Rows, rhs.m_Columns))
			{
				SetSampler3D(rhs.m_dwValue);
				return *this;
			}
		}
		break;
		case NVTYPEID_SAMPLERCUBE:
		{
			if (CheckTypeRead(rhs.m_Type, rhs.m_Rows, rhs.m_Columns))
			{
				SetSamplerCube(rhs.m_dwValue);
				return *this;
			}
		}
		break;
		case NVTYPEID_DWORD:
		{
			switch(rhs.m_Type)
			{
				case NVTYPEID_VEC4:
				{
					vec4* pVec4 = reinterpret_cast<vec4*>(rhs.m_pValue);
					m_dwValue = pVec4->x;
					return *this;
				}
				break;
				case NVTYPEID_VEC3:
				{
					vec3* pVec3 = reinterpret_cast<vec3*>(rhs.m_pValue);
					m_dwValue = pVec3->x;
					return *this;
				}
				break;
				case NVTYPEID_VEC2:
				{
					vec2* pVec2 = reinterpret_cast<vec2*>(rhs.m_pValue);
					m_dwValue = pVec2->x;
					return *this;
				}
				break;
				case NVTYPEID_FLOAT:
				{
					m_dwValue = rhs.m_fValue;
					return *this;
				}
				break;
				case NVTYPEID_DWORD:
				{
					m_dwValue = rhs.m_dwValue;
					return *this;
				}
				break;
				case NVTYPEID_INT:
				{
					m_dwValue = rhs.m_iValue;
					return *this;
				}
				break;
				case NVTYPEID_BOOL:
				{
					m_dwValue = rhs.m_bValue ? 1 : 0;
					return *this;
				}
				break;
				case NVTYPEID_STRING:
				{
					// TODO
				}
				break;
				default:
					break;
			}
		}
		break;
		case NVTYPEID_INT:
		{
			switch(rhs.m_Type)
			{
				case NVTYPEID_VEC4:
				{
					vec4* pVec4 = reinterpret_cast<vec4*>(rhs.m_pValue);
					m_iValue = pVec4->x;
					return *this;
				}
				break;
				case NVTYPEID_VEC3:
				{
					vec3* pVec3 = reinterpret_cast<vec3*>(rhs.m_pValue);
					m_iValue = pVec3->x;
					return *this;
				}
				break;
				case NVTYPEID_VEC2:
				{
					vec2* pVec2 = reinterpret_cast<vec2*>(rhs.m_pValue);
					m_iValue = pVec2->x;
					return *this;
				}
				break;
				case NVTYPEID_FLOAT:
				{
					m_iValue = rhs.m_fValue;
					return *this;
				}
				break;
				case NVTYPEID_DWORD:
				{
					m_iValue = rhs.m_dwValue;
					return *this;
				}
				break;
				case NVTYPEID_INT:
				{
					m_iValue = rhs.m_iValue;
					return *this;
				}
				break;
				case NVTYPEID_BOOL:
				{
					m_iValue = rhs.m_bValue ? 1.0f : 0.0f;
					return *this;
				}
				break;
				case NVTYPEID_STRING:
				{
					// TODO
				}
				break;

				default:
					break;
			}
		}
		break;
		case NVTYPEID_STRING:
		{
			switch(rhs.m_Type)
			{
				case NVTYPEID_STRING:
				{
					*reinterpret_cast<std::string*>(m_pValue) = *reinterpret_cast<std::string*>(rhs.m_pValue);
					return *this;
				}
				break;
			}
		}
		break;
		case NVTYPEID_VOID:
		{
			switch(rhs.m_Type)
			{
				case NVTYPEID_VOID:
				{
					m_pValue = rhs.m_pValue;
					return *this;
				}
				break;
			}
		}
		break;
		case NVTYPEID_UNKNOWN:
		{
			// Valid to copy an unknown, it just doesn't do anything...
			return *this;
		}
		break;
		
		default:
			assert(0);
			break;
	}

	ostringstream strStream;
	strStream << NVType::TypeIDToString(m_Type) << " = " << NVType::TypeIDToString(rhs.m_Type);
	GetSYSInterface()->Exception(NVException(NVEXCEPTIONID_INVALIDOPERATION, strStream.str().c_str()));
	return *this;
}

const NVType& NVType::operator += (const NVType& rhs)
{
	switch(m_Type)
	{
		case NVTYPEID_VEC4:
		{
			vec4* pVec4 = reinterpret_cast<vec4*>(m_pValue);
			switch(rhs.m_Type)
			{
				case NVTYPEID_VEC4:
				{
					*pVec4 += *reinterpret_cast<vec4*>(rhs.m_pValue);
					return *this;
				}
				break;
				case NVTYPEID_VEC3:
				{
					vec3* pVec3 = reinterpret_cast<vec3*>(rhs.m_pValue);
					pVec4->x += pVec3->x;
					pVec4->y += pVec3->y;
					pVec4->z += pVec3->z;
					return *this;
				}
				break;
				case NVTYPEID_VEC2:
				{
					vec2* pVec2 = reinterpret_cast<vec2*>(rhs.m_pValue);
					pVec4->x += pVec2->x;
					pVec4->y += pVec2->y;
					return *this;
				}
				break;
				case NVTYPEID_FLOAT:
				{
					pVec4->x += rhs.m_fValue;
					pVec4->y += rhs.m_fValue;
					pVec4->z += rhs.m_fValue;
					pVec4->w += rhs.m_fValue;
					return *this;
				}
				break;
				case NVTYPEID_DWORD:
				{
					pVec4->x += rhs.m_dwValue;
					pVec4->y += rhs.m_dwValue;
					pVec4->z += rhs.m_dwValue;
					pVec4->w += rhs.m_dwValue;
					return *this;
				}
				break;
				case NVTYPEID_INT:
				{
					pVec4->x += rhs.m_iValue;
					pVec4->y += rhs.m_iValue;
					pVec4->z += rhs.m_iValue;
					pVec4->w += rhs.m_iValue;
					return *this;
				}
				break;
				//	case NVTYPEID_BOOL:
				// case NVTYPEID_STRING: TODO

				default:
					break;
			}
		}
		break;
		case NVTYPEID_VEC3:
		{
			vec3* pVec3 = reinterpret_cast<vec3*>(m_pValue);
			switch(rhs.m_Type)
			{
				case NVTYPEID_VEC4:
				{
					vec4* pVec4 = reinterpret_cast<vec4*>(rhs.m_pValue);
					pVec3->x += pVec4->x;
					pVec3->y += pVec4->y;
					pVec3->z += pVec4->z;
					return *this;
				}
				break;
				case NVTYPEID_VEC3:
				{
					*pVec3 += *reinterpret_cast<vec3*>(rhs.m_pValue);
					return *this;
				}
				break;
				case NVTYPEID_VEC2:
				{
					vec2* pVec2 = reinterpret_cast<vec2*>(rhs.m_pValue);
					pVec3->x += pVec2->x;
					pVec3->y += pVec2->y;
					return *this;
				}
				break;
				case NVTYPEID_FLOAT:
				{
					pVec3->x += rhs.m_fValue;
					pVec3->y += rhs.m_fValue;
					pVec3->z += rhs.m_fValue;
					return *this;
				}
				break;
				case NVTYPEID_DWORD:
				{
					pVec3->x += rhs.m_dwValue;
					pVec3->y += rhs.m_dwValue;
					pVec3->z += rhs.m_dwValue;
					return *this;
				}
				break;
				case NVTYPEID_INT:
				{
					pVec3->x += rhs.m_iValue;
					pVec3->y += rhs.m_iValue;
					pVec3->z += rhs.m_iValue;
					return *this;
				}
				break;
				//case NVTYPEID_BOOL:
				//case NVTYPEID_STRING: TODO
				default:
					break;
			}
		}
		break;
		case NVTYPEID_VEC2:
		{
			vec2* pVec2 = reinterpret_cast<vec2*>(m_pValue);
			switch(rhs.m_Type)
			{
				case NVTYPEID_VEC4:
				{
					vec4* pVec4 = reinterpret_cast<vec4*>(rhs.m_pValue);
					pVec2->x += pVec4->x;
					pVec2->y += pVec4->y;
					return *this;
				}
				break;
				case NVTYPEID_VEC3:
				{
					vec3* pVec3 = reinterpret_cast<vec3*>(rhs.m_pValue);
					pVec2->x += pVec2->x;
					pVec2->y += pVec2->y;
					return *this;
				}
				break;
				case NVTYPEID_VEC2:
				{
					*pVec2 = *reinterpret_cast<vec2*>(rhs.m_pValue);
					return *this;
				}
				break;
				case NVTYPEID_FLOAT:
				{
					pVec2->x += rhs.m_fValue;
					pVec2->y += rhs.m_fValue;
					return *this;
				}
				break;
				case NVTYPEID_DWORD:
				{
					pVec2->x += rhs.m_dwValue;
					pVec2->y += rhs.m_dwValue;
					return *this;
				}
				break;
				case NVTYPEID_INT:
				{
					pVec2->x += rhs.m_iValue;
					pVec2->y += rhs.m_iValue;
					return *this;
				}
				break;
				// case NVTYPEID_BOOL:
				// case NVTYPEID_STRING: TODO
				default:
					break;
			}
		}
		break;
		case NVTYPEID_FLOAT:
		{
			switch(rhs.m_Type)
			{
				case NVTYPEID_VEC4:
				{
					vec4* pVec4 = reinterpret_cast<vec4*>(rhs.m_pValue);
					m_fValue += pVec4->x;
					return *this;
				}
				break;
				case NVTYPEID_VEC3:
				{
					vec3* pVec3 = reinterpret_cast<vec3*>(rhs.m_pValue);
					m_fValue += pVec3->x;
					return *this;
				}
				break;
				case NVTYPEID_VEC2:
				{
					vec2* pVec2 = reinterpret_cast<vec2*>(rhs.m_pValue);
					m_fValue += pVec2->x;
					return *this;
				}
				break;
				case NVTYPEID_FLOAT:
				{
					m_fValue += rhs.m_fValue;
					return *this;
				}
				break;
				case NVTYPEID_DWORD:
				{
					m_fValue += rhs.m_dwValue;
					return *this;
				}
				break;
				case NVTYPEID_INT:
				{
					m_fValue += rhs.m_iValue;
					return *this;
				}
				break;
				// case NVTYPEID_BOOL:
				// case NVTYPEID_STRING: TODO
				default:
					break;
			}
		}
		break;
		//case NVTYPEID_BOOL:
		//case NVTYPEID_MATRIX: TODO
		//case NVTYPEID_TEXTURE:
		case NVTYPEID_DWORD:
		{
			switch(rhs.m_Type)
			{
				case NVTYPEID_VEC4:
				{
					vec4* pVec4 = reinterpret_cast<vec4*>(rhs.m_pValue);
					m_dwValue += pVec4->x;
					return *this;
				}
				break;
				case NVTYPEID_VEC3:
				{
					vec3* pVec3 = reinterpret_cast<vec3*>(rhs.m_pValue);
					m_dwValue += pVec3->x;
					return *this;
				}
				break;
				case NVTYPEID_VEC2:
				{
					vec2* pVec2 = reinterpret_cast<vec2*>(rhs.m_pValue);
					m_dwValue += pVec2->x;
					return *this;
				}
				break;
				case NVTYPEID_FLOAT:
				{
					m_dwValue += rhs.m_fValue;
					return *this;
				}
				break;
				case NVTYPEID_DWORD:
				{
					m_dwValue += rhs.m_dwValue;
					return *this;
				}
				break;
				case NVTYPEID_INT:
				{
					m_dwValue += rhs.m_iValue;
					return *this;
				}
				break;
				//case NVTYPEID_BOOL:
				//case NVTYPEID_STRING: TODO
				default:
					break;
			}
		}
		break;
		case NVTYPEID_INT:
		{
			switch(rhs.m_Type)
			{
				case NVTYPEID_VEC4:
				{
					vec4* pVec4 = reinterpret_cast<vec4*>(rhs.m_pValue);
					m_iValue += pVec4->x;
					return *this;
				}
				break;
				case NVTYPEID_VEC3:
				{
					vec3* pVec3 = reinterpret_cast<vec3*>(rhs.m_pValue);
					m_iValue += pVec3->x;
					return *this;
				}
				break;
				case NVTYPEID_VEC2:
				{
					vec2* pVec2 = reinterpret_cast<vec2*>(rhs.m_pValue);
					m_iValue += pVec2->x;
					return *this;
				}
				break;
				case NVTYPEID_FLOAT:
				{
					m_iValue += rhs.m_fValue;
					return *this;
				}
				break;
				case NVTYPEID_DWORD:
				{
					m_iValue += rhs.m_dwValue;
					return *this;
				}
				break;
				case NVTYPEID_INT:
				{
					m_iValue += rhs.m_iValue;
					return *this;
				}
				break;
				//case NVTYPEID_BOOL:
				default:
					break;
			}
		}
		break;
		// case NVTYPEID_STRING: TODO
		//case NVTYPEID_UNKNOWN:
		default:
			assert(0);
			break;
	}

	ostringstream strStream;
	strStream << NVType::TypeIDToString(m_Type) << " += " << NVType::TypeIDToString(rhs.m_Type);
	GetSYSInterface()->Exception(NVException(NVEXCEPTIONID_INVALIDOPERATION, strStream.str().c_str()));
	return *this;
}

const NVType& NVType::operator -= (const NVType& rhs)
{
	switch(m_Type)
	{
		case NVTYPEID_VEC4:
		{
			vec4* pVec4 = reinterpret_cast<vec4*>(m_pValue);
			switch(rhs.m_Type)
			{
				case NVTYPEID_VEC4:
				{
					*pVec4 -= *reinterpret_cast<vec4*>(rhs.m_pValue);
					return *this;
				}
				break;
				case NVTYPEID_VEC3:
				{
					vec3* pVec3 = reinterpret_cast<vec3*>(rhs.m_pValue);
					pVec4->x -= pVec3->x;
					pVec4->y -= pVec3->y;
					pVec4->z -= pVec3->z;
					return *this;
				}
				break;
				case NVTYPEID_VEC2:
				{
					vec2* pVec2 = reinterpret_cast<vec2*>(rhs.m_pValue);
					pVec4->x -= pVec2->x;
					pVec4->y -= pVec2->y;
					return *this;
				}
				break;
				case NVTYPEID_FLOAT:
				{
					pVec4->x -= rhs.m_fValue;
					pVec4->y -= rhs.m_fValue;
					pVec4->z -= rhs.m_fValue;
					pVec4->w -= rhs.m_fValue;
					return *this;
				}
				break;
				case NVTYPEID_DWORD:
				{
					pVec4->x -= rhs.m_dwValue;
					pVec4->y -= rhs.m_dwValue;
					pVec4->z -= rhs.m_dwValue;
					pVec4->w -= rhs.m_dwValue;
					return *this;
				}
				break;
				case NVTYPEID_INT:
				{
					pVec4->x -= rhs.m_iValue;
					pVec4->y -= rhs.m_iValue;
					pVec4->z -= rhs.m_iValue;
					pVec4->w -= rhs.m_iValue;
					return *this;
				}
				break;
				//	case NVTYPEID_BOOL:
				// case NVTYPEID_STRING: TODO

				default:
					break;
			}
		}
		break;
		case NVTYPEID_VEC3:
		{
			vec3* pVec3 = reinterpret_cast<vec3*>(m_pValue);
			switch(rhs.m_Type)
			{
				case NVTYPEID_VEC4:
				{
					vec4* pVec4 = reinterpret_cast<vec4*>(rhs.m_pValue);
					pVec3->x -= pVec4->x;
					pVec3->y -= pVec4->y;
					pVec3->z -= pVec4->z;
					return *this;
				}
				break;
				case NVTYPEID_VEC3:
				{
					*pVec3 -= *reinterpret_cast<vec3*>(rhs.m_pValue);
					return *this;
				}
				break;
				case NVTYPEID_VEC2:
				{
					vec2* pVec2 = reinterpret_cast<vec2*>(rhs.m_pValue);
					pVec3->x -= pVec2->x;
					pVec3->y -= pVec2->y;
					return *this;
				}
				break;
				case NVTYPEID_FLOAT:
				{
					pVec3->x -= rhs.m_fValue;
					pVec3->y -= rhs.m_fValue;
					pVec3->z -= rhs.m_fValue;
					return *this;
				}
				break;
				case NVTYPEID_DWORD:
				{
					pVec3->x -= rhs.m_dwValue;
					pVec3->y -= rhs.m_dwValue;
					pVec3->z -= rhs.m_dwValue;
					return *this;
				}
				break;
				case NVTYPEID_INT:
				{
					pVec3->x -= rhs.m_iValue;
					pVec3->y -= rhs.m_iValue;
					pVec3->z -= rhs.m_iValue;
					return *this;
				}
				break;
				//case NVTYPEID_BOOL:
				//case NVTYPEID_STRING: TODO
				default:
					break;
			}
		}
		break;
		case NVTYPEID_VEC2:
		{
			vec2* pVec2 = reinterpret_cast<vec2*>(m_pValue);
			switch(rhs.m_Type)
			{
				case NVTYPEID_VEC4:
				{
					vec4* pVec4 = reinterpret_cast<vec4*>(rhs.m_pValue);
					pVec2->x -= pVec4->x;
					pVec2->y -= pVec4->y;
					return *this;
				}
				break;
				case NVTYPEID_VEC3:
				{
					vec3* pVec3 = reinterpret_cast<vec3*>(rhs.m_pValue);
					pVec2->x -= pVec2->x;
					pVec2->y -= pVec2->y;
					return *this;
				}
				break;
				case NVTYPEID_VEC2:
				{
					*pVec2 = *reinterpret_cast<vec2*>(rhs.m_pValue);
					return *this;
				}
				break;
				case NVTYPEID_FLOAT:
				{
					pVec2->x -= rhs.m_fValue;
					pVec2->y -= rhs.m_fValue;
					return *this;
				}
				break;
				case NVTYPEID_DWORD:
				{
					pVec2->x -= rhs.m_dwValue;
					pVec2->y -= rhs.m_dwValue;
					return *this;
				}
				break;
				case NVTYPEID_INT:
				{
					pVec2->x -= rhs.m_iValue;
					pVec2->y -= rhs.m_iValue;
					return *this;
				}
				break;
				// case NVTYPEID_BOOL:
				// case NVTYPEID_STRING: TODO
				default:
					break;
			}
		}
		break;
		case NVTYPEID_FLOAT:
		{
			switch(rhs.m_Type)
			{
				case NVTYPEID_VEC4:
				{
					vec4* pVec4 = reinterpret_cast<vec4*>(rhs.m_pValue);
					m_fValue -= pVec4->x;
					return *this;
				}
				break;
				case NVTYPEID_VEC3:
				{
					vec3* pVec3 = reinterpret_cast<vec3*>(rhs.m_pValue);
					m_fValue -= pVec3->x;
					return *this;
				}
				break;
				case NVTYPEID_VEC2:
				{
					vec2* pVec2 = reinterpret_cast<vec2*>(rhs.m_pValue);
					m_fValue -= pVec2->x;
					return *this;
				}
				break;
				case NVTYPEID_FLOAT:
				{
					m_fValue -= rhs.m_fValue;
					return *this;
				}
				break;
				case NVTYPEID_DWORD:
				{
					m_fValue -= rhs.m_dwValue;
					return *this;
				}
				break;
				case NVTYPEID_INT:
				{
					m_fValue -= rhs.m_iValue;
					return *this;
				}
				break;
				// case NVTYPEID_BOOL:
				// case NVTYPEID_STRING: TODO
				default:
					break;
			}
		}
		break;
		//case NVTYPEID_BOOL:
		//case NVTYPEID_MATRIX: TODO
		//case NVTYPEID_TEXTURE:
		case NVTYPEID_DWORD:
		{
			switch(rhs.m_Type)
			{
				case NVTYPEID_VEC4:
				{
					vec4* pVec4 = reinterpret_cast<vec4*>(rhs.m_pValue);
					m_dwValue -= pVec4->x;
					return *this;
				}
				break;
				case NVTYPEID_VEC3:
				{
					vec3* pVec3 = reinterpret_cast<vec3*>(rhs.m_pValue);
					m_dwValue -= pVec3->x;
					return *this;
				}
				break;
				case NVTYPEID_VEC2:
				{
					vec2* pVec2 = reinterpret_cast<vec2*>(rhs.m_pValue);
					m_dwValue -= pVec2->x;
					return *this;
				}
				break;
				case NVTYPEID_FLOAT:
				{
					m_dwValue -= rhs.m_fValue;
					return *this;
				}
				break;
				case NVTYPEID_DWORD:
				{
					m_dwValue -= rhs.m_dwValue;
					return *this;
				}
				break;
				case NVTYPEID_INT:
				{
					m_dwValue -= rhs.m_iValue;
					return *this;
				}
				break;
				//case NVTYPEID_BOOL:
				//case NVTYPEID_STRING: TODO
				default:
					break;
			}
		}
		break;
		case NVTYPEID_INT:
		{
			switch(rhs.m_Type)
			{
				case NVTYPEID_VEC4:
				{
					vec4* pVec4 = reinterpret_cast<vec4*>(rhs.m_pValue);
					m_iValue -= pVec4->x;
					return *this;
				}
				break;
				case NVTYPEID_VEC3:
				{
					vec3* pVec3 = reinterpret_cast<vec3*>(rhs.m_pValue);
					m_iValue -= pVec3->x;
					return *this;
				}
				break;
				case NVTYPEID_VEC2:
				{
					vec2* pVec2 = reinterpret_cast<vec2*>(rhs.m_pValue);
					m_iValue -= pVec2->x;
					return *this;
				}
				break;
				case NVTYPEID_FLOAT:
				{
					m_iValue -= rhs.m_fValue;
					return *this;
				}
				break;
				case NVTYPEID_DWORD:
				{
					m_iValue -= rhs.m_dwValue;
					return *this;
				}
				break;
				case NVTYPEID_INT:
				{
					m_iValue -= rhs.m_iValue;
					return *this;
				}
				break;
				//case NVTYPEID_BOOL:
				default:
					break;
			}
		}
		break;
		// case NVTYPEID_STRING: TODO
		//case NVTYPEID_UNKNOWN:
		default:
			assert(0);
			break;
	}

	ostringstream strStream;
	strStream << NVType::TypeIDToString(m_Type) << " -= " << NVType::TypeIDToString(rhs.m_Type);
	GetSYSInterface()->Exception(NVException(NVEXCEPTIONID_INVALIDOPERATION, strStream.str().c_str()));
	return *this;
}

const NVType& NVType::operator /= (const NVType& rhs)
{
	switch(m_Type)
	{
		case NVTYPEID_VEC4:
		{
			vec4* pVec4 = reinterpret_cast<vec4*>(m_pValue);
			switch(rhs.m_Type)
			{
				case NVTYPEID_VEC4:
				{
					vec4* pVec4rhs = reinterpret_cast<vec4*>(rhs.m_pValue);
					pVec4->x /= pVec4rhs->x;
					pVec4->y /= pVec4rhs->y;
					pVec4->z /= pVec4rhs->z;
					pVec4->w /= pVec4rhs->w;
					return *this;
				}
				break;
				case NVTYPEID_VEC3:
				{
					vec3* pVec3 = reinterpret_cast<vec3*>(rhs.m_pValue);
					pVec4->x /= pVec3->x;
					pVec4->y /= pVec3->y;
					pVec4->z /= pVec3->z;
					return *this;
				}
				break;
				case NVTYPEID_VEC2:
				{
					vec2* pVec2 = reinterpret_cast<vec2*>(rhs.m_pValue);
					pVec4->x /= pVec2->x;
					pVec4->y /= pVec2->y;
					return *this;
				}
				break;
				case NVTYPEID_FLOAT:
				{
					pVec4->x /= rhs.m_fValue;
					pVec4->y /= rhs.m_fValue;
					pVec4->z /= rhs.m_fValue;
					pVec4->w /= rhs.m_fValue;
					return *this;
				}
				break;
				case NVTYPEID_DWORD:
				{
					pVec4->x /= rhs.m_dwValue;
					pVec4->y /= rhs.m_dwValue;
					pVec4->z /= rhs.m_dwValue;
					pVec4->w /= rhs.m_dwValue;
					return *this;
				}
				break;
				case NVTYPEID_INT:
				{
					pVec4->x /= rhs.m_iValue;
					pVec4->y /= rhs.m_iValue;
					pVec4->z /= rhs.m_iValue;
					pVec4->w /= rhs.m_iValue;
					return *this;
				}
				break;
				//	case NVTYPEID_BOOL:
				// case NVTYPEID_STRING: TODO

				default:
					break;
			}
		}
		break;
		case NVTYPEID_VEC3:
		{
			vec3* pVec3 = reinterpret_cast<vec3*>(m_pValue);
			switch(rhs.m_Type)
			{
				case NVTYPEID_VEC4:
				{
					vec4* pVec4 = reinterpret_cast<vec4*>(rhs.m_pValue);
					pVec3->x /= pVec4->x;
					pVec3->y /= pVec4->y;
					pVec3->z /= pVec4->z;
					return *this;
				}
				break;
				case NVTYPEID_VEC3:
				{
					vec3 *pVec3rhs = reinterpret_cast<vec3*>(rhs.m_pValue);
					pVec3->x /= pVec3rhs->x;
					pVec3->y /= pVec3rhs->y;
					pVec3->z /= pVec3rhs->z;
					return *this;
				}
				break;
				case NVTYPEID_VEC2:
				{
					vec2* pVec2 = reinterpret_cast<vec2*>(rhs.m_pValue);
					pVec3->x /= pVec2->x;
					pVec3->y /= pVec2->y;
					return *this;
				}
				break;
				case NVTYPEID_FLOAT:
				{
					pVec3->x /= rhs.m_fValue;
					pVec3->y /= rhs.m_fValue;
					pVec3->z /= rhs.m_fValue;
					return *this;
				}
				break;
				case NVTYPEID_DWORD:
				{
					pVec3->x /= rhs.m_dwValue;
					pVec3->y /= rhs.m_dwValue;
					pVec3->z /= rhs.m_dwValue;
					return *this;
				}
				break;
				case NVTYPEID_INT:
				{
					pVec3->x /= rhs.m_iValue;
					pVec3->y /= rhs.m_iValue;
					pVec3->z /= rhs.m_iValue;
					return *this;
				}
				break;
				//case NVTYPEID_BOOL:
				//case NVTYPEID_STRING: TODO
				default:
					break;
			}
		}
		break;
		case NVTYPEID_VEC2:
		{
			vec2* pVec2 = reinterpret_cast<vec2*>(m_pValue);
			switch(rhs.m_Type)
			{
				case NVTYPEID_VEC4:
				{
					vec4* pVec4 = reinterpret_cast<vec4*>(rhs.m_pValue);
					pVec2->x /= pVec4->x;
					pVec2->y /= pVec4->y;
					return *this;
				}
				break;
				case NVTYPEID_VEC3:
				{
					vec3* pVec3 = reinterpret_cast<vec3*>(rhs.m_pValue);
					pVec2->x /= pVec2->x;
					pVec2->y /= pVec2->y;
					return *this;
				}
				break;
				case NVTYPEID_VEC2:
				{
					*pVec2 = *reinterpret_cast<vec2*>(rhs.m_pValue);
					return *this;
				}
				break;
				case NVTYPEID_FLOAT:
				{
					pVec2->x /= rhs.m_fValue;
					pVec2->y /= rhs.m_fValue;
					return *this;
				}
				break;
				case NVTYPEID_DWORD:
				{
					pVec2->x /= rhs.m_dwValue;
					pVec2->y /= rhs.m_dwValue;
					return *this;
				}
				break;
				case NVTYPEID_INT:
				{
					pVec2->x /= rhs.m_iValue;
					pVec2->y /= rhs.m_iValue;
					return *this;
				}
				break;
				// case NVTYPEID_BOOL:
				// case NVTYPEID_STRING: TODO
				default:
					break;
			}
		}
		break;
		case NVTYPEID_FLOAT:
		{
			switch(rhs.m_Type)
			{
				case NVTYPEID_VEC4:
				{
					vec4* pVec4 = reinterpret_cast<vec4*>(rhs.m_pValue);
					m_fValue /= pVec4->x;
					return *this;
				}
				break;
				case NVTYPEID_VEC3:
				{
					vec3* pVec3 = reinterpret_cast<vec3*>(rhs.m_pValue);
					m_fValue /= pVec3->x;
					return *this;
				}
				break;
				case NVTYPEID_VEC2:
				{
					vec2* pVec2 = reinterpret_cast<vec2*>(rhs.m_pValue);
					m_fValue /= pVec2->x;
					return *this;
				}
				break;
				case NVTYPEID_FLOAT:
				{
					m_fValue /= rhs.m_fValue;
					return *this;
				}
				break;
				case NVTYPEID_DWORD:
				{
					m_fValue /= rhs.m_dwValue;
					return *this;
				}
				break;
				case NVTYPEID_INT:
				{
					m_fValue /= rhs.m_iValue;
					return *this;
				}
				break;
				// case NVTYPEID_BOOL:
				// case NVTYPEID_STRING: TODO
				default:
					break;
			}
		}
		break;
		//case NVTYPEID_BOOL:
		//case NVTYPEID_MATRIX: TODO
		//case NVTYPEID_TEXTURE:
		case NVTYPEID_DWORD:
		{
			switch(rhs.m_Type)
			{
				case NVTYPEID_VEC4:
				{
					vec4* pVec4 = reinterpret_cast<vec4*>(rhs.m_pValue);
					m_dwValue /= pVec4->x;
					return *this;
				}
				break;
				case NVTYPEID_VEC3:
				{
					vec3* pVec3 = reinterpret_cast<vec3*>(rhs.m_pValue);
					m_dwValue /= pVec3->x;
					return *this;
				}
				break;
				case NVTYPEID_VEC2:
				{
					vec2* pVec2 = reinterpret_cast<vec2*>(rhs.m_pValue);
					m_dwValue /= pVec2->x;
					return *this;
				}
				break;
				case NVTYPEID_FLOAT:
				{
					m_dwValue /= rhs.m_fValue;
					return *this;
				}
				break;
				case NVTYPEID_DWORD:
				{
					m_dwValue /= rhs.m_dwValue;
					return *this;
				}
				break;
				case NVTYPEID_INT:
				{
					m_dwValue /= rhs.m_iValue;
					return *this;
				}
				break;
				//case NVTYPEID_BOOL:
				//case NVTYPEID_STRING: TODO
				default:
					break;
			}
		}
		break;
		case NVTYPEID_INT:
		{
			switch(rhs.m_Type)
			{
				case NVTYPEID_VEC4:
				{
					vec4* pVec4 = reinterpret_cast<vec4*>(rhs.m_pValue);
					m_iValue /= pVec4->x;
					return *this;
				}
				break;
				case NVTYPEID_VEC3:
				{
					vec3* pVec3 = reinterpret_cast<vec3*>(rhs.m_pValue);
					m_iValue /= pVec3->x;
					return *this;
				}
				break;
				case NVTYPEID_VEC2:
				{
					vec2* pVec2 = reinterpret_cast<vec2*>(rhs.m_pValue);
					m_iValue /= pVec2->x;
					return *this;
				}
				break;
				case NVTYPEID_FLOAT:
				{
					m_iValue /= rhs.m_fValue;
					return *this;
				}
				break;
				case NVTYPEID_DWORD:
				{
					m_iValue /= rhs.m_dwValue;
					return *this;
				}
				break;
				case NVTYPEID_INT:
				{
					m_iValue /= rhs.m_iValue;
					return *this;
				}
				break;
				//case NVTYPEID_BOOL:
				default:
					break;
			}
		}
		break;
		// case NVTYPEID_STRING: TODO
		//case NVTYPEID_UNKNOWN:
		default:
			assert(0);
			break;
	}

	ostringstream strStream;
	strStream << NVType::TypeIDToString(m_Type) << " /= " << NVType::TypeIDToString(rhs.m_Type);
	GetSYSInterface()->Exception(NVException(NVEXCEPTIONID_INVALIDOPERATION, strStream.str().c_str()));
	return *this;
}

const NVType& NVType::operator *= (const NVType& rhs)
{
	switch(m_Type)
	{
		case NVTYPEID_VEC4:
		{
			vec4* pVec4 = reinterpret_cast<vec4*>(m_pValue);
			switch(rhs.m_Type)
			{
				case NVTYPEID_VEC4:
				{
					vec4* pVec4rhs = reinterpret_cast<vec4*>(rhs.m_pValue);
					pVec4->x /= pVec4rhs->x;
					pVec4->y /= pVec4rhs->y;
					pVec4->z /= pVec4rhs->z;
					pVec4->w /= pVec4rhs->w;
					return *this;
				}
				break;
				case NVTYPEID_VEC3:
				{
					vec3* pVec3 = reinterpret_cast<vec3*>(rhs.m_pValue);
					pVec4->x *= pVec3->x;
					pVec4->y *= pVec3->y;
					pVec4->z *= pVec3->z;
					return *this;
				}
				break;
				case NVTYPEID_VEC2:
				{
					vec2* pVec2 = reinterpret_cast<vec2*>(rhs.m_pValue);
					pVec4->x *= pVec2->x;
					pVec4->y *= pVec2->y;
					return *this;
				}
				break;
				case NVTYPEID_FLOAT:
				{
					pVec4->x *= rhs.m_fValue;
					pVec4->y *= rhs.m_fValue;
					pVec4->z *= rhs.m_fValue;
					pVec4->w *= rhs.m_fValue;
					return *this;
				}
				break;
				case NVTYPEID_DWORD:
				{
					pVec4->x *= rhs.m_dwValue;
					pVec4->y *= rhs.m_dwValue;
					pVec4->z *= rhs.m_dwValue;
					pVec4->w *= rhs.m_dwValue;
					return *this;
				}
				break;
				case NVTYPEID_INT:
				{
					pVec4->x *= rhs.m_iValue;
					pVec4->y *= rhs.m_iValue;
					pVec4->z *= rhs.m_iValue;
					pVec4->w *= rhs.m_iValue;
					return *this;
				}
				break;
				//	case NVTYPEID_BOOL:
				// case NVTYPEID_STRING: TODO

				default:
					break;
			}
		}
		break;
		case NVTYPEID_VEC3:
		{
			vec3* pVec3 = reinterpret_cast<vec3*>(m_pValue);
			switch(rhs.m_Type)
			{
				case NVTYPEID_VEC4:
				{
					vec4* pVec4 = reinterpret_cast<vec4*>(rhs.m_pValue);
					pVec3->x *= pVec4->x;
					pVec3->y *= pVec4->y;
					pVec3->z *= pVec4->z;
					return *this;
				}
				break;
				case NVTYPEID_VEC3:
				{
					vec3* pVec3rhs = reinterpret_cast<vec3*>(rhs.m_pValue);
					pVec3->x *= pVec3rhs->x;
					pVec3->y *= pVec3rhs->y;
					pVec3->z *= pVec3rhs->z;
					return *this;
				}
				break;
				case NVTYPEID_VEC2:
				{
					vec2* pVec2 = reinterpret_cast<vec2*>(rhs.m_pValue);
					pVec3->x *= pVec2->x;
					pVec3->y *= pVec2->y;
					return *this;
				}
				break;
				case NVTYPEID_FLOAT:
				{
					pVec3->x *= rhs.m_fValue;
					pVec3->y *= rhs.m_fValue;
					pVec3->z *= rhs.m_fValue;
					return *this;
				}
				break;
				case NVTYPEID_DWORD:
				{
					pVec3->x *= rhs.m_dwValue;
					pVec3->y *= rhs.m_dwValue;
					pVec3->z *= rhs.m_dwValue;
					return *this;
				}
				break;
				case NVTYPEID_INT:
				{
					pVec3->x *= rhs.m_iValue;
					pVec3->y *= rhs.m_iValue;
					pVec3->z *= rhs.m_iValue;
					return *this;
				}
				break;
				//case NVTYPEID_BOOL:
				//case NVTYPEID_STRING: TODO
				default:
					break;
			}
		}
		break;
		case NVTYPEID_VEC2:
		{
			vec2* pVec2 = reinterpret_cast<vec2*>(m_pValue);
			switch(rhs.m_Type)
			{
				case NVTYPEID_VEC4:
				{
					vec4* pVec4 = reinterpret_cast<vec4*>(rhs.m_pValue);
					pVec2->x *= pVec4->x;
					pVec2->y *= pVec4->y;
					return *this;
				}
				break;
				case NVTYPEID_VEC3:
				{
					vec3* pVec3 = reinterpret_cast<vec3*>(rhs.m_pValue);
					pVec2->x *= pVec2->x;
					pVec2->y *= pVec2->y;
					return *this;
				}
				break;
				case NVTYPEID_VEC2:
				{
					*pVec2 = *reinterpret_cast<vec2*>(rhs.m_pValue);
					return *this;
				}
				break;
				case NVTYPEID_FLOAT:
				{
					pVec2->x *= rhs.m_fValue;
					pVec2->y *= rhs.m_fValue;
					return *this;
				}
				break;
				case NVTYPEID_DWORD:
				{
					pVec2->x *= rhs.m_dwValue;
					pVec2->y *= rhs.m_dwValue;
					return *this;
				}
				break;
				case NVTYPEID_INT:
				{
					pVec2->x *= rhs.m_iValue;
					pVec2->y *= rhs.m_iValue;
					return *this;
				}
				break;
				// case NVTYPEID_BOOL:
				// case NVTYPEID_STRING: TODO
				default:
					break;
			}
		}
		break;
		case NVTYPEID_FLOAT:
		{
			switch(rhs.m_Type)
			{
				case NVTYPEID_VEC4:
				{
					vec4* pVec4 = reinterpret_cast<vec4*>(rhs.m_pValue);
					m_fValue *= pVec4->x;
					return *this;
				}
				break;
				case NVTYPEID_VEC3:
				{
					vec3* pVec3 = reinterpret_cast<vec3*>(rhs.m_pValue);
					m_fValue *= pVec3->x;
					return *this;
				}
				break;
				case NVTYPEID_VEC2:
				{
					vec2* pVec2 = reinterpret_cast<vec2*>(rhs.m_pValue);
					m_fValue *= pVec2->x;
					return *this;
				}
				break;
				case NVTYPEID_FLOAT:
				{
					m_fValue *= rhs.m_fValue;
					return *this;
				}
				break;
				case NVTYPEID_DWORD:
				{
					m_fValue *= rhs.m_dwValue;
					return *this;
				}
				break;
				case NVTYPEID_INT:
				{
					m_fValue *= rhs.m_iValue;
					return *this;
				}
				break;
				// case NVTYPEID_BOOL:
				// case NVTYPEID_STRING: TODO
				default:
					break;
			}
		}
		break;
		//case NVTYPEID_BOOL:
		//case NVTYPEID_MATRIX: TODO
		//case NVTYPEID_TEXTURE:
		case NVTYPEID_DWORD:
		{
			switch(rhs.m_Type)
			{
				case NVTYPEID_VEC4:
				{
					vec4* pVec4 = reinterpret_cast<vec4*>(rhs.m_pValue);
					m_dwValue *= pVec4->x;
					return *this;
				}
				break;
				case NVTYPEID_VEC3:
				{
					vec3* pVec3 = reinterpret_cast<vec3*>(rhs.m_pValue);
					m_dwValue *= pVec3->x;
					return *this;
				}
				break;
				case NVTYPEID_VEC2:
				{
					vec2* pVec2 = reinterpret_cast<vec2*>(rhs.m_pValue);
					m_dwValue *= pVec2->x;
					return *this;
				}
				break;
				case NVTYPEID_FLOAT:
				{
					m_dwValue *= rhs.m_fValue;
					return *this;
				}
				break;
				case NVTYPEID_DWORD:
				{
					m_dwValue *= rhs.m_dwValue;
					return *this;
				}
				break;
				case NVTYPEID_INT:
				{
					m_dwValue *= rhs.m_iValue;
					return *this;
				}
				break;
				//case NVTYPEID_BOOL:
				//case NVTYPEID_STRING: TODO
				default:
					break;
			}
		}
		break;
		case NVTYPEID_INT:
		{
			switch(rhs.m_Type)
			{
				case NVTYPEID_VEC4:
				{
					vec4* pVec4 = reinterpret_cast<vec4*>(rhs.m_pValue);
					m_iValue *= pVec4->x;
					return *this;
				}
				break;
				case NVTYPEID_VEC3:
				{
					vec3* pVec3 = reinterpret_cast<vec3*>(rhs.m_pValue);
					m_iValue *= pVec3->x;
					return *this;
				}
				break;
				case NVTYPEID_VEC2:
				{
					vec2* pVec2 = reinterpret_cast<vec2*>(rhs.m_pValue);
					m_iValue *= pVec2->x;
					return *this;
				}
				break;
				case NVTYPEID_FLOAT:
				{
					m_iValue *= rhs.m_fValue;
					return *this;
				}
				break;
				case NVTYPEID_DWORD:
				{
					m_iValue *= rhs.m_dwValue;
					return *this;
				}
				break;
				case NVTYPEID_INT:
				{
					m_iValue *= rhs.m_iValue;
					return *this;
				}
				break;
				//case NVTYPEID_BOOL:
				default:
					break;
			}
		}
		break;
		// case NVTYPEID_STRING: TODO
		//case NVTYPEID_UNKNOWN:
		default:
			assert(0);
			break;
	}

	ostringstream strStream;
	strStream << NVType::TypeIDToString(m_Type) << " *= " << NVType::TypeIDToString(rhs.m_Type);
	GetSYSInterface()->Exception(NVException(NVEXCEPTIONID_INVALIDOPERATION, strStream.str().c_str()));
	return *this;
}

const NVType& NVType::operator - ()
{
	switch(m_Type)
	{
		case NVTYPEID_VEC4:
		{
			reinterpret_cast<vec4*>(m_pValue)->operator -();
			return *this;
		}
		break;
		case NVTYPEID_VEC3:
		{
			reinterpret_cast<vec3*>(m_pValue)->operator -();
			return *this;
		}
		break;
		case NVTYPEID_VEC2:
		{
			reinterpret_cast<vec2*>(m_pValue)->x = -reinterpret_cast<vec2*>(m_pValue)->x;
			reinterpret_cast<vec2*>(m_pValue)->y = -reinterpret_cast<vec2*>(m_pValue)->y;
			return *this;
		}
		break;
		case NVTYPEID_FLOAT:
		{
			m_fValue = -m_fValue;
			return *this;
		}
		break;
		case NVTYPEID_BOOL:
		{
			m_bValue = !m_bValue;
			return *this;
		}
		break;
		//case NVTYPEID_MATRIX:
		//case NVTYPEID_TEXTURE:
		case NVTYPEID_DWORD:
		{
			m_dwValue = -m_dwValue;
			return *this;
		}
		break;
		case NVTYPEID_INT:
		{
			m_iValue = -m_iValue;
			return *this;
		}
		break;
		//case NVTYPEID_STRING:
		default:
			break;
	}

	ostringstream strStream;
	strStream << NVType::TypeIDToString(m_Type) << " (negate) ";
	GetSYSInterface()->Exception(NVException(NVEXCEPTIONID_INVALIDOPERATION, strStream.str().c_str()));
	return *this;

}

SYS_API NVType operator + (const NVType& lhs, const NVType& rhs)
{
	NVType Answer(lhs);
	Answer += rhs;
	return Answer;
}

SYS_API NVType operator - (const NVType& lhs, const NVType& rhs)
{
	NVType Answer(lhs);
	Answer -= rhs;
	return Answer;
}

SYS_API NVType operator / (const NVType& lhs, const NVType& rhs)
{
	NVType Answer(lhs);
	Answer /= rhs;
	return Answer;
}

SYS_API NVType operator * (const NVType& lhs, const NVType& rhs)
{
	NVType Answer(lhs);
	Answer *= rhs;
	return Answer;
}

SYS_API bool StreamNVType(NVType& theValue, eNVTYPEID theType, void* pData, unsigned int Rows, unsigned int Columns)
{
	switch(theType)
	{
		case NVTYPEID_INT:
			theValue.SetDWORD((DWORD)*reinterpret_cast<int*>(pData));
			break;
		case NVTYPEID_DWORD:
			theValue.SetDWORD(*reinterpret_cast<unsigned long*>(pData));
			break;
		case NVTYPEID_VEC4:
			theValue.SetVec4(*reinterpret_cast<vec4*>(pData));
			break;
		case NVTYPEID_VEC3:
			theValue.SetVec3(*reinterpret_cast<vec3*>(pData));
			break;
		case NVTYPEID_VEC2:
			theValue.SetVec2(*reinterpret_cast<vec2*>(pData));
			break;
			// Streaming pointers doesn't make sense...
		case NVTYPEID_TEXTURE:
			theValue.SetTexture(NULL);			
			break;
		case NVTYPEID_TEXTURE2D:
			theValue.SetTexture2D(NULL);			
			break;
		case NVTYPEID_TEXTURE3D:
			theValue.SetTexture3D(NULL);
			break;
		case NVTYPEID_TEXTURECUBE:
			theValue.SetTextureCube(NULL);			
			break;
		case NVTYPEID_VOID:
			theValue.SetVoid(NULL);
			break;
		case NVTYPEID_PIXELSHADER:
			theValue.SetPixelShader(0);
			break;
		case NVTYPEID_VERTEXSHADER:
			theValue.SetVertexShader(0);
			break;
		case NVTYPEID_PIXELFRAGMENT:
			theValue.SetPixelShader(0);
			break;
		case NVTYPEID_VERTEXFRAGMENT:
			theValue.SetVertexShader(0);
			break;
		case NVTYPEID_SAMPLER:
			theValue.SetSampler(0);
			break;
		case NVTYPEID_SAMPLER2D:
			theValue.SetSampler2D(0);
			break;
		case NVTYPEID_SAMPLER3D:
			theValue.SetSampler3D(0);
			break;
		case NVTYPEID_SAMPLERCUBE:
			theValue.SetSamplerCube(0);
			break;
		case NVTYPEID_BOOL:
			theValue.SetBool(*reinterpret_cast<bool*>(pData));
			break;
		case NVTYPEID_FLOAT:
			theValue.SetFloat(*reinterpret_cast<float*>(pData));
			break;
		case NVTYPEID_MATRIX:
			theValue.SetMatrix(reinterpret_cast<float*>(pData), Rows, Columns);
			break;
		case NVTYPEID_STRING:
			theValue.SetString(reinterpret_cast<const char*>(pData));
			break;
		default:
			return false;
			break;
	}
	return true;
}

}; // nv_sys