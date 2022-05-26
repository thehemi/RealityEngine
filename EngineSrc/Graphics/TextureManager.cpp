//=========== (C) Copyright 2003, Tim Johnson. All rights reserved. ===========
// Name: TextureManager
// TODO: Use the nvDXT lib/dll for compression. Far better quality than Microsoft's
//
// TODO: Purge the config of files that no longer exist in the cache
// TODO(Very minor): Use a faster config save/lookup for our ddscache
//
//=============================================================================
#include "stdafx.h"
#include "TextureManager.h"
#include "GUISystem.h"
#include <fstream>
typedef char TCHAR, *PTCHAR; 
#include "DShowTextures.h"
//#include "dxtlib\nvdxtdll.h" // DXT compression the NVIDIA way
//#include "dxtlib\dxtlib.h" // DXT compression the NVIDIA way
int			TextureManager::maxTextures		= 0;
int			TextureManager::textureMemory	= 0;
float		TextureManager::FrameTime		= 0;
bool		TextureManager::CompressNormalMaps;
int			TextureManager::TextureSizePercent;
bool		TextureManager::SaveMips		= true;
ConfigFile	TextureManager::cache;
string		TextureManager::m_CachePath;
vector<TextureManager::TexInfo> TextureManager::textures;
vector<Texture*>				TextureManager::AnimatedTextures;

#define FMT_3DC MAKEFOURCC('A', 'T', 'I', '2') // ATI 3Dc

#define POOL_TYPE D3DPOOL_MANAGED
#define SRGB_FILTER 0//D3DX_FILTER_SRGB

void GenerateBumpMap(const CHAR* heightMap, LPDIRECT3DTEXTURE9& normalMap, D3DFORMAT format, float bumpScale);


