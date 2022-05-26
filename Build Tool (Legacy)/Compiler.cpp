//----------------------------------------------------------------------------------
// Compiler.cpp - All data manipulation and preparation is done here before
// being passed to the exporter.
//
// TODO: Use different method to deal with faces that cross boundaries
// Current parent node method creates too many extra pools (~20%)
//
// Tangent notes:
// ComputeNormals() seems to give better normals than 3dsmax (e.g. try a light on a square plane), 
// except on boxes, where it 100% breaks.
//
// NOTE: Keep an eye on matIds
//
// TODO: Lights on Models
// TODO: Animated texture keyframes
//
// For reference, standard collapsing/TUV insertion (with some welding):
// Teapot: 1024f/530v --> 992f/792v
// Sphere: 960f/482v  --> 960f/559v
// Box:    1200f/602v --> 1200f/726v
//
//----------------------------------------------------------------------------------
#include <stdafx.h>
#include <MMSystem.h>

CPRTSimulator    g_Simulator;

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void GenerateLightmapUVs(vector<Vertex>& vertices, vector<WORD>& indices)
{
	/*
	The technique used to do this is called planar mapping, and is very useful and pretty simple. 
	First you have to decide which plane to map on, this is done by check which component of the polygons normal 
	that is largest. If X is largest the you map on the YZ(or ZY) plane, if Y is largest the map on the XZ, then naturally 
	if Z is the largest you map to the XY plane. Remember to check the absolute value of the normal component, fabs(). 
	Now let's say that Y was largest, and you want to map to the XZ plane, then you just assign your lightmaps U values 
	*/
	for(int i=0;i<vertices.size();i++)
	{
		float x = fabsf(vertices[i].n.x);
		float y = fabsf(vertices[i].n.y);
		float z = fabsf(vertices[i].n.z);
		// X Major
		if(x > y && x > z){
			vertices[i].t.x = vertices[i].p.y;
			vertices[i].t.y = vertices[i].p.z;
		}
		// Y Major
		else if(y > x && y > z){
			vertices[i].t.x = vertices[i].p.x;
			vertices[i].t.y = vertices[i].p.z;
		}
		// Z Major
		else if(z > y && z > x){
			vertices[i].t.x = vertices[i].p.x;
			vertices[i].t.y = vertices[i].p.y;
		}
	}

	/*
	You could now say you have your UV's in 3D worldspace, but you want them in the 2D texture space. 
	To do that, you have to calculate the 2D bounding box of the polygon. That's easy, just set your 
	min and max UV's to the UV's of the first lightmap UV, and then loop though the others and see if they're 
	smaller of greater than that value
	*/
	Vector2 uvMin, uvMax;
	for(int i=0;i<indices.size();i+=3){
		struct Face{
			Vertex* v[3];
		};
		// Fill face with pointers, to simplify understanding
		Face face;
		face.v[0] = &vertices[indices[i + 0]];
		face.v[1] = &vertices[indices[i + 1]];
		face.v[2] = &vertices[indices[i + 2]];

		//uvMin.x = face.v[0]->t.x;
		//uvMin.y = face.v[0]->t.y;
		//uvMax.x = face.v[0]->t.x;
		//uvMax.y = face.v[0]->t.y;

		// Find max/min uvs for face
		for(int j = 0; j < 3; j++)
		{
			if( face.v[j]->t.x < uvMin.x )
				uvMin.x = face.v[j]->t.x;
			if( face.v[j]->t.y < uvMin.y )
				uvMin.y = face.v[j]->t.y;

			if( face.v[j]->t.x > uvMax.x )
				uvMax.x = face.v[j]->t.x;
			if( face.v[j]->t.y > uvMax.y )
				uvMax.y = face.v[j]->t.y;
		}

		// Find max/min uvs for ENTIRE mesh
	/*	if(uvMin.x < uvMin2.x)
			uvMin2.x = uvMin.x;
		if(uvMin.y < uvMin2.y)
			uvMin2.y = uvMin.y;
		if(uvMax.x > uvMax2.x)
			uvMax2.x = uvMax.x;
		if(uvMax.y > uvMax2.y)
			uvMax2.y = uvMax.y;*/
	}

	Vector2 uvDelta;
	uvDelta.x = uvMax.x - uvMin.x;
	uvDelta.y = uvMax.y - uvMin.y;
	for(int i=0;i<vertices.size();i++)
	{
		// Now get the Delta between these two, then you subtract the Min UV value from all lightmap UVs to 
		// make all lightmap UVs relative to that point.
		vertices[i].t.x -= uvMin.x;
		vertices[i].t.y -= uvMin.y;
		// But your lightmap coords must range from 0.0 to 1.0, 
		// so you divide you lightmap UVs by the Delta value. That's it. 
		vertices[i].t.x /= uvDelta.x;
		vertices[i].t.y /= uvDelta.y;
	}


	// Pack into quadrants
	for(int i=0;i<vertices.size();i++)
	{
		float x = fabsf(vertices[i].n.x);
		float y = fabsf(vertices[i].n.y);
		float z = fabsf(vertices[i].n.z);
		// X Major
		if(x > y && x > z){
			vertices[i].t.x = vertices[i].p.y;
			vertices[i].t.y = vertices[i].p.z;
		}
		// Y Major
		else if(y > x && y > z){
			vertices[i].t.x = vertices[i].p.x;
			vertices[i].t.y = vertices[i].p.z;
		}
		// Z Major
		else if(z > y && z > x){
			vertices[i].t.x = vertices[i].p.x;
			vertices[i].t.y = vertices[i].p.y;
		}
	}

}


