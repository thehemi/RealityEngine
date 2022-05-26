//=============================================================================
// Precacher - Data-driven resource & variable loading
//
// Loads items, forcing their resource managers to precache them
// Can precache from dynamic files, so very useful
//
// See FindVar() for a list of the files the precacher will parse for variables
//
// TODO: If a var is not found it should be written to a new log. The engine
// log will not be loaded at that stage
//=============================================================================
#include "stdafx.h"
#include "Engine.h"
#include "Precache.h"
#include <fstream>

vector<Precacher::ResourceClass> Precacher::ResourceClasses;
vector<Texture*>	Precacher::textures;
vector<Model*>		Precacher::models;
vector<Material*>	Precacher::materials;
vector<Sound*>		Precacher::sounds;
vector<ConfigFile>  Precacher::configs;

//-----------------------------------------------------------------------------
// Find the config a var is stored in. Returns NULL if var is not kept
//-----------------------------------------------------------------------------
ConfigFile* Precacher::FindVar(string& Class, string& var)
{
	if(!configs.size())
	{
		// Enumerate the maps
		vector<string> files;
		enumerateFiles("..\\data\\",files,1,".ini");
		// Fill the listbox
		for(int i=0;i<files.size();i++)
		{
			configs.resize(configs.size() + 1);
			configs[configs.size() - 1].Load("..\\data\\"+files[i]);
		}
	}

	for(int i=0;i<configs.size();i++){
		if(configs[i].KeyExists(var,Class))
			return &configs[i];
	}

	// TODO: Output to log or similar. Log will NOT be loaded at this point
	char buf[128];
	sprintf(buf,"Var %s in class %s is not mapped in any of the definition files",var.c_str(),Class.c_str());
	OutputDebugString(buf);
	return NULL;
}

