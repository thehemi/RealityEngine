//======= (C) Copyright 2004, Artificial Studios. All rights reserved. ========
/// Type: OBJ Loading & Saving Module
//
//=============================================================================
#include "stdafx.h"
#include "obj.h"
#include "Collision.h"
#include <fstream>
using namespace std;


OBJSave::CoordSystem OBJSave::s_CoordSystem = OBJSave::D3D;
float				 OBJSave::s_Scale = 100.0f; // Meters to centimeters

OBJLoad::CoordSystem OBJLoad::s_CoordSystem = OBJLoad::MAYA;
float				 OBJLoad::s_Scale = 0.01f; // Centimeters to meters 

// Vertex declaration
D3DVERTEXELEMENT9 VERTEX_DECL[] =
{
    { 0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_POSITION, 0}, 
    { 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_NORMAL,   0}, 
    { 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_TEXCOORD, 0},
    D3DDECL_END()
};


void D3DToMax(Vector& in){
	float y = in.y;
	in.y = in.z;
	in.z = y;
}

void D3DToMaya(Vector& in){
	float x = in.x;
	in.x = in.z;
	in.z = x;
}

//-----------------------------------------------------------------------------
// Writes this mesh to the stream as another OBJ segment
//-----------------------------------------------------------------------------
void OBJSave::SaveMesh(wofstream& out, Mesh* mesh, string name, Matrix& tm, int& faceOffset)
{
	// Hacky, invert model temporarily for export
	if(s_CoordSystem == OBJSave::MAX || s_CoordSystem == OBJSave::MAYA)
	{
		MeshOps::Convert(mesh,MeshOps::Invert);
	}

	LPD3DXMESH pMesh = mesh->GetHardwareMesh();

	if(name.length() == 0)
		name = "Unlabeled Mesh";
	out << L"g " << ToUnicode(name) << endl;

	// Extract buffers from mesh
	int offset = 0; // For when using subsets
	BYTE* Verts;
	pMesh->LockVertexBuffer(D3DLOCK_READONLY, (LPVOID*)&Verts);

	WORD* indices;
	pMesh->LockIndexBuffer(D3DLOCK_READONLY, (LPVOID*)&indices);
	bool bDWORDIndices = pMesh->GetOptions() & D3DXMESH_32BIT;
	int stride = pMesh->GetNumBytesPerVertex();

    Matrix rot = tm.GetRotationMatrix();

	out << "# " << pMesh->GetNumVertices() << " vertices" << endl;
	for(int i=0;i<pMesh->GetNumVertices();i++)
	{
        Vertex v = mesh->GetVertex(&Verts[i*stride]);
		Vector pos = tm * v.position;
		if(s_CoordSystem == MAX)
			D3DToMax(pos);
		if(s_CoordSystem == MAYA)
			D3DToMaya(pos);

		// Write positions
		out << L"v " << pos.x*s_Scale << L" " << pos.y*s_Scale << L" " << pos.z*s_Scale << endl;
	}

	for(int i=0;i<pMesh->GetNumVertices();i++)
	{
        Vertex v = mesh->GetVertex(&Verts[i*stride]);
		Vector norm = rot * v.normal;
		if(s_CoordSystem == MAX)
			D3DToMax(norm);
		if(s_CoordSystem == MAYA)
        {
			D3DToMaya(norm);
            norm = -norm;
        }

		out << L"vn " << norm.x << L" " << norm.y << L" " << norm.z << endl;
	}

	for(int i=0;i<pMesh->GetNumVertices();i++)
	{
        Vertex v = mesh->GetVertex(&Verts[i*stride]);
		Vector2 uv = v.tex;
		if(s_CoordSystem == MAX || s_CoordSystem == MAYA)
			out << L"vt " << uv.x << L" " << -uv.y << endl; // Flip V
		else
			out << L"vt " << uv.x << L" " << uv.y << endl;
	}

	// Write all faces
	// NOTE: +1 because OBJ uses 1-based indices
	for(int j=0;j<pMesh->GetNumFaces();j++){
		if(bDWORDIndices){
			DWORD* dwIndices = (DWORD*)indices;
			out << L"f " << dwIndices[j*3 + 0]+1+faceOffset <<"/"<< dwIndices[j*3 + 0]+1 <<"/" << dwIndices[j*3 + 0]+1+faceOffset;
			out << L" " << dwIndices[j*3 + 1]+1+faceOffset <<"/"<< dwIndices[j*3 + 1]+1 <<"/" << dwIndices[j*3 + 1]+1+faceOffset;
			out << L" " << dwIndices[j*3 + 2]+1+faceOffset <<"/"<< dwIndices[j*3 + 2]+1 <<"/" << dwIndices[j*3 + 2]+1+faceOffset << endl;
		}
		else{
			out << L"f " << indices[j*3 + 0]+1+faceOffset <<"/"<< indices[j*3 + 0]+1+faceOffset <<"/" << indices[j*3 + 0]+1+faceOffset;
			out << L" "  << indices[j*3 + 1]+1+faceOffset <<"/"<< indices[j*3 + 1]+1+faceOffset <<"/" << indices[j*3 + 1]+1+faceOffset;
			out << L" "  << indices[j*3 + 2]+1+faceOffset <<"/"<< indices[j*3 + 2]+1+faceOffset <<"/" << indices[j*3 + 2]+1+faceOffset << endl;
		}
	}
    faceOffset += pMesh->GetNumVertices();
	pMesh->UnlockVertexBuffer();
	pMesh->UnlockIndexBuffer();

	// Hacky, invert model temporarily after export
	if(s_CoordSystem == OBJSave::MAX || s_CoordSystem == OBJSave::MAYA)
	{
		MeshOps::Convert(mesh,MeshOps::Invert);
	}
}

