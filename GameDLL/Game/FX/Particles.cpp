//================================================================================
// Particles: Particles, particle systems, particle managers
// Particles include weapons fire, light flares, rain, explosions, sparks, etc
//================================================================================
#include "stdafx.h"
#include "Engine.h"
#include "Particles.h"
#include "FXManager.h"
#include "GameEngine.h"
#include "IndoorVolume.h"

Vector loc;
Vector Right;
Vector up;
COLOR color;

//--[Basic Particle System ]---------------------------------------------------------------------------
ParticleSystem::ParticleSystem(World* world, COLOR color, float spawninterval, int num, int max, Vector& centerpos, Vector& direction, Vector& acceleration,float thespeed, float dirvariance, float thespeedvariance, float thelifetime, float thelifetimevariance, float thetimefadeout, float particleSize, float sizeVariance, bool movingsource, Texture* theTexture, BlendMode blend) : FXSystem(world)
{
    stopspawning = false;
    preventIndoors = false;

    posVariance.x = 0.05f;
    posVariance.z = 0.05f;
    YaxisConstraint = 0;
    texture = theTexture;
    timefadeout = thetimefadeout;
    Location = centerpos;
    m_Color = color;
    sprayDir = direction;
    blendMode	= blend;
    numAlive		= num;
    maxParticles = max;
    particlesize = particleSize;
    sizevariance = sizeVariance;
    accel = acceleration;
    variance = dirvariance;
    speedvariance = thespeedvariance;
    lifetime = thelifetime;
    lifetimevariance = thelifetimevariance;
	m_AlphaMultiplier = 1;
    speed = thespeed;

    // We build the buffers to hold the maximum number of particle render data this generator will
    // hold at any one time
    assert(num<=maxParticles);

    // Note: Each particle *could* hold its own vertex data, but that would be very very slow and inefficient
    // hence the above arrays
    particles.resize(max);
    if(movingsource)SetDefaultsMoving();
    else SetDefaults();

    msSpawn = spawninterval;
    msCount = 0;
}

void ParticleSystem::SetMaxParticles(int max)
{
    maxParticles = max;
    particles.resize(maxParticles);

    if(numAlive < maxParticles - 1)
    {
        for(int i = numAlive; i < maxParticles; i++)
        {
            SetParticleDefaults(particles[i]);
            particles[i].life = RANDF()*particles[i].life;
        }
    }

    numAlive = maxParticles - 1;
}

// destructor
ParticleSystem::~ParticleSystem()
{
}

ParticleSystem::ParticleSystem(World* world, int num, COLOR color, float width, float height, Texture* theTexture,BlendMode blend) : FXSystem(world)
{
    blendMode	= blend;
    numAlive = num;
    particles.resize(num);
    texture = theTexture;
	m_AlphaMultiplier = 1;

    for(int i = 0; i < particles.size(); i++)
    {
        particles[i].color = color;
        particles[i].width = width;
        particles[i].height = height;
        particles[i].alive = true;
    }
}

// Signed rand() number between -1 and 1
inline float SRand(){ return RANDF()*2-1; }

// collide ray defined by ray origin (ro) and ray direction (rd)
// with the bounding box. Returns -1 on no collision and the face index 
// for first intersection if a collision is found together with 
// the distances to the collision points (tnear and tfar)
int RayBox(const Vector& min, const Vector& max,  const Vector& ro,const Vector& rd, float& tnear)
{
	float t1,t2,t;
	int ret=-1;

	tnear=-BIG_NUMBER;
	float tfar=BIG_NUMBER;

	int a,b;
	for( a=0;a<3;a++ )
	{
		if (rd[a]>-KINDA_SMALL_NUMBER && rd[a]<KINDA_SMALL_NUMBER)
			if (ro[a]<min[a] || ro[a]>max[a])
				return -1;
			else ;
		else 
		{
			t1=(min[a]-ro[a])/rd[a];
			t2=(max[a]-ro[a])/rd[a];
			if (t1>t2)
			{ 
				t=t1; t1=t2; t2=t; 
				b=3+a;
			}
			else
				b=a;
			if (t1>tnear)
			{
				tnear=t1;
				ret=b;
			}
			if (t2<tfar)
				tfar=t2;
			if (tnear>tfar || tfar<KINDA_SMALL_NUMBER)
				return -1;
		}
	}
	
	if (tnear>tfar || tfar<KINDA_SMALL_NUMBER)
		return -1;

	return ret;
}

