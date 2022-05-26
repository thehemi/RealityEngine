//================================================================================
// LENSFLARE
//================================================================================
#include "stdafx.h"
#include "Engine.h"
#include "LensFlare.h"
#include "GameEngine.h"
#include "Player.h"

Texture LensFlare::flare[11];
Texture LensFlare::burnOut;

LensFlare::LensFlare(World* world,Vector &sourcePos) : FXSystem(world)
{
	if(!flare[1].IsLoaded())
	{
	flare[0].Load("lensflare0.png");
	flare[1].Load("lensflare1.png");
	flare[2].Load("lensflare2.png");
	flare[3].Load("lensflare3.png");
	flare[4].Load("lensflare4.png");
	flare[5].Load("lensflare5.png");
	flare[6].Load("lensflare6.png");
	flare[7].Load("lensflare7.png");
	flare[8].Load("lensflare8.png");
	flare[9].Load("lensflare9.png");
	flare[10].Load("lensflare10.png");
	burnOut.Load("burnout.dds");
	}
	Location = sourcePos;
	fadeFactor = 0;
	opacityFactor = .65;
	angle = -1;
	forceDistanceRender = true;
	occlusionQuery = RenderDevice::Instance()->CreateOcclusionQuery();
}
void LensFlare::PostRender(Camera& cam)
{
	Actor* actorToIgnore = 0;
	if(GEngine.player)actorToIgnore = GEngine.player->getOwnerToIgnore();
	CollisionInfo info;
	//if(!MyWorld->CollisionCheckRay(actorToIgnore,cam.Location,Location,CHECK_EVERYTHING,info) || (info.touched && info.touched->IsHidden))
	if(occlusionQuery->GetPixels() > 0)
	{
		fadeFactor += 9.0*GDeltaTime;
		if(fadeFactor > 1.0 || (previousCamPos - cam.Location).Length() > 7)fadeFactor = 1.0;
	}
	else
	{
		fadeFactor -= 9.0*GDeltaTime;
		if(fadeFactor < 0 || (previousCamPos - cam.Location).Length() > 7)fadeFactor = 0;
	}
	if(fadeFactor > 0)
	{
	Canvas* canvas = RenderDevice::Instance()->GetCanvas();

Vector viewDir = cam.view.Inverse().GetDir();
Vector center = cam.Location + 5.0*viewDir;

	angle = viewDir.Dot((Location-cam.Location).Normalized());
	float alpha = (angle -.74)* 1600 ;

	if(alpha>200)	alpha = 200;
	if(alpha<0)		alpha = 0;
	
	// Exit if not looking at flares
	if(angle < .75f ) return;

	Vector lookatvector=Location-center;

	screenPos = GEngine.getCoordsScreenFromWorld(Location);
	canvas->Box(COLOR_RGBA(255,255,255,(int)(opacityFactor * fadeFactor*alpha*.3)),screenPos.x-100,screenPos.y-100,200,200,&flare[0],BLEND_SRCALPHA,BLEND_ONE);

	screenPos = GEngine.getCoordsScreenFromWorld(center + lookatvector.Normalized()*7.65);
	canvas->Box(COLOR_RGBA(255,255,255,(int)(opacityFactor * fadeFactor*alpha*.3)),screenPos.x-90,screenPos.y-90,180,180,&flare[1],BLEND_SRCALPHA,BLEND_ONE);
	screenPos = GEngine.getCoordsScreenFromWorld(center + lookatvector.Normalized()*4.7);
	canvas->Box(COLOR_RGBA(255,255,255,(int)(opacityFactor * fadeFactor*alpha*.3)),screenPos.x-55,screenPos.y-55,110,110,&flare[2],BLEND_SRCALPHA,BLEND_ONE);
	screenPos = GEngine.getCoordsScreenFromWorld(center + lookatvector.Normalized()*1.4);
	canvas->Box(COLOR_RGBA(255,255,255,(int)(opacityFactor * fadeFactor*alpha*.3)),screenPos.x-100,screenPos.y-100,200,200,&flare[4],BLEND_SRCALPHA,BLEND_ONE);
	screenPos = GEngine.getCoordsScreenFromWorld(center + lookatvector.Normalized()*.9);
	canvas->Box(COLOR_RGBA(255,255,255,(int)(opacityFactor * fadeFactor*alpha*.3)),screenPos.x-100,screenPos.y-100,200,200,&flare[5],BLEND_SRCALPHA,BLEND_ONE);

	screenPos = GEngine.getCoordsScreenFromWorld(center - lookatvector.Normalized()*.65);
	canvas->Box(COLOR_RGBA(255,255,255,(int)(opacityFactor * fadeFactor*alpha*.3)),screenPos.x-80,screenPos.y-80,160,160,&flare[6],BLEND_SRCALPHA,BLEND_ONE);
	screenPos = GEngine.getCoordsScreenFromWorld(center + lookatvector.Normalized()*.36);
	canvas->Box(COLOR_RGBA(255,255,255,(int)(opacityFactor * fadeFactor*alpha*.3)),screenPos.x-155,screenPos.y-155,310,310,&flare[7],BLEND_SRCALPHA,BLEND_ONE);
	screenPos = GEngine.getCoordsScreenFromWorld(center - lookatvector.Normalized()*1.2);
	canvas->Box(COLOR_RGBA(255,255,255,(int)(opacityFactor * fadeFactor*alpha*.2)),screenPos.x-104,screenPos.y-104,208,208,&flare[8],BLEND_SRCALPHA,BLEND_ONE);
	screenPos = GEngine.getCoordsScreenFromWorld(center + lookatvector.Normalized()*.1);
	canvas->Box(COLOR_RGBA(255,255,255,(int)(opacityFactor * fadeFactor*alpha*.3)),screenPos.x-100,screenPos.y-100,200,200,&flare[9],BLEND_SRCALPHA,BLEND_ONE);
	screenPos = GEngine.getCoordsScreenFromWorld(center - lookatvector.Normalized()*2.0);
	canvas->Box(COLOR_RGBA(255,255,255,(int)(opacityFactor * fadeFactor*alpha*.35)),screenPos.x-165,screenPos.y-165,330,330,&flare[10],BLEND_SRCALPHA,BLEND_ONE);

	}
	previousCamPos = cam.Location;

	RenderWrap::SetRS( D3DRS_ALPHABLENDENABLE, false );
	RenderWrap::SetRS( D3DRS_ZWRITEENABLE, false );
	RenderWrap::SetRS( D3DRS_COLORWRITEENABLE, false);
	RenderWrap::SetRS( D3DRS_FOGENABLE, false );
	occlusionQuery->BeginQuery();
	RenderDevice::Instance()->GetCanvas()->BillBoard(Location,.6,COLOR_RGBA(255,255,255,255),NULL,BLEND_SRCALPHA,BLEND_INVSRCALPHA);
	occlusionQuery->EndQuery();
	RenderWrap::SetRS( D3DRS_ALPHABLENDENABLE, true);
	RenderWrap::SetRS( D3DRS_ZWRITEENABLE, false);
	RenderWrap::SetRS( D3DRS_COLORWRITEENABLE, 0xFFFFFFF);
	RenderWrap::SetRS( D3DRS_FOGENABLE, true);
}
void LensFlare::PreFinalRender(Camera& cam)
{
if(angle != -1 && fadeFactor > 0 && angle>0.86)
	{
		float alpha=(angle-0.9)*1880;
		if(alpha>255) alpha = 255;
		if(alpha<0)	  alpha = 0;
	RenderDevice::Instance()->GetCanvas()->Box(COLOR_RGBA(255,255,255,(int)(opacityFactor * fadeFactor*alpha)),screenPos.x-1000,screenPos.y-1000,2000,2000,&burnOut,BLEND_SRCALPHA,BLEND_ONE);
	} 
}