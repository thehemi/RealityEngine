//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ==================
///
/// SurfaceDecal: FXSystem draws quad at fixed location and orientation, typically for impact decals on static geometry
/// such as bullet holes, explosion scorch marks, and blood splats
//============================================================================================

#ifndef SURFACEDECAL_H
#define SURFACEDECAL_H
//================================================================================
// SURFACE DECAL
//================================================================================
#include "FXManager.h"

//-------------------------------------------------------------------------------------------------
class GAME_API SurfaceDecal : public FXSystem
{	
	friend class FXManager;
	
protected:
	Texture* texture; // Texture for decal
	Vector Dir;
	float FadeTime;
	float size;
	float alpha;
	BlendMode dest;
	Vector StartRight; 
	Vector StartLeft;
	Vector EndRight;
	Vector EndLeft;
	COLOR m_Color;
public:
	void setPosDir(Vector& pos, Vector& dir);
	virtual void Tick();
	SurfaceDecal(World* world,Texture* tex,Vector& pos,Vector& dir,COLOR color, float Size = 8.f,float lifeTime = 8.f,float fadeTime = 2.f,BlendMode destblend = BLEND_ONE);
	virtual void		PostRender(Camera& cam);
};
#endif