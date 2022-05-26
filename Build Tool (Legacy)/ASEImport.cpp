//----------------------------------------------------------------------------------
// ASEImport.cpp - Reads modified ASE files (HSE) and converts the data
// to C++ structures for use in Compiler.cpp
//
// TODO: Add support for parents and group parents
//----------------------------------------------------------------------------------

#include "stdafx.h"
#include "ASEImport.h"



vector<Matrix> helperKeyframes;
vector<float> helperFrameTimes;



string oldLine; // for debugging
string curLine;

string ASEImport::GetLine(){
	static char str[2048];
	oldLine = str;
	file.getline(str,2048);
	curLine = str;
	return str;
}

// Gets the name out of a line such as:
//      *MATERIAL_LIST {
string ASEImport::GetName(const string& line){
	string name = line.substr(line.find("*"));
	return name.substr(0,name.find(" "));
}

// returns nth word
string ASEImport::GetWord(const string& s, int index){
	

	string temp = s;

	// Replace tabs with spaces so we can see tabbed words as being separate
	findandreplace(temp,"\t"," ");

	// Skip first word (the name)
	temp = temp.substr(temp.find("*"));
	temp = temp.substr(temp.find(" ")+1);
	trimLeft(temp);

	string word;

	// If this has "quotes", then take the whole thing, rather than the first word
	if(temp[0] == '\"'){
		// Lines with quotes can't have more than one element on (the quoted words)
		if(index != 0)
			cout << "Out-of-bounds (" << index << ") word error for line: " << s << endl;

		word = temp.substr(1,temp.find_last_of("\"")-1);
		return word;
	}

	// Add one for internal logic, so index 0 is the first word
	index++;


	
	for(int i=0;i<index;i++){
		if(temp.find(" ")!=-1){
			word = temp.substr(0,temp.find(" "));
			trimLeft(word);
			temp = temp.substr(temp.find(" ")+1,temp.length());
			trimLeft(temp);
		}
		else 
			word = temp.substr(0,temp.length());
	}

	return word;
}

// *BLAH 0 1 2
// Will return one of the numbers as a float
int ASEImport::GetInt(const string& line, int index){
	return (int)GetFloat(line,index);
}

float ASEImport::GetFloat(const string& line,int index){
	char *err1 = 0;
	
	// GetWord gets the nTh word from the string, words are ascii separated by spaces
	string word = GetWord(line,index);
	double flt = strtod(word.c_str(),&err1);

	if(err1 == word.c_str())
		Error("Error trying to convert '%s' to a float",GetWord(line,index).c_str());

	return flt;
}

Vector ASEImport::GetVector(const string& line, int offset){
	Vector v;
	v.x = GetFloat(line,offset+0);
	v.y = GetFloat(line,offset+1);
	v.z = GetFloat(line,offset+2);

	return v;
}

void ASEImport::DummyRead(const string& line){
	if(line.find("{") == -1)
		return; // This is a single line

	// Keep digging until we find the closing brace, taking care to count nested braces
	int depth = 1;
	while(depth){
		string line = GetLine();
		if(line.find("}")!= -1)
			depth--;
		else if(line.find("{") != -1)
			depth++;
	}
}


void ASEImport::ReadSCENE(){
	while(true){
		string line = GetLine();
		if(line.find("}") != -1) break;

		string name = GetName(line);

		if(name == ID_FILENAME);
		else if(name == ID_FIRSTFRAME) DummyRead(line);
		else if(name == ID_LASTFRAME) DummyRead(line);
		else if(name == ID_FRAMESPEED) DummyRead(line);
		else if(name == ID_TICKSPERFRAME) DummyRead(line);
		else if(name == ID_ENVMAP) DummyRead(line);
		else if(name == ID_STATICBGCOLOR) DummyRead(line);
		else if(name == ID_ANIMBGCOLOR) DummyRead(line);
		else if(name == ID_STATICAMBIENT) scene.ambient = GetVector(line,0);
		else if(name == ID_ANIMAMBIENT) DummyRead(line);
		else{ Error("Unexpected token for ReadSCENE: %s",line.c_str()); DummyRead(line); }
	}

}