//-----------------------------------------------------------------------------
// Loads all static resource cached by the actor MACROS
//-----------------------------------------------------------------------------
void Precacher::PrecacheResources(int classID)
{
	if(classID == -1)
		return;

	if(!ResourceClasses[classID].hasCached)
	{
		ResourceClasses[classID].hasCached = true;

		for(int i = 0; i < ResourceClasses[classID].resources.size();i++)
		{
			Resource* r = &ResourceClasses[classID].resources[i];

			if(r->m){
				r->m->Load(r->filename.c_str());
			}
			if(r->mat)
			{
				FindMedia(r->filename,"Materials");
				r->mat->Load(r->filename.c_str());
			}
			if(r->t){
				r->t->Load(r->filename);
			}
			if(r->s){
				if(r->b2DSound)
					r->s->Load2D(r->filename);
				else
					r->s->Load(r->filename);
			}

		}
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
Precacher::Resource* Precacher::GetResource(int& classID, string& className, string& varName){
	// If not initialized, add resourceclass for this class
	if(classID == -1)
	{
		bool found = false;
		// has existing resource based on classname
		for(int i = 0; i < ResourceClasses.size();i++)
		{
			if(className == ResourceClasses[i].className)
			{
				classID = i;
				found = true;
			}
		}
		// no existing resource based on classname
		if(!found)
		{
			classID = ResourceClasses.size();
			ResourceClasses.resize(ResourceClasses.size() + 1);
			ResourceClasses[classID].hasCached = false;
			ResourceClasses[classID].className = className;
		}
	}
	// Add single resource
	ResourceClasses[classID].resources.resize(ResourceClasses[classID].resources.size()+1);
	Resource* r = &ResourceClasses[classID].resources[ResourceClasses[classID].resources.size()-1];

	// Get resource name from file
	ConfigFile* cfg = FindVar(className,varName);
	if(cfg)
		r->filename = cfg->GetString(varName,className);

	return r;
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
Precacher::Resource* Precacher::GetResourceFileName(int& classID, string& className, string& fileName){
	// If not initialized, add resourceclass for this class
	if(classID == -1)
	{
		bool found = false;
		// has existing resource based on classname
		for(int i = 0; i < ResourceClasses.size();i++)
		{
			if(className == ResourceClasses[i].className)
			{
				classID = i;
				found = true;
			}
		}
		// no existing resource based on classname
		if(!found)
		{
			classID = ResourceClasses.size();
			ResourceClasses.resize(ResourceClasses.size() + 1);
			ResourceClasses[classID].hasCached = false;
			ResourceClasses[classID].className = className;
		}
	}
	// Add single resource
	ResourceClasses[classID].resources.resize(ResourceClasses[classID].resources.size()+1);
	Resource* r = &ResourceClasses[classID].resources[ResourceClasses[classID].resources.size()-1];

	r->filename = fileName;

	return r;
}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Precacher::CacheMe(string& className, string& varName, int& classID,Texture* texture)
{
	Resource* r = GetResource(classID, className, varName);
	r->t = texture;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Precacher::CacheMe(string& className, string& varName, int& classID, Sound* snd, bool b2D)
{
	Resource* r = GetResource(classID, className, varName);
	r->s = snd;
	r->b2DSound = b2D;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Precacher::CacheMe(string& className, string& varName, int& classID, Model* model)
{
	Resource* r = GetResource(classID, className, varName);
	r->m = model;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Precacher::CacheMe(string& className, string& varName, int& classID, Material** mat)
{
	*mat = new Material(varName);
	Resource* r = GetResource(classID, className, varName);
	r->mat = *mat;
}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Precacher::CacheMeFileName(string& className, string& fileName, int& classID,Texture* texture)
{
	Resource* r = GetResourceFileName(classID, className, fileName);
	r->t = texture;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Precacher::CacheMeFileName(string& className, string& fileName, int& classID, Sound* snd, bool b2D)
{
	Resource* r = GetResourceFileName(classID, className, fileName);
	r->s = snd;
	r->b2DSound = b2D;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Precacher::CacheMeFileName(string& className, string& fileName, int& classID, Model* model)
{
	Resource* r = GetResourceFileName(classID, className, fileName);
	r->m = model;
}
void Precacher::CacheMeFileName(const char* ClassName, const char* FileName, int& classID, Model* model)
{
	string className = ClassName;
	string fileName = FileName;
	Resource* r = GetResourceFileName(classID, className, fileName);
	r->m = model;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Precacher::CacheMeFileName(string& className, string& fileName, int& classID, Material** mat)
{
	*mat = new Material(fileName);
	Resource* r = GetResourceFileName(classID, className, fileName);
	r->mat = *mat;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Precacher::CacheMe(string& className, string& varName, int& classID, bool* x)
{
	ConfigFile* cfg = FindVar(className,varName);
	if(cfg)
		*x = cfg->GetBool(varName,className);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Precacher::CacheMe(string& className, string& varName, int& classID, int* x)
{
	ConfigFile* cfg = FindVar(className,varName);
	if(cfg)
		*x = cfg->GetInt(varName,className);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Precacher::CacheMe(string& className, string& varName, int& classID, float* x)
{
	ConfigFile* cfg = FindVar(className,varName);
	if(cfg)
		*x = cfg->GetFloat(varName,className);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Precacher::CacheMe(string& className, string& varName, int& classID, string* x)
{
	ConfigFile* cfg = FindVar(className,varName);
	if(cfg)
		*x = cfg->GetString(varName,className);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
Sound* Precacher::CacheSound(const char* name){
	Sound* sound = new Sound();
	sound->Load(name);
	sounds.push_back(sound);
	return sound;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
Texture* Precacher::CacheTexture(const char* name){
	Texture* texture = new Texture();
	texture->Load(name);
	textures.push_back(texture);
	return texture;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
Model* Precacher::CacheModel(const char* name){
	Model* model = new Model();
	model->Load(name);
	models.push_back(model);
	return model;
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
Material* Precacher::CacheMaterial(const char* name){
	Material* mat = new Material();
	mat->Load(name);
	materials.push_back(mat);
	return mat;
}


//-----------------------------------------------------------------------------
// Removes all cached items
// Cached items will be freed from memory if they are 
// the last remaining references only.
//-----------------------------------------------------------------------------
void Precacher::PurgeCache()
{
	for(int i=0;i<textures.size();i++)
		delete textures[i];
	for(int i=0;i<models.size();i++)
		delete models[i];
	for(int i=0;i<sounds.size();i++)
		delete sounds[i];
	for(int i=0;i<materials.size();i++)
		materials[i]->Release();

	textures.clear();
	models.clear();
	sounds.clear();
	materials.clear();

	for(int i = 0; i < ResourceClasses.size();i++)
	{
		for(int n = 0; n < ResourceClasses[i].resources.size();n++)
		{
			if(ResourceClasses[i].resources[n].t)ResourceClasses[i].resources[n].t->Destroy();
			if(ResourceClasses[i].resources[n].m)ResourceClasses[i].resources[n].m->Destroy();
			if(ResourceClasses[i].resources[n].mat)ResourceClasses[i].resources[n].mat->Release();
			if(ResourceClasses[i].resources[n].s)ResourceClasses[i].resources[n].s->Free();
		}
		ResourceClasses[i].resources.clear();
	}
	ResourceClasses.clear();
}

//-----------------------------------------------------------------------------
// Precache all resources listed in a file.
// The format of the file should be like so:
// ClassName
// ClassName
// ...
//-----------------------------------------------------------------------------
void Precacher::PrecacheFromFile(string file){
	ifstream in(file.c_str());

	if(!in)
		Error("Couldn't find precache file '%s'",file.c_str());

	while(true){
		char str[512];
		in.getline(str,512);
		PrecacheResources(str);
	}

	in.close();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Precacher::PrecacheResources(string& className)
{
	for(int i = 0; i < ResourceClasses.size();i++)
	{
		if(className == ResourceClasses[i].className)
		{
				PrecacheResources(i);
				return;
		}
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Precacher::PrecacheResources(const char* ClassName)
{
	string className = ClassName;
	for(int i = 0; i < ResourceClasses.size();i++)
	{
		if(className == ResourceClasses[i].className)
		{
				PrecacheResources(i);
				return;
		}
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Precacher::PurgeCache(string className)
{
	for(int i = 0; i < ResourceClasses.size();i++)
	{
		if(className == ResourceClasses[i].className)
		{
			for(int n = 0; n < ResourceClasses[i].resources.size();n++)
			{
				if(ResourceClasses[i].resources[n].t)ResourceClasses[i].resources[n].t->Destroy();
				if(ResourceClasses[i].resources[n].m)ResourceClasses[i].resources[n].m->Destroy();
				if(ResourceClasses[i].resources[n].mat)ResourceClasses[i].resources[n].mat->Release();
				if(ResourceClasses[i].resources[n].s)ResourceClasses[i].resources[n].s->Free();
			}

			ResourceClasses.erase(ResourceClasses.begin() + i);

			return;
		}
	}
}