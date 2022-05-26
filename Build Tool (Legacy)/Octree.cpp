//----------------------------------------------------------------
// Octree.cpp - Code for building quadtree or octrees
// used in either collision or rendering
//
// For rendering two factors are important, density and size.
// Bigger Size = More fillrate impact
// Bigger Density = More vertex impact
//
// Performance TODO: 3dsmax boxes that let us turn up the tesellation around areas like buildings
//
//----------------------------------------------------------------

#include <stdafx.h>



int triBoxOverlap(BBox& box, Face& face);
void SplitByBox(BBox& box, vector<Face>& faces, vector<Face>& boxFaces);

//----------------------------------------------------------------
// Is face excluded?
//----------------------------------------------------------------
bool CheckFaceExcluded(Light& light, Face& face){
	bool found = false;
	for(int b = 0; b< light.excludeIncludeMeshPointers.size();b++){
		if(face.srcMesh == light.excludeIncludeMeshPointers[b]){
			found = true;
		}
	}

	// Delete excluded faces from lit list...
	if((found && light.IsExcludeList) || (!found && !light.IsExcludeList)){
		return true;
	}
	return false;
}


//----------------------------------------------------------------
// Classifies faces into inside, outside, or bordering box
// More clever than PutInBox, but only works when the data has been split
//----------------------------------------------------------------
void PutInBox_AssumeSplitAlready(Light& light, vector<Face>& faces, vector<Face>& outFaces, vector<Face>& boxFaces, vector<Face>& borderFaces){
	BBox box = light.box;
	// Shrink the box a bit, or it includes borderlines as inside
	BBox abox = box;
	abox.min += Vector(0.001f,0.001f,0.001f);
	abox.max -= Vector(0.001f,0.001f,0.001f);

	for(int i=0;i<faces.size();i++){
		// Check include/exclude list
		if(CheckFaceExcluded(light,faces[i])){
			outFaces.push_back(faces[i]);
			continue;
		}

		// If it's a directional light it's obviously inside it, as it covers the entire scene
		if(light.type == LIGHT_DIR){
			boxFaces.push_back(faces[i]);
			continue;
		}

		int b1 = Inside(box,faces[i].v[0].p);
		int b2 = Inside(box,faces[i].v[1].p);
		int b3 = Inside(box,faces[i].v[2].p);

		bool partlyoutside=false, partlyinside=false;

		if(b1 == 0 || b2 == 0 || b3 == 0)
			partlyoutside = true;

		if(b1 == 1 || b2 == 1 || b3 == 1)
			partlyinside = true;

		bool overlap = triBoxOverlap(abox,faces[i]);

		// It crosses both sides
		if((partlyinside && partlyoutside) || (overlap  && partlyoutside)){
			borderFaces.push_back(faces[i]);
		}
		// It's completely inside
		else if(partlyinside && !partlyoutside){
			boxFaces.push_back(faces[i]);
		}
		// It's completely outside
		else if(partlyoutside && !partlyinside){
			outFaces.push_back(faces[i]);
		}
		// It sits tightly on the inside of the box
		else if(overlap){
			if(b1 != -1 || b2 != -1 || b3 != -1)
				cout << "ERROR -WHERE IS THIS TRIANGLE SITTING??"<<endl;

			boxFaces.push_back(faces[i]);
		}
		// It's exactly on the box!
		else{ //if(!partlyinside && !partlyoutside)

			// WARNING: This may be a wrong assumption!!
			boxFaces.push_back(faces[i]);
			//cout << "Warning: Borderline face, assuming inside box" << endl;
		}	
	}
}