Map ASEImport::ReadMAP(){
	Map map;
	memset(&map,0,sizeof(Map));

	while(true){
		string line = GetLine();
		if(line.find("}") != -1) break;

		string name = GetName(line);

		if(name == ID_TEXNAME)				map.name = GetWord(line,0);
		else if(name == ID_TEXCLASS)		map.mixmap = line.find("Mix")!=-1; // is mix map?
		else if(name == ID_TEXSUBNO)		DummyRead(line);
		else if(name == ID_TEXAMOUNT)		map.amount = GetFloat(line,0);
		else if(name == ID_BITMAP)			map.filename = GetWord(line,0);
		else if(name == ID_MAPTYPE)			DummyRead(line);
		else if(name == ID_U_OFFSET)		map.UOff = GetFloat(line,0);
		else if(name == ID_V_OFFSET)		map.VOff = GetFloat(line,0);
		else if(name == ID_U_TILING)		map.UTile = GetFloat(line,0);
		else if(name == ID_V_TILING)		map.VTile = GetFloat(line,0);
		else if(name == ID_ANGLE)			DummyRead(line);
		else if(name == ID_BLUR)			DummyRead(line);
		else if(name == ID_BLUR_OFFSET)		DummyRead(line);
		else if(name == ID_NOISE_AMT)		DummyRead(line);
		else if(name == ID_NOISE_SIZE)		DummyRead(line);
		else if(name == ID_NOISE_LEVEL)		DummyRead(line);
		else if(name == ID_NOISE_PHASE)		DummyRead(line);
		else if(name == ID_MAP_FILTERCOLOR) DummyRead(line);
		else if(name == ID_BMP_FILTER)		DummyRead(line);
		else if(name == ID_MAP_GENERIC)		map.mixmaps.push_back(ReadMAP()); // has sub-mixmaps
		else{ Error("Unexpected token for ReadMap: %s",line.c_str()); DummyRead(line); }
	}
	return map;
}

Material ASEImport::ReadMATERIAL(){
	Material mat;
	memset(&mat,0,sizeof(Material));

	while(true){
		string line = GetLine();
		if(line.find("}") != -1) break;

		string name = GetName(line);

		if(name == ID_MATNAME)				mat.name = GetWord(line,0);
		else if(name == ID_MATCLASS)		DummyRead(line);
		else if(name == ID_NUMSUBMTLS)		DummyRead(line);
		else if(name == ID_SUBMATERIAL)		mat.subMaterials.push_back(ReadMATERIAL());
		else if(name == ID_AMBIENT)			DummyRead(line);
		else if(name == ID_DIFFUSE)			mat.diffuseColor = GetVector(line,0);
		else if(name == ID_SPECULAR)		mat.specularColor = GetVector(line,0);
		else if(name == ID_SPECULAR_LEVEL)	mat.specularLevel = GetFloat(line,0);
		else if(name == ID_SHINE)			DummyRead(line);
		else if(name == ID_SHINE_STRENGTH)	DummyRead(line);
		else if(name == ID_TRANSPARENCY)	mat.xparency = GetFloat(line,0);
		else if(name == ID_WIRESIZE)		DummyRead(line);
		else if(name == ID_SHADING)			DummyRead(line);
		else if(name == ID_XP_FALLOFF)		DummyRead(line);
		else if(name == ID_SELFILLUM)		mat.selfIllum = GetVector(line,0);
		else if(name == ID_FALLOFF)			DummyRead(line);
		else if(name == ID_XP_TYPE)			mat.blendType = GetWord(line,0);
		else if(name == ID_TWOSIDED)		mat.twosided = true;
		else if(name == ID_MAP_OPACITY)		{ DummyRead(line);Warning("Found opacity map. Ignored. Please use the alpha channel in future"); }
		else if(name == ID_MAP_DIFFUSE)		mat.diffuse = ReadMAP();
		else if(name == ID_MAP_SELFILLUM)	mat.selfIllumMap = ReadMAP();
		else if(name == ID_MAP_BUMP)		mat.bump = ReadMAP();
		else if(name == ID_MAP_REFLECT)		mat.reflect = ReadMAP();
		else if(name == ID_MAP_SHINE)		mat.specular = ReadMAP();
		else if(name == ID_MAP_SPECULAR)	{cout << "Found MAP_SPECULAR. Expected MAP_SHINE" << endl;DummyRead(line);}
		else if(name == ID_MAP_AMBIENT)		{Warning("Strauss material found (%s). Only Blinn is supported\n",mat.name.c_str()); DummyRead(line); }
		else{ 
			Error("Unexpected token for ReadMATERIAL: %s",line.c_str()); DummyRead(line); 
		}
	}

	return mat;
}

