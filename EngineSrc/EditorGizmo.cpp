//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ==============
//	Maya/3dsmax style rotation and translation gizmo, and supporting ArcBall class
//
//=======================================================================================
#include "stdafx.h"
#include "EditorGizmo.h"
#include "HDR.h"

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
D3DMATRIX Create_matViewport(const D3DVIEWPORT9& Viewport)
{
	float   dwWidth = (float)Viewport.Width;
	float   dwHeight = (float)Viewport.Height;
	float   dwX  = (float)Viewport.X;
	float   dwY  = (float)Viewport.Y;
	float   dvMinZ = Viewport.MinZ;
	float   dvMaxZ = Viewport.MaxZ;

	// This is the formula from the DirectX 8 documentation
	return D3DXMATRIX(
		dwWidth/2, 0.0f,  0.0f,  0.0f,
		0.0f,  -dwHeight/2, 0.0f,  0.0f,
		0.0f,  0.0f,  dvMaxZ-dvMinZ, 0.0f,
		dwX+dwWidth/2, dwY+dwHeight/2, dvMinZ,  1.0f
		);
}


//--------------------------------------------------------------------------------------
// Project 3D->2D
//--------------------------------------------------------------------------------------
Vector Gizmo::To2D(Vector& v){
	Vector vec = v;
	D3DXVec3TransformCoord( (D3DXVECTOR3*)&vec, (D3DXVECTOR3*)&vec, (D3DXMATRIX*)&m_ScreenSpace );
	return vec;
}

//--------------------------------------------------------------------------------------
// Initializer
//--------------------------------------------------------------------------------------
void Gizmo::Initialize()
{
	TRANSFORM_PLANEMODE = Input::Instance()->GetControlHandle("MoveAlongPlane");

	// Create cylinder and give it normals
	DXASSERT(D3DXCreateCylinder(RenderWrap::dev,0,1.0f,3.0f,15,15,&m_pArrow,NULL));
	LPD3DXMESH newMesh;
	m_pArrow->CloneMeshFVF(D3DXMESH_MANAGED,D3DFVF_XYZ | D3DFVF_NORMAL,RenderWrap::dev,&newMesh);
	SAFE_RELEASE(m_pArrow);
	m_pArrow = newMesh;
	D3DXComputeNormals(m_pArrow,0);

	DXASSERT(D3DXCreateBox(RenderWrap::dev,1.2f,1.2f,1.2f,&m_pCube,NULL));
}

