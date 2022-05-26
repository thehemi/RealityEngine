//=========== (C) Copyright 2004, Artficial Studios. All rights reserved. =======
// Plane Class
// Author: Mostafa Mohamed
//
// ALOT TO BE ADDED
//===============================================================================

#include "StdAfx.h"
#include "vector.h"
#include "plane.h"

CPlane::CPlane(Vector planeNormal,float planeDistance)
{
    normal = planeNormal;
    distance = planeDistance;
}

