#ifndef LEAN_WINDOWS_H
#define LEAN_WINDOWS_H
#pragma once

#define NOMINMAX
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#define NOSERVICE
#define NOMCX
#define NOIME
#define NOSOUND
#define NOCOMM
#define NOKANJI
#define NORPC
#define NOPROXYSTUB
#define NOIMAGE
#define NOTAPE
/// ... Plus any others I don't need

#ifndef NO_ANSIUNI_ONLY
#	ifdef _UNICODE
#		define UNICODE_ONLY
#	else
#		define ANSI_ONLY
#	endif
#endif

#include <windows.h>

#endif
