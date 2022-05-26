//=========== (C) Copyright 2004, Artficial Studios. All rights reserved. =======
// C# Scripting System
// Author: Mostafa Mohamed
//
//
//===============================================================================
#include "stdafx.h"
#include "ScriptEngine.h"
#include "SSystemStub.h"
#include "Serializers\XMLsystem.h"
/*
bool Deserialize(void * system,void *node)
{
    return ((XMLSystem*)system)->GetBool((DOMNode*)node);
}

int Deserialize(void * system,void *node)
{
    return ((XMLSystem*)system)->GetInt((DOMNode*)node);
}
	*/

void Serialize(void * system,TCHAR * name,bool value,void *node)
{
    DOMElement* pNode = ((XMLSystem*)system)->CreateNode((DOMNode*)node,"Param");
    ((XMLSystem*)system)->Attrib("Name",name,pNode);
    ((XMLSystem*)system)->Attrib("Type","Bool",pNode);
    ((XMLSystem*)system)->Attrib("Value",value,pNode);
}

void Serialize(void * system,TCHAR * name,const char* value,void *node)
{
    DOMElement* pNode = ((XMLSystem*)system)->CreateNode((DOMNode*)node,"Param");
    ((XMLSystem*)system)->Attrib("Name",name,pNode);
    ((XMLSystem*)system)->Attrib("Type","String",pNode);
    ((XMLSystem*)system)->Attrib("Value",string(value),pNode);
}

void Serialize(void * system,TCHAR * name,int value,void *node)
{
    DOMElement* pNode = ((XMLSystem*)system)->CreateNode((DOMNode*)node,"Param");
    ((XMLSystem*)system)->Attrib("Name",name,pNode);
    ((XMLSystem*)system)->Attrib("Type","Int",pNode);
    ((XMLSystem*)system)->Attrib("Value",value,pNode);
}

void Serialize(void * system,TCHAR * name,float value,void *node)
{
    DOMElement* pNode = ((XMLSystem*)system)->CreateNode((DOMNode*)node,"Param");
    ((XMLSystem*)system)->Attrib("Name",name,pNode);
    ((XMLSystem*)system)->Attrib("Type","Float",pNode);
    ((XMLSystem*)system)->Attrib("Value",value,pNode);
}

void Serialize(void * system,TCHAR * name,Vector &value,void *node)
{
    DOMElement* pNode = ((XMLSystem*)system)->CreateNode((DOMNode*)node,"Param");
    ((XMLSystem*)system)->Attrib("Name",name,pNode);
    ((XMLSystem*)system)->Attrib("Type","Float3",pNode);
    ((XMLSystem*)system)->Attrib("Value",value,pNode);
}

ScriptEngine* ScriptEngine::Instance () {
    static ScriptEngine inst;
    return &inst;
}

void ScriptEngine::Intialize()
{
    if (LoadLibraries())
        SSystem_CreateDomain();
    else
        Error(_U("Can't intialize the scripting engine."));
}

Actor* ScriptEngine::CreateActor(const char * classname,World* world)
{
    if (!LibsInitialized())
        return 0;

   Actor* actor = NULL;
   SSystem_CreateActor(classname,world,(void**)&actor);
   return actor;
}

void ScriptEngine::Shutdown()
{
    if (LibsInitialized())
    {
        SSystem_DestroyDomain();
        FreeLibraries();
    }
}