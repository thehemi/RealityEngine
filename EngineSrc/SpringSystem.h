//=========== Copyright (c) 2003, Tim Johnson. All rights reserved. ===========
//
/// Spring System for cloth simulator.
//
//=============================================================================
#ifndef _SPRINGSYSTEM_H_
#define _SPRINGSYSTEM_H_

#ifndef DOXYGEN_IGNORE
struct SNode;
struct Spring {
	SNode *node;
	float naturalLength;
};
#endif

/// Set of springs within cloth simulation
struct SNode {
	SNode(Vector* position, Vector* norm){
		pos = position;
		normal = norm;
		dir = Vector(0, 0, 0);
		locked = false;
		nNormal = 0;
	}
	void addSpring(SNode *node){
		Spring spring;

		spring.node = node;
		spring.naturalLength = (*pos - *node->pos).Length();//distance(pos, node->pos);

		springs.push_back(spring);
	}

	Vector* pos;
	Vector* normal;
	Vector dir;

	vector<Spring> springs; /// NOTE: Used to be a Set. vector safe?
	unsigned int nNormal;

	bool locked;
};

typedef void (*ProcessNodeFunc)(SNode *node, float *attribs);

/// Manages & updates all springs within cloth simulation
class SpringSystem {
public:
	SpringSystem();

	void addRectField(unsigned int width, unsigned int height, void *pos, void *norm, unsigned int stride);

	void update(float dTime, ProcessNodeFunc process = NULL, float *attribs = NULL);
	void evaluateNormals();

	SNode *getNode(unsigned int index) const { return nodes[index]; }
	unsigned int getNodeCount() const { return nodes.size(); }

	void setGravity(const Vector &grav){ gravity = grav; }

protected:
	Vector gravity;
	vector<SNode*> nodes; /// NOTE: Used to be set
};


#endif /// _SPRINGSYSTEM_H_

