// Get the types, such as DWORD, UINT_PTR, etc.
// Can replace this with our own typedefs for eventual cross platform use...
#include "windows.h"

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(a) do { if ((a)) { (a)->Release(); (a) = NULL; } } while(0)
#endif

#ifndef SAFE_ADDREF
#define SAFE_ADDREF(a) do { if ((a)) { (a)->AddRef(); } } while(0)
#endif

#ifndef INTCALLTYPE
#define INTCALLTYPE __stdcall
#endif 
