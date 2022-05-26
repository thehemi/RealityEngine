//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
/// Canvas gives you high-level access to rendering, without needing to
/// work at the scene-graph level.
/// Author: Tim Johnson
//====================================================================================
#pragma once


//
/// A high-level drawing canvas.
//
enum BlendMode
{
	BLEND_NONE		  = 0,
	BLEND_ZERO              = 1,
	BLEND_ONE               = 2,
	BLEND_SRCCOLOR          = 3,
	BLEND_INVSRCCOLOR       = 4,
	BLEND_SRCALPHA          = 5,
	BLEND_INVSRCALPHA       = 6,
	BLEND_DESTALPHA         = 7,
	BLEND_INVDESTALPHA      = 8,
	BLEND_DESTCOLOR         = 9,
	BLEND_INVDESTCOLOR      = 10,
	BLEND_SRCALPHASAT       = 11,
	BLEND_BOTHSRCALPHA      = 12,
	BLEND_BOTHINVSRCALPHA   = 13,
	BLEND_FORCE_DWORD       = 0x7fffffff, /* force 32-bit size enum */
};


enum FontType{
	SystemFont,
	SmallFont,
	MediumFont,
	LargeFont,
	HUDFont
};

/// \brief Canvas gives you high-level access to screenspace-oriented rendering, without needing to
/// work at the scene-graph level.
class ENGINE_API Canvas {
protected:
	Canvas(){ memset(this,0,sizeof(Canvas)); xScale = 1; yScale = 1; }
public:
	int statNum; /// PrintStat()

	float xScale, yScale;


public:

	void RestoreDeviceObjects();
	void DeleteDeviceObjects();
	void InitDeviceObjects();
	void Cleanup();
	void InvalidateDeviceObjects();
	void GetFont(FontType WhichFont, struct ID3DXFont*& font, bool& shadowed);

	/// GUI Scaling
	/// For example, if you draw all widgets at 1024x768, then you can scale
	/// them with this to match any other resolution
	/// with currentWidth/1024.f currentHeight/768.f
	void SetScaling(float xScale=1, float yScale=1);

	/// Singleton
	static Canvas* Instance();

	int Width, Height;
	COLOR shadowColor;
	bool shadowSmallFont, shadowMediumFont, shadowLargeFont, shadowHUDFont;
	struct ID3DXFont  *m_SystemFont,*m_SmallFont, *m_MediumFont, *m_LargeFont, *m_HUDFont;
	struct ID3DXLine *m_Line;

	/// Advanced Text

	/// Prints a statistic to the screen. value is the statistic, threshold is the
	/// boundary at which it'll be drawn in red. If threshold is negative, any value
	/// below that will be drawn in red, rather than above, ignoring the minus sign
	/// AverageStat is whether or not to average that statistic over a few frames to
	/// smooth it out.
	/// Example: PrintStat("FPS",1.f/GDeltaTime,-30); /// -30 means < 30fps and it's red
	void PrintStat(string name, int value, int threshold, bool AverageStat = true);

	/// Text
	void Textf(FontType WhichFont, COLOR color, float x, float y, const char *fmt, ...);
	void TextCenteredf(FontType WhichFont, COLOR color, int x, int y, int x2, int y2, const char *fmt, ...);
	SIZE GetTextSize(FontType WhichFont, const char *fmt,...);

	/// 2D Drawing
	void Line(COLOR color, int width, int sx, int sy, int ex, int ey, BlendMode src=BLEND_SRCCOLOR,BlendMode dest=BLEND_INVSRCCOLOR);
	void BoxOutline(COLOR color, int outlineWidth, int sx, int sy, int width, int height, BlendMode src=BLEND_SRCCOLOR,BlendMode dest=BLEND_INVSRCCOLOR);
	void BoxD3D(COLOR color, int sx, int sy, int width, int height, LPDIRECT3DTEXTURE9 texture, BlendMode src=BLEND_SRCCOLOR,BlendMode dest=BLEND_INVSRCCOLOR);
	void Box(COLOR color, int sx, int sy, int width, int height, class Texture* texture, BlendMode src=BLEND_SRCCOLOR,BlendMode dest=BLEND_INVSRCCOLOR);
	void RotatedBox(COLOR color, int sx, int sy, int width, int height, float angleDeg, Vector& origin, Texture* texture, BlendMode src=BLEND_SRCCOLOR,BlendMode dest=BLEND_INVSRCCOLOR);
	void BoxWithOutline(COLOR boxcolor, COLOR linecolor, int outlineWidth, int sx, int sy, int width, int height, Texture* texture, BlendMode src=BLEND_SRCCOLOR,BlendMode dest=BLEND_INVSRCCOLOR);
	void BoxWithOutline(COLOR boxcolor, COLOR linecolor, int outlineWidth, RECT& box, Texture* texture, BlendMode src=BLEND_SRCCOLOR,BlendMode dest=BLEND_INVSRCCOLOR);
	void DrawLines(int NumVertices, void* vertices, int AVertexSize, bool strips);

	/// Basic polygon drawing
	void SimpleObject(int NumVertices, void* vertices, int AVertexSize, bool strips = true);
	void SimpleObject(int NumVertices, void* vertices, int NumIndices, void* indices, int AVertexSize, bool strips = true, bool cull = false,bool clearShaders = true);
	void BillBoard(Vector& pos, float size, COLOR color, Texture* texture, BlendMode src=BLEND_SRCCOLOR,BlendMode dest=BLEND_INVSRCCOLOR);
	void RotatedBillBoard(Vector& pos, Vector& origin, float size, float angleDeg, COLOR color, Texture* tex, BlendMode src,BlendMode dest);
	void Cube(BBox& cube);
	void CubeWireframe(BBox& cube, Matrix& screenMatrix, COLOR color);
};



