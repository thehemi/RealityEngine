//=========== Copyright (c) 2003, Tim Johnson. All rights reserved. ===========
//
//=============================================================================
#include "stdafx.h"
#include <cstdlib>
#include <io.h>

/// Helper functions
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

//-----------------------------------------------------------------------------
// Finds a resource file at a specified location (folder)
//-----------------------------------------------------------------------------
bool ResetCurrentDirectory()
{
	CHAR filename[512];
	GetModuleFileName( GetModuleHandle(NULL), filename, 512);
	string path = string(filename);
	path = path.substr(0,path.find_last_of("\\")+1);
	SetCurrentDirectory(path.c_str());
	// Add system dir only if needed
	if(AsLower(path).find("system") == -1 || !(AsLower(path).find("system") > path.length()-9))
		SetCurrentDirectory(".\\System\\");
	return true;
}

//-----------------------------------------------------------------------------
//
float clamp(float orig,float low,float high)
{
	return orig < low ? low : (orig > high ? high : orig);
}

//-----------------------------------------------------------------------------
/// Generates unique name, incrementing number
//-----------------------------------------------------------------------------
string GenerateNewName(string name)
{
	int offset = -1;
	for(int i=name.length()-1;i>=0;i--)
	{
		if(isdigit(name[i]))
			offset = i;
		else
			break;
	}

	if(offset == -1)
		return name + "01";

	int number = strtol(name.substr(offset).c_str(),0,10);
	number++;
	return name.substr(0,offset) + ToStr(number);
}


bool CompareIgnoreCase(const string& s1, const string& s2){
	if(s1.length() != s2.length())
		return false;
	for(int i=0;i<s1.length();i++){
		if(tolower(s1[i]) != tolower(s2[i]))
			return false;
	}
	return true;
}

string ToAnsi(const wstring &s) {
	static char pAnsiString[1024];
	wcstombs(pAnsiString, s.c_str(), s.length()+1);
	return string(pAnsiString);
}

wstring ToUnicode(const string &s) {
	static wchar_t pAnsiString[1024];
	mbstowcs(pAnsiString, s.c_str(), s.length()+1);
	return wstring(pAnsiString);
}

string StripExtension(const string& string){
	return string.substr(0,string.find_last_of(_U(".")));
}

/// If this starts spitting out angry words at you, remember to inspect ._Bx as well
string StripPath(const string& fileName){
	string ret = fileName;
	if(fileName.find_last_of(_U("\\"))!=-1){
		int i = fileName.find_last_of(_U("\\"));
		ret = fileName.substr(i+1);
	}
	else if(fileName.find_last_of(_U("/"))!=-1){
		int i = fileName.find_last_of(_U("\\"));
		ret = fileName.substr(i+1);
	}
	
	return ret;
}

int FindLastSlash(const string& str){
	int a = str.find_last_of(_U("/"));
	int b = str.find_last_of(_U("\\"));

	return max(a,b);
}

void trimLeft(string &value)
{
	string::size_type where = value.find_first_not_of(' ');
	if (where == string::npos)
		/// string has nothing but space
		value = _U("");
	else if (where != 0)
		value = value.substr(where);


	where = value.find_first_not_of('\t');
	if (where == string::npos)
		/// string has nothing but space
		value = _U("");
	else if (where != 0)
		value = value.substr(where);
}


void trimRight(string &value)
{ 
	string::size_type where = value.find_last_not_of(' ');
	if (where == string::npos)
		/// string has nothing but space
		value = _U("");
	else if (where != (value.length() - 1))
		value = value.substr(0, where + 1);

	where = value.find_last_not_of('\t');
	if (where == string::npos)
		/// string has nothing but space
		value = _U("");
	else if (where != (value.length() - 1))
		value = value.substr(0, where + 1);
}


void remove_whitespace (CHAR *the_string) {
  /// Removes all spaces from a string
  /// Does it by using the src_ptr to look at each character in the string
  /// If the character is a space, skip it,
  /// If the character is not a space, move it at the dst_ptr location.
  /// This does not preserve the original string, and it does not reallocate
  /// the unused space.
  CHAR *src_ptr = the_string;
  CHAR *dst_ptr = the_string;
  while ((*src_ptr) != 0) {
    if ((*src_ptr) != ' ' && (*src_ptr) != '\t') {
      if (src_ptr != dst_ptr) { (*dst_ptr) = (*src_ptr); }
      dst_ptr++;
    } src_ptr++;
  } *dst_ptr = 0;
}