//--------------------------------------------------------------------------------------
// Gizmo logic
//--------------------------------------------------------------------------------------
void Gizmo::OnUpdate(bool mouseDown, Vector2 mousePos, Camera& cam, Matrix& objMatrix)
{
	bool mouseJustDown = mouseDown && !m_bMouseDown;
	if(!m_Initialized){
		Initialize();
		m_Initialized = true;
	}

	if(m_PrevCursorPos.x == 0 && m_PrevCursorPos.y == 0)
		m_PrevCursorPos = mousePos;

	m_ArcBall.SetView(*(D3DXMATRIX*)&cam.view);
	// If mouse released in rotation mode, stop rotation
	if(m_Mode == MODE_ROTATE && !mouseDown && m_bMouseDown){
		m_ArcBall.OnEnd();
		m_PrevCursorPos.x = 0;
		m_PrevCursorPos.y = 0;
	}
	if(m_Mode == MODE_ROTATE && mouseJustDown){
		Matrix m_StartRot  = objMatrix.GetRotationMatrix() * cam.view.GetRotationMatrix();
		D3DXQUATERNION quat;
		D3DXQuaternionRotationMatrix( &quat, (D3DXMATRIX*)&m_StartRot );
		m_ArcBall.SetQuatNow( quat );
		m_ArcBall.OnBegin(mousePos.x,mousePos.y);
	}

	// Handle our plane mode, where user control-clicks a line to transform on the plane
	// formed by the line's normal
	if(mouseJustDown && Input::Instance()->ControlDown(TRANSFORM_PLANEMODE)){
		// Find the other two planes NOT selected
		DWORD flags = GIZMO_X | GIZMO_Y | GIZMO_Z;
		flags &= ~m_Axis;
		m_Axis = flags;
	}

	m_bMouseDown	= mouseDown;
	m_View			= cam.view;
	m_Project		= cam.projection;
	m_ViewDir		= cam.Direction;
	m_CursorPos		= mousePos;
	m_Object		= objMatrix;

	// Create screenspace matrix once only, for fast transforming
	D3DVIEWPORT9 viewport;
	RenderWrap::dev->GetViewport(&viewport);
	Matrix matWorld;
	D3DXMATRIX matViewport = Create_matViewport(viewport);
	D3DXMatrixMultiply ( &m_ScreenSpace, (D3DXMATRIX*)&matWorld, (D3DXMATRIX*)&m_View );
	D3DXMatrixMultiply ( &m_ScreenSpace, &m_ScreenSpace, (D3DXMATRIX*)&m_Project );
	D3DXMatrixMultiply ( &m_ScreenSpace, &m_ScreenSpace, (D3DXMATRIX*)&matViewport );

	// Unproject axes lines to 2D
	Vector loc = objMatrix[3];
	Vector  cen = loc;
	Matrix mat;
	m_Center2D = To2D(loc);
	float distance = (*(Vector*)&(m_View.Inverse()[3]) - *(Vector*)&loc).Length()*m_LineLength;
	// Scale line length by baseline screen width so it stays fixed size
	float scale = 800.f / RenderDevice::Instance()->GetViewportX();
	distance *= scale;

	m_X3D = cen;
	m_Y3D = cen;
	m_Z3D = cen;

	m_X3D.x += distance;
	m_X2D = To2D(m_X3D);
	
	m_Y3D.y += distance;
	m_Y2D = To2D(m_Y3D);
	
	m_Z3D.z += distance;
	m_Z2D = To2D(m_Z3D);

	// Calculate object matrix
	if(m_bMouseDown)
		UpdateMovement();

	// Which axes is mouse over?
	// !PLANEMODE is a hack to stop it overriding planemode
	if(!m_bMouseDown || (mouseJustDown && !Input::Instance()->ControlDown(TRANSFORM_PLANEMODE))){
		if(m_Mode == MODE_TRANSLATE || m_Mode == MODE_SCALE){
			m_Axis = 0;
			if(TestLine(m_Center2D,m_X2D))
				m_Axis |= GIZMO_X;
			if(TestLine(m_Center2D,m_Y2D))
				m_Axis |= GIZMO_Y;
			if(TestLine(m_Center2D,m_Z2D))
				m_Axis |= GIZMO_Z;
		}
	}

	m_PrevCursorPos = m_CursorPos;
}

