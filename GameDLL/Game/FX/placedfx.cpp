//================================================================================
// Various FX Entities
//================================================================================
#include "stdafx.h"
#include "placedfx.h"
#include "particles.h"
#include "misc\miscActors.h"
#include "fx\LensFlare.h"

Texture BansheeExplosion::explosionCenter;
Texture BansheeExplosion::explosionChunk;
Texture BansheeExplosion::explosionGlow;
Sound BansheeExplosion::explosionSound;

Texture TinyExplosion::explosionChunk;
Sound TinyExplosion::explosionSound;

Texture BouncySparks::sparkTex;
Texture BouncySparks::sparkGlowTex;

Texture StarshipDamageSparks::sparkTex;

Texture BouncyDirt::myTex;
Texture BouncyMetalChunks::myTex;

Texture SmallImpactSmoke::tex;
Texture BigSmoke::tex;
Texture StarshipDamageSmoke::tex;
Texture EngineSmoke::tex;
Texture TinyDirt::tex;
Texture BigDirt::tex;
Texture DirtBlastChunks::tex;

Texture HeadLamp::lightRing;
Texture HeadLight::lightRing;

Texture LEVELEFFECT_ZenReactorBall::tex;
Texture LEVELEFFECT_ZenReactorBall::particleTex;


Texture DecalExpMetal::myTex;
Texture DecalExpDirt::myTex;
Texture DecalBulletMetal::myTex;
Texture DecalBulletDirt::myTex;
Texture DecalShotgunMetal::myTex;
Texture DecalShotgunDirt::myTex;

Texture DecalBloodSplat::myTex;
Texture BloodSplatter::myTex;

Texture ZenDecalBloodSplat::myTex;
Texture ZenBloodSplatter::myTex;

Texture BloodSpraySprite::tex;
Texture ZenBloodSpraySprite::tex;

Texture LEVELEFFECT_SmokeStream::particleTex;
Texture LEVELEFFECT_BigSmokeHaze::particleTex;

Texture SharkEngineExhaust::myTex;

Texture BansheeEngineExhaust::myTex;

Texture CustomSmoke::tex;

BansheeEngineExhaust::BansheeEngineExhaust(World* world) : ParticleSystem(world,0,100,100,Location,Location,Location,2.63,.273,.83,600,125,460,.721,.2,true,&myTex)
{
	if(!myTex.IsLoaded())myTex.Load("particle.dds");
	opacityFactor = 0;
}
void BansheeEngineExhaust::Tick()
{
	for(int i=0;i<numAlive;i++)
	{
		Particle& par = particles[i];
		par.life -= GDeltaTime*1000;

		// If particle is dead, create fresh particle
		if(par.life <= 250)SetParticleDefaults(par);

		if(par.life < 630)
		{
			float newSize = par.width - 1.335 * GDeltaTime;
			if(newSize < 0)newSize = 0;
			par.height = par.width = newSize;
		}

		if(par.life < 580 && par.life > 560)par.color = COLOR_RGBA((int)(139.0*opacityFactor),(int)(213.0*opacityFactor),(int)(212.0*opacityFactor),(int)(30.0*opacityFactor));
		else if(par.life < 450)par.color = COLOR_RGBA(0,(int)(126.0*opacityFactor),(int)(255.0*opacityFactor),(int)(44.0*opacityFactor));

		// Fade out over the last second(s) of life by changing the particle's diffuse color
		if(par.life < timefadeout)
		{
			COLOR c = par.color;
			int a = par.life/timefadeout*62.0*opacityFactor;
			par.color = COLOR_RGBA(COLOR_GETRED(c),COLOR_GETGREEN(c),COLOR_GETBLUE(c),a);
		}

		par.position += par.velocity*GDeltaTime;
	}
}
void BansheeEngineExhaust::SetParticleDefaults(Particle& p)
{
	p.position	= Vector(0,0,0);					// set particle position to system center
	p.life		= lifetime + ((rand()%100)/100.0f)*lifetimevariance;			
	p.color		= COLOR_RGBA((int)(139.0*opacityFactor),(int)(213.0*opacityFactor),(int)(212.0*opacityFactor),(int)(30.0*opacityFactor));// original colors at full intensity
	Vector randDir= Vector(((rand()%100)/100.0f)*variance, ((rand()%100)/100.0f)*variance,((rand()%100)/100.0f)*variance) - Vector(((rand()%100)/100.0f)*variance, ((rand()%100)/100.0f)*variance,((rand()%100)/100.0f)*variance);
	p.velocity	= (randDir  +  Vector(0,0,-1)).Normalized() * (speed + ((rand()%100)/100.0f)*speedvariance);
	p.width = p.height = particlesize - ((rand()%100)/100.0f)*sizevariance;
}

void BansheeEngineExhaust::PostRender(Camera& cam)
{
	/*if(opacityFactor == 0 || numAlive == 0)return;

	assert(particles.size()<=maxParticles); // Paranoia - make sure we haven't exceeded the max capacity

	//	boundingBox.Reset();

	// Calculate billboard data for a camera-facing quad using the view matrix
	Matrix mat = cam.view;
	Vector rightVect(mat[0][0],mat[1][0],mat[2][0]);
	Vector upVect(mat[0][1],mat[1][1],mat[2][1]);

	Vector rotRight = Rotation.GetRight();
	Vector rotUp = Rotation.GetUp();
	Vector rotDir = Rotation.GetDir();

	// Build the quads for every single particle, and put them in two big lists (vertices, indices)
	for(int i=0;i<numAlive;i++)
	{
	if(COLOR_GETALPHA(particles[i].color) > 0)
	{
	Vector loc	= Location + rotRight * particles[i].position.x + rotUp*particles[i].position.y + rotDir*particles[i].position.z;
	Vector right = rightVect * particles[i].width;
	Vector up = upVect * particles[i].height;

	COLOR color = particles[i].color;
	LVertex vertices[4];
	vertices[0] = LVertex((loc-right)-up, color, 0.0f, 1.0f); // left top
	vertices[1] = LVertex((loc+right)-up, color, 1.0f, 1.0f); // right top
	vertices[2] = LVertex((loc-right)+up, color, 0.0f, 0.0f); // left bottom
	vertices[3] = LVertex((loc+right)+up, color, 1.0f, 0.0f); // right bottom

	//FXManager::Instance()->addBatchedQuad(texture,blendMode,vertices);
	}
	}*/
}


SharkEngineExhaust::SharkEngineExhaust(World* world) : ParticleSystem(world,0,50,50,Location,Location,Location,2.7,.28,.65,600,125,492,.65,.14,true,&myTex)
{
	if(!myTex.IsLoaded())myTex.Load("particle.dds");
	lastSpawnedSmokeTime = -BIG_NUMBER;
	opacityFactor = 0;
}
void SharkEngineExhaust::Tick()
{
	if(opacityFactor == 1.0 && GSeconds - lastSpawnedSmokeTime > .01)
	{
		(new EngineSmoke(MyWorld,Location))->Velocity = -Rotation.GetDir()*1.4;
		lastSpawnedSmokeTime = GSeconds;
	}
	for(int i=0;i<numAlive;i++)
	{
		Particle& par = particles[i];
		par.life -= GDeltaTime*1000;

		// If particle is dead, create fresh particle
		if(par.life <= 265)SetParticleDefaults(par);

		if(par.life < 630)
		{
			float newSize = par.width - 1.335 * GDeltaTime;
			if(newSize < 0)newSize = 0;
			par.height = par.width = newSize;
		}

		if(par.life < 580 && par.life > 560)par.color = COLOR_RGBA((int)(78.0*opacityFactor),(int)(4.0*opacityFactor),0,(int)(60.0*opacityFactor));
		else if(par.life < 560)par.color = COLOR_RGBA((int)(120.0*opacityFactor),(int)(48.0*opacityFactor),0,(int)(62.0*opacityFactor));

		// Fade out over the last second(s) of life by changing the particle's diffuse color
		if(par.life < timefadeout)
		{
			COLOR c = par.color;
			int a = par.life/timefadeout*20.f*opacityFactor;
			par.color = COLOR_RGBA(COLOR_GETRED(c),COLOR_GETGREEN(c),COLOR_GETBLUE(c),a);
		}

		par.position += par.velocity*GDeltaTime;
	}
}
void SharkEngineExhaust::SetParticleDefaults(Particle& p)
{
	p.position	= Vector(0,0,0);					// set particle position to system center
	p.life		= lifetime + ((rand()%100)/100.0f)*lifetimevariance;			
	p.color		= COLOR_RGBA((int)(78.0*opacityFactor),(int)(4.0*opacityFactor),0,(int)(20.0*opacityFactor));// original colors at full intensity
	Vector randDir= Vector(((rand()%100)/100.0f)*variance, ((rand()%100)/100.0f)*variance,((rand()%100)/100.0f)*variance) - Vector(((rand()%100)/100.0f)*variance, ((rand()%100)/100.0f)*variance,((rand()%100)/100.0f)*variance);
	p.velocity	= (randDir  +  Vector(0,0,-1)).Normalized() * (speed + ((rand()%100)/100.0f)*speedvariance);
	p.width = p.height = particlesize - ((rand()%100)/100.0f)*sizevariance;
}