vector<Material> ASEImport::ReadMATERIAL_LIST(string line){
	vector<Material> list;
	GetLine(); // MATERIAL_COUNT

	// Add all the materials in this list to our array
	while(GetLine().find("}") == -1){
		list.push_back(ReadMATERIAL());
	}
	return list;
}


void ASEImport::ReadMESH(Mesh& mesh){
	memset(&mesh,0,sizeof(Mesh));

	mesh.matID = -1;

	// Temps
	Vector2* TVerts = 0;
	int numTVerts = 0;

	Vector* CVerts = 0;
	int numCVerts = 0;


	while(true){
		string line = GetLine();
		if(line.find("}") != -1) break;

		string name = GetName(line);

		if(name == ID_TIMEVALUE)			DummyRead(line);
		else if(name == ID_MESH_NUMVERTEX){
			mesh.originalVerts.resize(GetInt(line,0));
			TotalVerticesRead += mesh.originalVerts.size();
		}
		else if(name == ID_MESH_NUMFACES ){	
			TotalFacesRead += GetInt(line,0);
			mesh.faces.resize(GetInt(line,0));
			mesh.originalIndices.resize(GetInt(line,0)*3);
		}
		else if(name == ID_MESH_VERTEX_LIST){
			for(int i=0;i<mesh.originalVerts.size();i++){
				line = GetLine();
				mesh.originalVerts[i] = GetVector(line,1) * UNITSCALE;
			}

			GetLine(); // }
		}
		else if(name == ID_MESH_FACE_LIST){
			for(int i=0;i<mesh.faces.size();i++){
				line = GetLine();
				// Clear tangents until they are used
				// That way we can compare vertices that don't have tangents
				mesh.faces[i].v[0].tan.Set(0,0,0);
				mesh.faces[i].v[1].tan.Set(0,0,0);
				mesh.faces[i].v[2].tan.Set(0,0,0);

				mesh.faces[i].v[0].p = mesh.originalVerts[GetInt(line,2)];
				mesh.faces[i].v[1].p = mesh.originalVerts[GetInt(line,4)];
				mesh.faces[i].v[2].p = mesh.originalVerts[GetInt(line,6)];
				mesh.faces[i].submatID = GetFloat(line,16);
				// Set default material
				// Will be changed later if this object has a material
				mesh.faces[i].matID = mesh.matID;
				mesh.faces[i].geomID = scene.geometry.size();

				mesh.originalIndices[i*3 + 0] = GetInt(line,2);
				mesh.originalIndices[i*3 + 1] = GetInt(line,4);
				mesh.originalIndices[i*3 + 2] = GetInt(line,6);
			}

			GetLine(); // }
		}
		else if(name == ID_MESH_NUMTVERTEX) {
			numTVerts = GetInt(line,0);
			TVerts = new Vector2[numTVerts];
		}
		else if(name == ID_MESH_TVERTLIST) {
			for(int i=0;i<numTVerts;i++){
				line = GetLine();
				Vector v = GetVector(line,1);
				TVerts[i] = Vector2(v.x,v.y);
			}

			GetLine(); // }
		}
		else if(name == ID_MESH_TFACELIST) {
			for(int i=0;i<mesh.faces.size();i++){
				line = GetLine();
				mesh.faces[i].v[0].t = TVerts[GetInt(line,1)];
				mesh.faces[i].v[1].t = TVerts[GetInt(line,2)];
				mesh.faces[i].v[2].t = TVerts[GetInt(line,3)];
			}

			GetLine(); // }
		}
		else if(name == ID_MESH_NUMTVFACES){
			if(GetInt(line,0) != mesh.faces.size())
				Error("Number of texture faces != number of faces. WTF?\n");
		}
		else if(name == ID_MESH_NORMALS){
			for(int i=0;i<mesh.faces.size();i++){

				line = GetLine(); // Face normal

				mesh.faces[i].v[0].n = GetVector(GetLine(),1);
				mesh.faces[i].v[1].n = GetVector(GetLine(),1);
				mesh.faces[i].v[2].n = GetVector(GetLine(),1);
			}

			GetLine(); // }
		}
		// ======== COLOR DATA =========
		else if(name == ID_MESH_NUMCVERTEX) {
			numCVerts = GetInt(line,0);
			CVerts = new Vector[numCVerts];
		}
		else if(name == ID_MESH_NUMCVFACES){
			int numCFaces = GetInt(line,0);
			assert(numCFaces == mesh.faces.size());
		}
		else if(name == ID_MESH_CVERTLIST) {
			for(int i=0;i<numCVerts;i++){
				line = GetLine();
				CVerts[i] = GetVector(line,1);
			}

			GetLine(); // }
		}
		else if(name == ID_MESH_CFACELIST) {
			for(int i=0;i<mesh.faces.size();i++){
				line = GetLine();
				for(int pp=0;pp<3;pp++){
					Vector c = CVerts[GetInt(line,pp+1)];
#ifdef VERTEX_COLORS
					mesh.faces[i].v[pp].c = COLOR_ARGB(255,c.x*255,c.y*255,c.z*255);
#endif
				}
			}

			GetLine(); // }
		}
		else{ 
			Error("Unexpected token for ReadMESH: %s",line.c_str()); DummyRead(line); 
		}
	}

	if(TVerts)
		delete[] TVerts;
	if(CVerts)
		delete[] CVerts;
}

