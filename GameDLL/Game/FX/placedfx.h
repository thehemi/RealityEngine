#ifndef PLACEDFX_H
#define PLACEDFX_H
#include "FXManager.h"
#include "LaserParticles.h"
#include "Lasers.h"
#include "GameEngine.h"
#include "Player.h"
#include "SurfaceDecal.h"

class LEVELEFFECT_Corona  : public FXSystem
{
public:
	Texture texture;
	float MaxDistance;
	float StartSize;
	float EndSize;
	float StartAlpha;
	float EndAlpha;
	float opacityFactor;
	bool DoesRotation;
	bool DoesFlickering;
	float FlickerPortion;
	Vector Color;
	OcclusionQuery* occlusionQuery;
	float occluderSize;
	LEVELEFFECT_Corona(World* world,Vector& theLocation,string TextureName,Vector color,float maxDistance,float startSize, float endSize, float startAlpha, float endAlpha, bool doesRotation,bool doesFlickering = false,float flickerPortion = 0, float OccluderSize = .07);
	virtual void PostRender(Camera& cam);
	virtual void Tick();
	virtual ~LEVELEFFECT_Corona();
	//cheap hack for mansion electric corona switch-on
	static float ChandelierOpacity;
};

class Corona : public FXSystem
{
public:
	float maxSize;
	int opacity;
	Texture* texture;
	float opacityFactor;
	bool firstRender;

	Corona(World* world,Vector& theLocation,Texture* tex,float MaxSize) : FXSystem(world)
	{
		maxSize = MaxSize;
		Location = theLocation;
		texture = tex;
		opacity = 255;
		opacityFactor = 0;
		firstRender = true;
	}

	virtual void PostRender(Camera& cam)
	{
		Actor* actorToIgnore = 0;
		if(GEngine.player)
		{
			actorToIgnore = GEngine.player->getOwnerToIgnore();
			if(actorToIgnore->GhostObject || actorToIgnore->CollisionFlags & Actor::CF_PASSABLE_BBOX)actorToIgnore = 0;
		}

		CollisionInfo info;
		if(MyWorld->CollisionCheckRay(actorToIgnore,cam.Location,Location,CHECK_EVERYTHING,info,true))
		{
			opacityFactor -= 4.0*GDeltaTime;
			if(opacityFactor < 0)opacityFactor = 0;
		}
		else
		{
			if(firstRender)opacityFactor = 1.0;
			opacityFactor += 4.0*GDeltaTime;
			if(opacityFactor > 1.0)opacityFactor = 1.0;
		}

		if(opacityFactor > 0)
		{
			Vector coords = GEngine.getCoordsScreenFromWorld(Location);
			float theSize = maxSize / ((Location - cam.Location).Length()*.0003);
			RenderDevice::Instance()->GetCanvas()->Box(COLOR_RGBA(255,255,255,(int)(opacityFactor*opacity)),coords.x - theSize/2.0,coords.y - theSize/2.0,theSize,theSize,texture,BLEND_SRCALPHA,BLEND_ONE);
		}
		firstRender = false;
	}
};
class FadingSmoke : public FXSystem
{
public:
	Texture* tex;
	float size;
	FadingSmoke(World* world,Vector location,Texture* particleTex) : FXSystem(world)
	{
		doCamDistanceSpawnCheck();
		isSorted = true;
		Location = location;
		tex = particleTex;
		LifeTime = 3.0f;
		size = (21.5 + rand()%30)/33.0f;
	}
	virtual void PostRender(Camera& cam);
};
class TinySmoke : public FXSystem
{
public:
	Texture* tex;
	float size;
	TinySmoke(World* world,Vector& location,Texture* particleTex) : FXSystem(world)
	{
		Location = location;
		tex = particleTex;
		LifeTime = 2.0f;
		size = (13.0 + rand()%13)/33.0f;
	}
	virtual void PostRender(Camera& cam);
};

class ZenHandGrenadeGlowTrail : public FXSystem
{
public:
	static Texture myTex;
	float size;
	ZenHandGrenadeGlowTrail(World* world,Vector& location) : FXSystem(world)
	{
		if(!myTex.IsLoaded())myTex.Load("laserblueimpact.dds");
		Location = location;
		LifeTime = 1.0f;
		size = (.27 + .2f*RANDF());
	}
	virtual void PostRender(Camera& cam);
};
class ZenHandGrenadeStuckGlow : public FXSystem
{
public:
	static Texture myTex;
	float size;
	ZenHandGrenadeStuckGlow(World* world,Vector& location) : FXSystem(world)
	{
		if(!myTex.IsLoaded())myTex.Load("laserblueimpact.dds");
		Location = location;
		LifeTime = .4f;
		size = (.1 + .1f*RANDF());
	}
	virtual void PostRender(Camera& cam);
};
class SmallGunSmoke : public FXSystem
{
public:
	float size;
	Texture* tex;
	SmallGunSmoke(World* world,Vector& location,Texture* particleTex) : FXSystem(world)
	{
		Location = location;
		tex = particleTex;
		LifeTime = 1.0f;
		size = .13 + .14*RANDF();
	}
	virtual void PostRender(Camera& cam);
};