void SharkEngineExhaust::PostRender(Camera& cam)
{
	/*if(opacityFactor == 0 || numAlive == 0)return;

	assert(particles.size()<=maxParticles); // Paranoia - make sure we haven't exceeded the max capacity

	//	boundingBox.Reset();

	// Calculate billboard data for a camera-facing quad using the view matrix
	Matrix mat = cam.view;
	Vector rightVect(mat[0][0],mat[1][0],mat[2][0]);
	Vector upVect(mat[0][1],mat[1][1],mat[2][1]);

	Vector rotRight = Rotation.GetRight();
	Vector rotUp = Rotation.GetUp();
	Vector rotDir = Rotation.GetDir();

	// Build the quads for every single particle, and put them in two big lists (vertices, indices)
	for(int i=0;i<numAlive;i++)
	{
	if(COLOR_GETALPHA(particles[i].color) > 0)
	{
	Vector loc	= Location + rotRight * particles[i].position.x + rotUp*particles[i].position.y + rotDir*particles[i].position.z;
	Vector right = rightVect * particles[i].width;
	Vector up = upVect * particles[i].height;

	COLOR color = particles[i].color;
	LVertex vertices[4];
	vertices[0] = LVertex((loc-right)-up, color, 0.0f, 1.0f); // left top
	vertices[1] = LVertex((loc+right)-up, color, 1.0f, 1.0f); // right top
	vertices[2] = LVertex((loc-right)+up, color, 0.0f, 0.0f); // left bottom
	vertices[3] = LVertex((loc+right)+up, color, 1.0f, 0.0f); // right bottom

	//FXManager::Instance()->addBatchedQuad(texture,blendMode,vertices);
	}
	}*/
}
BansheeExplosion::~BansheeExplosion()
{
	if(glowCorona)glowCorona->LifeTime = 0;
	if(lensFlare)lensFlare->LifeTime = 0;
}
void BansheeExplosion::PostRender(Camera& cam)
{
	Canvas* canvas = RenderDevice::Instance()->GetCanvas();
	if(LifeTime > .35)
	{
		canvas->RotatedBillBoard(Location,Location,1.9f*(1.0 - LifeTime/.7)+1.8f,GSeconds*140.0,COLOR_RGBA(255,255,255,(int)(220.0*((LifeTime - .35)/.35))),&explosionCenter,BLEND_SRCALPHA,BLEND_ONE);
		canvas->RotatedBillBoard(Location,Location,2.2f*(1.0 - LifeTime/.7)+2.1f,-GSeconds*175.0,COLOR_RGBA(255,255,255,(int)(220.0*((LifeTime - .35)/.35))),&explosionCenter,BLEND_SRCALPHA,BLEND_ONE);
	}
	if(LifeTime < .5)glowCorona->opacity = 255*(LifeTime/.5);
	lensFlare->opacityFactor = .54*((LifeTime - .4)/.3);
	if(lensFlare->opacityFactor < 0)lensFlare->opacityFactor = 0;
}

TinyExplosion::TinyExplosion(World* world,Vector& theLocation,Actor* Creator) : FXSystem(world)
{
	if(!explosionChunk.IsLoaded())
	{
		explosionChunk.Load("exp-chunk.dds");
		explosionSound.Load("explosion01.wav");
	}
	Location = theLocation;
	int team;
	if(Creator && Creator->IsSyncedActor())team = ((SynchedActor*)Creator)->team;
	else team = 0;
	if(world->m_IsServer)
	{
		for(int i = 0; i < Destroyable::DestroyablesInWorld.size(); i++)
		{
			float distance = (Location - Destroyable::DestroyablesInWorld[i]->Location).Length();
			if(Destroyable::DestroyablesInWorld[i]->MyModel)
				distance -= 1.2f*Destroyable::DestroyablesInWorld[i]->CollisionBox.max.Length();
			if(distance < .1)distance = .1;
			if(distance < 7)
			{
				Vector momentum = (Destroyable::DestroyablesInWorld[i]->Location - Location).Normalized()*((1-distance/4.5)*14.1) + Vector(0,(1-distance/4.5)*7.0,0);
				if(momentum.Length() > 16.0)momentum = momentum.Normalized()*16.0;

				if(distance > 3.5) Destroyable::DestroyablesInWorld[i]->SERVERtakeDamage((1 - (distance - 3.5)/3.5f)*42.f,Creator,team,Location,momentum,DAMAGETYPE_EXPLOSION);
				else Destroyable::DestroyablesInWorld[i]->SERVERtakeDamage(45,Creator,team,Location,momentum,DAMAGETYPE_EXPLOSION);
			}
		}
	}
	lensFlare = 0;
	doCamDistanceSpawnCheck();
	if(LifeTime != 0)
	{
		LifeTime = .7f;
		ParticleSystem* ps2;
		ps2 = new ParticleSystem(MyWorld,0,40,40,Location,Vector(0,0,0),Vector(0,0,0),1.2f,1.0f,.4f,700,0,300.0f,0.66f,0.31f,false,&explosionChunk);
		ps2->StopSpawningNow();
		ps2->maxAlpha = 40;
		ps2->LifeTime = .7;
		for(int i=0;i<4;i++)
		{
			Vector randVec = Vector(RANDF(),RANDF(),RANDF());
			(new BigSmoke(MyWorld,Location + randVec))->Velocity = 2*randVec;
		}
		explosionSound.Play(Location,Vector(0,0,0),SOUND_RANGE_EXPLOSION,NONE,1.0);
		lensFlare = new LensFlare(MyWorld,Location);
		lensFlare->opacityFactor = .4;
	}
}

TinyExplosion::~TinyExplosion()
{
	if(lensFlare)lensFlare->LifeTime = 0;
}
void TinyExplosion::PostRender(Camera& cam)
{
	lensFlare->opacityFactor = .4*((LifeTime - .4)/.3);
	if(lensFlare->opacityFactor < 0)lensFlare->opacityFactor = 0;
}

BansheeExplosion::BansheeExplosion(World* world,Vector& theLocation,Actor* Creator) : FXSystem(world)
{
	if(!explosionCenter.IsLoaded())
	{
		explosionCenter.Load("EXP-banshee.dds");
		explosionChunk.Load("exp-chunk.dds");
		explosionGlow.Load("exp-bansheeGLOW.dds");
		explosionSound.Load("explosion01.wav");
	}

	Location = theLocation;
	int team;
	if(Creator && Creator->IsSyncedActor())team = ((SynchedActor*)Creator)->team;
	else team = 0;

	if(world->m_IsServer)
	{
		for(int i = 0; i < Destroyable::DestroyablesInWorld.size(); i++)
		{
			float distance = (Location - Destroyable::DestroyablesInWorld[i]->Location).Length();
			if(Destroyable::DestroyablesInWorld[i]->MyModel)
				distance -= Destroyable::DestroyablesInWorld[i]->CollisionBox.max.Length();
			if(distance < .1)distance = .1;
			if(distance < 12.1)
			{
				Vector momentum = (Destroyable::DestroyablesInWorld[i]->Location - Location).Normalized()*((1-distance/12.1)*50.1) + Vector(0,(1-distance/12.1)*34.0,0);
				if(momentum.Length() > 26.0)momentum = momentum.Normalized()*26.0;

				if(distance > 6.0) Destroyable::DestroyablesInWorld[i]->SERVERtakeDamage((1-(distance - 6.0)/6.1)*67.0f,Creator,team,Location,momentum,DAMAGETYPE_EXPLOSION);
				else Destroyable::DestroyablesInWorld[i]->SERVERtakeDamage(85,Creator,team,Location,momentum,DAMAGETYPE_EXPLOSION);
			}
		}
	}
	glowCorona = 0;
	lensFlare = 0;
	doCamDistanceSpawnCheck();
	if(LifeTime != 0)
	{
		LifeTime = .7f;

		ParticleSystem* ps2;

		ps2 = new ParticleSystem(MyWorld,0,30,30,Location,Vector(0,0,0),Vector(0,0,0),3.7f,1.0f,2.2f,380.0f,120.0f,224.0f,1.f,0.1f,false,&explosionChunk);
		ps2->StopSpawningNow();
		ps2->maxAlpha = 24;
		ps2->LifeTime = .5;
		for(int i=0;i<4;i++)
		{
			ps2 = new ParticleSystem(MyWorld,0,30,30,Location + Vector(rand()%30,rand()%30,rand()%30)/33.0f,Vector(0,0,0),Vector(0,0,0),3.6f,1.0f,2.2f,380.0f,120.0f,224.0f,0.95f,0.22f,false,&explosionChunk);
			ps2->StopSpawningNow();
			ps2->maxAlpha = 24;
			ps2->LifeTime = .5;
		}

		int randAmount = 1.0;
		for(int i=0;i<13;i++)
		{
			Vector randVec = Vector(((rand()%200/100.f)-1.0f)*randAmount,((rand()%200/100.f)-1.0f)*randAmount,((rand()%200/100.f)-1.0f)*randAmount);
			BigSmoke* smoke = new BigSmoke(MyWorld,Location + randVec);
			smoke->Velocity = randVec;
		}

		Light* l = new Light(MyWorld,FloatColor(.8,.4,0,1),1.2f);
		l->Location = Location;
		l->AddKeyframe(LightState(FloatColor(1,.7,0,1),13),170);
		l->AddKeyframe(LightState(FloatColor(.26,.13,0,1),6),700);
		l->LifeTime = .7f;

		explosionSound.Play(Location,Vector(0,0,0),SOUND_RANGE_EXPLOSION,NONE,1.0);

		glowCorona = new Corona(world,Location,&explosionGlow,3.0f);

		CollisionInfo info;
		if(MyWorld->CollisionCheckRay(NULL,Location,Location+Vector(0,-1.5f,0),CHECK_GEOMETRY,info))
		{
			MaterialHelper::DoImpactFX(MyWorld,info.mat,info.point,info.normal,0,MaterialHelper::MATERIAL_IMPACTFX_EXPLOSIVE,info.touched);
		}
		if(MyWorld->CollisionCheckRay(NULL,Location,Location+Vector(0,1.5f,0),CHECK_GEOMETRY,info))
		{
			MaterialHelper::DoImpactFX(MyWorld,info.mat,info.point,info.normal,0,MaterialHelper::MATERIAL_IMPACTFX_EXPLOSIVE,info.touched);
		}
		lensFlare = new LensFlare(MyWorld,Location);
		lensFlare->opacityFactor = .54;
	}
}
void TinySmoke::PostRender(Camera &cam)
{
	Canvas* canvas = RenderDevice::Instance()->GetCanvas();

	if(LifeTime > 1.0)canvas->BillBoard(Location,size + size*(1-(LifeTime/2.0)),COLOR_RGBA(255,255,255,100),tex,BLEND_SRCALPHA,BLEND_INVSRCALPHA);
	else if(LifeTime > 0) canvas->BillBoard(Location,size + size*(1-(LifeTime/2.0)),COLOR_RGBA(255,255,255,100 - (int)(100*(1-LifeTime))),tex,BLEND_SRCALPHA,BLEND_INVSRCALPHA);
}

