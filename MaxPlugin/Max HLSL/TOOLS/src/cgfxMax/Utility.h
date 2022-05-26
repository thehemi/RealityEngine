/*********************************************************************NVMH4****
NVSDK not found!
Path:  
File:  Utility.h

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

See utility.cpp


******************************************************************************/

#ifndef UTILITY_H
#define UTILITY_H

#if _MSC_VER >= 1000
#pragma once
#endif 

#define SMALL_FLOAT				(1e-5)
#define DEG_RAD(A)				(((float)(A)) *  0.01745328f)
#define RAD_DEG(A)				(((float)(A)) *  57.2957823f)
#define MAX3(A,B,C)				(A > B && A > C  ? A : (B > C ? B : C))
#define MAX_TMUS				4

#define DDS_CUBEMAP_POSITIVEX 0x00000600 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEX
#define DDS_CUBEMAP_NEGATIVEX 0x00000a00 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEX
#define DDS_CUBEMAP_POSITIVEY 0x00001200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEY
#define DDS_CUBEMAP_NEGATIVEY 0x00002200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEY
#define DDS_CUBEMAP_POSITIVEZ 0x00004200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEZ
#define DDS_CUBEMAP_NEGATIVEZ 0x00008200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEZ

#define DDS_CUBEMAP_ALLFACES ( DDS_CUBEMAP_POSITIVEX | DDS_CUBEMAP_NEGATIVEX |\
                               DDS_CUBEMAP_POSITIVEY | DDS_CUBEMAP_NEGATIVEY |\
                               DDS_CUBEMAP_POSITIVEZ | DDS_CUBEMAP_NEGATIVEZ )


extern HINSTANCE g_hInstance;

typedef std::map<std::string, std::string> tmapEffectNameFile;
typedef std::vector<INode *> INodeList;

struct Vert3
{
    Point3			m_Pos;
	Point3			m_Normal;
    Point3			m_S;
    Point2			m_UV[MAX_TMUS];
	Point3			m_T;
    Point3			m_SxT;
	unsigned long	m_Sg;

};

typedef struct
{
    DWORD m_Num[3];
} Face3;

typedef enum 
{
	SHADER_VERTEXSHADER  = 0,
	SHADER_PIXELSHADER   = 1

} ShaderType;

typedef enum
{
	LG_DEFAULT,
	LG_SPEC,
	LG_NORMAL,
	LG_NORMALSPEC,
	LG_MAX

} LightGroup;

typedef enum
{
	CHANNEL_DIFFUSE,
	CHANNEL_NORMAL,
	CHANNEL_REFLECTION,
	CHANNEL_MAX
}RMatChannel;

typedef enum
{
	DIFFUSE_UV			= 0,
	NORMAL_UV			= 1,
	DETAIL_UV			= 2,
	FX_EMISSIVE_UV		= 3

}UVChannels;

typedef enum
{
	TEX_UNDEFINED,
	TEX_STANDARD,
	TEX_NORMAL,
	TEX_BUMP,
	TEX_CUBE

}TextureType;

class VNormal 
{
	public:

		Point3	m_Normal;
		Point3	m_S;
		Point3	m_T;
		Point3	m_SxT;
		DWORD	m_Smooth;
		VNormal *m_Next;
		BOOL	m_Init;

		VNormal()				   
		{	
			m_Smooth = 0; 
			m_Next   = NULL; 
			m_Init   = false; 

			m_Normal.Set(0.0f,0.0f,0.0f);
			m_S.Set(0.0f,0.0f,0.0f);
			m_T.Set(0.0f,0.0f,0.0f);
			m_SxT.Set(0.0f,0.0f,0.0f);
		}

		
		VNormal(Point3 &N,unsigned long Smooth, Point3 &S, Point3 &T) 
		{	
			m_Next	 = NULL;
			m_Init	 = true;
			m_Normal = N;
			m_Smooth = Smooth;
			m_S	  	 = S;
			m_T		 = T;
		}

