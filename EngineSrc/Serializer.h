//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
/// Type: Serializer
/// Info: Handles loading/saving, will check all registered serializers
/// 'Serializers' are loading/saving modules derived from ILoad/ISave
///
/// Author: Tim Johnson
//====================================================================================
#pragma once

//-----------------------------------------------------------------------------
/// Abstract loading class for serializers
//-----------------------------------------------------------------------------
class ENGINE_API ILoad
{
public:
	virtual bool LoadWorld(string name, World* world, bool IsSky){ SeriousWarning("Serializer does not support world loading"); return false; }
	virtual bool LoadModel(string name, StaticModel* Model){ SeriousWarning("Serializer does not support model loading"); return false; }
};


//-----------------------------------------------------------------------------
/// Abstract loading class for serializers
//-----------------------------------------------------------------------------
class ENGINE_API ISave
{
public:
	virtual bool SaveWorld(string name, World* world){ SeriousWarning("Serializer does not support world saving"); return false; }
	virtual bool SaveModel(string name, StaticModel* Model){ SeriousWarning("Serializer does not support model saving"); return false; }
	virtual bool Save(string name, vector<Actor*>& items){ return false; }
};

//-----------------------------------------------------------------------------
/// Main Serializer
//-----------------------------------------------------------------------------
class Serializer 
{
private:
	ILoad*	GetLoader(string filename);
	ISave*	GetSaver(string filename);

public:
	static Serializer* Serializer::Instance ()
	{
		static Serializer inst;
		return &inst;
	}

	bool LoadWorld(string name, World* world, bool IsSky);
	bool LoadModel(string name, StaticModel* Model);

	bool SaveWorld(string name, World* world, bool IsSky);
	bool SaveModel(string name, StaticModel* Model);
	bool Save(string name, vector<Actor*>& items);
};

