//----------------------------------------------------------------
// OptimizeStaticLighting.cpp - Associates static lights with
// the exact mesh segments they cast on, so there is absoloutely
// no extrataneous pixels or vertices lit.
// Lights can still move of course, at which point the engine
// falls back to the rendering tree
//
// FIXME ASAP: Never include "dynamic" lights on static list, because
// the engine will fill the dynamic list, and we'll have two lists
//----------------------------------------------------------------

#include <stdafx.h>

bool SPLIT_GEOMETRY = false;

void SplitByBox(BBox& box, vector<Face>& faces, vector<Face>& boxFaces); // Octree.cpp
void PutInBox_AssumeSplitAlready(Light& light, vector<Face>& faces, vector<Face>& outFaces, vector<Face>& boxFaces, vector<Face>& borderFaces);
void PutInBox(BBox& box, vector<Face>& faces, vector<Face>& outFaces, vector<Face>& boxFaces, vector<Face>& borderFaces);

struct LightPart{
	vector<Face> faces;
	vector<Light*> staticLights;
	BBox box;
};

//-------------------------------------------------------------------
// Finds a part corresponding to a light overlap combination
//-------------------------------------------------------------------
LightPart* FindCombination(LightPart* ignorePart, vector<Light*>& clipCombination, vector<LightPart*>& lightParts){
	LightPart* combo = NULL;
	if(clipCombination.size() == 0)
		return NULL;

	for(int b=0;b<lightParts.size();b++){
		LightPart* aPart = lightParts[b];
		// Obviously don't check against ourselves
		if(aPart == ignorePart)	continue;
		int match = 0;
		for(int d=0;d<aPart->staticLights.size();d++){
			for(int e=0;e<clipCombination.size();e++){
				if(clipCombination[e] == aPart->staticLights[d]){
					match++; continue;
				}
			}
		}
		// We've got a match if we found *exactly* the number of elements in clipCombination
		// and it's the same as the number of elements in aPart->staticLights
		if(match == clipCombination.size() && match == aPart->staticLights.size())
			combo = aPart;
	}
	return combo;
}

//-----------------------------------------------------------------------------------
// Snips all light segments by all other lights. This is to cover overlapping lights
// Each part is snipped by all lights
//   Though overlap segments are only saved once ever for each combination
//-----------------------------------------------------------------------------------
void ResolveOverlaps(LightPart* part, vector<Light>& worldLights, vector<LightPart*>& lightParts){
	// Snip the part by all lights that overlap it, that aren't recorded as being on the surface
	// already
	for(int a=0;a<worldLights.size();a++){
		Light& light = worldLights[a];
		LightFrame& lFrame = worldLights[a].keyframes[0];

		if(part->faces.size() == 0) // May become empty after any light snip. pointless checking it
			continue;

		int radius = lFrame.attenEnd;
		Vector pos = lFrame.tm[3];

		// If this part was already clipped by this light, skip it
		bool continueNow = false;
		for(int m=0;m<part->staticLights.size();m++){
			if(part->staticLights[m] == &light)
				continueNow = true;
		}
		if(continueNow) continue;

		// If light doesn't overlap part, skip light
		if(light.type == LIGHT_OMNI && Touches(light.box,part->box) == BOX_NOTOUCH)
			continue;

		// If overlap combination is stored, clip (to remove dup faces) but don't store it again..

		// Compare the combination of each existing part to clipCombination
		// This is the clip combination we are looking for
		vector<Light*> clipCombination = part->staticLights;
		clipCombination.push_back(&light);
		bool skip = FindCombination(part,clipCombination,lightParts) != NULL;

		// Snip part by light to get the |I|ntersection of both overlapping lights
		// Get inside and outside faces
#if SPLIT_GEOMETRY
		vector<Face> inside, outside;
		outside = part->faces;
		SplitByBox(light.box,outside,inside);
#else
		vector<Face> inside, outside;
		vector<Face> border;
		int in = part->faces.size();
		PutInBox_AssumeSplitAlready(light,part->faces,outside,inside,border);
		// Put bordering faces on the box face list too
		for(int g=0;g<border.size();g++)
			inside.push_back(border[g]);
#endif

		int inFaces = inside.size();
		int outFaces = outside.size();
		int totFaces = part->faces.size();

		// Should never happen, but can do if something gets broke
		if(inFaces == outFaces == totFaces) cout << "SHOULD NEVER HAPPEN" << endl;

		// If we were totally out of light, do nothing
		if(outFaces == totFaces && inFaces == 0){
		}
		// Otherwise we are in/out of light, so create new parts
		// !skip means we haven't created this combination before
		else if(inside.size() && !skip){
			LightPart* lp = new LightPart;
			lp->faces = inside;
			// copy all lights from parent part
			lp->staticLights = part->staticLights;
			// plus the light it was just clipped with
			lp->staticLights.push_back(&light);

			// Collision box
			// TODO: Make triangle-accurate test against light box
			// Will give a decent speed boost
			BBox newBox;
			for (int k=0; k<lp->faces.size(); k++){
				newBox += lp->faces[k].v[0].p;
				newBox += lp->faces[k].v[1].p;
				newBox += lp->faces[k].v[2].p;
			}

			lp->box = newBox;
			lightParts.push_back(lp);
		}

		// The clipped old part, now with the shared part removed
		part->faces = outside;
	}
}


