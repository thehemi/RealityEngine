
#include "stdafx.h"
#include <tchar.h>

#include "mdump.h"

LPCSTR MiniDumper::m_szAppName;
bool MiniDumper::ExceptionInProgress = false;

bool InitSymEng ( HANDLE procHandle );

MiniDumper::MiniDumper( LPCSTR szAppName )
{
	// if this assert fires then you have two instances of MiniDumper
	// which is not allowed
	assert( m_szAppName==NULL );

	m_szAppName = szAppName ? strdup(szAppName) : "Application";

	::SetUnhandledExceptionFilter( TopLevelFilter );
}

LONG MiniDumper::TopLevelFilter( struct _EXCEPTION_POINTERS *pExceptionInfo )
{
	LONG retval = EXCEPTION_CONTINUE_SEARCH;
	HWND hParent = NULL;						// find a better value for your app

	ExceptionInProgress = true;

	// firstly see if dbghelp.dll is around and has the function we need
	// look next to the EXE first, as the one in System32 might be old 
	// (e.g. Windows 2000)
	HMODULE hDll = NULL;
	char szDbgHelpPath[_MAX_PATH];

	if (GetModuleFileNameA( NULL, szDbgHelpPath, _MAX_PATH ))
	{
		char *pSlash = _tcsrchr( szDbgHelpPath, '\\' );
		if (pSlash)
		{
			_tcscpy( pSlash+1, "DBGHELP.DLL" );
			hDll = ::LoadLibrary( szDbgHelpPath );
		}
	}

	if (hDll==NULL)
	{
		// load any version we can
		hDll = ::LoadLibrary( "DBGHELP.DLL" );
	}

    InitSymEng(0);

	LPCTSTR szResult = NULL;

	if (hDll)
	{
		MINIDUMPWRITEDUMP pDump = (MINIDUMPWRITEDUMP)::GetProcAddress( hDll, "MiniDumpWriteDump" );
		if (pDump)
		{
			char szDumpPath[_MAX_PATH];
			char szScratch [_MAX_PATH];

			// work out a good place for the dump file
			//if (!GetTempPath( _MAX_PATH, szDumpPath ))
			//	_tcscpy( szDumpPath, "c:\\temp\\" );

			_tcscpy(szDumpPath,GetDir()); // Save to working dir

			strcat( szDumpPath,"\\" );
			strcat( szDumpPath, m_szAppName );
			strcat( szDumpPath, ".dmp" );

			// ask the user if they want to save a dump file
			if (true)//::MessageBox( NULL, "Something bad happened to this program, would you like to save a diagnostic file?", m_szAppName, MB_YESNO )==IDYES)
			{
				// create the file
				HANDLE hFile = ::CreateFile( szDumpPath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
											FILE_ATTRIBUTE_NORMAL, NULL );

				if (hFile!=INVALID_HANDLE_VALUE)
				{
					_MINIDUMP_EXCEPTION_INFORMATION ExInfo;

					ExInfo.ThreadId = ::GetCurrentThreadId();
					ExInfo.ExceptionPointers = pExceptionInfo;
					ExInfo.ClientPointers = NULL;

					// write the dump
                    BOOL bOK;
                    if(pExceptionInfo)
					    bOK = pDump( GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ExInfo, NULL, NULL );
                    else
                        bOK = pDump( GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, NULL, NULL, NULL );
					if (bOK)
					{
						sprintf( szScratch, "An unexpected error occured. Saved diagnostic file to '%s'.", szDumpPath );
						szResult = szScratch;
						retval = EXCEPTION_EXECUTE_HANDLER;
					}
					else
					{
						sprintf( szScratch, "Failed to save dump file to '%s' (error %d)", szDumpPath, GetLastError() );
						szResult = szScratch;
					}
					::CloseHandle(hFile);
				}
				else
				{
					sprintf( szScratch, "Failed to create dump file '%s' (error %d)", szDumpPath, GetLastError() );
					szResult = szScratch;
				}
			}
		}
		else
		{
			szResult = "DBGHELP.DLL too old. Make sure DBGHELP.DLL is in your game folder.";
		}
	}
	else
	{
		szResult = "DBGHELP.DLL not found. Make sure DBGHELP.DLL is in your game folder.";
	}


	string error;
	if(szResult)
		error += szResult;

	char pc[4000];
    if(pExceptionInfo)
	    PrintStackWalk( pc, 4000, pExceptionInfo);

	error += "\nStack Trace:\n";
	error += pc;
	

	Error(error.c_str());

	return retval;
}

