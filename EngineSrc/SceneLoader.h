//=========== (C) Copyright 2003, Tim Johnson. All rights reserved. ===========
/// Name: SceneLoader.h
/// Desc: Map/Model loading class. Used by Game and Model only.
/// NOTE: Mustn't be a singleton/static class, due to nested file loads
//=============================================================================
#pragma once


class SceneLoader {
private:
	int  m_bOldVersion; /// Hack so we can load 5007 files
	long m_FileVerts; /// For map statistics, written to log after load
	long m_FileInds;
	int					m_Version;
	string				m_FileName;
	FILE*				m_File;

	/// ONLY one of these will be set, and the other will be NULL
	World* m_WorldPtr;
	StaticModel* m_ModelPtr;

	void LoadScript(Script*& script, Model* Model, World* world);
	void LoadLight(Light*& light);
	void LoadLightFrame(LightState& l);

	void DummyLoadMaterial();
	void LoadModelFrame(ModelFrame*& frame);
	void LoadRenderTree(RenderTree*& tree);
	void LoadRenderableNode(RenderableNode*& node);
	void LoadCollisionMesh(CollisionMesh*& cMesh);
	void LoadMesh(Mesh*& mesh, string name);
	void LoadTextureMap(Material*& mat, Texture*& map);
	void LoadMaterial(Material*& mat);
	void LoadMaterials(vector<Material*>& materials, bool& hasPRT);
	void LoadEntity();
	void LoadPrefab(struct NodeData* data, Matrix tm);

	float   ReadFloat();
	string  ReadString();
	void	ReadString(char* str);
	void	ReadString(string& str);
	DWORD	ReadDWORD();
	int		ReadInt();
	bool	ReadBool();
	bool	OpenFile(const char* name);
	void	CloseFile();
	

protected:
	friend class StaticModel;
	friend class World;
	
	SceneLoader();
	bool LoadWorld(string name, World* world, bool IsSky);
	bool LoadModel(string name, StaticModel* Model);
};