class AttachedGunSmoke : public FXSystem
{
public:
	Texture* tex;
	float dirOffset;
	float upOffset;
	float rightOffset;
	float startSize;
	float growSize;
	float startLifeTime;
	float startAlpha;
	DWORD attachedToActorID;
	AttachedGunSmoke(World* world,SynchedActor* synchedActor,float DirOffset,float UpOffset,float RightOffset,Texture* particleTex, float startsize = .12, float growAddSize = .197, float lifeTime = .4, float startalpha = 210.0) : FXSystem(world)
	{
		attachedToActorID = synchedActor->actorID;
		dirOffset = DirOffset;
		upOffset = UpOffset;
		rightOffset = RightOffset;
		tex = particleTex;
		startAlpha = startalpha;
		startLifeTime = lifeTime;
		LifeTime = startLifeTime;
		startSize = startsize;
		growSize = growAddSize;
	}
	virtual void PostRender(Camera& cam);
};

class SmallImpactSmoke : public FXSystem
{
public:
	DEFINE_PRECACHING();
	DEFINE_TEXTURE(tex);
	SmallImpactSmoke(World* world,Vector& location) : FXSystem(world)
	{
		if(!tex.IsLoaded())tex.Load("tinysmoke.dds");

		Location = location;
		LifeTime = 1.0f;
	}
	virtual void PostRender(Camera& cam);
};

class CustomSmoke : public FXSystem
{
public:
	DEFINE_PRECACHING();
	DEFINE_TEXTURE(tex);
	float StartScale;
	float AddScale;
	float MaxLifeTime;
	float MaxAlpha;
	CustomSmoke(World* world,Vector& location, float lifeTime, float maxAlpha, float startScale, float addScale, bool IsSorted = false) : FXSystem(world)
	{
		if(!tex.IsLoaded())tex.Load("tinysmoke.dds");
		Location = location;
		LifeTime = lifeTime;

		MaxLifeTime = lifeTime;
		AddScale = addScale;
		StartScale = startScale;
		MaxAlpha = maxAlpha;
		isSorted = IsSorted;
	}
	virtual void PostRender(Camera& cam);
};


class BigSmoke : public FXSystem
{
public:
	DEFINE_PRECACHING();
	DEFINE_TEXTURE(tex);
	BigSmoke(World* world,Vector& location) : FXSystem(world)
	{
		if(!tex.IsLoaded())tex.Load("bigsmoke.dds");

		Location = location;
		LifeTime = 3.0f;
	}
	virtual void PostRender(Camera& cam);
};

class StarshipDamageSmoke : public FXSystem
{
public:
	DEFINE_PRECACHING();
	DEFINE_TEXTURE(tex);
	float size;
	StarshipDamageSmoke(World* world,Vector& location, float Size) : FXSystem(world)
	{
		if(!tex.IsLoaded())tex.Load("darksmoke.dds");
		Location = location;
		LifeTime = 3.0f;
		size = Size;
		doCamDistanceSpawnCheck();
	}
	virtual void PostRender(Camera& cam);
};

class BloodSpraySprite : public FXSystem
{
public:
	DEFINE_PRECACHING();
	DEFINE_TEXTURE(tex);
	float size;
	BloodSpraySprite(World* world,Vector& location) : FXSystem(world)
	{
		if(!tex.IsLoaded())tex.Load("bloodspray.dds");
		Location = location;
		LifeTime = .7f;
		size = .9f + .7f*RANDF();
		doCamDistanceSpawnCheck();
	}
	virtual void PostRender(Camera& cam);
};

class ZenBloodSpraySprite : public FXSystem
{
public:
	DEFINE_PRECACHING();
	DEFINE_TEXTURE(tex);
	float size;
	ZenBloodSpraySprite(World* world,Vector& location) : FXSystem(world)
	{
		if(!tex.IsLoaded())tex.Load("zenbloodspray.dds");
		Location = location;
		LifeTime = .7f;
		size = .9f + .7f*RANDF();
		doCamDistanceSpawnCheck();
	}
	virtual void PostRender(Camera& cam);
};

