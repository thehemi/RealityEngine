//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ==================
///
/// LightBeam: Overlapping blended billboards lined up to create the appearance of a volumetric light-beam
//============================================================================================

#ifndef LIGHTBEAM_H
#define LIGHTBEAM_H
#include "FXManager.h"

/// LightBeam: Overlapping blended billboards lined up to create the appearance of a volumetric light-beam
class GAME_API LightBeam : public FXSystem
{	
	friend class FXManager;

protected:
	struct BeamUnitInfo
	{
		Vector Position;
		float Size;
		COLOR color;
	};

	Texture* m_Texture; // Texture for beam unit

	vector<BeamUnitInfo>	BeamUnitInfos; // the postions of each beam unit, so that we don't have to recalc them needlessly every frame

public:
	int	m_NumUnits;
	COLOR m_Color;
	float m_DistanceMultiplier;
	float m_TotalScalingFactor;
	float m_StartSize;
	float m_SizeMultiplier;
	Vector Dir;
	void setPosDir(Vector& loc, Vector& dir);
	void setColor(COLOR color);
	void UpdateBeamsInfos();

//	alphaPercent *= .15f * ((Player)Owner).FlashLightState.Intensity / 3.0f;
//  float ScalingFactor = .0125f;
//	MVector Dir = Rotation.GetDir();
//	MVector startPos = Location + Rotation.GetDir() * .4f;
//	for (float i = 0; i < 41; i++)
//		MCanvas.BillBoard(startPos + Dir * (i * (i / 6.0f) * ScalingFactor), 3.0f * ScalingFactor + (46.7f / 40.0f) * i * ScalingFactor, MHelpers.ColorFromRGBA(255, 255, 255, (int)((100.0f - 100.0f * (i / 40.0f)) * alphaPercent)), FlashLightGlowTex, MBlendMode.MBLEND_SRCALPHA, MBlendMode.MBLEND_ONE);

	LightBeam(World* world, Texture* texture, int NumUnits, float DistanceMultiplier, float StartSize, float SizeMultiplier, float TotalScalingFactor, COLOR color);
	virtual void PostRender(Camera& cam);
};

#endif