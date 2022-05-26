//=========== Copyright (c) 2003, Tim Johnson. All rights reserved. ===========
// Canvas gives you high-level access to rendering, without needing to
// work at the scene-graph level.
//=============================================================================
#include "stdafx.h"
#include "d3dx9.h"


//-----------------------------------------------------------------------------
// Desc: Returns a singleton instance
//-----------------------------------------------------------------------------
Canvas* Canvas::Instance () 
{
	static Canvas inst;
	return &inst;
}


//-----------------------------------------------------------------------------
// Name: CreateD3DXFont()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CreateD3DXFont( LPD3DXFONT* ppd3dxFont, const TCHAR* pstrFont, DWORD dwSize, bool bold, bool cleartype, bool bScale=true )
{
	HRESULT hr;
	LPD3DXFONT pd3dxFontNew = NULL;
	HDC hDC;
	INT nHeight;

	hDC = GetDC( NULL );
	if(bScale)
		nHeight = -MulDiv( dwSize, GetDeviceCaps(hDC, LOGPIXELSY), 72 );
	ReleaseDC( NULL, hDC );

	hr = D3DXCreateFont( RenderWrap::dev,          // D3D device
		nHeight,               // Height
		0,                     // Width
		bold?FW_BOLD:FW_NORMAL,// Weight
		0,                     // MipLevels
		FALSE,                 // Italic
		DEFAULT_CHARSET,       // CharSet
		OUT_DEFAULT_PRECIS,    // OutputPrecision
		cleartype?5:ANTIALIASED_QUALITY,       // Quality
		DEFAULT_PITCH | FF_DONTCARE, // PitchAndFamily
		pstrFont,              // pFaceName
		&pd3dxFontNew);        // ppFont

	if( SUCCEEDED( hr ) )
		*ppd3dxFont = pd3dxFontNew;

	// Avoid GDI calls by getting all characters into video memory
	pd3dxFontNew->PreloadCharacters(0,127);
	return hr;
}

void Canvas::InitDeviceObjects(){
}


void Canvas::DeleteDeviceObjects(){
	SAFE_RELEASE(m_SystemFont);
	SAFE_RELEASE(m_SmallFont);
	SAFE_RELEASE(m_MediumFont);
	SAFE_RELEASE(m_LargeFont);
	SAFE_RELEASE(m_HUDFont);
	SAFE_RELEASE(m_Line);
}

void Canvas::RestoreDeviceObjects(){
	ResetCurrentDirectory();
	// Must recreate fonts on restore (fonts size with video)
	DeleteDeviceObjects();

	DXASSERT(D3DXCreateLine(RenderWrap::dev,&m_Line));
	m_Line->SetAntialias(TRUE);

	float scale = RenderDevice::Instance()->GetViewportX() / 1024.f;
	scale = scale - 1; // Change from an absolute multiplier to a change multiplier

	ConfigFile fonts;
	fonts.Load("fonts.ini");
	shadowColor = fonts.GetColor("ShadowColor");

	string sysFont = fonts.GetString("SystemFontName");
	string sName = fonts.GetString("SmallFontName");
	string mName = fonts.GetString("MediumFontName");
	string lName = fonts.GetString("LargeFontName");
	string hName = fonts.GetString("HUDFontName");

	int sysSize = fonts.GetInt("SystemFontSize");
	int sSize = fonts.GetInt("SmallFontSize");
	int mSize = fonts.GetInt("MediumFontSize");
	int lSize = fonts.GetInt("LargeFontSize");
	int hSize = fonts.GetInt("HUDFontSize");

	// Scale by percentage of relative screen size
	sSize += sSize * scale * fonts.GetFloat("SmallFontScale");
	mSize += mSize * scale * fonts.GetFloat("MediumFontScale");
	lSize += lSize * scale * fonts.GetFloat("LargeFontScale");
	hSize += hSize * scale * fonts.GetFloat("HUDFontScale");

	bool sBold = fonts.GetString("SmallFontFlags").find("BOLD")!=-1;
	bool mBold = fonts.GetString("MediumFontFlags").find("BOLD")!=-1;
	bool lBold = fonts.GetString("LargeFontFlags").find("BOLD")!=-1;
	bool hBold = fonts.GetString("HUDFontFlags").find("BOLD")!=-1;

	shadowSmallFont = fonts.GetBool("SmallFontShadow");
	shadowMediumFont = fonts.GetBool("MediumFontShadow");
	shadowLargeFont = fonts.GetBool("LargeFontShadow");
	shadowHUDFont = fonts.GetBool("HUDFontShadow");

	bool clearType = fonts.GetBool("ClearType");
	CreateD3DXFont(&m_SystemFont,sysFont.c_str(),sysSize,false,true);
	CreateD3DXFont(&m_SmallFont,sName.c_str(),sSize,sBold,clearType);
	CreateD3DXFont(&m_MediumFont,mName.c_str(),mSize,mBold,clearType);
	CreateD3DXFont(&m_LargeFont,lName.c_str(),lSize,lBold,clearType);
	CreateD3DXFont(&m_HUDFont,hName.c_str(),hSize,hBold,clearType);
}