Matrix ASEImport::ReadMatrix(string& startLine, int offset){
	Matrix tm;
	string line;
	if(startLine != "")
		line = startLine;
	else
		line = GetLine(); // TM0

	tm[0] = GetVector(line,offset);

	line = GetLine(); // TM1
	tm[1] = GetVector(line,offset);

	line = GetLine(); // TM2
	tm[2] = GetVector(line,offset);

	line = GetLine(); // TM3
	tm[3] = GetVector(line,offset) * UNITSCALE;
	return tm;
}

Matrix ASEImport::ReadTM(){
	Matrix tm;

	GetLine(); // name
	GetLine(); // inherit_pos
	GetLine(); // inherit_rot
	GetLine(); // inherit_scl

	tm = ReadMatrix();

	GetLine(); // TM_POS
	GetLine(); // ROT
	GetLine(); // ROT
	GetLine(); // SCL
	GetLine(); // SCL
	GetLine(); // SCL
	GetLine(); // }

	return tm;
}


void ASEImport::ReadTMAnimation(vector<Matrix>& keyframes, vector<float>& frameTimes){
	string name = GetName(GetLine()); // node_name
	if(GetLine().find(ID_TM_TRACK) == -1){
		Warning("Non-standard (non-TM) animation track found on: %s. Ignoring.",name.c_str());

		int depth = 1; // tm_animation
		if(curLine.find("{")!=-1)
			depth++;

		// Work our way out of the nest
		while(depth){
			string line = GetLine();
			if(line.find("}") != -1)
				depth--;
			if(line.find("{") != -1)
				depth++;
		}
		return;
	}

	while(true){
		string line = GetLine();
		if(line.find("}") != -1) break;

		// Get the time for this frame, the last ever time this is set
		// will then contain our final time duration
		frameTimes.push_back(GetFloat(line,0));

		keyframes.push_back(ReadMatrix(line,1));
	}

	GetLine(); // }
}

