//=========== (C) Copyright 2004, Tim Johnson. All rights reserved. ===========
/// Type: XML (IGame) Loading Module
/// Info: Loads XML files. Data is then passed to compiler/engine
//
/// 
//
//
//
//=============================================================================
#pragma once
#include "XMLSystem.h"
#include "Serializer.h"
#include "Editor.h"

//-----------------------------------------------------------------------------
/// Loads/Saves .XML Materials
//-----------------------------------------------------------------------------
class ENGINE_API MaterialSerializer
{
	XMLSystem			m_XML;

	void ReadMaterial(DOMElement* pNode);
	void WriteMaterials(DOMElement* pNode);

	MaterialLibrary*	m_Library;

public:

	bool Load(string filename, MaterialLibrary* lib);
	bool Save(string filename, MaterialLibrary* lib);

};

//-----------------------------------------------------------------------------
/// Loads .XML IGame files
//-----------------------------------------------------------------------------
class XMLLoad : public ILoad {
private:
	long m_FileVerts; /// For map statistics, written to log after load
	long m_FileInds;
    long m_TotalNodes;
    long m_ParsedNodes;
	string				m_FileName;
	vector<Material*>	m_Materials;
	XMLSystem			m_XML;

	/// ONLY one of these will be set, and the other will be NULL
	World*			m_pWorld;
	StaticModel*	m_pModel;

	/// Serialization routines
	bool ReadSHProperties(DOMElement* pNode, PRTSettings& opt);
	bool ReadFrame(DOMElement* pNode, ModelFrame*& frame, int depth=0);
	bool ReadMeshBinary(string filename, string name, Mesh*& mesh, int matID, CollisionMesh*& cMesh);
	void ReadMeshXML(string filename,string name,  Mesh*& mesh, vector<Material*>& materials);
	void ReadSceneInfo(DOMElement* pNode, World*& world);
	void ReadMaterials(DOMElement* pNode, vector<Material*>& materials);
	void ReadNode(DOMElement* pNode, Actor*& node);
	void ReadLight(DOMElement* pNode, Light*& light);
    void ReadSelectionList(DOMElement* pNode);

	void    ParseNode(class DOMElement* pCurrent);
	bool	OpenFile(const char* name);
	void	CloseFile();
	
public:
	friend class StaticModel;
	friend class World;
	
	XMLLoad();
	virtual bool LoadWorld(string name, World* world, bool IsSky);
	virtual bool LoadModel(string name, StaticModel* Model);
};


//-----------------------------------------------------------------------------
/// Saves .XML IGame files
//-----------------------------------------------------------------------------
class XMLSave : public ISave {
private:
	friend class XMLLoad;
	long m_FileVerts; /// For map statistics, written to log after load
	long m_FileInds;
	string				m_FileName;
	vector<Material*>	m_Materials;
    /// Buf/PRT filenames that are in the scene. The rest will be deleted
    vector<string>      m_DataFiles;
    vector<bool>        m_HasPRT;
	XMLSystem			m_XML;

	/// ONLY one of these will be set, and the other will be NULL
	World*			m_pWorld;
	StaticModel*	m_pModel;

	/// Serialization routines
	void WriteMeshBinary(string filename, Mesh*& mesh);
	void WriteFrame(DOMElement* pNode, ModelFrame*& frame, int depth=0);
	void WriteSHProperties(string recvName, DOMNode* pNode, PRTSettings& opt);
	void WriteSceneInfo(DOMElement* pNode, World*& world);
	void WriteMaterials(DOMElement* pNode, vector<Material*>& materials);
	void WriteNode(DOMElement* pNode, Actor*& node);
	void WriteLight(DOMElement* pNode, Light*& light);
    void WriteSelectionList(DOMElement* pNode, ActorSelectedList* list);

	void    ParseNode(class DOMElement* pCurrent);
	DOMElement*	CreateFile(const char* name);
	void	CloseFile();
	/// Deletes unused
    void ClearUnusedFiles();
public:
	friend class StaticModel;
	friend class World;
	
	XMLSave(){m_pWorld = NULL; m_pModel = NULL;}
	virtual bool SaveWorld(string name, World* world);
	virtual bool SaveModel(string name, StaticModel* Model){ return false; }
};

