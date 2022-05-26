//=========== Copyright (c) 2003, Tim Johnson. All rights reserved. ===========
// Low-level classes and structures used for rendering
//=============================================================================
#include "stdafx.h"
#include <dxerr9.h>
#include <d3dx9.h>
#include <crtdbg.h>
#include "PRTMesh.h"
#include "NVMeshMender.h"

D3DVERTEXELEMENT9 SimpleDecl[] =
{
	// First stream is first mesh
	{ 0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_POSITION, 0}, 
    D3DDECL_END()
};

D3DVERTEXELEMENT9 VertexDecl[] =
{
	// First stream is first mesh
	{ 0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_POSITION, 0}, 
	{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_NORMAL,   0}, 
	{ 0, 24, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_TANGENT, 0}, 
	{ 0, 36, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_TEXCOORD, 0}, 
    // Hack decl
   // { 0, 24, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_BLENDWEIGHT, 0}, 
    //{ 0, 24, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_BLENDINDICES, 0},
	//{ 0, 44, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_COLOR, 0}, 
	D3DDECL_END()
};

// PS2+ variant with overlapping decl elements to stop Draw call complaining
// about unused elements
D3DVERTEXELEMENT9 VertexDecl_PS2[] =
{
	// First stream is first mesh
	{ 0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_POSITION, 0}, 
	{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_NORMAL,   0}, 
	{ 0, 24, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_TANGENT, 0}, 
	{ 0, 36, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_TEXCOORD, 0}, 
    // Hack decl
    { 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_BLENDWEIGHT, 0}, 
    { 0, 0, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_BLENDINDICES, 0},
	{ 0, 0, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_COLOR, 0}, 
	D3DDECL_END()
};

D3DVERTEXELEMENT9 ColVertexDecl[] =
{
	// First stream is first mesh
	{ 0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_POSITION, 0}, 
	{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_NORMAL,   0}, 
	{ 0, 24, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_TANGENT, 0}, 
	{ 0, 36, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_TEXCOORD, 0}, 
	{ 0, 44, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_COLOR, 0}, 
	D3DDECL_END()
};

D3DVERTEXELEMENT9 ColVertexDecl_PS2[] =
{
	// First stream is first mesh
	{ 0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_POSITION, 0}, 
	{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_NORMAL,   0}, 
	{ 0, 24, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_TANGENT, 0}, 
	{ 0, 36, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_TEXCOORD, 0}, 
	{ 0, 44, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_COLOR, 0}, 
    // Hack decl
    { 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_BLENDWEIGHT, 0}, 
    { 0, 0, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_BLENDINDICES, 0},
	D3DDECL_END()
};

D3DVERTEXELEMENT9 VertexT2Decl[] =
{
	// First stream is first mesh
	{ 0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_POSITION, 0}, 
	{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_NORMAL,   0}, 
	{ 0, 24, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_TANGENT, 0}, 
	{ 0, 36, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_TEXCOORD, 0}, 
	{ 0, 44, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_TEXCOORD, 1}, 
    // Hack decl
    //{ 0, 24, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_BLENDWEIGHT, 0}, 
    //{ 0, 24, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_BLENDINDICES, 0},
	//{ 0, 44, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_COLOR, 0}, 
	D3DDECL_END()
};

// PS2+ variant with overlapping decl elements to stop Draw call complaining
// about unused elements
D3DVERTEXELEMENT9 VertexT2Decl_PS2[] =
{
	// First stream is first mesh
	{ 0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_POSITION, 0}, 
	{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_NORMAL,   0}, 
	{ 0, 24, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_TANGENT, 0}, 
	{ 0, 36, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_TEXCOORD, 0}, 
	{ 0, 44, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_TEXCOORD, 1}, 
    // Hack decl
    { 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_BLENDWEIGHT, 0}, 
    { 0, 0, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_BLENDINDICES, 0},
	{ 0, 0, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_COLOR, 0}, 
	D3DDECL_END()
};

D3DVERTEXELEMENT9 SkinnedVertexDecl[] =
{
	// First stream is first mesh
	{ 0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_POSITION, 0}, 
	{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_NORMAL, 0}, 
	{ 0, 24, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_TANGENT, 0}, 
	{ 0, 36, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0}, 
	{ 0, 44, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_BLENDWEIGHT,   0}, 
	{ 0, 56, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_BLENDINDICES, 0},
	D3DDECL_END()
};


D3DVERTEXELEMENT9 PackedVertexDecl[] =
{
	// First stream is first mesh
	{ 0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_POSITION, 0}, 
	{ 0, 12, D3DDECLTYPE_SHORT4N, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_NORMAL,   0}, 
    { 0, 20, D3DDECLTYPE_SHORT4N, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_TANGENT, 0},
	{ 0, 28, D3DDECLTYPE_SHORT2N, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_TEXCOORD, 0}, 
	
	D3DDECL_END()
};

D3DVERTEXELEMENT9 SHVertexDecl[] = 
{
	// TIM: Added custom decl elements & offset other elements
	{ 0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0}, 
	{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0}, 
	{ 0, 24, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT,  0}, 
	{ 0, 36, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0}, 
	// ---
	{0,  44, D3DDECLTYPE_FLOAT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 0},
	{0,  48, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 1},
	{0,  64, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 2},
	{0,  80, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 3},
	{0,  96, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 4},
	{0, 112, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 5},
	{0, 128, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 6},
	D3DDECL_END()
};

SHORT4 ToShort4(Vector4& v);
SHORT2 ToShort2(Vector2& v);
Vector4 FromShort4(SHORT4& v);
Vector2 FromShort2(SHORT2& v);