void Canvas::InvalidateDeviceObjects(){
	if(!m_Line)
		return;

	m_Line->OnLostDevice();
	m_SystemFont->OnLostDevice();
	m_SmallFont->OnLostDevice();
	m_MediumFont->OnLostDevice();
	m_LargeFont->OnLostDevice();
	m_HUDFont->OnLostDevice();
}

void Canvas::Cleanup(){

}


void Canvas::SetScaling(float xscale, float yscale){
	xScale = xscale;
	yScale = yscale;
}

void Canvas::GetFont(FontType WhichFont, ID3DXFont*& font, bool& shadowed){
	switch(WhichFont){
		case SystemFont:
			font = m_SystemFont;
			shadowed = false;
			return;
		case SmallFont:
			font = m_SmallFont;
			shadowed = shadowSmallFont;
			return;
		case MediumFont:
			font = m_MediumFont;
			shadowed = shadowMediumFont;
			return;
		case LargeFont:
			font = m_LargeFont;
			shadowed = shadowLargeFont;
			return;
		case HUDFont:
			font = m_HUDFont;
			shadowed = shadowHUDFont;
			return;
		default:
			Error("Impossible");
	}
}

void Canvas::Textf(FontType WhichFont,COLOR color, float x, float y,const char *fmt, ...){
	if(Engine::Instance()->RenderSys == NULL) return;

	x *= xScale;
	y *= yScale;

	RenderWrap::SetRS( D3DRS_ALPHABLENDENABLE,TRUE);
	RenderWrap::SetRS( D3DRS_SRCBLEND, BLEND_SRCCOLOR );
	RenderWrap::SetRS( D3DRS_DESTBLEND, BLEND_INVSRCCOLOR );

	va_list		argptr;
	char		msg[1024];
	va_start (argptr,fmt);
	vsprintf (msg,fmt,argptr);
	va_end (argptr);

	ID3DXFont* font;
	bool shadowed;
	GetFont(WhichFont,font,shadowed);

	RECT rc;
	if(shadowed){
		SetRect( &rc, x+1, y+1, 0, 0 );  
		// Multiply shadow color by alpha
		int R = COLOR_GETRED(shadowColor), G = COLOR_GETGREEN(shadowColor), B = COLOR_GETBLUE(shadowColor), A = COLOR_GETALPHA(shadowColor);
		A = A * (float)COLOR_GETALPHA(color)/255.f;

		font->DrawText( NULL, msg, -1, &rc, DT_NOCLIP, COLOR_ARGB(A,R,G,B));
	}

	SetRect( &rc, x, y, 0, 0 );   
	font->DrawText( NULL, msg, -1, &rc, DT_NOCLIP, color);
}

void Canvas::TextCenteredf(FontType WhichFont,COLOR color, int x, int y,int ex, int ey,const char *fmt, ...){
	if(Engine::Instance()->RenderSys == NULL) return;

	x *= xScale;
	y *= yScale;
	ex *= xScale;
	ey *= yScale;

	RenderWrap::SetRS( D3DRS_ALPHABLENDENABLE,TRUE);
	RenderWrap::SetRS( D3DRS_SRCBLEND, BLEND_SRCCOLOR );
	RenderWrap::SetRS( D3DRS_DESTBLEND, BLEND_INVSRCCOLOR );

	va_list		argptr;
	char		msg[1024];
	va_start (argptr,fmt);
	vsprintf (msg,fmt,argptr);
	va_end (argptr);

	ID3DXFont* font;
	bool shadowed;
	GetFont(WhichFont,font,shadowed);

	RECT rc;
	if(shadowed){
		SetRect( &rc, x+1, y+1, ex+1, ey+1 );   
		// Multiply shadow color by alpha
		int R = COLOR_GETRED(shadowColor), G = COLOR_GETGREEN(shadowColor), B = COLOR_GETBLUE(shadowColor), A = COLOR_GETALPHA(shadowColor);
		A = A * (float)COLOR_GETALPHA(color)/255.f;

		font->DrawText( NULL, msg, -1, &rc, DT_NOCLIP|DT_CENTER|DT_VCENTER, COLOR_ARGB(A,R,G,B));
	}

	SetRect( &rc, x, y, ex, ey );   
	font->DrawText( NULL, msg, -1, &rc, DT_NOCLIP|DT_CENTER|DT_VCENTER, color);
}