//----------------------------------------------------------------------------------
// Desc: Converts a tree of faces into optimized+lit pools
//----------------------------------------------------------------------------------
void Compiler::GenerateLitAndOptimizedPools(vector<Light>& worldLights, Node& node, int& nodeCount){
	for(int i=0;i<node.numChildren;i++){
		GenerateLitAndOptimizedPools(worldLights,*node.children[i],nodeCount);
	}

	if(node.faces.size()){
		GenerateLitPools(node.faces,node.pools,worldLights,Matrix());
		nodeCount++;
	}
}

//----------------------------------------------------------------------------------
// Desc: Tags the world faces with the lights that touch them, then groups by light
//
// NOTE: Lots of slow vector copying going on here
//----------------------------------------------------------------------------------
int Compiler::GenerateLitPools(vector<Face>& inFaces, vector<Pool>& outPools,vector<Light>& worldLights, Matrix& faceTM){
	if(!inFaces.size())
		return 0;

	// Triggers multiple times
	//cout << "\nProcessing lights.."<<endl;

	// Build bounding boxes for all the lights
	for(int i=0;i<worldLights.size();i++){
		LightFrame& light = worldLights[i].keyframes[0];
		BBox box;
		box.max = Vector(light.attenEnd,light.attenEnd,light.attenEnd);
		box.min = Vector(-light.attenEnd,-light.attenEnd,-light.attenEnd);
		box.max += light.tm[3];
		box.min += light.tm[3];
		// Put box in object space if faces are in object space
		if(!(faceTM == Matrix()))
			box = box.Transformed(faceTM.Inverse());
		worldLights[i].box = box;
	}
	

	int numFaces = 0;
	int numOriginalFaces = inFaces.size();
	vector<Face> originalFaces = inFaces;
	vector<Face> unlitFaces = inFaces;
	vector<LightPart*> lightParts;

	// Keep track of this for sanity checking later on
	int excludedFaceCount = 0;

	//
	// Get one lightPool for each light that overlaps any faces
	// Also clips 'faces' by all lights, so only the outside is left
	for(int i=0;i<worldLights.size();i++){
		Light& light = worldLights[i];

		LightPart* part = new LightPart;

		// Get the faces the light casts on..
#if SPLIT_GEOMETRY
		// Get the faces inside the box
		vector<Face> facesCopy = originalFaces;
		SplitByBox(light.box,facesCopy,part->faces);

		// Clip the remaining entire face set by this box
		vector<Face> dummy;
		SplitByBox(light.box,unlitFaces,dummy);
#else
		vector<Face> bordering;
		vector<Face> outside;
		PutInBox_AssumeSplitAlready(light,originalFaces,outside,part->faces,bordering);
		// Put bordering faces on the box face list too
		for(int g=0;g<bordering.size();g++)
			part->faces.push_back(bordering[g]);

		// Clip the remaining entire face set by this box
		vector<Face> newFaces, dum1, dum2;
		PutInBox_AssumeSplitAlready(light,unlitFaces,newFaces,dum1,dum2);
		unlitFaces = newFaces;
#endif

		// Add to part list
		if(part->faces.size()){
			part->staticLights.push_back(&light);


			if(SPLIT_GEOMETRY){
				part->box = light.box; // LightPart box is initially the box of its first light
			}
			else{
				// TODO: Put this back - Proper culling fit

				// Part box must enclose all geometry, or we can't check it for overlaps with other lights
				BBox newBox;
				for (int k=0; k<part->faces.size(); k++){
					newBox += part->faces[k].v[0].p;
					newBox += part->faces[k].v[1].p;
					newBox += part->faces[k].v[2].p;
				}
				part->box = newBox;
			}

			lightParts.push_back(part);

			
		}
	}

	// We've got our light parts, now we need to resolve it on other overlapping lights
	for(int i=0;i<lightParts.size();i++){
		ResolveOverlaps(lightParts[i],worldLights,lightParts);
	}

	//-------------------------------------------------------------------------------------------
	// Check for any excluded/included faces on the light exclusion list
	//
	//-------------------------------------------------------------------------------------------
	// This has to be iterated if anything lower in the chain is modified
	// e.g. if lightpart #10 modifies lightpart #5,  we must start again
/*	int iterations = 0;
	bool modifiedSomething = true;
	while(modifiedSomething){
		modifiedSomething = false;
		iterations++;
		// For each light part see if any lights exclude some faces
		for(int i=0;i<lightParts.size();i++){

			LightPart* part = lightParts[i];
			if(part->faces.size() == 0)
				continue;

			// For every light this part is responsible for
			for(int k=0;k<part->staticLights.size();k++){
				Light* light = part->staticLights[k];
				// Quick-exit
				if(light->excludeIncludeMeshPointers.size() == 0 && light->IsExcludeList)
					continue;

				// Find the part without this excluded light, in case we need to move faces to it
				vector<Light*> clipCombination = part->staticLights;
				clipCombination.erase(std::find(clipCombination.begin(),clipCombination.end(),light));
				LightPart* comboPart = FindCombination(part,clipCombination,lightParts);
				bool newCombo = false;
				// Doesn't exist, we better create it
				if(!comboPart && part->staticLights.size() > 1){
					comboPart = new LightPart;
					comboPart->staticLights = clipCombination;
					lightParts.push_back(comboPart);
					newCombo = true;
				}


				// For every face in the part
				for(int a=0;a<part->faces.size();a++){
					bool found = false;
					for(int b = 0; b< light->excludeIncludeMeshPointers.size();b++){
						if(part->faces[a].srcFrame == light->excludeIncludeMeshPointers[b]){
							found = true;
						}
					}

					// Delete excluded faces from lit list...
					if((found && light->IsExcludeList) || (!found && !light->IsExcludeList)){
						excludedFaceCount ++;

						// This part is shared by other lights, so use the combination without this light
						// and add it to there
						if(comboPart){
							if(!newCombo)
								modifiedSomething = true;
							// Add to 'unlit' 'outside' list
							comboPart->faces.push_back(part->faces[a]);
							// Remove from 'lit' 'inside' list
							part->faces.erase(part->faces.begin() + a);
							a--;
						}
						else{
							if(part->staticLights.size() > 1)
								Error("Static Lights > 1, Yet the clip combo could not be found! This will result in undefined behaviour");

							// Non-overlap, just remove straight from part, add straight to unlit
							// Add to 'unlit' 'outside' list
							unlitFaces.push_back(part->faces[a]);
							// Remove from 'lit' 'inside' list
							part->faces.erase(part->faces.begin() + a);
							a--;
						}
					}
				}

			}
		}
	}


	if(iterations > 2)
		cout << "Sorting out light include/exclude lists took " << iterations << " iterations :-("<< endl;
		*/
	//-------------------
	// Safety checking
	// If this triggers, blame your code, not this function
	// -------------------
	if(!SPLIT_GEOMETRY){
		int totalFaces = unlitFaces.size();
		for(int i=0;i<lightParts.size();i++){
			totalFaces += lightParts[i]->faces.size();
		}

		if(totalFaces + excludedFaceCount != numOriginalFaces && (totalFaces - numOriginalFaces) > 0)
			Error("Woah this is bad! Faces lost or duplicated: %d",totalFaces - numOriginalFaces);

	}


	// Generate our output pools
	for(int i=0;i<lightParts.size();i++){
		vector<Pool> pools;
		if(lightParts[i]->faces.size() == 0)
			continue;
		GeneratePoolsFromFaces(lightParts[i]->faces,pools);

		for(int j=0;j<pools.size();j++){
			pools[j].staticLights = lightParts[i]->staticLights;
			outPools.push_back(pools[j]);
			numFaces += pools[j].Stripified?pools[j].indices.size()-2:pools[j].indices.size()/3;
		}
	}

	// Add the remaining (unlit) part of the clipped mesh
	vector<Pool> pools;
	if(unlitFaces.size())
		GeneratePoolsFromFaces(unlitFaces,pools);
	for(int j=0;j<pools.size();j++){
		outPools.push_back(pools[j]);
		numFaces += pools[j].Stripified?pools[j].indices.size()-2:pools[j].indices.size()/3;
	}

	for(int i=0;i<lightParts.size();i++)
		delete lightParts[i];

	//cout << "Processing lights complete!" << endl;
	return numFaces;

}