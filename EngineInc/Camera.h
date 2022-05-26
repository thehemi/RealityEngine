//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
/// Author: Tim Johnson
///
/// A camera has a location, projection, and rotation
/// Cameras are used to define the player's world-space orientation,
/// positioning, speed, and field of view.
/// The active camera defines what's heard and seen
//===================================================================================
#pragma once

/// Enum for box in frustum
enum {
CULL_EXCLUSION = 0, /// aka 'false'
CULL_INTERSECT = 1, /// aka 'true'
CULL_INCLUSION = 2, /// aka 'true'
};

/// \brief A camera has a location, projection, and rotation
/// Cameras are used to define the player's world-space orientation,
/// positioning, speed, and field of view.
//-----------------------------------------------------------------------------
/// The active camera defines what's heard and seen.
/// Cameras can even support multiple frustums, to avoid culling when mirrors
/// or shadow projectors are using out of view geometry
//-----------------------------------------------------------------------------
class ENGINE_API Camera {
private:

	/// Planes for the camera's view frustum
	struct Frustum{
		/// Planes for the camera's view frustum
		struct Plane{
			Vector n;
			float d;
		};
		Plane planes[6];
	};
	Frustum cameraFrustum;

	int BoxInFrustum(const BBox& box, bool ignoreNearFar, Frustum& frustum);
	void CreateClipPlanes(Frustum& frustum, Matrix& vp);

	/// Extra frustums for when we have
	vector<Frustum> frustums;

public:

	/// These will be updated during Update()/SetProjection()
	Matrix projection;
	Matrix view;
	Matrix viewProjection;
	/// Update projection on next update call?
	bool bUpdateProjection;

public:
	/// View
	Vector Location;
	Vector Velocity;
	Vector Direction;
	float  Roll; /// i.e. tilt

	/// Projection
	float Fov, NearClip, FarClip;
	float OldFov,OldNearClip,OldFarClip;


	Camera();

	/// Is bounding box in view frustum
	int BoxInFrustum(const BBox& box, bool ignoreNearFar=false);
	
	/// Updates clip planes and matrices
	/// Call after setting or updating member variables for view and projection
	/// This builds the view and projection matrices
	void Update();

	/// Called by Update, this builds the frustum clip planes from the
	/// current view and projection matrices
	void CreateClipPlanes();

	void AddFrustum(Matrix& view, Matrix& projection);
	void ClearFrustums(){ frustums.clear(); }
};