class EngineSmoke : public FXSystem
{
public:
	float size;
	DEFINE_PRECACHING();
	DEFINE_TEXTURE(tex);
	EngineSmoke(World* world,Vector& location);
	virtual void PostRender(Camera& cam);
};

class TinyDirt : public FXSystem
{
public:
	DEFINE_PRECACHING();
	DEFINE_TEXTURE(tex);
	TinyDirt(World* world,Vector& location,Vector& surfaceNormal) : FXSystem(world)
	{
		if(!tex.IsLoaded())tex.Load("tinydirt.dds");
		Location = location;
		ParticleSystem* ps = new ParticleSystem(MyWorld,0,60,60,Location,surfaceNormal,Vector(0,-12,0),1.1f,.25,1.8f,700,100,700,0.045f,0.024f,false,&tex,BLEND_INVSRCALPHA);
		ps->StopSpawningNow();
		ps->LifeTime = .8;
		LifeTime = .8f;
	}
};

class BigDirt : public FXSystem
{
public:
	DEFINE_PRECACHING();
	DEFINE_TEXTURE(tex);
	float scaleFactor;
	float size;
	BigDirt(World* world,Vector& location,float startSize = 1.2f,float scalefactor = 1.0f) : FXSystem(world)
	{
		if(!tex.IsLoaded())tex.Load("bigdirt.dds");

		Location = location;
		LifeTime = 1.0f;
		size = startSize;
		scaleFactor = scalefactor;
	}
	virtual void PostRender(Camera& cam);
};

class DirtBlastChunks : public FXSystem
{
public:
	DEFINE_PRECACHING();
	DEFINE_TEXTURE(tex);
	ParticleSystem* particleSys;
	DirtBlastChunks(World* world,Vector& location,Vector& direction);
	virtual void Tick()
	{
		particleSys->Location = Location;
	}
};

class BouncySparks : public FXSystem
{
public:
	DEFINE_PRECACHING();
	DEFINE_TEXTURE(sparkTex);
	DEFINE_TEXTURE(sparkGlowTex);
	BouncySparks(World* world,Vector& location,Vector& sparksDir);
};

class StarshipDamageSparks : public FXSystem
{
public:
	DEFINE_PRECACHING();
	DEFINE_TEXTURE(sparkTex);
	StarshipDamageSparks(World* world,Vector& location,Vector& sparksDir);
};

class BouncyDirt : public BouncyParticles
{
public:
	DEFINE_PRECACHING();
	DEFINE_TEXTURE(myTex);
	BouncyDirt(World* world,Vector& location,Vector& sprayDir);
};
class BouncyMetalChunks : public BouncyParticles
{
public:
	DEFINE_PRECACHING();
	DEFINE_TEXTURE(myTex);
	BouncyMetalChunks(World* world,Vector& location,Vector& sprayDir);
};

class BansheeExplosion : public FXSystem
{
public:
	DEFINE_PRECACHING();
	DEFINE_TEXTURE(explosionCenter);
	DEFINE_TEXTURE(explosionChunk);
	DEFINE_TEXTURE(explosionGlow);
	DEFINE_SOUND(explosionSound);
	Corona* glowCorona;
	class LensFlare* lensFlare;
	BansheeExplosion(World* world,Vector& theLocation,Actor* Creator);
	virtual ~BansheeExplosion();
	virtual void PostRender(Camera& cam);
};

class TinyExplosion : public FXSystem
{
public:
	DEFINE_PRECACHING();
	DEFINE_TEXTURE(explosionChunk);
	DEFINE_SOUND(explosionSound);
	TinyExplosion(World* world,Vector& theLocation,Actor* Creator);
	virtual void PostRender(Camera& cam);
	virtual ~TinyExplosion();
	class LensFlare* lensFlare;
};


class HeadLamp : public FXSystem
{
public:
	Player* owner;
	DEFINE_PRECACHING();
	DEFINE_TEXTURE(lightRing);
	Vector startPos;
	Vector Dir;
	Vector unconstrainedDir;
	Light* light;
	bool isOn;
	float opacityFactor;
	//Corona* glowCorona;
	HeadLamp(World* world,Player* Owner);
	virtual ~HeadLamp()
	{
		light->LifeTime = 0;
		//glowCorona->LifeTime = 0;
	}
	virtual void setStartEndPoints(Vector& startpos,Vector& dir,Vector& UnconstrainedDir)
	{
		Location = startpos;
		startPos = startpos;
		Dir = dir;
		unconstrainedDir = UnconstrainedDir;
	}
	virtual void PostRender(Camera& cam);
	virtual void Tick();
};

