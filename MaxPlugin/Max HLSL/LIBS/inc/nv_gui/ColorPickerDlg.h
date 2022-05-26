/*********************************************************************NVMH4****
Path:  NVSDK\Common\include\nv_gui
File:  ColorPickerDlg.h

Copyright NVIDIA Corporation 2002
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED
*AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS
BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES
WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS,
BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS)
ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS
BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.



Comments:





******************************************************************************/

// ColorPickerDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CColorPickerDlg dialog
#include "Dib.h"

namespace ColorPicker
{

#define RADIUS		100
#define PI			3.14159265358
#define RECT_WIDTH	5
#define RAD2DEG(x)  ((180.0 * (x))/PI)
#define DEG2RAD(x)	(((x) * PI)/180.0)
#define TOSCALE(x)	(((x)*RADIUS)/255.0)
#define SCALETOMAX(x) (((x)*255.0)/RADIUS)
#define RED	0
#define GREEN 1
#define BLUE 2
#define BAD_SLOPE	1000000.0

struct HSVType;

struct RGBType
{
	COLORREF color() { return RGB(r,g,b); };
	HSVType toHSV();
	int r,g,b;
};

struct HSVType
{
	RGBType toRGB();
	int h,s,v;
};

struct LineDesc
{
	double x,y;
	double slope;
	double c;
};


int Distance(CPoint pt1,CPoint pt2);
CPoint PointOnLine(CPoint pt1,CPoint p2,int len,int maxlen);
double Slope(CPoint pt1,CPoint pt2);
double FindC(LineDesc& l); 
CPoint Intersection(LineDesc desc1,LineDesc desc2);
double AngleFromPoint(CPoint pt,CPoint center);
CPoint PtFromAngle(double angle,double sat,CPoint center);


class CColorPickerDlg : public CDialog
{
// Construction
public:
	CColorPickerDlg(const vec4& fColor,CWnd* pParent = NULL);   // standard constructor
	~CColorPickerDlg();
	COLORREF GetColor() { return color.color();};

// Dialog Data
	//{{AFX_DATA(CColorPickerDlg)
	enum { IDD = IDD_DIALOG_COLORS };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CColorPickerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CColorPickerDlg)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnSysColorChange();
	afx_msg void OnPaint();
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeEditBlue();
	afx_msg void OnChangeEditGreen();
	afx_msg void OnChangeEditHue();
	afx_msg void OnChangeEditRed();
	afx_msg void OnChangeEditSat();
	afx_msg void OnChangeEditVal();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void DrawFilledColor(CDC *pDC,CRect cr,COLORREF c);
	void DrawLines(CDC *pDC);
	void DrawXorRect(CDC *pDC,CRect& cr);
	void CalcSlopes();
	void CalcCuboid();

	void CreateBrightDIB();
	void SetDIBPalette();
	void DrawMarkers(CDC *pDC);
	void TrackPoint(CPoint pt);
	void CalcRects();
		
	BOOL InCircle(CPoint pt);
	BOOL InBright(CPoint pt);

	void SetSpinVals();
	virtual void SetEditVals();
	void DrawAll();

	void DrawRGB(CDC *pDC);
	void DrawHSB(CDC *pDC);

	void LoadMappedBitmap(CBitmap& bitmap,UINT nIdResource,CSize& size);

	CBitmap m_RgbBitmap,m_HsbBitmap;

	CDC memDC;
	CPoint m_Centre;
	DIBUtils::CDIB m_BrightDIB;
	int rgbWidth,rgbHeight;
	int hsbWidth,hsbHeight;

	int m_nMouseIn;
	CRect m_CurrentRect,brightMark	;
	CRect brightRect;

	HSVType hsvColor;	

	RGBType color, m_OldColor;
	CPoint Vertex,Top,Left,Right;
	CRect  rects[3];
	CPoint m_Cuboid[8];
	BOOL m_bInMouse;
	int nIndex;
	int RedLen,GreenLen,BlueLen;
	LineDesc lines[3];


	CRect rgbRect;
	CRect hsbRect;
	CRect OldColorRect,NewColorRect;

	BOOL m_bInitOver,m_bInDrawAll;

	
};

}; // namespace ColorPicker
