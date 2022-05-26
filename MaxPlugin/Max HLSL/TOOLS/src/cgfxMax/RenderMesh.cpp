/*********************************************************************NVMH4****
Path:  plugins\nvmax
File:  RenderMesh.cpp

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

A class to generate a VB from max internal data and keep it in a cache.
Each max object is auto-converted to a VB so that the plugin can render with it.
VB is currently a fixed format, though this will change in future.

  cmaughan - I fixed this from the original metalbump sample source.  It now creates
  32 bit index buffers which fixed a break on large models (my fallback for 16 bit only
  cards is less good - it simply clamps the num vertices.  A feature that needs fixing
  moving forward, though the assumption is that most artists will use this stuff with
  GeForce 3 or better).

  I also replaced some code that used tab<> with std::vector.  This improved mesh generation
  by 30x (from 30 seconds to <1 on a complicated model).

******************************************************************************/

#include "pch.h"
#include "RenderMesh.h"
#include "Utility.h"

D3DVERTEXELEMENT9 VertexDecl[] =
{
	// First stream is first mesh
	{ 0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_POSITION, 0}, 
	{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_NORMAL,   0}, 
	{ 0, 24, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_TANGENT, 0}, 
	{ 0, 36, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_BINORMAL, 0}, 
	{ 0, 48, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_COLOR, 0}, 
	{ 0, 52, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_TEXCOORD, 0}, 
	D3DDECL_END()
};
LPDIRECT3DVERTEXDECLARATION9 m_VertexDec = NULL;


DWORD	gVIndex[3];

RenderMesh::RenderMesh()
{
	m_VB		= NULL;
	m_IB		= NULL;
	m_NumVert	= 0;
	m_NumFace	= 0;
	m_Valid		= false;
	m_MapChannels.SetCount(4);
	m_MapChannels[0] = 1;
	m_MapChannels[1] = 2;
	m_MapChannels[2] = 3;
	m_MapChannels[3] = 4;

}


//______________________________________
//
//	Default destructor 
//
//______________________________________

RenderMesh::~RenderMesh()
{
	SAFE_RELEASE(m_VB);
	SAFE_RELEASE(m_IB);
}

//______________________________________
//
//	Destroy 
//
//______________________________________

void RenderMesh::Destroy()
{
	SAFE_RELEASE(m_VB);
	SAFE_RELEASE(m_IB);

	m_NumVert	= 0;
	m_NumFace	= 0;
	m_Valid		= false;

}


//_____________________________________
//
//	Evaluate 
//
//_____________________________________