SIZE Canvas::GetTextSize(FontType WhichFont, const char *fmt, ...){
	va_list		argptr;
	char		msg[1024];
	va_start (argptr,fmt);
	vsprintf (msg,fmt,argptr);
	va_end (argptr);

	SIZE size = {1,1};

	if(Engine::Instance()->RenderSys == NULL)
		return size;

	//D3DXFONT_DESC desc;
	//WhichFont->GetDesc(&desc);
	//size.cy = desc.Height;
	//size.cx = desc.Width;

	ID3DXFont* font;
	bool shadowed;
	GetFont(WhichFont,font,shadowed);

	HDC hdc = font->GetDC();
	// Make sure we return height on empty strings
	if(msg == ""){
		GetTextExtentPoint32(hdc,"X",strlen("X"),&size);
		size.cx = 0;
	}
	else
		GetTextExtentPoint32(hdc,msg,strlen(msg),&size);

	// Inverse don't scale, since these values will be fed back to the canvas and scaled again (d'oh!)
	size.cx /= xScale;
	size.cy /= yScale;
	return size;
}

void Canvas::SimpleObject(int NumVertices, void* vertices, int AVertexSize, bool strips){
	// Figure out the FVF
	DWORD FVF;
	if(AVertexSize == sizeof(Vertex))
		FVF = FVF_VERTEX;
	if(AVertexSize == sizeof(LVertex))
		FVF = FVF_LVERTEX;
	if(AVertexSize == sizeof(TLVertex))
		FVF = FVF_TLVERTEX;

	DWORD cull;
	if(FVF == FVF_TLVERTEX||FVF == FVF_LVERTEX){ // T/L culling is confusing and pointless (there aren't any back faces)
		cull = RenderWrap::SetRS( D3DRS_CULLMODE, D3DCULL_NONE);
		RenderWrap::SetRS(D3DRS_FOGENABLE,FALSE);
	}


	RenderWrap::dev->SetVertexShader(NULL);
	RenderWrap::dev->SetFVF(FVF);
	RenderWrap::dev->SetPixelShader(NULL);


	if(FAILED(RenderWrap::dev->DrawPrimitiveUP((strips?D3DPT_TRIANGLESTRIP:D3DPT_TRIANGLELIST),
		(strips?NumVertices - 2:NumVertices/3), vertices, AVertexSize)))
		Error("Canvas::SimpleObject() failed. Make sure you aren't trying to draw outside of Begin/EndRender()\nVerts: %d",NumVertices);

	if(FVF == FVF_TLVERTEX||FVF == FVF_LVERTEX){
		RenderWrap::SetRS( D3DRS_CULLMODE, cull);
		RenderWrap::SetRS(D3DRS_FOGENABLE,TRUE);
	}

}

void Canvas::SimpleObject(int NumVertices, void* vertices, int NumIndices, void* indices, int AVertexSize, bool strips, bool bCull, bool clearShaders){
	// Figure out the FVF
	DWORD FVF = -1;
	if(AVertexSize == sizeof(Vertex))
		FVF = FVF_VERTEX;
	if(AVertexSize == sizeof(LVertex))
		FVF = FVF_LVERTEX;
	if(AVertexSize == sizeof(TLVertex))
		FVF = FVF_TLVERTEX;

	if(FVF == -1)
		Error("AVertexSize not validly specified for SimpleObject() call");

	DWORD cull;
	if(FVF == FVF_TLVERTEX||FVF == FVF_LVERTEX){ // TL culling gets confusing
		if(!bCull)
			cull = RenderWrap::SetRS( D3DRS_CULLMODE, D3DCULL_NONE);
		RenderWrap::SetRS(D3DRS_FOGENABLE,FALSE);
	}

	if(clearShaders)
		RenderWrap::dev->SetVertexShader(NULL);

	RenderWrap::dev->SetFVF(FVF);
	RenderWrap::dev->SetPixelShader(NULL);
	RenderWrap::dev->SetStreamSource(0, NULL, 0, 0);

	if(FAILED(RenderWrap::dev->DrawIndexedPrimitiveUP((strips?D3DPT_TRIANGLESTRIP:D3DPT_TRIANGLELIST),
		0,NumVertices,(strips?NumIndices - 2:NumIndices/3),
		indices,D3DFMT_INDEX16, vertices, AVertexSize)))
		Error("Canvas::SimpleObject() failed. Make sure you aren't trying to draw outside of Begin/EndRender()\nVerts: %d",NumVertices);

	if(FVF == FVF_TLVERTEX||FVF == FVF_LVERTEX){
		if(!bCull)
			RenderWrap::SetRS( D3DRS_CULLMODE, cull);
		RenderWrap::SetRS(D3DRS_FOGENABLE,TRUE);
	}
}

