//=========== (C) Copyright 2004, Tim Johnson. All rights reserved. ===========
// Type: XML (IGame) Loading Module
// Info: Loads XML files. Data is then passed to compiler/engine
//
// XMLLoad
// XMLSave
//
// FUNCTIONALITY TODO:
// 1. PRT Dialog
// 2. Rotation Tool
// 3. Axis drag tool
// 4. Import mesh button and Win32 Dialog
// 5. Delete mesh = DEL_KEY
//
//=============================================================================
#include "stdafx.h"
#include "Collision.h"
#include "Frame.h"
#include "ispatialpartition.h"
#include "ShadowMapping.h"
#include "SharedStructures.h"
#include "ScriptSystem.h"
#include "Compiler\Compiler.h"
#include "Editor.h"
#include "classmap.h"
#include "Serializer.h"
#include "XMLSerializer.h"
#include "SSystemStub.h"
#include "Profiler.h"
#include "Client.h"

// This may be recursing _ALL_ children and their children, which is not what we want
//#define m_XML.FindFirstNode(x,y) ((DOMElement*)x->getElementsByTagName(L##y)->item(0))


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void XMLSave::WriteMeshBinary(string filename, Mesh*& mesh){
	FILE* file = fopen(filename.c_str(),"wb");
	if(!file){
		Warning("Couldn't open %s",filename.c_str());
		return;
	}

	// Find lowest index out of all materials, this will be the first submaterial this mesh uses
	int matIndex = 999;
	for(int i=0;i<mesh->m_Materials.size();i++)
    {
		for(int j=0;j<m_Materials.size();j++)
        {
            if(mesh->m_Materials[i]->m_Name == m_Materials[j]->m_Name && j < matIndex){
				matIndex = j;
				break;
			}
        }
	}

	BYTE version = 22;
	fwrite(&version,sizeof(version),1,file);

	vector<Mesh::SkinWeights> bones;
	mesh->GetBones(bones);
	int numBones = bones.size();
	fwrite(&numBones,sizeof(numBones),1,file);

	for(int i=0;i<numBones;i++)
	{
		// Name
		int nameLen = bones[i].name.length();
		fwrite(&nameLen,sizeof(nameLen),1,file);
		fwrite(bones[i].name.c_str(),nameLen,1,file);
		// Offset
		fwrite(&bones[i].offset,sizeof(bones[i].offset),1,file);
		// Indices
		int numIndices = bones[i].indices.size();
		fwrite(&numIndices,sizeof(numIndices),1,file);
		for(int j=0;j<numIndices;j++)
			fwrite(&bones[i].indices[j],sizeof(bones[i].indices[j]),1,file);
		// Weights
		int numWeights = bones[i].weights.size();
		fwrite(&numWeights,sizeof(numWeights),1,file);
		for(int j=0;j<numWeights;j++)
			fwrite(&bones[i].weights[j],sizeof(bones[i].weights[j]),1,file);
	}

	bool bOptimized = true;
	fwrite(&bOptimized,sizeof(bOptimized),1,file);

    int numSegments = mesh->m_AttribTable.size(); // m_Materials
	fwrite(&numSegments,sizeof(numSegments),1,file);


    // VB
    int stride	    = mesh->GetHardwareMesh()->GetNumBytesPerVertex();
	int vertexSize  = mesh->m_OriginalVertexSize;
    int numVertices = mesh->GetHardwareMesh()->GetNumVertices();
	// If no original size specified, guess original format
    if(!vertexSize)
    {
        if(stride == sizeof(SkinnedVertex))
            vertexSize = sizeof(Vertex);
        else
            vertexSize = stride;
    }
    fwrite(&vertexSize,sizeof(vertexSize),1,file);
	fwrite(&numVertices,sizeof(numVertices),1,file);
    BYTE* VerticesBuffer;
    mesh->GetHardwareMesh()->LockVertexBuffer(D3DLOCK_READONLY, (LPVOID*)&VerticesBuffer);
    for(int i=0;i<numVertices;i++)
        fwrite(&VerticesBuffer[stride*i],vertexSize,1,file);
    mesh->GetHardwareMesh()->UnlockVertexBuffer();

    // IB
    int indexSize = (mesh->GetHardwareMesh()->GetOptions() & D3DXMESH_32BIT)?sizeof(DWORD):sizeof(WORD);
	int numIndices = mesh->GetHardwareMesh()->GetNumFaces()*3;
    fwrite(&indexSize,sizeof(indexSize),1,file);
	fwrite(&numIndices,sizeof(numIndices),1,file);
    BYTE* Indices;
    mesh->GetHardwareMesh()->LockIndexBuffer(D3DLOCK_READONLY, (LPVOID*)&Indices);
    fwrite(&Indices[0],indexSize,numIndices,file);
    mesh->GetHardwareMesh()->UnlockIndexBuffer();

    // Write all material ids
    int numMats = mesh->m_Materials.size();
    fwrite(&numMats,sizeof(numMats),1,file);
    for(int i=0;i<mesh->m_Materials.size();i++)
    {
    	// Find mat id for this segment
		int subIndex = 9999;
		for(int j=0;j<m_Materials.size();j++){
            if(mesh->m_Materials[i]->m_Name == m_Materials[j]->m_Name){
				subIndex = j;
				break;
			}
		}

		// MatID
		int matID = subIndex - matIndex;
		assert(matID>=0);
		fwrite(&matID,sizeof(matID),1,file);
    }

    // Write all segments
	for(int i=0;i<numSegments;i++)
	{
		AttributeRange r = mesh->m_AttribTable[i];

        // Attribute range!
		fwrite(&r,sizeof(AttributeRange),1,file);

        // Mat ID remappings
        int num = mesh->m_AttribToMaterial.size();
        fwrite(&num,sizeof(num),1,file);
        for(int a=0;a<num;a++)
            fwrite(&mesh->m_AttribToMaterial[a],sizeof(mesh->m_AttribToMaterial[a]),1,file);
	}

	fclose(file);
}