//--------------------------------------------------------------------------------------
// Actual Gizmo transform/rotation calculation
//--------------------------------------------------------------------------------------
void Gizmo::UpdateMovement()
{
	int numAxes = 0;
	if(m_Axis&GIZMO_X)
		numAxes++;
	if(m_Axis&GIZMO_Y)
		numAxes++;
	if(m_Axis&GIZMO_Z)
		numAxes++;

	//
	// Translation Gizmo
	//
	if(m_Mode == MODE_TRANSLATE || m_Mode == MODE_SCALE)
	{
		D3DXMATRIX objMatrix;
		D3DXMatrixIdentity(&objMatrix);

		objMatrix._41 = m_Object[3].x;
		objMatrix._42 = m_Object[3].y;
		objMatrix._43 = m_Object[3].z;

		D3DVIEWPORT9 viewport;
		RenderWrap::dev->GetViewport(&viewport);
		D3DXMATRIX matView    =*(D3DXMATRIX*)&m_View;
		D3DXMATRIX matProject =*(D3DXMATRIX*)&m_Project;

		// In the picking code "world" could be set to be the average position of the 3 vertices of the picked triangle
		// At the moment it follows the object center (if it's at 0,0,0)
		// the 2nd arg is the prims' origin..
		D3DXVECTOR3 screen,screenX,screenY;
		D3DXVec3Project(&screen,&D3DXVECTOR3(0,0,0),&viewport,&matProject,&matView,&objMatrix);

		float z = 1; // Infinity, so we project on plane
		if(numAxes == 3)
			z = screen.z; // Non-axis constrained must use z component
		D3DXVECTOR3 old_world, new_world, view;
		
		// View->3D projected point
		screen = *(D3DXVECTOR3*)&(m_View.Inverse()[3]);
		D3DXVec3Unproject(&view,&screen,&viewport,&matProject,&matView,&objMatrix);
		// Last Mouse -> 3D point
		screen = D3DXVECTOR3(m_PrevCursorPos.x,m_PrevCursorPos.y,z);
		D3DXVec3Unproject(&old_world,&screen,&viewport,&matProject,&matView,&objMatrix);
		// Current Mouse -> 3D point
		screen = D3DXVECTOR3(m_CursorPos.x,m_CursorPos.y,z);
		D3DXVec3Unproject(&new_world,&screen,&viewport,&matProject,&matView,&objMatrix);

		// Delta movement
		Vector delta;

		//
		// If moving along one axis, we simply use MouseDir.MovementDir and calculate
		// the ratio of 2D to 3D movement
		//
		if(numAxes == 1)
		{
			Vector2 center  = *(Vector2*)&To2D(m_Object[3]);
			Vector axis3D = Vector(m_Axis&GIZMO_X?1:0,m_Axis&GIZMO_Y?1:0,m_Axis&GIZMO_Z?1:0);
			Vector2 offset  = *(Vector2*)&To2D(m_Object[3]+axis3D);
			Vector2 axisDir =  center - offset;
			// Get number of pixels to travel 1 meter along X
			float pixelsPerMeter = axisDir.Length();
			
			// How many "axis" pixels has cursor travelled?
			Vector2 mouseDir = m_CursorPos - m_PrevCursorPos;
			float pixelsMouse = mouseDir.Length();
			
			// How much has mouse moved along our direction/axis of rotation?
			mouseDir.Normalize();
			axisDir.Normalize();
			float incidentAngle = -mouseDir.Dot(axisDir);
			pixelsMouse = pixelsMouse * incidentAngle;

			// Set our movement. Only using one axis, but set all of them
			delta.x = pixelsMouse/pixelsPerMeter;
			delta.y = delta.x;
			delta.z = delta.x;

			if(m_Mode == MODE_SCALE)
				delta /= fObjectSize;
		}
		//
		// If we're axis constrained along a plane (2 axes), figure out the projection plane
		//
		else if(numAxes == 2)
		{
			bool bX = m_Axis&GIZMO_X?true:false, bY = m_Axis&GIZMO_Y?true:false, bZ = m_Axis&GIZMO_Z?true:false;

			// Get projection plane. It's the plane who's normal is the axis NOT selected
			Vector planeNormal;
			if(bX && bY)
				planeNormal = Vector(0,0,1);
			if(bX && bZ)
				planeNormal = Vector(0,1,0);
			if(bZ && bY)
				planeNormal = Vector(1,0,0);

			//
			// TIM: I had too much beer when I wrote this code, I can't even remember what it was meant to be doing!
			//
			// Vector planeNormal = m_ViewDir.GetNormal();
			// If projection plane goes against the translation plane, use the next major plane
			//if(planeNormal.x && bX)
			//	planeNormal = Vector(0,m_ViewDir.y,m_ViewDir.z).GetNormal();
			//if(planeNormal.y && bY)
			//	planeNormal = Vector(m_ViewDir.x,0,m_ViewDir.z).GetNormal();
			//if(planeNormal.z && bZ)
			//	planeNormal = Vector(m_ViewDir.x,m_ViewDir.y,0).GetNormal();
			//
			// Calculate the minor plane
			//Vector planeNormal2;
			//if(planeNormal.x)
			//	planeNormal2 = Vector(0,bY?0:m_ViewDir.y,bZ?0:m_ViewDir.z).GetNormal();
			//if(planeNormal.y)
			//	planeNormal2 = Vector(bX?0:m_ViewDir.x,0,bZ?0:m_ViewDir.z).GetNormal();
			//if(planeNormal.z)
			//	planeNormal2 = Vector(bX?0:m_ViewDir.x,bY?0:m_ViewDir.y,0).GetNormal();
			
			// Project our movement onto this limiting plane
			D3DXPLANE plane;
			D3DXPlaneFromPointNormal( &plane, &D3DXVECTOR3(0,0,0/*m_Object[3].x,m_Object[3].y,m_Object[3].z*/),(D3DXVECTOR3*)&planeNormal );
			D3DXPlaneIntersectLine(&old_world, &plane, &view, &old_world);
			D3DXPlaneIntersectLine(&new_world, &plane, &view, &new_world);

			delta = *(Vector*)&new_world - *(Vector*)&old_world;
		}
		else if(numAxes == 3){
			delta = *(Vector*)&new_world - *(Vector*)&old_world;
		}

		// Move across axes of constraint
		if(m_Axis & GIZMO_X)
			m_Object[3].x += delta.x;
		if(m_Axis & GIZMO_Y)
			m_Object[3].y += delta.y;
		if(m_Axis & GIZMO_Z)
			m_Object[3].z += delta.z;

		//
		// Scale along applicable axes
		//
		if(m_Mode == MODE_SCALE){
			Matrix mRot = m_Object.GetRotationMatrix();
			if(numAxes == 3)
				delta.z = delta.x = delta.y;
			float x,y,z;
			m_Object.GetScales(x,y,z);

			if(m_Axis & GIZMO_X)
				x += delta.x/2;
			if(m_Axis & GIZMO_Y)
				y += delta.y/2;
			if(m_Axis & GIZMO_Z)
				z += delta.z/2;

			// Don't ever let the scale invert
			if(x <= 0.1f)
				x = 0.1f;
			if(y <= 0.1f)
				y = 0.1f;
			if(z <= 0.1f)
				z = 0.1f;

			Matrix mScale;
			mScale.SetScales(x,y,z);
			m_Object = mScale * mRot;
		}
	}
	//
	// Rotation Gizmo
	//
	else if(m_Mode == MODE_ROTATE)
	{
		// We use the lovely arcball for unconstrained rotation
		if(numAxes > 1){
			m_ArcBall.OnMove(m_CursorPos.x,m_CursorPos.y);
			m_Object = *(Matrix*)&*m_ArcBall.GetRotationMatrix() * m_View.Inverse();
		}
		// We use 3dsmax-style axis rotation when constrained
		// 3dsmax style finds the axis off the rotation gizmo, then if the mouse travels parallel to that it
		// will rotate the object
		else{
			Vector2 dir = m_RotationLine[1] - m_RotationLine[0];
			dir.Normalize();

			Vector2 mouseDir = m_CursorPos - m_PrevCursorPos;
			float amount = mouseDir.Length()/55.f;
			mouseDir.Normalize();
			// How much has mouse moved along our direction/axis of rotation?
			float incidentAngle = -mouseDir.Dot(dir);
			float angle = amount * incidentAngle;

			Matrix rot;
			Vector vRot;
			if(m_Axis & GIZMO_X)
				vRot.x = angle;
			if(m_Axis & GIZMO_Y)
				vRot.y = -angle; // Funky
			if(m_Axis & GIZMO_Z)
				vRot.z = angle;
			rot.SetRotations(vRot.x,vRot.y,vRot.z);

			Vector loc = m_Object[3];
			m_Object = m_Object * rot;
			m_Object[3] = loc;
		}
	}
}

