//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ==================
///
/// Laser: FXSystem that draws a camera-oriented quad constrained along its z-axis
//============================================================================================

#ifndef LASERS_H
#define LASERS_H
//================================================================================
// LASERS
//================================================================================
#include "FXManager.h"

//-------------------------------------------------------------------------------------------------
// LASER CLASS
class GAME_API Laser : public FXSystem
{	
	friend class FXManager;
protected:
	Texture* texture; // Texture for Laser
	BlendMode				blendMode; // Blend Mode
	BBox					boundingBox; // For view-frustum culling
	float width;
	Vector startpos;
	Vector endpos;
	COLOR m_Color;
public:
	void setWidth(float value){width = value;}
	// constructor and destructor
	Laser(World* world,COLOR color,Vector& Startpos,Vector& Endpos,float Width, Texture* tex, BlendMode blend=BLEND_ONE);
	virtual ~Laser();
	virtual void		Tick();
	void SetEndPoints(Vector& Startpos, Vector& Endpos);
	void SetColor(COLOR color);
	virtual void		PostRender(Camera& cam);
};
#endif