Texture ZenHandGrenadeGlowTrail::myTex;
void ZenHandGrenadeGlowTrail::PostRender(Camera &cam)
{
	RenderDevice::Instance()->GetCanvas()->BillBoard(Location,size,COLOR_RGBA(255,255,255,(int)(125.f*LifeTime)),&myTex,BLEND_SRCALPHA,BLEND_ONE);
}

Texture ZenHandGrenadeStuckGlow::myTex;
void ZenHandGrenadeStuckGlow::PostRender(Camera &cam)
{
	RenderDevice::Instance()->GetCanvas()->BillBoard(Location,size,COLOR_RGBA(255,255,255,(int)(65.f*LifeTime/.4f)),&myTex,BLEND_SRCALPHA,BLEND_ONE);
}


void CustomSmoke::PostRender(Camera &cam)
{
	Canvas* canvas = RenderDevice::Instance()->GetCanvas();
	canvas->BillBoard(Location,StartScale + AddScale*(1.f-(LifeTime/MaxLifeTime)),COLOR_RGBA(255,255,255,(int)(MaxAlpha*(LifeTime/MaxLifeTime))),&tex,BLEND_SRCALPHA,BLEND_INVSRCALPHA);
}

void SmallGunSmoke::PostRender(Camera &cam)
{
	Canvas* canvas = RenderDevice::Instance()->GetCanvas();
	canvas->BillBoard(Location,size + .257*(1-LifeTime),COLOR_RGBA(255,255,255,(int)(74.0*LifeTime)),tex,BLEND_SRCALPHA,BLEND_INVSRCALPHA);
}

void AttachedGunSmoke::PostRender(Camera &cam)
{
	SynchedActor* synchedActor = GEngine.GetActorFromID(attachedToActorID);
	if(synchedActor)
	{
		Canvas* canvas = RenderDevice::Instance()->GetCanvas();
		canvas->BillBoard(synchedActor->Location + synchedActor->Rotation.GetDir()*dirOffset + synchedActor->Rotation.GetUp()*upOffset + synchedActor->Rotation.GetRight()*rightOffset,startSize + growSize*(1.0 - LifeTime/startLifeTime),COLOR_RGBA(255,255,255,(int)(startAlpha*LifeTime/startLifeTime)),tex,BLEND_SRCALPHA,BLEND_INVSRCALPHA);
	}
}
void SmallImpactSmoke::PostRender(Camera &cam)
{
	Canvas* canvas = RenderDevice::Instance()->GetCanvas();
	canvas->BillBoard(Location,.122 + .152*(1-LifeTime),COLOR_RGBA(255,255,255,(int)(175.0*LifeTime)),&tex,BLEND_SRCALPHA,BLEND_INVSRCALPHA);
}
void BigSmoke::PostRender(Camera &cam)
{
	RenderDevice::Instance()->GetCanvas()->BillBoard(Location,1.775 + 2.15*(1-(LifeTime/3.0)),COLOR_RGBA(255,255,255,(int)(160.0*(LifeTime/3.0))),&tex,BLEND_SRCALPHA,BLEND_INVSRCALPHA);
}
void BloodSpraySprite::PostRender(Camera &cam)
{
	RenderDevice::Instance()->GetCanvas()->BillBoard(Location,size + 1.77f*size*(1- LifeTime/.7),COLOR_RGBA(255,255,255,(int)(255.0*LifeTime/.7)),&tex,BLEND_SRCALPHA,BLEND_INVSRCALPHA);
}
void ZenBloodSpraySprite::PostRender(Camera &cam)
{
	RenderDevice::Instance()->GetCanvas()->BillBoard(Location,size + 1.77f*size*(1- LifeTime/.7),COLOR_RGBA(255,255,255,(int)(255.0*LifeTime/.7)),&tex,BLEND_SRCALPHA,BLEND_INVSRCALPHA);
}

void StarshipDamageSmoke::PostRender(Camera &cam)
{
	RenderDevice::Instance()->GetCanvas()->BillBoard(Location,size + 2*size*(1-(LifeTime/3.0)),COLOR_RGBA(255,255,255,(int)(140.0*(LifeTime/3.0))),&tex,BLEND_SRCALPHA,BLEND_INVSRCALPHA);
}


/*void TinyDirt::PostRender(Camera &cam)
{
Canvas* canvas = RenderDevice::Instance()->GetCanvas();
if(LifeTime>.6)GEngine.miscFunctions.DrawSimpleBillboard(cam.view,canvas,&tex,Location,11.0,8.0 + 20.0*(1.0-(LifeTime/.8)),COLOR_RGBA(255,255,255,255),BLEND_SRCALPHA,BLEND_INVSRCALPHA);
else if(LifeTime > .4) GEngine.miscFunctions.DrawSimpleBillboard(cam.view,canvas,&tex,Location,11.0,8.0 + 20.0*(1.0-(LifeTime/.8)),COLOR_RGBA(255,255,255,(int)(255.0*((LifeTime - .4)/.2))),BLEND_SRCALPHA,BLEND_INVSRCALPHA);
}*/

void BigDirt::PostRender(Camera &cam)
{
	Canvas* canvas = RenderDevice::Instance()->GetCanvas();
	canvas->BillBoard(Location,size + size*scaleFactor*(1-LifeTime),COLOR_RGBA(255,255,255,(int)(140.0*LifeTime)),&tex,BLEND_SRCALPHA,BLEND_INVSRCALPHA);
}
DirtBlastChunks::DirtBlastChunks(World* world,Vector& location,Vector& direction) : FXSystem(world)
{
	if(!tex.IsLoaded())tex.Load("dirtchunk.dds");
	Location = location;
	LifeTime = .75f;
	particleSys = new ParticleSystem(MyWorld,0,90,90,location,direction,Vector(0,-15,0),11,.457,5.4f,550.0,200.0,200.0,1.4f,0.9f,false,&tex,BLEND_INVSRCALPHA,true);
	particleSys->StopSpawningNow();
	particleSys->LifeTime = .75;
}

HeadLamp::HeadLamp(World* world,Player* Owner) : FXSystem(world)
{
	if(!lightRing.IsLoaded())lightRing.Load("lightring.dds");
	light = new Light(world,FloatColor(.988,.967,.847,1.0),7.42);
	isOn = false;
	owner = Owner;
	opacityFactor = 0;
}
void HeadLamp::PostRender(Camera& cam)
{		
	if(opacityFactor > 0)
	{
		float ScalingFactor = .02;
		Canvas* canvas = RenderDevice::Instance()->GetCanvas();
		for(float i = 0; i < 41; i++)
		{
			canvas->BillBoard(startPos + Dir*(i*(i/6.0)*ScalingFactor),3.0*ScalingFactor + (53.0/40.0)*i*ScalingFactor,COLOR_RGBA(255,255,255,(int)((34.0 - 34.0*(i/40.0))*opacityFactor)),&lightRing,BLEND_SRCALPHA,BLEND_ONE);
		}
	}
}
void HeadLamp::Tick()
{
	if(isOn)
	{
		opacityFactor += GDeltaTime;
		if(opacityFactor > 1.0)opacityFactor = 1.0;
	}
	else
	{
		opacityFactor -= GDeltaTime;
		if(opacityFactor < 0)opacityFactor = 0;
	}
	if(opacityFactor <= 0)light->IsHidden = true;
	else
	{
		light->IsHidden = false;
		CollisionInfo info;
		if(MyWorld->CollisionCheckRay(owner->getOwnerToIgnore(),startPos,startPos + unconstrainedDir*45.45,CHECK_EVERYTHING,info))
		{
			light->Location = info.point - unconstrainedDir*3.79;
		}
		else light->Location = startPos + unconstrainedDir*45.45;
		light->Tick();
		light->SetColor(FloatColor(.988*opacityFactor,.967*opacityFactor,.847*opacityFactor,1.0));
	}
}

void ZenHeadLamp::PostRender(Camera& cam)
{
	if(opacityFactor > 0)
	{
		float ScalingFactor = .02;
		Canvas* canvas = RenderDevice::Instance()->GetCanvas();
		for(float i = 0; i < 41; i++)
		{
			canvas->BillBoard(startPos + Dir*(i*(i/6.0)*ScalingFactor),3.0*ScalingFactor + (53.0/40.0)*i*ScalingFactor,COLOR_RGBA(30,255,72,(int)((34.0 - 34.0*(i/40.0))*opacityFactor)),&lightRing,BLEND_SRCALPHA,BLEND_ONE);
		}
	}
}
void ZenHeadLamp::Tick()
{
	if(isOn)
	{
		opacityFactor += GDeltaTime;
		if(opacityFactor > 1.0)opacityFactor = 1.0;
	}
	else
	{
		opacityFactor -= GDeltaTime;
		if(opacityFactor < 0)opacityFactor = 0;
	}
	if(opacityFactor <= 0)light->IsHidden = true;
	else
	{
		light->IsHidden = false;
		CollisionInfo info;
		if(MyWorld->CollisionCheckRay(owner->getOwnerToIgnore(),startPos,startPos + unconstrainedDir*45.45,CHECK_EVERYTHING,info))
		{
			light->Location = info.point - unconstrainedDir*3.79;
		}
		else light->Location = startPos + unconstrainedDir*45.45;
		light->Tick();
		light->SetColor(FloatColor(.12f*opacityFactor,opacityFactor,.28f*opacityFactor,1.0));
	}
}
HeadLight::HeadLight(World* world,Actor* Owner) : FXSystem(world)
{
	if(!lightRing.IsLoaded())lightRing.Load("lightring.dds");
	isOn = false;
	owner = Owner;
	opacityFactor = 0;
}
void HeadLight::PostRender(Camera& cam)
{		
	if(opacityFactor > 0)
	{
		float ScalingFactor = .02;
		Canvas* canvas = RenderDevice::Instance()->GetCanvas();
		for(float i = 0; i < 41; i++)
		{
			canvas->BillBoard(startPos + Dir*(i*(i/6.0)*ScalingFactor),3.0*ScalingFactor + (53.0/40.0)*i*ScalingFactor,COLOR_RGBA(255,255,255,(int)((34.0 - 34.0*(i/40.0))*opacityFactor)),&lightRing,BLEND_SRCALPHA,BLEND_ONE);
		}
	}
}
void HeadLight::Tick()
{
	if(isOn)
	{
		opacityFactor += GDeltaTime;
		if(opacityFactor > 1.0)opacityFactor = 1.0;
	}
	else
	{
		opacityFactor -= GDeltaTime;
		if(opacityFactor < 0)opacityFactor = 0;
	}
}

