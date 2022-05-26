//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ==================
///
/// LaserParticleSystem: A particle system that drawscamera-oriented quads
/// which are constrained along their z-axis, typically for particles that display a directional orientation like lasers
//============================================================================================

#ifndef LASERPARTICLES_H
#define LASERPARTICLES_H

#include "Particles.h"

class GAME_API LaserParticleSystem : public ParticleSystem
{	
public:
	float reboundVelocityPercentage;
	float maxBounceMagnitude;
	float minBounceMagnitude;
	float laserLength;
	bool isParticlesBouncing;
	LaserParticleSystem(float LaserLength,World* world,float spawninterval,int num, int max, Vector& centerpos, Vector& direction, Vector& acceleration, float thespeed, float dirvariance, float thespeedvariance, float thelifetime, float thelifetimevariance, float thetimefadeout, float particleSize, float sizeVariance,bool movingsource,bool bouncyParticles,Texture* theTexture,BlendMode blend=BLEND_ONE);
	virtual void		PostRender(Camera& cam);
	virtual void Tick();
	bool doesSegmentCrossPlane(Vector& planeP, Vector& planeN, Vector& lineStart, Vector& lineEnd,Vector& intersectionPoint);
	Vector predictedImpactPlaneNormal;
	Vector predictedImpactPlanePoint;
	bool predictedImpact;
};

#endif