//-----------------------------------------------------------------------------
// Saves .OBJ files
//-----------------------------------------------------------------------------
bool OBJSave::Save(string name, vector<Actor*>& items)
{
	wofstream out( name.c_str() );
	out << L"# Reality Engine .OBJ Export -- " << ToUnicode(name) << endl;

    int faceOffset = 0;
	for(int k=0;k<items.size();k++)
	{
		vector<ModelFrame*> frames;
		if(items[k]->MyModel)
			items[k]->MyModel->m_pFrameRoot->EnumerateMeshes(frames);
		for(int i=0;i<frames.size();i++)
			SaveMesh(out,frames[i]->GetMesh(),frames[i]->Name,frames[i]->CombinedTransformationMatrix,faceOffset);
	}

	out.close();
	return true;
}


//-----------------------------------------------------------------------------
// Saves .OBJ files
//-----------------------------------------------------------------------------
bool OBJSave::SaveModel(string name, StaticModel* m)
{
	wofstream out( name.c_str() );
	out << L"# Reality Engine .OBJ Export -- " << ToUnicode(name) << endl;

    int faceOffset = 0;
	vector<ModelFrame*> frames;
	m->m_pFrameRoot->EnumerateMeshes(frames);
	for(int i=0;i<frames.size();i++)
		SaveMesh(out,frames[i]->GetMesh(),frames[i]->Name,frames[i]->CombinedTransformationMatrix,faceOffset);

	out.close();
	return true;
}