Vector VectorUp = Vector(0,100,0);
Vector VectorDown = Vector(0,-1,0);

// set particle on default values for this system type
void ParticleSystem::SetParticleDefaults(Particle& p)
{
	if(posVariance.Length())
	{
    // Random spawn offset for this particle
    Vector randOffset(SRand(),SRand(),SRand());
    randOffset *= posVariance;

    p.position	= Location + randOffset;		// set particle position to system center
	}
	else
		p.position	= Location;		

	p.width = p.height = particlesize - RANDF()*sizevariance;

	p.mininumY = 0;

    // Don't spawn particles indoors, determine the collision point on the indoor volumes
    if(preventIndoors)
    {
        for(int i=0;i<IndoorVolume::IndoorVolumes.size();i++)
        {
			if(IndoorVolume::IndoorVolumes[i]->MyWorld != MyWorld || IndoorVolume::IndoorVolumes[i]->IndoorVolumeType != INDOORVOLUME_PARTICLES)
				continue;

			if(IndoorVolume::IndoorVolumes[i]->IsInsideThisVolume(p.position))
			{
				p.position = Vector(9999,-9999,9999);
				return;
			}

            BBox& bbox = IndoorVolume::IndoorVolumes[i]->volumeBox;
			float distance = 0;
			Vector StartPos = p.position+VectorUp;
            if(RayBox(bbox.min,bbox.max,StartPos,VectorDown,distance) != -1)
				p.mininumY = (StartPos + VectorDown*distance).y + p.height;
        }
    }
    
    p.alive     = true;							// particle is alive
    p.life		= lifetime + RANDF()*lifetimevariance;			
    p.color		= m_Color;// original colors at full intensity

    // Send the particle off in a random heading +- variance
    Vector randDir;
    if(variance)
        randDir.Set(SRand()*variance,SRand()*variance,SRand()*variance);
        
    // The sprayDir is the general direction the particles will head in, this will be factored with the particle's specific variance
    p.velocity	= (randDir  +  sprayDir).Normalized() * (speed + RANDF()*speedvariance);

    // Acceleration forces, for example gravity
    p.acceleration = accel;
    p.hasVelocity = true;
	if(YaxisConstraint > 0)
		p.YaxisConstraintVec.Set(0,(.4 + 1.3*RANDF())*YaxisConstraint,0);
}

//-----------------------------------------
// Update the state of all the particles
//-----------------------------------------
void ParticleSystem::Tick()
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
        if(par.life <= 0 && !stopspawning)
            SetParticleDefaults(par);

        // Fade out over the last second of life by changing the particle's diffuse color
        if(par.life < timefadeout)
        {
            COLOR c = par.color;
            int a = par.life/timefadeout*(float)COLOR_GETALPHA(m_Color);
            par.color = COLOR_RGBA(COLOR_GETRED(c),COLOR_GETGREEN(c),COLOR_GETBLUE(c),a);
        }
        par.position += par.velocity*DeltaTime;
        par.velocity += par.acceleration*DeltaTime;

		if(par.mininumY != 0 && par.position.y < par.mininumY)
			par.position = Vector(9999,-9999,9999);

        if(par.life <= 0 && stopspawning) par.color = COLOR_RGBA(0,0,0,0);
    }
}