//----------------------------------------------------------------
// Classifies faces into inside, outside, or bordering box
//----------------------------------------------------------------
void PutInBox(BBox& box, vector<Face>& faces, vector<Face>& outFaces, vector<Face>& boxFaces, vector<Face>& borderFaces){
	for(int i=0;i<faces.size();i++){
		// Expand the test box to include bordering faces as inside
		//BBox abox = box;
		//abox.min -= Vector(0.001f,0.001f,0.001f);
		//abox.max += Vector(0.001f,0.001f,0.001f);


		int b1 = Inside(box,faces[i].v[0].p);
		int b2 = Inside(box,faces[i].v[1].p);
		int b3 = Inside(box,faces[i].v[2].p);

		// Take no risks, class everything close to the edge as bordering
		if(b1 == -1 || b2 == -1 || b3 == -1){
			borderFaces.push_back(faces[i]);
			continue;
		}


		bool partlyoutside=false, partlyinside=false;

		if(b1 == 0 || b2 == 0 || b3 == 0)
			partlyoutside = true;

		if(b1 == 1 || b2 == 1 || b3 == 1)
			partlyinside = true;

		bool overlap = triBoxOverlap(box,faces[i]);

		// CONSIDER PARTLY OUTSIDE BUT BORDERING

		// It crosses both sides
		if((partlyinside && partlyoutside) || (overlap  && partlyoutside)){
			borderFaces.push_back(faces[i]);
		}
		// It's completely inside
		else if(partlyinside && !partlyoutside){
			boxFaces.push_back(faces[i]);
		}
		// It's completely outside
		else if(partlyoutside && !partlyinside)
			outFaces.push_back(faces[i]);
		// It's exactly on the box!
		else //if(!partlyinside && !partlyoutside)
			cout << "VERY RARE!!! FACE BORDERS BOX EXACTLY. COULD BE BROKEN CLIPPING!" << endl;
			
	}

	int tot = outFaces.size() + boxFaces.size() + borderFaces.size();
	if(tot != faces.size())
		cout << "ERRROR - PUTINBOX DIVIDE WAS INVALID" << endl;

}


//----------------------------------------------------------------
// Used for the rendering tree, because it doesn't duplicate vertices
// across nodes
// Instead, bordering faces are put in the first box they border, 
// and the box is enlarged to enclose them
// This seems like the best way to me
//----------------------------------------------------------------
void FillBoxes(Vector& center,const vector<Face>& faces, vector<BBox>& boxes,vector<vector<Face> >& facesOut, vector<Face>& bordering){
	int nodes = boxes.size(); // 8 for octree, 4 for quadtree

	vector<Face> curFaces = faces;

	for(int i=0;i<nodes;i++){
		vector<Face> remaining, border;
		PutInBox(boxes[i],curFaces,remaining,facesOut[i],border);

		curFaces = remaining;

		// Put all bordering faces in this box, but grow the box so it fits them
		if(border.size()){
			for(int j=0;j<border.size();j++){
				boxes[i] += border[j].v[0].p;
				boxes[i] += border[j].v[1].p;
				boxes[i] += border[j].v[2].p;

				facesOut[i].push_back(border[j]);
			}
		}
	}
}


/*
// This is the old and slow method. It puts borderline faces in the parent node, but that
// really really sucks, as 1/8th of the world ends up in the topmost node, and can't be culled!!
void FillBoxes(Vector& center,const vector<Face>& faces, vector<BBox>& boxes,vector<vector<Face> >& facesOut, vector<Face>& bordering){
	int nodes = boxes.size(); // 8 for octree, 4 for quadtree

	vector<Face> curFaces = faces;

	for(int i=0;i<nodes;i++){
		vector<Face> remaining, border;
		PutInBox(boxes[i],curFaces,remaining,facesOut[i],border);

		curFaces = remaining;

		// Put all bordering faces in our border list
		if(border.size()){
			int start = bordering.size();
			bordering.resize(start + border.size());
			for(int j=0;j<border.size();j++){
				bordering[start + j] = border[j];
			}
		}
	}
}
*/