void FadingSmoke::PostRender(Camera& cam)
{
	Canvas* canvas = RenderDevice::Instance()->GetCanvas();
	if(LifeTime > 2.0)canvas->BillBoard(Location,size + (size/2.0)*(1-((LifeTime)/4.0)),COLOR_RGBA(255,255,255,54),tex,BLEND_SRCALPHA,BLEND_INVSRCALPHA);
	else if(LifeTime > 0) canvas->BillBoard(Location,size + (size/2.0)*(1-(LifeTime/4.0)),COLOR_RGBA(255,255,255,54 - (int)(54.f*(1-(LifeTime/2.0)))),tex,BLEND_SRCALPHA,BLEND_INVSRCALPHA);
}


EngineSmoke::EngineSmoke(World* world,Vector& location) : FXSystem(world)
{
	if(!tex.IsLoaded())tex.Load("tinySmoke.dds");
	doCamDistanceSpawnCheck();
	isSorted = true;
	Location = location;
	LifeTime = 1.2f;
	size = (21.f + rand()%28)/33.f;
}
void EngineSmoke::PostRender(Camera& cam)
{
	Canvas* canvas = RenderDevice::Instance()->GetCanvas();
	if(LifeTime > .7)canvas->BillBoard(Location,size + (size/1.3)*(1-((LifeTime)/1.2)),COLOR_RGBA(255,255,255,50),&tex,BLEND_SRCALPHA,BLEND_INVSRCALPHA);
	else if(LifeTime > 0) canvas->BillBoard(Location,size + (size/1.3)*(1-(LifeTime/1.2)),COLOR_RGBA(255,255,255,50 - (int)(50*(1-(LifeTime/.7)))),&tex,BLEND_SRCALPHA,BLEND_INVSRCALPHA);
}

void LEVELEFFECT_ZenReactorBall::PostRender(Camera& cam)
{
	RenderDevice::Instance()->GetCanvas()->BillBoard(Location,7.182 + 1.46 * RANDF(),COLOR_RGBA((int)(70.0 + 185.0*abs(cos(GSeconds*3.4))),255,255,(int)(100.0 + 155.0*abs(sin(GSeconds*2.2)))),&tex,BLEND_SRCALPHA,BLEND_ONE);
}
LEVELEFFECT_ZenReactorBall::LEVELEFFECT_ZenReactorBall(World* world,Vector& loc) : FXSystem(world)
{
	if(!tex.IsLoaded()){tex.Load("so_zenergyball.dds");particleTex.Load("lfshot_impact.dds");}
	Location = loc;
	particleSys = new ParticleSystem(world,100,0,265,Location,Vector(0,0,0),Vector(0,7,0),5,1.0,4.4f,400,180,300,1.9f,0.424f,false,&particleTex);
}

DecalExpMetal::DecalExpMetal(World* world,Vector& pos,Vector& dir) : SurfaceDecal(world,&myTex,pos,dir,1.6f,100.0,20.0,BLEND_INVSRCALPHA)
{
	if(!myTex.IsLoaded())myTex.Load("decal_expmetal.dds");
}
DecalExpDirt::DecalExpDirt(World* world,Vector& pos,Vector& dir) : SurfaceDecal(world,&myTex,pos,dir,1.6f,100.0,20.0,BLEND_INVSRCALPHA)
{
	if(!myTex.IsLoaded())myTex.Load("decal_expdirt.dds");
}
DecalBulletMetal::DecalBulletMetal(World* world,Vector& pos,Vector& dir) : SurfaceDecal(world,&myTex,pos,dir,.02 + RANDF()/33.0f,50,10.0,BLEND_INVSRCALPHA)
{
	if(!myTex.IsLoaded())myTex.Load("decal_bulletmetal.dds");
}
DecalBulletDirt::DecalBulletDirt(World* world,Vector& pos,Vector& dir) : SurfaceDecal(world,&myTex,pos,dir,.0285 + .0364 * RANDF(),50,10.0,BLEND_INVSRCALPHA)
{
	if(!myTex.IsLoaded())myTex.Load("decal_bulletdirt.dds");
}
DecalShotgunMetal::DecalShotgunMetal(World* world,Vector& pos,Vector& dir) : SurfaceDecal(world,&myTex,pos,dir,.121 + .182 *RANDF(),50,10.0,BLEND_INVSRCALPHA)
{
	if(!myTex.IsLoaded())myTex.Load("decal_shotgunmetal.dds");
}
DecalShotgunDirt::DecalShotgunDirt(World* world,Vector& pos,Vector& dir) : SurfaceDecal(world,&myTex,pos,dir, .061 + .091 *RANDF(),50,10.0,BLEND_INVSRCALPHA)
{
	if(!myTex.IsLoaded())myTex.Load("decal_shotgundirt.dds");
}
DecalBloodSplat::DecalBloodSplat(World* world,Vector& pos,Vector& dir, float setSize) : SurfaceDecal(world,&myTex,pos,dir,setSize,60.0,20.0,BLEND_INVSRCALPHA)
{
	if(!myTex.IsLoaded())myTex.Load("bloodspurt.dds");
}
BloodSplatter::BloodSplatter(World* world,Actor* ignoreactor,Vector& location) : ParticleSystem(world,0,5,5,location,Vector(0,.3,0),Vector(0,-15.15,0),3.6,.95f,2.58,700.0f,100.0f,150.0f,.35,.213,false,&myTex,BLEND_INVSRCALPHA)
{
	if(!myTex.IsLoaded())myTex.Load("bloodspurt.dds");
	ignoreActor = ignoreactor;
	Location = location;
	particleRotationSpeed = 14.0;
	StopSpawningNow();
	LifeTime = .7;
	doCamDistanceSpawnCheck();
}

//-----------------------------------------
// Update the state of all the particles
//-----------------------------------------
void BloodSplatter::Tick(){
	if(GEngine.preferences.doHeavyFXsystems)
	{
		// Add new particles every 'msSpawn' milliseconds if we're not at max already
		msCount += GDeltaTime*1000;
		spawncountdown -= GDeltaTime;
		if(countingdownspawn && spawncountdown < 0) stopspawning = 1;
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
				CollisionInfo result;
				if(MyWorld->CollisionCheckRay(ignoreActor,par.oldPos,par.position,CHECK_GEOMETRY,result))
				{
					par.life = 0;
					new DecalBloodSplat(MyWorld,result.point,result.normal);
				}
				par.oldPos = par.position;
				if(par.hasVelocity)par.velocity += par.acceleration*GDeltaTime;
			}
			if(par.life <= 0 && stopspawning) par.color = COLOR_RGBA(0,0,0,0);
		}
	}
}


ZenDecalBloodSplat::ZenDecalBloodSplat(World* world,Vector& pos,Vector& dir,float setSize) : SurfaceDecal(world,&myTex,pos,dir,setSize,60.0,20.0,BLEND_INVSRCALPHA)
{
	if(!myTex.IsLoaded())myTex.Load("zenbloodspurt.dds");
}
ZenBloodSplatter::ZenBloodSplatter(World* world,Actor* ignoreactor,Vector& location) : ParticleSystem(world,0,5,5,location,Vector(0,.3,0),Vector(0,-15.15,0),3.6,.95f,2.58,700.0f,100.0f,150.0f,.35,.213,false,&myTex,BLEND_INVSRCALPHA)
{
	if(!myTex.IsLoaded())myTex.Load("zenbloodspurt.dds");
	ignoreActor = ignoreactor;
	Location = location;
	particleRotationSpeed = 14.0;
	StopSpawningNow();
	LifeTime = .7;
	doCamDistanceSpawnCheck();
}