class ZenHeadLamp : public HeadLamp
{
public:
	ZenHeadLamp(World* world,Player* Owner) : HeadLamp(world,Owner)
	{
	}
	virtual void Tick();
	virtual void PostRender(Camera& cam);
};


class HeadLight : public FXSystem
{
public:
	Actor* owner;
	DEFINE_PRECACHING();
	DEFINE_TEXTURE(lightRing);
	Vector startPos;
	Vector Dir;
	bool isOn;
	float opacityFactor;
	//Corona* glowCorona;
	HeadLight(World* world,Actor* Owner);
	virtual void setStartEndPoints(Vector& startpos,Vector& dir)
	{
		Location = startpos;
		startPos = startpos;
		Dir = dir;
	}
	virtual void PostRender(Camera& cam);
	virtual void Tick();
};
class LEVELEFFECT_MediumFire : public ParticleSystem
{
public:
	static Texture myTex;
	LEVELEFFECT_MediumFire(World* world,Vector& location);
	virtual void Tick();
	virtual void SetParticleDefaults(Particle& p);
	virtual void PostRender(Camera& cam);
	float opacityFactor;
	float fadeOffset;
	float windOffset;
};
class LEVELEFFECT_FireLight : public FXSystem
{
public:
	Light* myLight;
	LEVELEFFECT_FireLight(World* world,Vector& location);
	virtual void Tick();
};
class LEVELEFFECT_ChandelierLight : public FXSystem
{
public:
	bool flickeringOut;
	float opacity;
	float flickerOutRange;
	Light* myLight;
	LEVELEFFECT_ChandelierLight(World* world,Vector& location);
	virtual void Tick();
	void FlickerOut();
};
class LEVELEFFECT_CandelabraLight : public FXSystem
{
public:
	Light* myLight;
	LEVELEFFECT_CandelabraLight(World* world,Vector& location);
	virtual void Tick();
};
class LEVELEFFECT_ZenReactorBall : public FXSystem
{
public:
	DEFINE_PRECACHING();
	DEFINE_TEXTURE(tex);
	DEFINE_TEXTURE(particleTex);
	ParticleSystem* particleSys;
	LEVELEFFECT_ZenReactorBall(World* world,Vector& loc);
	virtual void PostRender(Camera& cam);
	virtual ~LEVELEFFECT_ZenReactorBall(){particleSys->LifeTime = 0;}
};
class LEVELEFFECT_SmokeStream : public FXSystem
{
public:
	DEFINE_PRECACHING();
	DEFINE_TEXTURE(particleTex);
	ParticleSystem* particleSys;
	LEVELEFFECT_SmokeStream(World* world,Vector& loc);
	virtual ~LEVELEFFECT_SmokeStream(){particleSys->LifeTime = 0;}
};
class LEVELEFFECT_BigSmokeHaze : public FXSystem
{
public:
	DEFINE_PRECACHING();
	DEFINE_TEXTURE(particleTex);
	ParticleSystem* particleSys;
	LEVELEFFECT_BigSmokeHaze(World* world,Vector& loc);
	virtual ~LEVELEFFECT_BigSmokeHaze(){particleSys->LifeTime = 0;}
};

class DecalExpMetal : public SurfaceDecal
{
public:
	static Texture myTex;
	DecalExpMetal(World* world,Vector& pos,Vector& dir);
};
class DecalExpDirt : public SurfaceDecal
{
public:
	static Texture myTex;
	DecalExpDirt(World* world,Vector& pos,Vector& dir);
};
class DecalBulletMetal : public SurfaceDecal
{
public:
	static Texture myTex;
	DecalBulletMetal(World* world,Vector& pos,Vector& dir);
};
class DecalBulletDirt : public SurfaceDecal
{
public:
	static Texture myTex;
	DecalBulletDirt(World* world,Vector& pos,Vector& dir);
};
class DecalShotgunMetal : public SurfaceDecal
{
public:
	static Texture myTex;
	DecalShotgunMetal(World* world,Vector& pos,Vector& dir);
};
class DecalShotgunDirt : public SurfaceDecal
{
public:
	static Texture myTex;
	DecalShotgunDirt(World* world,Vector& pos,Vector& dir);
};

class DecalBloodSplat : public SurfaceDecal
{
public:
	static Texture myTex;
	DecalBloodSplat(World* world,Vector& pos,Vector& dir,float setSize = .273 + .424 * RANDF());
};