//
// Simple line
//
void Canvas::Line(COLOR color, int width, int sx1, int sy1, int ex1, int ey1, BlendMode src,BlendMode dest){
	RenderWrap::ClearTextureLevel(0);

	// Scaling
	float sx = sx1 * xScale;
	float sy = sy1 * yScale;
	float ex = ex1 * xScale;
	float ey = ey1 * yScale;


	RenderWrap::SetRS( D3DRS_ALPHABLENDENABLE,(src!=BLEND_NONE));
	if(src != BLEND_NONE){
		RenderWrap::SetRS( D3DRS_SRCBLEND, src );
		RenderWrap::SetRS( D3DRS_DESTBLEND, dest );
	}

	if(width<=0)
		return;

	// Line is 1=start  2=end  3=start+width  4=end+width
	float widthx = 0;
	float widthy = 0;

	// Choose width axis
	if(abs(ex-sx) > 0)
		widthy = (width * yScale)>=1.6?(width * yScale):1; // Do scaling here, dependent on width
	else widthx= (width * xScale)>=1.6?(width * xScale):1;


	TLVertex v[4];
	/*v[0] =  TLVertex(Vector4( sx,sy, 0.0f, 1.0f),color,0,0);
	v[1] =  TLVertex(Vector4( ex,ey, 0.0f, 1.0f),color,1,0);
	v[2] =  TLVertex(Vector4( sx+widthx,sy+widthy, 0.0f, 1.0f),color,0,1);
	v[3] =  TLVertex(Vector4( ex+widthx,ey+widthy, 0.0f, 1.0f),color,1,1);*/

	v[0] =  TLVertex(Vector4( sx,sy+width, 0.0f, 1.0f),color,0,0);
	v[1] =  TLVertex(Vector4( sx,sy-width, 0.0f, 1.0f),color,1,0);
	v[2] =  TLVertex(Vector4( ex,ey+width, 0.0f, 1.0f),color,0,1);
	v[3] =  TLVertex(Vector4( ex,ey-width, 0.0f, 1.0f),color,1,1);

	SimpleObject(4,v,sizeof(TLVertex));
}

void Canvas::Cube(BBox& cube){

	LVertex pVertices[8];
	pVertices[0].position = cube.min;
	pVertices[1].position = Vector( cube.min.x, cube.max.y, cube.min.z );
	pVertices[2].position = Vector( cube.max.x, cube.max.y, cube.min.z );
	pVertices[3].position = Vector( cube.max.x, cube.min.y, cube.min.z );
	pVertices[4].position = Vector( cube.min.x, cube.min.y, cube.max.z );
	pVertices[5].position = Vector( cube.min.x, cube.max.y, cube.max.z );
	pVertices[6].position = cube.max;
	pVertices[7].position = Vector( cube.max.x, cube.min.y, cube.max.z );

	WORD indices[36];

	// front side
	indices[0]  = 0; indices[1]  = 1; indices[2]  = 2;
	indices[3]  = 0; indices[4]  = 2; indices[5]  = 3;

	// back side
	indices[6]  = 4; indices[7]  = 6; indices[8]  = 5;
	indices[9]  = 4; indices[10] = 7; indices[11] = 6;

	// left side
	indices[12] = 4; indices[13] = 5; indices[14] = 1;
	indices[15] = 4; indices[16] = 1; indices[17] = 0;

	// right side
	indices[18] = 3; indices[19] = 2; indices[20] = 6;
	indices[21] = 3; indices[22] = 6; indices[23] = 7;

	// top
	indices[24] = 1; indices[25] = 5; indices[26] = 6;
	indices[27] = 1; indices[28] = 6; indices[29] = 2;

	// bottom
	indices[30] = 4; indices[31] = 0; indices[32] = 3;
	indices[33] = 4; indices[34] = 3; indices[35] = 7;


	SimpleObject(8,pVertices,36,indices,sizeof(LVertex),false,false);
}


/*
D3DXVECTOR2 Vector& v, Matrix& screen){
	Vector vec = v;
	D3DXVec3TransformCoord( (D3DXVECTOR3*)&vec, (D3DXVECTOR3*)&vec, (D3DXMATRIX*)&screen );
	return *(D3DXVECTOR2*)&vec;
}
*/


void Canvas::DrawLines(int NumVertices, void* vertices, int AVertexSize, bool strips){
	// Figure out the FVF
	DWORD FVF;
	if(AVertexSize == sizeof(Vertex))
		FVF = FVF_VERTEX;
	if(AVertexSize == sizeof(LVertex))
		FVF = FVF_LVERTEX;
	if(AVertexSize == sizeof(TLVertex))
		FVF = FVF_TLVERTEX;

	DWORD cull;
	cull = RenderWrap::SetRS( D3DRS_CULLMODE, D3DCULL_NONE);
	RenderWrap::SetRS(D3DRS_FOGENABLE,FALSE);


	RenderWrap::dev->SetVertexShader(NULL);
	RenderWrap::dev->SetFVF(FVF);

	if(FAILED(RenderWrap::dev->DrawPrimitiveUP((strips?D3DPT_LINESTRIP:D3DPT_LINELIST),
		NumVertices-1, vertices, AVertexSize)))
		Error("Canvas::SimpleObject() failed. Make sure you aren't trying to draw outside of Begin/EndRender()\nVerts: %d",NumVertices);

	RenderWrap::SetRS( D3DRS_CULLMODE, cull);
	RenderWrap::SetRS(D3DRS_FOGENABLE,TRUE);
}


