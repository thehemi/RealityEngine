//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ==============
//	Maya/3dsmax style rotation and translation gizmo, and supporting ArcBall class
//
//=======================================================================================


//--------------------------------------------------------------------------------------
/// Calculates applied rotations in the rotation Editor Gizmo, using quaternions
class CArcBall
{
public:
    CArcBall();

    /// Functions to change behavior
    void Reset(); 
    void SetTranslationRadius( FLOAT fRadiusTranslation ) { m_fRadiusTranslation = fRadiusTranslation; }
    void SetWindow( INT nWidth, INT nHeight, FLOAT fRadius = 0.9f ) { m_nWidth = nWidth; m_nHeight = nHeight; m_fRadius = fRadius; m_vCenter = D3DXVECTOR2(m_nWidth/2.0f,m_nHeight/2.0f); }

    /// Call these from client and use GetRotationMatrix() to read new rotation matrix
    void OnBegin( int nX, int nY );  /// start the rotation (pass current mouse position)
    void OnMove( int nX, int nY );   /// continue the rotation (pass current mouse position)
    void OnEnd();                    /// end the rotation 

    /// Functions to get/set state
    D3DXMATRIX* GetRotationMatrix() { return D3DXMatrixRotationQuaternion(&m_mRotation, &m_qNow); };
    D3DXMATRIX* GetTranslationMatrix()      { return &m_mTranslation; }
    D3DXMATRIX* GetTranslationDeltaMatrix() { return &m_mTranslationDelta; }
    bool        IsBeingDragged()            { return m_bDrag; }
    D3DXQUATERNION GetQuatNow()             { return m_qNow; }
    void        SetQuatNow( D3DXQUATERNION q ) { m_qNow = q; }
	void		SetView(D3DXMATRIX view){ m_mView = view;m_mView._41 = 0;m_mView._42 = 0;m_mView._43 = 0; }
	void		SetAxis(D3DXVECTOR3 axis){ m_Axis = axis; }

    D3DXQUATERNION QuatFromBallPoints( const D3DXVECTOR3 &vFrom, const D3DXVECTOR3 &vTo );
    

protected:
	D3DXVECTOR3    m_Axis;
	D3DXMATRIXA16  m_mView;
    D3DXMATRIXA16  m_mRotation;         /// Matrix for arc ball's orientation
    D3DXMATRIXA16  m_mTranslation;      /// Matrix for arc ball's position
    D3DXMATRIXA16  m_mTranslationDelta; /// Matrix for arc ball's position

    INT            m_nWidth;   /// arc ball's window width
    INT            m_nHeight;  /// arc ball's window height
    D3DXVECTOR2    m_vCenter;  /// center of arc ball 
    FLOAT          m_fRadius;  /// arc ball's radius in screen coords
    FLOAT          m_fRadiusTranslation; /// arc ball's radius for translating the target

    D3DXQUATERNION m_qDown;             /// Quaternion before button down
    D3DXQUATERNION m_qNow;              /// Composite quaternion for current drag
    bool           m_bDrag;             /// Whether user is dragging arc ball

    POINT          m_ptLastMouse;      /// position of last mouse point
    D3DXVECTOR3    m_vDownPt;           /// starting point of rotation arc
    D3DXVECTOR3    m_vCurrentPt;        /// current point of rotation arc

    D3DXVECTOR3    ScreenToVector( float fScreenPtX, float fScreenPtY );
};


		
//-----------------------------------------------------------------------------
/// Gizmo for rotation, translation, and scaling
//-----------------------------------------------------------------------------
class Gizmo
{
protected:
	LPD3DXMESH	m_pArrow, m_pCube;
	Vector		m_X2D,m_Y2D,m_Z2D; /// 2D Projections
	Vector		m_X3D,m_Y3D,m_Z3D; /// 3D line/arrow points
	Vector		m_Center2D;
	Matrix		m_Object;
	Matrix		m_View, m_Project;
	Vector		m_ViewDir;
	D3DXMATRIX  m_ScreenSpace;
	Vector2		m_CursorPos, m_PrevCursorPos;
	Vector2		m_RotationLine[2]; /// Line off rotation gizmo in 2D
	float m_LineLength;
	float m_TestRadius;
	bool  m_bMouseDown;
	bool  m_Initialized;
	int	  TRANSFORM_PLANEMODE;
	CArcBall m_ArcBall;

	/// Translation widget lines
	void DoLines(LPD3DXLINE line);
	/// Renders rotation circle gizmo for specified axis
	bool DoCircle(LPD3DXLINE line, DWORD axis);
	/// Translation arrows
	void DrawObjects(LPD3DXMESH mesh);
	void UpdateMovement();
	void Initialize();

public:
	float	fObjectSize;
	Matrix GetScreenMatrix(){ return *(Matrix*)&m_ScreenSpace; }
	Vector GetLocation(){ return m_Object[3]; }
	Matrix GetRotation(){ return m_Object;    }
	Matrix GetScaling(){  return m_Object.GetScaleMatrix(); }

	static const int GIZMO_X = (1<<1);
	static const int GIZMO_Y = (1<<2);
	static const int GIZMO_Z = (1<<3);
	DWORD	m_Axis;
	enum Mode {
		MODE_TRANSLATE,
		MODE_ROTATE,
		MODE_SCALE,
	};
	Mode    m_Mode; /// Translate or rotation?
	
	Gizmo(){
		m_bMouseDown   = false;
		m_Mode		   = MODE_TRANSLATE;
		m_Axis = 0;
		m_LineLength = 0.15f;
		m_TestRadius = 14;
	}

	/// Mouse over gizmo?
	bool HasFocus()
	{
		return m_Axis != 0;
	}
	void LostFocus(){ m_Axis = 0; } /// Equivilent of killing focus
	/// Tests if mouse is touching line of gizmo
	bool TestLine(Vector& a, Vector& b)
	{
		/// Circle/line test
		double l = sqrtf((b.x-a.x)*(b.x-a.x)+(b.y-a.y)*(b.y-a.y));
		double s =((a.y-m_CursorPos.y)*(b.x-a.x)-
			(a.x-m_CursorPos.x)*(b.y-a.y))/(l*l);
		bool ret = (fabsf(s)*l) < m_TestRadius;
		if(ret){
			/// Circle intersects line, does it intersect segment?
			Vector2 vMin(b.x<a.x?b.x:a.x,b.y<a.y?b.y:a.y);
			Vector2 vMax(b.x>a.x?b.x:a.x,b.y>a.y?b.y:a.y);
			if(m_CursorPos.x+m_TestRadius < vMin.x || m_CursorPos.x-m_TestRadius > vMax.x || m_CursorPos.y+m_TestRadius < vMin.y || m_CursorPos.y-m_TestRadius > vMax.y)
				return false;
			return true;
		}
		return ret;
	}

	/// Project 3D->2D
	Vector To2D(Vector& v);

	/// Gizmo logic
	void OnUpdate(bool mouseDown, Vector2 mousePos, Camera& camera, Matrix& objMatrix);
	//
	void OnRender();
	/// Axis icon in bottom left
	void DrawAxisIcon(Camera& cam);
};