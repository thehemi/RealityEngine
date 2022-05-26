//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
/// Actor: The base class of all actors (Abstract)
///
///
/// Commonly shared files and methods
/// This should be part of your project's precompiled header includes
///
/// Author: Tim Johnson
//====================================================================================
#undef _UNICODE
#undef UNICODE

#ifndef SHARED_INCLUDED
#define SHARED_INCLUDED
#pragma message("Compiling Shared.h - this is PCH.\n") 
#pragma warning(disable: 4786) /// STL 255 trunctuation
#pragma warning(disable: 4800) /// Forcing int to bool
#pragma warning(disable: 4018) /// Signed/Unsigned mismatch 
#pragma warning(disable: 4244) /// Double to float
#pragma warning(disable: 4267) /// size_t to int
#pragma warning(disable: 4251) /// class 'XXX' needs to have dll-interface to be used by its clients
#pragma warning(disable: 4311) /// Pointer truncation from 'LPCSTR' to 'DWORD'
#pragma warning(disable: 4312) /// conversion from 'DWORD' to 'LPCSTR' of greater size
#pragma warning(disable: 4251) /// needs to have dll-interface to be used by clients of class
#pragma warning(disable: 4305) /// Double -> F32

#include <assert.h>
#include <tchar.h>
#include "Vector.h"
#include "Matrix.h"
#include "BBox.h"
#include <string>
#include <sstream>
#include <vector>
#include <list>
#include <algorithm>
#include "LEAN_WINDOWS.H"
#include "shlwapi.h"
typedef std::basic_string<CHAR> tstring;
using namespace std;

#include "ConfigFile.h"

#define _U 

#define DXASSERT(x) {HRESULT hr;if(FAILED(hr=(x))){ Error(" Error: %s, in: "###x,DXGetErrorString9(hr));}}

/// Delete contents of vector. Uses resize(0) so memory is not de-allocated/re-allocated
#define SAFE_DELETE_VECTOR(p) { for(int i=0;i<p.size();i++) { if(p[i]) delete p[i]; } p.resize(0); }
#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }

//#ifdef _DEBUG
//#define SAFE_RELEASE(p)      { if(p) { assert((p)->Release() == 0); (p)=NULL; } }
//#else
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }
#define SAFE_RELEASE_VECTOR(p) { for(int i=0;i<p.size();i++) SAFE_RELEASE(p[i]); p.clear(); }
//#endif

//-----------------------------------------------------------------------------
/// Vector
//-----------------------------------------------------------------------------
/// Erase an item from a vector. Can we make this a function?
#define vector_erase(vec,item)  \
{ for(int itera=0;itera<vec.size();itera++){ if(item == vec[itera]){ vec.erase(vec.begin() + itera); break; } } }

//-----------------------------------------------------------------------------
/// Maths
//-----------------------------------------------------------------------------
#define DEG2RAD(x) ((x)*(PI/180.))
#define RAD2DEG(x) ((x)*(180./PI))
#define RANDF() ((float)rand()/RAND_MAX)
#define crandom() (2.0 * (RANDF() - 0.5))
float clamp(float orig,float low,float high);


//-----------------------------------------------------------------------------
/// Unicode helpers
//-----------------------------------------------------------------------------
string  ToAnsi(const wstring &s);
wstring ToUnicode(const string &s);


//-----------------------------------------------------------------------------
/// Useful string functions
//-----------------------------------------------------------------------------
//
bool ResetCurrentDirectory();
bool CompareIgnoreCase(const string& s1, const string& s2);
string StripExtension(const string& str);
string StripPath(const string& filename);
int		FindLastSlash(const string& str);
void	trimLeft(string &value);
void	trimRight(string &value);
void	remove_whitespace (CHAR *the_string);
void	ToLowerCase(string& s);
string AsLower(const string& s);
string AsUpper(string s);
string GetWord(string s, int index);
void findandreplace( string& source, const string& find, const string& replace );
template <typename T> string ToStrW(T a_T)
{  /// Convert any type to a string
    std::ostringstream buffer;
    buffer << a_T;
    return buffer.str();
}