//--------------------------------------------------------------------------------------
// Transform arrows!
//--------------------------------------------------------------------------------------
void Gizmo::DrawObjects(LPD3DXMESH mesh)
{
	// Default renderstates needed
	RenderWrap::SetRS(D3DRS_ALPHABLENDENABLE,FALSE);
	RenderWrap::SetRS(D3DRS_FILLMODE,D3DFILL_SOLID);
	RenderWrap::dev->SetVertexShader(NULL);
	RenderWrap::dev->SetPixelShader(NULL);
	RenderWrap::SetRS(D3DRS_LIGHTING,TRUE);
	RenderWrap::SetRS(D3DRS_FOGENABLE,FALSE);
	RenderWrap::SetRS(D3DRS_ZWRITEENABLE,FALSE);
	RenderWrap::SetRS(D3DRS_ZFUNC,D3DCMP_ALWAYS);
	RenderWrap::SetView(m_View);
	RenderWrap::SetProjection(m_Project);

	RenderWrap::dev->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_SELECTARG1);
	RenderWrap::dev->SetTextureStageState(0,D3DTSS_COLORARG1,D3DTA_DIFFUSE );
	RenderWrap::dev->SetTextureStageState(0,D3DTSS_COLORARG2,D3DTA_DIFFUSE);
	RenderWrap::dev->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_DISABLE);


	// Setup a light and material for our arrows
	D3DXVECTOR3 vecDir;
	D3DLIGHT9 light;
	ZeroMemory( &light, sizeof(light) );
	light.Type = D3DLIGHT_POINT;
	light.Range = 100000;
	light.Diffuse = light.Specular = light.Ambient = D3DXCOLOR(1,1,1,1);
	light.Position = D3DXVECTOR3(1000,1000,0);
	light.Attenuation0 = 1.0f; 
	RenderWrap::dev->SetLight( 0, &light );
	RenderWrap::dev->LightEnable( 0, TRUE);
	D3DMATERIAL9  mtl;
	ZeroMemory(&mtl, sizeof(D3DMATERIAL9) );
	mtl.Ambient  = D3DXCOLOR(0.3,.3,.3,.3);

	// Get our fixed scale so the arrows never get bigger or smaller
	float scale = (*(Vector*)&(m_View.Inverse()[3]) - *(Vector*)&m_Object[3]).Length()*0.01f;
	// Scale line length by baseline screen width so it stays fixed size
	scale *= 800.f / RenderDevice::Instance()->GetViewportX();

	Matrix rot;
	Matrix world;

	D3DXCOLOR highlight = D3DXCOLOR(1,1,0,1);
	// +X Matrix
	rot.SetRotations(0,DEG2RAD(270),0);
	world.SetScales(scale,scale,scale);
	world *= rot;
	world[3] = m_X3D;
	// +X Rendering
	RenderWrap::SetWorld(world);
	mtl.Diffuse = m_Axis&GIZMO_X?highlight:D3DXCOLOR(1,0,0,1);
	RenderWrap::SetMaterial(&mtl);
	mesh->DrawSubset(0);
	// +Y Matrix
	rot.SetRotations(DEG2RAD(90),0,0);
	world = Matrix();
	world.SetScales(scale,scale,scale);
	world *= rot;
	world[3] = m_Y3D;
	// +Y Rendering
	RenderWrap::SetWorld(world);
	mtl.Diffuse = m_Axis&GIZMO_Y?highlight:D3DXCOLOR(0,1,0,1);
	RenderWrap::SetMaterial(&mtl);
	mesh->DrawSubset(0);
	// +Z Matrix
	rot.SetRotations(DEG2RAD(180),0,0);
	world = Matrix();
	world.SetScales(scale,scale,scale);
	world *= rot;
	world[3] = m_Z3D;
	// +Z Rendering
	RenderWrap::SetWorld(world);
	mtl.Diffuse = m_Axis&GIZMO_Z?highlight:D3DXCOLOR(0,0,1,1);
	RenderWrap::SetMaterial(&mtl);
	mesh->DrawSubset(0);

	RenderWrap::SetRS(D3DRS_ZFUNC,D3DCMP_LESSEQUAL);
	RenderWrap::SetRS(D3DRS_ZWRITEENABLE,TRUE);
}

