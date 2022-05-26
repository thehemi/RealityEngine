

//----------------------------------------------------------------------------------
// Desc: .X File importer for levels and models
//----------------------------------------------------------------------------------
class XImport{
private:

public:
	ImportFrame sceneRoot;
	bool Import(string filename, bool& success);
};

