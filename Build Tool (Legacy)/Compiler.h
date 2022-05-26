// Compiles the map into a game-ready format. This involves:
//
// -Creating a collision quadtree
// -Creating a view data octree
// -Slicing geometry around lights
// -Stripifying and cache coherency
// -Adding tangent data
// -Creating unique vertex and index pools and indices
//
// Copyright Timothy Johnson, 30.8.2002
#pragma once

extern CPRTSimulator    g_Simulator;

class Compiler {
private:
	bool CreateModel;
	bool CreateStrips;
	int MinRenderNodeSize;

	vector<Material> m_GlobalMaterials;

	//---------------------------------------------------------------
	// General compilation
	//---------------------------------------------------------------
	string GetFrameSHFile(ImportFrame* frame);
	bool FrameNeedsRecompiling(ImportFrame* frame, string file);
	void PreProcessTree(ImportFrame* frame);
	void GetStaticFaces(ImportFrame* srcFrame, vector<Face>& faces);
	void GetAllEntities(ImportFrame* srcFrame, vector<Entity>& entities);
	void GetAllLights(ImportFrame* srcFrame, vector<Light>& lights);
	void AddCollisionData(CollisionData& data, vector<Vertex>& vertices, vector<WORD>& indices, int matID,  bool localSpace);

	//---------------------------------------------------------------
	// Level compilation
	//---------------------------------------------------------------
	void GetCollisionDataFromTree(Node& node, CollisionData& data);
	bool GeneratePoolsFromFaces(vector<Face>& faces,vector<Pool>& pools);
	void MergeTreePools(Node& root, vector< Mesh* >& meshes);
	void GenerateLitAndOptimizedPools(vector<Light>& worldLights, Node& node, int& nodeCount);
		int GenerateLitPools(vector<Face>& inFaces, vector<Pool>& outPools,vector<Light>& worldLights, Matrix& faceTM);

	//---------------------------------------------------------------
	// Model compilation
	//---------------------------------------------------------------
	void CompileHierarchy(ImportFrame* srcFrame, ExportFrame*& outFrame);
	
	ImportFrame* m_pRootFrame;

public:

	Compiler(bool createStrips, int minRenderNodeSize);
	~Compiler();

	void CompileModelFile(ImportFrame* srcFrame, ModelFile& out);
	void CompileLevelFile(ImportFrame* srcFrame, LevelFile& out);
};