//-----------------------------------------------------------------------------
int FreadInt(FILE* file){ int x; fread(&x,sizeof(x),1,file); return x; }
#define FreadVector(v) {v.resize(FreadInt(file)); fread(&v[0],sizeof(v[0]),v.size(),file);}
//-----------------------------------------------------------------------------
// Bare-bones rendering mesh
// Binary format:
// <VertexSize>
// <NumVertices>
// <Vertices>
//
// <IndexSize>
// <NumIndices>
// <Indices>
//-----------------------------------------------------------------------------
bool XMLLoad::ReadMeshBinary(string filename, string name, Mesh*& mesh, int matIndex, CollisionMesh*& cMesh){
	mesh = new Mesh();

	FILE* file = fopen(filename.c_str(),"rb");
	if(!file){
		SeriousWarning("Cannot open mesh '%s'. It does not appear to exist at specified location.",filename.c_str());
		return false;
	}

    StartMiniTimer();
    
	bool bOptimized;
	int numSegments = 1;
	BYTE version;
	fread(&version,sizeof(version),1,file);

	// Version 13 and above has skinning support
	vector<Mesh::SkinWeights> bones;
	if(version > 12)
	{
		int numBones;
		fread(&numBones,sizeof(numBones),1,file);
		bones.resize(numBones);

		for(int i=0;i<bones.size();i++)
		{
			// Name
			int nameLen;
			fread(&nameLen,sizeof(nameLen),1,file);
			char name[MAX_PATH];
			fread(name,nameLen,1,file);
			name[nameLen] = 0;
			bones[i].name = name;
			// Offset
			fread(&bones[i].offset,sizeof(bones[i].offset),1,file);
			// Indices
			int numIndices;
			fread(&numIndices,sizeof(numIndices),1,file);
			bones[i].indices.resize(numIndices);
			for(int j=0;j<numIndices;j++)
				fread(&bones[i].indices[j],sizeof(bones[i].indices[j]),1,file);
			// Weights
			int numWeights;
			fread(&numWeights,sizeof(numWeights),1,file);
			bones[i].weights.resize(numWeights);
			for(int j=0;j<numWeights;j++)
				fread(&bones[i].weights[j],sizeof(bones[i].weights[j]),1,file);
		}
	}
	fread(&bOptimized,sizeof(bOptimized),1,file);
	fread(&numSegments,sizeof(numSegments),1,file);

    //
    // Data read
    //
	vector<int> matIDs;
    BYTE*   vertices = 0; 
	BYTE*   indices  = 0;
    int     NumIndices      = 0;
	int     NumVertices     = 0;
    int     VertexSize      = 0;
    int     IndexSize       = 0;


    //
    // Old loading format. Breaks meshes into segments. New format uses one VB/IB with different attribids
    //
    if(version < 20)
    {
	    vector<BYTE*> vertSegments;
	    vector<BYTE*> indSegments;
	    vector<int>	  indSize, vertSize;
	    for(int i=0;i<numSegments;i++)
	    {
		    if(version >= 11)
			    matIDs.push_back(FreadInt(file));

		    // Read vertices. Size depends on vertex format, so use generic BYTE* array
		    int vertexSize = FreadInt(file);
		    int numVertices = FreadInt(file); 
            int vertexStart = NumVertices;
            if(version>13)
                vertexStart = FreadInt(file);

		    int indexSize, numIndices;

		    if(version >= 11)
		    {
			    indexSize = FreadInt(file);
			    numIndices = FreadInt(file); 

			    if(numVertices == 0 || numIndices == 0)
				    continue;
		    }

		    if(numVertices < 0 || vertexSize < 0)
		    {
			    SeriousWarning("Mesh buf '%s' is corrupted. Skipping...",filename.c_str());
			    StopMiniTimer();
                return false;
		    }

		    BYTE* vertices = new BYTE[numVertices*vertexSize]; 
		    fread(&vertices[0],vertexSize,numVertices,file);
		    // Read indices. Size depends on 32-bit or 16-bit indices
		    if(version < 11)
		    {
			    indexSize = FreadInt(file);
			    numIndices = FreadInt(file); 
		    }
		    BYTE*   indices = new BYTE[indexSize*numIndices];
		    fread(&indices[0],indexSize,numIndices,file);

		    // Add attrib table segment
		    AttributeRange r;
		    r.AttribId = i;
		    r.FaceCount		= numIndices/3;
		    r.VertexCount	= numVertices;
		    r.FaceStart		= NumIndices/3;
            r.VertexStart	= vertexStart;
            mesh->m_AttribTable.push_back(r);

            NumVertices	+= numVertices;
            NumIndices	+= numIndices;
            vertSegments.push_back(vertices);
            indSegments.push_back(indices);
            indSize.push_back(indexSize);
            vertSize.push_back(vertexSize);
        }

	    if(NumVertices == 0 || NumIndices == 0)
	    {
		    SAFE_DELETE(mesh);
		    SeriousWarning("Mesh %s is empty (corrupted?). Skipping...",filename.c_str());
		    StopMiniTimer();
            return false;
	    }

        IndexSize  = indSize[0];
        VertexSize = vertSize[0];

	    // Create new buffers
	    vertices = new BYTE[NumVertices*vertSize[0]]; 
	    indices  = new BYTE[indSize[0]*NumIndices];
	    for(int i=0;i<mesh->m_AttribTable.size();i++)
	    {
		    AttributeRange& r = mesh->m_AttribTable[i];
		    memcpy(&vertices[r.VertexStart*vertSize[0]],&(vertSegments[i][0]),vertSize[0]*r.VertexCount);
		    memcpy(&indices[indSize[0]*r.FaceStart*3],&(indSegments[i][0]),indSize[0]*r.FaceCount*3);
		    delete[] vertSegments[i];
		    delete[] indSegments[i];
	    }
    }
    //
    // New loading format
    //
    else
    {
         VertexSize = FreadInt(file);
		 NumVertices = FreadInt(file); 
         vertices = new BYTE[NumVertices*VertexSize]; 
		 fread(&vertices[0],VertexSize,NumVertices,file);

         IndexSize  = FreadInt(file);
		 NumIndices = FreadInt(file); 
         indices = new BYTE[NumIndices*IndexSize]; 
		 fread(&indices[0],IndexSize,NumIndices,file);

         if(version > 20)
         {
             int numMats = FreadInt(file);
             for(int i=0;i<numMats;i++)
                 matIDs.push_back(FreadInt(file));
         }

         for(int i=0;i<numSegments;i++)
	     {
            if(version <= 20)
                matIDs.push_back(FreadInt(file));
            AttributeRange r;
            fread(&r,sizeof(AttributeRange),1,file);
            mesh->m_AttribTable.push_back(r);

            // Mat ID remappings
            if(version > 20)
            {
                int num = FreadInt(file);
                mesh->m_AttribToMaterial.resize(num);
                for(int a=0;a<num;a++)
                    fread(&mesh->m_AttribToMaterial[a],sizeof(mesh->m_AttribToMaterial[a]),1,file);
            }
         }
    }

    Profiler::Get()->MeshLoadSecs += StopMiniTimer()/1000.f;
    StartMiniTimer();

    m_FileVerts += NumVertices;
	m_FileInds  += NumIndices;


	// !!! Loading/Logic Code -- Could be abstracted !!!
	// Material loading
	vector<Material*> materials;
	if(matIndex == -1){
		materials.push_back(MaterialManager::Instance()->GetDefaultMaterial());
		// Increment ref count
		materials[0]->AddRef();
	}
	else{
		// Fill array with all materials
		for(int i=0;i<matIDs.size();i++){
			if(version >= 11)
			{
				if(matIndex+matIDs[i] < m_Materials.size())
					materials.push_back(m_Materials[matIndex+matIDs[i]]);
				else
					materials.push_back(MaterialManager::Instance()->GetDefaultMaterial());
			}
			else
			{
				if(matIndex+i < m_Materials.size())
					materials.push_back(m_Materials[matIndex+i]);
				else
					materials.push_back(MaterialManager::Instance()->GetDefaultMaterial());
			}
			// Increment ref count
            if(materials.back())
			    materials.back()->AddRef();
		}
	}

	fclose(file);
    
    if(!mesh->Create(vertices,indices,VertexSize,IndexSize,NumVertices,NumIndices,materials)){
        SeriousWarning("Mesh '%s' create failed with %d verts and %d inds",name.c_str(),NumVertices,NumIndices);
    }

    cMesh = new CollisionMesh;
	cMesh->Initialize(vertices,indices,VertexSize,IndexSize,NumVertices,NumIndices,materials);

	if(bones.size() && !Engine::Instance()->IsDedicated())
	{
		mesh->SetBones(bones);
		mesh->GenerateSkinInfo();
	}

	if(!bOptimized && !bones.size()  && !Engine::Instance()->IsDedicated()){
		mesh->Optimize(true);
		XMLSave s;
		s.m_Materials = m_Materials;
		s.WriteMeshBinary(filename,mesh);
	}

	delete[] indices;
	delete[] vertices;

    Profiler::Get()->MeshSetupSecs += StopMiniTimer()/1000.f;
	return true;
}

//----------------------------------------------------------------------------------
// 
//----------------------------------------------------------------------------------
void XMLLoad::ReadMeshXML(string filename, string name, Mesh*& mesh, vector<Material*>& materials)
{
	XMLSystem sys;
	if(!sys.Load(filename)){
		Warning("Couldn't load '%s'",filename.c_str());
		return;
	}

	LogPrintf("Loading mesh '%s'",filename.c_str());
	mesh = new Mesh();

	DOMElement* geom = (DOMElement*)sys.GetWalker()->nextNode();
	DOMElement* vertList = m_XML.FindFirstNode(geom,"Vertices");


	// Count # vertices
	int vertexSize = sizeof(Vertex);
	int numVertsRead = 0;
	int numVertices = ReadInt(vertList,"Count");
	BYTE* vertices = NULL; new BYTE[numVertices*vertexSize];

	// <Vertex...
	DOMNodeList* verts		= vertList->getElementsByTagName(L"Vertex");
	for(int j=0;j<verts->getLength();j++)
	{
		DOMElement* vert = (DOMElement*)verts->item(j);
		int numTex = 0;
		while(vert->getAttributeNode( ToUnicode(("Tex"+ToStr(numTex))).c_str()))
			numTex++;

		// Decide vertex type from number of tex coords
		if(vertices == NULL){
			if(numTex == 1)
				vertexSize = sizeof(Vertex);
			else if(numTex == 2)
				vertexSize = sizeof(VertexT2);

			vertices = new BYTE[numVertices*vertexSize];
		}


		Vector pos	= ReadVector(vert,"Pos");
		Vector norm, tan;
		Vector2 t[2];
		if(AttribExists(vert,"Norm"))
			norm	= ReadVector(vert,"Norm");
		if(AttribExists(vert,"Tangent"))
			tan		= ReadVector(vert,"Tangent");

		for(int i=0;i<numTex;i++){
			string tex = m_XML.GetString(vert->getAttributeNode(ToUnicode(("Tex"+ToStr(i))).c_str()));
			t[i].x = strtod(GetWord(tex,0).c_str(),NULL);
			t[i].y = strtod(GetWord(tex,1).c_str(),NULL);
		}

		// Now fill vertex structure
		if(vertexSize == sizeof(Vertex)){
			Vertex* v = (Vertex*)&vertices[numVertsRead*vertexSize];
			v->position = pos;
			v->normal = norm;
			v->tan = tan;
			v->tex = t[0];
		}
		else if(vertexSize == sizeof(VertexT2)){
			VertexT2* v = (VertexT2*)&vertices[numVertsRead*vertexSize];
			v->position = pos;
			v->normal = norm;
			v->tan = tan;
			v->tex[0] = t[0];
			v->tex[1] = t[1];
		}

		numVertsRead++;
	}


	DOMElement* faceList = m_XML.FindFirstNode(geom,"Faces");
	DOMNodeList* faces		= faceList->getElementsByTagName(L"Face");

	int indexSize = sizeof(WORD);
	if(numVertices > 65535)
		indexSize = sizeof(DWORD);

	BYTE* indices = new BYTE[faces->getLength()*3*indexSize];

	for(int j=0;j<faces->getLength();j++)
	{
		DOMElement* face = (DOMElement*)faces->item(j);
		if(indexSize == sizeof(DWORD)){
			DWORD d1 = ReadInt(face,"v2");
			DWORD d2 = ReadInt(face,"v1");
			DWORD d3 = ReadInt(face,"v0");
			((DWORD*)indices)[j*3 + 0] = d1;
			((DWORD*)indices)[j*3 + 1] = d2;
			((DWORD*)indices)[j*3 + 2] = d3;
		}
		else{
			WORD d1 = ReadInt(face,"v2");
			WORD d2 = ReadInt(face,"v1");
			WORD d3 = ReadInt(face,"v0");
			((WORD*)indices)[j*3 + 0] = d1;
			((WORD*)indices)[j*3 + 1] = d2;
			((WORD*)indices)[j*3 + 2] = d3;
		}
	}

	m_FileVerts += numVertices;
	m_FileInds  += faces->getLength()*3;

	// !!! Loading/Logic Code -- Could be abstracted !!!
	string parentFile;
	if(m_pWorld)
		parentFile = m_pWorld->m_FileName;
	else
		parentFile = m_pModel->m_FileName;

	mesh->Create(vertices,indices,vertexSize,indexSize,numVertices,faces->getLength()*3,materials);
	mesh->Optimize(true);
	// Get default SH settings
	if(m_pWorld)
		mesh->m_SHOptions = m_pWorld->m_DefaultSH;

	delete[] vertices;
	delete[] indices;
	sys.Destroy();
}