//-----------------------------------------------------------------------------
// Loads .OBJ files
//-----------------------------------------------------------------------------
bool OBJLoad::LoadModel(string name, StaticModel* m)
{
	CMeshLoader ml;
	if(FAILED(ml.Create(RenderWrap::dev,ToUnicode(name).c_str())))
		return false;

	Model* model = (Model*)m;
	SAFE_DELETE(model->m_pFrameRoot);

	model->m_pFrameRoot = new ModelFrame;
	Mesh* newMesh = new Mesh;
	newMesh->FillAttributesFromMesh(ml.GetMesh());
	

	// Expand mesh to standard vertex decl, so it can hold normals and tangents
	LPD3DXMESH pTemp;
	DXASSERT( ml.GetMesh()->CloneMesh( ml.GetMesh()->GetOptions(), VertexFormats::Instance()->FindFormat(sizeof(Vertex))->element, RenderWrap::dev, &pTemp ) );
	D3DXComputeNormals(pTemp,0);
	D3DXComputeTangent(pTemp,0,0,0,TRUE,0);

	// Copy mesh
	newMesh->SetMesh(pTemp);

	newMesh->m_pDeclaration = VertexFormats::Instance()->FindFormat(sizeof(Vertex))->decl;
	SAFE_RELEASE(pTemp);

    if(s_CoordSystem == OBJLoad::MAX || s_CoordSystem == OBJLoad::MAYA)
	{
		MeshOps::Convert(newMesh,MeshOps::Invert);
	}

	model->m_pFrameRoot->SetMesh(newMesh);

	// Collision mesh
	model->m_pFrameRoot->collisionMesh = new CollisionMesh;
	model->m_pFrameRoot->collisionMesh->Initialize(newMesh);

	model->InitAfterLoad();
	if(model->m_pFrameRoot)
		model->m_pFrameRoot->UpdateMatrices(model->m_RootTransform);

	ml.Destroy();
	return true;
}

//--------------------------------------------------------------------------------------
CMeshLoader::CMeshLoader()
{
    m_pd3dDevice = NULL;  
    m_pMesh = NULL;  

    ZeroMemory( m_strMediaDir, sizeof(m_strMediaDir) );
}


//--------------------------------------------------------------------------------------
CMeshLoader::~CMeshLoader()
{
    Destroy();
}


//--------------------------------------------------------------------------------------
void CMeshLoader::Destroy()
{
    for( int iMaterial=0; iMaterial < m_Materials.GetSize(); iMaterial++ )
    {
        DXMaterial* pMaterial = m_Materials.GetAt( iMaterial );
        
        // Avoid releasing the same texture twice
        for( int x=iMaterial+1; x < m_Materials.GetSize(); x++ )
        {
            DXMaterial* pCur = m_Materials.GetAt( x );
            if( pCur->pTexture == pMaterial->pTexture )
                pCur->pTexture = NULL;
        }

        SAFE_RELEASE( pMaterial->pTexture );
        SAFE_DELETE( pMaterial );
    }

    m_Materials.RemoveAll();
    m_Vertices.RemoveAll();
    m_Indices.RemoveAll();
    m_Attributes.RemoveAll();

    SAFE_RELEASE( m_pMesh );
    m_pd3dDevice = NULL;
}