//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void TextureManager::Initialize(const D3DSURFACE_DESC* pBackBufferSurfaceDesc){
	// Never run this more than once
	static bool run = false;
	if(run) return; 
	run = true;

	m_CachePath = Engine::Instance()->MainConfig->GetString("SearchPath") + "\\Cache\\";
	// Make sure cache directory exists
	CreateDirectory(m_CachePath.c_str(),NULL);
	string cacheFile = m_CachePath + "cache.ini";
	//---------------------------------------------
	// On the first run we'll unpack & cache all textures
	//---------------------------------------------
	if(!FileExists(cacheFile)){
		ofstream notice((m_CachePath + "NEVER PUT TEXTURES HERE.txt").c_str());
		notice << "This directory and all files (including this one) are 100% engine-generated. Modifying will cause serious problems, because:" << endl;
		notice << "1) This directory will never be searched for textures, only to find mirror images of textures in normal folders" << endl;
		notice << "2) This directory is rebuilt from SOURCE files if any meta-data changes (bump amount, associated spec/illum map, etc). This cannot be done if textures are inside this dir" << endl;
		notice.close();

		cache.Create(m_CachePath+cacheFile);
		cache.InsertRawLine("// This is a procedural file. Do not modify.");
		cache.InsertRawLine("// Clearing the contents of this file will result in the ddscache files being rebuilt again WHEN LOADED");
		cache.InsertRawLine("// Deleting the file outright will result in an INDISCRIMINATE PRECACHE");
		cache.InsertRawLine("// That means *all* .png,.bmp, and .tgas in \\Cry Havoc will be cached (good for straight after a fresh install)");

		GUISystem::Instance()->DoMessageBox("Texture Management","No texture cache exists yet.\nYou may experience some delays initially.",pBackBufferSurfaceDesc->Width/2,pBackBufferSurfaceDesc->Height/2);

		// Expand all our compressed resources
		//MessageBox(0,"Please wait for textures to be cached (this only needs to be done once after installing)\nBe patient, this may take a minute. A confirmation will be displayed after completion.","Precacher",0);
		
		// This won't work, since we don't know normal maps from diffuse maps
		/*vector<string> texFiles;
		enumerateFiles((Engine::Instance()->MainConfig->GetString("SearchPath") + "\\Textures\\").c_str(),texFiles,20);
		// Load all files to force the cache to be created (or expanded, if it's a cached mipless dds)
		int count=0;
		for(int i=0;i<texFiles.size();i++){
			string temp = texFiles[i];
			ToLowerCase(temp);
			Texture tex;
			if(temp.find(".dds")!=-1 || temp.find(".tga") !=-1 || temp.find(".png") !=-1 || temp.find(".bmp") !=-1){
				tex.Load(texFiles[i]);
				count++;
			}
			tex.Destroy();
		}
		char buf[256];
		sprintf(buf,"Done! %d textures proccessed.\nPlease restart the game after clicking OK.",count);
		MessageBox(0,buf,"Precacher",0);
		SendMessage(Engine::Instance()->hWnd,WM_SYSCOMMAND,SC_CLOSE,0);
		exit(0);*/
	}

	//---------------------------------------------
	// Load our cache ini and do some housekeeping
	// TODO: Housekeep files that no longer exist
	//---------------------------------------------
	cache.Load(cacheFile);

	//---------------------------------------------
	// This Handles the PackDDSFiles function, for stripping mips to make dds files smaller
	//---------------------------------------------
	if(Engine::Instance()->MainConfig->GetBool("DEV_PackDDSFiles")){
		Engine::Instance()->MainConfig->SetBool("DEV_PackDDSFiles",false);
		TextureManager::SaveMips = false;
		MessageBox(0,_U("Please wait for all DDS files to be stripped of their mip levels. This should cut file sizes by about 25%"),_U("DDS Packer"),0);
		vector<string> texFiles;
		enumerateFiles((Engine::Instance()->MainConfig->GetString("SearchPath") + "\\Textures\\").c_str(),texFiles,20);

		// Load all files to force the dds files to be packed and resaved
		int count=0;
		for(int i=0;i<texFiles.size();i++){
			string temp = texFiles[i];
			ToLowerCase(temp);
			Texture tex;
			if(temp.find(_U(".dds"))!=-1){
				tex.Load(texFiles[i]);
				count++;
			}
			tex.Destroy();
		}

		CHAR buf[256];
		sprintf(buf,_U("Done! %d textures have been packed.\nDEV_PackDDSFiles has been set to 'false'\nPlease do not launch the game again until you have readied the release, or all files will be unpacked\nClick OK to exit"),count);
		MessageBox(0,buf,_U("Precacher"),0);
		// Don't do this, nothing must get reloaded with mips, we must exit ASAP to avoid texture expansion
		//TextureManager::SaveMips = true;
		SendMessage(Engine::Instance()->hWnd,WM_SYSCOMMAND,SC_CLOSE,0);
		exit(0);
	}
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
bool TextureManager::FillChannel(Channel chan, string strPath, LPDIRECT3DTEXTURE9 ptex){
    D3DSURFACE_DESC sd;
	LPDIRECT3DDEVICE9 pd3ddev = RenderWrap::dev;
    LPDIRECT3DTEXTURE9 ptexAlpha;
    LPDIRECT3DSURFACE9 psurfAlpha;
    LPDIRECT3DSURFACE9 psurfTarget;
	LPDIRECT3DSURFACE9 psurf;
	
	(*(LPDIRECT3DTEXTURE9*)&ptex)->GetSurfaceLevel(0,&psurf);

    psurf->GetDesc(&sd);

    // Find the file, checking common extensions
	if(!FindMedia(strPath,"Textures")){
		string origFile = strPath;
		// Try DDS first
		strPath = StripExtension(strPath) + _U(".dds");

		if(!FindMedia(strPath,"Textures")){
			// Then TGA
			strPath = StripExtension(strPath) + _U(".tga");
			// Give up, restore original extension and display error
			if(!FindMedia(strPath,"Textures")){
				strPath = origFile;
				LogPrintf(_U("Can't find or load specular/self-illum/mix channel texture: %s. Error: %s\n"),strPath.c_str());
				return false;
			}
		}
	}


	// Load the channel texture into psurfAlpha, a new A8R8G8B8 surface
    DXASSERT(D3DXCreateTextureFromFileEx(pd3ddev, strPath.c_str(), sd.Width, sd.Height, 1, 0, 
        D3DFMT_A8R8G8B8, D3DPOOL_MANAGED , D3DX_FILTER_TRIANGLE, D3DX_FILTER_TRIANGLE, 0, NULL, NULL, &ptexAlpha));

    DXASSERT(ptexAlpha->GetSurfaceLevel(0, &psurfAlpha));

    // Copy the target surface into an A8R8G8B8 surface
    DXASSERT(pd3ddev->CreateOffscreenPlainSurface(sd.Width, sd.Height, D3DFMT_A8R8G8B8, D3DPOOL_SCRATCH , &psurfTarget, NULL));
    DXASSERT(D3DXLoadSurfaceFromSurface(psurfTarget, NULL, NULL, psurf, NULL, NULL, D3DX_FILTER_TRIANGLE, 0));

    // Fill in the alpha channels of psurfTarget based on the blue channel of psurfAlpha
    D3DLOCKED_RECT lrSrc;
    D3DLOCKED_RECT lrDest;

    DXASSERT(psurfAlpha->LockRect(&lrSrc, NULL, D3DLOCK_READONLY));
    DXASSERT(psurfTarget->LockRect(&lrDest, NULL, 0));

    DWORD xp;
    DWORD yp;
    DWORD* pdwRowSrc = (DWORD*)lrSrc.pBits;
    DWORD* pdwRowDest = (DWORD*)lrDest.pBits;
    DWORD* pdwSrc;
    DWORD* pdwDest;
    LONG dataBytesPerRow = 4 * sd.Width;

    for (yp = 0; yp < sd.Height; yp++)
    {
        pdwSrc = pdwRowSrc;
        pdwDest = pdwRowDest;
        for (xp = 0; xp < sd.Width; xp++)
        {
			// Always sample from RED
			unsigned char src = COLOR_GETRED(*pdwSrc);

			unsigned char a = COLOR_GETALPHA(*pdwDest);
			unsigned char r = COLOR_GETRED(*pdwDest);
			unsigned char g = COLOR_GETGREEN(*pdwDest);
			unsigned char b = COLOR_GETBLUE(*pdwDest);

			// Write to appropriate channel
			switch(chan){
				case CHAN_ALPHA:
					*pdwDest = COLOR_ARGB(src,r,g,b);
					break;
				case CHAN_RED:
					*pdwDest = COLOR_ARGB(a,src,g,b);
					break;
				case CHAN_GREEN:
					*pdwDest = COLOR_ARGB(a,r,src,b);
					break;
				case CHAN_BLUE:
					*pdwDest = COLOR_ARGB(a,r,g,src);
					break;
			}

            pdwSrc++;
            pdwDest++;
        }
        pdwRowSrc += lrSrc.Pitch / 4;
        pdwRowDest += lrDest.Pitch / 4;
    }

    psurfAlpha->UnlockRect();
    psurfTarget->UnlockRect();

	// Copy psurfTarget back into real surface
	DXASSERT(D3DXLoadSurfaceFromSurface(psurf, NULL, NULL, psurfTarget, NULL, NULL, 
		D3DX_FILTER_TRIANGLE, 0));
    
    // Release allocated interfaces
    SAFE_RELEASE(psurfTarget);
    SAFE_RELEASE(psurfAlpha);
    SAFE_RELEASE(ptexAlpha);
	psurf->Release();

	// Fill mip levels
	DXASSERT(D3DXFilterTexture(ptex, NULL, 0,  D3DX_DEFAULT));

	return true;
}


