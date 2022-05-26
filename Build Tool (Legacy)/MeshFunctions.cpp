//----------------------------------------------------------------------------------
// Specific Mesh operations, mainly D3DX
//
// TODO: Compare NVIDIA tangent generation with D3DX
//
//
//----------------------------------------------------------------------------------
#include <stdafx.h>
LPDIRECT3D9 g_pD3D = NULL;
LPDIRECT3DDEVICE9 g_pd3dDevice = NULL;

//----------------------------------------------------------------------------------
// Desc: Standard vertex shader declaration
//----------------------------------------------------------------------------------
D3DVERTEXELEMENT9 decl[] =
{
	// First stream is first mesh
	{ 0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_POSITION, 0}, 
	{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_NORMAL,   0}, 
	{ 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_TEXCOORD, 0}, 
	{ 0, 32, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_TANGENT, 0}, 
#ifdef VERTEX_COLORS
	{ 0, 44, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_COLOR, 0}, 
#endif
	D3DDECL_END()
};

//----------------------------------------------------------------------------------
// Desc: Input vertex format (no tangents)
//----------------------------------------------------------------------------------
D3DVERTEXELEMENT9 inputDecl[] =
{
	// First stream is first mesh
	{ 0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_POSITION, 0}, 
	{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_NORMAL,   0}, 
	{ 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_TEXCOORD, 0}, 
	D3DDECL_END()
};


//----------------------------------------------------------------------------------
// Desc: Input vertex format (no tangents)
//----------------------------------------------------------------------------------
D3DVERTEXELEMENT9 inputDeclFull[] =
{
	// First stream is first mesh
	{ 0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_POSITION, 0}, 
	{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_NORMAL,   0}, 
	{ 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_TEXCOORD, 0}, 
	{ 0, 32, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_TANGENT, 0}, 
	D3DDECL_END()
};

//----------------------------------------------------------------------------------
// Desc: Input vertex format (no tangents, no textures)
//----------------------------------------------------------------------------------
D3DVERTEXELEMENT9 inputDeclNoTexCoords[] =
{
	// First stream is first mesh
	{ 0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_POSITION, 0}, 
	{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_NORMAL,   0}, 
	D3DDECL_END()
};

struct InputVertex{
	Vector p;
	Vector n;
	Vector2 t;
};

struct InputVertexNoTexCoords{
	Vector p;
	Vector n;
};

//----------------------------------------------------------------------------------
// Desc: Initializes skeleton D3D needed for D3DX functions
//----------------------------------------------------------------------------------
HRESULT InitD3D(HWND hWnd)
{
	HRESULT hr;
	//if(!hWnd){
	//	Error("No window handle. WTF?");
	//	return E_FAIL;
	//}

	if( NULL == ( g_pD3D = Direct3DCreate9( D3D_SDK_VERSION ) ) ){
		Error("Direct3DCreate9 failed. Make sure you have DirectX 9.0 or later installed");
		return E_FAIL;
	}

	D3DDISPLAYMODE Mode;
	if( FAILED( hr=g_pD3D->GetAdapterDisplayMode( 0, &Mode ) ) ){
		Error("GetAdapterDisplayMode failed with '%s'. Make sure you have DirectX 9.0 or later installed",DXGetErrorString9(hr));
		return E_FAIL;
	}

	D3DPRESENT_PARAMETERS pp;
    memset(&pp, 0x00, sizeof(D3DPRESENT_PARAMETERS));

    pp.BackBufferWidth  = 1;
    pp.BackBufferHeight = 1;
    pp.BackBufferFormat = Mode.Format;
    pp.BackBufferCount  = 1;
    pp.SwapEffect       = D3DSWAPEFFECT_COPY;
    pp.Windowed         = TRUE;

    if(FAILED(hr = g_pD3D->CreateDevice(0, D3DDEVTYPE_HAL, GetDesktopWindow(), D3DCREATE_SOFTWARE_VERTEXPROCESSING|D3DCREATE_MULTITHREADED, &pp, &g_pd3dDevice)))
    {
        Error("CreateDevice failed. Error = '%s' hWnd = %x",DXGetErrorString9(hr),hWnd);
        return E_FAIL;
    }


	return S_OK;
}

//----------------------------------------------------------------------------------
// Desc:
//----------------------------------------------------------------------------------
void CleanupD3D()
{
	SAFE_RELEASE(g_pd3dDevice);
	SAFE_RELEASE(g_pD3D);
}