//--------------------------------------------------------------------------------------
HRESULT CMeshLoader::Create( IDirect3DDevice9* pd3dDevice, const WCHAR* strFilename )
{
    HRESULT hr;
    WCHAR str[ MAX_PATH ] = {0};

    // Start clean
    Destroy();

    // Store the device pointer
    m_pd3dDevice = pd3dDevice;

    // Load the vertex buffer, index buffer, and subset information from a file. In this case, 
    // an .obj file was chosen for simplicity, but it's meant to illustrate that ID3DXMesh objects
    // can be filled from any mesh file format once the necessary data is extracted from file.
    V_RETURN( LoadGeometryFromOBJ( strFilename ) );

    // Set the current directory based on where the mesh was found
    WCHAR wstrOldDir[MAX_PATH] = {0};
    GetCurrentDirectoryW( MAX_PATH, wstrOldDir );
    SetCurrentDirectoryW( m_strMediaDir );

    // Load material textures
    for( int iMaterial=0; iMaterial < m_Materials.GetSize(); iMaterial++ )
    {
        DXMaterial* pMaterial = m_Materials.GetAt( iMaterial );
        if( pMaterial->strTexture[0] )
        {   
            // Avoid loading the same texture twice
            bool bFound = false;
            for( int x=0; x < iMaterial; x++ )
            {
                DXMaterial* pCur = m_Materials.GetAt( x );
                if( 0 == wcscmp( pCur->strTexture, pMaterial->strTexture ) )
                {
                    bFound = true;
                    pMaterial->pTexture = pCur->pTexture;
                    break;
                }
            }

            // Not found, load the texture
            if( !bFound )
            {
                V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, pMaterial->strTexture ) );
                V_RETURN( D3DXCreateTextureFromFileW( pd3dDevice, pMaterial->strTexture, 
                                                    &(pMaterial->pTexture) ) );
            }
        }
    }

    // Restore the original current directory
    SetCurrentDirectoryW( wstrOldDir );

    // Create the encapsulated mesh
    ID3DXMesh* pMesh = NULL;
    V_RETURN( D3DXCreateMesh( m_Indices.GetSize() / 3, m_Vertices.GetSize(), 
                              D3DXMESH_MANAGED | D3DXMESH_32BIT, VERTEX_DECL, 
                              pd3dDevice, &pMesh ) ); 
    
    // Copy the vertex data
    VERTEX* pVertex;
    V_RETURN( pMesh->LockVertexBuffer( 0, (void**) &pVertex ) );
    memcpy( pVertex, m_Vertices.GetData(), m_Vertices.GetSize() * sizeof(VERTEX) );
    pMesh->UnlockVertexBuffer();
    m_Vertices.RemoveAll();
    
    // Copy the index data
    DWORD* pIndex;
    V_RETURN( pMesh->LockIndexBuffer( 0, (void**) &pIndex ) );
    memcpy( pIndex, m_Indices.GetData(), m_Indices.GetSize() * sizeof(DWORD) );
    pMesh->UnlockIndexBuffer();
    m_Indices.RemoveAll();
    
    // Copy the attribute data
    DWORD* pSubset;
    V_RETURN( pMesh->LockAttributeBuffer( 0, &pSubset ) );
    memcpy( pSubset, m_Attributes.GetData(), m_Attributes.GetSize() * sizeof(DWORD) );
    pMesh->UnlockAttributeBuffer();
    m_Attributes.RemoveAll();

    // Reorder the vertices according to subset and optimize the mesh for this graphics 
    // card's vertex cache. When rendering the mesh's triangle list the vertices will 
    // cache hit more often so it won't have to re-execute the vertex shader.
    DWORD* aAdjacency = new DWORD[pMesh->GetNumFaces() * 3];
    if( aAdjacency == NULL )
        return E_OUTOFMEMORY;

    V( pMesh->ConvertPointRepsToAdjacency(NULL, aAdjacency) );
    V( pMesh->OptimizeInplace(D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_VERTEXCACHE, aAdjacency, NULL, NULL, NULL) );
    
    SAFE_DELETE_ARRAY( aAdjacency );
    m_pMesh = pMesh;

    return S_OK;
}


