//=========== (C) Copyright 2003, Tim Johnson. All rights reserved. ===========
// Traces and outputs the stack as a string, even on release builds
//=============================================================================
#include "stdafx.h"
#include <psapi.h>
#include "dbghelp.h"

bool InitSymEng ( HANDLE procHandle );
void LoadProcessModules ( HANDLE procHandle );
int PrintStackWalk( char * pc, int iPcLen, EXCEPTION_POINTERS *pExPtrs);


typedef BOOL (WINAPI *TFEnumProcesses)(
	DWORD * lpidProcess, DWORD cb, DWORD * cbNeeded
);
typedef BOOL (WINAPI *TFEnumProcessModules)(
	HANDLE hProcess, HMODULE *lphModule, DWORD cb, LPDWORD lpcbNeeded
);
typedef DWORD (WINAPI *TFGetModuleBaseName)(
	HANDLE hProcess, HMODULE hModule, LPSTR lpBaseName, DWORD nSize
);
typedef DWORD (WINAPI *TFGetModuleFileNameEx)(
	HANDLE hProcess, HMODULE hModule, LPSTR lpFilename, DWORD nSize
);
typedef BOOL (WINAPI *TFGetModuleInformation)(
	HANDLE hProcess, HMODULE hModule, LPMODULEINFO lpmodinfo, DWORD cb
);

TFEnumProcesses FEnumProcesses;
TFEnumProcessModules FEnumProcessModules;
TFGetModuleBaseName FGetModuleBaseName;
TFGetModuleFileNameEx FGetModuleFileNameEx;
TFGetModuleInformation FGetModuleInformation;


/****************************************************************/
/*  Function : PrintStackToString                               */
/*  Input    : pc = char [] to write the stack dmp to           */
/*           : iPcLen = size of pc buffer above                 */
/*  Returns  : 0                                                */
/*  This function dumps the stack to a string. To get stack dump*/
/*  this func throws an exception, and the exception handler    */
/*  parses and dumps the stack                                  */
/****************************************************************/ 
int skipFrames = 0;
int PrintStackToString( char * pc, int iPcLen )
{
	skipFrames = 1;
    __try
    {
          /* we will throw an exception here to get the thread context
*/
        int * p = NULL ;
        *p = 0 ;
    }
    __except ( PrintStackWalk( pc, iPcLen, GetExceptionInformation()
))
    {
        return 0;
    } return 0;
}


