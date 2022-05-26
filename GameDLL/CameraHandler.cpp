//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
/// CameraHandler: Handles updating and animation of the world camera state, whether in FPS or in vehicles
///
//====================================================================================
#include "stdafx.h"
#include "CameraHandler.h"
#include "Editor.h"
#include "GameEngine.h"


//--------------------------------------------------------------------------------------
// Sets new view parameters for the camera 
//--------------------------------------------------------------------------------------
void CameraHandler::moveToFOV(float interpTime,float FOV)
{
if(interpTime == 0)
{
	FOVInterpolation = false;
	Fov = FOV;
}
else
{
	FOVInterpolation = true;
	FOVInterpTotalStep = FOV - Fov;
	FOVInterpDest = FOV;
	FOVInterpCurTime = 0;
	FOVInterpTotalTime = interpTime;
}
}

void CameraHandler::setToView(Vector& loc,Matrix& rot, float Roll, float FOV)
{
	if(!FOVInterpolation)
		Fov = FOV;
	else if(FOV != FOVInterpDest)
		{
			FOVInterpolation = false;
			Fov = FOV;
		}
		
	isInViewInterpolation = false;
	//just set the camera to the view
	CurCameraTransform = rot.GetRotationMatrix();
	if(Roll != 0)
	{
		// roll the camera about its z axis if we gotta
		Matrix invRot = CurCameraTransform.Inverse();
		Matrix doRotZ;
		doRotZ.SetRotations(0,0,DEG2RAD(Roll));
		invRot *= doRotZ;
		CurCameraTransform = invRot.Inverse();
	}
	CurCameraTransform.m3 = loc;
}

//--------------------------------------------------------------------------------------
//  Updates interpolation
//--------------------------------------------------------------------------------------
void CameraHandler::UpdateViewInterpolation()
{
	// set the camera's Matrix along the current % of the linear interpolation path.
	CurInterpolationTime += GDeltaTime;
	if(CurInterpolationTime < TotalInterpolationTime)
		CurCameraTransform = InterpStartCameraTransform + InterpTotalDeltaCameraTransform*(CurInterpolationTime/TotalInterpolationTime);
	else 
	{
		// end of interpolation, set to destination
		CurCameraTransform = InterpDestCameraTransform;
		isInViewInterpolation = false;
	}
}

//--------------------------------------------------------------------------------------
// Moves the camera to a new view point with optional interpolation
//--------------------------------------------------------------------------------------
void CameraHandler::moveToView(float interpTime,Vector& loc,Matrix& rot,float Roll, float FOV)
{
	moveToFOV(interpTime,FOV);

	// set up interpolation
	isInViewInterpolation = true;
	InterpDestCameraTransform = rot.GetRotationMatrix();

	if(Roll != 0)
	{
		// roll the transform about its z axis if we gotta
		Matrix invRot = InterpDestCameraTransform.Inverse();
		Matrix doRotZ;
		doRotZ.SetRotations(0,0,DEG2RAD(Roll));
		invRot *= doRotZ;
		InterpDestCameraTransform = invRot.Inverse();
	}
	InterpDestCameraTransform.m3 = loc;
	// set up interpolation
	InterpStartCameraTransform = CurCameraTransform;
	InterpTotalDeltaCameraTransform = InterpDestCameraTransform - InterpStartCameraTransform;
	CurInterpolationTime = 0;
	TotalInterpolationTime = interpTime;
}

//--------------------------------------------------------------------------------------
// Changes the interpolation destination 
//--------------------------------------------------------------------------------------
void CameraHandler::updateDestView(Vector& loc,Matrix& rot,float Roll, float FOV)
{
	// if not in interpolation, just set the current view to the transform,
	// otherwise set the destination transformation to it
	if(!isInViewInterpolation)
		setToView(loc,rot,Roll,FOV);
	else 
		moveToView(TotalInterpolationTime - CurInterpolationTime,loc,rot,Roll,FOV);
}

//--------------------------------------------------------------------------------------
// Updates camera 
//--------------------------------------------------------------------------------------
void CameraHandler::Tick(bool JustSet)
{
	if(!JustSet)
	{

	// delegate camera updating to the appropriate controller, either a DemoPlayer or Player
	//if(DemoPlayer::Instance())
	//	DemoPlayer::Instance()->UpdateCamera(this);

	if(FOVInterpolation)
	{
	float DeltaTime = GDeltaTime;
	FOVInterpCurTime += DeltaTime;
	if(FOVInterpCurTime > FOVInterpTotalTime)
	{
		FOVInterpolation = false;
		DeltaTime -= FOVInterpCurTime - FOVInterpTotalTime;
	}
	Fov += FOVInterpTotalStep*(DeltaTime/FOVInterpTotalTime);
	}

	// Update camera transformation interpolation if any
	if(isInViewInterpolation)
		UpdateViewInterpolation();

	}

	ApplyCameraValues();
}

void CameraHandler::ApplyCameraValues()
{
	// update the Camera object itself with the current transformation
	m_Camera.Fov = Fov;
	Engine::Instance()->InputSys->SetSensitivity(.2f*Fov/90.f);
	if(!g_Game.g_IsGameApp)
		m_Camera.bUpdateProjection = true;
	m_Camera.Update();
	Vector dir = CurCameraTransform.GetDir();
	D3DXMatrixLookAtLH((D3DXMATRIX*)&m_Camera.view,(D3DXVECTOR3*)&(CurCameraTransform.m3),(D3DXVECTOR3*)&(CurCameraTransform.m3 + dir*.5*CurCameraTransform.m3.Length()),(D3DXVECTOR3*)&CurCameraTransform.GetUp());
	m_Camera.viewProjection = m_Camera.view * m_Camera.projection;
	m_Camera.Direction = dir;
	m_Camera.Location = CurCameraTransform.m3;
	m_Camera.Fov = Fov;
	m_Camera.CreateClipPlanes();
}

//--------------------------------------------------------------------------------------
// Singleton class 
//--------------------------------------------------------------------------------------
CameraHandler* CameraHandler::Instance () 
{
	static CameraHandler inst;
	return &inst;
}


//--------------------------------------------------------------------------------------
// Returns current camera, with option for forcing return of CameraHandler's own default Camera 
//--------------------------------------------------------------------------------------
Camera* CameraHandler::GetCamera(bool ForceDefaultCamera)
{
	// if not in Editor mode or forcing return of internal camera, return the internal CameraHandler camera
	if(!Editor::Instance()->GetEditorMode() || ForceDefaultCamera)
		return &m_Camera;
	else
		return &Editor::Instance()->m_Camera; //in Editor mode, return the Editor camera
}