bool RenderMesh::Evaluate(IDirect3DDevice9 *Device, Mesh *aMesh, int MatIndex, bool NegScale)
{
	DWORD					i,j;
    DWORD					*DWIndice;
    WORD					*WIndice;
	SHADERVERTEX2			*Vertices;
	std::vector<Vert3>		Verts;
	std::vector<Face3>		Faces;
	float					U,V;
	DWORD					A,B,C;

	DISPDBG(5, "RenderMesh::Evaluate");
	if(!Device || !aMesh)
	{
		SAFE_RELEASE(m_VB);
		SAFE_RELEASE(m_IB);
		SAFE_RELEASE(m_VertexDec);

		m_Valid	= false;

		return(false);
	}	

	if(!m_Valid)
	{
		DISPDBG(3, "RenderMesh::EvaluateDirty");

		SAFE_RELEASE(m_VB);
		SAFE_RELEASE(m_IB);
		SAFE_RELEASE(m_VertexDec);

		D3DCAPS9 Caps;
		Device->GetDeviceCaps(&Caps);

		unsigned int MaxVertex;
		if (Caps.MaxVertexIndex <= 0xFFFF)
		{
			MaxVertex = 0xFFFF;
			
		}
		else
		{
			MaxVertex = Caps.MaxVertexIndex;
		}

		ConvertFaces(aMesh,MatIndex,Verts,Faces,NegScale);

		m_NumVert = Verts.size();
		m_NumFace = Faces.size();

		if(m_NumVert == 0 || m_NumFace == 0)
		{
			m_Valid	= true;

			return(true);
		}

		if(FAILED(Device->CreateVertexDeclaration(VertexDecl,&m_VertexDec)))
			goto FAILED;

		if(FAILED(Device->CreateVertexBuffer(m_NumVert * sizeof(SHADERVERTEX2),
											 D3DUSAGE_WRITEONLY, 0, 
											 D3DPOOL_MANAGED,&m_VB, NULL)))
		{
			goto FAILED;
		}

		// 16 bit indices
		if(MaxVertex <= 0xFFFF)
		{
			if(FAILED(Device->CreateIndexBuffer(m_NumFace * 3 * sizeof(WORD),
												D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, 
												D3DPOOL_MANAGED,&m_IB, NULL)))
			{
				goto FAILED;
			}

			if(FAILED(m_IB->Lock(0,m_NumFace * 3 * sizeof(WORD), 
							 (void**)&WIndice,0)))
			{									   
				goto FAILED;
			}

			for(i=0; i < m_NumFace; i++)
			{
				A = Faces[i].m_Num[0];
				B = Faces[i].m_Num[1];
				C = Faces[i].m_Num[2];

				assert(A < Verts.size());
				assert(B < Verts.size());
				assert(C < Verts.size());

				if ((A > MaxVertex) || (B > MaxVertex) || (C > MaxVertex))
					A = B = C = 0;

				WIndice[i * 3 + 0] = (WORD)A;
				WIndice[i * 3 + 1] = (WORD)B;
				WIndice[i * 3 + 2] = (WORD)C;
			}

			m_IB->Unlock();
		}
		// 32 bit indices
		else
		{
			if(FAILED(Device->CreateIndexBuffer(m_NumFace * 3 * sizeof(DWORD),
											D3DUSAGE_WRITEONLY, D3DFMT_INDEX32, 
											D3DPOOL_MANAGED,&m_IB, NULL)))
			{
				goto FAILED;
			}

			if(FAILED(m_IB->Lock(0,m_NumFace * 3 * sizeof(DWORD), 
							 (void**)&DWIndice,0)))
			{									   
				goto FAILED;
			}

			for(i=0; i < m_NumFace; i++)
			{
				A = Faces[i].m_Num[0];
				B = Faces[i].m_Num[1];
				C = Faces[i].m_Num[2];

				assert(A < Verts.size());
				assert(B < Verts.size());
				assert(C < Verts.size());

				if ((A > MaxVertex) || (B > MaxVertex) || (C > MaxVertex))
					A = B = C = 0;

				DWIndice[i * 3 + 0] = A;
				DWIndice[i * 3 + 1] = B;
				DWIndice[i * 3 + 2] = C;
			}

			m_IB->Unlock();
		}

    
		if(FAILED(m_VB->Lock(0,0,(void**)&Vertices,0)))
		{
			goto FAILED;
		}

		for(i=0; i < m_NumVert; i++)
		{
			Vertices[i].m_Pos	   = Verts[i].m_Pos;
			Vertices[i].m_Normal   = Verts[i].m_Normal;
			Vertices[i].m_Tangent  = Verts[i].m_S;
            Vertices[i].m_Binormal = Verts[i].m_T;

			for(j=0; j < NVMAX_TEXCOORDS; j++)
			{
				U = Verts[i].m_UV[j].x;
				V = Verts[i].m_UV[j].y;
				//U = -U;
				//Rotate2DPoint(U,V,DEG_RAD(180.0f));

				Vertices[i].m_TexCoord[j].x = U;
				Vertices[i].m_TexCoord[j].y = V;
			}
		}


		m_VB->Unlock();

		m_Valid = true;

		DISPDBG(1, "RenderMesh::Evaluate - Exit");
		return(true);

FAILED:

		SAFE_RELEASE(m_VB);
		SAFE_RELEASE(m_IB);

		m_Valid = false;
		return(false);
	
	}


	return(true);

}


//_____________________________________
//
//	Render 
//
//_____________________________________

bool RenderMesh::Render(IDirect3DDevice9 *Device)
{
	//DISPDBG(5, "RenderMesh::Render");

	if(m_Valid && Device && m_VB && m_IB)
	{
		HRESULT hr;

		// Not sure why this is sometimes NULL, but it happens
		if(!m_VertexDec){
			if(FAILED(Device->CreateVertexDeclaration(VertexDecl,&m_VertexDec)))
				MessageBox(0,"Vertex decl creation failed. This should never happen",0,0);
		}
	
		hr = Device->SetVertexDeclaration(m_VertexDec);
		if (FAILED(hr))
		{
			assert(!"Failed to set vertex decl");
		}

		hr = Device->SetStreamSource(0,m_VB,0,sizeof(SHADERVERTEX2));
		if (FAILED(hr))
		{
			assert(!"Failed to set stream source");
		}

		hr = Device->SetIndices(m_IB);
		if (FAILED(hr))
		{
			assert(!"Failed to set indices source");
		}

		hr = Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,0, m_NumVert,0,m_NumFace );
		if (FAILED(hr))
		{
			// this will trigger on normal effect reset after lost device, so don't allow it
			//assert(!"Failed to drawindexedprimitive");
		}

		assert(m_NumVert);
		assert(m_NumFace);
		
	
		return(true);
	}

	return(false);
}


