//================================================================================
// LightBeam: Overlapping blended billboards lined up to create the appearance of a volumetric light-beam
//================================================================================
#include "stdafx.h"
#include "Engine.h"
#include "LightBeam.h"
#include "FXManager.h"

LightBeam::LightBeam(World* world, Texture* texture, int NumUnits, float DistanceMultiplier, float StartSize, float SizeMultiplier,float TotalScalingFactor, COLOR color) : FXSystem(world)
{
	m_Texture = texture;
	BeamUnitInfos.resize(NumUnits);
	m_NumUnits = NumUnits;
	setColor(color);
	m_DistanceMultiplier = DistanceMultiplier;
	m_StartSize = StartSize;
	m_SizeMultiplier = SizeMultiplier;
	m_TotalScalingFactor = TotalScalingFactor;
	UpdateBeamsInfos();
}

void LightBeam::setColor(COLOR color)
{
m_Color = color;
float alpha = COLOR_GETALPHA(m_Color);
for(int i = 0; i < m_NumUnits; i++)
{
	BeamUnitInfos[i].color = COLOR_RGBA(COLOR_GETRED(m_Color),COLOR_GETGREEN(m_Color),COLOR_GETBLUE(m_Color),(int)(alpha - alpha*(i/(float)m_NumUnits)));
}
}

void LightBeam::setPosDir(Vector& loc, Vector& dir)
{
	Location = loc;
	Dir = dir;
	for(int i = 0; i < m_NumUnits; i++)
	{
		BeamUnitInfos[i].Position = Location + Dir*(i*(i*m_DistanceMultiplier)*m_TotalScalingFactor);
	}
}

void LightBeam::UpdateBeamsInfos()
{
BeamUnitInfos.resize(m_NumUnits);
float alpha = COLOR_GETALPHA(m_Color);
for(int i = 0; i < m_NumUnits; i++)
{
	BeamUnitInfos[i].color = COLOR_RGBA(COLOR_GETRED(m_Color),COLOR_GETGREEN(m_Color),COLOR_GETBLUE(m_Color),(int)(alpha - alpha*(i/(float)m_NumUnits)));
	BeamUnitInfos[i].Position = Location + Dir*(i*(i*m_DistanceMultiplier)*m_TotalScalingFactor);
	BeamUnitInfos[i].Size =  ((m_StartSize + i*m_SizeMultiplier) * m_TotalScalingFactor)/2.0f;
}
}

Vector BeamLoc;
Vector BeamRight;
Vector BeamUp;
void LightBeam::PostRender(Camera& cam)
{
	BlendMode blendMode = BLEND_ONE;

	Matrix mat = cam.view;
	Vector rightVect(mat[0][0],mat[1][0],mat[2][0]);
	Vector upVect(mat[0][1],mat[1][1],mat[2][1]);

	int batchArray = FXManager::getBatchForWriting(m_Texture,blendMode);
	if(batchArray == -1)return;
	VertexIndicesBatch* pBatch = &FXManager::vertexIndicesBatches[FXManager::batchedQuads[batchArray].myVertexIndicesBatch];

	int numQuads = FXManager::batchedQuads[batchArray].numQuads;

	for(int i=0;i<m_NumUnits;i++)
	{
		BeamLoc	=  BeamUnitInfos[i].Position;
		BeamRight = rightVect * BeamUnitInfos[i].Size;
		BeamUp = upVect * BeamUnitInfos[i].Size;

		COLOR color = BeamUnitInfos[i].color;

		if(numQuads > 1499)//QUADS_PER_BATCH-1)
		{
			FXManager::batchedQuads[batchArray].numQuads = numQuads;
			batchArray = FXManager::addNewBatch(m_Texture,blendMode);
			if(batchArray == -1)return;
			pBatch = &FXManager::vertexIndicesBatches[FXManager::batchedQuads[batchArray].myVertexIndicesBatch];
			numQuads = FXManager::batchedQuads[batchArray].numQuads;
		}

		int NumVertices = numQuads*4;
		int NumVerticesOff1 = NumVertices + 1;
		int NumVerticesOff2 = NumVertices + 2;
		int NumVerticesOff3 = NumVertices + 3;

		pBatch->theVertices[NumVertices].diffuse = color;
		pBatch->theVertices[NumVertices].position = (BeamLoc-BeamRight)-BeamUp;
		pBatch->theVertices[NumVertices].tu = 0;
		pBatch->theVertices[NumVertices].tv = 1;

		pBatch->theVertices[NumVerticesOff1].diffuse = color;
		pBatch->theVertices[NumVerticesOff1].position =  (BeamLoc+BeamRight)-BeamUp;
		pBatch->theVertices[NumVerticesOff1].tu = 1;
		pBatch->theVertices[NumVerticesOff1].tv = 1;

		pBatch->theVertices[NumVerticesOff2].diffuse = color;
		pBatch->theVertices[NumVerticesOff2].position = (BeamLoc-BeamRight)+BeamUp;
		pBatch->theVertices[NumVerticesOff2].tu = 0;
		pBatch->theVertices[NumVerticesOff2].tv = 0;

		pBatch->theVertices[NumVerticesOff3].diffuse = color;
		pBatch->theVertices[NumVerticesOff3].position = (BeamLoc+BeamRight)+BeamUp;
		pBatch->theVertices[NumVerticesOff3].tu = 1;
		pBatch->theVertices[NumVerticesOff3].tv = 0;

		numQuads++;
	}
	FXManager::batchedQuads[batchArray].numQuads = numQuads;
}