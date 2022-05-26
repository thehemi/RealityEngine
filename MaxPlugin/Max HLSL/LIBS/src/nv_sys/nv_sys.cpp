// nv_sys.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include <windows.h>

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{

	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			// Close the system runtime.
			nv_sys::FinalSYSShutdown();
			break;
    }
    return TRUE;
}

// NOTE: This is designed to insert the main page into our auto-generated documentation.
// DOXYGEN finds it and changes the front page appropriately.
/*! \mainpage NVIDIA Developer Relations SDK.
\section intro Introduction
Introducing the NVIDIA SDK...
\section install Installation
\subsection step1 Step 1: Installing the SDK.
*/

