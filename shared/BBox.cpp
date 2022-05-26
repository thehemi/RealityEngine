//=========== Copyright (c) 2003, Tim Johnson. All rights reserved. ===========
/// BBox:
/// Axis-aligned bounding box (AABB) class
//=============================================================================
#include "stdafx.h"
#include "BBox.h"

/// Don't take guesses with what indexes to what. this is a bit screwy
int BBox::facevert[6][4]=
		{	{1,7,2,4},{0,5,3,6},{1,4,6,3},
			{0,2,7,5},{1,3,5,7},{0,6,4,2}	};

int BBox::edgevert[12][2]=
		{	{0,6},{6,4},{4,2},{2,0},
			{1,3},{3,5},{5,7},{7,1},
			{0,5},{3,6},{4,1},{7,2}		};

int BBox::edgefaces[12][2]=
		{	{0,2},{4,2},{3,2},{1,2},
			{4,5},{0,5},{1,5},{3,5},
			{0,1},{0,4},{3,4},{1,3}		};

#define COS45			0.7071067811865475244f
float BBox::vertnorm[8][3]=
		{	{-COS45,-COS45,-COS45},
			{ COS45, COS45, COS45},
			{ COS45,-COS45,-COS45},
			{-COS45, COS45, COS45},
			{ COS45, COS45,-COS45},
			{-COS45,-COS45, COS45},
			{-COS45, COS45,-COS45},
			{ COS45,-COS45, COS45}	};

float BBox::edgenorm[12][3]=
		{	{-COS45,		 0,-COS45},
			{		  0, COS45,-COS45},
			{ COS45,		 0,-COS45},
			{		  0,-COS45,-COS45},
			{		  0, COS45, COS45},
			{-COS45,		 0, COS45},
			{		  0,-COS45, COS45},
			{ COS45,		 0, COS45},
			{-COS45,-COS45,			0},
			{-COS45, COS45,			0},
			{ COS45, COS45,			0},
			{ COS45,-COS45,			0}	};

/// Don't index into these, they aren't logically ordered to match the others
float BBox::facenorm[6][3]=
{	{-1,0,0}, 
	{0,-1,0}, 
	{0,0,-1}, 
	{1,0,0}, 
	{0,1,0}, 
	{0,0,1} 
};



/*
  float BBox::facenorm[6][3]=
{	{-1,0,0},//{-1,0,0}, /// x max
	{1,0,0},		//{0,-1,0}, /// x min
	{0,-1,0},		//{0,0,-1}, /// y max
	{0,1,0},			//{1,0,0},  /// y min
	{0,0,-1},		//{0,1,0},  /// z max
	{0,0,1},			//{0,0,1} }; /// z min
};
*/
  