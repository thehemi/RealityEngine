//======= (C) Copyright 2004, Artificial Studios. All rights reserved. ======
/// Type: DirectX X-File Loading Module
/// Info: Loads Maya and Max X-Files
//
//=============================================================================
#include "..\EngineSrc\Serializer.h"

//-----------------------------------------------------------------------------
/// Loads .X files
//-----------------------------------------------------------------------------
class ENGINE_API XFileLoad : public ILoad {
private:
	string			m_FileName;
	StaticModel*	m_pModel;

	void ConvertFrame(LPD3DXFRAME in, ModelFrame* out);
public:
	/// Static vars set before export
	static float s_Scale;
    static bool  s_UseSkinning;
    /// Strip mesh hierarchy?
    static bool  s_StripHierarchy;

	XFileLoad(){}
	virtual bool LoadModel(string name, StaticModel* Model);
};


//-----------------------------------------------------------------------------
/// Saves .X files
//-----------------------------------------------------------------------------
/*
class XFileSave : public ISave {
private:
	string			m_FileName;
	StaticModel*	m_pModel;

public:

	XFileSave(){}
	virtual bool SaveModel(string name, StaticModel* Model){ return false; }
};
*/