GeomObject ASEImport::ReadGEOMOBJECT(){
	GeomObject geom;

	// Use any parent helper animation we have
	if(helperKeyframes.size()){
		geom.keyframes = helperKeyframes;
		geom.keyframeTime = helperFrameTimes[helperFrameTimes.size()-1];
	}


	while(true){
		string line = GetLine();
		if(line.find("}") != -1) break;

		string name = GetName(line);

		if(name == ID_NODE_TM){
			geom.tm = ReadTM();

			if(helperKeyframes.size()){
				for(int i=0;i<geom.keyframes.size();i++){
					// TODO: Pivot point, but that way it needs a matrix stack to transform in/out
					//Matrix m =  geom.tm * geom.keyframes[i];
					//geom.keyframes[i] = geom.tm * geom.keyframes[i];
				}
			}

		}
		else if(name == ID_NODE_PROPBUFFER) geom.propbuffer = GetWord(line,0);
		else if(name == ID_NODE_NAME)		geom.name = GetWord(line,0);
		else if(name == ID_NODE_PARENT)		DummyRead(line);
		else if(name == ID_PROP_MOTIONBLUR)	DummyRead(line);
		else if(name == ID_PROP_CASTSHADOW)	DummyRead(line);
		else if(name == ID_PROP_RECVSHADOW)	DummyRead(line);
		else if(name == ID_MESH_ANIMATION)	DummyRead(line);
		else if(name == ID_TM_ANIMATION){
			vector<float> times;
			ReadTMAnimation(geom.keyframes,times);
			if(times.size())
				geom.keyframeTime = times[times.size()-1];
		}
		else if(name == ID_MATERIAL_REF){
			geom.mesh.matID = GetInt(line,0);

			if(abs(geom.mesh.matID) > 600)
				Error("geom object '%s' has a matID of %d",geom.name.c_str(),geom.mesh.matID);

			// Put this in ALL faces for easy tracking
			for(int i=0;i<geom.mesh.faces.size();i++)
				geom.mesh.faces[i].matID = geom.mesh.matID;

		}
		else if(name == ID_MESH )			ReadMESH(geom.mesh);
		else if(name == ID_WIRECOLOR)		DummyRead(line);
		else{ Error("Unexpected token for ReadGEOMOBJECT: %s",line.c_str()); DummyRead(line); }
	}

	// Sanity check. Make sure nobody sticks Entity in the name improperly
	if((geom.name.find("Entity") != -1 && geom.name.find("Entity") != 0) || geom.name.find("entity") != -1)
		Warning("Geometry '%s' has ambiguous 'Entity' name. Entity must be at the beginning in capitals for the object to be treated as a dummy mesh",geom.name.c_str());


	if(geom.mesh.matID == -1 && geom.name.find("Entity") != 0){
		Warning("%s is untextured and may not have UV coordinates.",geom.name.c_str());
	}

	return geom;
}


