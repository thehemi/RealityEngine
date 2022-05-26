#ifndef OCTREE_INCLUDED
#define OCTREE_INCLUDED

struct Pool;

//----------------------------------------------------------------------------------
// Desc: A node for an octree/quadtree
//----------------------------------------------------------------------------------
struct Node {
	Node(){ 
		numChildren = 0;
		children[0] = 0; children[1] = 0; children[2] = 0; children[3] = 0;
	}
	void DeleteAll(){
		for(int i=0;i<numChildren;i++){
			if(children[i]){
				children[i]->DeleteAll();
				delete children[i];
				children[i] = 0;
			}
		}
	}
	vector<Face> faces;
	BBox box;

	int numChildren;
	Node* children[4];

	// Compiled and Indexed data pooled by material
	vector<Pool> pools;
};


bool BuildTree(Node* aNode, const vector<Face>& faces, int depth, int MinNodeSize, int MinFaces, bool CollisionTree);

// Helper functions...
enum TouchType{
	BOX_NOTOUCH,
	BOX_CONTAINED,
	BOX_INTERSECTS,
};

TouchType Touches(BBox& b1, BBox& b2);
int Inside(BBox& b, Vector& p);

#endif // OCTREE_INCLUDED