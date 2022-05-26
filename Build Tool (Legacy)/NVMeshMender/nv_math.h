/*********************************************************************NVMH1****
File:
nv_math.h

Copyright (C) 1999, 2002 NVIDIA Corporation
This file is provided without support, instruction, or implied warranty of any
kind.  NVIDIA makes no guarantee of its fitness for a particular purpose and is
not liable under any circumstances for any damages or loss whatsoever arising
from the use or inability to use this file or items derived from it.

Comments:


******************************************************************************/
#ifndef _nv_math_h_
#define _nv_math_h_

#ifndef _nv_mathdecl_h_
#include "nv_mathdecl.h"
#endif // _nv_mathdecl_h_

#ifdef WIN32

#ifndef NV_MATH_PROJECT // defined if we are building nv_math.lib

#ifndef NV_MATH_EXPORTS // defined if we are building nv_math.lib for dlls

#if _MSC_VER >= 1300

#ifdef _DLL
    #pragma message("NV:Using (MULTITHREADED DLL):\n nv_math_mtdll_vc7.lib\n")
    #pragma comment(lib,"nv_math_mtdll_vc7.lib")
#else
    #ifdef _MT
        #pragma message("NV:Using (MULTITHREADED):\n nv_math_mt_vc7.lib\n")
        #pragma comment(lib,"nv_math_mt_vc7.lib")
    #else
        #pragma message("NV:Using (SINGLE THREADED):\n nv_math_vc7.lib\n")
        #pragma comment(lib,"nv_math_vc7.lib")
    #endif
#endif
#else
#ifdef _DLL
    #pragma message("NV:Using (MULTITHREADED DLL):\n nv_math_mtdll.lib\n")
    #pragma comment(lib,"nv_math_mtdll.lib")
#else
    #ifdef _MT
        #pragma message("NV:Using (MULTITHREADED):\n nv_math_mt.lib\n")
        #pragma comment(lib,"nv_math_mt.lib")
    #else
        #pragma message("NV:Using (SINGLE THREADED):\n nv_math.lib\n")
        #pragma comment(lib,"nv_math.lib")
    #endif
#endif
#endif
#endif // NV_MATH_EXPORTS
#endif // NV_MATH_PROJECT
#endif // WIN32


#include <assert.h>
#include <math.h>

#ifdef _WIN32
#include <limits>
#else
#include <limits.h>
#endif

#ifdef MACOS
#define sqrtf sqrt
#define sinf sin
#define cosf cos
#define tanf tan
#endif

#include <memory.h>
#include <stdlib.h>
#include <float.h>

typedef float nv_scalar;

#define nv_zero			      nv_scalar(0)
#define nv_zero_5             nv_scalar(0.5)
#define nv_one			      nv_scalar(1.0)
#define nv_two			      nv_scalar(2)
#define nv_half_pi            nv_scalar(3.14159265358979323846264338327950288419716939937510582 * 0.5)
#define nv_quarter_pi         nv_scalar(3.14159265358979323846264338327950288419716939937510582 * 0.25)
#define nv_pi			      nv_scalar(3.14159265358979323846264338327950288419716939937510582)
#define nv_two_pi			  nv_scalar(3.14159265358979323846264338327950288419716939937510582 * 2.0)
#define nv_oo_pi			  nv_one / nv_pi
#define nv_oo_two_pi	      nv_one / nv_two_pi
#define nv_oo_255   	      nv_one / nv_scalar(255)
#define nv_oo_128   	      nv_one / nv_scalar(128)
#define nv_to_rad             nv_pi / nv_scalar(180)
#define nv_to_deg             nv_scalar(180) / nv_pi
#define nv_eps		          nv_scalar(10e-6)
#define nv_double_eps	      nv_scalar(10e-6) * nv_two
#define nv_big_eps            nv_scalar(10e-6)
#define nv_small_eps          nv_scalar(10e-2)

struct vec2;
struct vec2t;
struct vec3;
struct vec3t;
struct vec4;
struct vec4t;

#ifndef _nv_algebra_h_
#include "nv_algebra.h"
#endif // _nv_algebra_h_

#endif //_nv_math_h_