SHORT4 ToShort4(Vector4& v)
{
    SHORT4 s;
    if(v.x > 1) v.x = 1;
    if(v.y > 1) v.y = 1;
    if(v.z > 1) v.z = 1;
    if(v.w > 1) v.w = 1;
    if(v.x < -1) v.x = -1;
    if(v.y < -1) v.y = -1;
    if(v.z < -1) v.z = -1;
    if(v.w < -1) v.w = -1;

    s.x = v.x * 32767.0;
    s.y = v.y * 32767.0;
    s.z = v.z * 32767.0;
    s.w = v.w * 32767.0;
    return s;
}

SHORT2 ToShort2(Vector2& v)
{
    assert(fabsf(v.x) <= 1.1f && fabsf(v.y) <= 1.1f);
    if(v.x > 1) v.x = 1;
    if(v.y > 1) v.y = 1;
    if(v.x < -1) v.x = -1;
    if(v.y < -1) v.y = -1;

    SHORT2 s;
    s.x = v.x * 32767.0;
    s.y = v.y * 32767.0;
    return s;
}

Vector4 FromShort4(SHORT4& v)
{
    return Vector4(v.x/32767.0f,v.y/32767.0f,v.z/32767.0f,v.w/32767.0f);
}

Vector2 FromShort2(SHORT2& v)
{
    return Vector2(v.x/32767.0f,v.y/32767.0f);
}

VertexFormats* VertexFormats::Instance()
{
    static VertexFormats v;
    return &v;
}


void VertexFormats::Initialize()
{
    AddFormat(SHVertexDecl,sizeof(SHVertex),"SHVertex");
    AddFormat(PackedVertexDecl,sizeof(PackedVertex),"PackedVertex");
    AddFormat(SkinnedVertexDecl,sizeof(SkinnedVertex),"SkinnedVertex");
    AddFormat(SimpleDecl,sizeof(SimpleVertex),"SimpleVertex");

    // Try adding the ideal formats, otherwise fall back to the ones with the missing
    // declaration elements. Missing elements mean Debug mode will complain
    // because the dynamic shaders use certain elements that don't always exist
    //if(!AddFormat(VertexDecl_PS2,sizeof(Vertex),"Vertex"))
        AddFormat(VertexDecl,sizeof(Vertex),"Vertex");
    //if(!AddFormat(VertexT2Decl_PS2,sizeof(VertexT2),"VertexT2"))
        AddFormat(VertexT2Decl,sizeof(VertexT2),"VertexT2");
    //if(!AddFormat(ColVertexDecl_PS2,sizeof(ColVertex),"ColVertex"))
        AddFormat(ColVertexDecl,sizeof(ColVertex),"ColVertex");
}


PRTSettings::PRTSettings()
{
	Enabled = false;
	dwNumRays    = 1024;
	dwNumBounces = 1;
	dwTextureSize = 64;
	bPerPixel = false;
	dwNumChannels = 3;
	dwOrder = 6;
	fLengthScale = 1;

	// Set SH defaults for skimmed milk
	bSubsurfaceScattering = false;
	ReducedScattering	= FloatColor(0.70f, 1.22f, 1.90f, 1.0f);
	Absoption			= FloatColor(0.0014f, 0.0025f, 0.0142f, 1.0f);
	fRelativeIndexOfRefraction = 1.3f;

	Diffuse = FloatColor(1,1,1,1);
	dwPredefinedMatIndex = 0;

	bAdaptive = false;
	bRobustMeshRefine = true;
	fRobustMeshRefineMinEdgeLength = 0.0f;
	dwRobustMeshRefineMaxSubdiv = 2;
	bAdaptiveDL = true;
	fAdaptiveDLMinEdgeLength = 0.03f;
	fAdaptiveDLThreshold = 8e-5f;
	dwAdaptiveDLMaxSubdiv = 3;
	bAdaptiveBounce = false;
	fAdaptiveBounceMinEdgeLength = 0.03f;
	fAdaptiveBounceThreshold = 8e-5f;
	dwAdaptiveBounceMaxSubdiv = 3;
	dwNumPCA = 24;
	dwNumClusters = 1;
	fSubSurfaceMultiplier = 2;

	//
	// Register editable vars
	//
	EditorVars.push_back(EditorVar(" SH Enabled",&Enabled,"SH - General"));
	EditorVars.push_back(EditorVar("Rays",&dwNumRays,"SH - General","Light rays casted. Higher gives more accurate results, but is slower. Default = 1024"));
	EditorVars.push_back(EditorVar("Bounces",&dwNumBounces,"SH - General","Bounces of light. Used in interreflections and scattered lighting. Each bounce requires another computing pass!"));
	EditorVars.push_back(EditorVar("PerPixel",&bPerPixel,"SH - General","Per-pixel lighting or per-vertex? Per pixel is more expensive, but often looks better."));
	EditorVars.push_back(EditorVar("Texture Size",&dwTextureSize,"SH - General","Per-pixel lighting texture size. Must be pow2 (32,64,128,etc)"));
	EditorVars.push_back(EditorVar("Diffuse",(Vector*)&Diffuse,"SH - General","Diffuse color in PRT calculations"));

	EditorVars.push_back(EditorVar("Subsurface Enabled",&bSubsurfaceScattering,"SH - Subsurface","Light rays casted. Higher gives more accurate results, but is slower. Default = 1024"));
	EditorVars.push_back(EditorVar("Reduced Scattering",(Vector*)&ReducedScattering,"SH - Subsurface","See A Practical Model for Subsurface Light Transport, SIGGRAPH 2001"));
	EditorVars.push_back(EditorVar("Rel.Refraction Index",&fRelativeIndexOfRefraction,"SH - Subsurface","Relative index of refraction is the ratio between two absolute indexes of refraction. An index of refraction is ratio of the sine of the angle of incidence to the sine of the angle of refraction. "));
	EditorVars.push_back(EditorVar("Subsurface Scale",&fLengthScale,"SH - Subsurface","Scaling factor for object. Smaller scale = more SSS."));
	EditorVars.push_back(EditorVar("Subsurface Multiplier",&fSubSurfaceMultiplier,"SH - Subsurface","Lighting multiplier for SSS component of calculations"));

}

