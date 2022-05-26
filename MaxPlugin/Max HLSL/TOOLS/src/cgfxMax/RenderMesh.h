/*********************************************************************NVMH4****
NVSDK not found!
Path:  plugins\nvmax
File:  RenderMesh.h

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

see rendermesh.cpp




******************************************************************************/

#ifndef RENDERMESH_H
#define RENDERMESH_H

#if _MSC_VER >= 1000
#pragma once
#endif 

typedef struct 
{
    Point3		m_Pos;
	Point3		m_Normal;
	Point3		m_S;
	Point2		m_UV[MAX_TMUS];

}SHADERVERTEX;


#define NVMAX_TEXCOORDS 2
typedef struct 
{
    Point3		m_Pos;
   	Point3		m_Normal;
	Point3		m_Tangent;
	Point3		m_Binormal;
  	DWORD		m_Diffuse;
  	Point3		m_TexCoord[NVMAX_TEXCOORDS];

}SHADERVERTEX2;
/*
static const DWORD NVMAXVERTEX_FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_NORMAL | D3DFVF_TEX8 | 
  	D3DFVF_TEXCOORDSIZE3(0) | D3DFVF_TEXCOORDSIZE3(1) | D3DFVF_TEXCOORDSIZE3(2) | D3DFVF_TEXCOORDSIZE3(3) | 
  	D3DFVF_TEXCOORDSIZE3(4) | D3DFVF_TEXCOORDSIZE3(5) | D3DFVF_TEXCOORDSIZE3(6) | D3DFVF_TEXCOORDSIZE3(7);
  	*/

//_____________________________________
//
//	BasisVert
//
//_____________________________________

typedef struct
{
	Point3	m_Normal;
	Point3	m_S;
	Point3	m_T;
	Point3	m_SxT;

}BasisVert;

//_____________________________________
//
// 	LineVertex 
//
//_____________________________________

typedef struct 
{
    D3DXVECTOR3		m_Pos;
    unsigned long	m_Color;      

}LINEVERTEX;


extern unsigned long gVertexDecl[];

class RenderMesh
{
	public:

		LPDIRECT3DVERTEXBUFFER9		m_VB;
		LPDIRECT3DINDEXBUFFER9		m_IB;
		DWORD						m_NumVert;
		DWORD						m_NumFace;
		bool						m_Valid;
		Tab<int>					m_MapChannels;
		//
		//	Constructors
		//
		RenderMesh();
		//
		//	Destructors
		//
		~RenderMesh();
		//
		//	Methods
		//
		bool		Evaluate(IDirect3DDevice9 *Device, Mesh *aMesh, int MatIndex, bool NegScale);
		void		SetMappingData(int * map);
		bool		Render(IDirect3DDevice9 *Device);
		void		Invalidate();
		void		Destroy();
		void		ConvertFaces(Mesh *mesh, int MatIndex, std::vector<Vert3>& verts, std::vector<Face3>& faces, bool NegScale);
		void		ComputeVertexNormals(Mesh *aMesh, std::vector<BasisVert> &FNormal, std::vector<VNormal> &VNorms,bool NegScale);
		BOOL		UVVertEqual(Point2 tv0, Point2 tv1); 
		bool		IsValid() { return m_Valid; }

};

//_____________________________________
//
// 	Invalidate
//
//_____________________________________

inline void RenderMesh::Invalidate()
{
	m_Valid = false;
}


//_____________________________________
//
//	UVVertEqual
//
//_____________________________________

inline BOOL RenderMesh::UVVertEqual(Point2 TV0, Point2 TV1) 
{
	if(TV0.x == TV1.x &&
	   TV0.y == TV1.y)
	{
		return(true);
	}

	return(false);	
}


#ifdef __cplusplus
extern "C" {
#endif



#ifdef __cplusplus
}
#endif

#endif


