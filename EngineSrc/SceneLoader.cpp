//=========== (C) Copyright 2003, Tim Johnson. All rights reserved. ===========
// Name: MapAndModelLoader.h
// Desc: Map loading class
//
// TODO ASAP: Support model-tagged lights and entities
//=============================================================================
#include "stdafx.h"
#include "SceneLoader.h"
#include "Collision.h"
#include "Frame.h"
#include "ispatialpartition.h"
#include "ShadowMapping.h"

const int EXPORTER_VERSION = 5101; // MUST MATCH LATEST BUILD TOOL

// Super-debugging Read statement!
//#define Read(x) {fread(&x,sizeof(x),1,file); LogPrintf(#x##" = %d",x);}
#define Read(x) fread(&x,sizeof(x),1,m_File);
#define ReadVector(v) v.resize(ReadInt()); fread(&v[0],sizeof(v[0]),v.size(),m_File);
#define ReadStringVector(v) v.resize(ReadInt()); for(int m=0;m<v.size();m++) ReadString(v[m]);
//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void SceneLoader::ReadString(char* str){
	int len;
	fread(&len,sizeof(int),1,m_File);
	fread(str,len,1,m_File);
	str[len] = '\0';
}
//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void SceneLoader::ReadString(string& str){
	int len;
	fread(&len,sizeof(int),1,m_File);
	str.resize(len);
	fread((char*)str.c_str(),len,1,m_File);
}
//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
string SceneLoader::ReadString(){
	string str;
	ReadString(str);
	return str;
}
//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
int SceneLoader::ReadInt(){
	int i;
	Read(i);
	return i;
}
//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
float SceneLoader::ReadFloat(){
	float f;
	Read(f);
	return f;
}
//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
DWORD SceneLoader::ReadDWORD(){
	DWORD d;
	Read(d);
	return d;
}
//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
bool SceneLoader::ReadBool(){
	bool b;
	Read(b);
	return b;
}

//-----------------------------------------------------------------------------
// !!TEMP!! Class/Entity data
//-----------------------------------------------------------------------------
struct NodeData {
	string	 filename;
	string	 classname;
	string	 parentclass;
	vector<string> parameters;
	vector<string> paramvalues;

	operator == (NodeData& rhs){
		return filename == rhs.filename && paramvalues.size() == rhs.paramvalues.size();
	}
};
//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void SceneLoader::LoadTextureMap(Material*& mat, Texture*& map){
	map = new Texture;
	Read(map->type);
	string texName = ReadString();
	// FIXME: Remove this
	string texTransform = ReadString();
	// UNUSED: Amount should also be used for other non-bump textures eventually
	ReadFloat();//Read(map->m_Amount);
	map->filename = ReadString();
	Read(map->uOff);
	Read(map->vOff);
	Read(map->uTile);
	Read(map->vTile);

	// Create shader vars for texture

	// U-Tile
	ShaderVar* var = new ShaderVar;
	var->Set(texName+"UTile",PARAM_FLOAT);
	var->data = &map->uTile;
	mat->m_Parameters.push_back(var);
	// V-Tile
	var = new ShaderVar;
	var->Set(texName+"VTile",PARAM_FLOAT);
	var->data = &map->vTile;
	mat->m_Parameters.push_back(var);
	// Texture object
	var = new ShaderVar;
	var->Set(texName,PARAM_TEXTURE);
	var->data = map;
	mat->m_Parameters.push_back(var);
}