//----------------------------------------------------------------------------------
// Draws a subset
//----------------------------------------------------------------------------------
void Mesh::DrawSubset(AttributeRange subset){
	if(m_bDontRender)
		return; // May be compiling or something

    LPDIRECT3DDEVICE9 dev = RenderWrap::dev;

	LPDIRECT3DVERTEXBUFFER9 pVertexBuffer = NULL;
	LPDIRECT3DINDEXBUFFER9  pIndexBuffer = NULL;
	
	m_pMesh->GetVertexBuffer(&pVertexBuffer);
	m_pMesh->GetIndexBuffer(&pIndexBuffer);
	

	// Set info
	dev->SetVertexDeclaration(m_pDeclaration);
	dev->SetStreamSource(0, pVertexBuffer, 0, m_pMesh->GetNumBytesPerVertex());
	dev->SetIndices(pIndexBuffer);

	// This is 100% correct. If it bombs, the error is elsewhere
	DXASSERT(dev->DrawIndexedPrimitive( D3DPT_TRIANGLELIST,
		0,subset.VertexStart, subset.VertexCount,subset.FaceStart*3, subset.FaceCount ));

	RenderDevice::Instance()->m_DrawCalls++;
	RenderDevice::Instance()->m_Vertices  += subset.VertexCount;
	RenderDevice::Instance()->m_Triangles += subset.FaceCount;
	m_DrawCalls++;

	if(pVertexBuffer){
		pVertexBuffer->Release();
		pIndexBuffer->Release();
	}
}

