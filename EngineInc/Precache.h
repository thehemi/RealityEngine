//=========== Copyright (c) 2004, Artificial Studios. All rights reserved. ===========
// Precacher - Data-driven resource & variable loading
//====================================================================================
#pragma once
#ifndef PRECACHER
#define PRECACHER
//#include "Log.h"

//--------------------------------------------------------------------------------------
/// \brief Registers, pre-loads, and purges static ( Model , Texture, Sound ) resources on demand
///
/// Also pre-loads static variables of various types on demand from data files
//--------------------------------------------------------------------------------------
class ENGINE_API Precacher {
	friend struct PrecacheMe;
private:
	/// Configs holding entity definitions
	static vector<ConfigFile> configs;
	/// Use this to find associated config
	static ConfigFile* FindVar(string& Class, string& var);

	/// Resources
	static vector<Texture*> textures;
	static vector<Model*> models;
	static vector<Sound*> sounds;
	static vector<Material*> materials;

	/// A tracked resource to be loaded within the precacher
	struct Resource
	{
		bool b2DSound;
		Texture* t;
		Model* m;
		Sound* s;
		Material* mat;
		string filename;
		Resource(){ t = NULL; m = NULL; s = NULL; mat = NULL; }
	};

	/// A single class' representation within the precacher, associating it with all resources it asks to precache
	/// (so that they can be loaded upon first instantiation of the class)
	struct ResourceClass
	{
		ResourceClass(){hasCached = false;}
		bool hasCached;
		string className;
		vector<Resource> resources;
	};
	
	static Resource* GetResource(int& classID, string& className, string& varName);
	static Resource* GetResourceFileName(int& classID, string& className, string& fileName);
	static vector<ResourceClass> ResourceClasses;

public:

	/// Loads and returns Sound resource from the filename, while storing it on internal list of tracked resources
	static Sound*	CacheSound(const char* name);
	/// Loads and returns Texture resource from the filename, while storing it on internal list of tracked resources
	static Texture* CacheTexture(const char* name);
	/// Loads and returns Model resource from the filename, while storing it on internal list of tracked resources
	static Model*	CacheModel(const char* name);
	/// Loads and returns Material resource from the filename, while storing it on internal list of tracked resources
	static Material* CacheMaterial(const char* name);

	/// Called by resource macros for registration  of static resources upon initialization of the application 
	static void CacheMe(string& className, string& varName, int& classID,Texture* texture);
	/// Called by resource macros for registration  of static resources upon initialization of the application 
	static void CacheMe(string& className, string& varName, int& classID,Sound* snd, bool b2D);
	/// Called by resource macros for registration  of static resources upon initialization of the application 
	static void CacheMe(string& className, string& varName, int& classID,Model* m);
	/// Called by resource macros for registration  of static resources upon initialization of the application 
	static void CacheMe(string& className, string& varName, int& classID,Material** mat);

	/// Called by resource macros for loading of static variables (from data files) upon initialization of the application 
	static void CacheMe(string& className, string& varName, int& classID,int* x);
	/// Called by resource macros for loading of static variables (from data files) upon initialization of the application 
	static void CacheMe(string& className, string& varName, int& classID,bool* x);
	/// Called by resource macros for loading of static variables (from data files) upon initialization of the application 
	static void CacheMe(string& className, string& varName, int& classID,float* x);
	/// Called by resource macros for loading of static variables (from data files) upon initialization of the application 
	static void CacheMe(string& className, string& varName, int& classID,string* x);

	/// Called by resource macros for registration  of static resources upon initialization of the application 
	static void CacheMeFileName(string& className, string& fileName, int& classID,Texture* texture);
	/// Called by resource macros for registration  of static resources upon initialization of the application 
	static void CacheMeFileName(string& className, string& fileName, int& classID,Sound* snd, bool b2D);
	/// Called by resource macros for registration  of static resources upon initialization of the application 
	static void CacheMeFileName(string& className, string& fileName, int& classID,Model* m);
	/// Called by resource macros for registration  of static resources upon initialization of the application 
	static void CacheMeFileName(string& className, string& fileName, int& classID,Material** mat);

	static void CacheMeFileName(const char* ClassName, const char* FileName, int& classID, Model* model);

	/// Loads data for all registered resources of a given class (by way of its automatic class resource registration ID), usually called once upon first initialization of an instance of said class
	static void PrecacheResources(int classID);
	/// Loads data for all registered resources of a given class (by way of its class name, for manual class precaching), usually called once upon first initialization of an instance of said class during
	static void PrecacheResources(string& className);
	static void PrecacheResources(const char* ClassName);

