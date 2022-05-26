//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ==================
///
/// ParticleSystem: Batches and runs basic movement physics for large groups of camera oriented quads
/// typically to simulate natural systems like smoke, rain, or fire
//============================================================================================

#ifndef PARTICLES_H
#define PARTICLES_H

#include "FXManager.h"

//-------------------------------------------------------------------------------------------------
// a particle
class Particle
{
public:
	// constructor and destructor (empty for speed)
	Particle()	{ alive = false; }
	~Particle()	{ }
	
	// public attributes
	bool			alive;
	Vector			position;
	Vector			oldPos;
	Vector			velocity;
	Vector			acceleration;
	COLOR			color;
	float			life;
	float			width;
	float			height;
	float			mininumY;
	Vector	YaxisConstraintVec;
	bool hasVelocity;
};

//-------------------------------------------------------------------------------------------------
// particle system class
class GAME_API ParticleSystem : public FXSystem
{	
	friend class Particle;
	friend class FXManager;
	
protected:
	Texture* texture; // Texture for particles
	vector<Particle> particles; // Dynamic array of all alive particles
	int	numAlive; // number of alive particles
	float msCount;  // Internal timer for the above
	bool stopspawning;
	int	maxParticles; // In use with static buffer, for speed
public:
	// constructor and destructor
	ParticleSystem(World* world, COLOR color, float spawninterval,int num, int max, Vector& centerpos, Vector& direction, Vector& acceleration, float thespeed, float dirvariance, float thespeedvariance, float thelifetime, float thelifetimevariance, float thetimefadeout, float particleSize, float sizeVariance,bool movingsource,Texture* theTexture, BlendMode blend=BLEND_ONE);
	ParticleSystem(World* world, int num, COLOR color, float width, float height, Texture* theTexture,BlendMode blend);

	bool ownResource;
	virtual ~ParticleSystem();
	virtual void		Tick();
	virtual void		PostRender(Camera& cam);
	virtual void StopSpawningNow(){stopspawning = true;}
	// main functions
	virtual void		SetParticleDefaults(Particle& p);
	virtual void		SetDefaults(){ for (int i=0; i<particles.size(); i++) SetParticleDefaults(particles[i]);  }
	virtual void		SetDefaultsMoving(){ for (int i=0; i<particles.size(); i++){SetParticleDefaults(particles[i]); particles[i].life = 0;}}

	virtual void SetMaxParticles(int max);

	 /// Blend Mode for particles
	BlendMode blendMode;
	/// Spawn time in ms for new particles
	float msSpawn;	
	/// Particles spray this way
	Vector sprayDir;
	float variance;
	float particlesize;
	float sizevariance;
	Vector accel;
	float speed;
	float timefadeout;
	float lifetime;
	float lifetimevariance;
	float speedvariance;
    float YaxisConstraint;
    Vector posVariance;
    bool preventIndoors;
	COLOR m_Color;
	float m_AlphaMultiplier;
};

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
class GAME_API BouncyParticles : public ParticleSystem
{
public:
	BouncyParticles(World* world, COLOR color,float spawninterval,int num, int max, Vector& centerpos, Vector& direction, Vector& acceleration, float thespeed, float dirvariance, float thespeedvariance, float thelifetime, float thelifetimevariance, float thetimefadeout, float particleSize, float sizeVariance,bool movingsource,Texture* theTexture, BlendMode blend=BLEND_ONE);
	virtual void Tick();
	virtual void		SetParticleDefaults(Particle& p);
	bool doesSegmentCrossPlane(Vector& planeP, Vector& planeN, Vector& lineStart, Vector& lineEnd,Vector& intersectionPoint);
	Vector predictedImpactPlaneNormal;
	Vector predictedImpactPlanePoint;
	bool predictedImpact;
	float reboundVelocityPercentage;
	float maxBounceMagnitude;
	float minBounceMagnitude;
};

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
/*class GAME_API WeatherParticles : public ParticleSystem
{
public:
	WeatherParticles(World* world, COLOR color,float spawninterval,int num, int max, Vector& centerpos, Vector& direction, Vector& acceleration, float thespeed, float dirvariance, float thespeedvariance, float thelifetime, float thelifetimevariance, float thetimefadeout, float particleSize, float sizeVariance,bool movingsource,Texture* theTexture, BlendMode blend=BLEND_ONE);
	virtual void Tick();
	virtual void		SetParticleDefaults(Particle& p);
	bool doesSegmentCrossPlane(Vector& planeP, Vector& planeN, Vector& lineStart, Vector& lineEnd,Vector& intersectionPoint);
	Vector predictedImpactPlaneNormal;
	Vector predictedImpactPlanePoint;
	bool predictedImpact;
	float reboundVelocityPercentage;
	float maxBounceMagnitude;
	float minBounceMagnitude;
};*/

#endif