//================================================================================
// Laser Particles
//================================================================================
#include "stdafx.h"
#include "LaserParticles.h"
#include "FXManager.h"
#include "GameEngine.h"
LaserParticleSystem::LaserParticleSystem(float LaserLength,World* world,float spawninterval,int num, int max, Vector& centerpos, Vector& direction, Vector& acceleration, float thespeed, float dirvariance, float thespeedvariance, float thelifetime, float thelifetimevariance, float thetimefadeout, float particleSize, float sizeVariance,bool movingsource,bool bouncyParticles,Texture* theTexture,BlendMode blend) : ParticleSystem(world,COLOR_RGBA(255,255,255,255),spawninterval,num, max, centerpos, direction, acceleration, thespeed, dirvariance, thespeedvariance, thelifetime,thelifetimevariance, thetimefadeout, particleSize, sizeVariance,movingsource,theTexture,blend)
{
	laserLength = LaserLength;
	reboundVelocityPercentage = .65;
	maxBounceMagnitude = 3.0f;
	minBounceMagnitude = 1.4f;
	isParticlesBouncing = bouncyParticles;
	predictedImpact = false;

	if(isParticlesBouncing)
	{
		//position = velocity*t + .5*accel*t^2
		Vector testpoint = Location + sprayDir.Normalized() * (speed + .4*speedvariance)*2.0 + .5 *accel*4.0;
		CollisionInfo result;
		if(MyWorld->CollisionCheckRay(NULL,Location + sprayDir.Normalized()/1.5,testpoint,CHECK_GEOMETRY,result))
		{
			predictedImpactPlaneNormal = result.normal;
			predictedImpactPlanePoint =  result.point;
			predictedImpact = true;
		}
	}
}

Vector StartRight; 
Vector StartLeft;
Vector EndRight;
Vector EndLeft;
Vector startpos;
Vector endpos;
void	LaserParticleSystem::PostRender(Camera& cam)
{
	if(!numAlive)return;

	int batchArray = FXManager::getBatchForWriting(texture,blendMode);
	if(batchArray == -1)return;
	VertexIndicesBatch* pBatch = &FXManager::vertexIndicesBatches[FXManager::batchedQuads[batchArray].myVertexIndicesBatch];

	int numQuads = FXManager::batchedQuads[batchArray].numQuads;

	for(int i=0;i<numAlive;i++)
	{
		startpos = particles[i].position - particles[i].velocity.Normalized()*laserLength*((float)particles[i].width/particlesize);
		endpos = particles[i].position + particles[i].velocity.Normalized()*laserLength*((float)particles[i].width/particlesize);

		StartRight= Cross(cam.Location-startpos, endpos - startpos);
		StartLeft = -StartRight;
		StartRight.Normalize();
		StartLeft.Normalize();
		StartLeft *= particles[i].width;
		StartRight *= particles[i].width;
		EndLeft = endpos + StartLeft;
		EndRight = endpos + StartRight;
		StartLeft += startpos ;
		StartRight += startpos;

		COLOR thecolor = particles[i].color;

		if(numQuads > 1499)//QUADS_PER_BATCH-1)
		{
			FXManager::batchedQuads[batchArray].numQuads = numQuads;
			batchArray = FXManager::addNewBatch(texture,blendMode);
			if(batchArray == -1)return;
			pBatch = &FXManager::vertexIndicesBatches[FXManager::batchedQuads[batchArray].myVertexIndicesBatch];
			numQuads = FXManager::batchedQuads[batchArray].numQuads;
		}

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

		numQuads++;
	}

	FXManager::batchedQuads[batchArray].numQuads = numQuads;
}

//-----------------------------------------
// Update the state of all the particles
//-----------------------------------------
void LaserParticleSystem::Tick()
{
	// Add new particles every 'msSpawn' milliseconds if we're not at max already
	msCount += GDeltaTime*1000;
	// NOTE: You may well want to start at the maximum number of alive polygons, it depends on the effect you're going for
	if(numAlive < maxParticles && msCount >= msSpawn){	
		if(!stopspawning)numAlive++;
		msCount = 0;
	}
	// Loop through and update all the particles
	for(int i=0;i<numAlive;i++){
		Particle& par = particles[i];
		par.life -= GDeltaTime*1000;

		// If particle is dead, create fresh particle
		if(par.life <= 0 && !stopspawning) SetParticleDefaults(par);

		// Fade out over the last second of life by changing the particle's diffuse color
		if(par.life < timefadeout){
			COLOR c = par.color;
			par.color = COLOR_RGBA(COLOR_GETRED(c),COLOR_GETGREEN(c),COLOR_GETBLUE(c),(int)(par.life/timefadeout*255.f));
		}

		if(par.hasVelocity)
		{
			par.position += par.velocity*GDeltaTime;
			if(par.oldPos == Vector(0,0,0))par.oldPos = par.position;
			if(isParticlesBouncing)
			{
					if(predictedImpact)
					{
					Vector resultingPoint;
					if(doesSegmentCrossPlane(predictedImpactPlanePoint,predictedImpactPlaneNormal,par.oldPos,par.position,resultingPoint))
					{
						float bounceMagnitude = par.velocity.Length()*reboundVelocityPercentage*((25 + rand()%75)/100.0f);
						if(bounceMagnitude > maxBounceMagnitude)bounceMagnitude = maxBounceMagnitude;
						par.velocity = -bounceMagnitude*par.velocity.Reflected(predictedImpactPlaneNormal).Normalized();
						if(par.velocity.Length() < minBounceMagnitude)
						{
							par.velocity = Vector(0,0,0);
							par.hasVelocity = false;
						}
						par.position = resultingPoint + par.velocity*GDeltaTime;
					}
					}
			}
			par.oldPos = par.position;
			if(par.hasVelocity)par.velocity += par.acceleration*GDeltaTime;
		}
		if(par.life <= 0 && stopspawning) par.color = COLOR_RGBA(0,0,0,0);
	}
}
bool LaserParticleSystem::doesSegmentCrossPlane(Vector& planeP, Vector& planeN, Vector& lineStart, Vector& lineEnd, Vector& intersectionPoint)
{
	float det1 = (lineStart - planeP).Dot(planeN);
	float det2 = (lineEnd - planeP).Dot(planeN);
	if(det1 > 0 && det2 < 0)
	{
		intersectionPoint = lineStart + (det1 / (det1 - det2)) * (lineEnd - lineStart);
		return true;
	}
	return false;
}