//--------------------------------------------------------------------------------------
HRESULT CMeshLoader::LoadGeometryFromOBJ( const WCHAR* strFileName )
{
    WCHAR strMaterialFilename[MAX_PATH] = {0};
    char str[MAX_PATH];
    HRESULT hr;

    // Find the file
    WideCharToMultiByte( CP_ACP, 0, strFileName, -1, str, MAX_PATH, NULL, NULL );

    // Store the directory where the mesh was found
    wcsncpy( m_strMediaDir, strFileName, MAX_PATH-1 );
    WCHAR* pch = wcsrchr( m_strMediaDir, L'\\' );
    if( pch )
        *pch = NULL;

    // Create temporary storage for the input data. Once the data has been loaded into
    // a reasonable format we can create a D3DXMesh object and load it with the mesh data.
    CGrowableArray< D3DXVECTOR3 > Positions;
    CGrowableArray< D3DXVECTOR2 > TexCoords;
    CGrowableArray< D3DXVECTOR3 > Normals;
    
    // The first subset uses the default material
    DXMaterial* pMaterial = new DXMaterial();
    if( pMaterial == NULL )
        return E_OUTOFMEMORY;

    InitMaterial( pMaterial );
    wcsncpy( pMaterial->strName, L"default", MAX_PATH-1 );
    m_Materials.Add( pMaterial );

    DWORD dwCurSubset = 0;

    // File input
    WCHAR strCommand[256] = {0};
    wifstream InFile( str );
    if( !InFile )
	{
        SeriousWarning( "wifstream::open", E_FAIL );
		return E_FAIL;
	}
    
    for(;;)
    {
        InFile >> strCommand;
        if( !InFile )
            break;
        
        if( 0 == wcscmp( strCommand, L"#" ) )
        {
            // Comment
        }
        else if( 0 == wcscmp( strCommand, L"v" ) )
        {
            // Vertex Position
            float x, y, z;

			if(OBJLoad::s_CoordSystem == OBJLoad::MAX)
				InFile >> x >> z >> y;
			if(OBJLoad::s_CoordSystem == OBJLoad::MAYA)
				InFile >> z >> y >> x;
			if(OBJLoad::s_CoordSystem == OBJLoad::D3D)
				InFile >> x >> y >> z;

			// Scale position and add it
			Positions.Add( OBJLoad::s_Scale*D3DXVECTOR3( x, y, z ) ); 
        }
        else if( 0 == wcscmp( strCommand, L"vt" ) )
        {
            // Vertex TexCoord
            float u, v;

			if(OBJLoad::s_CoordSystem == OBJLoad::MAX)
			{
				InFile >> u >> v;
				v = -v; // Flip V
			}
			else
				InFile >> u >> v;
            TexCoords.Add( D3DXVECTOR2( u, v ) );
        }
        else if( 0 == wcscmp( strCommand, L"vn" ) )
        {
            // Vertex Normal
            float x, y, z;

			if(OBJLoad::s_CoordSystem == OBJLoad::MAX)
				InFile >> x >> z >> y;
			if(OBJLoad::s_CoordSystem == OBJLoad::MAYA)
				InFile >> z >> y >> x;
			if(OBJLoad::s_CoordSystem == OBJLoad::D3D)
				InFile >> x >> y >> z;

            Normals.Add( D3DXVECTOR3( x, y, z ) );
        }
        else if( 0 == wcscmp( strCommand, L"f" ) )
        {
            // Face
            UINT iPosition, iTexCoord, iNormal;
            VERTEX vertex;
            
            for( UINT iFace=0; iFace < 3; iFace++ )
            {
                ZeroMemory( &vertex, sizeof(VERTEX) );

                // OBJ format uses 1-based arrays
                InFile >> iPosition;
                vertex.position = Positions[ iPosition-1 ];
                
                if( '/' == InFile.get() )
                {
                    // Optional texture coordinate
                    InFile >> iTexCoord;
                    vertex.texcoord = TexCoords[ iTexCoord-1 ];

                    if( '/' == InFile.get() )
                    {
                        // Optional vertex normal
                        InFile >> iNormal;
                        vertex.normal = Normals[ iNormal-1 ];
                    }
                }

                // If a duplicate vertex doesn't exist, add this vertex to the Vertices
                // list. Store the index in the Indices array. The Vertices and Indices
                // lists will eventually become the Vertex Buffer and Index Buffer for
                // the mesh.
                DWORD index = AddVertex( iPosition, &vertex );
                m_Indices.Add( index );
            }
            m_Attributes.Add( dwCurSubset );
        }
        else if( 0 == wcscmp( strCommand, L"mtllib" ) )
        {
            // DXMaterial library
            InFile >> strMaterialFilename;
        }
        else if( 0 == wcscmp( strCommand, L"usemtl" ) )
        {
            // DXMaterial
            WCHAR strName[MAX_PATH] = {0};
            InFile >> strName;
            
            bool bFound = false;
            for( int iMaterial=0; iMaterial < m_Materials.GetSize(); iMaterial++ )
            {
                DXMaterial* pCurMaterial = m_Materials.GetAt( iMaterial );
                if( 0 == wcscmp( pCurMaterial->strName, strName ) )
                {
                    bFound = true;
                    dwCurSubset = iMaterial;
                    break;
                }  
            }

            if( !bFound )
            {
                pMaterial = new DXMaterial();
                if( pMaterial == NULL )
                    return E_OUTOFMEMORY;

                dwCurSubset = m_Materials.GetSize();

                InitMaterial( pMaterial );
                wcsncpy( pMaterial->strName, strName, MAX_PATH-1 );

                m_Materials.Add( pMaterial );
            }
        }
        else
        {
            // Unimplemented or unrecognized command
        }

       // InFile.ignore( 1000, '\n' );
    }

    // Cleanup
    InFile.close();
    DeleteCache();

    // If an associated material file was found, read that in as well.
    if( strMaterialFilename[0] )
    {
        if(FAILED( LoadMaterialsFromMTL( strMaterialFilename ) ))
			Warning("Material load failed for .obj file");
    }

    return S_OK;
}


