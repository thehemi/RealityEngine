//=========== (C) Copyright 2003, Tim Johnson. All rights reserved. ===========
// Script: Game scripts
// Used to define useful extensions and level-specfic behaviour and events
// All Actor placement in the editor is via scripts
// This is so Actor properties can be easily configured without recompiling
//
// This class is the interface between the engine and the scripting system
// The engine communicates with this class, which in turn communicates with 
// the python script
//
// TODO: Need to check reference counting for leaks.
//=============================================================================
#include "stdafx.h"
#include <python.h>
#include "ScriptSystem.h"

Script::~Script(){
	ScriptSystem::Instance()->UnloadScript(scriptID);
}
/*
struct swig_type_info;
__declspec(dllimport) PyObject *        SWIG_NewPoitnerObj(void *, swig_type_info *,int own);


PyObject* MakeVoidPtr(void* ptr){
	return SWIG_NewPointerObj(ptr, 0,0);
}*/


//-----------------------------------------------------------------------------
// Name: InitProp()
// Desc: Creates & assigns an instance of propValue (a constructor or type)
//-----------------------------------------------------------------------------
void Script::InitProp(char* prop, char* propValue){
	PyObject* pyObj = PyRun_String(propValue,Py_eval_input,ScriptSystem::Instance()->scripts[scriptID].dict,ScriptSystem::Instance()->scripts[scriptID].dict);//engine.scriptSys.globals,engine.scriptSys.globals);//engine.scriptSys.scripts[map.objects[i]->scriptID].dict,
	int ret = PyObject_SetAttrString(classObj,prop,pyObj);
	if(ret < 0)
		Warning(_U("Error setting script variable '%s' to '%s': Reason: %s"),prop,propValue,GetPythonErrorMessage());
}


//-----------------------------------------------------------------------------
// Name: Create()
// Desc: Creates an object instance given a script and class
//-----------------------------------------------------------------------------
bool Script::Create(string& script, string& className){
	// Load the script, get the class, create an instance of the class, and assign to to classObj
	scriptID = ScriptSystem::Instance()->LoadScript((char*)script.c_str());
	if(scriptID == -1) //failed
		return false;

	classObj = ScriptSystem::Instance()->GetAttrib(scriptID,(char*)className.c_str());
	PyObject* arg = Py_BuildValue("()");
    if(!classObj)
    {
        SeriousWarning("The file %s has a different class name than file name. They must match.",className.c_str());
        return false;
    }
	classObj = PyInstance_New(classObj, arg, NULL);
	strcpy(this->className,className.c_str());

	// Minor optimization:
	// http://groups.google.com/groups?q=PyObject_CallMethod+PyObject_CallObject&selm=J66D5.40285%24g6.15995673%40news2.rdc2.tx.home.com&rnum=7
	// Load all the core functions into PyObjects now, to save
	// doing it for every call
	return true;
}

bool Script::HasFunction(char* funcName){
	if(scriptID == -1)
		return false;
	int val = PyObject_GetAttrString(classObj,funcName)!=0;
	if(!val) // Failed, so clear the error buffer
		PyErr_Clear();
	return val;
}

void Script::Initialize(Actor* actor, World* world, Matrix& transform){
	if(scriptID == -1)
		return;
	if(!PyObject_CallMethod(classObj,"Init","OOOO",MakePtr(this,"Script"),MakePtr(world,"World"),MakePtr(actor,"Actor"),MakePtr(&transform,"Matrix"))){
		Warning(_U("Class %s's Initialization failed.\n classObj = 0x%x\n Python traceback: %s"),className,classObj,GetPythonErrorMessage());
		scriptID = -1;
	}
}

//-----------------------------------------------------------------------------
// Name: Tick()
// Desc: Calls the frame tick method for the object
//-----------------------------------------------------------------------------
void Script::Tick(){
	if(scriptID == -1)
		return;
	PyObject* ret = PyObject_CallMethod(classObj,"Tick",NULL);
	if(!ret){
		Warning(_U("Error calling Script::Tick: %s"),GetPythonErrorMessage());
		scriptID = -1;
	}
}

//-----------------------------------------------------------------------------
// Name: Touched()
// Desc: Called when another object collides with this one
//-----------------------------------------------------------------------------
/*void Script::Touched(Actor* ent){
	PyObject* ret = PyObject_CallMethod(classObj,"Touched","O",ent);
	if(!ret)
		Error("Error calling Script::Touched: %s",GetPythonErrorMessage());
}
*/

//-----------------------------------------------------------------------------
// Name: Set Methods
// Desc: Various methods to set the object attributes
//-----------------------------------------------------------------------------
void Script::Set(char* attrib, int i){
	if(scriptID == -1)
		return;
	Set(attrib,(long)i);
}

void Script::Set(char* attrib, long i){
	if(scriptID == -1)
		return;
	PyObject_SetAttrString(classObj,attrib,PyInt_FromLong(i));
}

void Script::Set(char* attrib, double d){
	if(scriptID == -1)
		return;
	PyObject_SetAttrString(classObj,attrib,PyFloat_FromDouble(d));
}

void Script::Set(char* attrib, char* str){
	if(scriptID == -1)
		return;
	PyObject_SetAttrString(classObj,attrib,PyString_FromString(str));
}

void Script::Set(char* attrib, void* ptr){
	if(scriptID == -1)
		return;
	PyObject_SetAttrString(classObj,attrib,(PyObject*)(ptr));
}



//-----------------------------------------------------------------------------
// Name: Get Methods
// Desc: Various methods to get the object attributes
//-----------------------------------------------------------------------------
char* Script::GetString(char* attrib){
	char* str;
	if(!(str = PyString_AsString(PyObject_GetAttrString(classObj,attrib))))
		Error(_U("Paranoia: Script conversion might have failed for attrib: %s"),attrib);
	return str;
}


float Script::GetFloat(char* attrib){
	float f;
	if((f = PyFloat_AsDouble(PyObject_GetAttrString(classObj,attrib))) == -1)
		Error(_U("Paranoia: Script conversion might have failed for attrib: %s"),attrib);
	return f;
}


long Script::GetInt(char* attrib){
	long i;
	if((i = PyInt_AsLong(PyObject_GetAttrString(classObj,attrib))) == -1)
		Error(_U("Paranoia: Script conversion might have failed for attrib: %s"),attrib);
	return i;
}

void* Script::GetObj(char* attrib){
	void* obj;
	PyObject* pyObj;
	pyObj = PyObject_GetAttrString(classObj,attrib);

	//extern void* MakeObjectPtr(PyObject* pyObj);
	//obj = MakeObjectPtr(pyObj);
	//if(!obj)
	//	Error("DEBUG: Couldn't convert script object to C pointer");]
	obj = pyObj;

	return obj;
}

