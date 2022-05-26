/*********************************************************************NVMH4****
Path:  SDK\LIBS\inc\shared
File:  NV_StringFuncs.h

Copyright NVIDIA Corporation 2002
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED
*AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS
BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES
WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS,
BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS)
ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS
BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.



Comments:
A collection of useful string functions, built upon std::string

They are not all meant to be fast.  Many return vectors of strings, which will
require string copies and lots of work by the STL functions.  For processing
large strings or a lot of string data you will probably want to devise your
own routines.

A demo of using the functions is provided in:
SDK\DEMOS\common\src\NV_StringFuncs


******************************************************************************/


#ifndef _NV_STRINGFUNCS_GJ_H
#define _NV_STRINGFUNCS_GJ_H


#pragma warning(disable : 4786)
#include <string>
#include <vector>
#include <list>
#include <stdlib.h>


// sfsizet is size type for the string class used
typedef std::string::size_type sfsizet;



/////////////////////////////////////////////////////////////////////
// macro to expand vector of anything into single string
// example c is "%u " and the %<type> must match type held in vector
// Example:
// string dest;
// vector< int > vint;
// VEC_TO_STR( dest, vint, "%d " );
//
#ifndef VEC_TO_STR
#define VEC_TO_STR( a, b, c )														\
{																					\
	for( int vectocnt = 0; vectocnt < b.size(); vectocnt++ )									\
	{ a += StrPrintf( c, b.at(vectocnt) );													\
	}																				\
}
#endif

//////////////////////////
// Convert list of some type to vector of same type
// L is list, V is vector, T is type
// Example:
// list< string >  lStr;
// vector< string > vStr;
// lStr.push_back( "a" );
// lStr.push_back( "b" );
// LIST_TO_VEC( lStr, vStr, std::string );

#ifndef LIST_TO_VEC
#define LIST_TO_VEC( L, V, T )															\
{																						\
	std::list< T >::const_iterator ltovecmacroiter;										\
	for( ltovecmacroiter = L.begin(); ltovecmacroiter != L.end(); ltovecmacroiter++ )	\
	{																					\
		V.push_back( *ltovecmacroiter );												\
	}																					\
}
#endif

//////////////////////////
// Convert vector of some type to list of same type
// L is list, V is vector, T is type
//
#ifndef VEC_TO_LIST
#define VEC_TO_LIST( V, L, T )									\
{																\
	for( int vtolistc=0; vtolistc < V.size(); vtolistc++ )		\
	{	L.push_back( V.at(vtolistc) );							\
	}															\
}
#endif

/////////////////////////////////////////////////////////////////////


	// Create a string using va_args, just like printf, sprintf, etc.
std::string StrPrintf( const char * szFormat, ...  );


std::string StrToUpper( const std::string & strIn );
std::string StrToLower( const std::string & strIn );

	// Writes each string in the vector to a single output
	//  string, separating each by a space.
std::string StrsToString( const std::vector< std::string > & vstrIn );
	// same, but lets you specify the separator between strings
std::string StrsToString( const std::vector< std::string > & vstrIn,
						  const std::string & separator );
std::string StrsToString( const std::list< std::string > & lstrIn,
						  const std::string & separator );


	// Like CString::Replace()
	// Sequence and it's replacement can be different lengths or empty
std::string StrReplace( const std::string & sequence_to_replace, 
						const std::string & replacement_string,
						const std::string & strIn );
	// same as above, only it modifies pOut and also returns pOut
std::string * StrReplace( const std::string & sequence_to_replace, 
							const std::string & replacement_string,
							const std::string & strIn,
							std::string * pOut );


	// applies strtok repeatedly & acumulates ouput tokens 
	//  in output string separated by spaces
std::string StrTokenize( const std::string & strIn, const std::string & delimiters );

	// Same as above, but allows you to specify the separator between 
	//  tokens in the output
std::string StrTokenize( const std::string & strIn,
						 const std::string & delimiters,
						 const std::string & output_separator );

sfsizet StrFindCaseless( const std::string & strSearchIn, sfsizet start_pos, const std::string & strSearchFor );

std::vector< sfsizet > StrFindAllCaseless( const std::string & strSearchIn,
											sfsizet start_pos,
											const std::string & strSearchFor );

std::vector< sfsizet > StrFindAll( const std::string & strSearchIn,
											sfsizet start_pos,
											const std::string & strSearchFor );

std::vector< std::string> StrSort( const std::vector< std::string > & vStrIn );


/////////////////////////////////////////////////////////////////////

/*
//@@@@@
SortCaseless
Sort		// case sensitive


  _stricmp - compare two strings without regard to case
  _strlwr = convert string to lowercase
  _strupr
  _strrev = reverse string
  strtok = find next token in string



*/

#endif 
