//================================================================================
// Particles: Particles, particle systems, particle managers
// Particles include weapons fire, light flares, rain, explosions, sparks, etc
//================================================================================
#include "stdafx.h"
#include "Engine.h"
#include "Particles.h"
#include "FXManager.h"
#include "GameEngine.h"

/*
WeatherParticles::WeatherParticles(World* world, COLOR color,float spawninterval,int num, int max, Vector& centerpos, Vector& direction, Vector& acceleration, float thespeed, float dirvariance, float thespeedvariance, float thelifetime, float thelifetimevariance, float thetimefadeout, float particleSize, float sizeVariance,bool movingsource,Texture* theTexture, BlendMode blend) : ParticleSystem(world,color,spawninterval,num,max,centerpos,direction,acceleration,thespeed,dirvariance,thespeedvariance,thelifetime,thelifetimevariance,thetimefadeout,particleSize,sizeVariance,movingsource,theTexture,blend)
{
    reboundVelocityPercentage = .75;
    maxBounceMagnitude = 4.0f;
    minBounceMagnitude = 1.0f;

    predictedImpact = false;

    //position = velocity*t + .5*accel*t^2
    Vector testpoint = Location + sprayDir.Normalized() * (speed + .4*speedvariance)*2.4 + .5 *accel*5.29;
    CollisionInfo result;
    if(MyWorld->CollisionCheckRay(NULL,Location + sprayDir.Normalized()/1.6,testpoint,CHECK_GEOMETRY,result))
    {
        predictedImpactPlaneNormal = result.normal;
        predictedImpactPlanePoint =  result.point;
        predictedImpact = true;
    }
}

// set particle on default values for this system type
void WeatherParticles::SetParticleDefaults(Particle& p)
{
    // hardcoded variables for this example
    //float lifeTime = 500 + rand()%4000; // 4 seconds

    // Variance is in degrees (that's what MakeDirection takes)

    p.position	= Location;					// set particle position to system center
    p.oldPos.Set(0,0,0);
    p.alive     = true;							// particle is alive
    p.life		= lifetime + ((rand()%100)/100.0f)*lifetimevariance;			
    p.color		= COLOR_RGBA(COLOR_GETRED(m_Color),COLOR_GETGREEN(m_Color),COLOR_GETBLUE(m_Color), 255);// original colors at full intensity


    // Send the particle off in a random heading +- variance
    //Vector randDir = Vector::MakeDirection(((rand()%100)/100.0f)*variance, ((rand()%100)/100.0f)*variance,((rand()%100)/100.0f)*variance) - Vector::MakeDirection(((rand()%100)/100.0f)*variance, ((rand()%100)/100.0f)*variance,((rand()%100)/100.0f)*variance);
    Vector randDir= Vector(((rand()%100)/100.0f)*variance, ((rand()%100)/100.0f)*variance,((rand()%100)/100.0f)*variance) - Vector(((rand()%100)/100.0f)*variance, ((rand()%100)/100.0f)*variance,((rand()%100)/100.0f)*variance);
    // The sprayDir is the general direction the particles will head in, this will be factored with the particle's specific variance
    p.velocity	= (randDir  +  sprayDir).Normalized() * (speed + ((rand()%100)/100.0f)*speedvariance);
    p.width = p.height = particlesize - ((rand()%100)/100.0f)*sizevariance;

    // Acceleration forces, for example gravity
    p.acceleration = accel;
    p.hasVelocity = true;
    //if(doesParticleRotation)p.curRotationAngle = 6.28 * RANDF();
}

void WeatherParticles::Tick()
{
    float DeltaTime = GDeltaTime;
    if(DeltaTime > .1f)
        DeltaTime = .1f;

    // Add new particles every 'msSpawn' milliseconds if we're not at max already
    msCount += DeltaTime*1000;
    // NOTE: You may well want to start at the maximum number of alive polygons, it depends on the effect you're going for
    if(numAlive < maxParticles && msCount >= msSpawn)
    {	
        if(!stopspawning)numAlive++;
        msCount = 0;
    }

    // Loop through and update all the particles
    for(int i=0;i<numAlive;i++)
    {
        Particle& par = particles[i];
        par.life -= DeltaTime*1000;

        // If particle is dead, create fresh particle
        if(par.life <= 0 && !stopspawning) SetParticleDefaults(par);

        // Fade out over the last second of life by changing the particle's diffuse color
        if(par.life < timefadeout){
            COLOR c = par.color;
            int a = par.life/timefadeout*255.f;
            par.color = COLOR_RGBA(COLOR_GETRED(c),COLOR_GETGREEN(c),COLOR_GETBLUE(c),a);
        }
        if(par.hasVelocity)
        {
            par.position += par.velocity*DeltaTime;
            if(par.oldPos == Vector(0,0,0))par.oldPos = par.position;

            if(predictedImpact)
            {
                Vector resultingPoint;
                if(doesSegmentCrossPlane(predictedImpactPlanePoint,predictedImpactPlaneNormal,par.oldPos,par.position + (par.position - par.oldPos).Normalized()*par.width/1.2f,resultingPoint))
                {
                    float bounceMagnitude = par.velocity.Length()*reboundVelocityPercentage*((25 + rand()%75)/100.0f);
                    if(bounceMagnitude > maxBounceMagnitude)bounceMagnitude = maxBounceMagnitude;
                    par.velocity = -bounceMagnitude*par.velocity.Reflected(predictedImpactPlaneNormal).Normalized();
                    if(par.velocity.Length() < minBounceMagnitude)
                    {
                        par.velocity = Vector(0,0,0);
                        par.hasVelocity = false;
                    }
                    par.position = resultingPoint + par.velocity*DeltaTime;
                }
            }

            par.oldPos = par.position;
            if(par.hasVelocity)par.velocity += par.acceleration*DeltaTime;
        }
        if(par.life <= 0 && stopspawning) par.color = COLOR_RGBA(0,0,0,0);
    }
}

bool WeatherParticles::doesSegmentCrossPlane(Vector& planeP, Vector& planeN, Vector& lineStart, Vector& lineEnd, Vector& intersectionPoint)
{
    float det1 = (lineStart - planeP).Dot(planeN);
    float det2 = (lineEnd - planeP).Dot(planeN);
    if(det1 > 0 && det2 < 0)
    {
        intersectionPoint = lineStart + (det1 / (det1 - det2)) * (lineEnd - lineStart);
        return true;
    }
    return false;
}*/