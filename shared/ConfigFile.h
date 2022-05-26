//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
/// Name: ConfigFile
/// \brief Loads and parses config files
///
/// Author: Tim Johnson
//====================================================================================
#ifndef CONFIG_INCLUDED
#define CONFIG_INCLUDED

class ConfigFile {
private:
	/// Key Value pairs and manipulators
	vector<string> key, value, section;
	string GetValue(string keyname, string Section);
	void WriteValue(string Key, string Val);

	/// Actual lines. Used for rewriting config
	vector<string> Lines;

	string FileName;

public:
	/// Load a config file into memory. Does not keep file handle open
	bool Load(string filename);
	/// Create a new blank config file
	void Create(string filename);
	/// Adds a line of text directly, rather than through a key/value pair
	void InsertRawLine(string text);

	bool KeyExists(string Key, string Section="");

	/// Misc
	void GetSection(string SectionName, vector<string>& Keys, vector<string>& Values);

	/// Get functions
	bool		GetBool(string Key, string Section="");
	int			GetInt(string Key, string Section="");
	string		GetString(string Key, string Section="");
	float		GetFloat(string Key, string Section="");
	unsigned long GetColor(string Key, string Section="");
	Vector GetVector(string Key, string Section="");

	/// Set functions
	void		SetBool(string Key, bool Val);
	void		SetInt(string Key, int Val);
	void		SetString(string Key, string Val);
	void		SetFloat(string Key, float Val);
	void		SetColor(string Key, unsigned long color);
};


#endif