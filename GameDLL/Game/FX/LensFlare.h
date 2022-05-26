#ifndef LENSFLARE
#define LENSFLARE
#include "FXManager.h"

class LensFlare : public FXSystem {
public:
	DEFINE_PRECACHING();
	DEFINE_TEXTURE(burnOut);
	static Texture flare[11];
	Vector sourcePosition;
	float opacityFactor;
	float fadeFactor;
	LensFlare(World* world,Vector& sourcePos);
	virtual void PostRender(Camera& cam);
	virtual void PreFinalRender(Camera& cam);
	float angle;
	Vector screenPos;
	Vector previousCamPos;
	OcclusionQuery* occlusionQuery;
};

#endif