template <typename T> string ToStr(T a_T)
{  /// Convert any type to a string
    std::ostringstream buffer;
    buffer << a_T;
    return buffer.str();
}

/// Generates unique mesh name, incrementing number, used for cloning
string GenerateNewName(string name);


/// Directory
inline BOOL FileExists( const string pszPath ){return ::PathFileExistsA( pszPath.c_str() );}
void		enumerateDirectories( const CHAR * path, vector<string>& dirList, int depth = 2);
void		enumerateFiles( const CHAR * path, vector<string>& fileList, int depth = 2, const CHAR* ext = _U(".*"));
CHAR*		GetDir();

/// Color
typedef unsigned long COLOR;
#define COLOR_GETALPHA(rgb)      ((rgb) >> 24)
#define COLOR_GETRED(rgb)        (((rgb) >> 16) & 0xff)
#define COLOR_GETGREEN(rgb)      (((rgb) >> 8) & 0xff)
#define COLOR_GETBLUE(rgb)       ((rgb) & 0xff)
#define COLOR_RGBA(r, g, b, a)   ((COLOR) ((int)((a) << 24) | ((int)(r) << 16) | ((int)(g) << 8) | (int)(b)))
#define COLOR_ARGB(a, r, g, b)   ((COLOR) (((int)(a) << 24) | ((int)(r) << 16) | ((int)(g) << 8) | (int)(b)))
#define COLOR_MAKE(r, g, b, a)   ((COLOR) (((int)(a) << 24) | ((int)(r) << 16) | ((int)(g) << 8) | (int)(b)))

/// Floating point RGBA color structure
struct FloatColor {
	float r,g,b,a;
	FloatColor(Vector& v): r(v.x),g(v.y),b(v.z),a(1){}
	FloatColor(float R, float G, float B, float A): r(R),g(G),b(B),a(A){}
	FloatColor(float R, float G, float B): r(R),g(G),b(B),a(1){}
	FloatColor(): r(0),g(0),b(0),a(0){}

	FloatColor operator * (const float n) const{
		return FloatColor(r*n,g*n,b*n,a*n);
	}

	FloatColor operator * (const FloatColor& n) const{
		return FloatColor(r*n.r,g*n.g,b*n.b,a*n.a);
	}

	FloatColor operator - (const FloatColor& n) const{
		return FloatColor(r-n.r,g-n.g,b-n.b,a-n.a);
	}

	FloatColor operator + (const FloatColor& n) const{
		return FloatColor(r+n.r,g+n.g,b+n.b,a+n.a);
	}

	bool operator!=(FloatColor& rhs){
		return rhs.a != a || rhs.r != r || rhs.g != g || rhs.b != b;
	}

	DWORD DWORDColor(){
		return COLOR_ARGB((int)(a*255.f),(int)(r*255.f),(int)(g*255.f),(int)(b*255.f));
	}

	void Clamp(){
		r = clamp(r,0.0f,1.0f);
		g = clamp(g,0.0f,1.0f);
		b = clamp(b,0.0f,1.0f);
		a = clamp(a,0.0f,1.0f);
	}
};

#define ASFLOATCOLOR(col) \
	FloatColor(COLOR_GETRED(col)*.0039215f,COLOR_GETGREEN(col)*.0039215f,COLOR_GETBLUE(col)*.0039215f,COLOR_GETALPHA(col)*.0039215f)

#define MAX(a,b)            (((a) > (b)) ? (a) : (b))
#define MIN(a,b)            (((a) < (b)) ? (a) : (b))

/// Forward-delcarations for common types
typedef struct IDirectSound    *LPDIRECTSOUND;
typedef struct IDirectSoundBuffer    *LPDIRECTSOUNDBUFFER;
typedef struct IDirect3DTexture9* LPDIRECT3DTEXTURE9;


#endif