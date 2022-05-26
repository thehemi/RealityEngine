//=========== Copyright (c) 2003, Tim Johnson. All rights reserved. ===========
//
//
//=============================================================================
#include <stdafx.h>
#include "ClothSimulator.h"
#include "SpringSystem.h"
#include "Engine.h"


#define SPHERE_LEVEL 4
#define POW4(x) (1 << (2 * (x)))
#define SPHERE_SIZE (8 * 3 * POW4(SPHERE_LEVEL))

SphereHandle test;
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ClothSimulator::Initialize(){
	spring = new SpringSystem;
	AddCloth();
	test = AddSphere();
}

//-----------------------------------------------------------------------------
// Adds a sphere into the simulator
// Spheres can interact with cloth
//-----------------------------------------------------------------------------
SphereHandle ClothSimulator::AddSphere(){
	Sphere* s = new Sphere;
	m_Spheres.push_back(s);
	return (SphereHandle)s;
}

//-----------------------------------------------------------------------------
// Updates a sphere
//-----------------------------------------------------------------------------
void ClothSimulator::UpdateSphere(SphereHandle sphere, Vector& pos, float radius){
	Sphere* s = (Sphere*)sphere;
	s->pos	  = pos;
	if(radius != -1)
		s->size	  = radius;
}

//-----------------------------------------------------------------------------
// Adds a cloth to the simulator
// Inputs are number of segments along x and y
//-----------------------------------------------------------------------------
void ClothSimulator::AddCloth(Mesh* clothMesh){
	float CLOTH_SIZE_X = 25;
	float CLOTH_SIZE_Y = 25;
	float C_SIZE  = (4.f / CLOTH_SIZE_X);

	Cloth* cloth	 = new Cloth;
	cloth->clothMesh = clothMesh;
	cloth->SizeX	 = CLOTH_SIZE_X;
	cloth->SizeY	 = CLOTH_SIZE_Y;

	static int cornerInds[] = { 0, CLOTH_SIZE_X - 1, CLOTH_SIZE_X * CLOTH_SIZE_Y - 1, (CLOTH_SIZE_Y - 1) * CLOTH_SIZE_X };
	static float sum;

	int numIndices = (CLOTH_SIZE_X-1)*(CLOTH_SIZE_Y-1)*3*2;

	HRESULT hr = D3DXCreateMesh(numIndices, CLOTH_SIZE_X*CLOTH_SIZE_Y, D3DXMESH_DYNAMIC|D3DXMESH_WRITEONLY ,VertexFormats::Instance()->FindFormat(sizeof(Vertex))->element ,RenderWrap::dev, &cloth->dxMesh);
	if(FAILED(hr))
		Error("D3DXCreateMesh failed");

	// Create vertices
	//cloth->vertices = new Vertex*[CLOTH_SIZE_X];
	//for(int i=0;i<CLOTH_SIZE_X;i++)
	//	cloth->vertices[i] = new Vertex[CLOTH_SIZE_Y];

	for (int j = 0; j < CLOTH_SIZE_Y; j++){
		for (int i = 0; i < CLOTH_SIZE_X; i++){
			cloth->vertices[i][j].position = Vector(0,12,0) + Vector(C_SIZE * (i - 0.5f * (CLOTH_SIZE_X - 1)), 0, C_SIZE * (0.5f * (CLOTH_SIZE_Y - 1) - j));
			cloth->vertices[i][j].tex.x = float(i) / (CLOTH_SIZE_X - 1);
			cloth->vertices[i][j].tex.y = float(j) / (CLOTH_SIZE_Y - 1);
		}
	}
	
	// Fill the index buffer
	int inds = 0;
	WORD* indices;
	cloth->dxMesh->LockIndexBuffer(0, (LPVOID*)&indices);
	bool flip2 = false;
	for(int i=0;i<CLOTH_SIZE_Y-1;i++){
		bool flip = false;
		for (unsigned int j = 0; j < CLOTH_SIZE_X-1; j++){
			indices[inds++]	= CLOTH_SIZE_X*i + j;
			indices[inds++] = CLOTH_SIZE_X*(i+1) + j;
			indices[inds++] = CLOTH_SIZE_X*i + (j+1);

			indices[inds++]	= CLOTH_SIZE_X*i + j+1;
			indices[inds++] = CLOTH_SIZE_X*(i+1) + (j);
			indices[inds++] = CLOTH_SIZE_X*(i+1) + (j+1);
		}
	}
	cloth->dxMesh->UnlockIndexBuffer();

	assert(numIndices == inds);


	spring->addRectField(CLOTH_SIZE_X, CLOTH_SIZE_Y, cloth->vertices, ((char *) cloth->vertices) + sizeof(Vector), sizeof(Vertex));

	for (int i = 0; i < spring->getNodeCount(); i++){
		spring->getNode(i)->dir = Vector(0, 0, 0);
	}

	// Stop the corners of the cloth from moving by locking them
	for (int i = 0; i < 4; i++) spring->getNode(cornerInds[i])->locked = i<2;

	m_Cloths.push_back(cloth);
}

//-----------------------------------------------------------------------------
// Updates all cloths in the simulator
//-----------------------------------------------------------------------------
void ClothSimulator::Tick(Camera* cam){
	UpdateSphere(test,cam->Location,2);

	for(int i=0;i<m_Cloths.size();i++){
		Cloth* cloth = m_Cloths[i];
		//
		// Update springs
		//
		spring->update(min(GDeltaTime, 0.0125f));

		for (int k = 0; k < spring->getNodeCount(); k++){
			SNode *node = spring->getNode(k);

			bool include = true;
			// Handle the objects that interact with the cloth here
			for (int j = 0; j < m_Spheres.size(); j++){
				Vector v = *node->pos - m_Spheres[j]->pos;
				float dSqr = v.DotSelf();

				if (dSqr < m_Spheres[j]->size * m_Spheres[j]->size){
					// Don't let cloth go inside sphere
					*node->pos = (m_Spheres[j]->size * v.Normalized()) + m_Spheres[j]->pos;
					// Slow cloth down
					node->dir =Vector(0,0,0);//*= powf(0.015f, GDeltaTime);
					include = false;
				}
			}

		}
		spring->evaluateNormals();

		// Fill buffer with new vertex data
		Vertex* vertices;
		cloth->dxMesh->LockVertexBuffer(D3DLOCK_DISCARD, (LPVOID*)&vertices);
		int count = 0;
		for(int k=0;k<cloth->SizeY;k++){
			for(int j=0;j<cloth->SizeX;j++){
				vertices[count++] = cloth->vertices[k][j];
			}
		}
		cloth->dxMesh->UnlockVertexBuffer();

		//MaterialManager::Instance()->GetDefaultMaterial()->Apply
		RenderWrap::SetRS( D3DRS_CULLMODE, D3DCULL_NONE);
		cloth->dxMesh->DrawSubset(0);
		RenderWrap::SetRS( D3DRS_CULLMODE, D3DCULL_CCW);
	}

	
}