//-----------------------------------------------------------------------------
// Reads material data from stream, but ignores it. Used when we've found a reference
//-----------------------------------------------------------------------------
void SceneLoader::DummyLoadMaterial(){
	ReadInt(); // m_id

	int numParams = ReadInt(); // numParams
	for(int i=0;i<numParams;i++){
		ReadString();	 // Param name
		ShaderParamType  type = (ShaderParamType)ReadInt();	 // Param type
		if(type == PARAM_TEXTURE){
			string filename = ReadString();
		}
		else{
			// Allocate misc item
			int size = ReadInt();					 // Param data size
			char* data = new char[size];			 // Create storage on stack
			fread(data,size,1,m_File);				 // Param data
			delete[] data;
		}
	}

	// Read in all textures
	int loop = ReadInt();
	for(int i=0;i<loop;i++){
		TextureType d;
		Read(d);
		ReadString();
		ReadString();
		ReadFloat();
		ReadString();

		ReadFloat();
		ReadFloat();
		ReadFloat();
		ReadFloat();
	}
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void SceneLoader::LoadMaterial(Material*& mat){
	string matName = ReadString();
	LogPrintf(LOG_MEDIUM,_U("Loading material: %s."),matName.c_str());

	string shaderName, technique;
	ReadString(shaderName);
	ReadString(technique);

	bool bReference = ReadBool();

	// Tim: Try referencing materials from the global array, including other models!!
	// FIXME: Dangerous
	if(matName != "NULL" && shaderName != "" && !bReference){
		Material* m = MaterialManager::Instance()->FindMaterial(matName);
		if(m){
			mat = m;
			mat->AddRef();
			DummyLoadMaterial();
			return;
		}
	}

	if(bReference){
		// Try to find material
		Material* m = MaterialManager::Instance()->FindMaterial(matName);
		if(m){
			delete mat;
			mat = m;
			mat->AddRef();
			return;
		}
		else
			Error("Material %s not found via ref sys. Have you been using instancing in your scene?\nThis is utterly not allowed. You must 'clone' or 'reference' meshes!",matName.c_str());
	}

	// Don't allow loading from external directories, will just confuse things
	shaderName = StripPath(shaderName);

	// Replace NULL materials with default mat
	if(matName == "NULL" || shaderName == ""){ 
		delete mat;
		mat = MaterialManager::Instance()->GetDefaultMaterial();
		mat->AddRef();
		return; // Safe to return because build tool returns here too
	}

	mat = new Material(matName);
	mat->m_ID = ReadInt();

	// Read in all shader parameters
	int numParams = ReadInt();
	if(numParams > 1000 || numParams < 0){
		Error("File is corrupted around %s. Try re-exporting",matName.c_str());
	}

	mat->m_Parameters.resize(numParams);
	for(int i=0;i<mat->m_Parameters.size();i++){
		mat->m_Parameters[i] = new ShaderVar;
		string			 name = ReadString();	 // Param name
		ShaderParamType  type = (ShaderParamType)ReadInt();	 // Param type

		if(type == PARAM_TEXTURE){
			// Allocate a texture
			Texture* tex = new Texture;
			// NORMAL MAP
			if(AsLower(name).find("bump") != -1 || AsLower(name).find("normal") != -1){
				if(!tex->Load(ReadString(),TT_NORMALMAP)){
					// Load default if couldn't find tex
					tex->Load("DefaultNormal.dds",TT_NORMALMAP);
				}
			}
			// DIFFUSE MAP or similar
			else{
				string filename = ReadString();
				// Try to set the material type from this map if it's not already been found
				if(mat->m_Type == "unknown")
					mat->m_Type = MaterialManager::Instance()->GetType(filename);

				if(!tex->Load(filename)){
					// Load default if couldn't find tex
					tex->Load("DefaultTexture.dds");
				}
			}

			if(!Engine::Instance()->IsDedicated())  // Copy data to shader var. It'll be auto-freed upon destruction
				mat->m_Parameters[i]->Set(name,type,ShaderVar::VAR_DELETETEXTURE,tex);
			else 
				delete tex;
		}
		else{
			// Allocate misc item
			int size = ReadInt();					 // Param data size
			char* data = new char[size];			 // Create storage on stack
			fread(data,size,1,m_File);				 // Param data
			if(!Engine::Instance()->IsDedicated())
				mat->m_Parameters[i]->Set(name,type,ShaderVar::VAR_DELETEARRAY,data);  // Copy data to shader var. It'll be auto-freed upon destruction
			else
				delete[] data;
		}
	}

	// Read in all textures
	vector<Texture*> textures;
	textures.resize(ReadInt());
	for(int i=0;i<textures.size();i++)
		LoadTextureMap(mat,textures[i]);

	if(Engine::Instance()->IsDedicated())
		return;

	// Load textures+meta-data
	// Currently textures are only used for 3dsmax pipeline
	// plugin material doesn't have enough data to build proper textures
	for(int i=0;i<textures.size();i++){
		Texture* t = textures[i];
		if(t->type == TT_PUTINALPHA)
			continue;

		// See if there's a corresponding TT_PUTINALPHA for this texture
		// FIXME: Put back ASAP
		/*for(int j=0;j<textures.size();j++){
			if(textures[j]->type == TT_PUTINALPHA && t->m_hTexture.name == textures[j]->m_hTexture.name){
				t->alphamapname = textures[j]->filename;
				break;
			}
		}*/

		t->Load(t->filename,t->type,t->uOff,t->vOff,t->uTile,t->vTile,t->vAng);
		if(t->type == TT_DIFFUSE)
			mat->m_Type = MaterialManager::Instance()->GetType(t->filename);
	}

	// Create shader!
	if(!mat->Initialize(shaderName,technique)){
		// Error, such as not found, fall back to default material
		delete mat;
		mat = MaterialManager::Instance()->GetDefaultMaterial();
		mat->AddRef();
		return;
	}
}

//-----------------------------------------------------------------------------
// Static tree node. Must be done after light load
//-----------------------------------------------------------------------------
void SceneLoader::LoadRenderableNode(RenderableNode*& node){
	node = new RenderableNode;
	Read(node->meshID);
	Read(node->box);
	Read(node->subset);

	// Read static light positions and convert to references
	vector<Vector> staticLights;
	ReadVector(staticLights);
	for(int i=0;i<staticLights.size();i++){
		// Get references to all lights on the static light list, for quick lookup
		for(int j=0;j<m_WorldPtr->m_Lights.size();j++){
			if(staticLights[i] == m_WorldPtr->m_Lights[j]->curState.Position){
				node->staticLights.push_back(m_WorldPtr->m_Lights[j]);
				continue;
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Read a node and all its children from a tree
//-----------------------------------------------------------------------------
void SceneLoader::LoadRenderTree(RenderTree*& tree){
	tree = new RenderTree;

	Read(tree->box);
	tree->data.resize(ReadInt());

	for(int i=0;i<tree->data.size();i++){
		LoadRenderableNode(tree->data[i]);
	}

	Read(tree->numChildren);

	for(int i=0;i<tree->numChildren;i++){
		LoadRenderTree(tree->children[i]);
	}
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void SceneLoader::LoadLightFrame(LightState& light){
	DWORD time;
	Read(time);
	Read(light.Diffuse);
	// Add intensity into main color now
	light.Diffuse.r *= light.Diffuse.a;
	light.Diffuse.g *= light.Diffuse.a;
	light.Diffuse.b *= light.Diffuse.a;
	Read(light.Specular);
	Read(light.Position);
	Read(light.Direction);
	Read(light.Range);
	Read(light.Spot_Falloff);
	Read(light.Spot_Size);
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void SceneLoader::LoadLight(Light*& light){
	light = new Light(m_WorldPtr);
	ReadString(light->m_Name);
	Read(light->box);
	Read(light->m_Type);
	ReadString(light->m_sProjectionMap);
	// Use Shadow Map instead, if present
	string shadowMap = ReadString();
	if(shadowMap.size()){
		light->m_bShadowProjector = true;
		light->m_sProjectionMap = shadowMap;
		// FIXME: Hijacked static shadow maps for shadow mapping
		light->m_ShadowMap = new ShadowMap;
		light->m_ShadowMap->Initialize(Engine::Instance()->MainConfig->GetInt("ShadowMapSize"));
	}
	else if(light->m_Type == LIGHT_SPOT && light->m_sProjectionMap.size()){
		light->m_tProjectionMap.Load(light->m_sProjectionMap);
	}

	Read(light->m_IsExcludeList);
	ReadStringVector(light->m_ExcludeList);

	int keyframes = ReadInt();
	for(int j=0;j<keyframes;j++){
		LightState frame;
		LoadLightFrame(frame);
		// If only first frame, set cur state, otherwise add to keyframes
		if(j == 0)
		{
			light->curState = frame;
			light->Location = light->curState.Position;
		}
		else{
			light->keyframes.push_back(frame);
		}
	}

	// All scene lights are static until they move
	light->m_bDynamic = false;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void SceneLoader::LoadMaterials(vector<Material*>& materials, bool& hasPRT){
	StartMiniTimer();
	LogPrintf(LOG_MEDIUM,"\tLoading  materials...");
	LogPushPrefix("\t\t");

	materials.resize(ReadInt());

	for(int i=0;i<materials.size();i++){
		LoadMaterial(materials[i]);

		// Set PRT based on the material name, which gets changed to _PRT if
		// this is a PRT material
		materials[i]->m_bPRTEnabled = materials[i]->m_Name.find("_PRT") != -1 && hasPRT;

		// If this mat doesn't have PRT on, we must disable PRT for the mesh
		// The only reason this would _EVER_ happen is on a default material
		hasPRT = materials[i]->m_bPRTEnabled;
	}

	LogPopPrefix();
	LogPrintf(LOG_MEDIUM,"\tMaterial load took %f seconds",(float)StopMiniTimer());
}


//-----------------------------------------------------------------------------
// Separate from rendering representation. Allows us to use approximation
// meshes, etc
//-----------------------------------------------------------------------------
void SceneLoader::LoadCollisionMesh(CollisionMesh*& cMesh){
	cMesh = new CollisionMesh;
	Read(cMesh->localSpace);

	ReadVector(cMesh->vertices); // Only vectors
	cMesh->faces.resize(ReadInt());

	// Used for caching materials we've looked up
	int lastID = -1;
	Material* lastMat = NULL;

	// Indices
	for(int i=0;i<cMesh->faces.size();i++){
		CollisionMesh::Face& f = cMesh->faces[i];

		f.indices[2] = ReadDWORD();
		f.indices[1] = ReadDWORD();
		f.indices[0] = ReadDWORD();
		int matID	 = ReadInt();

		// Look up material associated with id
		// We cache the last id because there's a 95% coherency
		// and lookup is REALLY slow
		f.mat = NULL;
		if(matID == lastID)
			f.mat = lastMat;
		else{
			if(matID == -1){ // Invalid material, use default
				f.mat = MaterialManager::Instance()->GetDefaultMaterial();
				lastID = -1;
				lastMat = f.mat;
			}
			else{ // Look the material index up in our global array
				vector<Material*>& mats = MaterialManager::Instance()->m_Materials;
				for(int i=0;i<mats.size();i++){
					if(matID == mats[i]->m_ID){
						f.mat	= mats[i];
						lastID	= matID;
						lastMat = f.mat;
						break;
					}
				}
			}
		}

		Vector E0 = cMesh->vertices[f.indices[1]] - cMesh->vertices[f.indices[0]];
		Vector E1 = cMesh->vertices[f.indices[2]] - cMesh->vertices[f.indices[1]];
		Vector E2 = cMesh->vertices[f.indices[0]] - cMesh->vertices[f.indices[2]];

		// Face normal
		f.normal = Cross(E0,E1);
	}

	cMesh->Initialize();
}



//-----------------------------------------------------------------------------
// Bare-bones rendering mesh
//-----------------------------------------------------------------------------
void SceneLoader::LoadMesh(Mesh*& mesh, string name){
	mesh = new Mesh;

	bool hasPRT = false;
	int  vertexSize = sizeof(Vertex);
	if(!m_bOldVersion){
		hasPRT = ReadBool() && Engine::Instance()->MainConfig->GetBool("PRTEnabled");
	}
	Read(mesh->m_LocalBox);
	ReadVector(mesh->m_AttribTable);
	
	// Read vertices. Size depends on vertex format, so use generic BYTE* array
	BYTE* vertices;
	int numVerts = ReadInt(); 
	vertices = new BYTE[numVerts*vertexSize]; 
	fread(&vertices[0],vertexSize,numVerts,m_File);
	m_FileVerts	+= numVerts;

	vector<WORD>   indices;
	ReadVector(indices);
	m_FileInds	+= indices.size();
	
	mesh->m_Name = name;

	/*	PackedVertex* pverts = new PackedVertex[numVerts];
	for(int x=0;x<numVerts;x++){
	pverts[x].position = verts[x].position;
	pverts[x].tu = verts[x].tu;
	pverts[x].tv = verts[x].tv;
	#define PACK(v) D3DCOLOR_COLORVALUE((v.x+1)/2,(v.y+1)/2,(v.z+1)/2,1)
	pverts[x].normal = PACK(verts[x].normal);
	pverts[x].tan = PACK(verts[x].tan);
	}*/

	// Skinning data (NOT ANIMATION)
	mesh->bones.resize(ReadInt());
	for(int i=0;i<mesh->bones.size();i++){
		ReadString(mesh->bones[i].name);
		Read(mesh->bones[i].offset);
		ReadVector(mesh->bones[i].indices);
		ReadVector(mesh->bones[i].weights);
	}

	// Load materials *BEFORE* Mesh, so we can check up on them before deciding whether
	// to enable PRT
	LoadMaterials(mesh->m_Materials, hasPRT);

	string prtFile;
	// Get PRT filename via MapName_Data\MeshName.prt
	if(m_WorldPtr && hasPRT){
		string world = m_WorldPtr->m_FileName.substr(0,m_WorldPtr->m_FileName.find_last_of("."));
		if(world.find_last_of("\\") != -1)
			world = world.substr(world.find_last_of("\\"));
		prtFile = "..\\Maps\\" + world+"_Data";
		// FIXME: Should be GROUP name
		prtFile += "\\" + name + ".prt";
	}

	if(!Engine::Instance()->IsDedicated())
		mesh->Create(vertices,indices,vertexSize,numVerts,prtFile,mesh->m_Materials);

	delete[] vertices;

	if(!Engine::Instance()->IsDedicated() && mesh->bones.size())
		mesh->GenerateSkinInfo();
}


//-----------------------------------------------------------------------------
// Model Frame
//-----------------------------------------------------------------------------
void SceneLoader::LoadModelFrame(ModelFrame*& frame){
	frame = new ModelFrame;

	ReadString(frame->Name);

	LogPrintf(LOG_MEDIUM,"Loading Frame: %s",frame->Name.c_str());

	Read(frame->TransformationMatrix);

	if(ReadBool())
		LoadMesh(frame->mesh,frame->Name);
	if(ReadBool())
		LoadCollisionMesh(frame->collisionMesh);
	if(ReadBool())
		; // TODO: Read light
	if(ReadBool())
		LoadModelFrame(frame->pFrameFirstChild);
	if(ReadBool())
		LoadModelFrame(frame->pFrameSibling);
}

//-----------------------------------------------------------------------------
// World Prefab Loading + Insertion into tree
//-----------------------------------------------------------------------------
void SceneLoader::LoadPrefab(NodeData* data, Matrix tm){
	if(!m_WorldPtr)
		Error("Prefab script found in model!");

	// Extract prefab info from script
	// TODO:  use better statement structure to read the parameters
	string filename;
	bool ghostobject = false;
	bool ishidden	 = false;
	for(int i=0;i<data->parameters.size();i++)
	{
		if(data->parameters[i] == "filename")
		{
			filename = data->paramvalues[i];
		}
		else if(data->parameters[i] == "noclip")
		{
			if(data->paramvalues[i] == "false")
				ghostobject = false;
			else 
				ghostobject = true;
		}
		else if(data->parameters[i] == "invisible")
		{
			if(data->paramvalues[i] == "false")
				ishidden = false;
			else 
				ishidden = true;
		}
	}

	// Strip any "quotes" that would have come from script
	if(filename[0] == '\"' || filename[0] == '\'')
		filename = filename.substr(1);
	if(filename[filename.length()-1] == '\"' || filename[filename.length()-1] == '\'')
		filename = filename.substr(0,filename.length()-1);

	// Resolve it (script actually holds .max filename!)
	filename = filename.substr(0,filename.find_last_of(".")) + ".mdc";
	if(!FindMedia(filename,"Models")){
		Warning("Couldn't find prefab '%s'",filename.c_str());
		return;
	}

	// Load it
	Model* prefab = new Model;
	prefab->Load(filename.c_str());
	prefab->InitAfterLoad();

	// Kill the root transform in the prefab so it stays positioned like it is in the editor
	// after xrefing
	prefab->RemoveSceneOffset();

	Prefab* act = new Prefab(m_WorldPtr);
	act->MyModel = prefab;
	act->Rotation = tm.GetRotationMatrix();
	act->Location = tm[3];
	act->MyModel->m_AllowIncludeExclude = true; // Prefabs use include/exclude lists
	act->MyModel->SetTransform(act->Rotation,act->Location);
	act->IsHidden = ishidden;
	act->GhostObject = ghostobject;
}

// Used below in param handling
bool AreTheyAllDigits(const string& str)
{
	for(int i=0;i<str.length();i++){
		if(!isdigit(str[i]) && str[i] != '.' && str[i] != '-' && str[i]!='+')
			return false;
	}
	return true;
} 


//-----------------------------------------------------------------------------
// FIXME: Leak if Model isn't used by script. Need ref counting really.
//-----------------------------------------------------------------------------
void SceneLoader::LoadEntity(){
	Model* model = NULL;

	if(ReadBool()){
		// HACK: Should be xref model really
		model = new Model;
		LoadModelFrame(model->m_pFrameRoot);
		model->m_pFrameRoot->UpdateMatrices(model->m_RootTransform);
		model->InitAfterLoad();	
	}

	NodeData data;
	// We read the static TM for now
	// TODO: Support entity linking to frames for dynamic movement
	Matrix tm;
	Read(tm);
	ReadString(data.filename);
	ReadString(data.classname);
	ReadString(data.parentclass);
	ReadStringVector(data.parameters);
	ReadStringVector(data.paramvalues);

	if(data.classname == ""){
		Warning("Entity with no class, entity will be ignored!");
		return;
	}

	// Prefab loading -- special cased for speed
	if(AsLower(data.classname) == "prefab")
	{
		if(model != NULL)
			Error("A prefab script has a model attached, whereas it should have been xrefed. This is an internal bug which must be fixed");
		
		LoadPrefab(&data,tm); // Special case script
	}
	// StaticMesh loading -- special cased for speed
	else if(AsLower(data.classname) == "staticmesh")
	{
		if(model == NULL)
			Error("StaticMesh has a NULL model. Did you apply StaticMesh to a Light? You silly twat.");

		bool ghostobject = false;
		bool ishidden	 = false;
		for(int i=0;i<data.parameters.size();i++)
		{
			if(data.parameters[i] == "noclip")
				ghostobject = (data.paramvalues[i] == "true" || data.paramvalues[i] == "1"); //JERCHANGED
			else if(data.parameters[i] == "invisible")
				ishidden = (data.paramvalues[i] == "true" || data.paramvalues[i] == "1");
		}

		model->RemoveSceneOffset();
		Prefab* act = new Prefab(m_WorldPtr);
		act->MyModel = model;
		act->Rotation = tm.GetRotationMatrix();
		act->Location = tm[3];
		act->MyModel->SetTransform(act->Rotation,act->Location);
		act->GhostObject = ghostobject;
		act->IsHidden = ishidden;
	}
	else
	{
		Script* script = new Script;

		// Don't allow loading from non-game locations
		data.filename = StripPath(data.filename);

		//--------------------------------
		// Load Script
		//--------------------------------
		LogPrintf(LOG_MEDIUM,"Loading script.. Class: '%s' Script: '%s'(.py)",data.classname.c_str(),data.filename.c_str());

		if(!script->Create(data.classname,data.classname))
			Warning("Script Creation failed for %s",data.filename.c_str());

		// Parse properties
		for(int d=0;d<data.parameters.size();d++){
			string prop = data.parameters[d];
			string propValue = data.paramvalues[d];;

			// String
			if(propValue[0] == '\"'){
				// Strip quotes
				string sPropVal = propValue;
				if(sPropVal[0] == '\"')
					sPropVal = sPropVal.substr(1,sPropVal.length());
				if(sPropVal[sPropVal.size()-1] == '\"')
					sPropVal = sPropVal.substr(0,sPropVal.length()-1);
				script->Set((char*)prop.c_str(),(char*)sPropVal.c_str());
			}
			// Number
			else if(AreTheyAllDigits(propValue)){
				string str = propValue;
				// Float/Double
				if(str.find(".")!=-1){
					double d = atof(propValue.c_str());
					script->Set((char*)prop.c_str(),d);
				}
				// Int/Long
				else{
					long l = atol(propValue.c_str());
					script->Set((char*)prop.c_str(),l);
				}
			}
			// Object (NOT string)
			else{
				script->InitProp((char*)prop.c_str(),(char*)propValue.c_str());
			}
		}

		// Call the constructor, with the Model this script
		// was assigned to (if any)
		// and its orientation in the world
		script->Initialize(model,m_WorldPtr,tm);	

		// TODO: Support model loading of scripts
		// Only add scripts to map if they define the Tick() function
		if(script->HasFunction("Tick"))
			m_WorldPtr->m_Scripts.push_back(script);
		else
			delete script; // Has no further use
	}
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
bool SceneLoader::LoadModel(string name, StaticModel* model){
	m_ModelPtr = model;
	if(!OpenFile(name.c_str()))
		return false;

	LoadModelFrame(model->m_pFrameRoot);

	CloseFile();
	return true;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
bool SceneLoader::LoadWorld(string name, World* world, bool IsSky){
	m_WorldPtr = world;
	if(!OpenFile(name.c_str()))
		return false;

	BBox worldBox;
	Read(worldBox);
	// Create spatial partition covering entire world area.
	// NOTE: What happens if entity goes outside of bounds??
	if(!IsSky){
		Vector buffer(100,100,100); // To stop weird static out of bounds errors, or statically empty levels barfing
		SpatialPartition()->Init(worldBox.min - buffer,worldBox.max + buffer);
	}

	if(IsSky)
		ReadString();
	else
		world->LoadSkyWorld(ReadString());

	COLOR fogColor = ReadDWORD(); // NOTE: Don't collapse these into one function call, they don't get called in order
	DWORD fogStart = ReadDWORD();
	world->SetFog(fogColor,fogStart);
	world->SetClipPlane(ReadInt());
	world->m_MiniMap		= ReadString();
	world->m_MiniMapScale   = ReadFloat();

	string cubemap = ReadString();
	if(cubemap.length())
		world->LoadSHProbe(cubemap);

	int numLights = ReadInt();
	Light* light;  // Lights add themselves to the world, so no need to ref track
	for(int i=0;i<numLights;i++)
		LoadLight(light);

	// The following load the static scene tree
	world->m_BatchMeshes.resize(ReadInt());
	for(int i=0;i<world->m_BatchMeshes.size();i++)
		LoadMesh(world->m_BatchMeshes[i],"_WorldBatch"+ToStr(i));
	LoadRenderTree(world->m_Root);
	LoadCollisionMesh(world->m_CollisionMesh);

	int numEntities = ReadInt();
	for(int i=0;i<numEntities;i++)
		LoadEntity();

	CloseFile();
	return true;
}

//-----------------------------------------------------------------------------
// Opens the file
//-----------------------------------------------------------------------------
bool SceneLoader::OpenFile(const char* name){
	m_FileName = name;
	// If file didn't open, try another path
	if(  (m_ModelPtr && FindMedia(m_FileName,"Models")) || (m_WorldPtr && FindMedia(m_FileName,"Maps"))  ){
		if(!(m_File = fopen(m_FileName.c_str(),"rb"))){
			Error("Found file %s, but couldn't open it.",name);
			return false;
		}
	}
	else{
		Warning("Couldn't find file '%s'", name);
		return false;
	}

	LogPrintf("Loading '%s'",name);

	Read(m_Version);

	m_bOldVersion = false;
	if(m_Version == 5010){
		m_bOldVersion = 1; // Hack, so we can load older version
	}
	else if(m_Version != EXPORTER_VERSION){
		if(m_WorldPtr)
			Error("The map %s was exported with a different exporter version (version %d). The version of the game you are currently using takes version %d files.\nAttempting to skip loading..",name,m_Version,EXPORTER_VERSION);
		else
			Warning("The file %s was exported with a different exporter version (version %d). The version of the game you are currently using takes version %d files.\nAttempting to skip loading..",name,m_Version,EXPORTER_VERSION);

		return false;
	}

	if(m_ModelPtr)
		m_ModelPtr->m_FileName = name;
	else
		m_WorldPtr->m_FileName = name;

	m_FileVerts = 0;
	m_FileInds  = 0;

	StartMiniTimer();

	return true;
}

//-----------------------------------------------------------------------------
// Closes file & writes out summary info
//-----------------------------------------------------------------------------
void SceneLoader::CloseFile(){
	fclose(m_File);
	LogPrintf("\tGeometry (Models + Scripts + Tree) load took %f seconds",(float)StopMiniTimer());

	if(m_FileInds <= 0 || m_FileVerts <= 0)
		Error("%s load failed (reason - file appears empty/corrupt)",m_FileName.c_str());

	LogPrintf("\t%s loaded successfully!",m_FileName.c_str());
	LogPrintf("\tIndices: %d. Vertices: %d",m_FileInds,m_FileVerts);

	m_WorldPtr = NULL;
	m_ModelPtr = NULL;
	m_FileName = "(None)";
}

SceneLoader::SceneLoader(){
	m_WorldPtr = NULL;
	m_ModelPtr = NULL;
}