// Most particles are rendered after the rest of the scene to ensure blending works properly
void ParticleSystem::PostRender(Camera& cam)
{
    if(!numAlive)return;

    // Calculate billboard data for a camera-facing quad using the view matrix
    Matrix mat = cam.view;
    Vector rightVect(mat[0][0],mat[1][0],mat[2][0]);
    Vector upVect(mat[0][1],mat[1][1],mat[2][1]);
    int batchArray = FXManager::getBatchForWriting(texture,blendMode);
    if(batchArray == -1)return;
    VertexIndicesBatch* pBatch = &FXManager::vertexIndicesBatches[FXManager::batchedQuads[batchArray].myVertexIndicesBatch];

    int numQuads = FXManager::batchedQuads[batchArray].numQuads;

    for(int i = 0; i < numAlive; i++)
    {
		// Bias up vect to not face camera if the camera is looking up too sharply
		if(YaxisConstraint > 0)
			upVect = (upVect+particles[i].YaxisConstraintVec).Normalized();

        loc	=  particles[i].position;
        Right = rightVect * particles[i].width;
        up = upVect * particles[i].height;

		if(m_AlphaMultiplier == 1)
			color = particles[i].color;
		else
		{
			color = particles[i].color;
			int alpha = m_AlphaMultiplier*(float)COLOR_GETALPHA(color);
			if(alpha > 255)
				alpha = 255;
			color = COLOR_RGBA(COLOR_GETRED(color),COLOR_GETGREEN(color),COLOR_GETBLUE(color),alpha);
		}

        if(numQuads > 1499)//QUADS_PER_BATCH-1)
        {
            FXManager::batchedQuads[batchArray].numQuads = numQuads;
            batchArray = FXManager::addNewBatch(texture,blendMode);
            if(batchArray == -1)return;
            pBatch = &FXManager::vertexIndicesBatches[FXManager::batchedQuads[batchArray].myVertexIndicesBatch];
            numQuads = FXManager::batchedQuads[batchArray].numQuads;
        }


        int NumVertices = numQuads*4;
        int NumVerticesOff1 = NumVertices + 1;
        int NumVerticesOff2 = NumVertices + 2;
        int NumVerticesOff3 = NumVertices + 3;

        pBatch->theVertices[NumVertices].diffuse = color;
        pBatch->theVertices[NumVertices].position = (loc-Right)-up;
        pBatch->theVertices[NumVertices].tu = 0;
        pBatch->theVertices[NumVertices].tv = 1;

        pBatch->theVertices[NumVerticesOff1].diffuse = color;
        pBatch->theVertices[NumVerticesOff1].position = (loc+Right)-up;
        pBatch->theVertices[NumVerticesOff1].tu = 1;
        pBatch->theVertices[NumVerticesOff1].tv = 1;

        pBatch->theVertices[NumVerticesOff2].diffuse = color;
        pBatch->theVertices[NumVerticesOff2].position = (loc-Right)+up;
        pBatch->theVertices[NumVerticesOff2].tu = 0;
        pBatch->theVertices[NumVerticesOff2].tv = 0;

        pBatch->theVertices[NumVerticesOff3].diffuse = color;
        pBatch->theVertices[NumVerticesOff3].position = (loc+Right)+up;
        pBatch->theVertices[NumVerticesOff3].tu = 1;
        pBatch->theVertices[NumVerticesOff3].tv = 0;

        numQuads++;
    }
    FXManager::batchedQuads[batchArray].numQuads = numQuads;
}

BouncyParticles::BouncyParticles(World* world, COLOR color,float spawninterval,int num, int max, Vector& centerpos, Vector& direction, Vector& acceleration, float thespeed, float dirvariance, float thespeedvariance, float thelifetime, float thelifetimevariance, float thetimefadeout, float particleSize, float sizeVariance,bool movingsource,Texture* theTexture, BlendMode blend) : ParticleSystem(world,color,spawninterval,num,max,centerpos,direction,acceleration,thespeed,dirvariance,thespeedvariance,thelifetime,thelifetimevariance,thetimefadeout,particleSize,sizeVariance,movingsource,theTexture,blend)
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
void BouncyParticles::SetParticleDefaults(Particle& p)
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

void BouncyParticles::Tick()
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

bool BouncyParticles::doesSegmentCrossPlane(Vector& planeP, Vector& planeN, Vector& lineStart, Vector& lineEnd, Vector& intersectionPoint)
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