/****************************************************************/
/*  Function : PrintStackWalk                                   */
/*  Input    : pc = char [] to write the stack dmp to           */
/*           : iPcLen = size of pc buffer above                 */
/*           : pExPtrs = Pointer to the exception information   */
/*  Returns  : 0                                                */
/*  This function dumps the stack to a string.                  */
/*  This function is called as an exception handler so any      */
/*  exceptions thrown within this function should be caught     */
/*  locally otherwise we end up in an infinite loop....         */
/****************************************************************/ 
int PrintStackWalk( char * pc, int iPcLen, EXCEPTION_POINTERS *pExPtrs)
{
    STACKFRAME sf;
    CONTEXT * pContext;
    unsigned char symbolBuffer[ sizeof(IMAGEHLP_SYMBOL) + 512 ];
    PIMAGEHLP_SYMBOL pSymbol;
    DWORD symDisplacement;
    IMAGEHLP_LINE imghlp_line;
    IMAGEHLP_MODULE imghlp_module;
    HANDLE processHandle;
    HANDLE threadHandle;
    unsigned long lastError;
    BOOL rv;
    char fileName_Line[1024];
    char moduleName[1024];
    char functionName[1024];
    int  iStrLen;

    __try
    {
        pContext = pExPtrs->ContextRecord;

        processHandle = GetCurrentProcess();
        threadHandle = GetCurrentThread();

        sprintf( pc, "\n\nCall stack:\n");
        memset( &sf, 0, sizeof(sf) );

          /* Initialize the STACKFRAME structure */
        sf.AddrPC.Offset       = pContext->Eip;
        sf.AddrPC.Mode         = AddrModeFlat;
        sf.AddrStack.Offset    = pContext->Esp;
        sf.AddrStack.Mode      = AddrModeFlat;
        sf.AddrFrame.Offset    = pContext->Ebp;
        sf.AddrFrame.Mode      = AddrModeFlat;
    
          /* initialize the symbol engine */
		if(!InitSymEng ( processHandle)){
			strcpy(pc,"PSAPI.DLL is not installed on this machine. Cannot create stack trace");
			return 0;
		}

        *pc = (char ) NULL;
    
          /* start printing the stack one frame at a time */
        while ( 1 )
        {
            rv = StackWalk(  IMAGE_FILE_MACHINE_I386, 
                                (HANDLE) processHandle, 
                                threadHandle, 
                                &sf,
                                pContext,
                                NULL,
                                SymFunctionTableAccess,
                                SymGetModuleBase,
                                NULL );

            if ( !rv  )
            {
                  /* StackWalk failed! give up */
                lastError = GetLastError( );
                break;
            }

            if ( sf.AddrFrame.Offset == 0 )
            {
                  /* this frame offset is not valid */
                break;
            }

			// Skip this frame if we were told to skip the first X frames
			// (usually because those frames are PrintStackToString/Error, which we want
			// to ignore in the stack trace)
			if(skipFrames-- > 0)
				continue;
                
              /* Displacement of the input address, relative to the 
                 start of the symbol */
            symDisplacement = 0;  
        
            pSymbol = (PIMAGEHLP_SYMBOL) symbolBuffer;
            pSymbol->SizeOfStruct = sizeof(symbolBuffer);
            pSymbol->MaxNameLength = 512;
        
            rv = SymGetSymFromAddr( processHandle, sf.AddrPC.Offset, 
                                   &symDisplacement, pSymbol);
            if ( rv )
            {
                sprintf( functionName,  ("%s() "), pSymbol->Name);
                iStrLen = strlen( pc ) + strlen( functionName );
                if ( iPcLen > iStrLen )
                    strcat( pc, functionName);
                else
                    break;
            }
            else
            {
                  /* no symbol found for this address */
                lastError = GetLastError( );
            }

            rv = SymGetLineFromAddr(processHandle, sf.AddrPC.Offset,
                                    &symDisplacement, &imghlp_line);
            if ( rv )
            {
                sprintf( fileName_Line, ("0x%-8x + %d bytes [File=%sline=%d] "), sf.AddrPC.Offset,
                         symDisplacement, imghlp_line.FileName,
imghlp_line.LineNumber );
            }
            else    
            {
                  /* No line number found.  Print out the logical
address instead. */
                sprintf( fileName_Line, "Address = 0x%-8x (filenamenot found) ", sf.AddrPC.Offset );
            }

            iStrLen = strlen( pc ) + strlen( fileName_Line );
            if ( iPcLen > iStrLen && strlen( fileName_Line ))
                strcat( pc, fileName_Line );
            else
                break;
        
            rv = SymGetModuleInfo( processHandle, sf.AddrPC.Offset, 
                                   &imghlp_module);
            if ( rv )
            {
                sprintf( moduleName,  "[in %s]",
imghlp_module.ImageName);
            }
            else
            {
                lastError = GetLastError( );
            }

			int mLen = strlen( moduleName );
            iStrLen = strlen( pc ) + mLen;
            if ( iPcLen > iStrLen && mLen && moduleName[0] != -52 )
                strcat( pc, moduleName );
           // else
            //    break;
    
            strcat( pc, "\n");

        } /* while(1) */
    } 
    __except ( EXCEPTION_EXECUTE_HANDLER )
    {
          /* we need to catch any execptions within this function so
that
             they dont get sent to PrintStackToString() because this 
             function itself is the exception handler.  Otherwise, we
will
             go into an infinite loop... */
        strcpy(pc, "\nException thrown in PrintStackWalk()\n\n");
        return EXCEPTION_EXECUTE_HANDLER;
    } return EXCEPTION_EXECUTE_HANDLER;
}

