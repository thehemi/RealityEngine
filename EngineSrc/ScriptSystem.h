//=========== (C) Copyright 2003, Tim Johnson. All rights reserved. ===========
/// ScriptSystem: Python runtime script system management.
/// Used by Script 
//=============================================================================
#ifndef SCRIPTSYSTEM_H
#define SCRIPTSYSTEM_H

#include <python.h>

/// From game_wrap
char* GetPythonErrorMessage();

/// ScriptSystem: Python runtime script system management.
class ScriptSystem{
protected:
	friend class Engine;
	friend class Script;
	void Initialize();
	void Shutdown();

	/// Returns an ID for the script
	int LoadScript(char* name);
	/// Executes a script
	//bool RunScript(int ID);

	/// Used for the above function call
	PyObject* GetFunc(int ID, char* name);

	/// Get an attribute, like a class or variable
	PyObject* GetAttrib(int ID, char* attrib);

	/// Run a single command string
	bool RunCmd(char* cmdStr);

	/// Unloads and destroys a loaded script
	void UnloadScript(int ID);

	PyObject* globals;
	/// Contains Python Script within the ScriptingSystem
	struct Script{
		int id;
		PyObject* module,*dict;
		string name;
	};
	vector<Script> scripts;

	/// Returns an index into the script array, or -1 if not found
	int FindScript(int ID);
	
public:
	/// Singleton
	static ScriptSystem* Instance ();
	/// Finds a script
	int FindScript(char* name);
	/// Executes a script, adds to world array if necessary
	void RunScript(ScriptData& data, Matrix tm, Actor* actor, World* world);
};


#endif
