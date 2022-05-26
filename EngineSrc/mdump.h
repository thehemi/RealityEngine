
#if _MSC_VER < 1300
#define DECLSPEC_DEPRECATED
/// VC6: change this path to your Platform SDK headers
#include "M:\\dev7\\vs\\devtools\\common\\win32sdk\\include\\dbghelp.h"			/// must be XP version of file
#else
/// VC7: ships with updated headers
#include "dbghelp.h"
#endif

/// based on dbghelp.h
typedef BOOL (WINAPI *MINIDUMPWRITEDUMP)(HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType,
									CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
									CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
									CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam
									);

/// Creates dmp output file containing stack trace in the event of application crash
class MiniDumper
{
private:
	static LPCSTR m_szAppName;
public:
	static LONG WINAPI TopLevelFilter( struct _EXCEPTION_POINTERS *pExceptionInfo );
	


	static bool ExceptionInProgress;
	MiniDumper( LPCSTR szAppName );
};

