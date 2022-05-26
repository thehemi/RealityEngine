//================================================================================
// LASER
//================================================================================
#include "stdafx.h"
#include "Engine.h"
#include "lasers.h"
Laser::Laser(World* world,COLOR color, Vector& Startpos,Vector& Endpos,float Width,Texture* tex, BlendMode blend) : FXSystem(world)
{
	texture = tex;
	m_Color = color;
	blendMode	= blend;
	startpos = Startpos;
	endpos = Endpos;
	width = Width;
	Location = Startpos;
}

// destructor
Laser::~Laser()
{
}
void Laser::Tick(){}

void Laser::SetEndPoints(Vector& Startpos, Vector& Endpos)
{
	startpos = Startpos;
	endpos = Endpos;
	Location = Startpos;
}

void Laser::PostRender(Camera& cam)
{
	Vector CameraVector;
	Vector LaserVector;
	Vector StartRight; 
	Vector StartLeft;
	Vector EndRight;
	Vector EndLeft;
	CameraVector =cam.Location-startpos;
	LaserVector = endpos - startpos;
	StartRight= Cross(CameraVector, LaserVector);
	StartLeft = -StartRight;
	StartRight.Normalize();
	StartLeft.Normalize();
	StartLeft *= width;
	StartRight *= width;
	EndLeft = endpos + StartLeft;
	EndRight = endpos + StartRight;
	StartLeft += startpos ;
	StartRight += startpos;

	int batchArray = FXManager::getBatchForWriting(texture,blendMode);
	if(batchArray == -1)return;
	VertexIndicesBatch* pBatch = &FXManager::vertexIndicesBatches[FXManager::batchedQuads[batchArray].myVertexIndicesBatch];

	int numQuads = FXManager::batchedQuads[batchArray].numQuads;

		pBatch->theVertices[(numQuads*4)].diffuse = m_Color;
		pBatch->theVertices[(numQuads*4)].position = StartLeft;
		pBatch->theVertices[(numQuads*4)].tu = 0;
		pBatch->theVertices[(numQuads*4)].tv = texture->vTile + texture->vOff;

		pBatch->theVertices[(numQuads*4)+1].diffuse = m_Color;
		pBatch->theVertices[(numQuads*4)+1].position = StartRight;
		pBatch->theVertices[(numQuads*4)+1].tu = texture->uTile;
		pBatch->theVertices[(numQuads*4)+1].tv = texture->vTile + texture->vOff;

		pBatch->theVertices[(numQuads*4)+2].diffuse = m_Color;
		pBatch->theVertices[(numQuads*4)+2].position = EndLeft;
		pBatch->theVertices[(numQuads*4)+2].tu = 0;
		pBatch->theVertices[(numQuads*4)+2].tv = texture->vOff;

		pBatch->theVertices[(numQuads*4)+3].diffuse = m_Color;
		pBatch->theVertices[(numQuads*4)+3].position = EndRight;
		pBatch->theVertices[(numQuads*4)+3].tu = texture->uTile;
		pBatch->theVertices[(numQuads*4)+3].tv = texture->vOff;

	FXManager::batchedQuads[batchArray].numQuads++;
}

void Laser::SetColor(COLOR color)
{
	m_Color = color;
}