//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void XMLLoad::ReadMaterials(DOMElement* pNode, vector<Material*>& materials)
{
	if(Engine::Instance()->IsDedicated())
		return;

	// <MaterialList
	DOMNodeList* mats		= pNode->getElementsByTagName(L"Material");
	for(int i=0;i<mats->getLength();i++)
	{
		DOMElement* pMat		= ((DOMElement*)mats->item(i));
		string name				= ReadString(pMat,"Name");

		Material* mat = MaterialManager::Instance()->FindMaterial(name);
		if(mat)
			mat->AddRef();
		else
		{
			mat = new Material(name);

			DOMElement* shader		= m_XML.FindFirstNode(pMat,"Shader");
			if(shader)
			{
				string strShader		= ReadString(shader,"File");
				string strTechnique		= ReadString(shader,"Technique");

				DOMNodeList* params		= shader->getElementsByTagName(L"Parameter");
				if(!params || params->getLength() == 0)
					params		= shader->getElementsByTagName(L"Param");

				// TODO: Convert this to editorvar ReadParameter() system
				// Problem: Vars alloc memory, ReadParameter doesn't have the size
				for(int j=0;j<params->getLength();j++){
					ShaderVar* p	= new ShaderVar;

					BYTE* data = new BYTE[1024]; // Temp, until we know real size
					ZeroMemory(&data[0],1024);
					int   size = 0;
					bool ret = m_XML.ReadParameter(p->type,(BYTE*)data,params->item(j),size,p->name);

					// Check for duplicate param, if so, skip
					for(int u=0;u<mat->m_Parameters.size();u++)
					{
						if(mat->m_Parameters[u]->name == p->name){
							ret = false;
							break;
						}
					}

					if(ret)
					{
						if(p->type == EditorVar::TEXTURE){
							string value	= ((Texture*)data)->filename;
							// Allocate a texture
							Texture* tex = new Texture;
							tex->filename = value;
                            if(m_pWorld) // Too much memory for world to handle
                                tex->dontCache = true;
							tex->Load(value);
							p->Set(p->name,tex);

							// Try to set the material type from this map if it's not already been found
							if(mat->m_Type == "unknown")
								mat->m_Type = MaterialManager::Instance()->GetType(value);
						}
						else{
							// ShaderVar will alloc exact memory and copy from buffer
							p->Alloc(data,size);
						}

						// Buffer was temp, so delete
						delete[] data;

						//
						mat->m_Parameters.push_back(p);
					}
					else{
						delete[] data;
						delete p;
					}
				}

				// !!! Loading/Logic Code -- Could be abstracted !!!

				// Create shader!
				if(!mat->Initialize(strShader.c_str(),strTechnique.c_str())){
					// Error, such as not found, fall back to default material
					mat->Release();
					mat = MaterialManager::Instance()->GetDefaultMaterial();
					mat->AddRef();
				}
			}
			else
			{
				Warning("Material '%s' lacks shader, using default mat",mat->m_Name.c_str());
				mat->Release();
				mat = MaterialManager::Instance()->GetDefaultMaterial();
				mat->AddRef();
			}
		}
		materials.push_back(mat);

        // Update every 1%
        float percent = (float)m_ParsedNodes/(float)m_TotalNodes*100.0f;
        if(m_pWorld && m_pWorld->ProgressCallback)
        {
            m_pWorld->ProgressCallback(percent,"Loading Materials");
        }
        m_ParsedNodes++;
	}
}