//----------------------------------------------------------------
// Used for the collision tree, because it duplicate vertices
// But creates much more optimal trees
//----------------------------------------------------------------
void FillBoxes_Collision(Vector& center,const vector<Face>& faces, vector<BBox>& boxes,vector<vector<Face> >& facesOut, vector<Face>& bordering){
	int nodes = boxes.size(); // 8 for octree, 4 for quadtree

	vector<Face> faces2 = faces;

	for(int i=0;i<nodes;i++){
		vector<Face> remaining, border;
		PutInBox(boxes[i],faces2,remaining,facesOut[i],border);

		// Put all bordering faces in this node
		if(border.size()){
			int start = facesOut[i].size();
			facesOut[i].resize(start + border.size());
			for(int j=0;j<border.size();j++){
				facesOut[i][start + j] = border[j];
			}
		}
	}

}



//----------------------------------------------------------------
// Splits 'faces' by all 'boxes' and classifies them into 'facesOut', there is one facesOut for
// each box
//----------------------------------------------------------------
/*
void FillBoxes_SPLIT(Vector& center,vector<Face>& faces, vector<BBox>& boxes,vector<vector<Face> >& facesOut){
	int nodes = boxes.size(); // 8 for octree, 4 for quadtree
	for(int i=0;i<nodes;i++){
		SplitByBox(boxes[i],faces,facesOut[i]);
	}
}
*/

// Simple heuristic for measuring box size
long Size(BBox& b){
	Vector size = b.max - b.min;
	return size.x;
}

//----------------------------------------------------------------
// Builds a quadtree or octree
// If CollisionTree is specified, FillBoxes_Collision is used,
// duplicating vertices that cross node boundaries.
//----------------------------------------------------------------
bool BuildTree(Node* aNode, const vector<Face>& faces, int depth, int MinNodeSize, int MinFaces, bool CollisionTree){

	depth++;

	if(depth > 30)
		cout << "SANITY CHECK - Recursion depth is now " << depth << endl;

	// Terminating critera reached.
	// Create end node and return
	if(Size(aNode->box) < MinNodeSize || faces.size() < MinFaces){
		aNode->numChildren = 0;
		aNode->faces = faces;
		return false;
	}


	BBox box = aNode->box;

	// Create the 4 new bounding boxes
	Vector midPoint = (box.max + box.min) *0.5f;
	midPoint.y = box.max.y; // Remove Y axis, or it would be an octree

	vector<BBox> boxes(4);

	// Default values
	for(int j=0;j<4;j++){
		boxes[j].min = box.min;
		boxes[j].max = midPoint;
	}

	// quad tree looks like this from top: X is ---->
	// |1|2|
	// |3|4|

	// First quadrant (all default values, so nothing to do)

	// Second quadrant (X max)
	boxes[1].min.x = box.max.x;

	// Third quadrant (Z max)
	boxes[2].min.z = box.max.z;

	// Fourth (Z and X max)
	boxes[3].min = Vector(box.max.x,box.min.y,box.max.z);

	// Sort their max/mins for good measure
	for(j=0;j<4;j++){
		Sort(boxes[j].min,boxes[j].max);
	}


	// We now have four new boxes
	// Loop through the faces and fill each box
	// If a face overlaps two nodes, add it to the parent node
	vector<vector<Face> > boxfaces(4);

	// Class faces by box. Any face bordering a box goes in this node instead of its children
	if(CollisionTree)
		FillBoxes_Collision(midPoint,faces,boxes,boxfaces,aNode->faces);
	else
		FillBoxes(midPoint,faces,boxes,boxfaces,aNode->faces);

	// Check to see if we created redundant nodes (no further clipping)
	for(int j=0;j<4;j++){
		if(boxfaces[j].size() == faces.size()){
			// Box didn't help, time to stop recursing
			aNode->numChildren = 0;
			aNode->faces = faces;
			return false;
		}
	}

	// We now have our boxes, add them to our children list then recurse
	for(j=0;j<4;j++){
		// Node is empty
		if(boxfaces[j].size() == 0)
			continue;

		aNode->children[aNode->numChildren] = new Node;
		aNode->children[aNode->numChildren]->box = boxes[j];

		// Pass node and data
		// Node will either fill itself with data and terminate
		// or spawn children
		BuildTree(aNode->children[aNode->numChildren],boxfaces[j],depth,MinNodeSize,MinFaces,CollisionTree);

		aNode->numChildren++;
	}

	return true;
}