//-----------------------------------------
// Update the state of all the particles
//-----------------------------------------
void ZenBloodSplatter::Tick(){
	if(GEngine.preferences.doHeavyFXsystems)
	{
		// Add new particles every 'msSpawn' milliseconds if we're not at max already
		msCount += GDeltaTime*1000;
		spawncountdown -= GDeltaTime;
		if(countingdownspawn && spawncountdown < 0) stopspawning = 1;
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
				CollisionInfo result;
				if(MyWorld->CollisionCheckRay(ignoreActor,par.oldPos,par.position,CHECK_GEOMETRY,result))
				{
					par.life = 0;
					new ZenDecalBloodSplat(MyWorld,result.point,result.normal);
				}
				par.oldPos = par.position;
				if(par.hasVelocity)par.velocity += par.acceleration*GDeltaTime;
			}
			if(par.life <= 0 && stopspawning) par.color = COLOR_RGBA(0,0,0,0);
		}
	}
}
BouncySparks::BouncySparks(World* world,Vector& location,Vector& sparksDir) : FXSystem(world)
{
	if(!sparkTex.IsLoaded())
	{
		sparkTex.Load("spark.dds");
		sparkGlowTex.Load("sparkglow.dds");
	}
	Location = location;
	doCamDistanceSpawnCheck();
	if(LifeTime != 0)
	{
		int num = 3 + rand()%18;
		LaserParticleSystem* particleSys = new LaserParticleSystem(.02 + RANDF()*.035 ,MyWorld,0,num,num,location,sparksDir,Vector(0,-10.0,0),.3 + RANDF()*.8,.59f,3.8,2000.0f,500.0f,1000.0f,.008 + RANDF()*.013,.006,false,true,&sparkTex,&sparkGlowTex);
		particleSys->StopSpawningNow();
		particleSys->LifeTime = 2.5f;
		LifeTime = 0;
	}
}
StarshipDamageSparks::StarshipDamageSparks(World* world,Vector& location,Vector& sparksDir) : FXSystem(world)
{
	if(!sparkTex.IsLoaded())sparkTex.Load("spark.dds");
	Location = location;
	doCamDistanceSpawnCheck();
	if(LifeTime != 0)
	{
		LaserParticleSystem* particleSys = new LaserParticleSystem(.28,MyWorld,0,20,20,location,sparksDir,Vector(0,-18.5f,0),6.5,.59f,4.8,1100.f,0,600.f,.018,.013,false,false,&sparkTex,NULL);
		particleSys->StopSpawningNow();
		particleSys->LifeTime = 1.1f;
		LifeTime = 0;
	}
}

BouncyDirt::BouncyDirt(World* world,Vector& location,Vector& sparksDir) : BouncyParticles(world,0,130,130,location,sparksDir,Vector(0,-19,0),8.5,.33,7.0,4000,1000,1000,.1,.08,false,&myTex,BLEND_INVSRCALPHA)
{
	if(!myTex.IsLoaded())myTex.Load("dirtchunk2.dds");
	LifeTime = 5.0;
	StopSpawningNow();
	doCamDistanceSpawnCheck();
}
BouncyMetalChunks::BouncyMetalChunks(World* world,Vector& location,Vector& sparksDir) : BouncyParticles(world,0,70,70,location,sparksDir,Vector(0,-19,0),4.0,.77,10.0,4000,1000,1000,.06,.05,false,&myTex,BLEND_INVSRCALPHA)
{
	if(!myTex.IsLoaded())myTex.Load("metalchunk.dds");
	LifeTime = 5.0;
	StopSpawningNow();
	doCamDistanceSpawnCheck();
}
LEVELEFFECT_SmokeStream::LEVELEFFECT_SmokeStream(World* world,Vector& loc) : FXSystem(world)
{
	if(!particleTex.IsLoaded())particleTex.Load("tinySmoke.dds");
	Location = loc;
	particleSys = new ParticleSystem(world,50,0,145,loc,Vector(0,1,0),Vector(0,0,0),11,.21,3.0f,4200,0,12000,5,1.0f,false,&particleTex,BLEND_INVSRCALPHA);
}
LEVELEFFECT_BigSmokeHaze::LEVELEFFECT_BigSmokeHaze(World* world,Vector& loc) : FXSystem(world)
{
	if(!particleTex.IsLoaded())particleTex.Load("bigsmoke.dds");
	Location = loc;
	particleSys = new ParticleSystem(world,50,0,90,loc,Vector(0,1,0),Vector(0,0,0),6,.35,2.7f,7000,3000,10000,12,3.0f,false,&particleTex,BLEND_INVSRCALPHA);
}

Texture ForcePushWave::myTex;
ForcePushWave::ForcePushWave(World* world,Vector& Location,Matrix& Rot) : SurfaceDecal(world,&myTex,Location,Vector(0,1,0),.1,1.0,.7)
{
	if(!myTex.IsLoaded())
		myTex.Load("forcepush.dds");
	dir = Rot.GetDir();
	right = Rot.GetRight();
}
void ForcePushWave::Tick()
{
	size = .1 + 12.8f*(1.f -  LifeTime);
	EndRight  = Location + dir*size + right*size;
	EndLeft = Location + dir*size - right*size;
	StartRight = Location - dir*size + right*size;
	StartLeft = Location - dir*size - right*size;
	SurfaceDecal::Tick();
}

void ForcePushWave::PostRender(Camera& cam)
{
	if(!GEngine.preferences.doShotDecals)return;
	LVertex vertices[4];
	WORD indices[6];

	if(alpha > 255.0)alpha=255.0;
	if(alpha < 0)alpha = 0;
	COLOR thecolor;
	thecolor = COLOR_RGBA(255,255,255,(int)alpha);
	vertices[0] = LVertex(StartLeft, thecolor, 0.0f, 1.0f); // left top
	vertices[1] = LVertex(StartRight, thecolor, 1.0f, 1.0f); // right top
	vertices[2] = LVertex(EndLeft, thecolor, 0.0f, 0); // left bottom
	vertices[3] = LVertex(EndRight, thecolor, 1.0f, 0); // right bottom

	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;
	indices[3] = 2;
	indices[4] = 1;
	indices[5] = 3;
	RenderWrap::SetRS( D3DRS_SRCBLEND, BLEND_SRCALPHA); 
	RenderWrap::SetRS( D3DRS_DESTBLEND, dest );;

	RenderWrap::SetTSS( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE);//MODULATE  );//ADDSIGNED );
	RenderWrap::SetTSS( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);//TA_TEXTURE );
	RenderWrap::SetTSS( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE );

	texture->Set(0);

	Canvas* canvas = RenderDevice::Instance()->GetCanvas();

	canvas->SimpleObject(4,&vertices,6,&indices,sizeof(vertices[0]),false);

	/*vertices[0] = LVertex(-StartLeft, thecolor, 0.0f, 1.0f); // left top
	vertices[1] = LVertex(-StartRight, thecolor, 1.0f, 1.0f); // right top
	vertices[2] = LVertex(-EndLeft, thecolor, 0.0f, 0); // left bottom
	vertices[3] = LVertex(-EndRight, thecolor, 1.0f, 0); // right bottom

	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;
	indices[3] = 2;
	indices[4] = 1;
	indices[5] = 3;

	canvas->SimpleObject(4,&vertices,6,&indices,sizeof(vertices[0]),false);*/

	texture->UnSet(0);
}

Texture ForcePullWave::myTex;
ForcePullWave::ForcePullWave(World* world,Vector& Location,Matrix& Rot) : SurfaceDecal(world,&myTex,Location,Vector(0,1,0),.1,1.0,.7)
{
	if(!myTex.IsLoaded())
		myTex.Load("forcepull.dds");
	dir = Rot.GetDir();
	right = Rot.GetRight();
}
void ForcePullWave::Tick()
{
	size = 14 - 14.f*(1.f -  LifeTime);
	EndRight  = Location + dir*size + right*size;
	EndLeft = Location + dir*size - right*size;
	StartRight = Location - dir*size + right*size;
	StartLeft = Location - dir*size - right*size;
	SurfaceDecal::Tick();
}

void ForcePullWave::PostRender(Camera& cam)
{
	if(!GEngine.preferences.doShotDecals)return;
	LVertex vertices[4];
	WORD indices[6];

	if(alpha > 255.0)alpha=255.0;
	if(alpha < 0)alpha = 0;
	COLOR thecolor;
	thecolor = COLOR_RGBA(255,255,255,(int)alpha);
	vertices[0] = LVertex(StartLeft, thecolor, 0.0f, 1.0f); // left top
	vertices[1] = LVertex(StartRight, thecolor, 1.0f, 1.0f); // right top
	vertices[2] = LVertex(EndLeft, thecolor, 0.0f, 0); // left bottom
	vertices[3] = LVertex(EndRight, thecolor, 1.0f, 0); // right bottom

	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;
	indices[3] = 2;
	indices[4] = 1;
	indices[5] = 3;
	RenderWrap::SetRS( D3DRS_SRCBLEND, BLEND_SRCALPHA); 
	RenderWrap::SetRS( D3DRS_DESTBLEND, dest );;

	RenderWrap::SetTSS( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE);//MODULATE  );//ADDSIGNED );
	RenderWrap::SetTSS( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);//TA_TEXTURE );
	RenderWrap::SetTSS( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE );

	texture->Set(0);

	Canvas* canvas = RenderDevice::Instance()->GetCanvas();

	canvas->SimpleObject(4,&vertices,6,&indices,sizeof(vertices[0]),false);

	/*vertices[0] = LVertex(-StartLeft, thecolor, 0.0f, 1.0f); // left top
	vertices[1] = LVertex(-StartRight, thecolor, 1.0f, 1.0f); // right top
	vertices[2] = LVertex(-EndLeft, thecolor, 0.0f, 0); // left bottom
	vertices[3] = LVertex(-EndRight, thecolor, 1.0f, 0); // right bottom

	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;
	indices[3] = 2;
	indices[4] = 1;
	indices[5] = 3;

	canvas->SimpleObject(4,&vertices,6,&indices,sizeof(vertices[0]),false);*/

	texture->UnSet(0);
}

