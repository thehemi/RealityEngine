//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
/// Name: ConfigFile
/// Desc: Loads and parses config files
///
/// Author: Tim Johnson
//====================================================================================
#include "stdafx.h"
#include "ConfigFile.h"
#include <fstream>


//-----------------------------------------------------------------------------
/// Create a blank config
//-----------------------------------------------------------------------------
void ConfigFile::Create(string filename){
	Lines.resize(0);
	FileName = filename;
	
	ofstream cfg(FileName.c_str());
	cfg.close();
}

//-----------------------------------------------------------------------------
// Adds a line of text directly, rather than through a key/value pair
//-----------------------------------------------------------------------------
void ConfigFile::InsertRawLine(string line){
	Lines.push_back(line);

	ofstream cfg(FileName.c_str());
	for(int i=0;i<Lines.size()-1;i++){
		cfg << Lines[i] << endl;
	}
	/// Last line must NOT have a newline, or they will build up at the end of the ini
	cfg << Lines[Lines.size()-1];
	cfg.close();
}

//-----------------------------------------------------------------------------
/// Similar to GetValue, but returns true/false
//-----------------------------------------------------------------------------
bool ConfigFile::KeyExists(string Key, string Section){
	ToLowerCase(Key);
	for(int i=0;i<key.size();i++){
		if(Key == key[i] && (Section.length() == 0 || Section == section[i]))
			return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ConfigFile::WriteValue(string Key, string Val){
	/// Make a lowercase copy of the key so our search is not case sensitive
	string lKey = Key;
	ToLowerCase(lKey);
	string s1 = lKey + " ";
	string s2 = lKey + "=";

	int i;
	for(i=0;i<Lines.size();i++){
		string line = Lines[i];
		ToLowerCase(line);

		if(line.find(s1) == 0 || line.find(s2) == 0){
			/// We found the line containing the Key.
			/// Rewrite the line with the new Val
			Lines[i] = Key + " = " + Val;
			/// Also update the key/value pair for our own records
			for(int j=0;j<key.size();j++)
				if(key[j] == lKey){
					value[j] = Val;
				}
			break;
		}
	}

	/// Key doesn't exist, so add it
	if(i == Lines.size())
		Lines.push_back(Key + " = " + Val);
		//Error("ConfigFile::Set failed. Key '%s' was not found in file: '%s'",Key.c_str(),FileName.c_str());

    ResetCurrentDirectory();
	/// Rewrite the config file
	ofstream cfg(FileName.c_str());
	for(int i=0;i<Lines.size()-1;i++){
		cfg << Lines[i] << endl;
	}
	/// Last line must NOT have a newline, or they will build up at the end of the ini
	cfg << Lines[Lines.size()-1];
	cfg.close();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
Vector ConfigFile::GetVector(string Key, string Section){
	string value = GetValue(Key,Section);

	/// strtod puts errror codes in these if something goes wrong, like trying to convert alpha
	char *err1 = 0,*err2 = 0,*err3 = 0;

	/// Format: x y z
	Vector thevector;
	/// GetWord gets the nTh word from the string, words are ascii separated by spaces
	string v1 = GetWord(value,0).c_str();
	string v2 = GetWord(value,1).c_str();
	string v3 = GetWord(value,2).c_str();

	thevector.x = strtod(v1.c_str(),&err1);
	thevector.y = strtod(v2.c_str(),&err2);
	thevector.z = strtod(v3.c_str(),&err3);


	if (err1 == v1.c_str() || err2 == v1.c_str() || err3 == v1.c_str()){ /// Something went wrong!
		Warning("ConfigFile::GetVector: The value for the key '%s' was invalid: '%s'",Key.c_str(),value.c_str());
		thevector.Set(0,0,0);
	}

	return thevector;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ConfigFile::GetSection(string SectionName, vector<string>& Keys, vector<string>& Values){
	for(int i=0;i<section.size();i++){
		if(section[i] == SectionName){
			Keys.push_back(key[i]);
			Values.push_back(value[i]);
		}
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
string ConfigFile::GetValue(string Key, string Section){
	/// Convert input string to lowercase
	ToLowerCase(Key);
	
	for(int i=0;i<key.size();i++){
		if(Key == key[i] && (Section.length() == 0 || Section == section[i]))
			return value[i];
	}

	Error("ConfigFile::Get failed. Key '%s' was not found in file '%s'\nNote: Case is irrelevant.",Key.c_str(),FileName.c_str());
	return "";
}

unsigned long ConfigFile::GetColor(string Key, string Section){
	string value = GetString(Key,Section);
	/// Extract the individual parameters from the string
	/// Format: RGBA R G B A
	if(GetWord(value,0)!="RGBA")
		Error("ConfigFile::GetColor: request for invalid key/value pair: '%s=%s'. Check your config file.\nColor lines should be in correct format. Example: color = RGBA 100 200 100 155",Key.c_str(),value.c_str());
	
	return COLOR_RGBA(atoi(GetWord(value,1).c_str()),atoi(GetWord(value,2).c_str()),atoi(GetWord(value,3).c_str()),atoi(GetWord(value,4).c_str()));
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool ConfigFile::GetBool(string Key, string Section){
	string value = GetValue(Key,Section);
	if(value == "true"||value=="1")
		return true;
	if(value == "false"||value=="0")
		return false;

	/// Something went wrong!
	Error("ConfigFile::GetBool: The value for the key '%s' was invalid: '%s'",Key.c_str(),value.c_str());
	return false;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
int ConfigFile::GetInt(string Key, string Section){
	string value = GetValue(Key,Section);

	/// Convert string to number with error checking
	char* s;
	errno = 0;
	int number = strtol(value.c_str(),&s,10);
	if(s == value.c_str()) { /// Something went wrong!
		Error("ConfigFile::GetInt: The value for the key '%s' was invalid: '%s'",Key.c_str(),value.c_str());
	}

	return number;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
float ConfigFile::GetFloat(string Key, string Section){
	string value = GetValue(Key,Section);

	/// Convert string to number with error checking
	char* s;
	errno = 0;
	float number = strtod(value.c_str(),&s);
	if(s == value.c_str()) { /// Something went wrong!
		Error("ConfigFile::GetInt: The value for the key '%s' was invalid: '%s'",Key.c_str(),value.c_str());
	}

	return number;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
string ConfigFile::GetString(string Key, string Section){
	string value = GetValue(Key,Section);
	//if(value=="")
	//	Error("ConfigFile::GetString: The value for the key '%s' was invalid: '%s'",Key.c_str(),value.c_str());
	return value;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ConfigFile::SetString(string Key, string Val){
	WriteValue(Key,Val);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ConfigFile::SetBool(string Key, bool Val){
	WriteValue(Key,(Val?"true":"false"));
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ConfigFile::SetFloat(string Key, float Val){
   char ch[20];
   sprintf(ch, "%.2f", Val);
   WriteValue(Key,ch);
} 

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ConfigFile::SetInt(string Key, int Val){
   char ch[64];
   sprintf(ch, "%d", Val);
   WriteValue(Key,ch);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ConfigFile::SetColor(string Key, ULONG color){
	char ch[64];
	sprintf(ch, "RGBA %d %d %d %d", COLOR_GETRED(color),COLOR_GETGREEN(color),COLOR_GETBLUE(color),COLOR_GETALPHA(color));
	WriteValue(Key,ch);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool ConfigFile::Load(string filename){
	Lines.clear();
	key.clear();
	value.clear();
	section.clear();
	//ResolvePath(filename);
	ifstream config(filename.c_str());

	if(!config){
		Warning("Couldn't find config file: %s in current directory: %s",(char*)filename.c_str(),GetDir());
		return false;
	}

	FileName = filename;

	string curSection;
	/// Loop until we reach end of file
	while(!config.eof())
    {
		/// Read line
		string line;
		char str[512];
		config.getline(str,512);
		line = str;

		Lines.push_back(line);

		/// Remove comments
		if(line.find("//")!=-1)
			line = line.substr(0,line.find("//"));

		if(line.size() == 0)
			continue;

		/// Check for a section tag
		if(line.find("=") == -1){
			int sb = line.find("[");
			int cb = line.find("{");

			/// Extract [Tag]
			if(sb != -1){
				line = line.substr(sb+1);
				line = line.substr(0,line.find("]"));
				curSection = line;
			}
			/// Extract class Tag {
			if(cb != -1){
				line = line.substr(0,cb);
				trimRight(line);
				if(line.find(" ") != -1)
					line = line.substr(line.find(" ")+1);
				if(line.find("\t") != -1)
					line = line.substr(line.find("\t")+1);
				curSection = line;
			}
		}
		
		/// If this line has a var/value pair, save it
		if(line.find("=")!=-1){
			string skey, svalue;
			/// Get the key name
			skey = line.substr(0,line.find("="));
			/// remove whitespace on ends
			trimRight(skey); 
			trimLeft(skey); 

			/// Convert to lowercase
			for(int i = 0; i < skey.length(); i++)
				skey[i] = tolower(skey.c_str()[i]);

			/// Get the value
			svalue = line.substr(line.find("=")+1,line.length());
			/// remove whitespace on ends
			trimRight(svalue); 
			trimLeft(svalue); 

			/// Remove any "quotes"
			if(svalue.length())
			{
				if(svalue[0] == '\"')
					svalue = svalue.substr(1);
				if(svalue[svalue.length()-1] == '\"')
					svalue = svalue.substr(0,svalue.length()-1);
			}

			key.push_back(skey);
			value.push_back(svalue);
			section.push_back(curSection);
		}
	}

	config.close();
	return true;
}
