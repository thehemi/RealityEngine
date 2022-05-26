//=========== (C) Copyright 2004, Artficial Studios. All rights reserved. =======
// Plane Class
// Author: Mostafa Mohamed
//
//
//===============================================================================

#ifndef PLANE_INCLUDED
#define PLANE_INCLUDED

class CPlane
{
public:
    Vector normal;
    float distance;
public:
    CPlane(){}
    CPlane(Vector planeNormal,float planeDistance);

    float GetDistance(Vector point)
    {
        return normal.Dot(point) -  distance;
    }
};

#endif