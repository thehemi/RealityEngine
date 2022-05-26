//=========== Copyright (c) 2003, Tim Johnson. All rights reserved. ===========
// Low-level Functionality for graphics rendering classes
//
//=============================================================================
#include "stdafx.h"
#include "RenderWrap.h"
#include "RenderDevice.h"
#include "HDR.h"
#include "Editor.h"


#define INVALID_STATE 0x123456

IDirect3DDevice9* RenderWrap::dev = 0;
IDirect3D9 *RenderWrap::d3d = 0;

//-----------------------------------------------------------------------------
// Name: SetView()
// Desc: Sets the view matrix given a position and direction vector
//-----------------------------------------------------------------------------
Matrix RenderWrap::BuildView(Vector& pos, Vector& dir, float roll/*=0*/){
    D3DXMATRIX matView;
	D3DXMatrixLookAtLH( &matView, (D3DXVECTOR3*)&pos, (D3DXVECTOR3*)&(pos+(dir*pos.Length()*0.5f)), &D3DXVECTOR3( 0.0f, 1.0f,  0.0f ) );
	
	if(roll){
		D3DXMATRIX matRoll;
		D3DXMatrixRotationZ(&matRoll,DEG2RAD(roll));
		matView *= matRoll;
	}

	return *(Matrix*)&matView;
}


//-----------------------------------------------------------------------------
// Name: SetProjection()
// Desc: Sets and stores projection state
//-----------------------------------------------------------------------------
Matrix RenderWrap::BuildProjection(float fov, float nearclip, float farclip){
	D3DXMATRIX matProj;
	float fAspect;
	
	fAspect = 800.f / 600.f; // 1.333 default

	// D3DX takes vertical degrees, convert from horizontal FOV
	float vFov = fov;
	if(Editor::Instance()->GetEditorMode())
	{
		RenderDevice* d = RenderDevice::Instance();
		fAspect = d->m_fAspect;
		vFov = (vFov*1.33f) / fAspect; // Divide by aspect to get Farycry editor type behaviour - making window taller doesn't alter FOV, making thinner increases fov
	}
	else
		vFov = vFov * fAspect;

    D3DXMatrixPerspectiveFovLH( &matProj, DEG2RAD(vFov/2), fAspect, nearclip, farclip );
	return *(Matrix*)&matProj;
}

void RenderWrap::DrawPrim(bool strips, int numVerts, void* verts, int vertsize){
	dev->DrawPrimitiveUP((strips?D3DPT_TRIANGLESTRIP:D3DPT_TRIANGLELIST), numVerts, verts, vertsize);
}

Matrix mWorld;
Matrix curView;
Matrix mProj;

void RenderWrap::SetWorld(const Matrix &world){

	mWorld = world;
	dev->SetTransform(D3DTS_WORLD,(D3DMATRIX*)&world);
}

Matrix RenderWrap::GetWorld(){
	return mWorld;
}

Matrix RenderWrap::GetViewProjection()
{
	D3DXMATRIX mx;
	D3DXMATRIX mxView = *(D3DXMATRIX*)&RenderWrap::GetView();
	D3DXMATRIX mxProj = *(D3DXMATRIX*)&RenderWrap::GetProjection();
	D3DXMatrixMultiply( & mx, & mxView, & mxProj );
	return *(Matrix*)&mx;
}

void RenderWrap::SetView(Matrix &view){
	curView = view;
	dev->SetTransform(D3DTS_VIEW,(D3DMATRIX*)&view);
}

Matrix RenderWrap::GetView(){
	return curView;
}

void RenderWrap::SetProjection(Matrix &projection){
	mProj = projection;
	dev->SetTransform(D3DTS_PROJECTION,(D3DMATRIX*)&projection);
}

Matrix RenderWrap::GetProjection(){
	return mProj;
}

void RenderWrap::ClearTextureLevel(int level){
	dev->SetTexture(level,0);
}

