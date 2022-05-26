//=========== (C) Copyright 2004, Artficial Studios. All rights reserved. =======
/// C# Scripting System
/// Author: Mostafa Mohamed
//
//
//===============================================================================
#ifndef SCRIPTENGINE_H
#define SCRIPTENGINE_H

// Helper Functions for serializing MActors
void ENGINE_API Serialize(void * system,TCHAR * name,bool value,void *node);
void ENGINE_API Serialize(void * system,TCHAR * name,const char* value,void *node);
void ENGINE_API Serialize(void * system,TCHAR * name,int value,void *node);
void ENGINE_API Serialize(void * system,TCHAR * name,float value,void *node);
void ENGINE_API Serialize(void * system,TCHAR * name,Vector &value,void *node);
/*
bool ENGINE_API Deserialize(void * system,void *node);
int  ENGINE_API Deserialize(void * system,void *node);
*/
/// C# Scripting Engine
class ENGINE_API  ScriptEngine{
public:
    void Intialize();
    Actor* CreateActor(const char * classname,World* world);
    void Shutdown();
    static ScriptEngine* Instance();
};


#endif