//--------------------------------------------------------------------------------------
// Renders 3D gizmo colored lines 
//--------------------------------------------------------------------------------------
void Gizmo::OnRender()
{
	if(!m_Initialized){
		Initialize();
		m_Initialized = true;
	}

	// Is gizmo in view?
	Vector objDir = (this->m_View.Inverse()[3] - m_Object[3]).Normalized();
	m_ViewDir.Normalize();
	float deg = objDir.Dot(m_ViewDir);
	if(deg > -0.25f)
		return;


	LPD3DXLINE line = Canvas::Instance()->m_Line;
	line->SetAntialias(true);
	line->SetWidth(1.0f);
	DXASSERT(line->Begin());

	int size = 8;
	if(m_Mode == MODE_TRANSLATE || m_Mode == MODE_SCALE){

		DoLines(line);

		// Draw movement square
		D3DXVECTOR2 vLine[5]={D3DXVECTOR2(m_Center2D.x-size,m_Center2D.y-size),D3DXVECTOR2(m_Center2D.x+size,m_Center2D.y-size),
			D3DXVECTOR2(m_Center2D.x+size,m_Center2D.y+size),D3DXVECTOR2(m_Center2D.x-size,m_Center2D.y+size),
			D3DXVECTOR2(m_Center2D.x-size-1,m_Center2D.y-size-1)};
		line->Draw(vLine,5,0xffffff00);
	}
	// Rotation widget. Also handles mouseover messages here for speed
	// FIXME: Handle mouseover messages in proper place
	else
	{
		DWORD newAxis = 0; // Intermediate var so we don't overwrite m_Axis before calling
		if(DoCircle(line,GIZMO_X))
			newAxis |= GIZMO_X;
		if(DoCircle(line,GIZMO_Y))
			newAxis |= GIZMO_Y;
		if(DoCircle(line,GIZMO_Z))
			newAxis |= GIZMO_Z;

		if(!m_bMouseDown)
			m_Axis = newAxis;
		// Nothing selected, but mouse is down, so see if it's within radius of our widget for "free"
		// transform
		else if(m_Axis == 0)
		{
			Vector2 len = Vector2(m_Center2D.x,m_Center2D.y)-m_CursorPos;
			float dist = len.Length();
			float radius = 70;
			if(dist < radius)
				m_Axis = GIZMO_X | GIZMO_Y | GIZMO_Z;
		}

		m_ArcBall.SetAxis(D3DXVECTOR3(m_Axis&GIZMO_X?1:0,m_Axis&GIZMO_Y?1:0,m_Axis&GIZMO_Z?1:0));
	}

	line->End();

	if(m_Mode == MODE_TRANSLATE)
		DrawObjects(m_pArrow);
	if(m_Mode == MODE_SCALE)
		DrawObjects(m_pCube);
}
//--------------------------------------------------------------------------------------
// Translation widget lines
//--------------------------------------------------------------------------------------
void Gizmo::DoLines(LPD3DXLINE line)
{
	D3DXVECTOR2 vLine[2];

	DWORD cX = 0xffff0000, cY = 0xff00ff00, cZ = 0xff0000ff;
	if(m_Axis & GIZMO_X)
		cX = 0xffffff00;
	if(m_Axis & GIZMO_Y)
		cY = 0xffffff00;
	if(m_Axis & GIZMO_Z)
		cZ = 0xffffff00;

	// +X
	vLine[0] = *(D3DXVECTOR2*)&m_Center2D;
	vLine[1] = *(D3DXVECTOR2*)&m_X2D;
	DXASSERT(line->Draw(vLine,2,cX));
	// +Y
	vLine[1] = *(D3DXVECTOR2*)&m_Y2D;
	line->Draw(vLine,2,cY);
	// +Z
	vLine[1] = *(D3DXVECTOR2*)&m_Z2D;
	line->Draw(vLine,2,cZ);
}

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void Gizmo::DrawAxisIcon(Camera& cam)
{
	// Create screenspace matrix once only, for fast transforming
	D3DVIEWPORT9 viewport;
	RenderWrap::dev->GetViewport(&viewport);
	Matrix matWorld;
	D3DXMATRIX matViewport = Create_matViewport(viewport);
	D3DXMatrixMultiply ( &m_ScreenSpace, (D3DXMATRIX*)&matWorld, (D3DXMATRIX*)&cam.view );
	D3DXMatrixMultiply ( &m_ScreenSpace, &m_ScreenSpace, (D3DXMATRIX*)&cam.projection );
	D3DXMatrixMultiply ( &m_ScreenSpace, &m_ScreenSpace, (D3DXMATRIX*)&matViewport );

	LPD3DXLINE line = Canvas::Instance()->m_Line;
	line->SetAntialias(true);
	line->SetWidth(1.0f);
	DXASSERT(line->Begin());

	Vector pos = (cam.view.Inverse()[3]) + cam.Direction*10;

	// Scale line length by baseline screen width so it stays fixed size
	float scale = 400.f / RenderDevice::Instance()->GetViewportX();

	Vector x = pos;
	Vector y = pos;
	Vector z = pos;

	x.x += scale;
	y.y += scale;
	z.z += scale;

	D3DXVECTOR2 vLine[2];

	DWORD cX = 0xffff0000, cY = 0xff00ff00, cZ = 0xff0000ff;

	Vector offset = Vector(-RenderDevice::Instance()->GetViewportX()/2 + 20,RenderDevice::Instance()->GetViewportY()/2 - 20,0);
	Vector pos2D = To2D(pos) + offset;
	Vector X2D = To2D(x) + offset;
	Vector Y2D = To2D(y) + offset;
	Vector Z2D = To2D(z) + offset;

	// +X
	vLine[0] = *(D3DXVECTOR2*)&pos2D;
	vLine[1] = *(D3DXVECTOR2*)&X2D;
	DXASSERT(line->Draw(vLine,2,cX));
	// +Y
	vLine[1] = *(D3DXVECTOR2*)&Y2D;
	line->Draw(vLine,2,cY);
	// +Z
	vLine[1] = *(D3DXVECTOR2*)&Z2D;
	line->Draw(vLine,2,cZ);
	line->End();

	// Axis names
	Canvas* c = RenderDevice::Instance()->GetCanvas();
	c->Textf(SystemFont,cX,X2D.x,X2D.y-8,"x");
	c->Textf(SystemFont,cZ,Z2D.x,Z2D.y-8,"z");
	c->Textf(SystemFont,cY,Y2D.x-1.5f,Y2D.y-14,"y");
}

