//=========== Copyright (c) 2003, Tim Johnson. All rights reserved. ===========
// ScriptSystem: Python runtime script system management.
//
// NOTE: A bit evil, but has some special-cases for game scripts
//
//=============================================================================
#include "stdafx.h"
#include <python.h>
#include <stdio.h>
#include <compile.h>
#include <eval.h>
#include "ScriptSystem.h"

// Returns a singleton instance
ScriptSystem* ScriptSystem::Instance () {
    static ScriptSystem inst;
    return &inst;
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
// 
//-----------------------------------------------------------------------------
void ScriptSystem::RunScript(ScriptData& data, Matrix tm, Actor* actor, World* world)
{
	if(data.classname.length() == 0)
		return;

	::Script* script = new ::Script;
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
	script->Initialize(actor,world,tm);	

	// TODO: Support model loading of scripts
	// Only add scripts to map if they define the Tick() function
	if(script->HasFunction("Tick"))
		world->m_Scripts.push_back(script);
	else
		delete script; // Has no further use
}


#define GPEM_ERROR(what) {errorMsg = "Python had no useful info on this error (" ## what ## ")";goto done;}
char * GetPythonErrorMessage()
{
	char *result = NULL;
	char *errorMsg = NULL;
	PyObject *modStringIO = NULL;
	PyObject *modTB = NULL;
	PyObject *obFuncStringIO = NULL;
	PyObject *obStringIO = NULL;
	PyObject *obFuncTB = NULL;
	PyObject *argsTB = NULL;
	PyObject *obResult = NULL;
	PyObject *exc_typ, *exc_val, *exc_tb;
	/* Fetch the error state now before we cruch it */
	PyErr_Fetch(&exc_typ, &exc_val, &exc_tb);
	
	/* Import the modules we need - cStringIO and traceback */
	modStringIO = PyImport_ImportModule("StringIO");
	if (modStringIO==NULL) GPEM_ERROR("cant import cStringIO");
	modTB = PyImport_ImportModule("traceback");
	if (modTB==NULL) GPEM_ERROR("cant import traceback");
	
	/* Construct a cStringIO object */
	obFuncStringIO = PyObject_GetAttrString(modStringIO, "StringIO");
	if (obFuncStringIO==NULL) GPEM_ERROR("cant find cStringIO.StringIO");
	obStringIO = PyObject_CallObject(obFuncStringIO, NULL);
	if (obStringIO==NULL) GPEM_ERROR("StringIO.StringIO() failed");
	
	/* Get the traceback.print_exception function, and call it. */
	obFuncTB = PyObject_GetAttrString(modTB, "print_exception");
	if (obFuncTB==NULL) GPEM_ERROR("cant find traceback.print_exception");
	argsTB = Py_BuildValue("OOOOO",
		exc_typ ? exc_typ : Py_None,
		exc_val ? exc_val : Py_None,
		exc_tb  ? exc_tb  : Py_None,
		Py_None,
		obStringIO);
	if (argsTB==NULL) GPEM_ERROR("cant make print_exception arguments");
	
	obResult = PyObject_CallObject(obFuncTB, argsTB);
	if (obResult==NULL) GPEM_ERROR("traceback.print_exception() failed");
	
	/* Now call the getvalue() method in the StringIO instance */
	Py_DECREF(obFuncStringIO);
	obFuncStringIO = PyObject_GetAttrString(obStringIO, "getvalue");
	if (obFuncStringIO==NULL) GPEM_ERROR("cant find getvalue function");
	Py_DECREF(obResult);
	obResult = PyObject_CallObject(obFuncStringIO, NULL);
	if (obResult==NULL) GPEM_ERROR("getvalue() failed.");
	
	/* And it should be a string all ready to go - duplicate it. */
	if (!PyString_Check(obResult))
		GPEM_ERROR("getvalue() did not return a string");
	result = strdup(PyString_AsString(obResult));
done:
	if (result==NULL && errorMsg != NULL)
		result = strdup(errorMsg);
	Py_XDECREF(modStringIO);
	Py_XDECREF(modTB);
	Py_XDECREF(obFuncStringIO);
	Py_XDECREF(obStringIO);
	Py_XDECREF(obFuncTB);
	Py_XDECREF(argsTB);
	Py_XDECREF(obResult);
	
	/* Restore the exception state */
	PyErr_Restore(exc_typ, exc_val, exc_tb);
	return result;
}



int ScriptSystem::FindScript(char* name){
	for(int i=0;i<scripts.size();i++){
		if(scripts[i].name == name)
			return i;
	}
	return -1;
}

int ScriptSystem::FindScript(int ID){
	for(int i=0;i<scripts.size();i++){
		if(scripts[i].id == ID)
			return i;
	}
	return -1;
}

int ScriptSystem::LoadScript(char* name)
{
	static int id = 0;
	
	// See if this script has already been loaded
	//int index = FindScript(name);
	/*if(index!=-1){
		// Increase the reference count
		Py_INCREF(scripts[index].module);
		// Return the ID to the script
		return scripts[index].id;
	}*/
	
	//_tcscpy(name,ToLowerCase(string(name)));
	PyObject* pmod = PyImport_ImportModule(name);

	if(!pmod){
		Warning(("Couldn't load script '%s'. %s. \nExtended info: Current directory = %s\nPython search paths = %s"),name,GetPythonErrorMessage(),GetDir(),Py_GetPath());
		return -1;
	}

    //PyObject* my_python_object = PyObject_GetAttrString(pmod, "coolio");
	PyObject* dict=PyModule_GetDict(pmod);
	
	
	// Create the new script and add it to our array
	Script newScript;
	newScript.module = pmod;
	newScript.dict = dict;
	newScript.id = id++;
	newScript.name = name;
	scripts.push_back(newScript);
	return newScript.id;
}


bool ScriptSystem::RunCmd(char* cmdStr){
	int retval = PyRun_SimpleString(cmdStr);
	if(retval == -1) 
	{
		LogPrintf(_U("The command failed. Error information is not available in this mode. Check your case and syntax carefully."));
		return false;
	} 
	return true;
}

/*
bool ScriptSystem::RunScript(int ID){
int index = FindScript(ID);
if(index == -1)
return false;

  // execute the compiled statement
  PyObject *result = PyEval_EvalCode((PyCodeObject *)scripts[index].code, globals, 0);
  if(!result) 
  {
		Error("Script execution failed. %s",GetPythonErrorMessage());
		return false;
		} 
		else 
		{
		Py_DECREF(result);
		return true;
		}
		}
*/

void ScriptSystem::UnloadScript(int ID){
	int index = FindScript(ID);
	if(index == -1)
		return;
	
	Py_DECREF(scripts[index].module);
	// If the reference count is 0, delete the script entry
	if(scripts[index].module == 0){
		vector<Script>::iterator ppEachItem = scripts.begin();
		scripts.erase(ppEachItem+index);
	}
}


PyObject* ScriptSystem::GetAttrib(int ID, char* attrib){
	PyObject* obj = PyObject_GetAttrString(scripts[FindScript(ID)].module,attrib);
	//if(!obj)
	//	Error("Script object not found: %s", attrib);

	return obj;
}


PyObject* ScriptSystem::GetFunc(int ID, char* name){
	int index = FindScript(ID);
	if(index == -1)
		Warning(_U("GetFunc(): Script not found. Func: %s"),name);

	PyObject* foo = PyDict_GetItemString(scripts[index].dict, name);
	//if(!foo)
	//	Error("Failed trying to find function. %s",GetPythonErrorMessage());
	return foo;
}
/*
void ScriptSystem::RunFunction(int ID, char *command, char *format, ...) {
	int index = FindScript(ID);
	if(index == -1)
		Error("Script not found");
	
	va_list args;
    va_start (args, format);

	//PyObject* ptr = va_arg(args, PyObject*); 
	do{

	
	
	if(!args) Error("ScriptSystem::RunFunction: No arguments given");
	
	PyObject* foo = PyDict_GetItemString(scripts[index].dict, command);
	if(!foo)
		Error("Failed trying to find function. %s",GetPythonErrorMessage());
	
	PyObject* result = PyObject_CallFunction(foo,format,args);
	if(!result)
		Error("Failed trying to call function. %s",GetPythonErrorMessage());
	
	va_end(args);
	
	return;
}*/

//extern "C" __declspec(dllimport) void initvectorc(void);
//extern "C" __declspec(dllimport) void initgame(void);

void ScriptSystem::Initialize(){
	// Initialize python interpreter
	Py_Initialize();
	// SWIG interface init (see game.i)
//	initgame();
	// prepare an innocuous 'globals' dictionary
	globals = PyDict_New();
	PyDict_SetItemString(globals, "__builtins__",PyEval_GetBuiltins());
	

	string scripts = Engine::Instance()->MainConfig->GetString("SearchPath") + "\\Scripts\\";
	PySys_SetPath((char*)scripts.c_str()); 

	// Vector
//	initvectorc();
//	LoadScript("vector");
}

void ScriptSystem::Shutdown(){
	Py_Finalize();
}