//----------------------------------------------------------------------------------
// Draws a subset
//----------------------------------------------------------------------------------
void Mesh::DrawSubsetFVF(AttributeRange subset){
	if(m_bDontRender)
		return; // May be compiling or something

    LPDIRECT3DDEVICE9 dev = RenderWrap::dev;

	LPDIRECT3DVERTEXBUFFER9 pVertexBuffer = NULL;
	LPDIRECT3DINDEXBUFFER9  pIndexBuffer = NULL;
	
	m_pMesh->GetVertexBuffer(&pVertexBuffer);
	m_pMesh->GetIndexBuffer(&pIndexBuffer);
	
	// Set info
	dev->SetVertexDeclaration(0);
	dev->SetStreamSource(0, pVertexBuffer, 0, m_pMesh->GetNumBytesPerVertex());
	dev->SetIndices(pIndexBuffer);

	// This is 100% correct. If it bombs, the error is elsewhere
	DXASSERT(dev->DrawIndexedPrimitive( D3DPT_TRIANGLELIST,
		0,subset.VertexStart, subset.VertexCount,subset.FaceStart*3, subset.FaceCount ));

	if(pVertexBuffer){
		pVertexBuffer->Release();
		pIndexBuffer->Release();
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Material* Mesh::GetMaterial(int id)
{
    if(id < m_AttribToMaterial.size())
    {
        return m_Materials[m_AttribToMaterial[id]];
    }
    else if(m_pBoneComb)
	{
        LPD3DXBONECOMBINATION pBoneComb = reinterpret_cast<LPD3DXBONECOMBINATION>(m_pBoneComb->GetBufferPointer());
		return m_Materials[pBoneComb[id].AttribId];
	} 
	// Otherwise, the attribute id is simply the material id
	else if(id < m_AttribTable.size() && m_AttribTable[id].AttribId < m_Materials.size())
		return m_Materials[id];
    else if(m_Materials.size())
        return m_Materials[0];

    return 0;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
bool Mesh::HasLightMapping()
{
    return m_LightMap;// && this->m_SHOptions.bLightMapping;
}

//----------------------------------------------------------------------------------
// Spherical Harmonics states
//----------------------------------------------------------------------------------
void Mesh::SetSHStates(Shader* shader, AttributeRange subset)
{
    bool bTexture = UsingPRT() && m_pPRTMesh->GetCompBuffer()->IsTexture() &&!UsingColorVertex();
    bTexture = bTexture || m_LightMap;
    shader->SetMeshParams(UsingPRT()||UsingColorVertex()|| m_LightMap,bTexture,UsingColorVertex()|| m_LightMap);

    if(m_LightMap)
        shader->GetEffect()->SetTexture("tLightMap",m_LightMap->GetTexture());

	if(UsingPRT())
    {
	    m_pPRTMesh->ApplyShaderConstants(shader->GetEffect());
	    m_pPRTMesh->ApplyTex(RenderWrap::dev,8); // Must match shader slot
    }
}

//----------------------------------------------------------------------------------
// Sets the vertex shader bone matrices needed to render this pool
// FIXME ASAP: I think we're mixing up bone attribids and standard attrib ids
//----------------------------------------------------------------------------------
void Mesh::SetSkinningRenderStates(Shader* shader, AttributeRange subset, vector<Matrix*> pBoneMatrixPtrs){
	if(!m_pSkinInfo || !m_pBoneComb)
		return;

	// first calculate all the world matrices
	// for each attribute group in the mesh, calculate the set of matrices in the palette
	D3DXMATRIXA16 matTemp;
	LPD3DXBONECOMBINATION pBoneComb = reinterpret_cast<LPD3DXBONECOMBINATION>(m_pBoneComb->GetBufferPointer());

	D3DXMATRIX m_matView = *(D3DXMATRIX*)&RenderWrap::GetView();
	// first calculate all the world matrices
	for (int iPaletteEntry = 0; iPaletteEntry < m_NumPaletteEntries; ++iPaletteEntry)
	{
		int iMatrixIndex = pBoneComb[subset.AttribId].BoneId[iPaletteEntry];
		if (iMatrixIndex != UINT_MAX)
		{
			D3DXMatrixMultiply(&m_pBoneMatrices[iPaletteEntry], &m_pBoneOffsetMatrices[iMatrixIndex], (D3DXMATRIX*)pBoneMatrixPtrs[iMatrixIndex]);
		}
	}

	shader->SetSkinning(m_NumInfl -1, m_NumPaletteEntries, m_pBoneMatrices);
}
//--------------------------------------------------------------------------------------
Mesh::Mesh(){
    m_LightMap = 0;
    m_DrawCalls = -1;
	m_OriginalVertexSize = 0;
	m_pPRTMesh	= NULL;
	m_pSkinInfo = NULL;
	m_pMesh     = NULL;
	m_pSkinInfo = NULL;
	m_pBoneComb = NULL;
	m_pBoneMatrices = NULL;
	m_pBoneOffsetMatrices = NULL;
	m_bPRTMultiPass = false;
	m_bDontRender = false;
	m_UpdateTime = 0;
}
//--------------------------------------------------------------------------------------
Mesh::~Mesh(){
	SAFE_DELETE(m_pPRTMesh);
	SAFE_DELETE_ARRAY(m_pBoneMatrices);
	SAFE_DELETE_ARRAY(m_pBoneOffsetMatrices);
	SAFE_RELEASE(m_pMesh);
	SAFE_RELEASE(m_pSkinInfo);
	SAFE_RELEASE_VECTOR(m_Materials);
	SAFE_DELETE_ARRAY(m_pBoneComb);
    SAFE_DELETE(m_LightMap);
}

//----------------------------------------------------------------------------------
// Bounding boxes for mesh
//----------------------------------------------------------------------------------
HRESULT Mesh::UpdateBoundingInfo(const D3DXVECTOR3 *pFirstPosition, DWORD NumVertices, DWORD dwStride, BYTE* Indices, bool b32bit)
{
    // Update main box
    D3DXComputeBoundingBox(pFirstPosition,NumVertices,dwStride,(D3DXVECTOR3*)&m_LocalBox.min,(D3DXVECTOR3*)&m_LocalBox.max);

    m_AttribBoxes.clear();

    // Update all sub-boxes, but only if multiple attributes, or we'll waste cpu power checking them at runtime
    // Also discriminate based on size. Who cares about culling an item a few meters wide?
    if(m_AttribTable.size() > 1 && (m_LocalBox.max - m_LocalBox.min).Length() > 4)
    {
        for(int i=0;i<m_AttribTable.size();i++)
        {
            AttributeRange r = m_AttribTable[i];

            // Get vertices just for this subset
            BYTE* ptr = (BYTE*)pFirstPosition;
            Vector* verts = new Vector[r.FaceCount*3];
            if(b32bit)
            {
                DWORD* inds = (DWORD*)Indices;
                for(int a=0;a<r.FaceCount*3;a++)
                    verts[a] = *(Vector*)&ptr[inds[r.FaceStart*3+a]*dwStride];
            }
            else
            {
                WORD* inds = (WORD*)Indices;
                for(int a=0;a<r.FaceCount*3;a++)
                    verts[a] = *(Vector*)&ptr[inds[r.FaceStart*3+a]*dwStride];
            }

            
            BBox box;
            D3DXComputeBoundingBox((D3DXVECTOR3*)&verts[0],r.FaceCount*3,sizeof(Vector),(D3DXVECTOR3*)&box.min,(D3DXVECTOR3*)&box.max);
            m_AttribBoxes.push_back(box);

            delete[] verts;
        }
    }

    return S_OK;
}


//----------------------------------------------------------------------------------
// Desc: Creates a mesh from basics
//----------------------------------------------------------------------------------
bool Mesh::Create(BYTE* vertices, BYTE* indices, int vertexSize, int indexSize, int numVertices, int numIndices, vector<Material*> materials)
{
    // Build bbox, even on dedicated server
    DXASSERT(UpdateBoundingInfo((D3DXVECTOR3*)&vertices[0],numVertices,vertexSize,indices,indexSize==4?true:false));
		
    if(Engine::Instance()->IsDedicated())
        return true;

    //
    // HACK CODE - Cards that don't support packed vertices
    //
    Vertex* newVerts = 0;
    if(vertexSize == sizeof(PackedVertex) && !VertexFormats::Instance()->FindFormat(vertexSize))
    {
        newVerts = new Vertex[numVertices];
        vertexSize = sizeof(Vertex);
        PackedVertex* v = (PackedVertex*)vertices;
        for(int i=0;i<numVertices;i++)
        {
            newVerts[i].position    = v[i].pos;
            newVerts[i].normal      = FromShort4(v[i].norm);
            newVerts[i].tan         = FromShort4(v[i].tan);
            newVerts[i].tex         = FromShort2(v[i].tex);
        }
        vertices = (BYTE*)newVerts;
    }

    SAFE_RELEASE(m_pMesh);
	m_Materials  = materials;
	m_OriginalVertexSize = vertexSize;

	D3DVERTEXELEMENT9* Decl;
    VertexFormats::VFormat* v = VertexFormats::Instance()->FindFormat(vertexSize);
    if(!v)
    {
		Warning("Mesh has invalid vertex size, can't find appropriate declaration: %d bytes",vertexSize);
		return false;
	}

    m_pDeclaration = v->decl;
    Decl = v->element;

	DWORD flags = D3DXMESH_MANAGED;
	if(indexSize == sizeof(DWORD))
		flags |= D3DXMESH_32BIT;
	HRESULT hr = D3DXCreateMesh(numIndices/3, numVertices, flags , Decl ,RenderWrap::dev, &m_pMesh);
	if(FAILED(hr)){
		Warning("D3DXCreateMesh failed with %d indices and %d vertices",numIndices,numVertices);
		return false;
	}

	// Fill the mesh!
	BYTE* VerticesBuffer;
	m_pMesh->LockVertexBuffer(0, (LPVOID*)&VerticesBuffer);
	memcpy(VerticesBuffer, &vertices[0], numVertices * vertexSize);
	m_pMesh->UnlockVertexBuffer();

	BYTE* IndexesBuffer;
	m_pMesh->LockIndexBuffer(0, (LPVOID*)&IndexesBuffer);
	memcpy(IndexesBuffer, &indices[0], numIndices * indexSize);
	m_pMesh->UnlockIndexBuffer();

	UpdateBuffers();
	CalcData(false);

    SAFE_DELETE_ARRAY(newVerts);
	return true;
}

//----------------------------------------------------------------------------------
// 
//----------------------------------------------------------------------------------
void Mesh::FillAttributesFromMesh(ID3DXMesh* mesh)
{
	// Extract attribute table
	D3DXATTRIBUTERANGE	 attribTable[100]; 
	DWORD				 attribTableSize = 100;
	mesh->GetAttributeTable(attribTable,&attribTableSize);
	m_AttribTable.clear();
	for(int i=0;i<attribTableSize;i++)
	{
		AttributeRange r;
		r.AttribId = attribTable[i].AttribId;
		r.FaceCount = attribTable[i].FaceCount;
		r.FaceStart = attribTable[i].FaceStart;
		r.VertexStart = attribTable[i].VertexStart;
		r.VertexCount = attribTable[i].VertexCount;
		//r.VertexOffset = 0; // Unused
		m_AttribTable.push_back(r);

		// Add material segments if they don't exist
        if(i >= m_Materials.size())
        {
		    MaterialManager::Instance()->GetDefaultMaterial()->AddRef();
		    m_Materials.push_back(MaterialManager::Instance()->GetDefaultMaterial());
        }
	}
}

//----------------------------------------------------------------------------------
// Data mesh needs
//----------------------------------------------------------------------------------
void Mesh::UpdateBuffers()
{
	// Store IB/VB here
	if(m_pMesh)
	{
		m_pMesh->GetVertexBuffer(&m_pVertexBuffer);
		m_pMesh->GetIndexBuffer(&m_pIndexBuffer);
		// Decrement ref count so we never mess with the mesh release system
		// These pointers will only be used when we know the mesh is still valid
		m_pVertexBuffer->Release();
		m_pIndexBuffer->Release();
	}
}

//----------------------------------------------------------------------------------
// Desc: Extracts bones from D3DXSKININFO
//----------------------------------------------------------------------------------
void Mesh::GetBones(vector<SkinWeights>& bones){
	// Extract the skinning data
	if(m_pSkinInfo){
		for(int i=0;i<m_pSkinInfo->GetNumBones();i++){
			SkinWeights s;
			DWORD* indices = new DWORD[m_pSkinInfo->GetNumBoneInfluences(i)];
			s.weights.resize(m_pSkinInfo->GetNumBoneInfluences(i));
			// Get Indices & weights. Indices into temp array.
			m_pSkinInfo->GetBoneInfluence(i,indices,(FLOAT*)&s.weights[0]);
			// Copy DWORD indices into DWORD array
			for(int j=0;j<m_pSkinInfo->GetNumBoneInfluences(i);j++)
				s.indices.push_back(indices[j]);
			delete[] indices;

			s.offset = *(Matrix*)m_pSkinInfo->GetBoneOffsetMatrix(i);
			s.name = m_pSkinInfo->GetBoneName(i);
			bones.push_back(s);
		}
	}
}

//----------------------------------------------------------------------------------
// Writes bones to D3DXSKININFO
//----------------------------------------------------------------------------------
void Mesh::SetBones(vector<SkinWeights>& bones)
{
	// Create basic skin info to copy bones, weights, and indices to
	DXASSERT(D3DXCreateSkinInfo(m_pMesh->GetNumVertices(),SkinnedVertexDecl,bones.size(),&m_pSkinInfo));
	// Copy over reference bone data
	for(int i=0;i<bones.size();i++){
		DXASSERT(m_pSkinInfo->SetBoneName(i,bones[i].name.c_str()));
		DXASSERT(m_pSkinInfo->SetBoneInfluence(i, bones[i].weights.size(),&bones[i].indices[0], &bones[i].weights[0]));
		DXASSERT(m_pSkinInfo->SetBoneOffsetMatrix(i,(D3DXMATRIX*)&bones[i].offset));
	}
}

//----------------------------------------------------------------------------------
// Is color vertex?
//----------------------------------------------------------------------------------
bool Mesh::UsingColorVertex()
{
    return m_pMesh->GetNumBytesPerVertex() == sizeof(ColVertex) || m_pMesh->GetNumBytesPerVertex() == sizeof(PackedVertex);
}

//----------------------------------------------------------------------------------
// Data mesh needs
//----------------------------------------------------------------------------------
void Mesh::CalcData(bool calcBox)
{
	if(m_AttribTable.size() == 0)
	{
		// Build default attribute table
		AttributeRange r;
		r.AttribId = 0;
		r.FaceCount = m_pMesh->GetNumFaces();
		r.FaceStart = 0;
		r.VertexStart = 0;
		//r.VertexOffset = 0;
		r.VertexCount = m_pMesh->GetNumVertices();
		m_AttribTable.push_back(r);
		m_pMesh->SetAttributeTable((D3DXATTRIBUTERANGE*)&r,1);
	}
	else {
		// Set attrib buffer
		DWORD *pBuffer;
		m_pMesh->LockAttributeBuffer(0,&pBuffer);
		for(int i=0;i<m_AttribTable.size();i++)
		{
			AttributeRange r = m_AttribTable[i];
			for(int j=r.FaceStart;j<r.FaceCount+r.FaceStart;j++)
				pBuffer[j] = r.AttribId;
		}
		m_pMesh->UnlockAttributeBuffer();

		// Sett attrib table
		m_pMesh->SetAttributeTable((D3DXATTRIBUTERANGE*)&m_AttribTable[0],m_AttribTable.size());
	}

	if(calcBox)
	{
		// TODO: See if this is slow, if so precalculate
		BYTE* vertices;
		m_pMesh->LockVertexBuffer(D3DLOCK_READONLY, (LPVOID*)&vertices);
        BYTE* IndexesBuffer;
	    m_pMesh->LockIndexBuffer(0, (LPVOID*)&IndexesBuffer);
		DXASSERT(UpdateBoundingInfo((D3DXVECTOR3*)&vertices[0],m_pMesh->GetNumVertices(),m_pMesh->GetNumBytesPerVertex(),IndexesBuffer,m_pMesh->GetOptions()&D3DXMESH_32BIT));
		m_pMesh->UnlockVertexBuffer();
        m_pMesh->UnlockIndexBuffer();
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Mesh::Clone(Mesh* newMesh, bool cloneData)
{	
	// Grap data that we must not clone
	vector<EditorVar> shVars = newMesh->m_SHOptions.EditorVars;
	// Copy general data
	*newMesh	= *this;
	// We copied materials so add references
	for(int i=0;i<m_Materials.size();i++)
		m_Materials[i]->AddRef();
	// Copy back instance-specific vars
	newMesh->m_SHOptions.EditorVars = shVars;

	GetSystemTime(&newMesh->m_TimeModified);
	GetSystemTime(&newMesh->m_TimeMoved);
	newMesh->m_pPRTMesh = NULL;

	// Copy physical data
	if(cloneData)
	{
		DWORD flags = D3DXMESH_MANAGED;
		if(m_pMesh->GetOptions() & D3DXMESH_32BIT)
			flags |= D3DXMESH_32BIT;
		D3DVERTEXELEMENT9 Declaration[128];
		m_pMesh->GetDeclaration(Declaration);
		DXASSERT(m_pMesh->CloneMesh(flags,Declaration,RenderWrap::dev,&newMesh->m_pMesh));
	}
	else
		newMesh->m_pMesh = 0;

	newMesh->UpdateBuffers();
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Mesh::SetMesh(LPD3DXMESH mesh, bool setAttribs)
{
	DWORD flags = D3DXMESH_MANAGED;
	if(mesh->GetOptions() & D3DXMESH_32BIT)
		flags |= D3DXMESH_32BIT;
	D3DVERTEXELEMENT9 Declaration[128];
	if(m_pMesh)
		m_pMesh->GetDeclaration(Declaration);
	else
		mesh->GetDeclaration(Declaration);
	SAFE_RELEASE(m_pMesh);
    DXASSERT(mesh->CloneMesh(flags,Declaration,RenderWrap::dev,&m_pMesh));
    if(setAttribs)
    {
	    FillAttributesFromMesh(mesh);
	    CalcData(true);
    }
	UpdateBuffers();
}

//----------------------------------------------------------------------------------
// Attempts to load prt by checking ParentFile_Data\MeshName.prt
//----------------------------------------------------------------------------------
void Mesh::LoadPRT(string prtFile)
{
	if(FileExists(prtFile))
	{
		SAFE_DELETE(m_pPRTMesh);
		m_pPRTMesh = new CPRTMesh;
		m_pPRTMesh->LoadCompPRTBufferFromFile( (WCHAR*)ToUnicode(prtFile).c_str() );

        if(!m_pPRTMesh->GetCompBuffer()->IsTexture())
        {
            if(m_pPRTMesh->GetCompBuffer()->GetNumSamples() != m_pMesh->GetNumVertices())
            {
                // Buf file is outdated, delete and return
                SAFE_DELETE(m_pPRTMesh);
                DeleteFile(prtFile.c_str());
                return;
            }
        }

		if(!m_pPRTMesh->GetCompBuffer()->IsTexture())
			m_pPRTMesh->SetMesh(RenderWrap::dev,m_pMesh); // Input mesh will be deleted

		m_pPRTMesh->ExtractCompressedDataForPRTShader(RenderWrap::dev);
		if(!m_pPRTMesh->GetCompBuffer()->IsTexture()){
			m_pMesh = m_pPRTMesh->GetMesh();
			m_pMesh->AddRef();
			m_pDeclaration	= VertexFormats::Instance()->FindFormat(sizeof(SHVertex))->decl;
		}
	}  
    else
    {
        // No PRT, update mesh state
        m_SHOptions.Enabled = false;
    }

}

//----------------------------------------------------------------------------------
// Desc: Compares two decls
//----------------------------------------------------------------------------------
bool CompareDecl(D3DVERTEXELEMENT9* Declaration, D3DVERTEXELEMENT9* inputDecl){
	bool valid = true;
	for(int i=0;i<128;i++){
		if(Declaration[i].Stream == 0xFF && inputDecl[i].Stream == 0xFF) // End marker on both
			break;
		else if(Declaration[i].Stream == 0xFF || inputDecl[i].Stream == 0xFF){ // One has hit end marker before other
			valid = false;
			break;
		}
		else if(memcmp((void*)&Declaration[i],(void*)&inputDecl[i],sizeof(D3DVERTEXELEMENT9)) != 0){ // Non-matching
			valid = false;
			break;
		}
	}
	return valid;
}

//----------------------------------------------------------------------------------
// Desc: Optimizes an index/vertex array
// Strips are always slower on Radeon 9700 so far. Same for OptimizedMesh sample.
// driver bug?
//----------------------------------------------------------------------------------
void Mesh::Optimize(bool bWeld)
{
	// FIXME: Why was I worried about this?
	//if(pSkinInfo)
	//	Error("you cannot optimize a skinned m_pMesh in this way");

	int facesStart = m_pMesh->GetNumFaces();
	int	vertsStart = m_pMesh->GetNumVertices();

	bool strips = false;
	// Fixup the m_pMesh
	LPD3DXBUFFER buf = NULL,pAdjacency = NULL;
	if(!pAdjacency){
		D3DXCreateBuffer(m_pMesh->GetNumFaces() * 3 * sizeof(DWORD),&pAdjacency);
		DXASSERT(m_pMesh->GenerateAdjacency(0,(DWORD*)pAdjacency->GetBufferPointer()));
	}

	D3DXCLEANTYPE flags = D3DXCLEAN_BOWTIES;
	if(m_pSkinInfo)
		flags = D3DXCLEAN_SKINNING;

	LPD3DXMESH pTempMesh;
	DXASSERT(D3DXCleanMesh(flags,m_pMesh,(DWORD*)pAdjacency->GetBufferPointer(),&pTempMesh,(DWORD*)pAdjacency->GetBufferPointer(),&buf));

	m_pMesh->Release();
	m_pMesh = pTempMesh;

	if(FAILED(D3DXValidMesh(m_pMesh,(DWORD*)pAdjacency->GetBufferPointer(),&buf))){
		Warning("Mesh has errors");
	}

	DWORD dwOptFlags = D3DXMESHOPT_VERTEXCACHE | D3DXMESHOPT_COMPACT | D3DXMESHOPT_ATTRSORT;
	if(strips)
		dwOptFlags = D3DXMESHOPT_STRIPREORDER | D3DXMESHOPT_COMPACT | D3DXMESHOPT_ATTRSORT;

	// Welds away duplicate or near-duplicate vertices
	if(bWeld)
	{
		D3DXWELDEPSILONS Epsilons;
		memset(&Epsilons, 0, sizeof(Epsilons));
		Epsilons.Position = 1.0e-6f;
		Epsilons.Normal = 0.01f;
		Epsilons.Tangent = 0.01f;
		Epsilons.BlendWeights = 0.01f;
		Epsilons.Texcoord[0] = Epsilons.Texcoord[1]  = 0.01f;
		Epsilons.Texcoord[2] = Epsilons.Texcoord[3]  = 0.01f;
		Epsilons.Texcoord[4] = Epsilons.Texcoord[5]  = 0.01f;
		Epsilons.Texcoord[6] = Epsilons.Texcoord[7]  = 0.01f;    
		LPD3DXBUFFER remap = NULL;
		DXASSERT(D3DXWeldVertices(m_pMesh,D3DXWELDEPSILONS_WELDPARTIALMATCHES,&Epsilons,(DWORD*)pAdjacency->GetBufferPointer(),(DWORD*)pAdjacency->GetBufferPointer(),NULL,&remap));
		
		// Remap skin info if present
		if(m_pSkinInfo)
			m_pSkinInfo->Remap(m_pMesh->GetNumVertices(),(DWORD*)remap->GetBufferPointer());
		SAFE_RELEASE(remap);
	}

	LPD3DXBUFFER remap;
	DXASSERT(m_pMesh->OptimizeInplace( dwOptFlags, (DWORD*)pAdjacency->GetBufferPointer(), NULL, NULL, &remap));
	// Remap skin info (again) if present
	if(m_pSkinInfo)
		m_pSkinInfo->Remap(m_pMesh->GetNumVertices(),(DWORD*)remap->GetBufferPointer());
	SAFE_RELEASE(remap);

	SAFE_RELEASE(pAdjacency);

	int facesEnd = m_pMesh->GetNumFaces();
	int	vertsEnd = m_pMesh->GetNumVertices();


	// Extract attribute table
	D3DXATTRIBUTERANGE	 attribTable[100]; 
	DWORD				 attribTableSize = 100;
	m_pMesh->GetAttributeTable(attribTable,&attribTableSize);
	m_AttribTable.clear();
	for(int i=0;i<attribTableSize;i++)
	{
		AttributeRange r;
		r.AttribId = attribTable[i].AttribId;
		r.FaceCount = attribTable[i].FaceCount;
		r.FaceStart = attribTable[i].FaceStart;
		r.VertexStart = attribTable[i].VertexStart;
		r.VertexCount = attribTable[i].VertexCount;
		//r.VertexOffset = 0; // Unused
		m_AttribTable.push_back(r);
	}

    //BYTE* vertices;
	//m_pMesh->LockVertexBuffer(D3DLOCK_READONLY, (LPVOID*)&vertices);
	//DXASSERT(UpdateBoundingInfo((D3DXVECTOR3*)&vertices[0],m_pMesh->GetNumVertices(),m_pMesh->GetNumBytesPerVertex(),(D3DXVECTOR3*)&m_LocalBox.min,(D3DXVECTOR3*)&m_LocalBox.max));
	//m_pMesh->UnlockVertexBuffer();

	// This code is very poor, don't use it
	// If you do, the test cases are
	// 1) A plane. 2) A box
	//if(FAILED(D3DXComputeNormals(m_pMesh,(DWORD*)pAdjacency->GetBufferPointer())))
	//	Error("D3DXComputeTangent went boom");

	//if(FAILED(D3DXComputeTangent(m_pMesh,0,0,D3DX_DEFAULT, FALSE,NULL)))//(DWORD*)pAdjacency->GetBufferPointer())))
	//	Error("D3DXComputeTangent went boom");
/*
	vector<WORD> indices;
	vector<Vertex> vertices;
	BBox box;
	GetData(indices,vertices,box);
	for(int i=0;i<vertices.size();i++){
		Vertex v= vertices[i];
		v.tan=v.tan;
	}
	box=box;*/
}


//----------------------------------------------------------------------------------
// Name: GenerateSkinnedMeshAndConvertToPools()
// Desc: This function uses the pSkinInfo of the mesh 
//       container to generate the desired drawable mesh and bone combination 
//       table.
//
//----------------------------------------------------------------------------------
void Mesh::GenerateSkinInfo()
{
	// Now build a skinned mesh!
	LPD3DXBUFFER pAdjacency;

	D3DXCreateBuffer(m_pMesh->GetNumFaces() * 3 * sizeof(DWORD),&pAdjacency);
	DXASSERT(m_pMesh->GenerateAdjacency(0,(DWORD*)pAdjacency->GetBufferPointer()));

	// Get palette size
	// First 9 constants are used for other data.  Each 4x3 matrix takes up 3 constants.
	// (96 - 9) /3 i.e. Maximum constant count - used constants 
	UINT MaxMatrices = 26; 
	m_NumPaletteEntries = MIN(MaxMatrices, m_pSkinInfo->GetNumBones());

    int numFaces = m_pMesh->GetNumFaces();
    int numVerts = m_pMesh->GetNumVertices();
	// TODO/FIXME ASAP: m_AttribTable must be loaded into mesh, or multi-materials won't work
	// for skinned items.

	//	HRESULT hr;
	LPD3DXMESH temp;
	if(FAILED(m_pSkinInfo->ConvertToIndexedBlendedMesh(m_pMesh, D3DXMESHOPT_VERTEXCACHE|D3DXMESH_MANAGED, m_NumPaletteEntries, (DWORD*)pAdjacency->GetBufferPointer(), (DWORD*)pAdjacency->GetBufferPointer(), NULL, NULL,             
		&m_NumInfl,&m_NumAttributeGroups, &m_pBoneComb, &temp)))
	{
		LPD3DXBUFFER  buf;
		D3DXValidMesh(m_pMesh,(DWORD*)pAdjacency->GetBufferPointer(),&buf);
		Warning("ConvertToIndexedBlendedMesh failed. Verts: %d. Faces: %d Mesh Report: %s",m_pMesh->GetNumFaces(), m_pMesh->GetNumVertices(),buf);
		SAFE_RELEASE(buf);
	}

	m_pMesh->Release();
	m_pMesh = temp;

	// the vertex shader is expecting to interpret the UBYTE4 as a D3DCOLOR, so update the type 
	//   NOTE: this cannot be done with CloneMesh, that would convert the UBYTE4 data to float and then to D3DCOLOR
	//          this is more of a "cast" operation
	D3DVERTEXELEMENT9 Declaration[128];
	m_pMesh->GetDeclaration(Declaration);
	LPD3DVERTEXELEMENT9 pDeclCur;
	pDeclCur = Declaration;
	while (pDeclCur->Stream != 0xff)
	{
		if ((pDeclCur->Usage == D3DDECLUSAGE_BLENDINDICES) && (pDeclCur->UsageIndex == 0))
			pDeclCur->Type = D3DDECLTYPE_D3DCOLOR;
		pDeclCur++;
	}
	m_pMesh->UpdateSemantics(Declaration);

	// Remap this to our SkinnedVertex format
	m_pDeclaration = VertexFormats::Instance()->FindFormat(sizeof(SkinnedVertex))->decl;
	LPD3DXMESH tempMesh;
    m_pMesh->CloneMesh(m_pMesh->GetOptions(),VertexFormats::Instance()->FindFormat(sizeof(SkinnedVertex))->element,RenderWrap::dev,&tempMesh);
	SAFE_RELEASE(m_pMesh);
	m_pMesh = tempMesh;

	// Make sure it's skinned in the format we expect
	/*if(!CompareDecl(Declaration,SkinnedVertexDecl)){
		int bytes =m_pMesh->GetNumBytesPerVertex();
		Error("Vertex type mismatch. This is an internal error and should be reported immediately.");
	}*/
	
	// Get the new attribute table, which has been split based on how many
	// bones we can fit into one vertex shader
	m_AttribTable.clear();
	D3DXATTRIBUTERANGE  attribTable[100]; 
	DWORD				attribTableSize;
	m_pMesh->GetAttributeTable(attribTable,&attribTableSize);
	for(int i=0;i<attribTableSize;i++){
		AttributeRange attrib;
		attrib.AttribId		= attribTable[i].AttribId;
		attrib.FaceCount	= attribTable[i].FaceCount;
		attrib.FaceStart	= attribTable[i].FaceStart;
		attrib.VertexCount	= attribTable[i].VertexCount;
		attrib.VertexStart	= attribTable[i].VertexStart;
		//attrib.VertexOffset = 0;
		m_AttribTable.push_back(attrib);
	}

	// get each of the bone offset matrices so that we don't need to get them later
	SAFE_DELETE_ARRAY(m_pBoneOffsetMatrices);
	m_pBoneOffsetMatrices = new D3DXMATRIX[m_pSkinInfo->GetNumBones()];
	for (int iBone = 0; iBone < m_pSkinInfo->GetNumBones(); iBone++)
	{
		m_pBoneOffsetMatrices[iBone] = *(m_pSkinInfo->GetBoneOffsetMatrix(iBone));

		// Set to this new pointer
		m_pSkinInfo->SetBoneOffsetMatrix(iBone,&m_pBoneOffsetMatrices[iBone]);
	}

	// Allocate space for blend matrices
	SAFE_DELETE_ARRAY(m_pBoneMatrices);
	m_pBoneMatrices  = new D3DXMATRIXA16[m_pSkinInfo->GetNumBones()];

	SAFE_RELEASE(pAdjacency);
}

//--------------------------------------------------------------------------------------
bool Mesh::UsingPRT(){ return m_SHOptions.Enabled && m_pPRTMesh && RenderDevice::Instance()->GetPRT(); }


//--------------------------------------------------------------------------------------
// Mesh vertex functions. These handle conversion to/from packed or special types

//--------------------------------------------------------------------------------------
Vertex Mesh::GetVertex(BYTE* vertex)
{
    Vertex v;
    memcpy(&v,vertex,sizeof(Vertex));
    return v;
}

//-------------------------------------------------------------------------------------
void Mesh::SetVertex(BYTE* vertex, Vertex& v)
{
    // Add the new vertex data
    memcpy(vertex,&v,sizeof(Vertex));
}


