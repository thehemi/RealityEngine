/*********************************************************************NVMH4****
Path:  NVSDK\Common\src\nv_fx
File:  nvrenderdevice_fx.cpp

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
#ifndef __NVTYPE_H
#define __NVTYPE_H

namespace nv_sys
{

class INVObject;
class INVObjectSemantics;

typedef enum eNVTYPEID
{
	// Type ID's are FIXED.
	// Do not rearrange these, just add new to the end.
	NVTYPEID_VEC4 = 0,
	NVTYPEID_VEC3 = 1,
	NVTYPEID_VEC2 = 2,
	NVTYPEID_FLOAT = 3,
	NVTYPEID_BOOL = 4,
	NVTYPEID_MATRIX = 5,
	NVTYPEID_TEXTURE = 6,
	NVTYPEID_DWORD = 7,
	NVTYPEID_INT = 8,
	NVTYPEID_STRING = 9,
	NVTYPEID_VERTEXSHADER = 10,
	NVTYPEID_PIXELSHADER = 11,
	NVTYPEID_SAMPLER = 12,
	NVTYPEID_SAMPLER2D = 13,
	NVTYPEID_SAMPLER3D = 14,
	NVTYPEID_SAMPLERCUBE = 15,
	NVTYPEID_PIXELFRAGMENT = 16,
	NVTYPEID_VERTEXFRAGMENT = 17,
	NVTYPEID_VOID = 18,
	NVTYPEID_TEXTURE2D = 19,
	NVTYPEID_TEXTURE3D = 20,
	NVTYPEID_TEXTURECUBE = 21,
	NVTYPEID_INTERFACE = 22,

	NVTYPEID_UNKNOWN = 0x7FFFFFFF
} eNVTYPEID;

// A basic generic type holder.
// Assigning
class SYS_API NVType
{
public:
	NVType();
	NVType(const NVType& rhs);

private:
	void Alloc(const NVType& rhs);
	void Alloc(eNVTYPEID Type, unsigned int Rows = 0, unsigned int Columns = 0);
	void Free();

public:
	virtual INTCALLTYPE ~NVType();

	const NVType& operator = (const NVType& rhs);
	const NVType& operator += (const NVType& rhs);
	const NVType& operator -= (const NVType& rhs);
	const NVType& operator /= (const NVType& rhs);
	const NVType& operator *= (const NVType& rhs);
	const NVType& operator - ();

	SYS_API friend NVType operator + (const NVType& lhs, const NVType& rhs);
	SYS_API friend NVType operator - (const NVType& lhs, const NVType& rhs);
	SYS_API friend NVType operator / (const NVType& lhs, const NVType& rhs);
	SYS_API friend NVType operator * (const NVType& lhs, const NVType& rhs);

	bool CheckTypeRead(eNVTYPEID Type, unsigned int Rows = 0, unsigned int Columns = 0) const;
	bool CheckTypeWrite(eNVTYPEID Type, unsigned int Rows = 0, unsigned int Columns = 0);

	const NVType& Assign(const NVType& rhs);
	eNVTYPEID GetType() const;
	unsigned int GetRows() const;
	unsigned int GetColumns() const;

	static NVType CreateTextureType(nv_renderdevice::INVTexture* pTexture);
	static NVType CreateTexture2DType(nv_renderdevice::INVTexture* pTexture);
	static NVType CreateTexture3DType(nv_renderdevice::INVTexture* pTexture);
	static NVType CreateTextureCubeType(nv_renderdevice::INVTexture* pTexture);
	static NVType CreateDWORDType(const unsigned long& dwValue);
	static NVType CreateVec4Type(const vec4& Value);
	static NVType CreateVec3Type(const vec3& Value);
	static NVType CreateVec2Type(const vec2& Value);
	static NVType CreateFloatType(const float& dwValue);
	static NVType CreateBoolType(const bool& dwValue);
	static NVType CreateMatrixType(const float* pValue, unsigned int Rows, unsigned int Columns);
	static NVType CreateIntType(const int& iValue);
	static NVType CreateStringType(const char* pszString);
	static NVType CreateInterfaceType(INVObject* pValue);
	static NVType CreateVertexShaderType(const unsigned long& dwVertexShader);
	static NVType CreatePixelShaderType(const unsigned long& dwPixelShader);
	static NVType CreatePixelFragmentType(const unsigned long& dwPixelShader);
	static NVType CreateVertexFragmentType(const unsigned long& dwPixelShader);
	static NVType CreateSamplerType(const unsigned long& dwSampler);
	static NVType CreateSampler2DType(const unsigned long& dwSampler);
	static NVType CreateSampler3DType(const unsigned long& dwSampler);
	static NVType CreateSamplerCubeType(const unsigned long& dwSampler);
	static NVType CreateNULLType();
	static NVType CreateVoidType();

	void SetDWORD(const unsigned long& rhs);
	void SetVec4(const vec4& rhs);
	void SetVec3(const vec3& rhs);
	void SetVec2(const vec2& rhs);
	void SetFloat(const float& rhs);
	void SetBool(const bool& rhs);
	void SetMatrix(const float* pMatrix, int Rows, int Columns);
	void SetTexture(nv_renderdevice::INVTexture* rhs);
	void SetTexture2D(nv_renderdevice::INVTexture* rhs);
	void SetTexture3D(nv_renderdevice::INVTexture* rhs);
	void SetTextureCube(nv_renderdevice::INVTexture* rhs);
	void SetInt(const int& rhs);
	void SetString(const char* pszString);
	void SetInterface(INVObject* const prhs);
	void SetPixelShader(const unsigned long& rhs);
	void SetVertexShader(const unsigned long& rhs);
	void SetVertexFragment(const unsigned long& rhs);
	void SetPixelFragment(const unsigned long& rhs);
	void SetSampler(const unsigned long&  rhs);
	void SetSampler2D(const unsigned long&  rhs);
	void SetSampler3D(const unsigned long&  rhs);
	void SetSamplerCube(const unsigned long&  rhs);
	void SetNULL();
	void SetVoid(void* pVoid);

	const unsigned long& GetDWORD() const;
	const vec4& GetVec4() const;
	const vec3& GetVec3() const;
	const vec2& GetVec2() const;
	const float& GetFloat() const;
	const bool& GetBool() const;
	const float* GetMatrix()const;
	nv_renderdevice::INVTexture* GetTexture() const;
	nv_renderdevice::INVTexture* GetTexture2D() const;
	nv_renderdevice::INVTexture* GetTexture3D() const;
	nv_renderdevice::INVTexture* GetTextureCube() const;
	const int& GetInt() const;
	const char* GetString() const;
	const unsigned long& GetPixelShader() const;
	const unsigned long& GetVertexShader() const;
	const unsigned long& GetVertexFragment() const;
	const unsigned long& GetPixelFragment() const;
	const unsigned long& GetSampler() const;
	const unsigned long& GetSampler2D() const;
	const unsigned long& GetSampler3D() const;
	const unsigned long& GetSamplerCube() const;
	INVObject* GetInterface();
	const void* GetVoid() const;

	// Dump state
	void Dump() const;

	// A const pointer to the internal value.  
	// Use with great care.
	const void* GetValue(unsigned long& Bytes) const;

	static const char* TypeIDToString(eNVTYPEID TypeID);

	bool SetObjectSemantics(INVObjectSemantics* pSemantics);
	INVObjectSemantics* GetObjectSemantics() const;

private:
	eNVTYPEID m_Type;
	union
	{
		void* m_pValue;
		int m_iValue;
		unsigned long m_dwValue;
		bool m_bValue;
		float m_fValue;
		float* m_pFloatArray;
		INVObject* m_pInterface;
		nv_renderdevice::INVTexture* m_pTexture;
	};

	unsigned int m_Rows;
	unsigned int m_Columns;
	mutable INVObjectSemantics* m_pObjectSemantics;
};

SYS_API NVType operator + (const NVType& lhs, const NVType& rhs);
SYS_API NVType operator - (const NVType& lhs, const NVType& rhs);
SYS_API NVType operator / (const NVType& lhs, const NVType& rhs);
SYS_API NVType operator * (const NVType& lhs, const NVType& rhs);

// For filling a type from raw data in a typesafe way.
SYS_API bool StreamNVType(NVType& theValue, eNVTYPEID theType, void* pData, unsigned int Rows, unsigned int Columns);

}; // namespace nv_sys

#endif