Texture ZenPersonalShieldImpactGlow::myTex;
ZenPersonalShieldImpactGlow::ZenPersonalShieldImpactGlow(SynchedActor* attachedToActor,Vector& posOffset) : SurfaceDecal(attachedToActor->MyWorld,&myTex,Vector(0,0,0),Vector(0,1,0),.1,1.0,.7)
{
	if(!myTex.IsLoaded())myTex.Load("laserblueimpact.dds");
	PosOffset = posOffset;
	//SafeSetPointer(&AttachedToActor,(Actor**)&attachedToActor);
	AttachedToActorID = attachedToActor->actorID;
}
void ZenPersonalShieldImpactGlow::PostRender(Camera& cam)
{
	if(!GEngine.preferences.doShotDecals)return;
	Actor* AttachedToActor = SynchedActor::GetActorFromID(AttachedToActorID);
	if(!AttachedToActor)return;
	LVertex vertices[4];
	WORD indices[6];

	if(LifeTime != -1 && LifeTime < FadeTime)
		alpha = (LifeTime/FadeTime)*200.0f;

	if(alpha > 200.0)alpha=200.0;
	if(alpha < 0)alpha = 0;
	COLOR thecolor;
	thecolor = COLOR_RGBA(255,255,255,(int)alpha);

	setPosDir(.08 + .24f*(1.f -  LifeTime),AttachedToActor->Location + Vector(0,.18,0) + PosOffset,PosOffset);

	vertices[0] = LVertex(StartLeft, thecolor, 0.0f, 1.0f); // left top
	vertices[1] = LVertex(StartRight, thecolor, 1.0f, 1.0f); // right top
	vertices[2] = LVertex(EndLeft, thecolor, 0.0f, 0); // left bottom
	vertices[3] = LVertex(EndRight, thecolor, 1.0f, 0); // right bottom

	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;
	indices[3] = 2;
	indices[4] = 1;
	indices[5] = 3;
	RenderWrap::SetRS( D3DRS_SRCBLEND, BLEND_SRCALPHA); 
	RenderWrap::SetRS( D3DRS_DESTBLEND, dest );;

	RenderWrap::SetTSS( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE);//MODULATE  );//ADDSIGNED );
	RenderWrap::SetTSS( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);//TA_TEXTURE );
	RenderWrap::SetTSS( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE );

	texture->Set(0);
	Canvas* canvas = RenderDevice::Instance()->GetCanvas();
	canvas->SimpleObject(4,&vertices,6,&indices,sizeof(vertices[0]),false);
	/*vertices[0] = LVertex(-StartLeft, thecolor, 0.0f, 1.0f); // left top
	vertices[1] = LVertex(-StartRight, thecolor, 1.0f, 1.0f); // right top
	vertices[2] = LVertex(-EndLeft, thecolor, 0.0f, 0); // left bottom
	vertices[3] = LVertex(-EndRight, thecolor, 1.0f, 0); // right bottom
	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;
	indices[3] = 2;
	indices[4] = 1;
	indices[5] = 3;
	canvas->SimpleObject(4,&vertices,6,&indices,sizeof(vertices[0]),false);*/
	texture->UnSet(0);
}

LEVELEFFECT_ChandelierLight::LEVELEFFECT_ChandelierLight(World* world,Vector& location) : FXSystem(world)
{
	Location = location;
	myLight = new Light(MyWorld,FloatColor(1.f,.79,.59,1),50.f+17.5f*fabsf(sin(GSeconds*1.3)));
	myLight->Location = Location;
	flickeringOut = false;
	opacity=1;
}
void LEVELEFFECT_ChandelierLight::Tick()
{
	if(flickeringOut)
	{
		opacity -= GDeltaTime*.195;
		if(opacity < 0)
			myLight->IsHidden = true;
		else
		{
			myLight->SetColor(FloatColor(1.f*opacity,.79*opacity,.59*opacity,1));
			myLight->SetRange(flickerOutRange*opacity);
			myLight->Location = Location;
		}
	}
	else
	{

		myLight->SetColor(FloatColor(1.f,.79,.59,1));
		myLight->SetRange(50.f+17.5f*fabsf(sin(GSeconds*1.3)));
		myLight->Location = Location;

	}
}
void LEVELEFFECT_ChandelierLight::FlickerOut()
{
	if(!flickeringOut)
	{
		flickeringOut = true;
		flickerOutRange = 50.f+17.5f*fabsf(sin(GSeconds*1.3));
	}
}


LEVELEFFECT_CandelabraLight::LEVELEFFECT_CandelabraLight(World* world,Vector& location) : FXSystem(world)
{
	Location = location;
	myLight = new Light(MyWorld,FloatColor(1.f,.84,.36,1),10.f+3.f*fabsf(sin(GSeconds*1.7f)));
	myLight->Location = Location;
}
void LEVELEFFECT_CandelabraLight::Tick()
{
	myLight->SetColor(FloatColor(1.f,.84,.36,1));
	myLight->SetRange(10.f+3.f*fabsf(sin(GSeconds*1.7f)));
	myLight->Location = Location;
}

LEVELEFFECT_FireLight::LEVELEFFECT_FireLight(World* world,Vector& location) : FXSystem(world)
{
	Location = location;
	myLight = new Light(MyWorld,FloatColor(1.f,.53,0,255.f),10);
	myLight->Location = Location;
}
void LEVELEFFECT_FireLight::Tick()
{
	myLight->SetColor(FloatColor(1.f + -.05*fabsf(cos(-GSeconds*3.7 - PI)),.53 + .09*fabsf(sin(-GSeconds*3 - PI*1.3f)),.02+.08f*fabsf(cos(GSeconds*2 + PI*1.2)),255.f));
	myLight->SetRange(7.4f+2.6f*fabsf(sin(GSeconds/(.8f + .15*fabsf(cos(-GSeconds))))));
}

Texture LEVELEFFECT_MediumFire::myTex;

LEVELEFFECT_MediumFire::LEVELEFFECT_MediumFire(World* world,Vector& location) : ParticleSystem(world,0,77,77,Location,Location,Location,2.f,.35,.85,600,100,270,.721,.2,false,&myTex)
{
	if(!myTex.IsLoaded())myTex.Load("particle.dds");
	Location = location;
	fadeOffset = 2*PI*RANDF();
	windOffset = 2*PI*RANDF();
}
void LEVELEFFECT_MediumFire::Tick()
{
	opacityFactor = .41 + .1*fabsf(sin(GSeconds*1.2f +fadeOffset));
	for(int i=0;i<numAlive;i++)
	{
		Particle& par = particles[i];
		par.life -= GDeltaTime*1000;

		// If particle is dead, create fresh particle
		if(par.life <= 250)SetParticleDefaults(par);

		if(par.life < 630)
		{
			float newSize = par.width - 1.335 * GDeltaTime;
			if(newSize < 0)newSize = 0;
			par.height = par.width = newSize;
		}

		if(par.life < 473 && par.life > 429)par.color = COLOR_RGBA((int)(255.0*opacityFactor),(int)(180.0*opacityFactor),(int)(20.f*opacityFactor),(int)(18.f*opacityFactor));
		else if(par.life < 430)par.color = COLOR_RGBA((int)(240.0*opacityFactor),(int)(90.0*opacityFactor),0,(int)(10.f*opacityFactor));

		// Fade out over the last second(s) of life by changing the particle's diffuse color
		if(par.life < timefadeout)
		{
			COLOR c = par.color;
			int a = par.life/timefadeout*18.f*opacityFactor;
			par.color = COLOR_RGBA(COLOR_GETRED(c),COLOR_GETGREEN(c),COLOR_GETBLUE(c),a);
		}

		par.position += par.velocity*GDeltaTime;
	}
}
void LEVELEFFECT_MediumFire::SetParticleDefaults(Particle& p)
{
	p.position	= Location;					// set particle position to system center
	p.life		= lifetime + ((rand()%100)/100.0f)*lifetimevariance;			
	p.color		= COLOR_RGBA((int)(248.0*opacityFactor),(int)(180.0*opacityFactor),(int)(150.0*opacityFactor),(int)(19.f*opacityFactor));// original colors at full intensity
	Vector randDir= Vector(((rand()%100)/100.0f)*variance, ((rand()%100)/100.0f)*variance,((rand()%100)/100.0f)*variance) - Vector(((rand()%100)/100.0f)*variance, ((rand()%100)/100.0f)*variance,((rand()%100)/100.0f)*variance);
	p.velocity	= (randDir  +  (Vector(0,1,0) + .3f*Vector(sin(GSeconds+windOffset),0,-cos(GSeconds*1.2+windOffset))).Normalized()).Normalized() * (speed + ((rand()%100)/100.0f)*speedvariance);
	p.width = p.height = particlesize - ((rand()%100)/100.0f)*sizevariance;
}

void LEVELEFFECT_MediumFire::PostRender(Camera& cam)
{
	if(!GEngine.preferences.doHeavyFXsystems)return;
	// Calculate billboard data for a camera-facing quad using the view matrix
	Matrix mat = cam.view;
	Vector rightVect(mat[0][0],mat[1][0],mat[2][0]);
	Vector upVect(mat[0][1],mat[1][1],mat[2][1]);

	int batchArray = FXManager::getBatchForWriting(texture,blendMode);
	VertexIndicesBatch* pBatch = &FXManager::vertexIndicesBatches[FXManager::batchedQuads[batchArray].myVertexIndicesBatch];

	int numQuads = FXManager::batchedQuads[batchArray].numQuads;

	Vector loc;
	Vector right;
	Vector up;
	COLOR color;
	for(int i=0;i<numAlive;i++)
	{
		loc	=  particles[i].position;
		right = rightVect * particles[i].width;
		up = upVect * particles[i].height;

		color = particles[i].color;

		if(numQuads > 1499)//QUADS_PER_BATCH-1)
		{
			FXManager::batchedQuads[batchArray].numQuads = numQuads;
			batchArray = FXManager::addNewBatch(texture,blendMode);
			if(batchArray == -1)return;
			pBatch = &FXManager::vertexIndicesBatches[FXManager::batchedQuads[batchArray].myVertexIndicesBatch];
			numQuads = FXManager::batchedQuads[batchArray].numQuads;
		}

		pBatch->theVertices[(numQuads*4)].diffuse = color;
		pBatch->theVertices[(numQuads*4)].position = (loc-right)-up;
		pBatch->theVertices[(numQuads*4)].tu = 0;
		pBatch->theVertices[(numQuads*4)].tv = 1;

		pBatch->theVertices[(numQuads*4)+1].diffuse = color;
		pBatch->theVertices[(numQuads*4)+1].position = (loc+right)-up;
		pBatch->theVertices[(numQuads*4)+1].tu = 1;
		pBatch->theVertices[(numQuads*4)+1].tv = 1;

		pBatch->theVertices[(numQuads*4)+2].diffuse = color;
		pBatch->theVertices[(numQuads*4)+2].position = (loc-right)+up;
		pBatch->theVertices[(numQuads*4)+2].tu = 0;
		pBatch->theVertices[(numQuads*4)+2].tv = 0;

		pBatch->theVertices[(numQuads*4)+3].diffuse = color;
		pBatch->theVertices[(numQuads*4)+3].position = (loc+right)+up;
		pBatch->theVertices[(numQuads*4)+3].tu = 1;
		pBatch->theVertices[(numQuads*4)+3].tv = 0;

		numQuads++;
	}
	FXManager::batchedQuads[batchArray].numQuads = numQuads;
}
Texture LEVELEFFECT_Fire::myTex;

