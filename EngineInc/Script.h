//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
/// Python Scripting systems
///
/// Author: Tim Johnson
//====================================================================================
#ifndef OBJECT_H
#define OBJECT_H

/// Forward declarations
struct _object;
typedef struct _object PyScript;

#define MakeVoidPtr(x) PyCObject_FromVoidPtr(x,NULL)
#define MakePtr(x,desc) PyCObject_FromVoidPtrAndDesc(x,"_p_"##desc,0)

//----
/// \brief Script initialization data tagged to mesh
/// This is used as meta-data when re-exporting the scene.
//----
struct ENGINE_API ScriptData{
	string			filename;
	string			classname;
	string			parentclass;
	string			comments;
	vector<string>	parameters;
	vector<string>	paramvalues;
	bool			bIncludeModel;

	operator == (ScriptData& rhs){
		return filename == rhs.filename && paramvalues.size() == rhs.paramvalues.size();
	}
	ScriptData(){
		bIncludeModel = true;
	}
};

/// Script: Game scripts
/// \brief Used to define useful extensions and level-specfic behaviour and events.
/// This class is the interface between the engine and the Python scripting system
//
/// The engine communicates with this class, which in turn communicates with 
/// the python script
class ENGINE_API Script{
private:
	int scriptID;
	char className[128];

public:
	PyScript* classObj;

	Script(){ LifeTime = -1; }
	~Script();

	int LifeTime; /// In milliseconds. World will handle the destruction of this script, so the script
	/// must be in the world script list.
	/// 0 = never expires

	const char* ClassName(){ return className; }

	/// Property Initialization
	void InitProp(char* prop, char* propValue);

	/// Get methods
	long GetInt(char* attrib);
	float GetFloat(char* attrib);
	char* GetString(char* attrib);
	void* GetObj(char* attrib);

	/// Set methods
	void Set(char* attrib, long i);
	void Set(char* attrib, int i);
	void Set(char* attrib, double f);
	void Set(char* attrib, char* str);
	void Set(char* attrib, void* obj);

	/// The arguments are all the data regarding the placement
	/// in the level editor the script needs
	/// A script belongs to a world, is attatched to a model,
	/// and has a world transform
	void Initialize(Actor* actor, class World* world, Matrix& transform);

	void Tick();

	bool HasFunction(char* funcName);

	/// To call other custom functions use this function:
	//	if(!PyObject_CallMethod(classObj,"FuncName","O",MakePtr(someobject,"SomeObjectClass"))
	/// Where "O" is the format string. O being one pointer argument.
	/// "OIFS" would be ptr, int, float, string
	/// The MakePtr macro takes an object and its class name, and creates a pointer Python can understand
	/// If you don't care about the type. You can use MakeVoidPtr(someobject)
	/// Type checking is done on all ScriptInterface functions.
	/// This means you can't pass a MakePtr(obj,"Actor") to a function which takes a World*
	/// See Python/SWIG documentation to get a better understanding

	bool Create(string& script, string& className);

};



#endif