LightKeys ASEImport::ReadLIGHT(){
	LightKeys light;
	light.main.attenEnd = -1;
	string lightName;

	vector<float> TMKeyframeTimes;
	vector<Matrix> TMKeyframes;


	// Use any parent helper animation we have
	if(helperKeyframes.size()){
		TMKeyframes = helperKeyframes;
		TMKeyframeTimes = helperFrameTimes;
	}

	while(true){
		string line = GetLine();
		if(line.find("}") != -1) break;

		string name = GetName(line);

		if(name == "")
			cout << "ERROR! BLANK LINE. last line was "<<oldLine<<endl;

		if(name == ID_NODE_TM){
			light.main.tm = ReadTM();
		}
		else if(name == ID_NODE_NAME)
			lightName = GetWord(line,0);
		else if(name == ID_LIGHT_SETTINGS){
			while(true){
				string line = GetLine();
				if(line.find("}") != -1) break;

				string name = GetName(line);

				if(name == ID_LIGHT_TYPE)		light.main.type = GetWord(line,0);
				else if(name == ID_LIGHT_COLOR)		light.main.color = GetVector(line,0);
				else if(name == ID_LIGHT_INTENS)	light.main.intensity = GetFloat(line,0);
				else if(name == ID_LIGHT_ATTNEND)	light.main.attenEnd = GetFloat(line,0) * UNITSCALE;
				else if(name == ID_LIGHT_ATTNSTART)	light.main.attenStart = GetFloat(line,0) * UNITSCALE;
				else{ DummyRead(line);}
			}
		}
		else if(name == ID_TM_ANIMATION){
			ReadTMAnimation(TMKeyframes,TMKeyframeTimes);
		}
		else if(name == ID_LIGHT_ANIMATION){

			// Read a set of LIGHT_SETTINGS
			while(true){
				string aLine = GetLine(); // "LIGHT_SETTINGS {" or "}"
				if(aLine.find("}") != -1) break;
	
				// Read a single LIGHT_SETTINGS
				Light keyLight = light.main;
				while(true){
					string bLine = GetLine();
					if(bLine.find("}") != -1) break; // }

					string name = GetName(bLine);

					if(name == ID_TIMEVALUE){
						// Get the time for this frame
						keyLight.keyframeTime = GetFloat(bLine,0);
					}
					if(name == ID_LIGHT_TYPE)			keyLight.type = GetWord(bLine,0);
					else if(name == ID_LIGHT_COLOR)		keyLight.color = GetVector(bLine,0);
					else if(name == ID_LIGHT_INTENS)	keyLight.intensity = GetFloat(bLine,0);
					else if(name == ID_LIGHT_ATTNEND)	keyLight.attenEnd = GetFloat(bLine,0) * UNITSCALE;
					else if(name == ID_LIGHT_ATTNSTART)	keyLight.attenStart = GetFloat(bLine,0) * UNITSCALE;
					else{ DummyRead(bLine);}
				}
				light.keyframes.push_back(keyLight);
			}
		}
		else if(name == ID_LIGHT_EXCLUSIONLIST){
			while(true){
				string line = GetLine();
				if(line.find("}") != -1) break;

				string name = GetName(line);

				if(name == ID_LIGHT_EXCLINCLUDE )	light.main.IsExcludeList = (GetInt(line,0) == 0);
				else if(name == ID_LIGHT_EXCLUDED){
					light.main.excludeIncludeList.push_back(GetWord(line,0));
				}
				else{ DummyRead(line);}
			}

		}
	}


	// Merge keyframe data now
	// Let's assume that keys could be any combination
	// So tm keys are either inserted in light keys if the times match, or new ones are created
	for(int j=0;j<TMKeyframes.size();j++){
		bool match = false;
		float dist = 9999999;
		float nearest = 99999;
		int nearestIndex = -1;

		for(int i=0;i<light.keyframes.size();i++){
			// Monitor nearest frame, so we have a starting point
			// if we need to build our own
			if(fabsf(light.keyframes[i].keyframeTime-nearest) < dist){
				dist = fabsf(light.keyframes[i].keyframeTime-nearest);
				nearestIndex = i;
			}

			// If found matching keyframe, merge
			if(sameD(light.keyframes[i].keyframeTime,TMKeyframeTimes[j])){
				light.keyframes[i].tm = TMKeyframes[j];
				TMKeyframes.erase(TMKeyframes.begin()+j);
				TMKeyframeTimes.erase(TMKeyframeTimes.begin()+j);
				j--;
				match = true;
				break;
			}
		}

		// If no match, add new keyframe
		if(!match){
			Light newFrame = light.main;
			if(nearestIndex != -1)
				newFrame = light.keyframes[nearestIndex];

			newFrame.tm = TMKeyframes[j];
			newFrame.keyframeTime = TMKeyframeTimes[j];

			light.keyframes.push_back(newFrame);
		}
	}

	if(light.main.attenEnd == -1){
		light.main.attenEnd = 400;
		Warning("'Use Far Attenuation' was not checked for the light '%s'. This light will not work correctly.",lightName.c_str());
	}

	return light;
}


void ASEImport::ReadHELPER(){
	while(true){
		string line = GetLine();
		if(line.find("}") != -1) break; // }

		string name = GetName(line);

		if(name == ID_TM_ANIMATION){
			ReadTMAnimation(helperKeyframes,helperFrameTimes);
		}
		else DummyRead(line);
	}
	//GetLine();
}