//--------------------------------------------------------------------------------------
// Renders rotation circle gizmo for specified axis. Also hit-tests circle
// and returns state
//
// TODO: Max/Maya style sphere that doesn't show "backfacing" rotation lines
//--------------------------------------------------------------------------------------
bool Gizmo::DoCircle(LPD3DXLINE line, DWORD axis)
{
	float radius = (*(Vector*)&(m_View.Inverse()[3]) - *(Vector*)&m_Object[3]).Length()*m_LineLength;

	// Scale radius by baseline screen width so it stays fixed size
	float scale = 800.f / RenderDevice::Instance()->GetViewportX();
	radius *= scale;

	// generate line data
	vector<Vector2> LineVertices;
	// Half number of segments
	int segsHalf = 16;

	// first/last point
	Vector pt0 =  m_Object[3]+Vector(axis&GIZMO_Z||axis&GIZMO_Y?radius:0, axis&GIZMO_X?radius:0.0f, 0.0f);
	LineVertices.push_back(*(Vector2*)&To2D(pt0));

	// computed points
	Vector pt;
	for (int i = LineVertices.size(); i < segsHalf*2; i++)
	{
		pt = m_Object[3];
		// Figure out axis to draw circle along
		if(axis&GIZMO_Z){
			pt.x += (float)(radius * cosf(i * D3DX_PI / segsHalf));
			pt.y += (float)(radius * sinf(i * D3DX_PI / segsHalf));
		}
		else if(axis&GIZMO_Y){
			pt.x += (float)(radius * cosf(i * D3DX_PI / segsHalf));
			pt.z += (float)(radius * sinf(i * D3DX_PI / segsHalf));
		}
		else if(axis&GIZMO_X){
			pt.y += (float)(radius * cosf(i * D3DX_PI / segsHalf));
			pt.z += (float)(radius * sinf(i * D3DX_PI / segsHalf));
		}

		LineVertices.push_back(*(Vector2*)&To2D(pt));
	}

	LineVertices.push_back(*(Vector2*)&To2D(pt0));

	// Do hit test against circle, checking every "stride"th segment for speed
	bool bHit = false;
	if(!m_bMouseDown){
		int stride = 1;
		
		float radius2 = m_TestRadius; // Hacky!
		m_TestRadius = m_TestRadius/2;
		for(int i=stride;i<segsHalf*2;i+=stride){
			if(TestLine(*(Vector*)&LineVertices[i-stride],*(Vector*)&LineVertices[i])){
				bHit = true;
				//
				// Calculate and draw the line shooting off from this point on the circle. This is our mouse axis
				//
				int index = i-stride-1;
				if(index < 0)
					index = 0;
				Vector2 dir = LineVertices[index] - LineVertices[i+1];
				dir.x /= 2;
				dir.y /= 2;
				m_RotationLine[0] = LineVertices[i-stride];
				m_RotationLine[1] = LineVertices[i-stride]+dir;
				break;
			}
		}
		m_TestRadius = radius2;
	}
	else{
		// Mouse is down over gizmo, so show exit line
		//line->Draw((D3DXVECTOR2*)&m_RotationLine[0], 2, 0xffffff00);
	}

	DWORD color;
	if(axis & GIZMO_X)
		color = 0xffff0000;
	if(axis & GIZMO_Y)
		color = 0xff00ff00;
	if(axis & GIZMO_Z)
		color = 0xff0000ff;

	if((bHit||(m_Axis & GIZMO_X && m_bMouseDown)) && axis & GIZMO_X)
		color = 0xffffff00;
	if((bHit||(m_Axis & GIZMO_Y && m_bMouseDown)) && axis & GIZMO_Y)
		color = 0xffffff00;
	if((bHit||(m_Axis & GIZMO_Z && m_bMouseDown)) && axis & GIZMO_Z)
		color = 0xffffff00;

	line->Draw((D3DXVECTOR2*)&LineVertices[0], LineVertices.size(), color);

	return bHit;
}

