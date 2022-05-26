//=========== (C) Copyright 2004, Artficial Studios. All rights reserved. =======
// C# Scripting System
// Author: Mostafa Mohamed & Jeremy Stieglitz
//
//
//===============================================================================
#pragma once
using namespace System;
using namespace System::Collections;
using namespace Microsoft::CSharp;
using namespace System::Reflection;
using namespace System::Runtime::Remoting;
using namespace System::CodeDom::Compiler;
using namespace stdcli::language;

/// The global managed class of the ScriptingSystem, contains hashes on which to store key object types such as MActors and MWorlds
namespace ScriptingSystem
{
    private ref class ScriptingEngine
    {
         public:
            static AppDomain^       s_appDomain;
            static String^          s_tempPath;
            static Hashtable^       s_cache;
            static Hashtable^       s_dialogs;
            static ArrayList^       s_loadedFiles;
            static Assembly^        s_assembly;
            static bool             s_initialized;
			static LogicCore^		s_logicCore;
            static int              s_dialogIndex;
    public:
        static ScriptingEngine()
        {
            s_appDomain = nullptr;
            s_cache = gcnew Hashtable();
            s_loadedFiles = gcnew ArrayList();
            s_dialogs = gcnew Hashtable();
            s_initialized= false;
            s_dialogIndex = 0;
        }
    };
}