LEVELEFFECT_Fire::LEVELEFFECT_Fire(World* world,Vector& location,float theScale) : ParticleSystem(world,0,70,70,Location,Location,Location,2.f*theScale,.35,.85*theScale,600,125,460,.721*theScale,.2*theScale,false,&myTex)
{
	if(!myTex.IsLoaded())myTex.Load("particle.dds");
	Location = location;
	fadeOffset = 2*PI*RANDF();
	windOffset = 2*PI*RANDF();
	scale = theScale;
	opacityFactor = .68;
	MyCorona = new LEVELEFFECT_Corona(world,Location,"corona.dds",Vector(225,200,80),25,60,100,35,8,false,true,.55);
	//MyHDRglow = new HDRglow(MyWorld,Vector4(1.f,1.f,1,1),theScale*.38,100.f);
	//isFlickeringOut = false;
}
/*void LEVELEFFECT_Fire::FlickerOut()
{
if(!isFlickeringOut)
{
MyHDRglow->LifeTime = 0;
MyHDRglow = 0;
isFlickeringOut = true;
}
}*/
void LEVELEFFECT_Fire::Tick()
{
	/*if(isFlickeringOut)
	{
	opacityFactor -= GDeltaTime*.075;
	if(opacityFactor < 0)IsHidden = true;
	}
	else *///MyHDRglow->Location = Location;
	//opacityFactor = .53 + .35*fabsf(sin(GSeconds*1.2f +fadeOffset));
	MyCorona->Location = Location + Vector(0,.02,0);
	for(int i=0;i<numAlive;i++)
	{
		Particle& par = particles[i];
		par.life -= GDeltaTime*1000;

		// If particle is dead, create fresh particle
		if(par.life <= 250)SetParticleDefaults(par);

		if(par.life < 630)
		{
			float newSize = par.width - 1.335*scale * GDeltaTime;
			if(newSize < 0)newSize = 0;
			par.height = par.width = newSize;
		}

		if(par.life < 509 && par.life >= 440)par.color = COLOR_RGBA((int)(250.0*opacityFactor),(int)(225.0*opacityFactor),(int)(160.0*opacityFactor),(int)(21.0*opacityFactor));
		else if(par.life < 440)par.color = COLOR_RGBA((int)(255.0*opacityFactor),(int)(125.0*opacityFactor),50,(int)(13.0*opacityFactor));

		// Fade out over the last second(s) of life by changing the particle's diffuse color
		if(par.life < timefadeout)
		{
			COLOR c = par.color;
			int a = par.life/timefadeout*28.0*opacityFactor;
			par.color = COLOR_RGBA(COLOR_GETRED(c),COLOR_GETGREEN(c),COLOR_GETBLUE(c),a);
		}

		par.position += par.velocity*GDeltaTime;
	}
}
LEVELEFFECT_Fire::~LEVELEFFECT_Fire()
{
	//delete MyHDRglow;
	delete MyCorona;
}
void LEVELEFFECT_Fire::SetParticleDefaults(Particle& p)
{
	p.position	= Vector(0,0,0);					// set particle position to system center
	p.life		= lifetime + ((rand()%100)/100.0f)*lifetimevariance;			
	p.color		= COLOR_RGBA((int)(225.0*opacityFactor),(int)(200.0*opacityFactor),(int)(160.0*opacityFactor),(int)(14.0*opacityFactor));// original colors at full intensity
	Vector randDir= Vector(((rand()%100)/100.0f)*variance, ((rand()%100)/100.0f)*variance,((rand()%100)/100.0f)*variance) - Vector(((rand()%100)/100.0f)*variance, ((rand()%100)/100.0f)*variance,((rand()%100)/100.0f)*variance);
	p.velocity	= (randDir  +  (Vector(0,1,0) + .3f*Vector(sin(GSeconds+windOffset),0,-cos(GSeconds*1.2+windOffset))).Normalized()).Normalized() * (speed + ((rand()%100)/100.0f)*speedvariance);
	p.width = p.height = particlesize - ((rand()%100)/100.0f)*sizevariance;
}

void LEVELEFFECT_Fire::PostRender(Camera& cam)
{
	if(!GEngine.preferences.doHeavyFXsystems)return;
	// Calculate billboard data for a camera-facing quad using the view matrix
	Matrix mat = cam.view;
	Vector rightVect(mat[0][0],mat[1][0],mat[2][0]);
	Vector upVect(mat[0][1],mat[1][1],mat[2][1]);

	int batchArray = FXManager::getBatchForWriting(texture,blendMode);
	if(batchArray == -1)return;
	VertexIndicesBatch* pBatch = &FXManager::vertexIndicesBatches[FXManager::batchedQuads[batchArray].myVertexIndicesBatch];

	int numQuads = FXManager::batchedQuads[batchArray].numQuads;

	Vector loc;
	Vector right;
	Vector up;
	COLOR color;
	for(int i=0;i<numAlive;i++)
	{
		loc	=  Location + particles[i].position;
		right = rightVect * particles[i].width;
		up = upVect * particles[i].height;

		color = particles[i].color;

		if(numQuads > 1499)//QUADS_PER_BATCH-1)
		{
			FXManager::batchedQuads[batchArray].numQuads = numQuads;
			batchArray = FXManager::addNewBatch(texture,blendMode);
			if(batchArray == -1)return;
			pBatch = &FXManager::vertexIndicesBatches[FXManager::batchedQuads[batchArray].myVertexIndicesBatch];
			numQuads = FXManager::batchedQuads[batchArray].numQuads;
		}

		pBatch->theVertices[(numQuads*4)].diffuse = color;
		pBatch->theVertices[(numQuads*4)].position = (loc-right)-up;
		pBatch->theVertices[(numQuads*4)].tu = 0;
		pBatch->theVertices[(numQuads*4)].tv = 1;

		pBatch->theVertices[(numQuads*4)+1].diffuse = color;
		pBatch->theVertices[(numQuads*4)+1].position = (loc+right)-up;
		pBatch->theVertices[(numQuads*4)+1].tu = 1;
		pBatch->theVertices[(numQuads*4)+1].tv = 1;

		pBatch->theVertices[(numQuads*4)+2].diffuse = color;
		pBatch->theVertices[(numQuads*4)+2].position = (loc-right)+up;
		pBatch->theVertices[(numQuads*4)+2].tu = 0;
		pBatch->theVertices[(numQuads*4)+2].tv = 0;

		pBatch->theVertices[(numQuads*4)+3].diffuse = color;
		pBatch->theVertices[(numQuads*4)+3].position = (loc+right)+up;
		pBatch->theVertices[(numQuads*4)+3].tu = 1;
		pBatch->theVertices[(numQuads*4)+3].tv = 0;

		numQuads++;
	}
	FXManager::batchedQuads[batchArray].numQuads = numQuads;
}

BEGIN_REGISTRATION(DUSBsniperTrail);
REGISTER_TEXTURE(DUSBsniperTrail,myTex,"redStreak.dds");
DUSBsniperTrail::DUSBsniperTrail(World* world,Vector& startPoint, Vector& endPoint) : Laser(world,startPoint,endPoint,.1,&myTex)
{	
	DO_PRECACHING();
	LifeTime = .31;
}
void DUSBsniperTrail::Tick()
{
	Laser::Tick();
	if(LifeTime > 0)alpha = 255.f * LifeTime/.31f;
	else alpha = 0;
}


BEGIN_REGISTRATION(HDRglow);
REGISTER_MODEL(HDRglow,staticModel,"HDRglowBox.mdc");
ShaderVar* HDRglow::VarMatDiffuse = NULL;
ShaderVar* HDRglow::VarOverBright = NULL;
Material* HDRglow::MyMaterial = NULL;
HDRglow::HDRglow(World* world,Vector4 color, float scale, float overbright) : FXSystem(world)
{
	DO_PRECACHING();
	if(!MyMaterial)
	{
		string matName = "gl";
		MyMaterial = staticModel.FindMaterial(matName);
		VarMatDiffuse = MyMaterial->FindVar("MatDiffuse");
		VarOverBright = MyMaterial->FindVar("OverBright");
	}
	Color = color;
	OverBright = overbright;
	Scale = scale;
}
void HDRglow::HDRrender(Camera& cam)
{
	MyMaterial->m_Shader->SetVar(*VarMatDiffuse,&Color);
	MyMaterial->m_Shader->SetVar(*VarOverBright,&OverBright);
	MyMaterial->UpdateStates();

	staticModel.m_pFrameRoot->TransformationMatrix.m0.x = Scale;
	staticModel.m_pFrameRoot->TransformationMatrix.m1.y = Scale;
	staticModel.m_pFrameRoot->TransformationMatrix.m2.z = Scale;

	Matrix transform;
	transform.m3 = Location;

	staticModel.SetTransform(transform);

	staticModel.Draw(&cam,MyWorld,DRAW_IMMEDIATE | DRAW_UPDATELIGHTING);
}

float clamp(float val, float min, float max)
{
	return (val < min) ? min : (val > max ? max : val);
}