//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void XMLSave::WriteMaterials(DOMElement* pNode, vector<Material*>& materials)
{
	// <MaterialList
	DOMElement*  list = m_XML.CreateNode(pNode,"MaterialList");
	m_XML.Attrib("Count",(int)materials.size());

	for(int i=0;i<materials.size();i++)
	{
		Material* mat = materials[i];
		// <Material
		DOMElement* matNode = m_XML.CreateNode(list,"Material");
		m_XML.Attrib("index",i);
		m_XML.Attrib("Name",mat->m_Name);

		// TODO: Submaterials

		DOMElement* shader = m_XML.CreateNode(matNode,"Shader");
		m_XML.Attrib("File",mat->m_Shader->GetFilename());
		m_XML.Attrib("Technique",mat->m_Token);
		for(int j=0;j<mat->m_Parameters.size();j++)
		{
			ShaderVar* var = mat->m_Parameters[j];
			m_XML.WriteParameter((TCHAR*)mat->m_Parameters[j]->name.c_str(),mat->m_Parameters[j]->type,(BYTE*)mat->m_Parameters[j]->data,shader);
		}
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
bool XMLLoad::ReadFrame(DOMElement* pNode, ModelFrame*& frame, int depth)
{
	if(!AttribExists(pNode,"Name") || !AttribExists(pNode,"NodeType"))
		return false; // NULL node

	frame = new ModelFrame;
	// <Node...
	//	int		id		= ReadInt(pNode,"NodeID");
	string	type	= ReadString(pNode,"NodeType");
	frame->Name 	= ReadString(pNode,"Name");

	// <NodeTM...
	DOMElement* pTM	= m_XML.FindFirstNode(pNode,"NodeTM");
	Matrix tm;
	tm[0] = ReadVector(pTM,"Row0");
	tm[1] = ReadVector(pTM,"Row1");
	tm[2] = ReadVector(pTM,"Row2");
	tm[3] = ReadVector(pTM,"Row3");
	if(depth)
		frame->TransformationMatrix = tm;


	// <MaterialIndex
	int index = -1;
	DOMElement* matIndex	= m_XML.FindFirstNode(pNode,"MaterialIndex");
	if(matIndex){
		index = m_XML.GetInt(matIndex,true);
	}

	// <GeomData...
	DOMElement* geom	= m_XML.FindFirstNode(pNode,"GeomData");
	if(geom)
	{
		string filename = ReadString(geom,"Include");
		filename = StripPath(filename);

		// Mesh
		string fullPathFile = m_FileName.substr(0,m_FileName.find_last_of("\\")+1) + filename;
		//if(frame->m_Filename.find(".xml") != -1)
		//	ReadMeshXML(frame->m_Filename,frame->Name,frame->GetMesh(),materials);
		//else
		Mesh* newMesh = 0;
        if(ReadMeshBinary(fullPathFile,frame->Name,newMesh,index,frame->collisionMesh))
		{
			frame->SetMesh(newMesh);

			// <Time...
			DOMElement* time				= m_XML.FindFirstNode(geom,"Time");
			frame->GetMesh()->m_TimeMoved		= ReadTime(time,"Moved");
			frame->GetMesh()->m_TimeModified		= ReadTime(time,"Modified");

			ReadSHProperties(geom,frame->GetMesh()->m_SHOptions);

			// <CollisionData
			DOMElement* collision	= m_XML.FindFirstNode(pNode,"CollisionData");
			if(collision){
				Error("TODO: CollisionData not implemented");
            }
		}

	}


	//if(type == "CollisionMesh")
	//	LoadCollisionMesh(frame->collisionMesh);
	//if(type == "Light")
	//	; // TODO: Read light

	// Find first child
	// <Node...
	ModelFrame* pFrame = NULL;
	DOMNodeList*	pChildren =  pNode->getChildNodes();
	DOMElement*		pChild	  = NULL;
	for(int i=0;i<pChildren->getLength();i++){
		if(m_XML.GetName(pChildren->item(i)) == "Node"){
			pChild = (DOMElement*)pChildren->item(i);
			break;
		}
	}

	depth++;

	if(pChild){
		ReadFrame(pChild,pFrame,depth);
		frame->pFrameFirstChild = pFrame;
	}

	// Only get siblings if below root level
	DOMNode* pSibling = pNode->getNextSibling();
	if(depth > 1 && pSibling){
		// Somtimes we get a text node buffering a child, just because of pretty formatting, so skip it
		while(pSibling && pSibling->getNodeType() == DOMNode::TEXT_NODE)
			pSibling = pSibling->getNextSibling();

        ModelFrame* newFrame = NULL;
		if(pSibling && pSibling->getNodeType() == DOMNode::ELEMENT_NODE)
		{
			try
			{
				ReadFrame((DOMElement*)pSibling,newFrame,depth);
			}
			catch(...)
			{
				SeriousWarning("Error reading sibling of frame %s",frame->Name.c_str());
			}
			frame->pFrameSibling = newFrame;
		}
	}

    if(frame->pFrameSibling)
        assert(frame->pFrameSibling != frame->pFrameFirstChild);

	return geom!=0 || frame->pFrameSibling || frame->pFrameFirstChild;
}


//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void XMLSave::WriteFrame(DOMElement* pNode, ModelFrame*& frame, int depth)
{

	// <Node & <NodeTM
	// This is already written at root, so only write for child frames
	if(depth){
		pNode = m_XML.CreateNode(pNode,"Node");
		m_XML.Attrib("Name",frame->Name);
		m_XML.Attrib("NodeType","GeomData");

		m_XML.CreateNode(pNode,"NodeTM");
		m_XML.Attrib("Row0",frame->TransformationMatrix[0]);
		m_XML.Attrib("Row1",frame->TransformationMatrix[1]);
		m_XML.Attrib("Row2",frame->TransformationMatrix[2]);
		m_XML.Attrib("Row3",frame->TransformationMatrix[3]);
	}

	// <MaterialIndex
	// Find lowest index out of all materials, this will be the first submaterial this mesh uses
	int matIndex = 9999;
	for(int i=0;frame->GetMesh() && i<frame->GetMesh()->m_Materials.size();i++)
    {
		for(int j=0;j<m_Materials.size();j++)
        {
            if(frame->GetMesh()->m_Materials[i]->m_Name == m_Materials[j]->m_Name && j < matIndex){
				matIndex = j;
				break;
			}
        }
	}
    if(matIndex == 9999 && frame->GetMesh() && frame->GetMesh()->m_Materials.size())
    {
        SeriousWarning("Mat on mesh doesn't exist in scene Obj: %s Mat: %s",frame->Name.c_str(),frame->GetMesh()->m_Materials[0]->m_Name.c_str());
    }
    if(frame->GetMesh() && !frame->GetMesh()->m_Materials.size())
        SeriousWarning("No mats on Obj: %s",frame->Name.c_str());

	if(matIndex != 9999)
    {
		TCHAR buf[64];
		_stprintf(buf,"%d",matIndex);
		DOMElement* pMatNode = m_XML.CreateNode(pNode,"MaterialIndex");
		m_XML.CreateTextNode(pMatNode,buf);
	}

	// <GeomData...
	if(frame->GetMesh())
    {
		string filename = frame->Name + ".buf";
        m_DataFiles.push_back(frame->Name);
        m_HasPRT.push_back(frame->GetMesh()->UsingPRT());

		DOMElement* geom = m_XML.CreateNode(pNode,"GeomData");
		m_XML.Attrib("Include",filename);

		// <Time...
		m_XML.CreateNode(geom,"Time");
		m_XML.Attrib("Moved",frame->GetMesh()->m_TimeMoved);
		m_XML.Attrib("Modified",frame->GetMesh()->m_TimeModified);

		// SH Stuff
        if(frame->GetMesh()->m_SHOptions.Enabled || frame->GetMesh()->HasLightMapping() || frame->GetMesh()->UsingColorVertex())
			WriteSHProperties(frame->Name,geom,frame->GetMesh()->m_SHOptions);

		// Get the correct output directory
		string fullFilePath = m_FileName.substr(0,m_FileName.find_last_of("\\")+1) + filename;
		// Write!
		Mesh* mesh = frame->GetMesh();
		WriteMeshBinary(fullFilePath,mesh);
	}
	//else
	//	SeriousWarning("Frame %s has NULL mesh. This is not fatal, but may be a sign of corruption. Please report this!",frame->Name.c_str());

	// <CollisionData
	//m_XML.CreateNode(pNode,"CollisionData");
    
	// Children and siblings
	depth++;
	if(frame->pFrameFirstChild)
		WriteFrame(pNode,frame->pFrameFirstChild,depth);
	if(frame->pFrameSibling)
		WriteFrame((DOMElement*)pNode->getParentNode(),frame->pFrameSibling,depth);
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void XMLSave::WriteLight(DOMElement* pNode, Light*& light)
{
	DOMElement* lNode = m_XML.CreateNode(pNode,"LightData");

	// <LightData Type...
	string type = "Omni";
	if(light->m_Type == LIGHT_SPOT)
		type = "Free";
	else if(light->m_Type == LIGHT_DIR)
		type = "Directional";
	else if(light->m_Type == LIGHT_OMNI_PROJ)
		type = "OmniProjector";
	m_XML.Attrib("Type",type);

	// <Properties  
	DOMElement* props = m_XML.CreateNode(lNode,"Properties");  
	m_XML.CreateNode(props,"Prop");  
	m_XML.Attrib("name","Color");  
	m_XML.Attrib("value",*(Vector*)&light->GetCurrentState().Diffuse);  
	//  
	m_XML.CreateNode(props,"Prop");  
	m_XML.Attrib("name","Multiplier");  
	m_XML.Attrib("value",(float)light->GetCurrentState().Intensity);  
	//  
	m_XML.CreateNode(props,"Prop");  
	m_XML.Attrib("name","AttenuationFarEnd");  
	m_XML.Attrib("value",(float)light->GetCurrentState().Range);  
	//  
	m_XML.CreateNode(props,"Prop");  
	m_XML.Attrib("name","Falloff");  
	m_XML.Attrib("value",(float)light->GetCurrentState().Spot_Falloff);  
	//  
	m_XML.CreateNode(props,"Prop");  
	m_XML.Attrib("name","Hotspot");  
	m_XML.Attrib("value",(float)light->GetCurrentState().Spot_Size);

	// <ExcludeList
	DOMElement* list = m_XML.CreateNode(lNode,"ExcludeList");
	m_XML.Attrib("Count",(int)light->m_ExcludeList.size());
	m_XML.Attrib("Include",(bool)!light->m_IsExcludeList);

	for(int i=0;i<light->m_ExcludeList.size();i++){
		m_XML.CreateNode(list,"Light");
		m_XML.Attrib("Name",light->m_ExcludeList[i]);
	}

	if(light->m_bShadowProjector){
		// <ShadowMap
		m_XML.CreateNode(lNode,"ShadowMap");
		m_XML.Attrib("Name",light->m_tProjectionMap.filename);
	}
	else if(light->m_tProjectionMap.filename.find(".") != -1){
		// <ProjectionMap
		m_XML.CreateNode(lNode,"ProjectionMap");
		m_XML.Attrib("Name",light->m_tProjectionMap.filename);
	}
	if(light->m_tOmniProjCubeMap.filename.find(".") != -1)
	{
		// <OmniProjectionCubeMap
		m_XML.CreateNode(lNode,"OmniProjectionCubeMap");
		m_XML.Attrib("Name",light->m_tOmniProjCubeMap.filename);
	}
	if(light->m_tOmniProjCubeMapBlur.filename.find(".") != -1)
	{
		// <OmniProjectionCubeMapBlur
		m_XML.CreateNode(lNode,"OmniProjectionCubeMapBlur");
		m_XML.Attrib("Name",light->m_tOmniProjCubeMapBlur.filename);
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void XMLLoad::ReadLight(DOMElement* pNode, Light*& light)
{
	DOMElement* lightData	= m_XML.FindFirstNode(pNode,"LightData");
	string		type		= ReadString(lightData,"Type");

	// <LightData Type...
	if(type == "Omni")
		light->m_Type = LIGHT_OMNI;
	else if(type == "Targeted" || type == "Free")
		light->m_Type = LIGHT_SPOT;
	else if(type == "Directional" || type == "TargetedDirectional")
		light->m_Type = LIGHT_DIR;
	else if(type == "OmniProjector")
		light->m_Type = LIGHT_OMNI_PROJ;

	DOMElement* shadowMap	= m_XML.FindFirstNode(lightData,"ShadowMap");
	if(shadowMap && light->m_Type == LIGHT_SPOT){
		light->m_bShadowProjector = true;
	}
	// Check to see if already loaded by non-legacy system
	else if(light->m_Type == LIGHT_OMNI_PROJ && light->m_tOmniProjCubeMap.filename.find(".") == -1)
	{
		DOMElement* omniProjMap	= m_XML.FindFirstNode(lightData,"OmniProjectionCubeMap");
		if(omniProjMap)
		{
			light->m_tOmniProjCubeMap.Load(ReadString(omniProjMap,"Name"));
		}
		DOMElement* omniProjMapBlur	= m_XML.FindFirstNode(lightData,"OmniProjectionCubeMapBlur");
		if(omniProjMapBlur)
		{
			light->m_tOmniProjCubeMapBlur.Load(ReadString(omniProjMapBlur,"Name"));
		}
	}
	// Check to see if already loaded by non-legacy system
	else if(light->m_tProjectionMap.filename.find(".") == -1)
	{
		DOMElement* projMap	= m_XML.FindFirstNode(lightData,"ProjectionMap");
		if(projMap && light->m_Type == LIGHT_SPOT){
			light->m_bShadowProjector = false;
			light->m_tProjectionMap.Load(ReadString(projMap,"Name"));
		}
	}

	// <Properties...
	DOMNodeList* props = m_XML.FindFirstNode(lightData,"Properties")->getElementsByTagName(L"Prop");
	if(props){
		for(int i=0;i<props->getLength();i++){
			if(ReadString(props->item(i),"name") == "Color"){
				light->GetCurrentState().Diffuse = FloatColor(ReadVector(props->item(i),"value"));
			}
			else if(ReadString(props->item(i),"name") == "Multiplier"){
				light->GetCurrentState().Intensity = ReadFloat(props->item(i),"value");
			}
			else if(ReadString(props->item(i),"name") == "AttenuationFarEnd"){
				light->GetCurrentState().Range = ReadFloat(props->item(i),"value");
			}
			else if(ReadString(props->item(i),"name") == "Falloff"){
				light->GetCurrentState().Spot_Falloff = ReadFloat(props->item(i),"value");
			}
			else if(ReadString(props->item(i),"name") == "Hotspot"){
				light->GetCurrentState().Spot_Size = ReadFloat(props->item(i),"value");
			}

		}
	}

	// <ExcludeList
	DOMElement* excludeList	= m_XML.FindFirstNode(lightData,"ExcludeList");
	if(excludeList){
		light->m_IsExcludeList	= !ReadBool(excludeList,"Include");

		// <Light Name...
		DOMNodeList* params = excludeList->getElementsByTagName(L"Light");
		if(params){
			for(int i=0;i<params->getLength();i++){
				light->m_ExcludeList.push_back(ReadString(params->item(i),"Name"));
			}
		}
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void XMLLoad::ReadSelectionList(DOMElement* pNode)
{
    if(!AttribExists(pNode,"Name"))
        return;
    ActorSelectedList selectionList(0);
    string	name	= ReadString(pNode,"Name");
    selectionList.ListName = name;

    DOMElement* items	= m_XML.FindFirstNode(pNode,"Items");
    DOMNodeList* actorIDS = items->getElementsByTagName(L"Item");
    for(int j=0;j<actorIDS->getLength();j++)
    {
		unsigned int actorID = ReadInt(actorIDS->item(j),"ActorID");
        for (int i=0;i< m_pWorld->m_Actors.size();i++)
        {
            if (m_pWorld->m_Actors[i]->m_ActorID == actorID)
            {
                selectionList.m_SelectedActors.push_back(m_pWorld->m_Actors[i]);
                break;
            }
        }
    }

    Editor::Instance()->m_ActorLists.push_back(selectionList);
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void XMLLoad::ReadNode(DOMElement* pNode, Actor*& node)
{
	// <Node...
	//	int		id		= ReadInt(pNode,"NodeID");
	if(!AttribExists(pNode,"Name") || !AttribExists(pNode,"NodeType"))
		return; // NULL node

	string	name	= ReadString(pNode,"Name");
	string	type	= ReadString(pNode,"NodeType");
    
	// <NodeTM...
	DOMElement* pTM	= m_XML.FindFirstNode(pNode,"NodeTM");
	Matrix tm;
	tm[0] = ReadVector(pTM,"Row0");
	tm[1] = ReadVector(pTM,"Row1");
	tm[2] = ReadVector(pTM,"Row2");
	tm[3] = ReadVector(pTM,"Row3");


	// <CustomProperties>
	DOMElement* props	= m_XML.FindFirstNode(pNode,"CustomProperties");


	// Actor
	if(props)
	{
		DOMElement* actor = m_XML.FindFirstNode(props,"Actor");
		// Create actor!
		if(m_pWorld && actor)
		{
			string className = ReadString(actor,"Class");
			node = Factory::create(className,m_pWorld);

			//Client-side, we don't want to spawn NetworkActors that aren't linked by GUID. 
			// Such NetworkActors will be sent by the server later (if they still exist in the World, so just delete this Client-loading NetworkActor here.
			if(!m_pWorld->m_IsServer && !node->m_SpawnByName)
			{
				delete node;
				return;
			}

			if(node)
			{
				if(AttribExists(actor,"ActorID"))
					node->m_ActorID		= ReadInt(actor,"ActorID");

                if(AttribExists(actor,"GUID"))
                {
                    string sGuid = ReadString(actor,"GUID");
	                UuidFromString((unsigned char*)sGuid.c_str(),&node->m_GUID);
                }
   

                if(AttribExists(actor,"BakedTM"))
                {
                    DOMElement* pTM	= m_XML.FindFirstNode(actor,"BakedTM");
	                node->BakedTM[0] = ReadVector(pTM,"Row0");
	                node->BakedTM[1] = ReadVector(pTM,"Row1");
	                node->BakedTM[2] = ReadVector(pTM,"Row2");
	                node->BakedTM[3] = ReadVector(pTM,"Row3");
                }

				bool managed = false;
				if (node && node->IsScriptActor())
					managed = true;

				// Set known params
				node->m_Name		= name;
				node->Rotation		= tm.GetRotationMatrix();
				node->Location		= tm[3];
				node->bExportable	= true;
				// Set all editable params from file!
				DOMNodeList* params		= actor->getElementsByTagName(L"Param");
				if (managed)
				{
					for(int j=0;j<params->getLength();j++)
						m_XML.ReadParameter(node->GetManagedIndex(),params->item(j));
				}
				else
				{
					for(int j=0;j<params->getLength();j++)
					{
						string name = ReadString(params->item(j),"Name");
						for(int i=0;i<node->EditorVars.size();i++)
						{
                            string varName = node->EditorVars[i].name;
							if(name == varName)
							{
								EditorVar::Type type; // Don't need type, already have it from EditableVar
								int size; // unused
								string name; // unused
								if(m_XML.ReadParameter(type,(BYTE*)node->EditorVars[i].data,params->item(j),size,name))
									assert(type == node->EditorVars[i].type);
								//else
								//	Warning("Actor param '%s' not mapped in file. Please re-save file. If this problem persists, please report it!",node->EditorVars[i].name.c_str());
							}
						}
					}
				}

				// Light-specific initialization
				if(node->IsLight())
				{
					Light* l = (Light*)node;
					//
					if(l->m_tOmniProjCubeMap.filename.find(".") != -1)
						l->m_tOmniProjCubeMap.Load(l->m_tOmniProjCubeMap.filename);
					//
					if(l->m_tOmniProjCubeMapBlur.filename.find(".") != -1)
						l->m_tOmniProjCubeMapBlur.Load(l->m_tOmniProjCubeMapBlur.filename);
					//
					if(l->m_tProjectionMap.filename.find(".") != -1)
						l->m_tProjectionMap.Load(l->m_tProjectionMap.filename);
				}
			}
		}
	}

	ScriptData script;
	if(props){
		// <Script...
		DOMElement* scriptNode	= m_XML.FindFirstNode(props,"Script");
		if(scriptNode){
			script.filename	= ReadString(scriptNode,"File");
			script.classname	= ReadString(scriptNode,"Class");
			script.parentclass	= ReadString(scriptNode,"Parent");
			script.bIncludeModel = ReadBool(scriptNode,"IncludeModel");
			
			// <Param...
			DOMNodeList* params = scriptNode->getElementsByTagName(L"Param");
			if(params){
				for(int i=0;i<params->getLength();i++){
					script.parameters.push_back(ReadString(params->item(i),"Name"));
					script.paramvalues.push_back(ReadString(params->item(i),"Value"));
				}
			}
		}
	}

	// !!! Loading/Logic Code -- Could be abstracted !!!

	//
	// Load 'Model', Spawn 'Prefab'
	//
	if((node && !node->IsLight()) || (type == "GeomObject" && script.bIncludeModel) )
	{
		//
		// LEGACY CODE FOR MAX COMPATIBILITY!!
		//
		if(!node)
		{
			node = new Prefab(m_pWorld);
			node->m_Name		= name;
			node->Rotation		= tm.GetRotationMatrix();
			node->Location		= tm[3];


			bool ghostobject = false;
			bool ishidden	 = false;
			if(script.classname == "StaticMesh")
			{
				for(int i=0;i<script.parameters.size();i++)
				{
					if(script.parameters[i] == "noclip")
						ghostobject = (script.paramvalues[i] == "true" || script.paramvalues[i] == "1");
					else if(script.parameters[i] == "invisible")
						ishidden = (script.paramvalues[i] == "true" || script.paramvalues[i] == "1");
				}
				script.classname.clear();
				node->StaticObject = true;
			}
			else 
				node->StaticObject = false;

			node->GhostObject	= ghostobject;
			node->IsHidden		= ishidden;
		}

		Model* model = new Model;
		if(ReadFrame(pNode,model->m_pFrameRoot))
		{
			model->m_pFrameRoot->UpdateMatrices(model->m_RootTransform);
			model->InitAfterLoad();	
			model->RemoveSceneOffset();
			node->MyModel		= model;
			node->MyModel->bExportable = true;
			node->MyModel->SetTransform(node->Rotation,node->Location);
		}
		else
			delete model;
	}
	//
	// Load/Spawn 'Light'
	//
	else if(type == "Light" || (node && node->IsLight()))
	{
		//
		// LEGACY CODE FOR MAX COMPATIBILITY!!
		//
		if(!node)
		{
			node = new Light(m_pWorld);
			node->m_Name		= name;
			// Copy node data!
			node->Rotation		= tm.GetRotationMatrix();
			node->Location		= tm[3];

			if(script.classname == "Light")
			{
				for(int i=0;i<script.parameters.size();i++)
				{
					if(script.parameters[i] == "SphericalHarmonics")
					{
						if(script.paramvalues[i] == "true" || script.paramvalues[i] == "1")
							((Light*)node)->m_Method = LIGHT_METHOD_SH;
						else
							((Light*)node)->m_Method = LIGHT_METHOD_PERPIXEL;
					}
					else if(script.parameters[i] == "ForceMultiPassSH")
						((Light*)node)->m_ForceSHMultiPass = (script.paramvalues[i] == "true" || script.paramvalues[i] == "1");
				}
				script.classname.clear();
			}
		}

		if(!Editor::Instance()->GetEditorMode())
			((Light*)node)->_MakeStatic(); // Editor lights always dynamic for easy moving

		ReadLight(pNode,*(Light**)&node);
		((Light*)node)->GetCurrentState().Direction = -tm[1];
		((Light*)node)->GetCurrentState().Position  =  tm[3];
		// TODO: Support keyframes
		((Light*)node)->IgnoreKeyframes();

		if(((Light*)node)->m_Type == LIGHT_DIR)
			((Light*)node)->m_ForceSHMultiPass = false;

	}
	// Spawn 'Script' entity
	else if(script.classname.length())
	{
		//
		// LEGACY CODE FOR MAX COMPATIBILITY!!
		//
		if(!node)
		{
			node = new Actor(m_pWorld);
			node->m_Name		= name;
			// Keep it out of the 'real' scene tree
			node->GhostObject = true;
			node->Rotation		= tm.GetRotationMatrix();
			node->Location		= tm[3];
			node->bExportable	= true;
		}
	}

	if(node && script.classname.size())
	{
		node->script		= script;

		//HACK: scripts aren't staticobjects currently, to avoid baking their mesh's vertices
		//TODO: make dummy parameter so that scripts can determine this behavior
		node->StaticObject = false;

		// Scripting
		node->EditorVars.push_back(EditorVar("Name",&node->script.classname,"Scripting","Script name"));
		node->EditorVars.push_back(EditorVar("IncludeModel",&node->script.bIncludeModel,"Scripting","Script includes model?"));
		for(int i=0;i<node->script.parameters.size();i++)
		{
			node->EditorVars.push_back(EditorVar(node->script.parameters[i],&node->script.paramvalues[i],"Scripting","Script Parameter"));
		}
	}

	if(node)
	{
		node->bExportable = true;
		node->DeserializationComplete();
	}

	if(!node)
		SeriousWarning("Unexpected type: %s on %s",type.c_str(),name.c_str());

	if(Client::Instance()->IsConnected())
		Sleep(0);
}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool XMLLoad::ReadSHProperties(DOMElement* pNode, PRTSettings& opt)
{
	DOMElement* sh	= m_XML.FindFirstNode(pNode,"SphericalHarmonics");
	opt.Enabled = false;
	if(!sh)
		return false;

	opt.Enabled = true;

	// <InBlockers...
	DOMNodeList* inBlockers = m_XML.FindFirstNode(sh,"InBlockers")->getElementsByTagName(L"Blocker");
	if(inBlockers){
		for(int i=0;i<inBlockers->getLength();i++){
			opt.InBlockers.push_back(ReadString(inBlockers->item(i),"Name"));
		}
	}

	// <OutBlockers...
	DOMNodeList* outBlockers = m_XML.FindFirstNode(sh,"OutBlockers")->getElementsByTagName(L"Blocker");
	if(outBlockers){
		for(int i=0;i<outBlockers->getLength();i++){
			opt.OutBlockers.push_back(ReadString(outBlockers->item(i),"Name"));
		}
	}

	// <Settings..
	DOMElement* set	= m_XML.FindFirstNode(sh,"Settings");
	opt.dwNumRays = ReadInt(set,"NumRays");
	opt.dwOrder = ReadInt(set,"Order");
	opt.dwNumChannels = ReadInt(set,"NumChannels");
	opt.dwNumBounces = ReadInt(set,"NumBounces");
	opt.bSubsurfaceScattering = ReadBool(set,"SubsurfaceScattering");
	opt.fLengthScale = ReadFloat(set,"LengthScale");
	opt.Diffuse = ReadFloatColor(set,"Diffuse");
	opt.Absoption = ReadFloatColor(set,"Absoption");
	opt.ReducedScattering = ReadFloatColor(set,"ReducedScattering");
	opt.fRelativeIndexOfRefraction = ReadFloat(set,"RelativeIndexOfRefraction");
	opt.dwPredefinedMatIndex = ReadInt(set,"PredefinedMatIndex");
	opt.bAdaptive = ReadBool(set,"Adaptive");
	opt.bRobustMeshRefine = ReadBool(set,"RobustMeshRefine");
	opt.fRobustMeshRefineMinEdgeLength = ReadFloat(set,"RobustMeshRefineMinEdgeLength");
	opt.dwRobustMeshRefineMaxSubdiv = ReadInt(set,"RobustMeshRefineMaxSubdiv");
	opt.bAdaptiveDL = ReadBool(set,"AdaptiveDL");
	opt.fAdaptiveDLMinEdgeLength = ReadFloat(set,"AdaptiveDLMinEdgeLength");
	opt.fAdaptiveDLThreshold = ReadFloat(set,"AdaptiveDLThreshold");
	opt.dwAdaptiveDLMaxSubdiv = ReadInt(set,"AdaptiveDLMaxSubdiv");
	opt.bAdaptiveBounce = ReadBool(set,"AdaptiveBounce");
	opt.fAdaptiveBounceMinEdgeLength = ReadFloat(set,"AdaptiveBounceMinEdgeLength");
	opt.fAdaptiveBounceThreshold = ReadFloat(set,"AdaptiveBounceThreshold");
	opt.dwAdaptiveBounceMaxSubdiv = ReadInt(set,"AdaptiveBounceMaxSubdiv");
	opt.dwNumClusters = ReadInt(set,"NumClusters");
	opt.dwNumPCA = ReadInt(set,"NumPCA");
	opt.bPerPixel = ReadBool(set,"PerPixel");
	opt.dwTextureSize = ReadInt(set,"TextureSize");
	return true;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void XMLSave::WriteSHProperties(string recvName, DOMNode* pNode, PRTSettings& sh)
{
	DOMElement* shRoot = m_XML.CreateNode(pNode,"SphericalHarmonics");
	m_XML.Attrib("ReceiverName",recvName);

	// Indoor blockers
	DOMElement* blockers = m_XML.CreateNode(shRoot,"InBlockers");
	m_XML.Attrib("Count",(int)sh.InBlockers.size());
	for(int i=0;i<sh.InBlockers.size();i++)
	{
		m_XML.CreateNode(blockers,"Blocker");
		m_XML.Attrib("Name",sh.InBlockers[i]);
	}

	// Outdoor blockers
	DOMElement* blockers2 = m_XML.CreateNode(shRoot,"OutBlockers");
	m_XML.Attrib("Count",(int)sh.OutBlockers.size());
	for(int i=0;i<sh.OutBlockers.size();i++)
	{
		m_XML.CreateNode(blockers2,"Blocker");
		m_XML.Attrib("Name",sh.OutBlockers[i]);
	}

	m_XML.CreateNode(shRoot,"Settings");
	m_XML.Attrib("NumRays",sh.dwNumRays);
	m_XML.Attrib("Order",sh.dwOrder);
	m_XML.Attrib("NumChannels",sh.dwNumChannels);
	m_XML.Attrib("NumBounces",sh.dwNumBounces);
	m_XML.Attrib("SubsurfaceScattering",sh.bSubsurfaceScattering);
	m_XML.Attrib("LengthScale",sh.fLengthScale);
	m_XML.Attrib("Diffuse",*(Vector*)&sh.Diffuse);
	m_XML.Attrib("Absoption",*(Vector*)&sh.Absoption);
	m_XML.Attrib("ReducedScattering",*(Vector*)&sh.ReducedScattering);
	m_XML.Attrib("RelativeIndexOfRefraction",sh.fRelativeIndexOfRefraction);
	m_XML.Attrib("PredefinedMatIndex",sh.dwPredefinedMatIndex);
	m_XML.Attrib("Adaptive",sh.bAdaptive);
	m_XML.Attrib("RobustMeshRefine",sh.bRobustMeshRefine);
	m_XML.Attrib("RobustMeshRefineMinEdgeLength",sh.fRobustMeshRefineMinEdgeLength);
	m_XML.Attrib("RobustMeshRefineMaxSubdiv",sh.dwRobustMeshRefineMaxSubdiv);
	m_XML.Attrib("AdaptiveDL",sh.bAdaptiveDL);
	m_XML.Attrib("AdaptiveDLMinEdgeLength",sh.fAdaptiveDLMinEdgeLength);
	m_XML.Attrib("AdaptiveDLThreshold",sh.fAdaptiveDLThreshold);
	m_XML.Attrib("AdaptiveDLMaxSubdiv",sh.dwAdaptiveDLMaxSubdiv);
	m_XML.Attrib("AdaptiveBounce",sh.bAdaptiveBounce);
	m_XML.Attrib("AdaptiveBounceMinEdgeLength",sh.fAdaptiveBounceMinEdgeLength);
	m_XML.Attrib("AdaptiveBounceThreshold",sh.fAdaptiveBounceThreshold);
	m_XML.Attrib("AdaptiveBounceMaxSubdiv",sh.dwAdaptiveBounceMaxSubdiv);
	m_XML.Attrib("NumClusters",sh.dwNumClusters);
	m_XML.Attrib("NumPCA",sh.dwNumPCA);
	m_XML.Attrib("PerPixel",sh.bPerPixel);
	m_XML.Attrib("TextureSize",sh.dwTextureSize);
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void XMLSave::WriteNode(DOMElement* pNode, Actor*& node)
{
	if(!node->bExportable)
		return;

	// <Node...
	pNode = m_XML.CreateNode(pNode,"Node");
	m_XML.Attrib("Name",node->m_Name);
	if(node->IsLight())
		m_XML.Attrib("NodeType","Light");
	else
		m_XML.Attrib("NodeType","GeomObject");

	Matrix mat = node->Rotation;
	mat[3] = node->Location;
	m_XML.CreateNode(pNode,"NodeTM");
	m_XML.Attrib("Row0",mat[0]);
	m_XML.Attrib("Row1",mat[1]);
	m_XML.Attrib("Row2",mat[2]);
	m_XML.Attrib("Row3",mat[3]);

	DOMElement* cust = m_XML.CreateNode(pNode,"CustomProperties");

	// Actor
	DOMElement* actor = m_XML.CreateNode(cust,"Actor");
	m_XML.Attrib("ActorID",(int)node->m_ActorID);

    BYTE* buf;
	UuidToStringA(&node->m_GUID,&buf);
	m_XML.Attrib("GUID",(char*)buf);

	string classname = node->ClassName();

	if (!node->IsScriptActor())
    {
        m_XML.Attrib("Class",classname.c_str());
        for(int i=0;i<node->EditorVars.size();i++)
        {
            m_XML.WriteParameter((TCHAR*)node->EditorVars[i].name.c_str(),node->EditorVars[i].type,(BYTE*)node->EditorVars[i].data,actor);
        }
    }
    else
    //MANAGED ACTORS SERIALIZATION
    {
		m_XML.Attrib("Class",node->ClassName());
        SSystem_ActorSerialize(node->GetManagedIndex(),(void*)&m_XML,actor);
    }

    m_XML.CreateNode(pNode,"BakedTM");
	m_XML.Attrib("Row0",node->BakedTM[0]);
	m_XML.Attrib("Row1",node->BakedTM[1]);
	m_XML.Attrib("Row2",node->BakedTM[2]);
	m_XML.Attrib("Row3",node->BakedTM[3]);


	// Script Stuff
	if(node->script.filename.length())
	{
		DOMElement* script = m_XML.CreateNode(cust,"Script");
		m_XML.Attrib("File",node->script.filename);
		m_XML.Attrib("Class",node->script.classname);
		m_XML.Attrib("Parent",node->script.parentclass);
		m_XML.Attrib("IncludeModel",node->script.bIncludeModel);

		for(int i=0;i<node->script.parameters.size();i++){
			m_XML.CreateNode(script,"Param");
			m_XML.Attrib("Name",node->script.parameters[i]);
			m_XML.Attrib("Value",node->script.paramvalues[i]);
		}
	}

	if(node->IsLight())
		WriteLight(pNode,*(Light**)&node);
	else if(node->MyModel && node->MyModel->bExportable)
		WriteFrame(pNode,node->MyModel->m_pFrameRoot);
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void XMLSave::WriteSelectionList(DOMElement* pNode, ActorSelectedList* list)
{
    pNode = m_XML.CreateNode(pNode,"SelectionList");
    m_XML.Attrib("Name",list->ListName);
    DOMElement* items = m_XML.CreateNode(pNode,"Items");
    for (int i=0;i<list->m_SelectedActors.size();i++)
    {
        DOMElement* item = m_XML.CreateNode(items,"Item");
        m_XML.Attrib("ActorID",(int)list->m_SelectedActors[i]->m_ActorID);
    }
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void XMLLoad::ReadSceneInfo(DOMElement* pNode, World*& world)
{
	if(m_pModel)
		return;

	DOMElement* fog		= m_XML.FindFirstNode(pNode,"Fog");
	if(fog)
		world->SetFog(FloatColor(ReadVector(fog,"Color")),ReadFloat(fog,"Start"));

	DOMElement* clip		= m_XML.FindFirstNode(pNode,"Clip");
	world->SetClipPlane(ReadFloat(clip,"FarPlane"));

    DOMElement* lod		= m_XML.FindFirstNode(pNode,"LOD");
    if(lod)
        world->m_fLODCullBias = ReadFloat(lod,"LODCullBias");

    DOMElement* sky		= m_XML.FindFirstNode(pNode,"Sky");
    if(sky)
        world->m_fConeRadius = ReadFloat(sky,"LightCone");

	DOMElement* miniMap	= m_XML.FindFirstNode(pNode,"MiniMap");
	if(miniMap){
		//		world->m_MiniMap	= ReadString(miniMap,"Image");
		//		world->m_MiniMapScale = ReadFloat(miniMap,"Scale");
	}

	DOMElement* cam	= m_XML.FindFirstNode(pNode,"EditorCam");
	if(cam){
		Editor::Instance()->m_Camera.Direction = ReadVector(cam,"Direction");
		Editor::Instance()->m_Camera.Location  = ReadVector(cam,"Location");
	}

    DOMElement* occlusion	= m_XML.FindFirstNode(pNode,"Occlusion");
	if(occlusion){
		world->m_DisableOcclusionCulling = ReadBool(occlusion,"DisableOcclusion");
	}

    DOMElement* editor		= m_XML.FindFirstNode(pNode,"Editor");
    if (editor)
        ACTOR_ID_COUNT = ReadInt(editor,"CurrentActorID");

	// Read SH properties. 
	ReadSHProperties(pNode,world->m_DefaultSH);
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void XMLSave::WriteSceneInfo(DOMElement* pNode, World*& world)
{
	pNode = m_XML.CreateNode(pNode,"SceneInfo");
	m_XML.CreateNode(pNode,"Fog");
	m_XML.Attrib("Color",*(Vector*)&world->GetFogColor());
	m_XML.Attrib("Start",world->GetFogDistance());

    m_XML.CreateNode(pNode,"Sky");
    m_XML.Attrib("LightCone",world->m_fConeRadius);

	m_XML.CreateNode(pNode,"Clip");
	m_XML.Attrib("FarPlane",world->GetClipPlane());

    m_XML.CreateNode(pNode,"Occlusion");
    m_XML.Attrib("DisableOcclusion",world->m_DisableOcclusionCulling);

    m_XML.CreateNode(pNode,"LOD");
    m_XML.Attrib("LODCullBias",world->m_fLODCullBias);

	m_XML.CreateNode(pNode,"MiniMap");
	m_XML.Attrib("Image","");
	m_XML.Attrib("Scale",0);

	m_XML.CreateNode(pNode,"EditorCam");
	m_XML.Attrib("Direction",Editor::Instance()->m_Camera.Direction);
	m_XML.Attrib("Location",Editor::Instance()->m_Camera.Location);

    m_XML.CreateNode(pNode,"Editor");
    m_XML.Attrib("CurrentActorID",(int)ACTOR_ID_COUNT);

	// Read SH properties
	WriteSHProperties("[These are the scene defaults]",pNode,world->m_DefaultSH);
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void XMLLoad::ParseNode(DOMElement* pCurrent)
{
	int type = pCurrent->getNodeType();
	switch (type) {
		case DOMNode::ELEMENT_NODE:
			{
				// Top-Level objects
				if(m_XML.GetName(pCurrent) == "SceneInfo")
					ReadSceneInfo(pCurrent,m_pWorld);
				else if(m_XML.GetName(pCurrent) == "MaterialList")
				{
					ReadMaterials(pCurrent,m_Materials);
				}
                else if(m_XML.GetName(pCurrent) == "SelectionList")
				{
					ReadSelectionList(pCurrent);
				}
				else if(m_XML.GetName(pCurrent) == "Node")
                {
					if(m_pWorld){
						Actor* actor = NULL;

                        float percent = (float)m_ParsedNodes/(float)m_TotalNodes*100.0f;
                        if(m_pWorld && m_pWorld->ProgressCallback)
                        {
                            m_pWorld->ProgressCallback(percent,"Loading Scene Contents");
                        }
                        m_ParsedNodes++;
						ReadNode(pCurrent, actor);
					}
					// Load model frame. We keep trying nodes until we find a node that
					// fills m_pFrameRoot
					else if(!m_pModel->m_pFrameRoot){
						// Read frame, if valid use it!!
						ModelFrame* newFrame = NULL;
						if(ReadFrame(pCurrent,newFrame,1))
						{
                            Model* model = (Model*)m_pModel;
							model->m_pFrameRoot = newFrame;
						}
					}
				}
				break;
			}
		case DOMNode::ENTITY_REFERENCE_NODE:  
		case DOMNode::CDATA_SECTION_NODE:  
		case DOMNode::TEXT_NODE:  
		case DOMNode::PROCESSING_INSTRUCTION_NODE: 
			//Warning("Unexpected type in XML file"); 
			break;
	}
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
bool XMLLoad::LoadWorld(string name, World* world, bool IsSky){
	m_pWorld = world;

    Editor::Instance()->m_ActorLists.clear();

	if(!OpenFile(name.c_str()))
		return false;

	CloseFile();

	// We added a Ref for every Mesh that used this array, so now we can call release once
	// on each material so when the last mesh is removed, so is the material
	for(int i=0;i<m_Materials.size();i++)
		m_Materials[i]->Release();

	return true;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
bool XMLLoad::LoadModel(string name, StaticModel* model){
	m_pModel = model;
	if(!OpenFile(name.c_str()))
		return false;

	//ReadFrame(
	//LoadModelFrame(model->m_pFrameRoot);
	CloseFile();

	// We added a Ref for every Mesh that used this array, so now we can call release once
	// on each material so when the last mesh is removed, so is the material
	for(int i=0;i<m_Materials.size();i++)
		m_Materials[i]->Release();

	return true;
}

//-----------------------------------------------------------------------------
// Creates a file
//-----------------------------------------------------------------------------
bool XMLSave::SaveWorld(string name, World* world){
	m_FileName = name;
	m_pWorld = world;

	xercesc_2_5::DOMDocument* doc = m_XML.CreateDocument("IGame");

	DOMElement* root = doc->getDocumentElement();

	// Gather materials used by the actors we're exporting
	for(int i=0;i<world->m_Actors.size();i++)
    {
		if(world->m_Actors[i]->MyModel && world->m_Actors[i]->MyModel->bExportable)
        {
			world->m_Actors[i]->MyModel->m_pFrameRoot->FindMaterials(m_Materials);
		}
	}

	WriteSceneInfo(root,world);
	WriteMaterials(root,m_Materials);

	for(int i=0;i<world->m_Actors.size();i++)
		WriteNode(root,world->m_Actors[i]);

    for (int i=0;i<Editor::Instance()->m_ActorLists.size();i++)
        WriteSelectionList(root, &Editor::Instance()->m_ActorLists[i]);

	m_XML.Save(name);

    ClearUnusedFiles();
	return true;
}

//-----------------------------------------------------------------------------
// Deletes unused buf/prt files
//-----------------------------------------------------------------------------
void XMLSave::ClearUnusedFiles()
{
    //
    // Now clear up unused files
    //
    string path = m_FileName.substr(0,m_FileName.find_last_of("\\")+1);
    // Get all PRT/BUF files
    vector<string> prtFiles, bufFiles;
    enumerateFiles(path.c_str(),prtFiles,2,"*.prt");
    enumerateFiles(path.c_str(),bufFiles,2,"*.buf");
    // For each PRT file, is it used?
    for(int i=0;i<prtFiles.size();i++)
    {
        bool found = false;
        for(int j=0;j<m_DataFiles.size();j++)
        {
            string file = m_DataFiles[j]+".prt";
            if(AsLower(file) == AsLower(prtFiles[i]) && m_HasPRT[j])
            {
                found = true;
                break;
            }
        }
        if(!found)
            DeleteFile((path+prtFiles[i]).c_str());
    }

    // For each BUF file, is it used?
    for(int i=0;i<bufFiles.size();i++)
    {
        bool found = false;
        for(int j=0;j<m_DataFiles.size();j++)
        {
            if(AsLower(m_DataFiles[j]+".buf") == AsLower(bufFiles[i]))
            {
                found = true;
                break;
            }
        }
        if(!found)
            DeleteFile((path+bufFiles[i]).c_str());
    }
}

//-----------------------------------------------------------------------------
// Opens the file
//-----------------------------------------------------------------------------
bool XMLLoad::OpenFile(const char* name){
	m_FileName = name;

    if(m_pWorld && m_pWorld->ProgressCallback)
        m_pWorld->ProgressCallback(0,"Parsing File");

    StartMiniTimer();
	LogPrintf("Loading '%s'",name);
	m_FileVerts = 0;
	m_FileInds  = 0;

	if(!m_XML.Load(m_FileName)){
		return false;
	}

	if(m_pModel)
		m_pModel->m_FileName = name;
	else
		m_pWorld->m_FileName = name;


	// use the tree walker to recurse the nodes
	DOMTreeWalker* walker = m_XML.GetWalker();
    DOMNodeList* children = m_XML.GetDocument()->getDocumentElement()->getChildNodes();
    m_TotalNodes = children->getLength() * .60f;
    m_ParsedNodes = 0;
	DOMElement* pCurrent;
	for ( pCurrent = (DOMElement*)walker->nextNode(); pCurrent != 0; pCurrent = (DOMElement*)walker->nextSibling() )
	{
		ParseNode(pCurrent);
	}

	return true;
}

//-----------------------------------------------------------------------------
// Closes file & writes out summary info
//-----------------------------------------------------------------------------
void XMLLoad::CloseFile(){
	m_XML.Destroy();

//	if(m_FileInds <= 0 || m_FileVerts <= 0)
//		SeriousWarning("File '%s' appears empty, this could indicate corruption. Nevertheless it has been loaded.",m_FileName.c_str());


    LogPrintf("\t%s loaded in %f secs. Inds: %d Verts: %d",m_FileName.c_str(),StopMiniTimer()/1000.0f,m_FileInds,m_FileVerts);

	m_pWorld = NULL;
	m_pModel = NULL;
	m_FileName = "(None)";
}

XMLLoad::XMLLoad(){
	m_pWorld = NULL;
	m_pModel = NULL;
}




//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void MaterialSerializer::ReadMaterial(DOMElement* pMat)
{
	if(Engine::Instance()->IsDedicated())
		return;

	Material* mat = new Material();

	//string sGuid = ReadString(pMat,"GUID");
	//UuidFromString((unsigned char*)sGuid.c_str(),&mat->m_GUID);

	mat->m_Name				= ReadString(pMat,"Name");
	mat->m_Category			= ReadString(pMat,"Category");

	DOMElement* shader		= m_XML.FindFirstNode(pMat,"Shader");
	if(shader)
	{
		mat->m_ShaderName		= ReadString(shader,"File");
		mat->m_Token        	= ReadString(shader,"Technique");

		DOMNodeList* params		= shader->getElementsByTagName(L"Parameter");
		if(!params || params->getLength() == 0)
			params		= shader->getElementsByTagName(L"Param");

		// TODO: Convert this to editorvar ReadParameter() system
		// Problem: Vars alloc memory, ReadParameter doesn't have the size
		for(int j=0;j<params->getLength();j++){
			ShaderVar* p	= new ShaderVar;

			BYTE* data = new BYTE[1024]; // Temp, until we know real size
			ZeroMemory(&data[0],1024);
			int   size = 0;
			m_XML.ReadParameter(p->type,(BYTE*)data,params->item(j),size,p->name);

			if(p->type == EditorVar::TEXTURE){
				string value	= ((Texture*)data)->filename;
				// Allocate a texture
				Texture* tex = new Texture;
				tex->filename = value;
				//tex->Load(value);
				p->Set(p->name,tex);

				// Try to set the material type from this map if it's not already been found
				if(mat->m_Type == "unknown")
					mat->m_Type = MaterialManager::Instance()->GetType(value);
			}
			else{
				// ShaderVar will alloc exact memory and copy from buffer
				p->Alloc(data,size);
			}

			// Buffer was temp, so delete
			delete[] data;

			//
			mat->m_Parameters.push_back(p);
		}
	}
	else
	{
		Warning("Material '%s' lacks shader, using default mat",mat->m_Name.c_str());
		mat->Release();
		mat = MaterialManager::Instance()->GetDefaultMaterial();
		mat->AddRef();
	}

	m_Library->AddMaterial(mat);
}


//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void MaterialSerializer::WriteMaterials(DOMElement* pNode)
{
	DOMElement* lib = pNode;//m_XML.CreateNode(pNode,"MaterialLibrary");
	m_XML.Attrib("Name", m_Library->FileName,lib);
	for(int i=0;i<m_Library->Count();i++)
	{
		Material* mat = m_Library->GetMaterial(i);
		// <Material
		DOMElement* matNode = m_XML.CreateNode(lib,"Material");
		BYTE* buf;
		UuidToStringA(&mat->m_GUID,&buf);
		m_XML.Attrib("GUID",buf);
		m_XML.Attrib("Name",mat->m_Name);
		m_XML.Attrib("Category",mat->m_Category);

		// TODO: Submaterials

		DOMElement* shader = m_XML.CreateNode(matNode,"Shader");
		m_XML.Attrib("File",mat->m_Shader->GetFilename());
		m_XML.Attrib("Technique",mat->m_Token);
		for(int j=0;j<mat->m_Parameters.size();j++)
		{
			ShaderVar* var = mat->m_Parameters[j];
			m_XML.WriteParameter((TCHAR*)mat->m_Parameters[j]->name.c_str(),mat->m_Parameters[j]->type,(BYTE*)mat->m_Parameters[j]->data,shader);
		}
	}
}


//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
bool MaterialSerializer::Load(string filename, MaterialLibrary* lib)
{
	FindMedia(filename,"Materials");
	m_Library = lib;
	m_Library->FileName = filename;
	if(!m_XML.Load(filename)){
		return false;
	}

	// use the tree walker to recurse the nodes
	DOMTreeWalker* walker = m_XML.GetWalker();
	DOMElement* pCurrent;

	for ( pCurrent = (DOMElement*)walker->nextNode(); pCurrent != 0; pCurrent = (DOMElement*)walker->nextSibling() )
	{
		int type = pCurrent->getNodeType();
		switch (type) {
		case DOMNode::ELEMENT_NODE:
			{
				// Top-Level objects
				if(m_XML.GetName(pCurrent) == "Material")
					ReadMaterial(pCurrent);
			}
		}
	}

	m_XML.Destroy();
	return true;
}


//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
bool MaterialSerializer::Save(string name, MaterialLibrary* lib)
{
	m_Library = lib;
	m_Library->FileName = name;

	xercesc_2_5::DOMDocument* doc = m_XML.CreateDocument("MaterialLibrary");

	DOMElement* root = doc->getDocumentElement();

	WriteMaterials(root);

	m_XML.Save(name);

	return true;
}