//--------------------------------------------------------------------------------------
DWORD CMeshLoader::AddVertex( UINT hash, VERTEX* pVertex )
{
    // If this vertex doesn't already exist in the Vertices list, create a new entry.
    // Add the index of the vertex to the Indices list.
    bool bFoundInList = false;
    DWORD index = 0;

    // Since it's very slow to check every element in the vertex list, a hashtable stores
    // vertex indices according to the vertex position's index as reported by the OBJ file
    if( (UINT)m_VertexCache.GetSize() > hash )
    {
        CacheEntry* pEntry = m_VertexCache.GetAt( hash );
        while( pEntry != NULL )
        {
            VERTEX* pCacheVertex = m_Vertices.GetData() + pEntry->index;

            // If this vertex is identical to the vertex already in the list, simply
            // point the index buffer to the existing vertex
            if( 0 == memcmp( pVertex, pCacheVertex, sizeof(VERTEX) ) )
            {
                bFoundInList = true;
                index = pEntry->index;
                break;
            }

            pEntry = pEntry->pNext;
        }
    }

    // Vertex was not found in the list. Create a new entry, both within the Vertices list
    // and also within the hashtable cache
    if( !bFoundInList )
    {
        // Add to the Vertices list
        index = m_Vertices.GetSize();
        m_Vertices.Add( *pVertex );

        // Add this to the hashtable
        CacheEntry* pNewEntry = new CacheEntry;
        if( pNewEntry == NULL )
            return E_OUTOFMEMORY;

        pNewEntry->index = index;
        pNewEntry->pNext = NULL;

        // Grow the cache if needed
        while( (UINT)m_VertexCache.GetSize() <= hash )
        {
            m_VertexCache.Add( NULL );
        }

        // Add to the end of the linked list
        CacheEntry* pCurEntry = m_VertexCache.GetAt( hash );
        if( pCurEntry == NULL )
        {
            // This is the head element
            m_VertexCache.SetAt( hash, pNewEntry );
        }
        else
        {
            // Find the tail
            while( pCurEntry->pNext != NULL )
            {
                pCurEntry = pCurEntry->pNext;
            }

            pCurEntry->pNext = pNewEntry;
        } 
    }

    return index;
}


//--------------------------------------------------------------------------------------
void CMeshLoader::DeleteCache()
{
    // Iterate through all the elements in the cache and subsequent linked lists
    for( int i=0; i < m_VertexCache.GetSize(); i++ )
    {
        CacheEntry* pEntry = m_VertexCache.GetAt( i );
        while( pEntry != NULL )
        {
            CacheEntry* pNext = pEntry->pNext;
            SAFE_DELETE( pEntry );
            pEntry = pNext;
        }
    }

    m_VertexCache.RemoveAll();
}