void RenderWrap::SetMaterial(void* mat){
	dev->SetMaterial((D3DMATERIAL9*)mat);
}


//-----------------------------------------------------------------------------
// Name: SetMaterial()
// Desc: Sets and applys an object material
//-----------------------------------------------------------------------------
void RenderWrap::SetMaterial(D3DCOLOR rgb){
	D3DMATERIAL9  mtl;
	ZeroMemory(&mtl, sizeof(D3DMATERIAL9) );		
	mtl.Diffuse.r = mtl.Ambient.r = GetRValue(rgb)/255.f;
	mtl.Diffuse.g = mtl.Ambient.g = GetGValue(rgb)/255.f;
	mtl.Diffuse.b = mtl.Ambient.b = GetBValue(rgb)/255.f;
	mtl.Diffuse.a = mtl.Ambient.a = 1;	
	dev->SetMaterial(&mtl);
}

DWORD renderStates[256];


DWORD RenderWrap::SetRS( D3DRENDERSTATETYPE  rs, DWORD val )
{
	if(val == INVALID_STATE)
		Error("Render state = INVALID_STATE. (val is 0x%x) This means you read an invalid state from RenderWrap - probably an engine fault for not having the state cached",val);
	
	DWORD cur = renderStates[rs];
	//if( renderStates[ rs ] != val )
	{
		dev->SetRenderState( rs, val );
		renderStates[ rs ] = val;
	}
	return cur;
}

DWORD RenderWrap::GetRS( D3DRENDERSTATETYPE  rs ){
	return renderStates[rs];
}


DWORD textureStates[8][30]; //max stages, max states

DWORD RenderWrap::SetTSS(int stage, DWORD what, DWORD val){
/*#ifdef _DEBUG
	if(val == INVALID_STATE)
		Error("Texture state = INVALID_STATE. This means you read an invalid state from RenderWrap with GetTSS() and tried to apply it - probably an engine fault for not having the state cached");
#endif*/
	DWORD cur = textureStates[stage][what];
	////if(what > 11 && what <= 21)
	//	Error("SetTSS() Was called with a DX8 state. DX9 uses SetSS() for texture sampler states now");

	if(textureStates[stage][what] != val){
		dev->SetTextureStageState(stage,(D3DTEXTURESTAGESTATETYPE)what,val);
		textureStates[stage][what] = val;
	}
	return cur;
}

void RenderWrap::SetSS(int stage, D3DSAMPLERSTATETYPE what, DWORD val){
	dev->SetSamplerState(stage,what,val);
}

DWORD RenderWrap::GetSS(int stage, D3DSAMPLERSTATETYPE what){
	DWORD val;
	dev->GetSamplerState(stage,what,&val);
	return val;
}


DWORD RenderWrap::GetTSS(int stage, DWORD what){
	return textureStates[stage][what];
}


void RenderWrap::SetBlending(GfxBlend src, GfxBlend dest){
   SetRS(D3DRS_ALPHABLENDENABLE,(src != GFX_ZERO));
   SetRS( D3DRS_SRCBLEND, src );
   SetRS( D3DRS_DESTBLEND, dest );
}


void RenderWrap::Restore(){
	// Fill arrays with invalid values, so new states
	// will always pass the 'is-not-already-applied' test
	for(int i=0;i<256;i++)
		renderStates[i] = INVALID_STATE;

	for(int j=0;j<8;j++)
		for(int k=0;k<30;k++)
			textureStates[j][k] = INVALID_STATE;

		
	SetRS( D3DRS_FOGENABLE, TRUE );
	SetRS( D3DRS_FOGCOLOR, 0 );
	// Must set D3DFOG_NONE even though this is the supposed default
	SetRS( D3DRS_FOGVERTEXMODE, D3DFOG_NONE );  
	SetRS( D3DRS_FOGTABLEMODE,  D3DFOG_NONE );  
}

