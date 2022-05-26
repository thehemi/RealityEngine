//----------------------------------------------------------------------------------
// 
//----------------------------------------------------------------------------------

class XMLSystem
{
private:
	class XercesDOMParser *m_Parser;
	void ParseNode(class DOMNode* pCurrent);

public:
	bool Load(string filename);
	
};

XercesDOMParser * LoadFile(char* name);
