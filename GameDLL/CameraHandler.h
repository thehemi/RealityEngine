//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
/// CameraHandler: Handles updating and animation of the world camera state, whether in FPS or in vehicles
///
//====================================================================================
#pragma once
#ifndef CAMHANDLER_H
#define CAMHANDLER_H

//--------------------------------------------------------------------------------------
/// Handles updating and animation of the world camera state, whether in FPS or in vehicles
//--------------------------------------------------------------------------------------
class GAME_API CameraHandler
{
protected:
	/// Updates interpolation
	void UpdateViewInterpolation();
	/// Camera being used
	Camera m_Camera;
	/// Currently interpolating between points?
	bool isInViewInterpolation;

	bool FOVInterpolation;
	float FOVInterpTotalStep;
	float FOVInterpCurTime;
	float FOVInterpTotalTime;
	float FOVInterpDest;

public:
	/// Singleton class
	static CameraHandler* Instance();
	CameraHandler(){Fov=90;}
	/// Returns current camera, with option for forcing return of CameraHandler's own default Camera
	Camera* GetCamera(bool ForceDefaultCamera = false);
	/// Updates camera
	void Tick(bool JustSet = false);

	/// Sets new view parameters for the camera
	void setToView(Vector& loc,Matrix& rot,float Roll = 0, float FOV = 90);
	/// Moves the camera to a new view point with optional interpolation
	void moveToView(float interpTime,Vector& loc,Matrix& rot,float Roll = 0, float FOV = 90);
	void moveToFOV(float interpTime,float FOV);
	/// Changes the interpolation destination
	void updateDestView(Vector& loc,Matrix& rot,float Roll = 0, float FOV = 90);

	/// Destination of interpolation
	Matrix InterpDestCameraTransform;
	/// Interpolated delta so far
	Matrix InterpTotalDeltaCameraTransform;
	/// Original source before interpolation
	Matrix InterpStartCameraTransform;
	/// How far into interpolation in seconds
	float CurInterpolationTime;
	/// Total time of interpolation in seconds
	float TotalInterpolationTime;
	/// Is interpolating?
	bool isInCameraMove(){ return isInViewInterpolation; }
	Matrix CurCameraTransform;

	/// Current FOV to set Camera to
	float Fov;

	void ApplyCameraValues();
};

#endif