//-----------------------------------------------------------------------------
//
// FIXME ASAP: *COPY* surface levels, don't re-generate them.
//-----------------------------------------------------------------------------
bool CompressNormalMap(LPDIRECT3DTEXTURE9& ptex)
{
    D3DSURFACE_DESC sd;
	LPDIRECT3DDEVICE9 pd3ddev = RenderWrap::dev;
	LPDIRECT3DSURFACE9 psurf;


    // If we have 3DC, use it!!
    if(RenderDevice::Instance()->Supports3DC())
    {
        TextureManager::ChangeFormat(ptex,(D3DFORMAT)FMT_3DC);
        // Fill mip levels
	    DXASSERT(D3DXFilterTexture(ptex, NULL, 0,  D3DX_DEFAULT));
        return true;
    }

	// Convert to A8R8G8B8
	TextureManager::ChangeFormat(ptex,D3DFMT_A8R8G8B8);

	// 1. Lock surface
	(*(LPDIRECT3DTEXTURE9*)&ptex)->GetSurfaceLevel(0,&psurf);
    psurf->GetDesc(&sd);
    D3DLOCKED_RECT lrSrc;
    DXASSERT(psurf->LockRect(&lrSrc, NULL, 0));

    DWORD xp;
    DWORD yp;
    DWORD* pdwRowSrc = (DWORD*)lrSrc.pBits;
    DWORD* pdwSrc;
    LONG dataBytesPerRow = 4 * sd.Width;

	bool ps2 = RenderDevice::Instance()->PixelShaderVersion >= 2;

    for (yp = 0; yp < sd.Height; yp++)
    {
        pdwSrc = pdwRowSrc;
        for (xp = 0; xp < sd.Width; xp++)
        {
			unsigned char a = COLOR_GETALPHA(*pdwSrc);
			unsigned char r = COLOR_GETRED(*pdwSrc);
			unsigned char g = COLOR_GETGREEN(*pdwSrc);
			unsigned char b = COLOR_GETBLUE(*pdwSrc);

			// ps2.0: Put spec (a) in b, as we'll calculate it on the card
            // TODO: Remove me, hurts compression
			if(ps2)
				*pdwSrc = COLOR_RGBA(0,g,a,r);
			// ps1.1: Do nothing right now
			else
				*pdwSrc = COLOR_RGBA(r,g,b,a);
	
            pdwSrc++;
        }
        pdwRowSrc += lrSrc.Pitch / 4;
    }

    psurf->UnlockRect();
	psurf->Release();

	// Convert to DXT5 with mips
	TextureManager::ChangeFormat(ptex,D3DFMT_DXT5);

	// Fill mip levels
	DXASSERT(D3DXFilterTexture(ptex, NULL, 0,  D3DX_DEFAULT));

	return true;
}