void ToLowerCase(string& s){
	for(int i = 0; i < s.length(); i++)
		s[i] = tolower(s.c_str()[i]);
}

string AsLower(const string& s){
	string lower = s;
	for(int i = 0; i < lower.length(); i++)
		lower[i] = tolower(lower.c_str()[i]);
	return lower;
}

string AsUpper(string s){
	string lower = s;
	for(int i = 0; i < lower.length(); i++)
		lower[i] = toupper(lower.c_str()[i]);
	return lower;
}
/// returns nth word
string GetWord(string s, int index){
	index++; /// we use 1-based, user will probably use 0-based
	string temp = s;
	string word;
	for(int i=0;i<index;i++){
		if(temp.find(_U(" "))!=-1){
			word = temp.substr(0,temp.find(_U(" ")));
			temp = temp.substr(temp.find(_U(" "))+1,temp.length());
		}
		else 
			word = temp.substr(0,temp.length());
	}
	return word;
}


void findandreplace( string& source, const string& find, const string& replace )
{
	size_t j;

	for (;(j = source.find( find )) != string::npos;)
	{
		source.replace( j, find.length(), replace );

	}
}



unsigned int countSubStrings(CHAR *data, CHAR *find)
{
 unsigned int count = 0;
 CHAR *p = data;
 while((p = strstr (p,find)) != NULL) {
  count++;
  p++;
 } return count;
}


void strlower( CHAR * s )
{
     while ( (*s = tolower( *s ) ) )
           ++s ;
}


/// depth = number of recursions/subdirs
/// If this is too high, we might end up recursing massive directory structures
void enumerateDirectories( const CHAR * path, vector<string>& dirList, int depth/* = 2 */)
{
   HANDLE handle;
   WIN32_FIND_DATA findData;
   depth --;

   /// Make sure newpath ends with a slash
   string newPath = path;
   if(newPath[newPath.length()-1] != '\\' && newPath[newPath.length()-1] != '/')
		newPath += _U("\\");

   string searchPath = newPath + _U("*.*");
   handle = FindFirstFile( searchPath.c_str(), &findData );

   do
   {
      if ( findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
      {
         if ( findData.cFileName[0] != '.' && depth )
         {
			string dirPath = newPath + string(findData.cFileName) + _U("\\");
			dirList.push_back(dirPath);
			enumerateDirectories( dirPath.c_str(), dirList, depth );
         }
      }
   }
   while( handle && FindNextFile( handle, &findData ) );
   return;
} 



void enumerateFiles( const CHAR * path, vector<string>& fileList, int depth/* = 2 */, const CHAR* ext)
{
   HANDLE handle = NULL;
   WIN32_FIND_DATA findData;

   depth --;

    /// Make sure newpath ends with a slash
   string newPath = path;
   if(newPath.find(_U("\\")) != _tcslen(path)-1 || 
	   newPath.find(_U("/")) != _tcslen(path)-1)
		newPath += _U("\\");

   string e = ext;
   if(e[0] == '*')
	   e = e.substr(1);

   string searchPath = newPath + string(_U("*")) + e;

   findData.dwFileAttributes = 0;
   handle = FindFirstFile( searchPath.c_str(), &findData );

   do {
      if ( findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
      {
         if ( findData.cFileName[0] != '.' && depth )
         {
			string dirPath = newPath + string(findData.cFileName) + _U("\\");
			enumerateFiles( dirPath.c_str(), fileList, depth );
         }
      }
	  else if(findData.dwFileAttributes != NULL){
		  /// file
		  fileList.push_back(findData.cFileName);
	  }
   }
   while( handle && FindNextFile( handle, &findData ) );

   return;
} 

CHAR* GetDir(){
	static CHAR temp[2048];
	GetCurrentDirectory(2048,temp);
	return temp;
}