//
//
//
void Canvas::CubeWireframe(BBox& cube, Matrix& screenMatrix, COLOR color)
{
	RenderWrap::dev->SetTexture(0,0);
	// Need these states for AA lines (yes, need alpha!)
	RenderWrap::SetRS( D3DRS_DITHERENABLE, TRUE );	//edge antialising
	RenderWrap::SetRS( D3DRS_ALPHABLENDENABLE, FALSE ); // Need alpha for line AA to work  
	RenderWrap::SetRS(D3DRS_ANTIALIASEDLINEENABLE,TRUE);


	RenderWrap::SetTSS( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
    RenderWrap::SetTSS( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
    RenderWrap::SetTSS( 0, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
    //RenderWrap::SetTSS( 0, D3DTSS_ALPHAARG1, D3DTA_CONSTANT );
   
	D3DXMATRIX mx;
	D3DXMatrixIdentity(&mx);
	D3DXMATRIX mxView = *(D3DXMATRIX*)&RenderWrap::GetView();
	D3DXMATRIX mxProj = *(D3DXMATRIX*)&RenderWrap::GetProjection();
	D3DXMatrixMultiply( & mx, & mx, & mxView );
	D3DXMatrixMultiply( & mx, & mx, & mxProj );

	LVertex vLine[10];
	// Min
	vLine[0] = LVertex(cube.min,color);
	vLine[1] = LVertex(Vector(cube.max.x,cube.min.y,cube.min.z),color);
	vLine[2] = LVertex(Vector(cube.max.x,cube.min.y,cube.max.z),color);
	vLine[3] = LVertex(Vector(cube.min.x,cube.min.y,cube.max.z),color);
	vLine[4] = LVertex(cube.min,color);

	DrawLines(5,vLine,sizeof(LVertex),true);
	//m_Line->DrawTransform((D3DXVECTOR3*)vLine,5,&mx,color);

	// Max
	vLine[0] = LVertex(cube.max,color);
	vLine[1] = LVertex(Vector(cube.min.x,cube.max.y,cube.max.z),color);
	vLine[2] = LVertex(Vector(cube.min.x,cube.max.y,cube.min.z),color);
	vLine[3] = LVertex(Vector(cube.max.x,cube.max.y,cube.min.z),color);
	vLine[4] = LVertex(cube.max,color);
	DrawLines(5,vLine,sizeof(LVertex),true);

	// UP
	vLine[0] = LVertex(Vector(cube.min.x,cube.min.y,cube.min.z),color);
	vLine[1] = LVertex(Vector(cube.min.x,cube.max.y,cube.min.z),color);
	DrawLines(2,vLine,sizeof(LVertex),true);
	// UP
	vLine[0] = LVertex(Vector(cube.max.x,cube.min.y,cube.min.z),color);
	vLine[1] = LVertex(Vector(cube.max.x,cube.max.y,cube.min.z),color);
	DrawLines(2,vLine,sizeof(LVertex),true);
	// UP
	vLine[0] = LVertex(Vector(cube.min.x,cube.min.y,cube.max.z),color);
	vLine[1] = LVertex(Vector(cube.min.x,cube.max.y,cube.max.z),color);
	DrawLines(2,vLine,sizeof(LVertex),true);
	// UP
	vLine[0] = LVertex(Vector(cube.max.x,cube.min.y,cube.max.z),color);
	vLine[1] = LVertex(Vector(cube.max.x,cube.max.y,cube.max.z),color);
	DrawLines(2,vLine,sizeof(LVertex),true);
}

//
// Box outline
//
void Canvas::BoxOutline(COLOR color, int outlineWidth, int sx, int sy, int width, int height, BlendMode src,BlendMode dest){
	RenderWrap::SetRS( D3DRS_ALPHABLENDENABLE,(src!=BLEND_NONE));
	if(src != BLEND_NONE){
		RenderWrap::SetRS( D3DRS_SRCBLEND, src );
		RenderWrap::SetRS( D3DRS_DESTBLEND, dest );
	}

	// A-B B-C+X C-D D-A
	// Top line
	Line(color,outlineWidth,sx,sy,sx+width,sy,src,dest);
	// Left line down
	Line(color,outlineWidth,sx,sy,sx,sy+height,src,dest);
	// Right line down
	Line(color,outlineWidth,sx+width,sy,sx+width,sy+height+outlineWidth,src,dest);
	// Bottom line
	Line(color,outlineWidth,sx,sy+height,sx+width,sy+height,src,dest);

}

//
// Shaded box
//
void Canvas::Box(COLOR color, int sx, int sy, int width, int height, Texture* texture, BlendMode src,BlendMode dest){
	// Scaling
	sx *= xScale;
	sy *= yScale;
	width *= xScale;
	height *= yScale;
 
	RenderWrap::SetRS( D3DRS_ALPHABLENDENABLE,(src!=BLEND_NONE));
	if(src != BLEND_NONE){
		RenderWrap::SetRS( D3DRS_SRCBLEND, src );
		RenderWrap::SetRS( D3DRS_DESTBLEND, dest );
	}

	float uOff=0,vOff=0,uTile=1,vTile=1;
	if(texture){
		// 'Texture' doesn't seem to transform tex coordinate, so do manually
		uOff = texture->uOff;
		vOff = texture->vOff;
		uTile = texture->uTile;
		vTile = texture->vTile;
	}

	TLVertex v[4];
	v[0] =  TLVertex(Vector4( sx, sy, 0.0f, 1.0f),color,uOff,vOff);
	v[1] =  TLVertex(Vector4( sx + width, sy, 0.0f, 1.0f),color,uOff + uTile,vOff);
	v[2] =  TLVertex(Vector4( sx , sy + height, 0.0f, 1.0f),color,uOff,vOff + vTile);
	v[3] =  TLVertex(Vector4( sx + width, sy + height, 0.0f, 1.0f),color,uOff + uTile,vOff + vTile);

	if(texture)
		texture->Set(0);
	else
		RenderWrap::ClearTextureLevel(0);

	SimpleObject(4,v,sizeof(TLVertex));
}


//
// Shaded box
//
void Canvas::BoxD3D(COLOR color, int sx, int sy, int width, int height, LPDIRECT3DTEXTURE9 texture, BlendMode src,BlendMode dest){
	// Scaling
	sx *= xScale;
	sy *= yScale;
	width *= xScale;
	height *= yScale;

	RenderWrap::SetRS( D3DRS_ALPHABLENDENABLE,(src!=BLEND_NONE));
	if(src != BLEND_NONE){
		RenderWrap::SetRS( D3DRS_SRCBLEND, src );
		RenderWrap::SetRS( D3DRS_DESTBLEND, dest );
	}

	float uOff=0,vOff=0,uTile=1,vTile=1;

	TLVertex v[4];
	v[0] =  TLVertex(Vector4( sx, sy, 0.0f, 1.0f),color,uOff,vOff);
	v[1] =  TLVertex(Vector4( sx + width, sy, 0.0f, 1.0f),color,uOff + uTile,vOff);
	v[2] =  TLVertex(Vector4( sx , sy + height, 0.0f, 1.0f),color,uOff,vOff + vTile);
	v[3] =  TLVertex(Vector4( sx + width, sy + height, 0.0f, 1.0f),color,uOff + uTile,vOff + vTile);

	if(texture)
		RenderWrap::dev->SetTexture(0,texture);
	else
		RenderWrap::ClearTextureLevel(0);

	SimpleObject(4,v,sizeof(TLVertex));
}

void rotateVertex(TLVertex *pV,float angle, Vector &vOrigin) {
	float c=cos(angle);
	float s=sin(angle);
	float x=pV->position.x-vOrigin.x;
	float y=pV->position.y-vOrigin.y;
	float x1=c*x-s*y;
	float y1=s*x+c*y;
	pV->position.x=x1+vOrigin.x;
	pV->position.y=y1+vOrigin.y;
}

Matrix rotationMatrix(float theta, Vector &vPivot, Vector& vAxis) {
	Matrix r;
	float c = cos(theta);
	float s = sin(theta);

	// Compute the rotation (upper left 3x3 sub-matrix)

	r[0][0] = c + (1-c)*vAxis[0]*vAxis[0];
	r[0][1] =     (1-c)*vAxis[0]*vAxis[1] + s*vAxis[2];
	r[0][2] =     (1-c)*vAxis[0]*vAxis[2] - s*vAxis[1];
	r[1][0] =     (1-c)*vAxis[1]*vAxis[0] - s*vAxis[2];
	r[1][1] = c + (1-c)*vAxis[1]*vAxis[1];
	r[1][2] =     (1-c)*vAxis[1]*vAxis[2] + s*vAxis[0];
	r[2][0] =     (1-c)*vAxis[2]*vAxis[0] + s*vAxis[1];
	r[2][1] =     (1-c)*vAxis[2]*vAxis[1] - s*vAxis[0];
	r[2][2] = c + (1-c)*vAxis[2]*vAxis[2];

	// Compute the translation
	r[3][0] = vPivot[0] - r[0][0]*vPivot[0] - r[1][0]*vPivot[1] - r[2][0]*vPivot[2];
	r[3][1] = vPivot[1] - r[0][1]*vPivot[0] - r[1][1]*vPivot[1] - r[2][1]*vPivot[2];
	r[3][2] = vPivot[2] - r[0][2]*vPivot[0] - r[1][2]*vPivot[1] - r[2][2]*vPivot[2];
	return r;
}


void Canvas::RotatedBox(COLOR color, int sx, int sy, int width, int height, float angleDeg, Vector& origin, Texture* texture, BlendMode src,BlendMode dest){
	// Scaling
	sx *= xScale;
	sy *= yScale;
	width *= xScale;
	height *= yScale;

	origin.x *= xScale;
	origin.y *= yScale;

	RenderWrap::SetRS( D3DRS_ALPHABLENDENABLE,(src!=BLEND_NONE));
	if(src != BLEND_NONE){
		RenderWrap::SetRS( D3DRS_SRCBLEND, src );
		RenderWrap::SetRS( D3DRS_DESTBLEND, dest );
	}

	float uOff=0,vOff=0,uTile=1,vTile=1;
	if(texture){
		// Doesn't seem to transform tex coordinate either, so do manually
		uOff = texture->uOff;
		vOff = texture->vOff;
		uTile = texture->uTile;
		vTile = texture->vTile;
	}

	angleDeg = DEG2RAD(angleDeg);

	TLVertex v[4];
	v[0] =  TLVertex(Vector4( sx, sy, 0.0f, 1.0f),color,uOff,vOff);
	v[1] =  TLVertex(Vector4( sx + width, sy, 0.0f, 1.0f),color,uOff + uTile,vOff);
	v[2] =  TLVertex(Vector4( sx , sy + height, 0.0f, 1.0f),color,uOff,vOff + vTile);
	v[3] =  TLVertex(Vector4( sx + width, sy + height, 0.0f, 1.0f),color,uOff + uTile,vOff + vTile);

	for(int i=0;i<4;i++)
		rotateVertex(&v[i],angleDeg,origin);

	if(texture)
		texture->Set(0);
	else
		RenderWrap::ClearTextureLevel(0);

	SimpleObject(4,v,sizeof(TLVertex));
}

void Canvas::RotatedBillBoard(Vector& pos, Vector& origin, float size, float angleDeg, COLOR color, Texture* tex, BlendMode src,BlendMode dest){
	angleDeg = DEG2RAD(angleDeg);

	RenderWrap::SetRS( D3DRS_ALPHABLENDENABLE,(src!=BLEND_NONE));
	if(src != BLEND_NONE){
		RenderWrap::SetRS( D3DRS_SRCBLEND, src );
		RenderWrap::SetRS( D3DRS_DESTBLEND, dest );
	}

	if(tex)
		tex->Set(0);
	else
		RenderWrap::ClearTextureLevel(0);
	// Get vectors for building billboard flares
	Matrix mat = RenderWrap::GetView();
	// Note the column-ordering, it's particular to the way view matrices are built
	Vector rightVect = Vector(mat[0][0],mat[1][0],mat[2][0]).Normalized();
	Vector upVect    = Vector(mat[0][1],mat[1][1],mat[2][1]).Normalized();
	Vector dir = Vector(mat[0][2],mat[1][2],mat[2][2]).Normalized();

	Vector up = upVect * size;
	Vector right = rightVect * size;
	LVertex pVertices[4];
	pVertices[0] = LVertex((pos-right)-up, color, 0.0f, 1.0f);
	pVertices[1] = LVertex((pos+right)-up, color, 1.0f, 1.0f);
	pVertices[2] = LVertex((pos-right)+up, color, 0.0f, 0.0f);
	pVertices[3] = LVertex((pos+right)+up, color, 1.0f, 0.0f);

	Matrix rotMat = rotationMatrix(angleDeg,origin,dir);
	for(int i=0;i<4;i++)
		pVertices[i].position = rotMat * pVertices[i].position ;

	// Perhaps rotate the first flare(the glare) here?
	SimpleObject(4,pVertices,sizeof(LVertex));
}

//
// Shaded + Outlined box
//
void Canvas::BoxWithOutline(COLOR fill,COLOR outline, int outlineWidth, int sx, int sy, int width, int height, Texture* texture, BlendMode src,BlendMode dest){
	RenderWrap::SetRS( D3DRS_ALPHABLENDENABLE,(src!=BLEND_NONE));
	if(src != BLEND_NONE){
		RenderWrap::SetRS( D3DRS_SRCBLEND, src );
		RenderWrap::SetRS( D3DRS_DESTBLEND, dest );
	}

	BoxOutline(outline,outlineWidth,sx,sy,width,height,src,dest);
	Box(fill,sx+outlineWidth,sy+outlineWidth,width-outlineWidth,height-outlineWidth,texture,src,dest);
}

//
// Shaded + Outlined box
//
void Canvas::BoxWithOutline(COLOR fill,COLOR outline, int outlineWidth, RECT& r, Texture* texture, BlendMode src,BlendMode dest){
	int sx = r.left;
	int sy = r.top;
	int width = r.right - r.left;
	int height = r.bottom - r.top;

	RenderWrap::SetRS( D3DRS_ALPHABLENDENABLE,(src!=BLEND_NONE));
	if(src != BLEND_NONE){
		RenderWrap::SetRS( D3DRS_SRCBLEND, src );
		RenderWrap::SetRS( D3DRS_DESTBLEND, dest );
	}

	BoxOutline(outline,outlineWidth,sx,sy,width,height,src,dest);
	Box(fill,sx+outlineWidth,sy+outlineWidth,width-outlineWidth,height-outlineWidth,texture,src,dest);
}


void Canvas::BillBoard(Vector& pos, float size, COLOR color, Texture* tex, BlendMode src,BlendMode dest){
	RenderWrap::SetRS( D3DRS_ALPHABLENDENABLE,(src!=BLEND_NONE));
	if(src != BLEND_NONE){
		RenderWrap::SetRS( D3DRS_SRCBLEND, src );
		RenderWrap::SetRS( D3DRS_DESTBLEND, dest );
	}

	if(tex)
		tex->Set(0);
	else
		RenderWrap::ClearTextureLevel(0);
	// Get vectors for building billboard flares
	Matrix mat = RenderWrap::GetView();
	// Note the column-ordering, it's particular to the way view matrices are built
	Vector rightVect = Vector(mat[0][0],mat[1][0],mat[2][0]).Normalized();
	Vector upVect    = Vector(mat[0][1],mat[1][1],mat[2][1]).Normalized();

	Vector up = upVect * size;
	Vector right = rightVect * size;
	LVertex pVertices[4];
	pVertices[0] = LVertex((pos-right)-up, color, 0.0f, 1.0f);
	pVertices[1] = LVertex((pos+right)-up, color, 1.0f, 1.0f);
	pVertices[2] = LVertex((pos-right)+up, color, 0.0f, 0.0f);
	pVertices[3] = LVertex((pos+right)+up, color, 1.0f, 0.0f);

	// Perhaps rotate the first flare(the glare) here?
	SimpleObject(4,pVertices,sizeof(LVertex));
}




//-----------------------------
// Prints a stat to the screen
//-----------------------------
long AveragedVal[50];
long AccumulatingVal[50];
long FramesVal[50];

void Canvas::PrintStat(string name, int val, int threshold, bool AverageStat){
	static int frames = 0, maxStatNum = 0;
	static float UpdateRate = 0.25f, LastSeconds = -1;

	// Initialize all the values for the first ever run
	if(LastSeconds == -1){
		for(int i=0;i<50;i++){
			AveragedVal[i] = 0;
			AccumulatingVal[i] = 0;
		}
	}

	// Keep an averaged value of this stat, to compensate for timer
	// inaccuracies
	if(GSeconds > LastSeconds + UpdateRate){

		if(AccumulatingVal[statNum] > 0 && maxStatNum > 0 && FramesVal[statNum] > maxStatNum) // Avoid divide by 0
			AveragedVal[statNum] = AccumulatingVal[statNum] / FramesVal[statNum];
		else
			AveragedVal[statNum] = 0;

		FramesVal[statNum] = 0;

		AccumulatingVal[statNum] = 0;

		if(statNum == maxStatNum){
			LastSeconds = GSeconds;
		}
	}
	if(statNum > maxStatNum)
		maxStatNum = statNum;

	FramesVal[statNum]++;
	AccumulatingVal[statNum] += val;

	if(AverageStat)
		val = AveragedVal[statNum];

	const int stat_width = 90;
	COLOR color = 0xFF00FF00;

	int x = 1024 - 130;
	int y = 15;

	// Loop over to the next line if we reach the end of the screen
	/*int fit = 4;//RenderDevice::Instance()->GetViewportX() / stat_width*2;
	if(statNum >= fit){
		int times = fit/statNum;
		x -= times*100*fit;
		y += 20 * times;
	}*/
	y += statNum * 15;

	//x += statNum*stat_width;


	// Go red if above threshold
	if(threshold > 0 && val > threshold)
		color = 0xFFFF0000;
	// Go red if below threshold (if threshold is negative)
	if(threshold < 0 && val < -threshold)
		color = 0xFFFF0000;

	name += ": %d";

	Textf(SmallFont,color,x,y,name.c_str(),val);

	statNum ++;
}