//-----------------------------------------------------------------------------
// Checks for alpha data embedded in RGBA image
//-----------------------------------------------------------------------------
#define ALPHA_NONE		0
#define ALPHA_SOLID		1
#define ALPHA_GRADIENT	2
int CheckForAlpha(LPDIRECT3DBASETEXTURE9 ptex)
{
    HRESULT hr;
    D3DSURFACE_DESC sd;
	LPDIRECT3DDEVICE9 pd3ddev = RenderWrap::dev;
    LPDIRECT3DSURFACE9 psurfTarget;
	LPDIRECT3DSURFACE9 psurf;
	bool HasAlpha = false;
	int  NumGreys = 0;
	
	(*(LPDIRECT3DTEXTURE9*)&ptex)->GetSurfaceLevel(0,&psurf);

    psurf->GetDesc(&sd);

    // Copy the target surface into an A8R8G8B8 surface
    hr = pd3ddev->CreateOffscreenPlainSurface(sd.Width, sd.Height, D3DFMT_A8R8G8B8, D3DPOOL_SCRATCH, &psurfTarget, NULL);
    hr = D3DXLoadSurfaceFromSurface(psurfTarget, NULL, NULL, psurf, NULL, NULL, 
        D3DX_FILTER_TRIANGLE, 0);

    // Fill in the alpha channels of psurfTarget based on the blue channel of psurfAlpha
    D3DLOCKED_RECT lrDest;

    hr = psurfTarget->LockRect(&lrDest, NULL, 0);

	
    DWORD xp;
    DWORD yp;
    DWORD* pdwRowDest = (DWORD*)lrDest.pBits;
    DWORD* pdwDest;
    LONG dataBytesPerRow = 4 * sd.Width;

    for (yp = 0; yp < sd.Height; yp++)
    {
        pdwDest = pdwRowDest;
        for (xp = 0; xp < sd.Width; xp++)
        {
			// Check to see if the source has any alpha
			DWORD a = COLOR_GETALPHA(*pdwDest);
			if(a != 255)
				HasAlpha = true;

			if(a > 0 && a < 255){
				NumGreys++;
			}

            pdwDest++;
        }
        pdwRowDest += lrDest.Pitch / 4;
    }

    psurfTarget->UnlockRect();

    // Release allocated interfaces
    SAFE_RELEASE(psurfTarget);

	psurf->Release();

	if(NumGreys > 300)
		return ALPHA_GRADIENT;
	else if(HasAlpha)
		return ALPHA_SOLID;
	else
		return ALPHA_NONE;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void TextureManager::AddAnimatedTexture(Texture* tex){
	AnimatedTextures.push_back(tex);
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void TextureManager::RemoveAnimatedTexture(Texture* tex){
	vector_erase(AnimatedTextures,tex);
}

//-----------------------------------------------------------------------------
// Name: CheckDDSCache
// Desc: Modifies 'file' to point to a dds-cached version if one exists and is up-to-date
// Returns true if a cached version was found
//-----------------------------------------------------------------------------
bool TextureManager::CheckDDSCache(string& file, Texture* texture, bool originalExists){
	// We won't cache dds files. But users must be careful not to expect meta-data to be added
	if(file.find(_U(".dds")) != -1)
		return false;

	string fileName = file;

	// Strip path. Store just the file name in fileName
	if(fileName.find(_U("\\"))!=-1)
		fileName = fileName.substr(fileName.find_last_of(_U("\\"))+1);
	else if(fileName.find(_U("/"))!=-1)
		fileName = fileName.substr(fileName.find_last_of(_U("/"))+1);
	
	string path = m_CachePath + fileName.substr(0,fileName.find_last_of(_U("."))) + _U(".dds");

	bool standardFile = FileExists(path);
	if(!standardFile)
		return false; // no dds version exists in ddscache

	//-----------------------------------------------------------------
	// Compare the dates
	//-----------------------------------------------------------------
	if(originalExists){
		WIN32_FILE_ATTRIBUTE_DATA fxCompiled;
		WIN32_FILE_ATTRIBUTE_DATA fxUncompiled;
		GetFileAttributesEx( path.c_str(), GetFileExInfoStandard, &fxCompiled);
		GetFileAttributesEx( file.c_str(), GetFileExInfoStandard, &fxUncompiled);

		if(CompareFileTime(&fxCompiled.ftLastWriteTime,&fxUncompiled.ftLastWriteTime) == -1)
			return false; // DDS file is out of date, so don't use it
	}

	//-----------------------------------------------------------------
	// Now, let's check the meta-data to see if this file is up-to-date
	// Meta-data: alpha_name, bump_height, compressedBump
	//-----------------------------------------------------------------
	string name = fileName.substr(0,fileName.find_last_of(_U("."))) + _U(".dds");

	// If this texture isn't in the dds config, force a resave.
	// We must do a resave of the dds as well as config, because
	// we have no way of telling whether it needs resaving!
	if(!cache.KeyExists(name + "_CompressedBump"))
		return false; 

	bool compressedBump = cache.GetBool(name + "_CompressedBump");

	if(texture->type == TT_NORMALMAP && cache.GetBool(name + "_CompressedBump") != CompressNormalMaps)
		return false; // Cached normal map is out of date

	//-----------------------------------------------------------------
	// DDS version is newer than uncompiled file & there is no new meta-data, so use it
	//-----------------------------------------------------------------
	file = path;
	return true; 
}


//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void TextureManager::UpdateAnimatedTextures(){

	// FIXME: HACK FOR MOVIE TEXTURES
	// FIXME: PUT MOVIE TEXTURES IN THE ANIMATED ARRAY
	for(int i=0;i<textures.size();i++)
	{
		if(textures[i].m_Movie)
			textures[i].m_Movie->CheckMovieStatus();
	}

	static float time = 0;
	const float FRAME_TIME = FrameTime;
	time += GDeltaTime;
	if(time > FRAME_TIME){
		for(int i=0;i < AnimatedTextures.size();i++){
			Texture* t =  AnimatedTextures[i];

			// If randomized, dig up random frame
			if(t->isRandomized && t->texIDs.size() > 1){
				int oldValue = t->CurrentFrame;
				// Keep generating random frames until we get one that is different
				while(t->CurrentFrame == oldValue)
					t->CurrentFrame = rand()%t->texIDs.size();
			}
			else{
				// Normal animation, cycle frame
				t->CurrentFrame++;
				if(t->CurrentFrame >= t->texIDs.size())
					t->CurrentFrame = 0;
			}
		}
		time = 0; 
	}
}


//-----------------------------------------------------------------------------
// Name: Free
// Desc: Releases single texture
//-----------------------------------------------------------------------------
void TextureManager::Free(int texID){
	if(texID<textures.size() && textures[texID].ref == 0)
	{
		// HACK: Not checking ref count, because id3dxeffects or d3ddevice holds on to texture after used in scene
		// unless we specifically unset it
		if(textures[texID].m_Movie)
			textures[texID].m_Movie->m_pTexture = NULL;

		SAFE_RELEASE(textures[texID].tex);
		
		//SAFE_RELEASE(textures[texID].m_Movie);
		
	}
	if(textures.size() > texID)
		textures[texID].ref --;
}

//-----------------------------------------------------------------------------
// Name: Destroy
// Desc: Tells system to NEVER build this texture again, it's gone
// (but kept in the array so our indexes still fit)
//-----------------------------------------------------------------------------
void TextureManager::Destroy(int texID, Texture* owner){
	Free(texID);

	if(!textures.size())
		return;

	// Delete this owner
	for(int x=0;x<10;x++){
		if(textures[texID].otherOwners[x] == owner){
			textures[texID].otherOwners[x] = 0;
			break;
		}
	}


	if(textures[texID].ref < 0) // No owners left
		textures[texID].texture = 0;
	else {
		textures[texID].texture = 0;
		// Set new owner
		for(int x=0;x<10;x++){
			if(textures[texID].otherOwners[x] != 0){
				textures[texID].texture = textures[texID].otherOwners[x];
				break;
			}
		}
	}
}


//-----------------------------------------------------------------------------
// Name: FreeAll
// Desc: Releases all textures
// NOTE: We don't *NEED* this if textures all cleaned themselves up nicely,
// but we can't trust them or Jer to do that ;-)
//-----------------------------------------------------------------------------
void TextureManager::FreeAll(){
	textureMemory = 0;

	for(int i=0;i<textures.size();i++){
		// If this asserts, check that all shaders, etc have been invalidated and deleted FIRST
		// as they hold references to the texture until deletion
		//if(textures[i].m_Movie)
		//	textures[i].m_Movie->m_pTexture = NULL;

		SAFE_RELEASE(textures[i].tex);
	}
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void TextureManager::OnLostDevice(){
	for(int i=0;i<textures.size();i++)
	{
		if(textures[i].m_Movie)
			textures[i].m_Movie->OnLostDevice();
	}
}

//-----------------------------------------------------------------------------
// Name: RebuildAll
// Desc: Loads all textures into video memory. Called on device restore
// NOTE: Commented out because textures are all pool_managed, and restore themselves
//-----------------------------------------------------------------------------
void TextureManager::RestoreAll(){

	for(int i=0;i<textures.size();i++)
	{
		if(textures[i].m_Movie)
 			textures[i].m_Movie->Restore();
	}
	//textureMemory = 0;
/*	vector<TexInfo>::iterator p = textures.begin();
	int n =0;
	while(p != textures.end()){
		if(p->tex == 0 && p->texture)
			CreateSurface(&(*p));
		p++;
	}*/
}

//-----------------------------------------------------------------------------
// Name: CheckRuntimeCache
// Desc: Scans currently loaded textures to see if we've already loaded this one
//-----------------------------------------------------------------------------
int TextureManager::CheckRuntimeCache(string& name,Texture* texture){
	// Get index of actual name, path stripped
	int i1 = name.find_last_of(_U("/"));
	int i2 = name.find_last_of(_U("\\"));
	int i3 = (i1>i2?i1:i2);
	if(i3 == -1)
		i3 = 0;
	else
		i3++; // so we go one ahead of /

	string stripped_name = name.substr(i3);
	// Remove the .ext too, because it might be a cached dds now
	stripped_name = stripped_name.substr(0,stripped_name.rfind(_U("."))+1);
	ToLowerCase(stripped_name);


	for(int i=0;i<textures.size();i++){
		TexInfo& p = textures[i];

		if(!p.texture || p.ref < 0)
			continue; // Skip dead textures

		if(p.texture->type != texture->type)
			continue; // Skip different types (ie bump vs diffuse)

		// FIXME: Replace with references
		int fs = p.texture->filename.rfind(_U("/"));
		int ff = p.texture->filename.rfind(_U("\\"));
		if(ff > fs)
			fs = ff;

		string temp = p.texture->filename;
		ToLowerCase(temp);

		int index = temp.rfind(stripped_name);

		if(index != fs + 1) // Check that it's \strippedname, and not *strippedname
			continue; 

		if(index != string::npos){

			if(!p.tex && RenderWrap::dev)
				return -1;

			texture->format =   p.texture->format;
			texture->height =   p.texture->height;
			texture->width   =  p.texture->width;
			texture->depth   =  p.texture->depth;
			texture->type	  = p.texture->type;
			texture->filename = p.texture->filename;

			// Hacky code to keep a list of texture owners
			// Add to owner list
			for(int x=0;x<10;x++){
				if(p.otherOwners[x] == 0){
					p.otherOwners[x] = texture;
					break;
				}
			}

			p.ref++;// Add reference for refernece counting

			return i;
		}
	}

	return -1;
}



//-----------------------------------------------------------------------------
// Name: Load
// Desc: Loads a texture into the manager
//-----------------------------------------------------------------------------
int TextureManager::Load(Texture* texture){
	LogPrintf(LOG_HIGH,_U("Loading texture '%s'"),texture->filename.c_str());

	// Make texture entry
	TexInfo ti;
	ti.otherOwners[0] = texture;
	ti.tex = 0;
	ti.texture = texture;
	ti.ref = 0;

	// Take our place in the array early, because the below function might push another texture on
	int place = textures.size();
	textures.push_back(ti);

	// Create the texture surface
	CreateSurface(&ti);

	// Refill our place with the current data
	textures[place] = ti;

	return place;
}


//-----------------------------------------------------------------------------
// Name: FindTexture
// Desc: Finds a texture ID given a name
//-----------------------------------------------------------------------------
int TextureManager::FindTexture(string fileName, TextureType type){
	for(int i=0;i<textures.size();i++){
		TexInfo& p = textures[i];

		if(!p.texture || p.ref < 0)
			continue; // Skip dead textures

		if(p.texture->type != type)
			continue; // Skip different types (ie bump vs diffuse)

		if(p.texture->filename == fileName){
			return i;
		}
	}
	return -1;
}



//-----------------------------------------------------------------------------
// Name: UpdateAllLODs
// Desc: Updates texture LODs
//-----------------------------------------------------------------------------
void TextureManager::UpdateAllLODs()
{
    if(TextureSizePercent == 0)
        TextureSizePercent = 1;
	for(int i=0;i<textures.size();i++){
		TexInfo& tInfo = textures[i];
		if(!tInfo.tex || !tInfo.texture->usesLOD)
            continue;

		D3DSURFACE_DESC desc;
		((LPDIRECT3DTEXTURE9)(tInfo.tex))->GetLevelDesc(0,&desc);

		// Set the maximum mip level if the texture is too large
		if(desc.Width * desc.Height > 65536){
			int LOD = (100 / TextureSizePercent) - 1;
            // LOD can never be above GetLevelCount()
            if(LOD > ((LPDIRECT3DTEXTURE9)(tInfo.tex))->GetLevelCount()-1)
                LOD = ((LPDIRECT3DTEXTURE9)(tInfo.tex))->GetLevelCount()-1;
			tInfo.tex->SetLOD(LOD); // 0 = 100% 1 = 50%, etc
		}
	}
}


//-----------------------------------------------------------------------------
// Name: ChangeFormat
// Desc: Changes a texture's format
//-----------------------------------------------------------------------------
void TextureManager::ChangeFormat(LPDIRECT3DTEXTURE9& tex, D3DFORMAT format, int mips /*=0*/){
	D3DSURFACE_DESC desc;
	tex->GetLevelDesc(0,&desc);

	// Make new surface
	LPDIRECT3DTEXTURE9 pSurfTarget;
	HRESULT hr;
	if((hr = D3DXCreateTexture(RenderWrap::dev,desc.Width,desc.Height,mips,0,format,D3DPOOL_MANAGED,&pSurfTarget)) != S_OK)
		Warning(_U("ChangeFormat[D3DXCreateTexture] failed: \nD3DErr: %s"), DXGetErrorString9(hr));

	LPDIRECT3DSURFACE9 psurf;
	LPDIRECT3DSURFACE9 psurf2;
	tex->GetSurfaceLevel(0,&psurf);
	pSurfTarget->GetSurfaceLevel(0,&psurf2);

	// Copy the old texture to it
	if((hr = D3DXLoadSurfaceFromSurface(psurf2, NULL, NULL, psurf, NULL, NULL, D3DX_FILTER_BOX, 0)) != S_OK)
		Warning(_U("ChangeFormat[D3DXLoadSurfaceFromSurface] failed: \nD3DErr: %s"), DXGetErrorString9(hr));

	// Release the old, in with the new
	psurf2->Release();
	psurf->Release();
	SAFE_RELEASE(tex);
	tex = pSurfTarget;
}

//-----------------------------------------------------------------------------
// Name: Compress
// Desc: Compresses a D3D texture surface
//
//-----------------------------------------------------------------------------
LPDIRECT3DTEXTURE9 pCurrentTexture = 0; 
HRESULT LoadAllMipSurfaces(void * data, int iLevel, DWORD size)
{
    HRESULT hr;
    LPDIRECT3DSURFACE9 psurf;
    D3DSURFACE_DESC sd;
    D3DLOCKED_RECT lr;
       
    hr = pCurrentTexture->GetSurfaceLevel(iLevel, &psurf);
    
    if (FAILED(hr))
        return hr;
    psurf->GetDesc(&sd);
    
    
    hr = pCurrentTexture->LockRect(iLevel, &lr, NULL, 0);
    if (FAILED(hr))
        return hr;

	assert(size == sd.Width*sd.Height);
    
	memcpy(lr.pBits, data, sd.Width*sd.Height);
    
    hr = pCurrentTexture->UnlockRect(iLevel);
    
    SAFE_RELEASE(psurf);
    
    return 0;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void HeightmapToArray(const char* name, BYTE*& array, int& width, int& height);
/*void TextureManager::Compress(LPDIRECT3DTEXTURE9 tex, TexInfo* tInfo, D3DSURFACE_DESC& desc){
	// Convert to raw data
	BYTE* raw_data;
	INT w,h;
	HeightmapToArray(tInfo->texture->filename.c_str(),raw_data,w,h);

	CompressionOptions options;
    options.TextureFormat = dDXT5;

	
    DXASSERT(D3DXCreateTexture(RenderWrap::dev, desc.Width, desc.Height, 0,  0,   D3DFMT_DXT5,  D3DPOOL_MANAGED, &tex));
	//nvDXTcompress((unsigned char *)raw_data, desc.Width, desc.Height, 1, &options, 4, LoadAllMipSurfaces);
}*/

//-----------------------------------------------------------------------------
// Name: CreateSurface
// Desc: Creates a D3D texture surface
// This is a greyscale height map that we should convert to an RGB normal map
// TODO !!!!!!!!!!!!!!!!!!!!!!!
// Replace with nvidia command-line normalmapgen function
// Figure out whether to box filter mips, or build mips from src heightmap
// The latter causes exaggerated normals on angles unless anisotropy is high
// when using D3DXComputeNormalMap
//
// Perhaps the NVIDIA implementation won't have this problem
//
// TODO: Encode normalization factor in one of the extra channels
//-----------------------------------------------------------------------------
/*bool TextureManager::CreateNormalMap(TexInfo* tInfo, D3DSURFACE_DESC& desc){
	// -5.0f is the constant factor which maps almost perfectly to max's height system at 100% [With D3DXComputeNormalMap]
	float amplitude = tInfo->texture->m_Amount * 0.60f;

	// Figure out the bump format
	D3DFORMAT format = D3DFMT_A8R8G8B8;
	// DXT5 if we can
	if(Engine::Instance()->MainConfig->GetBool("CompressNormalMaps")){
		format = D3DFMT_DXT5;
	}

	// Create an uncompressed target for the initial normal map
	// Only 1 mip level to save time
	LPDIRECT3DTEXTURE9 normalMap;
	DXASSERT(D3DXCreateTexture(RenderWrap::dev,desc.Width,desc.Height,0,0,D3DFMT_A8R8G8B8,D3DPOOL_MANAGED,&normalMap));

	bool swizzle = false;
	// Fill target with bump
	if(Engine::Instance()->RenderSys->PixelShaderVersion >= 2 && format == D3DFMT_DXT5){ // Only do the swizzle DXT5 on ps2.0
		GenerateBumpMap(tInfo->texture->filename.c_str(),normalMap,format,amplitude);
		swizzle = true;
	}
	else // D3DFMT_DXT1 | R8G8B8 (they are same bump type)
		GenerateBumpMap(tInfo->texture->filename.c_str(),normalMap,D3DFMT_DXT1,amplitude);

	//DXASSERT(D3DXCreateTexture(RenderWrap::dev,desc.Width,desc.Height,0,0,D3DFMT_A8R8G8B8,D3DPOOL_MANAGED,(LPDIRECT3DTEXTURE9*)&tInfo->tex));
	 //DXASSERT(D3DXComputeNormalMap(normalMap,(LPDIRECT3DTEXTURE9)tInfo->tex,NULL,D3DX_NORMALMAP_COMPUTE_OCCLUSION,D3DX_CHANNEL_RED,amplitude));

	// Compress
	//Compress(normalMap,tInfo,desc);
	if(format != D3DFMT_A8R8G8B8)
		ChangeFormat(normalMap,format);

	// Delete input heightmap and replace with normal map
	SAFE_RELEASE(tInfo->tex);
	tInfo->tex = (LPDIRECT3DBASETEXTURE9)normalMap;

	return swizzle; // Swizzled?
}*/


//-----------------------------------------------------------------------------
// Name: CreateSurface
// Desc: Creates a D3D texture surface
//-----------------------------------------------------------------------------
void TextureManager::CreateSurface(TexInfo* tInfo){
	if(!RenderWrap::dev){
		// We used to support this, but no point since it never occurs any more
		Error(_U("Texture '%s' created before initialization"),tInfo->texture->filename.c_str());
		tInfo->tex = 0;
		return;
	}

	if(!tInfo->texture)
		Error(_U("This should never happen."));

	if(tInfo->texture->filename == _U("")){
		// We must create a new surface
		CreateNewTexture(tInfo);
		return;
	}

	
	// Remove case, for comparisons
	string filename		= AsLower(tInfo->texture->filename);
	// DDS are special-case
	bool isDDS			= filename.find(_U(".dds")) != -1;
	// The path wouldn't contain Cache unless CheckDDSCache() pointed it there
	// Cache is off-limits for normal textures
	bool alreadyCached	= filename.find(AsLower(m_CachePath)) != -1;
	
	bool bCacheTexture;
	// Main loading routine
	LoadTexture(*(LPDIRECT3DTEXTURE9*)&tInfo->tex,tInfo,filename.c_str(),bCacheTexture);
	if(!tInfo->tex){
		Warning(_U("Texture load failed for '%s'. File may be corrupt!"),filename.c_str());
		return;
	}

	// Compress normal maps
	if(!alreadyCached && tInfo->tex->GetType() == D3DRTYPE_TEXTURE && CompressNormalMaps && tInfo->texture->type == TT_NORMALMAP)
    {
		CompressNormalMap(*(LPDIRECT3DTEXTURE9*)&tInfo->tex);
		bCacheTexture = true;
	}


    if(TextureSizePercent == 0)
        TextureSizePercent = 1;

	D3DSURFACE_DESC desc;
	if(tInfo->tex) ((LPDIRECT3DTEXTURE9)(tInfo->tex))->GetLevelDesc(0,&desc);
	if(tInfo->texture->usesLOD)
	{
	// Set the maximum mip level if the texture is too large
		float fTextureSizePercent = 100;
		if(desc.Width * desc.Height > 65536){
			fTextureSizePercent = float(TextureSizePercent) / 100.f;
			int LOD = (100 / TextureSizePercent) - 1;
            // LOD can never be above GetLevelCount()
            if(LOD > ((LPDIRECT3DTEXTURE9)(tInfo->tex))->GetLevelCount()-1)
                LOD = ((LPDIRECT3DTEXTURE9)(tInfo->tex))->GetLevelCount()-1;
			tInfo->tex->SetLOD(LOD); // 0 = 100% 1 = 50%, etc
		}
	}

	// Set texture attributes
	tInfo->texture->width = desc.Width;
	tInfo->texture->height = desc.Height;

	if(bCacheTexture && Engine::Instance()->MainConfig->GetBool("CacheTextures")){
		CacheTextureToDisk(filename,tInfo);
	}

    if(!tInfo->texture->dontCache)
        tInfo->tex->PreLoad();
}


//-----------------------------------------------------------------------------
// Name: CacheTextureToDisk
// Desc: Saves a texture out to DDS file for fast loading next time
//-----------------------------------------------------------------------------
void TextureManager::CacheTextureToDisk(string fileName, TexInfo* tInfo){
	LogPrintf(LOG_HIGH,_U("Caching texture '%s' to DDSCache"),fileName.c_str());

	// Strip away path
	if(fileName.find(_U("\\"))!=-1)
		fileName = fileName.substr(fileName.find_last_of(_U("\\"))+1);
	else if(fileName.find(_U("/"))!=-1)
		fileName = fileName.substr(fileName.find_last_of(_U("/"))+1);

	// Replace extension with .dds
	string cachedName = m_CachePath;
	cachedName += fileName.substr(0,fileName.find_last_of(_U(".")));
	cachedName += _U(".dds");

	//--------------------------------------------------------
	// Store the meta-data too
	//--------------------------------------------------------
	string name = fileName.substr(0,fileName.find_last_of(_U("."))) + _U(".dds");
	cache.SetBool(name + "_CompressedBump",CompressNormalMaps && tInfo->texture->type == TT_NORMALMAP);

	//--------------------------------------------------------
	// If requested, save without mips 
	// (to save zip space when preparing an internet version)
	//--------------------------------------------------------
	HRESULT hr;
	if(!SaveMips){
		D3DSURFACE_DESC desc;
		((LPDIRECT3DTEXTURE9)tInfo->tex)->GetLevelDesc(0,&desc);
		// Make new surface
		LPDIRECT3DTEXTURE9 noMips;
		if((hr = D3DXCreateTexture(RenderWrap::dev,desc.Width,desc.Height,1,0,desc.Format,D3DPOOL_MANAGED,&noMips)) != S_OK)
			Warning(_U("CacheTextureToDisk[D3DXCreateTexture] failed: \nD3DErr: %s"), DXGetErrorString9(hr));
		
		LPDIRECT3DSURFACE9 source, dest;
		((LPDIRECT3DTEXTURE9)tInfo->tex)->GetSurfaceLevel(0,&source);
		noMips->GetSurfaceLevel(0,&dest);
		// Copy the old texture to it
		if((hr = D3DXLoadSurfaceFromSurface(dest, NULL, NULL, source, NULL, NULL, D3DX_FILTER_BOX, 0)) != S_OK)
			Warning(_U("CacheTextureToDisk[D3DXLoadSurfaceFromSurface] failed: \nD3DErr: %s"), DXGetErrorString9(hr));
		
		// Save mipless
		if((hr=D3DXSaveTextureToFile(cachedName.c_str(),D3DXIFF_DDS ,noMips,NULL)) != S_OK){
			Warning(_U("Trying to cache the texture \"%s\" failed with: %s"),cachedName.c_str(),DXGetErrorString9(hr));
		}

		// Release the old, in with the new
		source->Release();
		dest->Release();
		noMips->Release();
		return;
	}

	
	if((hr=D3DXSaveTextureToFile(cachedName.c_str(),D3DXIFF_DDS ,tInfo->tex,NULL)) != S_OK){
		Warning(_U("Trying to cache the texture \"%s\" failed with: %s"),cachedName.c_str(),DXGetErrorString9(hr));
	}
}


//--------------------------
// New texture creation
//--------------------------
void TextureManager::CreateNewTexture(TexInfo* tInfo){
	UINT m_dwHeight = tInfo->texture->height;
	UINT m_dwWidth  = tInfo->texture->width;
	UINT m_dwDepth  = tInfo->texture->depth;
	D3DFORMAT m_Format = (D3DFORMAT)tInfo->texture->format;
	LPDIRECT3DDEVICE9 m_pd3dDevice = RenderWrap::dev;
	HRESULT hr;

    switch(tInfo->texture->type){
            case TT_DIFFUSE:
                {
                    if ( FAILED( hr = D3DXCheckTextureRequirements(m_pd3dDevice, &m_dwWidth, 
                        &m_dwHeight, 0, 0, &m_Format, POOL_TYPE) ) )
                    {
                        SeriousWarning(_U("Can't find valid texture format: %s.\n"),DXGetErrorString9(hr));
                        return;
                    }

                    if (FAILED (hr = m_pd3dDevice->CreateTexture(m_dwWidth, m_dwHeight,
                        0, 0, m_Format, POOL_TYPE, (LPDIRECT3DTEXTURE9*)&tInfo->tex,0) ) )
                    {
                        SeriousWarning(_U("Can't create texture: %s\n"),DXGetErrorString9(hr));
                        return;
                    }
                }
                break;
            case TT_VOLUME:
                {
                    UINT levels = 0;
                    if ( FAILED( hr = D3DXCheckVolumeTextureRequirements(m_pd3dDevice, &m_dwWidth, 
                        &m_dwHeight, &m_dwDepth, &levels, 0, &m_Format, POOL_TYPE) ) )
                    {
                        SeriousWarning(_U("Can't find valid volume texture format: %s.\n"),DXGetErrorString9(hr));
                        return;
                    }

                    if (FAILED (hr = m_pd3dDevice->CreateVolumeTexture(m_dwWidth, m_dwHeight, m_dwDepth,
                        levels, 0, m_Format, POOL_TYPE, (LPDIRECT3DVOLUMETEXTURE9*)&tInfo->tex,0) ) )
                    {
                        SeriousWarning(_U("Can't create volume texture: %s\n"),DXGetErrorString9(hr));
                        return;
                    }

                    // Any reason to do so?
                    //D3DXFilterTexture(tInfo->tex,NULL, D3DX_DEFAULT , D3DX_DEFAULT );
                }
                break;
            case TT_CUBE:
                {
                    if ( FAILED( hr = D3DXCheckCubeTextureRequirements(m_pd3dDevice, &m_dwWidth, 
                        0, 0, &m_Format, POOL_TYPE) ) )
                    {
                        SeriousWarning(_U("Can't find valid cube texture format: %s\n"),DXGetErrorString9(hr));
                        return;
                    }

                    if (FAILED (hr = m_pd3dDevice->CreateCubeTexture(m_dwWidth,
                        0, 0, m_Format, POOL_TYPE, (LPDIRECT3DCUBETEXTURE9*)&tInfo->tex,0) ) )
                    {
                        SeriousWarning(_U("Can't create cube texture: %s\n"),DXGetErrorString9(hr));
                        return;
                    }
                }
                break;

    }

}


//-----------------------------------------------------------------------------
// Name: LoadTexture
// Desc: Loads a texture from file
//-----------------------------------------------------------------------------
void TextureManager::LoadTexture(LPDIRECT3DTEXTURE9& tex, TexInfo* t, const CHAR* name, bool& cacheIt){
	HRESULT hr;

	TextureType type = t->texture->type;

	// Direct DDS load. Don't touch the format
	if(string(name).find(_U(".dds")) !=-1){
		cacheIt = false;
		// Get the info to see if we need to resave the dds
		D3DXIMAGE_INFO info;
		D3DXGetImageInfoFromFile(name,&info);
		if(info.MipLevels <= 1) 
			cacheIt = true; // DDS has no mips. We need to resave with the full mip chain

		if(!SaveMips) 
			cacheIt = true; // Force a resave without mips

		// Figure out format and load
		// This'll generate mips if not already present
		if(info.ResourceType == D3DRTYPE_TEXTURE){
			if((hr=D3DXCreateTextureFromFile(RenderWrap::dev, name, &tex)) != S_OK){
				Warning(_U("Couldn't create compressed(DDS) texture: %s  (D3DErr: %s)"), name,DXGetErrorString9(hr));
				return;
			}
		}
		else if(info.ResourceType == D3DRTYPE_VOLUMETEXTURE){
			type = TT_VOLUME;
			if(D3DXCreateVolumeTextureFromFile(RenderWrap::dev,name,(LPDIRECT3DVOLUMETEXTURE9*)&tex)!=S_OK){
				Warning(_U("Couldn't create VOLUME texture: %s"), name);
				return;
		 }
		}
		else if(info.ResourceType == D3DRTYPE_CUBETEXTURE){
			type = TT_CUBE;
			if(D3DXCreateCubeTextureFromFile(RenderWrap::dev,name,(LPDIRECT3DCUBETEXTURE9*)&tex)!=S_OK){
				Warning(_U("Couldn't create CUBE texture: %s"), name);
				return;
		 }
		}
	}
	// Video texture
	else if(string(name).find(".avi") != -1 || string(name).find(".mpg") != -1 || string(name).find(".wmv") != -1 || string(name).find(".mov") != -1)
	{
		CTextureRenderer* pCTR;
		if(SUCCEEDED(CTextureRenderer::InitDShowTextureRenderer((char*)name,pCTR)))
        {
		    t->m_Movie = pCTR;
		    tex = pCTR->m_pTexture;
		    cacheIt = false;
        }
        else
            return;
	}
	// Some other format (tga,bmp,png,etc), let's figure out the requirements..
	else{ 
		cacheIt = true;

		// Create the texture UNCOMPRESSED with requested parameters
		// Uncompressed allows us to examine the content to decide the best compression
		if((hr=D3DXCreateTextureFromFileEx( RenderWrap::dev, name, 
			D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT , 0, D3DFMT_A8R8G8B8, 
				POOL_TYPE, D3DX_DEFAULT|SRGB_FILTER, // This triangle filters the source texture, I guess in case it's not pow2
				D3DX_DEFAULT, 0, NULL, NULL, &tex ))!=S_OK)
		{
			Warning(_U("Couldn't create texture: %s (D3DErr: %s)"), name,DXGetErrorString9(hr));
			return;
		}

		// We don't compress normal maps here
		bool bCompress = (type != TT_NORMALMAP);
		if(bCompress){
			// Figure out format based on alpha
			// DXT1 -- When alpha map is on/off (1 bit)
			// DXT3 -- When alpha map is crisp
			// DXT5 -- When alpha map is smooth
			D3DFORMAT fmt = D3DFMT_DXT1; // Default

			int ret = CheckForAlpha(tex);
			if(ret == ALPHA_GRADIENT)
				fmt = D3DFMT_DXT5;
			else if(ret == ALPHA_SOLID)
				fmt = D3DFMT_DXT3;

			// Release and reload
			SAFE_RELEASE(tex);
			if((hr=D3DXCreateTextureFromFileEx( RenderWrap::dev, name, D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT , 0, fmt , 
				POOL_TYPE, D3DX_DEFAULT|SRGB_FILTER, D3DX_DEFAULT, 0, NULL, NULL, &tex ))!=S_OK)
			{
				Warning(_U("Couldn't create texture: %s (D3DErr: %s)"), name,DXGetErrorString9(hr));
			}
		}
	}
}