//_____________________________________
//
//	ConvertFaces
//
//_____________________________________

void RenderMesh::ConvertFaces(Mesh *Mesh, int MatIndex, std::vector<Vert3> &Verts, std::vector<Face3> &Faces, bool NegScale)
{
    Face3			TmpFace;
    Vert3			TmpVert;
	std::vector<bool>	Written;
	DWORD			i,j,k,NumFace;
	DWORD			NumUV,UVCount,Index;
	DWORD			NumVert,Count,VIndex;
	Face			*aFace;
	std::vector<BasisVert>	FNormals;
	std::vector<VNormal>	Normals;
	UVVert			*UVVert;
	TVFace			*UVFace;
	Point3			S,T,SxT;
	unsigned long	Sg;

	DISPDBG(1, "RenderMesh::ConvertFaces");

	if(NegScale)
	{
		gVIndex[0] = 2;
		gVIndex[1] = 1;
		gVIndex[2] = 0;
	}
	else
	{
		gVIndex[0] = 0;
		gVIndex[1] = 1;
		gVIndex[2] = 2;
	}

	NumFace = 0;

	for(i=0; i < Mesh->getNumFaces(); i++) 
	{
		if(!Mesh->faces[i].Hidden())
		{
			Index = Mesh->getFaceMtlIndex(i) + 1;

			if(Index == MatIndex || MatIndex == 0)
			{
				NumFace++;
			}
		}

	}

	NumVert = Mesh->getNumVerts();
	Verts.resize(NumVert);
	Faces.resize(NumFace);

	if(NumVert == 0 || NumFace == 0)
	{
		return;
	}

	ComputeVertexNormals(Mesh,FNormals,Normals,NegScale);

	Written.assign(Mesh->getNumVerts(), false);

	NumUV = Mesh->getNumMaps();	
	

	if(NumUV)
	{	
		DISPDBG(1, "ConvertFaces::NumUV");
		Count = 0;

		if(NumUV > MAX_TMUS + 1)
		{
			NumUV = MAX_TMUS + 1;
		}

		for(i=0; i < Mesh->getNumFaces(); i++) 
		{
			aFace = &Mesh->faces[i];

			TmpFace.m_Num[0] = aFace->v[gVIndex[0]];
			TmpFace.m_Num[1] = aFace->v[gVIndex[1]];
			TmpFace.m_Num[2] = aFace->v[gVIndex[2]];

			Sg = aFace->smGroup;

			for(j=0; j < 3; j++) 
			{
				VIndex			 = aFace->v[gVIndex[j]];
				TmpVert.m_Pos	 = Mesh->verts[VIndex];

				if(Sg)
				{
					TmpVert.m_Normal = Normals[VIndex].GetNormal(Sg,S,T,SxT);
					TmpVert.m_S		 = S;
					TmpVert.m_T		 = T;
					TmpVert.m_SxT	 = SxT;

				}
				else
				{
					TmpVert.m_Normal = FNormals[i].m_Normal;
					TmpVert.m_S		 = FNormals[i].m_S;
					TmpVert.m_T		 = FNormals[i].m_T;
					TmpVert.m_SxT	 = FNormals[i].m_SxT;
				}

			//	TmpVert.m_S.y = -TmpVert.m_S.y;
			//	TmpVert.m_S.z = -TmpVert.m_S.z;


				UVCount		 = 0;
				TmpVert.m_Sg = Sg;

				for(k=0;k<m_MapChannels.Count();k++)
				{	
					int index = m_MapChannels[k];

					if(Mesh->getNumMapVerts(index))
					{
						UVVert = Mesh->mapVerts(index);
						UVFace = Mesh->mapFaces(index);

						// Flip coordinates around the right way
						// Using Rotate2DPoint ensures we get no negative UVs
						if(k == 0)
						{
							TmpVert.m_UV[k].x = -UVVert[UVFace[i].t[gVIndex[j]]].x;
							TmpVert.m_UV[k].y = UVVert[UVFace[i].t[gVIndex[j]]].y;

							Rotate2DPoint(TmpVert.m_UV[k].x,TmpVert.m_UV[k].y,DEG_RAD(180.0f));
						}
						// Need UVs in [0,1] for PRT which we use on second channel
						// If we do the above the coords are negative in places, so must do this
						else
						{
							TmpVert.m_UV[k].x = UVVert[UVFace[i].t[gVIndex[j]]].x;
							TmpVert.m_UV[k].y = UVVert[UVFace[i].t[gVIndex[j]]].y;

							//Rotate2DPoint(TmpVert.m_UV[k].x,TmpVert.m_UV[k].y,DEG_RAD(180.0f));
						}

					}
					 else
					{
						TmpVert.m_UV[k].x = 0.0f;
						TmpVert.m_UV[k].y = 0.0f;
					}
				}
				
				if(Written[VIndex]) 
				{
					if((Sg == 0) || 
					   (Verts[VIndex].m_Sg != TmpVert.m_Sg) ||	
					   (!UVVertEqual(Verts[VIndex].m_UV[0],TmpVert.m_UV[0])||(!UVVertEqual(Verts[VIndex].m_UV[1],TmpVert.m_UV[1]))))
					{
						TmpFace.m_Num[j] = Verts.size();
						Verts.push_back(TmpVert);
					}
				} 
				else 
				{
					Verts[VIndex] = TmpVert;
					Written[VIndex] = true;
				}

			}

			if(!Mesh->faces[i].Hidden())
			{
				Index = Mesh->getFaceMtlIndex(i) + 1;

				if(Index == MatIndex || MatIndex == 0)
				{
					Faces[Count++] = TmpFace;
				}

			}

		}

	}
	else
	{
		DISPDBG(1, "ConvertFaces::None-UV");
		for(i=0; i < Mesh->getNumFaces(); i++) 
		{
			aFace = &Mesh->faces[i];

			Faces[i].m_Num[0] = aFace->v[gVIndex[0]];
			Faces[i].m_Num[1] = aFace->v[gVIndex[1]];
			Faces[i].m_Num[2] = aFace->v[gVIndex[2]];

			for(j=0; j < 3; j++) 
			{
				VIndex					= aFace->v[gVIndex[j]];
				Verts[VIndex].m_Pos		= Mesh->verts[VIndex];
				Verts[VIndex].m_Normal	= Normals[VIndex].GetNormal(aFace->smGroup,S,T,SxT);
				Verts[VIndex].m_S		= Point3(0.0f,0.0f,0.0f);
				Verts[VIndex].m_T		= Point3(0.0f,0.0f,0.0f);
				Verts[VIndex].m_SxT		= Point3(0.0f,0.0f,0.0f);

				for(k=0; k < MAX_TMUS; k++)
				{
					Verts[VIndex].m_UV[k].x = 0.0f;
					Verts[VIndex].m_UV[k].y = 0.0f;
				}

			}

		}

	}

	DISPDBG(1, "RenderMesh::ConvertFaces - Exit");
}

