//================================================================================
// SURACE DECAL
//================================================================================
#include "stdafx.h"
#include "SurfaceDecal.h"
#include "GameEngine.h"

SurfaceDecal::SurfaceDecal(World* world,Texture* tex,Vector& pos,Vector& dir,COLOR color,float Size,float lifeTime,float fadeTime,BlendMode destblend) : FXSystem(world)
{
	dest = destblend;
	texture = tex;
	m_Color = color;
	LifeTime = lifeTime;
	FadeTime = fadeTime;
	size = Size;
	alpha = 255;
	Dir = dir.Normalized();
	Location = pos + Dir*.021;
	if((Vector(0,1,0) - Dir).IsNearlyZero())
	{
		StartRight = Location + Vector(0,0,1)*size + Vector(1,0,0)*size;
		StartLeft = Location + Vector(0,0,1)*size - Vector(1,0,0)*size;
		EndRight = Location - Vector(0,0,1)*size + Vector(1,0,0)*size;
		EndLeft = Location - Vector(0,0,1)*size - Vector(1,0,0)*size;
	}
	else if((Vector(0,-1,0) - Dir).IsNearlyZero())
	{
		StartRight = Location + Vector(0,0,-1)*size + Vector(-1,0,0)*size;
		StartLeft = Location + Vector(0,0,-1)*size - Vector(-1,0,0)*size;
		EndRight = Location - Vector(0,0,-1)*size + Vector(-1,0,0)*size;
		EndLeft = Location - Vector(0,0,-1)*size - Vector(-1,0,0)*size;
	}
	else
	{
		Matrix m = Matrix::LookTowards(Dir);
		StartRight = Location + m.GetUp()*size + m.GetRight()*size;
		StartLeft = Location + m.GetUp()*size - m.GetRight()*size;
		EndRight = Location - m.GetUp()*size + m.GetRight()*size;
		EndLeft = Location - m.GetUp()*size - m.GetRight()*size;
	}
}

void SurfaceDecal::setPosDir(Vector& pos, Vector& dir)
{
	Dir = dir.Normalized();
	Location = pos + Dir*.021;
		if((Vector(0,1,0) - Dir).IsNearlyZero())
		{
			StartRight = Location + Vector(0,0,1)*size + Vector(1,0,0)*size;
			StartLeft = Location + Vector(0,0,1)*size - Vector(1,0,0)*size;
			EndRight = Location - Vector(0,0,1)*size + Vector(1,0,0)*size;
			EndLeft = Location - Vector(0,0,1)*size - Vector(1,0,0)*size;
		}
		else if((Vector(0,-1,0) - Dir).IsNearlyZero())
		{
			StartRight = Location + Vector(0,0,-1)*size + Vector(-1,0,0)*size;
			StartLeft = Location + Vector(0,0,-1)*size - Vector(-1,0,0)*size;
			EndRight = Location - Vector(0,0,-1)*size + Vector(-1,0,0)*size;
			EndLeft = Location - Vector(0,0,-1)*size - Vector(-1,0,0)*size;
		}
		else
		{
		Matrix m = Matrix::LookTowards(Dir);
		StartRight = Location + m.GetUp()*size + m.GetRight()*size;
		StartLeft = Location + m.GetUp()*size - m.GetRight()*size;
		EndRight = Location - m.GetUp()*size + m.GetRight()*size;
		EndLeft = Location - m.GetUp()*size - m.GetRight()*size;
		}
}

void SurfaceDecal::Tick()
{
	if(LifeTime != -1 && LifeTime < FadeTime)alpha = (LifeTime/FadeTime)*255.0f;
	FXSystem::Tick();
}
void SurfaceDecal::PostRender(Camera& cam)
{
	if(alpha > 255.0)alpha=255.0;
	if(alpha < 0)alpha = 0;
	COLOR thecolor;
	thecolor = COLOR_RGBA(COLOR_GETRED(m_Color),COLOR_GETGREEN(m_Color),COLOR_GETBLUE(m_Color),(int)alpha);

	int batchArray = FXManager::getBatchForWriting(texture,dest);
	if(batchArray == -1)return;
	VertexIndicesBatch* pBatch = &FXManager::vertexIndicesBatches[FXManager::batchedQuads[batchArray].myVertexIndicesBatch];

	int numQuads = FXManager::batchedQuads[batchArray].numQuads;

	pBatch->theVertices[(numQuads*4)].diffuse = thecolor;
	pBatch->theVertices[(numQuads*4)].position = StartLeft;
	pBatch->theVertices[(numQuads*4)].tu = 0;
	pBatch->theVertices[(numQuads*4)].tv = 1;

	pBatch->theVertices[(numQuads*4)+1].diffuse = thecolor;
	pBatch->theVertices[(numQuads*4)+1].position = StartRight;
	pBatch->theVertices[(numQuads*4)+1].tu = 1;
	pBatch->theVertices[(numQuads*4)+1].tv = 1;

	pBatch->theVertices[(numQuads*4)+2].diffuse = thecolor;
	pBatch->theVertices[(numQuads*4)+2].position = EndLeft;
	pBatch->theVertices[(numQuads*4)+2].tu = 0;
	pBatch->theVertices[(numQuads*4)+2].tv = 0;

	pBatch->theVertices[(numQuads*4)+3].diffuse = thecolor;
	pBatch->theVertices[(numQuads*4)+3].position = EndRight;
	pBatch->theVertices[(numQuads*4)+3].tu = 1;
	pBatch->theVertices[(numQuads*4)+3].tv = 0;

	FXManager::batchedQuads[batchArray].numQuads++;
}


