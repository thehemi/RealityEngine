#include <stdafx.h>
#include <mmsystem.h>

const int MAP_VERSION = 5101; // MUST BE CHANGED IN EXPORTER WHEN THERE IS A MAJOR BUILD

//----------------------------------------------------------------------------------
// Desc: Misc crap
//----------------------------------------------------------------------------------
unsigned long bytes = 0;
int wrotePools, wroteVerts, wroteFaces;

// Writes an array
#define WRITE(data,size) \
	file.write((char*)(data),(size)); \
	bytes += (size);
 
// Writes an object
#define Write(x) (file.write((char*)&x,sizeof(x))); \
	bytes += sizeof(x);

int lastInt;
#define WriteInt(x) {lastInt = x; (file.write((char*)&lastInt,sizeof(lastInt))); \
	bytes += sizeof(lastInt); }

void Exporter::WriteString(string& str){
	int len = str.length();
	file.write((char*)&len,sizeof(int));
	file.write(str.c_str(),len);
}


//----------------------------------------------------------------------------------
// Desc: Export a rendering node pool
//----------------------------------------------------------------------------------
void Exporter::ExportPool(Pool& pool){
	if(pool.matID > 600)
		Error("pool.matID > 600");

	Write(pool.meshID);
	Write(pool.box);
	Write(pool.subset); // Write attribute table
	WriteInt(pool.staticLights.size());
	for(int i=0;i<pool.staticLights.size();i++){
		Write(pool.staticLights[i]->keyframes[0].tm[3]);
	}

}


//----------------------------------------------------------------------------------
// Desc: Export an entity + possible mesh
//----------------------------------------------------------------------------------
void Exporter::ExportEntitiesAndPrefabs(vector<Entity>& entities){
	WriteInt(entities.size());
	for(int i=0;i<entities.size();i++){
		bool hasModel = entities[i].modelFrame != NULL;
		Write(hasModel);
		if(hasModel)
			ExportFrames(entities[i].modelFrame);

		Write(entities[i].worldTM);

		NodeData& d = entities[i].data;
		WriteString(d.filename);
		WriteString(d.classname);
		WriteString(d.parentclass);

		WriteInt(d.parameters.size());
		for(int j=0;j<d.parameters.size();j++)
			WriteString(d.parameters[j]);
		WriteInt(d.paramvalues.size());
		for(int j=0;j<d.paramvalues.size();j++)
			WriteString(d.paramvalues[j]);
	}
}

//----------------------------------------------------------------------------------
// Desc: Export a mesh with subsets
//----------------------------------------------------------------------------------
void Exporter::ExportMesh(Mesh* mesh){
	// Build attrib table
	D3DXATTRIBUTERANGE  attribTable[100]; 
	DWORD				attribTableSize;
	AttributeRange		table[100];
	mesh->GetMesh()->GetAttributeTable(attribTable,&attribTableSize);
	for(int i=0;i<attribTableSize;i++){
		table[i].AttribId		= attribTable[i].AttribId;
		table[i].FaceCount		= attribTable[i].FaceCount;
		table[i].FaceStart		= attribTable[i].FaceStart;
		table[i].VertexCount	= attribTable[i].VertexCount;
		table[i].VertexStart	= attribTable[i].VertexStart;
		table[i].VertexOffset = 0;
	}

	BBox box;
	vector<WORD> indices;

	Write(mesh->m_HasSHFile);

	// Removed: PRT data is now stored in seperate files
	/*if(build.m_PRTEnabled){
		vector<SHVertex> vertices;
		mesh->GetSHData(indices,vertices,box);

		Write(box);
		// Write attribute table
		Write(attribTableSize);
		WRITE(table,sizeof(AttributeRange)*attribTableSize);

		// Vertices...
		WriteInt(vertices.size());
		WRITE(&vertices[0],sizeof(SHVertex)*vertices.size());
		wroteVerts += vertices.size();
	}
	else{*/
		vector<Vertex> vertices;
		mesh->GetData(indices,vertices,box);

		Write(box);
		// Write attribute table
		Write(attribTableSize);
		WRITE(table,sizeof(AttributeRange)*attribTableSize);

		// Vertices...
		WriteInt(vertices.size());
		WRITE(&vertices[0],sizeof(Vertex)*vertices.size());
		wroteVerts += vertices.size();

	// Indices output...
	WriteInt(indices.size());
	//if(mesh->Stripified)
	//	wroteFaces += indices.size() - 2;
	//else
	wroteFaces +=indices.size() / 3;
	WRITE(&indices[0],sizeof(WORD)*indices.size());

	vector<Mesh::SkinWeights> bones;
	mesh->GetBones(bones);
	WriteInt(bones.size());
	for(int i=0;i<bones.size();i++){
		WriteString(bones[i].name);
		Write(bones[i].offset);
		WriteInt(bones[i].indices.size());
		WRITE(&bones[i].indices[0],sizeof(bones[i].indices[0])*bones[i].indices.size());
		WriteInt(bones[i].weights.size());
		WRITE(&bones[i].weights[0],sizeof(bones[i].weights[0])*bones[i].weights.size());
	}

	ExportMaterials(mesh->materials);
}