void RenderMesh::ComputeVertexNormals(Mesh *aMesh, std::vector<BasisVert> &FNorms, std::vector<VNormal> &VNorms, bool NegScale) 
{
	Face			*Face;	
	Point3			*Verts;
	Point3			Vt0,Vt1,Vt2;
	Point3			Normal;
	int				i,j,k,A,B,C;
	int				NumUV,NumVert,NumFace;
	UVVert			*UVVert;
	TVFace			*UVFace;
    Vert3			Vert[3];
	Point3			S,T;
	Point3			Edge01,Edge02;
	Point3			Cp;
	float			U0,V0,U1,V1,U2,V2;
	int				UVCount;
	unsigned long	Sg;

	NumUV   = aMesh->getNumMaps();	
	NumVert	= aMesh->getNumVerts();
	NumFace = aMesh->getNumFaces(); 
	Face	= aMesh->faces;	
	Verts	= aMesh->verts;

	VNorms.resize(NumVert);
	FNorms.resize(NumFace);

	DISPDBG(1, "RenderMesh::ComputeVertexNormals");

	if(NumUV > MAX_TMUS + 1)
	{
		NumUV = MAX_TMUS + 1;
	}

	for(i=0; i < NumVert; i++) 
	{
		VNorms[i].Clear();
	}

	for(i=0; i < NumFace; i++, Face++) 
	{
		A = Face->v[gVIndex[0]];
		B = Face->v[gVIndex[1]];
		C = Face->v[gVIndex[2]];

		Vt0 = Verts[A];
		Vt1 = Verts[B];
		Vt2 = Verts[C];

		Normal = (Vt1 - Vt0) ^ (Vt2 - Vt0);

		Point3Normalize(Normal);

		for(j=0; j < 3; j++) 
		{
			UVCount = 0;

			for(k=0;k<m_MapChannels.Count();k++)
			{	
				int index = m_MapChannels[k];
				if(aMesh->getNumMapVerts(index))
				{
					UVVert = aMesh->mapVerts(index);
					UVFace = aMesh->mapFaces(index);

					Vert[j].m_UV[k].x = UVVert[UVFace[i].t[gVIndex[j]]].x;
					Vert[j].m_UV[k].y = UVVert[UVFace[i].t[gVIndex[j]]].y;

				}
				else
				{
					Vert[j].m_UV[k].x = 0.0f;
					Vert[j].m_UV[k].y = 0.0f;

				}
			}

		}

		S.Set(0.0f,0.0f,0.0f);
		T.Set(0.0f,0.0f,0.0f);

		U0 = -Vert[0].m_UV[DIFFUSE_UV].x;
		V0 = Vert[0].m_UV[DIFFUSE_UV].y;

		Rotate2DPoint(U0,V0,DEG_RAD(180.0f));

		U1 = -Vert[1].m_UV[DIFFUSE_UV].x;
		V1 = Vert[1].m_UV[DIFFUSE_UV].y;

		Rotate2DPoint(U1,V1,DEG_RAD(180.0f));

		U2 = -Vert[2].m_UV[DIFFUSE_UV].x;
		V2 = Vert[2].m_UV[DIFFUSE_UV].y;

		Rotate2DPoint(U2,V2,DEG_RAD(180.0f));

		// x, s, t
		Edge01 = Point3(Vt1.x - Vt0.x, 
						U1 - U0, 
						V1 - V0);

		Edge02 = Point3(Vt2.x - Vt0.x, 
						U2 - U0, 
						V2 - V0);


		Cp = CrossProd(Edge01,Edge02);
        Point3Normalize(Cp);

		if(fabs(Cp.x) > 0.0001f)
		{
			S.x = -Cp.y / Cp.x;
			T.x = -Cp.z / Cp.x;
		}

		// y, s, t
		Edge01 = Point3(Vt1.y - Vt0.y, 
					    U1 - U0, 
					    V1 - V0);

		Edge02 = Point3(Vt2.y - Vt0.y, 
				   	    U2 - U0, 
						V2 - V0);

		Cp = CrossProd(Edge01,Edge02);
        Point3Normalize(Cp);

		if(fabs(Cp.x) > 0.0001f)
		{
			S.y = -Cp.y / Cp.x;
			T.y = -Cp.z / Cp.x;
		}

		// z, s, t
		Edge01 = Point3(Vt1.z - Vt0.z, 
					    U1 - U0, 
					    V1 - V0);

		Edge02 = Point3(Vt2.z - Vt0.z, 
					    U2 - U0, 
					    V2 - V0);

		Cp = CrossProd(Edge01,Edge02);
        Point3Normalize(Cp);

		if(fabs(Cp.x) > 0.0001f)
		{
			S.z = -Cp.y / Cp.x;
			T.z = -Cp.z / Cp.x;
		}

		Point3Normalize(S);
		Point3Normalize(T);

		Sg = Face->smGroup;
		
		if(Sg)
		{
			VNorms[A].AddNormal(Normal,Sg,S,T);
			VNorms[B].AddNormal(Normal,Sg,S,T);
			VNorms[C].AddNormal(Normal,Sg,S,T);
		}
		else
		{
			T = CrossProd(S,Normal);
			Point3Normalize(T);

			S = CrossProd(Normal,T);
			Point3Normalize(S);

			FNorms[i].m_Normal = Normal;
			FNorms[i].m_S	   = S;
			FNorms[i].m_T	   = T;
			FNorms[i].m_SxT	   = Normal;
		}

	}

	for(i=0; i < NumVert; i++) 
	{
		VNorms[i].Normalize();
	}

	DISPDBG(1, "RenderMesh::ComputeVertexNormals - Leave");
}

void RenderMesh::SetMappingData(int * map)
{
	for(int i=0;i<4;i++)
	{
		m_MapChannels[i] = map[i];
	}
	
}