/****************************************************************/
/*  Function : InitSymEng                                       */
/*  Input    : procHandle = Handle to process for symbol engine */
/*  Returns  : void                                             */
/* Initializes the symbol engine if needed.                     */
/****************************************************************/ 
bool InitSymEng ( HANDLE procHandle )
{
	static int symEngInitialized = 0;

    if ( symEngInitialized  == 0 )
    {
		HINSTANCE m_hHandle = ::LoadLibrary(_T("PSAPI.DLL"));
		if(!m_hHandle)
			return false;
		// Load dynamically linked PSAPI routines
		FEnumProcesses = (TFEnumProcesses)::GetProcAddress(m_hHandle,("EnumProcesses"));
		FEnumProcessModules = (TFEnumProcessModules)::GetProcAddress(m_hHandle,("EnumProcessModules"));
		FGetModuleFileNameEx = (TFGetModuleFileNameEx)::GetProcAddress(m_hHandle,("GetModuleFileNameExA"));
		FGetModuleBaseName = (TFGetModuleBaseName)::GetProcAddress(m_hHandle,("GetModuleBaseNameA"));
		FGetModuleInformation = (TFGetModuleInformation)::GetProcAddress(m_hHandle,("GetModuleInformation"));

          /* Set up the symbol engine. */
        DWORD dwOpts = SymGetOptions ( );

        dwOpts |= SYMOPT_LOAD_LINES ;
        dwOpts |= SYMOPT_DEBUG;


          /* Turn on load lines. */
        SymSetOptions ( dwOpts );

          /* Initialize the symbol engine. */
        SymInitialize ( procHandle, NULL, TRUE );

        LoadProcessModules( procHandle ); 

        symEngInitialized = 1 ;
    }
	return true;
}

/****************************************************************/
/*  Function : LoadProcessModules                               */
/*  Input    : procHandle = Handle to process for symbol engine */
/*  Returns  : void                                             */
/* Loads the necassary modules.                                 */
/****************************************************************/ 
static void LoadProcessModules ( HANDLE procHandle )
{
#define MAX_MOD_HANDLES 1024
    int rv, freeModArray, moduleCount, error, i;
    HMODULE * pModHandle;
    HMODULE modHandleArray[MAX_MOD_HANDLES];
    DWORD bytesRequired;
    char modName[1024];
    char imgName[1024];
    MODULEINFO modInfo;


    pModHandle = modHandleArray;
    freeModArray = 0;

    rv = FEnumProcessModules( procHandle, modHandleArray, 
                             sizeof(modHandleArray), &bytesRequired );
    
    if ( rv == 0 )
    {
        error = GetLastError();
        return;
    }

    if ( bytesRequired > sizeof( modHandleArray ))
    {
        pModHandle = (HMODULE*)malloc ( bytesRequired );
        if ( pModHandle == NULL )
        {
              /* malloc failed */
            return;
        }
        freeModArray = 1;
        rv = FEnumProcessModules( procHandle, pModHandle, 
                                 sizeof(modHandleArray),
&bytesRequired );
    }

    moduleCount = bytesRequired / sizeof( HMODULE );

    for ( i = 0; i < moduleCount; i++ )
    {
        FGetModuleInformation( procHandle, modHandleArray[i], &modInfo,
sizeof( modInfo ));
        FGetModuleFileNameEx( procHandle, modHandleArray[i], imgName,
1024);
        FGetModuleBaseName( procHandle, modHandleArray[i], modName,
1024);

        rv = SymLoadModule( procHandle, modHandleArray[i], imgName,
modName,
                            (DWORD)modInfo.lpBaseOfDll, (DWORD)
modInfo.SizeOfImage );

        if ( !rv )
        {
            rv = GetLastError();
        }
        else
            rv = rv;
    } 
}