float LEVELEFFECT_Corona::ChandelierOpacity = 1;
LEVELEFFECT_Corona::LEVELEFFECT_Corona(World* world,Vector& theLocation,string TextureName,Vector color,float maxDistance,float startSize, float endSize, float startAlpha, float endAlpha, bool doesRotation,bool doesFlickering,float flickerPortion, float OccluderSize) : FXSystem(world)
{
	texture.Load(TextureName);
	occlusionQuery = RenderDevice::Instance()->CreateOcclusionQuery();
	Location = theLocation;
	Color = color;
	MaxDistance = maxDistance;
	StartSize = startSize;
	EndSize = endSize;
	StartAlpha = startAlpha;
	EndAlpha = endAlpha;
	DoesRotation = doesRotation;
	opacityFactor = 0;
	DoesFlickering = doesFlickering;
	FlickerPortion = flickerPortion;
	occluderSize = OccluderSize;
}
LEVELEFFECT_Corona::~LEVELEFFECT_Corona()
{
	RenderDevice::Instance()->FreeOcclusionQuery(occlusionQuery);
}
void LEVELEFFECT_Corona::Tick()
{
	if((Location - GEngine.cam.Location).Length() > MaxDistance || occlusionQuery->GetPixels() < 1)
		opacityFactor -= GDeltaTime * 9.4f;
	else 
		opacityFactor += GDeltaTime * 9.4f;
	opacityFactor = clamp(opacityFactor,0,1);
}
void LEVELEFFECT_Corona::PostRender(Camera& cam)
{
	float UseOpacityFactor = opacityFactor;
	if(DoesFlickering)
		UseOpacityFactor = (1.f - FlickerPortion)*UseOpacityFactor + RANDF()*FlickerPortion*UseOpacityFactor;
	else if(DoesRotation)
		UseOpacityFactor *= ChandelierOpacity;

	if(UseOpacityFactor > 0)
	{
		float lerpValue = clamp((Location - cam.Location).Length()/MaxDistance,0,1);
		float size = StartSize + ((float)(EndSize - StartSize))*lerpValue;
		float alpha = StartAlpha + ((float)(EndAlpha - StartAlpha))*lerpValue;
		//RenderDevice::Instance()->GetCanvas()->BillBoard(GEngine.getCoordsWorldFromScreen(GEngine.getCoordsScreenFromWorld(Location)+Vector(0,0,5)),2,COLOR_RGBA(Color.x,Color.y,Color.z,255),&texture,BLEND_SRCALPHA,BLEND_ONE);
		Vector coords = GEngine.getCoordsScreenFromWorld(Location);
			
		RenderDevice::Instance()->GetCanvas()->Box(COLOR_RGBA(Color.x,Color.y,Color.z,(int)(alpha*UseOpacityFactor)),coords.x-size/2,coords.y-size/2,size,size,&texture,BLEND_SRCALPHA,BLEND_ONE);

		//if(!DoesRotation)RenderDevice::Instance()->GetCanvas()->Box(COLOR_RGBA(Color.x,Color.y,Color.z,(int)(alpha*UseOpacityFactor)),coords.x-size/2,coords.y-size/2,size,size,&texture,BLEND_SRCALPHA,BLEND_ONE);
		//else RenderDevice::Instance()->GetCanvas()->RotatedBox(COLOR_RGBA(Color.x,Color.y,Color.z,(int)(alpha*UseOpacityFactor)),coords.x-size/2,coords.y-size/2,size,size,360.f*lerpValue,Vector(coords.x,coords.y,0),&texture,BLEND_SRCALPHA,BLEND_ONE);
	}

	RenderWrap::SetRS( D3DRS_ALPHABLENDENABLE, false );
	RenderWrap::SetRS( D3DRS_ZWRITEENABLE, false );
	RenderWrap::SetRS( D3DRS_COLORWRITEENABLE, false);
	RenderWrap::SetRS( D3DRS_FOGENABLE, false );
	occlusionQuery->BeginQuery();
	RenderDevice::Instance()->GetCanvas()->BillBoard(Location,occluderSize,COLOR_RGBA(255,255,255,255),NULL,BLEND_SRCALPHA,BLEND_INVSRCALPHA);
	occlusionQuery->EndQuery();
	RenderWrap::SetRS( D3DRS_ALPHABLENDENABLE, true);
	RenderWrap::SetRS( D3DRS_ZWRITEENABLE, false);
	RenderWrap::SetRS( D3DRS_COLORWRITEENABLE, 0xFFFFFFF);
	RenderWrap::SetRS( D3DRS_FOGENABLE, true);
}


//-----------------------------------------------------------------------------
// Fireflies! How cute...
//-----------------------------------------------------------------------------
FireFly::FireFly(World* world,Vector& theLocation,Vector& FlyZone,string TextureName,Vector color,float maxDistance,float startSize, float endSize, float startAlpha, float endAlpha, bool doesRotation,bool doesFlickering,float flickerPortion, float OccluderSize) : FXSystem(world)
{
	beeSound.Load("bee.wav");
	texture.Load(TextureName);
	occlusionQuery = RenderDevice::Instance()->CreateOcclusionQuery();
	Location = theLocation + Vector(0.2f,0.4f,-0.3f);
	Color = color;
	MaxDistance = maxDistance;
	StartSize = startSize;
	EndSize = endSize;
	StartAlpha = startAlpha;
	EndAlpha = endAlpha;
	DoesRotation = doesRotation;
	opacityFactor = 0;
	DoesFlickering = doesFlickering;
	FlickerPortion = flickerPortion;
	occluderSize = OccluderSize;
	m_Light = world->FindLight("FireFly1");
	m_Light->SetRange(m_Light->GetCurrentState().Range + 3.5f);
	m_FirstTick = true;
	beeSound.Play(myEmitter,SOUND_LOOP);
	flyZone = FlyZone;
}
FireFly::~FireFly()
{
	RenderDevice::Instance()->FreeOcclusionQuery(occlusionQuery);
	beeSound.Stop();
}
void FireFly::Tick()
{
	if(m_FirstTick){
		m_StartLoc = Location;
		m_FirstTick = false;
	}

	prevLoc = Location;
	Location.x = m_StartLoc.x + sin(GSeconds*2)*flyZone.x;
	Location.y = m_StartLoc.y + cos(GSeconds)*flyZone.y;
	Location.z = m_StartLoc.z + sin(GSeconds)*flyZone.z;

	myEmitter.Update3DParameters(Location,(Location-prevLoc)*(1/GDeltaTime)*6.f,270);

	if(m_Light)
		m_Light->Location = Location;

	/*if((Location - GEngine.cam.Location).Length() > MaxDistance || occlusionQuery->GetPixels() < 1)
		opacityFactor -= GDeltaTime * 9.4f;
	else 
		opacityFactor += GDeltaTime * 9.4f;
	opacityFactor = clamp(opacityFactor,0,1);*/
}

void FireFly::PostRender(Camera& cam)
{
	///float UseOpacityFactor = 1;//opacityFactor;
	//if(DoesFlickering)
	//	UseOpacityFactor = (1.f - FlickerPortion)*UseOpacityFactor + RANDF()*FlickerPortion*UseOpacityFactor;

	//opacityFactor = 1;

	//if(UseOpacityFactor > 0)
	//{
	//	float lerpValue = clamp((Location - cam.Location).Length()/MaxDistance,0,1);
	//	float size = StartSize + ((float)(EndSize - StartSize))*lerpValue;
	//	float alpha = StartAlpha + ((float)(EndAlpha - StartAlpha))*lerpValue;
	//	alpha = 255;
		//RenderDevice::Instance()->GetCanvas()->BillBoard(GEngine.getCoordsWorldFromScreen(GEngine.getCoordsScreenFromWorld(Location)+Vector(0,0,5)),2,COLOR_RGBA(Color.x,Color.y,Color.z,255),&texture,BLEND_SRCALPHA,BLEND_ONE);
		//Vector coords = GEngine.getCoordsScreenFromWorld(Location);
			
		//RenderDevice::Instance()->GetCanvas()->Box(COLOR_RGBA(Color.x,Color.y,Color.z,(int)(alpha*UseOpacityFactor)),coords.x-size/2,coords.y-size/2,size,size,&texture,BLEND_NONE,BLEND_NONE);//BLEND_SRCALPHA,BLEND_ONE);

		RenderDevice::Instance()->GetCanvas()->BillBoard(Location,occluderSize,COLOR_RGBA(180,229,239,255),&texture,BLEND_SRCALPHA,BLEND_ONE);
	
		//if(!DoesRotation)RenderDevice::Instance()->GetCanvas()->Box(COLOR_RGBA(Color.x,Color.y,Color.z,(int)(alpha*UseOpacityFactor)),coords.x-size/2,coords.y-size/2,size,size,&texture,BLEND_SRCALPHA,BLEND_ONE);
		//else RenderDevice::Instance()->GetCanvas()->RotatedBox(COLOR_RGBA(Color.x,Color.y,Color.z,(int)(alpha*UseOpacityFactor)),coords.x-size/2,coords.y-size/2,size,size,360.f*lerpValue,Vector(coords.x,coords.y,0),&texture,BLEND_SRCALPHA,BLEND_ONE);
	//}
/*
	RenderWrap::SetRS( D3DRS_ALPHABLENDENABLE, false );
	RenderWrap::SetRS( D3DRS_ZWRITEENABLE, false );
	RenderWrap::SetRS( D3DRS_COLORWRITEENABLE, false);
	RenderWrap::SetRS( D3DRS_FOGENABLE, false );
	occlusionQuery->BeginQuery();
	RenderDevice::Instance()->GetCanvas()->BillBoard(Location,occluderSize,COLOR_RGBA(255,255,255,255),NULL,BLEND_SRCALPHA,BLEND_INVSRCALPHA);
	occlusionQuery->EndQuery();
	RenderWrap::SetRS( D3DRS_ALPHABLENDENABLE, true);
	RenderWrap::SetRS( D3DRS_ZWRITEENABLE, false);
	RenderWrap::SetRS( D3DRS_COLORWRITEENABLE, 0xFFFFFFF);
	RenderWrap::SetRS( D3DRS_FOGENABLE, true);*/
}