//--------------------------------------------------------------------------------------
HRESULT CMeshLoader::LoadMaterialsFromMTL( const WCHAR* strFileName )
{
    HRESULT hr;

    // Set the current directory based on where the mesh was found
    WCHAR wstrOldDir[MAX_PATH] = {0};
    GetCurrentDirectoryW( MAX_PATH, wstrOldDir );
    SetCurrentDirectoryW( m_strMediaDir );

    // Find the file
    char cstrPath[MAX_PATH];
    WideCharToMultiByte( CP_ACP, 0, strFileName, -1, cstrPath, MAX_PATH, NULL, NULL );

    // File input
    WCHAR strCommand[256] = {0};
    wifstream InFile( cstrPath );
    if( !InFile )
	{
        Warning( "wifstream::open", E_FAIL );
		return E_FAIL;
	}
    
    // Restore the original current directory
    SetCurrentDirectoryW( wstrOldDir );

    DXMaterial* pMaterial = NULL;

    for(;;)
    {
        InFile >> strCommand;
        if( !InFile )
            break;
        
        if( 0 == wcscmp( strCommand, L"newmtl" ) )
        {
            // Switching active materials
            WCHAR strName[MAX_PATH] = {0};
            InFile >> strName;
            
            pMaterial = NULL;
            for( int i=0; i < m_Materials.GetSize(); i++ )
            {
                DXMaterial* pCurMaterial = m_Materials.GetAt( i );
                if( 0 == wcscmp( pCurMaterial->strName, strName ) )
                {
                    pMaterial = pCurMaterial;
                    break;
                }  
            }
        }
        
        // The rest of the commands rely on an active material
        if( pMaterial == NULL )
            continue;

        if( 0 == wcscmp( strCommand, L"#" ) )
        {
            // Comment
        }
        else if( 0 == wcscmp( strCommand, L"Ka" ) )
        {
            // Ambient color
            float r, g, b;
            InFile >> r >> g >> b;
            pMaterial->vAmbient = D3DXVECTOR3(r, g, b); 
        }
        else if( 0 == wcscmp( strCommand, L"Kd" ) )
        {
            // Diffuse color
            float r, g, b;
            InFile >> r >> g >> b;
            pMaterial->vDiffuse = D3DXVECTOR3(r, g, b); 
        }
        else if( 0 == wcscmp( strCommand, L"Ks" ) )
        {
            // Specular color
            float r, g, b;
            InFile >> r >> g >> b;
            pMaterial->vSpecular = D3DXVECTOR3(r, g, b); 
        }
        else if( 0 == wcscmp( strCommand, L"d" ) ||
                0 == wcscmp( strCommand, L"Tr" ) )
        {
            // Alpha
            InFile >> pMaterial->fAlpha;
        }
        else if( 0 == wcscmp( strCommand, L"Ns" ) )
        {
            // Shininess
            int nShininess;
            InFile >> nShininess;
            pMaterial->nShininess = nShininess;
        }
        else if( 0 == wcscmp( strCommand, L"illum" ) )
        {
            // Specular on/off
            int illumination;
            InFile >> illumination;
            pMaterial->bSpecular = (illumination == 2);
        }
        else if( 0 == wcscmp( strCommand, L"map_Kd" ) )
        {
            // Texture
            InFile >> pMaterial->strTexture;
        }
        
        else
        {
            // Unimplemented or unrecognized command
        }
       
        InFile.ignore( 1000, L'\n' );
    }

    InFile.close();

    return S_OK;
}


//--------------------------------------------------------------------------------------
void CMeshLoader::InitMaterial( DXMaterial* pMaterial )
{
    ZeroMemory( pMaterial, sizeof(DXMaterial) );

    pMaterial->vAmbient = D3DXVECTOR3(0.2f, 0.2f, 0.2f);
    pMaterial->vDiffuse = D3DXVECTOR3(0.8f, 0.8f, 0.8f);
    pMaterial->vSpecular = D3DXVECTOR3(1.0f, 1.0f, 1.0f);
    pMaterial->nShininess = 0;
    pMaterial->fAlpha = 1.0f;
    pMaterial->bSpecular = false;
    pMaterial->pTexture = NULL;
}