void ASEImport::ReadSKINNINGDATA(){
	string line = GetLine();
	if(GetInt(line,0) != 1)
		Error("File contains %d skinned meshes. Only 1 supported",GetInt(line,0));

	GetLine(); // MESH_DATA

	// Get name, and look up geometry for this skin data
	string name = GetWord(GetLine(),0);
	SkinData* skin = NULL;
	for(int i=0;i<scene.geometry.size();i++){
		if(name == scene.geometry[i].name){
			scene.geometry[i].hasSkinData = true;
			skin = &scene.geometry[i].skinData;
			break;
		}
	}
	assert(skin != NULL);

	skin->maxWeightsPerVertex = GetInt(GetLine(),0); // MAX_VERTEX_WEIGHTS 
	skin->maxWeightsPerFace = GetInt(GetLine(),0); // MAX_FACE_WEIGHTS
	skin->bones.resize(GetInt(GetLine(),0)); // NUM_BONES 

	for(int i=0;i<skin->bones.size();i++){
		SkinData::ABone* bone = &skin->bones[i];
		GetLine(); // BONE_DATA
		bone->name = GetWord(GetLine(),0); // NAME

		bone->indices.resize(GetInt(GetLine(),0)); // NUM_INDICES
		GetLine(); // {
		for(int j=0;j<bone->indices.size();j++)
			bone->indices[j] = GetInt(GetLine(),0);
		GetLine(); // }

		bone->weights.resize(GetInt(GetLine(),0)); // NUM_WEIGHTS
		GetLine(); // {
		for(int j=0;j<bone->weights.size();j++)
			bone->weights[j] = GetFloat(GetLine(),0);
		GetLine(); // }

		GetLine(); // MAT_OFFSET
		bone->matrixOffset = ReadMatrix();

		// Handled by animation file now
		//bone->keys.resize(GetInt(GetLine(),0)); // NUM_KEYS
		//GetLine(); // KEYS
		//for(int j=0;j<bone->keys.size();j++){
		//	bone->keys[j].time = GetInt(GetLine(),0);
		//	bone->keys[j].matrixOffset = ReadMatrix();
		//};
		//GetLine(); // KEYS }

		GetLine(); // BONE_DATA }
	}

	GetLine(); // MESH_DATA }
	GetLine(); // SKINNINGDATA }
}


// Processes the structures at the top level of the file
void ASEImport::ProcessTopLevel(){
	GetLine(); // identifier
	GetLine(); // comment

	scene.geometry.reserve(100);

	int groupDepth = 0;
	// Keep reading structs until we're done
	while(!file.eof()){

		string line = GetLine();
		if(line.length() == 0 || line.find("}") != -1){ 
			if(groupDepth > 0){
				helperKeyframes.resize(0);
				helperFrameTimes.resize(0);
				groupDepth--;
				continue;
			}
			else
				break;
		}

		string name = GetName(line);

		if(name == ID_SCENE)
			ReadSCENE();
		else if(name == ID_MATERIAL_LIST)
			scene.materials = ReadMATERIAL_LIST(line);
		else if(name == ID_GEOMETRY)
			scene.geometry.push_back(ReadGEOMOBJECT());
		else if(name == ID_GROUP){
			groupDepth++;
			continue;
		}
		else if(name == ID_HELPER)
			ReadHELPER();
		else if(name == ID_LIGHT)
			scene.lights.push_back(ReadLIGHT());
		else if(name == "*SKINNINGDATA")
			ReadSKINNINGDATA();
		else{
			Error("Unexpected type at top level: '%s'",name.c_str());
			DummyRead(line);
		}
	}


	// Map light exclusion/inclusion list geometry names into ids now
	for(int i=0;i<scene.lights.size();i++){
		Light& l = scene.lights[i].main;

		for(int j=0;j<l.excludeIncludeList.size();j++){
			bool found = false;
			for(int a=0;a<scene.geometry.size();a++){
				if(l.excludeIncludeList[j] == scene.geometry[a].name){
					l.excludeIncludeGeomIDs.push_back(a);
					found = true;
					break;
				}
			}
			if(!found)
				Error("Error -- couldn't match light exclusion list object '%s', to any geometry in the scene.",l.excludeIncludeList[j].c_str());
		}
	}

	
}


ASEImport::ASEImport(string filename, bool& success){
	TotalFacesRead = 0;
	TotalVerticesRead = 0;

	file.open(filename.c_str());

	if(!file){
		Error("Input file '%s' not found!", filename.c_str());
		success = false;
		return;
	}

	cout << "Opened file '" << filename << "'" << endl;
	cout << "Reading...";
	ProcessTopLevel();
	cout << "completed!" << endl;
	cout << "\tMaterials read:\t" << scene.materials.size() << endl;
	cout << "\tGeometry read:\t" << scene.geometry.size() << endl;
	cout << "\tLights read:\t" << scene.lights.size() << endl;
	cout << "\tTotal verts:\t" << TotalVerticesRead <<", total faces:\t" << TotalFacesRead << endl;

	file.close();

	success = true;
}