//----------------------------------------------------------------------------------
// Desc: Exports model frame hierarchy
//----------------------------------------------------------------------------------
void Exporter::ExportFrames(ExportFrame* frame){
	WriteString(frame->name);
	Write(frame->tm);

	bool hasMesh	= frame->mesh && frame->mesh->GetNumSubsets() > 0;
	bool hasCollisionData = frame->collisionData.vertices.size() > 0;
	bool hasLight	= frame->light.keyframes.size() > 0;
	bool hasChild	= frame->m_pChild != 0;
	bool hasNext	= frame->m_pNext != 0;

	Write(hasMesh);
	if(hasMesh)
		ExportMesh(frame->mesh);

	Write(hasCollisionData);
	if(hasCollisionData)
		ExportCollisionData(frame->collisionData);

	// TODO: Add entity output

	Write(hasLight);
	// TODO: Support lights
	//if(hasLight)
	//	ExportLight(frame->light);

	Write(hasChild);
	if(hasChild)
		ExportFrames(frame->m_pChild);

	Write(hasNext);
	if(hasNext)
		ExportFrames(frame->m_pNext);
}

//----------------------------------------------------------------------------------
// Desc: Write the scene rendering tree
//----------------------------------------------------------------------------------
void Exporter::ExportRenderNode(Node& node){
	Write(node.box);
	WriteInt(node.pools.size());
	for(int i=0;i<node.pools.size();i++){
		ExportPool(node.pools[i]);
	}

	Write(node.numChildren);

	for(int i=0;i<node.numChildren;i++)
		ExportRenderNode(*node.children[i]);
}

//----------------------------------------------------------------------------------
// Desc: Write the scene rendering & collision tree
//----------------------------------------------------------------------------------
void Exporter::ExportCollisionData(CollisionData& data){
	Write(data.localspace);
	WriteInt(data.vertices.size());
	WRITE(&data.vertices[0],sizeof(Vector)*data.vertices.size());

	WriteInt(data.indices.size()/3);
	// Write all the faces out
	for(DWORD i=0;i<data.indices.size()/3;i++){
		Write(data.indices[i*3 + 2]);
		Write(data.indices[i*3 + 1]);
		Write(data.indices[i*3 + 0]);
		Write(data.faceIDs[i])
	}
}

//----------------------------------------------------------------------------------
// Desc: Write the scene rendering & collision tree
//----------------------------------------------------------------------------------
void Exporter::ExportStaticGeometry(LevelFile& level){
	// Write the big fat meshes for the scene tree..
	WriteInt(level.meshes.size());
	for(int i=0;i<level.meshes.size();i++){
		ExportMesh(level.meshes[i]);
		delete level.meshes[i]; // Free them now
	}

	LogPrintf("File after writing static rendering vertex buffers: %dKB\n", bytes/1024);
	// Write static rendering tree
	ExportRenderNode(level.root);
	LogPrintf("File after writing static rendering tree: %dKB\n", bytes/1024);
	ExportCollisionData(level.collisionData);
}


//----------------------------------------------------------------------------------
// Desc: Outputs all texture maps
//----------------------------------------------------------------------------------
void Exporter::RecursiveTextureWrite(TexMap& map){
	Write(map.type);
	WriteString(map.texVar);
	WriteString(map.transformVar);
	Write(map.amount);
	WriteString(map.filename);

	// TODO: See if this is necessary in the new system
	// Whacky maths to deal with 3dsmax material tiling
	// calculations, which work from the middle outwards
	// The first part is because 3dsmax calculates the offset
	// as a factor of the tiling.
	// So 0.25 offset is used to move a 2.0 tiled texture 0.5 units
	float UOff = (map.uOff * map.uTile) + (map.uTile/2.f) + 0.5f;
	float VOff = (map.vOff * map.vTile) + (map.vTile/2.f) + 0.5f;

	Write(UOff);
	Write(VOff);

	Write(map.uTile);
	Write(map.vTile);
}