//--------------------------------------------------------------------------------------
CArcBall::CArcBall()
{
    Reset();
    m_vDownPt = D3DXVECTOR3(0,0,0);
    m_vCurrentPt = D3DXVECTOR3(0,0,0);

    RECT rc;
    GetClientRect( GetForegroundWindow(), &rc );
    SetWindow( rc.right, rc.bottom );
}


//--------------------------------------------------------------------------------------
void CArcBall::Reset()
{
    D3DXQuaternionIdentity( &m_qDown );
    D3DXQuaternionIdentity( &m_qNow );
    D3DXMatrixIdentity( &m_mRotation );
    D3DXMatrixIdentity( &m_mTranslation );
    D3DXMatrixIdentity( &m_mTranslationDelta );
    m_bDrag = FALSE;
    m_fRadiusTranslation = 1.0f;
    m_fRadius = 0.5f;
}

//--------------------------------------------------------------------------------------
D3DXVECTOR3 CArcBall::ScreenToVector( float fScreenPtX, float fScreenPtY )
{
    // Scale to screen
    FLOAT x   = -(fScreenPtX - m_nWidth/2)  / (m_fRadius*m_nWidth/2);
    FLOAT y   =  (fScreenPtY - m_nHeight/2) / (m_fRadius*m_nHeight/2);

    FLOAT z   = 0.0f;
    FLOAT mag = x*x + y*y;

    if( mag > 1.0f )
    {
        FLOAT scale = 1.0f/sqrtf(mag);
        x *= scale;
        y *= scale;
    }
    else
        z = sqrtf( 1.0f - mag );

    // Return vector
    return D3DXVECTOR3( x, y, z );
}