	/// Loads file containg list of resource files to store in global cache for faster loading during gameplay
	static void PrecacheFromFile(string file);

	/// Frees precached resources for a particular class
	static void PurgeCache(string className);
	/// Frees all precached resources
	static void PurgeCache();
};

#ifndef DOXYGEN_IGNORE
//--------------------------------------------------------------------------------------
// Macro helper class, because macros can't directly call functions
//--------------------------------------------------------------------------------------
struct PrecacheMe
{
	PrecacheMe(string className, string varName, int& classID,Texture* texture){Precacher::CacheMe(className, varName, classID,texture);}
	PrecacheMe(string className, string varName, int& classID,Sound* s, bool b2D){Precacher::CacheMe(className, varName, classID,s, b2D);}
	PrecacheMe(string className, string varName, int& classID,Model* m)	{Precacher::CacheMe(className, varName, classID,m);}
	PrecacheMe(string className, string varName, int& classID,Material** m)	{Precacher::CacheMe(className, varName, classID,m);}
	PrecacheMe(string className, string varName, int& classID,bool* m)	{Precacher::CacheMe(className, varName, classID,m);}
	PrecacheMe(string className, string varName, int& classID,int* m)	{Precacher::CacheMe(className, varName, classID,m);}
	PrecacheMe(string className, string varName, int& classID,float* m)	{Precacher::CacheMe(className, varName, classID,m);}
	PrecacheMe(string className, string varName, int& classID,string* m){Precacher::CacheMe(className, varName, classID,m);}
};

// Use to declare resources in headers
#define USES_PRECACHING()		static int myPrecacheID;
// Use to register resources or variables at the top of the cpp files..
#define RegBegin(CLASSNAME)	int CLASSNAME::myPrecacheID = -1;
#define RegTexture(CLASSNAME,VARNAME)	Texture CLASSNAME::VARNAME; static PrecacheMe _##CLASSNAME##VARNAME(#CLASSNAME,#VARNAME,CLASSNAME::myPrecacheID,&CLASSNAME::VARNAME);
#define RegSound(CLASSNAME,VARNAME)	Sound CLASSNAME::VARNAME;	static PrecacheMe _##CLASSNAME##VARNAME(#CLASSNAME,#VARNAME,CLASSNAME::myPrecacheID,&CLASSNAME::VARNAME,false);
#define RegSound2D(CLASSNAME,VARNAME)	Sound CLASSNAME::VARNAME;	static PrecacheMe _##CLASSNAME##VARNAME(#CLASSNAME,#VARNAME,CLASSNAME::myPrecacheID,&CLASSNAME::VARNAME,true);
#define RegModel(CLASSNAME,VARNAME)	Model CLASSNAME::VARNAME;	static PrecacheMe _##CLASSNAME##VARNAME(#CLASSNAME,#VARNAME,CLASSNAME::myPrecacheID,&CLASSNAME::VARNAME);
#define RegMaterial(CLASSNAME,VARNAME)	Material* CLASSNAME::VARNAME;	static PrecacheMe _##CLASSNAME##VARNAME(#CLASSNAME,#VARNAME,CLASSNAME::myPrecacheID,&CLASSNAME::VARNAME);
#define RegBool(CLASSNAME,VARNAME)		bool CLASSNAME::VARNAME;	static PrecacheMe _##CLASSNAME##VARNAME(#CLASSNAME,#VARNAME,CLASSNAME::myPrecacheID,&CLASSNAME::VARNAME);
#define RegInt(CLASSNAME,VARNAME)		int CLASSNAME::VARNAME;		static PrecacheMe _##CLASSNAME##VARNAME(#CLASSNAME,#VARNAME,CLASSNAME::myPrecacheID,&CLASSNAME::VARNAME);
#define RegFloat(CLASSNAME,VARNAME)	float CLASSNAME::VARNAME;	static PrecacheMe _##CLASSNAME##VARNAME(#CLASSNAME,#VARNAME,CLASSNAME::myPrecacheID,&CLASSNAME::VARNAME);
#define RegString(CLASSNAME,VARNAME)	string CLASSNAME::VARNAME;	static PrecacheMe _##CLASSNAME##VARNAME(#CLASSNAME,#VARNAME,CLASSNAME::myPrecacheID,&CLASSNAME::VARNAME);
// Use in constructor to load the resources
#define DoPrecache() Precacher::PrecacheResources(myPrecacheID);

//define a class' character name
#define CLASS_NAME(T) public: inline virtual const char* ClassName(){ return #T; }

#endif

#endif