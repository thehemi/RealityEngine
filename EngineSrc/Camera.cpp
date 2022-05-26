//=========== Copyright (c) 2003, Tim Johnson. All rights reserved. ===========
// A camera has a location, projection, and rotation
// The active camera defines what's heard and seen
//=============================================================================

#include "stdafx.h"
#include <d3dx9.h>

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
Camera::Camera(){
	bUpdateProjection = false;
	Fov = 90;
	NearClip = 0.170f;
	FarClip = 8000;
	Location = Vector(0,0,0);
	Direction = Vector(1,0,0);
	Roll = 0;
	Update();
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void Camera::Update(){
	// Build view mat
	view = RenderWrap::BuildView(Location,Direction,Roll);

	if(Fov != OldFov || NearClip != OldNearClip || FarClip != OldFarClip || bUpdateProjection)
	{
		projection = RenderWrap::BuildProjection(Fov,NearClip,FarClip);
		bUpdateProjection = false;
	}

	OldFov = Fov;
	OldNearClip = NearClip;
	OldFarClip = FarClip;

	viewProjection = view * projection;
	// Build new frustum planes for this frame. view * proj
	CreateClipPlanes();
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void Camera::AddFrustum(Matrix& view, Matrix& projection){
	Frustum f;
	CreateClipPlanes(f,view*projection);
	frustums.push_back(f);
}


//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void Camera::CreateClipPlanes(){
	CreateClipPlanes(cameraFrustum,viewProjection);
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void Camera::CreateClipPlanes(Frustum& frustum, Matrix& vp){
	// Need D3DX 'cause 4x4
	D3DXMATRIX   comboMatrix = *(D3DXMATRIX*)&vp;

	// Left clipping plane
	frustum.planes[0].n.x = -(comboMatrix._14 + comboMatrix._11);
	frustum.planes[0].n.y = -(comboMatrix._24 + comboMatrix._21);
	frustum.planes[0].n.z = -(comboMatrix._34 + comboMatrix._31);
	frustum.planes[0].d   = -(comboMatrix._44 + comboMatrix._41);

	// Right clipping plane
	frustum.planes[1].n.x = -(comboMatrix._14 - comboMatrix._11);
	frustum.planes[1].n.y = -(comboMatrix._24 - comboMatrix._21);
	frustum.planes[1].n.z = -(comboMatrix._34 - comboMatrix._31);
	frustum.planes[1].d   = -(comboMatrix._44 - comboMatrix._41);

	// Top clipping plane
	frustum.planes[2].n.x = -(comboMatrix._14 - comboMatrix._12);
	frustum.planes[2].n.y = -(comboMatrix._24 - comboMatrix._22);
	frustum.planes[2].n.z = -(comboMatrix._34 - comboMatrix._32);
	frustum.planes[2].d   = -(comboMatrix._44 - comboMatrix._42);

	// Bottom clipping plane
	frustum.planes[3].n.x = -(comboMatrix._14 + comboMatrix._12);
	frustum.planes[3].n.y = -(comboMatrix._24 + comboMatrix._22);
	frustum.planes[3].n.z = -(comboMatrix._34 + comboMatrix._32);
	frustum.planes[3].d   = -(comboMatrix._44 + comboMatrix._42);

	// Near clipping plane
	frustum.planes[4].n.x = -(comboMatrix._14 + comboMatrix._13);
	frustum.planes[4].n.y = -(comboMatrix._24 + comboMatrix._23);
	frustum.planes[4].n.z = -(comboMatrix._34 + comboMatrix._33);
	frustum.planes[4].d   = -(comboMatrix._44 + comboMatrix._43);

	// Far clipping plane
	frustum.planes[5].n.x = -(comboMatrix._14 - comboMatrix._13);
	frustum.planes[5].n.y = -(comboMatrix._24 - comboMatrix._23);
	frustum.planes[5].n.z = -(comboMatrix._34 - comboMatrix._33);
	frustum.planes[5].d   = -(comboMatrix._44 - comboMatrix._43);
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
int Camera::BoxInFrustum(const BBox& box, bool ignoreNearFar/*=false*/){
	// Check camera
	int ret = BoxInFrustum(box,ignoreNearFar,cameraFrustum);
	if(ret)
		return ret;

	// If apparently not visible, check other frustums to be safe
	for(int i=0;i<frustums.size();i++){
		ret = BoxInFrustum(box,ignoreNearFar,frustums[i]);
		if(ret)
			return ret;
	}

	return ret;
}


//-----------------------------------------------------------------------------
// Name: Camera::BoxInFrustum
//
// Desc: Culls an axis-aligned bounding box. The return codes are:
//       -- CULL_EXCLUSION (completely outside the frustum)
//       -- CULL_INTERSECT (partially visible)
//       -- CULL_INCLUSION (completely inside the frustum)
//-----------------------------------------------------------------------------
int Camera::BoxInFrustum(const BBox& box, bool ignoreNearFar, Frustum& f)
{

    Vector   minExtreme, maxExtreme;
    bool        intersect;

    // Initialize the intersection indicator.
    intersect = false;

	int count = 6;
	if(ignoreNearFar)
		count = 4;


    // For each of the six frustum planes
    for (int i = 0; i < count; i++)
    {
        // Find the minimum and maximum extreme points along the plane's
        // normal vector. Just understand this normal as a line of all
        // real numbers. 0 then lies on the plane, negative numbers are
        // in the halfspace opposing the normal, and positive numbers
        // are in the other halfspace. The terms "minimum" and "maximum"
        // should now make sense.
        for (int j = 0; j < 3; j++) // for each component x, y, and z.
        {
            if (f.planes[i].n[j] >= 0.0f)
            {
                minExtreme[j] = box.min[j];
                maxExtreme[j] = box.max[j];
            }
            else
            {
                minExtreme[j] = box.max[j];
                maxExtreme[j] = box.min[j];
            }
        }

        // If minimum extreme point is outside, then the whole AABB
        // must be outside.
        //if (m_frustumPlanes[i].DistanceToPoint(minExtreme) > 0.0f) return 0;
		if(f.planes[i].n.Dot(minExtreme) + f.planes[i].d > 0)
			return CULL_EXCLUSION;

        // The minimum extreme point is inside. Hence, if the maximum
        // extreme point is outside, then the AABB must intersect with
        // the plane. However, the AABB may still be outside another
        // plane.
        //if (m_frustumPlanes[i].DistanceToPoint(maxExtreme) >= 0.0f)
		if(f.planes[i].n.Dot( maxExtreme) + f.planes[i].d >= 0)
			intersect = true;
    }

    // The AABB is either partially or fully visible.
    if (intersect) return CULL_INTERSECT;

    return CULL_INCLUSION;
}