//----------------------------------------------------------------------------------
// Desc: Exports a material + maps
//----------------------------------------------------------------------------------
void Exporter::ExportMaterial(Material& mat){
	WriteString(mat.name);
	WriteString(mat.shader);
	WriteString(mat.technique);
	Write(mat.bReference);
	
	if(mat.name == "NULL" || mat.technique == "" || mat.bReference)
		return;
	
	WriteInt(mat.id);
	WriteInt(mat.params.size());
	for(int i=0;i<mat.params.size();i++){
		WriteString(mat.params[i].name);
		WriteInt(mat.params[i].type);
		
		switch(mat.params[i].type){
		case PARAM_INT:
			WriteInt(sizeof(int));
			WriteInt(mat.params[i].iVal);
			break;
		case PARAM_TEXTURE:
			WriteString(mat.params[i].sVal);
			break;
		case PARAM_FLOAT:
			WriteInt(sizeof(float));
			Write(mat.params[i].fVal);
			break;
		case PARAM_FLOAT3:
			WriteInt(sizeof(Vector));
			Write(*(Vector*)&mat.params[i].vVal);
			break;
		case PARAM_FLOAT4:
			WriteInt(sizeof(D3DXCOLOR));
			Write(mat.params[i].vVal);
			break;
		default:
			Error("Shader type %d not found.\nThis is an internal error. Inform tim@helixcore.com",mat.params[i].type);
		}

	}

	WriteInt(mat.maps.size());
	for(int i=0;i<mat.maps.size();i++){
		RecursiveTextureWrite(mat.maps[i]);
	}
}

//----------------------------------------------------------------------------------
// Desc: Exports all materials
//----------------------------------------------------------------------------------
void Exporter::ExportMaterials(vector<Material>& materials){
	WriteInt(materials.size());
	for(int i=0;i<materials.size();i++){
		ExportMaterial(materials[i]);
	}
}

//----------------------------------------------------------------------------------
// Desc: Exports a light frame
//----------------------------------------------------------------------------------
void Exporter::ExportLightFrame(LightFrame& l){
	Vector Position	= l.tm[3];
	Vector Direction = -l.tm[1]; // WTF?? For some reason this is correct
	FloatColor col(l.color.r,l.color.g,l.color.b);
	col.a = l.intensity; // Put Intensity in alpha, for constant compacting

	Write(l.time);

	Write(col); // Diffuse
	Write(col); // Spec
	Write(Position);
	Write(Direction);
	Write(l.attenEnd); // Range
	Write(l.falloff); // Hotspot falloff
	Write(l.hotsize); // Hitspot size
}

//----------------------------------------------------------------------------------
// Desc: Exports a list of lights
//----------------------------------------------------------------------------------
void Exporter::ExportLight(Light& light){
	// Write unanimated attributes
	WriteString(light.name);
	Write(light.box);
	Write(light.type);
	WriteString(light.projectionMap);
	WriteString(light.shadowMap);

	// Write include/exclude list
	Write(light.IsExcludeList);
	WriteInt(light.excludeIncludeList.size());
	for(int j=0;j<light.excludeIncludeList.size();j++)
		WriteString(light.excludeIncludeList[j]);

	// Write all keyframes
	WriteInt(light.keyframes.size());
	for(int j=0;j<light.keyframes.size();j++)
		ExportLightFrame(light.keyframes[j]);
}

//----------------------------------------------------------------------------------
// Desc: Exports globals (sky, ambient, etc) here
//----------------------------------------------------------------------------------
void Exporter::ExportSceneProperties(SceneProperties& props){
	WriteString(string(props.skyworld));
	Write(props.fogColor);
	Write(props.fogStart);
	Write(props.clipPlane);
	WriteString(string(props.miniMap));
	Write(props.miniMapScale);
	WriteString(string(props.cubeMap));
}

//----------------------------------------------------------------------------------
// Desc: Entrypoint for saving level
//----------------------------------------------------------------------------------
void Exporter::ExportLevelFile(string filename, LevelFile& level){
	wrotePools = 0;
	wroteVerts = 0;
	wroteFaces = 0;

	file.open(filename.c_str(), ios::binary);
	if(!file){
		Error("Couldn't write to file '%s'",filename.c_str());
		return;
	}

	cout << "Exporting level...";
	Write(MAP_VERSION);

	Write(level.root.box);

	ExportSceneProperties(level.sceneProperties);

	WriteInt(level.lights.size());
	for(int i=0;i<level.lights.size();i++){
		ExportLight(level.lights[i]);
	}

	ExportStaticGeometry(level);

	ExportEntitiesAndPrefabs(level.entities);

	file.close();
	LogPrintf("Exported to %s\n",filename.c_str());
	LogPrintf("Wrote %d pools, %d faces, and %d vertices\n",wrotePools,wroteFaces,wroteVerts);
}

//----------------------------------------------------------------------------------
// Desc: Entrypoint for saving level
//----------------------------------------------------------------------------------
void Exporter::ExportModelFile(string filename, ModelFile& model){
	wrotePools = 0;
	wroteVerts = 0;
	wroteFaces = 0;

	file.open(filename.c_str(), ios::binary);
	if(!file){
		Error("Couldn't write to file '%s'",filename.c_str());
		return;
	}

	LogPrintf("Exporting model...");
	Write(MAP_VERSION);
	//ExportLights(model.lights);
	ExportFrames(model.root);

	file.close();
	LogPrintf("exported to %s\n",filename.c_str());
	LogPrintf("Wrote %d pools, %d faces, and %d vertices\n",wrotePools,wroteFaces,wroteVerts);
}