//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void GenerateLightmapUVs(Mesh* mesh)
{
// Build the collision data directly from the render mesh
	if(mesh){
		for(int i=0;i<mesh->GetNumSubsets();i++){
			vector<Vertex> vertices;
			vector<WORD> indices;
			BBox box;
			mesh->GetData(indices,vertices,box);
			if(vertices.size()){
				GenerateLightmapUVs(vertices,indices);
				Vertex* VerticesBuffer;
				mesh->GetMesh()->LockVertexBuffer(D3DLOCK_DISCARD, (LPVOID*)&VerticesBuffer);
				memcpy(VerticesBuffer, &vertices[0], vertices.size() * sizeof(Vertex));
				mesh->GetMesh()->UnlockVertexBuffer();

				WORD* IndexesBuffer;
				mesh->GetMesh()->LockIndexBuffer(D3DLOCK_DISCARD, (LPVOID*)&IndexesBuffer);
				for(int i=0;i<indices.size();i++) IndexesBuffer[i] = indices[i];
				mesh->GetMesh()->UnlockIndexBuffer();

				//mesh->Create(vertices,indices,mesh->materials);
			}
		}
	}
}


//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
string Compiler::GetFrameSHFile(ImportFrame* frame)
{
	// Build mapname_data\meshname.ptr
	string file = build.outFile.substr(0,build.outFile.find_last_of(".")) + "_Data";
	CreateDirectory(file.c_str(),0);
	file += "\\" + string(frame->Name) + ".prt";
	return file;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
bool Compiler::FrameNeedsRecompiling(ImportFrame* frame, string file)
{
	// Only generate SH if we haven't already generated it for current mesh+timestamp
	bool bRecompile = true;
	// Extract file stamp
	HANDLE hFile = CreateFile(file.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);  
	if(hFile == INVALID_HANDLE_VALUE)
		return true; // File doesn't exist, so yes we must create it!

	FILETIME writeTime, moveTime;
	GetFileTime(hFile,0,0,&writeTime);
	CloseHandle(hFile);
	// Compare
	SystemTimeToFileTime(&frame->nodeData.timeMoved,&moveTime);
	if(CompareFileTime(&writeTime,&moveTime) == 1)
		return false; // Compiled file is newer, don't recompile

	return true;
}


//----------------------------------------------------------------------------------
// Desc: Pre-processes imported meshes and items
// 1. Applies scaling factor to all items
// 2. Generates tangents and correct normals for meshes
//----------------------------------------------------------------------------------
void Compiler::PreProcessTree(ImportFrame* frame){
	if(frame->m_pChild)
		PreProcessTree(frame->m_pChild);
	if(frame->m_pNext)
		PreProcessTree(frame->m_pNext);

	// If no material add NULL material. Engine will look it up and replace with default
	if(frame->materials.size() == 0){
		Material m;
		m.id = -1;
		m.name = "NULL";
		frame->materials.push_back(m);
	}

	if(frame->pMesh){
		// Build our compiled/scaled mesh
		frame->mesh->m_Name = frame->Name;
		frame->mesh->Create(frame->Name,frame->pMesh,frame->pSkinInfo,frame->pAdjacency,frame->materials,frame->CombinedTransformationMatrix);

		SAFE_RELEASE(frame->pMesh);
		SAFE_RELEASE(frame->pSkinInfo);
		SAFE_RELEASE(frame->pAdjacency);
	}
	// Remove scaling from matrices, now that it's encoded into mesh
	frame->CombinedTransformationMatrix = frame->CombinedTransformationMatrix.WithoutScaling();
	frame->TransformationMatrix = frame->TransformationMatrix.WithoutScaling();

	// Run PRT now if enabled
	if(build.m_PRTEnabled && frame->mesh && frame->nodeData.shEnabled)
	{
		frame->mesh->m_HasSHFile = true;
		// Since we have SH, let's rename our materials to differentiate them from non-SH copies
		// of the same material in max
		// Why? SH materials use different shaders!
		for(int i=0;i<frame->mesh->materials.size();i++)
			frame->mesh->materials[i].name += "_PRT";

		// Needs recompiling if frame mesh, or any blockers are modified
		string file = GetFrameSHFile(frame);
		bool bRecompile = false;
		if(FrameNeedsRecompiling(frame,file))
			bRecompile = true; 

		// Find inBlocker list
		vector<LPD3DXMESH>	inBlockers;
		vector<Matrix>		inBlockMats;

		for(int i=0;i<frame->nodeData.inBlockers.size();i++){
			ImportFrame* iFrame = m_pRootFrame->FindMeshFrame(frame->nodeData.inBlockers[i]);
			if(!iFrame){ // Must be outdated blocker list
				bRecompile = true;
				continue;
			}

			// See if blocker has moved, causing recompile
			if(!bRecompile && FrameNeedsRecompiling(iFrame,file))
				bRecompile = true; 

			if(iFrame->pMesh)
				inBlockers.push_back(iFrame->pMesh);
			else if(iFrame->mesh && iFrame->mesh->GetMesh())
				inBlockers.push_back(iFrame->mesh->GetMesh());
			else
				continue;

			// Put blocker into object space of receiver mesh
			Matrix objectSpace = iFrame->CombinedTransformationMatrix * frame->CombinedTransformationMatrix.Inverse();
			inBlockMats.push_back(objectSpace);
		}

		if(bRecompile || frame->nodeData.sh.bAdaptive)
		{
			if(frame->mesh->GetNumSubsets() != 1)
				LogPrintf("Warning: %s has %d subsets, only 1 material subset supported for PRT",frame->Name,frame->mesh->GetNumSubsets());

			if(frame->nodeData.sh.bPerPixel)
				LogPrintf("Generating Per-Pixel PRT for %s ...",frame->Name);
			else
				LogPrintf("Generating Per-Vertex PRT for %s ...",frame->Name);

			LPD3DXMESH combinedMesh = NULL;
			// Concanceate all blocker meshes into a single blocker we can pass to simulator
			if(inBlockers.size())
			{
				// Note: Combined mesh is made 32-bit
				DXASSERT(D3DXConcatenateMeshes(&inBlockers[0],inBlockers.size(),D3DXMESH_MANAGED|D3DXMESH_32BIT,
					(D3DXMATRIX*)&inBlockMats[0],NULL,NULL,g_pd3dDevice,&combinedMesh));
			}

			CPRTMesh prtMesh;
			frame->mesh->GetMesh()->AddRef(); // Stop PRTMesh from destroying source mesh!
			prtMesh.SetMesh(g_pd3dDevice,frame->mesh->GetMesh());
			CPRTSimulator::SIMULATOR_OPTIONS pOptions;

			// Set these settings, they aren't set by default..
			pOptions.bSaveCompressedResults = true;
			pOptions.Quality = D3DXSHCQUAL_SLOWHIGHQUALITY;
			
			pOptions.CopyFrom(frame->nodeData.sh);

			if(pOptions.bPerPixel)
				pOptions.bAdaptive = false;

			// Receiver group will be filename
			wcscpy( pOptions.strResultsFile, ToUnicode(GetFrameSHFile(frame)).c_str() );

			// Run!!
			g_Simulator.Run(g_pd3dDevice,&pOptions,&prtMesh,combinedMesh);

			while(g_Simulator.IsRunning())
				Sleep(50);

			if(g_Simulator.m_bFailed)
				Error(g_Simulator.strError);

			if(inBlockers.size())
				SAFE_RELEASE(combinedMesh);

			LogPrintf("done!\n");
			// Get the result mesh, in case of adaptive tessellation
			if(pOptions.bAdaptive){
				//frame->mesh->Create(frame->mesh->m_Name,(LPD3DXMESH)prtMesh.GetMesh(),
				//	NULL,NULL,frame->mesh->materials,Matrix(),false);
				//prtMesh.GetMesh()->AddRef();
				frame->mesh->SetMesh(prtMesh.GetMesh());
			}
		}
	}

	if(frame->light.keyframes.size()){
		// TODO: Fix the whole light mess. Decide how we'll properly animate lights
		for(int i=0;i<frame->light.keyframes.size();i++)
			frame->light.keyframes[i].tm = frame->CombinedTransformationMatrix;

		// Maybe?: Scale the attenuation end (range) by matrix scaling
		// YES, this was purposely done before converting to D3D coordinates
		/*int scale = frame->light->tm[0].x;
		if(fabsf(scale)<0.01f) scale = 1; // Happens sometimes
		frame->light->attenEnd = frame->light->attenEnd*scale;

		// Maybe?: Convert any keyframes too
		for(int j=0;j<light->.keyframes.size();j++){
		Light& light = light->.keyframes[j];
		int scale = light.tm[0].x;
		if(fabsf(scale)<0.01f) scale = 1;
		light.attenEnd = light.attenEnd*scale;
		ConvertMaxMatrix(light.tm);
		}*/
	}
}


//----------------------------------------------------------------------------------
// Desc: Extracts faces from scene in WORLD SPACE, including material ids
//----------------------------------------------------------------------------------
void Compiler::GetStaticFaces(ImportFrame* srcFrame, vector<Face>& faces){
	if(srcFrame->m_pChild)
		GetStaticFaces(srcFrame->m_pChild,faces);
	if(srcFrame->m_pNext)
		GetStaticFaces(srcFrame->m_pNext,faces);

	// Entities are kept as models
	if(srcFrame->type != TYPE_STATIC_GEOMETRY)
		return;

	if(!srcFrame->mesh || srcFrame->mesh->GetNumSubsets() == 0)
		return;

	vector<int> materialLookup;

	// For each material, see if it's stored, if not store it
	// Then store a global material id for this index
	for(int i=0;i<srcFrame->materials.size();i++){
		int index = -1;
		// Search global array
		for(int j=0;j<m_GlobalMaterials.size();j++){
			if(m_GlobalMaterials[j].name == srcFrame->mesh->materials[i].name){
				index = j;
				break;
			}
		}
		// Not stored, so store it
		if(index == -1){
			m_GlobalMaterials.push_back(srcFrame->materials[i]);
			index = m_GlobalMaterials.size() - 1;
		}

		// Store look-up ID
		materialLookup.push_back(index);
	}

	srcFrame->mesh->GetAllFaces(srcFrame,faces,srcFrame->CombinedTransformationMatrix, materialLookup);
}


//----------------------------------------------------------------------------------
// Desc: Gets all entities/prefabs from tree
//----------------------------------------------------------------------------------
void Compiler::GetAllEntities(ImportFrame* srcFrame, vector<Entity>& entities){
	if(srcFrame->m_pChild)
		GetAllEntities(srcFrame->m_pChild,entities);
	if(srcFrame->m_pNext)
		GetAllEntities(srcFrame->m_pNext,entities);

	if(srcFrame->type != TYPE_ENTITYORPREFAB)
		return;

	Entity e;
	// If an entity is tagged to a mesh we'll include it and all children
	// but we must discard sibling frames, because they aren't actually related
	if(srcFrame->mesh && srcFrame->mesh->GetNumSubsets()){
		ImportFrame* oldSibling = srcFrame->m_pNext;
		srcFrame->m_pNext = NULL;
		CompileHierarchy(srcFrame,e.modelFrame);
		srcFrame->m_pNext = oldSibling;
	}
	else
		srcFrame->materials.clear(); // Some non-mesh frames have materials. I may be wrong on this

	e.worldTM = *(Matrix*)&srcFrame->CombinedTransformationMatrix;
	e.data    = srcFrame->nodeData;

	entities.push_back(e);
}

//----------------------------------------------------------------------------------
// Desc: Extracts lights from hierarchy
//----------------------------------------------------------------------------------
void Compiler::GetAllLights(ImportFrame* srcFrame, vector<Light>& lights){
	if(srcFrame->m_pChild)
		GetAllLights(srcFrame->m_pChild,lights);
	if(srcFrame->m_pNext)
		GetAllLights(srcFrame->m_pNext,lights);

	// Get light
	if(srcFrame->light.keyframes.size())
		lights.push_back(srcFrame->light);
}



//----------------------------------------------------------------------------------
// MODEL FILE COMPILING CODE
//
//----------------------------------------------------------------------------------


//----------------------------------------------------------------------------------
// Desc: Compiles a hierarchy. Used by ModelFile alone
//----------------------------------------------------------------------------------
void Compiler::CompileHierarchy(ImportFrame* srcFrame, ExportFrame*& outFrame){
	outFrame = new ExportFrame;
	outFrame->name = srcFrame->Name;
	outFrame->tm   = srcFrame->TransformationMatrix;
	outFrame->mesh = srcFrame->mesh;
	outFrame->light = srcFrame->light;

	if(srcFrame->mesh && srcFrame->mesh->GetNumSubsets() && (srcFrame->mesh->GetNumSubsets() > srcFrame->materials.size())){
		Warning("Not expected, could be serious: NumSubsets != NumMaterials (%d v %d) on '%s'. Please tell tim@helixcore.com",srcFrame->mesh->GetNumSubsets(),srcFrame->materials.size(),srcFrame->Name);

		for(int i=0;i<srcFrame->materials.size();i++){
			Warning("Material #%d is '%s'",i,srcFrame->materials[i].name.c_str());
		}
	}

	// Build the collision data directly from the render mesh
	if(srcFrame->mesh){
		for(int i=0;i<srcFrame->mesh->GetNumSubsets();i++){
			vector<Vertex> vertices;
			vector<WORD> indices;
			BBox box;
			srcFrame->mesh->GetData(indices,vertices,box,i);
			if(vertices.size())
				AddCollisionData(outFrame->collisionData,vertices,indices,srcFrame->materials[i].id,true);
		}
	}

	if(srcFrame->m_pChild)
		CompileHierarchy(srcFrame->m_pChild,outFrame->m_pChild);
	if(srcFrame->m_pNext)
		CompileHierarchy(srcFrame->m_pNext,outFrame->m_pNext);
}

//----------------------------------------------------------------------------------
// Desc: Compiles a model file
//----------------------------------------------------------------------------------
void Compiler::CompileModelFile(ImportFrame* srcFrame, ModelFile& out){
	m_pRootFrame = srcFrame;
	PreProcessTree(srcFrame);
	CompileHierarchy(srcFrame, out.root);
}

//----------------------------------------------------------------------------------
// LEVEL FILE COMPILING CODE
//
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
// Desc: GeneratePoolsFromFaces
// Creates a UNIQUE vertex pool and indices into this pool
// There may be more output vertices than MAX reports for the model
// because MAX can decouple texture coordinates and we can't
//----------------------------------------------------------------------------------
#define MAX_INDICES_PER_POOL 65532
bool Compiler::GeneratePoolsFromFaces(vector<Face>& faces,vector<Pool>& pools){
	//
	// 1. Create grossly unoptimized pools
	//
	for(int i=0;i<faces.size();i++){
		// What material pool does this face go in?
		Pool* pool = 0;
		for(int a=0;a<pools.size();a++){
			if(faces[i].flatmatid == pools[a].matID && pools[a].indices.size() < MAX_INDICES_PER_POOL){
				pool = &pools[a];
			}
		}

		// Create a new pool for this material
		if(!pool){
			Pool p;
			p.matID = faces[i].flatmatid;
			pools.push_back(p);
			pool = &pools[pools.size()-1];
		}

		pool->box += faces[i].v[0].p;
		pool->box += faces[i].v[1].p;
		pool->box += faces[i].v[2].p;

		pool->vertices.push_back(faces[i].v[0]);
		pool->vertices.push_back(faces[i].v[1]);
		pool->vertices.push_back(faces[i].v[2]);
		int index = pool->indices.size();
		pool->indices.push_back(index+0);
		pool->indices.push_back(index+1);
		pool->indices.push_back(index+2);
	}

	//
	// 2. Optimize and weld each pool
	//
	for(int i=0;i<pools.size();i++){
		if(pools[i].vertices.size() < 3){
			Warning("Skipping a corrupt piece of geometry (<3 faces)");
			continue;
		}

		Mesh mesh;
		mesh.Create(pools[i].vertices,pools[i].indices);
		mesh.Optimize(true);
		mesh.GetData(pools[i].indices,pools[i].vertices,pools[i].box);

		if(pools[i].vertices.size() == 0 || pools[i].indices.size() == 0)
			cout << "Error creating vertex pool" << endl;
	}

	if(pools.size() == 0)
		return false;
	else
		return true;
}



//----------------------------------------------------------------------------------
// Recurses nodes putting all pools in a big list
// Used by MergePools, below
// Returns total number of vertices
//----------------------------------------------------------------------------------
long GetPools(vector<Pool*>& pools, Node& node){
	long verts = 0;
	for(int i=0;i<node.pools.size();i++){
		verts += node.pools[i].vertices.size();
		pools.push_back(&node.pools[i]);
	}

	for(int i=0;i<node.numChildren;i++){
		verts += GetPools(pools,*node.children[i]);
	}
	return verts;
}

//----------------------------------------------------------------------------------
// Desc: Merges all the little pools into big fat static VBs
//
// NOTE: Currently uses only one buffer, as D3D can store 2^32 indices this way
// as long as we use the offset. Only issue is blowing the memory on cards
//
// FIXME: This will categorically not work for multiple world meshes until we
// offset the material arrays into the unique meshes and only include materiasl
//
//----------------------------------------------------------------------------------
// Use these to limit huge meshes for level data
#define MAX_MESH_VERTICES 1000000
#define MAX_MESH_INDICES 65500 // Stupid D3D meshes don't use offsets
void Compiler::MergeTreePools(Node& root, vector< Mesh* > & meshes){
	vector<Pool*> pools;
	long verts = GetPools(pools,root);

	// We'll fill these and empty them into a mesh
	// each time they reach our target
	vector<Vertex> vertices;
	vector<WORD> indices;
	vertices.reserve(verts<MAX_MESH_VERTICES?verts:MAX_MESH_VERTICES);
	indices.reserve(verts*2<MAX_MESH_INDICES?verts*2:MAX_MESH_INDICES);

	vector<Material> newMats;
	int matOffset = 0;
	for(int i=0;i<pools.size();i++){
		pools[i]->meshID = meshes.size();

		// D3D adds these offset to all indices internally,
		// so we only pass the relative information + offsets
		pools[i]->subset.FaceCount = pools[i]->indices.size() / 3;
		pools[i]->subset.FaceStart = indices.size() / 3;
		pools[i]->subset.VertexCount = pools[i]->vertices.size();
		// We use the offset, not the start, so that indices always stay within WORD range
		pools[i]->subset.VertexStart = 0;//vertices.size(); 
		pools[i]->subset.VertexOffset = vertices.size();

		// Make sure this material hasn't been added to this global mesh yet
		int index = -1;
		for(int j=0;j<newMats.size();j++){
			if(m_GlobalMaterials[pools[i]->matID].name == newMats[j].name){
				index = j;
				break;
			}
		}
		if(index == -1){
			newMats.push_back(m_GlobalMaterials[pools[i]->matID]);
			index = newMats.size() - 1;
		}


		pools[i]->subset.AttribId = index;


		// Add the arrays to the big fat global arrays
		vertices.insert(vertices.end(), pools[i]->vertices.begin(), pools[i]->vertices.end());
		indices.insert(indices.end(), pools[i]->indices.begin(), pools[i]->indices.end());

		// Time to empty VB or IB?
		if(vertices.size() > MAX_MESH_VERTICES || indices.size() > MAX_MESH_INDICES){
			Mesh* mesh = new Mesh;
			mesh->Create(vertices,indices,newMats);
			// TODO: See how fast this is. It's better than doing it
			// in the other function, but probably too slow
			//mesh->Optimize(true);
			meshes.push_back(mesh);

			vertices.clear();
			indices.clear();
			newMats.clear();
		}
	}

	// Push back final mesh
	if(vertices.size() > 0){
		Mesh* mesh = new Mesh;
		mesh->Create(vertices,indices,newMats);
		meshes.push_back(mesh);
	}
}

//----------------------------------------------------------------------------------
// Desc: Add collision data from vertex/index list.
// matID is used to subsequently identify the faces in game, for 
// footstep material lookup, etc
//----------------------------------------------------------------------------------
void Compiler::AddCollisionData(CollisionData& data, vector<Vertex>& vertices, vector<WORD>& indices, int matID,  bool localSpace){
	int offset = data.vertices.size();

	// Add vertices
	for(int j=0;j<vertices.size();j++)
		data.vertices.push_back(vertices[j].p);

	// Add indices + offset
	for(int j=0;j<indices.size()/3;j++){
		// Flip face ordering
		data.indices.push_back(indices[j*3 + 2] + offset);
		data.indices.push_back(indices[j*3 + 1] + offset);
		data.indices.push_back(indices[j*3 + 0] + offset);
	}

	// Add face ids
	for(int j=0;j<indices.size()/3;j++)
		data.faceIDs.push_back(matID);

	data.localspace = localSpace;
}

//----------------------------------------------------------------------------------
// Desc: Extract the collision vertices from our rendering tree
// It's not the most efficient representation, but it saves having to do another
// near-exponential vertex merge
//----------------------------------------------------------------------------------
void Compiler::GetCollisionDataFromTree(Node& node, CollisionData& data){
	for(int i=0;i<node.numChildren;i++){
		GetCollisionDataFromTree(*node.children[i],data);
	}

	for(int i=0;i<node.pools.size();i++){
		Pool& p = node.pools[i];
		AddCollisionData(data,p.vertices,p.indices,m_GlobalMaterials[p.matID].id,false);
	}
}

//----------------------------------------------------------------------------------
// Desc: Compiles a level file including scene tree
//----------------------------------------------------------------------------------
void Compiler::CompileLevelFile(ImportFrame* srcFrame, LevelFile& out){
	m_pRootFrame = srcFrame;
	assert(srcFrame->sceneProperties);
	out.sceneProperties = *srcFrame->sceneProperties;

	float start = timeGetTime();
	LogPrintf("Preprocessing...\n");
	SetProgress(25);
	PreProcessTree(srcFrame);
	SetProgress(35);

	GetAllLights(srcFrame,out.lights);
	SetProgress(40);

	vector<Face> faces;
	GetStaticFaces(srcFrame,faces);
	SetProgress(45);

	GetAllEntities(srcFrame,out.entities);

	LogPrintf("Completed preprocess. Took %f seconds\n",(timeGetTime() - start) / 1000.f);

	// Calculate the entire world's bounding box
	BBox box;
	if(faces.size() == 0){
		// No static geometry, give default size
		box.min = -Vector(500,500,500);
		box.max = Vector(500,500,500);
	}

	for (int i=0; i<faces.size(); i++){
		box += faces[i].v[0].p;
		box += faces[i].v[1].p;
		box += faces[i].v[2].p;
	}

	out.root.box = box;

	start = timeGetTime();
	LogPrintf("Creating static rendering tree minus prefabs/entities. There are %d faces...",faces.size());
	BuildTree(&out.root,faces,0,MinRenderNodeSize,0,false);
	SetProgress(60);

	int nodeCount = 0;
	LogPrintf("completed, took %f seconds\n",(timeGetTime() - start) / 1000.f);
	start = timeGetTime();
	LogPrintf("Indexing and optimizing static rendering tree...");
	GenerateLitAndOptimizedPools(out.lights,out.root,nodeCount);
	LogPrintf("completed, took %f seconds. Nodes with data in: %d\n",(timeGetTime() - start) / 1000.f,nodeCount);
	SetProgress(70);
	start = timeGetTime();
	LogPrintf("Consolidating rendering tree...");
	GetCollisionDataFromTree(out.root,out.collisionData);
	SetProgress(80);
	MergeTreePools(out.root,out.meshes);
	SetProgress(90);
	LogPrintf("...completed, took %f seconds\n",(timeGetTime() - start) / 1000.f);
}

//----------------------------------------------------------------------------------
// Where it all happens
//----------------------------------------------------------------------------------
Compiler::Compiler(bool createStrips, int minRenderNodeSize){
	MinRenderNodeSize = minRenderNodeSize;
	CreateStrips = createStrips;
}

Compiler::~Compiler(){
}

