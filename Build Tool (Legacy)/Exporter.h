class Exporter {
private:
	ofstream file;

	void RecursiveTextureWrite(TexMap& map);
	void WriteString(string& str);
	
	//---------------------------------------------------------------
	// General compilation
	//---------------------------------------------------------------
	void ExportMaterials(vector<Material>& materials);
	void ExportMaterial(Material& mat);
	void ExportPool(Pool& pool);
	void ExportLight(Light& light);
	void ExportLightFrame(LightFrame& l);

	//---------------------------------------------------------------
	// Level compilation
	//---------------------------------------------------------------
	void ExportSceneProperties(SceneProperties& props);
	void ExportStaticGeometry(LevelFile& level);
	void ExportRenderNode(Node& node);
	void ExportCollisionData(CollisionData& data);
	void ExportEntitiesAndPrefabs(vector<Entity>& entities);

	//---------------------------------------------------------------
	// Model compilation
	//---------------------------------------------------------------
	void ExportMesh(Mesh* mesh);
	void ExportFrames(ExportFrame* frame);
		

public:

	void ExportModelFile(string filename, ModelFile& in);
	void ExportLevelFile(string filename, LevelFile& in);
};