//----------------------------------------------------------------------------------
// Desc:
//----------------------------------------------------------------------------------
int FindGlobalMaterial(Material& mat, vector<Material>& global){
	for(int i=0;i<global.size();i++){
		if(mat.name == global[i].name)
			return i;
	}
	//Error("Couldn't resolve material '%s'. This should never happen",mat.name.c_str());
	return 0;
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
// Desc: 
//----------------------------------------------------------------------------------
Mesh::~Mesh(){
	SAFE_RELEASE(mesh);
	SAFE_RELEASE(pAdjacency);
	SAFE_RELEASE(pSkinInfo);
}

//----------------------------------------------------------------------------------
// Desc: Creates a mesh from basics
//----------------------------------------------------------------------------------
void Mesh::Create(vector<Vertex>& vertices, vector<WORD>& indices, vector<Material> inMaterials){
	materials = inMaterials;

	SAFE_RELEASE(mesh);
	HRESULT hr = D3DXCreateMesh(indices.size()/3, vertices.size(), D3DXMESH_SYSTEMMEM , decl,g_pd3dDevice, &mesh);
	if(FAILED(hr))
		Error("D3DXCreateMesh failed with %d indices and %d vertices",indices.size(),vertices.size());

	// Fill the mesh!
	Vertex* VerticesBuffer;
	mesh->LockVertexBuffer(0, (LPVOID*)&VerticesBuffer);
	memcpy(VerticesBuffer, &vertices[0], vertices.size() * sizeof(Vertex));
	mesh->UnlockVertexBuffer();

	WORD* IndexesBuffer;
	mesh->LockIndexBuffer(0, (LPVOID*)&IndexesBuffer);
	for(int i=0;i<indices.size();i++) IndexesBuffer[i] = indices[i];
	mesh->UnlockIndexBuffer();
}


//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Mesh::SetMesh(LPD3DXMESH m)
{
	SAFE_RELEASE(mesh);
	SAFE_RELEASE(pAdjacency);
	SAFE_RELEASE(pSkinInfo);

	DXASSERT(m->CloneMesh(D3DXMESH_SYSTEMMEM|D3DXMESH_DYNAMIC,decl,g_pd3dDevice,&mesh));
	D3DXCreateBuffer(mesh->GetNumFaces() * 3 * sizeof(DWORD),&pAdjacency);
	DXASSERT(mesh->GenerateAdjacency(0,(DWORD*)pAdjacency->GetBufferPointer()));
}


//----------------------------------------------------------------------------------
// Desc: Parses a mesh into the format we need it
// Removes scaling (because non-uniform scaling causes all number of issues)
//
// Takes materials so that it can remap attribute ids to the global array
//
// TODO: We only really need to do this for models, which never go through the
// lit+optimized pools phase
//
// FIXME: AttribId might not map to material indexes. Depends on if skinning sections use new ids?
//----------------------------------------------------------------------------------
void Mesh::Create(string name, LPD3DXMESH inMesh, LPD3DXSKININFO inSkinInfo, LPD3DXBUFFER inAdjacency, vector<Material>& inMaterials, Matrix tm, bool optimize){
	SAFE_RELEASE(mesh);
	SAFE_RELEASE(pAdjacency);
	SAFE_RELEASE(pSkinInfo);

	// Copy input data
	materials = inMaterials;
	if(inSkinInfo)
		inSkinInfo->Clone(&pSkinInfo);

	if(!inAdjacency){
		D3DXCreateBuffer(inMesh->GetNumFaces() * 3 * sizeof(DWORD),&pAdjacency);
		DXASSERT(inMesh->GenerateAdjacency(0,(DWORD*)pAdjacency->GetBufferPointer()));
	}
	else{
		D3DXCreateBuffer(inAdjacency->GetBufferSize(),&pAdjacency);
		memcpy(pAdjacency->GetBufferPointer(),inAdjacency->GetBufferPointer(),inAdjacency->GetBufferSize());
	}

	// Scrub down mesh (probably not needed. only use when debug testing)
/*	LPD3DXBUFFER  buf;
	LPD3DXMESH pTempMesh;
	DXASSERT(D3DXCleanMesh(inMesh,(DWORD*)pAdjacency->GetBufferPointer(),&pTempMesh,(DWORD*)pAdjacency->GetBufferPointer(),&buf));
	inMesh->Release();
	inMesh = pTempMesh;
	DXASSERT(D3DXValidMesh(inMesh,(DWORD*)pAdjacency->GetBufferPointer(),&buf));
*/ 
	// Confirm mesh format matches
	bool hasTexCoords = true;
	D3DVERTEXELEMENT9 Declaration[128];
	inMesh->GetDeclaration(Declaration);
	if(!CompareDecl(Declaration,inputDecl) && !CompareDecl(Declaration,inputDeclFull)){
		// Not even a textureless decl...
		if(!CompareDecl(Declaration,inputDeclNoTexCoords)){
			Warning("!!SERIOUS!! Declaration for mesh is unexpected format.");
		}
		else{
			// It's a decl minus texture coords.
			hasTexCoords = false;
			Warning("Mesh '%s' lacks texture coordinates, it will be completely unshaded.",name.c_str());
		}
	}

	// Clone input mesh and add support for tangents and texture coordinates
	DXASSERT(inMesh->CloneMesh(D3DXMESH_SYSTEMMEM|D3DXMESH_DYNAMIC,decl,g_pd3dDevice,&mesh));
	
	int test = GetNumSubsets();
	// Generate tangents and normals!!
	// Get a remap buffer in case we have bone skinning info to move about
	vector<WORD> indices;
	vector<Vertex> vertices;
	BBox box;
	GetData(indices,vertices,box);
	// Use this as an opportunity to bake in scaling
	// (We remove scaling from the matrix elsewhere)
	for(int i=0;i<vertices.size();i++){
		vertices[i].p =  tm.GetScaleMatrix() * vertices[i].p;
		// Don't use the below, it's definitely wrong (staircase in mansion)
		// Put in world, bring back to local, but without scaling
		//vertices[i].p = tm * vertices[i].p;
		//vertices[i].p = scale * vertices[i].p;
	}
	vector<DWORD> dRemap;
	GenerateTangentsAndNormals(vertices,indices,dRemap);
	if(pSkinInfo)
		pSkinInfo->Remap(dRemap.size(),(DWORD*)&dRemap[0]);

	Vertex* VerticesBuffer;
	mesh->LockVertexBuffer(D3DLOCK_DISCARD, (LPVOID*)&VerticesBuffer);
	memcpy(VerticesBuffer, &vertices[0], vertices.size() * sizeof(Vertex));
	mesh->UnlockVertexBuffer();

	WORD* IndexesBuffer;
	mesh->LockIndexBuffer(D3DLOCK_DISCARD, (LPVOID*)&IndexesBuffer);
	for(int i=0;i<indices.size();i++) IndexesBuffer[i] = indices[i];
	mesh->UnlockIndexBuffer();

	//DXASSERT(D3DXComputeNormals(mesh,NULL));
	//DXASSERT(D3DXComputeTangent(mesh,0,0,D3DX_DEFAULT,TRUE,NULL));

	// Optimize the mesh for performance
	// NOTE: We really only need to weld models, not scene data (as it's welded later)
	if(optimize)
		Optimize(true);

	// Change attribute IDs to global material indices
	// TODO: Must alter face table too..
/*	mesh->GetAttributeTable(attribTable,&attribTableSize);
	for(int i=0;i<attribTableSize;i++){
		attribTable[i].AttribId = FindGlobalMaterial(localMaterials[attribTable[i].AttribId],globalMaterials);
	}
	mesh->SetAttributeTable(attribTable,attribTableSize);*/
}

//----------------------------------------------------------------------------------
// Desc: Extracts data from a D3DXMESH
// Subset can be -1 to get the whole mesh, or a number to get the segment
//----------------------------------------------------------------------------------
void Mesh::GetData(vector<WORD>& indices, vector<Vertex>& vertices, BBox& box, int subset/*=-1*/){
	D3DXATTRIBUTERANGE  attribTable[100]; DWORD attribTableSize = 100;
	mesh->GetAttributeTable(attribTable,&attribTableSize);

	box.Reset();
	int numIndices = mesh->GetNumFaces()*3;
	WORD* IndexesBuffer;
	mesh->LockIndexBuffer(D3DLOCK_READONLY, (LPVOID*)&IndexesBuffer);
	// Remap indices to remove internal offsets
	// Set the index buffer to the SUBSET of this mesh
	
	// Remove vertex offset from indices & add to new buffer
	if(subset == -1){
		indices.resize(numIndices);
		for(int p=0;p<numIndices;p++){
			indices[p] = IndexesBuffer[p];
		}
	}
	else{
		for(int p=attribTable[subset].FaceStart*3;p<attribTable[subset].FaceStart*3 + attribTable[subset].FaceCount*3;p++){
			indices.push_back(IndexesBuffer[p] - attribTable[subset].VertexStart);
		}

	}
	mesh->UnlockIndexBuffer();

	int numVertices = mesh->GetNumVertices();
	Vertex* pVertices;
	mesh->LockVertexBuffer(D3DLOCK_READONLY, (LPVOID*)&pVertices);
	
	if(subset == -1){
		vertices.resize(numVertices);
		for(int p=0;p<numVertices;p++){
			vertices[p] = pVertices[p];
			// Expand box
			box += vertices[p].p;
		}
	}
	else{
		for(int p=attribTable[subset].VertexStart;p<attribTable[subset].VertexStart+attribTable[subset].VertexCount;p++){
			vertices.push_back(pVertices[p]);
			// Expand box
			box += vertices.back().p;
		}
	}

	mesh->UnlockVertexBuffer();
}

//----------------------------------------------------------------------------------
// Desc: Spherical Harmonics equivilent of above
//----------------------------------------------------------------------------------
/*void Mesh::GetSHData(vector<WORD>& indices, vector<SHVertex>& vertices, BBox& box, int subset){
	D3DXATTRIBUTERANGE  attribTable[100]; 
	DWORD				attribTableSize = 100;
	mesh->GetAttributeTable(attribTable,&attribTableSize);

	box.Reset();
	int numIndices = mesh->GetNumFaces()*3;
	WORD* IndexesBuffer;
	mesh->LockIndexBuffer(D3DLOCK_READONLY, (LPVOID*)&IndexesBuffer);
	// Remap indices to remove internal offsets
	// Set the index buffer to the SUBSET of this mesh
	
	// Remove vertex offset from indices & add to new buffer
	if(subset == -1){
		indices.resize(numIndices);
		for(int p=0;p<numIndices;p++){
			indices[p] = IndexesBuffer[p];
		}
	}
	else{
		for(int p=attribTable[subset].FaceStart*3;p<attribTable[subset].FaceStart*3 + attribTable[subset].FaceCount*3;p++){
			indices.push_back(IndexesBuffer[p] - attribTable[subset].VertexStart);
		}

	}
	mesh->UnlockIndexBuffer();

	int size = mesh->GetNumBytesPerVertex();
	int sizeTwo = sizeof(SHVertex);
	int numVertices = mesh->GetNumVertices();
	SHVertex* pVertices;
	mesh->LockVertexBuffer(D3DLOCK_READONLY, (LPVOID*)&pVertices);
	
	if(subset == -1){
		vertices.resize(numVertices);
		for(int p=0;p<numVertices;p++){
			vertices[p] = pVertices[p];
			// Expand box
			box += vertices[p].p;
		}
	}
	else{
		for(int p=attribTable[subset].VertexStart;p<attribTable[subset].VertexStart+attribTable[subset].VertexCount;p++){
			vertices.push_back(pVertices[p]);
			// Expand box
			box += vertices.back().p;
		}
	}

	mesh->UnlockVertexBuffer();
}*/

//----------------------------------------------------------------------------------
// Desc: Extracts bones from D3DXSKININFO
//----------------------------------------------------------------------------------
void Mesh::GetBones(vector<SkinWeights>& bones){
	// Extract the skinning data
	if(pSkinInfo){
		for(int i=0;i<pSkinInfo->GetNumBones();i++){
			SkinWeights s;
			DWORD* indices = new DWORD[pSkinInfo->GetNumBoneInfluences(i)];
			s.weights.resize(pSkinInfo->GetNumBoneInfluences(i));
			// Get Indices & weights. Indices into temp array.
			pSkinInfo->GetBoneInfluence(i,indices,(FLOAT*)&s.weights[0]);
			// Copy DWORD indices into DWORD array
			for(int j=0;j<pSkinInfo->GetNumBoneInfluences(i);j++)
				s.indices.push_back(indices[j]);
			delete[] indices;

			s.offset = *(Matrix*)pSkinInfo->GetBoneOffsetMatrix(i);
			s.name = pSkinInfo->GetBoneName(i);
			bones.push_back(s);
		}
	}
}

//----------------------------------------------------------------------------------
// Desc: Extracts a flat list of faces from all the pools
// The faces hold everything needed to recompile into pools
// such as mateial ids
//
// Offsets are used so we can figure out which face maps to which material
//----------------------------------------------------------------------------------
void Mesh::GetAllFaces(ImportFrame* srcFrame, vector<Face>& faces, Matrix mat, vector<int> materialLookup){
	D3DXATTRIBUTERANGE  attribTable[100];  DWORD attribTableSize = 100;
	mesh->GetAttributeTable(attribTable,&attribTableSize);
	for(int i=0;i<GetNumSubsets();i++){
		vector<WORD> indices;
		vector<Vertex> vertices;
		BBox box;
		GetData(indices,vertices,box,i);

		for(int j=0;j<indices.size();j+=3){
			Face f;
			f.srcMesh = this;
			if(attribTable[i].AttribId >= materialLookup.size())
				Error("Out of range AttribId. Report to tim@artificialstudios.com with offending file");
			else
				f.flatmatid = materialLookup[attribTable[i].AttribId];

			for(int k=0;k<3;k++){
				f.v[k] = vertices[indices[j + k]];
				// Transform
				f.v[k].p = mat * f.v[k].p;
				f.v[k].n = mat.GetRotationMatrix() * f.v[k].n;
			}

			faces.push_back(f);
		}
	}
}


//----------------------------------------------------------------------------------
// Desc: Optimizes an index/vertex array
// Strips are always slower on Radeon 9700 so far. Same for OptimizedMesh sample.
// driver bug?
//----------------------------------------------------------------------------------
void Mesh::Optimize(bool bWeld){
	// FIXME: Why was I worried about this?
	//if(pSkinInfo)
	//	Error("you cannot optimize a skinned mesh in this way");

	bool strips = false;
	// Fixup the mesh
	LPD3DXBUFFER buf;
	if(!pAdjacency){
		D3DXCreateBuffer(mesh->GetNumFaces() * 3 * sizeof(DWORD),&pAdjacency);
		DXASSERT(mesh->GenerateAdjacency(0,(DWORD*)pAdjacency->GetBufferPointer()));
	}

	LPD3DXMESH pTempMesh;
	DXASSERT(D3DXCleanMesh(D3DXCLEAN_SKINNING,mesh,(DWORD*)pAdjacency->GetBufferPointer(),&pTempMesh,(DWORD*)pAdjacency->GetBufferPointer(),&buf));
	mesh->Release();
	mesh = pTempMesh;
	if(FAILED(D3DXValidMesh(mesh,(DWORD*)pAdjacency->GetBufferPointer(),&buf))){
		// TODO: Probably should do more cleaning here
	}
	
	DWORD dwOptFlags = D3DXMESHOPT_VERTEXCACHE | D3DXMESHOPT_COMPACT; // D3DXMESHOPT_ATTRSORT
	if(strips)
		dwOptFlags = D3DXMESHOPT_STRIPREORDER | D3DXMESHOPT_COMPACT;

	// Welds away duplicate or near-duplicate vertices
	if(bWeld){
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
		DXASSERT(D3DXWeldVertices(mesh,D3DXWELDEPSILONS_WELDPARTIALMATCHES,&Epsilons,(DWORD*)pAdjacency->GetBufferPointer(),(DWORD*)pAdjacency->GetBufferPointer(),NULL,&remap));
		
		// Remap skin info if present
		if(pSkinInfo)
			pSkinInfo->Remap(mesh->GetNumVertices(),(DWORD*)remap->GetBufferPointer());
		SAFE_RELEASE(remap);
	}

	LPD3DXBUFFER remap;
	DXASSERT(mesh->OptimizeInplace( dwOptFlags, (DWORD*)pAdjacency->GetBufferPointer(), NULL, NULL, &remap));
	// Remap skin info (again) if present
	if(pSkinInfo)
		pSkinInfo->Remap(mesh->GetNumVertices(),(DWORD*)remap->GetBufferPointer());
	SAFE_RELEASE(remap);

	// This code is very poor, don't use it
	// If you do, the test cases are
	// 1) A plane. 2) A box
	//if(FAILED(D3DXComputeNormals(mesh,(DWORD*)pAdjacency->GetBufferPointer())))
	//	Error("D3DXComputeTangent went boom");

	//if(FAILED(D3DXComputeTangent(mesh,0,0,D3DX_DEFAULT, FALSE,NULL)))//(DWORD*)pAdjacency->GetBufferPointer())))
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
// Desc: Create smoothing tangents for this mesh. This must be done on whole meshes
// (before world subdivision) as it needs to average connected faces
//
// TODO: Make sure my modification for "new_indices" to "indices" hasn't hurt anything
//----------------------------------------------------------------------------------
void Mesh::GenerateTangentsAndNormals(vector<Vertex>& new_verts, vector<WORD>& new_indices, vector<DWORD>& remap){
	// 1. Convert faces to indices/vertices temporarily
	/*
	vector<Vertex> new_verts;
	vector<WORD> new_indices;
	for(int k=0;k<faces.size();k++){
		new_verts.push_back(faces[k].v[0]);
		new_verts.push_back(faces[k].v[1]);
		new_verts.push_back(faces[k].v[2]);

		new_indices.push_back(k*3 + 0);
		new_indices.push_back(k*3 + 1);
		new_indices.push_back(k*3 + 2);
	}
	*/


	int number_of_vertices = new_verts.size();
	NVMeshMender aMender; 
	NVMeshMender::VAVector input;  // this is what you have 
	NVMeshMender::VAVector output; // this is what you want 
	NVMeshMender::VertexAttribute att;  // this is my working attribute 

	att.Name_ = "position"; 
	for ( int p = 0; p < number_of_vertices; p++ ) 
	{ 
		att.floatVector_.push_back( new_verts[p].p.x ); 
		att.floatVector_.push_back( new_verts[p].p.y ); 
		att.floatVector_.push_back( new_verts[p].p.z ); 
	} 
	input.push_back(att);
	att.floatVector_.clear(); 

	// REMAP BUFFER FOR SKINNING
	att.Name_ = "remap"; 
	for ( int p = 0; p < number_of_vertices; p++ ) 
	{ 
		att.floatVector_.push_back( p ); 
		att.floatVector_.push_back( p ); 
		att.floatVector_.push_back( p ); 
	} 
	input.push_back(att);
	att.floatVector_.clear(); 

	att.Name_ = "tex0"; 
	for ( int p = 0; p < number_of_vertices; p++ ) 
	{ 
		att.floatVector_.push_back( new_verts[p].t.x ); 
		att.floatVector_.push_back( new_verts[p].t.y ); 
		att.floatVector_.push_back( 0 ); //assuming your texcoords are 2d 
	} 
	input.push_back(att);
	att.floatVector_.clear(); 

	// Pass 3dsmax normals. If this is commented out it will generate normals for us.
	att.Name_ = "normal"; 
	for ( int p = 0; p < number_of_vertices; p++ ) 
	{ 
		att.floatVector_.push_back( new_verts[p].n.x ); 
		att.floatVector_.push_back( new_verts[p].n.y ); 
		att.floatVector_.push_back( new_verts[p].n.z );
	} 
	input.push_back(att); 
	att.floatVector_.clear(); 

#ifdef VERTEX_COLORS
	att.Name_ = "colors"; 
	for ( int i = 0; i < number_of_vertices; i++ ) 
	{ 
		att.floatVector_.push_back( COLOR_GETRED(new_verts[i].c) ); 
		att.floatVector_.push_back( COLOR_GETGREEN(new_verts[i].c) ); 
		att.floatVector_.push_back( COLOR_GETBLUE(new_verts[i].c) ); 
	} 
	input.push_back(att); 
	att.floatVector_.clear(); 
#endif

	att.Name_ = "indices"; 
	for ( int i = 0; i < new_indices.size(); i++ ) 
	{ 
		att.intVector_.push_back( new_indices[ i ] ); 
	} 
	input.push_back(att); 
	att.intVector_.clear(); 


	att.Name_ = "position";
	output.push_back(att);
	att.Name_ = "remap";
	output.push_back(att);
	att.Name_ = "indices";
	output.push_back(att);
	att.Name_ = "texcoord";
	output.push_back(att);
	att.Name_ = "tex0";
	output.push_back(att);
	att.Name_ = "normal";
	output.push_back(att);
	att.Name_ = "tangent";
	output.push_back(att);
	att.Name_ = "binormal";
	output.push_back(att);
#ifdef VERTEX_COLORS
	att.Name_ = "colors";
	output.push_back(att);
#endif

	bool bSuccess = aMender.Munge( input,              // these are my positions & new_indices 
		output,             // these are the outputs I requested, plus extra stuff generated on my behalf 
		3.141592654f / 2.5f,    // tangent space smooth angle 
		NULL,                   // no Texture matrix applied to my tex0 coords 
		// FIXME: Should be doing this, but requires we re-create mesh and remap attributes, ick
		NVMeshMender::DontFixTangents,            // fix degenerate bases & texture mirroring 
		NVMeshMender::DontFixCylindricalTexGen,/*FixCylindricalTexGen*/    // handle cylidrically mapped textures via vertex duplication 
		NVMeshMender::DontWeightNormalsByFaceSize // weight vertex normals by the triangle's size 
		);

	if ( !bSuccess ) cout << "aMender.Munge() failed!" << endl;

	for (int i = 0; i < output.size(); i++)
	{
		if (output[i].Name_ == "position")
		{
			//This is the first attribute, the floatVector_’s size / 3 is
			//the number of the new vertices existing
			new_verts.clear();
			new_verts.resize(output[i].floatVector_.size() / 3);
			for (int j = 0; j < output[i].floatVector_.size() / 3; j++)
			{
				new_verts[j].p.x = output[i].floatVector_[3 * j];
				new_verts[j].p.y = output[i].floatVector_[3 * j + 1];
				new_verts[j].p.z = output[i].floatVector_[3 * j + 2];
			}
		}
		// REMAP BUFFER FOR SKINNING
		if (output[i].Name_ == "remap")
		{
			remap.clear();
			remap.resize(output[i].floatVector_.size()/3);

			for (int j = 0; j < output[i].floatVector_.size(); j+=3)
			{
				remap[j/3] = output[i].floatVector_[j];
			}
		}

		//Check for every wanted attribute
		//Here’s a second instance
		if (output[i].Name_ == "normal")
		{
			for (int j = 0; j < output[i].floatVector_.size() / 3; j++)
			{
				new_verts[j].n.x = output[i].floatVector_[3 * j];
				new_verts[j].n.y = output[i].floatVector_[3 * j + 1];
				new_verts[j].n.z = output[i].floatVector_[3 * j + 2];
			}
		}
		//The new_indices
		if (output[i].Name_ == "indices")
		{
			new_indices.clear();
			new_indices.resize(output[i].intVector_.size());

			for (int j = 0; j < output[i].intVector_.size(); j++)
			{
				new_indices[j] = output[i].intVector_[j];
			}
		}

		if (output[i].Name_ == "tangent")
		{
			for (int j = 0; j < output[i].floatVector_.size() / 3; j++)
			{
				new_verts[j].tan.x = output[i].floatVector_[3 * j];
				new_verts[j].tan.y = output[i].floatVector_[3 * j + 1];
				new_verts[j].tan.z = output[i].floatVector_[3 * j + 2];
			}
		}

#ifdef VERTEX_COLORS
		if (output[i].Name_ == "colors")
		{
			int count = 0;
			for (int j = 0; j < output[i].floatVector_.size(); j+=3)
			{
				new_verts[count++].c = COLOR_ARGB(255,output[i].floatVector_[j],output[i].floatVector_[j+1],output[i].floatVector_[j+2]);
			}
		}
#endif

		if (output[i].Name_ == "tex0")
		{
			for (int j = 0; j < output[i].floatVector_.size() / 3; j++)
			{
				new_verts[j].t.x = output[i].floatVector_[3 * j];
				new_verts[j].t.y = output[i].floatVector_[3 * j + 1];
				//new_verts[3 * j].t.z = output[i].floatVector_[3 * j + 2];
			}
		}
	}


	// Finally, Map back to faces
	/*
	faces.clear();
	for(int j=0;j<new_indices.size();j+=3){
		Face f;
		f.geomID = geomID;
		f.flatmatid = pools[i].matID;
		f.v[2] = new_verts[new_indices[j+0]];
		f.v[1] = new_verts[new_indices[j+1]];
		f.v[0] = new_verts[new_indices[j+2]];
		faces.push_back(f);
	}
	*/
}