class BloodSplatter : public ParticleSystem
{
public:
	static Texture myTex;
	BloodSplatter(World* world,Actor* ignoreactor,Vector& location);
	virtual void Tick();
};

class ZenDecalBloodSplat : public SurfaceDecal
{
public:
	static Texture myTex;
	ZenDecalBloodSplat(World* world,Vector& pos,Vector& dir,float setSize = .273 + .424 * RANDF());
};

class ZenBloodSplatter : public ParticleSystem
{
public:
	static Texture myTex;
	ZenBloodSplatter(World* world,Actor* ignoreactor,Vector& location);
	virtual void Tick();
};
class SharkEngineExhaust : public ParticleSystem
{
public:
	float lastSpawnedSmokeTime;
	static Texture myTex;
	SharkEngineExhaust(World* world);
	virtual void Tick();
	virtual void SetParticleDefaults(Particle& p);
	virtual void PostRender(Camera& cam);
	float opacityFactor;
};
class BansheeEngineExhaust : public ParticleSystem
{
public:
	static Texture myTex;
	BansheeEngineExhaust(World* world);
	virtual void Tick();
	virtual void SetParticleDefaults(Particle& p);
	virtual void PostRender(Camera& cam);
	float opacityFactor;
};
class ForcePushWave : public SurfaceDecal
{
public:
	static Texture myTex;
	ForcePushWave(World* world,Vector& Location,Matrix& Rot);
	virtual void Tick();
	Vector dir;
	Vector right;
	virtual void PostRender(Camera& cam);
};
class ForcePullWave : public SurfaceDecal
{
public:
	static Texture myTex;
	ForcePullWave(World* world,Vector& Location,Matrix& Rot);
	virtual void Tick();
	Vector dir;
	Vector right;
	virtual void PostRender(Camera& cam);
};
class ZenPersonalShieldImpactGlow : public SurfaceDecal
{
public:
	static Texture myTex;
	Vector PosOffset;
	//Actor* AttachedToActor;
	DWORD AttachedToActorID;
	ZenPersonalShieldImpactGlow(SynchedActor* attachedToActor,Vector& posOffset);
	virtual void PostRender(Camera& cam);
};
class DUSBsniperTrail : public Laser
{
public:
	DEFINE_PRECACHING();
	DEFINE_TEXTURE(myTex);
	DUSBsniperTrail(World* world,Vector& startPoint, Vector& endPoint);
	virtual void Tick();
};

class HDRglow : public FXSystem
{
public:
	DEFINE_PRECACHING();
	DEFINE_MODEL(staticModel);
	static ShaderVar* VarMatDiffuse;
	static ShaderVar* VarOverBright;
	static Material* MyMaterial;
	Vector4 Color;
	float OverBright;
	float Scale;
	virtual void HDRrender(Camera& cam);
	HDRglow(World* world,Vector4 color, float scale, float overbright);
};

class LEVELEFFECT_Fire : public ParticleSystem
{
public:
	static Texture myTex;
	LEVELEFFECT_Fire(World* world,Vector& location, float theScale = 1.f);
	virtual void Tick();
	virtual void SetParticleDefaults(Particle& p);
	virtual void PostRender(Camera& cam);
	float opacityFactor;
	float fadeOffset;
	float windOffset;
	float scale;
	HDRglow* MyHDRglow;
	LEVELEFFECT_Corona* MyCorona;
	virtual ~LEVELEFFECT_Fire();
	//void FlickerOut();
	//bool isFlickeringOut;
};

//-----------------------------------------------------------------------------
/// Fireflies! How cute...
//-----------------------------------------------------------------------------
class FireFly  : public FXSystem
{
public:
	Sound beeSound;
	SoundEmitter myEmitter;
	bool   m_FirstTick;
	Vector m_StartLoc;
	Light *m_Light;
	Texture texture;
	float MaxDistance;
	float StartSize;
	float EndSize;
	float StartAlpha;
	float EndAlpha;
	float opacityFactor;
	bool DoesRotation;
	bool DoesFlickering;
	float FlickerPortion;
	Vector Color;
	OcclusionQuery* occlusionQuery;
	float occluderSize;
	Vector flyZone;
	FireFly(World* world,Vector& theLocation,Vector& FlyZone,string TextureName,Vector color,float maxDistance,float startSize, float endSize, float startAlpha, float endAlpha, bool doesRotation,bool doesFlickering = false,float flickerPortion = 0, float OccluderSize = .07);
	virtual void PostRender(Camera& cam);
	virtual void Tick();
	virtual ~FireFly();
	Vector prevLoc;
};


#endif