		~VNormal() 
		{
			delete m_Next;
		}

		void Clear()
		{
			m_Smooth = 0; 
			m_Next   = NULL; 
			m_Init   = false; 

			m_Normal.Set(0.0f,0.0f,0.0f);
			m_S.Set(0.0f,0.0f,0.0f);
			m_T.Set(0.0f,0.0f,0.0f);
			m_SxT.Set(0.0f,0.0f,0.0f);

		}

		void	AddNormal(Point3 &N, unsigned long Smooth, Point3 &S, Point3 &T);
		Point3& GetNormal(unsigned long Smooth, Point3 &S, Point3 &T, Point3 &SxT);
		void	Normalize();

};

inline D3DCOLOR VectorToRGBA(const D3DXVECTOR3 *V, float Height = 1.0f)
{
	unsigned long R,G,B,A;

    R = (unsigned long)((V->x + 1.0f ) * 127.5f);
    G = (unsigned long)((V->y + 1.0f ) * 127.5f);
    B = (unsigned long)((V->z + 1.0f ) * 127.5f);
    A = (unsigned long) (255.0f * Height);

    return((A << 24L) + (R << 16L) + (G << 8L) + (B << 0L));
}

inline unsigned long VectorToQ8W8V8U8(const D3DXVECTOR3 &Vector)
{
	D3DXVECTOR3 Scaled;
    signed char	Red,Green,Blue,Alpha;

	Scaled = Vector * 127.5f;
    Red    = (signed char)Scaled.x;
    Green  = (signed char)Scaled.y;
	Blue   = (signed char)Scaled.z;
	Alpha  = 0.0f;

    return (((unsigned long)(unsigned char)Alpha << 24 ) | 
			((unsigned long)(unsigned char)Blue  << 16 ) | 
			((unsigned long)(unsigned char)Green << 8  ) | 
			((unsigned long)(unsigned char)Red   << 0));
}

inline D3DCOLOR FloatToRGBA(float V)
{
	unsigned long R,G,B,A;

    R = (unsigned long)(V * 255.5f);
    G = (unsigned long)(V * 255.5f);
    B = (unsigned long)(V * 255.5f);
    A = (unsigned long)(V * 255.5f);

    return((A << 24L) + (R << 16L) + (G << 8L) + (B << 0L));
}

inline void Point3Normalize(Point3 &Normal)
{	
	float	Dist;

	Dist = Normal.Length();

	if(Dist)
	{
		Dist = 1.0f / Dist;
	
		Normal.x *= Dist;
		Normal.y *= Dist;
		Normal.z *= Dist;
	}
	else
	{
		Normal.x = 0.0f;
		Normal.y = 0.0f;
		Normal.z = 0.0f;
	}
	
}

inline void Rotate2DPoint(float &X, float &Y, float A)
{	
	float NewX,NewY,TempX,TempY;

	TempX = X;
	TempY = Y;

	NewX = TempX * (float)cos(-A) - TempY * (float)sin(-A);
	NewY = TempX * (float)sin(-A) + TempY * (float)cos(-A);

	X = NewX;
	Y = NewY;

}

inline bool TMNegParity(Matrix3 &Mat)
{
	return (DotProd(CrossProd(Mat.GetRow(0),Mat.GetRow(1)),Mat.GetRow(2)) < 0.0) ? 1 : 0;
}

inline int LargestPower2(int X)
{
	int i;

	for(i=31; i > 0; i--)
	{
		if(X & (1 << i))
		{
			return(1 << i);
		}
	}

	return(0);

}

TCHAR *GetString(int id);
std::string FindFile(const std::string& strFile);
bool GetFileResource(const char *Name, const char *Type, void **Data,unsigned long &Size);
TriObject* GetTriObjectFromNode(INode *Node, TimeValue T, bool &Delete);
INode* FindNodeRef(ReferenceTarget *Rt);
std::string GetFileNameAndExtension(const char* pszPath);

#endif