//--------------------------------------------------------------------------------------
// Constrains arcball to one axis. Thanks to Graphic Gems
//--------------------------------------------------------------------------------------
D3DXVECTOR3 ConstrainToAxis(D3DXVECTOR3 loose, D3DXVECTOR3 axis)
{
    D3DXVECTOR3 onPlane;
    register float norm;
    onPlane = loose - axis*D3DXVec3Dot(&axis, &loose);
    norm = D3DXVec3LengthSq(&onPlane);
    if (norm > 0.0) {
	if (onPlane.z < 0.0) onPlane = -onPlane;
	return ( onPlane* 1/sqrt(norm) );
    } /* else drop through */
    if (axis.z == 1) {
	onPlane = D3DXVECTOR3(1.0, 0.0, 0.0);
    } else {
	 D3DXVec3Normalize(&onPlane,&D3DXVECTOR3(-axis.y, axis.x, 0.0));
    }
    return (onPlane);
}

//--------------------------------------------------------------------------------------
D3DXQUATERNION CArcBall::QuatFromBallPoints(const D3DXVECTOR3 &vFrom1, const D3DXVECTOR3 &vTo1)
{
	D3DXVECTOR3 vTo = vTo1;
	D3DXVECTOR3 vFrom = vFrom1;
	D3DXVECTOR4 temp;

	// Constrain to axis, but first convert from view space to local space, then back again
	if(m_Axis.x == 0 || m_Axis.y == 0 || m_Axis.z == 0)
	{
		D3DXMATRIX mInv;
		D3DXMatrixInverse(&mInv,NULL,&m_mView);
		D3DXVec3Transform(&temp,&vTo,&mInv);
		vTo = *(D3DXVECTOR3*)&temp;
		D3DXVec3Transform(&temp,&vFrom,&mInv);
		vFrom = *(D3DXVECTOR3*)&temp;
		vTo = ConstrainToAxis(vTo,m_Axis);
		vFrom = ConstrainToAxis(vFrom,m_Axis);
		D3DXVec3Transform(&temp,&vTo,&m_mView);
		vTo = *(D3DXVECTOR3*)&temp;
		D3DXVec3Transform(&temp,&vFrom,&m_mView);
		vFrom = *(D3DXVECTOR3*)&temp;
	}

    D3DXVECTOR3 vPart;
    float fDot = D3DXVec3Dot(&vFrom, &vTo);
    D3DXVec3Cross(&vPart, &vFrom, &vTo);

    return D3DXQUATERNION(vPart.x, vPart.y, vPart.z, fDot);
}

//--------------------------------------------------------------------------------------
void CArcBall::OnBegin( int nX, int nY )
{
    m_bDrag = true;
    m_qDown = m_qNow;
    m_vDownPt = ScreenToVector( (float)nX, (float)nY );
}




//--------------------------------------------------------------------------------------
void CArcBall::OnMove( int nX, int nY )
{
    if (m_bDrag) 
    { 
        m_vCurrentPt = ScreenToVector( (float)nX, (float)nY );
        m_qNow = m_qDown * QuatFromBallPoints( m_vDownPt, m_vCurrentPt );
    }
}




//--------------------------------------------------------------------------------------
void CArcBall::OnEnd()
{
    m_bDrag = false;
}
