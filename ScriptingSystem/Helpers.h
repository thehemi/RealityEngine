//=========== (C) Copyright 2004, Artficial Studios. All rights reserved. =======
// C# Scripting System
// Author: Mostafa Mohamed
//
//
//===============================================================================
#pragma once
using namespace System;
using namespace System::Runtime::InteropServices;

// <summary>
/// Managed System helper functions, such as Managed-STL string conversion and error printing. 
// </summary>
namespace ScriptingSystem
{
    ref class Helpers
    {
    public:
        static std::string ToCppString(System::String ^ str)
        {
            System::IntPtr
                ptr(System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(str));
            std::string ret(static_cast<const char *>(static_cast<void *>(ptr)));
            System::Runtime::InteropServices::Marshal::FreeCoTaskMem(ptr);
            return(ret);
        }

        static String^ ToCLIString(std::string str)
        {
            return Marshal::PtrToStringAnsi(IntPtr((void*)str.c_str()),str.length());
        }

        static std::wstring ToCppStringw(System::String ^ str)
        {
            System::IntPtr
                ptr(System::Runtime::InteropServices::Marshal::StringToHGlobalUni(str));
            std::wstring  ret(static_cast<const WCHAR *>(static_cast<void *>(ptr)));
            System::Runtime::InteropServices::Marshal::FreeCoTaskMem(ptr);
            return(ret);
        }

        static String^ ToCLIString(std::wstring str)
        {
            return Marshal::PtrToStringUni (IntPtr((void*)str.c_str()),str.length());
        }

        static void PrintError(Exception^ ex,String ^ text)
        {
            string total = ToCppString(text) + "\n" + ToCppString(ex->Source) + "\n" + ToCppString(ex->StackTrace) + "\n" + ToCppString(ex->Message);
            //LogPrintf(total.c_str());
            SeriousWarning(total.c_str());
        }
    };
}